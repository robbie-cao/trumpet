#include <string.h>
#include <stdio.h>

#include "ISD9100.h"

#include "DrvI2C.h"
#include "OP.h"
#include "Conf.h"
#include "Log.h"

#define I2C_DEBUG               0
#define I2C_DEBUG_OP            0

#if I2C_DEBUG
#define I2C_LOG                 LOG
#else
#define I2C_LOG                 __LOG
#endif

#define LOG_TAG                 "I2C"


#define I2CDATABUFFERSIZE       0x32


// Below variables should be local variable
static uint8_t sTxDataCnt = 0;
static uint8_t sRxDataCnt = 0;
static uint8_t sRxDataBuf[I2CDATABUFFERSIZE];
static uint8_t sTxDataBuf[I2CDATABUFFERSIZE];
static uint8_t sDataReceived = FALSE;


/*---------------------------------------------------------------------------------------------------------*/
/* Function Prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
// I2CFUNC   = 0
// ARBITLOSS = 1
// BUSERROR  = 2
// TIMEOUT   = 3
static void I2C0SlaveCallback_I2cFunc(uint32_t status);
static void I2C0SlaveCallback_ArbitLoss(uint32_t status);
static void I2C0SlaveCallback_BusError(uint32_t status);
static void I2C0SlaveCallback_Timeout(uint32_t status);

static void DrvI2C_Ctrl(I2C_T *port, uint8_t u8start, uint8_t u8stop, uint8_t u8intFlag, uint8_t u8ack)
{

    uint32_t Reg = 0;

    if (u8start)
        Reg |= I2C_STA;
    if (u8stop)
        Reg |= I2C_STO;
    if (u8intFlag)
        Reg |= I2C_SI;
    if (u8ack)
        Reg |= I2C_AA;

    *((__IO uint32_t *)&I2C0->CTL) = (*((__IO uint32_t *)&I2C0->CTL) & ~0x3C) | Reg;
}


static void I2C0SlaveCallback_I2cFunc(uint32_t status)
{
    switch (status) {
        case slvAddrAckR  /* 0x60 */:
        case mstLostArbR  /* 0x68 */:
            // SLA+W has been received and ACK has been returned
            I2C_LOG("S+W\r\n");
            // Use trace log API
            // Log message as short as possible as it may impact timing
            // TODO
            sRxDataCnt = 0;
            DrvI2C_Ctrl(I2C0, 0, 0, 1, 1);
            break;
        case slvDataAckR  /* 0x80 */:
            // DATA has been received and ACK has been returned
            I2C_LOG("DAR\r\n");
            sRxDataBuf[sRxDataCnt++] = I2C_GET_DATA(I2C0);
            DrvI2C_Ctrl(I2C0, 0, 0, 1, 1);

            if (sRxDataCnt >= I2CDATABUFFERSIZE) {
                sRxDataCnt = 0;
            }

            break;
        case slvStopped   /* 0xA0 */:
            // STOP or Repeat START has been received
            I2C_LOG("SP\r\n");
            DrvI2C_Ctrl(I2C0, 0, 0, 1, 1);

            sDataReceived = TRUE;                 // FIXME - Good enough?
            I2C_DataReceiveidHandler();
            break;
        case slvAddrAckW  /* 0xA8 */:
        case mstLostArbW  /* 0xB0 */:
            // SLA+R has been received and ACK has been returned
            I2C_LOG("S+R\r\n");
            sTxDataCnt = 0;
            // First data replied to master
            I2C_SET_DATA(I2C0, sTxDataBuf[sTxDataCnt++]);
            DrvI2C_Ctrl(I2C0, 0, 0, 1, 1);
            break;
        case slvDataAckW  /* 0xB8 */:
            // DATA has been transmitted and ACK has been received
            I2C_LOG("DAW\r\n");
            // The following data replied to master
            I2C_SET_DATA(I2C0, sTxDataBuf[sTxDataCnt++]);
            if (sTxDataCnt > I2CDATABUFFERSIZE - 1) {
                sTxDataCnt = 0;
            }
            DrvI2C_Ctrl(I2C0, 0, 0, 1, 1);
            break;
        case slvDataNackW /* 0xC0 */:
            // DATA has been transmitted and NACK has been received
            I2C_LOG("DNW\r\n");
            DrvI2C_Ctrl(I2C0, 0, 0, 1, 1);
            break;
        case slvLastAckW  /* 0xC8 */:
            // Last DATA has been transmitted and ACK has been received
            I2C_LOG("LAW\r\n");
            DrvI2C_Ctrl(I2C0, 0, 0, 1, 1);
            break;
        case slvAddrAckG  /* 0x70 */:
        case mstLostArbG  /* 0x78 */:
        case slvDataNackR /* 0x88 */:
        case genDataAckR  /* 0x90 */:
        case genDataNackR /* 0x98 */:
        case i2cIdle      /* 0xF8 */:
        default:
            I2C_LOG("NAN - 0x%x\r\n", status);
            break;
    }
}

static void I2C0SlaveCallback_ArbitLoss(uint32_t status)
{
    I2C_LOG("I2C Arbit Loss\r\n");
    // TODO
}

static void I2C0SlaveCallback_BusError(uint32_t status)
{
    I2C_LOG("I2C Bus Error\r\n");
    // TODO
}

static void I2C0SlaveCallback_Timeout(uint32_t status)
{
    I2C_LOG("I2C Timeout\r\n");
    // TODO
}


