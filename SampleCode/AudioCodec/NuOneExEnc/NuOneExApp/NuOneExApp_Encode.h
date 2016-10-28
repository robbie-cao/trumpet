#ifndef _CONFIGNUONEEXAPP_ENCODE_H_
#define _CONFIGNUONEEXAPP_ENCODE_H_

#include "ConfigApp.h"
#include "NuOneEx.h"
#include "BufCtrl.h"

// -------------------------------------------------------------------------------------------------------------------------------
// PCM Ring Buffer Definitions 
// -------------------------------------------------------------------------------------------------------------------------------
#define NUONEEXAPP_IN_FRAME_NUM	  			2		// it can be : 2, 3, 4, 5, ....
#define NUONEEXAPP_IN_SAMPLES_PER_FRAME		NUONEEX_DECODE_SAMPLE_PER_FRAME
#define NUONEEXAPP_IN_BUF_SIZE 				(NUONEEXAPP_IN_FRAME_NUM*NUONEEXAPP_IN_SAMPLES_PER_FRAME)
 							
#if ( NUONEEXAPP_ENCODE_ADC_BUF_SIZE%8 )
	#error "NUONEEXAPP_IN_BUF_SIZE must be multiple of '8'."	
#endif						

#define NUONEEXAPP_ENCODE_MAX_BITRATE		E_NUONEEX_ENCODE_BPS_20				// NuOne bit rate selection
#define NUONEEXAPP_ENCODE_BUF_COUNT			2									// must >= 2
#define NUONEEXAPP_ENCODE_BUF_TOTAL_SIZE	((NUONEEXAPP_ENCODE_MAX_BITRATE>>3)*NUONEEXAPP_ENCODE_BUF_COUNT)
 							
#if ( NUONEEXAPP_ENCODE_BUF_TOTAL_SIZE%8 )
	#error "NUONEEXAPP_ENCODE_BUF_TOTAL_SIZE must be multiple of '8'."	
#endif		

typedef struct
{
	// Work buffer for NuOneEx decode library to keep private data during decoding.
	// (NUONEEX_ENCODE_WORK_BUF_SIZE+3)/4 : Force to do 4 byte alignment
	UINT32 au32WorkBuf[(NUONEEX_ENCODE_WORK_BUF_SIZE+3)/4];
	
	// Frame buffer for keeping encoded data after NuOneEx encoding 
	// (NUONEEXAPP_ENCODE_BUF_TOTAL_SIZE+3)/4 : Force to do 4 byte alignment
	UINT32 au32EncodeBuf[(NUONEEXAPP_ENCODE_BUF_TOTAL_SIZE+3)/4];
	
	// Input buffer to save PCM samples
	INT16 i16InBuf[NUONEEXAPP_IN_BUF_SIZE];
	
	// Pointer of temporary buffer array.
	// Temporary buffer is provided for NuOneEx encode library.
	UINT8 *pau8TempBuf;
	
	// The encoded buffer control structure provides these operations:
	// 1. stores encoded data.
	// 2. the read index which represents the first encoded data in the ring buffer
	// 3. the write index which represents the first free space in the ring buffer
	// 4. the frame size which represents the count of encoded data at each time
	S_BUF_CTRL	sEncodeBufCtrl;
	
	// The PCM input buffer control structure provides these operations:
	// 1. stores PCM data.
	// 2. the read index which represents the first PCM data for encoding in the ring buffer
	// 3. the write index which represents the first free space in the ring buffer
	// 4. the frame size which represents the count of input PCM at each time
	S_BUF_CTRL	sInBufCtrl;
	
} S_NUONEEX_APP_ENCODE;

//---------------------------------------------------------------------------------------------------------
//	Description
//		Initiate buffer controlling for NuOneEx encode application
//
//	Parameter
//  	psNuOneExAppEncode[in] :
//			Pointer of NuOneEx encode application handler.
//  	pu8EncodeTempBuf[in]
//É‹		Temporary buffer for NuXXX series codec application.
//É‹		Temporary buffer could be re-used by user after encoding one frame.
//
//	Return Value
//		None
//---------------------------------------------------------------------------------------------------------
void NuOneExApp_EncodeInitiate(S_NUONEEX_APP_ENCODE *psNuOneExAppEncode, UINT8 *pu8EncodeTempBuf);

//---------------------------------------------------------------------------------------------------------
//	Description:                                                                                           
//		Initiate NuOneEx encode required variables and header information.
//		
//		Due to this function does not enable ADC to record input PCMs.
//		Must call Record_StartRec() to start ADC recording if necessary!	
//
// 	Argument:
//		psNuOneExAppEncode[in] :
//			Pointer of NuOneEx encode application handler.
//		psAudioChunkHeader[in] :
//			Structure pointer of audio chunk header for recording information. 
//			Encoded information:
//			enocded format, sample rate, encoded total size and bit rate.
// 		u16SampleRate[in] :
//			Sample rate of input data to provide NuOneEx encoder.
// 		eBitPerSample[in] :
//			Bit per sample of input data to provide NuOneEx encoder.
//
//	Return:
//		FALSE : 
//			Bit rate is larger than NUONEEXAPP_ENCODE_MAX_BITRATE or sample rate is zero.
//		TRUE :
//			Initiate NuOneEx encoder successfully.
//---------------------------------------------------------------------------------------------------------
BOOL NuOneExApp_EncodeStart(S_NUONEEX_APP_ENCODE *psNuOneExAppEncode, S_AUDIOCHUNK_HEADER *psAudioChunkHeader, UINT16 u16SampleRate, enum eNuOneExEncodeBPS eBitPerSample);

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Encode data from ADC input buffer and put into encoded buffer.
//		Can check the function return value to know it is running out of audio data or encoding stopped. 
//
//		Due to this function does not write any encoded data to storage.
//		Must SPIFlashUtil_WriteEncodeData to flush encoded data and write to storage.
//
// 	Argument:
//		psNuOneExAppEncode[in] :
//			Pointer of NuOneEx encode application handler.
//
// 	Return:
// 		FALSE : 
//			Active flag of input buffer for ADC is not triggered.
//		TRUE :  
//			Encoding is going on.
//---------------------------------------------------------------------------------------------------------
BOOL NuOneExApp_EncodeProcess(S_NUONEEX_APP_ENCODE *psNuOneExAppEncode);

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Stop to ecode PCM data from ADC.
//		
//		Due to this function does not close ADC for encoding. 
//		Must call Record_StopRec() to close ADC playing!	
//		
//		Besides, programmer needs to call SPIFlashUtil_EndWriteEncodeData function
//		to update header information after completely encoding.
//
// 	Argument:
//		psNuOneExAppEncode[in] :
//			Pointer of NuOneEx encode application handler.
//
// 	Return:
// 		None.
//---------------------------------------------------------------------------------------------------------
void NuOneExApp_EncodeEnd(S_NUONEEX_APP_ENCODE *psNuOneExAppEncode);

#endif
