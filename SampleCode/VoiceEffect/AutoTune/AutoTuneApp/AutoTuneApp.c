/*--------------------------------------------------------------------------------------------------*/
/*                                                                                                  */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.       	                            */
/*                                                                                                  */
/*--------------------------------------------------------------------------------------------------*/
#include "AutoTuneApp.h"
#include "PlaybackRecord.h"
#include <string.h>


#define AUTOTUNEAPP_OUT_IN_SAMPLE_RATE_RATIO_x16	(AUTOTUNEAPP_OUT_IN_SAMPLE_RATE_RATIO*16)

// Because the ratio of input sample rate and output sample rate,
// bit7~bit4: integer part
// bit3~bit0: fraction part
#define AUTOTUNEAPP_IN_SAMPLES_PER_FRAME_x16	((AUTOTUNEAPP_OUT_SAMPLES_PER_FRAME*16)/AUTOTUNEAPP_OUT_IN_SAMPLE_RATE_RATIO)

															   
//----------------------------------------------------------------------------------------------------
// Initialize auto tune application.
//----------------------------------------------------------------------------------------------------
void
AutoTuneApp_Initiate(
	S_AUTOTUNEAPP *psAutoTuneApp)	// AutoTune app data structure
{
	// clear memoy buffer of AutoTune data structure 
	memset(psAutoTuneApp,0,sizeof(S_AUTOTUNEAPP));
	BUF_CTRL_SET_INACTIVE(&psAutoTuneApp->sOutBufCtrl);
}

//----------------------------------------------------------------------------------------------------
// Start to run AutoTune applicaiton.
//----------------------------------------------------------------------------------------------------
UINT32								// output sample rate
AutoTuneApp_StartPlay(
	S_AUTOTUNEAPP *psAutoTuneApp,	// AutoTune app data structure
	UINT8 u8ChannelID,				// mixer channel
	INT8 i8AutoTunePitchShift)		// pitch shift in semitone unit
{
	// Start play AutoTune
	UINT16 u16Size = AutoTune_StartPlay(psAutoTuneApp->au32WorkBuf,AUTOTUNEAPP_12K_WORK_BUF_SIZE,AUTOTUNEAPP_IN_SAMPLE_RATE,AUTOTUNEAPP_OUT_SAMPLE_RATE);
	if (u16Size > AUTOTUNEAPP_12K_WORK_BUF_SIZE)
		while(1);
	// Set pitch shift number of AutoTune
	AutoTune_SetPitchShift(psAutoTuneApp->au32WorkBuf,i8AutoTunePitchShift);

	// set dac buffer control.
	Playback_SetOutputBuf(&psAutoTuneApp->sOutBufCtrl,AUTOTUNEAPP_OUT_BUF_SIZE,psAutoTuneApp->i16OutBuf,AUTOTUNEAPP_PROCESS_SAMPLES,AUTOTUNEAPP_OUT_SAMPLE_RATE);
	// Add a channel to mixer
	Playback_Add(u8ChannelID,&psAutoTuneApp->sOutBufCtrl);
	psAutoTuneApp->u8ChannelID = u8ChannelID;
	
	if ( (psAutoTuneApp->u8CtrlFlag&AUTOTUNEAPP_CTRL_INPUT_SOURCE) == 0 )
		AutoTuneApp_SetInputFromADC(psAutoTuneApp);
	
	return AUTOTUNEAPP_OUT_SAMPLE_RATE;
}

//----------------------------------------------------------------------------------------------------
// Stop AutoTune application
//----------------------------------------------------------------------------------------------------
void 
AutoTuneApp_StopPlay(
	S_AUTOTUNEAPP *psAutoTuneApp	// AutoTune app data structure
	)
{
	BUF_CTRL_SET_INACTIVE(&psAutoTuneApp->sInBufCtrl);
	BUF_CTRL_SET_INACTIVE(&psAutoTuneApp->sOutBufCtrl);
	
	// Remvoe mixer channel
	Playback_Remove(psAutoTuneApp->u8ChannelID);
}


