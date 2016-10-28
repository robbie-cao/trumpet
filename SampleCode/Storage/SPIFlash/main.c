/****************************************************************
 *                                                              *
 * Copyright (c) Nuvoton Technology Corp. All rights reserved.  *
 *                                                              *
 ****************************************************************/
#include "Platform.h"
#include "SPIFlash.h"
#include "ConfigSysClk.h"
#include <stdio.h>

__align(4) UINT8 g_au8Buf[SPIFLASH_PAGE_SIZE*2];
__align(4) UINT8 g_au8CheckBuf[SPIFLASH_PAGE_SIZE*2];

void SPIFlashDemo(void);

S_SPIFLASH_HANDLER g_sSpiFlash;

int main()
{
	UINT32 u32SPIClk = 0;
	
	// ----------------------------------------------------------------------
	// Configure and select system clock
	// ----------------------------------------------------------------------
	SYSCLK_INITIATE();
	
	/* LDO On for I/O pad*/
	CLK_EnableLDO(CLK_LDOSEL_3_3V);
	
	printf("\n\nCPU @ %dHz\n", SystemCoreClock);
	
	/* SPI0: GPA0=MOSI0, GPA1=SLCKMOSI0, GPA2=SSB0, GPA3=MISO0 */
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA0MFP_Msk) ) | SYS_GPA_MFP_PA0MFP_SPI_MOSI0;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA1MFP_Msk) ) | SYS_GPA_MFP_PA1MFP_SPI_SCLK;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA2MFP_Msk) ) | SYS_GPA_MFP_PA2MFP_SPI_SSB0;	
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA3MFP_Msk) ) | SYS_GPA_MFP_PA3MFP_SPI_MISO0;	
	/* Reset IP module */
	CLK_EnableModuleClock(SPI0_MODULE);
	SYS_ResetModule(SPI0_RST);
	SPIFlash_Open(SPI0, SPI_SS0, 12000000, &g_sSpiFlash );


	u32SPIClk = SPIFlash_GetSPIClock(&g_sSpiFlash);
	printf("SPIFlash run on actual clock: %d.\n", u32SPIClk);

	printf("Press enter to start SPI flash detection....\n");
	if ( getchar() != 0x0d )
		while(1);

	SPIFlash_GetChipInfo(&g_sSpiFlash);
	if ( g_sSpiFlash.u32FlashSize == 0 )
	{
		printf("Can not find any SPI flash\n");
		while(1);
	}

	if(g_sSpiFlash.u8Flag & SPIFLASH_FLAG_WINBOND) 
		printf("\nFind a Winbond SPI flash with %d M-bit\n\n", g_sSpiFlash.u32FlashSize*8/1024/1024);
	if(g_sSpiFlash.u8Flag & SPIFLASH_FLAG_MXIC) 
		printf("\nFind a MXIC SPI flash with %d M-bit\n\n", g_sSpiFlash.u32FlashSize*8/1024/1024);
	if(g_sSpiFlash.u8Flag & SPIFLASH_FLAG_ATMEL) 
		printf("\nFind a ATmel SPI flash with %d M-bit\n\n", g_sSpiFlash.u32FlashSize*8/1024/1024);

	SPIFlash_GlobalProtect(&g_sSpiFlash,FALSE);


	SPIFlashDemo();


	printf("SPI erase/program/read sample end...\n");

	SPIFlash_Close(&g_sSpiFlash);
	
	
	/* SPI0: GPA0=MOSI0, GPA1=SLCKMOSI0, GPA2=SSB0, GPA3=MISO0 */
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA0MFP_Msk) );
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA1MFP_Msk) );
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA2MFP_Msk) );	
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA3MFP_Msk) );	
	CLK_DisableModuleClock(SPI0_MODULE);

	while(1);
}	// main

