/**
 * @file mcu_conf_common.h
 * @brief STM32G4 family common capabilities (G431 / G474 etc.; VESC-style family reuse)
 */
#ifndef ESC_STM32G4_CONF_COMMON_H
#define ESC_STM32G4_CONF_COMMON_H

#define ESC_MCU_HAS_DUAL_ADC    1
#define ESC_MCU_HAS_OPAMP       1
#define ESC_MCU_HAS_FDCAN       1
#define ESC_MCU_SHUNT_PHASES    3
#define ESC_MCU_PWM_FREQ_KHZ_MAX 40.0f

/* G4 shared: injected sampling, CORDIC, comparators — reused in hal_g4_*.c on real hardware */

#endif
