#ifndef __CONF_H__
#define __CONF_H__

#define VOLUME_MAX_BITS         4
#define VOLUME_MAX              (1 << VOLUME_MAX_BITS)
#define VOLUME_MIN              2
#define VOLUME_DEFAULT          8
#define VOLUME_STEP             2

#define CHANNEL_COUNT           3

#define SAMPLE_RATE             SAMPLE_RATE_SIREN7
#define BIT_RATE                BIT_RATE_32K
#define BANDWIDTH               7000    // G722 speech bandwidth: 50 ~ 7000 Hz


#define I2C_ADDRESS_0           0x50
#define I2C_ADDRESS_1           0x51
#define I2C_ADDRESS_2           0x52
#define I2C_ADDRESS_3           0x53

#define AUDIOBUFFERSIZE         320
#define COMPBUFSIZE             40  // According to BIT_RATE
#define S7DATA_BASE_ADDR_ON_SPI    0
#define FACTOR_BASE_BIT            8
#define FACTOR_BASE 			   128

#define SOUND_DECREASE_THRES_0	192
#define SOUND_DECREASE_THRES_1	64
#define SOUND_DECREASE_THRES_2	8
#define SOUND_DECREASE_STEP_0		8
#define SOUND_DECREASE_STEP_1		8
#define SOUND_DECREASE_STEP_2		8


#endif

/* vim: set ts=4 sw=4 tw=0 list : */
