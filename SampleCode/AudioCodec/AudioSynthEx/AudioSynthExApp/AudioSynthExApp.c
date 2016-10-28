#include "AudioSynthExApp.h"
#include "MicSpk.h"
#include <string.h>

extern S_AUDIO_CALLBACK g_asAppCallBack[];

DECLARE_AUDIOSYNTHEXAPP_DECODER()

// ---------------------------------------------------------------------------------------------
// Silence and Ramp down related functions
// --------------------------------------------------------------------------------------------- 
#define AUDIOSYNTHEX_SIL_SAMPLE_RATE		16000
#define AUDIOSYNTHEX_SIL_SAMPLE_PER_FRAME	8
typedef struct
{
	UINT32 u32SampleCount;
}S_AUDIOSYNTHEX_APP_SIL_CTRL;


//---------------------------------------------------------------------------------------------------------
UINT32 AudioSynthExApp_SilenceInitiate(
	UINT8 *pu8DecodeWorkBuf,
	UINT8 *pu8DecodeTempBuf,		
	UINT32 u32StartAddr,
	PFN_AUDIO_DATAREQUEST pfnReadDataCallback
)
{
	return AUDIOSYNTHEX_SIL_SAMPLE_RATE;
}

//---------------------------------------------------------------------------------------------------------
INT32 AudioSynthExApp_SilenceProcess(
	UINT8 *pu8DecodeWorkBuf,
	UINT8 *pu8DecodeTempBuf,		
	PINT16 pi16DecodedPcmBuf,
	PFN_AUDIO_DATAREQUEST pfnReadDataCallback,
	PFN_AUDIO_USREVENT pfnUserEventCallback
)
{
	S_AUDIOSYNTHEX_APP_SIL_CTRL *pu8SilenceWorkBuf;
	UINT8 u8DecoedPcmCount, i;

	pu8SilenceWorkBuf = (S_AUDIOSYNTHEX_APP_SIL_CTRL*)pu8DecodeWorkBuf;

	if ( pu8SilenceWorkBuf->u32SampleCount == 0 )
		return 0;

	if ( pu8SilenceWorkBuf->u32SampleCount > AUDIOSYNTHEX_SIL_SAMPLE_PER_FRAME )
		u8DecoedPcmCount = AUDIOSYNTHEX_SIL_SAMPLE_PER_FRAME;
	else
		u8DecoedPcmCount = pu8SilenceWorkBuf->u32SampleCount;

	pu8SilenceWorkBuf->u32SampleCount -= u8DecoedPcmCount;
	i = u8DecoedPcmCount;
	
	while(i--)
		*pi16DecodedPcmBuf ++ = 0; 

	return AUDIOSYNTHEX_SIL_SAMPLE_PER_FRAME;
}

