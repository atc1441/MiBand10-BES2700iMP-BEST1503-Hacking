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
#ifndef __APP_IBRT_TWS_EXT_CMD_HANDLER_H__
#define __APP_IBRT_TWS_EXT_CMD_HANDLER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define APP_FOCOUS_DATA_SIZE 72

#define APP_TWS_EXT_CMD_A2DP_PLAYING_DEVICE     (APP_TWS_BESAPP_EXT_CMD_PREFIX + 1)
#define APP_TWS_EXT_CMD_MOBILE_PLAYBACK_INFO    (APP_TWS_BESAPP_EXT_CMD_PREFIX + 2)
#define APP_TWS_EXT_CMD_SET_TRIGGER_TIME        (APP_TWS_BESAPP_EXT_CMD_PREFIX + 3)
#define APP_TWS_EXT_CMD_SET_ACTIVE_DEVICE       (APP_TWS_BESAPP_EXT_CMD_PREFIX + 4)
#define APP_TWS_EXT_CMD_SYNC_FOCUS_INFO         (APP_TWS_BESAPP_EXT_CMD_PREFIX + 5)
#define APP_TWS_EXT_CMD_SYNC_SWITCH_INFO        (APP_TWS_BESAPP_EXT_CMD_PREFIX + 6)
#define APP_TWS_EXT_CMD_SYNC_HEADFRAME_INFO     (APP_TWS_BESAPP_EXT_CMD_PREFIX + 7)
void app_ibrt_tws_ext_cmd_init(void);

void app_ibrt_send_ext_cmd_a2dp_playing_device(uint8_t curr_playing_device, bool is_response);
void app_ibrt_send_ext_cmd_mobile_playback_info(uint8_t* param, uint16_t length);
void app_ibrt_send_ext_cmd_set_trigger(uint8_t* param, uint16_t length);
void app_ibrt_send_ext_cmd_set_active_device(uint8_t device_id, uint8_t device_type);
void app_ibrt_send_ext_cmd_sync_focus_info(uint8_t* param, uint16_t length);
void app_tws_send_ext_switch_info(bt_bdaddr_t *device_addr);
void app_ibrt_send_ext_cmd_sync_headframe_info(uint8_t* param, uint16_t length);
#ifdef __cplusplus
}
#endif

#endif /* __APP_IBRT_TWS_EXT_CMD_HANDLER_H__ */

