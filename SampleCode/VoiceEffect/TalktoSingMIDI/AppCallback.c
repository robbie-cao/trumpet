#include <string.h>
#include "App.h"
#include "Framework.h"
#include "SPIFlash.h"

UINT32 Storage_ReadDataCallback(void *pu8Buf, UINT32 u32StartAddr, UINT32 u32Count);
UINT32 UserEvent_ProcessCallback(UINT16 u16EventIndex, UINT16 u16EventSubIndex);

S_AUDIO_CALLBACK const g_asAppCallBack[] =
{
	{
		Storage_ReadDataCallback,	// Read data callback
		NULL,
		UserEvent_ProcessCallback,	// User event procss callback
		Storage_ReadDataCallback	// Read MIDI wavetable callback
	}
};


UINT32 UserEvent_ProcessCallback(UINT16 u16EventIndex, UINT16 u16EventSubIndex)
{
	ULTRAIO_EVENTHANDLER(u16EventIndex,u16EventSubIndex);
	return 0;
}

//----------------------------------------------------------------------------------------------------
// Callback function to load midi / timbre data from APROM to midi synthesizer.
//----------------------------------------------------------------------------------------------------
#ifdef READ_FROM_SPI_FLASH
extern S_SPIFLASH_HANDLER 	g_sSpiFlash;
UINT32 
Storage_ReadDataCallback(
	void *pDesAddr, 
	UINT32 u32Position, 
	UINT32 u32ByteNum
)
{
	SPIFlash_Read(&g_sSpiFlash, AUDIOROM_STORAGE_START_ADDR+u32Position, pDesAddr, u32ByteNum);
	
	return u32ByteNum;
}
#endif
//----------------------------------------------------------------------------------------------------
// Callback function to load midi / timbre data from APROM to midi synthesizer.
//----------------------------------------------------------------------------------------------------
#ifdef READ_FROM_APROM
extern UINT32 u32MidiDataBegin, u32MidiDataEnd;
UINT32 
Storage_ReadDataCallback(
	void *pDesAddr, 
	UINT32 u32Position, 
	UINT32 u32ByteNum
)
{
	UINT32 u32DataAddr = (UINT32)&u32MidiDataBegin;
	memcpy(pDesAddr, (void *)(u32DataAddr + u32Position), u32ByteNum);
	return u32ByteNum;
}
#endif

