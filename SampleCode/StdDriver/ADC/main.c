/******************************************************************************
 * @file     main.c
 * @version  V3.00
 * $Revision: 2 $
 * $Date: 14/07/10 10:14a $
 * @brief    Demonstrate ADC function:
 *           1. MIC to speaker.
 *           2. ADC With ALC.
 *           3. ADC Compare Monitor.
 *           4. VMID selection.
 *           5. MICBIAS selection. 
 *           6. ADC Single Mode.
 *           7. ADC Cycle Mode.
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ISD9100.h"
/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/

#define abs(x)   (x>=0 ? x : -x)
#define sign(x)  (x>=0 ? '+' : '-')

#define CH          0
#define PDMA        PDMA0
#define PGA_GAIN    0  //default
#define ADC_SAMPLE_RATE  (16000)  //default
#define FRAME_SZIE       (8)
#define BUFFER_LENGTH    (FRAME_SZIE*2)

__align(4) int16_t i16Buffer[BUFFER_LENGTH];
uint16_t DPWMModFreqDiv[8] = {228, 156, 76, 52, 780, 524, 396, 268}; //Modulation Division
volatile uint8_t u8CmpMatch;

/*---------------------------------------------------------------------------------------------------------*/
/* Define Function Prototypes                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void ADC_MICTest(void);
void ADC_ALCTest(void);
void ADC_CompTest(void);
void ADC_TempTest(void);
void ADC_VMIDTest(void);
void ADC_MICBIASTest(void);
void ADC_SingleModeTest(void);
void ADC_CycleModeTest(void);

void PDMA_IRQHandler(void)
{
		
	if(PDMA_GCR->GLOBALIF & (1 << CH))
	{
		if (PDMA->CHIF & (0x4 << PDMA_CHIF_WAIF_Pos)) //Current transfer half complete flag
		{	
			PDMA->CHIF = (0x4 << PDMA_CHIF_WAIF_Pos); //Clear interrupt
			DPWM_WriteFIFO(&i16Buffer[0], FRAME_SZIE);
		}
		else //Current transfer finished flag 
		{		
			PDMA->CHIF = 0x1 << PDMA_CHIF_WAIF_Pos; //Clear interrupt
			DPWM_WriteFIFO(&i16Buffer[FRAME_SZIE], FRAME_SZIE);
		}
	}	
}

void ADC_IRQHandler(void)
{
	if (ADC_GetIntFlag(ADC_CMP0_INT))
	{
		u8CmpMatch |= 0x1;
		ADC_ClearIntFlag(ADC_CMP0_INT);
	}else if (ADC_GetIntFlag(ADC_CMP1_INT))
	{
		u8CmpMatch |= 0x2;
		ADC_ClearIntFlag(ADC_CMP1_INT);
	}
}

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable External OSC49M */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
	
    /* Switch HCLK clock source to CLK2X a frequency doubled output of OSC48M */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_48M, CLK_CLKDIV0_HCLK(1));

	/* Set ADC divisor from HCLK */
    CLK_SetModuleClock(ADC_MODULE, MODULE_NoMsk, CLK_CLKDIV0_ADC(1));
	
	/* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /* Set GPG multi-function pins for UART0 RXD and TXD */
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA8MFP_Msk) ) | SYS_GPA_MFP_PA8MFP_UART_TX;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA9MFP_Msk) ) | SYS_GPA_MFP_PA9MFP_UART_RX;

    /* Lock protected registers */
    SYS_LockReg();
}

void UART_Init(void)
{
    /* Reset IP */
	CLK_EnableModuleClock(UART_MODULE);
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 Baudrate(115200) */
    UART_Open( UART0,115200 );
}

