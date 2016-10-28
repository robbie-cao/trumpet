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

extern S_APP g_sApp;
extern volatile UINT8 g_u8AppCtrl;

void App_DetectBeatCallback(E_BNDETECTION_RESULT eDetectResult);
void App_DetectNoteCallback(UINT8 u8Note, UINT8 u8Volumn, UINT32 u32NoteSustainTime);
void App_DetectBeatCallback(E_BNDETECTION_RESULT eDetectResult);

extern void PowerDown_Enter(void);

extern void PowerDown(void);

extern void PowerDown_Exit(void);
//---------------------------------------------------------------------------------------------------------
// Initiate application.
//---------------------------------------------------------------------------------------------------------
void 
App_Initiate(void)
{
	// Initiate BNDetection application.
	BNDetectionApp_Initiate( &g_sApp.sBNDetectionApp);

	g_u8AppCtrl = APPCTRL_NO_ACTION;
	
	#if ( ULTRAIO_FW_CURVE_ENABLE )
	NVIC_SetPriority(ULTRAIO_TMR_IRQ, 1);
	#endif	
	
	// Initiate the audio playback.
	Playback_Initiate();
	
	// Light stand by led for initial ready.
	OUT3(0);	

}

//---------------------------------------------------------------------------------------------------------
// Start beat note detection. Start record PCM data from MIC.
//---------------------------------------------------------------------------------------------------------
void 
App_StartRec(void)
{
	// Start to process Beat-node-detect application.
	BNDetectionApp_StartRec(&g_sApp.sBNDetectionApp,
		App_DetectBeatCallback,
		App_DetectNoteCallback
	);
	
	// Start to record PCM data into Beat-node-detect input buffer.
	Record_StartRec();
}

//---------------------------------------------------------------------------------------------------------
//	Stop to record PCM data. Stop beat note detection.                                                                           
//---------------------------------------------------------------------------------------------------------
void
App_StopRec(void)
{
	// Stop mic.
	Record_StopRec();

	// Stop to process Beat-detect application.
	BNDetectionApp_StopRec(&g_sApp.sBNDetectionApp);
}

//---------------------------------------------------------------------------------------------------------
// Process recorded PCM data for beat note detection.
//---------------------------------------------------------------------------------------------------------
BOOL 					// TRUE: continue detection, FALSE: finish detection
App_ProcessRec(void)
{
	UINT8 u8ActiveProcessCount = 0;

	// Continue record PCM data for Beat-node-detect application.
	if( BNDetectionApp_ProcessRec( &g_sApp.sBNDetectionApp ) == TRUE )
		u8ActiveProcessCount++;
	
	if ( u8ActiveProcessCount )
		return TRUE;

	return FALSE;
}

//---------------------------------------------------------------------------------------------------------
// Function: App_DetectNoteCallback
//
// Description:                                                                                            
//   BNDetection application's node callback function.                                   
//---------------------------------------------------------------------------------------------------------
void App_DetectNoteCallback(UINT8 u8Note, UINT8 u8Volumn, UINT32 u32NoteSustainTime)
{
	if ( g_sApp.bFlash )
	{
		OUT5(1);
		OUT6(0);
	}
	else
	{
		OUT5(0);
		OUT6(1);
	}
	g_sApp.bFlash = !g_sApp.bFlash;
}

//----------------------------------------------------------------------------------------------------
// Callback function to seek position in midi buffer.
//----------------------------------------------------------------------------------------------------
void App_DetectBeatCallback(E_BNDETECTION_RESULT eDetectResult)
{
	if ( eDetectResult != eBNDETECTION_NONE )
	{
		// Light on LED as a beat detected
		OUT4(0);
	}
	else
	{
		// Light off LED as no beat detected
		OUT4(1);
	}
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
	if ((g_u8AppCtrl&APPCTRL_RECORD))
		App_StopRec();
	
	#if(POWERDOWN_ENABLE)
	PowerDown_Enter();
	PowerDown();
	PowerDown_Exit();
	#endif
}


