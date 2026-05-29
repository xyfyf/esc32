#ifndef ESC_BOOT_H
#define ESC_BOOT_H

#include <stdint.h>
#include <stdbool.h>

#define BOOT_FLASH_APP_BASE   0x08004000u
#define BOOT_FLASH_PAGE_SIZE  2048u

typedef struct {
    uint32_t image_size;
    uint32_t image_crc32;
    uint8_t  erase_done;
    uint8_t  ready;
} boot_state_t;

void boot_init(void);
bool boot_process_cmd(uint8_t cmd, const uint8_t *payload, uint16_t len,
                      uint8_t *rsp, uint16_t *rsp_len);
bool boot_should_stay_in_bl(void);

#endif
