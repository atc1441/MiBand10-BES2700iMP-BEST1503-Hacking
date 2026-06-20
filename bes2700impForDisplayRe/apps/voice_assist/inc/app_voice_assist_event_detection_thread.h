/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#ifndef __EVENT_DETECTION_THREAD_H__
#define __EVENT_DETECTION_THREAD_H__

#include "stdbool.h"
#include "event_detection.h"
#include "app_voice_assist_fir_lms_thread_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EVENT_DETECTION_MAILBOX_MAX (10)
#define HEAP_BUFF_EVENT_SIZE (34 * 1024 * FIR_CHANNEL_NUM)

typedef enum {
   EVENT_DETECTION_CLOSE = 0,
   EVENT_DETECTION_OPEN,
   EVENT_DETECTION_PROCESS,
   EVENT_DETECTION_QTY
} event_detection_cmd_t;

typedef struct {
    event_detection_cmd_t cmd;
    uint32_t ctrl;
    uint32_t ctrl_buf_len;
    uint8_t *ctrl_buf;
} EVENT_DETECTION_MESSAGE_T;

extern custom_allocator ext_allocator_event;

int event_detection_mailbox_put(EVENT_DETECTION_MESSAGE_T* msg_src);

int event_detection_mailbox_free(EVENT_DETECTION_MESSAGE_T* msg_p);

int event_detection_mailbox_get(EVENT_DETECTION_MESSAGE_T** msg_p);

int event_detection_thread_init(void);

void app_voice_assist_event_detection_thread_process(process_frame_data_t *data_buf, EventController* event_control);

void app_voice_assist_event_detection_thread_close(void);

osThreadId event_detection_get_thread_id(void);

int32_t ext_heap_init_event(void *buff);

int32_t ext_heap_deinit_event(void);

uint32_t ext_heap_event_get_used_buff_size(void);

#ifdef __cplusplus
    }
#endif

#endif//__EVENT_DETECTION_THREAD_H__