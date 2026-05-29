/**
 * Target: ESC-60 + STM32G431（规划）
 */
#ifndef ESC_TARGET_ESC60_G431_H
#define ESC_TARGET_ESC60_G431_H

#define ESC_TARGET_FILE_NAME       "ESC60_STM32G431_V1"
#define ESC_TARGET_FIRMWARE_NAME   "ESC-60-G431"
#define ESC_TARGET_ID              0x6081u
#define ESC_TARGET_HW_REVISION     0x0001u
#define ESC_TARGET_MCU_ID          ESC_MCU_STM32G431
#define ESC_TARGET_PRODUCT_ID      ESC_PRODUCT_ESC60

#define ESC_TARGET_PINMAP_HEADER   "boards/mcu/stm32g431/pinmap.h"
#define ESC_TARGET_HAL_SOURCE      "boards/mcu/stm32g431/hal_stub.c"
#define ESC_TARGET_MCU_CONF_HEADER "boards/mcu/stm32g431/mcu_conf.h"

#endif
