/*--------------------------------------------------------------------------------------------------*/
/*                                                                                                  */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.       	                            */
/*                                                                                                  */
/*--------------------------------------------------------------------------------------------------*/
#include "DelayApp.h"
#include "PlaybackRecord.h"
#include <string.h>
														   
//----------------------------------------------------------------------------------------------------
// Initialize Dely aplication.
//----------------------------------------------------------------------------------------------------
void
DelayApp_Initiate(
	S_DELAYAPP *psDelayApp)
{
	// clear memoy buffer of DelayApp data structure 
	memset(psDelayApp,0,sizeof(S_DELAYAPP));

	//--------------------------------------------
	// Delay Effect:
	//	Initiate Delay Effect library 
	//-------------------------------------------- 
	VoiceChange_InitDelay(
		(INT8*)&(psDelayApp->u32DelayWorkBuff[0]),	// Delay library working buffer
		DELAYAPP_DELAY_BUFF_COUNT,					// Dely working buffer size
		DELAYAPP_DELAY_START_DECAY);				// The decay setting at initiating
	BUF_CTRL_SET_INACTIVE(&psDelayApp->sOutBufCtrl);
}

//----------------------------------------------------------------------------------------------------
// Start to run Dely applicaiton.
//----------------------------------------------------------------------------------------------------
UINT32
DelayApp_StartPlay(
	S_DELAYAPP *psDelayApp,
	UINT8 u8PlaybackChannel,
	S_BUF_CTRL 	*psInBufCtrl

)
{
	// Save input buffer will be applied dely effect.
	psDelayApp->psInBufCtrl = psInBufCtrl;
	
	// Set output buffer control which control the PCMs with each effect applied.
	Playback_SetOutputBuf(&psDelayApp->sOutBufCtrl,psInBufCtrl->u16BufCount,
		psInBufCtrl->pi16Buf, psInBufCtrl->u16FrameSize, psInBufCtrl->u16SampleRate);
	
	// Pre-load one frame and this frame data are all 0.
	psDelayApp->sOutBufCtrl.u16BufWriteIdx = psInBufCtrl->u16BufCount >> 1;
	
	// Add to audio channel
	Playback_Add(u8PlaybackChannel,&psDelayApp->sOutBufCtrl);
	psDelayApp->u8PlaybackChannel = u8PlaybackChannel;	
	
	return psInBufCtrl->u16SampleRate;
}

//----------------------------------------------------------------------------------------------------
// Stop Delay application
//----------------------------------------------------------------------------------------------------
void 
DelayApp_StopPlay(
	S_DELAYAPP *psDelayApp	// Echo app data structure
	)
{
	BUF_CTRL_SET_INACTIVE(&psDelayApp->sOutBufCtrl);
	
	// Remove from audio channel
	Playback_Remove(psDelayApp->u8PlaybackChannel);
}

//----------------------------------------------------------------------------------------------------
// Apply delay effect on input buffer and save PCM with delay effect applied into output buffer
//----------------------------------------------------------------------------------------------------
BOOL
DelayApp_ProcessPlay(
	S_DELAYAPP *psDelayApp
)
{
	UINT8 i;
	S_BUF_CTRL 	*psInBufCtrl, *psOutBufCtrl; 
	INT16 *pi16InBuf;
	
	psInBufCtrl  = psDelayApp->psInBufCtrl;
	psOutBufCtrl = &psDelayApp->sOutBufCtrl;
	
	if (BUF_CTRL_IS_INACTIVE(psOutBufCtrl))
		return FALSE;
	
	while ((psInBufCtrl->u16BufReadIdx > psInBufCtrl->u16BufWriteIdx || psInBufCtrl->u16BufWriteIdx - psInBufCtrl->u16BufReadIdx >= psInBufCtrl->u16FrameSize))
	{
		pi16InBuf = &(psInBufCtrl->pi16Buf[psInBufCtrl->u16BufReadIdx]);
		
		for (i = 0 ; i < psInBufCtrl->u16FrameSize ; i++)
		{
			//--------------------------------------------------------------
			// Delay Effect:
			//	Apply delay effect on one PCM from input buffer
			//	and save PCM with delay effect applied into output buffer
			//--------------------------------------------------------------
			*pi16InBuf ++ = VoiceChange_DelayPc16(*pi16InBuf);
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
