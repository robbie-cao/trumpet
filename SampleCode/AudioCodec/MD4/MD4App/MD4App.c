#include <string.h>
#include "MD4App.h"
#include "AudioRom.h"
#include "PlaybackRecord.h"

extern S_AUDIO_CALLBACK g_asAppCallBack[];

//---------------------------------------------------------------------------------------------------------
void MD4App_DecodeInitiate(S_MD4_APP *psMD4App, UINT8 *pau8TempBuf, UINT32 u32CallbackIndex)
{
	memset( psMD4App, '\0', sizeof(S_MD4_APP) );
	// MD4 decoder will refer to this index and call which callback function of storage 
	psMD4App->u8CallbackIndex = (UINT8)u32CallbackIndex;
	BUF_CTRL_SET_INACTIVE(&psMD4App->sOutBufCtrl);
}

//---------------------------------------------------------------------------------------------------------
BOOL MD4App_DecodeStartPlayByID(S_MD4_APP *psMD4App, UINT32 u32AudioID, UINT32 u32RomStartAddr, UINT8 u8PlaybackChannel)
{
	S_ROM_AUDIO_CHUNK_INFO sAudioChunkInfo;
	
	// Get audio chunk information for audio chunk start address from ROM file.
	AudioRom_GetAudioChunkInfo( g_asAppCallBack[psMD4App->u8CallbackIndex].pfnReadDataCallback, 
								u32RomStartAddr, 
								u32AudioID, 
								&sAudioChunkInfo);	

	return MD4App_DecodeStartPlayByAddr(psMD4App, sAudioChunkInfo.u32AudioChunkAddr, u8PlaybackChannel);
}

//---------------------------------------------------------------------------------------------------------
BOOL MD4App_DecodeStartPlayByAddr(S_MD4_APP *psMD4App, UINT32 u32MD4StorageStartAddr, UINT8 u8PlaybackChannel)
{
	UINT16 u16SampleRate;
	
	// MD4 decoder initiates work buffer and returns sample rate.
	if ( (u16SampleRate = MD4_DecodeInitiate(	(UINT8*)psMD4App->au32DecodeWorkBuf, 
												NULL, 
												u32MD4StorageStartAddr, 
												g_asAppCallBack[psMD4App->u8CallbackIndex].pfnReadDataCallback )) == 0 )
		return FALSE;	

	#if (MD4APP_OUT_SAMPLES_PER_FRAME != MD4_DECODE_SAMPLE_PER_FRAME )
	// Programer can change samples per frame, default value is 8 samples
	MD4_DecodeSampleCounts((UINT8*)psMD4App->au32DecodeWorkBuf, MD4APP_OUT_SAMPLES_PER_FRAME);
	#endif
	
	// Initiate and set output buffer variable(include frame size, buffer size etc.) 
	Playback_SetOutputBuf( 	&psMD4App->sOutBufCtrl,
							MD4APP_OUT_BUF_SIZE,
							psMD4App->i16OutBuf,
							MD4APP_OUT_SAMPLES_PER_FRAME,
							u16SampleRate );

	// Trigger active flag of output buffer for MD4 decoding
	BUF_CTRL_SET_ACTIVE(&psMD4App->sOutBufCtrl);
	
	// Pre-decode one frame
	psMD4App->sOutBufCtrl.u16BufWriteIdx = MD4APP_OUT_SAMPLES_PER_FRAME;
	if ( MD4App_DecodeProcess(psMD4App) == FALSE )
	{
		BUF_CTRL_SET_INACTIVE(&psMD4App->sOutBufCtrl);
		return FALSE;
	}
	psMD4App->sOutBufCtrl.u16BufReadIdx = MD4APP_OUT_SAMPLES_PER_FRAME;
	
	// Record play channel index for stopping to play.
	psMD4App->u8PlaybackChannel = u8PlaybackChannel;
	// Add audio codec into channel and preper to play codec.
	Playback_Add(psMD4App->u8PlaybackChannel, &psMD4App->sOutBufCtrl);

	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
void MD4App_DecodeStopPlay(S_MD4_APP *psMD4App)
{
	// Clear active flag of output buffer for stoping MD4 decode.
	BUF_CTRL_SET_INACTIVE(&psMD4App->sOutBufCtrl);
	// Remove audio codec output buffer from play channel.
	Playback_Remove(psMD4App->u8PlaybackChannel);
}
	
//---------------------------------------------------------------------------------------------------------
BOOL MD4App_DecodeProcess(S_MD4_APP *psMD4App)
{
	INT16 *pi16OutBuf;
	
	if (BUF_CTRL_IS_INACTIVE(&psMD4App->sOutBufCtrl))
		return FALSE;
	
	while ( Playback_NeedUpdateOutputBuf(&psMD4App->sOutBufCtrl) )
	{
		// Check end of file
		if(MD4_DecodeIsEnd((UINT8*)psMD4App->au32DecodeWorkBuf))
		{
			// Trigger inactive flag of output buffer to stop MD4 decoding
			BUF_CTRL_SET_INACTIVE(&psMD4App->sOutBufCtrl);
			// Use to represnt no active(or end) of decoding
			psMD4App->sOutBufCtrl.u16SampleRate = 0; 
			return FALSE;
		}

		// Record output data buffer pointer(for duplicate & process)
		pi16OutBuf = (PINT16)&psMD4App->sOutBufCtrl.pi16Buf[psMD4App->sOutBufCtrl.u16BufWriteIdx];
		
		MD4_DecodeProcess(	(UINT8*)psMD4App->au32DecodeWorkBuf, 
							NULL, 
							pi16OutBuf, 
							g_asAppCallBack[psMD4App->u8CallbackIndex].pfnReadDataCallback, 
							g_asAppCallBack[psMD4App->u8CallbackIndex].pfnUserEventCallback);

		// Update write index of output buffer and avoid buffer overflow
		Playback_UpdateOutputBuf(&psMD4App->sOutBufCtrl);	

		// Duplicate data into buffer for using duplication callback function.
		if ( psMD4App->u8CtrlFlag&(MD4APP_CTRL_DUPLICATE_TO_BUF|MD4APP_CTRL_DUPLICATE_TO_FUNC) )
		{
			if ( psMD4App->u8CtrlFlag & MD4APP_CTRL_DUPLICATE_TO_BUF )
				BufCtrl_WriteWithCount(psMD4App->psDuplicateOutBufCtrl, MD4APP_OUT_SAMPLES_PER_FRAME, pi16OutBuf );
			else 
				psMD4App->pfnDuplicateFunc(MD4APP_OUT_SAMPLES_PER_FRAME, pi16OutBuf);
		}
	}	
	return TRUE;
}
