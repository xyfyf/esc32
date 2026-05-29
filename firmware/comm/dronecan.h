#ifndef ESC_DRONECAN_H
#define ESC_DRONECAN_H

#include <stdint.h>
#include <stdbool.h>

/* UAVCAN v0 message IDs (equipment.esc) */
#define UAVCAN_ESC_RAWCOMMAND_ID   1030u
#define UAVCAN_ESC_STATUS_ID       1034u
#define UAVCAN_ESC_STATUSEXT_ID    1035u
#define UAVCAN_NODE_STATUS_ID      341u

void dronecan_init(uint8_t node_id);
void dronecan_poll(void);
void dronecan_set_throttle_norm(float norm);
float dronecan_get_throttle_norm(void);
bool dronecan_armed(void);
bool dronecan_link_established(void);
void dronecan_on_fast_loop(void);

#endif
