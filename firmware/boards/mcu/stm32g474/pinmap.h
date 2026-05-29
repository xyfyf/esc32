/**
 * @file pinmap.h
 * @brief ESC-80 STM32G474 pin mapping (matches docs/hardware/STM32G474引脚与接口.md)
 */
#ifndef ESC_STM32G474_PINMAP_H
#define ESC_STM32G474_PINMAP_H

/* TIM1 PWM */
#define PIN_PWM_UH      GPIOA, 8
#define PIN_PWM_UL      GPIOB, 13
#define PIN_PWM_VH      GPIOA, 9
#define PIN_PWM_VL      GPIOB, 14
#define PIN_PWM_WH      GPIOA, 10
#define PIN_PWM_WL      GPIOB, 15

/* ADC */
#define PIN_ADC_IU      GPIOA, 0
#define PIN_ADC_IV      GPIOA, 1
#define PIN_ADC_IW      GPIOA, 2
#define PIN_ADC_VBUS    GPIOC, 0
#define PIN_ADC_NTC     GPIOC, 3

/* CAN */
#define PIN_CAN_TX      GPIOB, 9
#define PIN_CAN_RX      GPIOB, 8

/* UART debug */
#define PIN_UART_TX     GPIOA, 2
#define PIN_UART_RX     GPIOA, 3

/* PWM throttle input */
#define PIN_PWM_IN      GPIOA, 6

/* DRV8323 */
#define PIN_DRV_CS      GPIOB, 12
#define PIN_DRV_FAULT   GPIOB, 11

#endif
