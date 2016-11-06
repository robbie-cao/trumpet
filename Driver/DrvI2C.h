#ifndef __DRVI2C_H__
#define __DRVI2C_H__

#include "mtypes.h"

typedef enum {
    // I2C_MASTER mode statuses.
    mstStarted   = 0x08,
    mstRepStart  = 0x10,
    mstAddrAckW  = 0x18,
    mstAddrNackW = 0x20,
    mstDataAckW  = 0x28,
    mstDataNackW = 0x30,
    mstLostArb   = 0x38,
    mstAddrAckR  = 0x40,
    mstAddrNackR = 0x48,
    mstDataAckR  = 0x50,
    mstDataNackR = 0x58,

    // I2C_SLAVE mode statuses.
    slvAddrAckR  = 0x60,
    mstLostArbR  = 0x68,
    slvAddrAckG  = 0x70,
    mstLostArbG  = 0x78,
    slvDataAckR  = 0x80,
    slvDataNackR = 0x88,
    genDataAckR  = 0x90,
    genDataNackR = 0x98,
    slvStopped   = 0xA0,
    slvAddrAckW  = 0xA8,
    mstLostArbW  = 0xB0,
    slvDataAckW  = 0xB8,
    slvDataNackW = 0xC0,
    slvLastAckW  = 0xC8,

    // I2C_MASTER/SLAVE mode statuses.
    i2cIdle      = 0xF8
} i2cStatus_t;

void    I2C_DataReceiveidHandler(void);
uint8_t I2C_DataReceived(void);
void    I2C_DataClear(void);
void    I2C_DataTxBufPrepare(uint8_t *pSrc, uint8_t len);

void InitialI2C(void);

#endif

/* vim: set ts=4 sw=4 tw=0 list : */
