#ifndef __REG_H__
#define __REG_H__

#include <stdint.h>

#include "OP.h"

enum {
    POWER_MODE_DPD       = 0,
    POWER_MODE_SPD       = 1,
    POWER_MODE_DEEPSLEEP = 2,
    POWER_MODE_SLEEP     = 3,
    POWER_MODE_NORMAL    = 4,

    POWER_MODE_TOTAL
};

enum {
    CHANNEL_STATUS_IDLE     = 0,
    CHANNEL_STATUS_PLAY     = 1,
    CHANNEL_STATUS_PAUSE    = 2,
    CHANNEL_STATUS_COMPLETE = 3,

    CHANNEL_STATUS_TOTAL
};

enum {
    REG_CTL  = 0x00,    ///< RW, Ctrl
    REG_IFG  = 0x01,    ///< RO, Int Status Flag
    REG_VOL  = 0x02,    ///< RO, Volume
    REG_RFU  = 0x03,    ///< RO, Reserved
    REG_CH0  = 0x04,    ///< RO, Ch0 Status
    REG_CH1  = 0x08,    ///< RO, Ch1 Status
    REG_CH2  = 0x0C,    ///< RO, Ch2 Status

    REG_TOTAL  = 16
};

typedef struct {
    uint8_t  reset      : 1;    ///< CPU Reset
    uint8_t  softRst    : 1;    ///< Soft Reset
    uint8_t  powerMode  : 3;    ///< Power Mode: 000 - DPD, 001 - SPD, 010 - Deep Sleep, 011 - Sleep, 1xx - Normal
    uint8_t  rfu        : 2;
    uint8_t  intEn      : 1;    ///< 0 - Disable, 1 - Enable
} RegCtrl_t;

typedef struct {
    uint8_t  err        : 1;
    uint8_t  pm         : 1;
    uint8_t  vol        : 1;
    uint8_t  rfu        : 2;
    uint8_t  ch2        : 1;
    uint8_t  ch1        : 1;
    uint8_t  ch0        : 1;
} RegIntStatusFlag_t;

typedef struct {
    uint8_t  state      : 2;     ///< 00 - IDLE, 01 - PLAY, 10 - PAUSE, 11 - COMPLETED
    uint8_t  rfu        : 3;     ///< Reserved
    uint8_t  repeat     : 1;     ///< Loop play
    uint8_t  dominative : 1;     ///< 0 - Can be interrupt and reused, 1 - Not interruptable
    uint8_t  stopping   : 1;
} RegChStatus_t;

#define AUDIO_CHANNEL_MASK_STATE    (0x03 << 0)
#define AUDIO_CHANNEL_MASK_VOLUME   (0x0F << 2)

/// CC2541 (C8051 8-bit) and ISD9160 (ARM Cortex M0 32-bit)
/// share the same structure below, the data structure align
/// and endian are different in the 2 CPU architecture.
/// MUST pack the structure for the data transfer between
/// these 2 CPU.
#if defined (__ARMCC_VERSION)
#pragma pack(1)
#endif
typedef struct {
    union {
        RegChStatus_t   bits;
        uint8_t         value;
    } status;
    uint8_t     vol;             ///< 0 - MIN, 8 - MAX, 9~15 - NOT USE
    uint16_t    vpIdx;
} RegChInfo_t;

typedef RegChInfo_t     ChannelInfo_t;

#if defined (__ARMCC_VERSION)
#pragma pack(1)
#endif
typedef struct {
    RegCtrl_t           ctl;     ///<  1 Byte
    RegIntStatusFlag_t  ifg;     ///<  1 Byte
    uint8_t             vol;     ///<  1 Byte
    uint8_t             rfu;     ///<  1 Byte
    RegChInfo_t         ch[3];   ///< 12 Bytes
} RegMap_t;

int8_t Reg_Read(uint8_t addr, uint8_t *pBuf, uint8_t size);
int8_t Reg_Write(uint8_t addr, uint8_t *pBuf, uint8_t size);

void Reg_StatusChangeAlert(uint8_t alertBits);
void Reg_StatusClear(void);

void Reg_Init(void);

extern RegMap_t sRegisterMap;


#endif /* __REG_H__ */

/* vim: set ts=4 sw=4 tw=0 list : */
