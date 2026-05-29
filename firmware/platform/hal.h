#ifndef ESC_HAL_H
#define ESC_HAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void hal_init(void);
uint32_t hal_time_us(void);
uint32_t hal_millis(void);
void hal_delay_ms(uint32_t ms);

void hal_pwm_set(float duty_a, float duty_b, float duty_c);
void hal_adc_read(float *vbus, float *ibus, float *ia, float *ib, float *ic,
                  float *temp_mos);

uint16_t hal_pwm_input_us(void);
void hal_pwm_input_simulate(uint16_t us);

int hal_uart_write(const uint8_t *data, size_t len);
int hal_uart_read(uint8_t *data, size_t cap);

/* CAN / DroneCAN */
void hal_can_init(uint32_t baudrate);
int hal_can_send(uint32_t can_id, const uint8_t *data, uint8_t len);
int hal_can_receive(uint32_t *can_id, uint8_t *data, uint8_t *len, uint8_t cap);

/* NVM: simulation writes to a file; target MCU writes to internal Flash */
int hal_nvm_read(uint32_t offset, void *buf, size_t len);
int hal_nvm_write(uint32_t offset, const void *buf, size_t len);

bool hal_gpio_boot_pin_active(void);
void hal_system_reset(void);

#ifdef ESC_PLATFORM_SIM
int hal_udp_send(const uint8_t *data, size_t len);
int hal_udp_poll(uint8_t *buf, size_t cap);
#endif

#endif
