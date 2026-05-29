/**
 * @file pinmap.h
 * @brief ESC-120 STM32H743 pin plan (pending schematic finalization)
 */
#ifndef ESC_STM32H743_PINMAP_H
#define ESC_STM32H743_PINMAP_H

/* TIM1 three-phase PWM — final assignment per actual PCB */
#define PIN_PWM_UH      GPIOE, 9
#define PIN_PWM_UL      GPIOE, 8
#define PIN_PWM_VH      GPIOE, 11
#define PIN_PWM_VL      GPIOE, 10
#define PIN_PWM_WH      GPIOE, 13
#define PIN_PWM_WL      GPIOE, 12

/* ADC phase current / bus voltage */
#define PIN_ADC_IU      GPIOA, 0
#define PIN_ADC_IV      GPIOA, 1
#define PIN_ADC_VBUS    GPIOC, 0
#define PIN_ADC_NTC     GPIOC, 3

/* FDCAN */
#define PIN_CAN_TX      GPIOD, 1
#define PIN_CAN_RX      GPIOD, 0

/* PWM throttle */
#define PIN_PWM_IN      GPIOA, 6

/* Gate driver SPI */
#define PIN_DRV_CS      GPIOB, 12
#define PIN_DRV_FAULT   GPIOB, 11

#endif
