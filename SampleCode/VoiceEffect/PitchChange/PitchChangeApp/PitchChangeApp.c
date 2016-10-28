/*--------------------------------------------------------------------------------------------------*/
/*                                                                                                  */
/* Copyright(c) 2010 Nuvoton Technology Corp. All rights reserved.                                  */
/*                                                                                                  */
/*--------------------------------------------------------------------------------------------------*/
#include "App.h"
#include <string.h>

#define PITCHANGEAPP_OUT_IN_SAMPLE_RATE_RATIO_x16	(PITCHANGEAPP_OUT_IN_SAMPLE_RATE_RATIO*16)

// Because the ratio of input sample rate and output sample rate,
// bit7~bit4: integer part
// bit3~bit0: fraction part
#define PITCHANGEAPP_IN_SAMPLES_PER_FRAME_x16	((PITCHCHANGEAPP_OUT_SAMPLES_PER_FRAME*16)/PITCHANGEAPP_OUT_IN_SAMPLE_RATE_RATIO)

void
PitchZone_TuneVolume(
	void* pPZ);

void
PitchZone_SetTuneVolThd(
	void *pPZ,
	INT16 i16TuneVolThd
	);


//----------------------------------------------------------------------------------------------------
// Set pitch shifting number in semitone
//----------------------------------------------------------------------------------------------------
void 
PitchChangeApp_SetPitchShift(
	S_PITCHCHANGEAPP *psPitchChangeApp,		// pitch change app data structure 
	INT16 i16PitchShiftNum					// pitch shift number in semitone (positive: pitch up, negtive: pitch down)
	)
{
	PitchChange_SetPitchShift(psPitchChangeApp->au32PitchChangeBuf,i16PitchShiftNum);
}

//----------------------------------------------------------------------------------------------------
// Initialize Pitch Change application.
//----------------------------------------------------------------------------------------------------
void 
PitchChangeApp_Initiate(
	S_PITCHCHANGEAPP *psPitchChangeApp		// pitch change app data structure 
)
{
	// clear data buffer
	memset(psPitchChangeApp,0,sizeof(S_PITCHCHANGEAPP));
	BUF_CTRL_SET_INACTIVE(&psPitchChangeApp->sOutBufCtrl);
}

//----------------------------------------------------------------------------------------------------
// Start to play PitchChange app
//----------------------------------------------------------------------------------------------------
void 
PitchChangeApp_StartPlay(
	S_PITCHCHANGEAPP *psPitchChangeApp,		// pitch change app data structure 
	UINT8 u8ChannelID,						// channel ID of mixer
	INT16 i16PitchShiftNum					// pitch shift number in semitone (positive: pitch up, negtive: pitch down)
)
{
	
	// start to play Pitch Change
	UINT16 u16Size = PitchChange_StartPlay(psPitchChangeApp->au32PitchChangeBuf,PITCHCHANGEAPP_BUF_SIZE,PITCHCHANGEAPP_IN_SAMPLE_RATE,PITCHCHANGEAPP_OUT_SAMPLE_RATE);
	if (u16Size > PITCHCHANGEAPP_BUF_SIZE)
		while(1);
	
	// Set pitch shift number in semitone
	PitchChange_SetPitchShift(psPitchChangeApp->au32PitchChangeBuf,i16PitchShiftNum);
	PitchZone_SetTuneVolThd(psPitchChangeApp->au32PitchChangeBuf,64);	// set threshold of noise input

	// set out buffer control.
	Playback_SetOutputBuf(&psPitchChangeApp->sOutBufCtrl,PITCHCHANGEAPP_OUT_BUF_SIZE,psPitchChangeApp->i16OutBuf,PITCHCHANGEAPP_PROCESS_SAMPLES,PITCHCHANGEAPP_OUT_SAMPLE_RATE);
	
	// Add a channel to mixer
	Playback_Add(u8ChannelID,&psPitchChangeApp->sOutBufCtrl);
	psPitchChangeApp->u8ChannelID = u8ChannelID;
	
	if ( (psPitchChangeApp->u8CtrlFlag&PITCHCHANGEAPP_CTRL_INPUT_SOURCE) == 0 )
		PitchChangeApp_SetInputFromADC(psPitchChangeApp);
}


//----------------------------------------------------------------------------------------------------
// Stop Pitch Change app.
//----------------------------------------------------------------------------------------------------
void 
PitchChangeApp_StopPlay(
	S_PITCHCHANGEAPP *psPitchChangeApp		// pitch change app data structure 
)
{
	BUF_CTRL_SET_INACTIVE(&psPitchChangeApp->sOutBufCtrl);
	
	// Remvoe mixer channel
	Playback_Remove(psPitchChangeApp->u8ChannelID);
}

