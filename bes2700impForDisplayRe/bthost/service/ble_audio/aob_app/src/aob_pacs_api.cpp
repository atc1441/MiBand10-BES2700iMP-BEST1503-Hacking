
/**
 * @file aob_pacs_api.cpp
 * @author BES AI team
 * @version 0.1
 * @date 2022-04-18
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
#include "hal_trace.h"
#include "plat_types.h"
#include "heap_api.h"

#include "ble_audio_earphone_info.h"
#include "ble_audio_core_api.h"
#include "ble_app_dbg.h"

#include "aob_pacs_api.h"
#include "aob_mgr_gaf_evt.h"
#include "app_gaf_define.h"
#include "ble_audio_define.h"
#include "app_gaf_custom_api.h"

/****************************for server(earbud)*****************************/

/*********************external function declaration*************************/

/************************private macro defination***************************/

#ifndef LC3PLUS_SUPPORT
/// lc3 sink pac_lid
#define AOB_LC3_SINK_PAC_LID       (0x00)
/// non sink pac_lid
#define AOB_LC3_SRC_PAC_LID        (0x01)
#else
/// lc3 sink pac_lid
#define AOB_LC3_SINK_PAC_LID      (0x00)
/// lc3plus sink pac_lid
#define AOB_LC3PLUS_SINK_PAC_LID  (0x01)
/// lc3 source pac_lid
#define AOB_LC3_SRC_PAC_LID       (0x02)
/// lc3plus source pac_lid
#define AOB_LC3PLUS_SRC_PAC_LID   (0x03)
#endif /// LC3PLUS_SUPPORT
#define AOB_INVALID_PAC_LID       (0xFF)
/************************private type defination****************************/

/************************private variable defination************************/

/**********************private function declaration*************************/

static void aob_media_pacs_cccd_written_cb(uint8_t con_lid)
{
    const BLE_AUD_CORE_EVT_CB_T *ble_audio_core_evt_cb = ble_audio_get_evt_cb();
    if (ble_audio_core_evt_cb && ble_audio_core_evt_cb->ble_media_pacs_cccd_written_cb)
    {
        ble_audio_core_evt_cb->ble_media_pacs_cccd_written_cb(con_lid);
    }
}

static pacs_event_handler_t aob_pacs_event_cb =
{
    .pacs_cccd_written_cb   = aob_media_pacs_cccd_written_cb,
};

void aob_pacs_api_init(void)
{
    aob_mgr_pacs_evt_handler_t_register(&aob_pacs_event_cb);
}

static uint8_t aob_pacs_codec_id_2_pac_lid(uint8_t direction, const aob_codec_id_t *codec_id)
{
    uint8_t pac_lid = 0;
    if (app_bap_codec_is_lc3(codec_id))
    {
        pac_lid = direction ? AOB_LC3_SRC_PAC_LID : AOB_LC3_SINK_PAC_LID;
    }
#ifdef LC3PLUS_SUPPORT
    else if (app_bap_codec_is_lc3plus(codec_id))
    {
        pac_lid = direction ? AOB_LC3PLUS_SRC_PAC_LID : AOB_LC3PLUS_SINK_PAC_LID;;
    }
#endif
#ifdef HID_ULL_ENABLE
    else if (app_bap_codec_is_ull(codec_id))
    {
        pac_lid = direction ? AOB_LC3_SRC_PAC_LID : AOB_LC3_SINK_PAC_LID;
    }
#endif
    else
    {
        pac_lid = AOB_INVALID_PAC_LID;
    }
    return pac_lid;
}

void aob_pacs_add_sink_pac_record(aob_codec_id_t *codec_id, aob_codec_capa_t *p_codec_capa)
{
    uint8_t pac_lid = aob_pacs_codec_id_2_pac_lid(AOB_MGR_DIRECTION_SINK, codec_id);
    if (pac_lid >= AOB_INVALID_PAC_LID)
    {
        LOG_E("%s unsupport codec", __func__);
        return;
    }

    app_bap_capa_srv_add_pac_record(pac_lid, (app_gaf_codec_id_t *)codec_id,  p_codec_capa->capa, p_codec_capa->metadata);
}

void aob_pacs_clear_all_sink_capa(void)
{
    app_bap_capa_srv_clear_pac_record(AOB_LC3_SINK_PAC_LID);
#ifdef LC3PLUS_SUPPORT
    app_bap_capa_srv_clear_pac_record(AOB_LC3PLUS_SINK_PAC_LID);
#endif
}

