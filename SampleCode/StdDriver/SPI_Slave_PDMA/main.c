/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/08/01 10:00p $
 * @brief    Demonstrate the usage of PDMA transfer. 
 *           SPI interface is use as a host. 
 *           Totally 2 PDMA channels are used in this sample
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "ISD9100.h"

#define PDMA_TEST_COUNT (16)	    /* Test data count */

__align(4) uint32_t g_au32SourceData[PDMA_TEST_COUNT];
__align(4) uint32_t g_au32DestinationData[PDMA_TEST_COUNT];

uint32_t volatile u32IsTestOver = 0;

void PDMA_IRQHandler(void)
{
	uint32_t u32Status = PDMA_GET_INT_STATUS();
	
	/* CH0 */
    if (u32Status & (1<<0)) 
	{ 
        if (PDMA_GET_CH_INT_STS(0) & 0x2)
            u32IsTestOver |= (1<<0);
        PDMA_CLR_CH_INT_FLAG( 0, PDMA_CHIF_TXOKIF_Msk );
	}
	/* CH1 */
    else if (u32Status & (1<<1)) 
	{ 
        if (PDMA_GET_CH_INT_STS(1) & 0x2)
            u32IsTestOver |= (1<<1);
        PDMA_CLR_CH_INT_FLAG( 1, PDMA_CHIF_TXOKIF_Msk );
	}
	else
		printf( "unknowen or unconfigurate interrupt." );
}

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
	CLK_EnableModuleClock(PDMA_MODULE);

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
	SYS_ResetModule(PDMA_RST);
	
    /* Lock protected registers */
    SYS_LockReg();
}

void ConfigurePDMAChannel(void)
{
	uint32_t u32EndSrc, u32EndDst;
	
	/* Open Channel 1 for SPI0 TX, Channel 2 for SPI0 RX */
	PDMA_Open( 1 << 0 );
	PDMA_Open( 1 << 1 );
	
	PDMA_SetTransferMode(0, PDMA_SPI0_RX, 0, 0);
	PDMA_SetTransferMode(1, PDMA_SPI0_TX, 0, 0);
	
    /* Configure Channel 1 */
    PDMA_SetTransferCnt(1, PDMA_WIDTH_32, PDMA_TEST_COUNT);
    u32EndSrc = (uint32_t)g_au32SourceData;
    u32EndDst = (uint32_t)&(SPI0->TX0);
    PDMA_SetTransferAddr(1, u32EndSrc, PDMA_SAR_INC, u32EndDst, PDMA_DAR_FIX);
	PDMA1->DSCT_CTL = (PDMA1->DSCT_CTL&(~PDMA_DSCT_CTL_MODESEL_Msk)) | (2<<PDMA_DSCT_CTL_MODESEL_Pos);
    
    /* Configure Channel 0 */
    PDMA_SetTransferCnt(0, PDMA_WIDTH_32, PDMA_TEST_COUNT);
    u32EndSrc = (uint32_t)&(SPI0->RX0);
    u32EndDst = (uint32_t)g_au32DestinationData;
    PDMA_SetTransferAddr(0, u32EndSrc, PDMA_SAR_FIX, u32EndDst, PDMA_DAR_INC);
	PDMA0->DSCT_CTL = (PDMA0->DSCT_CTL&(~PDMA_DSCT_CTL_MODESEL_Msk)) | (1<<PDMA_DSCT_CTL_MODESEL_Pos);
    
	PDMA_EnableInt(1, PDMA_INTENCH_TXOKIEN_Msk);
	PDMA_EnableInt(0, PDMA_INTENCH_TXOKIEN_Msk);
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
    printf("|                    SPI Driver Sample Code with PDMA                    |\n");
    printf("+------------------------------------------------------------------------+\n");
	printf("Configure SPI1 as a slave.\n");
	printf("Pin define: PB1=SSB0, PB2=SCLK, PB3=MISO1, PB4=MOSI1\n");
    printf("SPI controller will transfer %d data to a off-chip master device.\n", PDMA_TEST_COUNT);
    printf("In the meanwhile the SPI controller will receive %d data from the off-chip master device.\n", PDMA_TEST_COUNT);
    printf("After the transfer is done, the %d received data will be printed out.\n", PDMA_TEST_COUNT);
    printf("The SPI slave configuration is ready.\n");

    for(u32Counter=0; u32Counter<PDMA_TEST_COUNT; u32Counter++)
    {
        g_au32SourceData[u32Counter] = 0xAA55AA00 + u32Counter;
        g_au32DestinationData[u32Counter] = 0;
    }

    printf("<<Press any key if the master device configuration is ready>>\n");
    getchar();
    printf("\n");
	
	ConfigurePDMAChannel();
	
    /* Enable PDMA IRQ */
    NVIC_EnableIRQ(PDMA_IRQn);
	
    /* SPI Trigger by PDMA */
    SPI_TRIGGER_RX_PDMA(SPI0);
    SPI_TRIGGER_TX_PDMA(SPI0);
	
	/* Trigger PDMA */
	PDMA_Trigger(0);
	PDMA_Trigger(1);	
	
    /* Wait for transfer done */
    while(u32IsTestOver<2);

    /* Disable PDMA interrupt */
	NVIC_DisableIRQ(PDMA_IRQn);
	
    printf("Received data:\n");
    for(u32Counter=0; u32Counter<PDMA_TEST_COUNT; u32Counter++)
        printf("%d:\t0x%X\n", u32Counter, g_au32DestinationData[u32Counter]);
	
    printf("\nThe data transfer was done.\n"); 
    printf("\nExit SPI driver slave sample code.\n");
    while(1);
}
