#ifndef _CONFIGIMAADPCMAPP_DECODE_H_
#define _CONFIGIMAADPCMAPP_DECODE_H_

#include "ConfigApp.h"
#include "ImaAdpcm.h"
#include "BufCtrl.h"

// -------------------------------------------------------------------------------------------------------------------------------
// Configuration: Output PCM Ring Buffer
// -------------------------------------------------------------------------------------------------------------------------------
#define IMAADPCMAPP_OUT_FRAME_NUM	  		2																// It can be : 2, 4, 6, ....
#define IMAADPCMAPP_OUT_SAMPLES_PER_FRAME	IMAADPCM_SAMPLE_PER_FRAME										// Samples per frame.
#define IMAADPCMAPP_OUT_BUF_SIZE 			(IMAADPCMAPP_OUT_FRAME_NUM*IMAADPCMAPP_OUT_SAMPLES_PER_FRAME)	// Output ring buffer size.
 							
#if ( IMAADPCMAPP_OUT_BUF_SIZE%8 )
	#error "IMAADPCMAPP_OUT_BUF_SIZE must be multiple of '8'."	
#endif	

#define IMAADPCMAPP_CTRL_DUPLICATE_TO_BUF	1
#define IMAADPCMAPP_CTRL_DUPLICATE_TO_FUNC	2
typedef BOOL (*PFN_IMAADPCMAPP_DUPLICATE_FUNC)(UINT16 u16WriteCount, INT16 *pi16Src);

// ImaAdpcm decode application handler
typedef struct
{
	// Work buffer for ImaAdpcm decode library to keep private data during decoding.
	// (IMAADPCM_DECODE_WORK_BUF_SIZE+3)/4 : Force to do 4 byte alignment
	UINT32 au32DecodeWorkBuf[(IMAADPCM_DECODE_WORK_BUF_SIZE+3)/4];
	
	// Pointer of temporary buffer array.
	// Temporary buffer is provided for ImaAdpcm decode library. 
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
		PFN_IMAADPCMAPP_DUPLICATE_FUNC pfnDuplicateFunc;
	};
	
	// The buffer to store decoded PCM and referenced by "sOutBufCtrl".
	INT16 i16OutBuf[IMAADPCMAPP_OUT_BUF_SIZE];

	// The index of read callback and event function to g_asAppCallBack[] array in AppCallback.c.
	// At ImaAdpcm decoder needs data,       it will call the read  callback funciton to get ImaAdpcm data.
	// At ImaAdpcm decoder discovers events, it will call the event callback funciton to handle event.
	UINT8 u8CallbackIndex;
	
	// The audio play channel to play the decoded data.
	UINT8 u8PlaybackChannel;
	
	// Duplicate control flag(To buffer/callback function)
	UINT8 u8CtrlFlag;
	
} S_IMAADPCM_APP_DECODE;

//---------------------------------------------------------------------------------------------------------
//	Description
//		Initiate for ImaAdpcm decode application.
//
//	Parameter
//  	psImaAdpcmAppDecode [in] :
//			Pointer of ImaAdpcm decode application handler.
//
//  	pau8TempBuf [in] :
//ƒÜ		Temporary buffer for NuXXX series decode application.
//ƒÜ		Temporary buffer could be re-used by user after decode per frame.
//
//		u32CallbackIndex [in] :
//			The index of read callback and event function to g_asAppCallBack[] array in AppCallback.c.
//			At ImaAdpcm decoder needs data,       it will call the read  callback funciton to get ImaAdpcm data.
//			At ImaAdpcm decoder discovers events, it will call the event callback funciton to handle event.
//
//	Return Value
//		None
//---------------------------------------------------------------------------------------------------------
void 
ImaAdpcmApp_DecodeInitiate(
	S_IMAADPCM_APP_DECODE *psImaAdpcmAppDecode, 
	UINT8 *pau8TempBuf, 
	UINT32 u32CallbackIndex
	);
	
//---------------------------------------------------------------------------------------------------------
//	Description:                                                                                           
//		Initiate to play the ImaAdpcm file represented by audio file ID.
//		The ImaAdpcm file will be discovered in audio rom file according the inputed audio ID.
//		
//		This function will decode first frame of ImaAdpcm data to output buffer after discovered the ImaAdpcm file.
//
//		Due to this function does not enable APU to play processed PCMs.
//		Must call Playback_StartPlay() to start APU playing if necessary!
//
// 	Argument:
//		psImaAdpcmAppDecode [in] :
//			Pointer of ImaAdpcm decode application handler.
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
//			Or start address of audio chunk is incorrect.
//		TRUE : 
//			Start play audio in ROM file.
//---------------------------------------------------------------------------------------------------------
BOOL 
ImaAdpcmApp_DecodeStartPlayByID(
	S_IMAADPCM_APP_DECODE *psImaAdpcmAppDecode, 
	UINT32 u32AudioID,
	UINT32 u32RomStartAddr,
	UINT8 u8PlaybackChannel
);
	
