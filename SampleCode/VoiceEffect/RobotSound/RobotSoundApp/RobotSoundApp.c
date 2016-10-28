//---------------------------------------------------------------------------------------------------------
//                                                                                                         	
// Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              	
//                                                                                                        	
//---------------------------------------------------------------------------------------------------------
#include <string.h>
#include "App.h"

#define ROBOTSOUNDAPP_OUT_IN_SAMPLE_RATE_RATIO_x16	(ROBOTSOUNDAPP_OUT_IN_SAMPLE_RATE_RATIO*16)

// Because the ratio of input sample rate and output sample rate,
// bit7~bit4: integer part
// bit3~bit0: fraction part
#define ROBOTSOUNDAPP_IN_SAMPLES_PER_FRAME_x16	((ROBOTSOUNDAPP_OUT_SAMPLES_PER_FRAME*16)/ROBOTSOUNDAPP_OUT_IN_SAMPLE_RATE_RATIO)

void
PitchZone_TuneVolume(
	void* pPZ);

void
PitchZone_SetTuneVolThd(
	void *pPZ,
	INT16 i16TuneVolThd
	);

//---------------------------------------------------------------------------------------------------------
//   initiate frobot sound app                                                                        
//---------------------------------------------------------------------------------------------------------
void 
RobotSoundApp_Initiate( 
	S_ROBOTSOUND_APP *pRobotSoundApp	// robot sound app data structure
)
{
	// clear data buffer
	memset(pRobotSoundApp,0,sizeof(S_ROBOTSOUND_APP));
	BUF_CTRL_SET_INACTIVE(&pRobotSoundApp->sOutBufCtrl);
}

//---------------------------------------------------------------------------------------------------------
//  Start robot sound app.                                                           
//---------------------------------------------------------------------------------------------------------
void 
RobotSoundApp_StartPlay( 
	S_ROBOTSOUND_APP *pRobotSoundApp,	// robot sound app data structure
	UINT8 u8ChannelID					// channel ID of mixer
)
{
	// start to play Robot Sound
	UINT16 u16Size = RobotSound_StartPlay(pRobotSoundApp->au32WorkBuf,
								ROBOTSOUNDAPP_WORK_BUF_SIZE,
								ROBOTSOUNDAPP_IN_SAMPLE_RATE,
								ROBOTSOUNDAPP_OUT_SAMPLE_RATE);
	if (u16Size > ROBOTSOUNDAPP_WORK_BUF_SIZE)
		while(1);	// work buffer size is not enough

	RobotSound_SetMonoPitch(pRobotSoundApp->au32WorkBuf,60);	// default is 140, larger number output lower pitch,smaller number output higher pitch
	PitchZone_SetTuneVolThd(pRobotSoundApp->au32WorkBuf,64);	// set threshold of noise input
	
	// set dac buffer control.
	Playback_SetOutputBuf(&pRobotSoundApp->sOutBufCtrl,ROBOTSOUNDAPP_OUT_BUF_SIZE,pRobotSoundApp->i16DACBuf,ROBOTSOUNDAPP_OUT_SAMPLES_PER_FRAME,ROBOTSOUNDAPP_OUT_SAMPLE_RATE);
	
	// Add a channel to mixer
	Playback_Add(u8ChannelID,&pRobotSoundApp->sOutBufCtrl);
	pRobotSoundApp->u8ChannelID = u8ChannelID;
	
	if ((pRobotSoundApp->u8CtrlFlag&ROBOTSOUNDAPP_CTRL_INPUT_SOURCE) == 0)
		RobotSoundApp_SetInputFromADC(pRobotSoundApp);
}

//---------------------------------------------------------------------------------------------------------
//   Stop robo sound app. 
//---------------------------------------------------------------------------------------------------------
void 
RobotSoundApp_StopPlay( 
	S_ROBOTSOUND_APP *pRobotSoundApp	// robot sound app data structure
)
{
	BUF_CTRL_SET_INACTIVE(&pRobotSoundApp->sInBufCtrl);
	BUF_CTRL_SET_INACTIVE(&pRobotSoundApp->sOutBufCtrl);
	
	// Remvoe mixer channel
	Playback_Remove(pRobotSoundApp->u8ChannelID);
}

