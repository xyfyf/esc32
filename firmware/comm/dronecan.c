/**
 * @file dronecan.c
 * @brief DroneCAN ESC 节点（UAVCAN DSDL 栈）
 */
#include "dronecan.h"
#include "uavcan/uavcan.h"
#include "uavcan/uavcan_dsdl.h"
#include "hal.h"
#include "params.h"
#include "app.h"
#include "target_meta.h"
#include <string.h>

static uavcan_inst_t s_ucan;
static float s_can_throttle;
static bool s_can_armed;
static bool s_can_link_seen;
static uint32_t s_last_cmd_ms;
static uint32_t s_status_ms;
static uint32_t s_node_status_ms;

static void on_message(uint16_t dtid, uint8_t src, const uint8_t *payload,
                       uint16_t len, void *user)
{
    (void)user;
    (void)src;
    if (dtid == UAVCAN_DSDL_ESC_RAWCOMMAND) {
        float norm;
        if (uavcan_dsdl_decode_raw_command(payload, len, g_params.esc_index,
                                         &norm) == 0) {
            s_can_throttle = norm;
            s_can_armed = true;
            s_can_link_seen = true;
            s_last_cmd_ms = hal_millis();
        }
    }
}

static void on_service_request(uint16_t service_id, uint8_t client_node,
                               const uint8_t *payload, uint16_t len,
                               void *user)
{
    (void)user;
    (void)payload;
    (void)len;
    if (service_id != UAVCAN_DSDL_PROTOCOL_GETNODEINFO) {
        return;
    }

    const esc_target_meta_t *meta = esc_target_meta();
    uavcan_node_info_t info = {
        .major = (uint8_t)(meta->hw_revision >> 8),
        .minor = (uint8_t)(meta->hw_revision & 0xFF),
        .name = g_params.device_name,
    };
    uint8_t buf[96];
    uint16_t n = uavcan_dsdl_encode_get_node_info_response(&info, buf, sizeof(buf));
    if (n > 0) {
        uavcan_respond_service(&s_ucan, UAVCAN_DSDL_PROTOCOL_GETNODEINFO,
                               client_node, buf, n);
    }
}

void dronecan_init(uint8_t node_id)
{
    uavcan_init(&s_ucan, node_id);
    s_ucan.on_message = on_message;
    s_ucan.on_service_request = on_service_request;
    s_can_throttle = 0.0f;
    s_can_armed = false;
    s_can_link_seen = false;
    s_last_cmd_ms = 0;
    s_status_ms = 0;
    s_node_status_ms = 0;
    hal_can_init(g_params.can_baudrate ? g_params.can_baudrate : 1000000u);
}

void dronecan_poll(void)
{
    uint32_t id;
    uint8_t data[8];
    uint8_t len;

    while (hal_can_receive(&id, data, &len, 8) > 0) {
        uavcan_on_can_frame(&s_ucan, id, data, len);
    }

    uint32_t now = hal_millis();
    if (s_can_link_seen && s_can_armed &&
        (now - s_last_cmd_ms) > g_params.ppm_lost_time_ms) {
        s_can_armed = false;
        s_can_throttle = 0.0f;
    }
}

void dronecan_set_throttle_norm(float norm)
{
    s_can_throttle = norm;
    s_can_armed = true;
    s_can_link_seen = true;
    s_last_cmd_ms = hal_millis();
}

float dronecan_get_throttle_norm(void)
{
    return s_can_throttle;
}

bool dronecan_armed(void)
{
    return s_can_armed;
}

bool dronecan_link_established(void)
{
    return s_can_link_seen;
}

void dronecan_on_fast_loop(void)
{
    uint32_t now = hal_millis();

    if (now - s_status_ms >= g_params.status_period_ms) {
        s_status_ms = now;
        esc_telem_rsp_t t;
        app_fill_telem(&t);

        uavcan_esc_status_t st = {
            .error_count = (uint8_t)t.fault_code,
            .voltage_v = t.vbus_mv / 1000.0f,
            .current_a = t.ibus_ma / 1000.0f,
            .rpm = t.rpm,
            .power_rating_pct = 0,
            .esc_temperature_c = (uint8_t)(t.temp_mos_c10 / 10),
        };
        uint8_t buf[8];
        uint16_t n = uavcan_dsdl_encode_status(&st, buf, sizeof(buf));
        if (n > 0) {
            uavcan_broadcast(&s_ucan, UAVCAN_DSDL_ESC_STATUS, buf, n);
        }

        uavcan_esc_status_ext_t se = {
            .voltage_v = t.vbus_mv / 1000.0f,
            .current_a = t.iq_ma,
            .temperature_c = (uint8_t)(t.temp_mos_c10 / 10),
            .esc_index = g_params.esc_index,
        };
        n = uavcan_dsdl_encode_status_ext(&se, buf, sizeof(buf));
        if (n > 0) {
            uavcan_broadcast(&s_ucan, UAVCAN_DSDL_ESC_STATUSEXT, buf, n);
        }
    }

    if (now - s_node_status_ms >= 1000u) {
        s_node_status_ms = now;
        uavcan_node_status_t ns = {
            .uptime_sec = now / 1000u,
            .health = 0,
            .mode = 0,
            .sub_mode = 0,
            .vendor_specific = 0,
        };
        uint8_t buf[8];
        uint16_t n = uavcan_dsdl_encode_node_status(&ns, buf, sizeof(buf));
        if (n > 0) {
            uavcan_broadcast(&s_ucan, UAVCAN_DSDL_PROTOCOL_NODESTATUS, buf, n);
        }
    }
}
