#include <string.h>

#include "Conf.h"
#include "Audio.h"
#include "Reg.h"
#include "Log.h"
#include "App.h"

// Temp reuse ATC for control
// FIXME
#ifdef NUONE_ATC
extern volatile uint8_t g_u8AppCtrl;
extern volatile uint8_t g_u8AtcCmd;
extern volatile uint8_t g_u8AtcCmd2;
extern uint32_t g_u32AtcParam;
#endif

//Audio main process
void Audio_Process(void)
{

}

void Audio_PlayCh(uint8_t ch, RegChInfo_t chInfo)
{
    // Temp reuse "AT+PLAY=ch,idx"
    // FIXME
    g_u8AtcCmd2 = 1;
    g_u32AtcParam = ((ch & 0xFF) << 24) | (chInfo.vpIdx & 0xFFFF);
}

void Audio_StopCh(uint8_t ch) //set stop flag
{
    // Temp reuse "AT+NUO=4"
    // FIXME
    g_u8AtcCmd = 1;
    g_u32AtcParam = 4;
}

void Audio_StopChImmediately(uint8_t ch) //stop all sound  no wait
{
    // TODO
}

uint8_t Audio_GetPlayStatus(void) //indify every channel playing status
{
    // Temp solution
    // FIXME
    return (g_u8AppCtrl & APPCTRL_PLAY);
}

void Audio_PauseCh(uint8_t ch)
{

}

void Audio_PauseResume(uint8_t ch)
{

}

void Audio_ReplayCh(uint8_t ch)
{

}

uint8_t Audio_VolumeUp(void) //return Volume value
{
    // TODO

    return VOLUME_DEFAULT;
}

uint8_t Audio_VolumeDown(void) //return Volume value
{
    // TODO

    return VOLUME_DEFAULT;
}

uint8_t Audio_SetVolume(uint8_t vol) //return Volume value
{
    // TODO

    return VOLUME_DEFAULT;
}

uint8_t Audio_GetVolume(void) //return Volume value
{
    // TODO

    return VOLUME_DEFAULT;
}

void Audio_Init(void)
{
}
