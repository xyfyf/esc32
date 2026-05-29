/**
 * @file target_meta.h
 * @brief 编译 Target 元数据（由 boards/targets/<TARGET>/target.h 提供）
 */
#ifndef ESC_TARGET_META_H
#define ESC_TARGET_META_H

#include <stdint.h>
#include "mcu_catalog.h"
#include "product.h"

#ifndef ESC_TARGET_ID
#define ESC_TARGET_ID 0x0001u
#endif

#ifndef ESC_TARGET_FILE_NAME
#define ESC_TARGET_FILE_NAME "ESC_SIM"
#endif

#ifndef ESC_TARGET_FIRMWARE_NAME
#define ESC_TARGET_FIRMWARE_NAME "esc32-sim"
#endif

#ifndef ESC_TARGET_HW_REVISION
#define ESC_TARGET_HW_REVISION 0x0001u
#endif

#ifndef ESC_TARGET_MCU_ID
#define ESC_TARGET_MCU_ID ESC_MCU_ID_BUILD
#endif

#ifndef ESC_TARGET_PRODUCT_ID
#define ESC_TARGET_PRODUCT_ID ESC_PRODUCT_ID
#endif

typedef struct {
    uint16_t target_id;
    uint8_t  mcu_id;
    uint16_t product_id;
    uint16_t hw_revision;
    const char *file_name;
    const char *firmware_name;
} esc_target_meta_t;

const esc_target_meta_t *esc_target_meta(void);

#endif
