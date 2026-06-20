/**
 * @file aob_volume_api.cpp
 * @author BES AI team
 * @version 0.1
 * @date 2021-06-30
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
/**
 ****************************************************************************************
 * @addtogroup AOB_APP
 * @{
 ****************************************************************************************
 */

/*****************************header include********************************/
#include "bluetooth_bt_api.h"
#include "app_bt_stream.h"
#include "audioflinger.h"
#include "hal_codec.h"
#include "nvrecord_ble.h"
#include "audio_player_adapter.h"
#include "app_gaf_custom_api.h"
#include "app_gaf_define.h"
#include "app_gaf_dbg.h"
#include "ble_audio_core_api.h"
#include "app_audio_active_device_manager.h"
#include "ble_audio_earphone_info.h"

#include "ble_tws.h"
#include "app_ble.h"
#include "app_arc_vcs_msg.h"

#include "aob_conn_api.h"
#include "aob_volume_api.h"

#ifdef IBRT
#include "app_tws_ibrt_conn_api.h"
#endif

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declaration*************************/
static void aob_tws_sync_volume_offset_status_handler();

/************************private variable defination************************/
static const BLE_AUD_CORE_EVT_CB_T *p_volume_cb = NULL;
static app_gaf_arc_vcs_volume_ind_t p_volume_ind;
static app_gaf_arc_vocs_offset_ind_t p_volume_offset_ind;
POSSIBLY_UNUSED static bool vol_synced = false;

/****************************function defination****************************/
app_gaf_arc_vcs_volume_ind_t *aob_volume_info_get(void)
{
    return &p_volume_ind;
}

static void aob_vol_control_loop_all(uint8_t opcode, uint8_t val)
{
    uint8_t con_lid = 0;

    for (con_lid = 0; con_lid < AOB_COMMON_MOBILE_CONNECTION_MAX; con_lid++)
    {
        if (app_ble_is_connection_on(con_lid))
        {
            app_arc_vcs_control_by_con_lid(con_lid, opcode, val, 0);
        }
    }
}

uint8_t aob_volume_get(void)
{
    return p_volume_ind.volume;
}

void aob_vol_mute(void)
{
    aob_vol_control_loop_all(GAF_ARC_VC_OPCODE_VOL_MUTE, 0);
}

void aob_vol_unmute(void)
{
    aob_vol_control_loop_all(GAF_ARC_VC_OPCODE_VOL_UNMUTE, 0);
}

void aob_vol_up(void)
{
    aob_vol_control_loop_all(GAF_ARC_VC_OPCODE_VOL_UP, 0);
}

void aob_vol_down(void)
{
    aob_vol_control_loop_all(GAF_ARC_VC_OPCODE_VOL_DOWN, 0);
}

void aob_vol_set_abs(uint8_t vol)
{
    int8_t leVol = aob_convert_local_vol_to_le_vol(vol);
    aob_vol_control_loop_all(GAF_ARC_VC_OPCODE_VOL_SET_ABS, leVol);
}

void aob_vol_set_and_notify_abs_volume(uint8_t con_lid, uint8_t le_vol)
{
    app_arc_vcs_update_info_req_by_con_lid(con_lid, APP_ARC_VC_UPDATE_VOL_BIT, 0, false);
    app_arc_vcs_control_by_con_lid(con_lid, GAF_ARC_VC_OPCODE_VOL_SET_ABS, le_vol, true);
}

void aob_vol_set_local_volume(uint8_t local_vol)
{
    app_bt_stream_volumeset(local_vol);
}

void aob_vol_set_volume_offset(uint8_t output_lid, uint32_t value)
{
    app_arc_vocs_set(output_lid, GAF_ARC_VOC_SET_TYPE_OFFSET, value);
}

void aob_vol_set_audio_location(uint8_t output_lid, uint32_t value)
{
    app_arc_vocs_set(output_lid, GAF_ARC_VOC_SET_TYPE_LOCATION, value);
}

void aob_vol_set_output_description(uint8_t output_lid, uint8_t *p_val, uint8_t val_len)
{
    app_arc_vocs_set_desc(output_lid, p_val, val_len);
}

