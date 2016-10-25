
/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/07/17 5:12p $
 * @brief    Demonstrate the usage of Timer driver by delay time, 
 *           callback function etc.
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "ISD9100.h"

volatile uint8_t u8Counter = 0;

void TMR0_IRQHandler(void)
{
    printf("Timer IRQ handler test #%d/3.\n", ++u8Counter );
    TIMER_ClearIntFlag(TIMER0);	
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
    CLK_EnableModuleClock(TMR0_MODULE);

    /* Select IP clock source */
    CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HCLK, 0);

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

int main(void)
{
	uint8_t u8Option = 0;
	
    /* Init System, IP clock and multi-function I/O
       In the end of SYS_Init() will issue SYS_LockReg()
       to lock protected register. If user want to write
       protected register, please issue SYS_UnlockReg()
       to unlock protected register if necessary */
    SYS_Init();

    /* Init UART to 115200-8n1 for print message */
    UART_Open(UART0, 115200);
	
    printf("\n+------------------------------------------------------------------------+\n");
    printf("|                      Timer Driver Sample Code                          |\n");
    printf("+------------------------------------------------------------------------+\n");
	printf("Select Test Item(1-2):\n");
	printf("Key 1: Timer IRQ Handeler.\n");
	printf("Key 2: Timer Delay 1 sec.\n");

    while(1) 
	{
		u8Option = getchar();

		switch(u8Option)
		{
			case '1':
				
                printf("Start to Timer IRQ Handeler(3 ticks, 1tick=1sec) test.\n" );
                // Using TIMER0 PERIODIC_MODE , 1 tick /sec.
                TIMER_Open( TIMER0, TIMER_PERIODIC_MODE, 1);
                u8Counter = 0;
				// Start Timer 0
                TIMER_Start(TIMER0);
                // Enable timer interrupt
                TIMER_EnableInt(TIMER0);
                NVIC_EnableIRQ(TMR0_IRQn);	
                // Wait interrupt happened 5 times.
			    while( u8Counter < 3 );
				// Close timer for next test.
				TIMER_Close(TIMER0);
			
                printf("Timer IRQ Handeler test complete.\n" );			
		        break;
			
			case '2':
                
                // Delay 1 sec via timer0.
                TIMER_Delay( TIMER0, 1000000);	
			
                printf("Timer Delay 1 sec test complete.\n" );
		        break;
		}				
    }
}

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
