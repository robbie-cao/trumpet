/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/	 

// ---------------------------------------------------------------------------------------------------------
//	Functions:
//		- Direct trigger key and matrix key falling, rising, long pressing handling functions.
//			* Direct trigger key handling functions are referenced in a lookup table "g_asTriggerKeyHandler" (in "ConfigIO.h").
//			* Matrix key handling functions are reference in a lookup talbe "g_asMatrixKeyHandler" (in "ConfigIO.h").
//			* Both lookup tables are used by keypad library to call at key action being identified.
//		- Default trigger key and matrix key handler is "Default_KeyHandler()"
//
//	Reference "Readme.txt" for more information.
// ---------------------------------------------------------------------------------------------------------

#include "App.h"
#include "Keypad.h"

extern volatile UINT8 g_u8AppCtrl;
extern S_APP g_sApp;

// ---------------------------------------------------------------------------------------------------------
// Show LEDs
// ---------------------------------------------------------------------------------------------------------
void ShowLeds(void)
{
	if (g_sApp.bEnableAutoTune)		  
 		OUT5(0);  
	else
 		OUT5(1);  
}
//----------------------------------------------------------------------------------------------------
// Event handler of direct trigger and key matrix. Application can use one handler to process all  
// events, or use different handler for each event.
//----------------------------------------------------------------------------------------------------
void PitchUp_KeypadHandler(UINT32 u32Param)
{
	g_sApp.i8AutoTunePitchShift += 1;
	if (g_sApp.i8AutoTunePitchShift > 12)
		g_sApp.i8AutoTunePitchShift = 12;
	AutoTune_SetPitchShift((UINT8*)g_sApp.sAutoTuneApp.au32WorkBuf,g_sApp.i8AutoTunePitchShift);
}

//----------------------------------------------------------------------------------------------------
void PitchDown_KeypadHandler(UINT32 u32Param)
{
	g_sApp.i8AutoTunePitchShift -= 1;
	if (g_sApp.i8AutoTunePitchShift < -6)
		g_sApp.i8AutoTunePitchShift = -6;
	AutoTune_SetPitchShift((UINT8*)g_sApp.sAutoTuneApp.au32WorkBuf,g_sApp.i8AutoTunePitchShift);
}

//----------------------------------------------------------------------------------------------------
void StartOrStopEnableAutoTune_KeypadHandler(UINT32 u32Param)
{	
	if ( (g_u8AppCtrl&APPCTRL_PLAY) == 0 )
	{
		App_StartPlay();
		OUT4(0);  
	}
	else
	{
		App_StopPlay();
		OUT4(1);  
	}
}

//----------------------------------------------------------------------------------------------------
void EnableAutoTune_KeypadHandler(UINT32 u32Param)
{
	if (g_sApp.bEnableAutoTune)
		g_sApp.bEnableAutoTune = FALSE;
	else
		g_sApp.bEnableAutoTune = TRUE;

	AutoTune_EnableAutoTune(g_sApp.sAutoTuneApp.au32WorkBuf,g_sApp.bEnableAutoTune);
	ShowLeds();
}

//----------------------------------------------------------------------------------------------------
void PowerDown_KeypadHandler(UINT32 u32Param)
{
	App_PowerDown();
}

//----------------------------------------------------------------------------------------------------
void Default_KeyHandler(UINT32 u32Param)
{
	// Within this function, key state can be checked with these keywords:
	//	TGnF: direct trigger key n is in falling state(pressing)
	//	TGnR: direct trigger key n is in rising state(releasing)
	//	TGnP: direct trigger key n is in long pressing state
	//	KEYmF: matrix key m is in falling state(pressing)
	//	KEYmR: matrix key m is in rising state(releasing)
	//	KEYmP: matrix key m is in long pressing state
	// the maxium value of n is defined by "TRIGGER_KEY_COUNT" in "ConfigIO.h"
	// the maxium value of m is defined by "MATRIX_KEY_COUNT"  in "ConfigIO.h"
	switch(u32Param)
	{
		case TG1F:
			PitchUp_KeypadHandler(0);
			break;
		case TG2F:
			PitchDown_KeypadHandler(0);
			break;
		case TG3F:
			StartOrStopEnableAutoTune_KeypadHandler(0);
			break;	
		case TG4F:
			EnableAutoTune_KeypadHandler(0);
			break;	
		case TG5P:
			PowerDown_KeypadHandler(0);
			break;	
	}
}

