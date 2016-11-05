/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/	 
purpose of this sample code:
		1. Demo how to record sound from microphone to SPI-flash.
		2. Play the recorded sound with Talk to Sing effects.
		3. Play midi together with Talk to Sing sound.


---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	LED:
		1. 	GPA13: ON - playing recoeded sound with Talk to Sing effect. FLASH - recording
		2. 	GPA14, GPA15: play mode. [ON,OFF] - Sing mode (default). [ON,ON] - RAP with pitch shift mode. [OFF,OFF] - RAP mode.
		3.  GPB4, GPB5: Talk to Sing mode. [ON,OFF] - beat match mode (default), [OFF, ON] - one to one mode, [ON,ON] - one to one with echo mode.

	Key:
		1. 	GPB0: click to play / stop recorded sound with selected modes.
		2. 	GPB1: press to start recording, release to stop recording.
		3. 	GPB2: click to change melody (midi). Press over 1.5 seconds to enable / disable background music.
		4. 	GPB3: click to change Talk to Sing mode.  beat match mode -> one to one mode -> one to one with echo mode.
		5.  GPB6: click to change play mode. sing mode -> RAP with pitch shift mode -> RAP with pitch shift mode -> RAP mode 
		6.  GPB7: power down(keep pressing than 4 second)/wake up.


---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
		1. 	Record 6 seconds at most. (changable)
		2.  Background music can be changed to wave instead of midi. But midi is still required for singing reference.
		3.  Midi can be stored in AP-ROM or SPI-flash if they take small memory size.
		4.  Recorded sound is stored in 16 bits format for better sound quality.
		5.  The resource files of background music, and Audio Tool project files are located in "AudioRes" folder under sample code folder.
		

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
