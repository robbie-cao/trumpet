#ifndef _CHORUSAPP_H_
#define _CHORUSAPP_H_

#include "Platform.h"
#include "ConfigApp.h"

#include "BufCtrl.h"
#include "PlaybackRecord.h"	 
#include "VoiceChange.h"

// -------------------------------------------------------------------------------------------------------------------------------
// ADC Related Definitions
// -------------------------------------------------------------------------------------------------------------------------------
#define CHORUSAPP_MAX_SAMPLE_RATE		12000				// maxium sample rate to apply echo effect

// -------------------------------------------------------------------------------------------------------------------------------
// Chorus Effect Related Definitions
// -------------------------------------------------------------------------------------------------------------------------------
#define CHORUSAPP_CHORUS_EFFET_TIME		0.020 				// Second. Period of time to be apply chorus effect
#define CHORUSAPP_CHORUS_BUFF_COUNT		(UINT32)(((CHORUSAPP_MAX_SAMPLE_RATE*CHORUSAPP_CHORUS_EFFET_TIME)*2))
															// Buffer size at calling VoiceChange_ChorusPc8()

typedef struct
{
	UINT32 		u32ChorusWorkBuff[(CHORUSAPP_CHORUS_BUFF_COUNT+3)/4];	// Work buffer for chorus effect
	S_BUF_CTRL 	*psInBufCtrl;
	S_BUF_CTRL 	sOutBufCtrl;					// Output buffer controller
	
	UINT8		u8PlaybackChannel;				// playback channel ID	
}S_CHORUSAPP;


//----------------------------------------------------------------------------------------------------
// Initialize echo effect application.
//----------------------------------------------------------------------------------------------------
void
ChorusApp_Initiate(
	S_CHORUSAPP *psChorusApp);		// Chorus app data structure

//----------------------------------------------------------------------------------------------------
// Start to run echo effect applicaiton.
//----------------------------------------------------------------------------------------------------
UINT32							// output sample rate
ChorusApp_StartPlay(
	S_CHORUSAPP *psChorusApp,		// Chorus app data structure
	UINT8 u8PlaybackChannel,
	S_BUF_CTRL 	*psInBufCtrl
	);

//----------------------------------------------------------------------------------------------------
// Stop echo effect application
//----------------------------------------------------------------------------------------------------
void 
ChorusApp_StopPlay(
	S_CHORUSAPP *psChorusApp	// Chorus app data structure
	);

BOOL
ChorusApp_ProcessPlay(
	S_CHORUSAPP *psChorusApp	// Chorus app data structure
	);

#endif
