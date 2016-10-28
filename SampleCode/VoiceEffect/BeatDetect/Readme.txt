/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
-----------------------------------------------------------f----------------------------------------------
	1. 	Demo how to detect sound energy pulse and show the energy level of pulse with 3 LEDs.


---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	LED:
		1. GPA14: ON - stand by.
		2. GPB4: Detection mode. ON - sound mode. OFF - percussion mode.
		3. GPA13: ON - energy of input sound > BEATDETECTAPP_ENGRYLEVEL1(define in App.h)
		4. GPA15: ON - energy of input sound > BEATDETECTAPP_ENGRYLEVEL2(define in App.h)
		5. GPB5:  ON - energy of input sound > BEATDETECTAPP_ENGRYLEVEL3(define in App.h)

	Key:
		1. GPB0: Start or Stop detect.
		2. GPB1: Change detection mode.
		3. GPB2: power down(keep pressing than 4 second)/wake up.
		

---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
	1. 	Move microphone and sound source far or close to test detected energy of input sound by number of LEDs ON.


---------------------------------------------------------------------------------------------------------
Must Know:
---------------------------------------------------------------------------------------------------------
	1.	The stack size is defined by "Stack_Size" in "startup_ISD9100.s" or in "startup_ISD9300.s". Can change the stack size
		by modifing the content of "Stack_Size".
	2.	The chip ROM size and SRAM size is specified by "ROM_SIZE" and "SRAM_SIZE"in  linker settings
		Open the linker setting and find the --pd="-DSRAM_SIZE=0xNNNN" --pd="-DROM_SIZE=0xMMMMM" in "misc controls" 
	3.	When the error happen:
			"L6388E: ScatterAssert expression (ImageLimit(_SRAM) < (0xNNNNNNNN + 0xMMMM)) failed on line xx" 
		It means the size of variables+stack is over chip SRAM size.
		Please open map file to see the overed SRAM size and reduce variables or stack size to fit chip SRAM size.
	4.	When the error happen:
			"L6220E: Load region _ROM size (NNNNN bytes) exceeds limit (MMM bytes)."
		It means the size of programs is over chip ROM size.
		Please open map file to see the overed programs size and reduce programs size to fit chip ROM size.
