#ifndef _ECHOAPP_H_
#define _ECHOAPP_H_

#include "Platform.h"
#include "ConfigApp.h"

#include "BufCtrl.h"
#include "PlaybackRecord.h"	 
#include "VoiceChange.h"

// -------------------------------------------------------------------------------------------------------------------------------
// ADC Related Definitions
// -------------------------------------------------------------------------------------------------------------------------------
#define ECHOAPP_MAX_SAMPLE_RATE		12000				// maxium sample rate to apply echo effect

// -------------------------------------------------------------------------------------------------------------------------------
// Echo Effect Related Definitions
// -------------------------------------------------------------------------------------------------------------------------------
#define ECHOAPP_ECHO_EFFET_TIME		0.20 				// Second. Period of time to be apply echo effect
#define ECHOAPP_ECHO_BUFF_COUNT		(UINT32)(((ECHOAPP_MAX_SAMPLE_RATE*ECHOAPP_ECHO_EFFET_TIME)*2))
														// Buffer size at calling VoiceChange_EchoLp6()

#define ECHOAPP_ECHO_START_DECAY	E_DECAY_TYPE_0_5	// means the PCM sample value is decayed to be "0.5*original PCM value" each time it is output.


typedef struct
{
	UINT32 		u32EchoWorkBuff[(ECHOAPP_ECHO_BUFF_COUNT+3)/4];	// Work buffer for echo effect
	S_BUF_CTRL 	*psInBufCtrl;
	S_BUF_CTRL 	sOutBufCtrl;					// Output buffer controller
	
	UINT8		u8PlaybackChannel;					// playback channel ID	
}S_ECHOAPP;


//----------------------------------------------------------------------------------------------------
// Initialize echo effect application.
//----------------------------------------------------------------------------------------------------
void
EchoApp_Initiate(
	S_ECHOAPP *psEchoApp);		// Echo app data structure

//----------------------------------------------------------------------------------------------------
// Start to run echo effect applicaiton.
//----------------------------------------------------------------------------------------------------
UINT32							// output sample rate
EchoApp_StartPlay(
	S_ECHOAPP *psEchoApp,		// Echo app data structure
	UINT8 u8PlaybackChannel,
	S_BUF_CTRL 	*psInBufCtrl
	);

//----------------------------------------------------------------------------------------------------
// Stop echo effect application
//----------------------------------------------------------------------------------------------------
void 
EchoApp_StopPlay(
	S_ECHOAPP *psEchoApp	// Echo app data structure
	);

BOOL
EchoApp_ProcessPlay(
	S_ECHOAPP *psEchoApp	// Echo app data structure
	);

__STATIC_INLINE void
EchoApp_ChangeDecay(
	S_ECHOAPP *psEchoApp,		// Echo app data structure
	E_DECAY_TYPE eEchoDecayType	// Decay type
	)
{
	VoiceChange_InitEcho(
		(INT8*)&(psEchoApp->u32EchoWorkBuff[0]), ECHOAPP_ECHO_BUFF_COUNT, eEchoDecayType);
}

#endif
