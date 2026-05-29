#include "pwm_in.h"
#include "pwm.h"

void pwm_in_init(pwm_input_t *in)
{
    in->pulse_us = 0;
    in->last_valid_us = 0;
    in->lost_ms = 0;
    in->valid = false;
    in->armed_seen = false;
    in->link_established = false;
}

void pwm_in_update(pwm_input_t *in, const esc_params_t *p, uint16_t raw_us,
                   uint32_t dt_ms)
{
    const uint16_t min_valid = 800;
    const uint16_t max_valid = 2200;

    if (raw_us >= min_valid && raw_us <= max_valid) {
        in->pulse_us = raw_us;
        in->last_valid_us = raw_us;
        in->lost_ms = 0;
        in->valid = true;
        if (!in->link_established) {
            in->link_established = true;
        }
        if (raw_us > p->normal_pwm_start_us + 50) {
            in->armed_seen = true;
        }
    } else {
        in->lost_ms += dt_ms;
        if (in->lost_ms >= p->ppm_lost_time_ms) {
            in->valid = false;
            in->pulse_us = p->normal_pwm_start_us;
        }
    }
}

float pwm_in_norm(const pwm_input_t *in, const esc_params_t *p)
{
    if (!in->valid) {
        return 0.0f;
    }
    return pwm_us_to_norm(p, in->pulse_us);
}

bool pwm_in_is_lost(const pwm_input_t *in, const esc_params_t *p)
{
    /* 仅「曾收到有效 PWM」后才视为丢失（上电无信号不报警） */
    return in->link_established && !in->valid &&
           in->lost_ms >= p->ppm_lost_time_ms;
}
