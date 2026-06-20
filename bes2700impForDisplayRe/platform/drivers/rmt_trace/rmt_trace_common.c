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
#include "rmt_trace_msg.h"

#ifdef RMT_TRC_IN_MSG_CHAN
int rmt_trace_check_trace(const void *data, unsigned int len)
{
    const struct RMT_TRC_MSG_T *trc_msg;
    uint32_t magic;

    if (data && len == sizeof(*trc_msg)) {
        trc_msg = (const struct RMT_TRC_MSG_T *)data;
        if (((uint32_t)&trc_msg->msg_magic & 0x3) == 0) {
            magic = trc_msg->msg_magic;
            if (magic == RMT_TRC_MSG_MAGIC_WORD) {
                return true;
            }
        }
    }

    return false;
}
#endif
