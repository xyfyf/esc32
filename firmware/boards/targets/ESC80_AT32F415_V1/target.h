/**
 * Target: ESC-80 + AT32F415 (cost variant / mid-small ESC)
 */
#ifndef ESC_TARGET_ESC80_AT32F415_H
#define ESC_TARGET_ESC80_AT32F415_H

#define ESC_TARGET_FILE_NAME       "ESC80_AT32F415_V1"
#define ESC_TARGET_FIRMWARE_NAME   "ESC-80-AT415"
#define ESC_TARGET_ID              0x8082u
#define ESC_TARGET_HW_REVISION     0x0001u
#define ESC_TARGET_MCU_ID          ESC_MCU_AT32F415
#define ESC_TARGET_PRODUCT_ID      ESC_PRODUCT_ESC80

#define ESC_TARGET_PINMAP_HEADER   "boards/mcu/at32f415/pinmap.h"
#define ESC_TARGET_HAL_SOURCE      "boards/mcu/at32f415/hal_stub.c"
#define ESC_TARGET_MCU_CONF_HEADER "boards/mcu/at32f415/mcu_conf.h"

#endif
