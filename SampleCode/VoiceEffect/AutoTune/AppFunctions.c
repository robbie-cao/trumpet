/*---------------------------------------------------------------------------------------------------------*/
/*																										   */
/* Copyright (c) Nuvoton Technology	Corp. All rights reserved.											   */
/*																										   */
/*---------------------------------------------------------------------------------------------------------*/

// ---------------------------------------------------------------------------------------------------------
//	Functions:
//		- Functions to handle main operations:
//			* Initiate application.
//			* Start audio playback.
//			* Stop  audio playback.
//			* Produce PCM data for audio playback.
//			* Start audio recording.
//			* Stop  audio recording.
//			* Use recorded data to do:
//				a. encoding
//				b. doing voice effect
//				c. write to storage
//				d. etc.
//		- The above functions use codes in "xxxApp" folder and "Utility" folder to complete necessary operations.
//
//	Reference "Readme.txt" for more information.
// ---------------------------------------------------------------------------------------------------------

#include "App.h"
#include "MicSpk.h"

extern S_APP g_sApp;

extern volatile UINT8 g_u8AppCtrl;

extern void ShowLeds(void);

extern void PowerDown_Enter(void);

extern void PowerDown(void);

extern void PowerDown_Exit(void);

// ------------------------------------------------------------------------------------------------------------------------------
// Initiate AutoTune memory and parameters.
// ------------------------------------------------------------------------------------------------------------------------------
void 
App_Initiate(void)
{
	// Initiate work buffer and parameters of auto tune.
	AutoTuneApp_Initiate(&g_sApp.sAutoTuneApp);
	
	// Initiate the audio playback.
	Playback_Initiate();
	
	// enable AutoTune
	g_sApp.bEnableAutoTune = TRUE;
	// Set how many semi-tones to shift as tuning
	g_sApp.i8AutoTunePitchShift = AUTOTUNEAPP_PITCH_SHIFT;
		
	// Light stand by led for initial ready.
	OUT3(0);

	g_u8AppCtrl = APPCTRL_NO_ACTION;
	
#if ( ULTRAIO_FW_CURVE_ENABLE )
	NVIC_SetPriority(ULTRAIO_TMR_IRQ, 1);
#endif	
}

// ------------------------------------------------------------------------------------------------------------------------------
// Start AutoTune, start record, start playback.
// ------------------------------------------------------------------------------------------------------------------------------
void 
App_StartPlay(void)
{
	// Set auto tune application play flag and return sample rate 
	AutoTuneApp_StartPlay(&g_sApp.sAutoTuneApp,0,g_sApp.i8AutoTunePitchShift);
	
	// Start ADC to get PCM from microphone according to returned sample rate of AutoTuneApp_StartRecord  
	Record_StartRec();
	// Start APU to play PCM from ring buffer.
	Playback_StartPlay();

	ShowLeds();
}

// ------------------------------------------------------------------------------------------------------------------------------
// Stop record, stop playback, stop AutoTune.
// ------------------------------------------------------------------------------------------------------------------------------
void 
App_StopPlay(void)
{
	// Stop record.
	Record_StopRec();
	// Stop playback.
	Playback_StopPlay();

	// Stop AutoTune app.
	AutoTuneApp_StopPlay(&g_sApp.sAutoTuneApp);
}

// ------------------------------------------------------------------------------------------------------------------------------
// Operation in main loop for playing.
// ------------------------------------------------------------------------------------------------------------------------------
BOOL 						// TRUE: continue playback, FALSE: finish playback
App_ProcessPlay(void)
{
	UINT8 u8ActiveProcessCount = 0;
	
	// Tune PCM pitch from MIC and put into ring buffer for APU play.
	if ( AutoTuneApp_ProcessPlay(&g_sApp.sAutoTuneApp) == TRUE )
		u8ActiveProcessCount ++;
	
	if ( u8ActiveProcessCount )
		return TRUE;
	return FALSE;
}

//---------------------------------------------------------------------------------------------------------
// Function: App_PowerDown
//
// Description:                                                                                            
//   Process flow of power-down for application. Include,
//   1. App_PowerDownProcess:Pre-process befor entering power-down.
//   2. PowerDown:Power down base process(PowerDown.c).
//   3. App_WakeUpProcess:Post-process after exiting power-down.
//   User could disable or enable this flow from flag(POWERDOWN_ENABLE) in ConfigApp.h.
//---------------------------------------------------------------------------------------------------------
void App_PowerDown(void)
{
	App_StopPlay();
	
	#if(POWERDOWN_ENABLE)
	PowerDown_Enter();
	PowerDown();
	PowerDown_Exit();
	#endif
}
