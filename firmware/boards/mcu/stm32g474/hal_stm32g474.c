/**
 * @file hal_stm32g474.c
 * @brief STM32G474 生产 HAL 骨架（CubeMX 生成后填入寄存器操作）
 *
 * 编译：mingw32-make target80-hal  （链接验证）
 * 真机：在 STM32CubeIDE 中合并本文件与 CubeMX 生成的 tim/adc/fdcan 初始化。
 */
#ifndef ESC_PLATFORM_SIM

#include "hal.h"
#include <string.h>

/* 引脚定义见 pinmap.h（真机编译时由 CubeMX 工程包含） */

/* TODO: #include "stm32g4xx_hal.h" */

static volatile uint32_t s_tick_ms;

void hal_init(void)
{
    s_tick_ms = 0;
    /* TODO: HAL_Init(); SystemClock_Config(); MX_GPIO_Init(); */
    /* TODO: MX_TIM1_Init(); MX_ADC1_Init(); MX_ADC2_Init(); MX_FDCAN1_Init(); */
    /* TODO: MX_TIM3_Init(); 输入捕获 PWM_IN */
    /* TODO: DRV8323 SPI 初始化 */
}

uint32_t hal_time_us(void)
{
    return s_tick_ms * 1000u;
}

uint32_t hal_millis(void)
{
    return s_tick_ms;
}

void hal_delay_ms(uint32_t ms)
{
    (void)ms;
    /* TODO: HAL_Delay(ms); */
}

void hal_pwm_set(float duty_a, float duty_b, float duty_c)
{
    (void)duty_a;
    (void)duty_b;
    (void)duty_c;
    /* TODO: __HAL_TIM_SET_COMPARE for TIM1 CH1/2/3 */
}

void hal_adc_read(float *vbus, float *ibus, float *ia, float *ib, float *ic,
                  float *temp_mos)
{
    *vbus = 0.0f;
    *ibus = 0.0f;
    *ia = 0.0f;
    *ib = 0.0f;
    *ic = 0.0f;
    *temp_mos = 25.0f;
    /* TODO: 注入采样 + 母线/NTC 换算 */
}

uint16_t hal_pwm_input_us(void)
{
    /* TODO: TIM3 输入捕获脉宽 µs */
    return 1000;
}

void hal_pwm_input_simulate(uint16_t us)
{
    (void)us;
}

int hal_uart_write(const uint8_t *data, size_t len)
{
    (void)data;
    (void)len;
    return 0;
}

int hal_uart_read(uint8_t *data, size_t cap)
{
    (void)data;
    (void)cap;
    return 0;
}

void hal_can_init(uint32_t baudrate)
{
    (void)baudrate;
    /* TODO: FDCAN1 1 Mbps, 过滤器接受扩展帧 */
}

int hal_can_send(uint32_t can_id, const uint8_t *data, uint8_t len)
{
    (void)can_id;
    (void)data;
    (void)len;
    return 0;
}

int hal_can_receive(uint32_t *can_id, uint8_t *data, uint8_t *len, uint8_t cap)
{
    (void)can_id;
    (void)data;
    (void)len;
    (void)cap;
    return 0;
}

int hal_nvm_read(uint32_t offset, void *buf, size_t len)
{
    (void)offset;
    memset(buf, 0xFF, len);
    return -1;
}

int hal_nvm_write(uint32_t offset, const void *buf, size_t len)
{
    (void)offset;
    (void)buf;
    (void)len;
    return -1;
}

bool hal_gpio_boot_pin_active(void)
{
    return false;
}

void hal_system_reset(void)
{
    /* TODO: NVIC_SystemReset(); */
}

#endif /* ESC_PLATFORM_SIM */
