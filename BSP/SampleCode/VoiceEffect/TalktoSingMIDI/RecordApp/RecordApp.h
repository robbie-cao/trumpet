#ifndef __CONFIGRECORDAPP_H__
#define __CONFIGRECORDAPP_H__

#include "Platform.h"
#include "BufCtrl.h"
#include "BNDetection.h"

// -------------------------------------------------------------------------------------------------------------------------------
// Declarations or ADC Ring Buffer
// -------------------------------------------------------------------------------------------------------------------------------

#if (__CHIP_SERIES__ == __N571P032_SERIES__)
#define LP8_FORMAT
//#define PC8_FORMAT
#define	RECORDAPP_IN_FRAME_NUM			11		// 11*256 => 0.235 sec
#else
#define	RECORDAPP_IN_FRAME_NUM			22		// 22*256/2 => 0.235 sec
#endif

#define RECORDAPP_IN_BYTES_PER_FRAME	(256)
#define RECORDAPP_IN_BUF_SIZE 			(RECORDAPP_IN_FRAME_NUM * RECORDAPP_IN_BYTES_PER_FRAME)

				
#if ( RECORDAPP_PCM_BUF_SIZE%8 )
	#error "RECORDAPP_PCM_BUF_SIZE must be multiple of (8)."	
#endif						


#if (defined(PC8_FORMAT))

#define SAMPLES_IN_BUF					256
#define SOUND_CNT_THRESHOLD				3
#define SILENCE_CNT_THRESHOLD			12
#define PREFIX_REC_FRAME_NUM			9

#elif (defined(LP8_FORMAT))

#define SAMPLES_IN_BUF					256
#define SOUND_CNT_THRESHOLD				3
#define SILENCE_CNT_THRESHOLD			12
#define PREFIX_REC_FRAME_NUM			9

#else

#define SAMPLES_IN_BUF					128
#define SOUND_CNT_THRESHOLD				6
#define SILENCE_CNT_THRESHOLD			24
#define PREFIX_REC_FRAME_NUM			18

#endif

#define SOUND_LOW_THRESHOLD				56


// modes of record app
typedef enum 
{
	eMODE_DETECTING = 0,		// detecting sound energy for auto start recording
	eMODE_RECORDING,			// recording sound to SPI-flash
	eMODE_RECORD_DONE,			// recording done
	eMODE_RECORD_OVERSIZE		// recording over size
} E_REC_MODE;


// structure to store record app data
typedef struct
{
	BOOL					m_bAutoStartStopRec;				// auto start / stop recording by detecting energy of input sound 
	UINT32					m_u32StartAddr;						// start address to record
	UINT32 					m_u32RecSampleCnt;					// total record samples count
	UINT32 					m_u32RecSampleCntMax;				// max record samples count
	INT8					m_ai8AdcBuf[RECORDAPP_IN_BUF_SIZE];	// buffer to store samples from ADC
	S_BUF_CTRL_CALLBACK 	m_sInBufCtrlCallback;				// buffer controller for input buffer
	INT16					m_i16BufReadIdx;					// buffer index for read samples in buffer for detecting or writing to SPI-flash
	INT16					m_i16BufWriteIdx;					// buffer index for write data to buffer for inputinf samples from ADC
	E_REC_MODE				m_eRecMode;							// mode od record app
	INT16					m_i16FrameCnt;						// frames count for checking if it's over sound / slience frame count threshold
}S_RECORDAPP;


//-----------------------------------------------------------------------------------------------------------
// Convert 13 bits PCM data to 8 bits data.
//-----------------------------------------------------------------------------------------------------------
UINT8							// 8 bit PCM data
RecordApp_Convert13To8BitPcm(	
	INT16 i16Pcm				// 13 bits PCM data
);

//-----------------------------------------------------------------------------------------------------------
// Convert 8 bits PCM data to 13 bits.
//-----------------------------------------------------------------------------------------------------------
INT16							// 13 bits PCM data
RecordApp_Convert8To13BitPcm(	
	UINT8 u8Pcm					// 8 bits PCM data
);

//-----------------------------------------------------------------------------------------------------------
// Convert data in buffer from 8 to 13 bits
//-----------------------------------------------------------------------------------------------------------
void				
RecordApp_Convert8To13BitBuf(
	UINT8* pu8Lp8Buf,			// buffer that store 8 bits PCM data (input)
	UINT16 u16SmplCnt,			// sample number to be converted
	INT16* pi16PcmBuf			// buffer hat store 13 bits PCM data (output)
	);

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
);

//----------------------------------------------------------------------------------------------------
// Stop record.
//----------------------------------------------------------------------------------------------------
void 
RecordApp_StopRec(
	S_RECORDAPP *psRecordApp	// record to SPI-flash app data structure
);

//----------------------------------------------------------------------------------------------------
// Record operation in main loop.
// It may be in recording or in detecting sound energy to start or stop recording.
//----------------------------------------------------------------------------------------------------
BOOL 							// TRUE: continue to detecting or recording, FALSE: finish recording (auto stop or up to record length)
RecordApp_ProcessRec(		
	S_RECORDAPP *psRecordApp	// record to SPI-flash app data structure
);
	
//----------------------------------------------------------------------------------------------------
// Set read sound data structure.
//----------------------------------------------------------------------------------------------------
void
RcordApp_SetSpiRecSnd(
	S_SPI_READ_SND* psSpiReadSnd
);
	
//----------------------------------------------------------------------------------------------------
// Seek PCM position of recorded sound.
//----------------------------------------------------------------------------------------------------
UINT32 						// position of PCM sample (not SPI-flash address)
RecordApp_SeekPcm(
	UINT32 u32Position		// position of PCM sample (not SPI-flash address)
);
	
//----------------------------------------------------------------------------------------------------
// Read PCM samples from SPI-flash.
//----------------------------------------------------------------------------------------------------
UINT32 						// PCM sample count had read to buffer
RecordApp_ReadPcm(
	void * pBuf, 			// buffer to store PCM samples
	UINT32 u32SmplNum		// sample number to be read
);

//----------------------------------------------------------------------------------------------------
// Get recording mode (state)
//----------------------------------------------------------------------------------------------------
__STATIC_INLINE E_REC_MODE RecordApp_GetRecodMode(S_RECORDAPP* psRecordApp)
{ return psRecordApp->m_eRecMode; }

#endif
