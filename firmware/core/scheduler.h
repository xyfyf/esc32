#ifndef ESC_SCHEDULER_H
#define ESC_SCHEDULER_H

#include <stdint.h>

typedef void (*task_fn_t)(void *ctx);

typedef struct {
    const char   *name;
    task_fn_t fn;
    void         *ctx;
    uint32_t      period_us;
    uint32_t      last_run_us;
    uint8_t       enabled;
} esc_task_t;

#define ESC_MAX_TASKS 16

void scheduler_init(void);
int  scheduler_register(esc_task_t *task);
void scheduler_run(uint32_t now_us);

#endif
