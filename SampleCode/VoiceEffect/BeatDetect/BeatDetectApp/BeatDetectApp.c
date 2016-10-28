/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include "BeatDetectApp.h"

//---------------------------------------------------------------------------------------------------------
// Initiate Beat Detection app.
//---------------------------------------------------------------------------------------------------------
void 
BeatDetectApp_Initiate( 
	S_BEATDETECT_APP *psBeatDetectApp	// Beat Detection app data structure
)
{
	// reset(initiate) application work buffer.
	memset( psBeatDetectApp, '\0', sizeof(S_BEATDETECT_APP) );
	// Set PCMs from ADC as default.
	psBeatDetectApp->sInBufCtrl.pfnFunc = NULL;
	psBeatDetectApp->sInBufCtrl.pu8Param = (void*)psBeatDetectApp;
	BUF_CTRL_SET_INACTIVE(&psBeatDetectApp->sInBufCtrl);
}

//---------------------------------------------------------------------------------------------------------
// Start record and beat detection.
//---------------------------------------------------------------------------------------------------------
void 
BeatDetectApp_StartRec( 
	S_BEATDETECT_APP *psBeatDetectApp,
	E_BNDETECTION_BEAT_DETECT_MODE 	eDetectMode,
	PFN_BEATDETECT_ENGRY_CALLBACK 	pfnEngryCallback
)
{
	// start Beat Detection record and detection
	UINT16 u16Size = BNDetection_BeatDetect_StartRec(psBeatDetectApp->au32WorkBuf,
		BEATDETECTAPP_WORK_BUF_SIZE, BEATDETECTAPP_MAX_INPUT_SAMPLE_RATE,eDetectMode);
	if (u16Size > BEATDETECTAPP_WORK_BUF_SIZE)
		while(1);	// work buffer size is not enough

	// engry callback function.
	psBeatDetectApp->pfnEngryCallback = pfnEngryCallback;

	// set adc buffer control.
	if ( (psBeatDetectApp->sInBufCtrl.pfnFunc == NULL) && (psBeatDetectApp->sInBufCtrl.pu8Param != NULL) )
		BeatDetectApp_SetInputFromADC(psBeatDetectApp, BEATDETECTAPP_MAX_INPUT_SAMPLE_RATE);
}

//---------------------------------------------------------------------------------------------------------
// Stop record and detection.
//---------------------------------------------------------------------------------------------------------
void 
BeatDetectApp_StopRec( 
	S_BEATDETECT_APP *psBeatDetectApp
)
{
	BUF_CTRL_SET_INACTIVE(&psBeatDetectApp->sInBufCtrl);
}

//---------------------------------------------------------------------------------------------------------
// Process recorded PCM samples for beat detection.    
//---------------------------------------------------------------------------------------------------------
BOOL 									// TRUE: continue detection, FALSE: finish detection
BeatDetectApp_ProcessRec( 
	S_BEATDETECT_APP *psBeatDetectApp
)
{
	INT16 i16Energy;
	E_BNDETECTION_RESULT eDetectResult;

	// check lib, need to detect or not.
	if( BNDetection_NeedDetectEx(psBeatDetectApp->au32WorkBuf) == TRUE )
	{
		// get beat detect's result.
		if ( (eDetectResult = BNDetection_BeatDetectEx(psBeatDetectApp->au32WorkBuf,&i16Energy)) !=  eBNDETECTION_NONE )
			psBeatDetectApp->pfnEngryCallback(i16Energy, eDetectResult);
	}
	return TRUE;
}

void
BeatDetectApp_SetInputFromADC(
	S_BEATDETECT_APP *psBeatDetectApp,
	UINT32 u32SampleRate
) 
{
	// set input(adc) buffer control(call-back structure).
	Record_SetInBufCallback(&psBeatDetectApp->sInBufCtrl,BNDetection_SetInputDataEx,psBeatDetectApp->au32WorkBuf);
		
	Record_Add((S_BUF_CTRL*)&(psBeatDetectApp->sInBufCtrl), u32SampleRate);
}

#if (BEATDETECTAPP_MULTI_INPUT_ENABLE)
BOOL
BeatDetectApp_SetInputFromBuf(
	S_BEATDETECT_APP *psBeatDetectApp,
	S_BUF_CTRL *psInPCMBuf
)
{
	if ( psInPCMBuf == NULL )
		return FALSE;

	// Set buffer handler "BufCtrl_ReadWithCount()" to handle input buffer "psInPCMBuf"
	Record_SetInBufCallback(&psBeatDetectApp->sInBufCtrl, (PFN_DATA_REQUEST_CALLBACK)BufCtrl_ReadWithCount, (void *)psInPCMBuf);

	return TRUE;
}

#endif // PITCHCHANGEAPP_MULTI_INPUT_ENABLE
