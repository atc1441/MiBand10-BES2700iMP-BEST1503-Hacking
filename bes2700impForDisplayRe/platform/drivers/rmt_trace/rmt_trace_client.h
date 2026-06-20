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
#ifndef __RMT_TRACE_CLIENT_H__
#define __RMT_TRACE_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "rmt_trace_msg.h"

typedef int (*RMT_TRC_TX_ACTIVE_CB)(unsigned int seq);
typedef void (*RMT_TRC_TX_IRQ_RUN_CB)(void);

struct RMT_TRC_CLIENT_CFG_T {
    const char *tag;

    RMT_TRC_SEND_CB send_cb;
    RMT_TRC_TX_ACTIVE_CB tx_active_cb;
    RMT_TRC_TX_IRQ_RUN_CB tx_irq_run_cb;
};

int rmt_trace_client_open(const struct RMT_TRC_CLIENT_CFG_T *cfg);

unsigned int rmt_trace_client_msg_handler(const void *data, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif

