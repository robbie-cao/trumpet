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
//		- The above functions use codes in "xxxApp" folder and "Utility" folder to complete necessary operations.
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

extern S_APP g_sApp;

extern volatile UINT8 g_u8AppCtrl;
// SPI flash handler.
extern S_SPIFLASH_HANDLER g_sSpiFlash;

//---------------------------------------------------------------------------------------------------------
// Function: App_Initiate
//
// Description:
//	Initiate application.
//
// Argument:
//
// Return:
//
//---------------------------------------------------------------------------------------------------------
void App_Initiate(void)
{
	g_u8AppCtrl = APPCTRL_NO_ACTION;
	
	// Initiate the audio playback.
	Playback_Initiate();
	
	#if ( ULTRAIO_FW_CURVE_ENABLE )
	NVIC_SetPriority(ULTRAIO_TMR_IRQ, 0);
	#endif	
	
	// Initiate MIDI audio decode lib with callback functions stored in g_asAppCallBack[0].
	MidiExApp_DecodeInitiate(&g_sApp.sMidiExApp, NULL, 0);
	
	// Get total audio number from ROM file header. 
	// The ROM file is placed on SPI Flash address "AUDIOROM_STORAGE_START_ADDR"
	g_sApp.u32TotalAudioNum = AudioRom_GetAudioNum( SPIFlash_ReadDataCallback, AUDIOROM_STORAGE_START_ADDR );

	// Initiate play audio id.
	g_sApp.u32PlayID = 0;
	
	// Light stand by(PB8) led for initial ready().
	OUT3(0);

}

//---------------------------------------------------------------------------------------------------------
// Function: App_StartPlay
//
// Description:                                                                                           
//	Start audio playback.
//
// Return:
// 	FALSE: fail
//	TRUE:  success
//---------------------------------------------------------------------------------------------------------
BOOL App_StartPlay(void)
{
		
	// Start MIDI decode lib to decode MIDI file stored from current midi id
	// And decode the first frame of PCMs.
	if ( MidiExApp_DecodeStartPlayByID(&g_sApp.sMidiExApp, g_sApp.u32PlayID, AUDIOROM_STORAGE_START_ADDR,0) == FALSE)
		return FALSE;
	
	// Start Ultraio Timer & HW pwm for UltraIO curve output
	ULTRAIO_START();
	
	// Start to playback audio with the PCM output buffer. 
	Playback_StartPlay();

	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
// Description:                                                                                            
//	Stop audio playback.                                                                             
//
// Return:
// 	FALSE: fail
//	TRUE:  success
//---------------------------------------------------------------------------------------------------------
BOOL App_StopPlay(void)
{
	// Stop to decode audio data from ROM file for stoping to play audio codec. 
	// Remove audio codec output buffer from play channel.
	MidiExApp_DecodeStopPlay(&g_sApp.sMidiExApp);
	
	// Stop speaker.
	Playback_StopPlay();

	// Stop Ultraio Timer & HW pwm.
	ULTRAIO_STOP();

	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
// Function: App_ProcessPlay
//
// Description:                                                                                            
//   Produce PCM data for audio playback
//
// Return:
//	FALSE: No PCM produced for audio playback
//	TRUE:  Have PCMs produced for audio playback                                      
//---------------------------------------------------------------------------------------------------------
BOOL App_ProcessPlay(void)
{
	UINT8 u8ActiveProcessCount = 0;

	// Continue decode MIDI data to produce PCMs for audio playback.
	if ( MidiExApp_DecodeProcess(&g_sApp.sMidiExApp) == TRUE )
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
