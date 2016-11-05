/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/07/15 2:52p $
 * @brief    ISD9100 General Purpose I/O Driver Sample Code
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "isd9100.h"
#include "gpio.h"

void GPAB_IRQHandler(void)
{
		static uint32_t Count=0;
	
    /* To check if PB2 interrupt occurred */
    if (PA->INTSRC & BIT15)
		{
        PA->INTSRC = BIT15;
        printf("PA15 INT occurred. %d\n",Count);
    }
		if (PB->INTSRC & BIT7)
		{
				PB->INTSRC = BIT7;
				printf("PB7 INT occurred. %d\n",Count);
		}
		Count++;
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
    int32_t i32Err;
		uint16_t PB7;
		uint32_t count=0;

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

    printf("\n\nCPU @ %dHz\n", SystemCoreClock);

    printf("+-------------------------------------+ \n");
    printf("|           GPIO Driver Sample Code   | \n");
    printf("+-------------------------------------+ \n");

    /*-----------------------------------------------------------------------------------------------------*/
    /* GPIO Basic Mode Test --- Use Pin Data Input/Output to control GPIO pin                              */
    /*-----------------------------------------------------------------------------------------------------*/
    printf("  >> Please connect PA.15 and PB.7 first << \n");
    printf("     Press any key to start test by using [Pin Data Input/Output Control] \n\n");
    //getchar();

    /* Configure PA.15 as Output mode and PB.7 as Input mode then close it */
    GPIO_SetMode(PA, BIT15, GPIO_MODE_OUTPUT);
    GPIO_SetMode(PB, BIT7, GPIO_MODE_INPUT);
		
    i32Err = 0;
    printf("  GPIO Output/Input test ...... \n");

    /* Use Pin Data Input/Output Control to pull specified I/O or get I/O pin status */
    GPIO_SET_OUT_DATA(PA, 0x00); // PA15=0
		PB7 = GPIO_GET_IN_DATA(PB) & 0x80;
    if (PB7 != 0) {
        i32Err = 1;
    }

    GPIO_SET_OUT_DATA(PA, 0x8000); // PA15=1
		PB7 = GPIO_GET_IN_DATA(PB) & 0x80;
    if (PB7 != 0x80) {
        i32Err = 1;
    }

    if ( i32Err ) {
        printf("  [FAIL] --- Please make sure PA.15 and PB.7 are connected. \n");
    } else {
        printf("  [OK] \n");
    }

    /* Configure PA.15 and PB.7 to default Quasi-bidirectional mode */
    GPIO_SetMode(PA, BIT15, GPIO_MODE_QUASI);
    GPIO_SetMode(PB, BIT7, GPIO_MODE_QUASI);


    /*-----------------------------------------------------------------------------------------------------*/
    /* GPIO Interrupt Function Test                                                                        */
    /*-----------------------------------------------------------------------------------------------------*/
    printf("\n  PA15, PB7 are used to test interrupt\n  and control LEDs(PA12,PA13,PB14)\n");

    /*Configure PA, PB for LED control */
    GPIO_SetMode(PA, BIT12, GPIO_MODE_OUTPUT);
		GPIO_SetMode(PA, BIT13, GPIO_MODE_OUTPUT);
		GPIO_SetMode(PB, BIT14, GPIO_MODE_OUTPUT);
		GPIO_SET_OUT_DATA(PA, ~0x1000);
		GPIO_SET_OUT_DATA(PB, ~0x0000);

    /* Configure PA15 as Input mode and enable interrupt by rising edge trigger */
    GPIO_SetMode(PA, BIT15, GPIO_MODE_INPUT);
    GPIO_EnableInt(PA, 15, GPIO_INT_RISING);
    NVIC_EnableIRQ(GPAB_IRQn);

    /*  Configure PB7 as Quasi-bi-direction mode and enable interrupt by both rising and falling edge trigger */
    GPIO_SetMode(PB, BIT7, GPIO_MODE_QUASI);
		GPIO_EnableInt(PB, 7, GPIO_INT_BOTH_EDGE);
		
    NVIC_EnableIRQ(GPAB_IRQn);

    /* Enable interrupt de-bounce function and select de-bounce sampling cycle time */
    GPIO_SET_DEBOUNCE_TIME(GPIO_DBCTL_DBCLKSRC_HCLK, GPIO_DBCTL_DBCLKSEL_1);
    GPIO_ENABLE_DEBOUNCE(PA, BIT15);
    GPIO_ENABLE_DEBOUNCE(PB, BIT7);

    /* Waiting for interrupts */
    while (1)
		{
			GPIO_SET_OUT_DATA(PA, ~((count>>14)&0xf000));
			GPIO_SET_OUT_DATA(PB, ~((count>>14)&0x80));
			count++;
		}

}