void ADC_Init(void)
{
	uint32_t u32Div;
	
	/* Reset IP */
	CLK_EnableModuleClock(ADC_MODULE);
	CLK_EnableModuleClock(ANA_MODULE);
    SYS_ResetModule(EADC_RST);
	SYS_ResetModule(ANA_RST);
	
	/* Enable Analog block power */
	ADC_ENABLE_SIGNALPOWER(ADC,
	                       ADC_SIGCTL_ADCMOD_POWER|
						   ADC_SIGCTL_IBGEN_POWER|
	                       ADC_SIGCTL_BUFADC_POWER|
	                       ADC_SIGCTL_BUFPGA_POWER);
	
	/* PGA Setting */
	ADC_MUTEON_PGA(ADC, ADC_SIGCTL_MUTE_PGA);
	ADC_MUTEOFF_PGA(ADC, ADC_SIGCTL_MUTE_IPBOOST);
	ADC_ENABLE_PGA(ADC, ADC_PGACTL_REFSEL_VMID, ADC_PGACTL_BOSST_GAIN_26DB);
	ADC_SetPGAGaindB(PGA_GAIN); // 0dB
	
	/* MIC circuit configuration */
	ADC_ENABLE_VMID(ADC, ADC_VMID_HIRES_DISCONNECT, ADC_VMID_LORES_CONNECT);
	ADC_EnableMICBias(ADC_MICBSEL_90_VCCA);
	ADC_SetAMUX(ADC_MUXCTL_MIC_PATH, ADC_MUXCTL_POSINSEL_NONE, ADC_MUXCTL_NEGINSEL_NONE);
	
	/* Open ADC block */
	ADC_Open();
	ADC_SET_OSRATION(ADC, ADC_OSR_RATION_192);
	u32Div = CLK_GetHIRCFreq()/ADC_SAMPLE_RATE/192;
	ADC_SET_SDCLKDIV(ADC, u32Div);
	ADC_SET_FIFOINTLEVEL(ADC, 7);
	
	ADC_MUTEOFF_PGA(ADC, ADC_SIGCTL_MUTE_PGA);
	
}

void DPWM_Init(void)
{
	/* Reset IP */
	CLK_EnableModuleClock(DPWM_MODULE);
	SYS_ResetModule(DPWM_RST);
	
	DPWM_Open();	 
	DPWM_SetSampleRate(ADC_SAMPLE_RATE); //Set sample rate
	DPWM_SET_MODFREQUENCY(DPWM,DPWM_CTL_MODUFRQ0);//Set FREQ_0
	
	/* Set GPG multi-function pins for SPK+ and SPK- */
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA12MFP_Msk) ) | SYS_GPA_MFP_PA12MFP_SPKP;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA13MFP_Msk) ) | SYS_GPA_MFP_PA13MFP_SPKM;
}

