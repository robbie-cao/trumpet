/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/07/13 03:00p $
 * @brief    Driver BOD demo function.
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "ISD9100.h"

void BOD_IRQHandler(void)
{
	uint32_t u32i;
	
	/* Clear interrupt flag */
    BOD_ClearIntFlag(BODTALM);
	
	if(BOD_GetOutput(BODTALM) == 1)
	{
		PB->DOUT |= BIT0;
		for( u32i=0; u32i<0xffff; u32i++);
		PB->DOUT &= (~BIT0);
		for( u32i=0; u32i<0xffff; u32i++);
	}	
}

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable External OSC49M */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Switch HCLK clock source to HXT */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_48M, CLK_CLKDIV0_HCLK(1));

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
	uint32_t u32Config0;

    /* Init System, IP clock and multi-function I/O */
    SYS_Init();

    /* Init UART to 115200-8n1 for print message */
    UART_Open(UART0, 115200);

	/* Set GPIOB Bit0 is ouput mode for displaying square wave */
	GPIO_SetMode( PB, BIT0, GPIO_MODE_OUTPUT);
	
	/* Initiate FMC and read config. */
	SYS_UnlockReg();
	FMC_Open();
	FMC_ReadConfig( &u32Config0, 1 );
	
    printf("\n+------------------------------------------------------------------------+\n");
    printf("|                      BOD Driver Sample Code                            |\n");           
    printf("+------------------------------------------------------------------------+\n");
    printf("|                     Brown out detection 3.0V                           |\n");
    printf("+------------------------------------------------------------------------+\n");
    printf("\n");
	
	/* Check config state */
	if(((u32Config0>>23)&0x01) == 0)
	{
		FMC_EnableConfigUpdate();
		/* Enable BOD */
		u32Config0 |= 0x00800000;
		FMC_WriteConfig( &u32Config0, 1 );
		printf("Set Config0[23] = 1, Please reset again....\n"); 
		/* wait chip reset */
		while(1);  
	}
	
	printf("If power supply lower than 3.0V, the GPB0 will ouput square wave. \n");
	
    /*---------------------------------------------------------------------------------------------------------*/
    /* Configure BOD                                                                                           */
    /*---------------------------------------------------------------------------------------------------------*/
	BOD_Open( BOD_BODEN_TIME_MULTIPLEXED, BOD_BODVL_30V);
	/* Enable hysteresis */
	BOD_EnableHyst(BODTALM);
	/* ON: 400us, OFF: 25.6ms */
	BOD_SetDetectionTime(3, 255);
	/* Enable interrupt */
	BOD_EnableInt(BODTALM);
	
    while(1);
}

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
