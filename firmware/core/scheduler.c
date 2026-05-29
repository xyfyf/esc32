#include "scheduler.h"
#include <string.h>

static esc_task_t *s_tasks[ESC_MAX_TASKS];
static int s_count;

void scheduler_init(void)
{
    memset(s_tasks, 0, sizeof(s_tasks));
    s_count = 0;
}

int scheduler_register(esc_task_t *task)
{
    if (s_count >= ESC_MAX_TASKS || task == NULL) {
        return -1;
    }
    s_tasks[s_count++] = task;
    task->last_run_us = 0;
    return 0;
}

void scheduler_run(uint32_t now_us)
{
    for (int i = 0; i < s_count; i++) {
        esc_task_t *t = s_tasks[i];
        if (!t->enabled || t->period_us == 0) {
            continue;
        }
        if (now_us - t->last_run_us >= t->period_us) {
            t->last_run_us = now_us;
            t->fn(t->ctx);
        }
    }
}
