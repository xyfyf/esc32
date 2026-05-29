#ifndef ESC_APP_H
#define ESC_APP_H

#include "state.h"
#include "../../shared/protocol/protocol.h"

void app_init(void);
void app_run_once(void);

uint16_t app_board_id(void);
void app_arm(void);
void app_disarm(void);
void app_set_throttle_us(uint16_t us);
void app_fill_telem(esc_telem_rsp_t *t);
esc_runtime_t *app_runtime(void);

#endif
