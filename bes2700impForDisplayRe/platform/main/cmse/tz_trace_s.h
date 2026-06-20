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
#ifndef __TZ_TRACE_S_H__
#define __TZ_TRACE_S_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NS_CALL                             __attribute__((cmse_nonsecure_call))

typedef NS_CALL HAL_TRACE_APP_NOTIFY_T      NS_TRACE_APP_NOTIFY_T;
typedef NS_CALL HAL_TRACE_APP_OUTPUT_T      NS_TRACE_APP_OUTPUT_T;

void cmse_trace_init(void);
void cmse_set_ns_start_flag(int started);

#ifdef __cplusplus
}
#endif

#endif
