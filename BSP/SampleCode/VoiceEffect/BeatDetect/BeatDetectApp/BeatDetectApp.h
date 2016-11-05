/*----------------------------------------------------------------------------------------------------------*/
/*                                                                                                         	*/
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              	*/
/*                                                                                                      	*/
/*----------------------------------------------------------------------------------------------------------*/
#ifndef __BEATDETECT_H__
#define __BEATDETECT_H__

#include "Platform.h"
#include "PlaybackRecord.h"
#include "BNDetection.h"

// -------------------------------------------------------------------------------------------------------------------------------
// Beat detection app definitions   
// -------------------------------------------------------------------------------------------------------------------------------
// The max sample rate of inputed PCM. This value can be changed.
//		This value will affect the working buffer size.
//		It means the value of "BEATDETECTAPP_WORK_BUF_SIZE" should be changed according to the value of "BEATDETECTAPP_MAX_INPUT_SAMPLE_RATE"
#define BEATDETECTAPP_MAX_INPUT_SAMPLE_RATE	(8000)
// The working buffer size of beat detection library.
//		This value should be changed according to the value of "BEATDETECTAPP_MAX_INPUT_SAMPLE_RATE"
#define BEATDETECTAPP_WORK_BUF_SIZE			0x284

// Enable more input path of raw PCMs which are used to detect beat or percussion.
// 1: Have raw PCMs inputed not only from ADC and from manual input to have larger RAM and ROM.
// 0: Have raw PCMs inputed     only from ADC and from manual input to have small  RAM and ROM.
#define BEATDETECTAPP_MULTI_INPUT_ENABLE	1

// The definition of callback function which will be called at a beat or a percussion detected.
typedef void (*PFN_BEATDETECT_ENGRY_CALLBACK)(INT16 i16Engry, E_BNDETECTION_RESULT eDetectResult);

// -------------------------------------------------------------------------------------------------------------------------------
// Structure of application.  
// -------------------------------------------------------------------------------------------------------------------------------
// Application data structure
typedef struct
{
	// Work buffer for beat detect library to keep private data during processing and must 4 byte alignment
	// (BEATDETECTAPP_WORK_BUF_SIZE+3)/4 : Force to do 4 byte alignment
	UINT32 au32WorkBuf[(BEATDETECTAPP_WORK_BUF_SIZE+3)/4]; 
	
	// Raw PCMs can be inputed from:
	// 1. from ADC ISR automatically
	//    Call PBeatDetectApp_SetInputFromADC() to get input from ADC before calling BeatDetectApp_StartPlay().
	// 2. by manual calling BeatDetectApp_Input().
	//    Call BeatDetectApp_SetInputManual() before calling BeatDetectApp_StartPlay().
	//    And send PCMs by calling BeatDetectApp_Input() after calling BeatDetectApp_StartPlay().
	// 3. from another PCM buffer control(S_BUF_CTRL*) automatically
	//    Call BeatDetectApp_SetInputFromBuf() to get input from another PCM buffer before calling BeatDetectApp_StartPlay().
	// 4. from a external function (PFN_DATA_REQUEST_CALLBACK) automatically
	//    Call BeatDetectApp_SetInputFromFunc() to get input from another PCM buffer before calling BeatDetectApp_StartPlay().
	S_BUF_CTRL_CALLBACK 			sInBufCtrl;
	
	// The callback function which will be called at a beat or a percussion detected.
	PFN_BEATDETECT_ENGRY_CALLBACK 	pfnEngryCallback;
	
}S_BEATDETECT_APP;

//---------------------------------------------------------------------------------------------------------
// Description
//		Initiate for beat detection application.
//
// Parameter
//		psBeatDetectApp [in] :
//			Pointer of beat detection application handler.
//
// Return Value
//		None
//---------------------------------------------------------------------------------------------------------
void 
BeatDetectApp_Initiate( 
	S_BEATDETECT_APP *psBeatDetectApp
);

//---------------------------------------------------------------------------------------------------------
// Description
//		Start record and detect beat .
//
//		Due to this function does not enable ADC to record PCMs.
//		Must call Record_StartRec() to start ADC recording if necessary!
//
// Parameter
//		psBeatDetectApp [in] :
//			Pointer of beat detection application handler.
//		eDetectMode [in] :
//			Select detect mode. There are two modes for selection.
//				E_BNDETECTION_BEAT_SOUND           Beat detection will emphasize sound energy.
//				E_BNDETECTION_BEAT_PERCUSSION      Beat detection will emphasize percussion energy.
//		pfnEngryCallback [in] :
//			The callback function which will be called at a beat or a percussion detected.
//			And the detected energe will be an argument for reference!
//
// Return Value
//		None
//---------------------------------------------------------------------------------------------------------
void 
BeatDetectApp_StartRec( 
	S_BEATDETECT_APP *psBeatDetectApp,
	E_BNDETECTION_BEAT_DETECT_MODE 	eDetectMode,
	PFN_BEATDETECT_ENGRY_CALLBACK 	pfnEngryCallback	
);

