/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/		 

// ---------------------------------------------------------------------------------------------------------
//	Functions:
//		- System clock configuration.
//		- Keypad configuration.
//		- SPI Flash configuration.
//		- Speaker configuration.
//		- MIC configuration.
//		- Output pin configuration.
//		- UltraIO configuration.
//		- Application Initiation.
//		- Processing loop:
//			* Codec processing(use functions in "AppFunctions.c").
//			* Voice effect processing(use functions in "AppFunctions.c").
//			* Keypad check and execution actions(use functions in "InputKeyActions.c").
//			* Etc.
//	
//	Reference "Readme.txt" for more information.
// ---------------------------------------------------------------------------------------------------------

#include "App.h"
#include "Framework.h"
#include "Keypad.h"
#include "ConfigSysClk.h"
#include "MicSpk.h"

#if( !defined(__CHIP_SERIES__) )
#error "Please update SDS version >= v5.0."
#endif

// Application control.
volatile UINT8 g_u8AppCtrl;
// Application handler.
S_APP g_sApp;

//---------------------------------------------------------------------------------------------------------
// Main Function                                                           
//---------------------------------------------------------------------------------------------------------
INT32 main()
{
										
	SYSCLK_INITIATE();				// Configure CPU clock source and operation clock frequency.
									// The configuration functions are in "ConfigSysClk.h"
	
	CLK_EnableLDO(CLK_LDOSEL_3_3V);	// Enable ISD9100 interl 3.3 LDO.
	
	OUTPUTPIN_INITIATE();			// Initiate output pin configuration.
									// The output pins configurations are defined in "ConfigIO.h".
	
	ULTRAIO_INITIATE();				// Initiate ultraio output configurations.
									// The ultraio output pin configurations are defined in "ConfigUltraIO.h"
	
	KEYPAD_INITIATE();				// Initiate keypad configurations including direct trigger key and matrix key
									// The keypad configurations are defined in "ConfigIO.h".
		
	PDMA_INITIATE();				// Initiate PDMA.
									// After initiation, the PDMA engine clock NVIC are enabled.
									// Use PdmaCtrl_Open() to set PDMA service channel for desired IP.
									// Use PdmaCtrl_Start() to trigger PDMA operation.
									// Reference "PdmaCtrl.h" for PDMA related APIs.
									// PDMA_INITIATE() must be call before SPK_INITIATE() and MIC_INITIATE(), if open MIC or speaker.
	
	SPK_INITIATE();					// Initiate speaker including pop-sound canceling.
									// After initiation, the APU is paused.
									// Use SPK_Resume(0) to start APU operation.
									// Reference "MicSpk.h" for speaker related APIs.

	MIC_INITIATE();					// Initiate MIC.
									// After initiation, the ADC is paused.
									// Use ADC_Resume() to start ADC operation.
									// Reference "MicSpk.h" for MIC related APIs.
	
																	
	App_Initiate();					// Initiate application for audio decode.

	while (1)
	{
		if ( g_u8AppCtrl&APPCTRL_PLAY )
		{
			if ( App_ProcessPlay() == FALSE )
				App_StopPlay();
		}

		TRIGGER_KEY_CHECK();		// Check and execute direct trigger key actions defined in "InputKeyActions.c"
									// Default trigger key handler is "Default_KeyHandler()"
									// The trigger key configurations are defined in "ConfigIO.h".
		
		MATRIX_KEY_CHECK();			// Check and execute matrix key actions defined in "InputKeyActions.c"
									// Default matrix key handler is "Default_KeyHandler()"
									// The matrix key configurations are defined in "ConfigIO.h".
	}
}

