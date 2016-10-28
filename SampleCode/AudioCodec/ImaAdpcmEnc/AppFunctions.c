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
#include "SPIFlashUtil.h"

extern UINT8 SPIFlash_Initiate(void);

extern void PowerDown_Enter(void);

extern void PowerDown(void);

extern void PowerDown_Exit(void);

extern S_APP g_sApp;

extern volatile UINT8 g_u8AppCtrl;

extern UINT32 g_u32SPIFlashRecAddr;	// Current recoding address in SPIFlash.

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
	// Initiate the control variable "g_u8AppCtrl" which is used for controling audio playback, audio recording and others.
	g_u8AppCtrl = APPCTRL_NO_ACTION;
	
	// Initiate the audio playback.
	Playback_Initiate();
	
	// Configure the UltraIO curve output funciton's interrupt priority.
	// Lower this priority if other interrupt services should be higher than UltraIO curve output function!
	#if ( ULTRAIO_FW_CURVE_ENABLE )
	NVIC_SetPriority(ULTRAIO_TMR_IRQ, 1);
	#endif	
	
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
	// Initiate ImaAdpcm audio decode lib with callback functions stored in g_asAppCallBack[0]
	ImaAdpcmApp_DecodeInitiate(&g_sApp.sImaAdpcmAppDecode, (UINT8 *)&g_sApp.uTempBuf, 0);
	
	// Start ImaAdpcm decode lib to decode ImaAdpcm file stored from address and played from audio channel 0.
	// And decode the first frame of PCMs.
	if ( ImaAdpcmApp_DecodeStartPlayByAddr(&g_sApp.sImaAdpcmAppDecode, AUDIOROM_STORAGE_START_ADDR, 0) == FALSE )
		return FALSE;

	// Light playback led(PB9) for display status.
	OUT4(0);
	
	// Start Ultraio Timer & HW pwm for UltraIO curve output
	ULTRAIO_START();
	
	// Start to playback audio. 
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
	ImaAdpcmApp_DecodeStopPlay( &g_sApp.sImaAdpcmAppDecode );
	
	// Stop speaker.
	Playback_StopPlay();
	
	// Stop Ultraio Timer & HW pwm.
	ULTRAIO_STOP();
	
	OUT4(1);
	
	return TRUE;
}

//---------------------------------------------------------------------------------------------------------
// Function: App_StartRec
//
// Description:                                                                                           
//	Record PCM data from MIC.
//
// Return:
// 	FALSE: fail
//	TRUE:  success
//---------------------------------------------------------------------------------------------------------
BOOL App_StartRec(void)
{
	S_AUDIOCHUNK_HEADER sAudioChunkHeader;
	
	// Initiate ImaAdpcm audio encode lib with temp buffer provided for lib.
	ImaAdpcmApp_EncodeInitiate(&g_sApp.sImaAdpcmAppEncode, (UINT8 *)&g_sApp.uTempBuf);
	
	// Start to encode ImaAdpcm data with sample rate, bit per frame and saved data information into audio chunk header.
	if (ImaAdpcmApp_EncodeStart(&g_sApp.sImaAdpcmAppEncode, &sAudioChunkHeader, ADC_SAMPLE_RATE, E_IMAADPCM_ENCODE_BPS_04) == FALSE)
		return FALSE;
	
	// SPIFlash utility function provide encode data write into SPIFlash.
	// detail info please refer "SPIFlashUtil.h"
	SPIFlahUtil_StartWriteEncodeData(&sAudioChunkHeader, AUDIOROM_STORAGE_START_ADDR, NULL);
	
	// Light record led for display status.
	OUT5(0);
							
	// Start to record PCM data into buffer for produc ImaAdpcm encode data.
	Record_StartRec();
	
	return TRUE;	
}

//---------------------------------------------------------------------------------------------------------
// Description:                                                                                            
//	Stop to record PCM data.                                                                             
//
// Return:
// 	FALSE: fail
//	TRUE:  success
//---------------------------------------------------------------------------------------------------------
void App_StopRec(void)
{
	// Stop mic.
	Record_StopRec();
	
	// Stop to ImaAdpcm encode process.
	ImaAdpcmApp_EncodeEnd(&g_sApp.sImaAdpcmAppEncode);
	
	// Write audio chunk header(encode data length) into SPIFlash.
	SPIFlashUtil_EndWriteEncodeData();

	OUT5(1);			
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
	
	// Continue decode ImaAdpcm data to produce PCMs for audio playback.
	if ( ImaAdpcmApp_DecodeProcess(&g_sApp.sImaAdpcmAppDecode) == TRUE )
		u8ActiveProcessCount ++;

	if ( u8ActiveProcessCount )
		return TRUE;
	
	return FALSE;
}

//---------------------------------------------------------------------------------------------------------
// Function: App_ProcessRec
//
// Description:                                                                                            
//   Record PCM data for providing Beat-detect.
//
// Return:
//	FALSE: No PCM produced for audio playback
//	TRUE:  Have PCMs produced for audio playback                                      
//---------------------------------------------------------------------------------------------------------
BOOL App_ProcessRec(void)
{
	// Write ImaAdpcm encode data into SPIFlash.
	SPIFlashUtil_WriteEncodeData(&g_sApp.sImaAdpcmAppEncode.sEncodeBufCtrl);

	// Check current record address is out size of SPIFlash.
	if (g_u32SPIFlashRecAddr >= g_sSpiFlash.u32FlashSize)
		return FALSE;	

	// Keep encode PCM buffer data to ImaAdpcm lib.
	ImaAdpcmApp_EncodeProcess(&g_sApp.sImaAdpcmAppEncode);
	
	return TRUE;
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
	App_StopRec();
	App_StopPlay();
	
	#if(POWERDOWN_ENABLE)
	PowerDown_Enter();
	PowerDown();
	PowerDown_Exit();
	#endif
}

