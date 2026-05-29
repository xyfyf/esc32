/**
 * @file mcu_port.c
 * @brief 由 Target 所含 mcu_conf.h 生成能力表（编译期，无运行时探测）
 */
#include "mcu_port.h"
#include "target.h"
#include "mcu_catalog.h"

#ifndef ESC_MCU_CONF_NAME
#define ESC_MCU_CONF_NAME "UNKNOWN"
#endif
#ifndef ESC_MCU_HAS_DUAL_ADC
#define ESC_MCU_HAS_DUAL_ADC 0
#endif
#ifndef ESC_MCU_HAS_OPAMP
#define ESC_MCU_HAS_OPAMP 0
#endif
#ifndef ESC_MCU_HAS_FDCAN
#define ESC_MCU_HAS_FDCAN 0
#endif
#ifndef ESC_MCU_SHUNT_PHASES
#define ESC_MCU_SHUNT_PHASES 3
#endif
#ifndef ESC_MCU_RECOMMENDED_PRODUCT
#define ESC_MCU_RECOMMENDED_PRODUCT ESC_PRODUCT_ESC80
#endif
#ifndef ESC_MCU_PWM_FREQ_KHZ_MAX
#define ESC_MCU_PWM_FREQ_KHZ_MAX 40.0f
#endif
#ifndef ESC_MCU_PORT_STATUS
#define ESC_MCU_PORT_STATUS "planned"
#endif

static const esc_mcu_caps_t s_caps = {
    .mcu_name = ESC_MCU_CONF_NAME,
    .mcu_id = ESC_TARGET_MCU_ID,
    .dual_adc = ESC_MCU_HAS_DUAL_ADC,
    .internal_opamp = ESC_MCU_HAS_OPAMP,
    .fdcan = ESC_MCU_HAS_FDCAN,
    .shunt_phases = ESC_MCU_SHUNT_PHASES,
    .recommended_product = ESC_MCU_RECOMMENDED_PRODUCT,
    .pwm_freq_khz_max = ESC_MCU_PWM_FREQ_KHZ_MAX,
    .port_status = ESC_MCU_PORT_STATUS,
};

const esc_mcu_caps_t *esc_mcu_capabilities(void)
{
    return &s_caps;
}