//---------------------------------------------------------------------------------------------------------
// 	Process PCM samples in input buffer, add robot sound effect and put into output buffer.
//---------------------------------------------------------------------------------------------------------
BOOL 									// TRUE: continue play, FALSE: finish play
RobotSoundApp_ProcessPlay( 
	S_ROBOTSOUND_APP *pRobotSoundApp	// robot sound app data structure
)
{
	S_BUF_CTRL *psOutBufCtrl = &pRobotSoundApp->sOutBufCtrl;
	if (BUF_CTRL_IS_INACTIVE(psOutBufCtrl))
		return FALSE;
	// check output buffer index to check if need fill new samples
	if(Playback_NeedUpdateOutputBuf(psOutBufCtrl))
	{
		// get effected PCM data from robot sound lib.
		RobotSound_FillPcmBuf( pRobotSoundApp->au32WorkBuf,&(pRobotSoundApp->sOutBufCtrl.pi16Buf[psOutBufCtrl->u16BufWriteIdx]));	
		PitchZone_TuneVolume(pRobotSoundApp->au32WorkBuf);	// compress volume of noise input
		
		// update output buffer index
		Playback_UpdateOutputBuf(psOutBufCtrl);
		
		if ( pRobotSoundApp->u8CtrlFlag&ROBOTSOUNDAPP_CTRL_INPUT_SOURCE )
		{
			// PCM inputed from PCM buffer not from ADC
			INT16 i16PCM[ROBOTSOUNDAPP_OUT_SAMPLES_PER_FRAME];
			UINT16 u16Count;
			S_BUF_CTRL *psBufCtrl;
			
			psBufCtrl = pRobotSoundApp->psInBufCtrl;
			u16Count = psBufCtrl->u16ReSamplingCalculation;
			u16Count += ROBOTSOUNDAPP_OUT_IN_SAMPLE_RATE_RATIO_x16;
			psBufCtrl->u16ReSamplingCalculation = u16Count&0xf;		// bit3~bit0: fraction part 
			u16Count >>= 4;											// bit7~bit4: integer part
			if ( pRobotSoundApp->u8CtrlFlag&ROBOTSOUNDAPP_CTRL_INPUT_FROM_BUF )
				BufCtrl_ReadWithCount(pRobotSoundApp->psInBufCtrl, u16Count, i16PCM);
			else// if ( psAutoTuneApp->u8CtrlFlag&AUTOTUNEAPP_CTRL_INPUT_FROM_FUNC )
				pRobotSoundApp->pfnInputFunc(u16Count, i16PCM);
		}
	}
	return TRUE;
}

BOOL
RobotSoundApp_SetInputFromBuf(
	S_ROBOTSOUND_APP *pRobotSoundApp,
	S_BUF_CTRL *psInPCMBuf
)
{
	if ( psInPCMBuf == NULL )
		return FALSE;
	pRobotSoundApp->sOutBufCtrl.u16SampleRate = 
		((UINT32)(((UINT32)psInPCMBuf->u16SampleRate)*ROBOTSOUNDAPP_OUT_IN_SAMPLE_RATE_RATIO))>>4;
	pRobotSoundApp->psInBufCtrl = psInPCMBuf;
	pRobotSoundApp->u8CtrlFlag = 
		(pRobotSoundApp->u8CtrlFlag&(~ROBOTSOUNDAPP_CTRL_INPUT_SOURCE))|ROBOTSOUNDAPP_CTRL_INPUT_FROM_BUF;
	
	Playback_Add(pRobotSoundApp->u8ChannelID,&pRobotSoundApp->sOutBufCtrl);

	return TRUE;
}

BOOL
RobotSoundApp_SetInputFromFunc(
	S_ROBOTSOUND_APP *pRobotSoundApp,
	PFN_ROBOTSOUNDAPP_INPUT_FUNC pfnInputFunc, UINT32 u32SampleRate)
{
	if ( pfnInputFunc == NULL )
		return FALSE;
	pRobotSoundApp->sOutBufCtrl.u16SampleRate = 
		((UINT32)(u32SampleRate*ROBOTSOUNDAPP_OUT_IN_SAMPLE_RATE_RATIO))>>4;
	pRobotSoundApp->pfnInputFunc = pfnInputFunc;
	pRobotSoundApp->u8CtrlFlag = 
		(pRobotSoundApp->u8CtrlFlag&(~ROBOTSOUNDAPP_CTRL_INPUT_SOURCE))|ROBOTSOUNDAPP_CTRL_INPUT_FROM_FUNC;
	
	Playback_Add(pRobotSoundApp->u8ChannelID,&pRobotSoundApp->sOutBufCtrl);

	return TRUE;
}

void
RobotSoundApp_SetInputFromADC(
	S_ROBOTSOUND_APP *pRobotSoundApp)		// robot sound app data structure
{
	// set input(adc) buffer control(call-back structure).
	pRobotSoundApp->u8CtrlFlag = pRobotSoundApp->u8CtrlFlag&(~ROBOTSOUNDAPP_CTRL_INPUT_SOURCE);
	Record_SetInBufCallback(&pRobotSoundApp->sInBufCtrl,PitchChange_SetInputData,pRobotSoundApp->au32WorkBuf);
	
	Record_Add((S_BUF_CTRL*)&(pRobotSoundApp->sInBufCtrl), ROBOTSOUNDAPP_IN_SAMPLE_RATE);
}
