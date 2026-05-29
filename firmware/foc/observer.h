#ifndef ESC_OBSERVER_H
#define ESC_OBSERVER_H

#include <stdint.h>
#include "params.h"

typedef enum {
    OBSERVER_PLL = 0,
    OBSERVER_SMO = 1,
} observer_type_t;

typedef struct {
    observer_type_t type;
    float kp_pll;
    float ki_pll;
    float theta;
    float omega;
    float filter_hz;
    /* SMO 状态 */
    float alpha_est;
    float beta_est;
    float lambda_smo;
    float rs;
    float ls;
} observer_t;

void observer_init(observer_t *obs, observer_type_t type, const esc_params_t *p);
void observer_update(observer_t *obs, float v_alpha, float v_beta,
                     float i_alpha, float i_beta, float vbus, float dt);

#endif
