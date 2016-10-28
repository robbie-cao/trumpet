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
	// the maxium value of m is defined by "MATRIX_KEY_COUNT"  in "IO/ConfigIO.h"
	switch(u32Param)
	{
		case TG1F:   printf ("PB0(SWB0) is falling state.\n");    break;
	    case TG1P:   printf ("PB0(SWB0) is pressing state.\n");   break;
		case TG1R:   printf ("PB0(SWB0) is rising state.\n");     break;
		case TG2F:   printf ("PB1(SWB1) is falling state.\n");    break;
	    case TG2P:   printf ("PB1(SWB1) is pressing state.\n");   break;
		case TG2R:   printf ("PB1(SWB1) is rising state.\n");     break;
		case TG3F:   g_u8AppCtrl |= BIT0;                     	  break;
		case TG3R:   g_u8AppCtrl &= ~BIT0;                        break;
		case TG4F:   g_u8AppCtrl |= BIT1;                         break;
		case TG4R:   g_u8AppCtrl &= ~BIT1;                        break;
	}
}
