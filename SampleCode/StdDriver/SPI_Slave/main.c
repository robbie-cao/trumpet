/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/08/01 10:00p $
 * @brief    ISD9100 SPI slave demo sample source.
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
    /* Configure SPI1 as a slave, MSB first, SPI Mode-0 timing, clock is from master provide */
    SPI_Open(SPI0, SPI_SLAVE, SPI_MODE_0, 100000, 0 );

	SPI_SET_MSB_FIRST(SPI0);
	
	SPI_SET_SUSPEND_CYCLE(SPI0,0);
	
	SPI_DISABLE_BYTE_REORDER(SPI0);
	
	SPI_SET_DATA_WIDTH(SPI0,32);
	
	SPI_SET_TX_NUM(SPI0,SPI_TXNUM_ONE);
	
    /* Disable the automatic hardware slave select function */
	SPI_DISABLE_AUTOSS(SPI0);
	
	SPI_SET_SS(SPI0,SPI_SS0);
	
	SPI_SET_SLAVE_ACTIVE_LEVEL(SPI0,SPI_SS_ACTIVE_LOW);
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
	printf("Configure SPI1 as a slave.\n");
	printf("Pin define: PB1=SSB0, PB2=SCLK, PB3=MISO1, PB4=MOSI1\n");
    printf("SPI controller will transfer %d data to a off-chip master device.\n", TEST_COUNT);
    printf("In the meanwhile the SPI controller will receive %d data from the off-chip master device.\n", TEST_COUNT);
    printf("After the transfer is done, the %d received data will be printed out.\n", TEST_COUNT);
    printf("The SPI slave configuration is ready.\n");

    for(u32Counter=0; u32Counter<TEST_COUNT; u32Counter++)
    {
        g_au32SourceData[u32Counter] = 0xAA55AA00 + u32Counter;
        g_au32DestinationData[u32Counter] = 0;
    }

    printf("<<Press any key if the master device configuration is ready>>\n");
    getchar();
    printf("\n");
	
	/* Wait for transfer done */
    for(u32Counter=0; u32Counter<TEST_COUNT; u32Counter++)
	{
		SPI_WRITE_TX0( SPI0, g_au32SourceData[u32Counter] );
		SPI_GO(SPI0);
		while( SPI_IS_BUSY(SPI0) );
		g_au32DestinationData[u32Counter] = SPI_READ_RX0(SPI0);
	}
	
    printf("Received data:\n");
    for(u32Counter=0; u32Counter<TEST_COUNT; u32Counter++)
        printf("%d:\t0x%X\n", u32Counter, g_au32DestinationData[u32Counter]);
	
    printf("\nThe data transfer was done.\n"); 
    printf("\nExit SPI driver slave sample code.\n");
    while(1);
}
