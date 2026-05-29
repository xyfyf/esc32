#ifndef ESC_FAULT_H
#define ESC_FAULT_H

#include <stdint.h>

typedef enum {
    ESC_FAULT_NONE = 0,
    ESC_FAULT_OVER_VOLTAGE,
    ESC_FAULT_UNDER_VOLTAGE,
    ESC_FAULT_OVER_CURRENT,
    ESC_FAULT_OVER_TEMP_MOS,
    ESC_FAULT_OVER_TEMP_MCU,
    ESC_FAULT_OVER_TEMP_CAP,
    ESC_FAULT_STALL,
    ESC_FAULT_THROTTLE_LOST,
    ESC_FAULT_DESYNC,
    ESC_FAULT_STARTUP_FAIL,
    ESC_FAULT_FLASH,
    ESC_FAULT_DRIVER,
    ESC_FAULT_COUNT
} fault_code_t;

const char *fault_to_string(fault_code_t code);

#endif