uint8_t aob_convert_local_vol_to_le_vol(uint8_t bt_vol)
{
    return co_range_value_map(bt_vol, TGT_VOLUME_LEVEL_MUTE, TGT_VOLUME_LEVEL_MAX, 0, MAX_AOB_VOL);
}

uint8_t aob_convert_le_vol_to_local_vol(uint8_t le_vol)
{
    return co_range_value_map(le_vol, 0, MAX_AOB_VOL, TGT_VOLUME_LEVEL_MUTE, TGT_VOLUME_LEVEL_MAX);
}

void aob_vol_store_volume_into_nv(uint8_t con_lid, uint8_t leVol)
{
    // update the volume info to BLE stack
    ble_bdaddr_t remote_addr = {{0}};
    bool ret = app_ble_get_peer_solved_addr(con_lid, &remote_addr);
    LOG_I("(d%d)%s, vol:%d, ret:%d", con_lid, __func__, leVol, ret);
    nv_record_ble_write_volume_via_bdaddr(remote_addr.addr, leVol);

    app_arc_vcs_update_info_req_by_con_lid(con_lid, APP_ARC_VC_UPDATE_VOL_MUTE_MASK, leVol, false);
}

void aob_vol_send_notification(uint8_t con_lid, enum app_vc_char_type char_type)
{
    app_arc_vcs_send_ntf(con_lid, (uint8_t)char_type);
}

void aob_vol_update_volume_info(uint8_t con_lid, uint8_t vol, bool muted)
{
    ble_bdaddr_t remote_addr = {{0}};

    /// TODO: If set vol caller is tws sync, con_lid is invalid,
    /// so we need active device but this is not the best way,
    /// tws sync with addr is the most suitable method
    if (BT_DEVICE_INVALID_ID == con_lid)
    {
        con_lid = app_audio_adm_get_le_audio_active_device();

        if (BT_DEVICE_INVALID_ID == con_lid)
        {
            LOG_E("%s no active device to set vol, use default!", __func__);
            con_lid = 0;
        }
    }

    /// Check if vol set into global value success
    bool status = ble_audio_earphone_info_set_vol_info(con_lid, vol, muted);

    if (!status)
    {
        LOG_I("%s connection is not configued before vol is changed", __func__);
        return;
    }

    app_ble_get_peer_solved_addr(con_lid, &remote_addr);
    if (!nv_record_ble_write_volume_via_bdaddr(remote_addr.addr, vol))
    {
        LOG_E("%s peer solved addr null, nv vol invalid!", __func__);
        AOB_MOBILE_INFO_T *p_info = ble_audio_earphone_info_get_mobile_info(con_lid);
        if (p_info)
        {
            /// Before this turn to false, get vol from @see ble_audio_earphone_info_get_vol_info
            p_info->nv_vol_invalid = true;
        }
        else
        {
            LOG_E("%s cann not get mobile %d info!", __func__, con_lid);
        }
    }
}

static void aob_vol_sync_volume_status_local(uint8_t con_lid)
{
    uint8_t localVol = aob_convert_le_vol_to_local_vol(p_volume_ind.volume);

    aob_vol_update_volume_info(con_lid, p_volume_ind.volume, p_volume_ind.mute);
    localVol = p_volume_ind.mute ? TGT_VOLUME_LEVEL_MUTE : localVol;

    uint8_t actv_con_lid = app_audio_adm_get_le_audio_active_device();

    if(app_audio_adm_get_bt_active_device() != BT_DEVICE_INVALID_ID)
    {
        LOG_I("%s bt_device is active don't vol set by vcc", __func__);
    }
    else if (con_lid != BT_DEVICE_INVALID_ID && actv_con_lid == con_lid)
    {
        LOG_I("%s set vol:%d->%d, mute:%d", __func__, p_volume_ind.volume, localVol, p_volume_ind.mute);
        app_bt_stream_volumeset(localVol);
    }

    if (!vol_synced)
    {
        app_arc_vcs_update_info_req_by_con_lid(con_lid, APP_ARC_VC_UPDATE_VOL_MUTE_MASK, p_volume_ind.volume, (bool)p_volume_ind.mute);
    }
    vol_synced = false;
}

