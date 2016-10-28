/*--------------------------------------------------------------------------------------------------*/
/*                                                                                                  */
/* Copyright(c) 2010 Nuvoton Technology Corp. All rights reserved.                                  */
/*                                                                                                  */
/*--------------------------------------------------------------------------------------------------*/

#include "App.h"
#include "SPIFlash.h"
#include "SPIFlashUtil.h"
#include "PlaybackRecord.h"
#include <string.h>

extern S_SPIFLASH_HANDLER 	g_sSpiFlash;

static S_SPI_READ_SND* g_psSpiReadSnd;

//----------------------------------------------------------------------------------------------------
// Check if need to process data in buffer.
//----------------------------------------------------------------------------------------------------
static
BOOL
RecordApp_NeedPocessInputBuf(
	S_RECORDAPP* psRecordApp)
{
	// check input buffer index to see if need to process the input samples
	return (( psRecordApp->m_i16BufReadIdx > psRecordApp->m_i16BufWriteIdx )|| 
		(( psRecordApp->m_i16BufWriteIdx - psRecordApp->m_i16BufReadIdx) >= RECORDAPP_IN_BYTES_PER_FRAME));
}

//----------------------------------------------------------------------------------------------------
// Update buffer indexes.
//----------------------------------------------------------------------------------------------------
static 
void
RecordApp_UpdateInputBuf(
	S_RECORDAPP* psRecordApp)
{
	// update buffer index if samples in buffer is processed
	psRecordApp->m_i16BufReadIdx += RECORDAPP_IN_BYTES_PER_FRAME ;
	if (psRecordApp->m_i16BufReadIdx >= RECORDAPP_IN_BUF_SIZE)
		psRecordApp->m_i16BufReadIdx = 0;
}

//-----------------------------------------------------------------------------------------------------------
// Convert 13 bits PCM data to 8 bits data.
//-----------------------------------------------------------------------------------------------------------
#if (defined(LP8_FORMAT) || defined(PC8_FORMAT))
UINT8							// 8 bit PCM data
RecordApp_Convert13To8BitPcm(	
	INT16 i16Pcm				// 13 bits PCM data
)
{
#if (defined(PC8_FORMAT))

	return ((UINT8)(i16Pcm >> 5));

#elif (defined(LP8_FORMAT))

	UINT8 u8Pcm;
	BOOL bMinus = FALSE;

	i16Pcm >>= 3;	  // 13 to 10 bits

	if (i16Pcm < 0)
	{
		i16Pcm = -i16Pcm;
		bMinus = TRUE;
	}

	// 10 bits to 8 bits

	if (i16Pcm >= 259)
		u8Pcm = (UINT8)(((i16Pcm - 259) >> 3) + 96);
	else if (i16Pcm >= 65)
		u8Pcm = (UINT8)(((i16Pcm - 65) >> 2) + 48);
	else if (i16Pcm >= 32)
		u8Pcm = (UINT8)(((i16Pcm - 32) >> 1) + 32); 
	else
		u8Pcm = (UINT8)(i16Pcm);
	
	if (bMinus == TRUE && u8Pcm != 0)
		u8Pcm |= 0x80;

	return u8Pcm;
#endif
}

//-----------------------------------------------------------------------------------------------------------
// Convert 8 bits PCM data to 13 bits.
//-----------------------------------------------------------------------------------------------------------
INT16							// 13 bits PCM data
RecordApp_Convert8To13BitPcm(	
	UINT8 u8Pcm					// 8 bits PCM data
)
{
#if (defined(PC8_FORMAT))
	
	return ((INT16)((INT8)u8Pcm) << 5);

#elif (defined(LP8_FORMAT))
	
	BOOL bMinus = FALSE;
	INT16 i16Pcm;

	// 8 to 10 bits

	if (u8Pcm & 0x80)
	{
		u8Pcm &= 0x7f;
		bMinus = TRUE;
	}

	if (u8Pcm >= 96)
		i16Pcm = ((INT16)u8Pcm - 96) * 8 + 259;
	else if (u8Pcm >= 48)
		i16Pcm = ((INT16)u8Pcm - 48) * 4 + 65;
	else if (u8Pcm >= 32)
		i16Pcm = ((INT16)u8Pcm - 32) * 2 + 32;
	else
		i16Pcm = (INT16)u8Pcm;

	if (bMinus == TRUE)
		i16Pcm = -i16Pcm;

	return (i16Pcm << 3);  // 10 to 13 bits
#endif
}

