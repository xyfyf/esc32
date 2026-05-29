/**
 * @file mcu_conf.h
 * @brief AT32F415 — 中小 ESC / G474 成本替代（资源优于 F421）
 */
#ifndef ESC_AT32F415_MCU_CONF_H
#define ESC_AT32F415_MCU_CONF_H

#define ESC_MCU_CONF_NAME           "AT32F415"
#define ESC_MCU_HAS_DUAL_ADC        1
#define ESC_MCU_HAS_OPAMP           0   /* 多为外置 INA240 */
#define ESC_MCU_HAS_FDCAN           1
#define ESC_MCU_SHUNT_PHASES        3
#define ESC_MCU_RECOMMENDED_PRODUCT ESC_PRODUCT_ESC80
#define ESC_MCU_PWM_FREQ_KHZ_MAX    40.0f
#define ESC_MCU_PORT_STATUS         "stub"

#endif
