/**
 * @file protocol.c
 */
#include "protocol.h"
#include <string.h>

static uint16_t crc16_step(uint16_t crc, uint8_t b)
{
    crc ^= (uint16_t)b;
    for (int i = 0; i < 8; i++) {
        if (crc & 1u) {
            crc = (uint16_t)((crc >> 1) ^ 0xA001u);
        } else {
            crc >>= 1;
        }
    }
    return crc;
}

uint16_t proto_crc16(const uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFFu;
    for (size_t i = 0; i < len; i++) {
        crc = crc16_step(crc, data[i]);
    }
    return crc;
}

size_t frame_pack(uint8_t cmd, const uint8_t *payload, uint16_t len,
                      uint8_t *out, size_t out_cap)
{
    size_t need = ESC_PROTO_HEADER_SIZE + len + ESC_PROTO_CRC_SIZE;
    if (out_cap < need) {
        return 0;
    }
    out[0] = ESC_PROTO_SYNC0;
    out[1] = ESC_PROTO_SYNC1;
    out[2] = cmd;
    out[3] = (uint8_t)(len & 0xFFu);
    out[4] = (uint8_t)((len >> 8) & 0xFFu);
    if (len > 0 && payload != NULL) {
        memcpy(&out[5], payload, len);
    }
    uint16_t crc = proto_crc16(&out[2], (size_t)(3u + len));
    out[5 + len] = (uint8_t)(crc & 0xFFu);
    out[6 + len] = (uint8_t)((crc >> 8) & 0xFFu);
    return need;
}

enum {
    PS_SYNC0,
    PS_SYNC1,
    PS_CMD,
    PS_LEN0,
    PS_LEN1,
    PS_PAYLOAD,
    PS_CRC0,
    PS_CRC1,
};

static struct {
    uint8_t  state;
    uint8_t  cmd;
    uint16_t len;
    uint16_t idx;
    uint8_t  buf[ESC_PROTO_MAX_PAYLOAD];
    uint8_t  crc0;
} rx;

void frame_reset(void)
{
    memset(&rx, 0, sizeof(rx));
    rx.state = PS_SYNC0;
}

int frame_feed(uint8_t byte, frame_handler_t handler, void *user)
{
    switch (rx.state) {
    case PS_SYNC0:
        if (byte == ESC_PROTO_SYNC0) {
            rx.state = PS_SYNC1;
        }
        break;
    case PS_SYNC1:
        if (byte == ESC_PROTO_SYNC1) {
            rx.state = PS_CMD;
        } else {
            rx.state = (byte == ESC_PROTO_SYNC0) ? PS_SYNC1 : PS_SYNC0;
        }
        break;
    case PS_CMD:
        rx.cmd = byte;
        rx.state = PS_LEN0;
        break;
    case PS_LEN0:
        rx.len = byte;
        rx.state = PS_LEN1;
        break;
    case PS_LEN1:
        rx.len |= (uint16_t)((uint16_t)byte << 8);
        if (rx.len > ESC_PROTO_MAX_PAYLOAD) {
            frame_reset();
            return -1;
        }
        rx.idx = 0;
        rx.state = (rx.len == 0) ? PS_CRC0 : PS_PAYLOAD;
        break;
    case PS_PAYLOAD:
        rx.buf[rx.idx++] = byte;
        if (rx.idx >= rx.len) {
            rx.state = PS_CRC0;
        }
        break;
    case PS_CRC0:
        rx.crc0 = byte;
        rx.state = PS_CRC1;
        break;
    case PS_CRC1: {
        uint16_t got = (uint16_t)rx.crc0 | ((uint16_t)byte << 8);
        uint8_t hdr[3 + ESC_PROTO_MAX_PAYLOAD];
        uint8_t cmd = rx.cmd;
        uint16_t plen = rx.len;
        hdr[0] = cmd;
        hdr[1] = (uint8_t)(plen & 0xFFu);
        hdr[2] = (uint8_t)((plen >> 8) & 0xFFu);
        if (plen > 0) {
            memcpy(&hdr[3], rx.buf, plen);
        }
        uint16_t calc = proto_crc16(hdr, (size_t)(3u + plen));
        if (got != calc) {
            frame_reset();
            return -2;
        }
        uint8_t pl_copy[ESC_PROTO_MAX_PAYLOAD];
        if (plen > ESC_PROTO_MAX_PAYLOAD) {
            plen = ESC_PROTO_MAX_PAYLOAD;
        }
        if (plen > 0) {
            memcpy(pl_copy, rx.buf, plen);
        }
        frame_reset();
        if (handler) {
            return handler(cmd, pl_copy, plen, user);
        }
        return 1;
    }
    default:
        frame_reset();
        break;
    }
    return 0;
}
