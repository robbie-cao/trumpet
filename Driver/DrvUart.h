#ifndef __DRVUART_H__
#define __DRVUART_H__

#include "ISD9100.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Port Number                                                                                             */
/*---------------------------------------------------------------------------------------------------------*/
#define DRVUART_PORT0		0x000


/*---------------------------------------------------------------------------------------------------------*/
/* define UART line status control			                                                         	   */
/*---------------------------------------------------------------------------------------------------------*/
#define DRVUART_LININT		LINIE
#define DRVUART_WAKEUPINT	WAKEIE
#define DRVUART_BUFERRINT	BUFERRIE
#define DRVUART_TOUTINT		RTOIE
#define DRVUART_MOSINT		MSIE
#define DRVUART_RLSNT		RLSIE
#define DRVUART_THREINT		THREIE
#define DRVUART_RDAINT		RDAIE

/*---------------------------------------------------------------------------------------------------------*/
/* DATA BIT                                                                                                */
/*---------------------------------------------------------------------------------------------------------*/
#define DRVUART_DATABITS_5		0x0
#define DRVUART_DATABITS_6		0x1
#define DRVUART_DATABITS_7		0x2
#define DRVUART_DATABITS_8		0x3

/*---------------------------------------------------------------------------------------------------------*/
/* PARITY Setting                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
#define DRVUART_PARITY_NONE		0x0
#define DRVUART_PARITY_ODD		0x1
#define DRVUART_PARITY_EVEN		0x3
#define DRVUART_PARITY_MARK		0x5
#define DRVUART_PARITY_SPACE	0x7

/*---------------------------------------------------------------------------------------------------------*/
/* STOP BIT                                                                                                */
/*---------------------------------------------------------------------------------------------------------*/
#define DRVUART_STOPBITS_1		0x0
#define DRVUART_STOPBITS_1_5	0x1
#define DRVUART_STOPBITS_2		0x1

/*---------------------------------------------------------------------------------------------------------*/
/* FIFO Select                                                                                             */
/*---------------------------------------------------------------------------------------------------------*/
#define DRVUART_FIFO_1BYTES		0x0
#define DRVUART_FIFO_4BYTES		0x1
#define DRVUART_FIFO_8BYTES		0x2
#define DRVUART_FIFO_14BYTES	0x3
#define DRVUART_FIFO_30BYTES	0x4
#define DRVUART_FIFO_46BYTES	0x5
#define DRVUART_FIFO_62BYTES	0x6

/*---------------------------------------------------------------------------------------------------------*/
/* Clock Source                                                                                            */
/*---------------------------------------------------------------------------------------------------------*/
#define DRVUART_CLKSRC_EXT			0x00
#define DRVUART_CLKSRC_PLL			0x40
#define DRVUART_CLKSRC_PLL_DIV2		0x80


/*---------------------------------------------------------------------------------------------------------*/
/* Define UART Macro		                                                                         	   */
/*---------------------------------------------------------------------------------------------------------*/
#define DMA_LINIE    	BIT23			/* LIN RX Break Field Detected Interrupt Enable */
#define DMA_BUFERRIE    BIT21			/* Buffer Error Interrupt Enable  */
#define DMA_RTOIE       BIT20			/* RX Time out Interrupt Enable	*/
#define DMA_MSIE        BIT19			/* MODEM Status Interrupt (Irpt_MOS) Enable	 */
#define DMA_RLSIE       BIT18			/* Receive Line Status Interrupt (Irpt_RLS) Enable */

#define LINIE           BIT7			/* LIN RX Break Field Detected Interrupt Enable */
#define WAKEIE          BIT6			/* Wake up interrupt enable */
#define BUFERRIE        BIT5			/* Buffer Error Interrupt Enable  */
#define RTOIE           BIT4			/* RX Time out Interrupt Enable	*/
#define MSIE            BIT3			/* MODEM Status Interrupt (Irpt_MOS) Enable	 */
#define RLSIE           BIT2			/* Receive Line Status Interrupt (Irpt_RLS) Enable */
#define THREIE          BIT1			/* Transmit Holding Register Empty Interrupt (Irpt_THRE) Enable */
#define RDAIE           BIT0			/* Receive Data Available Interrupt (Irpt_RDA) Enable and */
/* Time-out Interrupt (Irpt_TOUT) Enable */

#define MODE_TX  0
#define MODE_RX  1
/*---------------------------------------------------------------------------------------------------------*/
/*  Define UART initialization data structure                                                              */
/*---------------------------------------------------------------------------------------------------------*/
typedef enum
{
    MODE_UART = 0,
    MODE_IRCR = 1,
    MODE_LIN  = 2
} MODE_SEL;
void UART_Init(void);

#endif /* __DRVUART_H__ */

/* vim: set ts=4 sw=4 tw=0 nolist : */
