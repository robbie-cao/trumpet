/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/07/15 2:52p $
 * @brief    ISD9100 Series PWM Generator and Capture Timer Driver Sample Code
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "ISD9100.h"

void PWM0_IRQHandler(void)
{
    // Update PWM0 channel 0 period and duty
    PWM_SET_CNR(PWM0, PWM_CH0, 200);
    PWM_SET_CMR(PWM0, PWM_CH0, 50);
	
    // Clear channel 0 period interrupt flag
    PWM_ClearIntFlag(PWM0, 0);
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
    CLK_EnableModuleClock(PWM0_MODULE);

    /* Select PWM module clock source */
    CLK_SetModuleClock(PWM0_MODULE, CLK_CLKSEL1_PWM0CH01SEL_HCLK, 0);

    /* Reset PWM0 channel0~channel3 */
    SYS_ResetModule(PWM0_RST);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set GPA multi-function pins for UART0 RXD and TXD */
		SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA8MFP_Msk) ) | SYS_GPA_MFP_PA8MFP_UART_TX;
		SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA9MFP_Msk) ) | SYS_GPA_MFP_PA9MFP_UART_RX;
    /* Set GPA multi-function pins for PWM0 Channel0 */
		SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA12MFP_Msk) ) | SYS_GPA_MFP_PA12MFP_PWM0CH0;

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
    /* Init System, IP clock and multi-function I/O
       In the end of SYS_Init() will issue SYS_LockReg()
       to lock protected register. If user want to write
       protected register, please issue SYS_UnlockReg()
       to unlock protected register if necessary */

    /* Init System, IP clock and multi-function I/O */
    SYS_Init();

    /* Init UART0 for printf */
    UART_Init();

    printf("+------------------------------------------------------------------------+\n");
    printf("|                          PWM Driver Sample Code                        |\n");
    printf("|                                                                        |\n");
    printf("+------------------------------------------------------------------------+\n");
    printf("  This sample code will use PWM0 channel 0 to output waveform\n");
    printf("  I/O configuration:\n");
    printf("    waveform output pin: PWM0(PA.12)\n");
    printf("\nUse double buffer feature.\n");

    /*
        PWM0 channel 0 waveform of this sample shown below:

         ______                      ______                      _____
      __| 50   |_______150__________| 50   |_______150__________| 50   PWM waveform

    */

		/* set PWM0 channel 0 output configuration */
    PWM_ConfigOutputChannel(PWM0, PWM_CH0, 30000, 50);

    /* Enable PWM Output path for PWM0 channel 0 */
    PWM_EnableOutput(PWM0, 0x1);

    // Enable PWM channel 0 period interrupt
    PWM0->INTEN = PWM_INTEN_PIEN0_Msk;
    NVIC_EnableIRQ(PWM0_IRQn);

    // Start
    PWM_Start(PWM0, 0x1);

    while(1);
}