void PDMA_Init(void)
{
	volatile int32_t i = 10;
	
	/* Reset IP */
	CLK_EnableModuleClock(PDMA_MODULE);
	SYS_ResetModule(PDMA_RST);

	
	PDMA_GCR->GLOCTL |= (1 << CH) << PDMA_GLOCTL_CHCKEN_Pos; //PDMA Controller Channel Clock Enable
			
	PDMA->DSCT_CTL |= PDMA_DSCT_CTL_SWRST_Msk;   //Writing 1 to this bit will reset the internal state machine and pointers
	PDMA->DSCT_CTL |= PDMA_DSCT_CTL_CHEN_Msk;    //Setting this bit to 1 enables PDMA assigned channel operation 
	while(i--);                                  //Need a delay to allow reset
	
	PDMA_GCR->SVCSEL &= 0xfffff0ff;  //DMA channel is connected to ADC peripheral transmit request.
	PDMA_GCR->SVCSEL |= CH << PDMA_SVCSEL_DPWMTXSEL_Pos;  //DMA channel is connected to DPWM peripheral transmit request.
	
	PDMA->DSCT_ENDSA = (uint32_t)&ADC->DAT;    //Set source address
	PDMA->DSCT_ENDDA = (uint32_t)i16Buffer;    //Set destination address
	
	PDMA->DSCT_CTL |= 0x2 << PDMA_DSCT_CTL_SASEL_Pos;    //Transfer Source address is fixed.
	PDMA->DSCT_CTL |= 0x3 << PDMA_DSCT_CTL_DASEL_Pos;    //Transfer Destination Address is wrapped.
	PDMA->DSCT_CTL |= 0x2 << PDMA_DSCT_CTL_TXWIDTH_Pos;  //One half-word (16 bits) is transferred for every PDMA operation
	PDMA->DSCT_CTL |= 0x1 << PDMA_DSCT_CTL_MODESEL_Pos;  //Memory to IP mode (APB-to-SRAM).
	PDMA->DSCT_CTL |= 0x5 << PDMA_DSCT_CTL_WAINTSEL_Pos; //Wrap Interrupt: Both half and end buffer.
	
	PDMA->TXBCCH = BUFFER_LENGTH*2;          // Audio array total length, unit: sample.
	
	PDMA->INTENCH = 0x1 << PDMA_INTENCH_WAINTEN_Pos;;   //Wraparound Interrupt Enable
	
	ADC_ENABLE_PDMA(ADC);
	
	NVIC_ClearPendingIRQ(PDMA_IRQn);
	NVIC_EnableIRQ(PDMA_IRQn);
	PDMA->DSCT_CTL |= PDMA_DSCT_CTL_TXEN_Msk;    //Start PDMA transfer
}

uint8_t SingleEndInput_ChannelSelect()
{
    uint8_t u8Option;
    
    printf("  Select ADC channel:\n");
    printf("  [0] Channel 0\n");
    printf("  [1] Channel 1\n");
    printf("  [2] Channel 2\n");
    printf("  [3] Channel 3\n");
    printf("  [4] Channel 4\n");
    printf("  [5] Channel 5\n");
    printf("  [6] Channel 6\n");
    printf("  [7] Channel 7\n");
    printf("  Other keys: exit single mode test\n");
    u8Option = getchar();
   
    if(u8Option=='0')
        ADC_SetGPIOChannel(ADC_GPIO_SINGLEEND_CH0_N);
    else if(u8Option=='1')
        ADC_SetGPIOChannel(ADC_GPIO_SINGLEEND_CH1_N);
    else if(u8Option=='2')
        ADC_SetGPIOChannel(ADC_GPIO_SINGLEEND_CH2_N);
    else if(u8Option=='3')
        ADC_SetGPIOChannel(ADC_GPIO_SINGLEEND_CH3_N);
    else if(u8Option=='4')
        ADC_SetGPIOChannel(ADC_GPIO_SINGLEEND_CH4_N);
    else if(u8Option=='5')
        ADC_SetGPIOChannel(ADC_GPIO_SINGLEEND_CH5_N);
    else if(u8Option=='6')
        ADC_SetGPIOChannel(ADC_GPIO_SINGLEEND_CH6_N);
    else if(u8Option=='7')
        ADC_SetGPIOChannel(ADC_GPIO_SINGLEEND_CH7_N);
    else
        return 0xFF;

    u8Option = u8Option - '0';
    return u8Option;   // return the the active channel number 
}

uint8_t DifferentialInput_ChannelSelect()
{
    uint8_t u8Option;
    
    printf("  Select ADC channel:\n");
    printf("  [0] Differential input pair 0(CH0 and 1)\n");
    printf("  [1] Differential input pair 1(CH2 and 3)\n");
    printf("  [2] Differential input pair 2(CH4 and 5)\n");
    printf("  [3] Differential input pair 3(CH6 and 7)\n");
    printf("  Other keys: quit\n");
    u8Option = getchar();
    if(u8Option=='0')
    {
        ADC_SetGPIOChannel(ADC_GPIO_DIFFERENTIAL_CH01);
    }
    else if(u8Option=='1')
    {
        ADC_SetGPIOChannel(ADC_GPIO_DIFFERENTIAL_CH23);
    }
    else if(u8Option=='2')
    {
        ADC_SetGPIOChannel(ADC_GPIO_DIFFERENTIAL_CH45);
    }
    else if(u8Option=='3')
    {
        ADC_SetGPIOChannel(ADC_GPIO_DIFFERENTIAL_CH67);
    }
    else
        return 0xFF;
    return u8Option;
}