static void aob_vocs_offset_changed_cb(int16_t offset, uint8_t output_lid)
{
    p_volume_offset_ind.offset = offset;
    aob_tws_sync_volume_offset_status_handler();

    if (NULL != p_volume_cb)
    {
        p_volume_cb->ble_vocs_offset_changed_cb(offset, output_lid);
    }
}

static void aob_vocs_bond_data_changed_cb(uint8_t output_lid, uint8_t cli_cfg_bf)
{
    TRACE(2, " %s app_arc vocs bond_data_ind output_lid= %d, cli_cfg_bf = %02x",
          __func__, output_lid, cli_cfg_bf);

    if (NULL != p_volume_cb)
    {
        p_volume_cb->ble_vocs_bond_data_changed_cb(output_lid, cli_cfg_bf);
    }
}

#if defined(IBRT)
#ifdef BLE_AOB_VOLUME_SYNC_ENABLED
static void aob_tws_sync_volume_status_handler(void)
{
    // TWS sync vol with no conidx
    aob_vol_sync_volume_status_local(INVALID_BLE_CONIDX);
}
#endif
#endif

void aob_vol_sync_volume_info_cb(uint8_t *buf, uint8_t len)
{
    p_volume_ind = *((app_gaf_arc_vcs_volume_ind_t *)buf);
    LOG_I("%s volume %d mute %d", __func__, p_volume_ind.volume, p_volume_ind.mute);
}

void aob_tws_sync_volume_offset_status_handler(void)
{
    int16_t leVol = 0;
    uint8_t localVol = 0;
    leVol = p_volume_ind.mute ? TGT_VOLUME_LEVEL_MUTE : (p_volume_ind.volume + p_volume_offset_ind.offset);
    if (leVol > MAX_AOB_VOL)
    {
        leVol = MAX_AOB_VOL;
    }
    if (leVol < MIN_AOB_VOL)
    {
        leVol = MIN_AOB_VOL;
    }
    localVol = aob_convert_le_vol_to_local_vol(leVol);

    aob_vol_update_volume_info(INVALID_BLE_CONIDX, leVol, p_volume_ind.mute);
    /// configure codec
    app_bt_stream_volumeset(localVol);
}

uint8_t aob_vol_get_real_time_volume(uint8_t con_lid)
{
    ble_bdaddr_t remote_addr = {{0}};
    bool muted = false;

#ifdef CODEC_DAC_A2DP_VOLUME_128_LEVEL
    uint8_t default_vol = aob_convert_local_vol_to_le_vol(AUDIO_A2DP_OUTPUT_VOLUME_DEFAULT);
#else
    uint8_t default_vol = aob_convert_local_vol_to_le_vol(hal_codec_get_default_dac_volume_index());
#endif

    uint8_t leVol = default_vol;

    ble_audio_earphone_info_get_vol_info(con_lid, &leVol, &muted);

    if (muted)
    {
        LOG_W("%s,muted:%d", __func__, muted);
        return TGT_VOLUME_LEVEL_MUTE;
    }

    bool nv_invalid = false;
    AOB_MOBILE_INFO_T *mobile_info = ble_audio_earphone_info_get_mobile_info(con_lid);
    if (mobile_info)
    {
        nv_invalid = mobile_info->nv_vol_invalid;
    }
    if (!nv_invalid)
    {
        if (true == app_ble_get_peer_solved_addr(con_lid, &remote_addr))
        {
            nv_record_ble_read_volume_via_bdaddr((uint8_t *)remote_addr.addr, &leVol);
        }
        else
        {
            LOG_W("%s: null address", __func__);
            return default_vol;
        }
    }

    app_arc_vcs_update_info_req_by_con_lid(con_lid, APP_ARC_VC_UPDATE_VOL_BIT, leVol, false);

    LOG_I("%s vol:%d, dft vol:%d", __func__, leVol, default_vol);

    return leVol;
}

