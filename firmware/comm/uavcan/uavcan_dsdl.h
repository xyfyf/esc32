/**
 * @file uavcan_dsdl.h
 * @brief DSDL message IDs and equipment.esc / protocol codecs
 */
#ifndef ESC_UAVCAN_DSDL_H
#define ESC_UAVCAN_DSDL_H

#include <stdint.h>
#include <stdbool.h>

/* uavcan.equipment.esc */
#define UAVCAN_DSDL_ESC_RAWCOMMAND      1030u
#define UAVCAN_DSDL_ESC_STATUS          1034u
#define UAVCAN_DSDL_ESC_STATUSEXT       1035u

/* uavcan.protocol */
#define UAVCAN_DSDL_PROTOCOL_NODESTATUS  341u
#define UAVCAN_DSDL_PROTOCOL_GETNODEINFO 1u

typedef struct {
    uint8_t  error_count;
    float    voltage_v;
    float    current_a;
    int32_t  rpm;
    uint8_t  power_rating_pct;
    uint8_t  esc_temperature_c;
} uavcan_esc_status_t;

typedef struct {
    float    voltage_v;
    int16_t  current_a;
    uint8_t  temperature_c;
    uint8_t  esc_index;
} uavcan_esc_status_ext_t;

typedef struct {
    uint32_t uptime_sec;
    uint8_t  health;
    uint8_t  mode;
    uint8_t  sub_mode;
    uint16_t vendor_specific;
} uavcan_node_status_t;

typedef struct {
    uint8_t major;
    uint8_t minor;
    const char *name;
} uavcan_node_info_t;

int uavcan_dsdl_decode_raw_command(const uint8_t *payload, uint16_t len,
                                   uint8_t esc_index, float *throttle_norm);

uint16_t uavcan_dsdl_encode_status(const uavcan_esc_status_t *st,
                                   uint8_t *out, uint16_t cap);
uint16_t uavcan_dsdl_encode_status_ext(const uavcan_esc_status_ext_t *st,
                                       uint8_t *out, uint16_t cap);
uint16_t uavcan_dsdl_encode_node_status(const uavcan_node_status_t *st,
                                        uint8_t *out, uint16_t cap);
uint16_t uavcan_dsdl_encode_get_node_info_response(const uavcan_node_info_t *info,
                                                   uint8_t *out, uint16_t cap);

#endif