/*---------------------------------------------------------------------------------------------------------*/
/* Main function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
int main(void)
{
	uint8_t u8Option;
		
	/* Lock protected registers */
    if(SYS->REGLCTL == 1) // In end of main function, program issued CPU reset and write-protection will be disabled.
        SYS_LockReg();

    /* Init System, IP clock and multi-function I/O */
    SYS_Init(); //In the end of SYS_Init() will issue SYS_LockReg() to lock protected register. If user want to write protected register, please issue SYS_UnlockReg() to unlock protected register.

    /* Init UART for printf */
    UART_Init();
	
	while (1)
    {
		printf("\n\nCPU @ %dHz\n", SystemCoreClock);
		
		printf("\n\n");
		printf("+----------------------------------------------------------------------+\n");
		printf("|                       ADC Driver Sample Code                         |\n");
		printf("+----------------------------------------------------------------------+\n");
		printf("|  [1] MIC To SPK Test                                                 |\n");       
		printf("|  [2] ADC With ALC Test                                               |\n");       
		printf("|  [3] ADC Compare Monitor Test                                        |\n");
		printf("|  [4] ADC Temperature Monitor Test                                    |\n");
		printf("|  [5] VMID Test                                                       |\n");
		printf("|  [6] MICBIAS Test                                                    |\n");
		printf("|  [7] ADC Single Mode Test                                            |\n");
		printf("|  [8] ADC Cycle Mode Test                                             |\n");
		printf("|  [q] Quit                                                            |\n");
		printf("  Select the test number 1~8 or q:");
		u8Option = getchar();
	
	
	//getchar();
	//printf("\nADC Test Begin...............\n");
		if(u8Option == '1')
        {
            ADC_MICTest();    
        }
        else if(u8Option == '2')
        {
            ADC_ALCTest();
        }
        else if(u8Option == '3')
        {
            ADC_CompTest();
        }
        else if(u8Option == '4')
        {
            ADC_TempTest();
        }
        else if(u8Option == '5')
        {
            ADC_VMIDTest();
        }
        else if(u8Option == '6')
        {
            ADC_MICBIASTest();
        }
        else if(u8Option == '7')
        {
            ADC_SingleModeTest();    
        }   
        else if(u8Option == '8')
        {
            ADC_CycleModeTest(); 
        }             
        else if( (u8Option == 'q') || (u8Option == 'Q') )
        {                                
            printf("\nADC sample code exit.\n");
            break;
        }
	}
	while(1);
}

/*---------------------------------------------------------------------------------------------------------*/
/* Define Test Function Items                                                                              */
/*---------------------------------------------------------------------------------------------------------*/

