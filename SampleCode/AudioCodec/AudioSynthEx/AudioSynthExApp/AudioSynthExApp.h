#ifndef _CONFIGAUDIOSYNTEXHAPP_H_
#define _CONFIGAUDIOSYNTEXHAPP_H_

#include "ConfigApp.h"

#include "AudioCommon.h"
#include "NuSoundEx.h"
#ifndef __AU9110__
#include "NuVox53Ex.h"
#include "NuVox63Ex.h"
#include "NuLiteEx.h"
#endif
#include "NuOneEx.h"
#include "MD4.h"
#include "LP8.h"
#include "P16.h"
#include "MidiSynthEx.h"
#include "ImaAdpcm.h"
#include "PlaybackRecord.h"

// -------------------------------------------------------------------------------------------------------------------------------
// Codec enable or disable for saving resource; 1:enable  0:disable 
// -------------------------------------------------------------------------------------------------------------------------------
#define P16_ENABLE	 			(1)		
#define LP8_ENABLE	 			(1)	
#define MD4_ENABLE	 			(1)	
#define MIDI_ENABLE	 			(1)	
#define NUONE_ENABLE	 		(0)	
#define NUSOUND_ENABLE	 		(1)	
#define NULITE_ENABLE	 		(0)
#define NUVOX53_ENABLE	 		(0)	
#define NUVOX63_ENABLE	 		(0)	
#define IMAADPCM_ENABLE			(1)

// -------------------------------------------------------------------------------------------------------------------------------
// Configurations: PCM Ring Buffer 
// -------------------------------------------------------------------------------------------------------------------------------
#define AUDIOSYNTHEXAPP_OUT_FRAME_NUM	  			4		// it can be : 2, 3, 4, 5, ....
#define AUDIOSYNTHEXAPP_OUT_SAMPLES_PER_FRAME		(32)								
#define AUDIOSYNTHEXAPP_OUT_BUF_SIZE 				(AUDIOSYNTHEXAPP_OUT_FRAME_NUM*AUDIOSYNTHEXAPP_OUT_SAMPLES_PER_FRAME)

// -------------------------------------------------------------------------------------------------------------------------------
// Configurations: Duplicate Data Control
// -------------------------------------------------------------------------------------------------------------------------------
#define AUDIOSYNTHEXAPP_CTRL_DUPLICATE_TO_BUF	1
#define AUDIOSYNTHEXAPP_CTRL_DUPLICATE_TO_FUNC	2
typedef BOOL (*PFN_AUDIOSYNTHEXAPP_DUPLICATE_FUNC)(UINT16 u16WriteCount, INT16 *pi16Src);

// -------------------------------------------------------------------------------------------------------------------------------
// Configurations: MIDI Resource
// -------------------------------------------------------------------------------------------------------------------------------
#define AUDIOSYNTHEXAPP_MIDI_POLYPHONY_NUM				7			// The polyphonies can be played at the same time
#define AUDIOSYNTHEXAPP_MIDI_FIFIO_BUF_SIZE				256			// The buffer size to keep MIDI commands
#define AUDIOSYNTHEXAPP_MIDI_SAMPLE_RATE				12000		// The output sampling rate for MIDI decoding
#define AUDIOSYNTHEXAPP_MIDI_DECODE_SAMPLE_PER_FRAME	64			// The decoded PCM samples at each calling MIDISynthEx_DecodeProcess()
#define AUDIOSYNTHEXAPP_MIDI_CH_VOLUME					5			// Range of channel volume is in 0~127. 0 is mute.

