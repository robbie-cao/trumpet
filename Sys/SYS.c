#include "ISD9160.h"
#include "SYS.h"


void Sys_Reset(void)
{
    DrvSYS_ResetCPU();
}

void Sys_SoftReset(void)
{
    // TODO
}

void Sys_Config(uint8_t id, uint8_t data)
{
}

void Sys_Open(void)
{
}

void Sys_Close(void)
{
}


void Sys_SelfTest(void)
{
}

void Sys_Dump(uint8_t item)
{
}


void Sys_ReadChipId(uint8_t* pBuf)
{
}

void Sys_ReadInt(uint8_t* pBuf)
{
}

void Sys_ReadStatus(uint8_t* pBuf)
{
}

void Sys_ReadChannelStatus(uint8_t* pBuf, uint8_t ch)
{
}

void Sys_ReadVol(uint8_t* pBuf)
{
}

void Sys_ReadConfig(uint8_t* pBuf, uint8_t id)
{
}


void Sys_CheckDeviceStatus(uint8_t* pBuf)
{
}

void Sys_CheckJobQueue(uint8_t* pBuf)
{
}

void Sys_CheckFlashType(uint8_t* pBuf)
{
}

void Sys_CheckFlashStatus(uint8_t* pBuf)
{
}

void Sys_TimerDelay(uint32_t us)
{
    SysTick->LOAD = us * 49; /* Assume the internal 49MHz RC used */    // TODO: 49 -> define
    SysTick->VAL  = (0x00);
    SysTick->CTRL = (1 << SYSTICK_CLKSOURCE) | (1 << SYSTICK_ENABLE);

    /* Waiting for down-count to zero */
    while ((SysTick->CTRL & (1 << 16)) == 0);
}

/*---------------------------------------------------------------------------------------------------------*/
/* InitialSystemClock                                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
void InitialSystemClock(void)
{
    /* Unlock the protected registers */
    UNLOCKREG();

    /* HCLK clock source. */
    DrvSYS_SetHCLKSource(0);

    LOCKREG();

    /* HCLK clock frequency = HCLK clock source / (HCLK_N + 1) */
    DrvSYS_SetClockDivider(E_SYS_HCLK_DIV, 0);
}

/* vim: set ts=4 sw=4 tw=0 list : */