void ADC_MICTest(void)
{
	int32_t i32PGAGain, i32RealGain;
	uint8_t u8Div, u8Option;
	
	printf("\n\n=== MIC To SPK test ===\n");
	/* Init ADC */
	i32PGAGain = PGA_GAIN;
	ADC_Init();
	
	/* Init DPWM , Set DPWM clock source and sample rate */
	DPWM_Init();
	
	/* Init PDMA to move ADC FIFO to MIC buffer with wrapped-around mode  */
	PDMA_Init();
	printf("  The ADC and DPWM configuration is ready.\n");
	printf("  ADC sampling rate: %d Hz\n", ADC_GetSampleRate());
	printf("  DPWM sampling rate: %d Hz\n", DPWM_GetSampleRate());	 
	u8Div = DPWM_GET_MODFREQUENCY(DPWM);
	printf("  DPWM modulation frequency: %d Hz\n", (SystemCoreClock/DPWMModFreqDiv[u8Div]));	
	printf("  Please connect speaker or headphone to SPK+ and SPK- pin\n");
	printf("  Press any key to start...\n");
	getchar();
	
	ADC_START_CONV(ADC);
	DPWM_START_PLAY(DPWM);
	printf("\nChange ADC parameter\n");
    printf("  [i] increase PGA gain\n");
    printf("  [d] decrease PGA gain\n");
    printf("  [q] Exit\n");
	while(1)
	{
		u8Option = getchar();
		
		if(u8Option=='i')
			i32PGAGain+=50;	
		else if (u8Option=='d')
			i32PGAGain-=50;
		else if( (u8Option == 'q') || (u8Option == 'Q') )
			break;
		i32RealGain = ADC_SetPGAGaindB(i32PGAGain);
		printf("  Current PGA Gain = %c%d.%d dB\n\n\n", sign(i32RealGain), abs(i32RealGain)/100, abs(i32RealGain)%100);
	}
	DPWM_STOP_PLAY(DPWM);
	ADC_STOP_CONV(ADC);
	
	NVIC_ClearPendingIRQ(PDMA_IRQn);
	NVIC_DisableIRQ(PDMA_IRQn);
}

void ADC_ALCTest(void)
{
#define ALC_MAXGAIN (3525)  // 11.25dB
#define ALC_MINGAIN (-1200) // -12dB
#define ALC_TARGET  (-600)	// -6dB
#define ATTACK_TIME (2)     // Time = 500us * 2^ATTACK_TIME, ATTACK_TIME range: 0~10.
#define DECAY_TIME  (3)	    // Time = 125us * 2^DECAY_TIME, DECAY_TIME range: 0~10. 
#define HOLD_TIME  (0)	    // Time = 2^HOLD_TIME, HOLD_TIME range: 0~10.
#define NG_TH  (ADC_ALCCTL_NGTH6)	    //(-87+6*NG_TH) dB.
	
	int32_t i32PGAGain;
	uint8_t u8Option;
	
	printf("\n\n=== ADC With ALC Test ===\n");
	
	/* Init ADC */
	CLK_EnableModuleClock(BFAL_MODULE);
	SYS_ResetModule(BIQ_RST);
	ADC_Init();
	
	/* Init ALC */
	ADC_ENABLE_ALC(ADC, ADC_ALCCTL_NORMAL_MODE, ADC_ALCCTL_ABS_PEAK, ADC_ALCCTL_FASTDEC_OFF);
	ADC_ENABLE_NOISEGATE(ADC, ADC_ALCCTL_NGPEAK_ABS);
	
	printf("\nConfigure ALC parameter--->\n");
	i32PGAGain = ADC_SetALCMaxGaindB(ALC_MAXGAIN);
	printf("  PGA maximum gain boundary: %c%d.%d dB\n", sign(i32PGAGain), abs(i32PGAGain)/100, abs(i32PGAGain)%100);
	
	i32PGAGain = ADC_SetALCMinGaindB(ALC_MINGAIN);
	printf("  PGA minimum gain boundary: %c%d.%d dB\n", sign(i32PGAGain), abs(i32PGAGain)/100, abs(i32PGAGain)%100);
	
	i32PGAGain = ADC_SetALCTargetLevel(ALC_TARGET);
	printf("  ALC target level: %c%d.%d dB\n", sign(i32PGAGain), abs(i32PGAGain)/100, abs(i32PGAGain)%100);
	
	
	printf("  ALC attack time: %d us\n", 500*(2^ATTACK_TIME));
	ADC_SET_ALCATTACKTIME(ADC, ATTACK_TIME);
	
	printf("  ALC decay time: %d us\n", 125*(2^DECAY_TIME));
	ADC_SET_ALCDECAYTIME(ADC, DECAY_TIME);
	
	printf("  ALC hokd time: %d ms\n", (2^HOLD_TIME));
	ADC_SET_ALCHOLDTIME(ADC, HOLD_TIME);
	
	printf("  Noise gate threshold: -%d dB\n", (87-6*NG_TH));
	ADC_SET_NOISEGATE_TH(ADC, NG_TH);
	
	ADC_EnableInt(ADC_ALC_INT);
	NVIC_ClearPendingIRQ(ALC_IRQn);
	NVIC_EnableIRQ(ALC_IRQn);
	
	/* Init DPWM , Set DPWM clock source and sample rate */
	DPWM_Init();
	
	/* Init PDMA to move ADC FIFO to MIC buffer with wrapped-around mode  */
	PDMA_Init();
	
	printf("  Please connect speaker or headphone to SPK+ and SPK- pin\n");
	printf("  Press any key to start...\n");
	getchar();
	
	ADC_START_CONV(ADC);
	DPWM_START_PLAY(DPWM);
	
	while(1)
	{
	
		if( (u8Option == 'q') || (u8Option == 'Q') )
			break;
		u8Option = getchar();
	}
	
	DPWM_STOP_PLAY(DPWM);
	ADC_STOP_CONV(ADC);
	
	
	NVIC_ClearPendingIRQ(PDMA_IRQn);
	NVIC_ClearPendingIRQ(ALC_IRQn);
	NVIC_DisableIRQ(PDMA_IRQn);
	ADC_DisableInt(ADC_ALC_INT);
	NVIC_DisableIRQ(ALC_IRQn);
	ADC_DISABLE_ALC(ADC);
	ADC_DISABLE_NOISEGATE(ADC);
	SYS_ResetModule(BIQ_RST);
	CLK_DisableModuleClock(BFAL_MODULE);
}