#ifdef	__cplusplus
extern "C"
{
#endif

// -------------------------------------------------------------------------------------------------------------------------------
// Definition: Decoder functions
// -------------------------------------------------------------------------------------------------------------------------------
typedef struct sAudioSynthExAppDecoder
{
	PFN_AUDIO_DECODEINITIATE	pfnDecodeInitiate;
	PFN_AUDIO_DECODEPROCESS		pfnDecodeProcess;
	PFN_AUDIO_DECODEISEND		pfnDecodeIsEnd;
	UINT16 						u16FormatID;
}S_AUDIOSYNTHEX_APP_DECODER;

// The following struct defined all the buffers needed for MIDI decodding!
typedef struct
{
	// Work buffer for MidiSynthEx library to keep private data during decoding.
	UINT32 u32WorkBuf[(MIDISYNTHEX_DECODE_WORK_BUF_SIZE+3)/4];
	// This buffer for keeping required variables of polyphony channel during synthesizing
	UINT8 au8TotalPolyphonyBuf[AUDIOSYNTHEXAPP_MIDI_POLYPHONY_NUM][MIDISYNTHEX_DECODE_POLY_BUF_SIZE];
	// FIFO buffer for keeping midi commands read for storage.
	UINT8 au8FifoBuf[AUDIOSYNTHEXAPP_MIDI_FIFIO_BUF_SIZE];
} S_MIDI_TOTAL_WORK_BUF;

#if (P16_ENABLE)
	#define AUDIOSYNTHEXAPP_P16_DECODER				{P16_DecodeInitiate, P16_DecodeProcess, P16_DecodeIsEnd, AUDIO_FMT_PCM16},
	#define DECLARE_P16_WORKBUFF					UINT32 u32P16WorkBuf[(P16_DECODE_WORK_BUF_SIZE+3)/4];
	#define DECLARE_P16_PCMBUFF						INT16 i16P16DecodePcmBuf[P16_DECODE_SAMPLE_PER_FRAME];
#else
	#define AUDIOSYNTHEXAPP_P16_DECODER
	#define DECLARE_P16_WORKBUFF
	#define DECLARE_P16_PCMBUFF
#endif 

#if (LP8_ENABLE)
	#define AUDIOSYNTHEXAPP_LP8_DECODER				{LP8_DecodeInitiate, LP8_DecodeProcess, LP8_DecodeIsEnd, AUDIO_FMT_LP8},
	#define DECLARE_LP8_WORKBUFF					UINT32 u32LP8WorkBuf[(LP8_DECODE_WORK_BUF_SIZE+3)/4];
	#define DECLARE_LP8_PCMBUFF						INT16 i16LP8DecodePcmBuf[LP8_DECODE_SAMPLE_PER_FRAME];
#else
	#define AUDIOSYNTHEXAPP_LP8_DECODER			
	#define DECLARE_LP8_WORKBUFF					
	#define DECLARE_LP8_PCMBUFF
#endif
	
#if (MD4_ENABLE)
	#define AUDIOSYNTHEXAPP_MD4_DECODER				{MD4_DecodeInitiate, MD4_DecodeProcess, MD4_DecodeIsEnd, AUDIO_FMT_MDPCM4},
	#define DECLARE_MD4_WORKBUFF					UINT32 u32MD4WorkBuf[(MD4_DECODE_WORK_BUF_SIZE+3)/4];
	#define DECLARE_MD4_PCMBUFF						INT16 i16MD4DecodePcmBuf[MD4_DECODE_SAMPLE_PER_FRAME];
#else
	#define AUDIOSYNTHEXAPP_MD4_DECODER			
	#define DECLARE_MD4_WORKBUFF
	#define DECLARE_MD4_PCMBUFF
#endif

#if (MIDI_ENABLE)	
	#define AUDIOSYNTHEXAPP_MIDISYNTHEX_DECODER		{MIDISynthEx_DecodeInitiate, MIDISynthEx_DecodeProcess, MIDISynthEx_DecodeIsEnd, AUDIO_FMT_IMFMIDI},
	#define DECLARE_MIDISYNTHEX_WORKBUFF			S_MIDI_TOTAL_WORK_BUF sMIDITotalWorkBuf;
	#define DECLARE_MIDISYNTHEX_PCMBUFF				INT16 i16MIDIDecodePcmBuf[AUDIOSYNTHEXAPP_MIDI_DECODE_SAMPLE_PER_FRAME];
#else
	#define AUDIOSYNTHEXAPP_MIDISYNTHEX_DECODER	
	#define DECLARE_MIDISYNTHEX_WORKBUFF
	#define DECLARE_MIDISYNTHEX_PCMBUFF
#endif

#if (NUSOUND_ENABLE)	
	#define AUDIOSYNTHEXAPP_NUSOUNDEX_DECODER		{NuSoundEx_DecodeInitiate, NuSoundEx_DecodeProcess, NuSoundEx_DecodeIsEnd, AUDIO_FMT_NUSOUND},
	#define DECLARE_NUSOUNDEX_WORKBUFF				UINT32 u32NuSoundWorkBuf[(NUSOUNDEX_DECODE_WORK_BUF_SIZE+3)/4];
	#define DECLARE_NUSOUNDEX_TEMPBUFF				UINT32 u32NuSoundTempBuf[(NUSOUNDEX_DECODE_TEMP_BUF_SIZE+3)/4];
	#define DECLARE_NUSOUNDEX_PCMBUFF				INT16 i16NuSoundDecodePcmBuf[NUSOUNDEX_DECODE_SAMPLE_PER_FRAME];
#else
	#define AUDIOSYNTHEXAPP_NUSOUNDEX_DECODER		
	#define DECLARE_NUSOUNDEX_WORKBUFF
	#define DECLARE_NUSOUNDEX_TEMPBUFF
	#define DECLARE_NUSOUNDEX_PCMBUFF
#endif

#if (NUONE_ENABLE)
	#define AUDIOSYNTHEXAPP_NUONEEX_DECODER			{NuOneEx_DecodeInitiate, NuOneEx_DecodeProcess, NuOneEx_DecodeIsEnd, AUDIO_FMT_NUONE_E},
	#define DECLARE_NUONEEX_WORKBUFF				UINT32 u32NuOneWorkBuf[(NUONEEX_DECODE_WORK_BUF_SIZE+3)/4];
	#define DECLARE_NUONEEX_TEMPBUFF				UINT32 u32NuOneTempBuf[(NUONEEX_DECODE_TEMP_BUF_SIZE+3)/4];
	#define DECLARE_NUONEEX_PCMBUFF					INT16 i16NuOneDecodePcmBuf[NUONEEX_DECODE_SAMPLE_PER_FRAME];
#else
	#define AUDIOSYNTHEXAPP_NUONEEX_DECODER		
	#define DECLARE_NUONEEX_WORKBUFF
	#define DECLARE_NUONEEX_TEMPBUFF
	#define DECLARE_NUONEEX_PCMBUFF
#endif

#if (NULITE_ENABLE)
	#define AUDIOSYNTHEXAPP_NULITEEX_DECODER		{NuLiteEx_DecodeInitiate, NuLiteEx_DecodeProcess, NuLiteEx_DecodeIsEnd, AUDIO_FMT_NULITE_E},
	#define DECLARE_NULITEEX_WORKBUFF				UINT32 u32NuLiteWorkBuf[(NULITEEX_DECODE_WORK_BUF_SIZE+3)/4];
	#define DECLARE_NULITEEX_TEMPBUFF				UINT32 u32NuLiteTempBuf[(NULITEEX_DECODE_TEMP_BUF_SIZE+3)/4];
	#define DECLARE_NULITEEX_PCMBUFF				INT16 i16NuLiteDecodePcmBuf[NULITEEX_DECODE_SAMPLE_PER_FRAME];
#else
	#define AUDIOSYNTHEXAPP_NULITEEX_DECODER		
	#define DECLARE_NULITEEX_WORKBUFF
	#define DECLARE_NULITEEX_TEMPBUFF
	#define DECLARE_NULITEEX_PCMBUFF
#endif	

#if (NUVOX53_ENABLE)	
	#define AUDIOSYNTHEXAPP_NUVOX53EX_DECODER		{NuVox53Ex_DecodeInitiate, NuVox53Ex_DecodeProcess, NuVox53Ex_DecodeIsEnd, AUDIO_FMT_NUVOX53},
	#define DECLARE_NUVOX53EX_WORKBUFF				UINT32 u32NuVox53WorkBuf[(NUVOX53EX_DECODE_WORK_BUF_SIZE+3)/4];
	#define DECLARE_NUVOX53EX_TEMPBUFF				UINT32 u32NuVox53TempBuf[(NUVOX53EX_DECODE_TEMP_BUF_SIZE+3)/4];
	#define DECLARE_NUVOX53EX_PCMBUFF				INT16 i16NuVox53DecodePcmBuf[NUVOX53EX_DECODE_SAMPLE_PER_FRAME];
#else
	#define AUDIOSYNTHEXAPP_NUVOX53EX_DECODER		
	#define DECLARE_NUVOX53EX_WORKBUFF
	#define DECLARE_NUVOX53EX_TEMPBUFF
	#define DECLARE_NUVOX53EX_PCMBUFF
#endif

#if (NUVOX63_ENABLE)
	#define AUDIOSYNTHEXAPP_NUVOX63EX_DECODER		{NuVox63Ex_DecodeInitiate, NuVox63Ex_DecodeProcess, NuVox63Ex_DecodeIsEnd, AUDIO_FMT_NUVOX53},
	#define DECLARE_NUVOX63EX_WORKBUFF				UINT32 u32NuVox63WorkBuf[(NUVOX63EX_DECODE_WORK_BUF_SIZE+3)/4];
	#define DECLARE_NUVOX63EX_TEMPBUFF				UINT32 u32NuVox63TempBuf[(NUVOX63EX_DECODE_TEMP_BUF_SIZE+3)/4];
	#define DECLARE_NUVOX63EX_PCMBUFF				INT16 i16NuVox63DecodePcmBuf[NUVOX63EX_DECODE_SAMPLE_PER_FRAME];
#else
	#define AUDIOSYNTHEXAPP_NUVOX63EX_DECODER		
	#define DECLARE_NUVOX63EX_WORKBUFF			
	#define DECLARE_NUVOX63EX_TEMPBUFF			
	#define DECLARE_NUVOX63EX_PCMBUFF				
#endif

#if (IMAADPCM_ENABLE)
	#define AUDIOSYNTHEXAPP_IMAADPCM_DECODER		{ImaAdpcm_DecodeInitiate, ImaAdpcm_DecodeProcess, ImaAdpcm_DecodeIsEnd, AUDIO_FMT_IMAADPCM},
	#define DECLARE_IMAADPCM_WORKBUFF				UINT32 u32ImaAdpcmWorkBuf[(IMAADPCM_DECODE_WORK_BUF_SIZE+3)/4];
	#define DECLARE_IMAADPCM_TEMPBUFF				UINT32 u32ImaAdpcmTempBuf[(IMAADPCM_DECODE_TEMP_BUF_SIZE+3)/4];
	#define DECLARE_IMAADPCM_PCMBUFF				INT16 i16ImaAdpcmDecodePcmBuf[IMAADPCM_SAMPLE_PER_FRAME];
#else
	#define AUDIOSYNTHEXAPP_IMAADPCM_DECODER		
	#define DECLARE_IMAADPCM_WORKBUFF			
	#define DECLARE_IMAADPCM_TEMPBUFF			
	#define DECLARE_IMAADPCM_PCMBUFF				
#endif

// Declare decoder function list
#define	DECLARE_AUDIOSYNTHEXAPP_DECODER() \
S_AUDIOSYNTHEX_APP_DECODER const g_sAudioSynthExDecoderList[] = \
{ \
	AUDIOSYNTHEXAPP_NUSOUNDEX_DECODER \
	AUDIOSYNTHEXAPP_NUONEEX_DECODER \
	AUDIOSYNTHEXAPP_NULITEEX_DECODER \
	AUDIOSYNTHEXAPP_NUVOX53EX_DECODER \
	AUDIOSYNTHEXAPP_NUVOX63EX_DECODER \
	AUDIOSYNTHEXAPP_MIDISYNTHEX_DECODER \
	AUDIOSYNTHEXAPP_MD4_DECODER \
	AUDIOSYNTHEXAPP_LP8_DECODER \
	AUDIOSYNTHEXAPP_P16_DECODER \
	AUDIOSYNTHEXAPP_IMAADPCM_DECODER\
	NULL \
	};\

// -------------------------------------------------------------------------------------------------------------------------------
// Configurations: work buffer, decoded frame buffer and temporary buffer for codec library
// -------------------------------------------------------------------------------------------------------------------------------
// Union structure of codec work buffer 
typedef union						
{
	DECLARE_NUSOUNDEX_WORKBUFF
	DECLARE_NULITEEX_WORKBUFF	
	DECLARE_NUONEEX_WORKBUFF
	DECLARE_NUVOX53EX_WORKBUFF
	DECLARE_NUVOX63EX_WORKBUFF													
	DECLARE_MD4_WORKBUFF
	DECLARE_LP8_WORKBUFF	
	DECLARE_P16_WORKBUFF
	DECLARE_MIDISYNTHEX_WORKBUFF
	DECLARE_IMAADPCM_WORKBUFF
}U_AUDIOSYNTHEX_APP_WORKBUF;
// Union structure of codec PCM (frame) buffer
typedef union
{
	DECLARE_NUSOUNDEX_PCMBUFF
	DECLARE_NUONEEX_PCMBUFF
	DECLARE_NULITEEX_PCMBUFF
	DECLARE_NUVOX53EX_PCMBUFF
	DECLARE_NUVOX63EX_PCMBUFF
	DECLARE_MD4_PCMBUFF
	DECLARE_LP8_PCMBUFF
	DECLARE_P16_PCMBUFF
	DECLARE_MIDISYNTHEX_PCMBUFF
	DECLARE_IMAADPCM_PCMBUFF
}U_AUDIOSYNTHEX_APP_PCMBUF;
// Union structure of codec temp buffer
typedef union
{
	DECLARE_NUSOUNDEX_TEMPBUFF
	DECLARE_NUONEEX_TEMPBUFF
	DECLARE_NULITEEX_TEMPBUFF
	DECLARE_NUVOX53EX_TEMPBUFF
	DECLARE_NUVOX63EX_TEMPBUFF
	DECLARE_IMAADPCM_TEMPBUFF
	UINT32	u32Reserved;
}U_AUDIOSYNTHEX_APP_TEMPBUF;

// -------------------------------------------------------------------------------------------------------------------------------
// Configurations: structure definition for AudioSynthEx
// -------------------------------------------------------------------------------------------------------------------------------	
extern void AudioSyntheExApp_MIDIConfig(UINT8 *pu8DecodeWorkBuf,		
	UINT32 u32MidiDataStartAddr, UINT32 u32MidiDataSize,
	PFN_AUDIO_DATAREQUEST pfnReadMidiDataCallback, PFN_AUDIO_DATAREQUEST pfnReadWavetableCallback);

// Storage reading callback funciton
typedef struct sAudioSynthExAppStorCallBack
{
	PFN_AUDIO_DATAREQUEST		pfnReadDataCallback;
	PFN_AUDIO_USREVENT			pfnUserEventCallback;
	PFN_AUDIO_DATAREQUEST		pfnReadMidiWavTableCallback;
}S_AUDIOSYNTHEX_APP_STOR_CALLBACK;

typedef struct sAudioSynthExApp
{
	UINT32 u32EquDataAddr;				// start address of equation data in storage 
	UINT32 u32ROMStartAddr;				// ROM file start address, AudioSynthEx will refer to this value to add storage offset
	UINT32 *pau32AudioIdList;			// Programer can arrange audio or equation ID in application  
	UINT8  *pau8TempBuf;				// codec temp buffer --> U_AUDIOSYNTHEX_APP_TEMPBUF
	
	S_AUDIOSYNTHEX_APP_DECODER *pfnDecoder; // codec function list --> g_sAudioSynthExDecoderList[]
	UINT16 u16DecodedDataCount;				// remaining samples in PCM buffer it is decreased as processing. 	
	UINT16 u16ReadDataCount;				// offset samples in PCM buffer it is increased as processing.
	
	S_BUF_CTRL sOutBufCtrl;								// Treate as a ring buffer or a ping-pong buffer
	INT16 i16OutPcmBuf[AUDIOSYNTHEXAPP_OUT_BUF_SIZE];	// Buffer to store decoded PCM data for DAC output
	

	UINT8  u8Repeat;					// Audio repeat times for equation
	UINT8  u8StorIndex;					// Index for storage callback functions stored in g_asAppCallBack[]
	UINT8  u8ChannelID;					// Channel ID assignment for audio or equation playback
	
	U_AUDIOSYNTHEX_APP_WORKBUF sWorkBuf;		// work buffer to keep decoder neeeded information during decoding.
	U_AUDIOSYNTHEX_APP_PCMBUF sDecodePcmBuf;	// buffer to keep decoded PCM data
	
	
	union								// Duplicate data into buffer/callback function
	{
		S_BUF_CTRL *psDuplicateOutBufCtrl;
		PFN_AUDIOSYNTHEXAPP_DUPLICATE_FUNC pfnDuplicateFunc;
	};	
	UINT8 u8CtrlFlag;					// Duplicate control flag(To buffer/callback function)
	
}S_AUDIOSYNTHEX_APP;

#define AUDIOAYNTHEXAPP_EQUATION_ID_MARK		0x80000000
#define AUDIOSYNTHEXAPP_AUIDO_LIST_END_MARK		0xffffffff

#define AUDIOAYNTHEXAPP_MAKE_AUDIO_ID(id)		((id)&~(AUDIOAYNTHEXAPP_EQUATION_ID_MARK))
#define AUDIOAYNTHEXAPP_MAKE_EQUATION_ID(id)	((id)|(AUDIOAYNTHEXAPP_EQUATION_ID_MARK))
#define AUDIOAYNTHEXAPP_IS_EQUATION_ID(id)		(((id)&(AUDIOAYNTHEXAPP_EQUATION_ID_MARK))!=0)
#define AUDIOAYNTHEXAPP_IS_AUDIO_ID(id)			(((id)&(AUDIOAYNTHEXAPP_EQUATION_ID_MARK))==0)

// -----------------------------------------------------------------------------------------------------------------
// Functins of AudioSynthExApp
// -----------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------
//	Description
//		Initiate buffer controlling for AudioSynthEx application
//
//	Parameter
//  	psAudioSynthExApp [in] :
//			Pointer of AudioSynthEx application handler.
//		*pau8TempBuf [in] : 
//			Temporary buffer for NuXXX series decode application.
//			Temporary buffer could be re-used by user after decode per frame.
//  	u32ROMStartAdd [in] :
//ƒÜ		Start address of ROM file which will be referenced during playback.
//
//	Return Value
//		None
//---------------------------------------------------------------------------------------------------------
void AudioSynthExApp_DecodeInitiate(
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp,
	UINT8 *pau8TempBuf,
	UINT32 u32ROMStartAdd												
);

//---------------------------------------------------------------------------------------------------------
//	Description
//		Get header information from ROM file and fill into psRomHeader structure.
//
//	Parameter
//  	psRomHeader [in] :
//			Pointer of ROM file header structure.
//  	u32ROMStartAdd [in] :
//ƒÜ		Start address of ROM file in the storage.
//		u8StorageIndex [in] :
//ƒÜ		The index of read callback to g_asAppCallBack[] array in AppCallback.c.
//			The read callback funciton is used to get the ROM file header
//
//	Return Value
//		None
//---------------------------------------------------------------------------------------------------------
void AudioSynthExApp_GetRomHeadInfo(
	S_ROM_HEADER *psRomHeader,
	UINT32 u32ROMStartAdd,						
	UINT8 u8StorageIndex
);

//---------------------------------------------------------------------------------------------------------
//	Description
// 		Stat to play an audio file or an equation represented by ID.
//		The audio file or equation will be discovered in audio rom file according the inputed ID.
//
//		This function will decode first frame of audio file to output buffer after discovered the audio file.
//
//		Due to audio ID is from 0 and equation ID is also from 0,
//		in order to identify audio ID and equation ID, the equation ID must 
//		call "AUDIOAYNTHEXAPP_MAKE_EQUATION_ID(id)" then pass to this funciton.
//
//		Due to this function does not enable APU to play processed PCMs.
//		Must call Playback_StartPlay() to start APU playing if necessary!
//
//	Parameter
//  	psAudioSynthExApp [in] :
//			Pointer of AudioSynthEx application handler.
//  	u32AudioOrEquationID [in] :
//ƒÜ		Audio ID or
//			Equation ID. which is maked by call "AUDIOAYNTHEXAPP_MAKE_EQUATION_ID(id)"
//		u8Channel [in] :
//ƒÜ		Assign an audio channel for playback audio or equation.
//
//	Return Value
//		FALSE :
//			Codec format miss match or initiate fail. 
//			Or Equation/audio ID is greater than maxumum equation/audio numbers of ROM file.
//		TRUE :
//			Success.
//---------------------------------------------------------------------------------------------------------
BOOL AudioSynthExApp_StartPlay(			
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp,	
	UINT32 u32AudioOrEquationID,			
	UINT8 u8Channel							
);

//---------------------------------------------------------------------------------------------------------
//	Description
// 		Stat to play an audio file represented by ID.
//		The audio file will be discovered in audio rom file according the inputed audio ID.
//
//		This function will decode first frame of audio file to output buffer after discovered the audio file.
//
//		Due to this function does not enable APU to play processed PCMs.
//		Must call Playback_StartPlay() to start APU playing if necessary!
//
//	Parameter
//  	psAudioSynthExApp [in] :
//			Pointer of AudioSynthEx application handler.
//		u32AudioID [in] : 
//			Audio ID.
//		u8Channel [in] :
//			Assign an audio channel number for playback audio.
//
//	Return Value
//		FALSE :
//			Codec format miss match or initiate fail.
//			Or Audio ID is greater than maxumum audio numbers of ROM file.				
//		TRUE :
//			Success.
//---------------------------------------------------------------------------------------------------------
BOOL AudioSynthExApp_StartPlayAudio(			
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp,	
	UINT32 u32AudioID,
	UINT8 u8Channel
);

//---------------------------------------------------------------------------------------------------------
//	Description
// 		Stat to play an equation represented by ID.
//		The equation will be discovered in audio rom file according the inputed equation ID.
//
//		This function will decode first frame of audio file to output buffer after discovered the audio file.
//
//		Due to this function does not enable APU to play processed PCMs.
//		Must call Playback_StartPlay() to start APU playing if necessary!
//
//	Parameter
//  	psAudioSynthExApp [in] :
//			Pointer of AudioSynthEx application handler.
//		u32EquationID [in] : 
//			equation ID.
//		u8Channel [in] : 
//			Assign an audio channel number for playback equation.
//
//	Return Value
//		FALSE :
//			Codec format miss match or initiate fail.
//			Or Equation ID is greater than maxumum equation numbers of ROM file.				
//		TRUE :
//			Success.
//---------------------------------------------------------------------------------------------------------
BOOL AudioSynthExApp_StartPlayEquation(			
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp,
	UINT32 u32EquationID,
	UINT8 u8Channel
);

//---------------------------------------------------------------------------------------------------------
//	Description
// 		Stat to play an audio id list list which is an array and contains a count of audio ID or equation ID.
// 		The audio id list is termined by 0xffffffff.
// 		Because this list is be referenced in "AudioSynthExApp_DecodeProcess", keep this list in global not in stack!
//
//	Parameter
//  	psAudioSynthExApp [in] :
//			Pointer of AudioSynthEx application handler.
//		*pau32AudioIdList [in] : 
//			Audio ID list and termined by 0xffffffff
//			This audio id list must in global not in stack
//		u8Channel [in] : 
//			Assign an audio channel number for playback audio ID list.
//
//	Return Value
//		FALSE :
//			Codec format miss match or initiate fail.
//			Or equation/audio ID is greater than maxumum equation/audio numbers..				
//		TRUE :
//			Success.
//---------------------------------------------------------------------------------------------------------
BOOL AudioSynthExApp_StartPlayAudioList(
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp,	
	UINT32 *pau32AudioIdList,
	UINT8 u8Channel
);

//---------------------------------------------------------------------------------------------------------
//	Description
//		Decode audio file or euqation and produce PCMs to output ring buffer.
//		Can check the function return value to know it is running out of audio data or decoding stopped. 
//
//	Parameter
//  	psAudioSynthExApp [in] :
//			Pointer of AudioSynthEx application handler.
//
//	Return Value
// 		FALSE : 
//			Running out of audio data or decoding stopped.
//		TRUE :  
//			Decoding is going on.
//---------------------------------------------------------------------------------------------------------
BOOL AudioSynthExApp_DecodeProcess(
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp		
);

//---------------------------------------------------------------------------------------------------------
//	Description:                                                                                           
//		Stop to decode audio data from ROM file for stoping to play audio codec. 
//		
// 	Argument:
//		psAudioSynthExApp [in] :
//			Pointer of AudioSynthEx application handler.
//
//	Return:
//		None
//---------------------------------------------------------------------------------------------------------
void 
AudioSynthExApp_DecodeStopPlay(
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp
);

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Force inline function.
//		Duplicate output data into assign buffer.
//
// 	Argument:
//		psAudioSynthExApp [in] :
//			Pointer of AudioSynthEx application handler.
//
//		psOutBufCtrl [in] :
//			Assign buffer to save output data.
//
// 	Return:
// 		None
//---------------------------------------------------------------------------------------------------------
__attribute__((always_inline)) void
AudioSynthExApp_DuplicateOutputToBuf(
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp,
	S_BUF_CTRL *psOutBufCtrl
) 
{
	psAudioSynthExApp->u8CtrlFlag = (psAudioSynthExApp->u8CtrlFlag&(~AUDIOSYNTHEXAPP_CTRL_DUPLICATE_TO_FUNC))|AUDIOSYNTHEXAPP_CTRL_DUPLICATE_TO_BUF;
	psAudioSynthExApp->psDuplicateOutBufCtrl = psOutBufCtrl;
	psOutBufCtrl->u16SampleRate = psAudioSynthExApp->sOutBufCtrl.u16SampleRate;
}

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Force inline function.
//		Duplicate output data into callback function to provide application processing.
//
// 	Argument:
//		psAudioSynthExApp [in] :
//			Pointer of AudioSynthEx application handler.
//
//		pfnDuplicateFunc [in] :
//			Assign callback to process output data.
//
// 	Return:
// 		None
//---------------------------------------------------------------------------------------------------------
__attribute__((always_inline)) void
AudioSynthExApp_DuplicateOutputToFunc(
	S_AUDIOSYNTHEX_APP *psAudioSynthExApp,
	PFN_AUDIOSYNTHEXAPP_DUPLICATE_FUNC pfnDuplicateFunc
) 
{
	psAudioSynthExApp->u8CtrlFlag = (psAudioSynthExApp->u8CtrlFlag&(~AUDIOSYNTHEXAPP_CTRL_DUPLICATE_TO_BUF))|AUDIOSYNTHEXAPP_CTRL_DUPLICATE_TO_FUNC;
	psAudioSynthExApp->pfnDuplicateFunc = pfnDuplicateFunc;
}

#ifdef	__cplusplus
}
#endif


#endif
