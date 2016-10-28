/*----------------------------------------------------------------------------------------------------------*/
/*                                                                                                         	*/
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              	*/
/*                                                                                                      	*/
/*----------------------------------------------------------------------------------------------------------*/
#ifndef __CONFIGROBOTSOUND_H__
#define __CONFIGROBOTSOUND_H__

#include "Platform.h"
#include "PlaybackRecord.h"
#include "BNDetection.h"
#include "RobotSoundApp/PitchFrequence.h"

// -------------------------------------------------------------------------------------------------------------------------------
// Input and output sample rate definitions   
// -------------------------------------------------------------------------------------------------------------------------------
#define ROBOTSOUNDAPP_OUT_IN_SAMPLE_RATE_RATIO		1.5
#define ROBOTSOUNDAPP_IN_SAMPLE_RATE				(12000)
#define ROBOTSOUNDAPP_OUT_SAMPLE_RATE				(ROBOTSOUNDAPP_OUT_IN_SAMPLE_RATE_RATIO*ROBOTSOUNDAPP_IN_SAMPLE_RATE)	// DAC(SPK) sample rate.	

// -------------------------------------------------------------------------------------------------------------------------------
// DAC ring buffer definitions   
// -------------------------------------------------------------------------------------------------------------------------------
#define ROBOTSOUNDAPP_OUT_FRAME_NUM					(16)		// it can be : 3, 4, 5, ....
#define ROBOTSOUNDAPP_OUT_SAMPLES_PER_FRAME			(8)	
// Ring buffer size(INT16)
#define ROBOTSOUNDAPP_OUT_BUF_SIZE	   				(ROBOTSOUNDAPP_OUT_FRAME_NUM*ROBOTSOUNDAPP_OUT_SAMPLES_PER_FRAME)	   	

// -------------------------------------------------------------------------------------------------------------------------------
// Robot sound lib definitions   
// -------------------------------------------------------------------------------------------------------------------------------																				

// Work buffer size(Automatic cal, this must be provide for robot sound lib.)
#define ROBOTSOUNDAPP_WORK_BUF_SIZE					(0xD8C)

// -------------------------------------------------------------------------------------------------------------------------------
// Structure of application.  
// -------------------------------------------------------------------------------------------------------------------------------
#define ROBOTSOUNDAPP_CTRL_INPUT_FROM_BUF		1
#define ROBOTSOUNDAPP_CTRL_INPUT_FROM_FUNC		2
#define ROBOTSOUNDAPP_CTRL_INPUT_SOURCE		(ROBOTSOUNDAPP_CTRL_INPUT_FROM_BUF|ROBOTSOUNDAPP_CTRL_INPUT_FROM_FUNC)
typedef BOOL (*PFN_ROBOTSOUNDAPP_INPUT_FUNC)(UINT16 u16InputCount, INT16 *pi16InputBuf);
/* Application handler */
typedef struct
{
	
	UINT32					au32WorkBuf[(ROBOTSOUNDAPP_WORK_BUF_SIZE+3)/4];	// robot-sound work buffer for lib.(force to do 4 byte alignment)
	
	union
	{
		S_BUF_CTRL_CALLBACK 	sInBufCtrl;									// input(ADC) buffer control.	
		S_BUF_CTRL *psInBufCtrl;											// input buffer controller
		PFN_ROBOTSOUNDAPP_INPUT_FUNC pfnInputFunc;
	};
	S_BUF_CTRL				sOutBufCtrl;								// output(DAC) buffer control.
	INT16 					i16DACBuf[ROBOTSOUNDAPP_OUT_BUF_SIZE];		// DAC buffer(provide to sOutBufCtrl)
	UINT8					u8ChannelID;								// mixer channel ID
	UINT8					u8CtrlFlag;
}S_ROBOTSOUND_APP;


//---------------------------------------------------------------------------------------------------------
//   initiate frobot sound app                                                                        
//---------------------------------------------------------------------------------------------------------
void 
RobotSoundApp_Initiate( 
	S_ROBOTSOUND_APP *pRobotSoundApp	// robot sound app data structure
);

//---------------------------------------------------------------------------------------------------------
//  Start robot sound app.                                                           
//---------------------------------------------------------------------------------------------------------
void 
RobotSoundApp_StartPlay( 
	S_ROBOTSOUND_APP *pRobotSoundApp,	// robot sound app data structure
	UINT8 u8ChannelID					// channel ID of mixer
);

//---------------------------------------------------------------------------------------------------------
//   Stop robo sound app. 
//---------------------------------------------------------------------------------------------------------
void 
RobotSoundApp_StopPlay( 
	S_ROBOTSOUND_APP *pRobotSoundApp	// robot sound app data structure
);

//---------------------------------------------------------------------------------------------------------
// 	Process PCM samples in input buffer, add robot sound effect and put into output buffer.
//---------------------------------------------------------------------------------------------------------
BOOL 									// TRUE: continue play, FALSE: finish play
RobotSoundApp_ProcessPlay( 
	S_ROBOTSOUND_APP *pRobotSoundApp	// robot sound app data structure
);

//----------------------------------------------------------------------------------------------------
// Set robot sound source is from buffer. For example: The buffer is codec decoder output buffer.
//----------------------------------------------------------------------------------------------------
BOOL
RobotSoundApp_SetInputFromBuf(
	S_ROBOTSOUND_APP *pRobotSoundApp,		// robot sound app data structure
	S_BUF_CTRL *psInPCMBuf					// Structure pointer of source buffer 
);

//----------------------------------------------------------------------------------------------------
// Set robot sound source is from Callback function processing.
//----------------------------------------------------------------------------------------------------
BOOL
RobotSoundApp_SetInputFromFunc(
	S_ROBOTSOUND_APP *pRobotSoundApp,		// robot sound app data structure
	PFN_ROBOTSOUNDAPP_INPUT_FUNC pfnInputFunc,		// Callback input function 
	UINT32 u32SampleRate					// audio data sample rate after Callback function processing.
);

//----------------------------------------------------------------------------------------------------
// Set robot sound source is from ADC. Default source is from ADC.
//----------------------------------------------------------------------------------------------------
void
RobotSoundApp_SetInputFromADC(
	S_ROBOTSOUND_APP *pRobotSoundApp		// robot sound app data structure
);

#endif
