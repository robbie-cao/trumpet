#ifndef _CONFIGPITCHCHANGEAPP_H_
#define _CONFIGPITCHCHANGEAPP_H_

#include "Platform.h"
#include "BNDetection.h"
#include "VoiceChange.h"
#include "BufCtrl.h"
#include "PlaybackRecord.h"

#define PITCHANGEAPP_OUT_IN_SAMPLE_RATE_RATIO	1.5
#define PITCHCHANGEAPP_IN_SAMPLE_RATE		10000
#define PITCHCHANGEAPP_OUT_SAMPLE_RATE		(PITCHANGEAPP_OUT_IN_SAMPLE_RATE_RATIO*PITCHCHANGEAPP_IN_SAMPLE_RATE)

// -------------------------------------------------------------------------------------------------------------------------------
// DAC Ring Buffer Definitions 
// -------------------------------------------------------------------------------------------------------------------------------
#define PITCHCHANGEAPP_PROCESS_SAMPLES			8	
#define PITCHCHANGEAPP_OUT_FRAME_NUM	  		10
#define PITCHCHANGEAPP_OUT_SAMPLES_PER_FRAME	PITCHCHANGEAPP_PROCESS_SAMPLES								
#define PITCHCHANGEAPP_OUT_BUF_SIZE 			(PITCHCHANGEAPP_OUT_FRAME_NUM*PITCHCHANGEAPP_OUT_SAMPLES_PER_FRAME)
 							
#if ( PITCHCHANGEAPP_OUT_BUF_SIZE%8 )
	#error "PITCHCHANGEAPP_OUT_BUF_SIZE must be multiple of '8'."	
#endif						


#define PITCHCHANGEAPP_BUF_SIZE					0x9dc	// PitchChange work buffer size


#define PITCHCHANGEAPP_CTRL_INPUT_FROM_BUF		1
#define PITCHCHANGEAPP_CTRL_INPUT_FROM_FUNC		2
#define PITCHCHANGEAPP_CTRL_INPUT_SOURCE		(PITCHCHANGEAPP_CTRL_INPUT_FROM_BUF|PITCHCHANGEAPP_CTRL_INPUT_FROM_FUNC)
typedef BOOL (*PFN_PITCHCHANGEAPP_INPUT_FUNC)(UINT16 u16InputCount, INT16 *pi16InputBuf);
typedef struct
{
	UINT32	au32PitchChangeBuf[(PITCHCHANGEAPP_BUF_SIZE+3)/4];		// work buffer for PitchChange
	
	S_BUF_CTRL sOutBufCtrl;									// output buffer controller
	union
	{
		S_BUF_CTRL_CALLBACK sInBufCtrl;						// input buffer controller
		S_BUF_CTRL *psInBufCtrl;							// input buffer controller
		PFN_PITCHCHANGEAPP_INPUT_FUNC pfnInputFunc;
	};
	INT16	i16OutBuf[PITCHCHANGEAPP_OUT_BUF_SIZE];			// output PCM buffer
	UINT8	u8ChannelID;									// mixer channel ID	
	UINT8	u8CtrlFlag;
} S_PITCHCHANGEAPP;


//----------------------------------------------------------------------------------------------------
// Initialize Pitch Change application.
//----------------------------------------------------------------------------------------------------
void 
PitchChangeApp_Initiate(
	S_PITCHCHANGEAPP *psPitchChangeApp		// pitch change app data structure 
);

//----------------------------------------------------------------------------------------------------
// Start to play PitchChange app
//----------------------------------------------------------------------------------------------------
void 
PitchChangeApp_StartPlay(
	S_PITCHCHANGEAPP *psPitchChangeApp,		// pitch change app data structure 
	UINT8 u8ChannelID,						// channel ID of mixer
	INT16 i16PitchShiftNum					// pitch shift number in semitone (positive: pitch up, negtive: pitch down)
);

//----------------------------------------------------------------------------------------------------
// Stop Pitch Change app.
//----------------------------------------------------------------------------------------------------
void 
PitchChangeApp_StopPlay(
	S_PITCHCHANGEAPP *psPitchChangeApp		// pitch change app data structure 
);

//----------------------------------------------------------------------------------------------------
// Process to shift te pitch of PCM samples in input buffer and put into output buffer.
//----------------------------------------------------------------------------------------------------
BOOL 										// TRUE: continue play, FALSE: finish play
PitchChangeApp_ProcessPlay(
	S_PITCHCHANGEAPP *psPitchChangeApp		// pitch change app data structure 
);

//----------------------------------------------------------------------------------------------------
// Set pitch shifting number in semitone
//----------------------------------------------------------------------------------------------------
void 
PitchChangeApp_SetPitchShift(
	S_PITCHCHANGEAPP *psPitchChangeApp,		// pitch change app data structure 
	INT16 i16PitchShiftNum					// pitch shift number in semitone (positive: pitch up, negtive: pitch down)
);

//----------------------------------------------------------------------------------------------------
// Set pitch change source is from buffer. For example: The buffer is codec decoder output buffer.
//----------------------------------------------------------------------------------------------------
BOOL
PitchChangeApp_SetInputFromBuf(
	S_PITCHCHANGEAPP *psPitchChangeApp,		// pitch change app data structure
	S_BUF_CTRL *psInPCMBuf					// Structure pointer of source buffer 
);

//----------------------------------------------------------------------------------------------------
// Set pitch change source is from Callback function processing.
//----------------------------------------------------------------------------------------------------
BOOL
PitchChangeApp_SetInputFromFunc(
	S_PITCHCHANGEAPP *psPitchChangeApp,		// pitch change app data structure
	PFN_PITCHCHANGEAPP_INPUT_FUNC pfnInputFunc,		// Callback input function 
	UINT32 u32SampleRate					// audio data sample rate after Callback function processing.
);

//----------------------------------------------------------------------------------------------------
// Set pitch change source is from ADC. Default source is from ADC.
//----------------------------------------------------------------------------------------------------
void
PitchChangeApp_SetInputFromADC(
	S_PITCHCHANGEAPP *psPitchChangeApp		// pitch change app data structure
);


#endif
