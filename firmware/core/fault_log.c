#include "fault_log.h"
#include "hal.h"
#include <string.h>

_Static_assert(sizeof(fault_log_entry_t) == 18, "fault_log_entry_t wire size");

static fault_log_entry_t s_log[FAULT_LOG_CAPACITY];
static uint16_t s_head;
static uint16_t s_count;

void fault_log_init(void)
{
    memset(s_log, 0, sizeof(s_log));
    s_head = 0;
    s_count = 0;
    fault_log_load_flash();
}

void fault_log_push(fault_code_t code, uint32_t ms, uint16_t vbus_mv,
                    int16_t ibus_ma, int16_t temp_c10, int32_t rpm,
                    uint16_t throttle_us, uint8_t state)
{
    if (code == ESC_FAULT_NONE) {
        return;
    }
    fault_log_entry_t e;
    e.timestamp_ms = ms;
    e.code = (uint8_t)code;
    e.vbus_mv = vbus_mv;
    e.ibus_ma = ibus_ma;
    e.temp_c10 = temp_c10;
    e.rpm = rpm;
    e.throttle_us = throttle_us;
    e.state = state;

    s_log[s_head] = e;
    s_head = (uint16_t)((s_head + 1) % FAULT_LOG_CAPACITY);
    if (s_count < FAULT_LOG_CAPACITY) {
        s_count++;
    }
    fault_log_save_flash();
}

uint16_t fault_log_count(void)
{
    return s_count;
}

uint16_t fault_log_read(uint16_t offset, fault_log_entry_t *out, uint16_t max_n)
{
    if (offset >= s_count || max_n == 0) {
        return 0;
    }
    uint16_t n = s_count - offset;
    if (n > max_n) {
        n = max_n;
    }
    for (uint16_t i = 0; i < n; i++) {
        uint16_t idx = (uint16_t)((s_head + FAULT_LOG_CAPACITY - s_count + offset + i) %
                                  FAULT_LOG_CAPACITY);
        out[i] = s_log[idx];
    }
    return n;
}

void fault_log_clear(void)
{
    s_head = 0;
    s_count = 0;
    memset(s_log, 0, sizeof(s_log));
    fault_log_save_flash();
}

bool fault_log_save_flash(void)
{
    return hal_nvm_write(0, s_log, sizeof(s_log)) == 0;
}

bool fault_log_load_flash(void)
{
    if (hal_nvm_read(0, s_log, sizeof(s_log)) != 0) {
        return false;
    }
    /* 从 NVM 恢复：根据首条时间戳判断是否有效 */
    if (s_log[0].timestamp_ms == 0xFFFFFFFFu) {
        return false;
    }
    s_count = 0;
    s_head = 0;
    for (uint16_t i = 0; i < FAULT_LOG_CAPACITY; i++) {
        if (s_log[i].code != ESC_FAULT_NONE && s_log[i].timestamp_ms != 0) {
            s_count++;
        }
    }
    s_head = s_count % FAULT_LOG_CAPACITY;
    return s_count > 0;
}
