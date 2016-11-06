#include "ISD9160.h"
#include "PM.h"
#include "SpiFlash.h"

#define USE_DRV_PMU_API     0


void PM_PowerUp(void)
{
}

void PM_PowerDown(void)
{
}

static void SetSpiFlashPowerDown(void)
{
    SpiFlash_Open();        // LDO on or external power provided to SPI flash
    SpiFlash_PowerDown();   // LDO on or external power provided to SPI flash
}

/**
 * Standby Power Down
 */
void PM_StandbyPowerDown(void)
{
#if USE_DRV_PMU_API
    DrvPMU_StandbyPowerDown();
#else
    UNLOCKREG();
    SCB->SCR = SCB_SCR_SLEEPDEEP;
    SYSCLK->PWRCON.STANDBY_PD = 1;
    SYSCLK->PWRCON.STOP       = 0;
    SYSCLK->PWRCON.DEEP_PD    = 0;
    LOCKREG();
    __wfi();
#endif
}

/**
 * Deep Power Down
 * !!! CAN NOT WORK WHEN ICE (SWD) CONNECTED
 */
void PM_DeepPowerDown(void)
{
    // GPA0-GPA7 belong to LDO power plane, if LDO is off, and external pwoer is used for LDO power plane
    // Need to handle the circuit for external device on correct state after power down.
    // Like pull-high on  CS & CLK pin of SPI-flash.
    // All GPA0~GPA7 are in tri-state (high impedance) no matter LDO is on or off
    SetSpiFlashPowerDown();

#if USE_DRV_PMU_API
    DrvPMU_DeepPowerDown(DPDWAKEUPMODE_PINOSC10KWAKEUP, 0);
#else
    UNLOCKREG();
    SCB->SCR = SCB_SCR_SLEEPDEEP;
    SYSCLK->PWRCON.OSC10K_EN  = 0;
    SYSCLK->PWRCON.OSC10K_ENB = 1;        // Disable 10K OSC wakeup
    SYSCLK->PWRCON.PIN_ENB    = 0;
    SYSCLK->PWRCON.STANDBY_PD = 0;
    SYSCLK->PWRCON.STOP       = 0;
    SYSCLK->PWRCON.DEEP_PD    = 1;

    LOCKREG();
    __wfi();
#endif
}

void PM_DeepSleep(void)
{
    /* HCLK clock frequency = HCLK clock source / (HCLK_N + 1) */
    DrvSYS_SetClockDivider(E_SYS_HCLK_DIV, 15);
}

void PM_Stop(void)
{
#if USE_DRV_PMU_API
    DrvPMU_Stop();
#else
    UNLOCKREG();
    SCB->SCR = SCB_SCR_SLEEPDEEP;
    SYSCLK->PWRCON.STOP = 1;
    SYSCLK->PWRCON.STANDBY_PD = 0;
    SYSCLK->PWRCON.DEEP_PD = 0;

    LOCKREG();
    __wfi();
#endif
}

void PM_Wakeup(void)
{
    /* HCLK clock frequency = HCLK clock source / (HCLK_N + 1) */
    DrvSYS_SetClockDivider(E_SYS_HCLK_DIV, 0);
}

/* vim: set ts=4 sw=4 tw=0 list : */
