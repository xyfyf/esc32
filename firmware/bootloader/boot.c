#include "boot.h"
#include "target.h"
#include "../../shared/protocol/protocol.h"
#include "hal.h"
#include <string.h>

/*
 * OTA staging buffer. Size is target-tunable so MCUs with tight SRAM
 * (G431 / AT32F415 ≤ 32 KiB) can still link cleanly. The default of
 * 64 KiB matches the simulation and G474/H743 builds; tight-RAM targets
 * pre-define ESC_BOOT_IMAGE_BUF_SIZE in their mcu_conf.h.
 */
#ifndef ESC_BOOT_IMAGE_BUF_SIZE
#define ESC_BOOT_IMAGE_BUF_SIZE (64u * 1024u)
#endif

static boot_state_t s_boot;
static uint8_t s_image_buf[ESC_BOOT_IMAGE_BUF_SIZE];

void boot_init(void)
{
    memset(&s_boot, 0, sizeof(s_boot));
    memset(s_image_buf, 0xFF, sizeof(s_image_buf));
}

bool boot_should_stay_in_bl(void)
{
    return hal_gpio_boot_pin_active();
}

bool boot_process_cmd(uint8_t cmd, const uint8_t *payload, uint16_t len,
                      uint8_t *rsp, uint16_t *rsp_len)
{
    *rsp_len = 0;
    switch (cmd) {
    case ESC_CMD_FW_ERASE:
        memset(s_image_buf, 0xFF, sizeof(s_image_buf));
        s_boot.image_size = 0;
        s_boot.erase_done = 1;
        s_boot.ready = 1;
        rsp[0] = ESC_ERR_OK;
        *rsp_len = 1;
        return true;
    case ESC_CMD_FW_WRITE:
        if (len < 6) {
            return false;
        }
        {
            uint32_t off = (uint32_t)payload[0] | ((uint32_t)payload[1] << 8) |
                           ((uint32_t)payload[2] << 16) | ((uint32_t)payload[3] << 24);
            uint16_t n = (uint16_t)(len - 4);
            if (off + n > sizeof(s_image_buf)) {
                rsp[0] = ESC_ERR_PARAM_RANGE;
                *rsp_len = 1;
                return true;
            }
            memcpy(&s_image_buf[off], &payload[4], n);
            if (off + n > s_boot.image_size) {
                s_boot.image_size = off + n;
            }
            rsp[0] = ESC_ERR_OK;
            *rsp_len = 1;
        }
        return true;
    case ESC_CMD_FW_CRC:
        if (len < 4) {
            return false;
        }
        s_boot.image_crc32 = (uint32_t)payload[0] | ((uint32_t)payload[1] << 8) |
                             ((uint32_t)payload[2] << 16) | ((uint32_t)payload[3] << 24);
        if (hal_nvm_write(0x10000u, s_image_buf, s_boot.image_size) == 0) {
            rsp[0] = ESC_ERR_OK;
        } else {
            rsp[0] = ESC_ERR_BUSY;
        }
        *rsp_len = 1;
        return true;
    case ESC_CMD_FW_REBOOT:
        hal_system_reset();
        return true;
    default:
        return false;
    }
}
