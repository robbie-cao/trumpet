#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "DrvI2C.h"
#include "Audio.h"
#include "Reg.h"
#include "OP.h"
#include "Conf.h"
#include "Log.h"


/*
 * OP module will handle on register only.
 * Reset and SoftReset are the only exception.
 */

#define DEBUG_OP                0

#define LOG_TAG                 "OP"

#define OP_DATA_BUFFER_SIZE     32     // OP_DATA_BUFFER_SIZE musb be >= sizeof(RegMap_t)

#define AUDIO_DIRECT            1
// Swap buffer for cmds which read something from 9160
uint8_t sOpDataBuffer[OP_DATA_BUFFER_SIZE];

/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
static int8_t OP_Reset(OpCmd_t *pCmd);
static int8_t OP_SoftReset(OpCmd_t *pCmd);
static int8_t OP_SetPowerMode(OpCmd_t *pCmd);
static int8_t OP_SetIntEn(OpCmd_t *pCmd);

static int8_t OP_Play_VP(OpCmd_t *pCmd);
static int8_t OP_Play_VP_Loop(OpCmd_t *pCmd);

static int8_t OP_Stop(OpCmd_t *pCmd);
static int8_t OP_Replay(OpCmd_t *pCmd);
static int8_t OP_Pause(OpCmd_t *pCmd);
static int8_t OP_Resume(OpCmd_t *pCmd);
static int8_t OP_PlayPauseResume(OpCmd_t *pCmd);
static int8_t OP_StopAll(OpCmd_t *pCmd);
static int8_t OP_PauseAll(OpCmd_t *pCmd);

static int8_t OP_VolumeSet(OpCmd_t *pCmd);
static int8_t OP_VolumeUp(OpCmd_t *pCmd);
static int8_t OP_VolumeDown(OpCmd_t *pCmd);
static int8_t OP_VolumeMute(OpCmd_t *pCmd);
static int8_t OP_VolumeUnmute(OpCmd_t *pCmd);
static int8_t OP_VolumeUpAll(OpCmd_t *pCmd);
static int8_t OP_VolumeDownAll(OpCmd_t *pCmd);

static int8_t OP_ReadChipId(OpCmd_t *pCmd);
static int8_t OP_ReadStatus(OpCmd_t *pCmd);
static int8_t OP_ReadChannelInfo(OpCmd_t *pCmd);
static int8_t OP_ReadVolume(OpCmd_t *pCmd);
static int8_t OP_ReadConfig(OpCmd_t *pCmd);

// Global
//
OpCmdHandlerItem_t opCmdHandler[] =
{
    { OP_SYS_RESET            , OP_Reset             } ,
    { OP_SYS_SOFTRESET        , OP_SoftReset         } ,
    { OP_SYS_SETPOWERMODE     , OP_SetPowerMode      } ,
    { OP_SYS_SETINTEN         , OP_SetIntEn          } ,

    { OP_PLAY_VP              , OP_Play_VP           } ,
    { OP_PLAY_VP_LOOP         , OP_Play_VP_Loop      } ,

    { OP_PB_STOP              , OP_Stop              } ,
    { OP_PB_REPLAY            , OP_Replay            } ,
    { OP_PB_PAUSE             , OP_Pause             } ,
    { OP_PB_RESUME            , OP_Resume            } ,
    { OP_PB_PLAY_PAUSE_RESUME , OP_PlayPauseResume   } ,
    { OP_PB_STOP_ALL          , OP_StopAll           } ,
    { OP_PB_PAUSE_ALL         , OP_PauseAll          } ,

    { OP_VOLUME_SET           , OP_VolumeSet         } ,
    { OP_VOLUME_UP            , OP_VolumeUp          } ,
    { OP_VOLUME_DOWN          , OP_VolumeDown        } ,
    { OP_VOLUME_MUTE          , OP_VolumeMute        } ,
    { OP_VOLUME_UNMUTE        , OP_VolumeUnmute      } ,
    { OP_VOLUME_UP_ALL        , OP_VolumeUpAll       } ,
    { OP_VOLUME_DOWN_ALL      , OP_VolumeDownAll     } ,

    { OP_READ_CHIPID          , OP_ReadChipId        } ,
    { OP_READ_STATUS          , OP_ReadStatus        } ,
    { OP_READ_CHANNEL_INFO    , OP_ReadChannelInfo   } ,
    { OP_READ_VOL             , OP_ReadVolume        } ,
    { OP_READ_CONFIG          , OP_ReadConfig        } ,
};

