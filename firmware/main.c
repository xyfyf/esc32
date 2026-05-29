/**
 * @file main.c
 */
#include "app.h"
#include "hal.h"
#include <stdio.h>

#ifdef ESC_PLATFORM_SIM
int main(void)
{
    printf("esc32 simulator (FOC framework)\n");
    fflush(stdout);
    app_init();
    while (1) {
        app_run_once();
        hal_delay_ms(1);
    }
    return 0;
}
#else
int main(void)
{
    app_init();
    while (1) {
        app_run_once();
    }
    return 0;
}
#endif
