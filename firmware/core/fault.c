#include "fault.h"

static const char *s_names[] = {
    "NONE",
    "OVER_VOLTAGE",
    "UNDER_VOLTAGE",
    "OVER_CURRENT",
    "OVER_TEMP_MOS",
    "OVER_TEMP_MCU",
    "OVER_TEMP_CAP",
    "STALL",
    "THROTTLE_LOST",
    "DESYNC",
    "STARTUP_FAIL",
    "FLASH",
    "DRIVER",
};

const char *fault_to_string(fault_code_t code)
{
    if ((unsigned)code >= ESC_FAULT_COUNT) {
        return "UNKNOWN";
    }
    return s_names[code];
}
