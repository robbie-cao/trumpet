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
	g_sApp.bStartAdcApp = FALSE;
	// Initiate work buffer and parameters of Adc applicatin.
	AdcApp_Initiate(&g_sApp.sAdcApp);
	
	Playback_Initiate();
	
	// Light stand by led for initial ready.
	OUT3(0);

	g_u8AppCtrl = APPCTRL_NO_ACTION;
	g_sApp.eSoundEffType = E_SOUNEFF_ECHO;
	
#if ( ULTRAIO_FW_CURVE_ENABLE )
	NVIC_SetPriority(ULTRAIO_CLK_IRQ, 1);
#endif	
}

// ------------------------------------------------------------------------------------------------------------------------------
// Start AutoTune, start record, start playback.
// ------------------------------------------------------------------------------------------------------------------------------
void 
App_StartPlay(void)
{
	//  Start ADC application and not output to audio channel for playback
	AdcApp_StartPlay(&g_sApp.sAdcApp, PLAYBACK_CHANNEL_NONE, 12000);
	
	switch(g_sApp.eSoundEffType)
	{
	case E_SOUNEFF_ECHO:
		// Initiate work buffer and parameters of Echo applicatin.
		EchoApp_Initiate(&g_sApp.sEchoApp);
	
		// Start Echo application with input from ADC application
		// then output PCMs with echo effect applied to audio channel 0
		EchoApp_StartPlay(&g_sApp.sEchoApp, 0, &g_sApp.sAdcApp.sInBufCtrl);
		g_sApp.eDecayType = ECHOAPP_ECHO_START_DECAY;
		break;
	case E_SOUNEFF_MECHO:
		// Initiate work buffer and parameters of MEcho applicatin.
		MEchoApp_Initiate(&g_sApp.sMEchoApp);
	
		// Start Multiple-Echo application with input from ADC application
		// then output PCMs with multiple-echo effect applied to audio channel 0
		MEchoApp_StartPlay(&g_sApp.sMEchoApp, 0, &g_sApp.sAdcApp.sInBufCtrl);
		g_sApp.eDecayType = MECHOAPP_MECHO_START_DECAY;
		break;
	case E_SOUNEFF_DELAY:
		// Initiate work buffer and parameters of Delay applicatin.
		DelayApp_Initiate(&g_sApp.sDelayApp);
	
		// Start Delay application with input from ADC application
		// then output PCMs with delay effect applied to audio channel 0
		DelayApp_StartPlay(&g_sApp.sDelayApp, 0, &g_sApp.sAdcApp.sInBufCtrl);
		g_sApp.eDecayType = DELAYAPP_DELAY_START_DECAY;
		break;
	case E_SOUNEFF_CHORUS:
		// Initiate work buffer and parameters of Chorus applicatin.
		ChorusApp_Initiate(&g_sApp.sChorusApp);
	
		// Start Chorus application with input from ADC application
		// then output PCMs with chorus effect applied to audio channel 0
		ChorusApp_StartPlay(&g_sApp.sChorusApp, 0, &g_sApp.sAdcApp.sInBufCtrl);
		break;
	}
	
	
	// Start ADC to get PCM from microphone.  
	Record_StartRec();
	
	// Start APU to play PCM from ring buffer.
	Playback_StartPlay();

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

	// Stop ADC application
	AdcApp_StopPlay(&g_sApp.sAdcApp);
	
	
	switch(g_sApp.eSoundEffType)
	{
	case E_SOUNEFF_ECHO:
		// Stop Echo application
		EchoApp_StopPlay(&g_sApp.sEchoApp);
		break;
	case E_SOUNEFF_MECHO:
		// Stop MEcho application
		MEchoApp_StopPlay(&g_sApp.sMEchoApp);
		break;
	case E_SOUNEFF_DELAY:
		// Stop Delay application
		DelayApp_StopPlay(&g_sApp.sDelayApp);
		break;
	case E_SOUNEFF_CHORUS:
		// Stop Choru application
		ChorusApp_StopPlay(&g_sApp.sChorusApp);
		break;
	}
	
	
}

// ------------------------------------------------------------------------------------------------------------------------------
// Operation in main loop for playing.
// ------------------------------------------------------------------------------------------------------------------------------
BOOL 						// TRUE: continue playback, FALSE: finish playback
App_ProcessPlay(void)
{
	UINT8 u8ActiveProcessCount = 0;
	BOOL bFlag;
	
	// Process ADC recording 
//	if ( AdcApp_ProcessPlay(&g_sApp.sAdcApp) == TRUE )
//		u8ActiveProcessCount ++;
	
	
	switch(g_sApp.eSoundEffType)
	{
	case E_SOUNEFF_ECHO:
		// Apply echo effect on input buffer and save PCM with echo effect applied into output buffer
		bFlag = EchoApp_ProcessPlay(&g_sApp.sEchoApp);
		break;
	case E_SOUNEFF_MECHO:
		// Apply multiple-echo effect on input buffer and save PCM with multiple-echo effect applied into output buffer
		bFlag = MEchoApp_ProcessPlay(&g_sApp.sMEchoApp);
		break;
	case E_SOUNEFF_DELAY:
		// Apply delay effect on input buffer and save PCM with delay effect applied into output buffer
		bFlag = DelayApp_ProcessPlay(&g_sApp.sDelayApp);
		break;
	case E_SOUNEFF_CHORUS:
		// Apply chorus effect on input buffer and save PCM with chorus effect applied into output buffer
		bFlag = ChorusApp_ProcessPlay(&g_sApp.sChorusApp);
		break;
	}
	
	if ( bFlag == TRUE )
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