static void aob_vol_changed_cb(uint8_t con_lid, uint8_t volume, uint8_t mute,
                               uint8_t change_counter, uint8_t reason)
{
    LOG_I("%s con_lid %d, volume %d, mute %d, reason %d", __func__, con_lid, volume, mute, reason);
#ifndef __CUSTOMER_DEFINE_VCS_CONTROL__
    if ((p_volume_ind.volume != volume) || (p_volume_ind.mute != mute))
    {
        p_volume_ind.volume = volume;
        p_volume_ind.mute = mute;
#ifdef BLE_AOB_VOLUME_SYNC_ENABLED
#if defined(IBRT)
        if (app_ibrt_conn_is_tws_connected())
        {
            if (bt_callback_get_ui_role_master())
            {
                vol_synced = true;
                if (!app_ble_tws_sync_volume((uint8_t *)&p_volume_ind, sizeof(p_volume_ind)))
                {
                    vol_synced = false;
                    aob_vol_sync_volume_status_local(con_lid);
                }
            }
        }
        else
#endif
        {
            aob_vol_sync_volume_status_local(con_lid);
        }
#else
        aob_vol_sync_volume_status_local(con_lid);
#endif
    }
#endif ///__CUSTOMER_DEFINE_VCS_CONTROL__

    if ((NULL != p_volume_cb) && (NULL != p_volume_cb->ble_vol_changed))
    {
        p_volume_cb->ble_vol_changed(con_lid, volume, mute, change_counter, reason);
    }
}

void aob_vol_vcs_bond_data_changed_cb(uint8_t con_lid, uint8_t char_type, uint8_t cfg_bf)
{
    if ((NULL != p_volume_cb) &&
            (NULL != p_volume_cb->ble_vcs_cccd_changed_cb))
    {
        if (APP_VC_CHAR_TYPE_STATE == char_type)
        {
            uint8_t leVol = aob_vol_get_real_time_volume(con_lid);
            p_volume_cb->ble_vcs_cccd_changed_cb(con_lid, leVol, (bool)cfg_bf);
        }
    }
}

static vol_event_handler_t vol_event_cb =
{
    .vol_changed_cb = aob_vol_changed_cb,
    .vcs_bond_data_changed_cb = aob_vol_vcs_bond_data_changed_cb,
    .vocs_offset_changed_cb = aob_vocs_offset_changed_cb,
    .vocs_bond_data_changed_cb = aob_vocs_bond_data_changed_cb,
};

void aob_vol_api_init(void)
{
    p_volume_cb = ble_audio_get_evt_cb();
    aob_mgr_gaf_vol_evt_handler_register(&vol_event_cb);

#if defined(IBRT)
#ifdef BLE_AOB_VOLUME_SYNC_ENABLED
    app_ble_tws_sync_volume_register(aob_vol_sync_volume_info_cb,
                                     aob_tws_sync_volume_status_handler,
                                     aob_tws_sync_volume_offset_status_handler);
#endif
#endif
}

#ifdef AOB_MOBILE_ENABLED

/****************************for client(mobile)*****************************/
/*********************external function declaration*************************/

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declaration*************************/

/************************private variable defination************************/

/****************************function defination****************************/
static void aob_mobile_vol_stream_volumeset_handler(int8_t vol)
{
    uint32_t ret;
    struct AF_STREAM_CONFIG_T *stream_cfg = NULL;
    ret = af_stream_get_cfg(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg, false);
    if (ret == 0 && stream_cfg)
    {
        stream_cfg->vol = vol;
        af_stream_setup(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, stream_cfg);
    }
}

void aob_mobile_vol_mute(uint8_t con_lid)
{
    app_arc_vcc_control(con_lid, GAF_ARC_VC_OPCODE_VOL_MUTE, 0);
}

void aob_mobile_vol_unmute(uint8_t con_lid)
{
    app_arc_vcc_control(con_lid, GAF_ARC_VC_OPCODE_VOL_UNMUTE, 0);
}

void aob_mobile_vol_up(uint8_t con_lid)
{
    app_arc_vcc_control(con_lid, GAF_ARC_VC_OPCODE_VOL_UP, 0);
}

void aob_mobile_vol_down(uint8_t con_lid)
{
    app_arc_vcc_control(con_lid, GAF_ARC_VC_OPCODE_VOL_DOWN, 0);
}

void aob_mobile_vol_up_unmute(uint8_t con_lid)
{
    app_arc_vcc_control(con_lid, GAF_ARC_VC_OPCODE_VOL_UP_UNMUTE, 0);
}

void aob_mobile_vol_down_unmute(uint8_t con_lid)
{
    app_arc_vcc_control(con_lid, GAF_ARC_VC_OPCODE_VOL_DOWN_UNMUTE, 0);
}