//---------------------------------------------------------------------------------------------------------
BOOL AudioSynthExApp_SilenceIsEnd(
	UINT8 *pu8DecodeWorkBuf
)
{
	if (((S_AUDIOSYNTHEX_APP_SIL_CTRL*)pu8DecodeWorkBuf)->u32SampleCount)
		return FALSE;

	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
static __inline
void AudioSynthExApp_SilenceConfig(
	UINT8 *pu8DecodeWorkBuf,
	UINT32 u32SilenceSampleCount
)
{
	((S_AUDIOSYNTHEX_APP_SIL_CTRL*)pu8DecodeWorkBuf)->u32SampleCount =  u32SilenceSampleCount;
}

const S_AUDIOSYNTHEX_APP_DECODER g_sSilenceDecoder  = {AudioSynthExApp_SilenceInitiate, AudioSynthExApp_SilenceProcess, AudioSynthExApp_SilenceIsEnd, AUDIO_FMT_SILENCE};

// ---------------------------------------------------------------------------------------------
// AudioSynthEx related functions
// ---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------
void AudioSynthExApp_GetRomHeadInfo(
	S_ROM_HEADER *psRomHeader,
	UINT32 u32ROMStartAdd,						
	UINT8 u8StorageIndex
)
{	
	g_asAppCallBack[u8StorageIndex].pfnReadDataCallback(psRomHeader, u32ROMStartAdd, ROM_HEADER_SIZE);
}

//---------------------------------------------------------------------------------------------------------
void AudioSynthExApp_DecodeInitiate(
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp,
	UINT8 *pau8TempBuf,
	UINT32 u32ROMStartAdd													
)
{
	psAudioSynthExApp->pfnDecoder = NULL;
	psAudioSynthExApp->u8StorIndex = (UINT8)-1;
	psAudioSynthExApp->pau32AudioIdList = NULL;
	psAudioSynthExApp->u32ROMStartAddr = u32ROMStartAdd;
	psAudioSynthExApp->pau8TempBuf = pau8TempBuf;
	
	psAudioSynthExApp->sOutBufCtrl.pi16Buf = psAudioSynthExApp->i16OutPcmBuf;
	psAudioSynthExApp->sOutBufCtrl.u16BufCount = AUDIOSYNTHEXAPP_OUT_BUF_SIZE;
	psAudioSynthExApp->sOutBufCtrl.u16SampleRate = 0; // Use to represnt no active(or end) of decoding
	psAudioSynthExApp->sOutBufCtrl.u8Flag = S_BUF_CTRL_FLAG_BUF;
	BUF_CTRL_SET_INACTIVE(&psAudioSynthExApp->sOutBufCtrl);
}

//---------------------------------------------------------------------------------------------------------
S_AUDIOSYNTHEX_APP_DECODER * AudioSynthExApp_CheckAndGetDecoder(
	UINT16 u16AudioFormatID
)
{
	S_AUDIOSYNTHEX_APP_DECODER *psDecoder = (S_AUDIOSYNTHEX_APP_DECODER *)g_sAudioSynthExDecoderList;
	// Check audio's format and get decoder list.
	while( psDecoder->u16FormatID != NULL )
	{
		if ( psDecoder->u16FormatID == u16AudioFormatID )
			return psDecoder;
		else if( ( 	psDecoder->u16FormatID == AUDIO_FMT_NUONE_E 	|| 
					psDecoder->u16FormatID == AUDIO_FMT_NULITE_E 	||
					psDecoder->u16FormatID == AUDIO_FMT_IMAADPCM_E	) && 
				 ( psDecoder->u16FormatID == ( u16AudioFormatID + 1 ) ) )
			return psDecoder;
		
		psDecoder ++;
	}
	
	return NULL;
}

//---------------------------------------------------------------------------------------------------------
BOOL AudioSynthExApp_DecodeInitiateFromAudioChunk(
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp,
	UINT32 u32AudioChunkAddr, UINT32 u32AudioChunkSize,
	BOOL bChangeSR
)
{
	S_AUDIOCHUNK_HEADER sAudioChunkHeader;
	PFN_AUDIO_DATAREQUEST pfnReadDataCallback;	
	UINT16 u16SampleRate;

	pfnReadDataCallback = g_asAppCallBack[psAudioSynthExApp->u8StorIndex].pfnReadDataCallback;
	
	// Get Audio's format and sample rate.
	pfnReadDataCallback(&sAudioChunkHeader, psAudioSynthExApp->u32ROMStartAddr+u32AudioChunkAddr, 4 );
	if ( (psAudioSynthExApp->pfnDecoder = AudioSynthExApp_CheckAndGetDecoder(sAudioChunkHeader.u16FormatType)) == NULL )
		return FALSE;
	// Get Audio's format whether midi.
	if ( psAudioSynthExApp->pfnDecoder->u16FormatID == AUDIO_FMT_IMFMIDI )
	{
		// Configure midi buffer resource, wave table address, audio information and volume
		AudioSyntheExApp_MIDIConfig((UINT8 *)&(psAudioSynthExApp->sWorkBuf),		
			psAudioSynthExApp->u32ROMStartAddr+u32AudioChunkAddr, u32AudioChunkSize,
			pfnReadDataCallback,
			g_asAppCallBack[psAudioSynthExApp->u8StorIndex].pfnReadMidiWavTableCallback);
	}
	
	// Initiate codec according to decoder list
	u16SampleRate = psAudioSynthExApp->pfnDecoder->pfnDecodeInitiate(
		(UINT8 *)&(psAudioSynthExApp->sWorkBuf),
		(psAudioSynthExApp->pau8TempBuf),
		psAudioSynthExApp->u32ROMStartAddr+u32AudioChunkAddr,
		pfnReadDataCallback);
	if ( u16SampleRate == 0 )
		 return FALSE;
	psAudioSynthExApp->u16DecodedDataCount = 0;
	
	if ( bChangeSR )
	{
		// Trigger output buffer controlling for AudioSynthEx playback
		BUF_CTRL_SET_ACTIVE(&psAudioSynthExApp->sOutBufCtrl);
		psAudioSynthExApp->sOutBufCtrl.u16SampleRate = u16SampleRate;
		// Assign a channel number for mixer playing
		Playback_Add(psAudioSynthExApp->u8ChannelID,&psAudioSynthExApp->sOutBufCtrl);
	}
	
	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
BOOL AudioSynthExApp_GetAudioTrunkInfo(
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp,	// [Input] Audio's descriptor
	UINT32 u32AudioID,
	S_ROM_AUDIO_CHUNK_INFO *psAudioChunkInfo
)
{ 
	S_ROM_HEADER sRomHeader;
	PFN_AUDIO_DATAREQUEST pfnReadDataCallback;	
	
	psAudioSynthExApp->u8StorIndex = AUDIO_GET_STORAGE_INDEX(u32AudioID);
	pfnReadDataCallback = g_asAppCallBack[psAudioSynthExApp->u8StorIndex].pfnReadDataCallback;
	// Get ROM file header information
	pfnReadDataCallback(&sRomHeader, psAudioSynthExApp->u32ROMStartAddr, ROM_HEADER_SIZE);
	// Mark play audio not play equation
	psAudioSynthExApp->u32EquDataAddr = (UINT32)-1;	
	
	u32AudioID = AUDIO_GET_ID(u32AudioID);
	if (u32AudioID >= sRomHeader.u32TotalAudioNum)
		return FALSE;
	// Get audio chunk information, address = (ROM Start Addr) + ROM_HEADER_SIZE +(Audio ID)*(ROM_AUDIO_CHINK_INFO_SIZE)
	pfnReadDataCallback(psAudioChunkInfo,
		psAudioSynthExApp->u32ROMStartAddr+ROM_HEADER_SIZE+u32AudioID*ROM_AUDIO_CHINK_INFO_SIZE, ROM_AUDIO_CHINK_INFO_SIZE );
	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
BOOL AudioSynthExApp_DecodeInitiateByAudioID(			
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp,	
	UINT32 u32AudioID						
)
{
	S_ROM_AUDIO_CHUNK_INFO sAudioChunkInfo;
	
	if ( AudioSynthExApp_GetAudioTrunkInfo(psAudioSynthExApp, u32AudioID, &sAudioChunkInfo) == FALSE )
		return FALSE;
	if ( AudioSynthExApp_DecodeInitiateFromAudioChunk(psAudioSynthExApp,sAudioChunkInfo.u32AudioChunkAddr,sAudioChunkInfo.u32AudioChunkSize, TRUE) == FALSE )
		return FALSE;
	
	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
BOOL AudioSynthExApp_StartPlayAudio(			
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp,	
	UINT32 u32AudioID,
	UINT8 u8Channel
)
{
	UINT16 u16SampleRate;
	
	psAudioSynthExApp->pau32AudioIdList = NULL;
	
	psAudioSynthExApp->u8ChannelID = u8Channel;
	
	if ( AudioSynthExApp_DecodeInitiateByAudioID(psAudioSynthExApp, u32AudioID ) == FALSE )
		return FALSE;
	
	u16SampleRate = psAudioSynthExApp->sOutBufCtrl.u16SampleRate;
	// Initiate and set output buffer variable(include frame size, buffer size etc.) 
	Playback_SetOutputBuf( 	&psAudioSynthExApp->sOutBufCtrl,
							AUDIOSYNTHEXAPP_OUT_BUF_SIZE,
							psAudioSynthExApp->i16OutPcmBuf,
							AUDIOSYNTHEXAPP_OUT_SAMPLES_PER_FRAME,
							u16SampleRate );
	
	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
BOOL AudioSynthExApp_DecodeInitiateFromEqu(S_AUDIOSYNTHEX_APP *psAudioSynthExApp)
{
	S_ROM_SENTENCE_INFO sSentenceInfo;
	PFN_AUDIO_DATAREQUEST pfnReadDataCallback;	
	UINT16 u16SampleRate;
	
	pfnReadDataCallback = g_asAppCallBack[psAudioSynthExApp->u8StorIndex].pfnReadDataCallback;
	
	while(1)
	{	
		pfnReadDataCallback(&sSentenceInfo, psAudioSynthExApp->u32ROMStartAddr+psAudioSynthExApp->u32EquDataAddr, ROM_SENTENCE_INFO_SIZE);
		if ( sSentenceInfo.u16FormatType == ROM_SENTENCE_INFO_FORMAT_END )
			return FALSE;
		else if ( sSentenceInfo.u16FormatType == AUDIO_FMT_SILENCE ) // silence frame
		{
			sSentenceInfo.u16Repeat = 1;
			psAudioSynthExApp->pfnDecoder = (S_AUDIOSYNTHEX_APP_DECODER*)&g_sSilenceDecoder;
			AudioSynthExApp_SilenceConfig((UINT8 *)&(psAudioSynthExApp->sWorkBuf), sSentenceInfo.u32SilenceSampleCount);
			break;
		}
		else if ( (psAudioSynthExApp->pfnDecoder = AudioSynthExApp_CheckAndGetDecoder(sSentenceInfo.u16FormatType)) != NULL )
			break;
		// The current codec is not supported. Skip to check next.
		psAudioSynthExApp->u32EquDataAddr += ROM_SENTENCE_INFO_SIZE;
	}
	if ( psAudioSynthExApp->pfnDecoder->u16FormatID == AUDIO_FMT_IMFMIDI )
	{
		AudioSyntheExApp_MIDIConfig((UINT8 *)&(psAudioSynthExApp->sWorkBuf),		
			psAudioSynthExApp->u32ROMStartAddr+sSentenceInfo.u32AudioChunkAddr, sSentenceInfo.u32AudioChunkSize, pfnReadDataCallback,
			g_asAppCallBack[psAudioSynthExApp->u8StorIndex].pfnReadMidiWavTableCallback);
	}
	psAudioSynthExApp->u8Repeat = sSentenceInfo.u16Repeat;
 	
	u16SampleRate = psAudioSynthExApp->pfnDecoder->pfnDecodeInitiate(
		(UINT8 *)&(psAudioSynthExApp->sWorkBuf),
		psAudioSynthExApp->pau8TempBuf,
		psAudioSynthExApp->u32ROMStartAddr+sSentenceInfo.u32AudioChunkAddr,
		pfnReadDataCallback);

	if ( u16SampleRate == 0 )
		 return FALSE;
	if ( psAudioSynthExApp->pfnDecoder->u16FormatID == AUDIO_FMT_IMFMIDI )
		sSentenceInfo.u16SmplRate = u16SampleRate;

	psAudioSynthExApp->u16DecodedDataCount = 0;	

	BUF_CTRL_SET_ACTIVE(&psAudioSynthExApp->sOutBufCtrl);
	psAudioSynthExApp->sOutBufCtrl.u16SampleRate = sSentenceInfo.u16SmplRate;
	Playback_Add(psAudioSynthExApp->u8ChannelID,&psAudioSynthExApp->sOutBufCtrl);

	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
BOOL AudioSynthExApp_DecodeInitiateByEquID(
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp,
	UINT32 u32EquationID
)
{
	S_ROM_HEADER sRomHeader;
	PFN_AUDIO_DATAREQUEST pfnReadDataCallback;	
	
	psAudioSynthExApp->u8StorIndex = AUDIO_GET_STORAGE_INDEX(u32EquationID);
	u32EquationID = AUDIO_GET_ID(u32EquationID);
	pfnReadDataCallback = g_asAppCallBack[psAudioSynthExApp->u8StorIndex].pfnReadDataCallback;
	
	pfnReadDataCallback(&sRomHeader, psAudioSynthExApp->u32ROMStartAddr, ROM_HEADER_SIZE);
	
	if ( sRomHeader.u32TotalSentenceNum < u32EquationID )
		return FALSE;

	if ((sRomHeader.u32SentenceStartAddr) > (ROM_HEADER_SIZE+sRomHeader.u32TotalAudioNum*ROM_AUDIO_CHINK_INFO_SIZE) )
	{
		// Search equation start address by index table
		pfnReadDataCallback(
			&psAudioSynthExApp->u32EquDataAddr,
			psAudioSynthExApp->u32ROMStartAddr + ROM_HEADER_SIZE + sRomHeader.u32TotalAudioNum*ROM_AUDIO_CHINK_INFO_SIZE + u32EquationID*4, 4 );
	}
	else
	{
		// Search equation start address by sequentail search
		UINT16 u16Format;
		UINT32 u32EquDataAddr;

		u32EquDataAddr = sRomHeader.u32SentenceStartAddr;
		while(u32EquationID)
		{
			pfnReadDataCallback(&u16Format, psAudioSynthExApp->u32ROMStartAddr+u32EquDataAddr, ROM_SENTENCE_INFO_FORMAT_FIELD_SIZE );
			if (u16Format != ROM_SENTENCE_INFO_FORMAT_END )
			{
				u32EquDataAddr += ROM_SENTENCE_INFO_SIZE;
				continue;
			}
			u32EquDataAddr += ROM_SENTENCE_INFO_FORMAT_FIELD_SIZE;
			u32EquationID --;	
		}
		psAudioSynthExApp->u32EquDataAddr = u32EquDataAddr;
	}

	if (AudioSynthExApp_DecodeInitiateFromEqu(psAudioSynthExApp) == FALSE)
		return FALSE;

	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
BOOL AudioSynthExApp_StartPlayEquation(			
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp,
	UINT32 u32EquationID,
	UINT8 u8Channel
)
{
	UINT16 u16SampleRate;
	
	psAudioSynthExApp->pau32AudioIdList = NULL;
	
	psAudioSynthExApp->u8ChannelID = u8Channel;
	
	if ( AudioSynthExApp_DecodeInitiateByEquID(psAudioSynthExApp, u32EquationID) == FALSE )
		return FALSE;
	
	u16SampleRate = psAudioSynthExApp->sOutBufCtrl.u16SampleRate;
	// Initiate and set output buffer variable(include frame size, buffer size etc.) 
	Playback_SetOutputBuf( 	&psAudioSynthExApp->sOutBufCtrl,
							AUDIOSYNTHEXAPP_OUT_BUF_SIZE,
							psAudioSynthExApp->i16OutPcmBuf,
							AUDIOSYNTHEXAPP_OUT_SAMPLES_PER_FRAME,
							u16SampleRate );
	
	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
BOOL AudioSynthExApp_DecodeInitiateFromAudioIdList(
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp
)
{
	UINT32 u32AudioID;

	if (( psAudioSynthExApp->pau32AudioIdList == NULL) || 
		(*psAudioSynthExApp->pau32AudioIdList == AUDIOSYNTHEXAPP_AUIDO_LIST_END_MARK))	
	{
		psAudioSynthExApp->pau32AudioIdList = NULL;
		return FALSE;
	}
	
	u32AudioID = *psAudioSynthExApp->pau32AudioIdList ++;
	// Initate to play next audio from ID list pointed by "psAudioSynthExApp->pau32AudioIdList"
	if ( AUDIOAYNTHEXAPP_IS_AUDIO_ID(u32AudioID) )
	{
		if ( AudioSynthExApp_DecodeInitiateByAudioID(psAudioSynthExApp, u32AudioID ) == FALSE )
			return FALSE;
	}
	else
	{
		if ( AudioSynthExApp_DecodeInitiateByEquID(psAudioSynthExApp, u32AudioID ) == FALSE )
			return FALSE;
	}
	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
BOOL AudioSynthExApp_StartPlayAudioList(
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp,	
	UINT32 *pau32AudioIdList,
	UINT8 u8Channel
)
{	
	UINT16 u16SampleRate;
	
	psAudioSynthExApp->pau32AudioIdList = pau32AudioIdList;
	
	psAudioSynthExApp->u8ChannelID = u8Channel;
	
	if ( AudioSynthExApp_DecodeInitiateFromAudioIdList(psAudioSynthExApp) == FALSE )
		return FALSE;
	
	u16SampleRate = psAudioSynthExApp->sOutBufCtrl.u16SampleRate;
	// Initiate and set output buffer variable(include frame size, buffer size etc.) 
	Playback_SetOutputBuf( 	&psAudioSynthExApp->sOutBufCtrl,
							AUDIOSYNTHEXAPP_OUT_BUF_SIZE,
							psAudioSynthExApp->i16OutPcmBuf,
							AUDIOSYNTHEXAPP_OUT_SAMPLES_PER_FRAME,
							u16SampleRate );
	
	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
BOOL AudioSynthExApp_StartPlay(			
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp,
	UINT32 u32AudioOrEquationID,
	UINT8 u8Channel
)
{	
	psAudioSynthExApp->u8ChannelID = u8Channel;
	
	if ( AUDIOAYNTHEXAPP_IS_EQUATION_ID(u32AudioOrEquationID) )
		return AudioSynthExApp_StartPlayEquation(psAudioSynthExApp, u32AudioOrEquationID, u8Channel);
	return AudioSynthExApp_StartPlayAudio(psAudioSynthExApp, u32AudioOrEquationID, u8Channel);
}

//---------------------------------------------------------------------------------------------------------
BOOL AudioSynthExApp_DecodeProcess(
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp
)
{
	UINT32 u32SampleCount;
	UINT16 u16CopyCount;
	INT16 *pi16DecodePcmBuf, *pi16DesAddr;
	UINT8 *pau8WorkBuff;
	S_AUDIO_CALLBACK *psCallbackFunc;
	S_AUDIOSYNTHEX_APP_DECODER *psDecoder;
	S_BUF_CTRL *pOutBufCtrl;

	if ( (psDecoder = psAudioSynthExApp->pfnDecoder) == NULL )		 // This is to avoid no correct decoder found in AudioSynthExApp_DecodeInitiateFromAudioChunk()
		goto AudioSynthExApp_DecodeProcessEnd;

	pOutBufCtrl = &psAudioSynthExApp->sOutBufCtrl;
	while (Playback_NeedUpdateOutputBuf(&psAudioSynthExApp->sOutBufCtrl))
	{		
		psCallbackFunc = &g_asAppCallBack[psAudioSynthExApp->u8StorIndex];
					
		pi16DesAddr = &pOutBufCtrl->pi16Buf[pOutBufCtrl->u16BufWriteIdx];
		u32SampleCount = AUDIOSYNTHEXAPP_OUT_SAMPLES_PER_FRAME;
		pau8WorkBuff = (UINT8 *)&(psAudioSynthExApp->sWorkBuf);
		while(u32SampleCount!=0)
		{
			if ( psAudioSynthExApp->u16DecodedDataCount == 0 )
			{
				// Check more data can be decoded or not
				if ( psDecoder->pfnDecodeIsEnd(pau8WorkBuff) )
				{
					if ( ( psAudioSynthExApp->u32EquDataAddr == (UINT32)-1 ) ||	// play audio only not play equation
						 ( psAudioSynthExApp->u8Repeat == 0 ) )					// no repeat and no audio to play in an equation
					{
						if ( psAudioSynthExApp->pau32AudioIdList != NULL )
						{
							// Go on playing next audio from audio list
							if ( AudioSynthExApp_DecodeInitiateFromAudioIdList(psAudioSynthExApp) == FALSE )
								goto AudioSynthExApp_DecodeProcessEnd;

							// Maybe the storage index of the next audio file is not the same as current.
							// Should get the correct callback function again! 
							psCallbackFunc = &g_asAppCallBack[psAudioSynthExApp->u8StorIndex];
							//psDecoder = psAudioSynthExApp->pfnDecoder;
						}
						else
							goto AudioSynthExApp_DecodeProcessEnd;
					}
					else if ( (-- psAudioSynthExApp->u8Repeat) == 0 )
					{
						if ( psAudioSynthExApp->pau32AudioIdList != NULL )
						{
							// Go on playing next audio from audio list
							if ( AudioSynthExApp_DecodeInitiateFromAudioIdList(psAudioSynthExApp) == FALSE )
								goto AudioSynthExApp_DecodeProcessEnd;

							// Maybe the storage index of the next audio file is not the same as current.
							// Should get the correct callback function again! 
							psCallbackFunc = &g_asAppCallBack[psAudioSynthExApp->u8StorIndex];
						}
						else
						{
							// Go on playing next audio from equation
							psAudioSynthExApp->u32EquDataAddr += ROM_SENTENCE_INFO_SIZE;
							if (AudioSynthExApp_DecodeInitiateFromEqu(psAudioSynthExApp) == FALSE)
								goto AudioSynthExApp_DecodeProcessEnd;
						}
						//psDecoder = psAudioSynthExApp->pfnDecoder;
					}
					else
					{
						// Continue playing current audio file
						S_ROM_SENTENCE_INFO sSentenceInfo;
						
						psCallbackFunc->pfnReadDataCallback(&sSentenceInfo, psAudioSynthExApp->u32ROMStartAddr+psAudioSynthExApp->u32EquDataAddr, ROM_SENTENCE_INFO_SIZE);;
						if ( AudioSynthExApp_DecodeInitiateFromAudioChunk(psAudioSynthExApp, sSentenceInfo.u32AudioChunkAddr, sSentenceInfo.u32AudioChunkSize, FALSE) == FALSE )
							goto AudioSynthExApp_DecodeProcessEnd;
					}
					
					// Maybe the format of next audio file is not the same as current.
					// Should get the correct decoder again!
					psDecoder = psAudioSynthExApp->pfnDecoder;
				}
				do
				{
					// This is special for NuSound decoding, because the first 8 frames have no data!
					psAudioSynthExApp->u16DecodedDataCount = 
						psDecoder->pfnDecodeProcess(
							pau8WorkBuff, psAudioSynthExApp->pau8TempBuf, (INT16 *)&(psAudioSynthExApp->sDecodePcmBuf),
							psCallbackFunc->pfnReadDataCallback, psCallbackFunc->pfnUserEventCallback );
					if ( psDecoder->pfnDecodeIsEnd(pau8WorkBuff) )
						break;
				}while( psAudioSynthExApp->u16DecodedDataCount == 0 );
				psAudioSynthExApp->u16ReadDataCount = 0;
			}
			
			if ( u32SampleCount < psAudioSynthExApp->u16DecodedDataCount )
				u16CopyCount = u32SampleCount;
			else
				u16CopyCount = psAudioSynthExApp->u16DecodedDataCount;
			
			pi16DecodePcmBuf = (INT16 *)&(psAudioSynthExApp->sDecodePcmBuf) + psAudioSynthExApp->u16ReadDataCount;
			u32SampleCount -= u16CopyCount;
			psAudioSynthExApp->u16DecodedDataCount -= u16CopyCount;
			psAudioSynthExApp->u16ReadDataCount += u16CopyCount;
			while(u16CopyCount--)
				*pi16DesAddr ++ = *pi16DecodePcmBuf ++;
		}
		// Update write index of output buffer and avoid buffer overflow
		Playback_UpdateOutputBuf(&psAudioSynthExApp->sOutBufCtrl);	
		
		// Duplicate data into buffer for using duplication callback function.
		if ( psAudioSynthExApp->u8CtrlFlag&(AUDIOSYNTHEXAPP_CTRL_DUPLICATE_TO_BUF|AUDIOSYNTHEXAPP_CTRL_DUPLICATE_TO_FUNC) )
		{
			if ( psAudioSynthExApp->u8CtrlFlag & AUDIOSYNTHEXAPP_CTRL_DUPLICATE_TO_BUF )
				BufCtrl_WriteWithCount(psAudioSynthExApp->psDuplicateOutBufCtrl, AUDIOSYNTHEXAPP_OUT_SAMPLES_PER_FRAME, (INT16 *)&(psAudioSynthExApp->sDecodePcmBuf) );
			else 
				psAudioSynthExApp->pfnDuplicateFunc(AUDIOSYNTHEXAPP_OUT_SAMPLES_PER_FRAME, (INT16 *)&(psAudioSynthExApp->sDecodePcmBuf));
		}
	}
	return TRUE;
	
AudioSynthExApp_DecodeProcessEnd:
	psAudioSynthExApp->sOutBufCtrl.u16SampleRate = 0; // Use to represnt no active(or end) of decoding
	BUF_CTRL_SET_INACTIVE(&psAudioSynthExApp->sOutBufCtrl);
	
	return FALSE;
}

//---------------------------------------------------------------------------------------------------------
void 
AudioSynthExApp_DecodeStopPlay(
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp
)
{
	// Clear active flag of output buffer for stoping AudioSynthEx.
	BUF_CTRL_SET_INACTIVE(&psAudioSynthExApp->sOutBufCtrl);
	// Remove audio codec output buffer from play channel.
	Playback_Remove(psAudioSynthExApp->u8ChannelID);
}

__weak 
void AudioSyntheExApp_MIDIConfig(UINT8 *pu8DecodeWorkBuf,		
	UINT32 u32StartAddr, UINT32 u32DataSize,
	PFN_AUDIO_DATAREQUEST pfnReadDataCallback,
	PFN_AUDIO_DATAREQUEST pfnReadWavetableCallback)
{
}
