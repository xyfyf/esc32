/**
 * @file app.c
 */
#include "app.h"
#include "board.h"
#include "hal.h"
#include "scheduler.h"
#include "motor.h"
#include "protect.h"
#include "pwm_in.h"
#include "pwm.h"
#include "comm.h"
#include "dronecan.h"
#include "params.h"
#include "fault_log.h"
#include "motor_beep.h"
#include <string.h>

static esc_runtime_t s_rt;
static esc_motor_t s_motor;
static esc_sensors_t s_sns;
static pwm_input_t s_pwm_in;

static esc_task_t s_task_fast;
static esc_task_t s_task_slow;
static esc_task_t s_task_comm;
static esc_task_t s_task_can;

static float app_select_throttle(void)
{
    if (dronecan_armed()) {
        return dronecan_get_throttle_norm();
    }
    if (s_pwm_in.valid) {
        return pwm_in_norm(&s_pwm_in, &g_params);
    }
    return s_rt.throttle_norm;
}

static void task_fast(void *ctx)
{
    (void)ctx;
    float dt = 1.0f / (g_params.carrier_freq_khz * 1000.0f);
    hal_adc_read(&s_sns.vbus_v, &s_sns.ibus_a, &s_sns.ia, &s_sns.ib, &s_sns.ic,
                 &s_sns.temp_mos_c);

    if (motor_beep_enabled(&g_params) && motor_beep_active()) {
        float da, db, dc;
        motor_beep_fast_loop(&g_params, dt, &da, &db, &dc);
        hal_pwm_set(da, db, dc);
        return;
    }

    s_motor.foc_st.vbus = s_sns.vbus_v;
    motor_fast_loop(&s_motor, &g_params, dt);
    hal_pwm_set(s_motor.foc_st.duty_a, s_motor.foc_st.duty_b, s_motor.foc_st.duty_c);
    dronecan_on_fast_loop();
}

static void task_slow(void *ctx)
{
    (void)ctx;
    pwm_in_update(&s_pwm_in, &g_params, hal_pwm_input_us(), 1);

    float norm = app_select_throttle();
    s_rt.throttle_norm = norm;

    if (s_pwm_in.valid) {
        s_rt.throttle_us = s_pwm_in.pulse_us;
        s_rt.input_source = ESC_INPUT_PWM;
    } else if (dronecan_armed()) {
        s_rt.input_source = ESC_INPUT_CAN;
    }

    bool pwm_lost = pwm_in_is_lost(&s_pwm_in, &g_params);
    bool can_lost = dronecan_link_established() && !dronecan_armed();
    if ((pwm_lost && !dronecan_armed()) || can_lost) {
        s_sns.throttle_lost_ms = g_params.ppm_lost_time_ms;
    }

    if (motor_beep_enabled(&g_params)) {
        motor_beep_tick_1ms(&g_params, 1);
        if (motor_beep_boot_done() && s_pwm_in.link_established) {
            motor_beep_on_link_established();
        }
        if (!dronecan_armed()) {
            motor_beep_on_signal_lost(pwm_lost);
        } else {
            motor_beep_on_signal_lost(false);
        }
    }

    if (s_rt.state == ESC_STATE_ARMED && norm > 0.05f && !motor_beep_active()) {
        s_rt.state = ESC_STATE_RUNNING;
    }
    if (s_rt.state == ESC_STATE_RUNNING && norm < 0.02f) {
        s_rt.state = ESC_STATE_ARMED;
    }

    if (motor_beep_active()) {
        motor_set_throttle(&s_motor, &g_params, 0.0f, ESC_STATE_DISARMED);
    } else {
        motor_set_throttle(&s_motor, &g_params, norm, s_rt.state);
        motor_slow_loop(&s_motor, &g_params, 0.001f);
    }

    fault_code_t f = protect_check(&g_params, &s_sns, &s_rt);
    if (f != ESC_FAULT_NONE) {
        fault_log_push(f, s_rt.uptime_ms, (uint16_t)(s_sns.vbus_v * 1000.0f),
                       (int16_t)(s_sns.ibus_a * 1000.0f),
                       (int16_t)(s_sns.temp_mos_c * 10.0f), (int32_t)motor_get_rpm(&s_motor),
                       s_rt.throttle_us, (uint8_t)s_rt.state);
        state_set_fault(&s_rt, f);
        motor_set_throttle(&s_motor, &g_params, 0.0f, ESC_STATE_FAULT);
    }
}

