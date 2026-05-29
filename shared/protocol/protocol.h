/**
 * @file protocol.h
 * @brief esc32 host <-> firmware debug/tuning protocol (UART or UDP transport)
 *
 * Frame layout: [0xEC][0x32][CMD][LEN_L][LEN_H][PAYLOAD...][CRC16_L][CRC16_H]
 * CRC16: Modbus/IBM polynomial 0xA001, init 0xFFFF, over CMD+LEN+PAYLOAD
 */
#ifndef ESC_PROTOCOL_H
#define ESC_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

#define ESC_PROTO_SYNC0       0xECu
#define ESC_PROTO_SYNC1       0x32u
#define ESC_PROTO_HEADER_SIZE 5u
#define ESC_PROTO_CRC_SIZE    2u
#define ESC_PROTO_MAX_PAYLOAD 512u

/** Protocol version; must match firmware ESC_FW_PROTO_VERSION */
#define ESC_PROTO_VERSION     3u

typedef enum {
    ESC_CMD_PING = 0x01,
    ESC_CMD_GET_INFO = 0x02,
    ESC_CMD_GET_TELEM = 0x03,
    ESC_CMD_GET_PARAM = 0x10,
    ESC_CMD_SET_PARAM = 0x11,
    ESC_CMD_SAVE_PARAMS = 0x12,
    ESC_CMD_LOAD_DEFAULTS = 0x13,
    ESC_CMD_ARM = 0x20,
    ESC_CMD_DISARM = 0x21,
    ESC_CMD_SET_THROTTLE = 0x22,
    ESC_CMD_GET_FAULT_LOG = 0x30,
    ESC_CMD_FW_ERASE = 0xF0,
    ESC_CMD_FW_WRITE = 0xF1,
    ESC_CMD_FW_CRC = 0xF2,
    ESC_CMD_FW_REBOOT = 0xF3,
} esc_cmd_t;

typedef enum {
    ESC_ERR_OK = 0,
    ESC_ERR_BAD_CRC = 1,
    ESC_ERR_BAD_CMD = 2,
    ESC_ERR_BAD_STATE = 3,
    ESC_ERR_PARAM_RANGE = 4,
    ESC_ERR_BUSY = 5,
} esc_err_t;

#pragma pack(push, 1)

typedef struct {
    uint8_t  proto_version;
    uint8_t  mcu_id;          /* see firmware/include/mcu_catalog.h */
    uint16_t product_id;    /* 0x60 / 0x80 / 0x120 / 0x200 */
    uint16_t fw_version;      /* major<<8 | minor */
    uint16_t target_id;       /* build target id, e.g. 0x8081 */
    uint16_t hw_revision;     /* PCB revision */
    char     name[16];        /* Firmware name / target short name */
    char     build_date[12];
    uint8_t  feature_flags;   /* bit0 motor beep, bit1 UAVCAN */
    uint8_t  port_status;     /* 0=ready 1=stub 2=planned */
    uint8_t  reserved[2];
} esc_info_rsp_t;

#define ESC_INFO_FEAT_MOTOR_BEEP  0x01u
#define ESC_INFO_FEAT_UAVCAN      0x02u
#define ESC_INFO_PORT_READY       0u
#define ESC_INFO_PORT_STUB        1u
#define ESC_INFO_PORT_PLANNED     2u

typedef struct {
    uint16_t offset;
    uint16_t count;
} esc_fault_log_req_t;

typedef struct {
    uint16_t total;
    uint16_t returned;
    /* followed by `returned` fault_log_entry_t records (18 bytes each; see fault_log.h) */
} esc_fault_log_rsp_hdr_t;

typedef struct {
    uint32_t uptime_ms;
    uint8_t  state;
    uint8_t  fault_code;
    uint16_t vbus_mv;
    int16_t  ibus_ma;
    int16_t  temp_mos_c10;
    int32_t  rpm;
    int16_t  id_ma;
    int16_t  iq_ma;
    uint16_t throttle_us;
    uint8_t  input_source;
} esc_telem_rsp_t;

#pragma pack(pop)

uint16_t proto_crc16(const uint8_t *data, size_t len);

/** Pack frame into out; returns total length; buffer needs header+payload+crc */
size_t frame_pack(uint8_t cmd, const uint8_t *payload, uint16_t len,
                      uint8_t *out, size_t out_cap);

/**
 * Parser state machine: feed bytes; callback(cmd,payload,len) on complete frame
 * @return 0 continue, 1 frame done, <0 error (reset parser)
 */
typedef int (*frame_handler_t)(uint8_t cmd, const uint8_t *payload,
                                   uint16_t len, void *user);
int frame_feed(uint8_t byte, frame_handler_t handler, void *user);
void frame_reset(void);

#endif /* ESC_PROTOCOL_H */