#define OP_HANDLER_CNT()    (sizeof(opCmdHandler) / sizeof(opCmdHandler[0]))

int8_t OP_Handler(OpCmd_t *pCmd)
{
    uint8_t i;

#if 0
    PM_Wakeup();
#endif

    if (!pCmd) {
        return SYS_INVALID_PARAM;
    }

    for (i = 0; i < OP_HANDLER_CNT(); i++) {
        if (pCmd->cmd == opCmdHandler[i].cmd) {
            return opCmdHandler[i].handler(pCmd);
        }
    }

    return SYS_NOT_SUPPORT;
}

static int8_t OP_Reset(OpCmd_t *pCmd)
{
#if 0
    Sys_Reset();
#endif

    return SYS_GOOD;
}

static int8_t OP_SoftReset(OpCmd_t *pCmd)
{
#if 0
    Sys_SoftReset();
#endif

    return SYS_GOOD;
}

static int8_t OP_SetPowerMode(OpCmd_t *pCmd)
{
    sRegisterMap.ctl.powerMode = pCmd->data;

#if 0
    PM_DeepPowerDown();
#endif

    return SYS_GOOD;
}

static int8_t OP_SetIntEn(OpCmd_t *pCmd)
{
    sRegisterMap.ctl.intEn = !!pCmd->data;

    return SYS_GOOD;
}


static int8_t OP_Play_VP(OpCmd_t *pCmd)
{
    if (pCmd->chIdx >= CHANNEL_COUNT) {
        return SYS_INVALID_PARAM;
    }

#if DEBUG_OP
    LOGD(LOG_TAG, "PLAY  : CH - %d, VP - %d\r\n", pCmd->chIdx, pCmd->vpIdx);
#endif
    sRegisterMap.ch[pCmd->chIdx].status.bits.state = CHANNEL_STATUS_PLAY;
    sRegisterMap.ch[pCmd->chIdx].status.bits.repeat = 0;
    sRegisterMap.ch[pCmd->chIdx].vpIdx = pCmd->vpIdx;

#if AUDIO_DIRECT
    Audio_PlayCh(pCmd->chIdx, sRegisterMap.ch[pCmd->chIdx]);
#endif
    return SYS_GOOD;
}

static int8_t OP_Play_VP_Loop(OpCmd_t *pCmd)
{
    if (pCmd->chIdx >= CHANNEL_COUNT) {
        return SYS_INVALID_PARAM;
    }

#if DEBUG_OP
    LOGD(LOG_TAG, "LOOP  : CH - %d, VP - %d\r\n", pCmd->chIdx, pCmd->vpIdx);
#endif
    sRegisterMap.ch[pCmd->chIdx].status.bits.state = CHANNEL_STATUS_PLAY;
    sRegisterMap.ch[pCmd->chIdx].status.bits.repeat = 1;
    sRegisterMap.ch[pCmd->chIdx].vpIdx = pCmd->vpIdx;
#if AUDIO_DIRECT
    Audio_PlayCh(pCmd->chIdx, sRegisterMap.ch[pCmd->chIdx]);
#endif

    return SYS_GOOD;
}


