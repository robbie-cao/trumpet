#ifndef __CONFIGTALKTOSINGAPP_H__
#define __CONFIGTALKTOSINGAPP_H__

#include "Platform.h"
#include "ConfigApp.h"

#include "VoiceChange.h"
#include "BNDetection.h"
#include "MidiSynthEx.h"
//#include "PitchFrequence.h"
#include "PlaybackRecord.h"
#include "RecordApp\RecordApp.h"

// -------------------------------------------------------------------------------------------------------------------------------
// Declarations or ADC Ring Buffer
// -------------------------------------------------------------------------------------------------------------------------------
#define TALKTOSINGAPP_IN_SAMPLE_RATE			12000
#if ((ADC_DOWNSAMPLE*TALKTOSINGAPP_IN_SAMPLE_RATE) > 96000)
#error 	"ADC_DOWNSAMPLE*TALKTOSING_IN_SAMPLE_RATE can't over 96000!"

#endif

#define TALKTOSINGAPP_IN_FRAME_NUM	  			22
#define TALKTOSINGAPP_IN_SAMPLES_PER_FRAME		(256/2)
#define TALKTOSINGAPP_IN_BUF_SIZE 				(TALKTOSINGAPP_IN_FRAME_NUM*TALKTOSINGAPP_IN_SAMPLES_PER_FRAME)

// -------------------------------------------------------------------------------------------------------------------------------
// Declarations or DAC Ring Buffer
// -------------------------------------------------------------------------------------------------------------------------------
#define TALKTOSINGAPP_OUT_SAMPLE_RATE			(TALKTOSINGAPP_IN_SAMPLE_RATE*2)

#define TALKTOSINGAPP_OUT_FRAME_NUM	  			30
#define TALKTOSINGAPP_OUT_SAMPLES_PER_FRAME		8								
#define TALKTOSINGAPP_OUT_BUF_SIZE 				(TALKTOSINGAPP_OUT_FRAME_NUM*TALKTOSINGAPP_OUT_SAMPLES_PER_FRAME)
 							
#if ( TALKTOSINGAPP_PCM_BUF_SIZE%8 )
	#error "TALKTOSINGAPP_PCM_BUF_SIZE must be multiple of '8'."	
#endif						


// -------------------------------------------------------------------------------------------------------------------------------
// Declarations for talk to sing effect
// -------------------------------------------------------------------------------------------------------------------------------

#define TALKTOSINGAPP_REC_SEC					6
#define TALKTOSINGAPP_REC_MAX_SAMPLE_COUNT		(TALKTOSINGAPP_IN_SAMPLE_RATE*TALKTOSINGAPP_REC_SEC)


#define TALKTOSINGAPP_REC_SYLBINFO_SIZE			4096
#define TALKTOSINGAPP_REC_INFO_HEADER_SIZE		256


#define TALKTOSINGAPP_SYLBINFO_ADDR				(256*1024)
#define TALKTOSINGAPP_REC_INFO_ADDR				(TALKTOSINGAPP_SYLBINFO_ADDR+TALKTOSINGAPP_REC_SYLBINFO_SIZE)
#define TALKTOSINGAPP_REC_DATA_ADDR				(TALKTOSINGAPP_REC_INFO_ADDR+TALKTOSINGAPP_REC_INFO_HEADER_SIZE)

#define TALKTOSINGAPP_REC_SIGNATURE				0xa5a5a5a5

//#define TALKTOSINGAPP_OPTIONS					(TALKTOSING_SYLB_NOTE_BEST_MATCH|TALKTOSING_AUTOTUNE_PITCH)
#define TALKTOSINGAPP_OPTIONS					(TALKTOSING_ONE2ONE_SYLB|TALKTOSING_AUTOTUNE_PITCH)
#define TALKTOSINGAPP_NORMALIZE_BUF_SIZE		(40)   	
#define TALKTOSINGAPP_REPEAT_IN_PHRASE_BUF_SIZE	(0x1c)
#define TALKTOSINGAPP_KEY_RELEASE_TIME			(0.25)	// Second to discard sample to avoid key release sound 
#define TALKTOSINGAPP_SKIP_SAMPLE_NUM			(TALKTOSINGAPP_IN_SAMPLE_RATE*TALKTOSINGAPP_KEY_RELEASE_TIME)
#define TALKTOSINGAPP_MIN_SAMPLE_NUM			(TALKTOSINGAPP_IN_SAMPLE_RATE/2)
#define TALKTOSINGAPP_WORK_BUF_SIZE				(0x918)	
#define TALKTOSINGAPP_SYLBINFO_SIZE				(2400)	


