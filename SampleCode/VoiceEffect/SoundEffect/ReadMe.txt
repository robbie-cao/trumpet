/*----------------------------------------------------------------------------------------------------------*/
/*                                                                                                         	*/
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              	*/
/*                                                                                                      	*/
/*----------------------------------------------------------------------------------------------------------*/
purpose of this sample code:
		1. Demo how to input soud from microphone and output to speaker directly.


---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	LED:
		1. GPA14:  show system standby
		2. GPA15: show playing 

	Key:
		1. GPB0: start or stop playback
		2. GPB1: change sound effect function: echo->mecho->delay->chorus in circles and will stop playback.
		3. GPB2: Increase decay to have longer echo/mecho/delay/chorus effect
		4. GPB3: Decrease decay to have shorter echo/mecho/delay/chorus effect
		5. GPB6: power down(keep pressing than 4 second)/wake up


---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
	1. 	Playback can be with/without N-times up-sampling to reduce "metal sound"
	   	which is controlled by APU_UPSAMPLE defined in "ConfigApp.h"
	2.	ADC sampling rate is 8000Hz which is defined by ADC_SAMPLE_RATE "ConfigApp.h"
	3.	ADC record can be with/without N-times over-sampling to improve recording quality
		which is controlled by ADC_DOWNSAMPLE defined in "ConfigApp.h"
	4.	SPIFlash performance will decrease as frequency of use. programmer can set PCM_RING_BUF_SIZE
		,ring buffer size, in "ConfigApp.h" to avoid audio sample loss.


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
