/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/08/01 10:00p $
 * @brief    Access SPI flash through SPI interface.
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ISD9100.h"

#define SPI_FLASH_PORT      (SPI0)

#define TEST_NUMBER         (1)         /* page numbers */
#define TEST_LENGTH         (256)       /* length */

uint8_t au8SrcArray[TEST_LENGTH];
uint8_t au8DestArray[TEST_LENGTH];

void SpiFlash_SendRxData( uint32_t u32TxNum, uint8_t u8DWidth, uint32_t u32Tx0, uint32_t u32Tx1 )
{	
	/* set spi flash configuration */
	SPI_SET_TX_NUM(SPI_FLASH_PORT,u32TxNum);
	SPI_SET_DATA_WIDTH(SPI_FLASH_PORT,u8DWidth);
	
    /* send u32Data for receive request data */
    SPI_WRITE_TX0(SPI_FLASH_PORT, u32Tx0 );
	if( u32TxNum == SPI_TXNUM_TWO )
	    SPI_WRITE_TX1(SPI_FLASH_PORT, u32Tx1);
	
    /* transfer/receive action */
	SPI_GO(SPI_FLASH_PORT);
	while( SPI_IS_BUSY(SPI_FLASH_PORT) );
}

uint8_t SpiFlash_ReadStatusReg(void)
{		
    /* send Command: 0x05, read status register */
	SPI_SET_SS(SPI_FLASH_PORT,SPI_SS0);
	SpiFlash_SendRxData( SPI_TXNUM_ONE, 16, 0x05<<8, 0 );
	SPI_SET_SS(SPI_FLASH_PORT,SPI_SS_NONE);
	
	/* read spi flash current status */
    return (uint8_t)SPI_READ_RX0(SPI_FLASH_PORT);
}

void SpiFlash_WaitReady(void)
{
	/* while to wait status is not busy */
	while(SpiFlash_ReadStatusReg()&0x01);
}

void SpiFlash_NormalPageProgram(uint32_t u32StartAddress, uint8_t *u8DataBuffer)
{
	uint32_t u32i = 0;
	
    /* send command: 0x06, Write enable */
	SPI_SET_SS(SPI_FLASH_PORT,SPI_SS0);
    SpiFlash_SendRxData(SPI_TXNUM_ONE,8,0x06,0);
	SPI_SET_SS(SPI_FLASH_PORT,SPI_SS_NONE);	
	
	/* wait spi flash stand by */
	SpiFlash_WaitReady();

    /* active SS0*/
	SPI_SET_SS(SPI_FLASH_PORT,SPI_SS0);
	
    /* send Command: 0x02, write page data */
	SpiFlash_SendRxData( SPI_TXNUM_ONE, 32, ((uint32_t)0x02<<24)|(u32StartAddress&0xFFFFFF), 0);
	
	/* change big-endian to little-endian */
	SPI_ENABLE_BYTE_REORDER(SPI_FLASH_PORT);
	
    /* write data */
    while( u32i < TEST_LENGTH ) 
    {
		SPI_WRITE_TX0(SPI_FLASH_PORT,*((uint32_t*)&u8DataBuffer[u32i]));
		SPI_GO(SPI_FLASH_PORT);
		while(SPI_IS_BUSY(SPI_FLASH_PORT));    
		u32i += 4;
    }
	
	/* change little-endian to big-endian */
	SPI_DISABLE_BYTE_REORDER(SPI_FLASH_PORT);
	
	/* de-active */
	SPI_SET_SS(SPI_FLASH_PORT,SPI_SS_NONE);
}

void SpiFlash_NormalRead(uint32_t u32StartAddress, uint8_t *u8DataBuffer)
{
	uint32_t u32i = 0;
	
    /* active SS0*/
	SPI_SET_SS(SPI_FLASH_PORT,SPI_SS0);
	
    /* send Command: 0x03, read data */
	SpiFlash_SendRxData( SPI_TXNUM_ONE, 32, ((uint32_t)0x03<<24)|(u32StartAddress&0xFFFFFF), 0);
	
	/* change big-endian to little-endian */
	SPI_ENABLE_BYTE_REORDER(SPI_FLASH_PORT);
	
    /* read data */
    while( u32i < TEST_LENGTH ) 
    {	
		SPI_GO(SPI_FLASH_PORT);
		while(SPI_IS_BUSY(SPI_FLASH_PORT));      
		*((uint32_t*)&u8DataBuffer[u32i]) = SPI_READ_RX0(SPI_FLASH_PORT);
		u32i += 4;
    }
	
	/* change little-endian to big-endian */
	SPI_DISABLE_BYTE_REORDER(SPI_FLASH_PORT);
	
	/* de-active */
	SPI_SET_SS(SPI_FLASH_PORT,SPI_SS_NONE);	
}

void SpiFlash_ChipErase(void)
{
    /* send command: 0x06, Write enable */
	SPI_SET_SS(SPI_FLASH_PORT,SPI_SS0);
    SpiFlash_SendRxData(SPI_TXNUM_ONE,8,0x06,0);
	SPI_SET_SS(SPI_FLASH_PORT,SPI_SS_NONE);	
	
	/* wait spi flash stand by */
	SpiFlash_WaitReady();

    /* send command: 0xC7, Chip Erase */
	SPI_SET_SS(SPI_FLASH_PORT,SPI_SS0);
	SpiFlash_SendRxData(SPI_TXNUM_ONE,8,0xC7,0);
	SPI_SET_SS(SPI_FLASH_PORT,SPI_SS_NONE);	
}

