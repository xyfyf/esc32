/**
 * Target: PC 仿真（无硬件）
 * FILE_NAME: ESC_SIM
 */
#ifndef ESC_TARGET_ESC_SIM_H
#define ESC_TARGET_ESC_SIM_H

#define ESC_TARGET_FILE_NAME       "ESC_SIM"
#define ESC_TARGET_FIRMWARE_NAME   "esc32-sim"
#define ESC_TARGET_ID              0x0001u
#define ESC_TARGET_HW_REVISION     0x0001u
#define ESC_TARGET_MCU_ID          ESC_MCU_SIM

#ifndef ESC_TARGET_PRODUCT_ID
#define ESC_TARGET_PRODUCT_ID      ESC_PRODUCT_ESC80
#endif

#define ESC_TARGET_HAL_SOURCE      "boards/mcu/sim/hal_sim.c"

#endif
