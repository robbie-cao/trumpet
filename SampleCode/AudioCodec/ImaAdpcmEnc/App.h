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
#include "ImaAdpcmApp/ImaAdpcmApp_Decode.h"
#include "ImaAdpcmApp/ImaAdpcmApp_Encode.h"
#include "PlaybackRecord.h"
#include "BufCtrl.h"
#include "SpiFlashMap.h"
#include "ConfigIO.h"

typedef struct
{
	union
	{
		S_IMAADPCM_APP_DECODE sImaAdpcmAppDecode;
		S_IMAADPCM_APP_ENCODE sImaAdpcmAppEncode;
	};
	union
	{
		UINT32 u32ImaAdpcmDecodeTempBuf[(IMAADPCM_DECODE_TEMP_BUF_SIZE+3)/4];
		UINT32 u32ImaAdpcmEncodeTempBuf[(IMAADPCM_ENCODE_TEMP_BUF_SIZE+3)/4];
	}uTempBuf;
	
} S_APP;

#endif //#ifndef _APP_H_
