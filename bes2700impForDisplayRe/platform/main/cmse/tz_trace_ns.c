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

#include "cmsis.h"
#include "string.h"
#include "hal_trace.h"
#include "tz_trace_ns.h"
#ifdef USE_TRACE_ID
#include "hal_trace_id.h"
#endif

static uint8_t cmns_trc_buf[200];

static int cmns_trace_global_tag_handler(char *buf, unsigned int buf_len)
{
#ifdef USE_TRACE_ID
    uint32_t id = hal_trace_get_id(buf);
    hal_trace_set_id(buf, SET_BITFIELD(id, USER_ID_SECURE, USER_ID_SECURE_NS));
    return (int)id;
#else
    unsigned int len;
    const char tag[] = "NS/";
    const unsigned int tag_len = sizeof(tag) - 1;

    if (buf_len) {
        len = (buf_len <= tag_len) ? buf_len : tag_len;
        memcpy(buf, tag, len);
        return len;
    }

    return 0;
#endif
}

static void cmns_trace_notify_handler(enum HAL_TRACE_STATE_T state)
{
    // disable non-security irq
    int_lock_global();

    if (state == HAL_TRACE_STATE_CRASH_END) {
        TRACE_IMM(TR_ATTR_NO_TS, " ");
        TRACE_IMM(TR_ATTR_NO_TS, "------------------------------------------");
        TRACE_IMM(TR_ATTR_NO_TS, "Start of non-secure application crash dump");
        TRACE_IMM(TR_ATTR_NO_TS, "------------------------------------------");
        TRACE_IMM(TR_ATTR_NO_TS, " ");

        hal_trace_crash_dump_callback();
        TRACE_FLUSH();

        TRACE_IMM(0, " ");
        TRACE_IMM(0, "----------------------------------------");
        TRACE_IMM(0, "End of non-secure application crash dump");
        TRACE_IMM(0, "----------------------------------------");
        TRACE_IMM(0, " ");
    }

#if !(defined(ROM_BUILD) || defined(PROGRAMMER))
#ifdef DUMP_CRASH_LOG
    hal_trace_app_notify_callback(state);
#endif
#endif
}

static void cmns_trace_output_handler(const unsigned char *buf, unsigned int len)
{
    TRACE_OUTPUT(buf, len);
    TRACE_FLUSH();
}

void cmns_trace_init()
{
    hal_trace_global_tag_register(cmns_trace_global_tag_handler);
    cmse_register_ns_trace_callback(cmns_trc_buf, sizeof(cmns_trc_buf), cmns_trace_notify_handler, cmns_trace_output_handler);

}
