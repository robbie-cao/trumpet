/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/07/15 14:14a $
 * @brief    This code is for standby/ deep power down test, need to run without 
 *           ICE (Nu-Link dongle). GPA13 are used to indicate the program running
 *           status.
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "ISD9100.h"

#define LED_INDICATE (BIT13)

volatile uint32_t u32Counter = 0;

void RTC_IRQHandler(void)
{
	RTC_CLEAR_TICK_INT_FLAG;	
	RTC_CLEAR_ALARM_INT_FLAG;
}

void SYS_Init(void)
{
	/* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable External OSC49M */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk|CLK_PWRCTL_LXTEN_Msk);
	
	/* Switch HCLK clock source to CLK2X a frequency doubled output of OSC48M */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_48M, CLK_CLKDIV0_HCLK(1));
	
	/* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();
	
	/* Lock protected registers */
    SYS_LockReg();
}

void UART_Init(void)
{
    /* Reset IP */
	CLK_EnableModuleClock(UART_MODULE);
    SYS_ResetModule(UART0_RST);
	
	/* Set GPG multi-function pins for UART0 RXD and TXD */
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA8MFP_Msk) ) | SYS_GPA_MFP_PA8MFP_UART_TX;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA9MFP_Msk) ) | SYS_GPA_MFP_PA9MFP_UART_RX;
	
    /* Configure UART0 and set UART0 Baudrate(115200) */
    UART_Open( UART0,115200 );
}

void RTC_Init(uint8_t u8Option)
{
	
	S_RTC_TIME_DATA_T sCurTime;
	
	/* Enable RTC APB clock */
	CLK_EnableModuleClock(RTC_MODULE);
	 
	/* sPt = 0, only reset RTC  */
	RTC_Open(0);
	
	RTC_EnableWakeUp();
	
	RTC_CLEAR_ALARM_INT_FLAG;

	RTC_CLEAR_TICK_INT_FLAG;

	/* Set Tick setting */
	RTC_SetTickPeriod(RTC_TICK_1_SEC);
	
	/* Time Setting */
    if (SBRAM->D[0] == 0)
	{
		sCurTime.u32Year       = 2014;
		sCurTime.u32Month      = 1;
		sCurTime.u32Day        = 1;
		sCurTime.u32Hour       = 16;
		sCurTime.u32Minute     = 10;
		sCurTime.u32Second     = 0;
		sCurTime.u32DayOfWeek  = RTC_WEDNESDAY;
		sCurTime.u32TimeScale  = RTC_CLOCK_24;
		/* Set current time*/
		RTC_SetDateAndTime(&sCurTime);
	}
	
	if (u8Option == '2')
	{
		/* wait writint to RTC IP */
		CLK_SysTickDelay(60);
		/* wait loading back to RTC_TIME */
		CLK_SysTickDelay(60);
		/* Get the current time */
        RTC_GetDateAndTime(&sCurTime);
		printf("\n\n  Current Time:%d/%02d/%02d %02d:%02d:%02d\n",sCurTime.u32Year,sCurTime.u32Month,sCurTime.u32Day,sCurTime.u32Hour,sCurTime.u32Minute,sCurTime.u32Second);
		
		/* Set alarm time*/
		sCurTime.u32Second += 5;
		RTC_SetAlarmDateAndTime(&sCurTime);
		/* wait writint to RTC IP */
		CLK_SysTickDelay(60);
		/* wait loading back to RTC_TALM */
		CLK_SysTickDelay(60);
		/* Enable RTC alarm Interrupt for wake up */	
		RTC_EnableInt(RTC_INTEN_ALMIEN_Msk);
		RTC_DisableInt(RTC_INTEN_TICKIEN_Msk);
	}
	else/* Enable RTC Tick Interrupt for wake up */	
	{
		RTC_EnableInt(RTC_INTEN_TICKIEN_Msk);
		RTC_DisableInt(RTC_INTEN_ALMIEN_Msk);
	}
	
	
	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_EnableIRQ(RTC_IRQn); 
}

