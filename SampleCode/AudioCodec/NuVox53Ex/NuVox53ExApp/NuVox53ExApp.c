#include <string.h>
#include "NuVox53ExApp.h"
#include "AudioRom.h"
#include "PlaybackRecord.h"

extern S_AUDIO_CALLBACK g_asAppCallBack[];

//---------------------------------------------------------------------------------------------------------
void NuVox53ExApp_DecodeInitiate(S_NUVOX53EX_APP *psNuVox53ExApp, UINT8 *pau8TempBuf, UINT32 u32CallbackIndex)
{
	memset( psNuVox53ExApp, '\0', sizeof(S_NUVOX53EX_APP) );
	// NuVox53Ex decoder will refer to this index and call which callback function of storage 
	psNuVox53ExApp->u8CallbackIndex = (UINT8)u32CallbackIndex;
	// Input temp buffer to provide library used.
	psNuVox53ExApp->pau8TempBuf = pau8TempBuf;
	BUF_CTRL_SET_INACTIVE(&psNuVox53ExApp->sOutBufCtrl);
}

//---------------------------------------------------------------------------------------------------------
BOOL NuVox53ExApp_DecodeStartPlayByID(S_NUVOX53EX_APP *psNuVox53ExApp, UINT32 u32AudioID, UINT32 u32RomStartAddr, UINT8 u8PlaybackChannel)
{
	S_ROM_AUDIO_CHUNK_INFO sAudioChunkInfo;
	
	// Get audio chunk information for audio chunk start address from ROM file.
	AudioRom_GetAudioChunkInfo( g_asAppCallBack[psNuVox53ExApp->u8CallbackIndex].pfnReadDataCallback, 
								u32RomStartAddr, 
								u32AudioID, 
								&sAudioChunkInfo);	

	return NuVox53ExApp_DecodeStartPlayByAddr(psNuVox53ExApp, sAudioChunkInfo.u32AudioChunkAddr, u8PlaybackChannel);
}

//---------------------------------------------------------------------------------------------------------
BOOL NuVox53ExApp_DecodeStartPlayByAddr(S_NUVOX53EX_APP *psNuVox53ExApp, UINT32 u32NuVox53ExStorageStartAddr, UINT8 u8PlaybackChannel)
{
	UINT16 u16SampleRate;
	
	// NuVox53Ex decoder initiates work buffer and returns sample rate.
	if ( (u16SampleRate = NuVox53Ex_DecodeInitiate(	(UINT8*)psNuVox53ExApp->au32DecodeWorkBuf, 
													psNuVox53ExApp->pau8TempBuf, 
													u32NuVox53ExStorageStartAddr, 
													g_asAppCallBack[psNuVox53ExApp->u8CallbackIndex].pfnReadDataCallback )) == 0 )
		return FALSE;	
	
	// Initiate and set output buffer variable(include frame size, buffer size etc.) 
	Playback_SetOutputBuf( 	&psNuVox53ExApp->sOutBufCtrl,
							NUVOX53EXAPP_OUT_BUF_SIZE,
							psNuVox53ExApp->i16OutBuf,
							NUVOX53EXAPP_OUT_SAMPLES_PER_FRAME,
							u16SampleRate );

	// Trigger active flag of output buffer for NuVox53Ex decoding
	BUF_CTRL_SET_ACTIVE(&psNuVox53ExApp->sOutBufCtrl);
	
	// Pre-decode one frame
	psNuVox53ExApp->sOutBufCtrl.u16BufWriteIdx = NUVOX53EXAPP_OUT_SAMPLES_PER_FRAME;
	if ( NuVox53ExApp_DecodeProcess(psNuVox53ExApp) == FALSE )
	{
		BUF_CTRL_SET_INACTIVE(&psNuVox53ExApp->sOutBufCtrl);
		return FALSE;
	}
	psNuVox53ExApp->sOutBufCtrl.u16BufReadIdx = NUVOX53EXAPP_OUT_SAMPLES_PER_FRAME;
	
	// Record play channel index for stopping to play.
	psNuVox53ExApp->u8PlaybackChannel = u8PlaybackChannel;
	// Add audio codec into channel and preper to play codec.
	Playback_Add(psNuVox53ExApp->u8PlaybackChannel, &psNuVox53ExApp->sOutBufCtrl);

	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
void NuVox53ExApp_DecodeStopPlay(S_NUVOX53EX_APP *psNuVox53ExApp)
{
	// Clear active flag of output buffer for stoping NuVox53Ex decode.
	BUF_CTRL_SET_INACTIVE(&psNuVox53ExApp->sOutBufCtrl);
	// Remove audio codec output buffer from play channel.
	Playback_Remove(psNuVox53ExApp->u8PlaybackChannel);
}

//---------------------------------------------------------------------------------------------------------
BOOL NuVox53ExApp_DecodeProcess(S_NUVOX53EX_APP *psNuVox53ExApp)
{
	INT16 *pi16OutBuf;
	
	if (BUF_CTRL_IS_INACTIVE(&psNuVox53ExApp->sOutBufCtrl))
		return FALSE;
	
	if ( Playback_NeedUpdateOutputBuf(&psNuVox53ExApp->sOutBufCtrl) )
	{
		// Check end of file
		if(NuVox53Ex_DecodeIsEnd((UINT8*)psNuVox53ExApp->au32DecodeWorkBuf))
		{
			// Trigger inactive flag of output buffer to stop NuVox53Ex decoding
			BUF_CTRL_SET_INACTIVE(&psNuVox53ExApp->sOutBufCtrl);
			// Use to represnt no active(or end) of decoding
			psNuVox53ExApp->sOutBufCtrl.u16SampleRate = 0;
			return FALSE;
		}

		// Record output data buffer pointer(for duplicate & process)
		pi16OutBuf = (PINT16)&psNuVox53ExApp->sOutBufCtrl.pi16Buf[psNuVox53ExApp->sOutBufCtrl.u16BufWriteIdx];
		
		NuVox53Ex_DecodeProcess((UINT8*)psNuVox53ExApp->au32DecodeWorkBuf, 
								psNuVox53ExApp->pau8TempBuf, 
								pi16OutBuf, 
								g_asAppCallBack[psNuVox53ExApp->u8CallbackIndex].pfnReadDataCallback, 
								g_asAppCallBack[psNuVox53ExApp->u8CallbackIndex].pfnUserEventCallback);

		// Update write index of output buffer and avoid buffer overflow
		Playback_UpdateOutputBuf(&psNuVox53ExApp->sOutBufCtrl);	
		
		// Duplicate data into buffer for using duplication callback function.
		if ( psNuVox53ExApp->u8CtrlFlag&(NUVOX53EXAPP_CTRL_DUPLICATE_TO_BUF|NUVOX53EXAPP_CTRL_DUPLICATE_TO_FUNC) )
		{
			if ( psNuVox53ExApp->u8CtrlFlag & NUVOX53EXAPP_CTRL_DUPLICATE_TO_BUF )
				BufCtrl_WriteWithCount(psNuVox53ExApp->psDuplicateOutBufCtrl, NUVOX53EXAPP_OUT_SAMPLES_PER_FRAME, pi16OutBuf );
			else 
				psNuVox53ExApp->pfnDuplicateFunc(NUVOX53EXAPP_OUT_SAMPLES_PER_FRAME, pi16OutBuf);
		}
	}	
	return TRUE;
}
