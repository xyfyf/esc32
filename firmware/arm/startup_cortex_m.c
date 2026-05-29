/**
 * @file startup_cortex_m.c
 * @brief Minimal Cortex-M startup code shared by all ARM targets.
 *
 * Provides:
 *   - Vector table with the first 16 system handlers (sufficient for link).
 *     Peripheral IRQs default to a weak common handler; the real HAL on a
 *     CubeMX project will override them.
 *   - Reset_Handler that zeroes .bss, copies .data, then jumps to main().
 *   - A tiny SystemInit() stub that the HAL stub leaves untouched.
 *
 * This file is shared by every Cortex-M{0,3,4,7} target. On the real
 * hardware bring-up, swap in the vendor-supplied startup_<dev>.s and
 * keep this file out of the build.
 */

#include <stdint.h>

extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _estack;

extern int main(void);

void __attribute__((weak)) SystemInit(void) {}

void Default_Handler(void)
{
    while (1) {
    }
}

void NMI_Handler(void)         __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)    __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)         __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)    __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)      __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)     __attribute__((weak, alias("Default_Handler")));

void Reset_Handler(void)
{
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }
    for (dst = &_sbss; dst < &_ebss; ) {
        *dst++ = 0;
    }
    SystemInit();
    (void)main();
    while (1) {
    }
}

__attribute__((section(".isr_vector")))
const void * const g_pfnVectors[] = {
    (void *)&_estack,
    (void *)Reset_Handler,
    (void *)NMI_Handler,
    (void *)HardFault_Handler,
    (void *)MemManage_Handler,
    (void *)BusFault_Handler,
    (void *)UsageFault_Handler,
    0, 0, 0, 0,
    (void *)SVC_Handler,
    (void *)DebugMon_Handler,
    0,
    (void *)PendSV_Handler,
    (void *)SysTick_Handler,
};
