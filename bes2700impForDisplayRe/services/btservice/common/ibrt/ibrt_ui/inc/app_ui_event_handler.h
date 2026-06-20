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
#ifndef __APP_UI_EVENT_AHNDLER_H__
#define __APP_UI_EVENT_AHNDLER_H__

void app_ui_event_handler_init();

app_ui_evt_t app_ui_map_event(ibrt_conn_evt_header *ev_pkt);

app_ui_evt_t app_ui_map_le_event(void *pkt);

void app_ui_bt_event_handler(ibrt_conn_evt_header *pkt);

void app_ui_evt_set_next_event(app_ui_evt_t evt,app_ui_evt_t extra_evt = APP_UI_EV_NONE,uint8_t link_id = MOBILE_LINK_ID_INVALID);

void app_ui_le_audio_event_handler(void *event);

#endif /* __APP_UI_EVENT_AHNDLER_H__ */
