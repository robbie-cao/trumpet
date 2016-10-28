#include <string.h>
#include "ImaAdpcmApp_Decode.h"
#include "AudioRom.h"
#include "PlaybackRecord.h"

extern S_AUDIO_CALLBACK g_asAppCallBack[];

//---------------------------------------------------------------------------------------------------------
void ImaAdpcmApp_DecodeInitiate(S_IMAADPCM_APP_DECODE *psImaAdpcmAppDecode, UINT8 *pau8TempBuf, UINT32 u32CallbackIndex)
{
	memset( psImaAdpcmAppDecode, '\0', sizeof(S_IMAADPCM_APP_DECODE) );
	// ImaAdpcm decoder will refer to this index and call which callback function of storage 
	psImaAdpcmAppDecode->u8CallbackIndex = (UINT8)u32CallbackIndex;
	// Input temp buffer to provide library used.
	psImaAdpcmAppDecode->pau8TempBuf = pau8TempBuf;
	BUF_CTRL_SET_INACTIVE(&psImaAdpcmAppDecode->sOutBufCtrl);
}

//---------------------------------------------------------------------------------------------------------
BOOL ImaAdpcmApp_DecodeStartPlayByID(S_IMAADPCM_APP_DECODE *psImaAdpcmAppDecode, UINT32 u32AudioID, UINT32 u32RomStartAddr, UINT8 u8PlaybackChannel)
{
	S_ROM_AUDIO_CHUNK_INFO sAudioChunkInfo;
	
	// Get audio chunk information for audio chunk start address from ROM file.
	AudioRom_GetAudioChunkInfo( g_asAppCallBack[psImaAdpcmAppDecode->u8CallbackIndex].pfnReadDataCallback, 
								u32RomStartAddr, 
								u32AudioID, 
								&sAudioChunkInfo);	

	return ImaAdpcmApp_DecodeStartPlayByAddr(psImaAdpcmAppDecode, sAudioChunkInfo.u32AudioChunkAddr, u8PlaybackChannel);
}

