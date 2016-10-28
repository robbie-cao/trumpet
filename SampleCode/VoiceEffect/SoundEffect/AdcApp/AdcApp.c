/*--------------------------------------------------------------------------------------------------*/
/*                                                                                                  */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.       	                            */
/*                                                                                                  */
/*--------------------------------------------------------------------------------------------------*/
#include "AdcApp.h"
#include "PlaybackRecord.h"
#include <string.h>
														   
//----------------------------------------------------------------------------------------------------
// Initialize ADC application.
//----------------------------------------------------------------------------------------------------
void
AdcApp_Initiate(
	S_ADCAPP *psAdcApp)
{
	// clear memoy buffer of AdcApp data structure 
	memset(psAdcApp,0,sizeof(S_ADCAPP));
	BUF_CTRL_SET_INACTIVE(&psAdcApp->sInBufCtrl);
}

//----------------------------------------------------------------------------------------------------
// Start to run ADC applicaiton.
//----------------------------------------------------------------------------------------------------
UINT32
AdcApp_StartPlay(
	S_ADCAPP *psAdcApp,
	UINT8 u8PlaybackChannel,
	UINT32 u32AdcSampleRate)
{
	// Set input buffer control.
	Record_SetInBufRecord(  &psAdcApp->sInBufCtrl, 
							ADCAPP_IN_BUF_SIZE,
							psAdcApp->i16InBuf,
							ADCAPP_IN_SAMPLES_PER_FRAME,
							u32AdcSampleRate);
	
	psAdcApp->sInBufCtrl.u16FrameSize = ADCAPP_IN_SAMPLES_PER_FRAME;
	psAdcApp->sInBufCtrl.u16BufReadIdx = psAdcApp->sInBufCtrl.u16BufWriteIdx = ADCAPP_IN_BUF_SIZE >> 1;	// half of buffer size to be output, and half to be input
	
	// Add to ADC record channel
	Record_Add(&psAdcApp->sInBufCtrl, u32AdcSampleRate);
	
	return u32AdcSampleRate;
}

//----------------------------------------------------------------------------------------------------
// Stop ADC application
//----------------------------------------------------------------------------------------------------
void 
AdcApp_StopPlay(
	S_ADCAPP *psAdcApp	// AutoTune app data structure
	)
{
	BUF_CTRL_SET_INACTIVE(&psAdcApp->sInBufCtrl);
}

BOOL
AdcApp_ProcessPlay(S_ADCAPP *psAdcApp)
{
	if (BUF_CTRL_IS_INACTIVE(&(psAdcApp->sInBufCtrl)))
		return FALSE;
	
	return TRUE;
}
