/**
 * @file params.h
 * @brief 电调参数块（电机 / FOC / 保护 / 通信）
 */
#ifndef ESC_PARAMS_H
#define ESC_PARAMS_H

#include <stdint.h>
#include <stdbool.h>
#include "types.h"

#define ESC_PARAMS_MAGIC   0x45505431u /* 'EPT1' */
#define ESC_PARAMS_VERSION 1u
#define ESC_CONFIG_NAME_LEN 32
#define ESC_DEVICE_NAME_LEN 32

typedef struct {
    uint32_t magic;
    uint16_t struct_version;
    uint16_t crc16;

    char config_name[ESC_CONFIG_NAME_LEN];
    char device_name[ESC_DEVICE_NAME_LEN];

    /* 电机 */
    uint16_t motor_kv;
    uint16_t motor_pole_pairs;
    float    motor_ld_uh;
    float    motor_lq_uh;
    float    motor_rs_mohm;
    float    motor_max_current_a;
    uint32_t motor_max_rpm;

    /* 观测器 / FOC */
    uint8_t  observer_type;
    float    observer_coef;
    float    observer_filter_freq_hz;
    float    current_loop_coef;
    float    carrier_freq_khz;
    uint8_t  field_weakening_enable;
    float    field_weakening_max_current_a;
    float    ls_coef;

    /* 速度环 */
    float s_speed_loop_kp;
    float s_speed_loop_ki;
    float f_speed_loop_kp;
    float f_speed_loop_ki;
    float speed_ref_lpf_hz;
    uint8_t speed_loop_anti_windup_enable;
    float   speed_loop_anti_windup_coef;

    /* 位置环（预留） */
    float position_loop_kp;
    float position_loop_ki;
    float position_loop_kd;
    float position_loop_pid_limit_1;
    float position_loop_pid_limit_2;

    /* 油门 / PWM */
    uint8_t  throttle_type;
    uint8_t  ctrl_input_type;
    uint16_t normal_pwm_start_us;
    uint16_t normal_pwm_end_us;
    uint16_t positive_ppm_start;
    uint16_t positive_ppm_end;
    uint16_t negative_ppm_start;
    uint16_t negative_ppm_end;
    uint16_t ppm_lost_time_ms;
    uint16_t throttle_lost_flag_time_ms;
    uint8_t  throttle_recover_check;
    uint8_t  direction;
    uint8_t  ppm_curve_type;
    uint16_t hyst_ppm;

    /* 加减速 */
    float max_acc_krpmps;
    float max_dec_krpmps;
    float max_accel_current_a;
    float max_decel_current_a;
    float idling_speed_rpm;
    float idling_acc_speed_rpm;
    float idling_acc_krpmps;
    float min_startup_speed;
    uint8_t min_startup_speed_enable;

    /* 功率 / 电流限制 */
    float ibus_max_current_a;
    uint16_t ibus_limit_duration_ms;
    float power_limit_w;
    float max_power_limit_w;

    /* 保护 */
    uint8_t low_voltage_protect_enable;
    float   low_protect_voltage_ratio;
    float   vbus_lower_limit_v;
    float   vbus_upper_limit_v;
    float   low_protect_voltage_limit_1_v;
    float   low_protect_voltage_limit_2_v;
    uint8_t over_voltage_protect_enable;
    float   over_voltage_threshold_v;
    float   overt_voltage_tolerance_v;
    float   overt_voltage_tolerance2_v;
    uint8_t brake_over_voltage_protect_enable;
    uint16_t overcurrent_count;
    uint8_t high_temp_protect_enable;
    float mos_high_temp_limit_1_c;
    float mos_high_temp_limit_2_c;
    float mcu_high_temp_limit_1_c;
    float mcu_high_temp_limit_2_c;
    float cap_high_temp_limit_1_c;
    float cap_high_temp_limit_2_c;
    uint8_t stall_enable;
    uint16_t stall_count;
    float stall_full_speed_current_a;
    float stall_idle_speed_current_a;
    uint16_t stall_protected_duration_ms;
    uint8_t stall_recover_enable;

    /* 通信 */
    uint8_t  node_id;
    uint8_t  esc_index;
    uint32_t can_baudrate;
    uint32_t uart_baudrate;
    uint16_t status_period_ms;
    uint16_t status1_period_ms;
    uint16_t status2_period_ms;
    uint16_t status3_period_ms;
    uint16_t status4_period_ms;

    /* 滤波 */
    float dcbus_lpf_hz;

    /* 曲线 21 点 */
    int16_t custom_positive_speed_curve[ESC_CURVE_POINTS];
    int16_t custom_negative_speed_curve[ESC_CURVE_POINTS];
    int16_t normal_custom_speed_curve[ESC_CURVE_POINTS];
    int16_t normal_custom_acc_curve[ESC_CURVE_POINTS];
    int16_t normal_custom_dec_curve[ESC_CURVE_POINTS];

    /* 其它 */
    uint8_t stop_type;
    uint8_t acc_type;
    uint8_t acc_comp_enable;
    float   acc_comp_coef;
    uint8_t noload_detect_enable;
    float   load_detect_current_a;
    float   load_dectect_speed_rpm;
    uint8_t motor_sound_enable;
    uint8_t pndef_motor_sound_volume;
    uint8_t standby_led_type;
    uint8_t led_color;
    uint8_t rs_temp_comp_enable;
    float   rs_temp_comp_coef;
    uint8_t pndef_tailwind_headwind_enable;
    uint8_t position_mode_enable;
    uint8_t position_mode_enable_work_type;
    float   position_mode_max_current_a;
    float   position_mode_acc_krpms;
    float   position_mode_current_loop_coef;
    float   position_mode_speed_loop_kp;
    float   position_mode_speed_loop_ki;
    float   position_mode_speed_loop_kd;
    float   position_mode_speed_pll_freq;
    float   normal_pwm_curve_comp_coef;
    float   postive_pwm_curve_comp_coef;
    float   negative_pwm_curve_comp_coef;
    float   fast_stop_decel_krpmps;
    float   fast_stop_decel_current_a;
    uint8_t rs485_led_port;

} esc_params_t;

void params_load_defaults(esc_params_t *p, uint8_t board_id);
bool params_validate(const esc_params_t *p);
uint16_t params_crc(const esc_params_t *p);
bool params_load_flash(esc_params_t *p);
bool params_save_flash(const esc_params_t *p);

/** 参数描述符：上位机按 index 读写 */
typedef enum {
    ESC_PARAM_TYPE_U8 = 0,
    ESC_PARAM_TYPE_U16,
    ESC_PARAM_TYPE_U32,
    ESC_PARAM_TYPE_I16,
    ESC_PARAM_TYPE_FLOAT,
    ESC_PARAM_TYPE_STRING,
} param_type_t;

typedef struct {
    const char       *name;
    param_type_t  type;
    uint16_t          offset;
    float             min_val;
    float             max_val;
    float             default_val;
} param_desc_t;

uint16_t params_desc_count(void);
const param_desc_t *params_desc_get(uint16_t index);
int params_get_by_name(const esc_params_t *p, const char *name,
                          float *out_val);
int params_set_by_name(esc_params_t *p, const char *name, float val);

extern esc_params_t g_params;

#endif
