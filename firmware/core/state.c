#include "state.h"
#include <string.h>

void state_init(esc_runtime_t *rt)
{
    memset(rt, 0, sizeof(*rt));
    rt->state = ESC_STATE_DISARMED;
    rt->latched_fault = ESC_FAULT_NONE;
    rt->input_source = ESC_INPUT_NONE;
}

void state_set_fault(esc_runtime_t *rt, fault_code_t fault)
{
    rt->latched_fault = fault;
    rt->state = ESC_STATE_FAULT;
}