//-----------------------------------------------------------------------------------------------------------
// Convert data in buffer from 8 to 13 bits
//-----------------------------------------------------------------------------------------------------------
void				
RecordApp_Convert8To13BitBuf(
	UINT8* pu8Lp8Buf,			// buffer that store 8 bits PCM data (input)
	UINT16 u16SmplCnt,			// sample number to be converted
	INT16* pi16PcmBuf			// buffer hat store 13 bits PCM data (output)
	)
{
	UINT16 i;

	for (i = 0 ; i < u16SmplCnt ; i++)
		*pi16PcmBuf++ = RecordApp_Convert8To13BitPcm(*pu8Lp8Buf++);
}

#endif

//-----------------------------------------------------------------------------------------------------------
// Copy data to input buffer and update indexes.
//-----------------------------------------------------------------------------------------------------------
static 
UINT8
RecordApp_SetInputData(
	void* pRecordApp,
	INT16 i16SmplNum,
	INT16* pi16InputBuf
)								  
{
	S_RECORDAPP* psRecordApp = (S_RECORDAPP*)pRecordApp;
	INT16 i;

#if (defined(LP8_FORMAT) || defined(PC8_FORMAT))
	// convert 13 bits data to 8 bits data, and store it input input buffer
	UINT8* pu8RecData = (UINT8*)(&(psRecordApp->m_ai8AdcBuf[psRecordApp->m_i16BufWriteIdx]));
	for (i = 0 ; i < i16SmplNum ; i++)
		*pu8RecData++ = RecordApp_Convert13To8BitPcm(*pi16InputBuf++);
	psRecordApp->m_i16BufWriteIdx += i16SmplNum;

#else
	// store PCM samples into inut buffer
	INT16* pi16RecData = (INT16*)(&(psRecordApp->m_ai8AdcBuf[psRecordApp->m_i16BufWriteIdx]));
	for (i = 0 ; i < i16SmplNum ; i++)
		*pi16RecData++ = *pi16InputBuf++;
	psRecordApp->m_i16BufWriteIdx += i16SmplNum << 1;

#endif

	// rollback input index to 0 if input buffer is filled to top index
	if (psRecordApp->m_i16BufWriteIdx >= RECORDAPP_IN_BUF_SIZE)
		psRecordApp->m_i16BufWriteIdx = 0;

	if (psRecordApp->m_i16BufWriteIdx == psRecordApp->m_i16BufReadIdx)
	{
		// buffer is fullfilled, drop a frame to leave space for further recording
		RecordApp_UpdateInputBuf(psRecordApp);
		// return dropped samples number
		return 1;
	}
	return 0;
}

//----------------------------------------------------------------------------------------------------
// Detect sound energy of PCM samples in buffer.
//----------------------------------------------------------------------------------------------------
static
INT8							// return 1 if it's over threshld for SOUND_CNT_THRESHOLD frames
								// return -1 if it's under threshld for SOUND_CNT_THRESHOLD frames
								// return 0 otherwise.
RecordApp_DetectSound(
	S_RECORDAPP *psRecordApp	//	record to SPI-flash app data structure
)
{
	INT16 i;
	INT32 i32PcmSum = 0;
	INT16 i16Pcm;
#if (defined(LP8_FORMAT) || defined(PC8_FORMAT))
	INT8* pi8RecData;
#else
	INT16* pi16RecData;
#endif

	// check buffer index to see if need to detect
	if(RecordApp_NeedPocessInputBuf(psRecordApp))
	{
#if (defined(LP8_FORMAT) || defined(PC8_FORMAT))
		// convert 8 bits data to 13 bits data and summerize abslute value in a frame
		pi8RecData = (INT8*)&(psRecordApp->m_ai8AdcBuf[psRecordApp->m_i16BufReadIdx]);
		for (i = 0 ; i < SAMPLES_IN_BUF ; i++)
		{
			i16Pcm = RecordApp_Convert8To13BitPcm(pi8RecData[i]);
			i32PcmSum += (i16Pcm > 0)? i16Pcm : -i16Pcm;
		}
		// get avergage of summerized data
		i32PcmSum >>= 8;
#else
		// summerize abslute value of data in a frame
		pi16RecData = (INT16*)&(psRecordApp->m_ai8AdcBuf[psRecordApp->m_i16BufReadIdx]);
		for (i = 0 ; i < SAMPLES_IN_BUF ; i++)
		{
			i16Pcm = pi16RecData[i];
			i32PcmSum += (i16Pcm > 0)? i16Pcm : -i16Pcm;
		}
		// get avergage of summerized data
		i32PcmSum >>= 7;
#endif

		// check if it's over or under threshold value
		if (i32PcmSum > SOUND_LOW_THRESHOLD)
		{
			// return 1 if it's over threshld for SOUND_CNT_THRESHOLD frames
			if (psRecordApp->m_i16FrameCnt <= 0)
				psRecordApp->m_i16FrameCnt = 1;
			else if (psRecordApp->m_i16FrameCnt < SOUND_CNT_THRESHOLD)
				psRecordApp->m_i16FrameCnt++;
			else
				return 1;
		}
		else if (i32PcmSum <= SOUND_LOW_THRESHOLD)
		{
			// return -1 if it's under threshld for SOUND_CNT_THRESHOLD frames
			if (psRecordApp->m_i16FrameCnt >= 0)
				psRecordApp->m_i16FrameCnt = -1;
			else if (psRecordApp->m_i16FrameCnt > -SILENCE_CNT_THRESHOLD)
				psRecordApp->m_i16FrameCnt--;
			else 
				return -1;
		}
		// it it's in detecting mode, update buffer index
		if (psRecordApp->m_eRecMode == eMODE_DETECTING)
			RecordApp_UpdateInputBuf(psRecordApp);
	}
	return 0;
}

