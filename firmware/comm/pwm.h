#ifndef ESC_PWM_H
#define ESC_PWM_H

#include <stdint.h>
#include "params.h"

float pwm_us_to_norm(const esc_params_t *p, uint16_t us);
uint16_t pwm_norm_to_us(const esc_params_t *p, float norm);

#endif
