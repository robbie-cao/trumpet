#include <string.h>
#include "NuOneExApp_Decode.h"
#include "AudioRom.h"
#include "PlaybackRecord.h"

extern S_AUDIO_CALLBACK g_asAppCallBack[];

//---------------------------------------------------------------------------------------------------------
void NuOneExApp_DecodeInitiate(S_NUONEEX_APP_DECODE *psNuOneExAppDecode, UINT8 *pau8TempBuf, UINT32 u32CallbackIndex)
{
	memset( psNuOneExAppDecode, '\0', sizeof(S_NUONEEX_APP_DECODE) );
	// NuOneEx decoder will refer to this index and call which callback function of storage 
	psNuOneExAppDecode->u8CallbackIndex = (UINT8)u32CallbackIndex;
	// Input temp buffer to provide library used.
	psNuOneExAppDecode->pau8TempBuf = pau8TempBuf;
	BUF_CTRL_SET_INACTIVE(&psNuOneExAppDecode->sOutBufCtrl);
}

//---------------------------------------------------------------------------------------------------------
BOOL NuOneExApp_DecodeStartPlayByID(S_NUONEEX_APP_DECODE *psNuOneExAppDecode, UINT32 u32AudioID, UINT32 u32RomStartAddr, UINT8 u8PlaybackChannel)
{
	S_ROM_AUDIO_CHUNK_INFO sAudioChunkInfo;
	
	// Get audio chunk information for audio chunk start address from ROM file.
	AudioRom_GetAudioChunkInfo( g_asAppCallBack[psNuOneExAppDecode->u8CallbackIndex].pfnReadDataCallback, 
								u32RomStartAddr, 
								u32AudioID, 
								&sAudioChunkInfo);	

	return NuOneExApp_DecodeStartPlayByAddr(psNuOneExAppDecode, sAudioChunkInfo.u32AudioChunkAddr, u8PlaybackChannel);
}

//---------------------------------------------------------------------------------------------------------
BOOL NuOneExApp_DecodeStartPlayByAddr(S_NUONEEX_APP_DECODE *psNuOneExAppDecode, UINT32 u32NuOneExStorageStartAddr, UINT8 u8PlaybackChannel)
{
	UINT16 u16SampleRate;
	
	// NuOneEx decoder initiates work buffer and returns sample rate.
	if ( (u16SampleRate = NuOneEx_DecodeInitiate(	(UINT8*)psNuOneExAppDecode->au32DecodeWorkBuf, 
													psNuOneExAppDecode->pau8TempBuf, 
													u32NuOneExStorageStartAddr, 
													g_asAppCallBack[psNuOneExAppDecode->u8CallbackIndex].pfnReadDataCallback )) == 0 )
		return FALSE;	
	
	// Initiate and set output buffer variable(include frame size, buffer size etc.) 
	Playback_SetOutputBuf( 	&psNuOneExAppDecode->sOutBufCtrl,
							NUONEEXAPP_OUT_BUF_SIZE,
							psNuOneExAppDecode->i16OutBuf,
							NUONEEXAPP_OUT_SAMPLES_PER_FRAME,
							u16SampleRate );
	
	// Trigger active flag of output buffer for NuOneEx decoding
	BUF_CTRL_SET_ACTIVE(&psNuOneExAppDecode->sOutBufCtrl);
	
	// Pre-decode one frame
	psNuOneExAppDecode->sOutBufCtrl.u16BufWriteIdx = NUONEEXAPP_OUT_SAMPLES_PER_FRAME;
	if ( NuOneExApp_DecodeProcess(psNuOneExAppDecode) == FALSE )
	{
		BUF_CTRL_SET_INACTIVE(&psNuOneExAppDecode->sOutBufCtrl);
		return FALSE;
	}
	psNuOneExAppDecode->sOutBufCtrl.u16BufReadIdx = NUONEEXAPP_OUT_SAMPLES_PER_FRAME;
	
	// Record play channel index for stopping to play.
	psNuOneExAppDecode->u8PlaybackChannel = u8PlaybackChannel;
	// Add audio codec into channel and preper to play codec.
	Playback_Add(psNuOneExAppDecode->u8PlaybackChannel, &psNuOneExAppDecode->sOutBufCtrl);
	
	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
void NuOneExApp_DecodeStopPlay(S_NUONEEX_APP_DECODE *psNuOneExAppDecode)
{
	// Clear active flag of output buffer for stoping NuOneEx decode.
	BUF_CTRL_SET_INACTIVE(&psNuOneExAppDecode->sOutBufCtrl);
	// Remove audio codec output buffer from play channel.
	Playback_Remove(psNuOneExAppDecode->u8PlaybackChannel);
}

//---------------------------------------------------------------------------------------------------------
BOOL NuOneExApp_DecodeProcess(S_NUONEEX_APP_DECODE *psNuOneExAppDecode)
{
	INT16 *pi16OutBuf;
	
	if (BUF_CTRL_IS_INACTIVE(&psNuOneExAppDecode->sOutBufCtrl))
		return FALSE;
	
	if ( Playback_NeedUpdateOutputBuf(&psNuOneExAppDecode->sOutBufCtrl) )
	{
		// Check end of file
		if(NuOneEx_DecodeIsEnd((UINT8*)psNuOneExAppDecode->au32DecodeWorkBuf))
		{
			// Trigger inactive flag of output buffer to stop NuOneEx decoding
			BUF_CTRL_SET_INACTIVE(&psNuOneExAppDecode->sOutBufCtrl);
			// Use to represnt no active(or end) of decoding
			psNuOneExAppDecode->sOutBufCtrl.u16SampleRate = 0;
			return FALSE;
		}
		
		// Record output data buffer pointer(for duplicate & process)
		pi16OutBuf = (PINT16)&psNuOneExAppDecode->sOutBufCtrl.pi16Buf[psNuOneExAppDecode->sOutBufCtrl.u16BufWriteIdx];
		
		NuOneEx_DecodeProcess(	(UINT8*)psNuOneExAppDecode->au32DecodeWorkBuf, 
								psNuOneExAppDecode->pau8TempBuf, 
								pi16OutBuf, 
								g_asAppCallBack[psNuOneExAppDecode->u8CallbackIndex].pfnReadDataCallback, 
								g_asAppCallBack[psNuOneExAppDecode->u8CallbackIndex].pfnUserEventCallback);

		// Update write index of output buffer and avoid buffer overflow
		Playback_UpdateOutputBuf(&psNuOneExAppDecode->sOutBufCtrl);	
		
		// Duplicate data into buffer for using duplication callback function.
		if ( psNuOneExAppDecode->u8CtrlFlag&(NUONEEXAPP_CTRL_DUPLICATE_TO_BUF|NUONEEXAPP_CTRL_DUPLICATE_TO_FUNC) )
		{
			if ( psNuOneExAppDecode->u8CtrlFlag & NUONEEXAPP_CTRL_DUPLICATE_TO_BUF )
				BufCtrl_WriteWithCount(psNuOneExAppDecode->psDuplicateOutBufCtrl, NUONEEXAPP_OUT_SAMPLES_PER_FRAME, pi16OutBuf );
			else 
				psNuOneExAppDecode->pfnDuplicateFunc(NUONEEXAPP_OUT_SAMPLES_PER_FRAME, pi16OutBuf);
		}
	}	
	return TRUE;
}
