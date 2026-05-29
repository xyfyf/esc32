/**
 * @file uavcan_dsdl.c
 */
#include "uavcan_dsdl.h"
#include <string.h>

static uint16_t float16_pack(float v)
{
    if (v <= 0.0f) {
        return 0;
    }
    /* 简化：线性映射到 0..65500，真机可换 IEEE754 half */
    float s = v * 1000.0f;
    if (s > 65500.0f) {
        s = 65500.0f;
    }
    return (uint16_t)s;
}

static void pack_int18_rpm(int32_t rpm, uint8_t out[3])
{
    int32_t v = rpm;
    if (v > 131071) {
        v = 131071;
    }
    if (v < -131072) {
        v = -131072;
    }
    uint32_t u = (uint32_t)(v & 0x3FFFF);
    out[0] = (uint8_t)(u & 0xFF);
    out[1] = (uint8_t)((u >> 8) & 0xFF);
    out[2] = (uint8_t)((u >> 16) & 0x3F);
}

int uavcan_dsdl_decode_raw_command(const uint8_t *payload, uint16_t len,
                                   uint8_t esc_index, float *throttle_norm)
{
    if (!payload || !throttle_norm || len < 2) {
        return -1;
    }
    uint8_t n = payload[0];
    if (esc_index >= n || esc_index >= 20) {
        return -1;
    }
    int8_t raw = (int8_t)payload[1 + esc_index];
    float norm = (float)(raw + 128) / 256.0f;
    if (norm < 0.0f) {
        norm = 0.0f;
    }
    if (norm > 1.0f) {
        norm = 1.0f;
    }
    *throttle_norm = norm;
    return 0;
}

uint16_t uavcan_dsdl_encode_status(const uavcan_esc_status_t *st, uint8_t *out,
                                   uint16_t cap)
{
    if (!st || !out || cap < 7) {
        return 0;
    }
    uint16_t v = float16_pack(st->voltage_v);
    uint16_t i = float16_pack(st->current_a);
    out[0] = st->error_count;
    out[1] = (uint8_t)(v & 0xFF);
    out[2] = (uint8_t)(v >> 8);
    out[3] = (uint8_t)(i & 0xFF);
    out[4] = (uint8_t)(i >> 8);
    pack_int18_rpm(st->rpm, &out[5]);
    return 7;
}

uint16_t uavcan_dsdl_encode_status_ext(const uavcan_esc_status_ext_t *st,
                                       uint8_t *out, uint16_t cap)
{
    if (!st || !out || cap < 7) {
        return 0;
    }
    uint32_t mv = (uint32_t)(st->voltage_v * 1000.0f);
    out[0] = (uint8_t)(mv & 0xFF);
    out[1] = (uint8_t)((mv >> 8) & 0xFF);
    out[2] = (uint8_t)((mv >> 16) & 0xFF);
    out[3] = (uint8_t)(st->current_a & 0xFF);
    out[4] = (uint8_t)((uint16_t)st->current_a >> 8);
    out[5] = st->esc_index;
    out[6] = st->temperature_c;
    return 7;
}

uint16_t uavcan_dsdl_encode_node_status(const uavcan_node_status_t *st,
                                        uint8_t *out, uint16_t cap)
{
    if (!st || !out || cap < 7) {
        return 0;
    }
    out[0] = (uint8_t)(st->uptime_sec & 0xFF);
    out[1] = (uint8_t)((st->uptime_sec >> 8) & 0xFF);
    out[2] = (uint8_t)((st->uptime_sec >> 16) & 0xFF);
    out[3] = (uint8_t)((st->uptime_sec >> 24) & 0xFF);
    out[4] = (uint8_t)(st->health | (st->mode << 2));
    out[5] = st->sub_mode;
    out[6] = (uint8_t)(st->vendor_specific & 0xFF);
    return 7;
}

uint16_t uavcan_dsdl_encode_get_node_info_response(const uavcan_node_info_t *info,
                                                   uint8_t *out, uint16_t cap)
{
    if (!info || !out || cap < 16) {
        return 0;
    }
    uint16_t pos = 0;
    out[pos++] = 1; /* protocol major */
    out[pos++] = 0; /* protocol minor */
  out[pos++] = info->major;
    out[pos++] = info->minor;
    memset(&out[pos], 0, 16);
    pos += 16;
    out[pos++] = 1;
    out[pos++] = 0;
    out[pos++] = 0;
    memset(&out[pos], 0, 8);
    pos += 8;
    if (info->name) {
        size_t n = strlen(info->name);
        if (n > 80) {
            n = 80;
        }
        if (pos + 1 + n > cap) {
            return 0;
        }
        out[pos++] = (uint8_t)n;
        memcpy(&out[pos], info->name, n);
        pos += (uint16_t)n;
    } else {
        out[pos++] = 0;
    }
    return pos;
}