uint16_t SpiFlash_ReadMidDid(void)
{
    /* send Command: 0x90, Read Manufacturer/Device ID */
	SPI_SET_SS(SPI_FLASH_PORT,SPI_SS0);
	SpiFlash_SendRxData(SPI_TXNUM_TWO,24,((uint32_t)0x90)<<16,0);
	SPI_SET_SS(SPI_FLASH_PORT,SPI_SS_NONE);
	
	/* read spi flash DeviceID(0x00FF) & ManufacID(0xFF00) */
    return ( SPI_READ_RX1(SPI_FLASH_PORT)&0xFFFF );
}

void SpiFlash_Init()
{
    /* Configure SPI_FLASH_PORT as a master, MSB first, SPI Mode-0 timing, clock is 8MHz */
    SPI_Open(SPI_FLASH_PORT, SPI_MASTER, SPI_MODE_0, 8000000, 0);

	SPI_SET_MSB_FIRST(SPI_FLASH_PORT);
	
	SPI_SET_SUSPEND_CYCLE(SPI_FLASH_PORT,3);
	
	SPI_DISABLE_BYTE_REORDER(SPI_FLASH_PORT);
	
	SPI_SET_DATA_WIDTH(SPI_FLASH_PORT,8);
	
	SPI_SET_TX_NUM(SPI_FLASH_PORT,SPI_TXNUM_ONE);
	
    /* Disable the automatic hardware slave select function */
    SPI_DISABLE_AUTOSS(SPI_FLASH_PORT);
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

    /* Switch HCLK clock source to HXT */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKSEL0_HIRCFSEL_48M, CLK_CLKDIV0_HCLK(1));

	/* LDO On */
	CLK->APBCLK0 |= CLK_APBCLK0_ANACKEN_Msk;
	ANA->LDOPD &= ~ANA_LDOPD_PD_Msk;
	ANA->LDOSEL = 3; 
	
    /* Enable IP clock */
    CLK_EnableModuleClock(UART_MODULE);
    CLK_EnableModuleClock(SPI0_MODULE);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* SPI0: GPA0=MOSI0, GPA1=SLCKMOSI0, GPA2=SSB0, GPA3=MISO0 */
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA0MFP_Msk) ) | SYS_GPA_MFP_PA0MFP_SPI_MOSI0;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA1MFP_Msk) ) | SYS_GPA_MFP_PA1MFP_SPI_SCLK;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA2MFP_Msk) ) | SYS_GPA_MFP_PA2MFP_SPI_SSB0;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA3MFP_Msk) ) | SYS_GPA_MFP_PA3MFP_SPI_MISO0;	

    /* Set GPG multi-function pins for UART0 RXD and TXD */
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA8MFP_Msk) ) | SYS_GPA_MFP_PA8MFP_UART_TX;
	SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA9MFP_Msk) ) | SYS_GPA_MFP_PA9MFP_UART_RX;

	/* Reset IP module */
	SYS_ResetModule(SPI0_RST);
	SYS_ResetModule(UART0_RST);
	
    /* Lock protected registers */
    SYS_LockReg();
}

int main()
{
	uint32_t u32Counter;
	
    /* Init System, IP clock and multi-function I/O */
    SYS_Init();
	
	/* Initiate SPI Flash */
	SpiFlash_Init();
	
    /* Init UART to 115200-8n1 for print message */
    UART_Open(UART0, 115200);

    printf("\n+------------------------------------------------------------------------+\n");
    printf("|               ISD9100 SPI Sample with SPI Flash                        |\n");
    printf("+------------------------------------------------------------------------+\n");
	
    /* Wait ready */
    SpiFlash_WaitReady();
	
    printf("Flash ID = 0x%X\n", SpiFlash_ReadMidDid() );

    printf("Erase chip ...");
    /* Erase SPI flash */
    SpiFlash_ChipErase();
	
    /* Wait ready */
    SpiFlash_WaitReady();
    printf("[OK]\n");
	
    /* init source data buffer */
    for(u32Counter=0; u32Counter<TEST_LENGTH; u32Counter++) 
        au8SrcArray[u32Counter] = u32Counter;	
	
    printf("Start to normal write data to Flash ...");
    /* Program SPI flash */
    for(u32Counter=0; u32Counter<TEST_NUMBER; u32Counter++) 
    {
        /* page program */
        SpiFlash_NormalPageProgram(u32Counter*TEST_LENGTH, au8SrcArray);
        SpiFlash_WaitReady();
    }
    printf("[OK]\n");
	
    /* clear destination data buffer */
	memset( au8DestArray, '\0', sizeof(au8DestArray) );
	
    printf("Normal Read & Compare ...");
    /* Read SPI flash */
	for(u32Counter=0; u32Counter<TEST_NUMBER; u32Counter++) 
        SpiFlash_NormalRead(u32Counter*TEST_LENGTH, au8DestArray);
  
	if( memcmp( au8DestArray, au8SrcArray, sizeof(au8DestArray) ) == 0 )
		printf("[OK]\n");
	else
        printf("[FAIL]\n");

    while(1);
}
