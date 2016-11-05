/*---------------------------------------------------------------------------------------------------------*/
/*																										   */
/* Copyright (c) Nuvoton Technology	Corp. All rights reserved.											   */
/*																										   */
/*---------------------------------------------------------------------------------------------------------*/

// ---------------------------------------------------------------------------------------------------------
//	Functions:
//		- Functions to handle main operations:
//			* Initiate application.
//			* Start audio playback.
//			* Stop  audio playback.
//			* Produce PCM data for audio playback.
//			* Start audio recording.
//			* Stop  audio recording.
//			* Use recorded data to do:
//				a. encoding
//				b. doing voice effect
//				c. write to storage
//				d. etc.
//		- The above functions use codes in "xxxApp" folder and "Framework" folder to complete necessary operations.
//
//	Reference "Readme.txt" for more information.
// ---------------------------------------------------------------------------------------------------------

#include "App.h"
#include "MicSpk.h"
#include "AudioRom.h"
#include "AudioRes/Output/AudioRes_AudioInfo.h"

extern UINT32 SPIFlash_ReadDataCallback(void *pu8Buf, UINT32 u32StartAddr, UINT32 u32Count);

extern UINT8 SPIFlash_Initiate(void);

extern void PowerDown_Enter(void);

extern void PowerDown(void);

extern void PowerDown_Exit(void);

extern S_SPIFLASH_HANDLER g_sSpiFlash; 

extern S_APP g_sApp;

extern volatile UINT8 g_u8AppCtrl;

// ------------------------------------------------------------------------------------------------------------------------------
// Functions of Application

void App_Initiate(void)
{
	S_ROM_HEADER sRomHeader;
	
	// Initiate the control variable "g_u8AppCtrl" which is used for controling audio playback, audio recording and others.
	g_u8AppCtrl = APPCTRL_NO_ACTION;
	
	// Configure the UltraIO curve output funciton's interrupt priority.
	// Lower this priority if other interrupt services should be higher than UltraIO curve output function!
	#if ( ULTRAIO_FW_CURVE_ENABLE )
	NVIC_SetPriority(ULTRAIO_TMR_IRQ, 1);
	#endif
	
	// Initiate the audio playback.
	Playback_Initiate();
	
	// Initiate audio synthesizer with callback functions stored in g_asAppCallBack[0].
	AudioSynthExApp_DecodeInitiate(&g_sApp.sAudioSynthExApp, (UINT8 *)&g_sApp.uTempBuf,AUDIOROM_STORAGE_START_ADDR);
	
	// Get Rom header. The ROM file is placed on SPI Flash address "AUDIOROM_STORAGE_START_ADDR"
	AudioSynthExApp_GetRomHeadInfo(&sRomHeader, AUDIOROM_STORAGE_START_ADDR, 0);
	
	// Total audio numbers in ROM file
	g_sApp.g_u16TotalAudioNum = sRomHeader.u32TotalAudioNum;
	
	// Total equation numbers in ROM file
	g_sApp.g_u16TotalEquationNum = sRomHeader.u32TotalSentenceNum;
	g_sApp.g_u8PlayID = 0;
	
	// Light stand by(PB8) led for initial ready().
	OUT3(0);
		
}

BOOL App_StartPlay(void)
{
	// Play an equation by calling "AudioSynthExApp_StartPlayEquation"
	// Play an audio    by calling "AudioSynthExApp_StartPlayAudio"
	if (g_u8AppCtrl&APPCTRL_MODE_EQUATION )
	{
		if ( AudioSynthExApp_StartPlayEquation((S_AUDIOSYNTHEX_APP*)&g_sApp.sAudioSynthExApp, g_sApp.g_u8PlayID, 0) == FALSE )
			return FALSE;
	}
	else if ( AudioSynthExApp_StartPlayAudio((S_AUDIOSYNTHEX_APP*)&g_sApp.sAudioSynthExApp, g_sApp.g_u8PlayID, 0) == FALSE )
		return FALSE;
	
	// Start Ultraio Timer & HW pwm for UltraIO curve output
	ULTRAIO_START();
	
	// Start to playback audio with the PCM output buffer. Because audio synthesizer will automatically 
	// change sample after pasring audio chunk or equation, second argument of Playback_StartPlay() sets
	// for zero.
	Playback_StartPlay();

	return TRUE;
}

void App_StopPlay(void)
{
	// Stop to decode audio data from ROM file for stoping to play audio codec. 
	// Remove audio codec output buffer from play channel.
	AudioSynthExApp_DecodeStopPlay((S_AUDIOSYNTHEX_APP*)&g_sApp.sAudioSynthExApp);
	
	// Stop speaker.
	Playback_StopPlay();
	
	// Stop Ultraio Timer & HW pwm.
	ULTRAIO_STOP();
}

BOOL App_ProcessPlay(void)
{
	UINT8 u8ActiveProcessCount;
	
	u8ActiveProcessCount = 0;
	
	if ( AudioSynthExApp_DecodeProcess(&g_sApp.sAudioSynthExApp) == TRUE )
		u8ActiveProcessCount ++;
	
	if ( u8ActiveProcessCount )
		return TRUE;
	return FALSE;
}

//---------------------------------------------------------------------------------------------------------
// Function: App_PowerDown
//
// Description:                                                                                            
//   Process flow of power-down for application. Include,
//   1. App_PowerDownProcess:Pre-process befor entering power-down.
//   2. PowerDown:Power down base process(PowerDown.c).
//   3. App_WakeUpProcess:Post-process after exiting power-down.
//   User could disable or enable this flow from flag(POWERDOWN_ENABLE) in ConfigApp.h.
//---------------------------------------------------------------------------------------------------------
void App_PowerDown(void)
{
	App_StopPlay();
	
	#if(POWERDOWN_ENABLE)
	PowerDown_Enter();
	PowerDown();
	PowerDown_Exit();
	#endif
}
