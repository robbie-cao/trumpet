#ifndef _CONFIGNUONEEXAPP_DECODE_H_
#define _CONFIGNUONEEXAPP_DECODE_H_

#include "ConfigApp.h"
#include "NuOneEx.h"
#include "BufCtrl.h"

// -------------------------------------------------------------------------------------------------------------------------------
// Configuration: Output PCM Ring Buffer
// -------------------------------------------------------------------------------------------------------------------------------
#define NUONEEXAPP_OUT_FRAME_NUM	  		2																// It can be : 2, 4, 6, ....
#define NUONEEXAPP_OUT_SAMPLES_PER_FRAME	NUONEEX_DECODE_SAMPLE_PER_FRAME								// Samples per frame.
#define NUONEEXAPP_OUT_BUF_SIZE 			(NUONEEXAPP_OUT_FRAME_NUM*NUONEEXAPP_OUT_SAMPLES_PER_FRAME)	// Output ring buffer size.

#if ( NUONEEXAPP_OUT_BUF_SIZE%8 )
	#error "NUONEEXAPP_OUT_BUF_SIZE must be multiple of '8'."
#endif

#define NUONEEXAPP_CTRL_DUPLICATE_TO_BUF	1
#define NUONEEXAPP_CTRL_DUPLICATE_TO_FUNC	2
typedef BOOL (*PFN_NUONEEXAPP_DUPLICATE_FUNC)(UINT16 u16WriteCount, INT16 *pi16Src);

// NuOneEx decode application handler
typedef struct
{
	// Work buffer for NuOneEx decode library to keep private data during decoding.
	// (NUONEEX_DECODE_WORK_BUF_SIZE+3)/4 : Force to do 4 byte alignment
	UINT32 au32DecodeWorkBuf[(NUONEEX_DECODE_WORK_BUF_SIZE+3)/4];

	// Pointer of temporary buffer array.
	// Temporary buffer is provided for NuOneEx decode library.
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
		PFN_NUONEEXAPP_DUPLICATE_FUNC pfnDuplicateFunc;
	};

	// The buffer to store decoded PCM and referenced by "sOutBufCtrl".
	INT16 i16OutBuf[NUONEEXAPP_OUT_BUF_SIZE];

	// The index of read callback and event function to g_asAppCallBack[] array in AppCallback.c.
	// At NuOneEx decoder needs data,       it will call the read  callback funciton to get NuOneEx data.
	// At NuOneEx decoder discovers events, it will call the event callback funciton to handle event.
	UINT8 u8CallbackIndex;

	// The audio play channel to play the decoded data.
	UINT8 u8PlaybackChannel;

	// Duplicate control flag(To buffer/callback function)
	UINT8 u8CtrlFlag;

} S_NUONEEX_APP_DECODE;

//---------------------------------------------------------------------------------------------------------
//	Description
//		Initiate for NuOneEx decode application.
//
//	Parameter
//  	psNuOneExAppDecode [in] :
//			Pointer of NuOneEx decode application handler.
//
//  	pau8TempBuf [in] :
//ƒÜ		Temporary buffer for NuXXX series decode application.
//ƒÜ		Temporary buffer could be re-used by user after decode per frame.
//
//		u32CallbackIndex [in] :
//			The index of read callback and event function to g_asAppCallBack[] array in AppCallback.c.
//			At NuOneEx decoder needs data,       it will call the read  callback funciton to get NuOneEx data.
//			At NuOneEx decoder discovers events, it will call the event callback funciton to handle event.
//
//	Return Value
//		None
//---------------------------------------------------------------------------------------------------------
void
NuOneExApp_DecodeInitiate(
	S_NUONEEX_APP_DECODE *psNuOneExAppDecode,
	UINT8 *pau8TempBuf,
	UINT32 u32CallbackIndex
	);

//---------------------------------------------------------------------------------------------------------
//	Description:
//		Initiate to play the NuOneEx file represented by audio file ID.
//		The NuOneEx file will be discovered in audio rom file according the inputed audio ID.
//
//		This function will decode first frame of NuOneEx data to output buffer after discovered the NuOneEx file.
//
//		Due to this function does not enable APU to play processed PCMs.
//		Must call Playback_StartPlay() to start APU playing if necessary!
//
// 	Argument:
//		psNuOneExAppDecode [in] :
//			Pointer of NuOneEx decode application handler.
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
NuOneExApp_DecodeStartPlayByID(
	S_NUONEEX_APP_DECODE *psNuOneExAppDecode,
	UINT32 u32AudioID,
	UINT32 u32RomStartAddr,
	UINT8 u8PlaybackChannel
);

