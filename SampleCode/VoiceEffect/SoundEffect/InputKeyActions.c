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

#include "APP.h"
#include "Keypad.h"

extern volatile UINT8 g_u8AppCtrl;
extern S_APP g_sApp;

extern void App_StartPlay(void);
extern void App_StopPlay(void);

void ShowLeds(void)
{
	if (g_sApp.bStartAdcApp)		  
 		OUT4(0);
	else
 		OUT4(1);
}
//----------------------------------------------------------------------------------------------------
void StartStopAdc_KeypadHandler(UINT32 u32Param)
{
	if ( g_sApp.bStartAdcApp == FALSE )
	{
		g_sApp.bStartAdcApp = TRUE;
		App_StartPlay();
	}
	else
	{
		g_sApp.bStartAdcApp = FALSE;
		App_StopPlay();
	}
	ShowLeds();
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
	// the maxium value of n is defined by "TRIGGER_KEY_COUNT" in "./ConfigIO.h"
	// the maxium value of m is defined by "MATRIX_KEY_COUNT"  in "./ConfigIO.h"
	switch(u32Param)
	{
		case TG1R:
			StartStopAdc_KeypadHandler(0);		
			break;
		case TG2R:
			if ( g_sApp.bStartAdcApp == TRUE )
				StartStopAdc_KeypadHandler(0);
			if ( ++g_sApp.eSoundEffType > E_SOUNEFF_CHORUS )
				g_sApp.eSoundEffType = E_SOUNEFF_ECHO;
			StartStopAdc_KeypadHandler(0);
			break;
		case TG3F:
			// Increase echo decay to have longer effect
			if (g_sApp.eDecayType != E_DECAY_TYPE_1_0)
				g_sApp.eDecayType += 1;
			switch(g_sApp.eSoundEffType)
			{
			case E_SOUNEFF_ECHO:
				EchoApp_ChangeDecay(&g_sApp.sEchoApp, g_sApp.eDecayType);
				break;
			case E_SOUNEFF_MECHO:
				MEchoApp_ChangeDecay(&g_sApp.sMEchoApp, g_sApp.eDecayType);
				break;
			case E_SOUNEFF_DELAY:
				DelayApp_ChangeDecay(&g_sApp.sDelayApp, g_sApp.eDecayType);
				break;
			case E_SOUNEFF_CHORUS:
				break;
			}
			
			break;
		case TG4F:
			// Decrease echo decay to have shoter effect
			if (g_sApp.eDecayType != E_DECAY_TYPE_0_0)
				g_sApp.eDecayType -= 1;
			
			switch(g_sApp.eSoundEffType)
			{
			case E_SOUNEFF_ECHO:
				EchoApp_ChangeDecay(&g_sApp.sEchoApp, g_sApp.eDecayType);
				break;
			case E_SOUNEFF_MECHO:
				MEchoApp_ChangeDecay(&g_sApp.sMEchoApp, g_sApp.eDecayType);
				break;
			case E_SOUNEFF_DELAY:
				DelayApp_ChangeDecay(&g_sApp.sDelayApp, g_sApp.eDecayType);
				break;
			case E_SOUNEFF_CHORUS:
				break;
			}
			break;
			
		case TG5P:
			App_PowerDown();
			break;
			
	}
}

