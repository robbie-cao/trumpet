#include <string.h>
#include "NuSoundExApp.h"
#include "AudioRom.h"
#include "PlaybackRecord.h"

extern S_AUDIO_CALLBACK g_asAppCallBack[];

//---------------------------------------------------------------------------------------------------------
void NuSoundExApp_DecodeInitiate(S_NUSOUNDEX_APP *psNuSoundExApp, UINT8 *pau8TempBuf, UINT32 u32CallbackIndex)
{
	memset( psNuSoundExApp, '\0', sizeof(S_NUSOUNDEX_APP) );
	// NuSoundEx decoder will refer to this index and call which callback function of storage 
	psNuSoundExApp->u8CallbackIndex = (UINT8)u32CallbackIndex;
	// Input temp buffer to provide library used.
	psNuSoundExApp->pau8TempBuf = pau8TempBuf;
	BUF_CTRL_SET_INACTIVE(&psNuSoundExApp->sOutBufCtrl);
}

//---------------------------------------------------------------------------------------------------------
BOOL NuSoundExApp_DecodeStartPlayByID(S_NUSOUNDEX_APP *psNuSoundExApp, UINT32 u32AudioID, UINT32 u32RomStartAddr, UINT8 u8PlaybackChannel)
{
	S_ROM_AUDIO_CHUNK_INFO sAudioChunkInfo;
	
	// Get audio chunk information for audio chunk start address from ROM file.
	// The ROM file is placed on SPI Flash address "AUDIOROM_STORAGE_START_ADDR".
	AudioRom_GetAudioChunkInfo( g_asAppCallBack[psNuSoundExApp->u8CallbackIndex].pfnReadDataCallback, 
								u32RomStartAddr, 
								u32AudioID, 
								&sAudioChunkInfo);	

	return NuSoundExApp_DecodeStartPlayByAddr(psNuSoundExApp, sAudioChunkInfo.u32AudioChunkAddr, u8PlaybackChannel);
}

//---------------------------------------------------------------------------------------------------------
BOOL NuSoundExApp_DecodeStartPlayByAddr(S_NUSOUNDEX_APP *psNuSoundExApp, UINT32 u32NuSoundExStorageStartAddr, UINT8 u8PlaybackChannel)
{
	UINT16 u16SampleRate;
	
	// NuSoundEx decoder initiates work buffer and returns sample rate.
	if ( (u16SampleRate = NuSoundEx_DecodeInitiate(	(UINT8*)psNuSoundExApp->au32DecodeWorkBuf, 
													psNuSoundExApp->pau8TempBuf, 
													u32NuSoundExStorageStartAddr, 
													g_asAppCallBack[psNuSoundExApp->u8CallbackIndex].pfnReadDataCallback )) == 0 )
		return FALSE;	

	// Initiate and set output buffer variable(include frame size, buffer size etc.) 
	Playback_SetOutputBuf( 	&psNuSoundExApp->sOutBufCtrl,
							NUSOUNDEXAPP_OUT_BUF_SIZE,
							psNuSoundExApp->i16OutBuf,
							NUSOUNDEXAPP_OUT_SAMPLES_PER_FRAME,
							u16SampleRate );
	
	// Trigger active flag of output buffer for NuSoundEx decoding
	BUF_CTRL_SET_ACTIVE(&psNuSoundExApp->sOutBufCtrl);
	
	// Pre-decode one frame
	psNuSoundExApp->sOutBufCtrl.u16BufWriteIdx = NUSOUNDEXAPP_OUT_SAMPLES_PER_FRAME;
	if ( NuSoundExApp_DecodeProcess(psNuSoundExApp) == FALSE )
	{
		BUF_CTRL_SET_INACTIVE(&psNuSoundExApp->sOutBufCtrl);
		return FALSE;
	}
	psNuSoundExApp->sOutBufCtrl.u16BufReadIdx = NUSOUNDEXAPP_OUT_SAMPLES_PER_FRAME;
	
	// Record play channel index for stopping to play.
	psNuSoundExApp->u8PlaybackChannel = u8PlaybackChannel;
	// Add audio codec into channel and preper to play codec.
	Playback_Add(psNuSoundExApp->u8PlaybackChannel, &psNuSoundExApp->sOutBufCtrl);

	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
void NuSoundExApp_DecodeStopPlay(S_NUSOUNDEX_APP *psNuSoundExApp)
{
	// Clear active flag of output buffer for stoping NuSoundEx decode.
	BUF_CTRL_SET_INACTIVE(&psNuSoundExApp->sOutBufCtrl);
	// Remove audio codec output buffer from play channel.
	Playback_Remove(psNuSoundExApp->u8PlaybackChannel);
}

//---------------------------------------------------------------------------------------------------------
BOOL NuSoundExApp_DecodeProcess(S_NUSOUNDEX_APP *psNuSoundExApp)
{
	INT16 *pi16OutBuf;
	
	if (BUF_CTRL_IS_INACTIVE(&psNuSoundExApp->sOutBufCtrl))
		return FALSE;
	
	if ( Playback_NeedUpdateOutputBuf(&psNuSoundExApp->sOutBufCtrl) ) 
	{
		// Check end of file
		if(NuSoundEx_DecodeIsEnd((UINT8*)psNuSoundExApp->au32DecodeWorkBuf))
		{
			// Trigger inactive flag of output buffer to stop NuSoundEx decoding
			BUF_CTRL_SET_INACTIVE(&psNuSoundExApp->sOutBufCtrl);
			// Use to represnt no active(or end) of decoding
			psNuSoundExApp->sOutBufCtrl.u16SampleRate = 0;
			return FALSE;
		}
		
		// Record output data buffer pointer(for duplicate & process)
		pi16OutBuf = (PINT16)&psNuSoundExApp->sOutBufCtrl.pi16Buf[psNuSoundExApp->sOutBufCtrl.u16BufWriteIdx];
		
		NuSoundEx_DecodeProcess((UINT8*)psNuSoundExApp->au32DecodeWorkBuf, 
								psNuSoundExApp->pau8TempBuf, 
								pi16OutBuf, 
								g_asAppCallBack[psNuSoundExApp->u8CallbackIndex].pfnReadDataCallback, 
								g_asAppCallBack[psNuSoundExApp->u8CallbackIndex].pfnUserEventCallback);

		// Update write index of output buffer and avoid buffer overflow
		Playback_UpdateOutputBuf(&psNuSoundExApp->sOutBufCtrl);	
		
		// Duplicate data into buffer for using duplication callback function.
		if ( psNuSoundExApp->u8CtrlFlag&(NUSOUNDEXAPP_CTRL_DUPLICATE_TO_BUF|NUSOUNDEXAPP_CTRL_DUPLICATE_TO_FUNC) )
		{
			if ( psNuSoundExApp->u8CtrlFlag & NUSOUNDEXAPP_CTRL_DUPLICATE_TO_BUF )
				BufCtrl_WriteWithCount(psNuSoundExApp->psDuplicateOutBufCtrl, NUSOUNDEXAPP_OUT_SAMPLES_PER_FRAME, pi16OutBuf );
			else 
				psNuSoundExApp->pfnDuplicateFunc(NUSOUNDEXAPP_OUT_SAMPLES_PER_FRAME, pi16OutBuf);
		}
	}	
	return TRUE;
}
