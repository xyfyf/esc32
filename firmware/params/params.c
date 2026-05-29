/**
 * @file params.c
 */
#include "params.h"
#include "../../shared/protocol/protocol.h"
#include "hal.h"
#include <string.h>
#include <stddef.h>

#define PARAMS_NVM_OFFSET 0x2000u

esc_params_t g_params;

#define OFF(field) ((uint16_t)offsetof(esc_params_t, field))

static const param_desc_t s_desc[] = {
    {"motor_kv", ESC_PARAM_TYPE_U16, OFF(motor_kv), 100, 3000, 170},
    {"motor_pole_pairs", ESC_PARAM_TYPE_U16, OFF(motor_pole_pairs), 1, 64, 21},
    {"motor_ld", ESC_PARAM_TYPE_FLOAT, OFF(motor_ld_uh), 1, 500, 29},
    {"motor_lq", ESC_PARAM_TYPE_FLOAT, OFF(motor_lq_uh), 1, 500, 42},
    {"motor_rs", ESC_PARAM_TYPE_FLOAT, OFF(motor_rs_mohm), 1, 500, 21.6f},
    {"motor_max_current", ESC_PARAM_TYPE_FLOAT, OFF(motor_max_current_a), 1, 300, 80},
    {"motor_max_rpm", ESC_PARAM_TYPE_U32, OFF(motor_max_rpm), 500, 50000, 6000},
    {"observer_type", ESC_PARAM_TYPE_U8, OFF(observer_type), 0, 5, 1},
    {"observer_coef", ESC_PARAM_TYPE_FLOAT, OFF(observer_coef), 0, 10, 1},
    {"observer_filter_freq", ESC_PARAM_TYPE_FLOAT, OFF(observer_filter_freq_hz), 10, 2000, 400},
    {"carrier_freq_khz", ESC_PARAM_TYPE_FLOAT, OFF(carrier_freq_khz), 8, 48, 20},
    {"s_speed_loop_kp", ESC_PARAM_TYPE_FLOAT, OFF(s_speed_loop_kp), 0, 10, 0.05f},
    {"s_speed_loop_ki", ESC_PARAM_TYPE_FLOAT, OFF(s_speed_loop_ki), 0, 10, 0.01f},
    {"ibus_max_current", ESC_PARAM_TYPE_FLOAT, OFF(ibus_max_current_a), 0, 300, 60},
    {"power_limit", ESC_PARAM_TYPE_FLOAT, OFF(power_limit_w), 0, 10000, 2200},
    {"node_id", ESC_PARAM_TYPE_U8, OFF(node_id), 1, 127, 30},
    {"esc_index", ESC_PARAM_TYPE_U8, OFF(esc_index), 0, 15, 0},
    {"normal_pwm_start", ESC_PARAM_TYPE_U16, OFF(normal_pwm_start_us), 800, 1500, 1050},
    {"normal_pwm_end", ESC_PARAM_TYPE_U16, OFF(normal_pwm_end_us), 1500, 2200, 1950},
    {"low_voltage_protect_enable", ESC_PARAM_TYPE_U8, OFF(low_voltage_protect_enable), 0, 1, 1},
    {"vbus_lower_limit", ESC_PARAM_TYPE_FLOAT, OFF(vbus_lower_limit_v), 10, 80, 42},
    {"vbus_upper_limit", ESC_PARAM_TYPE_FLOAT, OFF(vbus_upper_limit_v), 20, 90, 60},
    {"stall_enable", ESC_PARAM_TYPE_U8, OFF(stall_enable), 0, 1, 1},
    {"motor_sound_enable", ESC_PARAM_TYPE_U8, OFF(motor_sound_enable), 0, 1, 1},
    {"motor_sound_volume", ESC_PARAM_TYPE_U8, OFF(pndef_motor_sound_volume), 1, 15, 8},
};

uint16_t params_crc(const esc_params_t *p)
{
    esc_params_t tmp;
    memcpy(&tmp, p, sizeof(tmp));
    tmp.crc16 = 0;
    return proto_crc16((const uint8_t *)&tmp, sizeof(tmp));
}

bool params_validate(const esc_params_t *p)
{
    if (p->magic != ESC_PARAMS_MAGIC) {
        return false;
    }
    if (p->crc16 != params_crc(p)) {
        return false;
    }
    if (p->motor_pole_pairs < 1) {
        return false;
    }
    if (p->normal_pwm_end_us <= p->normal_pwm_start_us) {
        return false;
    }
    return true;
}

bool params_load_flash(esc_params_t *p)
{
    if (hal_nvm_read(PARAMS_NVM_OFFSET, p, sizeof(*p)) != 0) {
        return false;
    }
    return params_validate(p);
}

bool params_save_flash(const esc_params_t *p)
{
    esc_params_t tmp = *p;
    tmp.crc16 = params_crc(&tmp);
    return hal_nvm_write(PARAMS_NVM_OFFSET, &tmp, sizeof(tmp)) == 0;
}

uint16_t params_desc_count(void)
{
    return (uint16_t)(sizeof(s_desc) / sizeof(s_desc[0]));
}

