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
#ifndef __APP_UI_EVT_THREAD_H__
#define __APP_UI_EVT_THREAD_H__

/* function type, used when move function to ui thread */
#define APP_FUNC_TYPE_BASIC             0   // The function parameters are ordinary variables, up to four parameters are supported
#define APP_FUNC_TYPE_COMPLEX           1   // Same as above, except the first para is app_ui_evt_pkt pointer

typedef struct
{
    /* Distinguish IBRT UI/IBRT CORE/LE_AUDIO_CORE/LE_AUDIO_UI Event */
    uint32_t evt_module_type;

    void *p;
}app_ux_event_pkt;

typedef void (*APP_UI_BT_EVENT_HANDLER)(ibrt_conn_evt_header *pkt, int pkt_len);
typedef void (*APP_UI_LE_EVENT_HANDLER)(void *pkt, int pkt_len);

#ifdef __cplusplus
extern "C" {
#endif

void app_ui_event_thread_init();

void app_ui_register_bt_event_handler(APP_UI_BT_EVENT_HANDLER handler);

void app_ui_register_le_event_handler(APP_UI_LE_EVENT_HANDLER handler);

int app_ui_notify_ui_event(void *event);

int app_ui_notify_bt_core_event(void *event);

int app_ui_notify_le_core_event(void *event);

int app_ui_move_func_to_ui_thread(uint32_t func, uint32_t type, uint32_t para0,
                                  uint32_t para1, uint32_t para2, uint32_t para3);

void* app_ui_evt_heap_cmalloc(uint32_t size);

void app_ibrt_ui_evt_heap_free(void *rmem);

#ifdef __cplusplus
}
#endif

#endif /* __APP_UX_CENTER_EVT_RECEIVER_H__ */

