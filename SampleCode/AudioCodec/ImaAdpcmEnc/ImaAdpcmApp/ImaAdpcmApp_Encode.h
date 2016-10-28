#ifndef _CONFIGIMAADPCMAPP_ENCODE_H_
#define _CONFIGIMAADPCMAPP_ENCODE_H_

#include "ConfigApp.h"
#include "ImaAdpcm.h"
#include "BufCtrl.h"

// -------------------------------------------------------------------------------------------------------------------------------
// PCM Ring Buffer Definitions 
// -------------------------------------------------------------------------------------------------------------------------------
#define IMAADPCMAPP_IN_FRAME_NUM	  		2		// it can be : 2, 3, 4, 5, ....
#define IMAADPCMAPP_IN_SAMPLES_PER_FRAME	IMAADPCM_SAMPLE_PER_FRAME
#define IMAADPCMAPP_IN_BUF_SIZE 			(IMAADPCMAPP_IN_FRAME_NUM*IMAADPCMAPP_IN_SAMPLES_PER_FRAME)
 							
#if ( IMAADPCMAPP_ENCODE_ADC_BUF_SIZE%8 )
	#error "IMAADPCMAPP_ENCODE_ADC_BUF_SIZE must be multiple of '8'."	
#endif						

#define IMAADPCMAPP_ENCODE_BUF_SIZE			E_IMAADPCM_SPF_32_DATASIZE
#define IMAADPCMAPP_ENCODE_BUF_COUNT		20									// must >= 2
#define IMAADPCMAPP_ENCODE_BUF_TOTAL_SIZE	(IMAADPCMAPP_ENCODE_BUF_SIZE*IMAADPCMAPP_ENCODE_BUF_COUNT)
 							
#if ( IMAADPCMAPP_ENCODE_BUF_TOTAL_SIZE%8 )
	#error "IMAADPCMAPP_ENCODE_BUF_TOTAL_SIZE must be multiple of '8'."	
#endif	

typedef struct
{
	// Work buffer for ImaAdpcm library to keep private data during decoding.
	// (IMAADPCM_ENCODE_WORK_BUF_SIZE+3)/4 : Force to do 4 byte alignment
	UINT32 au32WorkBuf[(IMAADPCM_ENCODE_WORK_BUF_SIZE+3)/4];
	
	// Frame buffer for keeping encoded data after ImaAdpcm encoding 
	// (IMAADPCM_ENCODE_WORK_BUF_SIZE+3)/4 : Force to do 4 byte alignment
	UINT32 au32EncodeBuf[(IMAADPCMAPP_ENCODE_BUF_TOTAL_SIZE+3)/4];
	
	// Input buffer to save PCM samples.
	INT16 i16InBuf[IMAADPCMAPP_IN_BUF_SIZE];
	
	// Pointer of temporary buffer array.
	// Temporary buffer is provided for ImaAdpcm encode library.
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
	
	INT8 ai8EncodeTemp[IMAADPCMAPP_ENCODE_BUF_SIZE*2];
	INT32 i32EncodeDataByte;

		
} S_IMAADPCM_APP_ENCODE;

//---------------------------------------------------------------------------------------------------------
//	Description
//		Initiate buffer controlling for ImaAdpcm encode application
//
//	Parameter
//  	psImaAdpcmAppEncode[in] :
//			Pointer of ImaAdpcm encode application handler.
//  	pu8EncodeTempBuf[in]
//É‹		Temporary buffer for NuXXX series codec application.
//É‹		Temporary buffer could be re-used by user after encoding one frame.
//
//	Return Value
//		None
//---------------------------------------------------------------------------------------------------------
void ImaAdpcmApp_EncodeInitiate(S_IMAADPCM_APP_ENCODE *psImaAdpcmAppEncode, UINT8 *pu8EncodeTempBuf);

//---------------------------------------------------------------------------------------------------------
//	Description:                                                                                           
//		Initiate ImaAdpcm encode required variables and header information.
//		
//		Due to this function does not enable ADC to record input PCMs.
//		Must call Record_StartRec() to start ADC recording if necessary!	
//
// 	Argument:
//		psImaAdpcmAppEncode[in] :
//			Pointer of ImaAdpcm encode application handler.
//		psAudioChunkHeader[in] :
//			Structure pointer of audio chunk header for recording information. 
//			Encoded information:
//			enocded format, sample rate, encoded total size and bit rate.
// 		u16SampleRate[in] :
//			Sample rate of input data to provide ImaAdpcm encoder.
// 		u32BitPerFrame[in] :
//			Bit per frame of input data to provide ImaAdpcm encoder.
//
//	Return:
//		FALSE : 
//			Bit rate is larger than IMAADPCMAPP_ENCODE_MAX_BITRATE or sample rate is zero.
//		TRUE :
//			Initiate ImaAdpcm encoder successfully.
//---------------------------------------------------------------------------------------------------------
BOOL ImaAdpcmApp_EncodeStart(S_IMAADPCM_APP_ENCODE *psImaAdpcmAppEncode, S_AUDIOCHUNK_HEADER *psAudioChunkHeader, UINT16 u16SampleRate, UINT32 u32BitPerFrame);

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Encode data from input buffer and put encode data into encoded buffer.
//		Can check the function return value to know it is running out of audio data or encoding stopped. 
//
//		Due to this function does not write any encoded data to storage.
//		Must write storage function to write encoded data to storage.
//
// 	Argument:
//		psImaAdpcmAppDecode[in] :
//			Pointer of ImaAdpcm encode application handler.
//
// 	Return:
// 		FALSE : 
//			Active flag of input buffer for ADC is not triggered.
//		TRUE :  
//			Encoding is going on.
//---------------------------------------------------------------------------------------------------------
BOOL ImaAdpcmApp_EncodeProcess(S_IMAADPCM_APP_ENCODE *psImaAdpcmAppEncode);

//---------------------------------------------------------------------------------------------------------
// 	Description:     
//		Stop to encode PCM data.	
//
// 	Argument:
//		psImaAdpcmAppDecode[in] :
//			Pointer of ImaAdpcm encode application handler.
//
// 	Return:
// 		None.
//---------------------------------------------------------------------------------------------------------
void ImaAdpcmApp_EncodeEnd(S_IMAADPCM_APP_ENCODE *psImaAdpcmAppEncode);

UINT32 ImaAdpcmApp_EncodeFlush(S_IMAADPCM_APP_ENCODE *psImaAdpcmAppEncode);


#endif
