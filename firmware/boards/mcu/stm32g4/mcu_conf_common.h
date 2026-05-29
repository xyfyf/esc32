/**
 * @file mcu_conf_common.h
 * @brief STM32G4 系共用能力（G431 / G474 等同系，对标 VESC 的 STM32 分族复用）
 */
#ifndef ESC_STM32G4_CONF_COMMON_H
#define ESC_STM32G4_CONF_COMMON_H

#define ESC_MCU_HAS_DUAL_ADC    1
#define ESC_MCU_HAS_OPAMP       1
#define ESC_MCU_HAS_FDCAN       1
#define ESC_MCU_SHUNT_PHASES    3
#define ESC_MCU_PWM_FREQ_KHZ_MAX 40.0f

/* G4 共享：注入采样、CORDIC、比较器 — 真机 HAL 在 hal_g4_*.c 复用 */

#endif
