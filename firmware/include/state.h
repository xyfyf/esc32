#ifndef ESC_STATE_H
#define ESC_STATE_H

#include <stdint.h>
#include "fault.h"

typedef enum {
    ESC_STATE_BOOT = 0,
    ESC_STATE_DISARMED,
    ESC_STATE_ARMED,
    ESC_STATE_RUNNING,
    ESC_STATE_FAULT,
} esc_state_t;

typedef enum {
    ESC_INPUT_NONE = 0,
    ESC_INPUT_PWM,
    ESC_INPUT_CAN,
    ESC_INPUT_SERIAL,
    ESC_INPUT_DEBUG,
} input_source_t;

typedef struct {
    esc_state_t   state;
    fault_code_t  latched_fault;
    input_source_t input_source;
    uint32_t          uptime_ms;
    float             throttle_norm; /* 0..1 */
    uint16_t          throttle_us;
} esc_runtime_t;

void state_init(esc_runtime_t *rt);
void state_set_fault(esc_runtime_t *rt, fault_code_t fault);

#endif
