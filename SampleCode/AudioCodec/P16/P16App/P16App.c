#include <string.h>
#include "P16App.h"
#include "AudioRom.h"
#include "PlaybackRecord.h"

extern S_AUDIO_CALLBACK g_asAppCallBack[];

//---------------------------------------------------------------------------------------------------------
void P16App_DecodeInitiate(S_P16_APP *psP16App, UINT8 *pau8TempBuf, UINT32 u32CallbackIndex)
{
	memset( psP16App, '\0', sizeof(S_P16_APP) );
	// P16 decoder will refer to this index and call which callback function of storage 
	psP16App->u8CallbackIndex = (UINT8)u32CallbackIndex;
	BUF_CTRL_SET_INACTIVE(&psP16App->sOutBufCtrl);
}

//---------------------------------------------------------------------------------------------------------
BOOL P16App_DecodeStartPlayByID(S_P16_APP *psP16App, UINT32 u32AudioID, UINT32 u32RomStartAddr, UINT8 u8PlaybackChannel)
{
	S_ROM_AUDIO_CHUNK_INFO sAudioChunkInfo;
	
	// Get audio chunk information for audio chunk start address from ROM file.
	AudioRom_GetAudioChunkInfo( g_asAppCallBack[psP16App->u8CallbackIndex].pfnReadDataCallback, 
								u32RomStartAddr, 
								u32AudioID, 
								&sAudioChunkInfo);	

	return P16App_DecodeStartPlayByAddr(psP16App, sAudioChunkInfo.u32AudioChunkAddr, u8PlaybackChannel);
}

//---------------------------------------------------------------------------------------------------------
BOOL P16App_DecodeStartPlayByAddr(S_P16_APP *psP16App, UINT32 u32P16StorageStartAddr, UINT8 u8PlaybackChannel)
{
	UINT16 u16SampleRate;
	
	// P16 decoder initiates work buffer and returns sample rate.
	if ( (u16SampleRate = P16_DecodeInitiate( 	(UINT8*)psP16App->au32DecodeWorkBuf,
												NULL,
												u32P16StorageStartAddr,
												g_asAppCallBack[psP16App->u8CallbackIndex].pfnReadDataCallback )) == 0 )
		return FALSE;	
	
	// Initiate and set output buffer variable(include frame size, buffer size etc.) 
	Playback_SetOutputBuf( 	&psP16App->sOutBufCtrl,
							P16APP_OUT_BUF_SIZE,
							psP16App->i16OutBuf,
							P16APP_OUT_SAMPLES_PER_FRAME,
							u16SampleRate );

	#if (P16APP_OUT_SAMPLES_PER_FRAME != P16_DECODE_SAMPLE_PER_FRAME )
	// Programer can change samples per frame, default value is 8 samples
	P16_DecodeSampleCounts((UINT8*)psP16App->au8DecodeWorkBuf, P16APP_OUT_SAMPLES_PER_FRAME);
	#endif
	
	// Trigger active flag of output buffer for P16 decoding
	BUF_CTRL_SET_ACTIVE(&psP16App->sOutBufCtrl);
	
	// Pre-decode one frame
	psP16App->sOutBufCtrl.u16BufWriteIdx = P16APP_OUT_SAMPLES_PER_FRAME;
	if ( P16App_DecodeProcess(psP16App) == FALSE )
	{
		BUF_CTRL_SET_INACTIVE(&psP16App->sOutBufCtrl);
		return FALSE;
	}
	psP16App->sOutBufCtrl.u16BufReadIdx = P16APP_OUT_SAMPLES_PER_FRAME;
	
	// Record play channel index for stopping to play.
	psP16App->u8PlaybackChannel = u8PlaybackChannel;
	// Add audio codec into channel and preper to play codec.
	Playback_Add(psP16App->u8PlaybackChannel, &psP16App->sOutBufCtrl);

	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
void P16App_DecodeStopPlay(S_P16_APP *psP16App)
{
	// Clear active flag of output buffer for stoping P16 decode.
	BUF_CTRL_SET_INACTIVE(&psP16App->sOutBufCtrl);
	// Remove audio codec output buffer from play channel.
	Playback_Remove(psP16App->u8PlaybackChannel);
}
	
//---------------------------------------------------------------------------------------------------------
BOOL P16App_DecodeProcess(S_P16_APP *psP16App)
{
	INT16 *pi16OutBuf;
	
	if (BUF_CTRL_IS_INACTIVE(&psP16App->sOutBufCtrl))
		return FALSE;
	
	while ( Playback_NeedUpdateOutputBuf(&psP16App->sOutBufCtrl) )
	{
		// Check end of file
		if(P16_DecodeIsEnd((UINT8*)psP16App->au32DecodeWorkBuf))
		{
			// Trigger inactive flag of output buffer to stop P16 decoding
			BUF_CTRL_SET_INACTIVE(&psP16App->sOutBufCtrl);
			// Use to represnt no active(or end) of decoding
			psP16App->sOutBufCtrl.u16SampleRate = 0; 
			return FALSE;
		}

		// Record output data buffer pointer(for duplicate & process)
		pi16OutBuf = (PINT16)&psP16App->sOutBufCtrl.pi16Buf[psP16App->sOutBufCtrl.u16BufWriteIdx];
		
		P16_DecodeProcess(	(UINT8*)psP16App->au32DecodeWorkBuf,
							NULL,
							pi16OutBuf, 
							g_asAppCallBack[psP16App->u8CallbackIndex].pfnReadDataCallback,
							g_asAppCallBack[psP16App->u8CallbackIndex].pfnUserEventCallback);

		// Update write index of output buffer and avoid buffer overflow
		Playback_UpdateOutputBuf(&psP16App->sOutBufCtrl);	
		
		// Duplicate data into buffer for using duplication callback function.
		if ( psP16App->u8CtrlFlag&(P16APP_CTRL_DUPLICATE_TO_BUF|P16APP_CTRL_DUPLICATE_TO_FUNC) )
		{
			if ( psP16App->u8CtrlFlag & P16APP_CTRL_DUPLICATE_TO_BUF )
				BufCtrl_WriteWithCount(psP16App->psDuplicateOutBufCtrl, P16APP_OUT_SAMPLES_PER_FRAME, pi16OutBuf );
			else 
				psP16App->pfnDuplicateFunc(P16APP_OUT_SAMPLES_PER_FRAME, pi16OutBuf);
		}
	}	
	return TRUE;
}
