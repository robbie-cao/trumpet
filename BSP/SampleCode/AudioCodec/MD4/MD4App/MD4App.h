#ifndef _CONFIGMD4APP_H_
#define _CONFIGMD4APP_H_

#include "ConfigApp.h"
#include "MD4.h"
#include "BufCtrl.h"

// -------------------------------------------------------------------------------------------------------------------------------
// Configuration: Output PCM Ring Buffer
// -------------------------------------------------------------------------------------------------------------------------------
#define MD4APP_OUT_FRAME_NUM	  		2													// It can be : 2, 4, 6, ....
#define MD4APP_OUT_SAMPLES_PER_FRAME	MD4_DECODE_SAMPLE_PER_FRAME							// Decoded PCM samples per frame.
#define MD4APP_OUT_BUF_SIZE 			(MD4APP_OUT_FRAME_NUM*MD4APP_OUT_SAMPLES_PER_FRAME)	// Output ring buffer size.
 							
#if ( MD4APP_OUT_BUF_SIZE%8 )
	#error "MD4APP_OUT_BUF_SIZE must be multiple of '8'."	
#endif	

#define MD4APP_CTRL_DUPLICATE_TO_BUF	1
#define MD4APP_CTRL_DUPLICATE_TO_FUNC	2
typedef BOOL (*PFN_MD4APP_DUPLICATE_FUNC)(UINT16 u16WriteCount, INT16 *pi16Src);

// MD4 application handler
typedef struct
{
	// Work buffer for MD4 library to keep private data during decoding.
	// (MD4_DECODE_WORK_BUF_SIZE+3)/4 : Force to do 4 byte alignment
	UINT32 au32DecodeWorkBuf[(MD4_DECODE_WORK_BUF_SIZE+3)/4];
	
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
		PFN_MD4APP_DUPLICATE_FUNC pfnDuplicateFunc;
	};
	
	// The buffer to store decoded PCM and referenced by "sOutBufCtrl".
	INT16 i16OutBuf[MD4APP_OUT_BUF_SIZE];
	
	// The index of read callback and event function to g_asAppCallBack[] array in AppCallback.c.
	// At MD4 decoder needs data,       it will call the read  callback funciton to get MD4 data.
	// At MD4 decoder discovers events, it will call the event callback funciton to handle event.
	UINT8 u8CallbackIndex;
	
	// The audio play channel to play the decoded data.
	UINT8 u8PlaybackChannel;
	
	// Duplicate control flag(To buffer/callback function)
	UINT8 u8CtrlFlag;

} S_MD4_APP;

//---------------------------------------------------------------------------------------------------------
//	Description
//		Initiate for MD4 decode application.
//
//	Parameter
//  	psMD4App [in] :
//			Pointer of MD4 decode application handler.
//
//  	pau8TempBuf [in] :
//ƒÜ		Temporary buffer for NuXXX series decode application.
//ƒÜ		In MD4 decode application, this buffer is reserved.
//
//		u32CallbackIndex [in] :
//			The index of read callback and event function to g_asAppCallBack[] array in AppCallback.c.
//			At MD4 decoder needs data,       it will call the read  callback funciton to get MD4 data.
//			At MD4 decoder discovers events, it will call the event callback funciton to handle event.
//
//	Return Value
//		None
//---------------------------------------------------------------------------------------------------------
void 
MD4App_DecodeInitiate(
	S_MD4_APP *psMD4App, 
	UINT8 *pau8TempBuf, 
	UINT32 u32CallbackIndex
	);

//---------------------------------------------------------------------------------------------------------
//	Description:                                                                                           
//		Initiate to play the MD4 file represented by audio file ID.
//		The MD4 file will be discovered in audio rom file according the inputed audio ID.
//		
//		This function will decode first frame of MD4 data to output buffer after discovered the MD4 file.
//
//		Due to this function does not enable APU to play processed PCMs.
//		Must call Playback_StartPlay() to start APU playing if necessary!
//
// 	Argument:
//		psMD4App [in] :
//			Pointer of MD4 decode application handler.
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
MD4App_DecodeStartPlayByID(
	S_MD4_APP *psMD4App, 
	UINT32 u32AudioID,
	UINT32 u32RomStartAddr,
	UINT8 u8PlaybackChannel
	);

//---------------------------------------------------------------------------------------------------------
//	Description:                                                                                           
//		Initiate to play the MD4 file represented by storage address.
//		The MD4 file will be discovered in storage according to the inputed start address.
//		
//		This function will decode first frame of MD4 data to output buffer after discovered the MD4 file.
//
//		Due to this function does not enable APU to play processed PCMs.
//		Must call Playback_StartPlay() to start APU playing if necessary!
//
// 	Argument:
//		psMD4App [in] :
//			Pointer of MD4 decode application handler.
//
//		u32MD4StorageStartAddr [in] :
//			Start address to load MD4 encode data in the storage. 
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
MD4App_DecodeStartPlayByAddr(
	S_MD4_APP *psMD4App, 
	UINT32 u32MD4StorageStartAddr,
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
//		psMD4App [in] :
//			Pointer of MD4 decode application handler.
//
//	Return:
//		None
//---------------------------------------------------------------------------------------------------------
void 
MD4App_DecodeStopPlay(
	S_MD4_APP *psMD4App
	);

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Decode MD4 and produce PCMs to output ring buffer.
//		Can check the function return value to know it is running out of audio data or decoding stopped. 
//
// 	Argument:
//		psMD4App [in] :
//			Pointer of MD4 decode application handler.
//
// 	Return:
// 		FALSE : 
//			Running out of audio data or decoding stopped.
//		TRUE :  
//			Decoding is going on.
//---------------------------------------------------------------------------------------------------------
BOOL 
MD4App_DecodeProcess(
	S_MD4_APP *psMD4App
	);
	
//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Force inline function.
//		Duplicate output data into assign buffer.
//
// 	Argument:
//		psMD4App [in] :
//			Pointer of MD4 decode application handler.
//
//		psOutBufCtrl [in] :
//			Assign buffer to save output data.
//
// 	Return:
// 		None
//---------------------------------------------------------------------------------------------------------
__STATIC_INLINE void
MD4App_DuplicateOutputToBuf(
	S_MD4_APP *psMD4App,
	S_BUF_CTRL *psOutBufCtrl
) 
{
	psMD4App->u8CtrlFlag = (psMD4App->u8CtrlFlag&(~MD4APP_CTRL_DUPLICATE_TO_FUNC))|MD4APP_CTRL_DUPLICATE_TO_BUF;
	psMD4App->psDuplicateOutBufCtrl = psOutBufCtrl;
	psOutBufCtrl->u16SampleRate = psMD4App->sOutBufCtrl.u16SampleRate;
}

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Force inline function.
//		Duplicate output data into callback function to provide application processing.
//
// 	Argument:
//		psMD4App [in] :
//			Pointer of MD4 decode application handler.
//
//		pfnDuplicateFunc [in] :
//			Assign callback to process output data.
//
// 	Return:
// 		None
//---------------------------------------------------------------------------------------------------------
__STATIC_INLINE void
MD4App_DuplicateOutputToFunc(
	S_MD4_APP *psMD4App,
	PFN_MD4APP_DUPLICATE_FUNC pfnDuplicateFunc
) 
{
	psMD4App->u8CtrlFlag = (psMD4App->u8CtrlFlag&(~MD4APP_CTRL_DUPLICATE_TO_BUF))|MD4APP_CTRL_DUPLICATE_TO_FUNC;
	psMD4App->pfnDuplicateFunc = pfnDuplicateFunc;
}

#endif
