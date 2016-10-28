#include <string.h>
#include "ImaAdpcmApp_Encode.h"
#include "PlaybackRecord.h"

UINT32 u32TotalEncodeDataByte=0;

void ImaAdpcmApp_EncodeInitiate(S_IMAADPCM_APP_ENCODE *psImaAdpcmAppEncode, UINT8 *pu8EncodeTempBuf)
{
	memset( psImaAdpcmAppEncode, '\0', sizeof(S_IMAADPCM_APP_ENCODE) );
	
	// Initiate buffer controlling variables for encoding
	psImaAdpcmAppEncode->sEncodeBufCtrl.pi16Buf = (INT16*)psImaAdpcmAppEncode->au32EncodeBuf;
	psImaAdpcmAppEncode->sEncodeBufCtrl.u16SampleRate = 0;
	psImaAdpcmAppEncode->pau8TempBuf = pu8EncodeTempBuf;

	psImaAdpcmAppEncode->i32EncodeDataByte=0;
	
	u32TotalEncodeDataByte=0; // total bytes of encoded data
	BUF_CTRL_SET_INACTIVE(&psImaAdpcmAppEncode->sInBufCtrl);
}

BOOL ImaAdpcmApp_EncodeStart( S_IMAADPCM_APP_ENCODE *psImaAdpcmAppEncode, S_AUDIOCHUNK_HEADER *psAudioChunkHeader, 
							  UINT16 u16SampleRate, UINT32 u32BitPerFrame)
{
	if ( (u16SampleRate == 0) )
		return FALSE;
	else
	{
		psAudioChunkHeader->u16SmplRate = u16SampleRate;
		psAudioChunkHeader->u32BitPerFrame = IMAADPCM_ENCODE_BIT_PER_FRAME(IMAADPCMAPP_IN_SAMPLES_PER_FRAME);
	}
	
	// ImaAdpcm encoder initiates work buffer.
	// Set bit rate and sample rate information for audio chunk header.
	ImaAdpcm_EncodeInitiate((UINT8 *)psImaAdpcmAppEncode->au32WorkBuf, psImaAdpcmAppEncode->pau8TempBuf, 
		psAudioChunkHeader, (enum eImaAdpcmEncodeBPS)psAudioChunkHeader->u32BitPerFrame, psAudioChunkHeader->u16SmplRate);
	ImaAdpcm_EncodeSampleCount((UINT8 *)psImaAdpcmAppEncode->au32WorkBuf, IMAADPCMAPP_IN_SAMPLES_PER_FRAME);
			
	// Reset encode buffer read index and write index
	psImaAdpcmAppEncode->sEncodeBufCtrl.u16BufWriteIdx = 0;
	psImaAdpcmAppEncode->sEncodeBufCtrl.u16BufReadIdx = 0;
	
	// Set Encoded frame size, Storage Utility will refer to this size to write data.
	psImaAdpcmAppEncode->sEncodeBufCtrl.u16FrameSize =  (psAudioChunkHeader->u32BitPerFrame)>>3;
	psImaAdpcmAppEncode->sEncodeBufCtrl.u16BufCount = (psImaAdpcmAppEncode->sEncodeBufCtrl.u16FrameSize)*IMAADPCMAPP_ENCODE_BUF_COUNT;
	
	// Set input buffer size, PCM buffer pointer, frame size and sample rate.
	Record_SetInBufRecord(  &psImaAdpcmAppEncode->sInBufCtrl, 
							IMAADPCMAPP_IN_BUF_SIZE,
							psImaAdpcmAppEncode->i16InBuf,
							IMAADPCMAPP_IN_SAMPLES_PER_FRAME,
							psAudioChunkHeader->u16SmplRate);
					
	// Set application input buffer to record(ADC) output buffer.
	Record_Add(&psImaAdpcmAppEncode->sInBufCtrl, psAudioChunkHeader->u16SmplRate);
	
	return TRUE;
}

