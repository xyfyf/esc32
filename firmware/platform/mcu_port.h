/**
 * @file mcu_port.h
 * @brief MCU 能力描述（对标 VESC 的 hw_*.h 硬件能力宏 + 编译期选型）
 *
 * 核心算法（FOC/保护/协议）只依赖 hal.h；
 * 本模块暴露「双 ADC / 运放 / 推荐产品档」等，供参数默认值与上位机展示。
 */
#ifndef ESC_MCU_PORT_H
#define ESC_MCU_PORT_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    const char *mcu_name;
    uint8_t     mcu_id;
    bool        dual_adc;
    bool        internal_opamp;
    bool        fdcan;
    uint8_t     shunt_phases;       /* 2 或 3 */
    uint16_t    recommended_product;
    float       pwm_freq_khz_max;
    const char *port_status;        /* "ready" | "stub" | "planned" */
} esc_mcu_caps_t;

const esc_mcu_caps_t *esc_mcu_capabilities(void);

#endif
