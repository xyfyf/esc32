#ifndef ESC_FOC_H
#define ESC_FOC_H

#include <stdint.h>
#include "types.h"

typedef struct {
    float ia, ib, ic;
    float i_alpha, i_beta;
    float id, iq;
    float vd, vq;
    float v_alpha, v_beta;
    float theta_elec;
    float omega_elec;
    float vbus;
    float duty_a, duty_b, duty_c;
} foc_state_t;

typedef struct {
    esc_pid_t id_pi;
    esc_pid_t iq_pi;
    float pwm_period_s;
    uint8_t sector;
} foc_ctrl_t;

void foc_init(foc_ctrl_t *foc, float pwm_freq_hz);
void foc_clarke(const float ia, const float ib, const float ic,
                    float *alpha, float *beta);
void foc_park(const float alpha, const float beta, const float theta,
                  float *d, float *q);
void foc_inv_park(const float vd, const float vq, const float theta,
                      float *alpha, float *beta);
void foc_svpwm(const float v_alpha, const float v_beta, const float vbus,
                   float *da, float *db, float *dc);
float pid_step(esc_pid_t *pid, float err, float dt);
void foc_current_step(foc_ctrl_t *foc, foc_state_t *st,
                          float id_ref, float iq_ref, float dt);

#endif
