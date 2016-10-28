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
#include "BeatDetectApp/BeatDetectApp.h"
#include "PlaybackRecord.h"
#include "BufCtrl.h"
#include "ConfigIO.h"

#define BEATDETECTAPP_ENGRYLEVEL1 		(0)	   		// energy level 1
#define BEATDETECTAPP_ENGRYLEVEL2		(128)		// energy level 2		
#define BEATDETECTAPP_ENGRYLEVEL3		(256)	   	// energy level 3


typedef struct
{
	
	S_BEATDETECT_APP 				sBeatDetectApp;	// Beat-detect application handler.
	E_BNDETECTION_BEAT_DETECT_MODE 	eDetectMode;
	
} S_APP;


//---------------------------------------------------------------------------------------------------------
//	Initiate Beat Detection memory and parameters.
//---------------------------------------------------------------------------------------------------------
void 
App_Initiate(void);

//---------------------------------------------------------------------------------------------------------
//	Start Beat detection. Start to record PCM data from MIC.
//---------------------------------------------------------------------------------------------------------
void 
App_StartRec(void);

//---------------------------------------------------------------------------------------------------------
//	Stop to record PCM data. Stop Beat Detection.
//---------------------------------------------------------------------------------------------------------
void 
App_StopRec(void);

//---------------------------------------------------------------------------------------------------------
//   Record PCM data for beat detection.
//---------------------------------------------------------------------------------------------------------
BOOL 					// TRUE: continue detection, FALSE: finish detection
App_ProcessRec(void);

// ------------------------------------------------------------------------------------------------------------------------------
// Operation for power-down request.
// ------------------------------------------------------------------------------------------------------------------------------
void 				
App_PowerDown(void);


#endif //#ifndef _APP_H_

