/**
 * @file pinmap.h
 * @brief ESC-60 + STM32G431CBU6 pin plan (same family as G474, smaller package)
 */
#ifndef ESC_STM32G431_PINMAP_H
#define ESC_STM32G431_PINMAP_H

/* TIM1 three-phase PWM */
#define PIN_PWM_UH      GPIOA, 8
#define PIN_PWM_UL      GPIOA, 7
#define PIN_PWM_VH      GPIOB, 0
#define PIN_PWM_VL      GPIOB, 1
#define PIN_PWM_WH      GPIOB, 4
#define PIN_PWM_WL      GPIOB, 5

/* ADC + op-amp (G431 has internal OPAMP; final channel mapping per CubeMX project) */
#define PIN_ADC_IU      GPIOA, 0
#define PIN_ADC_IV      GPIOA, 1
#define PIN_ADC_VBUS    GPIOA, 2
#define PIN_ADC_NTC     GPIOB, 14

#define PIN_CAN_TX      GPIOB, 9
#define PIN_CAN_RX      GPIOB, 8
#define PIN_PWM_IN      GPIOA, 6
#define PIN_DRV_CS      GPIOB, 12
#define PIN_DRV_FAULT   GPIOB, 11

#endif
