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
#ifndef __ANC_FIR_LMS_THREAD_H__
#define __ANC_FIR_LMS_THREAD_H__

#include "stdbool.h"
#include "event_detection.h"
#include "app_voice_assist_fir_lms_thread_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ANC_FIR_LMS_MAILBOX_MAX (10)

typedef enum {
   ANC_FIR_LMS_CLOSE = 0,
   ANC_FIR_LMS_OPEN,
   ANC_FIR_LMS_PROCESS,
   ANC_FIR_LMS_QTY
} fir_lms_cmd_t;

typedef struct {
    fir_lms_cmd_t cmd;
    uint32_t ctrl;
    uint32_t ctrl_buf_len;
    uint8_t *ctrl_buf;
} ANC_FIR_LMS_MESSAGE_T;

int fir_lms_mailbox_put(ANC_FIR_LMS_MESSAGE_T* msg_src);

int fir_lms_mailbox_free(ANC_FIR_LMS_MESSAGE_T* msg_p);

int fir_lms_mailbox_get(ANC_FIR_LMS_MESSAGE_T** msg_p);

int fir_lms_thread_init(void);

void app_voice_assist_fir_lms_thread_process(process_frame_data_t *data_buf, EventController* event_control);

void app_voice_assist_fir_lms_thread_close(void);

osThreadId fir_lms_get_thread_id(void);

#ifdef __cplusplus
    }
#endif

#endif//__ANC_FIR_LMS_THREAD_H__