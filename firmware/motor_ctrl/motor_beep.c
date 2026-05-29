/**
 * @file motor_beep.c
 * @brief 固定电角度 + 音频频率调制三相占空比（无 FOC 运行）
 */
#include "motor_beep.h"
#include <math.h>
#include <string.h>

#ifdef ESC_PLATFORM_SIM
#include <stdio.h>
#endif

#ifndef ESC_FEATURE_MOTOR_BEEP
#define ESC_FEATURE_MOTOR_BEEP 1
#endif

typedef struct {
    float    hz;
    uint16_t duration_ms;
} beep_note_t;

/* 完整上电/联机旋律（三音上行，常见电调风格） */
static const beep_note_t s_melody_full[] = {
    { 523.25f, 120 }, /* C5 */
    { 659.25f, 120 }, /* E5 */
    { 783.99f, 180 }, /* G5 */
};
#define MELODY_FULL_COUNT (sizeof(s_melody_full) / sizeof(s_melody_full[0]))

/* 丢失报警：短-停-短 */
static const beep_note_t s_alarm_pair[] = {
    { 880.0f, 80 },
    { 0.0f,   60 },
    { 880.0f, 80 },
};
#define ALARM_PAIR_COUNT (sizeof(s_alarm_pair) / sizeof(s_alarm_pair[0]))
#define ALARM_CYCLE_MS   1200u

typedef struct {
    motor_beep_request_t request;
    const beep_note_t   *notes;
    uint8_t              note_count;
    uint8_t              note_idx;
    uint32_t             note_elapsed_ms;
    float                phase;
    float                amplitude;
    bool                 boot_done;
    bool                 link_melody_done;
    bool                 alarm_latched;
    uint32_t             alarm_cycle_ms;
} motor_beep_state_t;

static motor_beep_state_t s_beep;

static float volume_to_amp(const esc_params_t *p)
{
    float v = (float)p->pndef_motor_sound_volume;
    if (v <= 0.0f) {
        v = 8.0f;
    }
    if (v > 15.0f) {
        v = 15.0f;
    }
    return 0.04f + v * 0.008f;
}

static void start_sequence(const beep_note_t *notes, uint8_t count,
                           motor_beep_request_t kind)
{
    s_beep.request = kind;
    s_beep.notes = notes;
    s_beep.note_count = count;
    s_beep.note_idx = 0;
    s_beep.note_elapsed_ms = 0;
    s_beep.phase = 0.0f;
}

void motor_beep_init(void)
{
    memset(&s_beep, 0, sizeof(s_beep));
}

void motor_beep_reset_session(void)
{
    s_beep.boot_done = false;
    s_beep.link_melody_done = false;
    s_beep.alarm_latched = false;
}

bool motor_beep_enabled(const esc_params_t *p)
{
#if !ESC_FEATURE_MOTOR_BEEP
    (void)p;
    return false;
#else
    return p->motor_sound_enable != 0;
#endif
}

bool motor_beep_active(void)
{
    return s_beep.request != MOTOR_BEEP_NONE;
}

bool motor_beep_boot_done(void)
{
    return s_beep.boot_done;
}

void motor_beep_request(motor_beep_request_t req)
{
    if (req == MOTOR_BEEP_NONE) {
        s_beep.request = MOTOR_BEEP_NONE;
        return;
    }
    if (req == MOTOR_BEEP_MELODY_FULL) {
        start_sequence(s_melody_full, (uint8_t)MELODY_FULL_COUNT,
                       MOTOR_BEEP_MELODY_FULL);
#ifdef ESC_PLATFORM_SIM
        printf("[beep] melody full\n");
        fflush(stdout);
#endif
    }
}

void motor_beep_on_link_established(void)
{
    if (!s_beep.boot_done || s_beep.link_melody_done) {
        return;
    }
    if (motor_beep_active()) {
        return;
    }
    s_beep.link_melody_done = true;
    motor_beep_request(MOTOR_BEEP_MELODY_FULL);
#ifdef ESC_PLATFORM_SIM
    printf("[beep] link melody\n");
    fflush(stdout);
#endif
}

