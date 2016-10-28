/*--------------------------------------------------------------------------------------------------*/
/*                                                                                                  */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.       	                            */
/*                                                                                                  */
/*--------------------------------------------------------------------------------------------------*/
#include "EchoApp.h"
#include "PlaybackRecord.h"
#include <string.h>
														   
//----------------------------------------------------------------------------------------------------
// Initialize Echo application.
//----------------------------------------------------------------------------------------------------
void
EchoApp_Initiate(
	S_ECHOAPP *psEchoApp)
{
	// clear memoy buffer of EchoApp data structure 
	memset(psEchoApp,0,sizeof(S_ECHOAPP));
	
	//--------------------------------------------
	// Echo Effect:
	//	Initiate Echo Effect library 
	//--------------------------------------------
	VoiceChange_InitEcho(
		(INT8*)&(psEchoApp->u32EchoWorkBuff[0]),	// Echo library working buffer
		ECHOAPP_ECHO_BUFF_COUNT,					// Echo working buffer size
		ECHOAPP_ECHO_START_DECAY);					// The decay setting at initiating
	BUF_CTRL_SET_INACTIVE(&psEchoApp->sOutBufCtrl);
}

//----------------------------------------------------------------------------------------------------
// Start to run Echo applicaiton.
//----------------------------------------------------------------------------------------------------
UINT32
EchoApp_StartPlay(
	S_ECHOAPP *psEchoApp,
	UINT8 u8PlaybackChannel,
	S_BUF_CTRL 	*psInBufCtrl

)
{
	// Save input buffer will be applied echo effect.
	psEchoApp->psInBufCtrl = psInBufCtrl;
	
	// Set output buffer control which control the PCMs with each effect applied.
	Playback_SetOutputBuf(&psEchoApp->sOutBufCtrl,psInBufCtrl->u16BufCount,
		psInBufCtrl->pi16Buf, psInBufCtrl->u16FrameSize, psInBufCtrl->u16SampleRate);
	
	// Pre-load one frame and this frame data are all 0.
	psEchoApp->sOutBufCtrl.u16BufWriteIdx = psInBufCtrl->u16BufCount >> 1;
	
	// Add to audio channel
	Playback_Add(u8PlaybackChannel,&psEchoApp->sOutBufCtrl);
	psEchoApp->u8PlaybackChannel = u8PlaybackChannel;	
	
	return psInBufCtrl->u16SampleRate;
}

//----------------------------------------------------------------------------------------------------
// Stop Echo application
//----------------------------------------------------------------------------------------------------
void 
EchoApp_StopPlay(
	S_ECHOAPP *psEchoApp	// Echo app data structure
	)
{
	BUF_CTRL_SET_INACTIVE(&psEchoApp->sOutBufCtrl);
	
	// Remove from audio channel
	Playback_Remove(psEchoApp->u8PlaybackChannel);
}

//----------------------------------------------------------------------------------------------------
// Apply echo effect on input buffer and save PCM with echo effect applied into output buffer
//----------------------------------------------------------------------------------------------------
BOOL
EchoApp_ProcessPlay(
	S_ECHOAPP *psEchoApp
)
{
	UINT8 i;
	S_BUF_CTRL 	*psInBufCtrl, *psOutBufCtrl; 
	INT16 *pi16InBuf;
	
	psInBufCtrl  = psEchoApp->psInBufCtrl;
	psOutBufCtrl = &psEchoApp->sOutBufCtrl;

	if (BUF_CTRL_IS_INACTIVE(psOutBufCtrl))
		return FALSE;
	
	while ((psInBufCtrl->u16BufReadIdx > psInBufCtrl->u16BufWriteIdx || psInBufCtrl->u16BufWriteIdx - psInBufCtrl->u16BufReadIdx >= psInBufCtrl->u16FrameSize))
	{
		pi16InBuf = &(psInBufCtrl->pi16Buf[psInBufCtrl->u16BufReadIdx]);

		for (i = 0 ; i < psInBufCtrl->u16FrameSize ; i++)
		{
			//-----------------------------------------------------------
			// Echo Effect:
			// 	Apply echo effect on one PCM from input buffer
			//	and save PCM with echo effect applied into output buffer
			//-----------------------------------------------------------
			*pi16InBuf ++ = VoiceChange_EchoPc16(*pi16InBuf);
		}
		
		// Update write index of output buffer and avoid buffer overrun
		psOutBufCtrl->u16BufWriteIdx += psInBufCtrl->u16FrameSize;
		if (psOutBufCtrl->u16BufWriteIdx >= psOutBufCtrl->u16BufCount)
			psOutBufCtrl->u16BufWriteIdx = 0;
		
		// Update read index of input buffer and avoid buffer overrun
		psInBufCtrl->u16BufReadIdx  += psInBufCtrl->u16FrameSize;
		if (psInBufCtrl->u16BufReadIdx >= psInBufCtrl->u16BufCount)
			psInBufCtrl->u16BufReadIdx = 0;
	}	
	return TRUE;
}
