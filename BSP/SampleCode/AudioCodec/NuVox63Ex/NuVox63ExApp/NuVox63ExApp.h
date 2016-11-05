#ifndef _CONFIGNUVOX63EXAPP_H_
#define _CONFIGNUVOX63EXAPP_H_

#include "ConfigApp.h"
#include "NuVox63Ex.h"
#include "BufCtrl.h"

// -------------------------------------------------------------------------------------------------------------------------------
// Configuration: Output PCM Ring Buffer
// -------------------------------------------------------------------------------------------------------------------------------
#define NUVOX63EXAPP_OUT_FRAME_NUM	  		2																// It can be : 2, 4, 6, ....
#define NUVOX63EXAPP_OUT_SAMPLES_PER_FRAME	NUVOX63EX_DECODE_SAMPLE_PER_FRAME								// Samples per frame.
#define NUVOX63EXAPP_OUT_BUF_SIZE 			(NUVOX63EXAPP_OUT_FRAME_NUM*NUVOX63EXAPP_OUT_SAMPLES_PER_FRAME)	// Output ring buffer size.
 							
#if ( NUVOX63EXAPP_OUT_BUF_SIZE%8 )
	#error "NUVOX63EXAPP_OUT_BUF_SIZE must be multiple of '8'."	
#endif		

#define NUVOX63EXAPP_CTRL_DUPLICATE_TO_BUF	1
#define NUVOX63EXAPP_CTRL_DUPLICATE_TO_FUNC	2
typedef BOOL (*PFN_NUVOX63EXAPP_DUPLICATE_FUNC)(UINT16 u16WriteCount, INT16 *pi16Src);

// NuVox63Ex application handler
typedef struct
{
	// Work buffer for NuVox63Ex library to keep private data during decoding.
	// (NUVOX63EX_DECODE_WORK_BUF_SIZE+3)/4 : Force to do 4 byte alignment
	UINT32 au32DecodeWorkBuf[(NUVOX63EX_DECODE_WORK_BUF_SIZE+3)/4];
	
	// Pointer of temporary buffer array.
	// Temporary buffer is provided for NuVox63Ex decode library. 
	UINT8 *pau8TempBuf;

	// The output buffer control provides these operations:
	//	1. stores decoded PCMs
	//	2. the read  index which represents the first decoded PCM data in the ring buffer
	//	3. the write index which represents the first free space in the ring buffer
	//	4. the frame size  which represents the count of decoded PCMs at each time
	S_BUF_CTRL	sOutBufCtrl;
	
	// Duplicate data into buffer/callback function
	union
	{
		S_BUF_CTRL *psDuplicateOutBufCtrl;
		PFN_NUVOX63EXAPP_DUPLICATE_FUNC pfnDuplicateFunc;
	};
	
	// The buffer to store decoded PCM and referenced by "sOutBufCtrl".
	INT16 i16OutBuf[NUVOX63EXAPP_OUT_BUF_SIZE];
	
	// The index of read callback and event function to g_asAppCallBack[] array in AppCallback.c.
	// At NuVox63Ex decoder needs data,       it will call the read  callback funciton to get NuVox63Ex data.
	// At NuVox63Ex decoder discovers events, it will call the event callback funciton to handle event.
	UINT8 u8CallbackIndex;
	
	// The audio play channel to play the decoded data.
	UINT8 u8PlaybackChannel;
	
	// Duplicate control flag(To buffer/callback function)
	UINT8 u8CtrlFlag;
	
} S_NUVOX63EX_APP;

//---------------------------------------------------------------------------------------------------------
//	Description
//		Initiate for NuVox63Ex decode application.
//
//	Parameter
//  	psNuVox63ExApp [in] :
//			Pointer of NuVox63Ex decode application handler.
//
//  	pau8TempBuf [in] :
//ƒÜ		Temporary buffer for NuXXX series decode application.
//ƒÜ		Temporary buffer could be re-used by user after decode per frame.
//
//		u32CallbackIndex [in] :
//			The index of read callback and event function to g_asAppCallBack[] array in AppCallback.c.
//			At NuVox63Ex decoder needs data,       it will call the read  callback funciton to get NuVox63Ex data.
//			At NuVox63Ex decoder discovers events, it will call the event callback funciton to handle event.
//
//	Return Value
//		None
//---------------------------------------------------------------------------------------------------------
void 
NuVox63ExApp_DecodeInitiate(
	S_NUVOX63EX_APP *psNuVox63ExApp, 
	UINT8 *pau8TempBuf, 
	UINT32 u32CallbackIndex
	);
	
//---------------------------------------------------------------------------------------------------------
//	Description:                                                                                           
//		Initiate to play the NuVox63Ex file represented by audio file ID.
//		The NuVox63Ex file will be discovered in audio rom file according the inputed audio ID.
//		
//		This function will decode first frame of NuVox63Ex data to output buffer after discovered the NuVox63Ex file.
//
//		Due to this function does not enable APU to play processed PCMs.
//		Must call Playback_StartPlay() to start APU playing if necessary!
//
// 	Argument:
//		psNuVox63ExApp [in] :
//			Pointer of NuVox63Ex decode application handler.
//
//		u32AudioID [in] :
//			Index of audio file in audio ROM file.  
//
//		u32RomStartAddr [in] :
//			The start address of audio ROM file in sorage.
//
//		u8PlaybackChannel [in] : 
//			Assign the audio play channel to play the decoded data.
//
//	Return:
// 		FALSE : 
//			Codec format in ROM file dis-match or 
//			Start address of audio chunk is incorrect.
//		TRUE : 
//			Start play audio in ROM file.
//---------------------------------------------------------------------------------------------------------
BOOL 
NuVox63ExApp_DecodeStartPlayByID(
	S_NUVOX63EX_APP *psNuVox63ExApp, 
	UINT32 u32AudioID,
	UINT32 u32RomStartAddr,
	UINT8 u8PlaybackChannel
	);
	