//---------------------------------------------------------------------------------------------------------
BOOL ImaAdpcmApp_DecodeStartPlayByAddr(S_IMAADPCM_APP_DECODE *psImaAdpcmAppDecode, UINT32 u32ImaAdpcmStorageStartAddr, UINT8 u8PlaybackChannel)
{
	UINT16 u16SampleRate;
	
	// ImaAdpcm decoder initiates work buffer and returns sample rate.
	if ( (u16SampleRate = ImaAdpcm_DecodeInitiate(	(UINT8*)psImaAdpcmAppDecode->au32DecodeWorkBuf, 
													psImaAdpcmAppDecode->pau8TempBuf, 
													u32ImaAdpcmStorageStartAddr, 
													g_asAppCallBack[psImaAdpcmAppDecode->u8CallbackIndex].pfnReadDataCallback )) == 0 )
		return FALSE;	
	
	#if (IMAADPCMAPP_OUT_SAMPLES_PER_FRAME != IMAADPCM_SAMPLE_PER_FRAME )
	// set the sample counts of one ImaAdpcm frame
	ImaAdpcm_DecodeSampleCount((UINT8 *)psImaAdpcmAppDecode->au32DecodeWorkBuf, IMAADPCMAPP_OUT_SAMPLES_PER_FRAME);
	#endif

	// Initiate and set output buffer variable(include frame size, buffer size etc.) 
	Playback_SetOutputBuf( 	&psImaAdpcmAppDecode->sOutBufCtrl,
							IMAADPCMAPP_OUT_BUF_SIZE,
							psImaAdpcmAppDecode->i16OutBuf,
							IMAADPCMAPP_OUT_SAMPLES_PER_FRAME,
							u16SampleRate );
	
	// Trigger active flag of output buffer for ImaAdpcm decoding
	BUF_CTRL_SET_ACTIVE(&psImaAdpcmAppDecode->sOutBufCtrl);

	// Pre-decode one frame
	psImaAdpcmAppDecode->sOutBufCtrl.u16BufWriteIdx = IMAADPCMAPP_OUT_SAMPLES_PER_FRAME;
	if ( ImaAdpcmApp_DecodeProcess(psImaAdpcmAppDecode) == FALSE )
	{
		BUF_CTRL_SET_INACTIVE(&psImaAdpcmAppDecode->sOutBufCtrl);
		return FALSE;
	}
	psImaAdpcmAppDecode->sOutBufCtrl.u16BufReadIdx = IMAADPCMAPP_OUT_SAMPLES_PER_FRAME;
	
	// Record play channel index for stopping to play.
	psImaAdpcmAppDecode->u8PlaybackChannel = u8PlaybackChannel;
	// Add audio codec into channel and preper to play codec.
	Playback_Add(psImaAdpcmAppDecode->u8PlaybackChannel, &psImaAdpcmAppDecode->sOutBufCtrl);

	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
void ImaAdpcmApp_DecodeStopPlay(S_IMAADPCM_APP_DECODE *psImaAdpcmAppDecode)
{
	// Clear active flag of output buffer for stoping ImaAdpcm decode.
	BUF_CTRL_SET_INACTIVE(&psImaAdpcmAppDecode->sOutBufCtrl);
	// Remove audio codec output buffer from play channel.
	Playback_Remove(psImaAdpcmAppDecode->u8PlaybackChannel);
}

//---------------------------------------------------------------------------------------------------------
BOOL ImaAdpcmApp_DecodeProcess(S_IMAADPCM_APP_DECODE *psImaAdpcmAppDecode)
{
	INT16 *pi16OutBuf;
	
	if (BUF_CTRL_IS_INACTIVE(&psImaAdpcmAppDecode->sOutBufCtrl))
		return FALSE;
	
	if ( Playback_NeedUpdateOutputBuf(&psImaAdpcmAppDecode->sOutBufCtrl) )
	{
		// Check end of file
		if(ImaAdpcm_DecodeIsEnd((UINT8*)psImaAdpcmAppDecode->au32DecodeWorkBuf))
		{
			// Trigger inactive flag of output buffer to stop ImaAdpcm decoding
			BUF_CTRL_SET_INACTIVE(&psImaAdpcmAppDecode->sOutBufCtrl);
			// Use to represnt no active(or end) of decoding
			psImaAdpcmAppDecode->sOutBufCtrl.u16SampleRate = 0;
			return FALSE;
		}

		// Record output data buffer pointer(for duplicate & process)
		pi16OutBuf = (PINT16)&psImaAdpcmAppDecode->sOutBufCtrl.pi16Buf[psImaAdpcmAppDecode->sOutBufCtrl.u16BufWriteIdx];
		
		ImaAdpcm_DecodeProcess(	(UINT8*)psImaAdpcmAppDecode->au32DecodeWorkBuf, 
								psImaAdpcmAppDecode->pau8TempBuf, 
								pi16OutBuf, 
								g_asAppCallBack[psImaAdpcmAppDecode->u8CallbackIndex].pfnReadDataCallback, 
								g_asAppCallBack[psImaAdpcmAppDecode->u8CallbackIndex].pfnUserEventCallback);

		// Update write index of output buffer and avoid buffer overflow
		Playback_UpdateOutputBuf(&psImaAdpcmAppDecode->sOutBufCtrl);

		// Duplicate data into buffer for using duplication callback function.
		if ( psImaAdpcmAppDecode->u8CtrlFlag&(IMAADPCMAPP_CTRL_DUPLICATE_TO_BUF|IMAADPCMAPP_CTRL_DUPLICATE_TO_FUNC) )
		{
			if ( psImaAdpcmAppDecode->u8CtrlFlag & IMAADPCMAPP_CTRL_DUPLICATE_TO_BUF )
				BufCtrl_WriteWithCount(psImaAdpcmAppDecode->psDuplicateOutBufCtrl, IMAADPCMAPP_OUT_SAMPLES_PER_FRAME, pi16OutBuf );
			else 
				psImaAdpcmAppDecode->pfnDuplicateFunc(IMAADPCMAPP_OUT_SAMPLES_PER_FRAME, pi16OutBuf);
		}		
	}	
	return TRUE;
}
