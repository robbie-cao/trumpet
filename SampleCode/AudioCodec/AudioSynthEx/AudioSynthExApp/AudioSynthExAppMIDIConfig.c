#include "AudioSynthExApp.h"
#include "AudioRes/Output/AudioRes_AudioInfo.h"
#include "SPIFlashMap.h"
#include "MicSpk.h"

//---------------------------------------------------------------------------------------------------------
//	Description
//		Configure buffer resource for MidiSynthEx library.
//		Configure wave table address.
//		Configure playback midi address, size, sample rate and storage callback function.
//		Set sample numbers per frame.
//		Set voumle.
//
//	Parameter
//  	pu8DecodeWorkBuf[in] :
//			Pointer of AudioSynthEx application handler.
//  	u32StartAddr[in] :
//ƒÜ		Start address of playback midi.
//		u32DataSize[in] :
//ƒÜ		Total size of playback midi.
//		pfnReadMidiDataCallback[in] :
//			Callback function for read midi data.
//		pfnReadWavetableCallback[in] :
//			Callback function for read wave table.
//
//	Return Value
//		None
//---------------------------------------------------------------------------------------------------------
void AudioSyntheExApp_MIDIConfig(UINT8 *pu8DecodeWorkBuf,		
	UINT32 u32MidiDataStartAddr, UINT32 u32MidiDataSize, 
	PFN_AUDIO_DATAREQUEST pfnReadMidiDataCallback, 
	PFN_AUDIO_DATAREQUEST pfnReadWavetableCallback
)
{
	// Configure polyphony buffer and FIFO buffer.
	MIDISynthEx_ResoureConfig((UINT8*)(((S_MIDI_TOTAL_WORK_BUF*)pu8DecodeWorkBuf)->u32WorkBuf),
		(UINT8*)(((S_MIDI_TOTAL_WORK_BUF*)pu8DecodeWorkBuf)->au8TotalPolyphonyBuf), AUDIOSYNTHEXAPP_MIDI_POLYPHONY_NUM,		
		((S_MIDI_TOTAL_WORK_BUF*)pu8DecodeWorkBuf)->au8FifoBuf, AUDIOSYNTHEXAPP_MIDI_FIFIO_BUF_SIZE);
	
	// Configure start address and storage callback function for wave table.
	MIDISynthEx_WTBConfig(pu8DecodeWorkBuf, AUDIOROM_STORAGE_START_ADDR+MIDISYN_WTBOFFSET, pfnReadWavetableCallback);
	
	// Set midi data start address, total size, sample rate and storage callback function.
	MIDISynthEx_AudioConfig(pu8DecodeWorkBuf, u32MidiDataStartAddr,u32MidiDataSize,AUDIOSYNTHEXAPP_MIDI_SAMPLE_RATE, pfnReadMidiDataCallback);
	
	// Set output samples per frame.
	MIDISynthEx_DecodeSampleCounts(pu8DecodeWorkBuf, AUDIOSYNTHEXAPP_MIDI_DECODE_SAMPLE_PER_FRAME );
	
	// Ser midi output volume.
	MidiSyn_SetVolume(1,AUDIOSYNTHEXAPP_MIDI_CH_VOLUME);
}

