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
#ifndef __RMT_TRACE_SERVER_H__
#define __RMT_TRACE_SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"
#include "stdint.h"
#include "rmt_trace_msg.h"

typedef void (*RMT_TRC_SERVER_DUMP_CB)(void);

struct RMT_TRC_SERVER_CFG_T {
    const char *name;
    const uint8_t *buf_start;
    const uint16_t *entry_start;
    uint16_t buf_total_size;
    uint16_t entry_total_cnt;
    bool first_msg;
    bool in_msg_chan;
    RMT_TRC_SEND_CB send_cb;
    struct RMT_TRC_CTRL_T *ctrl;
};

unsigned int rmt_trace_server_msg_handler(struct RMT_TRC_SERVER_CFG_T *cfg, const void *data, unsigned int len);

void rmt_trace_server_dump(struct RMT_TRC_SERVER_CFG_T *cfg);

int rmt_trace_server_dump_register(RMT_TRC_SERVER_DUMP_CB cb);

int rmt_trace_server_dump_deregister(RMT_TRC_SERVER_DUMP_CB cb);

#ifdef __cplusplus
}
#endif

#endif