void SPIFlashDemo(void)
{ 
	UINT32 u32Addr, j, u32ErasedBytes;
	UINT8 u8TestPatten;

	/* ------------------------*/
	/* Demo blocking APIs      */
	/* ------------------------*/
	printf("Start to Demo APIs...\n");
	// ------------------------------------------------------------------------------------------------------
	// Whole chip erase
	printf("\tStart erase whole chip...\n");
	
	SPIFlash_EraseChip(&g_sSpiFlash);
	// Program each page	
	printf("\tStart program pages...\n");
	u8TestPatten = 0x1;
	for( u32Addr = 0; u32Addr < g_sSpiFlash.u32FlashSize; u32Addr += SPIFLASH_PAGE_SIZE, u8TestPatten++ )
	{
		SPIFlash_BurstRead(&g_sSpiFlash, u32Addr, g_au8Buf, SPIFLASH_PAGE_SIZE);
		for( j = 0; j < SPIFLASH_PAGE_SIZE; j ++ )
			if ( g_au8Buf[j] != 0xff )
			{
				printf("\t\tErase whole chip failed!\n");
				while(1);
			}
		for( j = 0; j < SPIFLASH_PAGE_SIZE; j ++ )
			g_au8Buf[j] = u8TestPatten;

		SPIFlash_WritePage(&g_sSpiFlash, u32Addr, g_au8Buf);
	}
	// Read each page and check the content	 
	printf("\tStart verify pages...\n");
	u8TestPatten = 1;
	for( u32Addr = 0; u32Addr < g_sSpiFlash.u32FlashSize; u32Addr += SPIFLASH_PAGE_SIZE, u8TestPatten++ )
	{
		SPIFlash_BurstRead(&g_sSpiFlash, u32Addr, g_au8Buf, SPIFLASH_PAGE_SIZE);
		for( j = 0; j < SPIFLASH_PAGE_SIZE; j ++ )
			if ( g_au8Buf[j] != u8TestPatten )
			{
				printf("\t\tVerify failed in %dth element!\n", j);
				while(1);
			}
	}
 	// ------------------------------------------------------------------------------------------------------
	// One block(64Kbyte or 32KByte or 4Kbytes) chip errase	 
	printf("\tStart erase 1 block...\n");
	if ( g_sSpiFlash.u8Flag & SPIFLASH_FLAG_ERASE_4K )
	{
		SPIFlash_Erase4K(&g_sSpiFlash, 0, 1);
		u32ErasedBytes = 4*1024;
	}
	else if ( g_sSpiFlash.u8Flag & SPIFLASH_FLAG_ERASE_32K )
	{
		SPIFlash_Erase32K(&g_sSpiFlash, 0, 1);
		u32ErasedBytes = 32*1024;
	}
	else
	{
		SPIFlash_Erase64K(&g_sSpiFlash, 0, 1);
		u32ErasedBytes = 64*1024;
	}

	printf("\tStart program 1 block...\n");
	u8TestPatten = 0x1;
	for( u32Addr = 0; u32Addr < u32ErasedBytes; u32Addr += SPIFLASH_PAGE_SIZE, u8TestPatten++ )
	{
		SPIFlash_BurstRead(&g_sSpiFlash, u32Addr, g_au8Buf, SPIFLASH_PAGE_SIZE);
		for( j = 0; j < SPIFLASH_PAGE_SIZE; j ++ )
			if ( g_au8Buf[j] != 0xff )
			{
				printf("\t\tErase 1 block failed!\n");
				while(1);
			}
		for( j = 0; j < SPIFLASH_PAGE_SIZE; j ++ )
			g_au8Buf[j] = u8TestPatten;

		SPIFlash_WritePage(&g_sSpiFlash, u32Addr, g_au8Buf);
	}		
	printf("\tStart verify 1 block...\n");
	u8TestPatten = 1;
	for( u32Addr = 0; u32Addr < u32ErasedBytes; u32Addr += SPIFLASH_PAGE_SIZE, u8TestPatten++ )
	{
		SPIFlash_BurstRead(&g_sSpiFlash, u32Addr, g_au8Buf, SPIFLASH_PAGE_SIZE);
		for( j = 0; j < SPIFLASH_PAGE_SIZE; j ++ )
			if ( g_au8Buf[j] != u8TestPatten ) 
			{
				printf("\t\tVerify failed in %dth element!\n", j);
				while(1);
			}
	}
}