//----------------------------------------------------------------------------------------------------
// Start record. 
// It may not record PCM samples to SPI-flash immediately. But detects sound energy to start or stop
// recording to SPI-flash.
//----------------------------------------------------------------------------------------------------
void 
RecordApp_StartRec(
	S_RECORDAPP *psRecordApp,	// record to SPI-flash app data structure
	UINT32 u32StartAddr, 		// start address of recorded data in SPI-flash
	BOOL bAutoStartStopRec, 	// TRUE: auto start or stop recording by detecting energy of input sound
	UINT16 u16SampleRate, 		// sample rate of input PCM samples
	UINT16 u16Sec				// length of total rcordd sound in seconds 
)
{
	// if bAutoStartStopRec == TRUE, it auto detect sound energy to start / stop recording PCM samples to SPI-flash
	psRecordApp->m_bAutoStartStopRec = bAutoStartStopRec;

	if (psRecordApp->m_bAutoStartStopRec == FALSE)
	{
		// Start recroding voice to SPI-flash
		psRecordApp->m_eRecMode = eMODE_RECORDING;
		// Initiate 4K page partial writ(pre-erasing first 4k page)
		SPIFlashUtil_4KPagePartialWriteInitiate(u32StartAddr, NULL);
	}
	else
	{
		// start detecting energy of input sound to start/stop recording sound to SPI-flash
		psRecordApp->m_eRecMode = eMODE_DETECTING;
		psRecordApp->m_u32StartAddr = u32StartAddr;
	}

	// reset read/write index of buffer, and recorded samples count
	psRecordApp->m_i16BufReadIdx = psRecordApp->m_i16BufWriteIdx = 0;
	psRecordApp->m_u32RecSampleCnt = 0;
	psRecordApp->m_u32RecSampleCntMax = u16SampleRate * u16Sec;
	
	// clear input buffer
	memset(psRecordApp->m_ai8AdcBuf,0,sizeof(psRecordApp->m_ai8AdcBuf));
	
	// set callback function for ADC handler
	Record_SetInBufCallback(&psRecordApp->m_sInBufCtrlCallback,RecordApp_SetInputData,psRecordApp);
	
	Record_Add((S_BUF_CTRL*)&psRecordApp->m_sInBufCtrlCallback,TALKTOSINGAPP_IN_SAMPLE_RATE);
	return;
}

//----------------------------------------------------------------------------------------------------
// Stop record.
//----------------------------------------------------------------------------------------------------
void 
RecordApp_StopRec(
	S_RECORDAPP *psRecordApp	// record to SPI-flash app data structure
)
{	
	// Wait the last SPI flash writing operation completed
	SPIFlashUtil_4KPagePartialWritWaitComplete();
	// set recording done 
	psRecordApp->m_eRecMode = eMODE_RECORD_DONE;
}

