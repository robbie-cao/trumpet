#ifndef _CONFIGNUSOUNDEXAPP_H_
#define _CONFIGNUSOUNDEXAPP_H_

#include "ConfigApp.h"
#include "NuSoundEx.h"
#include "BufCtrl.h"

// -------------------------------------------------------------------------------------------------------------------------------
// Configuration: Output PCM Ring Buffer
// -------------------------------------------------------------------------------------------------------------------------------
#define NUSOUNDEXAPP_OUT_FRAME_NUM	  		2																// It can be : 2, 4, 6, ....
#define NUSOUNDEXAPP_OUT_SAMPLES_PER_FRAME	NUSOUNDEX_DECODE_SAMPLE_PER_FRAME								// Samples per frame.
#define NUSOUNDEXAPP_OUT_BUF_SIZE 			(NUSOUNDEXAPP_OUT_FRAME_NUM*NUSOUNDEXAPP_OUT_SAMPLES_PER_FRAME)	// Output ring buffer size.
 							
#if ( NUSOUNDEXAPP_OUT_BUF_SIZE%8 )
	#error "NUSOUNDEXAPP_OUT_BUF_SIZE must be multiple of '8'."	
#endif		

#define NUSOUNDEXAPP_CTRL_DUPLICATE_TO_BUF	1
#define NUSOUNDEXAPP_CTRL_DUPLICATE_TO_FUNC	2
typedef BOOL (*PFN_NUSOUNDEXAPP_DUPLICATE_FUNC)(UINT16 u16WriteCount, INT16 *pi16Src);

// NuSoundEx application handler
typedef struct
{
	// Work buffer for NuSoundEx library to keep private data during decoding.
	// (NUSOUNDEX_DECODE_WORK_BUF_SIZE+3)/4 : Force to do 4 byte alignment
	UINT32 au32DecodeWorkBuf[(NUSOUNDEX_DECODE_WORK_BUF_SIZE+3)/4];
	
	// Pointer of temporary buffer array.
	// Temporary buffer is provided for NuSoundEx decode library. 
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
		PFN_NUSOUNDEXAPP_DUPLICATE_FUNC pfnDuplicateFunc;
	};
	
	// The buffer to store decoded PCM and referenced by "sOutBufCtrl".
	INT16 i16OutBuf[NUSOUNDEXAPP_OUT_BUF_SIZE];
	
	// The index of read callback and event function to g_asAppCallBack[] array in AppCallback.c.
	// At NuSoundEx decoder needs data,       it will call the read  callback funciton to get NuSoundEx data.
	// At NuSoundEx decoder discovers events, it will call the event callback funciton to handle event.
	UINT8 u8CallbackIndex;
	
	// The audio play channel to play the decoded data.
	UINT8 u8PlaybackChannel;
	
	// Duplicate control flag(To buffer/callback function)
	UINT8 u8CtrlFlag;
	
} S_NUSOUNDEX_APP;

//---------------------------------------------------------------------------------------------------------
//	Description
//		Initiate for NuSoundEx decode application.
//
//	Parameter
//  	psNuSoundExApp [in] :
//			Pointer of NuSoundEx decode application handler.
//
//  	pau8TempBuf [in] :
//ƒÜ		Temporary buffer for NuXXX series decode application.
//ƒÜ		Temporary buffer could be re-used by user after decode per frame.
//
//		u32CallbackIndex [in] :
//			The index of read callback and event function to g_asAppCallBack[] array in AppCallback.c.
//			At NuSoundEx decoder needs data,       it will call the read  callback funciton to get NuSoundEx data.
//			At NuSoundEx decoder discovers events, it will call the event callback funciton to handle event.
//
//	Return Value
//		None
//---------------------------------------------------------------------------------------------------------
void 
NuSoundExApp_DecodeInitiate(
	S_NUSOUNDEX_APP *psNuSoundExApp, 
	UINT8 *pau8TempBuf, 
	UINT32 u32CallbackIndex
	);
	
