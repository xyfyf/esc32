/**
 * @file target.h
 * @brief 由 ESC_TARGET 编译宏选择 boards/targets/<name>/target.h
 */
#ifndef ESC_TARGET_H
#define ESC_TARGET_H

#include "mcu_catalog.h"
#include "product.h"

#if defined(ESC_TARGET_ESC80_STM32G474_V1)
#include "../boards/targets/ESC80_STM32G474_V1/target.h"
#elif defined(ESC_TARGET_ESC80_AT32F415_V1)
#include "../boards/targets/ESC80_AT32F415_V1/target.h"
#elif defined(ESC_TARGET_ESC60_STM32G431_V1)
#include "../boards/targets/ESC60_STM32G431_V1/target.h"
#elif defined(ESC_TARGET_ESC120_STM32H743_V1)
#include "../boards/targets/ESC120_STM32H743_V1/target.h"
#elif defined(ESC_TARGET_ESC200_STM32H743_V1)
#include "../boards/targets/ESC200_STM32H743_V1/target.h"
#else
#include "../boards/targets/ESC_SIM/target.h"
#endif

/* MCU 能力宏（供 platform/mcu_port.c，对标 VESC hw_*.h） */
#if defined(ESC_TARGET_ESC60_STM32G431_V1)
#include "../boards/mcu/stm32g431/mcu_conf.h"
#elif defined(ESC_TARGET_ESC80_STM32G474_V1)
#include "../boards/mcu/stm32g474/mcu_conf.h"
#elif defined(ESC_TARGET_ESC80_AT32F415_V1)
#include "../boards/mcu/at32f415/mcu_conf.h"
#elif defined(ESC_TARGET_ESC120_STM32H743_V1) || defined(ESC_TARGET_ESC200_STM32H743_V1)
#include "../boards/mcu/stm32h743/mcu_conf.h"
#elif defined(ESC_TARGET_ESC_SIM)
#define ESC_MCU_CONF_NAME           "SIM"
#define ESC_MCU_HAS_DUAL_ADC        0
#define ESC_MCU_HAS_OPAMP           0
#define ESC_MCU_HAS_FDCAN           0
#define ESC_MCU_SHUNT_PHASES        3
#define ESC_MCU_RECOMMENDED_PRODUCT ESC_PRODUCT_ESC80
#define ESC_MCU_PWM_FREQ_KHZ_MAX    40.0f
#define ESC_MCU_PORT_STATUS         "ready"
#endif

#include "target_meta.h"

#endif
