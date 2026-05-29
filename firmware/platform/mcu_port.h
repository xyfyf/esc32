/**
 * @file mcu_port.h
 * @brief MCU capability descriptor (VESC-style hw_*.h macros + build-time selection)
 *
 * Core algorithms (FOC/protection/protocol) depend only on hal.h;
 * This module exposes dual ADC / op-amp / recommended product tier for defaults and host UI.
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
    uint8_t     shunt_phases;       /* 2 or 3 */
    uint16_t    recommended_product;
    float       pwm_freq_khz_max;
    const char *port_status;        /* "ready" | "stub" | "planned" */
} esc_mcu_caps_t;

const esc_mcu_caps_t *esc_mcu_capabilities(void);

#endif