void SPD_WakeUp_RTC(void)
{
	if (u32Counter<5)
	{
	    
		u32Counter++;
		SBRAM->D[0]=u32Counter;
		
		if((u32Counter&1)==1)  
	    {
	      PA->DOUT = (PA->DOUT&~LED_INDICATE)| LED_INDICATE;
	    }else{
	      PA->DOUT = (PA->DOUT&~LED_INDICATE)| 0;
	    }    	  
		while(UART_GET_TX_EMPTY(UART0)==0);
        CLK_StandbyPowerDown();	  	
	}
	else
		SBRAM->D[1] = 0;
	
}

/*---------------------------------------------------------------------------------------------------------*/
/* MAIN function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/    
int main (void)
{
	uint8_t u8Option;
	
	/* Lock protected registers */
	if(SYS->REGLCTL == 1) // In end of main function, program issued CPU reset and write-protection will be disabled.
		SYS_LockReg();

	/* Init System, IP clock and multi-function I/O */
	SYS_Init(); //In the end of SYS_Init() will issue SYS_LockReg() to lock protected register. If user want to write protected register, please issue SYS_UnlockReg() to unlock protected register.
	
	/* Init UART for printf */
    UART_Init();
	
	/* Will be reset by SPD wakeup, and has wakeup problem in next wakeup */
	CLK_EnableModuleClock(SBRAM_MODULE);
 	u32Counter= SBRAM->D[0];
	
	printf("\n\nWake-up And Reset");
	printf("\n\nCPU @ %dHz\n", SystemCoreClock);
			
	/* Standby power down will also has RSTS.PMU, RSTS.POR and RSTS.PAD flag 
	   POR or reset also has RSTS.PMU flag*/
	if(CLK_GET_POWERDOWNFLAG(CLK, CLK_PWRSTSF_SPDF) == 0)		//Check with Standby PD flag for first power on
	{
  		SYS_UnlockReg();

    	GPIO_SetMode(PA, LED_INDICATE, GPIO_MODE_OUTPUT);	 //Use GPIOA12 to know the problem point
		PA->DOUT = (PA->DOUT&~LED_INDICATE)| LED_INDICATE;

		SBRAM->D[1] = 0; // ment selection
		
		u8Option = 0;

 	}
	CLK_CLEAR_POWERDOWNFLAG(CLK, CLK_PWRSTSF_SPDF);		//Clear the Standby PD flag 
	
	
	if ( u8Option ==0)
	{
menu:
		printf("\n\n");
		printf("+----------------------------------------------------------------------+\n");
		printf("|                       CLK Driver Sample Code                         |\n");
		printf("+----------------------------------------------------------------------+\n");
		printf("|  [1] Standby Power Down (SPD) wake up from RTC tick,                 |\n");
		printf("|      CPU will wake up at each tick, tick unit: 1 second              |\n");       		
		printf("|  [2] Standby Power Down (SPD) wake up from RTC alarm                 |\n"); 
		printf("|      CPU will wake up at alarm time, alarm unit: 5 second            |\n");
		printf("|  [3] Deep Power Down (SPD) wake up from WAKEUP pin,                  |\n");
		printf("|      A high to low transition on the WAKEUP pin                      |\n");
		printf("|  [q] Quit                                                            |\n");
		printf("   Select the test number 1~3 or q:");
		PA->DOUT = (PA->DOUT&~LED_INDICATE)| 0;
		u8Option = getchar();
		SBRAM->D[1] = u8Option;
		SBRAM->D[0] = 0; // test count
		u32Counter = 0;
	}
	else 
		u8Option = SBRAM->D[1];
    
	if(u8Option == '1')
	{
		RTC_Init(u8Option);
		SPD_WakeUp_RTC();
	}
	else if(u8Option == '2')
	{
		RTC_Init(u8Option);
		SPD_WakeUp_RTC();
	}
	else if(u8Option == '3')
	{
		u8Option = 0;
		PA->DOUT = (PA->DOUT&~LED_INDICATE)| LED_INDICATE;
		printf("\n\n   Enter Deep Power Down! Please Prees WAKEUP pin to wake up.");
		while(UART_GET_TX_EMPTY(UART0)==0);
		CLK_DeepPowerDown(CLK_DPDWAKEUP_PIN,0);
	}
	else if( (u8Option == 'q') || (u8Option == 'Q') )
    {                                
        printf("\n\n   CLK sample code exit.\n");
		while(1);
    }	
	
	goto menu;

}


/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/