// structure to store header information of recorded PCM for TalkToSing
typedef struct
{
	UINT32 u32RecSignature;					// signature
	UINT32 u32RecSmplCnt;					// total recorded sample number
	UINT32 u32SylbNum;						// syllable number
	UINT32 u32SkipHeadSmplCnt;				// head silence sample number that will be skipped
	UINT32 u32EffectiveSmplCnt;				// effective sample number taht will be played
	UINT32 u32UsedSylbPitchInfoBufSize;		// buffer size used for syllable/pitch information
} S_TALK2SING_PCM_HEADER;

typedef struct
{
	INT8 	i16MelodyID;			// midi ID for TalkToSing melody
	INT8 	i8ChNum;				// channel number of midi for TalkToSing melody
	UINT8	u8Option;				// option for TalkToSing
	INT8	i8Echo;					// echo effect for TalkToSing
} S_TALKTOSINGAPP_PARAM;


typedef struct
{
	UINT8			au8TalkToSingBuf[TALKTOSINGAPP_WORK_BUF_SIZE];					// work bufer for TalkToSing library
	UINT8 			au8SylbInfoBuf[TALKTOSINGAPP_SYLBINFO_SIZE];					// buffer to store syllable/pitch information
	UINT16			au16NormalizeBuf[TALKTOSINGAPP_NORMALIZE_BUF_SIZE];				// volume normalization data
	UINT8			au8RepeatInPhraseBuf[TALKTOSINGAPP_REPEAT_IN_PHRASE_BUF_SIZE];	// work buffer for RepeatInPhrase feature
	
	S_SPI_READ_SND 	sSpiReadSnd;								// structure for call back function to read PCM samples
	INT16 			ai16DacBuf[TALKTOSINGAPP_OUT_BUF_SIZE];		// ring buffer to store TalkToSing PCM samples 
	S_BUF_CTRL 		sOutBufCtrl;								// buffer controller for ai16DacBuf
	UINT8 			u8ChannelID;								// channel ID of mixer channel
	UINT8 			u8CallbackIndex;

	#ifdef USE_AUDIOPROCESS_IRQ
	UINT16 u16ProcessTrigCounter;
#endif
}S_TALKTOSINGAPP;


//-----------------------------------------------------------------------------------------------------------
// Recording done and analysis talk to sing information.
//-----------------------------------------------------------------------------------------------------------
BOOL 										// TRUE: the recorded sound is legal to play Talk to Sing, FALSE: fail to play
TalkToSingApp_RecordingDone(
	S_TALKTOSINGAPP *psTalkToSingApp,		// Talk to Sing app data structure
	UINT32 u32RecSampleCnt, 				// total recorded samples number
	BOOL bSkipKeyPop						// skip pop sound of keypad or not (no pop sound as if it's auto start/stop record)
);

//----------------------------------------------------------------------------------------------------
// Initiate Talk to Sing application.
//----------------------------------------------------------------------------------------------------
void 
TalkToSingApp_Initiate(
	S_TALKTOSINGAPP *psTalkToSingApp,		// Talk to Sing app data structure
	UINT32 u32CallbackIndex					// index of callback function to read audio data
	);

//----------------------------------------------------------------------------------------------------
// Start play TalkToSing.
//----------------------------------------------------------------------------------------------------
BOOL 
TalkToSingApp_StartPlay(
	S_TALKTOSINGAPP *psTalkToSingApp,			// Talk to Sing app data structure
	S_TALKTOSINGAPP_PARAM* psTalkToSinfParam,	// parameters to start talk to sing
	UINT8 u8ChannelID							// channel ID of mixer
);

//----------------------------------------------------------------------------------------------------
// Stop play Talk to Sing.
//----------------------------------------------------------------------------------------------------
void 
TalkToSingApp_StopPlay(
	S_TALKTOSINGAPP *psTalkToSingApp		// Talk to Sing app data structure
);

//----------------------------------------------------------------------------------------------------
// Proces to generate Talk to Sing PCM samples and put into output buffer.
//----------------------------------------------------------------------------------------------------
BOOL 
TalkToSingApp_ProcessPlay(
	S_TALKTOSINGAPP *psTalkToSingApp		// Talk to Sing app data structure
);



#endif