//----------------------------------------------------------------------------------------------------
// Record operation in main loop.
// It may be in recording or in detecting sound energy to start or stop recording.
//----------------------------------------------------------------------------------------------------
BOOL 							// TRUE: continue to detecting or recording, FALSE: finish recording (auto stop or up to record length)
RecordApp_ProcessRec(		
	S_RECORDAPP *psRecordApp	// record to SPI-flash app data structure
)
{
	if (psRecordApp->m_eRecMode == eMODE_RECORDING)
	{
		// in recording to SPI-flash mode
		// check if SPI-flash is busy or not
		if ( SPIFlashUtil_4KPagePartialWriting() == FALSE )
		{
			if (psRecordApp->m_bAutoStartStopRec == TRUE)
			{
				// detect sound energy for auto stop recording 
				INT8 i8Result = RecordApp_DetectSound(psRecordApp);
				if (i8Result == -1)
				{
					// sound energy of frames are lower than threshold, return false to stop recording
					psRecordApp->m_eRecMode = eMODE_RECORD_DONE;
					return FALSE;
				}
			}
			// check if need to store data in buffer into SPI-flash
			if(RecordApp_NeedPocessInputBuf(psRecordApp))
			{
				// check if recorded sample number is over size
				if ((psRecordApp->m_u32RecSampleCnt + SAMPLES_IN_BUF) > psRecordApp->m_u32RecSampleCntMax)
				{
					// it's over size, return false to stop recording
					psRecordApp->m_eRecMode = eMODE_RECORD_OVERSIZE;
					return FALSE;
				}
				// update recorded samples count
				psRecordApp->m_u32RecSampleCnt += SAMPLES_IN_BUF;
				// write a frame into SPI-flash
				SPIFlashUtil_4KPagePartialWriteStart((UINT8*)(&psRecordApp->m_ai8AdcBuf[psRecordApp->m_i16BufReadIdx]), RECORDAPP_IN_BYTES_PER_FRAME);
				// update buffer indexes
				RecordApp_UpdateInputBuf(psRecordApp);
			}
		}
	}
	else if (psRecordApp->m_eRecMode == eMODE_DETECTING)
	{
		// detect energy of input sound for auto start recording sound into SPI-flash
		INT8 i8Result = RecordApp_DetectSound(psRecordApp);
		if (i8Result == 1)
		{
			// Initiate 4K page partial writ(pre-erasing first 4k page)
			SPIFlashUtil_4KPagePartialWriteInitiate(psRecordApp->m_u32StartAddr, NULL);
			// set start index for recording data into SPI-flash
			psRecordApp->m_i16BufReadIdx = psRecordApp->m_i16BufWriteIdx - (PREFIX_REC_FRAME_NUM * RECORDAPP_IN_BYTES_PER_FRAME);
			if (psRecordApp->m_i16BufReadIdx < 0)
				psRecordApp->m_i16BufReadIdx += RECORDAPP_IN_BUF_SIZE;
			psRecordApp->m_i16BufReadIdx = (psRecordApp->m_i16BufReadIdx >> 8) << 8;	// align 256 bytes
			// set to recording mode ( record data into SPI-flash)
			psRecordApp->m_eRecMode = eMODE_RECORDING;
		}
	}
	return TRUE;
}

//----------------------------------------------------------------------------------------------------
// Set read sound data structure.
//----------------------------------------------------------------------------------------------------
void
RcordApp_SetSpiRecSnd(
	S_SPI_READ_SND* psSpiReadSnd
)
{
	// set g_psSpiReadSnd for call back functions
	g_psSpiReadSnd = psSpiReadSnd;
}

//----------------------------------------------------------------------------------------------------
// Seek PCM position of recorded sound.
//----------------------------------------------------------------------------------------------------
UINT32 						// position of PCM sample (not SPI-flash address)
RecordApp_SeekPcm(
	UINT32 u32Position		// position of PCM sample (not SPI-flash address)
)
{
	return (g_psSpiReadSnd->m_i32RWSmplCnt = u32Position);
}

//----------------------------------------------------------------------------------------------------
// Read PCM samples from SPI-flash.
//----------------------------------------------------------------------------------------------------
UINT32 						// PCM sample count had read to buffer
RecordApp_ReadPcm(
	void * pBuf, 			// buffer to store PCM samples
	UINT32 u32SmplNum		// sample number to be read
)
{
	// check if read samples count is over size
	if (g_psSpiReadSnd->m_i32RWSmplCnt >= g_psSpiReadSnd->m_i32RecSmplCnt)
		return 0;

	// update read sampe number if it's over size
	if ((g_psSpiReadSnd->m_i32RWSmplCnt + u32SmplNum) > g_psSpiReadSnd->m_i32RecSmplCnt )
		u32SmplNum = g_psSpiReadSnd->m_i32RecSmplCnt - g_psSpiReadSnd->m_i32RWSmplCnt;

	if (u32SmplNum > 0)
	{
#if (defined(LP8_FORMAT) || defined(PC8_FORMAT))
		// read data from SPI-flash and convert to 13 bits
		SPIFlash_Read(&g_sSpiFlash, g_psSpiReadSnd->m_u32RecAddr+g_psSpiReadSnd->m_i32RWSmplCnt, ((PUINT8)pBuf)+u32SmplNum, u32SmplNum);	
		RecordApp_Convert8To13BitBuf(((PUINT8)pBuf)+u32SmplNum,u32SmplNum,pBuf);
#else
		// read fata from SPI-flash
		SPIFlash_Read(&g_sSpiFlash, g_psSpiReadSnd->m_u32RecAddr+(g_psSpiReadSnd->m_i32RWSmplCnt<<1), (PUINT8)pBuf, u32SmplNum<<1);	
#endif
		// update read samples number
		g_psSpiReadSnd->m_i32RWSmplCnt += u32SmplNum;
	}
	return u32SmplNum;
}

