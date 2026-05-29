/**
 * @file uavcan_crc.c
 * @brief UAVCAN transfer CRC-16-CCITT-FALSE
 */
#include "uavcan_crc.h"

void uavcan_crc_init(uavcan_crc_t *crc)
{
    crc->value = 0xFFFFu;
}

void uavcan_crc_add(uavcan_crc_t *crc, uint8_t byte)
{
    uint16_t v = crc->value ^ (uint16_t)((uint16_t)byte << 8);
    for (uint8_t bit = 0; bit < 8; bit++) {
        if (v & 0x8000u) {
            v = (uint16_t)((v << 1) ^ 0x1021u);
        } else {
            v <<= 1;
        }
    }
    crc->value = v;
}

void uavcan_crc_add_buffer(uavcan_crc_t *crc, const uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        uavcan_crc_add(crc, data[i]);
    }
}

uint16_t uavcan_crc_finish(const uavcan_crc_t *crc)
{
    return crc->value;
}
