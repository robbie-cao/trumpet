/*--------------------------------------------------------------------------------------------------*/
/*                                                                                                  */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.       	                            */
/*                                                                                                  */
/*--------------------------------------------------------------------------------------------------*/
#include "MEchoApp.h"
#include "PlaybackRecord.h"
#include <string.h>
														   
//----------------------------------------------------------------------------------------------------
// Initialize Echo application.
//----------------------------------------------------------------------------------------------------
void
MEchoApp_Initiate(
	S_MECHOAPP *psMEchoApp)
{
	// clear memoy buffer of EchoApp data structure 
	memset(psMEchoApp,0,sizeof(S_MECHOAPP));

	//--------------------------------------------
	// MEcho Effect:
	//	Initiate Multiple-Echo Effect library 
	//-------------------------------------------- 
	VoiceChange_InitMEcho(
		MECHOAPP_MECHO_NUM,
		(INT8*)&(psMEchoApp->u32MEchoWorkBuff[0]),	// MEcho library working buffer
		MECHOAPP_MECHO_BUFF_COUNT,					// MEcho working buffer size
		MECHOAPP_MECHO_START_DECAY);				// The decay setting at initiating
	BUF_CTRL_SET_INACTIVE(&psMEchoApp->sOutBufCtrl);
}

//----------------------------------------------------------------------------------------------------
// Start to run Echo applicaiton.
//----------------------------------------------------------------------------------------------------
UINT32
MEchoApp_StartPlay(
	S_MECHOAPP *psMEchoApp,
	UINT8 u8PlaybackChannel,
	S_BUF_CTRL 	*psInBufCtrl

)
{
	// Save input buffer will be applied echo effect.
	psMEchoApp->psInBufCtrl = psInBufCtrl;
	
	// Set output buffer control which control the PCMs with each effect applied.
	Playback_SetOutputBuf(&psMEchoApp->sOutBufCtrl,psInBufCtrl->u16BufCount,
		psInBufCtrl->pi16Buf, psInBufCtrl->u16FrameSize, psInBufCtrl->u16SampleRate);
	
	// Pre-load one frame and this frame data are all 0.
	psMEchoApp->sOutBufCtrl.u16BufWriteIdx = psInBufCtrl->u16BufCount >> 1;
	
	// Add to audio channel
	Playback_Add(u8PlaybackChannel,&psMEchoApp->sOutBufCtrl);
	psMEchoApp->u8PlaybackChannel = u8PlaybackChannel;	
	
	return psInBufCtrl->u16SampleRate;
}

//----------------------------------------------------------------------------------------------------
// Stop Echo application
//----------------------------------------------------------------------------------------------------
void 
MEchoApp_StopPlay(
	S_MECHOAPP *psMEchoApp	// Echo app data structure
	)
{
	BUF_CTRL_SET_INACTIVE(&psMEchoApp->sOutBufCtrl);
	
	// Remove from audio channel
	Playback_Remove(psMEchoApp->u8PlaybackChannel);
}

//----------------------------------------------------------------------------------------------------
// Apply multiple-echo effect on input buffer and save PCM with multiple-echo effect applied into output buffer
//----------------------------------------------------------------------------------------------------
BOOL
MEchoApp_ProcessPlay(
	S_MECHOAPP *psMEchoApp
)
{
	UINT8 i;
	S_BUF_CTRL 	*psInBufCtrl, *psOutBufCtrl; 
	INT16 *pi16InBuf;
	
	psInBufCtrl  = psMEchoApp->psInBufCtrl;
	psOutBufCtrl = &psMEchoApp->sOutBufCtrl;

	if (BUF_CTRL_IS_INACTIVE(psOutBufCtrl))
		return FALSE;
	
	while ((psInBufCtrl->u16BufReadIdx > psInBufCtrl->u16BufWriteIdx || psInBufCtrl->u16BufWriteIdx - psInBufCtrl->u16BufReadIdx >= psInBufCtrl->u16FrameSize))
	{
		pi16InBuf = &(psInBufCtrl->pi16Buf[psInBufCtrl->u16BufReadIdx]);
		
		for (i = 0 ; i < psInBufCtrl->u16FrameSize ; i++)
		{
			//-------------------------------------------------------------------
			// MEcho Effect:
			// 	Apply multiple-echo effect on one PCM from input buffer
			//	and save PCM with echo multiple-effect applied into output buffer
			//-------------------------------------------------------------------
			*pi16InBuf ++ = VoiceChange_MEchoPc16(*pi16InBuf);
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
