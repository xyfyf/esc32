#include "mcu_catalog.h"

const char *esc_mcu_id_to_string(uint8_t mcu_id)
{
    switch (mcu_id) {
    case ESC_MCU_SIM: return "SIM";
    case ESC_MCU_STSPIN32F0: return "STSPIN32F0";
    case ESC_MCU_STM32F051: return "STM32F051";
    case ESC_MCU_STM32G071: return "STM32G071";
    case ESC_MCU_STM32G431: return "STM32G431";
    case ESC_MCU_STM32G474: return "STM32G474";
    case ESC_MCU_STM32H743: return "STM32H743";
    case ESC_MCU_GD32E230: return "GD32E230";
    case ESC_MCU_GD32F303: return "GD32F303";
    case ESC_MCU_AT32F415: return "AT32F415";
    case ESC_MCU_AT32F421: return "AT32F421";
    case ESC_MCU_AT32F435: return "AT32F435";
    case ESC_MCU_CKS32F051: return "CKS32F051";
    default: return "UNKNOWN";
    }
}
