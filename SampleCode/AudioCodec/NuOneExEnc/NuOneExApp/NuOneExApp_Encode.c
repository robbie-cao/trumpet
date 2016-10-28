#include <string.h>
#include "NuOneExApp_Encode.h"
#include "PlaybackRecord.h"

void NuOneExApp_EncodeInitiate(S_NUONEEX_APP_ENCODE *psNuOneExAppEncode, UINT8 *pu8EncodeTempBuf)
{
	memset( psNuOneExAppEncode, '\0', sizeof(S_NUONEEX_APP_ENCODE) );
	
	// Initiate buffer controlling variables for encoding
	psNuOneExAppEncode->sEncodeBufCtrl.pi16Buf = (INT16*)psNuOneExAppEncode->au32EncodeBuf;
	psNuOneExAppEncode->sEncodeBufCtrl.u16SampleRate = 0;
	psNuOneExAppEncode->pau8TempBuf = pu8EncodeTempBuf;
	BUF_CTRL_SET_INACTIVE(&psNuOneExAppEncode->sInBufCtrl);
}

BOOL NuOneExApp_EncodeStart(S_NUONEEX_APP_ENCODE *psNuOneExAppEncode, S_AUDIOCHUNK_HEADER *psAudioChunkHeader,
							UINT16 u16SampleRate, enum eNuOneExEncodeBPS eBitPerSample)
{
	if ( (eBitPerSample > NUONEEXAPP_ENCODE_MAX_BITRATE) || (u16SampleRate == 0) )
		return FALSE;
	else
	{
		psAudioChunkHeader->u16SmplRate = u16SampleRate;
		psAudioChunkHeader->u32BitPerFrame = eBitPerSample;
	}
	
	// NuOneEx encoder initiates work buffer.
	// Set bit rate and sample rate information for audio chunk header.
	NuOneEx_EncodeInitiate((UINT8 *)psNuOneExAppEncode->au32WorkBuf, psNuOneExAppEncode->pau8TempBuf, 
		psAudioChunkHeader, (enum eNuOneExEncodeBPS)psAudioChunkHeader->u32BitPerFrame, psAudioChunkHeader->u16SmplRate);

	// Reset encode buffer read index and write index
	psNuOneExAppEncode->sEncodeBufCtrl.u16BufWriteIdx = 0;
	psNuOneExAppEncode->sEncodeBufCtrl.u16BufReadIdx = 0;
	
	// Set Encoded frame size, Storage Utility will refer to this size to write data.
	psNuOneExAppEncode->sEncodeBufCtrl.u16FrameSize =  (psAudioChunkHeader->u32BitPerFrame)>>3;
	psNuOneExAppEncode->sEncodeBufCtrl.u16BufCount = (psNuOneExAppEncode->sEncodeBufCtrl.u16FrameSize)*NUONEEXAPP_ENCODE_BUF_COUNT;

	// Set input buffer size, PCM buffer pointer, frame size and sample rate.
	Record_SetInBufRecord(  &psNuOneExAppEncode->sInBufCtrl, 
							NUONEEXAPP_IN_BUF_SIZE,
							psNuOneExAppEncode->i16InBuf,
							NUONEEX_ENCODE_SAMPLE_PER_FRAME,
							psAudioChunkHeader->u16SmplRate);
	
	// Set application input buffer to record(ADC) output buffer.
	Record_Add(&psNuOneExAppEncode->sInBufCtrl, psAudioChunkHeader->u16SmplRate);
	
	return TRUE;
}

BOOL NuOneExApp_EncodeProcess(S_NUONEEX_APP_ENCODE *psNuOneExAppEncode)
{
	UINT32 u32DataLength;
	S_BUF_CTRL *psEncodeBufCtrl, *psInBufCtrl;

	if (BUF_CTRL_IS_INACTIVE(&psNuOneExAppEncode->sInBufCtrl))
		return FALSE;
	
	psEncodeBufCtrl = &psNuOneExAppEncode->sEncodeBufCtrl;
	psInBufCtrl = &psNuOneExAppEncode->sInBufCtrl;

	while(( psInBufCtrl->u16BufReadIdx > psInBufCtrl->u16BufWriteIdx )|| 
		(( psInBufCtrl->u16BufWriteIdx - psInBufCtrl->u16BufReadIdx)>= NUONEEX_ENCODE_SAMPLE_PER_FRAME))
	{
		// Process encoding and return encoded length
		u32DataLength = NuOneEx_EncodeProcess((UINT8 *)psNuOneExAppEncode->au32WorkBuf, psNuOneExAppEncode->pau8TempBuf,
			psInBufCtrl->pi16Buf + psInBufCtrl->u16BufReadIdx,
			((INT8*)(psEncodeBufCtrl->pi16Buf)) + psEncodeBufCtrl->u16BufWriteIdx );
		
		// Update write index of encoded buffer
		if ((psEncodeBufCtrl->u16BufWriteIdx+=u32DataLength) >= psNuOneExAppEncode->sEncodeBufCtrl.u16BufCount)
			psEncodeBufCtrl->u16BufWriteIdx = 0;
		
		// Update read index of ADC input buffer
		if ((psInBufCtrl->u16BufReadIdx+=NUONEEX_ENCODE_SAMPLE_PER_FRAME) >= NUONEEXAPP_IN_BUF_SIZE)
			psInBufCtrl->u16BufReadIdx = 0;
	}
	return TRUE;
}

void NuOneExApp_EncodeEnd(S_NUONEEX_APP_ENCODE *psNuOneExAppEncode)
{
	BUF_CTRL_SET_INACTIVE(&psNuOneExAppEncode->sInBufCtrl);
}
