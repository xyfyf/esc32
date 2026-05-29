/**
 * @file mcu_conf.h
 * @brief AT32F415 — mid/small ESC / cost-effective alternative to G474 (richer than F421)
 */
#ifndef ESC_AT32F415_MCU_CONF_H
#define ESC_AT32F415_MCU_CONF_H

#define ESC_MCU_CONF_NAME           "AT32F415"
#define ESC_MCU_HAS_DUAL_ADC        1
#define ESC_MCU_HAS_OPAMP           0   /* typically external INA240 */
#define ESC_MCU_HAS_FDCAN           1
#define ESC_MCU_SHUNT_PHASES        3
#define ESC_MCU_RECOMMENDED_PRODUCT ESC_PRODUCT_ESC80
#define ESC_MCU_PWM_FREQ_KHZ_MAX    40.0f
#define ESC_MCU_PORT_STATUS         "stub"

#endif
