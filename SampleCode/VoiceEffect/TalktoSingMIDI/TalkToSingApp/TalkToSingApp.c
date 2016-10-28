/*--------------------------------------------------------------------------------------------------*/
/*                                                                                                  */
/* Copyright(c) 2010 Nuvoton Technology Corp. All rights reserved.                                  */
/*                                                                                                  */
/*--------------------------------------------------------------------------------------------------*/

#include "TalkToSingApp.h"
#include "SpiFlashMap.h"
#include <string.h>

extern S_SPIFLASH_HANDLER g_sSpiFlash;
extern S_AUDIO_CALLBACK g_asAppCallBack[];

#if (defined(LP8_FORMAT) || defined(PC8_FORMAT))
#define TALKTOSINGAPP_8BIT					TRUE
#define TALKTOSINGAPP_SKIP_SAMPLE_BYTE		(TALKTOSINGAPP_SKIP_SAMPLE_NUM)
#else		
#define TALKTOSINGAPP_8BIT					FALSE
#define TALKTOSINGAPP_SKIP_SAMPLE_BYTE		(TALKTOSINGAPP_SKIP_SAMPLE_NUM * 2)
#endif

//----------------------------------------------------------------------------------------------------
// Read information of recorded samples.
//----------------------------------------------------------------------------------------------------
BOOL
TalkToSingApp_ReadRecInfo(
	S_TALKTOSINGAPP *psTalkToSingApp)
{
	S_TALK2SING_PCM_HEADER *psPcmHeader = (S_TALK2SING_PCM_HEADER*)psTalkToSingApp->au8SylbInfoBuf;
	S_SPI_READ_SND* psSpiRecSnd = &psTalkToSingApp->sSpiReadSnd;

	// read header of recorded sound
	SPIFlash_Read(&g_sSpiFlash, TALKTOSINGAPP_REC_INFO_ADDR, psTalkToSingApp->au8SylbInfoBuf, 256);	

	// compare signature
	if (psPcmHeader->u32RecSignature == TALKTOSINGAPP_REC_SIGNATURE)
	{
		// init information for call back function to read recorded sound
#if (defined(LP8_FORMAT) || defined(PC8_FORMAT))
		psSpiRecSnd->m_u32RecAddr = TALKTOSINGAPP_REC_DATA_ADDR + psPcmHeader->u32SkipHeadSmplCnt;
#else
		psSpiRecSnd->m_u32RecAddr = TALKTOSINGAPP_REC_DATA_ADDR + (psPcmHeader->u32SkipHeadSmplCnt << 1);
#endif
		psSpiRecSnd->m_i32RecSmplCnt = psPcmHeader->u32EffectiveSmplCnt;
		psSpiRecSnd->m_i32RWSmplCnt = 0;

		// copy nomalization data
		memcpy(psTalkToSingApp->au16NormalizeBuf,
			psTalkToSingApp->au8SylbInfoBuf+sizeof(S_TALK2SING_PCM_HEADER),psPcmHeader->u32SylbNum*sizeof(INT16));
	
		// read syllables/pitches data
		SPIFlash_Read(&g_sSpiFlash,TALKTOSINGAPP_SYLBINFO_ADDR,psTalkToSingApp->au8SylbInfoBuf,psPcmHeader->u32UsedSylbPitchInfoBufSize);

		return (psPcmHeader->u32SylbNum > 0);
	}
	return FALSE;
}

