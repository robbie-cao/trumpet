#ifndef _DELAYAPP_H_
#define _DELAYAPP_H_

#include "Platform.h"
#include "ConfigApp.h"

#include "BufCtrl.h"
#include "PlaybackRecord.h"	 
#include "VoiceChange.h"

// -------------------------------------------------------------------------------------------------------------------------------
// ADC Related Definitions
// -------------------------------------------------------------------------------------------------------------------------------
#define DELAYAPP_MAX_SAMPLE_RATE		12000

// -------------------------------------------------------------------------------------------------------------------------------
// Delay Effect Related Definitions
// -------------------------------------------------------------------------------------------------------------------------------
#define DELAYAPP_DELAY_EFFET_TIME		0.20 				// Second. Period of time to be apply delay effect
#define DELAYAPP_DELAY_BUFF_COUNT		(UINT32)(((DELAYAPP_MAX_SAMPLE_RATE*DELAYAPP_DELAY_EFFET_TIME)*2))
														// Buffer size at calling VoiceChange_DelayLp6()

#define DELAYAPP_DELAY_START_DECAY		E_DECAY_TYPE_0_5	// means the PCM sample value is decayed to be "0.5*original PCM value" each time it is output.


typedef struct
{
	UINT32 		u32DelayWorkBuff[(DELAYAPP_DELAY_BUFF_COUNT+3)/4];	// Work buffer for delay effect
	S_BUF_CTRL 	*psInBufCtrl;
	S_BUF_CTRL 	sOutBufCtrl;					// Output buffer controller
	
	UINT8		u8PlaybackChannel;					// playback channel ID	
}S_DELAYAPP;


//----------------------------------------------------------------------------------------------------
// Initialize auto tune application.
//----------------------------------------------------------------------------------------------------
void
DelayApp_Initiate(
	S_DELAYAPP *psDelayApp);		// AdcEcho app data structure

//----------------------------------------------------------------------------------------------------
// Start to run AutoTune applicaiton.
//----------------------------------------------------------------------------------------------------
UINT32							// output sample rate
DelayApp_StartPlay(
	S_DELAYAPP *psDelayApp,		// AdcEcho app data structure
	UINT8 u8PlaybackChannel,
	S_BUF_CTRL 	*psInBufCtrl
	);

//----------------------------------------------------------------------------------------------------
// Stop AutoTune application
//----------------------------------------------------------------------------------------------------
void 
DelayApp_StopPlay(
	S_DELAYAPP *psDelayApp	// AdcEcho app data structure
	);

BOOL
DelayApp_ProcessPlay(
	S_DELAYAPP *psDelayApp	// AdcEcho app data structure
	);

__STATIC_INLINE void
DelayApp_ChangeDecay(
	S_DELAYAPP *psDelayApp,		// AdcEcho app data structure
	E_DECAY_TYPE eEchoDecayType	// Decay type
	)
{
	VoiceChange_InitDelay(
		(INT8*)&(psDelayApp->u32DelayWorkBuff[0]), DELAYAPP_DELAY_BUFF_COUNT, eEchoDecayType);
}

#endif
