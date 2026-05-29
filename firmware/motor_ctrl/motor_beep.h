/**
 * @file motor_beep.h
 * @brief Motor winding audio: boot melody / link melody / signal-loss beeps
 */
#ifndef ESC_MOTOR_BEEP_H
#define ESC_MOTOR_BEEP_H

#include <stdint.h>
#include <stdbool.h>
#include "params.h"

typedef enum {
    MOTOR_BEEP_NONE = 0,
    MOTOR_BEEP_MELODY_FULL,
    MOTOR_BEEP_ALARM,
} motor_beep_request_t;

void motor_beep_init(void);
void motor_beep_reset_session(void);

bool motor_beep_enabled(const esc_params_t *p);
bool motor_beep_active(void);
bool motor_beep_boot_done(void);

void motor_beep_request(motor_beep_request_t req);
void motor_beep_on_link_established(void);
void motor_beep_on_signal_lost(bool lost);

void motor_beep_tick_1ms(const esc_params_t *p, uint32_t dt_ms);
void motor_beep_fast_loop(const esc_params_t *p, float dt,
                          float *duty_a, float *duty_b, float *duty_c);

#endif
