/******************************************************************************
 * @file     main.c
 * @version  V3.00
 * $Revision: 3 $
 * $Date: 14/07/09 1:14p $
 * @brief    Demonstrate DPWM play 8kHz audio with different sample rate 
 *           and modulation frequency
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ISD9100.h"
/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
#define CH          0
#define PDMA        PDMA0
#define FRAME_SIZE    (8)
#define ARRAY_LENGTH (FRAME_SIZE*2)


extern uint32_t u32audioBegin, u32audioEnd;

__align(4) int16_t i16SrcArray[ARRAY_LENGTH];

uint32_t DPWMSampleRateFreq[4] = {8000, 16000, 32000, 48000}; //Sample rate
uint16_t DPWMModFreqDiv[8] = {228, 156, 76, 52, 780, 524, 396, 268}; //Modulation Division
int16_t *pi16AudioAdd;
int16_t *pi16AudioEnd;

void PDMA_IRQHandler(void)
{
	uint8_t i;

		
	if(PDMA_GCR->GLOBALIF & (1 << CH))
	{
		if (PDMA->CHIF & (0x4 << PDMA_CHIF_WAIF_Pos)) //Current transfer half complete flag
		{	
			PDMA->CHIF = (0x4 << PDMA_CHIF_WAIF_Pos); //Clear interrupt
			for(i = 0; i<FRAME_SIZE; i++)
				i16SrcArray[i] = *(pi16AudioAdd + i);
		}
		else //Current transfer finished flag 
		{		
			PDMA->CHIF = 0x1 << PDMA_CHIF_WAIF_Pos; //Clear interrupt
			for(i = 0; i<FRAME_SIZE; i++)
				i16SrcArray[FRAME_SIZE+i] = *(pi16AudioAdd + i);
		}
		pi16AudioAdd += FRAME_SIZE;
	}
	
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
	
    /* Switch HCLK clock source to CLK2X a frequency doubled output of OSC48M */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_48M, CLK_CLKDIV0_HCLK(1));

    /* Enable IP clock */
    CLK_EnableModuleClock(UART_MODULE);
	CLK_EnableModuleClock(DPWM_MODULE);
	CLK_EnableModuleClock(PDMA_MODULE);

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
/* InitialDPWM                                                                                             */
/*---------------------------------------------------------------------------------------------------------*/
void DPWM_Init(uint32_t u32SampleRate)
{  
	/* Reset IP */
	SYS_ResetModule(DPWM_RST);
	
	DPWM_Open();	 
	DPWM_SetSampleRate(u32SampleRate); //Set sample rate
	DPWM_SET_MODFREQUENCY(DPWM,DPWM_CTL_MODUFRQ0);//Set FREQ_0
	DPWM_ENABLE_PDMA(DPWM);
	
	 /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set GPG multi-function pins for SPK+ and SPK- */
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA12MFP_Msk) ) | SYS_GPA_MFP_PA12MFP_SPKP;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA13MFP_Msk) ) | SYS_GPA_MFP_PA13MFP_SPKM;
}

/*---------------------------------------------------------------------------------------------------------*/
/* Set PDMA0 to move ADC FIFO to MIC buffer with wrapped-around mode                                       */
/*---------------------------------------------------------------------------------------------------------*/
void PDMA_Init(void)
{
	volatile int32_t i = 10;
	
	/* Reset IP */
	SYS_ResetModule(PDMA_RST);

	
	PDMA_GCR->GLOCTL |= (1 << CH) << PDMA_GLOCTL_CHCKEN_Pos; //PDMA Controller Channel Clock Enable
			
	PDMA->DSCT_CTL |= PDMA_DSCT_CTL_SWRST_Msk;   //Writing 1 to this bit will reset the internal state machine and pointers
	PDMA->DSCT_CTL |= PDMA_DSCT_CTL_CHEN_Msk;    //Setting this bit to 1 enables PDMA assigned channel operation 
	while(i--);                                  //Need a delay to allow reset
	
	PDMA_GCR->SVCSEL &= 0xffff0fff;  //DMA channel is connected to DPWM peripheral transmit request.
	PDMA_GCR->SVCSEL |= CH << PDMA_SVCSEL_DPWMTXSEL_Pos;  //DMA channel is connected to DPWM peripheral transmit request.
	
	PDMA->DSCT_ENDSA = (uint32_t)&i16SrcArray[0];    //Set source address
	PDMA->DSCT_ENDDA = (uint32_t)&DPWM->DATA;     //Set destination address
	
	PDMA->DSCT_CTL |= 0x3 << PDMA_DSCT_CTL_SASEL_Pos;    //Transfer Source address is wrapped.
	PDMA->DSCT_CTL |= 0x2 << PDMA_DSCT_CTL_DASEL_Pos;    //Transfer Destination Address is fixed.
	PDMA->DSCT_CTL |= 0x2 << PDMA_DSCT_CTL_TXWIDTH_Pos;  //One half-word (16 bits) is transferred for every PDMA operation
	PDMA->DSCT_CTL |= 0x2 << PDMA_DSCT_CTL_MODESEL_Pos;  //Memory to IP mode (SRAM-to-APB).
	PDMA->DSCT_CTL |= 0x5 << PDMA_DSCT_CTL_WAINTSEL_Pos; //Wrap Interrupt: Both half and end buffer.
	
	PDMA->TXBCCH = ARRAY_LENGTH*2;          // Audio array total length, unit: sample.
	
	PDMA->INTENCH = 0x1 << PDMA_INTENCH_WAINTEN_Pos;;   //Wraparound Interrupt Enable

	NVIC_ClearPendingIRQ(PDMA_IRQn);
	NVIC_EnableIRQ(PDMA_IRQn);
	PDMA->DSCT_CTL |= PDMA_DSCT_CTL_TXEN_Msk;    //Start PDMA transfer
}


