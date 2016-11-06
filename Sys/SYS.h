#ifndef __SYS_H__
#define __SYS_H__

/* System */
// OP_SYS_RESET                0x10
// OP_SYS_CONFIG               0x11
// OP_SYS_OPEN                 0x12
// OP_SYS_CLOSE                0x13
//
// OP_SYS_SELFTEST             0x1E
// OP_SYS_DUMP                 0x1F

/* Read/Check Status */
// OP_READ_CHIPID              0x70
// OP_READ_INT                 0x71
// OP_READ_STATUS              0x72
// OP_READ_CHANNEL_STATUS      0x73
// OP_READ_VOL                 0x74
// OP_READ_CONFIG              0x75
//
// OP_CHECK_DEVICE_STATUS      0x78
// OP_CHECK_JOB_QUEUE          0x79
// OP_CHECK_FLASH_TYPE         0x7A
// OP_CHECK_FLASH_STATUS       0x7B

void Sys_Reset(void);
void Sys_SoftReset(void);
void Sys_Config(uint8_t id, uint8_t data);
void Sys_Open(void);
void Sys_Close(void);

void Sys_SelfTest(void);
void Sys_Dump(uint8_t item);


void Sys_ReadChipId(uint8_t* pBuf);
void Sys_ReadInt(uint8_t* pBuf);
void Sys_ReadStatus(uint8_t* pBuf);
void Sys_ReadChannelStatus(uint8_t* pBuf, uint8_t ch);
void Sys_ReadVol(uint8_t* pBuf);
void Sys_ReadConfig(uint8_t* pBuf, uint8_t id);

void Sys_CheckDeviceStatus(uint8_t* pBuf);
void Sys_CheckJobQueue(uint8_t* pBuf);
void Sys_CheckFlashType(uint8_t* pBuf);
void Sys_CheckFlashStatus(uint8_t* pBuf);

void Sys_TimerDelay(uint32_t us);

void InitialSystemClock(void);

#endif /* __SYS_H__ */

/* vim: set ts=4 sw=4 tw=0 list : */
