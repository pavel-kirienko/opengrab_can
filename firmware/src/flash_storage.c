/*
 * Pavel Kirienko, 2013 (pavel.kirienko@gmail.com)
 */

#include "sys/sys.h"
#include <string.h>
#include <errno.h>
#include <stm32f10x_flash.h>

#ifndef F_SIZE
#   define F_SIZE           (*((uint16_t*)0x1FFFF7E0))
#endif
#define FLASH_END           ((F_SIZE * 1024) + FLASH_BASE)

#if defined (STM32F10X_HD) || defined (STM32F10X_HD_VL) || defined (STM32F10X_CL) || defined (STM32F10X_XL)
#   define FLASH_PAGE_SIZE  0x800
#else
#   define FLASH_PAGE_SIZE  0x400
#endif

#define FLASH_ADR_DATA      (FLASH_END - FLASH_PAGE_SIZE)
#define FLASH_ADR_CRC       (FLASH_END - 4)
#define DATA_SIZE_MAX       (FLASH_PAGE_SIZE - 4)

static int _read(void* pdata, int len, uint32_t* pcrc)
{
    ASSERT_ALWAYS(pdata);
    ASSERT_ALWAYS(pcrc);

    memcpy(pdata, (void*)FLASH_ADR_DATA, len);
    *pcrc = *((uint32_t*)FLASH_ADR_CRC);
    return 0;
}

static int _erase(void)
{
    int result = 0;

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

    if (FLASH_ErasePage(FLASH_ADR_DATA) != FLASH_COMPLETE)
    {
        DBGTRACE("flashstorage", "page erase failed");
        result = -EIO;
    }

    FLASH_Lock();
    return result;
}

static int _write(const void* pdata, int len, uint32_t crc)
{
    ASSERT_ALWAYS(pdata);

    int result = 0;

    // Size alignment
    ASSERT(((size_t)pdata) % 2 == 0);       // Make sure the data is aligned
    const uint16_t* data_halfwords = pdata;

    int num_data_halfwords = len / 2;
    if (num_data_halfwords * 2 < len)
        num_data_halfwords += 1;

    ASSERT(num_data_halfwords * 2 >= len);
    ASSERT(num_data_halfwords * 2 <= DATA_SIZE_MAX);

    // Erase the storage sector
    result = _erase();
    if (result)
        goto leave;

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

    // Write the data
    size_t next_flash_address = FLASH_ADR_DATA;
    for (int i = 0; i < num_data_halfwords; i++)
    {
        const FLASH_Status flash_status = FLASH_ProgramHalfWord(next_flash_address, data_halfwords[i]);
        next_flash_address += 2;
        if (flash_status != FLASH_COMPLETE)
        {
            DBGTRACE("flashstorage", "data write failed");
            result = -EIO;
            goto leave;
        }
    }

    // Write CRC
    const FLASH_Status flash_status = FLASH_ProgramWord(FLASH_ADR_CRC, crc);
    if (flash_status != FLASH_COMPLETE)
    {
        DBGTRACE("flashstorage", "CRC write failed");
        result = -EIO;
        goto leave;
    }

    // Check the correctness
    if ((0 != memcmp((void*)FLASH_ADR_DATA, pdata, len)) ||
        (crc != *((uint32_t*)FLASH_ADR_CRC)))
    {
        DBGTRACE("flashstorage", "write OK, postcheck failed");
        result = -EIO;
        goto leave;
    }

    DBGTRACE("flashstorage", "%d bytes written OK", len);

leave:
    FLASH_Lock();
    return result;
}

// http://www.hackersdelight.org/hdcodetxt/crc.c.txt
static uint32_t _crc32(const void* pdata, int len)
{
    ASSERT_ALWAYS(pdata);
    ASSERT_ALWAYS(len >= 0);

    uint32_t crc = 0xFFFFFFFF;
    const uint8_t* pbyte = (const uint8_t*)pdata;
    while (len--)
    {
        const uint32_t byte32 = *pbyte++;
        crc = crc ^ byte32;
        for (int j = 7; j >= 0; j--)
        {
            uint32_t mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }
    return ~crc;
}

int flashStorageRead(void* dst, int len)
{
    if (len < 0 || len > DATA_SIZE_MAX || dst == NULL)
        return -EINVAL;

    uint32_t crc = 0;
    const int res = _read(dst, len, &crc);
    if (res)
        return res;

    if (_crc32(dst, len) != crc)
    {
        DBGTRACE("flashstorage", "read invalid checksum 0x%08x", (unsigned int)crc);
        return -EFTYPE;
    }
    return 0;
}

int flashStorageWrite(const void* src, int len)
{
    if (len < 0 || len > DATA_SIZE_MAX || src == NULL)
        return -EINVAL;

    const uint32_t crc = _crc32(src, len);
    return _write(src, len, crc);
}

int flashStorageErase(void)
{
    return _erase();
}
