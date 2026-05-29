/**
 * @file uavcan.c
 * @brief UAVCAN v0 单/多帧传输（CRC + 尾字节）
 */
#include "uavcan.h"
#include "uavcan_crc.h"
#include "hal.h"
#include <string.h>

#define TAIL_START  0x80u
#define TAIL_END    0x40u
#define TAIL_TOGGLE 0x20u
#define TAIL_MASK   0x1Fu

typedef struct {
    uint16_t data_type_id;
    uint8_t  source_node;
    uint8_t  transfer_id;
    bool     toggle;
    uint16_t len;
    uint8_t  buf[UAVCAN_MAX_PAYLOAD];
    uavcan_crc_t crc;
    bool     active;
} uavcan_rx_xfer_t;

static uavcan_rx_xfer_t s_rx;

static uint8_t make_tail(bool start, bool end, bool toggle, uint8_t tid)
{
    uint8_t t = tid & TAIL_MASK;
    if (toggle) {
        t |= TAIL_TOGGLE;
    }
    if (end) {
        t |= TAIL_END;
    }
    if (start) {
        t |= TAIL_START;
    }
    return t;
}

uint32_t uavcan_make_message_id(uint8_t priority, uint16_t data_type_id,
                                uint8_t source_node)
{
    return ((uint32_t)priority << 24) | ((uint32_t)data_type_id << 8) |
           (uint32_t)source_node;
}

static uint32_t make_service_response_id(uint8_t priority, uint16_t service_id,
                                         uint8_t client_node, uint8_t server_node)
{
    uint16_t dt = (uint16_t)(0x8000u | ((uint16_t)client_node << 8) |
                             (service_id & 0xFFu));
    return ((uint32_t)priority << 24) | ((uint32_t)dt << 8) | server_node;
}

static bool parse_service_request(uint32_t can_id, uint8_t my_node,
                                  uint16_t *service_id, uint8_t *client_node)
{
    uint16_t dt = (uint16_t)((can_id >> 8) & 0xFFFFu);
    if (dt & 0x8000u) {
        return false;
    }
    uint8_t dest = (uint8_t)((dt >> 8) & 0x7Fu);
    if (dest != my_node) {
        return false;
    }
    *service_id = dt & 0xFFu;
    *client_node = (uint8_t)(can_id & 0x7Fu);
    return true;
}

static void rx_reset(void)
{
    memset(&s_rx, 0, sizeof(s_rx));
}

static void rx_deliver(uavcan_inst_t *inst)
{
    if (!inst->on_message || s_rx.len == 0) {
        return;
    }
    inst->on_message(s_rx.data_type_id, s_rx.source_node, s_rx.buf, s_rx.len,
                     inst->user);
}

static void rx_consume_frame(uavcan_inst_t *inst, uint32_t can_id,
                             const uint8_t *data, uint8_t len)
{
    if (len < 1) {
        return;
    }

    uint16_t dt = (uint16_t)((can_id >> 8) & 0xFFFFu);
    if (dt & 0x8000u) {
        return;
    }

    uint8_t src = (uint8_t)(can_id & 0x7Fu);
    uint8_t tail = data[len - 1];
    bool start = (tail & TAIL_START) != 0;
    bool end = (tail & TAIL_END) != 0;
    bool toggle = (tail & TAIL_TOGGLE) != 0;
    uint8_t tid = tail & TAIL_MASK;
    uint8_t plen = (uint8_t)(len - 1);

    if (start) {
        rx_reset();
        s_rx.active = true;
        s_rx.data_type_id = dt;
        s_rx.source_node = src;
        s_rx.transfer_id = tid;
        s_rx.toggle = toggle;
        uavcan_crc_init(&s_rx.crc);
    } else if (!s_rx.active || s_rx.data_type_id != dt ||
               s_rx.source_node != src || s_rx.transfer_id != tid) {
        return;
    }

    if (!start && toggle == s_rx.toggle) {
        return;
    }
    s_rx.toggle = toggle;

    if (start && end) {
        memcpy(s_rx.buf, data, plen);
        s_rx.len = plen;
        rx_deliver(inst);
        rx_reset();
        return;
    }

    if (s_rx.len + plen > UAVCAN_MAX_PAYLOAD) {
        rx_reset();
        return;
    }

    uavcan_crc_add_buffer(&s_rx.crc, data, plen);
    memcpy(&s_rx.buf[s_rx.len], data, plen);
    s_rx.len = (uint16_t)(s_rx.len + plen);

    if (!end) {
        return;
    }

    if (s_rx.len < 2) {
        rx_reset();
        return;
    }
    s_rx.len -= 2;
    uint16_t crc_rx = (uint16_t)s_rx.buf[s_rx.len] |
                      ((uint16_t)s_rx.buf[s_rx.len + 1] << 8);
    if (uavcan_crc_finish(&s_rx.crc) != crc_rx) {
        rx_reset();
        return;
    }
    rx_deliver(inst);
    rx_reset();
}

