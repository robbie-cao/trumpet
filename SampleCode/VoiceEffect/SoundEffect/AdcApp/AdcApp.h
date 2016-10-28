#ifndef _ADCPP_H_
#define _ADCPP_H_

#include "Platform.h"
#include "ConfigApp.h"

#include "BufCtrl.h"
#include "PlaybackRecord.h"

// -------------------------------------------------------------------------------------------------------------------------------
// PCM Input Related Definitions 
// -------------------------------------------------------------------------------------------------------------------------------
#define ADCAPP_IN_FRAME_NUM	  			8
#define ADCAPP_IN_SAMPLES_PER_FRAME		8								
#define ADCAPP_IN_BUF_SIZE 				(ADCAPP_IN_FRAME_NUM * ADCAPP_IN_SAMPLES_PER_FRAME)
 							
#if ( ADCPP_OUT_BUF_SIZE%8 )
	#error "ADCAPP_IN_BUF_SIZE must be multiple of '8'."	
#endif

typedef struct
{
	S_BUF_CTRL 	sInBufCtrl;					// Output buffer controller
	INT16 		i16InBuf[ADCAPP_IN_BUF_SIZE];		// Buffer to store recorded PCM samples 
	
	UINT8		u8PlaybackChannel;					// playback channel ID	
}S_ADCAPP;


//----------------------------------------------------------------------------------------------------
// Initialize auto tune application.
//----------------------------------------------------------------------------------------------------
void
AdcApp_Initiate(
	S_ADCAPP *psAdcApp);		// ADC app data structure

//----------------------------------------------------------------------------------------------------
// Start to run AutoTune applicaiton.
//----------------------------------------------------------------------------------------------------
UINT32							// output sample rate
AdcApp_StartPlay(
	S_ADCAPP *psAdcApp,			// ADC app data structure
	UINT8 u8PlaybackChannel,	// Playback channel
	UINT32 u32AdcSampleRate);	// ADC record sample rate

//----------------------------------------------------------------------------------------------------
// Stop AutoTune application
//----------------------------------------------------------------------------------------------------
void 
AdcApp_StopPlay(
	S_ADCAPP *psAdcApp	// ADC app data structure
	);

BOOL
AdcApp_ProcessPlay(S_ADCAPP *psAdcApp);
	
#endif