//---------------------------------------------------------------------------------------------------------
// Description
//		Stop record and detect beat.
//
//		Due to this function does not close ADC to record PCMs.
//		Must call Record_StopRec() to close ADC recording if necessary!
//
// Parameter
//		psBeatDetectApp [in] :
//			Pointer of beat detection application handler.
//
// Return Value
//		None
//---------------------------------------------------------------------------------------------------------
void 
BeatDetectApp_StopRec( 
	S_BEATDETECT_APP *psBeatDetectApp
);

//---------------------------------------------------------------------------------------------------------
// Description
//		Process recorded PCM samples to detect beats.
//
// Parameter
//		psBeatDetectApp [in] :
//			Pointer of beat detection application handler.
//
// Return Value
//		FALSE : 
//			Detecting is stopped.
//		TRUE : 
//			Detecting is going on.
//---------------------------------------------------------------------------------------------------------
BOOL
BeatDetectApp_ProcessRec( 
	S_BEATDETECT_APP *psBeatDetectApp
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
//		psBeatDetectApp [in] :
//			Pointer of beat detection application handler.
//		u32SampleRate [in] :
//			The input sample rate.
//	
// Return Value
//		None
//----------------------------------------------------------------------------------------------------
void
BeatDetectApp_SetInputFromADC(
	S_BEATDETECT_APP *psBeatDetectApp,
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
//		psBeatDetectApp [in] :
//			Pointer of beat detection application handler.
//		u32SampleRate [in] :
//			The input sample rate.	
//	
// Return Value
//		None
//----------------------------------------------------------------------------------------------------
__STATIC_INLINE
void
BeatDetectApp_SetInputManual(
	S_BEATDETECT_APP *psBeatDetectApp,
	UINT32 u32SampleRate
)
{
	psBeatDetectApp->sInBufCtrl.pfnFunc = NULL;
	psBeatDetectApp->sInBufCtrl.pu8Param = NULL;
}

//----------------------------------------------------------------------------------------------------
// Description
//		Input PCM data directly to beat detect app. 
//
//		This function works different with BeatDetectApp_SetInputFromBuf() and BeatDetectApp_SetInputFromFunc().
//		Must call this function to input PCM data continuely.	
//	
// Parameter
//		psBeatDetectApp [in] :
//			Pointer of beat detection application handler.
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
BeatDetectApp_Input(
	S_BEATDETECT_APP *psBeatDetectApp,
	INT16 i16PcmCount,
	INT16 ai16Pcm[]
)
{
	return BNDetection_SetInputDataEx(psBeatDetectApp->au32WorkBuf, i16PcmCount, ai16Pcm);
}

#if (BEATDETECTAPP_MULTI_INPUT_ENABLE)
//----------------------------------------------------------------------------------------------------
// Description
//		Set the beat detect app to gets PCMs from buffer automatically.
//
//		Call this function to get input from buffer before calling BeatDetectApp_StartPlay().
//		Then the beat detect app will gets PCMs from buffer automatically.
//
// Parameter
//		psBeatDetectApp [in] :
//			Pointer of beat detection application handler.
//		psInPCMBuf [in] :	
//			the buffer that stores the PCMs and the pitch change app can read PCMs from it.
// Return Value
//		FALSE :
//			Set input buffer failed.
//		TRUE :
//			Set input buffer successful.
//----------------------------------------------------------------------------------------------------
BOOL
BeatDetectApp_SetInputFromBuf(
	S_BEATDETECT_APP *psBeatDetectApp,
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
//		psBeatDetectApp [in] :
//			Pointer of beat detection application handler.
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
BeatDetectApp_SetInputFromFunc(
	S_BEATDETECT_APP *psBeatDetectApp,
	PFN_DATA_REQUEST_CALLBACK pfnFunc,
	void* pParamOfFunc
)
{
	Record_SetInBufCallback(&psBeatDetectApp->sInBufCtrl, pfnFunc, pParamOfFunc);
}
#endif

#endif
