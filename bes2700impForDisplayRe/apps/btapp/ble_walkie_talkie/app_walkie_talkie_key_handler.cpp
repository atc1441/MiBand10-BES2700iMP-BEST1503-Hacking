/**
 * @copyright Copyright (c) 2015-2022 BES Technic.
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
 */
#include "cmsis_os.h"
#include "hal_trace.h"
#include "app_key.h"
#include "app_walkie_talkie_key_handler.h"
#include "app_walkie_talkie.h"


static void app_walkie_talkie_speech_btn_down(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    TRACE(0,"W-T-KBD:Long press to speech");
    app_walkie_talkie_handle_event(WT_TALK_BTN_DOWN);
}

static void app_walkie_talkie_speech_btn_up(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    TRACE(0,"W-T-KBD:Long press to release speech");
    app_walkie_talkie_handle_event(WT_TALK_BTN_UP);
}

static const APP_KEY_HANDLE wt_key_handle_cfg[] = {
    {{APP_KEY_CODEC_WALKIE_TALKIE,APP_KEY_EVENT_DOWN},  "Start speech", app_walkie_talkie_speech_btn_down, NULL},
    {{APP_KEY_CODEC_WALKIE_TALKIE,APP_KEY_EVENT_UP  },  "Stop speech" , app_walkie_talkie_speech_btn_up, NULL},
};

void app_walkie_talkie_key_init(void)
{
    uint8_t i = 0;
    TRACE(1,"W-T-KBD:%s",__func__);

    for (i = 0; i< ARRAY_SIZE(wt_key_handle_cfg); i++)
    {
        app_key_handle_registration(&wt_key_handle_cfg[i]);
    }
}

