/**
 * Target: ESC-80 + STM32G474RET6 首版硬件
 * FILE_NAME: ESC80_STM32G474_V1
 */
#ifndef ESC_TARGET_ESC80_G474_H
#define ESC_TARGET_ESC80_G474_H

#define ESC_TARGET_FILE_NAME       "ESC80_STM32G474_V1"
#define ESC_TARGET_FIRMWARE_NAME   "ESC-80-G474"
#define ESC_TARGET_ID              0x8081u   /* 0x80 产品域 + 0x81 首版 */
#define ESC_TARGET_HW_REVISION     0x0001u
#define ESC_TARGET_MCU_ID          ESC_MCU_STM32G474
#define ESC_TARGET_PRODUCT_ID      ESC_PRODUCT_ESC80

#define ESC_TARGET_PINMAP_HEADER   "boards/mcu/stm32g474/pinmap.h"
#define ESC_TARGET_HAL_SOURCE      "boards/mcu/stm32g474/hal_stub.c"
#define ESC_TARGET_MCU_CONF_HEADER "boards/mcu/stm32g474/mcu_conf.h"

#endif
