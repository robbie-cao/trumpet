/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#ifndef _APP_H_
#define _APP_H_	  	 

#include "ConfigApp.h"
#include "Framework.h"

// -------------------------------------------------------------------------------------------------------------------------------
// g_u8AppCtrl Bit Field Definitions 
// -------------------------------------------------------------------------------------------------------------------------------
#define APPCTRL_NO_ACTION				0
#define APPCTRL_PLAY					BIT0 
#define APPCTRL_PLAY_STOP				BIT1
#define APPCTRL_RECORD					BIT2

// -------------------------------------------------------------------------------------------------------------------------------
// Application Related Definitions 
// -------------------------------------------------------------------------------------------------------------------------------
#include "BNDetectionApp/BNDetectionApp.h"
#include "PlaybackRecord.h"
#include "BufCtrl.h"
#include "ConfigIO.h"

#define	APP_TIMBRE_NUM					(4)
#define APP_MIN_TIMBRE_ID				BNDET_BIRD_12K
#define APP_MAX_TIMBRE_ID				BNDET_FROG_8K


typedef struct
{
	S_BNDETECTIONAPP 	sBNDetectionApp;		// Beat Note Detection application data structure.
	BOOL 				bFlash;
} S_APP;

//---------------------------------------------------------------------------------------------------------
// Initiate application.
//---------------------------------------------------------------------------------------------------------
void 
App_Initiate(void);

//---------------------------------------------------------------------------------------------------------
// Start beat note detection. Start record PCM data from MIC.
//---------------------------------------------------------------------------------------------------------
void 
App_StartRec(void);

//---------------------------------------------------------------------------------------------------------
//	Stop to record PCM data. Stop beat note detection.                                                                           
//---------------------------------------------------------------------------------------------------------
void
App_StopRec(void);

//---------------------------------------------------------------------------------------------------------
// Process recorded PCM data for beat note detection.
//---------------------------------------------------------------------------------------------------------
BOOL 					// TRUE: continue detection, FALSE: finish detection
App_ProcessRec(void);

//---------------------------------------------------------------------------------------------------------                                                                                          
// Process flow of power-down for application. Include,
// 1. App_PowerDownProcess:Pre-process befor entering power-down.
// 2. PowerDown:Power down base process(PowerDown.c).
// 3. App_WakeUpProcess:Post-process after exiting power-down.
// User could disable or enable this flow from flag(POWERDOWN_ENABLE) in ConfigApp.h.
//---------------------------------------------------------------------------------------------------------
void
App_PowerDown(void);

#endif //#ifndef _APP_H_

