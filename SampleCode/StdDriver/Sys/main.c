/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/07/17 5:40p $
 * @brief    Demonstrate the usage of SYS driver by changing different 
 *           HCLK setting for the system clock source. 
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "stdio.h"
#include "ISD9100.h"

#define SIGNATURE       0x125ab234
#define FLAG_ADDR       0x20002FFC

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
    /* Set GPG multi-function pins for UART0 RXD and TXD */
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
int32_t main (void)
{
    uint32_t u32data;
    uint8_t u8Option = 0;

    /* Lock protected registers */
    if(SYS_IsRegLocked() == 0) // In end of main function, program issued CPU reset and write-protection will be disabled.
        SYS_LockReg();
    
    /* Init System, IP clock and multi-function I/O */
    SYS_Init(); //In the end of SYS_Init() will issue SYS_LockReg() to lock protected register. If user want to write protected register, please issue SYS_UnlockReg() to unlock protected register.

    /* Init UART0 for printf */
    UART_Init();

    /*
        This sample code will show some function about system manager controller and clock controller:
        1. Read PDID
        2. Get and clear reset source
        3. Lock & unlock protected register.
	    4. Change system clock.
	    5. Reset 
    */

    printf("\n+------------------------------------------------------------------------+\n");
    printf("|                      System Driver Sample Code                         |\n");
    printf("+------------------------------------------------------------------------+\n");
    printf("CPU @ %dHz\n", SystemCoreClock );

    if (M32(FLAG_ADDR) == SIGNATURE) 
    {
        printf("  CPU Reset success!\n");
        M32(FLAG_ADDR) = 0;
        printf("  Press any key to continue ...\n");
        getchar();
    }

    /*---------------------------------------------------------------------------------------------------------*/
    /* Misc system function test                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Read Part Device ID */
    printf("Product ID 0x%x\n", SYS_ReadPDID() );

    /* Get reset source from last operation */
    u32data = SYS_GetResetSrc();
    printf("Reset Source 0x%x\n", u32data );

    /* Clear reset source */
    SYS_ClearResetSrc( u32data );

    /* Unlock protected registers for Brown-Out Detector settings */
    SYS_UnlockReg();

    /* Check if the write-protected registers are unlocked before BOD setting and CPU Reset */
    if ( SYS_IsRegLocked() == 0 ) 
        printf("Protected Address is Unlocked\n");

    printf("Select HCLK Frequency(1-8), Exit(Other Key):\n");
    printf("Key 1: Internal OSC 49152000 Hz\n");
    printf("Key 2: Internal OSC 32768000 Hz\n");
    printf("Key 3: External Xtal 32768 Hz(Only support semihost mode to display)\n");
    printf("Key 4: Internal OSC 10000 Hz(Only support semihost mode to display)\n");
	
    do
    {
        u8Option = getchar();
		
        switch(u8Option)
        {
            case '1':
            CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_48M, CLK_CLKDIV0_HCLK(1));
            break;

            case '2':
            CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_32M, CLK_CLKDIV0_HCLK(1));
            break;									

            case '3':
            CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_LXT, 0, CLK_CLKDIV0_HCLK(1));
            break;	

            case '4':
            CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_LIRC, 0, CLK_CLKDIV0_HCLK(1));
            break;	

            default:
            u8Option = 0;
            break;
        }
		
        if( u8Option != 0 )
        {
            UART_Init();
            printf("HCLK frequency became %d Hz\n", CLK_GetHCLKFreq());
        }	
    }while( u8Option != 0 );
	
    /* Write a signature work to SRAM to check if it is reset by software */
    M32(FLAG_ADDR) = SIGNATURE;
    printf("\n\n  >>> Reset CPU <<<\n");

    /* Reset CPU */
    SYS_ResetCPU();
}
