/**
 * @file mcu_catalog.h
 * @brief MCU family catalog (common 32-bit ESC MCUs + FOC extension tiers)
 *
 * Family ID: high byte = vendor, low byte = series index; used by host filtering and OTA packaging.
 * New MCU: register here -> boards/mcu/<family>/ -> boards/targets/<TARGET>/.
 */
#ifndef ESC_MCU_CATALOG_H
#define ESC_MCU_CATALOG_H

#include <stdint.h>

/* Vendor domain */
#define ESC_MCU_VENDOR_SIM    0x00u
#define ESC_MCU_VENDOR_ST     0x10u
#define ESC_MCU_VENDOR_GD     0x20u
#define ESC_MCU_VENDOR_AT     0x30u
#define ESC_MCU_VENDOR_WCH    0x40u

#define ESC_MCU_ID(vendor, idx) ((uint8_t)((vendor) | ((idx) & 0x0Fu)))

/* --- Simulation --- */
#define ESC_MCU_SIM           ESC_MCU_ID(ESC_MCU_VENDOR_SIM, 0)

/* --- ST: same family as mainstream 32-bit ESCs --- */
#define ESC_MCU_STSPIN32F0    ESC_MCU_ID(ESC_MCU_VENDOR_ST, 0)  /* integrated predriver */
#define ESC_MCU_STM32F051     ESC_MCU_ID(ESC_MCU_VENDOR_ST, 1)
#define ESC_MCU_STM32G071     ESC_MCU_ID(ESC_MCU_VENDOR_ST, 2)
#define ESC_MCU_STM32G431     ESC_MCU_ID(ESC_MCU_VENDOR_ST, 3)  /* ESC-60, G4 series priority port */
#define ESC_MCU_STM32G474     ESC_MCU_ID(ESC_MCU_VENDOR_ST, 4)  /* ESC-80 first production target */
#define ESC_MCU_STM32H743     ESC_MCU_ID(ESC_MCU_VENDOR_ST, 5)  /* high-power tier */

/* --- GD --- */
#define ESC_MCU_GD32E230      ESC_MCU_ID(ESC_MCU_VENDOR_GD, 0)
#define ESC_MCU_GD32F303      ESC_MCU_ID(ESC_MCU_VENDOR_GD, 1)

/* --- Artery AT32 --- */
#define ESC_MCU_AT32F415      ESC_MCU_ID(ESC_MCU_VENDOR_AT, 0)  /* mid/small ESC, dedicated HAL */
#define ESC_MCU_AT32F421      ESC_MCU_ID(ESC_MCU_VENDOR_AT, 1)
#define ESC_MCU_AT32F435      ESC_MCU_ID(ESC_MCU_VENDOR_AT, 2)

/* --- CKS/WCH (not for production; compatibility register only) --- */
#define ESC_MCU_CKS32F051     ESC_MCU_ID(ESC_MCU_VENDOR_WCH, 0)

#ifndef ESC_MCU_ID_BUILD
#ifdef ESC_PLATFORM_SIM
#define ESC_MCU_ID_BUILD ESC_MCU_SIM
#else
#define ESC_MCU_ID_BUILD ESC_MCU_STM32G474
#endif
#endif

const char *esc_mcu_id_to_string(uint8_t mcu_id);

#endif
