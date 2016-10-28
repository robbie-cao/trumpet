/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/09/02 02:10a $
 * @brief    Demo sample how to use library "Keypad"(Direct Key) API.
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "Platform.h"
#include "Framework.h"
#include "Keypad.h"
#include "ConfigSysClk.h"

#if( !defined(__CHIP_SERIES__) )
#error "Please update SDS version >= 5.0"
#endif

// Application control.
volatile UINT8 g_u8AppCtrl;

/**
  * @brief  main process function.
  */
int32_t main (void)
{	
	SYSCLK_INITIATE();				// Configure CPU clock source and operation clock frequency.
									// The configuration functions are in "ConfigSysClk.h"
	
	CLK_EnableLDO(CLK_LDOSEL_3_3V);	// Enable ISD9100 interl LDO3.3V.
	
	KEYPAD_INITIATE();				// Initiate keypad configurations including direct trigger key and matrix key
									// The keypad configurations are defined in "IO\ConfigIO.h".

    printf("\n+------------------------------------------------------------------------+\n");
    printf("|                Library Keypad Demo Sample(Direct key)                  |\n");
    printf("+------------------------------------------------------------------------+\n");
    printf("<<Please press SWB0,SWB1,SWB2&SWB3(Combo) on demo board>>\n");
	
	while (1)
	{
		if( (g_u8AppCtrl&BIT0) && (g_u8AppCtrl&BIT1) )
		{
			printf ("Both PB2(SWB2) and PB3(SWB3) are pressing(Combo demo).\n");
			g_u8AppCtrl = 0;
		}
		
		TRIGGER_KEY_CHECK();		// Check and execute direct trigger key actions defined in "InputKeyActions.c"
									// Default trigger key handler is "Default_KeyHandler()"
									// The trigger key configurations are defined in "IO\ConfigIO.h".
		
		MATRIX_KEY_CHECK();			// Check and execute matrix key actions defined in "InputKeyActions.c"
									// Default matrix key handler is "Default_KeyHandler()"
									// The matrix key configurations are defined in "IO\ConfigIO.h".
	}
}
