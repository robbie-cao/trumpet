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
void StartStop_KeypadHandler(UINT32 u32Param)
{
	if(g_u8AppCtrl&APPCTRL_RECORD)
		App_StopRec();
	else
		App_StartRec();
}

// ---------------------------------------------------------------------------------------------------------
void ChangeDetectMode_KeypadHandler(UINT32 u32Param)
{
	if(g_u8AppCtrl&APPCTRL_RECORD)
		App_StopRec();

	// change detect mode(eBNDETECTION_BEAT_SOUND<->eBNDETECTION_BEAT_PERCUSSION).
	if( g_sApp.eDetectMode == eBNDETECTION_BEAT_SOUND )	
	{
		g_sApp.eDetectMode = eBNDETECTION_BEAT_PERCUSSION;
		OUT5(1);
	}
	else
	{
		g_sApp.eDetectMode = eBNDETECTION_BEAT_SOUND;
		OUT5(0);
	}
	App_StartRec();
}

// ---------------------------------------------------------------------------------------------------------
void PowerDown_KeypadHandler(UINT32 u32Param)
{
	App_PowerDown();
}

// ---------------------------------------------------------------------------------------------------------
void Default_KeyHandler(UINT32 u32Param)
{
	switch(u32Param)
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
		case TG1F:
			StartStop_KeypadHandler(0);
			break;
		case TG2F:
			ChangeDetectMode_KeypadHandler(0);
			break;
		case TG3P:
			PowerDown_KeypadHandler(0);
			break;	
	}
}
