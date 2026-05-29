#ifndef ESC_MOTOR_H
#define ESC_MOTOR_H

#include <stdint.h>
#include "foc.h"
#include "observer.h"
#include "state.h"
#include "params.h"
#include "types.h"

typedef enum {
    MOTOR_MODE_STOP = 0,
    MOTOR_MODE_OPENLOOP,
    MOTOR_MODE_SENSORLESS,
} motor_mode_t;

typedef enum {
    MOTOR_START_IDLE = 0,
    MOTOR_START_ALIGN,
    MOTOR_START_OPENLOOP,
    MOTOR_START_HANDOVER,
    MOTOR_START_DONE,
} motor_start_phase_t;

typedef struct {
    motor_mode_t mode;
    motor_start_phase_t start_phase;
    foc_ctrl_t foc;
    foc_state_t foc_st;
    observer_t observer;
    esc_pid_t speed_pi;
    float iq_ref_a;
    float speed_ref_rpm;
    float openloop_omega;
    float rpm_meas;
    uint32_t loop_count;
    uint32_t start_timer_ms;
    uint8_t sensorless_ok;
} esc_motor_t;

void motor_init(esc_motor_t *m, const esc_params_t *params);
void motor_set_throttle(esc_motor_t *m, const esc_params_t *params,
                        float norm, esc_state_t state);
void motor_fast_loop(esc_motor_t *m, const esc_params_t *params, float dt);
void motor_slow_loop(esc_motor_t *m, const esc_params_t *params, float dt);
float motor_get_rpm(const esc_motor_t *m);

#endif
