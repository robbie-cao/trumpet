#include "ISD9160.h"
#include "UART.h"
#include "ATC.h"

#define USE_DRV_UART_API    1

#define DEBUG_UART          0
#if DEBUG_UART
#define LOGD_U              LOGD
#else
#define LOGD_U              __LOGD
#endif


#define RXBUFSIZE           64

#define LOG_TAG             "UART"


#if USE_DRV_UART_API
/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
static uint8_t sBuf[RXBUFSIZE + 1];
static uint8_t sPos = 0;

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
static void UART_INT_HANDLE(uint32_t u32IntStatus);

/*---------------------------------------------------------------------------------------------------------*/
/* UART Callback function                                                                                  */
/*---------------------------------------------------------------------------------------------------------*/
static void UART_INT_HANDLE(uint32_t u32IntStatus)
{
    uint8_t bInChar[1] = { 0xFF };

    if (u32IntStatus & RDAIE) {
        /* Get all the input characters */
        while (UART0->ISR.RDA_IF == 1) {
            /* Get the character from UART Buffer */
            DrvUART_Read(UART_PORT0, bInChar, 1);
#if DEBUG_UART
            LOGD(LOG_TAG, "%d - %c\r\n", sPos, bInChar[0]);
#else
            /* Echo the recieved char */
            LOG("%c", bInChar[0]);
#endif

            /* Check if buffer full */
            if (sPos < RXBUFSIZE) {
                /* Enqueue the character */
                sBuf[sPos++] = bInChar[0];
            }
            sBuf[sPos] = '\0';
            if (strstr((char *)sBuf, "\r") || strstr((char *)sBuf, "\n") || sPos >= RXBUFSIZE) {
                LOG("\r\n");
                ATC_Handler(sBuf);
                sPos = 0;
            }
        }
    }
#if 0
    if (u32IntStatus & THREIE) {
        // Do something if transmit FIFO register empty
    }
    if (u32IntStatus & BUFERRIE) {
        // Do something if Tx or Rx FIFO overflows
    }
#endif

}
#endif

/*---------------------------------------------------------------------------------------------------------*/
/* InitialUART                                                                                             */
/*---------------------------------------------------------------------------------------------------------*/
void InitialUART(void)
{
#if USE_DRV_UART_API
    STR_UART_T sParam;

    /* Step 1. Enable and Select UART clock source*/
    UNLOCKREG();
    SYSCLK->PWRCON.OSC49M_EN = 1;
    SYSCLK->PWRCON.OSC10K_EN = 1;
    SYSCLK->PWRCON.XTL32K_EN = 1;
    SYSCLK->CLKSEL0.STCLK_S  = 3;   // Use internal HCLK

    SYSCLK->CLKSEL0.HCLK_S   = 0;   /* Select HCLK source as 48MHz */
    SYSCLK->CLKDIV.HCLK_N    = 0;   /* Select no division          */
    SYSCLK->CLKSEL0.OSCFSel  = 0;   /* 1 = 32MHz, 0 = 48MHz */
    LOCKREG();

    /* Step 2. GPIO initial */
    DrvGPIO_InitFunction(FUNC_UART0);

    /* Step 3. Select UART Operation mode */
    sParam.u32BaudRate       = 115200;
    sParam.u8cDataBits       = DRVUART_DATABITS_8;
    sParam.u8cStopBits       = DRVUART_STOPBITS_1;
    sParam.u8cParity         = DRVUART_PARITY_NONE;
    sParam.u8cRxTriggerLevel = DRVUART_FIFO_1BYTES;

    if (DrvUART_Open(UART_PORT0, &sParam) == 0) {
        printf("\r\n----------------------------------------\r\n");

        /* Step 4. Enable Interrupt and install the call back function */

        DrvUART_EnableInt(
                UART_PORT0,
                (/*DRVUART_THREINT |*/ DRVUART_RDAINT),
                UART_INT_HANDLE
                );
    }

#else

    /* Reset IP */
    SYS->IPRSTC2.UART0_RST = 1;
    SYS->IPRSTC2.UART0_RST = 0;

    /* Enable UART clock */
    SYSCLK->APBCLK.UART0_EN = 1;

    /* Data format */
    UART0->LCR.WLS = 3;

    /* Configure the baud rate */
    M32(&UART0->BAUD) = 0x3F0001A8; /* Internal 48MHz, 115200 bps */


    /* Multi-Function Pin: Enable UART0:Tx Rx */
    SYS->GPA_ALT.GPA8 = 1;
    SYS->GPA_ALT.GPA9 = 1;
#endif
}

/* vim: set ts=4 sw=4 tw=0 list : */
