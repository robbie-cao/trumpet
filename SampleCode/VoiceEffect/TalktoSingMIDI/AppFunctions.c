#include "App.h"
#include "AudioMixer.h"
#include "AudioRes/Output/AudioRes_AudioInfo.h"

extern UINT8 SPIFlash_Initiate(void);

extern S_APP g_sApp;
extern volatile UINT8 g_u8AppCtrl;
extern S_BUF_CTRL *g_sMixerInBufCtrlList[];

extern void PowerDown_Enter(void);

extern void PowerDown(void);

extern void PowerDown_Exit(void);

#define INDCATOR_PORT		GPIOB
#define INDCATOR_PIN		BIT7

const INT8 g_i8ChnlNum[APP_END_MELODY_ID - APP_START_MELODY_ID + 1] = {2,0,0};

// ------------------------------------------------------------------------------------------------------------------------------
// Initiate app.
// ------------------------------------------------------------------------------------------------------------------------------
void 
App_Initiate(void)
{
	// Initiate the audio playback.
	Playback_Initiate();

	// Initiate TalkToSing app.
//	TalkToSingApp_Initiate(&g_sApp.sTalkToSingApp,0);
	
	// Initiate app data.
	g_sApp.bEnableMusic = TRUE;
	g_sApp.sTalkToSingParam.i16MelodyID = APP_START_MELODY_ID;
	g_sApp.sTalkToSingParam.u8Option = TALKTOSINGAPP_OPTIONS;
	g_sApp.sTalkToSingParam.i8ChNum = g_i8ChnlNum[g_sApp.sTalkToSingParam.i16MelodyID];
	g_sApp.u8KeyPressingCnt = 0;
	
	// Initiate Midi app.
//	MidiExApp_DecodeInitiate(&g_sApp.sMidiExApp, NULL, 0);
	
	ShowLeds();
	
	// Add codes before here	
	
	g_u8AppCtrl = APPCTRL_NO_ACTION;

#if ( ULTRAIO_FW_CURVE_ENABLE )
	NVIC_SetPriority(ULTRAIO_CLK_IRQ, 1);
#endif	

#ifdef USE_AUDIOMIXER
	AudioMixer_Initiate(&g_sApp.sMixerCtrl, g_sMixerInBufCtrlList);
#endif
	
}

// ------------------------------------------------------------------------------------------------------------------------------
// Start playback TalkToSing sound and midi (background music)
// ------------------------------------------------------------------------------------------------------------------------------
BOOL 					// TRUE: success to start, FALSE: fail to start
App_StartPlay(void)
{
	// Initiate TalkToSing app.
	TalkToSingApp_Initiate(&g_sApp.sTalkToSingApp,0);
	// Start to play TalkToSing sound.
	if ( (TalkToSingApp_StartPlay(&g_sApp.sTalkToSingApp, &g_sApp.sTalkToSingParam, 0)) == 0 )
		return FALSE;
	
	// Initiate Midi app.
	MidiExApp_DecodeInitiate(&g_sApp.sMidiExApp, NULL, 0);
	// Start to play melody
	if ( (MidiExApp_DecodeStartPlayByID(&g_sApp.sMidiExApp, g_sApp.sTalkToSingParam.i16MelodyID, AUDIOROM_STORAGE_START_ADDR,1)) == FALSE)
		return FALSE;

	OUT2(0);
	
	// Add codes before here	
	
	// start apu
	Playback_StartPlay();
	
	return TRUE;
}

// ------------------------------------------------------------------------------------------------------------------------------
// Stop playback TalkToSing and Midi.
// ------------------------------------------------------------------------------------------------------------------------------
void 
App_StopPlay(void)
{
	// Stop apu
	Playback_StopPlay();

	// Stop play TalkToSing
	TalkToSingApp_StopPlay(&g_sApp.sTalkToSingApp);
	// stop play Midi
	MidiExApp_DecodeStopPlay(&g_sApp.sMidiExApp);

	OUT2(1);
}

// ------------------------------------------------------------------------------------------------------------------------------
// Start record PCM samples to SPI-flash
// ------------------------------------------------------------------------------------------------------------------------------
void 
App_StartRec(void)
{
	g_sApp.u16LEDFlashing = 0;

	// start record app
	RecordApp_StartRec(&g_sApp.sRecordApp,TALKTOSINGAPP_REC_DATA_ADDR,FALSE,TALKTOSINGAPP_IN_SAMPLE_RATE,TALKTOSINGAPP_REC_SEC);

	// Start ADC
	Record_StartRec();

	g_u8AppCtrl |= APPCTRL_WRITE;		// Mark writing data to storage
}

