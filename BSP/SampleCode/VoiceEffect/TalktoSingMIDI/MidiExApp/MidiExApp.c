#include <string.h>
#include "App.h"
#include "SPIFlashMap.h"
#include "AudioRom.h"
#include "AudioRes/Output/AudioRes_AudioInfo.h"

extern S_AUDIO_CALLBACK g_asAppCallBack[];


void MidiExApp_Config(S_MIDIEX_APP *psMidiExApp,
	UINT32 u32StartAddr, UINT32 u32DataSize,
	UINT32 u32WaveTableStartAddr
)
{
	// Configure polyphony buffer and FIFO buffer.
	MIDISynthEx_ResoureConfig((UINT8 *)psMidiExApp->au32DecodeWorkBuf,
		(UINT8 *)&psMidiExApp->au8TotalPolyphonyBuf[0][0], MIDI_POLYPHONY_NUM,		
		(UINT8 *)psMidiExApp->au8FifoBuf, MIDI_FIFIO_BUF_SIZE);
	
	// Configure start address and storage callback function for wave table.
	MIDISynthEx_WTBConfig((UINT8 *)psMidiExApp->au32DecodeWorkBuf, u32WaveTableStartAddr, g_asAppCallBack[psMidiExApp->u8CallbackIndex].pfnReadMidiWavTableCallback);
	
	// Set midi data start address, total size, sample rate and storage callback function.
	MIDISynthEx_AudioConfig((UINT8 *)psMidiExApp->au32DecodeWorkBuf, u32StartAddr,u32DataSize,MIDI_SAMPLE_RATE, g_asAppCallBack[psMidiExApp->u8CallbackIndex].pfnReadDataCallback);
	
	// Set output samples per frame.
	MIDISynthEx_DecodeSampleCounts((UINT8 *)psMidiExApp->au32DecodeWorkBuf, MIDIEXAPP_OUT_SAMPLES_PER_FRAME );
	
	// Ser midi output volume.
	MidiSyn_SetVolume(1,MIDI_CH_VOLUME);
}

void MidiExApp_DecodeInitiate(S_MIDIEX_APP *psMidiExApp, UINT8 *pau8TempBuf, UINT32 u32CallbackIndex)
{
	memset( psMidiExApp, '\0', sizeof(S_MIDIEX_APP) );
	psMidiExApp->u8CallbackIndex = (UINT8)u32CallbackIndex;
	BUF_CTRL_SET_INACTIVE(&psMidiExApp->sOutBufCtrl);
}

BOOL MidiExApp_DecodeStartPlayByID(S_MIDIEX_APP *psMidiExApp, UINT32 u32AudioID, UINT32 u32RomStartAddr, UINT8 u8ChannelID)
{
	S_ROM_AUDIO_CHUNK_INFO sAudioChunkInfo;
	
	// Get audio chunk information for audio chunk start address from ROM file.
	// The ROM file is placed on SPI Flash address "AUDIOROM_STORAGE_START_ADDR".
	AudioRom_GetAudioChunkInfo(g_asAppCallBack[psMidiExApp->u8CallbackIndex].pfnReadDataCallback, u32RomStartAddr, u32AudioID, &sAudioChunkInfo);
	
	// Configure midi resource and wavtable
	MidiExApp_Config(psMidiExApp,
		sAudioChunkInfo.u32AudioChunkAddr, 
		sAudioChunkInfo.u32AudioChunkSize,
		u32RomStartAddr+MIDISYN_WTBOFFSET
	);
	
	return MidiExApp_DecodeStartPlayByAddr(psMidiExApp,sAudioChunkInfo.u32AudioChunkAddr, u8ChannelID);
}

