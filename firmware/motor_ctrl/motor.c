#include <math.h>
#include <string.h>
#include "motor.h"
#include "trig.h"

#define START_ALIGN_MS       80u
#define START_OPENLOOP_MS    400u
#define HANDOVER_OMEGA_RADS  80.0f
#define SENSORLESS_LOCK_MS   100u

static float clampf(float x, float lo, float hi)
{
    if (x < lo) {
        return lo;
    }
    if (x > hi) {
        return hi;
    }
    return x;
}

void motor_init(esc_motor_t *m, const esc_params_t *params)
{
    memset(m, 0, sizeof(*m));
    m->mode = MOTOR_MODE_STOP;
    m->start_phase = MOTOR_START_IDLE;
    foc_init(&m->foc, params->carrier_freq_khz * 1000.0f);
    m->foc_st.vbus = 48.0f;
    observer_init(&m->observer, (observer_type_t)params->observer_type, params);
    m->speed_pi.kp = params->s_speed_loop_kp;
    m->speed_pi.ki = params->s_speed_loop_ki;
    m->speed_pi.kd = 0.0f;
    m->speed_pi.integral = 0.0f;
    m->speed_pi.out_min = -params->motor_max_current_a;
    m->speed_pi.out_max = params->motor_max_current_a;
    m->openloop_omega = 30.0f;
}

void motor_set_throttle(esc_motor_t *m, const esc_params_t *params,
                        float norm, esc_state_t state)
{
    norm = clampf(norm, 0.0f, 1.0f);
    if (state != ESC_STATE_RUNNING && state != ESC_STATE_ARMED) {
        m->iq_ref_a = 0.0f;
        m->speed_ref_rpm = 0.0f;
        m->mode = MOTOR_MODE_STOP;
        m->start_phase = MOTOR_START_IDLE;
        m->speed_pi.integral = 0.0f;
        return;
    }

    float rpm_max = (float)params->motor_max_rpm;
    if (rpm_max < 500.0f) {
        rpm_max = 6000.0f;
    }
    m->speed_ref_rpm = norm * rpm_max;

    if (state == ESC_STATE_ARMED && norm < 0.02f) {
        m->mode = MOTOR_MODE_STOP;
        m->start_phase = MOTOR_START_IDLE;
        m->iq_ref_a = 0.0f;
        return;
    }

    if (m->start_phase == MOTOR_START_DONE &&
        m->observer.omega > HANDOVER_OMEGA_RADS) {
        m->mode = MOTOR_MODE_SENSORLESS;
    } else if (m->start_phase != MOTOR_START_IDLE) {
        m->mode = MOTOR_MODE_OPENLOOP;
    } else if (norm > 0.05f) {
        m->start_phase = MOTOR_START_ALIGN;
        m->start_timer_ms = 0;
        m->mode = MOTOR_MODE_OPENLOOP;
    }
}

static void motor_run_speed_pi(esc_motor_t *m, const esc_params_t *params, float dt)
{
    float err = m->speed_ref_rpm - m->rpm_meas;
    m->iq_ref_a = pid_step(&m->speed_pi, err, dt);
    float imax = params->motor_max_current_a;
    if (params->ibus_max_current_a > 0.0f) {
        imax = fminf(imax, params->ibus_max_current_a);
    }
    m->iq_ref_a = clampf(m->iq_ref_a, -imax, imax);

    if (params->field_weakening_enable && m->rpm_meas > params->motor_max_rpm * 0.85f) {
        /* Field weakening: allow a small Iq headroom at high speed */
        m->iq_ref_a *= 1.05f;
    }
}

static void motor_update_start(esc_motor_t *m, float dt)
{
    m->start_timer_ms += (uint32_t)(dt * 1000.0f);

    switch (m->start_phase) {
    case MOTOR_START_ALIGN:
        m->foc_st.theta_elec = 0.0f;
        m->iq_ref_a = 2.0f;
        if (m->start_timer_ms > START_ALIGN_MS) {
            m->start_phase = MOTOR_START_OPENLOOP;
            m->start_timer_ms = 0;
        }
        break;
    case MOTOR_START_OPENLOOP:
        m->foc_st.theta_elec += m->openloop_omega * dt;
        if (m->foc_st.theta_elec > 2.0f * ESC_PI) {
            m->foc_st.theta_elec -= 2.0f * ESC_PI;
        }
        m->iq_ref_a = 5.0f + m->speed_ref_rpm * 0.002f;
        if (m->observer.omega > HANDOVER_OMEGA_RADS) {
            m->sensorless_ok++;
        } else {
            m->sensorless_ok = 0;
        }
        if (m->sensorless_ok > 20 || m->start_timer_ms > START_OPENLOOP_MS) {
            m->start_phase = MOTOR_START_HANDOVER;
            m->start_timer_ms = 0;
        }
        break;
    case MOTOR_START_HANDOVER:
        m->foc_st.theta_elec = m->observer.theta;
        if (m->start_timer_ms > SENSORLESS_LOCK_MS) {
            m->start_phase = MOTOR_START_DONE;
        }
        break;
    default:
        break;
    }
}

void motor_fast_loop(esc_motor_t *m, const esc_params_t *params, float dt)
{
    m->loop_count++;

    if (m->mode == MOTOR_MODE_STOP) {
        m->foc_st.duty_a = m->foc_st.duty_b = m->foc_st.duty_c = 0.5f;
        return;
    }

    if (m->mode == MOTOR_MODE_OPENLOOP) {
        motor_update_start(m, dt);
    } else {
        m->foc_st.theta_elec = m->observer.theta;
        m->foc_st.omega_elec = m->observer.omega;
    }

    /* Simulated motor model */
    float iq_act = m->iq_ref_a * 0.85f;
    m->foc_st.ia = iq_act * sinf(m->foc_st.theta_elec);
    m->foc_st.ib = iq_act * sinf(m->foc_st.theta_elec - 2.0943951f);
    m->foc_st.ic = -m->foc_st.ia - m->foc_st.ib;

    foc_current_step(&m->foc, &m->foc_st, 0.0f, m->iq_ref_a, dt);

    observer_update(&m->observer, m->foc_st.v_alpha, m->foc_st.v_beta,
                    m->foc_st.i_alpha, m->foc_st.i_beta, m->foc_st.vbus, dt);

    float pp = (float)params->motor_pole_pairs;
    if (pp < 1.0f) {
        pp = 1.0f;
    }
    m->rpm_meas = m->observer.omega * 60.0f / (2.0f * ESC_PI * pp);
}

void motor_slow_loop(esc_motor_t *m, const esc_params_t *params, float dt)
{
    if (m->mode == MOTOR_MODE_SENSORLESS && m->start_phase == MOTOR_START_DONE) {
        motor_run_speed_pi(m, params, dt);
    }
}

float motor_get_rpm(const esc_motor_t *m)
{
    return m->rpm_meas;
}
