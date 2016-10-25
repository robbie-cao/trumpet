/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/07/01 10:00p $
 * @brief    Access temperature alarm event via TALARM interrupt.
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ISD9100.h"

/* Config-Bit to start or stop the SysTick Timer */
#define SYSTICK_ENABLE              0    
/* Clocksource has the offset 2 in SysTick Control and Status Register */
#define SYSTICK_CLKSOURCE           2 
/* Waiting for 12M Xtal stalble */
#define SYSTEM_DELAY_US             5000

void TALARM_IRQHandler(void)
{
	/* Clear tempture alarm interrupt flag */
	TALARM_ClearIntFlag(BODTALM);

	printf("Temperature Sense event occurs\n");
}

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable External OSC49M */
    CLK_EnableXtalRC(CLK_PWRCTL_LXTEN_Msk);

	/* Assume the internal 22MHz RC used(waiting for 12Xtal stalble) */
    SysTick->LOAD = SYSTEM_DELAY_US * 22; 
    SysTick->VAL = (0x00);
    SysTick->CTRL = (1 << SYSTICK_CLKSOURCE) | (1<<SYSTICK_ENABLE);

    /* Waiting for down-count to zero(waiting for 12Xtal stalble) */
    while((SysTick->CTRL & (1 << 16)) == 0);

	/* Set Uart CLK source */
	CLK_SetModuleClock(UART_MODULE, NULL, CLK_CLKDIV0_UART(1));
	
    /* Enable IP clock */
    CLK_EnableModuleClock(UART_MODULE);
	
    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set GPG multi-function pins for UART0 RXD and TXD */
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA8MFP_Msk) ) | SYS_GPA_MFP_PA8MFP_UART_TX;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA9MFP_Msk) ) | SYS_GPA_MFP_PA9MFP_UART_RX;

    /* Lock protected registers */
    SYS_LockReg();
}

/* Main */
int main(void)
{
    /* Init System, IP clock and multi-function I/O */
    SYS_Init();	

    /* Init UART to 115200-8n1 for print message */
    UART_Open(UART0, 115200);

    printf("\n+------------------------------------------------------------------------+\n");
    printf("|                           TALAM Sample Code                            |\n");
    printf("+------------------------------------------------------------------------+\n");
    printf("| Temperature Alarm Sense Level = 105C                                   |\n");
    printf("+------------------------------------------------------------------------+\n");
    printf("\n");
	
	/* Initate tempture alarm & detect value */
	TALARM_Open(TALARM_TALMVL_105C);
	/* Enable interrupt */
	TALARM_EnableInt(BODTALM);
	NVIC_EnableIRQ(TALARM_IRQn);

    while(1);
}

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
