/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/07/15 2:52p $
 * @brief    ISD9100 CRC Driver Sample Code
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "isd9100.h"
#include "gpio.h"

#define MAX_PACKET_TEST 510
#define CRC_PKT 32

// CRC data should be aligned on word boundary
uint8_t CRCdata[MAX_PACKET_TEST+2];

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

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
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
/* MAIN function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
int main (void)
{
    int32_t  i,j,TestPassed=1;
		uint16_t CalcCRC;
    uint32_t *u32ptr;

		/* Lock protected registers */
    if(SYS->REGLCTL == 1) // In end of main function, program issued CPU reset and write-protection will be disabled.
        SYS_LockReg();

		/* Init System, IP clock and multi-function I/O
       In the end of SYS_Init() will issue SYS_LockReg()
       to lock protected register. If user want to write
       protected register, please issue SYS_UnlockReg()
       to unlock protected register if necessary */
    SYS_Init();

    /* Init UART0 for printf */
    UART_Init();



		printf("\n\n");
		printf("+----------------------------------------------------------------------+\n");
		printf("|                       CRC Driver Sample Code                         |\n");
		printf("|                                                                      |\n");
    printf("+----------------------------------------------------------------------+\n");
		printf("\n");
		printf("Enter any key to start test.\n");
		getchar();

		// Fill data array
		for (i=0; i<MAX_PACKET_TEST; i++)
			CRCdata[i]= rand();

    CRC_Open();

		// Test a 64byte CRC
		u32ptr = (uint32_t *)CRCdata;
    CRC_Init( CRC_LSB, CRC_PKT);
		CalcCRC = CRC_Calc( u32ptr, CRC_PKT);
    printf("\nCalculate CRC of ");
		for (i=0;i<CRC_PKT;i++)
			printf("%02x", CRCdata[i]);
   	printf("\nCRC %d bytes =  %04X\nAdd CRC to packet and recalculate to check, result should be zero\n",i, CalcCRC);
    
		// Test by adding CRC to packet and recalculating
		CRCdata[i++] = CalcCRC >> 8;
		CRCdata[i++] = CalcCRC & 0xFF;
		u32ptr = (uint32_t *)CRCdata;
    CRC_Init( CRC_LSB, CRC_PKT+2);
		CalcCRC = CRC_Calc( u32ptr, CRC_PKT+2);
		if (CalcCRC != 0)
			printf("CRC Failed check CRC=%04X\n",CalcCRC);
		else
			printf("CRC Passed check CRC=%04X\n",CalcCRC);
		printf("Calculate CRC of various packets and check results\n");
		for (j=2; j<=MAX_PACKET_TEST ; j+=2)
		{
      // Do CRC for j bytes
      u32ptr = (uint32_t *)CRCdata;
      CRC_Init( CRC_LSB, j);
			CalcCRC = CRC_Calc( u32ptr, j);

      printf(".");
      if (j%64==0)
				printf("\nCRC %d bytes %x \n",j, CalcCRC);
			// Now check CRC
      CRCdata[j++] =  CRC->CHECKSUM >> 8;
      CRCdata[j++] =  CRC->CHECKSUM & 0xff;
      u32ptr = (uint32_t *)CRCdata;
			CRC_Init( CRC_LSB, j);
			CalcCRC = CRC_Calc( u32ptr, j);
      j-=2;
			if (CalcCRC != 0)
			{
				printf("\nCRC Fail %d bytes %x ",j, CRC->CHECKSUM);
				TestPassed =0;
			}
      CRCdata[j++] = rand();
      CRCdata[j++] = rand();
      j-=2;
		}
		if (TestPassed)
			printf("\n\nCRC driver sample passed all tests.\n");
		else
			printf("\n\nCRC driver sample failed.\n");

		CRC_Close();
}


