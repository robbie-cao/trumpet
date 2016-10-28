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
#include "NuVox63ExApp/NuVox63ExApp.h"
#include "PlaybackRecord.h"
#include "BufCtrl.h"
#include "SpiFlashMap.h"
#include "ConfigIO.h"

typedef struct
{
	S_NUVOX63EX_APP sNuVox63ExApp;
	
	union
	{
		UINT32 u32NuVox63ExTempBuf[(NUVOX63EX_DECODE_TEMP_BUF_SIZE+3)/4];
	}uTempBuf;
	
	// Current audio play id.
	UINT32 u32PlayID;
	// Total audio number(load from rom header.)
	UINT32 u32TotalAudioNum;
	
} S_APP;

#endif //#ifndef _APP_H_

