#include <string.h>
#include "LP8App.h"
#include "AudioRom.h"
#include "PlaybackRecord.h"

extern S_AUDIO_CALLBACK g_asAppCallBack[];

//---------------------------------------------------------------------------------------------------------
void LP8App_DecodeInitiate(S_LP8_APP *psLP8App, UINT8 *pau8TempBuf, UINT32 u32CallbackIndex)
{
	memset( psLP8App, '\0', sizeof(S_LP8_APP) );
	// LP8 decoder will refer to this index and call which callback function of storage 
	psLP8App->u8CallbackIndex = (UINT8)u32CallbackIndex;
	BUF_CTRL_SET_INACTIVE(&psLP8App->sOutBufCtrl);
}

//---------------------------------------------------------------------------------------------------------
BOOL LP8App_DecodeStartPlayByID(S_LP8_APP *psLP8App, UINT32 u32AudioID, UINT32 u32RomStartAddr, UINT8 u8PlaybackChannel)
{
	S_ROM_AUDIO_CHUNK_INFO sAudioChunkInfo;
	
	// Get audio chunk information for audio chunk start address from ROM file.
	AudioRom_GetAudioChunkInfo( g_asAppCallBack[psLP8App->u8CallbackIndex].pfnReadDataCallback, 
								u32RomStartAddr, 
								u32AudioID, 
								&sAudioChunkInfo);	

	return LP8App_DecodeStartPlayByAddr(psLP8App, sAudioChunkInfo.u32AudioChunkAddr, u8PlaybackChannel);
}

//---------------------------------------------------------------------------------------------------------
BOOL LP8App_DecodeStartPlayByAddr(S_LP8_APP *psLP8App, UINT32 u32LP8StorageStartAddr, UINT8 u8PlaybackChannel)
{
	UINT16 u16SampleRate;
	
	// LP8 decoder initiates work buffer and returns sample rate.
	if ( (u16SampleRate = LP8_DecodeInitiate( 	(UINT8*)psLP8App->au32DecodeWorkBuf,
												NULL,
												u32LP8StorageStartAddr,
												g_asAppCallBack[psLP8App->u8CallbackIndex].pfnReadDataCallback )) == 0 )
		return FALSE;	

	#if (LP8APP_OUT_SAMPLES_PER_FRAME != LP8_DECODE_SAMPLE_PER_FRAME )
	// Programer can change samples per frame, default value is 8 samples
	LP8_DecodeSampleCounts((UINT8*)psLP8App->au32DecodeWorkBuf, LP8APP_OUT_SAMPLES_PER_FRAME);
	#endif
	
	// Initiate and set output buffer variable(include frame size, buffer size etc.) 
	Playback_SetOutputBuf( 	&psLP8App->sOutBufCtrl,
							LP8APP_OUT_BUF_SIZE,
							psLP8App->i16OutBuf,
							LP8APP_OUT_SAMPLES_PER_FRAME,
							u16SampleRate );
	
	// Trigger active flag of output buffer for LP8 decoding
	BUF_CTRL_SET_ACTIVE(&psLP8App->sOutBufCtrl);
	
	// Pre-decode one frame
	psLP8App->sOutBufCtrl.u16BufWriteIdx = LP8APP_OUT_SAMPLES_PER_FRAME;
	if ( LP8App_DecodeProcess(psLP8App) == FALSE )
	{
		BUF_CTRL_SET_INACTIVE(&psLP8App->sOutBufCtrl);
		return FALSE;
	}
	psLP8App->sOutBufCtrl.u16BufReadIdx = LP8APP_OUT_SAMPLES_PER_FRAME;
	
	// Record play channel index for stopping to play.
	psLP8App->u8PlaybackChannel = u8PlaybackChannel;
	// Add audio codec into channel and preper to play codec.
	Playback_Add(psLP8App->u8PlaybackChannel, &psLP8App->sOutBufCtrl);
	
	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
void LP8App_DecodeStopPlay(S_LP8_APP *psLP8App)
{
	// Clear active flag of output buffer for stoping LP8 decode.
	BUF_CTRL_SET_INACTIVE(&psLP8App->sOutBufCtrl);
	// Remove audio codec output buffer from play channel.
	Playback_Remove(psLP8App->u8PlaybackChannel);
}
	
//---------------------------------------------------------------------------------------------------------
BOOL LP8App_DecodeProcess(S_LP8_APP *psLP8App)
{
	INT16 *pi16OutBuf;
	
	if (BUF_CTRL_IS_INACTIVE(&psLP8App->sOutBufCtrl))
		return FALSE;
	
	while ( Playback_NeedUpdateOutputBuf(&psLP8App->sOutBufCtrl) )
	{
		// Check end of file
		if(LP8_DecodeIsEnd((UINT8*)psLP8App->au32DecodeWorkBuf))
		{
			// Trigger inactive flag of output buffer to stop LP8 decoding
			BUF_CTRL_SET_INACTIVE(&psLP8App->sOutBufCtrl);
			// Use to represnt no active(or end) of decoding
			psLP8App->sOutBufCtrl.u16SampleRate = 0; 
			return FALSE;
		}

		// Record output data buffer pointer(for duplicate & process)
		pi16OutBuf = (PINT16)&psLP8App->sOutBufCtrl.pi16Buf[psLP8App->sOutBufCtrl.u16BufWriteIdx];
		
		LP8_DecodeProcess(	(UINT8*)psLP8App->au32DecodeWorkBuf,
							NULL,
							pi16OutBuf, 
							g_asAppCallBack[psLP8App->u8CallbackIndex].pfnReadDataCallback,
							g_asAppCallBack[psLP8App->u8CallbackIndex].pfnUserEventCallback);

		// Update write index of output buffer and avoid buffer overflow
		Playback_UpdateOutputBuf(&psLP8App->sOutBufCtrl);	

		// Duplicate data into buffer for using duplication callback function.
		if ( psLP8App->u8CtrlFlag&(LP8APP_CTRL_DUPLICATE_TO_BUF|LP8APP_CTRL_DUPLICATE_TO_FUNC) )
		{
			if ( psLP8App->u8CtrlFlag & LP8APP_CTRL_DUPLICATE_TO_BUF )
				BufCtrl_WriteWithCount(psLP8App->psDuplicateOutBufCtrl, LP8APP_OUT_SAMPLES_PER_FRAME, pi16OutBuf );
			else 
				psLP8App->pfnDuplicateFunc(LP8APP_OUT_SAMPLES_PER_FRAME, pi16OutBuf);
		}
	}	
	return TRUE;
}
