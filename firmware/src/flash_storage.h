/*
 * Generic flash storage for STM32 with checksum protection.
 * Data is allocated inside the last flash page; data length should not exceed (FLASH_PAGE_SIZE - 4).
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#ifndef FLASH_STORAGE_H_
#define FLASH_STORAGE_H_

/**
 * These functions return 0 on success or standard negative error code.
 * @{
 */
int flashStorageRead(void* dst, int len);
int flashStorageWrite(const void* src, int len);
int flashStorageErase(void);
/**
 * @}
 */

#endif
