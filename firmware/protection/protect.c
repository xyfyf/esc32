#include "protect.h"

fault_code_t protect_check(const esc_params_t *params,
                                   const esc_sensors_t *sns,
                                   esc_runtime_t *rt)
{
    if (params->over_voltage_protect_enable &&
        sns->vbus_v > params->over_voltage_threshold_v) {
        return ESC_FAULT_OVER_VOLTAGE;
    }
    if (params->low_voltage_protect_enable &&
        sns->vbus_v < params->vbus_lower_limit_v &&
        rt->state == ESC_STATE_RUNNING) {
        return ESC_FAULT_UNDER_VOLTAGE;
    }
    if (params->ibus_max_current_a > 0.0f &&
        sns->ibus_a > params->ibus_max_current_a) {
        return ESC_FAULT_OVER_CURRENT;
    }
    if (params->high_temp_protect_enable &&
        sns->temp_mos_c > params->mos_high_temp_limit_2_c) {
        return ESC_FAULT_OVER_TEMP_MOS;
    }
    if (sns->throttle_lost_ms > params->ppm_lost_time_ms &&
        rt->state == ESC_STATE_RUNNING) {
        return ESC_FAULT_THROTTLE_LOST;
    }
    return ESC_FAULT_NONE;
}
