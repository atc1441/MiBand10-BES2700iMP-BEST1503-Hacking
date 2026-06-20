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
#ifndef __APP_AUDIO_FOCUS_STACK_H__
#define __APP_AUDIO_FOCUS_STACK_H__

#include <stdbool.h>
#include <stdlib.h>
#include "list.h"

#include "app_audio_focus_control.h"

#define MAX_AUDIO_FOCUS_STACK_SIZE     5


typedef list_t audio_focus_stack_t;

#ifdef __cplusplus
extern "C" {
#endif

audio_focus_stack_t* app_audio_focus_stack_new();
audio_focus_req_info_t* app_audio_focus_stack_top(audio_focus_stack_t* af_stack);
bool app_audio_focus_stack_push(audio_focus_stack_t* af_stack, audio_focus_req_info_t* focus_request);
bool app_audio_focus_stack_pop(audio_focus_stack_t* af_stack);
size_t app_audio_focus_stack_length(audio_focus_stack_t* af_stack);
bool app_audio_focus_stack_empty(audio_focus_stack_t* af_stack);
void app_audio_focus_stack_foreach(audio_focus_stack_t* af_stack, list_iter_cb visit,void* ctx);
void app_audio_focus_stack_destroy(audio_focus_stack_t* af_stack);
bool app_audio_focus_stack_insert_after_top(audio_focus_stack_t* af_stack,audio_focus_req_info_t* data);
bool app_audio_focus_stack_remove(audio_focus_stack_t* af_stack,void* data);
bool app_audio_focus_get_focus_by_idx(audio_focus_stack_t* af_stack, size_t index, audio_focus_req_info_t** data);
bool app_audio_focus_find_foucus(audio_focus_stack_t* af_stack, audio_focus_req_info_t* data);
audio_focus_req_info_t* app_audio_focus_get_curr_device_focus(audio_focus_stack_t* af_stack, BT_AUDIO_DEVICE_T* device);
bool app_audio_focus_stack_push_to_bottom(audio_focus_stack_t* af_stack, audio_focus_req_info_t* focus_request);
#ifdef __cplusplus
    }
#endif

#endif//__APP_AUDIO_FOCUS_STACK_H__