void motor_beep_on_signal_lost(bool lost)
{
    if (lost) {
        if (!s_beep.alarm_latched) {
            s_beep.alarm_latched = true;
        }
        if (!motor_beep_active()) {
            s_beep.alarm_cycle_ms = 0;
            start_sequence(s_alarm_pair, (uint8_t)ALARM_PAIR_COUNT,
                           MOTOR_BEEP_ALARM);
#ifdef ESC_PLATFORM_SIM
            printf("[beep] alarm\n");
            fflush(stdout);
#endif
        }
    } else {
        s_beep.alarm_latched = false;
        if (s_beep.request == MOTOR_BEEP_ALARM) {
            s_beep.request = MOTOR_BEEP_NONE;
        }
    }
}

static void advance_notes(uint32_t dt_ms)
{
    if (!s_beep.notes || s_beep.note_idx >= s_beep.note_count) {
        motor_beep_request_t finished = s_beep.request;
        s_beep.request = MOTOR_BEEP_NONE;
        if (finished == MOTOR_BEEP_MELODY_FULL && !s_beep.boot_done) {
            s_beep.boot_done = true;
        }
        return;
    }

    s_beep.note_elapsed_ms += dt_ms;
    const beep_note_t *n = &s_beep.notes[s_beep.note_idx];

    if (s_beep.note_elapsed_ms >= n->duration_ms) {
        s_beep.note_elapsed_ms = 0;
        s_beep.note_idx++;
        if (s_beep.note_idx >= s_beep.note_count) {
            if (s_beep.request == MOTOR_BEEP_ALARM) {
                s_beep.note_idx = 0;
                s_beep.request = MOTOR_BEEP_NONE;
            } else {
                motor_beep_request_t finished = s_beep.request;
                s_beep.request = MOTOR_BEEP_NONE;
                if (finished == MOTOR_BEEP_MELODY_FULL && !s_beep.boot_done) {
                    s_beep.boot_done = true;
                }
            }
        }
    }
}

void motor_beep_fast_loop(const esc_params_t *p, float dt,
                          float *duty_a, float *duty_b, float *duty_c)
{
    if (!motor_beep_enabled(p) || s_beep.request == MOTOR_BEEP_NONE) {
        *duty_a = *duty_b = *duty_c = 0.5f;
        return;
    }

    if (s_beep.note_idx >= s_beep.note_count) {
        *duty_a = *duty_b = *duty_c = 0.5f;
        return;
    }

    const beep_note_t *n = &s_beep.notes[s_beep.note_idx];
    float amp = volume_to_amp(p);

    if (n->hz <= 1.0f) {
        *duty_a = *duty_b = *duty_c = 0.5f;
        return;
    }

    const float two_pi = 6.2831853f;
    s_beep.phase += two_pi * n->hz * dt;
    if (s_beep.phase > two_pi) {
        s_beep.phase -= two_pi;
    }

    float s = sinf(s_beep.phase) * amp;
    *duty_a = 0.5f + s;
    *duty_b = 0.5f + sinf(s_beep.phase - 2.0943951f) * amp;
    *duty_c = 0.5f + sinf(s_beep.phase + 2.0943951f) * amp;
}

/* 由 app 慢环调用：推进音符与报警周期 */
void motor_beep_tick_1ms(const esc_params_t *p, uint32_t dt_ms)
{
    if (!motor_beep_enabled(p)) {
        return;
    }

    if (s_beep.request == MOTOR_BEEP_NONE) {
        if (s_beep.alarm_latched) {
            s_beep.alarm_cycle_ms += dt_ms;
            if (s_beep.alarm_cycle_ms >= ALARM_CYCLE_MS) {
                s_beep.alarm_cycle_ms = 0;
                start_sequence(s_alarm_pair, (uint8_t)ALARM_PAIR_COUNT,
                               MOTOR_BEEP_ALARM);
            }
        }
        return;
    }

    advance_notes(dt_ms);
}
