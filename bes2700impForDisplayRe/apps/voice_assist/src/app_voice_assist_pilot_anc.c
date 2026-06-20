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
#include "app_anc.h"
#include "app_anc_assist.h"
#include "app_voice_assist_pilot_anc.h"
#include "cmsis_os.h"

#define ANC_ASSIST_TIMING_PEROID_MS (1000 * 20)

static void pilot_anc_handler(void const *param);
osTimerDef(pilot_anc, pilot_anc_handler);
static osTimerId pilot_anc_id = NULL;
static void pilot_anc_handler(void const *param)
{
	TRACE(0, "[%s]", __FUNCTION__);

	if (app_anc_work_status()) {
		app_voice_assist_pilot_anc_open();
	}
}

static void app_voice_assist_pilot_anc_checker_start(void)
{
    pilot_anc_id = osTimerCreate(osTimer(pilot_anc), osTimerPeriodic, NULL);
    if (pilot_anc_id != NULL) {
        osTimerStart(pilot_anc_id, ANC_ASSIST_TIMING_PEROID_MS);
    }
}

static int32_t last_stop_flag = 0;

static int32_t _voice_assist_pilot_anc_callback(void *buf, uint32_t len, void *other);

int32_t app_voice_assist_pilot_anc_init(void)
{
    app_anc_assist_register(ANC_ASSIST_USER_PILOT_ANC, _voice_assist_pilot_anc_callback);

    app_voice_assist_pilot_anc_checker_start();

    return 0;
}

int32_t app_voice_assist_pilot_anc_open(void)
{
    app_anc_assist_open(ANC_ASSIST_USER_PILOT_ANC);

    last_stop_flag = 0;

    return 0;
}

int32_t app_voice_assist_pilot_anc_close(void)
{
    app_anc_assist_close(ANC_ASSIST_USER_PILOT_ANC);

    return 0;
}

extern int32_t get_pilot_stop_flag(void);

static int32_t _voice_assist_pilot_anc_callback(void *buf, uint32_t len, void *other)
{
    // TRACE(0, "[%s] len = %d", __func__, len);

    AncAssistRes *assist_res = (AncAssistRes *)other;

    // close pilot anc when pilot stops
    if (last_stop_flag == 0 && get_pilot_stop_flag()) {
        TRACE(0, "[%s] pilot finished, %d/%d", __FUNCTION__, last_stop_flag, get_pilot_stop_flag());
        app_voice_assist_pilot_anc_close();
    }

    if (assist_res->curve_changed[0] == 1 && assist_res->curve_id[0] == ANC_ASSIST_ALGO_ID_PILOT) {
        TRACE(0,"!!!!!!!!!!!!!!!!!!!!!!!! current state is %d",assist_res->curve_index[0]);
    }

    last_stop_flag = get_pilot_stop_flag();

    return 0;
}

