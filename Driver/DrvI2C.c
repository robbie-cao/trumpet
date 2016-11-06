#include "ISD9160.h"
#include "I2C.h"
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
            DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 1);
            break;
        case slvDataAckR  /* 0x80 */:
            // DATA has been received and ACK has been returned
            I2C_LOG("DAR\r\n");
            sRxDataBuf[sRxDataCnt++] = DrvI2C_ReadData(I2C_PORT0);
            DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 1);

            if (sRxDataCnt >= I2CDATABUFFERSIZE) {
                sRxDataCnt = 0;
            }

            break;
        case slvStopped   /* 0xA0 */:
            // STOP or Repeat START has been received
            I2C_LOG("SP\r\n");
            DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 1);

            sDataReceived = TRUE;                 // FIXME - Good enough?
            I2C_DataReceiveidHandler();
            break;
        case slvAddrAckW  /* 0xA8 */:
        case mstLostArbW  /* 0xB0 */:
            // SLA+R has been received and ACK has been returned
            I2C_LOG("S+R\r\n");
            sTxDataCnt = 0;
            // First data replied to master
            DrvI2C_WriteData(I2C_PORT0, sTxDataBuf[sTxDataCnt++]);
            DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 1);
            break;
        case slvDataAckW  /* 0xB8 */:
            // DATA has been transmitted and ACK has been received
            I2C_LOG("DAW\r\n");
            // The following data replied to master
            DrvI2C_WriteData(I2C_PORT0, sTxDataBuf[sTxDataCnt++]);
            if (sTxDataCnt > I2CDATABUFFERSIZE - 1) {
                sTxDataCnt = 0;
            }
            DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 1);
            break;
        case slvDataNackW /* 0xC0 */:
            // DATA has been transmitted and NACK has been received
            I2C_LOG("DNW\r\n");
            DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 1);
            break;
        case slvLastAckW  /* 0xC8 */:
            // Last DATA has been transmitted and ACK has been received
            I2C_LOG("LAW\r\n");
            DrvI2C_Ctrl(I2C_PORT0, 0, 0, 1, 1);
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
/* InitialI2C                                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void InitialI2C(void)
{
    /* GPIO initial and select operation mode for I2C */
    DrvGPIO_InitFunction(FUNC_I2C0);                // Set PA.10 = I2C0 SDA; Set PA.11 = I2C0 SCL

    DrvI2C_Open(I2C_PORT0, (DrvSYS_GetHCLK() * 1000), 48000);   // Clock = 48Kbps; as slave this does not matter

    DrvI2C_SetAddress(I2C_PORT0, 0, I2C_ADDRESS_0, 0x00);    // Address is 0b1010000 = 0x50, addr0[0] = 1 --> slave mode,
    DrvI2C_SetAddress(I2C_PORT0, 1, I2C_ADDRESS_1, 0x00);    // Address is 0b1010001 = 0x51, addr0[0] = 1 --> slave mode,
    DrvI2C_SetAddress(I2C_PORT0, 2, I2C_ADDRESS_2, 0x00);    // Address is 0b1010010 = 0x52, addr0[0] = 1 --> slave mode,
    DrvI2C_SetAddress(I2C_PORT0, 3, I2C_ADDRESS_3, 0x00);    // Address is 0b1010011 = 0x53, addr0[0] = 1 --> slave mode,

    /* Set AA bit, I2C0 as slave */
    DrvI2C_Ctrl(I2C_PORT0, 0, 0, 0, 1);

    sRxDataCnt = 0;
    memset(sRxDataBuf, 0, sizeof(sRxDataBuf));

    DrvI2C_EnableInt(I2C_PORT0);                    // Enable I2C0 interrupt and set corresponding NVIC bit

    /*
     * I2CFUNC   = 0,
     * ARBITLOSS = 1,
     * BUSERROR  = 2,
     * TIMEOUT	 = 3
     */
    DrvI2C_InstallCallback(I2C_PORT0, I2CFUNC, I2C0SlaveCallback_I2cFunc);
    DrvI2C_InstallCallback(I2C_PORT0, ARBITLOSS, I2C0SlaveCallback_ArbitLoss);
    DrvI2C_InstallCallback(I2C_PORT0, BUSERROR, I2C0SlaveCallback_BusError);
    DrvI2C_InstallCallback(I2C_PORT0, TIMEOUT, I2C0SlaveCallback_Timeout);
}

/* vim: set ts=4 sw=4 tw=0 list : */
