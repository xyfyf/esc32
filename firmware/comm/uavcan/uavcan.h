/**
 * @file uavcan.h
 * @brief UAVCAN v0 transport layer + DSDL codec (DroneCAN-compatible subset)
 */
#ifndef ESC_UAVCAN_H
#define ESC_UAVCAN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define UAVCAN_MAX_PAYLOAD  128u

typedef void (*uavcan_msg_handler_t)(uint16_t data_type_id, uint8_t src_node,
                                     const uint8_t *payload, uint16_t len,
                                     void *user);

typedef void (*uavcan_service_handler_t)(uint16_t service_id, uint8_t client_node,
                                         const uint8_t *payload, uint16_t len,
                                         void *user);

typedef struct {
    uint8_t  node_id;
    uint8_t  priority_low;
    uint8_t  priority_high;
    uint8_t  transfer_id;
    uavcan_msg_handler_t     on_message;
    uavcan_service_handler_t on_service_request;
    void *user;
} uavcan_inst_t;

void uavcan_init(uavcan_inst_t *inst, uint8_t node_id);
void uavcan_on_can_frame(uavcan_inst_t *inst, uint32_t can_id,
                         const uint8_t *data, uint8_t len);

int uavcan_broadcast(uavcan_inst_t *inst, uint16_t data_type_id,
                     const uint8_t *payload, uint16_t len);

int uavcan_respond_service(uavcan_inst_t *inst, uint16_t service_id,
                           uint8_t client_node, const uint8_t *payload,
                           uint16_t len);

uint32_t uavcan_make_message_id(uint8_t priority, uint16_t data_type_id,
                                uint8_t source_node);

#endif
