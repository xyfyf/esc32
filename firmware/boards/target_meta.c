#include "target_meta.h"

static const esc_target_meta_t s_meta = {
    .target_id     = ESC_TARGET_ID,
    .mcu_id        = ESC_TARGET_MCU_ID,
    .product_id    = ESC_TARGET_PRODUCT_ID,
    .hw_revision   = ESC_TARGET_HW_REVISION,
    .file_name     = ESC_TARGET_FILE_NAME,
    .firmware_name = ESC_TARGET_FIRMWARE_NAME,
};

const esc_target_meta_t *esc_target_meta(void)
{
    return &s_meta;
}
