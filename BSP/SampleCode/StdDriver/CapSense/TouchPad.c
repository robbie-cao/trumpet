/**************************************************************************//**
 * @file     TouchPad.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/07/29 10:00a $
 * @brief    This touch sensing method is only adopted on ISD-9160_TOUCH board.
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include "TouchPad.h"

typedef struct
{
	uint8_t u8StatusFlag;
	uint8_t u8TempCount;
	uint8_t u8PressedCounter;
	uint8_t u8ReleaseCounter;
	int32_t i32LowerValue;
	int32_t i32CurrentValue;
	
}S_TOUCHPAD_HANDLER;

const uint16_t THRESHOLD_PRESS[TOUCHPAD_MAX_KEY_COUNT] = {800,1200,750,800,800,750,800,1200};

const uint16_t THRESHOLD_RELEASE[TOUCHPAD_MAX_KEY_COUNT] = {350,200,300,350,350,300,350,200};

S_TOUCHPAD_HANDLER sTouchPad_Handler[TOUCHPAD_MAX_KEY_COUNT];

uint16_t u16PadStatus, u16PadEnablePin;

uint8_t u8PadIndex;

volatile uint8_t u8CapSenseIRQFlag;

/**
  * @brief  This function is capture sense's interrupt handler. 
  * @param  None.
  * @return None
  */
void CAPS_IRQHandler (void)
{
    u8CapSenseIRQFlag = 1;
	CAPSENSE_DISABLE_INTERRUPT();
}

/**
  * @brief  This function is to reset capture sense's counter & interrupt control(internal function). 
  * @param  None.
  * @return None
  */
void TouchPad_Reset(void)
{
	CapSense_ResetCounter();
	CAPSENSE_ENABLE_INTERRUPT();
	CAPSENSE_ENABLE();
}

/**
  * @brief  This function is to calibrate touch pad initiate value(internal function). 
  * @param  None.
  * @return None
  */
void TouchPad_Calibration(void)
{
	uint8_t u8i, u8j;
	uint32_t u32TmpCounter;
	
	for( u8i=0; u8i<TOUCHPAD_MAX_KEY_COUNT; u8i++)
    {
		if( u16PadEnablePin&(1<<u8i) )
		{
			ACMP->POSSEL = u8PadIndex<<ACMP_POSSEL_POSSEL_Pos;
			ACMP->CTL0 &= (~ACMP_CTL0_NEGSEL_Msk);
			ACMP->CTL0 |= ACMP_CTL0_ACMPEN_Msk;
			u32TmpCounter=0;
			
			for( u8j=0; u8j<TOUCHPAD_CAL_AVERAGE_COUNT; u8j++)
			{
				TouchPad_Reset();
				while ( u8CapSenseIRQFlag==0 );
				u32TmpCounter += CapSense_GetCounter();
				u8CapSenseIRQFlag = 0;
			}
			sTouchPad_Handler[u8i].i32CurrentValue = u32TmpCounter/TOUCHPAD_CAL_AVERAGE_COUNT;
			sTouchPad_Handler[u8i].i32LowerValue = sTouchPad_Handler[u8i].i32CurrentValue;
			sTouchPad_Handler[u8i].u8TempCount = 0;
		}
	}	
}

/**
  * @brief  This function is to initiate touch pad controlling for ISD9100 
  * @param  u16Pin is enable pin of capture sense.
  * @return None
  */
void TouchPad_Initiate( uint16_t u16Pin )
{
	/* reset variable(capture sense interrupt flag, key flag, current pad index) */
	u8CapSenseIRQFlag = 0;
	u8PadIndex = 0;
	u16PadStatus = 0; 
	u16PadEnablePin = u16Pin;
	
	/* depend on chip, ISD9100 analog compare gpio (GPB0~GPB7) = 8 */ 
	GPIO_SetMode( PB, u16PadEnablePin&0x00FF, GPIO_MODE_INPUT ); 

	/* enable gpio current source */
	CAPSENSE_ENABLE_CURRENT_SOURCE_PIN(	u16PadEnablePin );
	
	/* select source value for capture sense */
	CapSense_SelectCurrentSourceValue(CAPSENSE_CURCTL0_VALSEL_1000NA);
	
	/* set control configuration(cycle count & low time) */
    CapSense_SetCycleCounts( 4, CAPSENSE_CTRL_LOWTIME_8CYCLES );
	
	NVIC_EnableIRQ(CAPS_IRQn);

	/* calibration touch pad value */
	TouchPad_Calibration();
	
	/* reset counter & interrupt control for preper scanning */
	TouchPad_Reset();
}

