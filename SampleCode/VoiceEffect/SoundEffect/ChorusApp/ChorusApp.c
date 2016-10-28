/*--------------------------------------------------------------------------------------------------*/
/*                                                                                                  */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.       	                            */
/*                                                                                                  */
/*--------------------------------------------------------------------------------------------------*/
#include "ChorusApp.h"
#include "PlaybackRecord.h"
#include <string.h>
														   
//----------------------------------------------------------------------------------------------------
// Initialize Chorus application.
//----------------------------------------------------------------------------------------------------
void
ChorusApp_Initiate(
	S_CHORUSAPP *psChorusApp)
{
	// clear memoy buffer of ChorusApp data structure 
	memset(psChorusApp,0,sizeof(S_CHORUSAPP));
	
	//--------------------------------------------
	// Chorus Effect:
	//	Initiate Chorus library 
	//--------------------------------------------
	VoiceChange_InitChorus(
		(INT8*)&(psChorusApp->u32ChorusWorkBuff[0]),	// Chorus library working buffer
		CHORUSAPP_CHORUS_BUFF_COUNT);					// Chorus working buffer size
	BUF_CTRL_SET_INACTIVE(&psChorusApp->sOutBufCtrl);
}

//----------------------------------------------------------------------------------------------------
// Start to run Chorus applicaiton.
//----------------------------------------------------------------------------------------------------
UINT32
ChorusApp_StartPlay(
	S_CHORUSAPP *psChorusApp,
	UINT8 u8PlaybackChannel,
	S_BUF_CTRL 	*psInBufCtrl

)
{
	// Save input buffer will be applied chorus effect.
	psChorusApp->psInBufCtrl = psInBufCtrl;
	
	// Set output buffer control which control the PCMs with each effect applied.
	Playback_SetOutputBuf(&psChorusApp->sOutBufCtrl,psInBufCtrl->u16BufCount,
		psInBufCtrl->pi16Buf, psInBufCtrl->u16FrameSize, psInBufCtrl->u16SampleRate);
	
	// Pre-load one frame and this frame data are all 0.
	psChorusApp->sOutBufCtrl.u16BufWriteIdx = psInBufCtrl->u16BufCount >> 1;
	
	// Add to audio channel
	Playback_Add(u8PlaybackChannel,&psChorusApp->sOutBufCtrl);
	psChorusApp->u8PlaybackChannel = u8PlaybackChannel;	
	
	return psInBufCtrl->u16SampleRate;
}

//----------------------------------------------------------------------------------------------------
// Stop Chorus application
//----------------------------------------------------------------------------------------------------
void 
ChorusApp_StopPlay(
	S_CHORUSAPP *psChorusApp	// Chorus app data structure
	)
{
	BUF_CTRL_SET_INACTIVE(&psChorusApp->sOutBufCtrl);
	
	// Remove from audio channel
	Playback_Remove(psChorusApp->u8PlaybackChannel);
}

//----------------------------------------------------------------------------------------------------
// Apply chorus effect on input buffer and save PCM with chorus effect applied into output buffer
//----------------------------------------------------------------------------------------------------
BOOL
ChorusApp_ProcessPlay(
	S_CHORUSAPP *psChorusApp
)
{
	UINT8 i;
	S_BUF_CTRL 	*psInBufCtrl, *psOutBufCtrl; 
	INT16 *pi16InBuf;
	
	psInBufCtrl  = psChorusApp->psInBufCtrl;
	psOutBufCtrl = &psChorusApp->sOutBufCtrl;

	if (BUF_CTRL_IS_INACTIVE(psOutBufCtrl))
		return FALSE;
	
	while ((psInBufCtrl->u16BufReadIdx > psInBufCtrl->u16BufWriteIdx || psInBufCtrl->u16BufWriteIdx - psInBufCtrl->u16BufReadIdx >= psInBufCtrl->u16FrameSize))
	{
		pi16InBuf = &(psInBufCtrl->pi16Buf[psInBufCtrl->u16BufReadIdx]);
		
		for (i = 0 ; i < psInBufCtrl->u16FrameSize ; i++)
		{
			//------------------------------------------------------------
			// Chorus Effect:
			//	Apply chorus effect on one PCM from input buffer 
			//	and save PCM with chorus effect applied into output buffer
			//------------------------------------------------------------
			*pi16InBuf ++ = VoiceChange_ChorusPc16(*pi16InBuf);
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