const param_desc_t *params_desc_get(uint16_t index)
{
    if (index >= params_desc_count()) {
        return NULL;
    }
    return &s_desc[index];
}

int params_get_by_name(const esc_params_t *p, const char *name, float *out_val)
{
    if (strcmp(name, "motor_kv") == 0) {
        *out_val = (float)p->motor_kv;
        return 0;
    }
    if (strcmp(name, "motor_pole_pairs") == 0) {
        *out_val = (float)p->motor_pole_pairs;
        return 0;
    }
    if (strcmp(name, "node_id") == 0) {
        *out_val = (float)p->node_id;
        return 0;
    }
    for (uint16_t i = 0; i < params_desc_count(); i++) {
        if (strcmp(s_desc[i].name, name) != 0) {
            continue;
        }
        const uint8_t *base = (const uint8_t *)p;
        switch (s_desc[i].type) {
        case ESC_PARAM_TYPE_U8:
            *out_val = (float)base[s_desc[i].offset];
            return 0;
        case ESC_PARAM_TYPE_U16:
            *out_val = (float)*(const uint16_t *)(base + s_desc[i].offset);
            return 0;
        case ESC_PARAM_TYPE_U32:
            *out_val = (float)*(const uint32_t *)(base + s_desc[i].offset);
            return 0;
        case ESC_PARAM_TYPE_FLOAT:
            *out_val = *(const float *)(base + s_desc[i].offset);
            return 0;
        default:
            return -1;
        }
    }
    return -1;
}

int params_set_by_name(esc_params_t *p, const char *name, float val)
{
    for (uint16_t i = 0; i < params_desc_count(); i++) {
        if (strcmp(s_desc[i].name, name) != 0) {
            continue;
        }
        if (val < s_desc[i].min_val || val > s_desc[i].max_val) {
            return -2;
        }
        uint8_t *base = (uint8_t *)p;
        switch (s_desc[i].type) {
        case ESC_PARAM_TYPE_U8:
            base[s_desc[i].offset] = (uint8_t)val;
            break;
        case ESC_PARAM_TYPE_U16:
            *(uint16_t *)(base + s_desc[i].offset) = (uint16_t)val;
            break;
        case ESC_PARAM_TYPE_U32:
            *(uint32_t *)(base + s_desc[i].offset) = (uint32_t)val;
            break;
        case ESC_PARAM_TYPE_FLOAT:
            *(float *)(base + s_desc[i].offset) = val;
            break;
        default:
            return -1;
        }
        p->crc16 = params_crc(p);
        return 0;
    }
    return -1;
}

void params_load_defaults(esc_params_t *p, uint8_t board_id)
{
    (void)board_id;
    memset(p, 0, sizeof(*p));
    p->magic = ESC_PARAMS_MAGIC;
    p->struct_version = ESC_PARAMS_VERSION;

    strncpy(p->config_name, "ESC-80-default", ESC_CONFIG_NAME_LEN - 1);
    strncpy(p->device_name, "esc32", ESC_DEVICE_NAME_LEN - 1);

    p->motor_kv = 170;
    p->motor_pole_pairs = 21;
    p->motor_ld_uh = 29.0f;
    p->motor_lq_uh = 42.0f;
    p->motor_rs_mohm = 21.6f;
    p->motor_max_current_a = 80.0f;
    p->motor_max_rpm = 6000;

    p->observer_type = 1;
    p->observer_coef = 1.0f;
    p->observer_filter_freq_hz = 400.0f;
    p->current_loop_coef = 1.0f;
    p->carrier_freq_khz = 20.0f;
    p->s_speed_loop_kp = 0.05f;
    p->s_speed_loop_ki = 0.01f;
    p->f_speed_loop_kp = 0.08f;
    p->f_speed_loop_ki = 0.02f;
    p->speed_ref_lpf_hz = 10.0f;

    p->normal_pwm_start_us = 1050;
    p->normal_pwm_end_us = 1950;
    p->ppm_lost_time_ms = 500;
    p->throttle_lost_flag_time_ms = 300;
    p->node_id = 30;
    p->can_baudrate = 1000000;
    p->uart_baudrate = 115200;
    p->status_period_ms = 50;

    p->ibus_max_current_a = 60.0f;
    p->power_limit_w = 2200.0f;
    p->low_voltage_protect_enable = 1;
    p->vbus_lower_limit_v = 42.0f;
    p->vbus_upper_limit_v = 60.0f;
    p->over_voltage_protect_enable = 1;
    p->over_voltage_threshold_v = 63.0f;
    p->mos_high_temp_limit_1_c = 95.0f;
    p->mos_high_temp_limit_2_c = 105.0f;
    p->stall_enable = 1;
    p->stall_count = 5;
    p->max_acc_krpmps = 50.0f;
    p->max_dec_krpmps = 80.0f;
    p->dcbus_lpf_hz = 20.0f;
    p->motor_sound_enable = 1;
    p->pndef_motor_sound_volume = 8;

    p->crc16 = params_crc(p);
}
