#include "pwm.h"

float pwm_us_to_norm(const esc_params_t *p, uint16_t us)
{
    if (us <= p->normal_pwm_start_us) {
        return 0.0f;
    }
    if (us >= p->normal_pwm_end_us) {
        return 1.0f;
    }
    float span = (float)(p->normal_pwm_end_us - p->normal_pwm_start_us);
    return (float)(us - p->normal_pwm_start_us) / span;
}

uint16_t pwm_norm_to_us(const esc_params_t *p, float norm)
{
    if (norm <= 0.0f) {
        return p->normal_pwm_start_us;
    }
    if (norm >= 1.0f) {
        return p->normal_pwm_end_us;
    }
    float span = (float)(p->normal_pwm_end_us - p->normal_pwm_start_us);
    return (uint16_t)(p->normal_pwm_start_us + (uint16_t)(norm * span));
}