//----------------------------------------------------------------------------------------------------
// Operations in main loop for playing. 
// It gets PCM samples from Auto Tune library and put into output buffer.
//----------------------------------------------------------------------------------------------------
BOOL 								// TRUE: continue playback, FALSE: stop playback
AutoTuneApp_ProcessPlay(
	S_AUTOTUNEAPP *psAutoTuneApp)	// AutoTune app data structure
{
	S_BUF_CTRL *psOutBufCtrl = &psAutoTuneApp->sOutBufCtrl;
	if (BUF_CTRL_IS_INACTIVE(psOutBufCtrl))
		return FALSE;
	// check output buffer index to check if need fill new samples
	if(Playback_NeedUpdateOutputBuf(psOutBufCtrl))
	{
		// fill AutoTune PCM samples into ring buffer
		AutoTune_FillPcmBuf((UINT8*)psAutoTuneApp->au32WorkBuf,&psOutBufCtrl->pi16Buf[psOutBufCtrl->u16BufWriteIdx]);
		// update output buffer index
		Playback_UpdateOutputBuf(psOutBufCtrl);
				
		if ( psAutoTuneApp->u8CtrlFlag&AUTOTUNEAPP_CTRL_INPUT_SOURCE )
		{
			// PCM inputed from PCM buffer not from ADC
			INT16 i16PCM[AUTOTUNEAPP_OUT_SAMPLES_PER_FRAME];
			UINT16 u16Count;
			S_BUF_CTRL *psBufCtrl;

			psBufCtrl = psAutoTuneApp->psInBufCtrl;
			u16Count = psBufCtrl->u16ReSamplingCalculation;
			u16Count += AUTOTUNEAPP_IN_SAMPLES_PER_FRAME_x16;
			psBufCtrl->u16ReSamplingCalculation = u16Count&0xf;		// bit3~bit0: fraction part 
			u16Count >>= 4;											// bit7~bit4: integer part
			if ( psAutoTuneApp->u8CtrlFlag&AUTOTUNEAPP_CTRL_INPUT_FROM_BUF )
				BufCtrl_ReadWithCount(psAutoTuneApp->psInBufCtrl, u16Count, i16PCM);
			else// if ( psAutoTuneApp->u8CtrlFlag&AUTOTUNEAPP_CTRL_INPUT_FROM_FUNC )
				psAutoTuneApp->pfnInputFunc(u16Count, i16PCM);
				
			AutoTune_SetInputData(psAutoTuneApp->au32WorkBuf, u16Count, i16PCM);
		}
	}
	return TRUE;
}
BOOL
AutoTuneApp_SetInputFromBuf(
	S_AUTOTUNEAPP *psAutoTuneApp,
	S_BUF_CTRL *psInPCMBuf
)
{
	if ( psInPCMBuf == NULL )
		return FALSE;
	psAutoTuneApp->sOutBufCtrl.u16SampleRate = 
		((UINT32)(((UINT32)psInPCMBuf->u16SampleRate)*AUTOTUNEAPP_OUT_IN_SAMPLE_RATE_RATIO_x16))>>4;
	psAutoTuneApp->psInBufCtrl = psInPCMBuf;
	psAutoTuneApp->u8CtrlFlag = 
		(psAutoTuneApp->u8CtrlFlag&(~AUTOTUNEAPP_CTRL_INPUT_SOURCE))|AUTOTUNEAPP_CTRL_INPUT_FROM_BUF;
	
	Playback_Add(psAutoTuneApp->u8ChannelID,&psAutoTuneApp->sOutBufCtrl);

	return TRUE;
}

BOOL
AutoTuneApp_SetInputFromFunc(
	S_AUTOTUNEAPP *psAutoTuneApp,
	PFN_AUTOTUNEAPP_INPUT_FUNC pfnInputFunc, UINT32 u32SampleRate)
{
	if ( pfnInputFunc == NULL )
		return FALSE;
	psAutoTuneApp->sOutBufCtrl.u16SampleRate = 
		((UINT32)(u32SampleRate*AUTOTUNEAPP_OUT_IN_SAMPLE_RATE_RATIO_x16))>>4;
	psAutoTuneApp->pfnInputFunc = pfnInputFunc;
	psAutoTuneApp->u8CtrlFlag = 
		(psAutoTuneApp->u8CtrlFlag&(~AUTOTUNEAPP_CTRL_INPUT_SOURCE))|AUTOTUNEAPP_CTRL_INPUT_FROM_FUNC;
	
	Playback_Add(psAutoTuneApp->u8ChannelID,&psAutoTuneApp->sOutBufCtrl);

	return TRUE;
}

void
AutoTuneApp_SetInputFromADC(
	S_AUTOTUNEAPP *psAutoTuneApp
) 
{	// set input(adc) buffer control(call-back structure).
	psAutoTuneApp->u8CtrlFlag = psAutoTuneApp->u8CtrlFlag&(~AUTOTUNEAPP_CTRL_INPUT_SOURCE);
	Record_SetInBufCallback(&psAutoTuneApp->sInBufCtrl,PitchChange_SetInputData,psAutoTuneApp->au32WorkBuf);
	
	Record_Add((S_BUF_CTRL*)&(psAutoTuneApp->sInBufCtrl), AUTOTUNEAPP_IN_SAMPLE_RATE);
}
