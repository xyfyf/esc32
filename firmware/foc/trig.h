#ifndef ESC_TRIG_H
#define ESC_TRIG_H

#include <stdint.h>

#ifndef ESC_PI
#define ESC_PI 3.14159265358979323846f
#endif

#define ESC_CLAMP(x, lo, hi) \
    ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

float sin_fast(float rad);
float cos_fast(float rad);

#endif
