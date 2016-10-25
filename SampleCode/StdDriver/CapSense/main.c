/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/07/14 10:00a $
 * @brief    Demo driver capsense.c via touch pad controlling.
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include "ISD9100.h"
#include "TouchPad.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Init System Clock                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void Sys_Init(void)
{
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
	CLK_EnableModuleClock(ANA_MODULE);	
    CLK_EnableModuleClock(ACMP_MODULE);	

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();
	
    /* Lock protected registers */
    SYS_LockReg();	
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int main (void)
{
	uint8_t u8k;
    uint16_t u16CurrentStatus = 0, u16RecordStatus = 0;

    /* Init System Clock */
	Sys_Init();
	
    /*---------------------------------------------------------------------------------------------------------*/
    /* Initiate touch pad and gpio to provide for capture sense                                                */
    /*---------------------------------------------------------------------------------------------------------*/
	/* Clock source configuration */ 
	CLK_SetModuleClock(PWM0_MODULE, CLK_CLKSEL1_PWM0CH01SEL_HCLK, NULL);
	
    /* Set GPA & GPB multi-function pins for touch pad */
	SYS->GPB_MFP  = (SYS->GPB_MFP & (~SYS_GPB_MFP_PB0MFP_Msk) ) | SYS_GPB_MFP_PB0MFP_CMP0;
	SYS->GPB_MFP  = (SYS->GPB_MFP & (~SYS_GPB_MFP_PB1MFP_Msk) ) | SYS_GPB_MFP_PB1MFP_CMP1;
	SYS->GPB_MFP  = (SYS->GPB_MFP & (~SYS_GPB_MFP_PB2MFP_Msk) ) | SYS_GPB_MFP_PB2MFP_CMP2;
	SYS->GPB_MFP  = (SYS->GPB_MFP & (~SYS_GPB_MFP_PB3MFP_Msk) ) | SYS_GPB_MFP_PB3MFP_CMP3;
	SYS->GPB_MFP  = (SYS->GPB_MFP & (~SYS_GPB_MFP_PB4MFP_Msk) ) | SYS_GPB_MFP_PB4MFP_CMP4;
	SYS->GPB_MFP  = (SYS->GPB_MFP & (~SYS_GPB_MFP_PB5MFP_Msk) ) | SYS_GPB_MFP_PB5MFP_CMP5;
	SYS->GPB_MFP  = (SYS->GPB_MFP & (~SYS_GPB_MFP_PB6MFP_Msk) ) | SYS_GPB_MFP_PB6MFP_CMP6;
	SYS->GPB_MFP  = (SYS->GPB_MFP & (~SYS_GPB_MFP_PB7MFP_Msk) ) | SYS_GPB_MFP_PB7MFP_CMP7;
	
	/* Touch pad initiate configuration */
	TouchPad_Initiate(  CAPSENSE_CURCTL0_CURSRCEN_GPIOB0 | CAPSENSE_CURCTL0_CURSRCEN_GPIOB1 | 
						CAPSENSE_CURCTL0_CURSRCEN_GPIOB2 | CAPSENSE_CURCTL0_CURSRCEN_GPIOB3 | 
						CAPSENSE_CURCTL0_CURSRCEN_GPIOB4 | CAPSENSE_CURCTL0_CURSRCEN_GPIOB5 |
						CAPSENSE_CURCTL0_CURSRCEN_GPIOB6 | CAPSENSE_CURCTL0_CURSRCEN_GPIOB7 );
	
	while(1)
	{
		TouchPad_Scan();
	
		u16CurrentStatus = TouchPad_GetStatus(); 
		
		for( u8k = 0; u8k<TOUCHPAD_MAX_KEY_COUNT ; u8k++ )
		{
			if( (u16CurrentStatus&1<<u8k) == 0 && (u16RecordStatus&1<<u8k) != 0 )
			{
				u16RecordStatus &= ~(1<<u8k);
				printf( "Key %2d Released.\n", u8k );
			}
			else if( (u16CurrentStatus&1<<u8k) != 0 && (u16RecordStatus&1<<u8k) == 0 )
			{
				u16RecordStatus |= (1<<u8k);
				printf( "Key %2d Pressed.\n", u8k );
			}
		}
	}
}

