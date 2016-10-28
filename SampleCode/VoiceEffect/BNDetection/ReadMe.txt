/*----------------------------------------------------------------------------------------------------------*/
/*                                                                                                         	*/
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              	*/
/*                                                                                                      	*/
/*----------------------------------------------------------------------------------------------------------*/
purpose of this sample code:
	1. Demo how to detect note of sound inpout from microphone and playback with animal sound effect.


---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	LED:
		1. GPA14: show system standby
		2. GPA15: show beat deteted 
		3. GPB4, GPB5: show note detected
		
	Key:
		1. GPB0: start or stop detect
		2. GPB3: power down(keep pressing than 4 second)/wake up


---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
	1. 	Animal effect sounds were builded in BNDetection_AudioInfoMerge.ROM.
		The BNDetection_AudioInfoMerge.ROM was placed in ".\AudioRes\Output" path.
	2.	Animal effect sounds format is MD4 codec. Now,BNDetectionApp used MD4 to play
		detection reslut.
	2. 	The input sampling rate is defined in ConfigBNDetectionApp.h.
	3. 	Playback can be with/without N-times up-sampling to reduce "metal sound".
	   	which is controlled by "APU_UPSAMPLE" defined in "ConfigApp.h",


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
