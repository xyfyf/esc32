/**
 * Target: ESC-120 + STM32H743 (planned)
 */
#ifndef ESC_TARGET_ESC120_H743_H
#define ESC_TARGET_ESC120_H743_H

#define ESC_TARGET_FILE_NAME       "ESC120_STM32H743_V1"
#define ESC_TARGET_FIRMWARE_NAME   "ESC-120-H743"
#define ESC_TARGET_ID              0x1201u
#define ESC_TARGET_HW_REVISION     0x0001u
#define ESC_TARGET_MCU_ID          ESC_MCU_STM32H743
#define ESC_TARGET_PRODUCT_ID      ESC_PRODUCT_ESC120

#define ESC_TARGET_PINMAP_HEADER   "boards/mcu/stm32h743/pinmap.h"
#define ESC_TARGET_HAL_SOURCE      "boards/mcu/stm32h743/hal_stub.c"

#endif