void aob_mobile_vol_set_abs(uint8_t con_lid, uint8_t local_vol)
{
    uint8_t le_vol = aob_convert_local_vol_to_le_vol(local_vol);
    app_arc_vcc_control(con_lid, GAF_ARC_VC_OPCODE_VOL_SET_ABS, le_vol);
}

void aob_mobile_vol_set_abs_le_vol(uint8_t con_lid, uint8_t le_vol)
{
    app_arc_vcc_control(con_lid, GAF_ARC_VC_OPCODE_VOL_SET_ABS, le_vol);
}

// Volume offset control api
void aob_mobile_vol_set_volume_offset(uint8_t con_lid, uint8_t output_lid, int16_t value)
{
    app_arc_vocc_control(con_lid, output_lid, GAF_ARC_VOC_SET_TYPE_OFFSET, value);
}

void aob_mobile_vocc_read(uint8_t con_lid, uint8_t output_lid, uint8_t char_type)
{
    app_arc_vocc_read(con_lid, output_lid, char_type);
}

void aob_mobile_vol_set_audio_location(uint8_t con_lid, uint8_t output_lid, int16_t value)
{
    app_arc_vocc_control(con_lid, output_lid, GAF_ARC_VOC_SET_TYPE_LOCATION, value);
}

void aob_mobile_aic_set_input_gain(uint8_t con_lid, uint8_t input_lid, int16_t gain)
{
    app_arc_aicc_control(con_lid, input_lid, GAF_ARC_AIC_OPCODE_SET_GAIN, gain);
}

void aob_mobile_aic_set_input_mute(uint8_t con_lid, uint8_t input_lid, bool mute)
{
    app_arc_aicc_control(con_lid, input_lid, mute ?
                         GAF_ARC_AIC_OPCODE_MUTE : GAF_ARC_AIC_OPCODE_UNMUTE, 0);
}

void aob_mobile_aic_set_input_gain_mode(uint8_t con_lid, uint8_t input_lid, bool auto_mode)
{
    app_arc_aicc_control(con_lid, input_lid, auto_mode ?
                         GAF_ARC_AIC_OPCODE_SET_AUTO_MODE : GAF_ARC_AIC_OPCODE_SET_MANUAL_MODE, 0);
}

void aob_mobile_aic_read_input_char(uint8_t con_lid, uint8_t input_lid, uint8_t char_type)
{
    app_arc_aicc_read(con_lid, input_lid, char_type);
}

void aob_mobile_mic_read_char(uint8_t con_lid, uint8_t input_lid, uint8_t char_type)
{
    app_arc_aicc_read(con_lid, input_lid, char_type);
}

static void aob_mobile_vol_changed_cb(uint8_t con_lid, uint8_t volume, uint8_t mute, uint8_t change_counter, uint8_t reason)
{
    int8_t vol = 0;

    if (mute)
    {
        vol = TGT_VOLUME_LEVEL_MUTE;
    }
    else
    {
        vol = aob_convert_le_vol_to_local_vol(volume);
    }
    aob_mobile_vol_stream_volumeset_handler(vol);
}

static void aob_mobile_vocc_offset_changed_cb(uint8_t con_lid, int16_t value, uint8_t output_lid)
{
    // TODO:handle it.
    LOG_I("%s con_lid %d, value %d", __func__, con_lid, value);
}

static void aob_mobile_vocc_bond_data_changed_cb(uint8_t con_lid, uint8_t output_lid)
{
    TRACE(2, " %s app_arc vocc bond_data_ind con_lid = %d, output_id = %04x",
          __func__, con_lid, output_lid);
}

static vol_event_handler_t vol_mobile_event_cb =
{
    .vol_changed_cb = aob_mobile_vol_changed_cb,
    .vocc_offset_changed_cb = aob_mobile_vocc_offset_changed_cb,
    .vocc_bond_data_changed_cb = aob_mobile_vocc_bond_data_changed_cb,
};

void aob_vol_mobile_api_init(void)
{
    aob_mgr_gaf_mobile_vol_evt_handler_register(&vol_mobile_event_cb);
}
#endif

/// @} AOB_APP
