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
#include "hal_trace.h"
#include "anc_assist.h"
#include "app_anc_assist.h"
#include "app_voice_assist_ultrasound.h"
#include "app_voice_assist_wd.h"
#include "cmsis_os.h"

typedef struct
{
    uint32_t tick;
} ultrasound_state_t;

#define ANC_ASSIST_ULTRASOUND_PEROID_MS (500)
static ultrasound_state_t ultrasound;
static osTimerId app_anc_assist_id = NULL;

extern bool g_opened_flag;
extern AncAssistConfig anc_assist_cfg;
static void app_anc_assist_handler(void const *param);
osTimerDef(app_anc_assist, app_anc_assist_handler);
static void app_anc_assist_handler(void const *param)
{
	TRACE(0, "[%s]", __FUNCTION__);

#if 1
	if (g_opened_flag == false) {
		app_voice_assist_ultrasound_open();
	}
#endif
}

static int32_t _voice_assist_ultrasound_callback(void *buf, uint32_t len, void *other);

int32_t app_voice_assist_ultrasound_init(void)
{
    app_anc_assist_register(ANC_ASSIST_USER_ULTRASOUND, _voice_assist_ultrasound_callback);

    app_anc_assist_checker_start();

    return 0;
}

void app_anc_assist_checker_start(void)
{
    app_anc_assist_id = osTimerCreate(osTimer(app_anc_assist), osTimerPeriodic, NULL);
    if (app_anc_assist_id != NULL) {
        osTimerStart(app_anc_assist_id, ANC_ASSIST_ULTRASOUND_PEROID_MS);
    }
}

int32_t app_voice_assist_ultrasound_open(void)
{
    app_anc_assist_open(ANC_ASSIST_USER_ULTRASOUND);

    ultrasound.tick = 0;

    return 0;
}

int32_t app_voice_assist_ultrasound_close(void)
{
    app_anc_assist_close(ANC_ASSIST_USER_ULTRASOUND);

    return 0;
}

static int32_t _voice_assist_ultrasound_callback(void *buf, uint32_t len, void *other)
{
    // // TRACE(0, "[%s] len = %d", __func__, len);

    // uint32_t *res = (uint32_t *)buf;

    // if (res[0] == true) {
    //     TRACE(0, "[%s] ultrasound status changed", __func__);
    //     app_voice_assist_wd_open();
    // }

    // ultrasound.tick += 1;

    // if (anc_assist_cfg.pilot_cfg.debug_en) {
    //     TRACE(2, "[%s] ultrasound.tick = %d", __func__, ultrasound.tick);
    // }

    // if (ultrasound.tick == anc_assist_cfg.pilot_cfg.ultrasound_stop_tick) {
    //     app_voice_assist_ultrasound_close();
    // }

    return 0;
}

