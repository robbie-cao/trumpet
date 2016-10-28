#ifndef _CONFIGMIDIEXAPP_H_
#define _CONFIGMIDIEXAPP_H_

#include "ConfigApp.h"
#include "MIDISynthEx.h"
#include "BufCtrl.h"

// -------------------------------------------------------------------------------------------------------------------------------
// MIDI Resource Related Definitions 
// -------------------------------------------------------------------------------------------------------------------------------
#define MIDI_POLYPHONY_NUM				5			// The polyphonies can be played at the same time
#define MIDI_FIFIO_BUF_SIZE				128			// The buffer size to keep MIDI commands
#define MIDI_SAMPLE_RATE				12000		// The output sampling rate for MIDI decoding
#define MIDI_DECODE_SAMPLE_PER_FRAME	32			// The decoded PCM samples at each calling MIDISynthEx_DecodeProcess()
#define MIDI_CH_VOLUME					5			// Range of channel volume is in 0~127. 0 is mute.

// -------------------------------------------------------------------------------------------------------------------------------
// Configuration: PCM Ring Buffer
// -------------------------------------------------------------------------------------------------------------------------------
#define MIDIEXAPP_OUT_FRAME_NUM	  				2		// it can be : 2, 3, 4, 5, ....
#define MIDIEXAPP_OUT_SAMPLES_PER_FRAME			MIDI_DECODE_SAMPLE_PER_FRAME						
#define MIDIEXAPP_OUT_BUF_SIZE 					(MIDIEXAPP_OUT_FRAME_NUM*MIDIEXAPP_OUT_SAMPLES_PER_FRAME)
 							
#if ( MIDIEXAPP_OUT_BUF_SIZE%8 )
	#error "MIDIEXAPP_OUT_BUF_SIZE must be multiple of '8'."	
#endif	

#define MIDIEXAPP_CTRL_DUPLICATE_TO_BUF	1
#define MIDIEXAPP_CTRL_DUPLICATE_TO_FUNC	2
typedef BOOL (*PFN_MIDIEXAPP_DUPLICATE_FUNC)(UINT16 u16WriteCount, INT16 *pi16Src);

typedef struct
{
	// Work buffer for MidiSynthEx library to keep private data during decoding.
	// (MIDISYNTHEX_DECODE_WORK_BUF_SIZE+3)/4 : Force to do 4 byte alignment
	UINT32 au32DecodeWorkBuf[(MIDISYNTHEX_DECODE_WORK_BUF_SIZE+3)/4];	
	
	// Save polyphony variables.  
	// (MIDISYNTHEX_DECODE_POLY_BUF_SIZE+3)/4 : Force to do 4 byte alignment.
	// The more computing is proportional to polyphony numbers. 
	UINT32 au8TotalPolyphonyBuf[MIDI_POLYPHONY_NUM][(MIDISYNTHEX_DECODE_POLY_BUF_SIZE+3)/4];
	
	// Queue midi events read from ROM file in storage
	// (MIDI_FIFIO_BUF_SIZE+3)/4 : Force to do 4 byte alignment.
	UINT32 au8FifoBuf[(MIDI_FIFIO_BUF_SIZE+3)/4];
	
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
		PFN_MIDIEXAPP_DUPLICATE_FUNC pfnDuplicateFunc;
	};
	
	// Output (ring) buffer for saving decoded data 
	INT16 i16OutBuf[MIDIEXAPP_OUT_BUF_SIZE];

	// The index of read callback and event function to g_asAppCallBack[] array in AppCallback.c.
	// At MidiSynthEx needs data,       it will call the read  callback funciton to get midi commands.
	// At MidiSynthEx discovers events, it will call the event callback funciton to handle event.
	UINT8 u8CallbackIndex;
	
	// The audio play channel to play the decoded data.
	UINT8 u8ChannelID;					
	
	// Duplicate control flag(To buffer/callback function)
	UINT8 u8CtrlFlag;
	
} S_MIDIEX_APP;

//---------------------------------------------------------------------------------------------------------
//	Description
//		This function will configure required setting for MidiSynthEx library:
//			*Buffer resource for keeping interanl variables during synthesizing.
//			*Wave table address. This address places baisc instruments for synthesizing.
//			*Playback midi address, size, sample rate and storage callback function.
//			*Sample numbers per frame.
//			*Midi channel volume.
//
//	Parameter
//  	psMidiExApp[in] :
//			Pointer of MidiSynthEx decode application handler.
//  	u32StartAddr[in] :
//ƒÜ		Start address of playback midi.
//		u32DataSize[in] :
//ƒÜ		Total size of playback midi.
//		u32WaveTableStartAddr[in] :
//			Start address of wave table. It is generated from audio tool and defined in AudioRes_AudioInfo.h
//
//	Return Value
//		None
//---------------------------------------------------------------------------------------------------------
void MidiExApp_Config(S_MIDIEX_APP *psMidiExApp, UINT32 u32StartAddr, UINT32 u32DataSize,	UINT32 u32WaveTableStartAddr);

//---------------------------------------------------------------------------------------------------------
//	Description
//		Initiate buffer controlling for MidiSynthEx decode application
//
//	Parameter
//  	psMidiExApp[in] :
//			Pointer of MidiSynthEx decode application handler.
//  	pau8TempBuf[in]
//			Temporary buffer for NuXXX series decode application.
//			In MidiSynthEx decode application, this buffer is reserved.
//		u32CallbackIndex[in]
//			The index of read callback and event function to g_asAppCallBack[] array in AppCallback.c.
//			At MidiSynthEx library needs MIDI data,  it will call the read data callback funciton to get MIDI data.
//			At MidiSynthEx library needs wave table, it will call the read wave table callback funciton to get MIDI data.
//			At MidiSynthEx library discovers events, it will call the event callback funciton to handle event.
//
//	Return Value
//		None
//---------------------------------------------------------------------------------------------------------
void MidiExApp_DecodeInitiate(S_MIDIEX_APP *psMidiExApp, UINT8 *pau8TempBuf, UINT32 u32CallbackIndex);