void I2C_DataReceiveidHandler(void)
{
    uint8_t res = SYS_GOOD;
    uint16_t tmp = 0;
    OpCmd_t *pCmd = (OpCmd_t *)sRxDataBuf;

    (void)res;
#if I2C_DEBUG_OP
    LOGD(LOG_TAG, "RH - 0x%02x %02x %02x %02x %02x %02x\r\n",
            sRxDataBuf[0],
            sRxDataBuf[1],
            sRxDataBuf[2],
            sRxDataBuf[3],
            sRxDataBuf[4],
            sRxDataBuf[5]
            );
#endif

    // Big Endian(CC2541 - 8051) -> Little Endian(ISD9160 - ARM M0)
#if 0
    // FIXME:
    // Not work - WHY?
    tmp = ENDIAN_CONVERT_16(pCmd->vpIdx);
    pCmd->vpIdx = tmp;
#elif 0
    // FIXME:
    // Not work either - STRANGE!
    // Use res to swap Byte 4 and 5 for vpIdx endian conversion
    res = sRxDataBuf[4];
    sRxDataBuf[4] = sRxDataBuf[5];
    sRxDataBuf[5] = res;
#else
    // It works
    tmp = sRxDataBuf[4] | (sRxDataBuf[5] << 8);
    pCmd->vpIdx = tmp;
#endif

#if I2C_DEBUG_OP
    LOGD(LOG_TAG, "L - %d\r\n", pCmd->len);
    LOGD(LOG_TAG, "C - %d\r\n", pCmd->cmd);
    LOGD(LOG_TAG, "D - %d\r\n", pCmd->data);
    LOGD(LOG_TAG, "H - %d\r\n", pCmd->chIdx);
    LOGD(LOG_TAG, "V - %d\r\n", pCmd->vpIdx);
#endif

    res = OP_Handler(pCmd);

    LOGV(LOG_TAG, "Op Handler: %d\r\n", res);

    sRxDataCnt = 0;
}

uint8_t I2C_DataReceived(void)
{
    return sDataReceived;
}

void I2C_DataClear(void)
{
    sDataReceived = FALSE;
}

void I2C_DataTxBufPrepare(uint8_t *pSrc, uint8_t len)
{
    uint8_t size = (len < I2CDATABUFFERSIZE) ? len : I2CDATABUFFERSIZE;
    memcpy(sTxDataBuf, pSrc, size);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C0 IRQ Handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void I2C0_IRQHandler(void)
{
    uint32_t u32Status;

    u32Status = I2C_GET_STATUS(I2C0);

    if (I2C_GET_TIMEOUT_FLAG(I2C0)) {
        /* Clear I2C0 Timeout Flag */
        I2C_ClearTimeoutFlag(I2C0);
    } else {
        I2C0SlaveCallback_I2cFunc(u32Status);
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/* InitialI2C                                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void InitialI2C(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable IP clock */
    CLK_EnableModuleClock(I2C0_MODULE);

    /* GPIO initial and select operation mode for I2C */
    // Set PA.10 = I2C0 SDA; Set PA.11 = I2C0 SCL
    //SYS->GPH_MFPL = SYS_GPH_MFPL_PH3MFP_I2C0_SCL | SYS_GPH_MFPL_PH4MFP_I2C0_SDA ;
    SYS->GPB_MFP  = (SYS->GPB_MFP & (~SYS_GPA_MFP_PA11MFP_Msk) ) | SYS_GPA_MFP_PA11MFP_I2C_SCL;
    SYS->GPB_MFP  = (SYS->GPB_MFP & (~SYS_GPA_MFP_PA10MFP_Msk) ) | SYS_GPA_MFP_PA10MFP_I2C_SDA;

    /* Lock protected registers */
    SYS_LockReg();

    //DrvI2C_Open(I2C0, (DrvSYS_GetHCLK() * 1000), 48000);   // Clock = 48Kbps; as slave this does not matter
    /* Open I2C0 and set clock to 100k */
    I2C_Open(I2C0, 100000);
    /* Get I2C0 Bus Clock */
    printf("I2C clock %d Hz\n", I2C_GetBusClockFreq(I2C0));

    /* Set I2C0 4 Slave Addresses */
    I2C_SetSlaveAddr(I2C0, 0, I2C_ADDRESS_0, I2C_GCMODE_DISABLE);   /* Slave Address : 0x15 */
    I2C_SetSlaveAddr(I2C0, 1, I2C_ADDRESS_1, I2C_GCMODE_DISABLE);   /* Slave Address : 0x35 */
    I2C_SetSlaveAddr(I2C0, 2, I2C_ADDRESS_2, I2C_GCMODE_DISABLE);   /* Slave Address : 0x55 */
    I2C_SetSlaveAddr(I2C0, 3, I2C_ADDRESS_3, I2C_GCMODE_DISABLE);   /* Slave Address : 0x75 */

    /* Set AA bit, I2C0 as slave */
    DrvI2C_Ctrl(I2C0, 0, 0, 0, 1);

    sRxDataCnt = 0;
    memset(sRxDataBuf, 0, sizeof(sRxDataBuf));

#if 0
    /*
     * I2CFUNC   = 0,
     * ARBITLOSS = 1,
     * BUSERROR  = 2,
     * TIMEOUT	 = 3
     */
    DrvI2C_InstallCallback(I2C0, I2CFUNC, I2C0SlaveCallback_I2cFunc);
    DrvI2C_InstallCallback(I2C0, ARBITLOSS, I2C0SlaveCallback_ArbitLoss);
    DrvI2C_InstallCallback(I2C0, BUSERROR, I2C0SlaveCallback_BusError);
    DrvI2C_InstallCallback(I2C0, TIMEOUT, I2C0SlaveCallback_Timeout);
#endif

    I2C_EnableInt(I2C0);
    NVIC_EnableIRQ(I2C0_IRQn);
}

/* vim: set ts=4 sw=4 tw=0 list : */
