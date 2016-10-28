/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2010 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------------------------*/
/* Include related header files                                                                            */
/*---------------------------------------------------------------------------------------------------------*/
#include "Platform.h"
#include "SDCard.h"
#include "ConfigSysClk.h"
#include <stdio.h>

//#define _SDCARD_USED_CALLBACK_FUNC_	// For user callback function to do something

UINT8	g_u8DataBuff[512*2] __attribute__ ((aligned ((32))));// 1k Test Buffer
S_SDCARD_HANDLER g_sSDCard;

#if defined _SDCARD_USED_CALLBACK_FUNC_
UINT32 g_u32SDCardWaitBusyCB=0;

void Smpl_SDCardWaitBusyCB(void)
{
	g_u32SDCardWaitBusyCB++;	
}
#endif

int main(void)
{
	UINT32 u32SPIClk, u32SDCardSize=0;
	UINT16 u16i, u16j;
	
	// ----------------------------------------------------------------------
	// Configure and select system clock
	// ----------------------------------------------------------------------
	SYSCLK_INITIATE();
	
	/* LDO On for I/O pad*/
	CLK_EnableLDO(CLK_LDOSEL_3_3V);
	
	printf("\n\nCPU @ %dHz\n", SystemCoreClock);
	
	/* SPI0: GPA0=MOSI0, GPA1=SLCK, GPB0=SSB1, GPA3=MISO0 */
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA0MFP_Msk) ) | SYS_GPA_MFP_PA0MFP_SPI_MOSI0;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA1MFP_Msk) ) | SYS_GPA_MFP_PA1MFP_SPI_SCLK;
	SYS->GPB_MFP  = (SYS->GPB_MFP & (~SYS_GPB_MFP_PB0MFP_Msk) ) | SYS_GPB_MFP_PB0MFP_SPI_SSB1;	
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA3MFP_Msk) ) | SYS_GPA_MFP_PA3MFP_SPI_MISO0;	
	
	/* Reset IP module */
	CLK_EnableModuleClock(SPI0_MODULE);
	SYS_ResetModule(SPI0_RST);

	if (SDCard_Open(SPI0, SPI_SS1, 12000000, &g_sSDCard ) != E_SUCCESS ) 
	{
		printf("The SD Card Init Fail.\n");
		return 1;
	}
	else
		printf("The SD Card Init Success.\n");
	
	u32SPIClk = SDCard_GetSPIClock(&g_sSDCard); 
	printf("The SPI Working Clock for SDCard : %dHz.\n",u32SPIClk);
	
#if defined _SDCARD_USED_CALLBACK_FUNC_
	SDCard_RegWaitBusyCallBackFunc(&g_sSDCard, Smpl_SDCardWaitBusyCB);
#endif
																				  
	
	u32SDCardSize = SDCard_GetCardSize(&g_sSDCard);

	if(SDCard_GetCardType(&g_sSDCard)==eTYPE_SD20_HIGH_CARD)
		printf("The SD Card Type: SD 2.0.\n");
	else
		printf("The SD Card Type: SD 1.0.\n");	

	printf("The SD Card Size: %d MByte.\n\n",(u32SDCardSize/1024));
	
	u16i = 0;
//----------------------------------------------------------------
//			   1. Read/Write SD Card 
//----------------------------------------------------------------
	for(u16j=0;u16j<(1024/4);u16j++)
	{
		g_u8DataBuff[u16i++]=0xa5;
		g_u8DataBuff[u16i++]=0x5a;
		g_u8DataBuff[u16i++]=0x69;
		g_u8DataBuff[u16i++]=0x96;
	}

	//====================================================
	//		   		Write Testing
	//====================================================
	SDCard_Write(&g_sSDCard, 0x00000000,2,&g_u8DataBuff[0]);


	//====================================================
	//		        Read Testing
	//====================================================
	SDCard_Read(&g_sSDCard, 0x00000000,2,&g_u8DataBuff[0]);
	
	
	u16i = 0;

	for(u16j=0;u16j<(1024/4);u16j++)
	{
		if(g_u8DataBuff[u16i++]!=0xa5)
			goto FailFig1;
		if(g_u8DataBuff[u16i++]!=0x5a)
			goto FailFig1;
		if(g_u8DataBuff[u16i++]!=0x69)
			goto FailFig1;
		if(g_u8DataBuff[u16i++]!=0x96)
			goto FailFig1;
	}
FailFig1:
	if(u16i!=1024)	
		printf("The SD Card Verify 1k Byte Data Fail!\n\n");
	else
		printf("The SD Card Verify 1k Byte Data Pass!\n\n");

#if defined _SDCARD_USED_CALLBACK_FUNC_
	printf("The Callback Function Calls Times: %d. \n\n", g_u32SDCardWaitBusyCB);
#endif

	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA0MFP_Msk) );
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA1MFP_Msk) );
	SYS->GPB_MFP  = (SYS->GPB_MFP & (~SYS_GPB_MFP_PB0MFP_Msk) );	
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA3MFP_Msk) );	
	
	return 1;
}

