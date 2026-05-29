#ifndef ESC_FAULT_LOG_H
#define ESC_FAULT_LOG_H

#include <stdint.h>
#include <stdbool.h>
#include "fault.h"

#define FAULT_LOG_CAPACITY 64u

#pragma pack(push, 1)
typedef struct {
    uint32_t      timestamp_ms;
    uint8_t       code;
    uint16_t      vbus_mv;
    int16_t       ibus_ma;
    int16_t       temp_c10;
    int32_t       rpm;
    uint16_t      throttle_us;
    uint8_t       state;
} fault_log_entry_t;
#pragma pack(pop)

void fault_log_init(void);
void fault_log_push(fault_code_t code, uint32_t ms, uint16_t vbus_mv,
                    int16_t ibus_ma, int16_t temp_c10, int32_t rpm,
                    uint16_t throttle_us, uint8_t state);
uint16_t fault_log_count(void);
uint16_t fault_log_read(uint16_t offset, fault_log_entry_t *out, uint16_t max_n);
void fault_log_clear(void);
bool fault_log_save_flash(void);
bool fault_log_load_flash(void);

#endif