void ADC_CompTest(void)
{
#define CMP0_DATA    (50)       // 50, 16-bits
#define CMP0_CNT     (16)       // max :16
#define CMP1_DATA    (-50)      // -50, 16-bits
#define CMP1_CNT     (16)       // max :16

	uint8_t u8Option;
	
	printf("\n\n=== ADC CMP test ===\n");
	/* Init ADC */
	ADC_Init();
	
	/* Init DPWM , Set DPWM clock source and sample rate */
	DPWM_Init();
	
	/* Init PDMA to move ADC FIFO to MIC buffer with wrapped-around mode  */
	PDMA_Init();
	
	/* Init Compare */
	u8CmpMatch = 0;
	ADC_ENABLE_CMP0(ADC, ADC_ADCMPR_CMPCOND_LESS_THAN, (uint32_t)CMP0_DATA, CMP0_CNT);
	//ADC_ENABLE_CMP1(ADC, ADC_ADCMPR_CMPCOND_GREATER_OR_EQUAL, (uint32_t)CMP1_DATA, CMP1_CNT);
	
	ADC_EnableInt(ADC_CMP0_INT|ADC_CMP1_INT);
	NVIC_ClearPendingIRQ(ADC_IRQn);
	NVIC_EnableIRQ(ADC_IRQn);
	
	printf("  Please connect speaker or headphone to SPK+ and SPK- pin\n");
	printf("  Press any key to start...\n");
	getchar();
	
	ADC_START_CONV(ADC);
	DPWM_START_PLAY(DPWM);
	
	while(1)
	{
		if( (u8Option == 'q') || (u8Option == 'Q') )
			break;
		if (u8CmpMatch & 0x3)
		{
			u8CmpMatch = 0;
			printf("  Meets condition ...\n");
			printf("  Press any key to continue or [q] exist...\n");
			u8Option = getchar();
		}
	}
	DPWM_STOP_PLAY(DPWM);
	ADC_STOP_CONV(ADC);
	
	NVIC_ClearPendingIRQ(ADC_IRQn);
	NVIC_DisableIRQ(ADC_IRQn);
}