BOOL MidiExApp_DecodeStartPlayByAddr(S_MIDIEX_APP *psMidiExApp, UINT32 u32MidiStorageStartAddr, UINT8 u8ChannelID)
{
	UINT16 u16SampleRate;
	
	// MidiSynthEx decoder initiates work buffer and returns sample rate.
	if ( (u16SampleRate = MIDISynthEx_DecodeInitiate((UINT8*)psMidiExApp->au32DecodeWorkBuf, NULL, u32MidiStorageStartAddr, 
			g_asAppCallBack[psMidiExApp->u8CallbackIndex].pfnReadDataCallback )) == 0 )
		return FALSE;	

	// Initiate and set output buffer variable(include frame size, buffer size etc.) 
	Playback_SetOutputBuf(	&psMidiExApp->sOutBufCtrl,
							MIDIEXAPP_OUT_BUF_SIZE,
							psMidiExApp->i16OutBuf,
							MIDIEXAPP_OUT_SAMPLES_PER_FRAME,
							u16SampleRate);
	
	// Set active flag of output buffer for stoping MidiSynthEx decode.
	BUF_CTRL_SET_ACTIVE(&psMidiExApp->sOutBufCtrl);
	
	// Pre-decode one frame
	psMidiExApp->sOutBufCtrl.u16BufWriteIdx = MIDIEXAPP_OUT_SAMPLES_PER_FRAME;
	if ( MidiExApp_DecodeProcess(psMidiExApp) == FALSE )
	{
		BUF_CTRL_SET_INACTIVE(&psMidiExApp->sOutBufCtrl);
		return FALSE;
	}
	psMidiExApp->sOutBufCtrl.u16BufReadIdx = MIDIEXAPP_OUT_SAMPLES_PER_FRAME;
	
	// Add MidiSynthEx into channel and preper to play codec.
	psMidiExApp->u8ChannelID = u8ChannelID;
	Playback_Add(psMidiExApp->u8ChannelID, &psMidiExApp->sOutBufCtrl);
		
	return TRUE;
}

void MidiExApp_DecodeStopPlay(S_MIDIEX_APP *psMidiExApp)
{
	// Clear active flag of output buffer for stoping MidiSynthEx decode.
	BUF_CTRL_SET_INACTIVE(&psMidiExApp->sOutBufCtrl);
	// Remove audio codec output buffer from play channel.
	Playback_Remove(psMidiExApp->u8ChannelID);
}

BOOL MidiExApp_DecodeProcess(S_MIDIEX_APP *psMidiExApp)
{
	INT16 *pi16OutBuf;
	
	if (BUF_CTRL_IS_INACTIVE(&psMidiExApp->sOutBufCtrl))
		return FALSE;

	if(Playback_NeedUpdateOutputBuf(&psMidiExApp->sOutBufCtrl))
	{
		// Check end of file
		if(MIDISynthEx_DecodeIsEnd((UINT8*)psMidiExApp->au32DecodeWorkBuf))
		{
			// Trigger inactive flag of output buffer to stop MidiSynthEx decoding
			BUF_CTRL_SET_INACTIVE(&psMidiExApp->sOutBufCtrl);
			// Use to represnt no active(or end) of decoding
			psMidiExApp->sOutBufCtrl.u16SampleRate = 0; 
			return FALSE;
		}

		// Record output data buffer pointer(for duplicate & process)
		pi16OutBuf = (PINT16)&psMidiExApp->sOutBufCtrl.pi16Buf[psMidiExApp->sOutBufCtrl.u16BufWriteIdx];
		
		MIDISynthEx_DecodeProcess(
			(UINT8*)psMidiExApp->au32DecodeWorkBuf, 
			NULL, 
			pi16OutBuf, 
			g_asAppCallBack[psMidiExApp->u8CallbackIndex].pfnReadDataCallback, 
			g_asAppCallBack[psMidiExApp->u8CallbackIndex].pfnUserEventCallback);
		
		// Update write index of output buffer and avoid buffer overflow
		Playback_UpdateOutputBuf(&psMidiExApp->sOutBufCtrl);
		
		// Duplicate data into buffer for using duplication callback function.
		if ( psMidiExApp->u8CtrlFlag&(MIDIEXAPP_CTRL_DUPLICATE_TO_BUF|MIDIEXAPP_CTRL_DUPLICATE_TO_FUNC) )
		{
			if ( psMidiExApp->u8CtrlFlag & MIDIEXAPP_CTRL_DUPLICATE_TO_BUF )
				BufCtrl_WriteWithCount(psMidiExApp->psDuplicateOutBufCtrl, MIDIEXAPP_OUT_SAMPLES_PER_FRAME, pi16OutBuf );
			else 
				psMidiExApp->pfnDuplicateFunc(MIDIEXAPP_OUT_SAMPLES_PER_FRAME, pi16OutBuf);
		}
	}	
	return TRUE;
}
