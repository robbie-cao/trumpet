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

extern void PowerDown_Enter(void);

extern void PowerDown(void);

extern void PowerDown_Exit(void);

//---------------------------------------------------------------------------------------------------------
//	Initiate application.
//---------------------------------------------------------------------------------------------------------
void 
App_Initiate(void)
{
	// Initiate Pitch-change application.
	PitchChangeApp_Initiate(&g_sApp.sPitchChange);
	
	Playback_Initiate();
	
	// Initatee default pitch index for application.
	g_sApp.i16PitchIdx = APP_DEFAULTPITCH_INDEX; 
	g_sApp.i16PitchIdxOrg = APP_DEFAULTPITCH_INDEX; 

	// Light stand by led for initial ready.
	OUT3(0);
	
	g_u8AppCtrl = APPCTRL_NO_ACTION;

	#if ( ULTRAIO_FW_CURVE_ENABLE )
	NVIC_SetPriority(ULTRAIO_TMR_IRQ, 1);
	#endif	
}

//---------------------------------------------------------------------------------------------------------
//	Start audio playback.
//---------------------------------------------------------------------------------------------------------
void 
App_StartPlay(void)
{
	// Start to process pitch-change application playback part.
	PitchChangeApp_StartPlay(&g_sApp.sPitchChange,0,g_sApp.i16PitchIdx);

	// Start to record PCMs input from ADC
	Record_StartRec();
	
	// Start to playback PCMs from pitch-change application output buffer.
	Playback_StartPlay();

	// Start Ultraio Timer & HW pwm for UltraIO curve output
	ULTRAIO_START();
}

//---------------------------------------------------------------------------------------------------------
//	Stop audio playback.                                                                             
//---------------------------------------------------------------------------------------------------------
void
App_StopPlay(void)
{
	// Stop speaker.
	Playback_StopPlay();
	// Stop MIC.
	Record_StopRec();
	
	// Stop Ultraio Timer & HW pwm.
	ULTRAIO_STOP();

	// Stop pitch change app.
	PitchChangeApp_StopPlay(&g_sApp.sPitchChange);
}

//---------------------------------------------------------------------------------------------------------
//   Produce PCM data for audio playback
//---------------------------------------------------------------------------------------------------------
BOOL 						// TRUE: continue playback, FALSE: finish playback
App_ProcessPlay(void)
{
	UINT8 u8ActiveProcessCount = 0;

	// Process pitch-change application to produce PCMs for audio playback.
	if ( PitchChangeApp_ProcessPlay(&g_sApp.sPitchChange) == TRUE )
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