void ADC_TempTest(void)
{
	uint32_t u32ConversionData;
	int32_t i32Temp;
	uint8_t i, u8Option;
	
	printf("\n\n=== ADC temperature monitor test ===\n");
	
	/* Reset IP */
	ADC_Init();
	
	/* PGA gain Setting */
	ADC_ENABLE_PGA(ADC, ADC_PGACTL_REFSEL_VBG, ADC_PGACTL_BOSST_GAIN_0DB);
	ADC_SetPGAGaindB(525); // 5.25dB
	
	/*  Select PGA input path */
	ADC_SetAMUX(ADC_MUXCTL_TEMP_PATH, ADC_MUXCTL_POSINSEL_NONE, ADC_MUXCTL_NEGINSEL_NONE);
	
	/* Interrupt occurs when one sample is converted */
	ADC_SET_FIFOINTLEVEL(ADC, 1);
	
	printf("  Temperature monitor configure ok...\n");
		
	/* Enable ADC Interrupt */ 
	ADC_EnableInt(ADC_FIFO_INT);
	ADC_START_CONV(ADC);
	
	/* Get first samples*/
	for(i=1;i<=20;i++) {
		 
		while(ADC_GetIntFlag(ADC_FIFO_INT));
		u32ConversionData = ADC_GET_FIFODATA(ADC);
    }
	
	while(1)
    {
        printf("  Start to get temperature[y/n]:\n");
        
        u8Option = getchar();
        
		if( (u8Option == 'n') || (u8Option == 'N') )
			break;
                
		while(ADC_GetIntFlag(ADC_FIFO_INT));
        u32ConversionData = ADC_GET_FIFODATA(ADC);

        i32Temp = 27+ (u32ConversionData - 0x42EA) / 50;   //refer to TRM for Temperature formula 

        printf("  Temperature result: (%d) degree. AD Convernt Data: (%d)\n\n", i32Temp, u32ConversionData);      
    } 
	
	ADC_STOP_CONV(ADC);
}

void ADC_VMIDTest(void)
{
    /* Open Analog block */
    CLK_EnableModuleClock(ANA_MODULE);
    SYS_ResetModule(ANA_RST);
	

    printf("\n\n=== ADC VMID test ===\n");
    /* MIC circuit configuration */
    ADC_DISABLE_VMID(ADC);    

    printf("  Pull VMID pin to ground\n");

    ADC_ENABLE_VMID(ADC,   
        ADC_VMID_HIRES_DISCONNECT,   
        ADC_VMID_LORES_DISCONNECT);   

    printf("  High Resistance And Low Resistance disconnect from VMID\n");

    ADC_ENABLE_VMID(ADC,    
        ADC_VMID_HIRES_CONNECT,       
        ADC_VMID_LORES_DISCONNECT);  

    printf("  High Resistanc reference to VMID\n");

    ADC_ENABLE_VMID(ADC,  
        ADC_VMID_HIRES_DISCONNECT,
        ADC_VMID_LORES_CONNECT);    

    printf("  Low Resistanc reference to VMID\n");
	 
    ADC_ENABLE_VMID(ADC,    
        ADC_VMID_HIRES_CONNECT,        
        ADC_VMID_LORES_CONNECT);    

    printf("  High Resistanc And Low Resistanc reference to VMID\n");
	
	printf("  Press any key to exit...\n");
	getchar();

}

