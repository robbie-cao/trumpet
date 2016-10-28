/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#ifndef __APP_H__
#define __APP_H__
  	 
#include "ConfigApp.h"
#include "Framework.h"

// -------------------------------------------------------------------------------------------------------------------------------
// g_u8AppCtrl Bit Field Definitions
// -------------------------------------------------------------------------------------------------------------------------------
#define APPCTRL_NO_ACTION			0
#define APPCTRL_PLAY				BIT0
#define APPCTRL_PLAY_STOP			BIT1
#define APPCTRL_RECORD				BIT2

// -------------------------------------------------------------------------------------------------------------------------------
// Application Related Definitions 
// -------------------------------------------------------------------------------------------------------------------------------
#include "AdcApp/AdcApp.h"
#include "EchoApp/EchoApp.h"
#include "MEchoApp/MEchoApp.h"
#include "DelayApp/DelayApp.h"
#include "ChorusApp/ChorusApp.h"
#include "PlaybackRecord.h"
#include "BufCtrl.h"
#include "ConfigIO.h"

typedef enum
{
	E_SOUNEFF_ECHO,
	E_SOUNEFF_MECHO,
	E_SOUNEFF_DELAY,
	E_SOUNEFF_CHORUS
}E_SNDEFF_TYPE;

typedef struct
{
	S_ADCAPP	sAdcApp;				// data structure for Adc app
	union
	{
		S_ECHOAPP	sEchoApp;			// data structure for Echo app
		S_MECHOAPP	sMEchoApp;			// data structure for MEcho app
		S_DELAYAPP	sDelayApp;			// data structure for Delay app
		S_CHORUSAPP	sChorusApp;			// data structure for Chorus app
	};
	
	BOOL bStartAdcApp;
	E_DECAY_TYPE eDecayType;
	E_SNDEFF_TYPE eSoundEffType;
} S_APP;


// ------------------------------------------------------------------------------------------------------------------------------
// Initiate AutoTune memory and parameters.
// ------------------------------------------------------------------------------------------------------------------------------
void 
App_Initiate(void);

// ------------------------------------------------------------------------------------------------------------------------------
// Start AutoTune, start record, start playback.
// ------------------------------------------------------------------------------------------------------------------------------
void 
App_StartPlay(void);

// ------------------------------------------------------------------------------------------------------------------------------
// Stop record, stop playback, stop AutoTune.
// ------------------------------------------------------------------------------------------------------------------------------
void 
App_StopPlay(void);

// ------------------------------------------------------------------------------------------------------------------------------
// Operation in main loop for playing.
// ------------------------------------------------------------------------------------------------------------------------------
BOOL 						// TRUE: continue playback, FALSE: finish playback
App_ProcessPlay(void);

// ------------------------------------------------------------------------------------------------------------------------------
// Operation for power-down request.
// ------------------------------------------------------------------------------------------------------------------------------
void 				
App_PowerDown(void);


#endif //#ifndef __APP_H__

