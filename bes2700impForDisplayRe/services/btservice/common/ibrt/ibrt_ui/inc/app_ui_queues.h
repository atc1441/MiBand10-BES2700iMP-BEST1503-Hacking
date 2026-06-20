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
#ifndef __APP_UI_QUEUES_H__
#define __APP_UI_QUEUES_H__

#define UI_EVENT_Q_LENGTH (15)

typedef enum _ret {
    RET_OK,
    RET_INVALID_PARAMS,
    RET_FAIL
} result_t;

typedef result_t (*data_visit_func)(void *ctx, void *data);

typedef struct {
    app_ui_evt_t event;
    bt_bdaddr_t addr;
    uint8_t mask;
    void *param;
} app_ui_event_t;

typedef struct
{
    app_ui_event_t ibrt_event_que[UI_EVENT_Q_LENGTH];
    uint8_t front;
    uint8_t rear;
    uint16_t size;
    uint16_t capacity;
} app_ui_evt_queue_t;

result_t app_ui_queue_init(app_ui_evt_queue_t *queue);
uint8_t app_ui_queue_size(const app_ui_evt_queue_t *queue);
result_t app_ui_queue_push(app_ui_evt_queue_t *queue, const app_ui_event_t *event);
result_t app_ui_queue_pop(app_ui_evt_queue_t *queue);
result_t app_ui_queue_front(const app_ui_evt_queue_t *queue, app_ui_event_t *event);
result_t app_ui_queue_back(const app_ui_evt_queue_t *queue, app_ui_event_t *event);
result_t app_ui_queue_foreach(const app_ui_evt_queue_t *queue, data_visit_func visit, void *ctx);
bool app_ui_queue_find_data(const app_ui_evt_queue_t *queue, app_ui_event_t *event);
bool app_ui_queue_find_data_ext(const app_ui_evt_queue_t *queue, data_visit_func visit, void *ctx);
bool app_ui_queue_empty(const app_ui_evt_queue_t *queue);
bool app_ui_queue_full(const app_ui_evt_queue_t *queue);
uint8_t app_ui_queue_find_data_count(const app_ui_evt_queue_t *queue, app_ui_evt_t event);
bool app_ui_queue_find_device_event(const app_ui_evt_queue_t *queue, app_ui_event_t *event);
bool app_ui_queue_move_event_to_head(const app_ui_evt_queue_t *queue, app_ui_evt_t event);
int app_ui_queue_test();

#endif /* __APP_UI_QUEUES_H__ */

