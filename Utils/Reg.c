#include "ISD9160.h"
#include "GPIO.h"
#include "SYS.h"
#include "OP.h"
#include "Reg.h"

RegMap_t sRegisterMap;

int8_t Reg_Read(uint8_t addr, uint8_t *pBuf, uint8_t size)
{
    if (addr >= REG_TOTAL || addr + size >= REG_TOTAL) {
        return SYS_INVALID_PARAM;
    }

    memcpy(pBuf, ((uint8_t *)&sRegisterMap) + addr, size);

    return SYS_GOOD;
}

int8_t Reg_Write(uint8_t addr, uint8_t *pBuf, uint8_t size)
{
    if (addr >= REG_TOTAL || addr + size >= REG_TOTAL) {
        return SYS_INVALID_PARAM;
    }

    memcpy(((uint8_t *)&sRegisterMap) + addr, pBuf, size);

    return SYS_GOOD;
}

void Reg_StatusChangeAlert(uint8_t alertBits)
{
    *(uint8_t *)&sRegisterMap.ifg |= alertBits;
    DrvGPIO_ClrBit(GPA, STATUS_ALERT_PIN);
}

void Reg_StatusClear(void)
{
    *(uint8_t *)&sRegisterMap.ifg = 0;
    DrvGPIO_SetBit(GPA, STATUS_ALERT_PIN);
}


void Reg_Init(void)
{
    memset(&sRegisterMap, 0, sizeof(sRegisterMap));

    sRegisterMap.ctl.powerMode = ISD9160_POWER_STATE_NORMAL;
    sRegisterMap.ctl.intEn = 1;
    sRegisterMap.vol = VOLUME_DEFAULT;
}

/* vim: set ts=4 sw=4 tw=0 list : */
