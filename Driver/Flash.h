#ifndef __FLASH_H__
#define __FLASH_H__

/* Data/Flash */
// OP_DATA_READ                0x20    // Read data from address A
// OP_DATA_WRITE               0x21    // Write data to address A
// OP_DATA_ERASE               0x22    // Erase data at address A
//
// OP_FLASH_READ               0x28    // Read data from flash address A
// OP_FLASH_WRITE              0x29    // Write data to flash address A
// OP_FLASH_ERASE              0x2A    // Erase data at flash address A

void Data_Read(uint16_t addr, uint8_t *pBuf, uint16_t len);
void Data_Write(uint16_t addr, uint8_t *pBuf, uint16_t len);
void Data_Erase(uint16_t addr, uint8_t *pBuf, uint16_t len);
void Flash_Read(uint16_t addr, uint8_t *pBuf, uint16_t len);
void Flash_Write(uint16_t addr, uint8_t *pBuf, uint16_t len);
void Flash_Erase(uint16_t addr, uint8_t *pBuf, uint16_t len);


#endif /* __FLASH_H__ */

/* vim: set ts=4 sw=4 tw=0 list : */
