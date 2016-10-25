/******************************************************************************
 * @file     ISD9100_isr.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/07/15 2:52p $
 * @brief    ISD9100 ISR source file
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ISD9100.h"

#include "config.h"

/* Write data to Tx FIFO */
#define _DRVI2S_WRITE_TX_FIFO(u32Data)  I2S0->TX = u32Data

/* Write data from Rx FIFO */
#define _DRVI2S_READ_RX_FIFO()   	 	I2S0->RX

/* Read word data number in Tx FIFO */
#define _DRVI2S_READ_TX_FIFO_LEVEL()  	((I2S0->STATUS & I2S_STATUS_TXCNT_Msk) >> I2S_STATUS_TXCNT_Pos)

/* Read word data number in Rx FIFO */
#define _DRVI2S_READ_RX_FIFO_LEVEL()   	((I2S0->STATUS & I2S_STATUS_RXCNT_Msk) >> I2S_STATUS_RXCNT_Pos)

static uint32_t PcmBuff[BUFF_LEN] = {0};
extern uint32_t volatile u32BuffPos;

/*---------------------------------------------------------------------------------------------------------*/
/*  I2S Tx Threshold Level Callback Function when Tx FIFO is less than Tx FIFO Threshold Level             */
/*---------------------------------------------------------------------------------------------------------*/
void Tx_thresholdCallbackfn(uint32_t status)
{
	uint32_t u32Len, i;
	uint32_t * pBuff;

	pBuff = &PcmBuff[0];

	/* Read Tx FIFO free size */
	u32Len = 8 - _DRVI2S_READ_TX_FIFO_LEVEL();

	if (u32BuffPos >= 8)
	{
		for	(i = 0; i < u32Len; i++)
		{
	   		_DRVI2S_WRITE_TX_FIFO(pBuff[i]);
		}

		for (i = 0; i < BUFF_LEN - u32Len; i++)
		{
			pBuff[i] = pBuff[i + u32Len];
		}

		u32BuffPos -= u32Len;
	}
	else
	{
		for	(i = 0; i < u32Len; i++)
		{
	   		_DRVI2S_WRITE_TX_FIFO(0x00);
		}
	}
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2S Rx Threshold Level Callback Function when Rx FIFO is more than Rx FIFO Threshold Level             */
/*---------------------------------------------------------------------------------------------------------*/
void Rx_thresholdCallbackfn(uint32_t status)
{
	uint32_t u32Len, i;
	uint32_t *pBuff;

	if (u32BuffPos < (BUFF_LEN-8))
	{
		pBuff = &PcmBuff[u32BuffPos];

		/* Read Rx FIFO Level */
		u32Len = _DRVI2S_READ_RX_FIFO_LEVEL();

		for ( i = 0; i < u32Len; i++ )
		{
			pBuff[i] = _DRVI2S_READ_RX_FIFO();
		}

		u32BuffPos += u32Len;

		if (u32BuffPos >= BUFF_LEN)
		{
			u32BuffPos =	0;
		}
	}
}

void I2S_IRQHandler(void)
{
    uint32_t u32Reg;

    u32Reg = I2S_GET_INT_FLAG(I2S0, I2S_STATUS_TXIF_Msk | I2S_STATUS_RXIF_Msk);

    if (u32Reg & I2S_STATUS_TXIF_Msk) {
				if (I2S0->IEN & I2S_IEN_TXTHIEN_Msk)
        {
						Tx_thresholdCallbackfn(u32Reg);
            //if (I2SHandler.TxFifoThresholdFn)
				      //  I2SHandler.TxFifoThresholdFn(u32Reg);
        }
    }

    if (u32Reg & I2S_STATUS_RXIF_Msk) {
				if (I2S0->IEN & I2S_IEN_RXTHIEN_Msk)
        {
						Rx_thresholdCallbackfn(u32Reg);
            //if (I2SHandler.RxFifoThresholdFn)
              //  I2SHandler.RxFifoThresholdFn(u32Reg);
				}
    }
}

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
