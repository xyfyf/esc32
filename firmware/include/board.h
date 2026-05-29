#ifndef ESC_BOARD_H
#define ESC_BOARD_H

/**
 * @deprecated Use product.h / target.h instead; this header is kept for backward compatibility.
 */
#include "product.h"
#include "target.h"
#include "params.h"

typedef esc_product_profile_t board_profile_t;

static inline const board_profile_t *board_profile(void)
{
    return esc_product_profile();
}

void board_apply_limits(esc_params_t *p);
void board_load_defaults(esc_params_t *p);

#endif
