#ifndef _CONFIGAUTOTUNEAPP_H_
#define _CONFIGAUTOTUNEAPP_H_

#include "Platform.h"
#include "ConfigApp.h"

//#include "Audio/PitchFrequence.h"
#include "BNDetection.h"
//#include "Audio/VoiceChange.h"
#include "BufCtrl.h"
#include "PlaybackRecord.h"

#define AUTOTUNEAPP_PROCESS_SAMPLES				8
// -------------------------------------------------------------------------------------------------------------------------------
// DAC Ring Buffer Definitions 
// -------------------------------------------------------------------------------------------------------------------------------
#define AUTOTUNEAPP_OUT_FRAME_NUM	  			64
#define AUTOTUNEAPP_OUT_SAMPLES_PER_FRAME		AUTOTUNEAPP_PROCESS_SAMPLES								
#define AUTOTUNEAPP_OUT_BUF_SIZE 				(AUTOTUNEAPP_OUT_FRAME_NUM*AUTOTUNEAPP_OUT_SAMPLES_PER_FRAME)
 							
#if ( AUTOTUNEAPP_OUT_BUF_SIZE%8 )
	#error "AUTOTUNEAPP_OUT_BUF_SIZE must be multiple of '8'."	
#endif						

// -------------------------------------------------------------------------------------------------------------------------------
// PCM Input Related Definitions 
// -------------------------------------------------------------------------------------------------------------------------------

#define AUTOTUNEAPP_OUT_IN_SAMPLE_RATE_RATIO	(1)		// performance may not be enough as if ratio is greater than 1
#define AUTOTUNEAPP_IN_SAMPLE_RATE 				12000
#define AUTOTUNEAPP_OUT_SAMPLE_RATE 			(AUTOTUNEAPP_IN_SAMPLE_RATE*AUTOTUNEAPP_OUT_IN_SAMPLE_RATE_RATIO)

// -------------------------------------------------------------------------------------------------------------------------------
// Autotune Application Related Definitions 
// -------------------------------------------------------------------------------------------------------------------------------
#define AUTOTUNEAPP_12K_WORK_BUF_SIZE			(0xad4)	// work buffer size for AutoTune
#define AUTOTUNEAPP_PITCH_SHIFT					3		// shift how many semi-tones from current pitch, positive value: pitch up, negitive value: pitch down


#define AUTOTUNEAPP_CTRL_INPUT_FROM_BUF		1
#define AUTOTUNEAPP_CTRL_INPUT_FROM_FUNC	2
#define AUTOTUNEAPP_CTRL_INPUT_SOURCE		(AUTOTUNEAPP_CTRL_INPUT_FROM_BUF|AUTOTUNEAPP_CTRL_INPUT_FROM_FUNC)
typedef BOOL (*PFN_AUTOTUNEAPP_INPUT_FUNC)(UINT16 u16InputCount, INT16 *pi16InputBuf);

typedef struct
{
	UINT32 		au32WorkBuf[(AUTOTUNEAPP_12K_WORK_BUF_SIZE+3)/4];	// work buffer fro AutoTune library
	S_BUF_CTRL 	sOutBufCtrl;								// output buffer controller
	union
	{
		S_BUF_CTRL_CALLBACK sInBufCtrl;						// input buffer controller
		S_BUF_CTRL *psInBufCtrl;							// input buffer controller pointer
		PFN_AUTOTUNEAPP_INPUT_FUNC pfnInputFunc;
	};
	INT16 	i16OutBuf[AUTOTUNEAPP_OUT_BUF_SIZE];		// output buffer to store output PCM samples 
	UINT8	u8ChannelID;								// mixer channel ID
	UINT8	u8CtrlFlag;
	
}S_AUTOTUNEAPP;


//----------------------------------------------------------------------------------------------------
// Initialize auto tune application.
//----------------------------------------------------------------------------------------------------
void
AutoTuneApp_Initiate(
	S_AUTOTUNEAPP *psAutoTuneApp);	// AutoTune app data structure

//----------------------------------------------------------------------------------------------------
// Start to run AutoTune applicaiton.
//----------------------------------------------------------------------------------------------------
UINT32								// output sample rate
AutoTuneApp_StartPlay(
	S_AUTOTUNEAPP *psAutoTuneApp,	// AutoTune app data structure
	UINT8 u8ChannelID,				// mixer channel
	INT8 i8AutoTunePitchShift);		// pitch shift in semitone unit

//----------------------------------------------------------------------------------------------------
// Stop AutoTune application
//----------------------------------------------------------------------------------------------------
void 
AutoTuneApp_StopPlay(
	S_AUTOTUNEAPP *psAutoTuneApp	// AutoTune app data structure
	);

//----------------------------------------------------------------------------------------------------
// Operations in main loop for playing. 
// It gets PCM samples from Auto Tune library and put into output buffer.
//----------------------------------------------------------------------------------------------------
BOOL 								// TRUE: continue playback, FALSE: stop playback
AutoTuneApp_ProcessPlay(
	S_AUTOTUNEAPP *psAutoTuneApp);	// AutoTune app data structure

//----------------------------------------------------------------------------------------------------
// Set AutoTune source is from buffer. For example: The buffer is codec decoder output buffer.
//----------------------------------------------------------------------------------------------------
BOOL
AutoTuneApp_SetInputFromBuf(
	S_AUTOTUNEAPP *psAutoTuneApp,	// AutoTune app data structure
	S_BUF_CTRL *psInPCMBuf);		// Structure pointer of source buffer 

//----------------------------------------------------------------------------------------------------
// Set AutoTune source is from Callback function processing.
//----------------------------------------------------------------------------------------------------
BOOL
AutoTuneApp_SetInputFromFunc(
	S_AUTOTUNEAPP *psAutoTuneApp,	// AutoTune app data structure
	PFN_AUTOTUNEAPP_INPUT_FUNC pfnInputFunc,	// Callback input function 
	UINT32 u32SampleRate);			// audio data sample rate after Callback function processing.

//----------------------------------------------------------------------------------------------------
// Set AutoTune source is from ADC. Default source is from ADC.
//----------------------------------------------------------------------------------------------------
void
AutoTuneApp_SetInputFromADC(
	S_AUTOTUNEAPP *psAutoTuneApp);	// AutoTune app data structure
	
#endif
