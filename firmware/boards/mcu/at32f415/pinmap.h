/**
 * @file pinmap.h
 * @brief ESC-80 成本档 + AT32F415（引脚待原理图定稿）
 */
#ifndef ESC_AT32F415_PINMAP_H
#define ESC_AT32F415_PINMAP_H

#define PIN_PWM_UH      GPIOA, 8
#define PIN_PWM_UL      GPIOB, 13
#define PIN_PWM_VH      GPIOA, 9
#define PIN_PWM_VL      GPIOB, 14
#define PIN_PWM_WH      GPIOA, 10
#define PIN_PWM_WL      GPIOB, 15

#define PIN_ADC_IU      GPIOA, 0
#define PIN_ADC_IV      GPIOA, 1
#define PIN_ADC_VBUS    GPIOC, 0
#define PIN_ADC_NTC     GPIOC, 3

#define PIN_CAN_TX      GPIOB, 9
#define PIN_CAN_RX      GPIOB, 8
#define PIN_PWM_IN      GPIOA, 6
#define PIN_DRV_CS      GPIOB, 12
#define PIN_DRV_FAULT   GPIOB, 11

#endif
