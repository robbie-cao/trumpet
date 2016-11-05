#ifndef _MECHOAPP_H_
#define _MECHOAPP_H_

#include "Platform.h"
#include "ConfigApp.h"

#include "BufCtrl.h"
#include "PlaybackRecord.h"	 
#include "VoiceChange.h"

// -------------------------------------------------------------------------------------------------------------------------------
// ADC Related Definitions
// -------------------------------------------------------------------------------------------------------------------------------
#define MECHOAPP_MAX_SAMPLE_RATE		12000			// Maxium sample rate to apply multiple-echos effect

// -------------------------------------------------------------------------------------------------------------------------------
// Echo Effect Related Definitions
// -------------------------------------------------------------------------------------------------------------------------------
#define MECHOAPP_MECHO_NUM				3				// Define how many echoes to be operated.
														// This value can be 1, 2, 3 ~ dependend on SRAM size 
														// If define 1, it is the same as echo effect
#define MECHOAPP_MECHO_EFFET_TIME		0.27 			// Second. Period of time to be apply multiple-echos effect
#define MECHOAPP_MECHO_BUFF_COUNT		(UINT32)(((MECHOAPP_MAX_SAMPLE_RATE*MECHOAPP_MECHO_EFFET_TIME)*2))
														// Buffer size at calling VoiceChange_MEchoLp6()

#define MECHOAPP_MECHO_START_DECAY	E_DECAY_TYPE_0_125	// means the PCM sample value is decayed to be "0.125*original PCM value" each time it is output.


typedef struct
{
	UINT32 		u32MEchoWorkBuff[(MECHOAPP_MECHO_BUFF_COUNT+3)/4];	// Work buffer for multiple-echos effect
	S_BUF_CTRL 	*psInBufCtrl;
	S_BUF_CTRL 	sOutBufCtrl;					// Output buffer controller
	
	UINT8		u8PlaybackChannel;					// playback channel ID	
}S_MECHOAPP;


//----------------------------------------------------------------------------------------------------
// Initialize multiple-echos effect application.
//----------------------------------------------------------------------------------------------------
void
MEchoApp_Initiate(
	S_MECHOAPP *psMEchoApp);		// MEcho app data structure

//----------------------------------------------------------------------------------------------------
// Start to run multiple-echos effect applicaiton.
//----------------------------------------------------------------------------------------------------
UINT32							// output sample rate
MEchoApp_StartPlay(
	S_MECHOAPP *psMEchoApp,		// MEcho app data structure
	UINT8 u8PlaybackChannel,
	S_BUF_CTRL 	*psInBufCtrl
	);

//----------------------------------------------------------------------------------------------------
// Stop multiple-echos effect application
//----------------------------------------------------------------------------------------------------
void 
MEchoApp_StopPlay(
	S_MECHOAPP *psMEchoApp	// MEcho app data structure
	);

BOOL
MEchoApp_ProcessPlay(
	S_MECHOAPP *psMEchoApp	// MEcho app data structure
	);

__STATIC_INLINE void
MEchoApp_ChangeDecay(
	S_MECHOAPP *psMEchoApp,		// MEcho app data structure
	E_DECAY_TYPE eEchoDecayType	// Decay type
	)
{
	VoiceChange_InitMEcho(
		MECHOAPP_MECHO_NUM, (INT8*)&(psMEchoApp->u32MEchoWorkBuff[0]), MECHOAPP_MECHO_BUFF_COUNT, eEchoDecayType);
}

#endif
