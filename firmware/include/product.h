/**
 * @file product.h
 * @brief 电调产品系列（功率档），与 MCU / Target 解耦
 *
 * 命名：ESC-{60|80|120|200}
 * 编译：-DESC_PRODUCT_ID=0x80  或  -DESC_BOARD_ID=0x80（兼容旧宏）
 */
#ifndef ESC_PRODUCT_H
#define ESC_PRODUCT_H

#include <stdint.h>

#define ESC_PRODUCT_ESC60   0x0060u
#define ESC_PRODUCT_ESC80   0x0080u
#define ESC_PRODUCT_ESC120  0x0120u
#define ESC_PRODUCT_ESC200  0x0200u

/* 兼容旧名称 */
#define BOARD_ID_ESC60   ESC_PRODUCT_ESC60
#define BOARD_ID_ESC80   ESC_PRODUCT_ESC80
#define BOARD_ID_ESC120  ESC_PRODUCT_ESC120

#ifndef ESC_PRODUCT_ID
#ifdef ESC_BOARD_ID
#define ESC_PRODUCT_ID ESC_BOARD_ID
#else
#define ESC_PRODUCT_ID ESC_PRODUCT_ESC80
#endif
#endif

#ifndef ESC_BOARD_ID
#define ESC_BOARD_ID ESC_PRODUCT_ID
#endif

typedef struct {
    uint16_t product_id;
    uint16_t hw_revision;   /* PCB 硬件修订，如 0x0001 */
    float    ibus_cont_a;
    float    ibus_peak_a;
    float    vbus_max_v;
    const char *sku;        /* 如 "ESC-80" */
} esc_product_profile_t;

const esc_product_profile_t *esc_product_profile(void);

#endif
