#ifndef __OP_H__
#define __OP_H__

#include "mtypes.h"

#define ENDIAN_CONVERT_16(x)        ((((x) & 0xFF) << 8) | (((x) & 0xFF00) >> 8))
#define ENDIAN_CONVERT_32(x)        ((ENDIAN_CONVERT_16((x) & 0xFFFF) << 16) | (ENDIAN_CONVERT_16(((x) >> 16) & 0xFFFF)))


#define SYS_TICK_INT_RATE           100

#define SYS_GOOD                    0x00    //
#define SYS_ERR                     0xFF    //
#define SYS_BUS_BUSY                0xFE    //
#define SYS_NOT_SUPPORT             0xFD    //
#define SYS_INVALID_PARAM           0xFC    //

#define SYS_IDLE                    0x80
#define SYS_PLAYING                 0x81
#define SYS_PAUSE                   0x82
#define SYS_BUSY                    0x83

/* System */
#define OP_SYS_RESET                0x10
#define OP_SYS_SOFTRESET            0x11
#define OP_SYS_SETPOWERMODE         0x12
#define OP_SYS_SETINTEN             0x13
#define OP_SYS_CONFIG               0x14
#define OP_SYS_OPEN                 0x15
#define OP_SYS_CLOSE                0x16

#define OP_SYS_SELFTEST             0x1E
#define OP_SYS_DUMP                 0x1F

/* Data/Flash */
#define OP_DATA_READ                0x20    // Read data from address A
#define OP_DATA_WRITE               0x21    // Write data to address A
#define OP_DATA_ERASE               0x22    // Erase data at address A

#define OP_FLASH_READ               0x28    // Read data from flash address A
#define OP_FLASH_WRITE              0x29    // Write data to flash address A
#define OP_FLASH_ERASE              0x2A    // Erase data at flash address A

/* Play */
#define OP_PLAY_VP                  0x30
#define OP_PLAY_VP_LOOP             0x31    // Can be merge to OP_PLAY_VP with param - TODO
#define OP_PLAY_VM                  0x32
#define OP_PLAY_VM_LOOP             0x33
#define OP_PLAY_VM_INDIRECT         0x34
#define OP_PLAY_SILENCE             0x35
#define OP_PLAY_SILENCE_LOOP        0x36
#define OP_PLAY_INSERT_SILENCE      0x37

/* Play Back */
#define OP_PB_STOP                  0x40
#define OP_PB_REPLAY                0x41
#define OP_PB_PAUSE                 0x42
#define OP_PB_RESUME                0x43
#define OP_PB_REPEAT                0x44
#define OP_PB_PLAY_PAUSE_RESUME     0x45    // Play current index if no playing, pause if playing, resume if pause
#define OP_PB_STOP_ALL              0x46
#define OP_PB_PAUSE_ALL             0x47

#define OP_PB_CANCEL_LAST_ONE       0x4D
#define OP_PB_CANCEL_ALL            0x4E
#define OP_PB_CLEAR_ALL             0x4F

/* Volume */
#define OP_VOLUME_SET               0x50
#define OP_VOLUME_UP                0x51
#define OP_VOLUME_DOWN              0x52
#define OP_VOLUME_MUTE              0x53
#define OP_VOLUME_UNMUTE            0x54
#define OP_VOLUME_SET_ALL           0x55
#define OP_VOLUME_UP_ALL            0x56
#define OP_VOLUME_DOWN_ALL          0x57

/* Power Management */
#define OP_PM_POWER_UP              0x60
#define OP_PM_POWER_DOWN            0x61
#define OP_PM_SPD                   0x62    // Standby Power Down
#define OP_PM_DPD                   0x63    // Deep Power Down
#define OP_PM_DS                    0x64    // Deep Sleep
#define OP_PM_STOP                  0x65    // Stop
#define OP_PM_WAKEUP                0x66    // Wakeup

/* Read/Check Status */
#define OP_READ_CHIPID              0x70
#define OP_READ_INT                 0x71
#define OP_READ_STATUS              0x72
#define OP_READ_CHANNEL_INFO        0x73
#define OP_READ_VOL                 0x74
#define OP_READ_CONFIG              0x75

#define OP_CHECK_DEVICE_STATUS      0x78
#define OP_CHECK_JOB_QUEUE          0x79
#define OP_CHECK_FLASH_TYPE         0x7A
#define OP_CHECK_FLASH_STATUS       0x7B

enum {
    ISD9160_POWER_STATE_DPD = 0,
    ISD9160_POWER_STATE_SPD,
    ISD9160_POWER_STATE_DEEPSLEEP,
    ISD9160_POWER_STATE_SLEEP,
    ISD9160_POWER_STATE_NORMAL,

    ISD9160_POWER_STATE_TOTAL
};

enum {
    AUDIO_CHANNEL_STATE_IDLE = 0,
    AUDIO_CHANNEL_STATE_PLAYING,
    AUDIO_CHANNEL_STATE_PAUSE,
    AUDIO_CHANNEL_STATE_COMPLETED,    // Intermediate state between PLAY and IDLE

    AUDIO_CHANNEL_STATE_TOTAL
};

// CC2541 (C8051 8-bit) and ISD9160 (ARM Cortex M0 32-bit)
// share the same structure below, the data structure align
// and endian are different in the 2 CPU architecture.
// MUST pack the structure for the data transfer between
// these 2 CPU.
#if defined (__ARMCC_VERSION)
#pragma pack(1)
#endif
typedef struct {
    uint8_t  len;
    uint8_t  cmd;
    uint8_t  data;
    uint8_t  chIdx;
    uint16_t vpIdx;
    uint8_t  extra[2];
    uint16_t crc;
} OpCmd_t;


typedef int8_t (*OpCmdHandler_t)(OpCmd_t *pCmd);

typedef struct {
    uint8_t         cmd;
    OpCmdHandler_t  handler;
} OpCmdHandlerItem_t;

int8_t OP_Handler(OpCmd_t *pCmd);

#endif /* __OP_H__ */

/* vim: set ts=4 sw=4 tw=0 list : */
