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
#ifndef __BT_A2DP_SERVICE_H__
#define __BT_A2DP_SERVICE_H__
#include "adapter_service.h"
#include "a2dp_common_define.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * standard a2dp (sink role) interface
 *
 */

typedef struct {
    uint8_t error_code;
    uint8_t codec_type;
    uint8_t codec_info_len;
    uint8_t *codec_info;
    uint8_t *cp_info;
    uint8_t cp_info_len;
    uint8_t cp_type;
} bt_a2dp_opened_param_t;

typedef struct {
    uint8_t error_code;
} bt_a2dp_closed_param_t;

typedef struct {
    uint8_t error_code;
    uint8_t codec_type;
    uint8_t codec_info_len;
    uint8_t *codec_info;
    uint8_t *cp_info;
    uint8_t cp_info_len;
    uint16_t cp_type;
} bt_a2dp_stream_start_param_t;

typedef struct {
    uint8_t error_code;
    uint8_t codec_type;
    uint8_t codec_info_len;
    uint8_t *codec_info;
    uint8_t *cp_info;
    uint8_t cp_info_len;
    uint16_t cp_type;
} bt_a2dp_stream_reconfig_param_t;

typedef struct {
    uint8_t error_code;
} bt_a2dp_stream_suspend_param_t;

typedef struct {
    uint8_t error_code;
} bt_a2dp_stream_close_param_t;

typedef struct {
    uint8_t *buf;
    uint16_t len;
} bt_a2dp_stream_data_param_t;

typedef struct {
    uint8_t message_type : 2;
    uint8_t packet_type : 2;
    uint8_t transaction : 4;
    uint8_t signal_id : 6;
    uint8_t reserve : 2;
} bt_a2dp_signal_msg_header_t;

typedef struct {
    uint8_t trans_lable;
    uint8_t cmd_id;
    uint16_t data_len;
    uint8_t *cmd_data;
} bt_a2dp_custom_cmd_req_param_t;

typedef struct {
    bool accepted;
    uint8_t cmd_id;
    uint16_t data_len;
    uint8_t *cmd_data;
} bt_a2dp_custom_cmd_rsp_param_t;

typedef union {
    bt_a2dp_opened_param_t *opened;
    bt_a2dp_closed_param_t *closed;
    bt_a2dp_stream_start_param_t *stream_start;
    bt_a2dp_stream_reconfig_param_t *stream_reconfig;
    bt_a2dp_stream_suspend_param_t *stream_suspend;
    bt_a2dp_stream_close_param_t *stream_close;
    bt_a2dp_stream_data_param_t *stream_data;
    bt_a2dp_custom_cmd_req_param_t *custom_cmd_req;
    bt_a2dp_custom_cmd_rsp_param_t *custom_cmd_rsp;
} bt_a2dp_callback_param_t;

typedef enum {
    BT_A2DP_EVENT_OPENED = BT_EVENT_A2DP_OPENED,
    BT_A2DP_EVENT_CLOSED,
    BT_A2DP_EVENT_STREAM_START,
    BT_A2DP_EVENT_STREAM_RECONFIG,
    BT_A2DP_EVENT_STREAM_SUSPEND,
    BT_A2DP_EVENT_STREAM_CLOSE,
    BT_A2DP_EVENT_STREAM_DATA_IND,
    BT_A2DP_EVENT_CUSTOM_CMD_REQ,
    BT_A2DP_EVENT_CUSTOM_CMD_RSP,
    BT_A2DP_EVENT_END,
} bt_a2dp_event_t;

#if BT_A2DP_EVENT_END != BT_EVENT_A2DP_END
#error "bt_a2dp_event_t error define"
#endif

typedef int (*bt_a2dp_callback_t)(const bt_bdaddr_t *bd_addr, bt_a2dp_event_t event, bt_a2dp_callback_param_t param);

bt_status_t bt_a2dp_init(bt_a2dp_callback_t callback);
bt_status_t bt_a2dp_cleanup(void);
bt_status_t bt_a2dp_connect(const bt_bdaddr_t *bd_addr);
bt_status_t bt_a2dp_disconnect(const bt_bdaddr_t *bd_addr);
bt_status_t bt_a2dp_accept_custom_cmd(const bt_bdaddr_t *bd_addr, const bt_a2dp_custom_cmd_req_param_t *cmd, bool accept);
bt_status_t bt_a2dp_send_custom_cmd(const bt_bdaddr_t *bd_addr, uint8_t custom_cmd_id, const uint8_t *data, uint16_t len);

/**
 * standard a2dp (source role) interface
 *
 */

#ifdef BT_SOURCE

typedef enum {
    BT_A2DP_SOURCE_EVENT_OPENED = BT_EVENT_A2DP_SOURCE_OPENED,
    BT_A2DP_SOURCE_EVENT_CLOSED,
    BT_A2DP_SOURCE_EVENT_STREAM_START,
    BT_A2DP_SOURCE_EVENT_STREAM_RECONFIG,
    BT_A2DP_SOURCE_EVENT_STREAM_SUSPEND,
    BT_A2DP_SOURCE_EVENT_STREAM_CLOSE,
    BT_A2DP_SOURCE_EVENT_END,
} bt_a2dp_source_event_t;

#if BT_A2DP_SOURCE_EVENT_END != BT_EVENT_A2DP_SOURCE_END
#error "bt_a2dp_source_callback_t error define"
#endif

typedef int (*bt_a2dp_source_callback_t)(const bt_bdaddr_t *bd_addr, bt_a2dp_source_event_t event, bt_a2dp_callback_param_t param);

bt_status_t bt_a2dp_source_init(bt_a2dp_source_callback_t callback);
bt_status_t bt_a2dp_source_cleanup(void);
bt_status_t bt_a2dp_source_connect(const bt_bdaddr_t *bd_addr);
bt_status_t bt_a2dp_source_disconnect(const bt_bdaddr_t *bd_addr);
bt_status_t bt_a2dp_source_start_stream(const bt_bdaddr_t *bd_addr);
bt_status_t bt_a2dp_source_suspend_stream(const bt_bdaddr_t *bd_addr);

#endif /* BT_SOURCE */
#ifdef __cplusplus
}
#endif
#endif /* __BT_A2DP_SERVICE_H__ */

