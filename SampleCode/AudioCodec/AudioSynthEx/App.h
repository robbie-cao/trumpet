//---------------------------------------------------------------------------------------------------------*/
//                                                                                                        
// Copyright(c) Nuvoton Technology Corp. All rights reserved.                                             
//                                                                                                        
//---------------------------------------------------------------------------------------------------------*/
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
#define APPCTRL_MODE_EQUATION		BIT3
#define APPCTRL_RECORD				BIT4
#define APPCTRL_RECORD_END			BIT5
#define APPCTRL_PAUSE				BIT6
#define APPCTRL_MUTE				BIT7

// -------------------------------------------------------------------------------------------------------------------------------
// Application Related Definitions 
// -------------------------------------------------------------------------------------------------------------------------------
#include "AudioSynthExApp/AudioSynthExApp.h"
#include "PlaybackRecord.h"
#include "SpiFlashMap.h"
#include "ConfigIO.h"

typedef struct
{
	S_AUDIOSYNTHEX_APP	sAudioSynthExApp;
	union
	{
		// Temp buffer can be shared among channels
		U_AUDIOSYNTHEX_APP_TEMPBUF TempBuf;		
	} uTempBuf;
	
	UINT8	g_u8PlayID;
	UINT16	g_u16TotalAudioNum;
	UINT16	g_u16TotalEquationNum;
} S_APP;


#endif //#ifndef __APP_H__

