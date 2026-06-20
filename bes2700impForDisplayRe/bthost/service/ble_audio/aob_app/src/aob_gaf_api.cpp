/**
 * @file aob_ux_stm.cpp
 * @author BES AI team
 * @version 0.1
 * @date 2020-08-31
 *
 * @copyright Copyright (c) 2015-2021 BES Technic.
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

/*****************************header include********************************/
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_aud.h"
#include "app_trace_rx.h"
#include "plat_types.h"
#include "heap_api.h"
#include "nvrecord_ble.h"

#include "app_gaf.h"
#include "app_gaf_dbg.h"
#include "ble_audio_define.h"
#include "app_gaf_custom_api.h"

#include "aob_mgr_gaf_evt.h"
#include "aob_gaf_api.h"
#include "aob_csip_api.h"
#include "aob_media_api.h"
#include "aob_conn_api.h"
#include "aob_volume_api.h"
#include "aob_bis_api.h"

#include "app_ble.h"

/*********************external function declaration*************************/

/************************private macro defination***************************/
#define INVALID_ASE_LID          (0xFF)

/************************private type defination****************************/

/**********************private function declaration*************************/

/************************private variable defination************************/

/****************************function defination****************************/
static void _event_report(uint16_t evt_type, void *evt, uint32_t size)
{
    LOG_D("%s evt_type 0x%04x", __func__, evt_type);

    aob_mgr_gaf_evt_handle(evt_type, evt);
}

#ifdef AOB_MOBILE_ENABLED
static void _mobile_event_report(uint16_t evt_type, void *evt, uint32_t size)
{
    LOG_D("%s evt_type 0x%04x", __func__, evt_type);

    aob_mgr_gaf_mobile_evt_handle(evt_type, evt);
}
#endif

/**
 ****************************************************************************************
 * @brief earbuds initialize qos req for codec cfg req_ind - cfm callback function.
 *
 * @param[in] cb_func     callback function for qos req fill
 ****************************************************************************************
 */
void aob_gaf_api_get_qos_req_info_cb_init(void *cb_func)
{
    aob_media_ascs_register_codec_req_handler_cb((get_qos_req_cfg_info_cb)cb_func);
}

void aob_gaf_earbuds_init(aob_gaf_capa_info_t *capa_info, uint32_t role_bf)
{
    uint8_t con_lid = 0;
    app_bap_capa_srv_dir_t sink_capa_info;
    app_bap_capa_srv_dir_t src_capa_info;

    sink_capa_info.nb_pacs = capa_info->sink_nb_pacs;
    sink_capa_info.location_bf = capa_info->sink_location_bf;
    sink_capa_info.context_bf_supp = capa_info->sink_context_bf_supp;
    for (con_lid = 0; con_lid < BLE_CONNECTION_MAX; con_lid++)
    {
        sink_capa_info.context_bf_ava[con_lid] = capa_info->sink_ava_bf;
    }

    src_capa_info.nb_pacs = capa_info->src_nb_pacs;
    src_capa_info.location_bf = capa_info->src_location_bf;
    src_capa_info.context_bf_supp = capa_info->src_context_bf_supp;
    for (con_lid = 0; con_lid < BLE_CONNECTION_MAX; con_lid++)
    {
        src_capa_info.context_bf_ava[con_lid] = capa_info->src_ava_bf;
    }

    nv_record_bleaudio_init();

    /// register event report handler
    GAF_EVT_REPORT_BUNDLE_T bundle =
    {
        .earbud_report = _event_report,
#ifdef AOB_MOBILE_ENABLED
        .mobile_report = _mobile_event_report,
#endif
    };
    app_gaf_evt_report_register(&bundle);

    app_gaf_earbuds_init(&sink_capa_info, &src_capa_info, role_bf);

    aob_mgr_gaf_evt_init();

    app_ble_gap_update_local_database_hash();
}

void aob_gaf_earbuds_deinit(void)
{
    app_gaf_earbuds_deinit();
}

#ifdef AOB_MOBILE_ENABLED

void aob_gaf_mobile_init(void)
{
    aob_mgr_gaf_mobile_evt_init();
    /// register event report handler
    GAF_EVT_REPORT_BUNDLE_T bundle =
    {
        .earbud_report = _event_report,
        .mobile_report = _mobile_event_report,
    };
    app_gaf_evt_report_register(&bundle);

    app_gaf_mobile_init(); //gaf core
}
#endif

void aob_gaf_bis_init(void)
{
#ifdef APP_BLE_BIS_SRC_ENABLE
    aob_bis_src_api_init();
#endif
#ifdef APP_BLE_BIS_ASSIST_ENABLE
    aob_bis_assist_api_init();
#endif
#ifdef APP_BLE_BIS_DELEG_ENABLE
    aob_bis_deleg_api_init();
#endif
#ifdef APP_BLE_BIS_SINK_ENABLE
    aob_bis_sink_api_init();
#endif

#if defined(APP_BLE_BIS_ASSIST_ENABLE) || \
    defined(APP_BLE_BIS_DELEG_ENABLE) || defined(APP_BLE_BIS_SINK_ENABLE)
    aob_bis_scan_api_init();
#endif
}