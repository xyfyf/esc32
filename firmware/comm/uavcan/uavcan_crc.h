#ifndef ESC_UAVCAN_CRC_H
#define ESC_UAVCAN_CRC_H

#include <stdint.h>

typedef struct {
    uint16_t value;
} uavcan_crc_t;

void uavcan_crc_init(uavcan_crc_t *crc);
void uavcan_crc_add(uavcan_crc_t *crc, uint8_t byte);
void uavcan_crc_add_buffer(uavcan_crc_t *crc, const uint8_t *data, uint16_t len);
uint16_t uavcan_crc_finish(const uavcan_crc_t *crc);

#endif
