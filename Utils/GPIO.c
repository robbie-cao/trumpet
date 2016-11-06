#include "ISD9160.h"
#include "GPIO.h"

/*---------------------------------------------------------------------------------------------------------*/
/* InitialGPIO                                                                                             */
/*---------------------------------------------------------------------------------------------------------*/
void InitialGPIO(void)
{
#if 0
    DrvGPIO_Open(GPB, 0, IO_QUASI);
    DrvGPIO_SetBit(GPB, 0);
    DrvGPIO_Open(GPB, 1, IO_QUASI);
    DrvGPIO_SetBit(GPB, 1);
    DrvGPIO_Open(GPB, 2, IO_QUASI);
    DrvGPIO_SetBit(GPB, 2);
#endif

    // PA13 is used as ALERT from 9160 to 2541
    DrvGPIO_Open(GPA, STATUS_ALERT_PIN, IO_OUTPUT);
    DrvGPIO_SetBit(GPA, STATUS_ALERT_PIN);
}

/* vim: set ts=4 sw=4 tw=0 list : */