static void task_comm(void *ctx)
{
    (void)ctx;
    comm_poll();
}

static void task_can(void *ctx)
{
    (void)ctx;
    dronecan_poll();
}

void app_init(void)
{
    hal_init();
    state_init(&s_rt);
    fault_log_init();
    pwm_in_init(&s_pwm_in);
    motor_beep_init();
    motor_beep_reset_session();

    if (!params_load_flash(&g_params)) {
        board_load_defaults(&g_params);
    } else {
        board_apply_limits(&g_params);
    }

    motor_init(&s_motor, &g_params);
    if (motor_beep_enabled(&g_params)) {
        motor_beep_request(MOTOR_BEEP_MELODY_FULL);
    }
    comm_init();
    dronecan_init(g_params.node_id);
    scheduler_init();

    uint32_t fast_us = (uint32_t)(1000000.0f / (g_params.carrier_freq_khz * 1000.0f));

    s_task_fast.name = "foc_fast";
    s_task_fast.fn = task_fast;
    s_task_fast.period_us = fast_us;
    s_task_fast.enabled = 1;

    s_task_slow.name = "slow_1k";
    s_task_slow.fn = task_slow;
    s_task_slow.period_us = 1000;
    s_task_slow.enabled = 1;

    s_task_comm.name = "comm";
    s_task_comm.fn = task_comm;
    s_task_comm.period_us = 2000;
    s_task_comm.enabled = 1;

    s_task_can.name = "dronecan";
    s_task_can.fn = task_can;
    s_task_can.period_us = 5000;
    s_task_can.enabled = 1;

    scheduler_register(&s_task_fast);
    scheduler_register(&s_task_slow);
    scheduler_register(&s_task_comm);
    scheduler_register(&s_task_can);
}

void app_run_once(void)
{
    s_rt.uptime_ms = hal_millis();
#ifdef ESC_PLATFORM_SIM
    /* Simulation: poll UDP each loop to avoid debug protocol timeouts from scheduler granularity */
    comm_poll();
#endif
    scheduler_run(hal_time_us());
}

uint16_t app_board_id(void)
{
    return esc_product_profile()->product_id;
}

void app_arm(void)
{
    if (s_rt.state == ESC_STATE_FAULT) {
        return;
    }
    s_rt.state = ESC_STATE_ARMED;
}

void app_disarm(void)
{
    s_rt.state = ESC_STATE_DISARMED;
    motor_set_throttle(&s_motor, &g_params, 0.0f, s_rt.state);
}

void app_set_throttle_us(uint16_t us)
{
    s_rt.throttle_us = us;
    s_rt.input_source = ESC_INPUT_DEBUG;
    float norm = pwm_us_to_norm(&g_params, us);
    s_rt.throttle_norm = norm;
    hal_pwm_input_simulate(us);
    if (s_rt.state == ESC_STATE_ARMED && norm > 0.05f) {
        s_rt.state = ESC_STATE_RUNNING;
    }
    motor_set_throttle(&s_motor, &g_params, norm, s_rt.state);
}

void app_fill_telem(esc_telem_rsp_t *t)
{
    memset(t, 0, sizeof(*t));
    t->uptime_ms = s_rt.uptime_ms;
    t->state = (uint8_t)s_rt.state;
    t->fault_code = (uint8_t)s_rt.latched_fault;
    t->vbus_mv = (uint16_t)(s_sns.vbus_v * 1000.0f);
    t->ibus_ma = (int16_t)(s_sns.ibus_a * 1000.0f);
    t->temp_mos_c10 = (int16_t)(s_sns.temp_mos_c * 10.0f);
    t->rpm = (int32_t)motor_get_rpm(&s_motor);
    t->id_ma = (int16_t)(s_motor.foc_st.id * 1000.0f);
    t->iq_ma = (int16_t)(s_motor.foc_st.iq * 1000.0f);
    t->throttle_us = s_rt.throttle_us;
    t->input_source = (uint8_t)s_rt.input_source;
}

esc_runtime_t *app_runtime(void)
{
    return &s_rt;
}
