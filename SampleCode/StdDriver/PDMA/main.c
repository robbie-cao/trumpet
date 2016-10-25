/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/07/15 2:52p $
 * @brief    ISD9100 Series PDMA Driver Sample Code
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ISD9100.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/

#define TEST_CH     3 // 0~3

uint32_t PDMA_TEST_LENGTH = 64;
uint8_t SrcArray[256];
uint8_t DestArray[256];
uint32_t volatile u32IsTestOver = 0xFF;

void PDMA_IRQHandler(void)
{
    uint32_t status = PDMA_GET_INT_STATUS();

    if(status & 0x1)    /* CH0 */
    {
        if(PDMA_GET_CH_INT_STS(0) & 0x2)
            u32IsTestOver = 0;
        PDMA_CLR_CH_INT_FLAG(0, PDMA_CHIF_TXOKIF_Msk);
    }
    else if(status & 0x2)      /* CH1 */
    {
        if(PDMA_GET_CH_INT_STS(1) & 0x2)
            u32IsTestOver = 1;
        PDMA_CLR_CH_INT_FLAG(1, PDMA_CHIF_TXOKIF_Msk);
    }
    else if(status & 0x4)      /* CH2 */
    {
        if(PDMA_GET_CH_INT_STS(2) & 0x2)
            u32IsTestOver = 2;
        PDMA_CLR_CH_INT_FLAG(2, PDMA_CHIF_TXOKIF_Msk);
    }
    else if(status & 0x8)      /* CH3 */
    {
        if(PDMA_GET_CH_INT_STS(3) & 0x2)
            u32IsTestOver = 3;
        PDMA_CLR_CH_INT_FLAG(3, PDMA_CHIF_TXOKIF_Msk);
    }
    else
        printf("unknown interrupt !!\n");
}

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable External XTL32K */
    CLK_EnableXtalRC(CLK_PWRCTL_LXTEN_Msk);

    /* Enable External OSC49M */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
	
    /* Enable External OSC10K */
    CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);
	
    /* Switch HCLK clock source to HXT */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_48M, CLK_CLKDIV0_HCLK(1));

    /* Enable IP clock */
    CLK_EnableModuleClock(UART_MODULE);
		CLK_EnableModuleClock(PDMA_MODULE);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
    SystemCoreClockUpdate();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set GPA multi-function pins for UART0 RXD and TXD */
		SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA8MFP_Msk) ) | SYS_GPA_MFP_PA8MFP_UART_TX;
		SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA9MFP_Msk) ) | SYS_GPA_MFP_PA9MFP_UART_RX;

    /* Lock protected registers */
    SYS_LockReg();
}

void UART_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UR                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset IP */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 Baudrate(115200) */
    UART_Open( UART0,115200 );
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
		int32_t i;
	
    /* Init System, IP clock and multi-function I/O
       In the end of SYS_Init() will issue SYS_LockReg()
       to lock protected register. If user want to write
       protected register, please issue SYS_UnlockReg()
       to unlock protected register if necessary */

    /* Init System, IP clock and multi-function I/O */
    SYS_Init();

    /* Init UART0 for printf */
    UART_Init();

    printf("\n\nCPU @ %dHz <%d>\n", SystemCoreClock, TEST_CH);

    printf("+--------------------------------------+ \n");
    printf("|    ISD9100 PDMA Driver Sample Code   | \n");
    printf("+--------------------------------------+ \n");
	
		memset(SrcArray,0,256);
		memset(DestArray,0,256);
		for (i=0;i<256;i++) SrcArray[i]=i;

    /* Open Channel TEST_CH */
    PDMA_Open(1 << TEST_CH);
    PDMA_SetTransferCnt(TEST_CH, PDMA_WIDTH_32, PDMA_TEST_LENGTH);
    PDMA_SetTransferAddr(TEST_CH, (uint32_t)SrcArray, PDMA_SAR_INC, (uint32_t)DestArray, PDMA_DAR_INC);
    PDMA_EnableInt(TEST_CH, PDMA_INTENCH_TXOKIEN_Msk);
    NVIC_EnableIRQ(PDMA_IRQn);
    u32IsTestOver = 0xFF;
    PDMA_Trigger(TEST_CH);
    while(u32IsTestOver == 0xFF);

    if(u32IsTestOver == TEST_CH)
		{
			for (i=0;i<256;i++)
			{
				if (SrcArray[i]!=DestArray[i])
				{
					printf("data compare error...\n");
					while (1);
				}
			}
      printf("test done...\n");
			PDMA_Close();
			while (1);
		}

		while (1);
}