// ------------------------------------------------------------------------------------------------------------------------------
// Stop record PCM samples to SPI-flash.
// ------------------------------------------------------------------------------------------------------------------------------
void 
App_StopRec(void)
{
	// Stop ADC
	Record_StopRec();				// Stop voice recording

	// Stop record app
	RecordApp_StopRec(&g_sApp.sRecordApp);
	
	g_u8AppCtrl &= ~APPCTRL_WRITE;	// Mark no writing data to storage
}

// ------------------------------------------------------------------------------------------------------------------------------
// Process to play TalkToSing sound and midi. Put samples into output buffers for mixing.
// ------------------------------------------------------------------------------------------------------------------------------
BOOL 					// TRUE: continue playback, FALSE: finish playback
App_ProcessPlay(void)
{
	UINT8 u8ActiveProcessCount = 0;
	
	// Add your process after here

	// Get TalkToSing PCM samples and put into output PCM buffer
	if (TalkToSingApp_ProcessPlay(&g_sApp.sTalkToSingApp) == TRUE)
		u8ActiveProcessCount ++;

	// Get Midi PCM samples and put into output PCM buffer
	if (MidiExApp_DecodeProcess(&g_sApp.sMidiExApp) == TRUE )
		u8ActiveProcessCount ++;
	
	// Add your process before here

	if ( u8ActiveProcessCount )
		return TRUE;
	return FALSE;
}

// ------------------------------------------------------------------------------------------------------------------------------
// Record PCM samples in input buffer to SPI-flash,
// ------------------------------------------------------------------------------------------------------------------------------
BOOL 					// TRUE: continue record, FALSE: finish record.
App_ProcessRec(void)
{
	UINT8 u8ActiveProcessCount = 0;
	
	// Add your process after here

	// Record PCm samples to SPI-flash
	if ( RecordApp_ProcessRec(&g_sApp.sRecordApp) == TRUE )
		u8ActiveProcessCount ++;
	
	// Add your process before here

	if ( u8ActiveProcessCount )
		return TRUE;
	return FALSE;
}

// ------------------------------------------------------------------------------------------------------------------------------
// Operation in main loop to record or playback.
// ------------------------------------------------------------------------------------------------------------------------------
void 
App_Process(void)
{
	// process in main loop according to different app mode
	if ( g_u8AppCtrl & APPCTRL_PLAY )
	{
		// Process of play mode
		if ( App_ProcessPlay() == FALSE )
		{
			// Play finish, stop play
			App_StartOrStopPlaySound();
		}
	}
	else if (g_u8AppCtrl & APPCTRL_WRITE)
	{
		// process of record mode
		if ( App_ProcessRec() == FALSE )
		{
			// Mean recording reach the end of desired length. Start to playback
			App_StartOrStopRecord();
		}
	}
}

//-----------------------------------------------------------------------------------------------------------
// Start or stop record.
//-----------------------------------------------------------------------------------------------------------
void 
App_StartOrStopRecord(void)
{
	if (g_u8AppCtrl & APPCTRL_PLAY)
		return;
	
	if ((g_u8AppCtrl & APPCTRL_WRITE) == 0)
	{
		// Start recording if it's not recording
		App_StartRec();
	}
	else
	{
		// Stop recording if it's recording
		App_StopRec();
		
		// Turn on LED as analysis and playback
		OUT2(0);	

		// Recording done, write header and analyze recorded sound.
		if (TalkToSingApp_RecordingDone(&g_sApp.sTalkToSingApp,g_sApp.sRecordApp.m_u32RecSampleCnt,TRUE) == TRUE)
		{
			// Start to play TalToSing and melody
			if (App_StartPlay() == TRUE)
				return;
		}
		// tuen off LED if it's failed to playback
		OUT2(1);
	}
}

//-----------------------------------------------------------------------------------------------------------
// Start or stop playback.
//-----------------------------------------------------------------------------------------------------------
void App_StartOrStopPlaySound(void)
{
	if (g_u8AppCtrl & APPCTRL_WRITE)
		return;
	if (g_u8AppCtrl & APPCTRL_PLAY)
	{
		// Stop playback if it's playing
		App_StopPlay();
	}
	else
	{
		// Start playback if it's not playing
		App_StartPlay();
	}
}