void aob_pacs_add_src_pac_record(aob_codec_id_t *codec_id, aob_codec_capa_t *p_codec_capa)
{
    uint8_t pac_lid = aob_pacs_codec_id_2_pac_lid(APP_GAF_DIRECTION_SRC, codec_id);
    if (pac_lid >= AOB_INVALID_PAC_LID)
    {
        LOG_E("%s unsupport codec", __func__);
        return;
    }

    app_bap_capa_srv_add_pac_record(pac_lid, (app_gaf_codec_id_t *)codec_id,  p_codec_capa->capa, p_codec_capa->metadata);
}

void aob_pacs_add_chan_capa_record(uint32_t channel_type, const uint8_t *p_desc_val, uint8_t val_len)
{
    if (p_desc_val == NULL || val_len == 0)
    {
        LOG_E("%s invalid param", __func__);
        return;
    }

    app_bap_capa_srv_add_chan_capa_record(channel_type, p_desc_val, val_len);
}

void aob_pacs_add_pref_aud_cfg_record(uint16_t use_case_id, uint16_t data_present_bits,
                                             const app_gaf_pref_aud_cfg_data_t *p_pref_aud_cfg_field)
{
    if (data_present_bits == 0 || p_pref_aud_cfg_field == NULL)
    {
        LOG_E("%s invalid param", __func__);
        return;
    }

    app_bap_capa_srv_add_pref_aud_cfg_record(use_case_id, data_present_bits, p_pref_aud_cfg_field);
}

void aob_pacs_del_pref_aud_cfg_record(uint16_t use_case_id)
{
    app_bap_capa_srv_delete_pref_aud_cfg_record_by_use_case(use_case_id);
}

void aob_pacs_add_sink_pac_v2_record(aob_codec_id_t *codec_id, aob_codec_capa_t *p_codec_capa)
{
    if (codec_id == NULL || p_codec_capa == NULL || p_codec_capa->capa == NULL)
    {
        LOG_E("%s invalid param", __func__);
        return;
    }

    app_bap_capa_srv_add_pac_v2_record(APP_GAF_DIRECTION_SINK, (app_gaf_codec_id_t *)codec_id,
                                       p_codec_capa->capa, p_codec_capa->metadata);
}

void aob_pacs_add_src_pac_v2_record(aob_codec_id_t *codec_id, aob_codec_capa_t *p_codec_capa)
{
    if (codec_id == NULL || p_codec_capa == NULL || p_codec_capa->capa == NULL)
    {
        LOG_E("%s invalid param", __func__);
        return;
    }

    app_bap_capa_srv_add_pac_v2_record(APP_GAF_DIRECTION_SRC, (app_gaf_codec_id_t *)codec_id,
                                       p_codec_capa->capa, p_codec_capa->metadata);
}

void aob_pacs_clear_all_src_capa(void)
{
    app_bap_capa_srv_clear_pac_record(AOB_LC3_SRC_PAC_LID);
#ifdef LC3PLUS_SUPPORT
    app_bap_capa_srv_clear_pac_record(AOB_LC3PLUS_SRC_PAC_LID);
#endif
}

void aob_pacs_set_audio_location(AOB_MGR_LOCATION_BF_E location)
{
    app_bap_capa_srv_set_supp_location_bf(APP_GAF_DIRECTION_SINK, (uint32_t)location);
    app_bap_capa_srv_set_supp_location_bf(APP_GAF_DIRECTION_SRC, (uint32_t)location);
}

uint32_t aob_pacs_get_cur_audio_location(uint8_t direction)
{
    return app_bap_capa_srv_get_location_bf((enum app_gaf_direction)direction);
}

void aob_pacs_set_ava_audio_context(uint8_t con_lid, uint16_t context_bf_ava_sink, uint16_t context_bf_ava_src)
{
    app_bap_capa_srv_set_ava_context_bf(con_lid, context_bf_ava_sink, context_bf_ava_src);
}

void aob_pacs_set_supp_audio_context(uint8_t con_lid, AOB_MGR_CONTEXT_TYPE_BF_E context_bf_supp)
{
    app_bap_capa_srv_set_supp_context_bf(con_lid, APP_GAF_DIRECTION_SINK, (uint16_t)context_bf_supp);
    app_bap_capa_srv_set_supp_context_bf(con_lid, APP_GAF_DIRECTION_SRC, (uint16_t)context_bf_supp);
}

#ifdef AOB_MOBILE_ENABLED
void aob_pacs_mobile_set_location(uint8_t con_lid, uint32_t location)
{
    app_bap_capa_cli_set_remote_location_bf(con_lid, APP_GAF_DIRECTION_SINK, location);
    app_bap_capa_cli_set_remote_location_bf(con_lid, APP_GAF_DIRECTION_SRC, location);
}

uint16_t aob_pacs_mobile_get_peer_ava_context(uint8_t con_lid, uint8_t direction)
{
    return app_bap_capa_cli_get_ava_context_bf(con_lid, direction);
}
#endif