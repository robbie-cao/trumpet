/**************************************************************************//**
 * @file     TouchPad.h
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/07/08 11:00a $
 * @brief    ISD9100 Touch Pad(ISD9160_TOUCH_REV-A) Header File
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

#ifndef __TOUCHPAD_H__
#define __TOUCHPAD_H__

#include "ISD9100.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define TOUCHPAD_MAX_KEY_COUNT       (8)
#define TOUCHPAD_CAL_AVERAGE_COUNT   (16)
#define TOUCHPAD_LOW_STEP            (30)
#define TOUCHPAD_GRAND_COUNT         (2)

void     TouchPad_Scan(void);
void     TouchPad_Initiate( uint16_t u16Pin );
uint16_t TouchPad_GetStatus(void);

#ifdef __cplusplus
}
#endif
	
#endif

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
