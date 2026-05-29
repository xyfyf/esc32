#include <math.h>
#include <string.h>
#include "motor.h"
#include "trig.h"

#define START_ALIGN_MS       80u
#define START_OPENLOOP_MS    400u
#define HANDOVER_OMEGA_RADS  80.0f
#define SENSORLESS_LOCK_MS   100u

#ifdef ESC_PLATFORM_SIM
/*
 * Simulator-only rotor dynamics state.
 *
 * On real hardware the observer recovers omega from the measured back-EMF.
 * In simulation there is no physical motor, so we integrate a 1st-order
 * mechanical model (Iq -> torque -> omega) and feed it back into the
 * firmware as if it came from the observer. Two scalars are also exposed
 * so the simulation HAL (hal_sim.c) can synthesize a believable bus
 * voltage / bus current for the ADC pretend-reads.
 */
float g_sim_ibus_a = 0.0f;
float g_sim_vbus_v = 48.0f;
static float s_sim_omega_mech = 0.0f;
#endif

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

#ifdef ESC_PLATFORM_SIM
    {
        /*
         * Simulator rotor model — runs every fast loop tick (including the
         * MOTOR_MODE_STOP path, so the motor decelerates back to zero when
         * disarmed). Behaviour: 1st-order tracking toward speed_ref_rpm
         * with ~150 ms time constant. iq / ibus / vbus are derived from
         * the resulting omega so the GUI shows believable curves.
         *
         * Constants are tuned in motor-time (dt ≈ 50 µs per call), giving
         * roughly 0.5 s wallclock to reach a new throttle target.
         */
        float w_target = (m->mode == MOTOR_MODE_STOP)
                             ? 0.0f
                             : (m->speed_ref_rpm * (2.0f * ESC_PI / 60.0f));
        float w_max = ((float)params->motor_max_rpm * 1.05f) *
                      (2.0f * ESC_PI / 60.0f);
        if (w_target > w_max) {
            w_target = w_max;
        }
        const float tau_motor = 0.0015f;  /* fast tracking in motor-time */
        float gain = dt / tau_motor;
        if (gain > 1.0f) {
            gain = 1.0f;
        }
        s_sim_omega_mech += (w_target - s_sim_omega_mech) * gain;
        if (s_sim_omega_mech < 0.0f) {
            s_sim_omega_mech = 0.0f;
        }
        float w_mech = s_sim_omega_mech;

        float pp = (float)params->motor_pole_pairs;
        if (pp < 1.0f) {
            pp = 1.0f;
        }
        m->rpm_meas       = w_mech * 60.0f / (2.0f * ESC_PI);
        m->observer.omega = w_mech * pp;
        m->observer.theta = fmodf(m->observer.theta + m->observer.omega * dt,
                                  2.0f * ESC_PI);

        /*
         * Bus current ≈ propeller-drag power / Vbus + a small fixed
         * controller loss. Coefficients are tuned so an ESC-80 preset
         * draws ~25 A at full throttle (≈ 1.2 kW mechanical) with a
         * ~1.5 V battery sag, matching typical bench-test numbers.
         */
        float p_mech = 3.0e-3f * w_mech * w_mech;       /* W */
        float p_loss = 1.5f;
        float vbus   = 48.0f - p_mech * 1.0e-3f;
        if (vbus < 30.0f) {
            vbus = 30.0f;
        }
        g_sim_vbus_v = vbus;
        g_sim_ibus_a = (p_mech + p_loss) / vbus;
        if (g_sim_ibus_a < 0.0f) {
            g_sim_ibus_a = 0.0f;
        }
    }
#endif

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

#ifndef ESC_PLATFORM_SIM
    /*
     * On real hardware the RPM estimate comes from the back-EMF observer.
     * In the simulator it is already produced by the rotor model at the
     * top of this function, so keep that value and skip the override.
     */
    float pp = (float)params->motor_pole_pairs;
    if (pp < 1.0f) {
        pp = 1.0f;
    }
    m->rpm_meas = m->observer.omega * 60.0f / (2.0f * ESC_PI * pp);
#endif
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
