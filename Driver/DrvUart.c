#include <string.h>
#include <stdio.h>

#include "ISD9100.h"

#include "mtypes.h"
#include "DrvUart.h"
#include "ATC.h"

#define USE_DRV_UART_API    1

#define DEBUG_UART          0
#if DEBUG_UART
#define LOGD_U              LOGD
#else
#define LOGD_U              __LOGD
#endif

#define LOG     printf

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


/*---------------------------------------------------------------------------------------------------------*/
/* Interrupt Handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void UART0_IRQHandler(void)
{
    uint8_t bInChar[1] = { 0xFF };
    uint32_t u32uart0IntStatus;

    u32uart0IntStatus = inpw(&UART0->INTSTS) ;

    if (u32uart0IntStatus & RDAIE) {
        /* Get all the input characters */
        while (UART0->INTSTS & BIT0) { /* RDAIF = 1 */
            /* Get the character from UART Buffer */
            UART_Read(UART0, bInChar, 1);
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
    if (u32uart0IntStatus & THREIE) {
        // Do something if transmit FIFO register empty
    }
    if (u32uart0IntStatus & BUFERRIE) {
        // Do something if Tx or Rx FIFO overflows
    }
#endif

}
#endif

/*---------------------------------------------------------------------------------------------------------*/
/* Initial UART                                                                                            */
/*---------------------------------------------------------------------------------------------------------*/
void UART_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set GPG multi-function pins for UART0 RXD and TXD */
    SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA8MFP_Msk) ) | SYS_GPA_MFP_PA8MFP_UART_TX;
    SYS->GPA_MFP  = (SYS->GPA_MFP & (~SYS_GPA_MFP_PA9MFP_Msk) ) | SYS_GPA_MFP_PA9MFP_UART_RX;

    /* Lock protected registers */
    SYS_LockReg();

    /* Reset IP */
    CLK_EnableModuleClock(UART_MODULE);
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 Baudrate(115200) */
    UART_Open(UART0, 115200);

    UART_ENABLE_INT(UART0, UART_INTEN_RDAIEN_Msk);
    NVIC_EnableIRQ(UART0_IRQn);
}

/* vim: set ts=4 sw=4 tw=0 list : */
