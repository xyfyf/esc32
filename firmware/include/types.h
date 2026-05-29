#ifndef ESC_TYPES_H
#define ESC_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#ifndef ESC_CURVE_POINTS
#define ESC_CURVE_POINTS 21
#endif

typedef struct {
    float kp;
    float ki;
    float kd;
    float integral;
    float out_min;
    float out_max;
} esc_pid_t;

#endif
