#ifndef ESC_PWM_IN_H
#define ESC_PWM_IN_H

#include <stdint.h>
#include <stdbool.h>
#include "params.h"

typedef struct {
    uint16_t pulse_us;
    uint16_t last_valid_us;
    uint32_t lost_ms;
    bool     valid;
    bool     armed_seen;
    bool     link_established;
} pwm_input_t;

void pwm_in_init(pwm_input_t *in);
void pwm_in_update(pwm_input_t *in, const esc_params_t *p, uint16_t raw_us, uint32_t dt_ms);
float pwm_in_norm(const pwm_input_t *in, const esc_params_t *p);
bool pwm_in_is_lost(const pwm_input_t *in, const esc_params_t *p);

#endif
