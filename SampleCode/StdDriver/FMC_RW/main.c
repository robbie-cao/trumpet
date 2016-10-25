/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/07/15 2:52p $
 * @brief    Show FMC read flash IDs, erase, read, and write functions
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "ISD9100.h"

#define APROM_TEST_BASE             0x10000
#define DATA_FLASH_TEST_BASE        0x19000
#define DATA_FLASH_TEST_END         0x23400

#define TEST_PATTERN                0xA5A5A5A5

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

static int  set_data_flash_base(uint32_t u32DFBA)
{
    uint32_t   au32Config[4];

    if (FMC_ReadConfig(au32Config, 4) < 0) {
        printf("\nRead User Config failed!\n");
        return -1;
    }

    if ((!(au32Config[0] & 0x1)) && (au32Config[1] == u32DFBA))
        return 0;

    FMC_EnableConfigUpdate();

    au32Config[0] &= ~0x1;
    au32Config[1] = u32DFBA;

    if (FMC_WriteConfig(au32Config, 4) < 0)
        return -1;

    // Perform chip reset to make new User Config take effect
    SYS->IPRST0 = SYS_IPRST0_CHIPRST_Msk;
    return 0;
}


int32_t fill_data_pattern(uint32_t u32StartAddr, uint32_t u32EndAddr, uint32_t u32Pattern)
{
    uint32_t u32Addr;

    for (u32Addr = u32StartAddr; u32Addr < u32EndAddr; u32Addr += 4) {
        FMC_Write(u32Addr, u32Pattern);
    }
    return 0;
}

int32_t  verify_data(uint32_t u32StartAddr, uint32_t u32EndAddr, uint32_t u32Pattern)
{
    uint32_t    u32Addr;
    uint32_t    u32data;

    for (u32Addr = u32StartAddr; u32Addr < u32EndAddr; u32Addr += 4) {
        u32data = FMC_Read(u32Addr);
        if (u32data != u32Pattern) {
            printf("\nFMC_Read data verify failed at address 0x%x, read=0x%x, expect=0x%x\n", u32Addr, u32data, u32Pattern);
            return -1;
        }
    }
    return 0;
}

int32_t  flash_test(uint32_t u32StartAddr, uint32_t u32EndAddr, uint32_t u32Pattern)
{
    uint32_t    u32Addr;

    for (u32Addr = u32StartAddr; u32Addr < u32EndAddr; u32Addr += FMC_FLASH_PAGE_SIZE) {
        printf("    Flash test address: 0x%x    \r", u32Addr);

        // Erase page
        if (FMC_Erase(u32Addr) < 0) {
            printf("\nPage 0x%x erase failed!\n", u32Addr);
            return -1;
        }

        // Verify if page contents are all 0xFFFFFFFF
        if (verify_data(u32Addr, u32Addr + FMC_FLASH_PAGE_SIZE, 0xFFFFFFFF) < 0) {
            printf("\nPage 0x%x erase verify failed!\n", u32Addr);
            return -1;
        }

        // Write test pattern to fill the whole page
        if (fill_data_pattern(u32Addr, u32Addr + FMC_FLASH_PAGE_SIZE, u32Pattern) < 0) {
            printf("Failed to write page 0x%x!\n", u32Addr);
            FMC_Erase(u32Addr);
            return -1;
        }

        // Verify if page contents are all equal to test pattern
        if (verify_data(u32Addr, u32Addr + FMC_FLASH_PAGE_SIZE, u32Pattern) < 0) {
            printf("\nData verify failed!\n ");
            FMC_Erase(u32Addr);
            return -1;
        }

        // Erase page
        if (FMC_Erase(u32Addr) < 0) {
            printf("\nPage 0x%x erase failed!\n", u32Addr);
            return -1;
        }

        // Verify if page contents are all 0xFFFFFFFF
        if (verify_data(u32Addr, u32Addr + FMC_FLASH_PAGE_SIZE, 0xFFFFFFFF) < 0) {
            printf("\nPage 0x%x erase verify failed!\n", u32Addr);
            return -1;
        }
    }
    printf("\r    Flash Test Passed.          \n");
    return 0;
}


int main()
{
    uint32_t u32Data, au32Config[4];

    /* Lock protected registers */
    if (SYS->REGLCTL == 1) // In end of main function, program issued CPU reset and write-protection will be disabled.
        SYS_LockReg();

    /* Init System, IP clock and multi-function I/O */
    SYS_Init(); //In the end of SYS_Init() will issue SYS_LockReg() to lock protected register. If user want to write protected register, please issue SYS_UnlockReg() to unlock protected register.

    /* Init UART0 for printf */
    UART_Init();

    /* Enable FMC ISP function */
    SYS_UnlockReg();
    FMC_Open();
		
    /* Read Company ID */
    u32Data = FMC_ReadCID();
    if (u32Data != 0xda) {
        printf("Wrong CID: 0x%x\n", u32Data);
        goto lexit;
    }

    /*
     *  Enable Data Flash and set Data Flash base address as DATA_FLASH_TEST_BASE.
     */
    if (set_data_flash_base(DATA_FLASH_TEST_BASE) < 0) {
        printf("Failed to set Data Flash base address!\n");
        goto lexit;
    }

    printf("\n\n");
    printf("+------------------------------------------------------------------------+\n");
    printf("|               Flash Memory Controller Driver Sample Code               |\n");
    printf("+------------------------------------------------------------------------+\n");
    printf("  NOTE: This sample must be applied to ISD9100.\n");

    /* Read BS */
    printf("  Boot Mode .................................. ");
    if (FMC_GetBootSource() == IS_BOOT_FROM_APROM)
        printf("[APROM]\n");
    else {
        printf("[LDROM]\n");
        printf("  WARNING: The driver sample code must execute in AP mode!\n");
        goto lexit;
    }

    u32Data = FMC_ReadCID();
    printf("  Company ID ................................. [0x%08x]\n", u32Data);

    u32Data = FMC_ReadDID();
    printf("  Device ID .................................. [0x%08x]\n", u32Data);

    /* Read Data Flash base address */
    u32Data = FMC_ReadDataFlashBaseAddr();
    printf("  Data Flash Base Address .................... [0x%08x]\n", u32Data);

    FMC_ReadConfig(au32Config, 3);
    printf("  User Config 0 .............................. [0x%08x]\n", au32Config[0]);
    printf("  User Config 1 .............................. [0x%08x]\n", au32Config[1]);
    printf("  User Config 2 .............................. [0x%08x]\n", au32Config[2]);

    //printf("  ISPSTS...................................... [0x%08x]\n", FMC->ISPSTS);

    printf("\n\nLDROM test =>\n");
    FMC_EnableLDUpdate();
    if (flash_test(FMC_LDROM_BASE, FMC_LDROM_END, TEST_PATTERN) < 0) {
        printf("\n\nLDROM test failed!\n");
        goto lexit;
    }
    FMC_DisableLDUpdate();

    printf("\n\nData Flash test =>\n");
    if (flash_test(DATA_FLASH_TEST_BASE, DATA_FLASH_TEST_END, TEST_PATTERN) < 0) {
        printf("\n\nUHB test failed!\n");
        goto lexit;
    }

lexit:

    /* Disable FMC ISP function */
    FMC_Close();

    /* Lock protected registers */
    SYS_LockReg();

    printf("\nFMC Sample Code Completed.\n");

    while (1);
}

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