static int8_t OP_Stop(OpCmd_t *pCmd)
{
    if (pCmd->chIdx >= CHANNEL_COUNT) {
        return SYS_INVALID_PARAM;
    }


#if DEBUG_OP
    LOGD(LOG_TAG, "STOP  : CH - %d, VP - %d\r\n", pCmd->chIdx, pCmd->vpIdx);
#endif
    sRegisterMap.ch[pCmd->chIdx].status.bits.state = CHANNEL_STATUS_IDLE;
    // Other status bits and vpIdx will be update by main loop

#if AUDIO_DIRECT
    Audio_StopCh(pCmd->chIdx);
#endif

    return SYS_GOOD;
}

static int8_t OP_Replay(OpCmd_t *pCmd)
{
#if DEBUG_OP
    LOGD(LOG_TAG, "REPLAY: CH - %d, VP - %d\r\n", pCmd->chIdx, pCmd->vpIdx);
#endif

    return SYS_GOOD;
}

static int8_t OP_Pause(OpCmd_t *pCmd)
{
    if (pCmd->chIdx >= CHANNEL_COUNT) {
        return SYS_INVALID_PARAM;
    }

#if DEBUG_OP
    LOGD(LOG_TAG, "PAUSE : CH - %d, VP - %d\r\n", pCmd->chIdx, pCmd->vpIdx);
#endif
    if (sRegisterMap.ch[pCmd->chIdx].status.bits.state == CHANNEL_STATUS_PLAY) {
        sRegisterMap.ch[pCmd->chIdx].status.bits.state = CHANNEL_STATUS_PAUSE;
        // Other status bits and vpIdx will be update by main loop
    } else {
        // TODO
    }

    return SYS_GOOD;
}

static int8_t OP_Resume(OpCmd_t *pCmd)
{
    if (pCmd->chIdx >= CHANNEL_COUNT) {
        return SYS_INVALID_PARAM;
    }

#if DEBUG_OP
    LOGD(LOG_TAG, "RESUME: CH - %d, VP - %d\r\n", pCmd->chIdx, pCmd->vpIdx);
#endif
    if (sRegisterMap.ch[pCmd->chIdx].status.bits.state == CHANNEL_STATUS_PAUSE) {
        sRegisterMap.ch[pCmd->chIdx].status.bits.state = CHANNEL_STATUS_PLAY;
        // Other status bits and vpIdx will be update by main loop
    } else {
        // TODO
    }

    return SYS_GOOD;
}

static int8_t OP_PlayPauseResume(OpCmd_t *pCmd)
{
    // TODO

    return SYS_GOOD;
}

static int8_t OP_StopAll(OpCmd_t *pCmd)
{
    uint8_t i = 0;

#if DEBUG_OP
    LOGD(LOG_TAG, "STOP A\r\n");
#endif
    for (i = 0; i < CHANNEL_COUNT; i++) {
        sRegisterMap.ch[i].status.bits.state = CHANNEL_STATUS_IDLE;
        // Other status bits and vpIdx will be update by main loop
#if AUDIO_DIRECT
        Audio_StopChImmediately(i);
#endif
    }

    return SYS_GOOD;
}

static int8_t OP_PauseAll(OpCmd_t *pCmd)
{
    uint8_t i = 0;

#if DEBUG_OP
    LOGD(LOG_TAG, "PAUSEA\r\n");
#endif

    for (i = 0; i < CHANNEL_COUNT; i++) {
        if (sRegisterMap.ch[pCmd->chIdx].status.bits.state == CHANNEL_STATUS_PLAY) {
            sRegisterMap.ch[pCmd->chIdx].status.bits.state = CHANNEL_STATUS_PAUSE;
            // Other status bits and vpIdx will be update by main loop
        }
    }

    return SYS_GOOD;
}


static int8_t OP_VolumeSet(OpCmd_t *pCmd)
{
#if DEBUG_OP
    LOGD(LOG_TAG, "VOLSET: CH - %d, VOL - %d\r\n", pCmd->chIdx, pCmd->data);
#endif
    sRegisterMap.vol = (pCmd->data <= VOLUME_MAX) ? pCmd->data: VOLUME_MAX;
    // Set volume by channel?
    // Or a new function to do that?
    // TODO
    Audio_SetVolume(sRegisterMap.vol);

    return SYS_GOOD;
}