//---------------------------------------------------------------------------------------------------------
//	Description:                                                                                           
//		Initiate to play the NuVox63Ex file represented by storage address.
//		The NuVox63Ex file will be discovered in storage according to the inputed start address.
//		
//		This function will decode first frame of NuVox63Ex data to output buffer after discovered the NuVox63Ex file.
//
//		Due to this function does not enable APU to play processed PCMs.
//		Must call Playback_StartPlay() to start APU playing if necessary!
//
// 	Argument:
//		psNuVox63ExApp [in] :
//			Pointer of NuVox63Ex decode application handler.
//
//		u32NuVox63ExStorageStartAddr [in] :
//			Start address to load NuVox63Ex encode data in the storage. 
//			Porgrammer can call	AudioRom_GetAudioChunkInfo(in AudioRom.c) to parse ROM file and get address.
//
//		u8PlaybackChannel [in] : 
//			Assign the audio play channel to play the decoded data.
//
//	Return:
// 		FALSE : 
//			Codec format in ROM file dis-match or 
//			Start address of audio chunk is incorrect.
//		TRUE : 
//			Start play audio in ROM file.
//---------------------------------------------------------------------------------------------------------
BOOL 
NuVox63ExApp_DecodeStartPlayByAddr(
	S_NUVOX63EX_APP *psNuVox63ExApp, 
	UINT32 u32NuVox63ExStorageStartAddr,
	UINT8 u8PlaybackChannel
	);
	
//---------------------------------------------------------------------------------------------------------
//	Description:                                                                                           
//		Stop to decode audio data and remove from audio play channel.
//
//		Due to this function does not close APU to play PCMs. 
//		Must call Playback_StopPlay() to close APU playing if necessary!
//
// 	Argument:
//		psNuVox63ExApp [in] :
//			Pointer of NuVox63Ex decode application handler.
//
//	Return:
//		None
//---------------------------------------------------------------------------------------------------------
void 
NuVox63ExApp_DecodeStopPlay(
	S_NUVOX63EX_APP *psNuVox63ExApp
	);

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Decode NuVox63Ex and produce PCMs to output ring buffer.
//		Can check the function return value to know it is running out of audio data or decoding stopped. 
//
// 	Argument:
//		psNuVox63ExApp [in] :
//			Pointer of NuVox63Ex decode application handler.
//
// 	Return:
// 		FALSE : 
//			Running out of audio data or decoding stopped.
//
//		TRUE :  
//			Decoding is going on.
//---------------------------------------------------------------------------------------------------------
BOOL 
NuVox63ExApp_DecodeProcess(
	S_NUVOX63EX_APP *psNuVox63ExApp
	);
	
//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Force inline function.
//		Duplicate output data into assign buffer.
//
// 	Argument:
//		psNuVox63ExApp [in] :
//			Pointer of NuVox63Ex decode application handler.
//
//		psOutBufCtrl [in] :
//			Assign buffer to save output data.
//
// 	Return:
// 		None
//---------------------------------------------------------------------------------------------------------
__STATIC_INLINE void
NuVox63ExApp_DuplicateOutputToBuf(
	S_NUVOX63EX_APP *psNuVox63ExApp,
	S_BUF_CTRL *psOutBufCtrl
) 
{
	psNuVox63ExApp->u8CtrlFlag = (psNuVox63ExApp->u8CtrlFlag&(~NUVOX63EXAPP_CTRL_DUPLICATE_TO_FUNC))|NUVOX63EXAPP_CTRL_DUPLICATE_TO_BUF;
	psNuVox63ExApp->psDuplicateOutBufCtrl = psOutBufCtrl;
	psOutBufCtrl->u16SampleRate = psNuVox63ExApp->sOutBufCtrl.u16SampleRate;
}

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Force inline function.
//		Duplicate output data into callback function to provide application processing.
//
// 	Argument:
//		psNuVox63ExApp [in] :
//			Pointer of NuVox63Ex decode application handler.
//
//		pfnDuplicateFunc [in] :
//			Assign callback to process output data.
//
// 	Return:
// 		None
//---------------------------------------------------------------------------------------------------------
__STATIC_INLINE void
NuVox63ExApp_DuplicateOutputToFunc(
	S_NUVOX63EX_APP *psNuVox63ExApp,
	PFN_NUVOX63EXAPP_DUPLICATE_FUNC pfnDuplicateFunc
) 
{
	psNuVox63ExApp->u8CtrlFlag = (psNuVox63ExApp->u8CtrlFlag&(~NUVOX63EXAPP_CTRL_DUPLICATE_TO_BUF))|NUVOX63EXAPP_CTRL_DUPLICATE_TO_FUNC;
	psNuVox63ExApp->pfnDuplicateFunc = pfnDuplicateFunc;
}

#endif