//---------------------------------------------------------------------------------------------------------
//	Description:                                                                                           
//		Initiate to play the Midi file represented by audio file ID.
//		The Midi file will be discovered in audio rom file according to inputed audio ID.
//
//		This function will decode first frame of Midi data to output buffer after discovered the Midi file.
//
//		Due to this function does not enable APU to play processed PCMs.
//		Must call Playback_StartPlay() to start APU playing if necessary!
//
//		(This function is suitable for ROM file playback)
//
// 	Argument:
//		psMidiExApp [in] :
//			Pointer of MidiSynthEx decode application handler.
//		u32AudioID [in] :
//			Index of audio file in audio ROM file.  
//		u32RomStartAddr [in] :
//			The start address of audio ROM file in sorage.
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
BOOL MidiExApp_DecodeStartPlayByID(S_MIDIEX_APP *psMidiExApp, UINT32 u32AudioID, UINT32 u32RomStartAddr, UINT8 u8PlaybackChannel);

//---------------------------------------------------------------------------------------------------------
//	Description:                                                                                           
//		Initiate to play the Midi file represented by storage address.
//		The Midi file will be discovered in storage according to the inputed start address.
//
//		This function will decode first frame of Midi data to output buffer after discovered the Midi file.
//
//		Due to this function does not enable APU to play processed PCMs.
//		Must call Playback_StartPlay() to start APU playing if necessary!
//
//		(This function is suitable for bin file playback)
//
// 	Argument:
//		psMidiExApp [in] :
//			Pointer of MidiSynthEx decode application handler.
//		u32MidiStorageStartAddr [in] :
//			Start address to load midi data in the storage. 
//			If playing ROM file, Porgrammer can call AudioRom_GetAudioChunkInfo(in AudioRom.c) 
//			to parse ROM file and get address.
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
BOOL MidiExApp_DecodeStartPlayByAddr(S_MIDIEX_APP *psMidiExApp, UINT32 u32MidiStorageStartAddr, UINT8 u8PlaybackChannel);

//---------------------------------------------------------------------------------------------------------
//	Description:                                                                                           
//		Stop to decode audio data from ROM file for stoping to play audio codec.
//
//		Due to this function does not close APU to play PCMs. 
//		Must call Playback_StopPlay() to close APU playing if necessary!
//
// 	Argument:
//		psMidiExApp [in] :
//			Pointer of MidiSynthEx decode application handler.
//
//	Return:
//		None
//---------------------------------------------------------------------------------------------------------
void MidiExApp_DecodeStopPlay(S_MIDIEX_APP *psMidiExApp);

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Decode midi and produce PCMs to output ring buffer.
//		Can check the function return value to know it is running out of audio data or decoding stopped. 
//
// 	Argument:
//		psMidiExApp [in] :
//			Pointer of MidiSynthEx decode application handler.
//
// 	Return:
// 		FALSE : 
//			Running out of audio data or decoding stopped.
//
//		TRUE :  
//			Decoding is going on.
//---------------------------------------------------------------------------------------------------------
BOOL MidiExApp_DecodeProcess(S_MIDIEX_APP *psMidiExApp);

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Force inline function.
//		Duplicate output data into assign buffer.
//
// 	Argument:
//		psMidiExApp [in] :
//			Pointer of MidiSynthEx decode application handler.
//
//		psOutBufCtrl [in] :
//			Assign buffer to save output data.
//
// 	Return:
// 		None
//---------------------------------------------------------------------------------------------------------
__STATIC_INLINE void
MidiExApp_DuplicateOutputToBuf(
	S_MIDIEX_APP *psMidiExApp,
	S_BUF_CTRL *psOutBufCtrl
) 
{
	psMidiExApp->u8CtrlFlag = (psMidiExApp->u8CtrlFlag&(~MIDIEXAPP_CTRL_DUPLICATE_TO_FUNC))|MIDIEXAPP_CTRL_DUPLICATE_TO_BUF;
	psMidiExApp->psDuplicateOutBufCtrl = psOutBufCtrl;
	psOutBufCtrl->u16SampleRate = psMidiExApp->sOutBufCtrl.u16SampleRate;
}

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Force inline function.
//		Duplicate output data into callback function to provide application processing.
//
// 	Argument:
//		psMidiExApp [in] :
//			Pointer of MidiSynthEx decode application handler.
//
//		pfnDuplicateFunc [in] :
//			Assign callback to process output data.
//
// 	Return:
// 		None
//---------------------------------------------------------------------------------------------------------
__STATIC_INLINE void
MidiExApp_DuplicateOutputToFunc(
	S_MIDIEX_APP *psMidiExApp,
	PFN_MIDIEXAPP_DUPLICATE_FUNC pfnDuplicateFunc
) 
{
	psMidiExApp->u8CtrlFlag = (psMidiExApp->u8CtrlFlag&(~MIDIEXAPP_CTRL_DUPLICATE_TO_BUF))|MIDIEXAPP_CTRL_DUPLICATE_TO_FUNC;
	psMidiExApp->pfnDuplicateFunc = pfnDuplicateFunc;
}

#endif
