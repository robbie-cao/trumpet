#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "Reg.h"
#include "Conf.h"


void Audio_Process(void);
void Audio_PlayCh(uint8_t ch, RegChInfo_t chInfo);
void Audio_StopCh(uint8_t ch);
void Audio_StopChImmediately(uint8_t ch);
uint8_t Audio_GetPlayStatus(void);

void Audio_PauseCh(uint8_t ch);
void Audio_PauseResume(uint8_t ch);
void Audio_ReplayCh(uint8_t ch);
uint8_t Audio_VolumeUp(void);
uint8_t Audio_VolumeDown(void);
uint8_t Audio_SetVolume(uint8_t vol);
uint8_t Audio_GetVolume(void);
void Audio_Init(void);
#endif