//---------------------------------------------------------------------------------------------------------
//	Description:                                                                                           
//		Initiate to play the ImaAdpcm file represented by storage address.
//		The ImaAdpcm file will be discovered in storage according to the inputed start address.
//		
//		This function will decode first frame of ImaAdpcm data to output buffer after discovered the ImaAdpcm file.
//
//		Due to this function does not enable APU to play processed PCMs.
//		Must call Playback_StartPlay() to start APU playing if necessary!
//
// 	Argument:
//		psImaAdpcmAppDecode [in] :
//			Pointer of ImaAdpcm decode application handler.
//
//		u32ImaAdpcmStorageStartAddr [in] :
//			Start address to load ImaAdpcm encode data in the storage. 
//			Porgrammer can call	AudioRom_GetAudioChunkInfo(in AudioRom.c) to parse ROM file and get address.
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
ImaAdpcmApp_DecodeStartPlayByAddr(
	S_IMAADPCM_APP_DECODE *psImaAdpcmAppDecode, 
	UINT32 u32ImaAdpcmStorageStartAddr,
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
//		psImaAdpcmAppDecode [in] :
//			Pointer of ImaAdpcm decode application handler.
//
//	Return:
//		None
//---------------------------------------------------------------------------------------------------------
void 
ImaAdpcmApp_DecodeStopPlay(
	S_IMAADPCM_APP_DECODE *psImaAdpcmAppDecode
);

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Decode ImaAdpcm and produce PCMs to output ring buffer.
//		Can check the function return value to know it is running out of audio data or decoding stopped. 
//
// 	Argument:
//		psImaAdpcmAppDecode [in] :
//			Pointer of ImaAdpcm decode application handler.
//
// 	Return:
// 		FALSE : 
//			Running out of audio data or decoding stopped.
//		TRUE :  
//			Decoding is going on.
//---------------------------------------------------------------------------------------------------------
BOOL 
ImaAdpcmApp_DecodeProcess(
	S_IMAADPCM_APP_DECODE *psImaAdpcmAppDecode
	);
	
//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Force inline function.
//		Duplicate output data into assign buffer.
//
// 	Argument:
//		psImaAdpcmAppDecode [in] :
//			Pointer of ImaAdpcm decode application handler.
//
//		psOutBufCtrl [in] :
//			Assign buffer to save output data.
//
// 	Return:
// 		None
//---------------------------------------------------------------------------------------------------------
__STATIC_INLINE void
ImaAdpcmApp_DuplicateOutputToBuf(
	S_IMAADPCM_APP_DECODE *psImaAdpcmAppDecode,
	S_BUF_CTRL *psOutBufCtrl
) 
{
	psImaAdpcmAppDecode->u8CtrlFlag = (psImaAdpcmAppDecode->u8CtrlFlag&IMAADPCMAPP_CTRL_DUPLICATE_TO_FUNC)|IMAADPCMAPP_CTRL_DUPLICATE_TO_BUF;
	psImaAdpcmAppDecode->psDuplicateOutBufCtrl = psOutBufCtrl;
	psOutBufCtrl->u16SampleRate = psImaAdpcmAppDecode->sOutBufCtrl.u16SampleRate;
}

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Force inline function.
//		Duplicate output data into callback function to provide application processing.
//
// 	Argument:
//		psImaAdpcmAppDecode [in] :
//			Pointer of ImaAdpcm decode application handler.
//
//		pfnDuplicateFunc [in] :
//			Assign callback to process output data.
//
// 	Return:
// 		None
//---------------------------------------------------------------------------------------------------------
__STATIC_INLINE void
ImaAdpcmApp_DuplicateOutputToFunc(
	S_IMAADPCM_APP_DECODE *psImaAdpcmAppDecode,
	PFN_IMAADPCMAPP_DUPLICATE_FUNC pfnDuplicateFunc
) 
{
	psImaAdpcmAppDecode->u8CtrlFlag = (psImaAdpcmAppDecode->u8CtrlFlag&IMAADPCMAPP_CTRL_DUPLICATE_TO_BUF)|IMAADPCMAPP_CTRL_DUPLICATE_TO_FUNC;
	psImaAdpcmAppDecode->pfnDuplicateFunc = pfnDuplicateFunc;
}

#endif
