/*--------------------------------------------------------------------------------------------------*/
/*                                                                                                  */
/* Copyright(c) 2010 Nuvoton Technology Corp. All rights reserved.                                  */
/*                                                                                                  */
/*--------------------------------------------------------------------------------------------------*/
#include "Platform.h"
#include <string.h>
#include "BNDetection.h"
//#include "Keypad.h"
#include "App.h"

#define HARMONYSNDAPP_OUT_IN_SAMPLE_RATE_RATIO_x16	(HARMONYSNDAPP_OUT_IN_SAMPLE_RATE_RATIO*16)

// Because the ratio of input sample rate and output sample rate,
// bit7~bit4: integer part
// bit3~bit0: fraction part
#define HARMONYSNDAPP_IN_SAMPLES_PER_FRAME_x16	((HARMONYSNDAPP_OUT_SAMPLES_PER_FRAME*16)/HARMONYSNDAPP_OUT_IN_SAMPLE_RATE_RATIO)

extern S_APP g_sApp; 

void ShowLeds(void);

// parameters for channels of harmony sound
const S_HARMONYSND_CH_PARAMETERS g_asCHParameters[HARMONYSNDAPP_CH_NUM] = 
{ 
	{0,3,100,1,255,0x40,100}, 
	{1,-3,200,1,255,0x00,100} 
};

//----------------------------------------------------------------------------------------------------
// Enable a harmony channel.
//----------------------------------------------------------------------------------------------------
void
HarmonySndApp_ChannelEnable(
	S_HARMONYSNDAPP *psHarmonySndApp,	// harmony sound app data structure
	UINT8 u8Channel						// channel index to be enabled
)
{
	HarmonySnd_Enable(psHarmonySndApp->m_au8HmySndBuf,u8Channel);
}

//----------------------------------------------------------------------------------------------------
// Disable a harmony channel.
//----------------------------------------------------------------------------------------------------
void
HarmonySndApp_ChannelDisable(
	S_HARMONYSNDAPP *psHarmonySndApp,	// harmony sound app data structure
	UINT8 u8Channel						// channel index to be disabled
)
{
	HarmonySnd_Disable(psHarmonySndApp->m_au8HmySndBuf,u8Channel);	
}

//----------------------------------------------------------------------------------------------------
// Initilize Harmony Sound application.
//----------------------------------------------------------------------------------------------------
void 
HarmonySndApp_Initiate(
	S_HARMONYSNDAPP *psHarmonySndApp	// harmony sound app data structure
)
{
	// clear data buffer
	memset(psHarmonySndApp,0,sizeof(S_HARMONYSNDAPP));
	BUF_CTRL_SET_INACTIVE(&psHarmonySndApp->sOutBufCtrl);
}

//----------------------------------------------------------------------------------------------------
// Start play harmony sound. Initiate harmony channels and output buffer.
//----------------------------------------------------------------------------------------------------
void
HarmonySndApp_StartPlay(
	S_HARMONYSNDAPP *psHarmonySndApp,	// harmony sound app data structure
	UINT8 u8ChannelID					// channel ID of mixer
)
{
	UINT8 u8ChNum;
	// Start to play harmony sound
	UINT16 u16Size = HarmonySound_StartPlay(psHarmonySndApp->m_au8HmySndBuf,
								HARMONYSNDAPP_WORK_BUF_SIZE,
								HARMONYSNDAPP_IN_SAMPLE_RATE,
								HARMONYSNDAPP_OUT_SAMPLE_RATE,
								HARMONYSNDAPP_CH_NUM);
	if (u16Size > HARMONYSNDAPP_WORK_BUF_SIZE)
		while(1);

	// Initiate harmony channels
	for (u8ChNum = 0; u8ChNum<HARMONYSNDAPP_CH_NUM; u8ChNum++)
	{
		HarmonySnd_SetPitchShiftSemiTone(psHarmonySndApp->m_au8HmySndBuf,
										 g_asCHParameters[u8ChNum].u8ChannelIndex,
										 g_asCHParameters[u8ChNum].i8ShiftPitch,
										 g_asCHParameters[u8ChNum].u8Volume,
										 g_asCHParameters[u8ChNum].u8ModRange,
										 g_asCHParameters[u8ChNum].u8ModDepth,
										 g_asCHParameters[u8ChNum].u8Modfreq,
										 g_asCHParameters[u8ChNum].u8ModDelayTime);
		// Default: all channel are enabled
	}

	// set dac buffer control.
	Playback_SetOutputBuf(&psHarmonySndApp->sOutBufCtrl,HARMONYSNDAPP_OUT_BUF_SIZE,psHarmonySndApp->i16DacBuf,HARMONYSNDAPP_OUT_SAMPLES_PER_FRAME,HARMONYSNDAPP_OUT_SAMPLE_RATE);
	
	// Add a channel to mixer
	psHarmonySndApp->u8ChannelID = u8ChannelID;
	Playback_Add(u8ChannelID,&psHarmonySndApp->sOutBufCtrl);
	
	
	if ( (psHarmonySndApp->u8CtrlFlag&HARMONYSNDAPP_CTRL_INPUT_SOURCE) == 0 )
		HarmonySndApp_SetInputFromADC(psHarmonySndApp);
}

//---------------------------------------------------------------------------------------------------
// Stop to playback.
//----------------------------------------------------------------------------------------------------
void
HarmonySndApp_StopPlay(
	S_HARMONYSNDAPP *psHarmonySndApp	// harmony sound app data structure
)
{
	BUF_CTRL_SET_INACTIVE(&psHarmonySndApp->sInBufCtrl);
	BUF_CTRL_SET_INACTIVE(&psHarmonySndApp->sOutBufCtrl);
	
	// Remvoe mixer channel
	Playback_Remove(psHarmonySndApp->u8ChannelID);
}

