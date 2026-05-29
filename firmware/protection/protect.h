#ifndef ESC_PROTECT_H
#define ESC_PROTECT_H

#include <stdbool.h>
#include "fault.h"
#include "params.h"
#include "state.h"

typedef struct {
    float vbus_v;
    float ibus_a;
    float ia;
    float ib;
    float ic;
    float temp_mos_c;
    float temp_mcu_c;
    uint32_t throttle_lost_ms;
} esc_sensors_t;

fault_code_t protect_check(const esc_params_t *params,
                                   const esc_sensors_t *sns,
                                   esc_runtime_t *rt);

#endif
