#include "App.h"
#include <string.h>

extern S_APP g_sApp;

//----------------------------------------------------------------------------------------------------
// Event handler of direct trigger and key matrix. Application can use one handler to process all  
// events, or use different handler for each event.
//----------------------------------------------------------------------------------------------------

void Melody_R_KeypadHandler(UINT32 u32Param)
{
	if (g_sApp.u8KeyPressingCnt==0)
		App_PlayNextMelody();
	else
		App_ToggleMusic();
	ShowLeds();
}

//----------------------------------------------------------------------------------------------------
void Melody_P_KeypadHandler(UINT32 u32Param)
{
	g_sApp.u8KeyPressingCnt++;
}

//----------------------------------------------------------------------------------------------------
void Melody_F_KeypadHandler(UINT32 u32Param)
{
	g_sApp.u8KeyPressingCnt = 0;
}

//----------------------------------------------------------------------------------------------------
void PowerDown_KeypadHandler(UINT32 u32Param)
{
	App_PowerDown();
}

//----------------------------------------------------------------------------------------------------
void Default_KeyHandler(UINT32 u32Param)
{
	switch(u32Param)
	{
		case TG1F:
			App_StartOrStopPlaySound();
			break;
		case TG2F:
		case TG2R:
			App_StartOrStopRecord();
			break;
		case TG3R:
			Melody_R_KeypadHandler(0);
			break;
		case TG2P:
		case TG3P:
			Melody_P_KeypadHandler(0);
			break;
		case TG3F:
			Melody_F_KeypadHandler(0);
			break;
		case TG4F:
			App_SwitchTalkToSingMode();
			break;
		case TG5F:
			App_SwitchSingMode();
			break;
		case TG6P:
			PowerDown_KeypadHandler(0);
			break;	
	}
}
