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
#include <stdio.h>
#include "ConfigIO.h"
#include "Keypad.h"

extern volatile UINT8 g_u8AppCtrl;

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
	switch(u32Param)
	{
		case KEY1F:   printf ("S01(In:PB0,Out:PB2) is falling state.\n");    break;
	    case KEY1P:   printf ("S01(In:PB0,Out:PB2) is pressing state.\n");   break;
		case KEY1R:   printf ("S01(In:PB0,Out:PB2) is rising state.\n");     break;
		case KEY2F:   printf ("S02(In:PB1,Out:PB2) is falling state.\n");    break;
	    case KEY2P:   printf ("S02(In:PB1,Out:PB2) is pressing state.\n");   break;
		case KEY2R:   printf ("S02(In:PB1,Out:PB2) is rising state.\n");     break;
		case KEY3F:   g_u8AppCtrl |= BIT0;      			                  break;
		case KEY3R:   g_u8AppCtrl &= ~BIT0;          				  	      break;
		case KEY4F:   g_u8AppCtrl |= BIT1;            					      break;
		case KEY4R:   g_u8AppCtrl &= ~BIT1;             	  			      break;
    }
}
