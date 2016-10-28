/*--------------------------------------------------------------------------------------------------*/
/*                                                                                                  */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                       */
/*                                                                                                  */
/*--------------------------------------------------------------------------------------------------*/
#include "BNDetectionApp.h"
#include <string.h>

UINT32 BNDetectionApp_DetectNoteResult( void * pBuf, UINT32 u32ByteNum );
UINT32 BNDetectionApp_DetectNoteResultSeek( UINT32 u32Position );

S_BNDETECTIONAPP *g_psCurrentBNDetectionApp = NULL;

//----------------------------------------------------------------------------------------------------
// Init Beat Note Detection application.
//----------------------------------------------------------------------------------------------------
void 
BNDetectionApp_Initiate(
	S_BNDETECTIONAPP *psBNDetectionApp
)
{
	// clear work biffer of BNDetection App
	memset(psBNDetectionApp,0,sizeof(S_BNDETECTIONAPP));
	
	// Set PCMs from ADC as default.
	psBNDetectionApp->sInBufCtrl.pfnFunc = NULL;
	psBNDetectionApp->sInBufCtrl.pu8Param = (void*)psBNDetectionApp;
	
	psBNDetectionApp->bDetectDeltaTime = TRUE;
	BUF_CTRL_SET_INACTIVE(&psBNDetectionApp->sInBufCtrl);
}

//----------------------------------------------------------------------------------------------------
// Start record, but not start detection immediatly.
// It start detection after the keypad pop sound is skipped (in BNDetectionApp_SetInputData()).
//----------------------------------------------------------------------------------------------------
void
BNDetectionApp_StartRec(
	S_BNDETECTIONAPP *psBNDetectionApp,
	PFN_BNDETECT_BEAT_CALLBACK pfnDetectBeatCallback,
	PFN_BNDETECT_NOTE_CALLBACK pfnDetectNoteCallback
)
{
	UINT16 u16Size;
	UINT16 u16SR;
	
	u16SR = BNDETECTIONAPP_IN_SAMPLE_RATE;
	// start beat / note detection
	u16Size = BNDetection_StartPlay(
		psBNDetectionApp->au32WorkBuf,
		BNDETECTIONAPP_WORK_BUF_SIZE,
		u16SR, //BNDETECTIONAPP_IN_SAMPLE_RATE,
		0,
		BNDetectionApp_DetectNoteResult,
		BNDetectionApp_DetectNoteResultSeek);
	
	if (u16Size > BNDETECTIONAPP_WORK_BUF_SIZE)
		while(1);	// work buffer size is not enough
	
	psBNDetectionApp->pfnDetectBeatCallback = pfnDetectBeatCallback;
	psBNDetectionApp->pfnDetectNoteCallback = pfnDetectNoteCallback;
	// initiate input buffer controller
	//if ( (psBNDetectionApp->sInBufCtrl.pfnFunc == NULL) && (psBNDetectionApp->sInBufCtrl.pu8Param != NULL) )
	BNDetectionApp_SetInputFromADC(psBNDetectionApp, BNDETECTIONAPP_IN_SAMPLE_RATE);
}

//----------------------------------------------------------------------------------------------------
// Stop beat note detection. Stop record.
//----------------------------------------------------------------------------------------------------
void
BNDetectionApp_StopRec(
	S_BNDETECTIONAPP *psBNDetectionApp
)
{	
	// stop detection
	BNDetection_StopDetectEx(psBNDetectionApp->au32WorkBuf);
}

//----------------------------------------------------------------------------------------------------
// Process recorded PCM samples for beat note detection. It writes detected result into RAM buffer
// with midi events.
//----------------------------------------------------------------------------------------------------
BOOL 
BNDetectionApp_ProcessRec(
	S_BNDETECTIONAPP *psBNDetectionApp
)
{
	E_BNDETECTION_RESULT eResult;
	
	g_psCurrentBNDetectionApp = psBNDetectionApp;
	// check if frame is fullfilled to be detected
	if (BNDetection_NeedDetectEx(psBNDetectionApp->au32WorkBuf))
	{
		
		// beat / note detection and output midi events to midi buffer
		if (BNDetection_BeatNoteDetectEx(psBNDetectionApp->au32WorkBuf,&eResult) == FALSE)
			eResult = eBNDETECTION_NONE;
		psBNDetectionApp->pfnDetectBeatCallback(eResult);
	}
	
	return TRUE;
}

UINT32 BNDetectionApp_DetectNoteResult( void * pBuf, UINT32 u32ByteNum )
{
	BYTE *pbMidiCmd;
	UINT32 u32DeltaTime; // unit 4ms
	
	if (u32ByteNum <= 4 )
	{
		pbMidiCmd = pBuf;
		if ( g_psCurrentBNDetectionApp->bDetectDeltaTime )
		{
			// The incoming data represents midi delat time and its unit is 4 ms.
			if ( u32ByteNum!= 1 )
			{
				// Mean variable length quantity and the IMF format is DeltaTime%256, 0xf3, DelatTime/256, 0
				u32DeltaTime = ((UINT32)pbMidiCmd[2])*256 + pbMidiCmd[0];
				
			}
			else
				u32DeltaTime = *pbMidiCmd;
			g_psCurrentBNDetectionApp->u32NoteSustainTime = u32DeltaTime*4;
			
			g_psCurrentBNDetectionApp->bDetectDeltaTime = FALSE;
		}
		else
		{
			// The incoming data represents midi command.
			switch(pbMidiCmd[0]&0xf0)
			{
			case 0x80:
				// Means note off
				g_psCurrentBNDetectionApp->pfnDetectNoteCallback(
					pbMidiCmd[1], 
					g_psCurrentBNDetectionApp->u8Volumn,
					g_psCurrentBNDetectionApp->u32NoteSustainTime );
				break;
			case 0x90:
				// Means note on
				g_psCurrentBNDetectionApp->u8Volumn = pbMidiCmd[2];
				break;
			default:
					break;
			}
			g_psCurrentBNDetectionApp->bDetectDeltaTime = TRUE;
		}
	}
	
	return u32ByteNum;
}

UINT32 BNDetectionApp_DetectNoteResultSeek( UINT32 u32Position )
{
	return u32Position;
}

void
BNDetectionApp_SetInputFromADC(
	S_BNDETECTIONAPP *psBNDetectionApp,
	UINT32 u32SampleRate
) 
{
	// set input(adc) buffer control(call-back structure).
	Record_SetInBufCallback(&psBNDetectionApp->sInBufCtrl, BNDetection_SetInputDataEx,psBNDetectionApp->au32WorkBuf);
		
	Record_Add((S_BUF_CTRL*)&(psBNDetectionApp->sInBufCtrl), u32SampleRate);
}

#if (BNDETECTIONAPP_MULTI_INPUT_ENABLE)
BOOL
BNDetectionApp_SetInputFromBuf(
	S_BNDETECTIONAPP *psBNDetectionApp,
	S_BUF_CTRL *psInPCMBuf
)
{
	if ( psInPCMBuf == NULL )
		return FALSE;

	// Set buffer handler "BufCtrl_ReadWithCount()" to handle input buffer "psInPCMBuf"
	Record_SetInBufCallback(&psBNDetectionApp->sInBufCtrl, (PFN_DATA_REQUEST_CALLBACK)BufCtrl_ReadWithCount, (void *)psInPCMBuf);

	return TRUE;
}

#endif // BNDETECTIONAPP_MULTI_INPUT_ENABLE
