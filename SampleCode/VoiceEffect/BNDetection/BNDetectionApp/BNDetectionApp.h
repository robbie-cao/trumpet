/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#ifndef __BNDETECTIONAPP_H__
#define __BNDETECTIONAPP_H__	 

#include "BNDetection.h"
#include "PlaybackRecord.h"

// -------------------------------------------------------------------------------------------------------------------------------
// Beat and Note detection app definitions    
// -------------------------------------------------------------------------------------------------------------------------------
#define BNDETECTIONAPP_IN_SAMPLE_RATE			(12000)	// Can't be changed because 12K sample rate is best for beat and note detection 

// -------------------------------------------------------------------------------------------------------------------------------
// BNDetection definitions   
// -------------------------------------------------------------------------------------------------------------------------------																				
#define BNDETECTIONAPP_WORK_BUF_SIZE			(0x77c)	// Work buffer size for beat and note detection library and no need changed

// Enable more input path of raw PCMs which are used to detect beat or percussion.
// 1: Have raw PCMs inputed not only from ADC and from manual input to have larger RAM and ROM.
// 0: Have raw PCMs inputed     only from ADC and from manual input to have small  RAM and ROM.
#define BNDETECTIONAPP_MULTI_INPUT_ENABLE	1

//----------------------------------------------------------------------------------------------------
// Description
//		The function will be called at a beat is detected.
// Parameter
//		eDetectResult [in] :
//			the result to describe the detected beat
// Return Value
//		None
//----------------------------------------------------------------------------------------------------
typedef void (*PFN_BNDETECT_BEAT_CALLBACK)(E_BNDETECTION_RESULT eDetectResult);

//----------------------------------------------------------------------------------------------------
// Description
//		The function will be called at a note is detected.
// Parameter
//		u8Note [in] :
//			the detected note.
//		u8Volumn [in] :
//			the volumn of the detected note
//		u32NoteSustainTime [in] :
//			the period of time during which the detected note remains before it becomes inaudible.
//			the time unit is min second.
// Return Value
//		None
//----------------------------------------------------------------------------------------------------
typedef void (*PFN_BNDETECT_NOTE_CALLBACK)(UINT8 u8Note, UINT8 u8Volumn, UINT32 u32NoteSustainTime);

typedef struct
{
	// BNDetection work buffer for lib.(force to do 4 byte alignment)
	UINT32	au32WorkBuf[(BNDETECTIONAPP_WORK_BUF_SIZE+3)/4];
	
	PFN_BNDETECT_BEAT_CALLBACK pfnDetectBeatCallback;
	PFN_BNDETECT_NOTE_CALLBACK pfnDetectNoteCallback;
	S_BUF_CTRL_CALLBACK sInBufCtrl;									// record and callback control.	
	
	UINT32 u32NoteSustainTime;
	UINT8 u8Volumn;
	BOOL bDetectDeltaTime;
}S_BNDETECTIONAPP;


//----------------------------------------------------------------------------------------------------
// Description
//		Initiate beat note detection application.
// Parameter
//		psBNDetectionApp [in] :
//			Pointer of beat note detection application handler.
// Return Value
//		None
//----------------------------------------------------------------------------------------------------
void 
BNDetectionApp_Initiate(
	S_BNDETECTIONAPP *psBNDetectionApp
);

//----------------------------------------------------------------------------------------------------
// Description
//		Start record and detect beat and note but not start detection immediatly.
//		It starts detecting after the keypad pop sound is skipped
//
//		Due to this function does not enable ADC to record PCMs.
//		Must call Record_StartRec() to start ADC recording if necessary!
//
// Parameter
//		psBNDetectionApp [in] :
//			Pointer of beat note detection application handler.
//
// Return Value
//		None
//----------------------------------------------------------------------------------------------------
void
BNDetectionApp_StartRec(
	S_BNDETECTIONAPP *psBNDetectionApp,
	PFN_BNDETECT_BEAT_CALLBACK pfnDetectBeatCallback,
	PFN_BNDETECT_NOTE_CALLBACK pfnDetectNoteCallback
);

//----------------------------------------------------------------------------------------------------
// Description
//		Stop record and detect beat and note.
//
//		Due to this function does not close ADC to record PCMs.
//		Must call Record_StopRec() to close ADC recording if necessary!
// Parameter
//		psBNDetectionApp [in] :
//			Pointer of beat note detection application handler.
//
// Return Value
//		None
//----------------------------------------------------------------------------------------------------
void
BNDetectionApp_StopRec(
	S_BNDETECTIONAPP *psBNDetectionApp
);

//----------------------------------------------------------------------------------------------------
// Description
//		Process recorded PCM samples for beat note detection.
//		At detecting a note, it will write midi event into into RAM buffer.
//		The written midi event will be played in BNDetectionApp_ProcessRec().
//
// Parameter
//		psBNDetectionApp [in] :
///			Pointer of beat note detection application handler.
//
// Return Value
//		FALSE : 
//			Detecting is stopped.
//		TRUE : 
//			Detecting is going on.
//----------------------------------------------------------------------------------------------------
BOOL 
BNDetectionApp_ProcessRec(
	S_BNDETECTIONAPP *psBNDetectionApp
);

