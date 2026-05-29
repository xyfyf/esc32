#ifndef ESC_PLATFORM_SIM

#include "hal.h"
#include <string.h>

void hal_init(void) {}
uint32_t hal_time_us(void) { return 0; }
uint32_t hal_millis(void) { return 0; }
void hal_delay_ms(uint32_t ms) { (void)ms; }
void hal_pwm_set(float a, float b, float c) { (void)a; (void)b; (void)c; }
void hal_adc_read(float *vbus, float *ibus, float *ia, float *ib, float *ic,
                  float *temp_mos)
{
    *vbus = 0;
    *ibus = 0;
    *ia = 0;
    *ib = 0;
    *ic = 0;
    *temp_mos = 25;
}
uint16_t hal_pwm_input_us(void) { return 1000; }
void hal_pwm_input_simulate(uint16_t us) { (void)us; }
int hal_uart_write(const uint8_t *d, size_t n) { (void)d; (void)n; return 0; }
int hal_uart_read(uint8_t *d, size_t c) { (void)d; (void)c; return 0; }
void hal_can_init(uint32_t b) { (void)b; }
int hal_can_send(uint32_t id, const uint8_t *d, uint8_t l) { (void)id; (void)d; (void)l; return 0; }
int hal_can_receive(uint32_t *id, uint8_t *d, uint8_t *l, uint8_t c)
{
    (void)id;
    (void)d;
    (void)l;
    (void)c;
    return 0;
}
int hal_nvm_read(uint32_t o, void *b, size_t l) { (void)o; memset(b, 0xFF, l); return -1; }
int hal_nvm_write(uint32_t o, const void *b, size_t l) { (void)o; (void)b; (void)l; return 0; }
bool hal_gpio_boot_pin_active(void) { return false; }
void hal_system_reset(void) {}

#endif
