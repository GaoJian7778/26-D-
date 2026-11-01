/**
 * flash_store.c - Persistent storage implementation
 * Uses STM32F407 internal flash sector 11 (128KB).
 */
#include "stm32f4xx.h"
#include "stm32f4xx_flash.h"
#include "flash_store.h"
#include <string.h>
#include <stddef.h>

/* ── CRC32 for data integrity ── */
uint32_t FlashStore_CRC32(const uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFF;
    for (uint32_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
    }
    return ~crc;
}

uint8_t FlashStore_HasData(void)
{
    const PersistentData *pd = (const PersistentData *)FLASH_STORE_ADDR;
    if (pd->magic != FLASH_STORE_MAGIC) return 0;

    uint32_t calc = FlashStore_CRC32((const uint8_t *)pd, offsetof(PersistentData, crc));
    return (calc == pd->crc) ? 1 : 0;
}

uint8_t FlashStore_Load(PersistentData *pd)
{
    if (!FlashStore_HasData()) return 0;
    memcpy(pd, (const void *)FLASH_STORE_ADDR, sizeof(PersistentData));
    return 1;
}

void FlashStore_Erase(void)
{
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    FLASH_EraseSector(FLASH_STORE_SECTOR, VoltageRange_3);
    FLASH_Lock();
}

void FlashStore_Save(const PersistentData *pd)
{
    FlashStore_Erase();  /* must erase before write */

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    const uint32_t *src = (const uint32_t *)pd;
    uint32_t addr = FLASH_STORE_ADDR;

    for (uint32_t i = 0; i < (sizeof(PersistentData) + 3) / 4; i++) {
        FLASH_ProgramWord(addr, src[i]);
        addr += 4;
    }

    FLASH_Lock();
}