//---------------------------------------------------------------------------------------------------------
//	Description:                                                                                           
//		Initiate to play the NuSoundEx file represented by audio file ID.
//		The NuSoundEx file will be discovered in audio rom file according the inputed audio ID.
//		
//		This function will decode first frame of NuSoundEx data to output buffer after discovered the NuSoundEx file.
//
//		Due to this function does not enable APU to play processed PCMs.
//		Must call Playback_StartPlay() to start APU playing if necessary!
//
// 	Argument:
//		psNuSoundExApp [in] :
//			Pointer of NuSoundEx decode application handler.
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
NuSoundExApp_DecodeStartPlayByID(
	S_NUSOUNDEX_APP *psNuSoundExApp, 
	UINT32 u32AudioID,
	UINT32 u32RomStartAddr,
	UINT8 u8PlaybackChannel
	);
	
//---------------------------------------------------------------------------------------------------------
//	Description:                                                                                           
//		Initiate to play the NuSoundEx file represented by storage address.
//		The NuSoundEx file will be discovered in storage according to the inputed start address.
//		
//		This function will decode first frame of NuSoundEx data to output buffer after discovered the NuSoundEx file.
//
//		Due to this function does not enable APU to play processed PCMs.
//		Must call Playback_StartPlay() to start APU playing if necessary!
//
// 	Argument:
//		psNuSoundExApp [in] :
//			Pointer of NuSoundEx decode application handler.
//
//		u32NuSoundExStorageStartAddr [in] :
//			Start address to load NuSoundEx encode data in the storage. 
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
NuSoundExApp_DecodeStartPlayByAddr(
	S_NUSOUNDEX_APP *psNuSoundExApp, 
	UINT32 u32NuSoundExStorageStartAddr,
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
//		psNuSoundExApp [in] :
//			Pointer of NuSoundEx decode application handler.
//
//	Return:
//		None
//---------------------------------------------------------------------------------------------------------
void 
NuSoundExApp_DecodeStopPlay(
	S_NUSOUNDEX_APP *psNuSoundExApp
	);

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Decode NuSoundEx and produce PCMs to output ring buffer.
//		Can check the function return value to know it is running out of audio data or decoding stopped. 
//
// 	Argument:
//		psNuSoundExApp [in] :
//			Pointer of NuSoundEx decode application handler.
//
// 	Return:
// 		FALSE : 
//			Running out of audio data or decoding stopped.
//
//		TRUE :  
//			Decoding is going on.
//---------------------------------------------------------------------------------------------------------
BOOL 
NuSoundExApp_DecodeProcess(
	S_NUSOUNDEX_APP *psNuSoundExApp
	);
	
//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Force inline function.
//		Duplicate output data into assign buffer.
//
// 	Argument:
//		psNuSoundExApp [in] :
//			Pointer of NuSoundEx decode application handler.
//
//		psOutBufCtrl [in] :
//			Assign buffer to save output data.
//
// 	Return:
// 		None
//---------------------------------------------------------------------------------------------------------
__STATIC_INLINE void
NuSoundExApp_DuplicateOutputToBuf(
	S_NUSOUNDEX_APP *psNuSoundExApp,
	S_BUF_CTRL *psOutBufCtrl
) 
{
	psNuSoundExApp->u8CtrlFlag = (psNuSoundExApp->u8CtrlFlag&(~NUSOUNDEXAPP_CTRL_DUPLICATE_TO_FUNC))|NUSOUNDEXAPP_CTRL_DUPLICATE_TO_BUF;
	psNuSoundExApp->psDuplicateOutBufCtrl = psOutBufCtrl;
	psOutBufCtrl->u16SampleRate = psNuSoundExApp->sOutBufCtrl.u16SampleRate;
}

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Force inline function.
//		Duplicate output data into callback function to provide application processing.
//
// 	Argument:
//		psNuSoundExApp [in] :
//			Pointer of NuSoundEx decode application handler.
//
//		pfnDuplicateFunc [in] :
//			Assign callback to process output data.
//
// 	Return:
// 		None
//---------------------------------------------------------------------------------------------------------
__STATIC_INLINE void
NuSoundExApp_DuplicateOutputToFunc(
	S_NUSOUNDEX_APP *psNuSoundExApp,
	PFN_NUSOUNDEXAPP_DUPLICATE_FUNC pfnDuplicateFunc
) 
{
	psNuSoundExApp->u8CtrlFlag = (psNuSoundExApp->u8CtrlFlag&(~NUSOUNDEXAPP_CTRL_DUPLICATE_TO_BUF))|NUSOUNDEXAPP_CTRL_DUPLICATE_TO_FUNC;
	psNuSoundExApp->pfnDuplicateFunc = pfnDuplicateFunc;
}

#endif
