#include <math.h>
#include "observer.h"
#include "trig.h"
#include "params.h"

void observer_init(observer_t *obs, observer_type_t type, const esc_params_t *p)
{
    obs->type = type;
    obs->kp_pll = 800.0f * p->observer_coef;
    obs->ki_pll = 40000.0f * p->observer_coef;
    obs->theta = 0.0f;
    obs->omega = 0.0f;
    obs->filter_hz = p->observer_filter_freq_hz;
    obs->alpha_est = 0.0f;
    obs->beta_est = 0.0f;
    obs->lambda_smo = 0.15f;
    obs->rs = p->motor_rs_mohm * 0.001f;
    obs->ls = (p->motor_ld_uh + p->motor_lq_uh) * 0.5f * 1e-6f;
    if (obs->ls < 1e-6f) {
        obs->ls = 30e-6f;
    }
}

static void observer_pll(observer_t *obs, float v_alpha, float v_beta, float dt)
{
    float angle = atan2f(v_beta, v_alpha);
    float err = angle - obs->theta;
    while (err > ESC_PI) {
        err -= 2.0f * ESC_PI;
    }
    while (err < -ESC_PI) {
        err += 2.0f * ESC_PI;
    }
    obs->omega += obs->ki_pll * err * dt;
    if (obs->omega < 0.0f) {
        obs->omega = 0.0f;
    }
    obs->theta += obs->omega * dt + obs->kp_pll * err * dt;
    while (obs->theta >= 2.0f * ESC_PI) {
        obs->theta -= 2.0f * ESC_PI;
    }
    while (obs->theta < 0.0f) {
        obs->theta += 2.0f * ESC_PI;
    }
}

static void observer_smo(observer_t *obs, float v_alpha, float v_beta,
                         float i_alpha, float i_beta, float dt)
{
    float i_err_alpha = obs->alpha_est - i_alpha;
    float i_err_beta = obs->beta_est - i_beta;

    float z_alpha = obs->lambda_smo * (i_err_alpha > 0.0f ? 1.0f : -1.0f);
    float z_beta = obs->lambda_smo * (i_err_beta > 0.0f ? 1.0f : -1.0f);

    float d_alpha = v_alpha - obs->rs * obs->alpha_est - z_alpha;
    float d_beta = v_beta - obs->rs * obs->beta_est - z_beta;

    obs->alpha_est += (d_alpha / obs->ls) * dt;
    obs->beta_est += (d_beta / obs->ls) * dt;

  float bemf_alpha = obs->alpha_est - obs->rs * i_alpha;
  float bemf_beta = obs->beta_est - obs->rs * i_beta;

    observer_pll(obs, bemf_alpha, bemf_beta, dt);
}

void observer_update(observer_t *obs, float v_alpha, float v_beta,
                     float i_alpha, float i_beta, float vbus, float dt)
{
    (void)vbus;
    if (obs->type == OBSERVER_SMO) {
        observer_smo(obs, v_alpha, v_beta, i_alpha, i_beta, dt);
    } else {
        observer_pll(obs, v_alpha, v_beta, dt);
    }
}
