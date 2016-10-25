/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/08/01 10:00p $
 * @brief    ISD9100 SPI master demo sample source.
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "ISD9100.h"

#define TEST_COUNT 			(16)	    /* Test data count */

__align(4) uint32_t g_au32SourceData[TEST_COUNT];
__align(4) uint32_t g_au32DestinationData[TEST_COUNT];

void SPI_Init()
{
    /* Configure SPI0 as a master, MSB first, SPI Mode-0 timing, clock is 1MHz */
    SPI_Open(SPI0, SPI_MASTER, SPI_MODE_0, 100000,0);

	SPI_SET_MSB_FIRST(SPI0);
	
	SPI_SET_SUSPEND_CYCLE(SPI0,8);
	
	SPI_DISABLE_BYTE_REORDER(SPI0);
	
	SPI_SET_DATA_WIDTH(SPI0,32);
	
	SPI_SET_TX_NUM(SPI0,SPI_TXNUM_ONE);
	
    /* Enable the automatic hardware slave select function */
    SPI_ENABLE_AUTOSS(SPI0,SPI_SS0,SPI_SS_ACTIVE_LOW);
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

	/* LDO On */
	CLK->APBCLK0 |= CLK_APBCLK0_ANACKEN_Msk;
	ANA->LDOPD &= ~ANA_LDOPD_PD_Msk;
	ANA->LDOSEL = 3; 
	
    /* Enable IP clock */
    CLK_EnableModuleClock(UART_MODULE);
    CLK_EnableModuleClock(SPI0_MODULE);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* SPI0: GPA0=MOSI0, GPA1=SLCKMOSI0, GPA2=SSB0, GPA3=MISO0 */
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA0MFP_Msk) ) | SYS_GPA_MFP_PA0MFP_SPI_MOSI0;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA1MFP_Msk) ) | SYS_GPA_MFP_PA1MFP_SPI_SCLK;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA2MFP_Msk) ) | SYS_GPA_MFP_PA2MFP_SPI_SSB0;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA3MFP_Msk) ) | SYS_GPA_MFP_PA3MFP_SPI_MISO0;	

    /* Set GPG multi-function pins for UART0 RXD and TXD */
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA8MFP_Msk) ) | SYS_GPA_MFP_PA8MFP_UART_TX;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA9MFP_Msk) ) | SYS_GPA_MFP_PA9MFP_UART_RX;

	/* Reset IP module */
	SYS_ResetModule(SPI0_RST);
	SYS_ResetModule(UART0_RST);
	
    /* Lock protected registers */
    SYS_LockReg();
}

int main()
{
	uint32_t u32Counter = 0;
	
    /* Init System, IP clock and multi-function I/O */
    SYS_Init();
	
	/* Initiate SPI */
	SPI_Init();
	
    /* Init UART to 115200-8n1 for print message */
    UART_Open(UART0, 115200);
	
    printf("\n+------------------------------------------------------------------------+\n");
    printf("|                       SPI Driver Sample Code                           |\n");
    printf("+------------------------------------------------------------------------+\n");
    printf("Configure SPI0 as a master.\n");
    printf("SPI clock rate: %d Hz\n", SPI_GetBusClock(SPI0));
    printf("SPI controller will transfer %d data to a off-chip slave device.\n", TEST_COUNT);
    printf("In the meanwhile the SPI controller will receive %d data from the off-chip slave device.\n", TEST_COUNT);
    printf("After the transfer is done, the %d received data will be printed out.\n", TEST_COUNT);
    printf("The SPI master configuration is ready.\n");

    for(u32Counter=0; u32Counter<TEST_COUNT; u32Counter++)
    {
        g_au32SourceData[u32Counter] = 0x55AA5500 + u32Counter;
        g_au32DestinationData[u32Counter] = 0;
    }

    printf("Before starting the data transfer, make sure the slave device is ready.\n");
    printf("<<Press any key to start the transfer>>\n");
    getchar();
    printf("\n");
	
	/* Wait for transfer done */
    for(u32Counter=0; u32Counter<TEST_COUNT; u32Counter++)
	{
		SPI_WRITE_TX0( SPI0, g_au32SourceData[u32Counter] );
		SPI_GO(SPI0);
		while( SPI_IS_BUSY(SPI0) && SPI_GET_RX_FIFO_FULL_FLAG(SPI0) );
		g_au32DestinationData[u32Counter] = SPI_READ_RX0(SPI0);
	}
	
    printf("Received data:\n");
    for(u32Counter=0; u32Counter<TEST_COUNT; u32Counter++)
        printf("%d:\t0x%X\n", u32Counter, g_au32DestinationData[u32Counter]);
	
    printf("\nThe data transfer was done.\n"); 
    printf("\nExit SPI driver master sample code.\n");
    while(1);
}