//----------------------------------------------------------------------------------------------------
// Process to shift te pitch of PCM samples in input buffer and put into output buffer.
//----------------------------------------------------------------------------------------------------
BOOL 										// TRUE: continue play, FALSE: finish play
PitchChangeApp_ProcessPlay(
	S_PITCHCHANGEAPP *psPitchChangeApp		// pitch change app data structure 
)
{
	S_BUF_CTRL *psOutBufCtrl = &psPitchChangeApp->sOutBufCtrl;
	if (BUF_CTRL_IS_INACTIVE(psOutBufCtrl))
		return FALSE;
	// check output buffer index to check if need fill new samples
	if (Playback_NeedUpdateOutputBuf(psOutBufCtrl))
	{
		// fill pitch changed PCM samples into ring buffer
		PitchChange_FillPcmBuf(psPitchChangeApp->au32PitchChangeBuf,&(psPitchChangeApp->i16OutBuf[psOutBufCtrl->u16BufWriteIdx]));	// pitch-change
		PitchZone_TuneVolume(psPitchChangeApp->au32PitchChangeBuf);		// compress volume of noise input

		// update output buffer index
		Playback_UpdateOutputBuf(psOutBufCtrl);
		
		if ( psPitchChangeApp->u8CtrlFlag&PITCHCHANGEAPP_CTRL_INPUT_SOURCE )
		{
			// PCM inputed from PCM buffer not from ADC
			INT16 i16PCM[PITCHCHANGEAPP_OUT_SAMPLES_PER_FRAME];
			UINT16 u16Count;
			S_BUF_CTRL *psBufCtrl;

			psBufCtrl = psPitchChangeApp->psInBufCtrl;
			u16Count = psBufCtrl->u16ReSamplingCalculation;
			u16Count += PITCHANGEAPP_IN_SAMPLES_PER_FRAME_x16;
			psBufCtrl->u16ReSamplingCalculation = u16Count&0xf;		// bit3~bit0: fraction part 
			u16Count >>= 4;											// bit7~bit4: integer part
	
			if ( psPitchChangeApp->u8CtrlFlag&PITCHCHANGEAPP_CTRL_INPUT_FROM_BUF )
				BufCtrl_ReadWithCount(psPitchChangeApp->psInBufCtrl, u16Count, i16PCM);
			else //if ( psPitchChangeApp->u8CtrlFlag&PITCHCHANGEAPP_CTRL_INPUT_FROM_FUNC )
				psPitchChangeApp->pfnInputFunc(u16Count, i16PCM);
			
			PitchChange_SetInputData(psPitchChangeApp->au32PitchChangeBuf, u16Count, i16PCM);
		}
	}
	return TRUE;
}

BOOL
PitchChangeApp_SetInputFromBuf(
	S_PITCHCHANGEAPP *psPitchChangeApp,		// pitch change app data structure
	S_BUF_CTRL *psInPCMBuf
)
{
	if ( psInPCMBuf == NULL )
		return FALSE;
	psPitchChangeApp->sOutBufCtrl.u16SampleRate = 
		((UINT32)(((UINT32)psInPCMBuf->u16SampleRate)*PITCHANGEAPP_OUT_IN_SAMPLE_RATE_RATIO_x16))>>4;
	psPitchChangeApp->psInBufCtrl = psInPCMBuf;
	psPitchChangeApp->u8CtrlFlag = (psPitchChangeApp->u8CtrlFlag&(~PITCHCHANGEAPP_CTRL_INPUT_SOURCE))|PITCHCHANGEAPP_CTRL_INPUT_FROM_BUF;
	
	Playback_Add(psPitchChangeApp->u8ChannelID,&psPitchChangeApp->sOutBufCtrl);

	return TRUE;
}


BOOL
PitchChangeApp_SetInputFromFunc(
	S_PITCHCHANGEAPP *psPitchChangeApp,
	PFN_PITCHCHANGEAPP_INPUT_FUNC pfnInputFunc, UINT32 u32SampleRate
)
{
	if ( pfnInputFunc == NULL )
		return FALSE;
	psPitchChangeApp->sOutBufCtrl.u16SampleRate = 
		((UINT32)(u32SampleRate*PITCHANGEAPP_OUT_IN_SAMPLE_RATE_RATIO_x16))>>4;
	psPitchChangeApp->pfnInputFunc = pfnInputFunc;
	psPitchChangeApp->u8CtrlFlag = (psPitchChangeApp->u8CtrlFlag&(~PITCHCHANGEAPP_CTRL_INPUT_SOURCE))|PITCHCHANGEAPP_CTRL_INPUT_FROM_FUNC;
	
	Playback_Add(psPitchChangeApp->u8ChannelID,&psPitchChangeApp->sOutBufCtrl);

	return TRUE;
}

void
PitchChangeApp_SetInputFromADC(
	S_PITCHCHANGEAPP *psPitchChangeApp		// pitch change app data structure
) 
{	// set input(adc) buffer control(call-back structure).
	psPitchChangeApp->u8CtrlFlag &= (~PITCHCHANGEAPP_CTRL_INPUT_SOURCE);
	Record_SetInBufCallback(&psPitchChangeApp->sInBufCtrl,PitchChange_SetInputData,psPitchChangeApp->au32PitchChangeBuf);
	
	Record_Add((S_BUF_CTRL*)&(psPitchChangeApp->sInBufCtrl), PITCHCHANGEAPP_IN_SAMPLE_RATE);
}
