/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/07/15 2:52p $
 * @brief    This is an I2S demo using NAU8822 audio codec, and used to play 
 *           back the input from MIC interface..
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ISD9100.h"
#include "config.h"

#define NAU8822_ADDR    0x1A                /* NAU8822 Device ID */

//uint32_t PcmBuff[BUFF_LEN] = {0};
uint32_t volatile u32BuffPos = 0;

void Delay(int count)
{
    volatile uint32_t i;
    for (i = 0; i < count ; i++)
		{
			i=i;
		}
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Write 9-bit data to 7-bit address register of WAU8822 with I2C0                                        */
/*---------------------------------------------------------------------------------------------------------*/
void I2C_WriteWAU8822(uint8_t u8addr, uint16_t u16data)
{

    I2C_START(I2C0);
    I2C_WAIT_READY(I2C0);

    I2C_SET_DATA(I2C0, NAU8822_ADDR<<1);
    I2C_SET_CONTROL_REG(I2C0, I2C_SI);
    I2C_WAIT_READY(I2C0);

    I2C_SET_DATA(I2C0, (uint8_t)((u8addr << 1) | (u16data >> 8)));
    I2C_SET_CONTROL_REG(I2C0, I2C_SI);
    I2C_WAIT_READY(I2C0);

    I2C_SET_DATA(I2C0, (uint8_t)(u16data & 0x00FF));
    I2C_SET_CONTROL_REG(I2C0, I2C_SI);
    I2C_WAIT_READY(I2C0);

    I2C_STOP(I2C0);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  WAU8822 Settings with I2C interface                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
void WAU8822_Setup()
{
    printf("\nConfigure WAU8822 ...");

    I2C_WriteWAU8822(0,  0x000);   /* Reset all registers */
    Delay(0x200);

    I2C_WriteWAU8822(1,  0x1DA); 
    I2C_WriteWAU8822(2,  0x1BF);   // Enable L/R Headphone, ADC Mix/Boost, ADC 
    I2C_WriteWAU8822(3,  0x1FF);   // Enable L/R main mixer, DAC 
    I2C_WriteWAU8822(4,  0x010);   // 16-bit word length, I2S format, Stereo 
    I2C_WriteWAU8822(5,  0x000);   // Companding control and loop back mode (all disable) 
    I2C_WriteWAU8822(6,  0x008);   // Divide by 6, 16K 
    I2C_WriteWAU8822(7,  0x006);   // 16K for internal filter coefficients 
    I2C_WriteWAU8822(10, 0x008);   // DAC soft mute is disabled, DAC oversampling rate is 128x 
    I2C_WriteWAU8822(14, 0x108);   // ADC HP filter is disabled, ADC oversampling rate is 128x 
    I2C_WriteWAU8822(15, 0x1FF);   // ADC left digital volume control 
    I2C_WriteWAU8822(16, 0x1FF);   // ADC right digital volume control 
    I2C_WriteWAU8822(44, 0x033);   // LMICN/LMICP is connected to PGA 
		I2C_WriteWAU8822(0x2d, 0x13F);   /* Left PGA gain*/
		I2C_WriteWAU8822(0x2e, 0x13F);   /* Right PGA gain*/	
		I2C_WriteWAU8822(0x2f, 0x100);   /* LLIN connected, and its Gain value */
		I2C_WriteWAU8822(0x30, 0x100);   /* RLIN connected, and its Gain value */
    I2C_WriteWAU8822(50, 0x001);   // Left DAC connected to LMIX 
    I2C_WriteWAU8822(51, 0x001);   // Right DAC connected to RMIX 

		printf("[OK]\n");
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
    CLK_EnableModuleClock(I2C0_MODULE);
    CLK_EnableModuleClock(I2S0_MODULE);

    /* Select I2S module clock source */
    CLK_SetModuleClock(I2S0_MODULE, CLK_CLKSEL2_I2S0SEL_HCLK, 0);

    /* Reset I2S */
    SYS_ResetModule(I2S0_RST);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set GPA multi-function pins for UART0 RXD and TXD */
		SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA8MFP_Msk) ) | SYS_GPA_MFP_PA8MFP_UART_TX;
		SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA9MFP_Msk) ) | SYS_GPA_MFP_PA9MFP_UART_RX;
    /* Set GPA10,GPA11 multi-function pins for I2C0 */
		SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA10MFP_Msk) ) | SYS_GPA_MFP_PA10MFP_I2C_SDA;
		SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA11MFP_Msk) ) | SYS_GPA_MFP_PA11MFP_I2C_SCL;
		GPIO_SetMode(PA, BIT10, GPIO_MODE_OPEN_DRAIN);
		GPIO_SetMode(PA, BIT11, GPIO_MODE_OPEN_DRAIN);

    /* Set multi function pin for I2S1 */
    /* GPA4, GPA5, GPA6, GPA7, GPB1  */
		SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA4MFP_Msk) ) | SYS_GPA_MFP_PA4MFP_I2S_FS;
		SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA5MFP_Msk) ) | SYS_GPA_MFP_PA5MFP_I2S_BCLK;
		SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA6MFP_Msk) ) | SYS_GPA_MFP_PA6MFP_I2S_SDI;
		SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA7MFP_Msk) ) | SYS_GPA_MFP_PA7MFP_I2S_SDO;
		SYS->GPB_MFP  = (SYS->GPB_MFP & (~SYS_GPB_MFP_PB1MFP_Msk) ) | SYS_GPB_MFP_PB1MFP_MCLK;

    /* Lock protected registers */
    SYS_LockReg();
}

