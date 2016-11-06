/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/07/17 10:00p $
 * @brief    Uart driver demo sample.
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "ISD9100.h"

#include "DrvUart.h"

/* Buffer size, this buffer for uart receive & send data. */
#define UART_BUF_SIZE      64

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

#if 0
    /* Enable IP clock */
    CLK_EnableModuleClock(UART_MODULE);
#endif

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
	uint8_t u8Buffer[UART_BUF_SIZE];
	
    /* Init System, IP clock and multi-function I/O */
    SYS_Init();

#if 0
    /* Init UART to 115200-8n1 for print message */
    UART_Open(UART0, 115200);
#endif
    UART_Init();

    printf("\n+------------------------------------------------------------------------+\n");
    printf("|                    ISD9100 Uart Demo Sample                            |\n");
    printf("+------------------------------------------------------------------------+\n");
	printf("Press any key to test.\n");
	
	while(1)
	{
#if 0
		memset( u8Buffer, '\0', sizeof(u8Buffer) );
		if( UART_Read( UART0, u8Buffer, sizeof(u8Buffer) ) > 0 )
			UART_Write( UART0, u8Buffer, sizeof(u8Buffer) );
#endif
	}
}

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
