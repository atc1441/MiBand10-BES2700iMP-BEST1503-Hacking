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
/*--------------------------------------------------------------------
                           GENERAL INCLUDES
--------------------------------------------------------------------*/
#include "cmsis_os.h"
#include "app_overlay.h"
#include "hal_trace.h"
#include "hal_location.h"

/*--------------------------------------------------------------------
                               MACROS
--------------------------------------------------------------------*/
#define APP_OVERLAY_RETRY_CNT       (10)
#define APP_OVERLAY_RETRY_DELAY     (100)

/*--------------------------------------------------------------------
                              VARIABLES
--------------------------------------------------------------------*/
osMutexDef(app_overlay_mutex);

static osMutexId app_overlay_mutex_id = NULL;
static APP_OVERLAY_ID_T app_overlay_id = APP_OVERLAY_ID_QTY;
osMutexDef(app_overlay_subsys_mutex);
static osMutexId app_overlay_subsys_mutex_id = NULL;
static APP_OVERLAY_SUBSYS_ID_T app_overlay_subsys_id[APP_OVERLAY_SUBSYS_QTY] = {APP_OVERLAY_SUBSYS_ID_QTY};

/*--------------------------------------------------------------------
                           OVERLAY PROCEDURES
--------------------------------------------------------------------*/
APP_OVERLAY_ID_T app_get_current_overlay(void)
{
    return app_overlay_id;
}

void app_overlay_select(enum APP_OVERLAY_ID_T id)
{
#if !defined(FPGA) || defined(FPGA_BUILD_IN_FLASH)
    int32_t retry_cnt = APP_OVERLAY_RETRY_CNT;

    TRACE(3,"%s id:%d->%d", __func__, app_overlay_id, id);
    // sanity check
    if (id >= APP_OVERLAY_ID_QTY) {
        // invalid overlay ID
        return;
    }

    while (retry_cnt > 0) {
        if (osOK == osMutexWait(app_overlay_mutex_id, APP_OVERLAY_RETRY_DELAY)) {
            if (APP_OVERLAY_ID_QTY == app_overlay_id) {
                // overlay has been unloaded
                break;
            } else {
                // still in use,release waiting for unloading
                osMutexRelease(app_overlay_mutex_id);
                osDelay(APP_OVERLAY_RETRY_DELAY);
            }
        }
        retry_cnt--;
    }

    // There is an overlay in use.
    if (APP_OVERLAY_ID_QTY != app_overlay_id) {
        ASSERT(0, "%s, overlay not unload", __func__);
    }

    if (HAL_OVERLAY_RET_OK != app_overlay_load(id)) {
        ASSERT(0, "%s failed to load", __func__);
    }
    app_overlay_id = id;
    osMutexRelease(app_overlay_mutex_id);
#endif
}

/*******************************************
 * Keep using unload all instead of unloading by specifying an ID,
 * as the premise of overlay is that no two overlay IDs are used simultaneously.
 * Additionally, modifying the selection function helps avoid potential timing issues.
 *******************************************/
void app_overlay_unloadall(void)
{
    osMutexWait(app_overlay_mutex_id, osWaitForever);
    if (app_overlay_id != APP_OVERLAY_ID_QTY){
        TRACE(3,"%s id:%d->%d", __func__, app_overlay_id, APP_OVERLAY_ID_QTY);
        app_overlay_unload(app_overlay_id);
        app_overlay_id = APP_OVERLAY_ID_QTY;
    }
    osMutexRelease(app_overlay_mutex_id);
}

void app_overlay_open(void)
{
    if (app_overlay_mutex_id == NULL) {
        app_overlay_mutex_id = osMutexCreate(osMutex(app_overlay_mutex));
    }
}

void app_overlay_close(void)
{
    app_overlay_unloadall();
    if (app_overlay_mutex_id != NULL) {
        osMutexDelete(app_overlay_mutex_id);
        app_overlay_mutex_id = NULL;
    }
}

/*--------------------------------------------------------------------
                        SUBSYS OVERLAY PROCEDURES
--------------------------------------------------------------------*/
APP_OVERLAY_SUBSYS_ID_T app_get_current_overlay_subsys(enum APP_OVERLAY_SUBSYS_T subsys)
{
    return app_overlay_subsys_id[subsys];
}

void app_overlay_subsys_select(enum APP_OVERLAY_SUBSYS_T subsys, enum APP_OVERLAY_SUBSYS_ID_T id)
{
#if !defined(FPGA) || defined(FPGA_BUILD_IN_FLASH)
    int32_t retry_cnt = APP_OVERLAY_RETRY_CNT;
    APP_OVERLAY_SUBSYS_ID_T idOld = app_overlay_subsys_id[subsys];

    TRACE(3," %s subsys:%d id:%d->%d", __func__, subsys, idOld, id);
    // sanity check
    if ((subsys >= APP_OVERLAY_SUBSYS_QTY) || (id >= APP_OVERLAY_SUBSYS_ID_QTY)) {
        // invalid overlay ID or subsys
        return;
    }

    while (retry_cnt > 0) {
        if (osOK == osMutexWait(app_overlay_subsys_mutex_id, APP_OVERLAY_RETRY_DELAY)) {
            idOld = app_overlay_subsys_id[subsys];
            if (APP_OVERLAY_SUBSYS_ID_QTY == idOld) {
                // overlay has been unloaded
                break;
            } else {
                // still in use,release waiting for unloading
                osMutexRelease(app_overlay_subsys_mutex_id);
                osDelay(APP_OVERLAY_RETRY_DELAY);
            }
        }
        retry_cnt--;
    }

    // There is an overlay in use.
    if (APP_OVERLAY_SUBSYS_ID_QTY != idOld) {
        ASSERT(0, "%s, overlay not unload", __func__);
    }

    if (HAL_OVERLAY_RET_OK != app_overlay_subsys_load(subsys, id)) {
        ASSERT(0, "%s failed to load", __func__);
    }
    app_overlay_subsys_id[subsys] = id;
    osMutexRelease(app_overlay_subsys_mutex_id);
#endif
}

void app_overlay_subsys_unloadall(enum APP_OVERLAY_SUBSYS_T subsys)
{
    APP_OVERLAY_SUBSYS_ID_T overlay_id = APP_OVERLAY_SUBSYS_ID_QTY;

    osMutexWait(app_overlay_subsys_mutex_id, osWaitForever);
    overlay_id = app_overlay_subsys_id[subsys];
    if (overlay_id != APP_OVERLAY_SUBSYS_ID_QTY) {
        app_overlay_subsys_unload(subsys, overlay_id);
        app_overlay_subsys_id[subsys] = APP_OVERLAY_SUBSYS_ID_QTY;
    }
    osMutexRelease(app_overlay_subsys_mutex_id);
}

void app_overlay_subsys_open(void)
{
    // overlay section will be loaded as the first index during m55 boot-up
    app_overlay_subsys_id[APP_OVERLAY_M55] = APP_OVERLAY_SUBSYS_ID_QTY;
    if (app_overlay_subsys_mutex_id == NULL) {
        app_overlay_subsys_mutex_id = osMutexCreate(osMutex(app_overlay_subsys_mutex));
    }
}

void app_overlay_subsys_close(enum APP_OVERLAY_SUBSYS_T subsys)
{
    app_overlay_subsys_unloadall(subsys);
    if (app_overlay_subsys_mutex_id != NULL) {
        osMutexDelete(app_overlay_subsys_mutex_id);
        app_overlay_subsys_mutex_id = NULL;
    }
}
