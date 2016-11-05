#include <string.h>
#include "NuVox63ExApp.h"
#include "AudioRom.h"
#include "PlaybackRecord.h"

extern S_AUDIO_CALLBACK g_asAppCallBack[];

//---------------------------------------------------------------------------------------------------------
void NuVox63ExApp_DecodeInitiate(S_NUVOX63EX_APP *psNuVox63ExApp, UINT8 *pau8TempBuf, UINT32 u32CallbackIndex)
{
	memset( psNuVox63ExApp, '\0', sizeof(S_NUVOX63EX_APP) );
	// NuVox63Ex decoder will refer to this index and call which callback function of storage 
	psNuVox63ExApp->u8CallbackIndex = (UINT8)u32CallbackIndex;
	// Input temp buffer to provide library used.
	psNuVox63ExApp->pau8TempBuf = pau8TempBuf;
	BUF_CTRL_SET_INACTIVE(&psNuVox63ExApp->sOutBufCtrl);
}

//---------------------------------------------------------------------------------------------------------
BOOL NuVox63ExApp_DecodeStartPlayByID(S_NUVOX63EX_APP *psNuVox63ExApp, UINT32 u32AudioID, UINT32 u32RomStartAddr, UINT8 u8PlaybackChannel)
{
	S_ROM_AUDIO_CHUNK_INFO sAudioChunkInfo;
	
	// Get audio chunk information for audio chunk start address from ROM file.
	AudioRom_GetAudioChunkInfo( g_asAppCallBack[psNuVox63ExApp->u8CallbackIndex].pfnReadDataCallback, 
								u32RomStartAddr, 
								u32AudioID, 
								&sAudioChunkInfo);	

	return NuVox63ExApp_DecodeStartPlayByAddr(psNuVox63ExApp, sAudioChunkInfo.u32AudioChunkAddr, u8PlaybackChannel);
}

//---------------------------------------------------------------------------------------------------------
BOOL NuVox63ExApp_DecodeStartPlayByAddr(S_NUVOX63EX_APP *psNuVox63ExApp, UINT32 u32NuVox63ExStorageStartAddr, UINT8 u8PlaybackChannel)
{
	UINT16 u16SampleRate;
	
	// NuVox63Ex decoder initiates work buffer and returns sample rate.
	if ( (u16SampleRate = NuVox63Ex_DecodeInitiate(	(UINT8*)psNuVox63ExApp->au32DecodeWorkBuf, 
													psNuVox63ExApp->pau8TempBuf, 
													u32NuVox63ExStorageStartAddr, 
													g_asAppCallBack[psNuVox63ExApp->u8CallbackIndex].pfnReadDataCallback )) == 0 )
		return FALSE;	

	// Initiate and set output buffer variable(include frame size, buffer size etc.) 
	Playback_SetOutputBuf( 	&psNuVox63ExApp->sOutBufCtrl,
							NUVOX63EXAPP_OUT_BUF_SIZE,
							psNuVox63ExApp->i16OutBuf,
							NUVOX63EXAPP_OUT_SAMPLES_PER_FRAME,
							u16SampleRate );
	
	// Trigger active flag of output buffer for NuVox63Ex decoding
	BUF_CTRL_SET_ACTIVE(&psNuVox63ExApp->sOutBufCtrl);
	
	// Pre-decode one frame
	psNuVox63ExApp->sOutBufCtrl.u16BufWriteIdx = NUVOX63EXAPP_OUT_SAMPLES_PER_FRAME;
	if ( NuVox63ExApp_DecodeProcess(psNuVox63ExApp) == FALSE )
	{
		BUF_CTRL_SET_INACTIVE(&psNuVox63ExApp->sOutBufCtrl);
		return FALSE;
	}
	psNuVox63ExApp->sOutBufCtrl.u16BufReadIdx = NUVOX63EXAPP_OUT_SAMPLES_PER_FRAME;
	
	// Record play channel index for stopping to play.
	psNuVox63ExApp->u8PlaybackChannel = u8PlaybackChannel;
	// Add audio codec into channel and preper to play codec.
	Playback_Add(psNuVox63ExApp->u8PlaybackChannel, &psNuVox63ExApp->sOutBufCtrl);

	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
void NuVox63ExApp_DecodeStopPlay(S_NUVOX63EX_APP *psNuVox63ExApp)
{
	// Clear active flag of output buffer for stoping NuVox63Ex decode.
	BUF_CTRL_SET_INACTIVE(&psNuVox63ExApp->sOutBufCtrl);
	// Remove audio codec output buffer from play channel.
	Playback_Remove(psNuVox63ExApp->u8PlaybackChannel);
}

//---------------------------------------------------------------------------------------------------------
BOOL NuVox63ExApp_DecodeProcess(S_NUVOX63EX_APP *psNuVox63ExApp)
{
	INT16 *pi16OutBuf;
	
	if (BUF_CTRL_IS_INACTIVE(&psNuVox63ExApp->sOutBufCtrl))
		return FALSE;
	
	if ( Playback_NeedUpdateOutputBuf(&psNuVox63ExApp->sOutBufCtrl) )
	{
		// Check end of file
		if(NuVox63Ex_DecodeIsEnd((UINT8*)psNuVox63ExApp->au32DecodeWorkBuf))
		{
			// Trigger inactive flag of output buffer to stop NuVox63Ex decoding
			BUF_CTRL_SET_INACTIVE(&psNuVox63ExApp->sOutBufCtrl);
			// Use to represnt no active(or end) of decoding
			psNuVox63ExApp->sOutBufCtrl.u16SampleRate = 0;
			return FALSE;
		}

		// Record output data buffer pointer(for duplicate & process)
		pi16OutBuf = (PINT16)&psNuVox63ExApp->sOutBufCtrl.pi16Buf[psNuVox63ExApp->sOutBufCtrl.u16BufWriteIdx];
		
		NuVox63Ex_DecodeProcess((UINT8*)psNuVox63ExApp->au32DecodeWorkBuf, 
								psNuVox63ExApp->pau8TempBuf, 
								pi16OutBuf, 
								g_asAppCallBack[psNuVox63ExApp->u8CallbackIndex].pfnReadDataCallback, 
								g_asAppCallBack[psNuVox63ExApp->u8CallbackIndex].pfnUserEventCallback);

		// Update write index of output buffer and avoid buffer overflow
		Playback_UpdateOutputBuf(&psNuVox63ExApp->sOutBufCtrl);	
		
		// Duplicate data into buffer for using duplication callback function.
		if ( psNuVox63ExApp->u8CtrlFlag&(NUVOX63EXAPP_CTRL_DUPLICATE_TO_BUF|NUVOX63EXAPP_CTRL_DUPLICATE_TO_FUNC) )
		{
			if ( psNuVox63ExApp->u8CtrlFlag & NUVOX63EXAPP_CTRL_DUPLICATE_TO_BUF )
				BufCtrl_WriteWithCount(psNuVox63ExApp->psDuplicateOutBufCtrl, NUVOX63EXAPP_OUT_SAMPLES_PER_FRAME, pi16OutBuf );
			else 
				psNuVox63ExApp->pfnDuplicateFunc(NUVOX63EXAPP_OUT_SAMPLES_PER_FRAME, pi16OutBuf);
		}
	}	
	return TRUE;
}
