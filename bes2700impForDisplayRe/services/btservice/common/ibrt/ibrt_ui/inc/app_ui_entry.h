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
#ifndef __APP_UI_ENTRY_H__
#define __APP_UI_ENTRY_H__


void app_ui_mobile_push_evt_to_cache(app_ui_btmob_sm_t *p_mobile_link_sm,app_ui_evt_t mgr_evt,uint32_t param);
void app_ui_start_run_evt_on_mobile_link(app_ui_btmob_sm_t* mobile_link_sm,app_ui_evt_t trigger_evt, uint32_t param0 = 0,uint32_t param1 = 0);

void app_ui_super_state_machine_on_event(app_ui_evt_t evt,app_ui_evt_t extra_evt = APP_UI_EV_NONE,uint8_t link_id = MOBILE_LINK_ID_INVALID);;
void app_ui_handle_event(ibrt_conn_evt_header *pkt);

#if BLE_AUDIO_ENABLED
void app_ui_start_run_evt_on_le_link(le_device_sm *device, app_ui_evt_t trigger_evt, uint32_t param0, uint32_t param1);
void app_ui_handle_le_event(void *event);
#endif

#endif
