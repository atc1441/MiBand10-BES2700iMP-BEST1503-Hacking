/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef __SOURCE_COMMON_DEFINE_H__
#define __SOURCE_COMMON_DEFINE_H__
#include "bt_common_define.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * common define
 */

#define A2DP_SOURCE_NON_TYPE_NON         0
#define A2DP_SOURCE_NON_TYPE_LHDC        1
#define A2DP_SOURCE_NON_TYPE_LHDCV5      2
#define A2DP_SOURCE_NON_TYPE_LDAC        3

typedef enum bt_source_event {
    BT_SOURCE_EVENT_SEARCH_RESULT = 0, // one result with information
    BT_SOURCE_EVENT_SEARCH_COMPLETE, // complete with status
    BT_SOURCE_EVENT_A2DP_SOURCE_CONNECT_FAIL,
    BT_SOURCE_EVENT_A2DP_SOURCE_STREAM_OPEN,
    BT_SOURCE_EVENT_A2DP_SOURCE_STREAM_CLOSE,
} bt_source_event_t;

typedef struct bt_source_event_param_t bt_source_event_param_t;

typedef struct bt_source_event_param_t {
    union {
        struct {
            app_bt_search_result_t *result;
        } search_result;
        struct {
            char status;
        } search_complete;
        struct {
            bt_bdaddr_t *addr;
        } a2dp_source_connect_fail;
        struct {
            bt_bdaddr_t *addr;
            int device_id;
        } a2dp_source_stream_open;
        struct {
            bt_bdaddr_t *addr;
            int device_id;
        } a2dp_source_stream_close;
    } p;
} bt_source_event_param_t;

typedef int (*bt_source_callback_t)(bt_source_event_t event, bt_source_event_param_t *param);

#ifdef __cplusplus
}
#endif
#endif /* __SOURCE_COMMON_DEFINE_H__ */
