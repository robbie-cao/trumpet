/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/	 

// ---------------------------------------------------------------------------------------------------------
//	Functions:
//		- Direct trigger key and matrix key falling, rising, long pressing handling functions.
//			* Direct trigger key handling functions are referenced in a lookup table "g_asTriggerKeyHandler" (in "IO\ConfigIO.h").
//			* Matrix key handling functions are reference in a lookup talbe "g_asMatrixKeyHandler" (in "IO\ConfigIO.h").
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
void EnableCH0_KeypadHandler(UINT32 u32Param)
{
	if ( (g_u8AppCtrl&APPCTRL_PLAY) == 0 )
		return;
	
	if (g_sApp.u16ChannelEnabled & (1))
	{
		HarmonySndApp_ChannelDisable(&g_sApp.sHarmonySndApp,0);
		g_sApp.u16ChannelEnabled &= ~1;
		OUT4(1);
	}
	else
	{
		HarmonySndApp_ChannelEnable(&g_sApp.sHarmonySndApp,0);
		g_sApp.u16ChannelEnabled |= 1;
		OUT4(0);	
	}
}

// ---------------------------------------------------------------------------------------------------------
void EnableCH1_KeypadHandler(UINT32 u32Param)
{
	if ( (g_u8AppCtrl&APPCTRL_PLAY) == 0 )
		return;
	
	if (g_sApp.u16ChannelEnabled & (2))
	{
		HarmonySndApp_ChannelDisable(&g_sApp.sHarmonySndApp,1);
		g_sApp.u16ChannelEnabled &= ~2;
		OUT6(1);
	}
	else
	{
		HarmonySndApp_ChannelEnable(&g_sApp.sHarmonySndApp,1);
		g_sApp.u16ChannelEnabled |= 2;
		OUT6(0);
	}
}

// ---------------------------------------------------------------------------------------------------------
void PlayOrStop_KeypadHandler(UINT32 u32Param)
{	
	if ( (g_u8AppCtrl&APPCTRL_PLAY) == 0 )
	{
		App_StartPlay();
		// Default: all channel are enabled
		HarmonySndApp_ChannelEnable(&g_sApp.sHarmonySndApp,0);
		OUT4(0);
		HarmonySndApp_ChannelEnable(&g_sApp.sHarmonySndApp,1);
		OUT6(0);
	}
	else
	{
		App_StopPlay();
		OUT4(1);
		OUT6(1);
	}
}

// ---------------------------------------------------------------------------------------------------------
void PowerDown_KeypadHandler(UINT32 u32Param)
{
	App_PowerDown();
}

// ---------------------------------------------------------------------------------------------------------
void Default_KeyHandler(UINT32 u32Param)
{
	// Within this function, key state can be checked with these keywords:
	//	TGnF: direct trigger key n is in falling state(pressing)
	//	TGnR: direct trigger key n is in rising state(releasing)
	//	TGnP: direct trigger key n is in long pressing state
	//	KEYmF: matrix key m is in falling state(pressing)
	//	KEYmR: matrix key m is in rising state(releasing)
	//	KEYmP: matrix key m is in long pressing state
	// the maxium value of n is defined by "TRIGGER_KEY_COUNT" in "IO/ConfigIO.h"
	// the maxium value of m is defined by "MATRIX_KEY_COUNT"  in "IO/ConfigIO.h"
	switch(u32Param)
	{
		case TG1F:
			EnableCH0_KeypadHandler(0);
			break;
		case TG2F:
			EnableCH1_KeypadHandler(0);
			break;
		case TG3F:
			PlayOrStop_KeypadHandler(0);
			break;
		case TG4P:
			PowerDown_KeypadHandler(0);
			break;	
	}
}
