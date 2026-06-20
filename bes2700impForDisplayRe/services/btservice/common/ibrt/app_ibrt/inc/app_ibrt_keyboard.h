/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifndef __APP_IBRT_KEYBOARD__
#define __APP_IBRT_KEYBOARD__
#include "app_key.h"
#include "bluetooth_bt_api.h"

#ifdef IBRT_SEARCH_UI
void app_ibrt_search_ui_handle_key_v2(bt_bdaddr_t *remote, APP_KEY_STATUS *status, void *param);
#else
void app_ibrt_normal_ui_handle_key_v2(bt_bdaddr_t *remote, APP_KEY_STATUS *status, void *param);
#endif

void app_ibrt_keyboard_request_handler_v2(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

enum SWITCH_A2DP_ERROR{
    SWITCH_A2DP_NO_ERROR,
    SWITCH_A2DP_NOW,
    SWITCH_A2DP_RSP_NO_ERROR,
    SWITCH_A2DP_RSP_ERROR,
};

enum TONGGLE_A2DP_CIS_ERROR{
    TOGGLE_A2DP_CIS_NO_ERROR,
    TOGGLE_A2DP_CIS_NOW,
    TOGGLE_A2DP_CIS_RSP_NO_ERROR,
    TOGGLE_A2DP_CIS_RSP_ERROR,
};

#define IBRT_ACTION_PLAY            0x01
#define IBRT_ACTION_PAUSE           0x02
#define IBRT_ACTION_FORWARD         0x03
#define IBRT_ACTION_BACKWARD        0x04
#define IBRT_ACTION_AVRCP_VOLUP     0x05
#define IBRT_ACTION_AVRCP_VOLDN     0x06
#define IBRT_ACTION_HFSCO_CREATE    0x07
#define IBRT_ACTION_HFSCO_DISC      0x08
#define IBRT_ACTION_REDIAL          0x09
#define IBRT_ACTION_ANSWER          0x0a
#define IBRT_ACTION_HANGUP          0x0b
#define IBRT_ACTION_LOCAL_VOLUP     0x0c
#define IBRT_ACTION_LOCAL_VOLDN     0x0d
#define IBRT_ACTION_SWITCH_A2DP     0x0e
#define IBRT_ACTION_AVRCP_ABS_VOL   0x0f
#define IBRT_ACTION_SWITCH_SCO      0x10
#define IBRT_ACTION_TELL_MASTER_CONN_PROFILE      0x11
#define IBRT_ACTION_TELL_MASTER_DISC_PROFILE      0x12
#define IBRT_ACTION_HID_SEND_CAPTURE              0x13
#define IBRT_ACTION_SEND_TOTA_DATA                0x14
#define IBRT_ACTION_TELL_MASTER_DISC_RFCOMM       0x15
#define IBRT_ACTION_HOLD_ACTIVE_CALL              0x16
#define IBRT_ACTION_3WAY_HUNGUP_INCOMING          0x17
#define IBRT_ACTION_RELEASE_ACTIVE_ACCEPT_ANOTHER 0x18
#define IBRT_ACTION_HOLD_SWITCH                   0x19
#define IBRT_ACTION_RELEASE_HOLD_CALL             0x1A
#define IBRT_ACTION_OPEN_VOICE_ASSISTANT          0x1B
#define IBRT_ACTION_CLOSE_VOICE_ASSISTANT         0x1C
#define IBRT_ACTION_HID_SENSOR_REPORT             0x1D
#define IBRT_ACTION_AVRCP_FAST_FORWARD_START      0x1E
#define IBRT_ACTION_AVRCP_FAST_FORWARD_STOP       0x1F
#define IBRT_ACTION_AVRCP_REWIND_START            0x20
#define IBRT_ACTION_AVRCP_REWIND_STOP             0x21
#define IBRT_ACTION_TOGGLE_A2DP_CIS               0x22

#ifdef __cplusplus
extern "C" {
#endif

void app_ibrt_if_start_user_action_v2(uint8_t device_id, uint8_t action, uint32_t param, uint32_t param2);
void app_ibrt_ui_perform_user_action_v2(uint8_t *p_buff, uint16_t length);
void app_ibrt_keyboard_sync_volume_info_v2(uint8_t device_id);
void app_ibrt_keyboard_start_perform_a2dp_switching(void);
void app_ibrt_keyboard_start_perform_a2dp_cis_toggle(void);
#ifdef __cplusplus
}
#endif

#endif