//-----------------------------------------------------------------------------------------------------------
// Flashing LED as recording.
//-----------------------------------------------------------------------------------------------------------
void App_RecordLEDFlashing(void)
{
	if (g_u8AppCtrl & APPCTRL_WRITE)
	{
		g_sApp.u16LEDFlashing++;
		
		if (g_sApp.u16LEDFlashing == 30)
			OUT2(0);
		else if (g_sApp.u16LEDFlashing == 60)
		{
			OUT2(1);
			g_sApp.u16LEDFlashing = 0;
		}
	}
}

//----------------------------------------------------------------------------------------------------
// Display LEDs.
//----------------------------------------------------------------------------------------------------
void
ShowLeds(void)
{
	OUT2(1);
	OUT3(1);
	OUT4(1);
	OUT5(1);
	OUT6(1);

	if (g_sApp.sTalkToSingParam.u8Option & TALKTOSING_AUTOTUNE_PITCH)
	{
		OUT3(0);
		if (g_sApp.sTalkToSingParam.u8Option & TALKTOSING_TUNE_SYLB_AVG_PITCH)
			OUT4(0);
	}
	
	if (g_sApp.sTalkToSingParam.u8Option & TALKTOSING_SYLB_NOTE_BEST_MATCH)
		OUT5(0);
	else
	{
		if (g_sApp.sTalkToSingParam.i8Echo == 0)
			OUT6(0);
		else
		{
			OUT5(0);
			OUT6(0);
		}
	}
}

//----------------------------------------------------------------------------------------------------
// Play next melody.
//----------------------------------------------------------------------------------------------------
void
App_PlayNextMelody(void)
{
	if (g_sApp.sTalkToSingParam.i16MelodyID == APP_END_MELODY_ID)
		g_sApp.sTalkToSingParam.i16MelodyID = APP_START_MELODY_ID;
	else
		g_sApp.sTalkToSingParam.i16MelodyID++;
	g_sApp.sTalkToSingParam.i8ChNum = g_i8ChnlNum[g_sApp.sTalkToSingParam.i16MelodyID-APP_START_MELODY_ID];
}

//----------------------------------------------------------------------------------------------------
// Enable / disable music.
//----------------------------------------------------------------------------------------------------
void
App_ToggleMusic(void)
{
	if (g_sApp.bEnableMusic == TRUE)
	{
		g_sApp.bEnableMusic = FALSE;
		BUF_CTRL_SET_MUTE(&g_sApp.sMidiExApp.sOutBufCtrl);
	}
	else
	{
		g_sApp.bEnableMusic = TRUE;
		BUF_CTRL_SET_UNMUTE(&g_sApp.sMidiExApp.sOutBufCtrl);
	}
}

//----------------------------------------------------------------------------------------------------
// Switch TalkToSing modes.
//----------------------------------------------------------------------------------------------------
void
App_SwitchTalkToSingMode(void)
{
	if (g_sApp.sTalkToSingParam.u8Option & TALKTOSING_SYLB_NOTE_BEST_MATCH)
	{
		g_sApp.sTalkToSingParam.u8Option &= ~TALKTOSING_SYLB_NOTE_BEST_MATCH;
		g_sApp.sTalkToSingParam.u8Option |= TALKTOSING_ONE2ONE_SYLB;
		g_sApp.sTalkToSingParam.i8Echo = 0;
	}
	else
	{
		if (g_sApp.sTalkToSingParam.i8Echo == 0)
		{
			g_sApp.sTalkToSingParam.i8Echo = 1;
		}
		else if (g_sApp.sTalkToSingParam.i8Echo == 1)
		{
			g_sApp.sTalkToSingParam.u8Option &= ~TALKTOSING_ONE2ONE_SYLB;
			g_sApp.sTalkToSingParam.u8Option |= TALKTOSING_SYLB_NOTE_BEST_MATCH;
		}
	}
	ShowLeds();
}

//----------------------------------------------------------------------------------------------------
// Switch singing mode.
//----------------------------------------------------------------------------------------------------
void
App_SwitchSingMode(void)
{
	if (g_sApp.sTalkToSingParam.u8Option & TALKTOSING_AUTOTUNE_PITCH)
	{
		if (g_sApp.sTalkToSingParam.u8Option & TALKTOSING_TUNE_SYLB_AVG_PITCH)
			g_sApp.sTalkToSingParam.u8Option &= ~(TALKTOSING_AUTOTUNE_PITCH|TALKTOSING_TUNE_SYLB_AVG_PITCH);
		else
			g_sApp.sTalkToSingParam.u8Option |= TALKTOSING_TUNE_SYLB_AVG_PITCH;
	}
	else
		g_sApp.sTalkToSingParam.u8Option |= (TALKTOSING_AUTOTUNE_PITCH);
	ShowLeds();
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