UINT8 
BNDetectionApp_SetInputData(
	void *pParam,
	INT16 i16DataBufCount,
	INT16 ai16DataBuf[]
);
	
//----------------------------------------------------------------------------------------------------
// Description
//		Set the beat detect app to gets PCMs from ADC automatically.
//
//		Call this function before calling BeatDetectApp_StartPlay().
//		Then the beat detect pp will gets PCMs from ADC automatically.
//
//		The default input of the pitch change App is from ADC. No need to call this funciton again
//		if the input is from ADC!
//
// Parameter
//		psBNDetectionApp [in] :
//			Pointer of beat note detection application handler.
//		u32SampleRate [in] :
//			The input sample rate.
//	
// Return Value
//		None
//----------------------------------------------------------------------------------------------------
void
BNDetectionApp_SetInputFromADC(
	S_BNDETECTIONAPP *psBNDetectionApp,
	UINT32 u32SampleRate
);

//----------------------------------------------------------------------------------------------------
// Description
//		Set the beat detect app to gets PCMs by calling BeatDetectApp_Input().
//
//		Call this function before calling BeatDetectApp_StartPlay() then the beat detect app will not gets PCMs automatically.
//		And must call "BeatDetectApp_Input()" to input PCMs to the pitch change App.
//
// Parameter
//		psBNDetectionApp [in] :
//			Pointer of beat note detection application handler.
//		u32SampleRate [in] :
//			The input sample rate.	
//	
// Return Value
//		None
//----------------------------------------------------------------------------------------------------
__STATIC_INLINE
void
BNDetectionApp_SetInputManual(
	S_BNDETECTIONAPP *psBNDetectionApp,
	UINT32 u32SampleRate
)
{
	psBNDetectionApp->sInBufCtrl.pfnFunc = NULL;
	psBNDetectionApp->sInBufCtrl.pu8Param = NULL;
}

//----------------------------------------------------------------------------------------------------
// Description
//		Input PCM data directly to beat detect app. 
//
//		This function works different with BeatDetectApp_SetInputFromBuf() and BeatDetectApp_SetInputFromFunc().
//		Must call this function to input PCM data continuely.	
//	
// Parameter
//		psBNDetectionApp [in] :
//			Pointer of beat note detection application handler.
//		i16PcmCount [in] :
//			The count of PCM stored in ai16Pcm arrary.	
//		ai16Pcm [in] :
//			The PCMs which should be processed by the beat detect app.	
//	
// Return Value
//		the count of processed PCMs
//----------------------------------------------------------------------------------------------------
__STATIC_INLINE
UINT8
BNDetectionApp_Input(
	S_BNDETECTIONAPP *psBNDetectionApp,
	INT16 i16PcmCount,
	INT16 ai16Pcm[]
)
{
	return BNDetectionApp_SetInputData(psBNDetectionApp->au32WorkBuf, i16PcmCount, ai16Pcm);
}

#if (BNDETECTIONAPP_MULTI_INPUT_ENABLE)
//----------------------------------------------------------------------------------------------------
// Description
//		Set the beat detect app to gets PCMs from buffer automatically.
//
//		Call this function to get input from buffer before calling BeatDetectApp_StartPlay().
//		Then the beat detect app will gets PCMs from buffer automatically.
//
// Parameter
//		psBNDetectionApp [in] :
//			Pointer of beat note detection application handler.
//		psInPCMBuf [in] :	
//			the buffer that stores the PCMs and the pitch change app can read PCMs from it.
// Return Value
//		FALSE :
//			Set input buffer failed.
//		TRUE :
//			Set input buffer successful.
//----------------------------------------------------------------------------------------------------
BOOL
BNDetectionApp_SetInputFromBuf(
	S_BNDETECTIONAPP *psBNDetectionApp,
	S_BUF_CTRL *psInPCMBuf
);

//----------------------------------------------------------------------------------------------------
// Description
//		Set the beat detect app to gets PCMs from external function automatically.
//
//		Call this function before calling BeatDetectApp_StartPlay().
//		Then the beat detect app will gets PCMs from external function automatically.
//	
// Parameter
//		psBNDetectionApp [in] :
//			Pointer of beat note detection application handler.
//		pfnFunc [in] :
//			The function to get PCMs.	
//		pParamOfFunc [in] :
//			The parameter should be passed to pfnFunc.
//	
// Return Value
//		none
//----------------------------------------------------------------------------------------------------
__STATIC_INLINE
void 
BNDetectionApp_SetInputFromFunc(
	S_BNDETECTIONAPP *psBNDetectionApp,
	PFN_DATA_REQUEST_CALLBACK pfnFunc,
	void* pParamOfFunc
)
{
	Record_SetInBufCallback(&psBNDetectionApp->sInBufCtrl, pfnFunc, pParamOfFunc);
}
#endif


#endif //__BNDETECTIONAPP_H__