/*---------------------------------------------------------------------------------------------------------*/
/* MAIN function                                                                           	   			   */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{				
    uint8_t u8Option, u8Exit=0;

	uint8_t u8Div;
    
    /* Lock protected registers */
    if(SYS->REGLCTL == 1) // In end of main function, program issued CPU reset and write-protection will be disabled.
        SYS_LockReg();

    /* Init System, IP clock and multi-function I/O */
    SYS_Init(); //In the end of SYS_Init() will issue SYS_LockReg() to lock protected register. If user want to write protected register, please issue SYS_UnlockReg() to unlock protected register.

    /* Init UART for printf */
    UART_Init();

    printf("\n\nCPU @ %dHz\n", SystemCoreClock);

	DPWM_Init(8000);	//Enable DPWM clock, Set DPWM clock source and sample rate 
	
	/* Init audio data */
	memset(i16SrcArray, 0, sizeof(i16SrcArray));
	pi16AudioAdd = (int16_t *)&u32audioBegin;
	pi16AudioEnd = (int16_t *)&u32audioEnd;
	printf("\n\n");
    printf("+----------------------------------------------------------------------+\n");
    printf("|                       DPWM Driver Sample Code                        |\n");
    printf("+----------------------------------------------------------------------+\n");
    printf("\n");
	printf("The DPWM configuration is ready.\n");
	printf("DPWM sampling rate: %d Hz\n", DPWM_GetSampleRate());	 
	u8Div = DPWM_GET_MODFREQUENCY(DPWM);
	printf("DPWM modulation frequency: %d Hz\n", (SystemCoreClock/DPWMModFreqDiv[u8Div]));
	printf("Please connect speaker or headphone to SPK+ and SPK- pin\n");
	printf("Press Enter key to start...\n");

    getchar();
	printf("\nDPWM Test Begin...............\n");
	
   	
	while(u8Exit == 0)
    {   
       
		PDMA_Init();	//PDMA initialization
		DPWM_START_PLAY(DPWM);
				
		while (pi16AudioAdd < pi16AudioEnd);
		
		DPWM_STOP_PLAY(DPWM);
		
		PDMA->DSCT_CTL &= ~PDMA_DSCT_CTL_TXEN_Msk;   //Stop PDMA transfer
		NVIC_DisableIRQ(PDMA_IRQn);
		
		printf("Key 0: Change DPWM clock frequency\n");
		printf("Key 1: Change DPWM modulation frequency\n");
	    printf("Key 2: Exit\n");
        u8Option = getchar();
        printf("\n");

        switch(u8Option)
		{
		    case '0':
				printf("Select sample rate: \n");
				printf("Key 0: 8000 Hz\n");
		        printf("Key 1: 16000 Hz\n");
	            printf("Key 2: 32000 Hz\n");
				printf("Key 3: 48000 Hz\n");
			    u8Option = getchar()&0x03;
				DPWM_SetSampleRate(DPWMSampleRateFreq[u8Option]);	
				printf("Playing 8k Hz audio by DPWM sampling rate %d Hz\n", DPWM_GetSampleRate()); 
			    break;
			case '1':
			    printf("Select division: \n");
				printf("Key 0: 228\n");
		        printf("Key 1: 156\n");
	            printf("Key 2: 76\n");
				printf("Key 3: 52\n");
		        printf("Key 4: 780\n");
	            printf("Key 5: 524\n");
				printf("Key 6: 396\n");
	            printf("Key 7: 268\n");
				u8Option = getchar()&0x07;
				DPWM_SET_MODFREQUENCY(DPWM,u8Option);
				u8Div = DPWM_GET_MODFREQUENCY(DPWM);
				printf("DPWM modulation frequency became %d Hz\n", (SystemCoreClock)/DPWMModFreqDiv[u8Div]);
				break;
			case '2':
				u8Exit = 1;
				printf("Exit DPWM Demo\n");
			    
				break;
		}
		memset(i16SrcArray, 0, sizeof (i16SrcArray));
		pi16AudioAdd = (int16_t *)&u32audioBegin;
		printf("\n");
	}
	CLK_DisableModuleClock(DPWM_MODULE);
	CLK_DisableModuleClock(PDMA_MODULE);
	//CLK_DisableModuleClock(UART_MODULE);
	while(1);
}	

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/

