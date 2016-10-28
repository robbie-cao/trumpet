#ifndef _CONFIGLP8APP_H_
#define _CONFIGLP8APP_H_

#include "ConfigApp.h"
#include "LP8.h"
#include "BufCtrl.h"

// -------------------------------------------------------------------------------------------------------------------------------
// Configuration: Output PCM Ring Buffer
// -------------------------------------------------------------------------------------------------------------------------------
#define LP8APP_OUT_FRAME_NUM	  		2													// It can be : 2, 4, 6, ....
#define LP8APP_OUT_SAMPLES_PER_FRAME	LP8_DECODE_SAMPLE_PER_FRAME							// Decoded PCM samples per frame.
#define LP8APP_OUT_BUF_SIZE 			(LP8APP_OUT_FRAME_NUM*LP8APP_OUT_SAMPLES_PER_FRAME)	// Output ring buffer size.
 							
#if ( LP8APP_OUT_BUF_SIZE%8 )
	#error "LP8APP_OUT_BUF_SIZE must be multiple of '8'."	
#endif	

#define LP8APP_CTRL_DUPLICATE_TO_BUF	1
#define LP8APP_CTRL_DUPLICATE_TO_FUNC	2
typedef BOOL (*PFN_LP8APP_DUPLICATE_FUNC)(UINT16 u16WriteCount, INT16 *pi16Src);

// LP8 application handler
typedef struct
{
	// Work buffer for LP8 library to keep private data during decoding.
	// (LP8_DECODE_WORK_BUF_SIZE+3)/4 : Force to do 4 byte alignment
	UINT32 au32DecodeWorkBuf[(LP8_DECODE_WORK_BUF_SIZE+3)/4];
	
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
		PFN_LP8APP_DUPLICATE_FUNC pfnDuplicateFunc;
	};
	
	// The buffer to store decoded PCM and referenced by "sOutBufCtrl".
	INT16 i16OutBuf[LP8APP_OUT_BUF_SIZE];
	
	// The index of read callback and event function to g_asAppCallBack[] array in AppCallback.c.
	// At LP8 decoder needs data,       it will call the read  callback funciton to get LP8 data.
	// At LP8 decoder discovers events, it will call the event callback funciton to handle event.
	UINT8 u8CallbackIndex;
	
	// The audio play channel to play the decoded data.
	UINT8 u8PlaybackChannel;
	
	// Duplicate control flag(To buffer/callback function)
	UINT8 u8CtrlFlag;

} S_LP8_APP;

//---------------------------------------------------------------------------------------------------------
//	Description
//		Initiate for LP8 decode application.
//
//	Parameter
//  	psLP8App [in] :
//			Pointer of LP8 decode application handler.
//
//  	pau8TempBuf [in] :
//ƒÜ		Temporary buffer for NuXXX series decode application.
//ƒÜ		In LP8 decode application, this buffer is reserved.
//
//		u32CallbackIndex [in] :
//			The index of read callback and event function to g_asAppCallBack[] array in AppCallback.c.
//			At LP8 decoder needs data,       it will call the read  callback funciton to get LP8 data.
//			At LP8 decoder discovers events, it will call the event callback funciton to handle event.
//
//	Return Value
//		None
//---------------------------------------------------------------------------------------------------------
void 
LP8App_DecodeInitiate(
	S_LP8_APP *psLP8App, 
	UINT8 *pau8TempBuf, 
	UINT32 u32CallbackIndex
	);

//---------------------------------------------------------------------------------------------------------
//	Description:                                                                                           
//		Initiate to play the LP8 file represented by audio file ID.
//		The LP8 file will be discovered in audio rom file according the inputed audio ID.
//		
//		This function will decode first frame of LP8 data to output buffer after discovered the LP8 file.
//
//		Due to this function does not enable APU to play processed PCMs.
//		Must call Playback_StartPlay() to start APU playing if necessary!
//
// 	Argument:
//		psLP8App [in] :
//			Pointer of LP8 decode application handler.
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
//			Codec format in ROM file dis-match. 
//			Or audio ID is greater than maxumum audio numbers.
//		TRUE : 
//			Start play audio in ROM file.
//---------------------------------------------------------------------------------------------------------
BOOL 
LP8App_DecodeStartPlayByID(
	S_LP8_APP *psLP8App, 
	UINT32 u32AudioID,
	UINT32 u32RomStartAddr,
	UINT8 u8PlaybackChannel
	);

