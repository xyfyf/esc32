/**
 * @file comm.c
 */
#include "comm.h"
#include "../../shared/protocol/protocol.h"
#include "hal.h"
#include "version.h"
#include "params.h"
#include "app.h"
#include "board.h"
#include "target_meta.h"
#include "mcu_port.h"
#include "features.h"
#include "fault_log.h"
#include "boot.h"
#include <string.h>
#include <stdio.h>

static uint8_t s_tx[ESC_PROTO_HEADER_SIZE + ESC_PROTO_MAX_PAYLOAD + ESC_PROTO_CRC_SIZE];

static void comm_reply(uint8_t cmd, const void *payload, uint16_t len)
{
    size_t n = frame_pack(cmd, (const uint8_t *)payload, len, s_tx, sizeof(s_tx));
    if (n > 0) {
        hal_uart_write(s_tx, n);
    }
}

static int on_frame(uint8_t cmd, const uint8_t *payload, uint16_t len, void *user)
{
    (void)user;
    if (boot_should_stay_in_bl()) {
        uint8_t brsp[16];
        uint16_t blen = 0;
        if (boot_process_cmd(cmd, payload, len, brsp, &blen) && blen > 0) {
            comm_reply(cmd, brsp, blen);
        }
        return 1;
    }

    switch (cmd) {
    case ESC_CMD_PING: {
        uint8_t pong = ESC_PROTO_VERSION;
        comm_reply(ESC_CMD_PING, &pong, 1);
        break;
    }
    case ESC_CMD_GET_INFO: {
        const esc_target_meta_t *tm = esc_target_meta();
        esc_info_rsp_t info;
        memset(&info, 0, sizeof(info));
        info.proto_version = ESC_PROTO_VERSION;
        info.mcu_id = tm->mcu_id;
        info.product_id = app_board_id();
        info.fw_version = ESC_FW_VERSION_U16;
        info.target_id = tm->target_id;
        info.hw_revision = tm->hw_revision;
        strncpy(info.name, tm->firmware_name, sizeof(info.name) - 1);
        snprintf(info.build_date, sizeof(info.build_date), "%.11s", __DATE__);
        info.feature_flags = ESC_INFO_FEAT_UAVCAN;
#if ESC_FEATURE_MOTOR_BEEP
        if (g_params.motor_sound_enable) {
            info.feature_flags |= ESC_INFO_FEAT_MOTOR_BEEP;
        }
#endif
        {
            const esc_mcu_caps_t *cap = esc_mcu_capabilities();
            if (cap->port_status[0] == 'r') {
                info.port_status = ESC_INFO_PORT_READY;
            } else if (cap->port_status[0] == 's') {
                info.port_status = ESC_INFO_PORT_STUB;
            } else {
                info.port_status = ESC_INFO_PORT_PLANNED;
            }
        }
        comm_reply(ESC_CMD_GET_INFO, &info, sizeof(info));
        break;
    }
    case ESC_CMD_GET_TELEM: {
        esc_telem_rsp_t t;
        app_fill_telem(&t);
        comm_reply(ESC_CMD_GET_TELEM, &t, sizeof(t));
        break;
    }
    case ESC_CMD_GET_PARAM: {
        if (len < 2) {
            break;
        }
        uint16_t idx = (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);
        const param_desc_t *d = params_desc_get(idx);
        if (!d) {
            break;
        }
        float val;
        if (params_get_by_name(&g_params, d->name, &val) != 0) {
            break;
        }
        uint8_t rsp[38];
        rsp[0] = (uint8_t)(idx & 0xFF);
        rsp[1] = (uint8_t)((idx >> 8) & 0xFF);
        memcpy(&rsp[2], &val, sizeof(val));
        memset(&rsp[6], 0, 32);
        strncpy((char *)&rsp[6], d->name, 31);
        comm_reply(ESC_CMD_GET_PARAM, rsp, sizeof(rsp));
        break;
    }
    case ESC_CMD_SET_PARAM: {
        uint8_t err = (uint8_t)ESC_ERR_OK;
        if (len >= 4 && len < 36) {
            uint16_t idx = (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);
            uint16_t v16 = 0;
            float val = 0.0f;
            const param_desc_t *d = params_desc_get(idx);
            if (!d) {
                err = (uint8_t)ESC_ERR_BAD_CMD;
            } else if (len >= 6) {
                memcpy(&val, &payload[2], sizeof(val));
            } else if (len >= 4) {
                v16 = (uint16_t)payload[2] | ((uint16_t)payload[3] << 8);
                val = (float)v16;
            }
            if (d && err == ESC_ERR_OK) {
                int pr = 0;
                switch (d->type) {
                case ESC_PARAM_TYPE_U8:
                    *(uint8_t *)((uint8_t *)&g_params + d->offset) = (uint8_t)v16;
                    g_params.crc16 = params_crc(&g_params);
                    break;
                case ESC_PARAM_TYPE_U16:
                    if (strcmp(d->name, "motor_kv") == 0) {
                        g_params.motor_kv = v16;
                    } else if (strcmp(d->name, "motor_pole_pairs") == 0) {
                        g_params.motor_pole_pairs = v16;
                    } else if (strcmp(d->name, "node_id") == 0) {
                        g_params.node_id = (uint8_t)v16;
                    } else {
                        *(uint16_t *)((uint8_t *)&g_params + d->offset) = v16;
                    }
                    g_params.crc16 = params_crc(&g_params);
                    break;
                default:
                    pr = params_set_by_name(&g_params, d->name, val);
                    break;
                }
                if (pr != 0) {
                    err = (uint8_t)ESC_ERR_PARAM_RANGE;
                } else {
                    board_apply_limits(&g_params);
                }
            }
        } else if (len >= 36) {
            char name[32];
            memcpy(name, payload, 31);
            name[31] = '\0';
            float val;
            memcpy(&val, &payload[32], sizeof(val));
            if (params_set_by_name(&g_params, name, val) != 0) {
                err = (uint8_t)ESC_ERR_PARAM_RANGE;
            } else {
                board_apply_limits(&g_params);
            }
        } else {
            err = (uint8_t)ESC_ERR_BAD_CMD;
        }
        comm_reply(ESC_CMD_SET_PARAM, &err, 1);
        break;
    }
    case ESC_CMD_SAVE_PARAMS:
        params_save_flash(&g_params);
        comm_reply(ESC_CMD_SAVE_PARAMS, "\x00", 1);
        break;
    case ESC_CMD_LOAD_DEFAULTS:
        board_load_defaults(&g_params);
        comm_reply(ESC_CMD_LOAD_DEFAULTS, "\x00", 1);
        break;
    case ESC_CMD_ARM:
        app_arm();
        comm_reply(ESC_CMD_ARM, "\x00", 1);
        break;
    case ESC_CMD_DISARM:
        app_disarm();
        comm_reply(ESC_CMD_DISARM, "\x00", 1);
        break;
    case ESC_CMD_SET_THROTTLE:
        if (len >= 2) {
            uint16_t us = (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);
            app_set_throttle_us(us);
        }
        comm_reply(ESC_CMD_SET_THROTTLE, "\x00", 1);
        break;
    case ESC_CMD_GET_FAULT_LOG: {
        uint16_t off = 0;
        uint8_t max_n = 8;
        if (len >= 2) {
            off = (uint16_t)payload[0] | ((uint16_t)payload[1] << 8);
        }
        if (len >= 3) {
            max_n = payload[2];
            if (max_n > 8) {
                max_n = 8;
            }
        }
        uint8_t rsp[4 + 8 * sizeof(fault_log_entry_t)];
        esc_fault_log_rsp_hdr_t *hdr = (esc_fault_log_rsp_hdr_t *)rsp;
        hdr->total = fault_log_count();
        hdr->returned = fault_log_read(off, (fault_log_entry_t *)&rsp[4], max_n);
        comm_reply(ESC_CMD_GET_FAULT_LOG, rsp,
                   (uint16_t)(4 + hdr->returned * sizeof(fault_log_entry_t)));
        break;
    }
    case ESC_CMD_FW_ERASE:
    case ESC_CMD_FW_WRITE:
    case ESC_CMD_FW_CRC:
    case ESC_CMD_FW_REBOOT: {
        uint8_t brsp[8];
        uint16_t blen = 0;
        boot_process_cmd(cmd, payload, len, brsp, &blen);
        if (blen > 0) {
            comm_reply(cmd, brsp, blen);
        }
        break;
    }
    default:
        break;
    }
    return 1;
}

void comm_init(void)
{
    frame_reset();
    boot_init();
}

void comm_poll(void)
{
    uint8_t buf[512];
    int n = hal_uart_read(buf, sizeof(buf));
    for (int i = 0; i < n; i++) {
        frame_feed(buf[i], on_frame, NULL);
    }
}
