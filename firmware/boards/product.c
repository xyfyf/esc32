#include "product.h"
#include "params.h"
#include <string.h>

static const esc_product_profile_t s_products[] = {
    { ESC_PRODUCT_ESC60,  0x0001, 60.0f,  120.0f, 60.0f,  "ESC-60" },
    { ESC_PRODUCT_ESC80,  0x0001, 80.0f,  150.0f, 63.0f,  "ESC-80" },
    { ESC_PRODUCT_ESC120, 0x0001, 120.0f, 200.0f, 75.6f, "ESC-120" },
    { ESC_PRODUCT_ESC200, 0x0001, 200.0f, 300.0f, 75.6f, "ESC-200" },
};

const esc_product_profile_t *esc_product_profile(void)
{
    for (unsigned i = 0; i < sizeof(s_products) / sizeof(s_products[0]); i++) {
        if (s_products[i].product_id == ESC_PRODUCT_ID) {
            return &s_products[i];
        }
    }
    return &s_products[1];
}

void board_apply_limits(esc_params_t *p)
{
    const esc_product_profile_t *pp = esc_product_profile();
    if (p->ibus_max_current_a <= 0.0f || p->ibus_max_current_a > pp->ibus_cont_a) {
        p->ibus_max_current_a = pp->ibus_cont_a;
    }
    if (p->motor_max_current_a > pp->ibus_peak_a) {
        p->motor_max_current_a = pp->ibus_peak_a;
    }
    p->over_voltage_threshold_v = pp->vbus_max_v;
}

void board_load_defaults(esc_params_t *p)
{
    const esc_product_profile_t *pp = esc_product_profile();
    params_load_defaults(p, (uint8_t)(pp->product_id & 0xFFu));
    strncpy(p->config_name, pp->sku, ESC_CONFIG_NAME_LEN - 1);
    p->ibus_max_current_a = pp->ibus_cont_a;
    p->motor_max_current_a = pp->ibus_peak_a;
    board_apply_limits(p);
    p->crc16 = params_crc(p);
}