//---------------------------------------------------------------------------------------------------------
//	Description:
//		Initiate to play the NuOneEx file represented by storage address.
//		The NuOneEx file will be discovered in storage according to the inputed start address.
//
//		This function will decode first frame of NuOneEx data to output buffer after discovered the NuOneEx file.
//
//		Due to this function does not enable APU to play processed PCMs.
//		Must call Playback_StartPlay() to start APU playing if necessary!
//
// 	Argument:
//		psNuOneExAppDecode [in] :
//			Pointer of NuOneEx decode application handler.
//
//		u32NuOneExStorageStartAddr [in] :
//			Start address to load NuOneEx encode data in the storage.
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
NuOneExApp_DecodeStartPlayByAddr(
	S_NUONEEX_APP_DECODE *psNuOneExAppDecode,
	UINT32 u32NuOneExStorageStartAddr,
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
//		psNuOneExAppDecode [in] :
//			Pointer of NuOneEx decode application handler.
//
//	Return:
//		None
//---------------------------------------------------------------------------------------------------------
void
NuOneExApp_DecodeStopPlay(
	S_NUONEEX_APP_DECODE *psNuOneExAppDecode
);

//---------------------------------------------------------------------------------------------------------
// 	Description:
//		Decode NuOneEx and produce PCMs to output ring buffer.
//		Can check the function return value to know it is running out of audio data or decoding stopped.
//
// 	Argument:
//		psNuOneExAppDecode [in] :
//			Pointer of NuOneEx decode application handler.
//
// 	Return:
// 		FALSE :
//			Running out of audio data or decoding stopped.
//		TRUE :
//			Decoding is going on.
//---------------------------------------------------------------------------------------------------------
BOOL
NuOneExApp_DecodeProcess(
	S_NUONEEX_APP_DECODE *psNuOneExAppDecode
	);

//---------------------------------------------------------------------------------------------------------
// 	Description:
//		Force inline function.
//		Duplicate output data into assign buffer.
//
// 	Argument:
//		psNuOneExAppDecode [in] :
//			Pointer of NuOneEx decode application handler.
//
//		psOutBufCtrl [in] :
//			Assign buffer to save output data.
//
// 	Return:
// 		None
//---------------------------------------------------------------------------------------------------------
__STATIC_INLINE void
NuOneExApp_DuplicateOutputToBuf(
	S_NUONEEX_APP_DECODE *psNuOneExAppDecode,
	S_BUF_CTRL *psOutBufCtrl
)
{
	psNuOneExAppDecode->u8CtrlFlag = (psNuOneExAppDecode->u8CtrlFlag&(~NUONEEXAPP_CTRL_DUPLICATE_TO_FUNC))|NUONEEXAPP_CTRL_DUPLICATE_TO_BUF;
	psNuOneExAppDecode->psDuplicateOutBufCtrl = psOutBufCtrl;
	psOutBufCtrl->u16SampleRate = psNuOneExAppDecode->sOutBufCtrl.u16SampleRate;
}

//---------------------------------------------------------------------------------------------------------
// 	Description:
//		Force inline function.
//		Duplicate output data into callback function to provide application processing.
//
// 	Argument:
//		psNuOneExAppDecode [in] :
//			Pointer of NuOneEx decode application handler.
//
//		pfnDuplicateFunc [in] :
//			Assign callback to process output data.
//
// 	Return:
// 		None
//---------------------------------------------------------------------------------------------------------
__STATIC_INLINE void
NuOneExApp_DuplicateOutputToFunc(
	S_NUONEEX_APP_DECODE *psNuOneExAppDecode,
	PFN_NUONEEXAPP_DUPLICATE_FUNC pfnDuplicateFunc
)
{
	psNuOneExAppDecode->u8CtrlFlag = (psNuOneExAppDecode->u8CtrlFlag&(~NUONEEXAPP_CTRL_DUPLICATE_TO_BUF))|NUONEEXAPP_CTRL_DUPLICATE_TO_FUNC;
	psNuOneExAppDecode->pfnDuplicateFunc = pfnDuplicateFunc;
}

#endif

/* vim: set ts=4 sw=4 tw=0 noexpandtab : */