void I2C0_Init(void)
{
    /* Open I2C0 and set clock to 48 */
    I2C_Open(I2C0, 48000);

    /* Get I2C0 Bus Clock */
    printf("I2C clock %d Hz\n", I2C_GetBusClockFreq(I2C0));

    /* Set I2C0 4 Slave Addresses */
    //I2C_SetSlaveAddr(I2C0, 0, 0x15, I2C_GCMODE_DISABLE);   /* Slave Address : 0x15 */
    //I2C_SetSlaveAddr(I2C0, 1, 0x35, I2C_GCMODE_DISABLE);   /* Slave Address : 0x35 */
    //I2C_SetSlaveAddr(I2C0, 2, 0x55, I2C_GCMODE_DISABLE);   /* Slave Address : 0x55 */
    //I2C_SetSlaveAddr(I2C0, 3, 0x75, I2C_GCMODE_DISABLE);   /* Slave Address : 0x75 */

    //I2C_EnableInt(I2C0);
    //NVIC_EnableIRQ(I2C0_IRQn);
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
    uint32_t u32startFlag = 1;

    /* Init System, IP clock and multi-function I/O */
    SYS_Init();

		CLK->APBCLK0 |= CLK_APBCLK0_ANACKEN_Msk;
		ANA->LDOPD &= ~ANA_LDOPD_PD_Msk;
		ANA->LDOSEL = 3;

    /* Init UART0 for printf */
    UART_Init();

    printf("+------------------------------------------------------------------------+\n");
    printf("|                   I2S Driver Sample Code with WAU8822                  |\n");
    printf("+------------------------------------------------------------------------+\n");
    printf("  NOTE: This sample code needs to work with WAU8822.\n");

    /* Init I2C0 to access WAU8822 */
    I2C0_Init();

    I2S_Open(I2S0, I2S_MODE_MASTER, 16000, I2S_DATABIT_16, I2S_STEREO, I2S_FORMAT_I2S, I2S_I2S);

    CLK_SetModuleClock(I2S0_MODULE, CLK_CLKSEL2_I2S0SEL_HCLK, 0);

    /* Initialize WAU8822 codec */
    WAU8822_Setup();

    /* Set MCLK and enable MCLK */
    I2S_EnableMCLK(I2S0, 4096000 );

    /* Enable Rx threshold level interrupt */
    I2S_EnableInt(I2S0, I2S_IEN_RXTHIEN_Msk);

    /* Enable I2S Rx function to receive data */
    I2S_ENABLE_RX(I2S0);

    while(1) {
        if (u32startFlag) {
            /* Enable I2S Tx function to send data when data in the buffer is more than half of buffer size */
            if (u32BuffPos >= BUFF_LEN/2) {
                I2S_EnableInt(I2S0, I2S_IEN_TXTHIEN_Msk);
                I2S_ENABLE_TX(I2S0);
                u32startFlag = 0;
            }
        }
    }
}

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