/**
  * @brief  This function is to scan pad pressing or releasing. 
  *         Please locate this function into main loop or cycle interrupt. 
  * @param  None.
  * @return None
  */
void TouchPad_Scan(void)
{
	if ( u8CapSenseIRQFlag )
	{
		if( u16PadEnablePin&(1<<u8PadIndex) )
		{
			sTouchPad_Handler[u8PadIndex].i32CurrentValue = CapSense_GetCounter();
			
			/* Pad Key in Pressing state */
			if ((sTouchPad_Handler[u8PadIndex].i32CurrentValue - sTouchPad_Handler[u8PadIndex].i32LowerValue) > THRESHOLD_PRESS[u8PadIndex])
			{
				sTouchPad_Handler[u8PadIndex].u8PressedCounter++;
				sTouchPad_Handler[u8PadIndex].u8ReleaseCounter=0;
				if(( (u16PadStatus&(1<<u8PadIndex))==0 )&&(sTouchPad_Handler[u8PadIndex].u8PressedCounter>TOUCHPAD_GRAND_COUNT))
				{
					u16PadStatus |= (1<<u8PadIndex);
					sTouchPad_Handler[u8PadIndex].u8PressedCounter=0;
					sTouchPad_Handler[u8PadIndex].u8ReleaseCounter=0;
				}

			}
			/* Pad Key in Releasing state */
			else if ((sTouchPad_Handler[u8PadIndex].i32CurrentValue - sTouchPad_Handler[u8PadIndex].i32LowerValue) < THRESHOLD_RELEASE[u8PadIndex])
			{
				sTouchPad_Handler[u8PadIndex].u8ReleaseCounter++;
				sTouchPad_Handler[u8PadIndex].u8PressedCounter=0;
				if( ( (u16PadStatus&(1<<u8PadIndex))!=0 ) && (sTouchPad_Handler[u8PadIndex].u8ReleaseCounter>TOUCHPAD_GRAND_COUNT))
				{
					u16PadStatus &= ~(1<<u8PadIndex);
					sTouchPad_Handler[u8PadIndex].u8ReleaseCounter=0;
					sTouchPad_Handler[u8PadIndex].u8PressedCounter=0;
				}
				if((sTouchPad_Handler[u8PadIndex].i32LowerValue - sTouchPad_Handler[u8PadIndex].i32CurrentValue)> TOUCHPAD_LOW_STEP)
				{
					sTouchPad_Handler[u8PadIndex].u8TempCount++;
					if( sTouchPad_Handler[u8PadIndex].u8TempCount>TOUCHPAD_GRAND_COUNT)
					{
						sTouchPad_Handler[u8PadIndex].i32LowerValue -= TOUCHPAD_LOW_STEP;
						sTouchPad_Handler[u8PadIndex].u8TempCount=0;
					}
				}
				else
					sTouchPad_Handler[u8PadIndex].u8TempCount=0;
			}
		}

		u8CapSenseIRQFlag=0;

		if( (++u8PadIndex) >= TOUCHPAD_MAX_KEY_COUNT)
			u8PadIndex = 0;
		
		ACMP->POSSEL = u8PadIndex<<ACMP_POSSEL_POSSEL_Pos;
		
		TouchPad_Reset();
	}
}

/**
  * @brief  This function is to get pad press state.
  *         1:Pressing, 0:Releasing.
  * @param  None.
  * @return Pad status(16 keys)
  */
uint16_t TouchPad_GetStatus(void)
{
	return u16PadStatus;
}