void ADC_MICBIASTest(void)
{
	/* Open Analog block */
    CLK_EnableModuleClock(ANA_MODULE);
    SYS_ResetModule(ANA_RST);
	
	printf("\n\n=== ADC MIC bias test ===\n");
	
	ADC_EnableMICBias(ADC_MICBSEL_90_VCCA);
	printf("  MICBIAS Sel: 90% VCCA \n");

    ADC_EnableMICBias(ADC_MICBSEL_65_VCCA);
    printf("  MICBIAS Sel: 65% VCCA \n");

    ADC_EnableMICBias(ADC_MICBSEL_75_VCCA);
    printf("  MICBIAS Sel: 75% VCCA \n");

	ADC_EnableMICBias(ADC_MICBSEL_50_VCCA);
    printf("  MICBIAS Sel: 50% VCCA \n");

	ADC_EnableMICBias(ADC_MICBSEL_24V);
    printf("  MICBIAS Sel: 2.4V \n");

    ADC_EnableMICBias(ADC_MICBSEL_17V);
    printf("  MICBIAS Sel: 1.7V \n");

    ADC_EnableMICBias(ADC_MICBSEL_20V);
    printf("  MICBIAS Sel: 2.0V \n");

    ADC_EnableMICBias(ADC_MICBSEL_13V);
    printf("  MICBIAS Sel: 1.3V \n");
	
	printf("  Press any key to exit...\n");
	getchar();
}

void ADC_SingleModeTest(void)
{
	uint32_t u32ConversionData;
	uint8_t u8Option, u8InputMode, j;
	
	printf("\n\n=== ADC single mode test ===\n");
	
	/* Init ADC */
	ADC_Init();
	
	 while(1)
    {
single_menu:
        printf("  Select input mode:\n");
        printf("  [1] Single end input\n");
        printf("  [2] Differential input\n");
        printf("  [q] Exit single mode test\n");
        u8Option = getchar();
        if(u8Option=='1')
            u8InputMode = 1; // single-end
        else if(u8Option=='2')
            u8InputMode = 2; // differential 
        else if(u8Option=='q')
            return ;
        else
            goto single_menu;
        
        if(u8InputMode==1)        
            SingleEndInput_ChannelSelect();     // Select the active channel
        else
            DifferentialInput_ChannelSelect();  // Select the active channel 
                
        // Enable ADC Interrupt function
        ADC_EnableInt(ADC_FIFO_INT);
    
        // Start A/D conversion 
        ADC_START_CONV(ADC);

        // Wait ADC interrupt 
        while(ADC_GetIntFlag(ADC_FIFO_INT));

        for(j=1;j<=8;j++) {
            u32ConversionData = ADC_GET_FIFODATA(ADC);
            printf("  0x%X (%d)\n\n", u32ConversionData, u32ConversionData);
        }
		
		// stop A/D conversion 
        ADC_STOP_CONV(ADC);
    }   
	
}

void ADC_CycleModeTest(void)
{
	uint32_t u32ConversionData[8];
	uint8_t u8Option, u8InputMode, i, j;
	
	
	printf("\n\n=== ADC single mode test ===\n");
	
	/* Init ADC */
	ADC_Init();
	
	 while(1)
    {
cycle_menu:
		printf("  Select input mode:\n");
        printf("  [1] Single end input\n");
        printf("  [2] Differential input\n");
        printf("  [q] Exit single mode test\n");
        u8Option = getchar();
        if(u8Option=='1')
            u8InputMode = 1; // single-end
        else if(u8Option=='2')
            u8InputMode = 2; // differential 
        else if(u8Option=='q')
            return ;
		else
            goto cycle_menu;
        
        if(u8InputMode==1)        
            SingleEndInput_ChannelSelect();     // Select the active channel
        else
            DifferentialInput_ChannelSelect();  // Select the active channel 
		
		for(i=1;i<=8;i++)
		{
                
			// Enable ADC Interrupt function
            ADC_EnableInt(ADC_FIFO_INT);
    
            // Start A/D conversion 
            ADC_START_CONV(ADC);;

            // Wait ADC interrupt 
            while(ADC_GetIntFlag(ADC_FIFO_INT));

            printf("--\n");
            for(j=0;j<=7;j++)
			{
				u32ConversionData[j] = ADC_GET_FIFODATA(ADC);
				printf("  0x%X (%d)\n\n", u32ConversionData[j], u32ConversionData[j]);
            }
        }
	}	
}

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/