void uavcan_init(uavcan_inst_t *inst, uint8_t node_id)
{
    memset(inst, 0, sizeof(*inst));
    inst->node_id = node_id;
    inst->priority_low = 24;
    inst->priority_high = 16;
    rx_reset();
}

void uavcan_on_can_frame(uavcan_inst_t *inst, uint32_t can_id,
                         const uint8_t *data, uint8_t len)
{
    uint16_t service_id;
    uint8_t client_node;

    if (parse_service_request(can_id, inst->node_id, &service_id, &client_node)) {
        if (inst->on_service_request && len > 0) {
            inst->on_service_request(service_id, client_node, data,
                                     (uint16_t)(len - 1), inst->user);
        }
        return;
    }

    rx_consume_frame(inst, can_id, data, len);
}

static int send_frame(uint32_t id, const uint8_t *data, uint8_t len)
{
    return hal_can_send(id, data, len);
}

static int send_transfer(uint32_t can_id, uint8_t tid, const uint8_t *payload,
                         uint16_t len)
{
    if (len <= 7) {
        uint8_t frame[8];
        memcpy(frame, payload, len);
        frame[len] = make_tail(true, true, false, tid);
        return send_frame(can_id, frame, (uint8_t)(len + 1));
    }

    uint8_t buf[UAVCAN_MAX_PAYLOAD + 2];
    if ((uint32_t)len + 2u > sizeof(buf)) {
        return -1;
    }
    memcpy(buf, payload, len);
    uavcan_crc_t crc;
    uavcan_crc_init(&crc);
    uavcan_crc_add_buffer(&crc, payload, len);
    uint16_t cs = uavcan_crc_finish(&crc);
    buf[len] = (uint8_t)(cs & 0xFF);
    buf[len + 1] = (uint8_t)(cs >> 8);
    uint16_t total = (uint16_t)(len + 2);

    uint16_t offset = 0;
    bool toggle = false;
    while (offset < total) {
        uint8_t frame[8];
        uint8_t flen = 0;
        bool start = (offset == 0);
        uint8_t max_data = start ? 6u : 7u;
        uint8_t chunk = (uint8_t)((total - offset) > max_data ? max_data
                                                              : (total - offset));
        memcpy(frame, &buf[offset], chunk);
        flen = chunk;
        offset = (uint16_t)(offset + chunk);
        bool end = (offset >= total);
        frame[flen] = make_tail(start, end, toggle, tid);
        flen++;
        if (send_frame(can_id, frame, flen) < 0) {
            return -1;
        }
        toggle = !toggle;
    }
    return 0;
}

int uavcan_broadcast(uavcan_inst_t *inst, uint16_t data_type_id,
                     const uint8_t *payload, uint16_t len)
{
    uint32_t id =
        uavcan_make_message_id(inst->priority_low, data_type_id, inst->node_id);
    return send_transfer(id, inst->transfer_id++, payload, len);
}

int uavcan_respond_service(uavcan_inst_t *inst, uint16_t service_id,
                           uint8_t client_node, const uint8_t *payload,
                           uint16_t len)
{
    uint32_t id = make_service_response_id(inst->priority_high, service_id,
                                           client_node, inst->node_id);
    return send_transfer(id, inst->transfer_id++, payload, len);
}