//-----------------------------------------------------------------------------------------------------------
// Recording done and analysis talk to sing information.
//-----------------------------------------------------------------------------------------------------------
BOOL 										// TRUE: the recorded sound is legal to play Talk to Sing, FALSE: fail to play
TalkToSingApp_RecordingDone(
	S_TALKTOSINGAPP *psTalkToSingApp,		// Talk to Sing app data structure
	UINT32 u32RecSampleCnt, 				// total recorded samples number
	BOOL bSkipKeyPop						// skip pop sound of keypad or not (no pop sound as if it's auto start/stop record)
)
{
	UINT16 u16UsedSylbPitchInfoBufSize = TALKTOSINGAPP_SYLBINFO_SIZE;
	INT16 i, i16SylbNum;
	S_TALK2SING_PCM_HEADER* psPcmHeader = (S_TALK2SING_PCM_HEADER*)psTalkToSingApp->ai16DacBuf;
	S_SPI_READ_SND* psSpiRecSnd = &psTalkToSingApp->sSpiReadSnd;
	UINT16 u16Size;

	memset(psPcmHeader,0,sizeof(S_TALK2SING_PCM_HEADER));
	
	// init information for call back function to read recorded sound
	RcordApp_SetSpiRecSnd(&psTalkToSingApp->sSpiReadSnd);
	psSpiRecSnd->m_i32RecSmplCnt = u32RecSampleCnt;
	psSpiRecSnd->m_u32RecAddr = TALKTOSINGAPP_REC_DATA_ADDR;
	psPcmHeader->u32SkipHeadSmplCnt = 0;

	if (bSkipKeyPop)
	{
		//trim TALKTOSINGAPP_KEY_RELEASE_TIME sec to remove key pop sound
		psSpiRecSnd->m_u32RecAddr += TALKTOSINGAPP_SKIP_SAMPLE_BYTE;
		psPcmHeader->u32SkipHeadSmplCnt += TALKTOSINGAPP_SKIP_SAMPLE_NUM;
		psSpiRecSnd->m_i32RecSmplCnt -= TALKTOSINGAPP_SKIP_SAMPLE_NUM * 2; 	// skip pressing and releasing pop sound samples
	}
	psSpiRecSnd->m_i32RWSmplCnt = 0;

	if (psSpiRecSnd->m_i32RecSmplCnt >= TALKTOSINGAPP_MIN_SAMPLE_NUM)
	{
		u16Size = TalkToSing_StartAnalysis(psTalkToSingApp->au8TalkToSingBuf,
											TALKTOSINGAPP_WORK_BUF_SIZE,
											TALKTOSINGAPP_IN_SAMPLE_RATE,
											psTalkToSingApp->au8SylbInfoBuf,
											&u16UsedSylbPitchInfoBufSize,
											psTalkToSingApp->au16NormalizeBuf,
											TALKTOSINGAPP_NORMALIZE_BUF_SIZE,
											RecordApp_SeekPcm, RecordApp_ReadPcm,
											psSpiRecSnd,
											&psPcmHeader->u32SkipHeadSmplCnt,
											&i16SylbNum,
											TALKTOSINGAPP_8BIT
											);
		if (u16Size > TALKTOSINGAPP_WORK_BUF_SIZE)
			while(1);
		else if (u16Size == 0)
			return FALSE;

		// init PCM header
		psPcmHeader->u32RecSignature = TALKTOSINGAPP_REC_SIGNATURE;
		psPcmHeader->u32RecSmplCnt = u32RecSampleCnt; 
		psPcmHeader->u32SylbNum = i16SylbNum; 
		psPcmHeader->u32EffectiveSmplCnt = psSpiRecSnd->m_i32RecSmplCnt; 
		psPcmHeader->u32UsedSylbPitchInfoBufSize = u16UsedSylbPitchInfoBufSize; 

		// Write syllable / pitch information into storage
		SPIFlash_Erase4K(&g_sSpiFlash,TALKTOSINGAPP_SYLBINFO_ADDR/SPIFLASH_BLOCK_SIZE,1);
		for (i = 0  ; i < u16UsedSylbPitchInfoBufSize; i+=256)
			SPIFlash_WritePage(&g_sSpiFlash,TALKTOSINGAPP_SYLBINFO_ADDR+i,psTalkToSingApp->au8SylbInfoBuf+i);	
		
		// Write normalization information into storage by using
		// psTalkToSingApp->au8SylbInfoBuf as tempory buffer
#if (TALKTOSINGAPP_SYLBINFO_SIZE<256)
#error "TALKTOSINGAPP_SYLBINFO_SIZE must >= 256!"
#endif
		memset(psTalkToSingApp->au8SylbInfoBuf, 0, 256);
		memcpy(psTalkToSingApp->au8SylbInfoBuf, psPcmHeader, sizeof(S_TALK2SING_PCM_HEADER));
		memcpy(psTalkToSingApp->au8SylbInfoBuf+sizeof(S_TALK2SING_PCM_HEADER),psTalkToSingApp->au16NormalizeBuf,i16SylbNum*sizeof(INT16));

		SPIFlash_WritePage(&g_sSpiFlash,TALKTOSINGAPP_REC_INFO_ADDR,(PUINT8)psTalkToSingApp->au8SylbInfoBuf);

		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------------------------------
// Initiate Talk to Sing application.
//----------------------------------------------------------------------------------------------------
void 
TalkToSingApp_Initiate(
	S_TALKTOSINGAPP *psTalkToSingApp,		// Talk to Sing app data structure
	UINT32 u32CallbackIndex					// index of callback function to read audio data
	)
{
	memset(psTalkToSingApp,0,sizeof(S_TALKTOSINGAPP));
	psTalkToSingApp->u8CallbackIndex = u32CallbackIndex;
	// init read record sound information for call back function
	RcordApp_SetSpiRecSnd(&psTalkToSingApp->sSpiReadSnd);
}

//----------------------------------------------------------------------------------------------------
// Start play TalkToSing.
//----------------------------------------------------------------------------------------------------
BOOL 
TalkToSingApp_StartPlay(
	S_TALKTOSINGAPP *psTalkToSingApp,			// Talk to Sing app data structure
	S_TALKTOSINGAPP_PARAM* psTalkToSinfParam,	// parameters to start talk to sing
	UINT8 u8ChannelID							// channel ID of mixer
)
{
	UINT16 u16Size;

	// read information of recorded sound
	if (TalkToSingApp_ReadRecInfo(psTalkToSingApp) == FALSE)
		return FALSE;

	u16Size = TalkToSing_StartPlay(psTalkToSingApp->au8TalkToSingBuf,
									TALKTOSINGAPP_WORK_BUF_SIZE,
									TALKTOSINGAPP_IN_SAMPLE_RATE,
									TALKTOSINGAPP_OUT_SAMPLE_RATE,
									psTalkToSingApp->au8SylbInfoBuf,
									psTalkToSingApp->au16NormalizeBuf,
									psTalkToSingApp->au8RepeatInPhraseBuf,
									RecordApp_SeekPcm, RecordApp_ReadPcm,
									g_asAppCallBack[psTalkToSingApp->u8CallbackIndex].pfnReadDataCallback,
									psTalkToSinfParam->u8Option,
									psTalkToSinfParam->i8Echo,
									psTalkToSinfParam->i16MelodyID,
									psTalkToSinfParam->i8ChNum);
									
	if (u16Size > TALKTOSINGAPP_WORK_BUF_SIZE)
		while(1);
	else if (u16Size == 0)
		return FALSE;
	
	// Start to play talk to sing
#ifdef USE_AUDIOPROCESS_IRQ
	// Due to pre-decode one frame frist, need to decode next frame immediate.
	// Otherwise will meet buffer empty condition!
	// So set the counter value to the trigger threshold value.
	psTalkToSingApp->u16ProcessTrigCounter = TALKTOSINGAPP_OUT_SAMPLE_RATE;
#endif

	// init output PCM ring buffer
	Playback_SetOutputBuf(
		&psTalkToSingApp->sOutBufCtrl,
		TALKTOSINGAPP_OUT_BUF_SIZE,
		psTalkToSingApp->ai16DacBuf,
		TALKTOSINGAPP_OUT_SAMPLES_PER_FRAME,
		TALKTOSINGAPP_OUT_SAMPLE_RATE);

	// Add a channel to mixer
	psTalkToSingApp->u8ChannelID = u8ChannelID;
	Playback_Add(u8ChannelID, &psTalkToSingApp->sOutBufCtrl);
	
	return TRUE;
}

//----------------------------------------------------------------------------------------------------
// Stop play Talk to Sing.
//----------------------------------------------------------------------------------------------------
void 
TalkToSingApp_StopPlay(
	S_TALKTOSINGAPP *psTalkToSingApp		// Talk to Sing app data structure
)
{
	// Clear active flag of output buffer for stoping MidiSynthEx decode.
	BUF_CTRL_SET_INACTIVE(&psTalkToSingApp->sOutBufCtrl);
	// Remove audio codec output buffer from play channel.
	Playback_Remove(psTalkToSingApp->u8ChannelID);
}

//----------------------------------------------------------------------------------------------------
// Proces to generate Talk to Sing PCM samples and put into output buffer.
//----------------------------------------------------------------------------------------------------
BOOL 
TalkToSingApp_ProcessPlay(
	S_TALKTOSINGAPP *psTalkToSingApp		// Talk to Sing app data structure
)
{	
	S_BUF_CTRL *psOutBufCtrl = &psTalkToSingApp->sOutBufCtrl;
	// check output buffer index to check if need fill new samples
	if(Playback_NeedUpdateOutputBuf(psOutBufCtrl))
	{
		// fill TalkToSing PCM samples into ring buffer
		if (TalkToSing_FillPcmBufEx(psTalkToSingApp->au8TalkToSingBuf,&psOutBufCtrl->pi16Buf[psOutBufCtrl->u16BufWriteIdx]) == FALSE)
		{
			// playback finish, stop playback
			TalkToSingApp_StopPlay(psTalkToSingApp);
			return FALSE;
		}
		// update outut buffer index
		Playback_UpdateOutputBuf(psOutBufCtrl);
	}
	return TRUE;
}
