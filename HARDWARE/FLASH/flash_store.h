/**
 * flash_store.h - Persistent storage (STM32F407 internal flash)
 * Sector 11 (0x080E0000, 128KB) for data, survives power loss.
 */
#ifndef __FLASH_STORE_H
#define __FLASH_STORE_H
#include "stm32f4xx.h"

#define FLASH_STORE_ADDR    0x08020000
#define FLASH_STORE_SECTOR  FLASH_Sector_5
#define FLASH_STORE_MAGIC   0xD07D07D0   /* D题 D题 D0 */


typedef struct {
    uint32_t magic;
    uint8_t  nofly_bitmap[80];
    uint8_t  target_grid[80];
    uint16_t target_counts[3];
    uint16_t mission_elapsed;
    uint8_t  exit_col, exit_row;
    uint8_t  guide_path[80];       /* 引导线: col0,row0,col1,row1,... */
    uint8_t  guide_path_len;       /* 实际点数 */
    uint8_t  reserved[1];
    uint32_t crc;
} PersistentData;

/* ── public API ── */
uint8_t  FlashStore_Load(PersistentData *pd);        /* 1=valid data loaded, 0=invalid/empty */
void     FlashStore_Save(const PersistentData *pd);   /* save current state to flash */
void     FlashStore_Erase(void);                      /* wipe saved data */
uint8_t  FlashStore_HasData(void);                    /* check if flash has valid saved data */
uint32_t FlashStore_CRC32(const uint8_t *data, uint32_t len);  /* checksum helper */

#endif
