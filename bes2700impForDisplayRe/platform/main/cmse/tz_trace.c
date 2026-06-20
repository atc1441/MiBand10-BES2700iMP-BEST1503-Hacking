/***************************************************************************
 *
 * Copyright 2015-2021 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#include "plat_types.h"
#include "arm_cmse.h"
#include "hal_location.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_uart.h"
#include "string.h"
#include "tz_trace_s.h"
#include "tz_trace_ns.h"
#ifdef USE_TRACE_ID
#include "hal_trace_id.h"
#endif

#ifdef DEBUG
static uint8_t *ns_trc_buf;
static uint32_t ns_trc_buf_len;
static NS_TRACE_APP_NOTIFY_T ns_trc_notif_cb;
static NS_TRACE_APP_OUTPUT_T ns_trc_output_cb;
static bool ns_started;

static int cmse_trace_global_tag_handler(char *buf, unsigned int buf_len)
{
#ifdef USE_TRACE_ID
    uint32_t id = hal_trace_get_id(buf);
    hal_trace_set_id(buf, SET_BITFIELD(id, USER_ID_SECURE, USER_ID_SECURE_SE));
    return (int)id;
#else
    unsigned int len;
    const char tag[] = "SE/";
    const unsigned int tag_len = sizeof(tag) - 1;

    if (buf_len) {
        len = (buf_len <= tag_len) ? buf_len : tag_len;
        memcpy(buf, tag, len);
        return len;
    }

    return 0;
#endif
}

static void cmse_trace_notify_handler(enum HAL_TRACE_STATE_T state)
{
    if (ns_trc_notif_cb) {
        ns_trc_notif_cb(state);
    }
}

static void cmse_trace_output_handler(const unsigned char *buf, unsigned int len)
{
    if (!ns_started) {
        hal_uart_printf_output(buf, len);
        return;
    }

    if (ns_trc_buf && ns_trc_buf_len && ns_trc_output_cb) {
        if (len > ns_trc_buf_len) {
            len = ns_trc_buf_len;
        }
        memcpy(ns_trc_buf, buf, len);
        ns_trc_output_cb(ns_trc_buf, len);
    }
}

void cmse_trace_init(void)
{
    hal_uart_printf_init();
    hal_trace_global_tag_register(cmse_trace_global_tag_handler);
    hal_trace_app_register(HAL_TRACE_APP_REG_ID_0, cmse_trace_notify_handler, cmse_trace_output_handler);
}

void cmse_set_ns_start_flag(int started)
{
    if (started) {
        while (hal_uart_printf_busy());
    }
    ns_started = !!started;
}
#endif

__attribute__((cmse_nonsecure_entry))
int cmse_register_ns_trace_callback(uint8_t *buf, uint32_t len, HAL_TRACE_APP_NOTIFY_T notif_cb, HAL_TRACE_APP_OUTPUT_T out_cb)
{
#ifdef DEBUG
    buf = cmse_check_address_range(buf, len, CMSE_NONSECURE);
    if (!buf) {
        return 1;
    }
    ns_trc_buf = buf;
    ns_trc_buf_len = len;
    ns_trc_notif_cb = (NS_TRACE_APP_NOTIFY_T)cmse_nsfptr_create(notif_cb);
    ns_trc_output_cb = (NS_TRACE_APP_OUTPUT_T)cmse_nsfptr_create(out_cb);
#endif
    return 0;
}