BOOL ImaAdpcmApp_EncodeProcess(S_IMAADPCM_APP_ENCODE *psImaAdpcmAppEncode)
{
	UINT32 u32DataLength;
	S_BUF_CTRL *psEncodeBufCtrl, *psInBufCtrl;
	
	if (BUF_CTRL_IS_INACTIVE(&psImaAdpcmAppEncode->sInBufCtrl))
		return FALSE;
	
	psEncodeBufCtrl = &psImaAdpcmAppEncode->sEncodeBufCtrl;
	psInBufCtrl = &psImaAdpcmAppEncode->sInBufCtrl;

	while(( psInBufCtrl->u16BufReadIdx > psInBufCtrl->u16BufWriteIdx )|| 
		(( psInBufCtrl->u16BufWriteIdx - psInBufCtrl->u16BufReadIdx)>= IMAADPCMAPP_IN_SAMPLES_PER_FRAME))
	{
		// Process encoding and return encoded length
		u32DataLength = ImaAdpcm_EncodeProcess((UINT8 *)psImaAdpcmAppEncode->au32WorkBuf, psImaAdpcmAppEncode->pau8TempBuf,
			psInBufCtrl->pi16Buf + psInBufCtrl->u16BufReadIdx,
			//((INT8*)(psEncodeBufCtrl->pi16Buf)) + psEncodeBufCtrl->u16BufWriteIdx );
			(INT8*)(psImaAdpcmAppEncode->ai8EncodeTemp + psImaAdpcmAppEncode->i32EncodeDataByte) ); // decode to temp buffer
		
		
		u32TotalEncodeDataByte+=u32DataLength;
		
		psImaAdpcmAppEncode->i32EncodeDataByte += u32DataLength;
		if (psImaAdpcmAppEncode->i32EncodeDataByte>=IMAADPCMAPP_ENCODE_BUF_SIZE)
		{
			// copy to original buffer
			memcpy((INT8*)psEncodeBufCtrl->pi16Buf+psEncodeBufCtrl->u16BufWriteIdx, psImaAdpcmAppEncode->ai8EncodeTemp, IMAADPCMAPP_ENCODE_BUF_SIZE); 
			psImaAdpcmAppEncode->i32EncodeDataByte -= IMAADPCMAPP_ENCODE_BUF_SIZE;
			// move forward
			memcpy(psImaAdpcmAppEncode->ai8EncodeTemp, psImaAdpcmAppEncode->ai8EncodeTemp+IMAADPCMAPP_ENCODE_BUF_SIZE, psImaAdpcmAppEncode->i32EncodeDataByte); 
			
			// Update write index of encoded buffer
			psEncodeBufCtrl->u16BufWriteIdx+=IMAADPCMAPP_ENCODE_BUF_SIZE;
			if (psEncodeBufCtrl->u16BufWriteIdx >= psImaAdpcmAppEncode->sEncodeBufCtrl.u16BufCount)
				psEncodeBufCtrl->u16BufWriteIdx = 0;
		}
		// Update read index of ADC input buffer
		psInBufCtrl->u16BufReadIdx+=IMAADPCMAPP_IN_SAMPLES_PER_FRAME;
		if (psInBufCtrl->u16BufReadIdx >= IMAADPCMAPP_IN_BUF_SIZE)
			psInBufCtrl->u16BufReadIdx = 0;
		
	}
	return TRUE;
}

void ImaAdpcmApp_EncodeEnd(S_IMAADPCM_APP_ENCODE *psImaAdpcmAppEncode)
{
	BUF_CTRL_SET_INACTIVE(&psImaAdpcmAppEncode->sInBufCtrl);
}

UINT32 ImaAdpcmApp_EncodeFlush(S_IMAADPCM_APP_ENCODE *psImaAdpcmAppEncode)
{
	UINT32 u32DataLength;
	S_BUF_CTRL *psEncodeBufCtrl;
	
	psEncodeBufCtrl = &psImaAdpcmAppEncode->sEncodeBufCtrl;

	// Process encoding and return encoded length
	u32DataLength = ImaAdpcm_EncodeFlush((UINT8 *)psImaAdpcmAppEncode->au32WorkBuf, psImaAdpcmAppEncode->pau8TempBuf,
		//((INT8*)(psEncodeBufCtrl->pi16Buf)) + psEncodeBufCtrl->u16BufWriteIdx );
		(INT8*)(psImaAdpcmAppEncode->ai8EncodeTemp + psImaAdpcmAppEncode->i32EncodeDataByte) ); // decode to temp buffer
	
	u32TotalEncodeDataByte+=u32DataLength;
	
	psImaAdpcmAppEncode->i32EncodeDataByte += u32DataLength;
	if (psImaAdpcmAppEncode->i32EncodeDataByte>=IMAADPCMAPP_ENCODE_BUF_SIZE)
	{
		// copy to original buffer
		memcpy((INT8*)psEncodeBufCtrl->pi16Buf+psEncodeBufCtrl->u16BufWriteIdx, psImaAdpcmAppEncode->ai8EncodeTemp, IMAADPCMAPP_ENCODE_BUF_SIZE); 
		psImaAdpcmAppEncode->i32EncodeDataByte -= IMAADPCMAPP_ENCODE_BUF_SIZE;
		// move forward
		memcpy(psImaAdpcmAppEncode->ai8EncodeTemp, psImaAdpcmAppEncode->ai8EncodeTemp+IMAADPCMAPP_ENCODE_BUF_SIZE, psImaAdpcmAppEncode->i32EncodeDataByte); 
			
		// Update write index of encoded buffer
		psEncodeBufCtrl->u16BufWriteIdx+=IMAADPCMAPP_ENCODE_BUF_SIZE;
		if (psEncodeBufCtrl->u16BufWriteIdx >= psImaAdpcmAppEncode->sEncodeBufCtrl.u16BufCount)
			psEncodeBufCtrl->u16BufWriteIdx = 0;
	}
	if (u32DataLength==0 && psImaAdpcmAppEncode->i32EncodeDataByte!=0)
	{
		memcpy((INT8*)psEncodeBufCtrl->pi16Buf+psEncodeBufCtrl->u16BufWriteIdx, psImaAdpcmAppEncode->ai8EncodeTemp, psImaAdpcmAppEncode->i32EncodeDataByte); // copy to original buffer
		memset((INT8*)psEncodeBufCtrl->pi16Buf+psEncodeBufCtrl->u16BufWriteIdx+psImaAdpcmAppEncode->i32EncodeDataByte, 0, IMAADPCMAPP_ENCODE_BUF_SIZE-psImaAdpcmAppEncode->i32EncodeDataByte);

		// Update write index of encoded buffer
		psEncodeBufCtrl->u16BufWriteIdx+=IMAADPCMAPP_ENCODE_BUF_SIZE;
		if (psEncodeBufCtrl->u16BufWriteIdx >= psImaAdpcmAppEncode->sEncodeBufCtrl.u16BufCount)
			psEncodeBufCtrl->u16BufWriteIdx = 0;
	}
	return u32DataLength;
}
