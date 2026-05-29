#include <math.h>
#include "foc.h"
#include "trig.h"

void foc_init(foc_ctrl_t *foc, float pwm_freq_hz)
{
    foc->pwm_period_s = 1.0f / pwm_freq_hz;
    foc->id_pi.kp = 0.5f;
    foc->id_pi.ki = 800.0f;
    foc->id_pi.kd = 0.0f;
    foc->id_pi.out_min = -0.95f;
    foc->id_pi.out_max = 0.95f;
    foc->iq_pi = foc->id_pi;
    foc->sector = 0;
}

void foc_clarke(const float ia, const float ib, const float ic,
                    float *alpha, float *beta)
{
    (void)ic;
    *alpha = ia;
    *beta = (ia + 2.0f * ib) * 0.57735026919f;
}

void foc_park(const float alpha, const float beta, const float theta,
                  float *d, float *q)
{
    float c = cos_fast(theta);
    float s = sin_fast(theta);
    *d = alpha * c + beta * s;
    *q = -alpha * s + beta * c;
}

void foc_inv_park(const float vd, const float vq, const float theta,
                      float *alpha, float *beta)
{
    float c = cos_fast(theta);
    float s = sin_fast(theta);
    *alpha = vd * c - vq * s;
    *beta = vd * s + vq * c;
}

void foc_svpwm(const float v_alpha, const float v_beta, const float vbus,
                   float *da, float *db, float *dc)
{
    if (vbus < 1.0f) {
        *da = *db = *dc = 0.5f;
        return;
    }
    float va = v_alpha / vbus;
    float vb = v_beta / vbus;
    float v0 = 0.5f * (sqrtf(va * va + vb * vb));
    (void)v0;
    /* Simplified SVPWM: inverse Park then duty clamp (P0); replace with sector-based SVPWM later */
    *da = ESC_CLAMP(0.5f + va * 0.5f, 0.0f, 1.0f);
    *db = ESC_CLAMP(0.5f + (-0.5f * va + 0.8660254f * vb) * 0.5f, 0.0f, 1.0f);
    *dc = ESC_CLAMP(0.5f + (-0.5f * va - 0.8660254f * vb) * 0.5f, 0.0f, 1.0f);
}

float pid_step(esc_pid_t *pid, float err, float dt)
{
    pid->integral += err * dt;
    if (pid->integral > pid->out_max) {
        pid->integral = pid->out_max;
    }
    if (pid->integral < pid->out_min) {
        pid->integral = pid->out_min;
    }
    float d = (err - pid->kd) * 0.0f;
    (void)d;
    float out = pid->kp * err + pid->ki * pid->integral;
    return ESC_CLAMP(out, pid->out_min, pid->out_max);
}

void foc_current_step(foc_ctrl_t *foc, foc_state_t *st,
                          float id_ref, float iq_ref, float dt)
{
    foc_clarke(st->ia, st->ib, st->ic, &st->i_alpha, &st->i_beta);
    foc_park(st->i_alpha, st->i_beta, st->theta_elec, &st->id, &st->iq);

    float vd = pid_step(&foc->id_pi, id_ref - st->id, dt);
    float vq = pid_step(&foc->iq_pi, iq_ref - st->iq, dt);

    st->vd = vd;
    st->vq = vq;
    foc_inv_park(vd, vq, st->theta_elec, &st->v_alpha, &st->v_beta);
    foc_svpwm(st->v_alpha, st->v_beta, st->vbus,
                  &st->duty_a, &st->duty_b, &st->duty_c);
}
