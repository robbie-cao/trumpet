#ifndef _CONFIGP16APP_H_
#define _CONFIGP16APP_H_

#include "ConfigApp.h"
#include "P16.h"
#include "BufCtrl.h"

// -------------------------------------------------------------------------------------------------------------------------------
// Configuration: Output PCM Ring Buffer
// -------------------------------------------------------------------------------------------------------------------------------
#define P16APP_OUT_FRAME_NUM	  		2													// It can be : 2, 4, 6, ....
#define P16APP_OUT_SAMPLES_PER_FRAME	P16_DECODE_SAMPLE_PER_FRAME							// Decoded PCM samples per frame.
#define P16APP_OUT_BUF_SIZE 			(P16APP_OUT_FRAME_NUM*P16APP_OUT_SAMPLES_PER_FRAME)	// Output ring buffer size.
 							
#if ( P16APP_OUT_BUF_SIZE%8 )
	#error "P16APP_OUT_BUF_SIZE must be multiple of '8'."	
#endif	

#define P16APP_CTRL_DUPLICATE_TO_BUF	1
#define P16APP_CTRL_DUPLICATE_TO_FUNC	2
typedef BOOL (*PFN_P16APP_DUPLICATE_FUNC)(UINT16 u16WriteCount, INT16 *pi16Src);

// P16 application handler
typedef struct
{
	// Work buffer for P16 library to keep private data during decoding.
	// (P16_DECODE_WORK_BUF_SIZE+3)/4 : Force to do 4 byte alignment
	UINT32 au32DecodeWorkBuf[(P16_DECODE_WORK_BUF_SIZE+3)/4];
	
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
		PFN_P16APP_DUPLICATE_FUNC pfnDuplicateFunc;
	};
	
	// The buffer to store decoded PCM and referenced by "sOutBufCtrl".
	INT16 i16OutBuf[P16APP_OUT_BUF_SIZE];
	
	// The index of read callback and event function to g_asAppCallBack[] array in AppCallback.c.
	// At P16 decoder needs data,       it will call the read  callback funciton to get P16 data.
	// At P16 decoder discovers events, it will call the event callback funciton to handle event.
	UINT8 u8CallbackIndex;
	
	// The audio play channel to play the decoded data.
	UINT8 u8PlaybackChannel;
	
	// Duplicate control flag(To buffer/callback function)
	UINT8 u8CtrlFlag;

} S_P16_APP;

//---------------------------------------------------------------------------------------------------------
//	Description
//		Initiate for P16 decode application.
//
//	Parameter
//  	psP16App [in] :
//			Pointer of P16 decode application handler.
//
//  	pau8TempBuf [in] :
//ƒÜ		Temporary buffer for NuXXX series decode application.
//ƒÜ		In P16 decode application, this buffer is reserved.
//
//		u32CallbackIndex [in] :
//			The index of read callback and event function to g_asAppCallBack[] array in AppCallback.c.
//			At P16 decoder needs data,       it will call the read  callback funciton to get P16 data.
//			At P16 decoder discovers events, it will call the event callback funciton to handle event.
//
//	Return Value
//		None
//---------------------------------------------------------------------------------------------------------
void 
P16App_DecodeInitiate(
	S_P16_APP *psP16App, 
	UINT8 *pau8TempBuf, 
	UINT32 u32CallbackIndex
	);

//---------------------------------------------------------------------------------------------------------
//	Description:                                                                                           
//		Initiate to play the P16 file represented by audio file ID.
//		The P16 file will be discovered in audio rom file according the inputed audio ID.
//		
//		This function will decode first frame of P16 data to output buffer after discovered the P16 file.
//
//		Due to this function does not enable APU to play processed PCMs.
//		Must call Playback_StartPlay() to start APU playing if necessary!
//
// 	Argument:
//		psP16App [in] :
//			Pointer of P16 decode application handler.
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
P16App_DecodeStartPlayByID(
	S_P16_APP *psP16App, 
	UINT32 u32AudioID,
	UINT32 u32RomStartAddr,
	UINT8 u8PlaybackChannel
	);

//---------------------------------------------------------------------------------------------------------
//	Description:                                                                                           
//		Initiate to play the P16 file represented by storage address.
//		The P16 file will be discovered in storage according to the inputed start address.
//		
//		This function will decode first frame of P16 data to output buffer after discovered the P16 file.
//
//		Due to this function does not enable APU to play processed PCMs.
//		Must call Playback_StartPlay() to start APU playing if necessary!
//
// 	Argument:
//		psP16App [in] :
//			Pointer of P16 decode application handler.
//
//		u32P16StorageStartAddr [in] :
//			Start address to load P16 encode data in the storage. 
//			Porgrammer can call	AudioRom_GetAudioChunkInfo()(in AudioRom.c) to parse ROM file and get address.
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
P16App_DecodeStartPlayByAddr(
	S_P16_APP *psP16App, 
	UINT32 u32P16StorageStartAddr,
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
//		psP16App [in] :
//			Pointer of P16 decode application handler.
//
//	Return:
//		None
//---------------------------------------------------------------------------------------------------------
void 
P16App_DecodeStopPlay(
	S_P16_APP *psP16App
	);

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Decode P16 and produce PCMs to output ring buffer.
//		Can check the function return value to know it is running out of audio data or decoding stopped. 
//
// 	Argument:
//		psP16App [in] :
//			Pointer of P16 decode application handler.
//
// 	Return:
// 		FALSE : 
//			Running out of audio data or decoding stopped.
//
//		TRUE :  
//			Decoding is going on.
//---------------------------------------------------------------------------------------------------------
BOOL 
P16App_DecodeProcess(
	S_P16_APP *psP16App
	);
	
//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Force inline function.
//		Duplicate output data into assign buffer.
//
// 	Argument:
//		psP16App [in] :
//			Pointer of P16 decode application handler.
//
//		psOutBufCtrl [in] :
//			Assign buffer to save output data.
//
// 	Return:
// 		None
//---------------------------------------------------------------------------------------------------------
__STATIC_INLINE void
P16App_DuplicateOutputToBuf(
	S_P16_APP *psP16App,
	S_BUF_CTRL *psOutBufCtrl
) 
{
	psP16App->u8CtrlFlag = (psP16App->u8CtrlFlag&(~P16APP_CTRL_DUPLICATE_TO_FUNC))|P16APP_CTRL_DUPLICATE_TO_BUF;
	psP16App->psDuplicateOutBufCtrl = psOutBufCtrl;
	psOutBufCtrl->u16SampleRate = psP16App->sOutBufCtrl.u16SampleRate;
}

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Force inline function.
//		Duplicate output data into callback function to provide application processing.
//
// 	Argument:
//		psP16App [in] :
//			Pointer of P16 decode application handler.
//
//		pfnDuplicateFunc [in] :
//			Assign callback to process output data.
//
// 	Return:
// 		None
//---------------------------------------------------------------------------------------------------------
__STATIC_INLINE void
P16App_DuplicateOutputToFunc(
	S_P16_APP *psP16App,
	PFN_P16APP_DUPLICATE_FUNC pfnDuplicateFunc
) 
{
	psP16App->u8CtrlFlag = (psP16App->u8CtrlFlag&(~P16APP_CTRL_DUPLICATE_TO_BUF))|P16APP_CTRL_DUPLICATE_TO_FUNC;
	psP16App->pfnDuplicateFunc = pfnDuplicateFunc;
}

#endif