//---------------------------------------------------------------------------------------------------------
//	Description:                                                                                           
//		Initiate to play the LP8 file represented by storage address.
//		The LP8 file will be discovered in storage according to the inputed start address.
//		
//		This function will decode first frame of LP8 data to output buffer after discovered the LP8 file.
//
//		Due to this function does not enable APU to play processed PCMs.
//		Must call Playback_StartPlay() to start APU playing if necessary!
//
// 	Argument:
//		psLP8App [in] :
//			Pointer of LP8 decode application handler.
//
//		u32LP8StorageStartAddr [in] :
//			Start address to load LP8 encode data in the storage. 
//			Porgrammer can call	AudioRom_GetAudioChunkInfo()(in AudioRom.c) to parse ROM file and get address.
//
//		u8PlaybackChannel [in] : 
//			Assign the audio play channel to play the decoded data.
//
//	Return:
// 		FALSE : 
//			Codec format in ROM file dis-match.
//			Or start address of audio chunk is incorrect.
//		TRUE : 
//			Start play audio in ROM file.
//---------------------------------------------------------------------------------------------------------
BOOL 
LP8App_DecodeStartPlayByAddr(
	S_LP8_APP *psLP8App, 
	UINT32 u32LP8StorageStartAddr,
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
//		psLP8App [in] :
//			Pointer of LP8 decode application handler.
//
//	Return:
//		None
//---------------------------------------------------------------------------------------------------------
void 
LP8App_DecodeStopPlay(
	S_LP8_APP *psLP8App
	);

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Decode LP8 and produce PCMs to output ring buffer.
//		Can check the function return value to know it is running out of audio data or decoding stopped. 
//
// 	Argument:
//		psLP8App [in] :
//			Pointer of LP8 decode application handler.
//
// 	Return:
// 		FALSE : 
//			Running out of audio data or decoding stopped.
//		TRUE :  
//			Decoding is going on.
//---------------------------------------------------------------------------------------------------------
BOOL 
LP8App_DecodeProcess(
	S_LP8_APP *psLP8App
	);

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Force inline function.
//		Duplicate output data into assign buffer.
//
// 	Argument:
//		psLP8App [in] :
//			Pointer of LP8 decode application handler.
//
//		psOutBufCtrl [in] :
//			Assign buffer to save output data.
//
// 	Return:
// 		None
//---------------------------------------------------------------------------------------------------------
__STATIC_INLINE void
LP8App_DuplicateOutputToBuf(
	S_LP8_APP *psLP8App,
	S_BUF_CTRL *psOutBufCtrl
) 
{
	psLP8App->u8CtrlFlag = (psLP8App->u8CtrlFlag&(~LP8APP_CTRL_DUPLICATE_TO_FUNC))|LP8APP_CTRL_DUPLICATE_TO_BUF;
	psLP8App->psDuplicateOutBufCtrl = psOutBufCtrl;
	psOutBufCtrl->u16SampleRate = psLP8App->sOutBufCtrl.u16SampleRate;
}

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Force inline function.
//		Duplicate output data into callback function to provide application processing.
//
// 	Argument:
//		psLP8App [in] :
//			Pointer of LP8 decode application handler.
//
//		pfnDuplicateFunc [in] :
//			Assign callback to process output data.
//
// 	Return:
// 		None
//---------------------------------------------------------------------------------------------------------
__STATIC_INLINE void
LP8App_DuplicateOutputToFunc(
	S_LP8_APP *psLP8App,
	PFN_LP8APP_DUPLICATE_FUNC pfnDuplicateFunc
) 
{
	psLP8App->u8CtrlFlag = (psLP8App->u8CtrlFlag&(~LP8APP_CTRL_DUPLICATE_TO_BUF))|LP8APP_CTRL_DUPLICATE_TO_FUNC;
	psLP8App->pfnDuplicateFunc = pfnDuplicateFunc;
}

#endif