static int8_t OP_VolumeUp(OpCmd_t *pCmd)
{
    // TODO
    return SYS_GOOD;
}

static int8_t OP_VolumeDown(OpCmd_t *pCmd)
{
    // TODO
    return SYS_GOOD;
}

static int8_t OP_VolumeMute(OpCmd_t *pCmd)
{
    // TODO
    return SYS_GOOD;
}

static int8_t OP_VolumeUnmute(OpCmd_t *pCmd)
{
    // TODO
    return SYS_GOOD;
}

static int8_t OP_VolumeUpAll(OpCmd_t *pCmd)
{
    // TODO
    return SYS_GOOD;
}

static int8_t OP_VolumeDownAll(OpCmd_t *pCmd)
{
    // TODO
    return SYS_GOOD;
}


static int8_t OP_ReadChipId(OpCmd_t *pCmd)
{
    // Read Chip ID from SOC and copy it to sOpDataBuffer
    // TODO
    return SYS_GOOD;
}

static int8_t OP_ReadStatus(OpCmd_t *pCmd)
{
    RegMap_t *pRegMap = (RegMap_t *)&sOpDataBuffer;

    memcpy(sOpDataBuffer, &sRegisterMap, sizeof(RegMap_t));
#if DEBUG_OP
    LOGD(LOG_TAG, "RDSTAT: CH0 - 0x%02x %d %04d, CH1 - 0x%02x %d %04d, CH2 - 0x%02x %d %04d\r\n",
            pRegMap->ch[0].status.value, pRegMap->ch[0].vol, pRegMap->ch[0].vpIdx,
            pRegMap->ch[1].status.value, pRegMap->ch[1].vol, pRegMap->ch[1].vpIdx,
            pRegMap->ch[2].status.value, pRegMap->ch[2].vol, pRegMap->ch[2].vpIdx
        );
#endif
    // Little Endian(ISD9160 - ARM M0) -> Big Endian(CC2541 - 8051)
    pRegMap->ch[0].vpIdx = ENDIAN_CONVERT_16(pRegMap->ch[0].vpIdx);
    pRegMap->ch[1].vpIdx = ENDIAN_CONVERT_16(pRegMap->ch[1].vpIdx);
    pRegMap->ch[2].vpIdx = ENDIAN_CONVERT_16(pRegMap->ch[2].vpIdx);
    I2C_DataTxBufPrepare(sOpDataBuffer, sizeof(RegMap_t));

    return SYS_GOOD;
}

static int8_t OP_ReadChannelInfo(OpCmd_t *pCmd)
{
    RegChInfo_t *pChInfo = (RegChInfo_t *)&sOpDataBuffer;

    if (pCmd->chIdx >= CHANNEL_COUNT) {
        return SYS_INVALID_PARAM;
    }
    memcpy(sOpDataBuffer, sRegisterMap.ch + pCmd->chIdx, sizeof(RegChInfo_t));
#if DEBUG_OP
    LOGD(LOG_TAG, "RDINFO: CH%d - 0x%02x %d %04d\r\n",
            pCmd->chIdx,
            pChInfo->status.value,
            pChInfo->vol,
            pChInfo->vpIdx
        );
#endif
    // Little Endian(ISD9160 - ARM M0) -> Big Endian(CC2541 - 8051)
    pChInfo->vpIdx = ENDIAN_CONVERT_16(pChInfo->vpIdx);
    I2C_DataTxBufPrepare(sOpDataBuffer, sizeof(RegChInfo_t));

    return SYS_GOOD;
}

static int8_t OP_ReadVolume(OpCmd_t *pCmd)
{
    sOpDataBuffer[0] = sRegisterMap.vol;

    return SYS_GOOD;
}

static int8_t OP_ReadConfig(OpCmd_t *pCmd)
{
    // Copy config data to sOpDataBuffer
    // TODO
    return SYS_GOOD;
}


/* vim: set ts=4 sw=4 tw=0 list : */
