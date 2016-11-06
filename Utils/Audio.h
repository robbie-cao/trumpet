#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "Reg.h"
#include "Conf.h"
#include "Lib\LibSiren7.h"

typedef struct {
    RegChInfo_t playingInfo;
    uint32_t  totalG722Size;
    uint32_t  audioSampleCount;
    uint32_t  audioDataAddr;
    uint32_t  startAddr;
    int16_t  audioDataBuffer[AUDIOBUFFERSIZE];
    sSiren7_CODEC_CTL sEnDeCtlCh;
    sSiren7_DEC_CTX  sS7Dec_CtxCh;
    uint16_t  factor; //sound volume decrese before stop,this is decrese factor,
    uint16_t  factor_shadow;
    uint32_t  byte_cnt;
    uint8_t   zeroCrossFlag;  //mark volume cross zero 0--current positive   1-- negative
} ChSoundInfo_t;

typedef struct {
    ChSoundInfo_t on_PlaySoundInfo;
    RegChInfo_t next_PlaySoundInfo;
    uint8_t   smValue; // state machine value
}CH_t;

enum{
    SPEAKER_OFF = 0,
    SPEAKER_ON
};

typedef struct {
	CH_t CH[CHANNEL_COUNT];
	uint32_t speakerBufferAddr;
	uint8_t playingStatus;
    uint8_t speakerStatus;
	uint8_t volume;
	uint8_t volumePrev;
}Audio_t;

void Audio_Process(void);
void Audio_PlayCh(uint8_t ch,RegChInfo_t chInfo);
void Audio_StopCh(uint8_t ch);
void Audio_StopChImmediately(uint8_t ch);
uint8_t Audio_GetPlayStatus(void);
void Audio_SoundDcrease(ChSoundInfo_t *soundInfo, int i);

void Audio_PauseCh(uint8_t ch);
void Audio_PauseResume(uint8_t ch);
void Audio_ReplayCh(uint8_t ch);
uint8_t Audio_VolumeUp(void);
uint8_t Audio_VolumeDown(void);
uint8_t Audio_SetVolume(uint8_t vol);
uint8_t Audio_GetVolume(void);
void Audio_Init(void);
#endif