//----------------------------------------------------------------------------------------------------
// Process PCM samples in input buffer according to setting of harmony channels to get 
// PCM samples of harmony sound, and output them into output buffer.
//----------------------------------------------------------------------------------------------------
BOOL
HarmonySndApp_ProcessPlay(
	S_HARMONYSNDAPP *psHarmonySndApp	// harmony sound app data structure
)
{
	S_BUF_CTRL *psOutBufCtrl = &psHarmonySndApp->sOutBufCtrl;
	if (BUF_CTRL_IS_INACTIVE(psOutBufCtrl))
		return FALSE;
	// check output buffer index to check if need fill new samples
	if(Playback_NeedUpdateOutputBuf(psOutBufCtrl))
	{
		// fill harmony sound PCM samples into ring buffer
		HarmonySnd_FillPcmBuf(psHarmonySndApp->m_au8HmySndBuf,&psHarmonySndApp->i16DacBuf[psOutBufCtrl->u16BufWriteIdx]);
		
		// update output buffer index
		Playback_UpdateOutputBuf(psOutBufCtrl);
		
		if ( psHarmonySndApp->u8CtrlFlag&HARMONYSNDAPP_CTRL_INPUT_SOURCE )
		{
			// PCM inputed from PCM buffer not from ADC
			INT16 i16PCM[HARMONYSNDAPP_OUT_SAMPLES_PER_FRAME];
			UINT16 u16Count;
			S_BUF_CTRL *psBufCtrl;

			psBufCtrl = psHarmonySndApp->psInBufCtrl;
			u16Count = psBufCtrl->u16ReSamplingCalculation;
			u16Count += HARMONYSNDAPP_IN_SAMPLES_PER_FRAME_x16;
			psBufCtrl->u16ReSamplingCalculation = u16Count&0xf;		// bit3~bit0: fraction part 
			u16Count >>= 4;											// bit7~bit4: integer part
			if ( psHarmonySndApp->u8CtrlFlag&HARMONYSNDAPP_CTRL_INPUT_FROM_FUNC )
				BufCtrl_ReadWithCount(psHarmonySndApp->psInBufCtrl, u16Count, i16PCM);
			else// if ( psAutoTuneApp->u8CtrlFlag&AUTOTUNEAPP_CTRL_INPUT_FROM_FUNC )
				psHarmonySndApp->pfnInputFunc(u16Count, i16PCM);
				
			PitchChange_SetInputData(psHarmonySndApp->m_au8HmySndBuf, u16Count, i16PCM);
		}
	}
	return TRUE;
}

BOOL
HarmonySndApp_SetInputFromBuf(
	S_HARMONYSNDAPP *psHarmonySndApp,		// harmony sound app data structure
	S_BUF_CTRL *psInPCMBuf					// Structure pointer of source buffer 
)
{
	if ( psInPCMBuf == NULL )
		return FALSE;
	psHarmonySndApp->sOutBufCtrl.u16SampleRate = 
		((UINT32)(((UINT32)psInPCMBuf->u16SampleRate)*HARMONYSNDAPP_OUT_IN_SAMPLE_RATE_RATIO_x16))>>4;
	psHarmonySndApp->psInBufCtrl = psInPCMBuf;
	psHarmonySndApp->u8CtrlFlag = 
		(psHarmonySndApp->u8CtrlFlag&(~HARMONYSNDAPP_CTRL_INPUT_SOURCE))|HARMONYSNDAPP_CTRL_INPUT_FROM_BUF;
	
	Playback_Add(psHarmonySndApp->u8ChannelID,&psHarmonySndApp->sOutBufCtrl);

	return TRUE;
}

BOOL
HarmonySndApp_SetInputFromFunc(
	S_HARMONYSNDAPP *psHarmonySndApp,		// harmony sound app data structure
	PFN_HARMONYSNDAPP_INPUT_FUNC pfnInputFunc,		// Callback input function 
	UINT32 u32SampleRate					// audio data sample rate after Callback function processing.
)
{
	if ( pfnInputFunc == NULL )
		return FALSE;
	psHarmonySndApp->sOutBufCtrl.u16SampleRate = 
		((UINT32)(u32SampleRate*HARMONYSNDAPP_OUT_IN_SAMPLE_RATE_RATIO_x16))>>4;
	psHarmonySndApp->pfnInputFunc = pfnInputFunc;
	psHarmonySndApp->u8CtrlFlag = 
		(psHarmonySndApp->u8CtrlFlag&(~HARMONYSNDAPP_CTRL_INPUT_SOURCE))|HARMONYSNDAPP_CTRL_INPUT_FROM_FUNC;
	
	Playback_Add(psHarmonySndApp->u8ChannelID,&psHarmonySndApp->sOutBufCtrl);

	return TRUE;
}

void
HarmonySndApp_SetInputFromADC(
	S_HARMONYSNDAPP *psHarmonySndApp		// harmony sound app data structure
)
{
	// set input(adc) buffer control(call-back structure).
	psHarmonySndApp->u8CtrlFlag = psHarmonySndApp->u8CtrlFlag&(~HARMONYSNDAPP_CTRL_INPUT_SOURCE);
	Record_SetInBufCallback(&psHarmonySndApp->sInBufCtrl,PitchChange_SetInputData,psHarmonySndApp->m_au8HmySndBuf);
	
	Record_Add((S_BUF_CTRL*)&(psHarmonySndApp->sInBufCtrl), HARMONYSNDAPP_IN_SAMPLE_RATE);
}
