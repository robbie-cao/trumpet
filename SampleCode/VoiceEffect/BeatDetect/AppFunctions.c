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

// Pre-defined for Beat-detect application, callback function to response about energy of input sound. It shows LED in this function.
void App_BeatDetectAppEngryCallback(INT16 i16Engry, E_BNDETECTION_RESULT eDetectResult);

//---------------------------------------------------------------------------------------------------------
//	Initiate Beat Detection memory and parameters.
//---------------------------------------------------------------------------------------------------------
void 
App_Initiate(void)
{
	// The default is to detect sound not percussion.
	g_sApp.eDetectMode = eBNDETECTION_BEAT_SOUND;

	// Light stand by led for initial ready.
	OUT3(0);		
	// Light detect mode led(Light:eBNDETECTION_BEAT_SOUND, Dark:eBNDETECTION_BEAT_SOUND)
	OUT5(0);

	g_u8AppCtrl = APPCTRL_NO_ACTION;

	#if ( ULTRAIO_FW_CURVE_ENABLE )
	NVIC_SetPriority(ULTRAIO_TMR_IRQ, 1);
	#endif	
}

//---------------------------------------------------------------------------------------------------------
//	Start Beat detection. Start to record PCM data from MIC.
//---------------------------------------------------------------------------------------------------------
void 
App_StartRec(void)
{
	// Initiate Beat-detect application and config initiate variable.
	BeatDetectApp_Initiate( &g_sApp.sBeatDetectApp);	
	
	// Start to process Beat-detect application and config detect mode.
	BeatDetectApp_StartRec(&g_sApp.sBeatDetectApp, g_sApp.eDetectMode, App_BeatDetectAppEngryCallback);		
	
	// Start to record PCM data into Beat-detect input buffer.
	Record_StartRec();	
}

//---------------------------------------------------------------------------------------------------------
// Stop to record PCM data. Stop Beat Detection.
//---------------------------------------------------------------------------------------------------------
void 
App_StopRec(void)
{
	// Stop mic.
	Record_StopRec();

	// Stop to process Beat-detect application.
	BeatDetectApp_StopRec(&g_sApp.sBeatDetectApp);
}

//---------------------------------------------------------------------------------------------------------
// Process recorded PCM data for beat detection.
//---------------------------------------------------------------------------------------------------------
BOOL 					// TRUE: continue detection, FALSE: finish detection
App_ProcessRec(void)
{
	UINT8 u8ActiveProcessCount = 0;
	
	// Continue record PCM data for Beat-detect.
	if( BeatDetectApp_ProcessRec( &g_sApp.sBeatDetectApp ) == TRUE )
		u8ActiveProcessCount++;
	
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
	App_StopRec();
	
	#if(POWERDOWN_ENABLE)
	PowerDown_Enter();
	PowerDown();
	PowerDown_Exit();
	#endif
}

//---------------------------------------------------------------------------------------------------------
// Function: App_BeatDetectAppEngryCallback
//
// Description:                                                                                            
//   Beat-detect application's engry callback function.(for S_BEATDETECT_VAR_INIT)  
//
// Argument:                                                                                               
//   i16Engry: detect engry value   
//   eDetectResult: Beat detect mode(E_BNDETECTION_RESULT).
//
// Return:
//	                                    
//---------------------------------------------------------------------------------------------------------
void App_BeatDetectAppEngryCallback(INT16 i16Engry, E_BNDETECTION_RESULT eDetectResult)
{
	switch(eDetectResult)
	{
		case eBNDETECTION_BEAT_THD_HI:
			break;	
		case eBNDETECTION_BEAT_THD_NO:
		case eBNDETECTION_BEAT_LO:
			i16Engry = 0;
			break;				
		default:																																					
			return;  
	}
	
	if( i16Engry > BEATDETECTAPP_ENGRYLEVEL1 )
		OUT2(0);
	else
		OUT2(1);

	if( i16Engry > BEATDETECTAPP_ENGRYLEVEL2 )
		OUT4(0);
	else
		OUT4(1);

	if( i16Engry > BEATDETECTAPP_ENGRYLEVEL3 )
		OUT6(0);
	else
		OUT6(1);
}
