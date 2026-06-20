/***************************************************************************
 *
 * Copyright 2020-2025 BES.
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
/**
 ****************************************************************************************
 * @addtogroup GFPS
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#ifdef SASS_ENABLED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "app_bt.h"
#include "app_bt_func.h"
#ifdef IBRT
#include "app_tws_ibrt.h"
#include "app_ibrt_internal.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_tws_ctrl_thread.h"
#include "earbud_profiles_api.h"
#include "earbud_ux_api.h"
#endif
#include "bluetooth.h"
#include "app_media_player.h"
#include "gfps_crypto.h"
#include "gfps_sass.h"
#include "gfps_ble.h"
#include "bluetooth_ble_api.h"
#include "app_hfp.h"
#include "app_bt_media_manager.h"
#include "gfps_rfcomm.h"
#include "../../utils/encrypt/aes.h"
#include "nvrecord_fp_account_key.h"
#include "audio_policy.h"
#include "app_audio_control.h"
#include "app_ui_api.h"
#include "gfps.h"
#ifdef BT_SERVICE_ENABLE
#include "app_ui_devices_mgr.h"
#endif

#if BLE_AUDIO_ENABLED
#include "mc_common.h"
#include "aob_mgr_gaf_evt.h"
#include "aob_media_api.h"
#include "ble_audio_core_api.h"
#include "ble_audio_earphone_info.h"
#include "app_ibrt_keyboard.h"
#include "app_audio_active_device_manager.h"
#include "aob_media_api.h"
#include "aob_stream_handler.h"
#endif

SassConnInfo sassInfo;
SassStateInfo sassAdv;

#ifdef __cplusplus
extern "C" {
#endif
bool gfps_sass_select_steal_device(const bt_bdaddr_t *conn_req_addr, bt_bdaddr_t *steal_addr);
#ifdef __cplusplus
}
#endif

#define  SASS_INVALID_DEV_ID   (0xFF)

void gfps_sass_update_active_info(uint8_t device_id);
void gfps_sass_switched_callback(uint8_t selectedId);
void gfps_sass_a2dp_cis_switched_callback(uint8_t selectedId);
void gfps_sass_cis_switched_callback(uint8_t selectedId);
bool gfps_sass_is_need_ntf_status(uint8_t device_id);
bool gfps_sass_ntf_conn_status(uint8_t devId, bool isUseAdv, uint8_t *state);
void gfps_sass_update_switch_dev(uint8_t oldActive, uint8_t newActive);
void gfps_sass_set_resume_dev(uint8_t devId, bool isMusic);
uint8_t gfps_sass_get_resume_dev(void);
bool gfps_sass_is_dev_streaming(uint8_t devId);
uint8_t gfps_sass_get_last_active_dev(void);

#ifndef BT_SERVICE_ENABLE
POSSIBLY_UNUSED static ibrt_ext_conn_policy_cb_t sass_ibrt_callback = {
    .accept_incoming_conn_req_hook          = NULL,
    .accept_extra_incoming_conn_req_hook    = gfps_sass_select_steal_device,
    .disallow_start_reconnect_mob_hook      = NULL,
};
#endif

void gfps_sass_init(void)
{
    memset((void *)&sassInfo, 0, sizeof(SassConnInfo));
    sassInfo.reconnInfo.evt = 0xFF;
    sassInfo.activeId = SASS_INVALID_DEV_ID;
    sassInfo.lastActId = SASS_INVALID_DEV_ID;
    sassInfo.headState = SASS_HEAD_STATE_OFF;
    sassInfo.connAvail = SASS_CONN_AVAILABLE;
    sassInfo.preference = SASS_HFP_VS_A2DP_BIT;
    sassInfo.config.acceptNew = false;
    sassInfo.config.resume = true;
    sassInfo.config.resumeId = SASS_INVALID_DEV_ID;
#ifdef IBRT_UI
    sassInfo.isMulti = true;
#endif
    sassInfo.IfSendSwitchNTF = true;
    for(int i= 0; i < SASS_DEVICE_NUM; i++)
    {
        sassInfo.connInfo[i].connId = SASS_INVALID_DEV_ID;
        sassInfo.connInfo[i].secondId = SASS_INVALID_DEV_ID;
        sassInfo.connInfo[i].devType = SASS_DEV_TYPE_INVALID;
    }

    memset((void *)&sassAdv, 0, sizeof(SassStateInfo));
    sassAdv.lenType = (3 << 4) + SASS_CONN_STATE_TYPE;
    SET_SASS_STATE(sassAdv.state, HEAD_ON, 1);
    SET_SASS_STATE(sassAdv.state, CONN_AVAILABLE, 1);

#ifndef BT_SERVICE_ENABLE
    app_ui_register_ext_conn_policy_callback(&sass_ibrt_callback);
#endif

    app_bt_audio_switch_a2dp_cmp_cb_init(gfps_sass_switched_callback);
#if BLE_AUDIO_ENABLED
    app_audio_toggle_a2dp_cis_cmp_cb_init(gfps_sass_a2dp_cis_switched_callback);
    app_ble_audio_switch_cis_cmp_register(gfps_sass_cis_switched_callback);
#endif

    TRACE(1, "%s", __func__);
}

#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
void gfps_sass_get_sync_info(uint8_t *buf, uint16_t *len)
{
    SassConnInfo *info = (SassConnInfo *)buf;
    memcpy((uint8_t *)info, (uint8_t *)&sassInfo, sizeof(SassConnInfo));
    *len = sizeof(SassConnInfo);
}

void gfps_sass_set_sync_info(uint8_t *buf, uint16_t len)
{
    SassConnInfo *info = (SassConnInfo *)buf;
    SassBtInfo temp[SASS_DEVICE_NUM];
    bool updateHead = false;
    if (info->headState != sassInfo.headState)
    {
        updateHead = true;
    }

    info->headState = sassInfo.headState;
    memcpy((uint8_t *)&temp, (uint8_t *)&(sassInfo.connInfo), sizeof(SassBtInfo) * SASS_DEVICE_NUM);
    memcpy((uint8_t *)&sassInfo, buf, sizeof(SassConnInfo));

    for(int i= 0; i < SASS_DEVICE_NUM; i++)
    {
        if (info->connInfo[i].connId != SASS_INVALID_DEV_ID)
        {
            for(int j= 0; j < SASS_DEVICE_NUM; j++)
            {
                if (!memcmp((void *)temp[j].btAddr.address, \
                    (void *)info->connInfo[i].btAddr.address, sizeof(bt_bdaddr_t)))
                {
                    TRACE(3, "%s master id:%d, slave id:%d", __func__, sassInfo.connInfo[j].connId, temp[j].connId);
                    sassInfo.connInfo[j].connId = temp[j].connId;
                    sassInfo.connInfo[j].secondId = temp[j].secondId;
                    break;
                }
            }
        }
    }

    if (updateHead)
    {
        gfps_sass_update_head_state(sassInfo.headState);
    }
}

void gfps_sass_sync_info(void)
{
    SassConnInfo info = {0};
    uint16_t len = 0;
    gfps_sass_get_sync_info((uint8_t *)&info, &len);
    tws_ctrl_send_cmd(APP_TWS_CMD_SEND_SASS_INFO, (uint8_t *)&info, len);

    TRACE(2, "%s len:%d", __func__, len);
}

void gfps_sass_role_switch_prepare(void)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if (TWS_UI_SLAVE != app_ibrt_if_get_ui_role()&& p_ibrt_ctrl->init_done)
    {
        gfps_sass_sync_info();
    }
}
#endif

uint8_t *gfps_sass_gen_session_nonce(uint8_t id)
{
    uint8_t *snPtr = NULL;
    for(int i= 0; i < SASS_DEVICE_NUM; i++)
    {
        if (sassInfo.connInfo[i].connId == id ||
            sassInfo.connInfo[i].secondId == id)
        {
            for(int n = 0; n < SESSION_NOUNCE_NUM; n++)
            {
                sassInfo.connInfo[i].session[n] = (uint8_t)rand();
            }
            snPtr = sassInfo.connInfo[i].session;
            TRACE(1, "sass dev %d session nounce is:", id);
            DUMP8("%02x ", sassInfo.connInfo[i].session, 8);
            break;
        }
    }
    return snPtr;
}

bool gfps_sass_get_session_nonce(uint8_t id, uint8_t *session)
{
    bool ret = false;
    uint8_t zero[SESSION_NOUNCE_NUM] = {0};
    for(int i= 0; i < SASS_DEVICE_NUM; i++)
    {
        if ((sassInfo.connInfo[i].connId == id ||
             sassInfo.connInfo[i].secondId == id) &&
            memcmp(sassInfo.connInfo[i].session, zero, SESSION_NOUNCE_NUM))
        {
            memcpy(session, sassInfo.connInfo[i].session, SESSION_NOUNCE_NUM);
            TRACE(0, "%s session nounce is:", __func__);
            DUMP8("%2x ", session, SESSION_NOUNCE_NUM);
            ret = true;
            break;
        }
    }

    return ret;
}

SassBtInfo *gfps_sass_get_free_handler(void)
{
    SassBtInfo *handler = NULL;
    for(int i= 0; i < SASS_DEVICE_NUM; i++)
    {
        if ((sassInfo.connInfo[i].connId == SASS_INVALID_DEV_ID) &&
            (sassInfo.connInfo[i].secondId == SASS_INVALID_DEV_ID))
        {
            handler = &(sassInfo.connInfo[i]);
            break;
        }
    }
    return handler;
}

SassBtInfo *gfps_sass_get_connected_dev(uint8_t id)
{
    SassBtInfo *handler = NULL;
    for(int i= 0; i < SASS_DEVICE_NUM; i++)
    {
        if (sassInfo.connInfo[i].connId == id  ||
            sassInfo.connInfo[i].secondId == id)
        {
            handler = &(sassInfo.connInfo[i]);
            break;
        }
    }
    return handler;
}

SassBtInfo *gfps_sass_get_connected_dev_by_addr(const bt_bdaddr_t *addr)
{
    SassBtInfo *handler = NULL;
    if (addr == NULL)
    {
        return NULL;
    }

    for(int i= 0; i < SASS_DEVICE_NUM; i++)
    {
        if (!memcmp(sassInfo.connInfo[i].btAddr.address, addr->address, sizeof(bt_bdaddr_t)))
        {
            handler = &(sassInfo.connInfo[i]);
            break;
        }
    }
    return handler;
}

SassBtInfo *gfps_sass_get_other_connected_dev(uint8_t id)
{
    SassBtInfo *info = NULL;
    for(int i= 0; i < SASS_DEVICE_NUM; i++)
    {
        if (sassInfo.connInfo[i].connId != id &&
            sassInfo.connInfo[i].secondId != id &&
            (sassInfo.connInfo[i].connId != SASS_INVALID_DEV_ID ||
            sassInfo.connInfo[i].secondId != SASS_INVALID_DEV_ID))
        {
            info = (SassBtInfo *)&(sassInfo.connInfo[i]);
            break;
        }
    }
    return info;
}

bool gfps_sass_is_key_valid(uint8_t *accKey)
{
    uint8_t empty[FP_ACCOUNT_KEY_SIZE] = {0};
    bool ret = false;
    if (!accKey)
        return false;

    if (memcmp(accKey, empty, FP_ACCOUNT_KEY_SIZE) && \
        (accKey[0] == SASS_NOT_IN_USE_ACCOUNT_KEY))
    {
        ret = true;
    }
    return ret;
}

bool gfps_sass_is_other_sass_dev(uint8_t id)
{
    bool ret = false;
    SassBtInfo *otherInfo = gfps_sass_get_other_connected_dev(id);
    if (otherInfo)
    {
        ret = otherInfo->isSass;
    }
    return ret;
}

bool gfps_sass_check_sass_mode(SassBtInfo *sInfo)
{
    bool ret = false;
    if (!sInfo)
    {
        return false;
    }

    if (sInfo->ntfSassMode && sInfo->isSass)
    {
        ret = true;
    }
    else if (sInfo->ntfSassMode && !sInfo->isSass)
    {
        ret = false;
    }
    else
    {
        uint8_t devKey[FP_ACCOUNT_KEY_SIZE] = {0};
        nv_record_fp_get_key_by_addr(sInfo->btAddr.address, devKey);
        if (gfps_sass_is_key_valid(devKey))
        {
            sInfo->isSass = true;
            ret = true;
            if (!gfps_sass_is_key_valid(sInfo->accKey))
            {
                memcpy(sInfo->accKey, devKey, FP_ACCOUNT_KEY_SIZE);
            }
        }
        else
        {
            sInfo->isSass = false;
            ret = false;
        }
    }
    return ret;
}

bool gfps_sass_is_sass_dev(SassBtInfo *sInfo)
{
    bool ret = false;
    if (sInfo)
    {
        ret = sInfo->isSass;
    }
    return ret;
}

bool gfps_sass_is_truely_sass_dev(uint8_t device_id)
{
    bool ret = false;
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(device_id);
    if (sInfo && sInfo->isSass && sInfo->ntfSassMode)
    {
        ret = true;
    }
    return ret;
}

bool gfps_sass_is_there_sass_dev()
{
    bool ret = false;
    SassBtInfo *sInfo;
    for(int i = 0; i < SASS_DEVICE_NUM; i++)
    {
        sInfo = &(sassInfo.connInfo[i]);
        if ((sInfo->connId != SASS_INVALID_DEV_ID) && sInfo->isSass)
        {
            ret = true;
            break;
        }
    }

    TRACE(1, "there is a sass dev ? %d", ret);
    return ret;
}

bool gfps_sass_is_there_in_use_dev(void)
{
    bool ret = false;
    SassBtInfo *sInfo;
    uint8_t devKey[16] = {0};
    for(int i = 0; i < SASS_DEVICE_NUM; i++)
    {
        sInfo = &(sassInfo.connInfo[i]);
        if (sInfo->connId != SASS_INVALID_DEV_ID)
        {
            nv_record_fp_get_key_by_addr(sInfo->btAddr.address, devKey);
            if (sInfo->isSass || 
                (gfps_sass_is_key_valid(devKey) && \
                !memcmp(sassInfo.inuseKey, devKey, FP_ACCOUNT_KEY_SIZE)))
            {
                ret = true;
                break;
            }
        }
    }

    TRACE(1, "there is a in use key ? %d", ret);
    return ret;
}

bool gfps_sass_is_adv_in_use_key(void)
{
    uint8_t activeDev = gfps_sass_get_active_dev();
    uint8_t lastDev = gfps_sass_get_last_active_dev();
    uint8_t devKey[FP_ACCOUNT_KEY_SIZE] = {0};
    bool ret = false;
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(activeDev);
    SassBtInfo *lastInfo = gfps_sass_get_connected_dev(lastDev);

    // last dev is non-SASS dev
    if (activeDev == 0xFF && lastDev != 0xFF && !gfps_sass_is_sass_dev(lastInfo))
    {
        ret = false;
    }//there is no active action until now
    else if (activeDev == 0xFF && gfps_sass_is_there_in_use_dev())
    {
        ret = true;
    }
    else if ((activeDev != 0xFF) && gfps_sass_is_sass_dev(sInfo) && \
        gfps_sass_get_inuse_acckey_by_id(activeDev, devKey))
    {
        ret = true;
    }
    else
    {
    }

    return ret;
}

void gfps_sass_get_adv_data(uint8_t *buf, uint8_t *len)
{
    *len =  (sassAdv.lenType >> 4) + 1;
    memcpy(buf, (uint8_t *)&sassAdv, *len);
    TRACE(1, "sass adv data, len:%d", *len);
    DUMP8("%02x ", buf, *len);
}

#ifdef SASS_SECURE_ENHACEMENT
void gfps_sass_encrypt_connection_state(uint8_t *iv,  uint8_t *inUseKey, 
                                            uint8_t *outputData, uint8_t *dataLen, bool LT, 
                                            bool isUseAdv, uint8_t *inputData)
{
    uint8_t sassBuf[SASS_ADV_LEN_MAX] = {0};//{0x35, 0x85, 0x38, 0x09};
    uint8_t sassLen = 0;
    uint8_t hkdfKey[16] = {0};
    char str[13] = "SASS-RRD-KEY";

    inUseKey[0] = NONE_IN_USET_ACCOUNT_KEY_HEADER;
    // HKDF(accountkey, NULL, UTF8("SASS-RRD-KEY"), 16);
    gfps_hdkf(NULL, 0, inUseKey, 16, (uint8_t *)str, 12, hkdfKey, 16);
    //TRACE(0, "gfps_hdkf is:");
    //DUMP8("%2x ", hkdfKey, 16);

    if (isUseAdv || !inputData)
    {
        gfps_sass_get_adv_data(sassBuf, &sassLen);
    }
    else
    {
        memcpy(sassBuf, inputData, SASS_ADV_LEN_MAX);
        sassLen = SASS_ADV_LEN_MAX;
    }

    if (LT)
    {
        AES128_CTR_encrypt_buffer(sassBuf, sassLen, hkdfKey, iv, &(outputData[1]));
        outputData[0] = (sassLen << 4) + APP_GFPS_RANDOM_RESOLVABLE_DATA_TYPE;
        *dataLen = (sassLen + 1);
    }
    else
    {
        AES128_CTR_encrypt_buffer(sassBuf + 1, sassLen-1, hkdfKey, iv, outputData);
        *dataLen = (sassLen - 1);
    }

}
#else
void gfps_sass_encrypt_adv_data(uint8_t *FArray, uint8_t sizeOfFilter, uint8_t *inUseKey, 
                                               uint8_t *outputData, uint8_t *dataLen,
                                               bool isUseAdv, uint8_t *inputData)
{
    uint8_t sassBuf[4] = {0};//{0x35, 0x85, 0x38, 0x09};
    uint8_t outBuf[16 + 1] = {0};
    uint8_t iv[16] = {0}; //{0x8c, 0xa9, 0X0c, 0x08, 0x1c, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sassLen = 0;
    uint8_t tempLen = *dataLen;

    if (isUseAdv || !inputData)
    {
        gfps_sass_get_adv_data(sassBuf, &sassLen);
    }
    else
    {
        memcpy(sassBuf, inputData, SASS_ADV_LEN_MAX);
    }

    memcpy(iv, FArray, sizeOfFilter);
    AES128_CTR_encrypt_buffer(sassBuf, sassLen, inUseKey, iv, outBuf + 1);
    TRACE(0, "encrypt connection state is:");
    DUMP8("%2x ", outBuf + 1, sassLen);
    outBuf[0] = (sassLen << 4) + APP_GFPS_RANDOM_RESOLVABLE_DATA_TYPE;
    memcpy(outputData + tempLen, outBuf, sassLen + 1);
    tempLen += (sassLen + 1);
    *dataLen = tempLen;
}

#endif

void gfps_sass_update_adv_data(void)
{
    uint8_t tempDev = 0;
    for(int i = 0; i < SASS_DEVICE_NUM; i++)
    {
        if (sassInfo.connInfo[i].connId != SASS_INVALID_DEV_ID)
        {
            if ((tempDev & SASS_DEV_TYPE_PHONEA) && \
                (sassInfo.connInfo[i].devType == SASS_DEV_TYPE_PHONEA))
            {
                tempDev |= SASS_DEV_TYPE_PHONEB;
            }
            else if (sassInfo.connInfo[i].devType != SASS_DEV_TYPE_INVALID)
            {
                tempDev |= sassInfo.connInfo[i].devType;
            }else
            {
            }
        }
    }
    sassAdv.devBitMap = tempDev;

    sassAdv.state = 0;
    SET_SASS_STATE(sassAdv.state, HEAD_ON, sassInfo.headState);
    SET_SASS_STATE(sassAdv.state, CONN_AVAILABLE, sassInfo.connAvail);
    SET_SASS_STATE(sassAdv.state, FOCUS_MODE, sassInfo.focusMode);
    SET_SASS_STATE(sassAdv.state, AUTO_RECONN, sassInfo.autoReconn);
    SET_SASS_CONN_STATE(sassAdv.state, CONN_STATE, sassInfo.connState);
}

void gfps_sass_set_sass_mode(uint8_t device_id, uint16_t sassVer, bool state)
{
    //uint8_t empty[FP_ACCOUNT_KEY_SIZE] = {0};
    uint8_t sKey[FP_ACCOUNT_KEY_SIZE] = {0};
    //bt_bdaddr_t invalidAddr = {0};
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(device_id);
    if (sInfo)
    {
        sInfo->ntfSassMode = true;
        if (!sassVer || !state) {
            sInfo->isSass = false;
        } else {
            sInfo->isSass = true;
        }
        TRACE(2, "sass ver:0x%4x, state:%d", sassVer, state);
        if (!state)
        {
            nv_record_fp_get_key_by_addr(sInfo->btAddr.address, sKey);
            nv_record_fp_delete_addr(sInfo->btAddr.address, sKey);
 
            if (!memcmp(sassInfo.inUseAddr.address, sInfo->btAddr.address, sizeof(bt_bdaddr_t)) || \
                !memcmp(sassInfo.inuseKey, sInfo->accKey, FP_ACCOUNT_KEY_SIZE))
            {
                SassBtInfo *otherInfo = gfps_sass_get_other_connected_dev(device_id);
                if (otherInfo && otherInfo->isSass && otherInfo->ntfSassMode)
                {
                    if (gfps_sass_is_key_valid(otherInfo->accKey))
                    {
                        gfps_sass_set_inuse_acckey(otherInfo->accKey, &(otherInfo->btAddr));
                    }
                }
                else
                {
                    memcpy(sassInfo.inuseKey, sassInfo.lastInuseKey, FP_ACCOUNT_KEY_SIZE);
                }
            }
             memset(sInfo->accKey, 0, FP_ACCOUNT_KEY_SIZE);
        }
        else
        {
            if (gfps_sass_is_need_ntf_status(device_id))
            {
                gfps_sass_ntf_conn_status(device_id, true, NULL);
            }
        }
    }
}

//workaround for the rfcomm event delayed transport
void gfps_sass_set_default_sass_mode(uint8_t devId)
{
    uint8_t devKey[FP_ACCOUNT_KEY_SIZE] = {0};
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(devId);
    if (!sInfo)
    {
        return;
    }

    nv_record_fp_get_key_by_addr(sInfo->btAddr.address, devKey);
    if (gfps_sass_is_key_valid(devKey))
    {
        sInfo->isSass = true;
        if (!gfps_sass_is_key_valid(sInfo->accKey))
        {
            memcpy(sInfo->accKey, devKey, FP_ACCOUNT_KEY_SIZE);
        }
    }
    else
    {
        sInfo->isSass = false;
    }
}

void gfps_sass_set_multi_status(const bt_bdaddr_t *addr, bool isMulti)
{
    SassEvtParam evtParam;
    SASS_CONN_AVAIL_E availstate;
    uint8_t num = app_ibrt_if_get_connected_mobile_count();
    uint8_t total = isMulti ? SASS_MAX_DEVICE_NUM : 1;
    TRACE(3, "%s num:%d, total:%d", __func__, num, total);
    sassInfo.isMulti = isMulti;
    if (addr)
    {
        evtParam.devId = SET_BT_ID(app_bt_get_device_id_byaddr((bt_bdaddr_t *)addr));
    }
    else
    {
        evtParam.devId = 0;
    }
    
    evtParam.event = SASS_EVT_UPDATE_MULTI_STATE;
    if (num >= total)
    {
        availstate = SASS_CONN_NONE_AVAILABLE;
    }
    else
    {
        availstate = SASS_CONN_AVAILABLE;
    }
    evtParam.state.connAvail = availstate;
    gfps_sass_update_state(&evtParam);
}

bool gfps_sass_get_multi_status()
{
    return sassInfo.isMulti;
}

void gfps_sass_switch_max_link(uint8_t device_id, uint8_t type)  //need to change later
{
    bool leAud = false;
#if BLE_AUDIO_ENABLED
    leAud = true;
#endif

    if(IS_BT_DEVICE(device_id))
    {
        bt_bdaddr_t *mobile_addr = &(app_bt_get_device(GET_BT_ID(device_id))->remote);
        TRACE(3,"%s, type:%d, id:%d", __func__, type, GET_BT_ID(device_id));
        if ((type == SASS_LINK_SWITCH_TO_SINGLE_POINT) && (app_ibrt_if_get_connected_mobile_count()> 1))
        {
            struct BT_DEVICE_T *revDevice = NULL;
            for(int i = 0; i < SASS_MAX_DEVICE_NUM; i++)
            {
                revDevice = app_bt_get_device(i);
                if (revDevice->acl_is_connected)
                {
                    AppIbrtCallStatus callState;
                    AppIbrtA2dpState a2dpState;
                    app_ibrt_if_get_a2dp_state(&(revDevice->remote), &a2dpState);
                    app_ibrt_if_get_hfp_call_status(&(revDevice->remote), &callState);
                    if ((callState != IBRT_PROFILE_NO_CALL) || (a2dpState == IBRT_PROFILE_A2DP_STREAMING))
                    {
                        mobile_addr = &(revDevice->remote);
                        break;
                    }
                }
            }
        }

        if (type == SASS_LINK_SWITCH_TO_MULTI_POINT)
        {
            app_ui_change_mode_ext(leAud, true, mobile_addr);
        }
        else
        {
            app_ui_change_mode_ext(leAud, false, mobile_addr);
        }
    }
    else
    {
        //BLE_AUDIO_SINK_DEVICE_T *device = app_ble_audio_get_device(device_id);
  /*      bt_bdaddr_t *mobile_addr = &(app_ble_audio_get_device(device_id)->address.addr);
        TRACE(3,"%s, type:%d, id:%d", __func__, type, device_id);
        if ((type == SASS_LINK_SWITCH_TO_SINGLE_POINT) && (app_ibrt_if_get_connected_mobile_count() + ble_audio_get_mobile_connected_dev_lids() > 1))
        {
            struct BT_DEVICE_T *revDevice = NULL;
            for(int i = 0; i < SASS_MAX_DEVICE_NUM; i++)
            {
                revDevice = app_bt_get_device(i);
                if (revDevice->acl_is_connected)
                {
                    AppIbrtCallStatus callState;
                    AppIbrtA2dpState a2dpState;
                    app_ibrt_if_get_a2dp_state(&(revDevice->remote), &a2dpState);
                    app_ibrt_if_get_hfp_call_status(&(revDevice->remote), &callState);
                    if ((callState != IBRT_PROFILE_NO_CALL) || (a2dpState == IBRT_PROFILE_A2DP_STREAMING))
                    {
                        mobile_addr = &(revDevice->remote);
                        break;
                    }
                }
            }
        }

        if (type == SASS_LINK_SWITCH_TO_MULTI_POINT)
        {
            app_ui_change_mode_ext(leAud, true, mobile_addr);
        }
        else
        {
            app_ui_change_mode_ext(leAud, false, mobile_addr);
        }*/
    }


}

bool gfps_sass_is_any_dev_connected()
{
    bool ret = false;
    for(int i = 0; i < SASS_DEVICE_NUM; i++)
    {
        if (sassInfo.connInfo[i].connId != SASS_INVALID_DEV_ID) {
            ret = true;
            break;
        }
    }
    return ret;
}

void gfps_sass_set_custom_data(uint8_t data)
{
    sassAdv.cusData = data;
}

void gfps_sass_get_cap_data(uint8_t *buf, uint32_t *len)
{
    uint16_t sass_ver = SASS_VERSION;
    uint16_t capbility = 0;

    capbility |= SASS_STATE_ON_BIT;
#ifdef SUPPORT_DYNAMIC_MULTIPOINT
    capbility |= SASS_MULTIPOINT_BIT;
#endif
    if (gfps_sass_get_multi_status())
    {
        capbility |= SASS_MULTIPOINT_ON_BIT;
    }
    capbility |= SASS_ON_HEAD_BIT;
    if (gfps_sass_get_head_state())
    {
        capbility |= SASS_ON_HEAD_ON_BIT;
    }
    *len = NTF_CAP_LEN;
    buf[0] = (sass_ver >> 8) & 0xFF;
    buf[1] = (sass_ver & 0xFF);
    buf[2] = (capbility >> 8) & 0xFF;
    buf[3] = (capbility & 0xFF);
}

void gfps_sass_set_switch_pref(uint8_t pref)
{
    sassInfo.preference = pref;
}

uint8_t gfps_sass_get_switch_pref(void)
{
    TRACE(1, "get reconnect dev %d", sassInfo.preference);

    return sassInfo.preference;
}

void gfps_sass_set_active_dev(uint8_t device_id)
{
    if (sassInfo.activeId != device_id)
    {
        sassInfo.lastActId = sassInfo.activeId;
        sassInfo.activeId = device_id;
    }
}

void gfps_sass_update_active_info(SassBtInfo *sInfo)
{
    if (!sInfo)
    {
        TRACE(1, "%s sass error:: there is no device info!!!", __func__);
    }
    uint8_t oldActive = gfps_sass_get_active_dev();
    uint8_t newActive = sInfo->connId;

    gfps_sass_set_active_dev(newActive);
    if (sInfo && sInfo->isSass && sInfo->ntfSassMode &&
        gfps_sass_is_key_valid(sInfo->accKey))
    {
        gfps_sass_set_inuse_acckey(sInfo->accKey, &(sInfo->btAddr));
    }

    if(sassInfo.IfSendSwitchNTF)
    {
        gfps_sass_update_switch_dev(oldActive, newActive);
    }
    else
    {
        sassInfo.IfSendSwitchNTF = true;
    }
    sInfo->isActived = true;

    if (gfps_sass_get_resume_dev() == sInfo->connId)
    {
        gfps_sass_set_resume_dev(SASS_INVALID_DEV_ID, false);
    }
}

uint8_t gfps_sass_get_active_dev(void)
{
    return sassInfo.activeId;
}

uint8_t gfps_sass_get_last_active_dev(void)
{
    return sassInfo.lastActId;
}

void gfps_sass_set_last_active_dev(uint8_t devId)
{
    sassInfo.lastActId = devId;
}

SassBtInfo *gfps_sass_get_inactive_dev(void)
{
    SassBtInfo *sInfo = NULL;
    for (int i= 0; i < SASS_DEVICE_NUM; i++)
    {
        if ((sassInfo.connInfo[i].connId != SASS_INVALID_DEV_ID) && \
            (sassInfo.activeId != sassInfo.connInfo[i].connId))
        {
            sInfo = &(sassInfo.connInfo[i]);
            break;
        }
    }
    return sInfo;
}

void gfps_sass_set_reconnecting_dev(bt_bdaddr_t *addr, uint8_t evt)
{
    memcpy(sassInfo.reconnInfo.reconnAddr.address, addr->address, sizeof(bt_bdaddr_t));
    sassInfo.reconnInfo.evt = evt;
    TRACE(0, "set reconnect dev");
}

bool gfps_sass_is_reconnect_dev(bt_bdaddr_t *addr)
{
    bool ret = false;
    if (addr && \
        !memcmp(sassInfo.reconnInfo.reconnAddr.address, addr->address, sizeof(bt_bdaddr_t)))
    {
        ret = true;
    }
    TRACE(1, "sass is reconnect dev ? %d", ret);
    return ret;
}

void gfps_sass_set_disconnecting_dev(bt_bdaddr_t *addr)
{
    if (addr)
    {
        memcpy(sassInfo.disconectingAddr.address, addr->address, sizeof(bt_bdaddr_t));
        TRACE(0, "disconnecting_dev is:");
        DUMP8("%2x ", addr->address, sizeof(bt_bdaddr_t));
    }
}

bool gfps_sass_is_disconnect_dev(bt_bdaddr_t *addr)
{
    bool ret = false;
    if (addr && \
        !memcmp(sassInfo.disconectingAddr.address, addr->address, sizeof(bt_bdaddr_t)))
    {
        ret = true;
    }
    TRACE(1, "sass is disconnect dev ? %d", ret);
    return ret;
}

uint8_t gfps_sass_get_hun_flag()
{
    return sassInfo.hunId;
}

void gfps_sass_set_hun_flag(uint8_t device_id)
{
    sassInfo.hunId = device_id;
}

bool gfps_sass_is_need_resume(bt_bdaddr_t *addr)
{
    bool ret = false;
    if (addr && 
        !memcmp(sassInfo.reconnInfo.reconnAddr.address, addr->address, sizeof(bt_bdaddr_t)) && \
        (sassInfo.reconnInfo.evt == SASS_EVT_SWITCH_BACK_AND_RESUME))
    {
        ret = true;
    }
    return ret;
}

bool gfps_sass_is_profile_connected(bt_bdaddr_t *addr)
{
    bool ret = false;
    for(int i= 0; i < SASS_DEVICE_NUM; i++)
    {
        if (!memcmp(sassInfo.connInfo[i].btAddr.address, addr->address, sizeof(bt_bdaddr_t)))
        {
            if(IS_BT_DEVICE(sassInfo.connInfo[i].connId))
            {
                if (GET_PROFILE_STATE(sassInfo.connInfo[i].audState, CONNECTION, A2DP) && \
                    GET_PROFILE_STATE(sassInfo.connInfo[i].audState, CONNECTION, HFP) && \
                    GET_PROFILE_STATE(sassInfo.connInfo[i].audState, CONNECTION, AVRCP))
                {
                    ret = true;
                }
                break;
            }
            else
            {
                if (GET_PROFILE_STATE(sassInfo.connInfo[i].audState, CONNECTION, LEA))
                {
                    ret = true;
                }
                break;
            }
        }
    }

    TRACE(1, "sass is profile connected ? %d", ret);
    return ret;
}

void gfps_sass_update_last_dev(bt_bdaddr_t *addr)
{
    if (addr)
    {
        memcpy(sassInfo.lastDev.address, addr->address, sizeof(bt_bdaddr_t));
    }
}
void gfps_sass_update_last_dev_connId(uint8_t connId)
{
    if(connId != SASS_INVALID_DEV_ID)
    {
        sassInfo.lastConnId = connId;
    }
}

void gfps_sass_clear_last_dev_connId()
{
    sassInfo.lastConnId = SASS_INVALID_DEV_ID;
}

uint8_t gfps_sass_get_last_dev_connId()
{
    return sassInfo.lastConnId;
}

void gfps_sass_clear_last_dev()
{
    memset(sassInfo.lastDev.address, 0, sizeof(bt_bdaddr_t));
    sassInfo.lastConnId = SASS_INVALID_DEV_ID;
}

void gfps_sass_get_last_dev(bt_bdaddr_t *lastAddr)
{
    memcpy(lastAddr->address, sassInfo.lastDev.address, sizeof(bt_bdaddr_t));
}

void gfps_sass_clear_reject_hf_dev(uint8_t devId)
{
    if(app_bt_hf_get_reject_dev() == devId)
    {
        app_bt_hf_set_reject_dev(SASS_INVALID_DEV_ID);
    }
}

SASS_CONN_STATE_E gfps_sass_get_state(SassBtInfo *sInfo, SASS_CONN_STATE_E entry)
{
    SASS_CONN_STATE_E state;
    uint8_t devId;
    SassBtInfo *otherInfo;

    if (!sInfo)
    {
       return sassInfo.connState;
    }

    devId = sInfo->connId;
    otherInfo = gfps_sass_get_other_connected_dev(devId);

    state = entry;
    if (otherInfo)
    {
        if ((entry == SASS_STATE_NO_CONNECTION) && (sassInfo.activeId == devId))
        {
            gfps_sass_update_active_info(otherInfo);
            TRACE(2, "change activeId to:%d as devId:%d disconnect", sassInfo.activeId, devId);
        }

        if (sassInfo.activeId == otherInfo->connId) {
            state = otherInfo->state;
        }
        else if (sassInfo.activeId == devId) {
            state = entry;
        }
        else if (((otherInfo->state >= SASS_STATE_ONLY_A2DP) && (entry < SASS_STATE_ONLY_A2DP)) ||
                  (entry == SASS_STATE_NO_CONNECTION))
        {
            state = otherInfo->state;
            if(IS_BT_DEVICE(otherInfo->connId))
            {
                if (((otherInfo->state == SASS_STATE_ONLY_A2DP ||
                     otherInfo->state == SASS_STATE_A2DP_WITH_AVRCP) &&
                    (SET_BT_ID(app_bt_audio_get_curr_playing_a2dp()) == otherInfo->connId)) ||
                    (otherInfo->state == SASS_STATE_HFP &&
                    (SET_BT_ID(app_bt_audio_get_curr_playing_sco()) == otherInfo->connId)))
                {
                    gfps_sass_update_active_info(otherInfo);
                    TRACE(2, "change activeId to:%d as devId:%d disconnect", sassInfo.activeId, devId);
                }
            }
#if BLE_AUDIO_ENABLED
            else
            {
                TRACE(1,"other info->state is %d", otherInfo->state);
                if ((otherInfo->state == SASS_STATE_LE_AUD_MEDIA_WITHOUT_CTRL ||
                     otherInfo->state == SASS_STATE_LE_AUD_MEDIA_CTRL ||
                     otherInfo->state == SASS_STATE_LE_AUD_CALL) &&
                     gfps_sass_is_dev_streaming(otherInfo->connId))
                 {
                     gfps_sass_update_active_info(otherInfo);
                     TRACE(2, "change activeId to:%d as devId:%d disconnect", sassInfo.activeId, devId);
                 }
            }
#endif
        }
        else {
        }

        if (state == SASS_STATE_NO_CONNECTION)
        {
            state = SASS_STATE_NO_DATA;
        }
    }
    TRACE(4, "activeId:%d, devId:%d, entry:%d lastAct:%d", sassInfo.activeId, devId, entry, sassInfo.lastActId);

    return state;
}

SASS_CONN_STATE_E gfps_sass_set_seeker_state(SassBtInfo *info)
{
    SASS_CONN_STATE_E connState = SASS_STATE_NO_CONNECTION;
    if (info)
    {
         //remove connection state as sometimes sco comes first.
         if (GET_PROFILE_STATE(info->audState, AUDIO, HFP)) 
         {
            info->state = SASS_STATE_HFP;
         }
         else if (GET_PROFILE_STATE(info->audState, AUDIO, AVRCP)) 
         {
            info->state = SASS_STATE_A2DP_WITH_AVRCP;
         }
         else if (GET_PROFILE_STATE(info->audState, AUDIO, A2DP) && \
             GET_PROFILE_STATE(info->audState, CONNECTION, A2DP)) 
         {
            info->state = SASS_STATE_ONLY_A2DP;
         }
         else if (GET_PROFILE_STATE(info->audState, CONNECTION, LEA) && \
             GET_PROFILE_STATE(info->audState, CALL, LEA))
         {
            info->state = SASS_STATE_LE_AUD_CALL;
         }
         else if (GET_PROFILE_STATE(info->audState, CONNECTION, LEA) && \
             GET_PROFILE_STATE(info->audState, MUSIC, LEA))
         {
            info->state = SASS_STATE_LE_AUD_MEDIA_CTRL;
         }
         else if (GET_PROFILE_STATE(info->audState, CONNECTION, LEA) && \
             GET_PROFILE_STATE(info->audState, GAME, LEA))
         {
            info->state = SASS_STATE_LE_AUD_MEDIA_WITHOUT_CTRL;
         }
         else if (GET_PROFILE_STATE(info->audState, CONNECTION, HFP) || \
             GET_PROFILE_STATE(info->audState, CONNECTION, A2DP)  || \
             GET_PROFILE_STATE(info->audState, CONNECTION, AVRCP) || \
             GET_PROFILE_STATE(info->audState, CONNECTION, LEA))
         {
            info->state = SASS_STATE_NO_DATA;
         }
         else 
         {
            info->state = SASS_STATE_NO_CONNECTION;
         }
    }
    
     connState = info->state;
     
     TRACE(3, "info->state:%d 0x%0x %d", info->state, info->audState, connState);
     return connState;
}

SASS_CONN_STATE_E gfps_sass_update_conn_state(SassBtInfo *info)
{
    SASS_CONN_STATE_E connState = SASS_STATE_NO_CONNECTION;
    gfps_sass_set_seeker_state(info);
    connState = gfps_sass_get_state(info, info->state);
    return connState;
}

void gfps_sass_set_pending_proc(SassPendingProc *pending)
{
    memcpy(&sassInfo.pending, pending, sizeof(SassPendingProc));
}

void gfps_sass_get_pending_proc(SassPendingProc *pending)
{
    memcpy(pending, &sassInfo.pending, sizeof(SassPendingProc));
}

void gfps_sass_clear_pending_proc(void)
{
    memset(&sassInfo.pending, 0, sizeof(SassPendingProc));
}

void gfps_sass_send_pause(SassBtInfo *sInfo)
{
    TRACE(3, "%s %p waitPausedone:%d", __func__, sInfo, sInfo ? sInfo->waitPauseDone : 0);
    if (!sInfo || (TWS_UI_SLAVE == app_ibrt_if_get_ui_role()))
    {
        TRACE(2, "%s slave cannot exe sass switch", __func__);
        return;
    }

    if(IS_BT_DEVICE(sInfo->connId))
    {
        if (!sInfo->waitPauseDone)
        {
            app_ibrt_if_a2dp_send_pause(GET_BT_ID(sInfo->connId));
            sInfo->waitPauseDone = true;
        }
    }
#if BLE_AUDIO_ENABLED
    else
    {
        bes_ble_aob_media_pause(sInfo->connId);
    }
#endif
}

bool gfps_sass_is_profile_exchanged(SassBtInfo *sInfo)
{
    bool ret = true;
#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
    if (app_tws_ibrt_tws_link_connected())
    {
        if (IS_BT_DEVICE(sInfo->connId))
        {
            ret = app_ibrt_conn_is_profile_exchanged(&(sInfo->btAddr));
        }
#if BLE_AUDIO_ENABLED
        else
        {
            ret = sInfo->proExchanged;
        }
#endif
    }
#endif
    return ret;
}

static bool gfps_sass_is_dev_media_streaming(SassBtInfo *sInfo)
{
    bool ret = false;
    if (IS_BT_DEVICE(sInfo->connId) && \
        GET_PROFILE_STATE(sInfo->audState, AUDIO, A2DP))
    {
        ret = true;
    }

#if BLE_AUDIO_ENABLED
    if (!IS_BT_DEVICE(sInfo->connId) && \
        (GET_PROFILE_STATE(sInfo->audState, GAME, LEA) || \
        GET_PROFILE_STATE(sInfo->audState, MUSIC, LEA)))
    {
        ret = true;
    }
#endif

    return ret;
}

static bool gfps_sass_is_dev_media_ready(SassBtInfo *sInfo)
{
    bool ret = false;
    if (IS_BT_DEVICE(sInfo->connId) && \
        (GET_PROFILE_STATE(sInfo->audState, AUDIO, A2DP) || \
        GET_PROFILE_STATE(sInfo->audState, AUDIO, AVRCP)))
    {
        ret = true;
    }

#if BLE_AUDIO_ENABLED
    if (!IS_BT_DEVICE(sInfo->connId) && \
        (GET_PROFILE_STATE(sInfo->audState, GAME, LEA) || \
        GET_PROFILE_STATE(sInfo->audState, MUSIC, LEA)))
    {
        ret = true;
    }
#endif

    return ret;
}

static bool gfps_sass_is_dev_music(SassBtInfo *sInfo)
{
    bool ret = false;
    if (IS_BT_DEVICE(sInfo->connId) && \
        GET_PROFILE_STATE(sInfo->audState, AUDIO, AVRCP))
    {
        ret = true;
    }

#if BLE_AUDIO_ENABLED
    if (!IS_BT_DEVICE(sInfo->connId) && \
        GET_PROFILE_STATE(sInfo->audState, MUSIC, LEA))
    {
        ret = true;
    }
#endif

    return ret;
}

static bool gfps_sass_is_dev_calling(SassBtInfo *sInfo)
{
    bool ret = false;
    if (IS_BT_DEVICE(sInfo->connId) && \
        GET_PROFILE_STATE(sInfo->audState, AUDIO, HFP))
    {
        ret = true;
    }

#if BLE_AUDIO_ENABLED
    if (!IS_BT_DEVICE(sInfo->connId) && \
        GET_PROFILE_STATE(sInfo->audState, CALL, LEA))
    {
        ret = true;
    }
#endif

    return ret;
}

bool gfps_sass_is_dev_streaming(uint8_t devId)
{
    bool ret = false;
    if (IS_BT_DEVICE(devId))
    {
        ret = app_bt_is_a2dp_streaming(GET_BT_ID(devId));
    }
#if BLE_AUDIO_ENABLED
    else
    {
        ret = aob_media_is_device_any_ase_in_streamimg_state(devId);
    }
#endif
    return ret;
}

void gfps_sass_switch_media(uint8_t awayId, uint8_t destId, bool update)
{
    SassBtInfo *awayInfo = gfps_sass_get_connected_dev(awayId);
    SassBtInfo *destInfo = gfps_sass_get_connected_dev(destId);
    if (awayInfo)
    {
        gfps_sass_update_last_dev(&(awayInfo->btAddr));
        gfps_sass_update_last_dev_connId(awayId);
    }

    if (!destInfo)
    {
        TRACE(2, "%s sass error:: invalid dest device id:%d", __func__, destId);
        return;
    }

    if (TWS_UI_SLAVE == app_ibrt_if_get_ui_role())
    {
        TRACE(1, "%s sass error:: slave cannot exe sass switch", __func__);
        return;
    }

    if(IS_BT_DEVICE(destId))
    {
        TRACE(0,"to switch a2dp");
        if(IS_BT_DEVICE(awayId))
        {
            //a2dp to a2dp
            app_ibrt_if_switch_streaming_a2dp();
        }
#if BLE_AUDIO_ENABLED
        else
        {
            //lea to a2dp
            app_ibrt_keyboard_start_perform_a2dp_cis_toggle();
        }
#endif
    }
#if BLE_AUDIO_ENABLED
    else
    {
        TRACE(0,"to switch lea media");
        if(IS_BT_DEVICE(awayId))
        {
            //a2dp to lea
            app_ibrt_keyboard_start_perform_a2dp_cis_toggle();
        }
        else
        {
            //lea to lea
            app_ble_audio_switch_focus(BLE_AUDIO_TWS_MASTER);
        }
    }
#endif

    gfps_sass_update_active_info(destInfo);
    if (update)
    {
        SassEvtParam evtParam;
        evtParam.devId = destId;
        evtParam.event = SASS_EVT_UPDATE_CONN_STATE;
        evtParam.state.connState = gfps_sass_update_conn_state(destInfo);
        gfps_sass_update_state(&evtParam);
    }
}

void gfps_sass_exe_pending_switch_media(uint8_t devId)
{
    SassPendingProc pending;
    if (TWS_UI_SLAVE == app_ibrt_if_get_ui_role())
    {
        TRACE(2, "%s slave cannot exe sass switch", __func__);
        return;
    }

    uint8_t currId = SET_BT_ID(app_bt_audio_get_curr_playing_a2dp());
#if BLE_AUDIO_ENABLED
    if(currId == BT_DEVICE_INVALID_ID)
    {
        currId = app_audio_adm_get_le_audio_active_device();
    }
#endif
    gfps_sass_get_pending_proc(&pending);
    if ((currId != SASS_INVALID_DEV_ID) && (currId != devId) && \
        (pending.proc == SASS_PROCESS_SWITCH_ACTIVE_SRC) && \
        (pending.activeId == devId) && \
        (pending.status & SASS_SWITCH_STATUS_REC_SWITCH))
    {
        gfps_sass_clear_pending_proc();
        gfps_sass_switch_media(currId, devId, true);
    }
}

void gfps_sass_set_profile_exchanged(uint8_t devId, bool isExchanged)
{
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(devId);
    if (sInfo)
    {
        sInfo->proExchanged = isExchanged;
        if(isExchanged && gfps_sass_is_dev_streaming(devId))
        {
            if(IS_BT_DEVICE(devId))
            {
                gfps_sass_exe_pending_switch_media(devId);
            }
#if BLE_AUDIO_ENABLED
            else
            {
                if(GET_PROFILE_STATE(sInfo->audState, MUSIC, LEA) || GET_PROFILE_STATE(sInfo->audState, GAME, LEA))
                {
                      gfps_sass_exe_pending_switch_media(devId);
                }
            }
#endif
        }
    }
}

void gfps_sass_set_peer_streaming_state(bt_bdaddr_t *addr, uint8_t state)
{
#if BLE_AUDIO_ENABLED
    SassBtInfo * sInfo = gfps_sass_get_connected_dev_by_addr(addr);
    if (sInfo)
    {
        TRACE(0,"%s, d(%d) slave state is %d", __func__, sInfo->connId, state);
        if (state == AOB_MGR_STREAM_STATE_STREAMING)
        {
            gfps_sass_set_profile_exchanged(sInfo->connId, true);
        }
        else
        {
            gfps_sass_set_profile_exchanged(sInfo->connId, false);
        }
    }
    else
    {
        TRACE(0,"%s, cannot get device by addr", __func__);
        DUMP8("%02x", addr->address, 6);
    }
#endif
}

void gfps_sass_switched_callback(uint8_t selectedId)
{
    SassBtInfo *otherInfo = gfps_sass_get_other_connected_dev(SET_BT_ID(selectedId));
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(SET_BT_ID(selectedId));
    if (otherInfo && (otherInfo->connId != SASS_INVALID_DEV_ID) && \
        GET_PROFILE_STATE(otherInfo->audState, AUDIO, A2DP) && \
        app_bt_is_a2dp_streaming(GET_BT_ID(otherInfo->connId)))
    {
       //app_ibrt_if_a2dp_send_pause(otherInfo->connId);
       gfps_sass_send_pause(otherInfo);
    }

    if (sInfo && sInfo->isNeedResume && (!GET_PROFILE_STATE(sInfo->audState, AUDIO, AVRCP)))
    {
        app_bt_ibrt_audio_play_a2dp_stream(selectedId);
        sInfo->isNeedResume = false;
    }
}

#if BLE_AUDIO_ENABLED
void gfps_sass_a2dp_cis_switched_callback(uint8_t selectedId)
{
    TRACE(2,"%s,id is %d ", __func__, selectedId);
    SassBtInfo *otherInfo = gfps_sass_get_other_connected_dev(selectedId);
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(selectedId);

    if (IS_BT_DEVICE(selectedId))
    {
        if (otherInfo && (otherInfo->connId != SASS_INVALID_DEV_ID))
        {
            if(IS_BT_DEVICE(otherInfo->connId))
            {
                if(GET_PROFILE_STATE(otherInfo->audState, AUDIO, A2DP) && \
                    app_bt_is_a2dp_streaming(GET_BT_ID(otherInfo->connId)))
                {
                    gfps_sass_send_pause(otherInfo);
                }
            }
            else
            {
                if((GET_PROFILE_STATE(otherInfo->audState, MUSIC, LEA) || GET_PROFILE_STATE(otherInfo->audState, GAME, LEA)) && \
                   aob_media_is_device_any_ase_in_streamimg_state(otherInfo->connId))
                   {
                       gfps_sass_send_pause(otherInfo);
                   }
            }
        }

        if (sInfo && sInfo->isNeedResume && (!GET_PROFILE_STATE(sInfo->audState, AUDIO, AVRCP)))
        {
            app_bt_ibrt_audio_play_a2dp_stream(GET_BT_ID(selectedId));
            sInfo->isNeedResume = false;
        }
    }
    else
    {
        if (otherInfo && (otherInfo->connId != SASS_INVALID_DEV_ID))
        {
            if(IS_BT_DEVICE(otherInfo->connId))
            {
                if(GET_PROFILE_STATE(otherInfo->audState, AUDIO, A2DP) && \
                    app_bt_is_a2dp_streaming(GET_BT_ID(otherInfo->connId)))
                {
                    gfps_sass_send_pause(otherInfo);
                }
            }
            else
            {
                if((GET_PROFILE_STATE(otherInfo->audState, MUSIC, LEA) || GET_PROFILE_STATE(otherInfo->audState, GAME, LEA)) && \
                   aob_media_is_device_any_ase_in_streamimg_state(otherInfo->connId))
                   {
                       gfps_sass_send_pause(otherInfo);
                   }
            }
        }

        if (sInfo && sInfo->isNeedResume && (!GET_PROFILE_STATE(sInfo->audState, MUSIC, LEA)))
        {
            bes_ble_aob_media_play(selectedId);
            sInfo->isNeedResume = false;
        }
    }
}

void gfps_sass_cis_switched_callback(uint8_t selectedId)
{
    TRACE(2,"%s,id is %d ", __func__, selectedId);
    SassBtInfo *otherInfo = gfps_sass_get_other_connected_dev(selectedId);
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(selectedId);
    if (otherInfo && (otherInfo->connId != SASS_INVALID_DEV_ID))
    {
        if((GET_PROFILE_STATE(otherInfo->audState, MUSIC, LEA) || GET_PROFILE_STATE(otherInfo->audState, GAME, LEA)) && \
            aob_media_is_device_any_ase_in_streamimg_state(otherInfo->connId))
        {
            gfps_sass_send_pause(otherInfo);
        }
    }

    if (sInfo && sInfo->isNeedResume && (!GET_PROFILE_STATE(sInfo->audState, MUSIC, LEA)))
    {
        bes_ble_aob_media_play(selectedId);
        sInfo->isNeedResume = false;
    }
}
#endif

SASS_CONN_STATE_E gfps_sass_get_conn_state(void)
{
    return sassInfo.connState;
}

void gfps_sass_set_conn_state(SASS_CONN_STATE_E state)
{
    TRACE(1, "set sass conn state:0x%0x", state);

    sassInfo.connState = state;
}

SASS_HEAD_STATE_E gfps_sass_get_head_state(void)
{
    return sassInfo.headState;
}

void gfps_sass_set_head_state(SASS_HEAD_STATE_E headstate)
{
    sassInfo.headState = headstate;
}

void gfps_sass_set_conn_available(SASS_CONN_AVAIL_E available)
{
    sassInfo.connAvail= available;
}

void gfps_sass_set_focus_mode(SASS_FOCUS_MODE_E focus)
{
    sassInfo.focusMode = focus;
}

void gfps_sass_set_auto_reconn(SASS_AUTO_RECONN_E focus)
{
    sassInfo.autoReconn = focus;
}

void gfps_sass_set_init_conn(uint8_t device_id, bool bySass)
{
    SassBtInfo *info = gfps_sass_get_connected_dev(device_id);
    if (info)
    {
        info->initbySass = bySass;
    }
}

void gfps_sass_set_inuse_acckey(uint8_t *accKey, bt_bdaddr_t *addr)
{
    if (accKey)
    {
        memcpy(sassInfo.lastInuseKey, sassInfo.inuseKey, FP_ACCOUNT_KEY_SIZE);
        memcpy(sassInfo.inuseKey, accKey, FP_ACCOUNT_KEY_SIZE);
    }

    if (addr)
    {
        memcpy(sassInfo.inUseAddr.address, addr->address, sizeof(bt_bdaddr_t));
    }
}

void gfps_sass_set_inuse_acckey_by_dev(uint8_t device_id, uint8_t *accKey)
{
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(device_id);
    if (sInfo)
    {
        gfps_sass_set_inuse_acckey(accKey, &(sInfo->btAddr));

        memcpy(sInfo->accKey, accKey, FP_ACCOUNT_KEY_SIZE);
        sInfo->updated = true;
    }
    TRACE(1, "sass dev %d inuse acckey is:", device_id);
    DUMP8("0x%2x ", accKey, FP_ACCOUNT_KEY_SIZE);
}

bool gfps_sass_get_inuse_acckey(uint8_t *accKey)
{
    uint8_t *kPointer = NULL;
    bool ret = false;

    if (!gfps_sass_is_key_valid(sassInfo.inuseKey))
    {
        SassBtInfo *sInfo = NULL;
        for(int i= 0; i < SASS_DEVICE_NUM; i++)
        {
            sInfo = &(sassInfo.connInfo[i]);
            if (sInfo->connId != SASS_INVALID_DEV_ID)
            {
                if (gfps_sass_check_sass_mode(sInfo))
                {
                    kPointer = sInfo->accKey;
                    ret = true;
                    break;
                }
            }
        } 
    }
    else
    {
        kPointer = sassInfo.inuseKey;
        ret = true;
    }

    if (ret)
    {
        memcpy(accKey, kPointer, FP_ACCOUNT_KEY_SIZE);
        TRACE(0, "get inuse acckey is:");
        DUMP8("%2x ", kPointer, FP_ACCOUNT_KEY_SIZE);
    }

    return ret;
}

bool gfps_sass_get_inuse_acckey_by_id(uint8_t device_id, uint8_t *accKey)
{
    bool ret = false;
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(device_id);
    if (sInfo && gfps_sass_is_key_valid(sInfo->accKey))
    {
        memcpy(accKey, sInfo->accKey, FP_ACCOUNT_KEY_SIZE);
        ret = true;
    }
    TRACE(1, "get sass dev %d inuse acckey is:", device_id);
    DUMP8("%2x ", accKey, FP_ACCOUNT_KEY_SIZE);
    return ret;
}

void gfps_sass_update_head_state(SASS_HEAD_STATE_E state)
{
    SassEvtParam evtParam;
    evtParam.devId = SET_BT_ID(0);
    evtParam.event = SASS_EVT_UPDATE_HEAD_STATE;
    evtParam.state.headState = state;
    gfps_sass_update_state(&evtParam);
}

SASS_REASON_E gfps_sass_get_switch_reason(uint8_t devId)
{
    SASS_REASON_E reason;
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(devId);
    if (sInfo == NULL)
    {
        return SASS_REASON_UNSPECIFIED;
    }
    if(IS_BT_DEVICE(sInfo->connId))
    {
        #if 0
        if (state == SASS_STATE_ONLY_A2DP || state == SASS_STATE_A2DP_WITH_AVRCP){
            reason = SASS_REASON_A2DP;
        }else if (state == SASS_STATE_HFP) {
            reason = SASS_REASON_HFP;
        }else {
            reason = SASS_REASON_UNSPECIFIED;
        }
        #else
        if (GET_PROFILE_STATE(sInfo->audState, AUDIO, HFP)){
            reason = SASS_REASON_HFP;
        }else if (GET_PROFILE_STATE(sInfo->audState, AUDIO, A2DP) || \
                  GET_PROFILE_STATE(sInfo->audState, AUDIO, AVRCP)) {
            reason = SASS_REASON_A2DP;
        }else {
            reason = SASS_REASON_UNSPECIFIED;
        }
        #endif
    }
    else
    {
        if (GET_PROFILE_STATE(sInfo->audState, CALL,LEA)){
            reason = SASS_REASON_HFP;
        }else if (GET_PROFILE_STATE(sInfo->audState, MUSIC, LEA))
        {
            reason = SASS_REASON_A2DP;
        }else if(GET_PROFILE_STATE(sInfo->audState, GAME, LEA))
        {
            reason = SASS_REASON_A2DP;
        }
        else {
            reason = SASS_REASON_UNSPECIFIED;
        }
    }
    return reason;
}

SASS_DEV_TYPE_E gfps_sass_get_dev_type_by_cod(uint8_t *cod)
{
    SASS_DEV_TYPE_E type;
    uint32_t devCod = cod[0] + (cod[1] << 8) + (cod[2] << 16);
    TRACE(4, "%s type: 0x%02x%02x%02x", __func__, cod[0], cod[1], cod[2]);

    if ((devCod & COD_TYPE_PHONE) == COD_TYPE_PHONE)
    {
        type = SASS_DEV_TYPE_PHONEA;
    }
    else if ((devCod & COD_TYPE_LAPTOP) == COD_TYPE_LAPTOP)
    {
        type = SASS_DEV_TYPE_LAPTOP;
    }
    else if ((devCod & COD_TYPE_TABLET) == COD_TYPE_TABLET)
    {
        type = SASS_DEV_TYPE_TABLET;
    }
    else if ((devCod & COD_TYPE_TV) == COD_TYPE_TV)
    {
        type = SASS_DEV_TYPE_TV;
    }
    else
    {
        type = SASS_DEV_TYPE_PHONEA;//SASS_DEV_TYPE_INVALID;
    }

    return type;
}

void gfps_sass_check_if_need_reconnect(uint8_t devId)
{
    uint8_t invalidAddr[6] = {0};
    bt_bdaddr_t *reconnAddr = &(sassInfo.reconnInfo.reconnAddr);
#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
    if (TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
#endif
    {
        if(IS_BT_DEVICE(devId))
        {
            if (memcmp(reconnAddr->address, invalidAddr, sizeof(bt_bdaddr_t)) && \
                !app_bt_is_acl_connected_byaddr(reconnAddr))
            {
                TRACE(1, "%s try to reconnect dev", __func__);
                DUMP8("%02x ", reconnAddr->address, 6);
                app_ibrt_if_choice_mobile_connect((bt_bdaddr_t *)&(sassInfo.reconnInfo.reconnAddr), SASS_CONNECT_COUNT);
            }
        }
    }
}

void gfps_sass_check_if_need_hun(uint8_t device_id, uint8_t event)
{
    SassBtInfo *otherInfo = gfps_sass_get_other_connected_dev(device_id);
    if (otherInfo == NULL)
    {
        return;
    }

    uint8_t activeId = gfps_sass_get_active_dev();

    if ((activeId != SASS_INVALID_DEV_ID) && (activeId == otherInfo->connId))
    {
        switch(event)
        {
            case BTIF_HF_EVENT_AUDIO_CONNECTED:
                if (GET_PROFILE_STATE(otherInfo->audState, AUDIO, A2DP))
                {
                    gfps_sass_set_hun_flag(device_id);
                }
                break;

            default:
                break;
        }
    }
}

void gfps_sass_get_acckey_from_nv(uint8_t *addr, uint8_t *key)
{
    nv_record_fp_get_key_by_addr(addr, key);
}

void gfps_sass_set_need_ntf_status(uint8_t device_id, bool en)
{
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(device_id);
    if (sInfo)
    {
        sInfo->needNtfStatus = en;
    }
}

bool gfps_sass_is_need_ntf_status(uint8_t device_id)
{
    bool ret = false;
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(device_id);
    if (sInfo)
    {
        ret = sInfo->needNtfStatus;
    }
    return ret;
}

void gfps_sass_set_need_ntf_switch(uint8_t device_id, bool en)
{
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(device_id);
    if (sInfo)
    {
        sInfo->needNtfSwitch = en;
    }
}

bool gfps_sass_is_need_ntf_switch(uint8_t device_id)
{
    bool ret = false;
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(device_id);
    if (sInfo)
    {
        ret = sInfo->needNtfSwitch;
    }
    return ret;
}

void gfps_sass_send_session_nonce(uint8_t devId)
{
    FP_MESSAGE_STREAM_T req = {FP_MSG_GROUP_DEVICE_INFO, FP_MSG_DEVICE_INFO_SESSION_NONCE, 0, 8};
    if(!gfps_sass_get_session_nonce(devId, req.data))
    {
        uint8_t *snPtr = gfps_sass_gen_session_nonce(devId);
        if (snPtr)
        {
            memcpy(req.data, snPtr, SESSION_NOUNCE_NUM);
        }
    }
    gfps_send(devId, (uint8_t *)&req, FP_MESSAGE_RESERVED_LEN+8);
}

void gfps_sass_get_capability(uint8_t devId)
{
    TRACE(1,"%s",__func__);
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_SASS,
         FP_MSG_SASS_GET_CAPBILITY,
         0,
         0};
    gfps_send(devId, ( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN);
}

void gfps_sass_ntf_capability(uint8_t devId)
{
    TRACE(2,"%s %d",__func__, devId);
    uint32_t dataLen = 0;
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_SASS,
         FP_MSG_SASS_NTF_CAPBILITY};
    gfps_sass_get_cap_data(req.data, &dataLen);
    req.dataLenHighByte = (uint8_t)(dataLen & 0xFF00);
    req.dataLenLowByte = (uint8_t)(dataLen & 0xFF);
    gfps_send(devId, ( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + dataLen);
}

void gfps_sass_set_capability(uint8_t devId,uint8_t *data)
{
    uint16_t sassVer =( data[0] << 8) | data[1];
    bool state = (data[2] & 0x80) ? true: false;
    gfps_sass_set_sass_mode(devId, sassVer, state);

    if (!state)
    {
        SassEvtParam evtParam = {
            .event = SASS_EVT_UPDATE_INUSE_ACCKEY,
            .devId = devId,
        };
        gfps_sass_update_state(&evtParam);
    }    
    else
    {
#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
        if (TWS_UI_MASTER == app_ibrt_if_get_ui_role())
        {
            gfps_sass_sync_info();
        }
#endif
    }
}

void gfps_sass_set_multipoint_hdl(uint8_t devId, uint8_t *data)
{
    uint8_t enable = data[0];
    if(enable) {
        gfps_sass_switch_max_link(devId, SASS_LINK_SWITCH_TO_MULTI_POINT);
    }else {
        gfps_sass_switch_max_link(devId, SASS_LINK_SWITCH_TO_SINGLE_POINT);
    }

    TRACE(2,"%s swtich:%d",__func__, enable);
}

void gfps_sass_set_switch_pref_hdl(uint8_t devId, uint8_t *data)
{
    uint8_t pref = data[0];
    gfps_sass_set_switch_pref(pref);
}

void gfps_sass_ntf_switch_pref(uint8_t devId)
{
    TRACE(1,"%s",__func__);
    uint16_t dataLen = 2;
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_SASS,
         FP_MSG_SASS_NTF_SWITCH_PREFERENCE,
         0,
         2};
    req.data[0] = gfps_sass_get_switch_pref();
    req.data[1] = 0;
    gfps_send(devId, ( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + dataLen);
}

bool gfps_sass_ntf_switch_evt(uint8_t devId, uint8_t reason)
{
    uint32_t nameLen = 0;
    bool ret = true;
    uint8_t *namePtr = NULL;
    uint8_t activeId = gfps_sass_get_active_dev();

    namePtr = nv_record_fp_get_name_ptr(&nameLen);
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_SASS,
         FP_MSG_SASS_NTF_SWITCH_EVT,
         0,
         (uint8_t)(2+nameLen)};

    req.data[0] = reason;
    req.data[1] = (devId == activeId) ? SASS_DEV_THIS_DEVICE : SASS_DEV_ANOTHER;
    if(namePtr)
    {
        memcpy(req.data + 2, namePtr, nameLen);
    }

    ret = gfps_send(devId, ( uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + 2 + nameLen);
    if (ret == false)
    {
        gfps_sass_set_need_ntf_switch(devId, true);
    }
    else
    {
        gfps_sass_set_need_ntf_switch(devId, false);
    }
    return ret;
}

bool gfps_sass_ntf_conn_status(uint8_t devId, bool isUseAdv, uint8_t *state)
{
    uint8_t advlen, len, activeId;
    uint8_t account[FP_ACCOUNT_KEY_SIZE] = {0};
    uint8_t outBuf[FP_ACCOUNT_KEY_SIZE] = {0};
    //uint8_t actKey[FP_ACCOUNT_KEY_SIZE] = {0};
    uint8_t iv[FP_ACCOUNT_KEY_SIZE] = {0};
    uint8_t devKey[FP_ACCOUNT_KEY_SIZE] = {0};
    uint8_t memNonce[SESSION_NOUNCE_NUM] = {0};
    bool ret = true;
    bool devKeyVaild = false;
    SassBtInfo * otherInfo = NULL;
    FP_MESSAGE_STREAM_T req =
        {FP_MSG_GROUP_SASS,
         FP_MSG_SASS_NTF_CONN_STATUS,
         0,
         0};

    if (!gfps_sass_get_session_nonce(devId, iv))
    {
        ret = false;
        TRACE(0, "sass error:cannot get session nonce!!!");
        goto NTF_EXIT;
    }

    activeId = gfps_sass_get_active_dev();
    devKeyVaild = gfps_sass_get_inuse_acckey_by_id(devId, devKey);
#if 0
    if (isUseAdv)
    {
        bool valid = gfps_sass_get_inuse_acckey(account);
        if (!valid && devKeyVaild)
        {
           memcpy(account, devKey, FP_ACCOUNT_KEY_SIZE);
        }
        else if (!valid && !devKeyVaild)
        {
            ret = false; 
            TRACE(0, "sass error:cannot get dev inuse account key!!!");
            goto NTF_EXIT;
        }
        else{
        }
    }
    else
    {
        if (!devKeyVaild)
        {
            ret = false; 
            TRACE(0, "sass error:cannot get dev inuse account key!!!");
            goto NTF_EXIT;
        }
        memcpy(account, devKey, FP_ACCOUNT_KEY_SIZE);
        DUMP8("%2x ", state, 4);  
    }
#else
    if (!devKeyVaild)
    {
        ret = false; 
        TRACE(0, "sass error:cannot get dev inuse account key!!!");
        goto NTF_EXIT;
    }
    else
    {
        memcpy(account, devKey, FP_ACCOUNT_KEY_SIZE);
    }

#endif
    for(int i = 0; i < SESSION_NOUNCE_NUM; i++)
    {
         memNonce[i] = (uint8_t)rand();
         iv[SESSION_NOUNCE_NUM + i] = memNonce[i];
    }

#ifdef SASS_SECURE_ENHACEMENT
    gfps_sass_encrypt_connection_state(iv, account, outBuf, &advlen, false, isUseAdv, state);
#else
    uint8_t adv[5] = {0};
    gfps_sass_get_adv_data(adv, &advlen);
    AES128_CTR_encrypt_buffer(adv+1, advlen-1, account, iv, outBuf, isUseAdv, state);
    advlen--;
#endif

    len = advlen + 1 + SESSION_NOUNCE_NUM;
    req.dataLenLowByte = len;

    otherInfo = gfps_sass_get_other_connected_dev(devId);
    if (!otherInfo || activeId == devId)
    {
        req.data[0] = SASS_DEV_IS_ACTIVE;
    }
    else if ((otherInfo && gfps_sass_is_sass_dev(otherInfo)) || (activeId == SASS_INVALID_DEV_ID))
    {
        req.data[0] = SASS_DEV_IS_PASSIVE;
    }
    else
    {
        req.data[0] = SASS_DEV_IS_PSSIVE_WITH_NONSASS;
    }
    TRACE(4, "%s dev:%d active:%d req.data[0]:%d", __func__, devId, activeId, req.data[0]);

    memcpy(req.data + 1, outBuf, advlen);
    memcpy(req.data + advlen + 1, memNonce, SESSION_NOUNCE_NUM);
    ret = gfps_send(devId, (uint8_t * )&req, FP_MESSAGE_RESERVED_LEN + len);  

NTF_EXIT:
    if (ret == false)
    {
        gfps_sass_set_need_ntf_status(devId, true);
    }
    else
    {
        gfps_sass_set_need_ntf_status(devId, false);
    }
    return ret;
}

void gfps_sass_get_conn_hdl(uint8_t devId)
{
    gfps_sass_ntf_conn_status(devId, true, NULL);
    TRACE(1,"%s",__func__);
}

void gfps_sass_set_init_conn(uint8_t devId, uint8_t *data)
{
    bool isSass = data[0];
    gfps_sass_set_init_conn(devId, isSass);
    TRACE(1,"connection is triggered by sass? %d", isSass);
}

void gfps_sass_set_custom_data(uint8_t devId, uint8_t *data)
{
    uint8_t param = data[0];
    uint8_t activeDev = gfps_sass_get_active_dev();
    if ((activeDev != SASS_INVALID_DEV_ID) && (activeDev != devId))
    {
        return;
    }

    SassEvtParam evtParam;
    evtParam.event = SASS_EVT_UPDATE_CUSTOM_DATA;
    evtParam.devId = devId;
    evtParam.state.cusData = param;
    gfps_sass_update_state(&evtParam);
}

void gfps_sass_set_drop_dev(uint8_t devId, uint8_t *data)
{
    //drop this device
    if(data[0] == 1)
    {
        SassBtInfo *info = gfps_sass_get_connected_dev(devId);
        if (info)
        {
            memcpy(sassInfo.dropDevAddr.address, info->btAddr.address, sizeof(bt_bdaddr_t));
        }
    }
}

void gfps_sass_set_resume_dev(uint8_t devId, bool isMusic)
{
    sassInfo.config.resumeId = devId;
    sassInfo.config.isLastMusic = isMusic;
}

uint8_t gfps_sass_get_resume_dev(void)
{
     return sassInfo.config.resumeId;
}

uint8_t gfps_sass_is_resume_dev_music(void)
{
     return sassInfo.config.isLastMusic;
}

bool gfps_sass_is_need_resume_after_call(void)
{
     return sassInfo.config.resume;
}

bool gfps_sass_is_accept_new_media(void)
{
     return sassInfo.config.acceptNew;
}

void gfps_sass_update_switch_dev(uint8_t oldActive, uint8_t newActive)
{
    //SASS_CONN_STATE_E state = gfps_sass_get_conn_state();
    if ((newActive == SASS_INVALID_DEV_ID) || (oldActive == newActive))
    {
        return;
    }

    SASS_REASON_E reason = gfps_sass_get_switch_reason(newActive);
    SassBtInfo *otherInfo = gfps_sass_get_other_connected_dev(newActive);
    SassBtInfo *sInfo = gfps_sass_get_connected_dev(newActive);

    TRACE(3,"sass_ntf_switch oldActive:%d, newActive:%d, reason:%d", oldActive, newActive, reason);
    if ((oldActive != SASS_INVALID_DEV_ID) && (newActive != SASS_INVALID_DEV_ID))
    {
        gfps_sass_ntf_switch_evt(newActive, reason);
        if (otherInfo && (otherInfo->connId != SASS_INVALID_DEV_ID))
        {
            gfps_sass_ntf_switch_evt(otherInfo->connId, reason);
        }
    }
    else //old = SASS_INVALID_DEV_ID, new !=SASS_INVALID_DEV_ID
    {
        //1. only 1 device; 2.there is no active history before
        if ((!otherInfo && !sInfo->isActived) || (otherInfo && !otherInfo->isActived))
        {
            gfps_sass_ntf_switch_evt(newActive, reason);
        }

        if (!gfps_sass_get_multi_status() && otherInfo && (otherInfo->connId != SASS_INVALID_DEV_ID))
        {
            gfps_sass_ntf_switch_evt(otherInfo->connId, reason);
        }
    }
}

void gfps_sass_check_nonsass_switch(uint8_t devId)
{
    SassPendingProc pending;
    uint8_t currId = SET_BT_ID(app_bt_audio_get_curr_playing_a2dp());
#if BLE_AUDIO_ENABLED
    if(currId == BT_DEVICE_INVALID_ID)
    {
        currId = app_audio_adm_get_le_audio_music_stream_device();
    }
#endif
    gfps_sass_get_pending_proc(&pending);
    if ((currId != SASS_INVALID_DEV_ID) && (currId != devId) && \
        (pending.proc == SASS_PROCESS_SWITCH_ACTIVE_SRC) && \
        (pending.activeId == devId) && \
        (pending.status & SASS_SWITCH_STATUS_REC_SWITCH))
    {
        gfps_sass_switch_media(currId, devId, true);
        gfps_sass_clear_pending_proc();
    }
}

void gfps_sass_set_pending_proc_status(uint8_t devId, SASS_SWITCH_STATUS_E status)
{
    SassPendingProc pending;
    gfps_sass_get_pending_proc(&pending);
    if (pending.proc == SASS_PROCESS_SWITCH_ACTIVE_SRC)
    {
        pending.activeId = devId;
        pending.status |= status;
    }
    else
    {
        pending.proc = SASS_PROCESS_SWITCH_ACTIVE_SRC;
        pending.activeId = devId;
        pending.status = status;
    }
    gfps_sass_set_pending_proc(&pending);
}

void gfps_sass_non_sass_switch(SassBtInfo *sInfo, SassBtInfo *otherInfo)
{
    if (!sInfo || !otherInfo)
    {
        return;
    }

    uint8_t devId = sInfo->connId;
    uint8_t currId = otherInfo->connId;
    bool isStreaming = gfps_sass_is_dev_media_streaming(sInfo);
    bool isExchanged = gfps_sass_is_profile_exchanged(sInfo);
    /* fix streaming keep for a few minutes*/
    if (isStreaming && isExchanged)
    {
        gfps_sass_clear_pending_proc();
        gfps_sass_switch_media(currId, devId, false);
    }
    else if (isStreaming && !isExchanged)
    {
        gfps_sass_set_pending_proc_status(devId, SASS_SWITCH_STATUS_REC_SWITCH);
        gfps_sass_set_pending_proc_status(devId, SASS_SWITCH_STATUS_STREAMING);
    }
    else
    {
        gfps_sass_set_pending_proc_status(devId, SASS_SWITCH_STATUS_REC_SWITCH);
    }
}

void gfps_sass_check_if_switch_media(SassBtInfo *sInfo, bool devIssass, SassBtInfo *otherInfo, bool otherIssass)
{
    uint8_t currId, devId;
    SassPendingProc pending;
    bool isDevMusic = false, isDevGame = false;

    if (sInfo == NULL)
    {
        return;
    }

    devId = sInfo->connId;
    currId = SET_BT_ID(app_bt_audio_get_curr_playing_a2dp());
#if BLE_AUDIO_ENABLED
    if(currId == BT_DEVICE_INVALID_ID)
    {
        currId = app_audio_get_curr_playing_le_devid();
    }
#endif

    if(IS_BT_DEVICE(devId))
    {
        TRACE(0,"bt connect");
        isDevMusic = GET_PROFILE_STATE(sInfo->audState, AUDIO, AVRCP);
        isDevGame = (GET_PROFILE_STATE(sInfo->audState, AUDIO, A2DP) & \
                        (!GET_PROFILE_STATE(sInfo->audState, AUDIO, AVRCP)));
    }
#if BLE_AUDIO_ENABLED
    else
    {
        TRACE(0,"le connect");
        isDevMusic = GET_PROFILE_STATE(sInfo->audState, MUSIC, LEA);
        isDevGame = GET_PROFILE_STATE(sInfo->audState, GAME, LEA);
    }
#endif

    TRACE(5,"%s, currId:%d, devId:%d, isDevMusic:%d, isDevGame:%d", __func__, currId, devId, isDevMusic, isDevGame);
    if (otherInfo)
    {
        POSSIBLY_UNUSED bool isOtherGame, isOtherCall, isOtherMusic;

        if(IS_BT_DEVICE(otherInfo->connId))
        {
            isOtherGame = (GET_PROFILE_STATE(otherInfo->audState, AUDIO, A2DP) & \
                              (!GET_PROFILE_STATE(otherInfo->audState, AUDIO, AVRCP)));
            
            isOtherMusic = (GET_PROFILE_STATE(otherInfo->audState, AUDIO, A2DP) & \
                            GET_PROFILE_STATE(otherInfo->audState, AUDIO, AVRCP));
    
            isOtherCall = GET_PROFILE_STATE(otherInfo->audState, AUDIO, HFP);
        }
        else
        {
            isOtherGame = GET_PROFILE_STATE(otherInfo->audState, GAME, LEA);
            isOtherMusic = GET_PROFILE_STATE(otherInfo->audState, MUSIC, LEA);
            isOtherCall = GET_PROFILE_STATE(otherInfo->audState, CALL, LEA);
        }
        TRACE(5, "%s otherIssass:%d devIssass:%d currId:%d iscall:%d", __func__, otherIssass, devIssass, currId, isOtherCall);

        if (isOtherCall && (isDevGame || isDevMusic))
        {
            gfps_sass_send_pause(sInfo);
        }
        else if ((currId != BT_DEVICE_INVALID_ID) && (currId != devId))
        {
            if (devIssass && otherIssass)
            {
                gfps_sass_get_pending_proc(&pending);
                TRACE(0,"pending.proc is %d, activeID is %d status 0x%0x", pending.proc, pending.activeId, pending.status);
                if ((pending.proc == SASS_PROCESS_SWITCH_ACTIVE_SRC) && \
                    (pending.activeId == devId) && \
                    (pending.status & SASS_SWITCH_STATUS_REC_SWITCH) && \
                    gfps_sass_is_profile_exchanged(sInfo))
                {
                    gfps_sass_clear_pending_proc();
                    gfps_sass_switch_media(currId, devId, false);
                }
                /*else if (IS_BT_DEVICE(sInfo->connId) && isDevGame && isOtherMusic)
                {
                    gfps_sass_send_pause(sInfo);
                }*/
                else
                {
                    TRACE(1, "%s waiting profile exchange or switch cmd", __func__);
                    pending.proc = SASS_PROCESS_SWITCH_ACTIVE_SRC;
                    pending.activeId = devId;
                    pending.status |= SASS_SWITCH_STATUS_STREAMING;
                    gfps_sass_set_pending_proc(&pending);
                }
            }
            else if (!devIssass)
            {
                uint8_t devKey[16] = {0};
                memset(devKey, 0, 16);
                nv_record_fp_get_key_by_addr(sInfo->btAddr.address, devKey);
                 //workaround for the case a2dp event come first before gfps connection.
                if (!gfps_sass_is_key_valid(devKey)) //dev B is non-SASS
                {
                    //dev A is SASS or do not accept new media
                    if (otherIssass || !gfps_sass_is_accept_new_media())
                    {
                        /*waiting for the lea streaming to stop*/
                        if (IS_BT_DEVICE(otherInfo->connId) ||
                            gfps_sass_is_dev_media_streaming(otherInfo))
                        {
                            gfps_sass_send_pause(sInfo);
                        }
                    }
                    else // dev A is non-SASS and accept new media
                    {
                        gfps_sass_non_sass_switch(sInfo, otherInfo);
                    }
                }
                else if (isDevMusic || (isDevGame && isOtherGame))
                {
                    SassPendingProc pending;
                    pending.proc = SASS_PROCESS_SWITCH_ACTIVE_SRC;
                    pending.activeId = sInfo->connId;
                    pending.status = SASS_SWITCH_STATUS_STREAMING;
                    gfps_sass_set_pending_proc(&pending);
                }
                else
                {
                    TRACE(1, "%s wait a2dp or avrcp event or sass event!!", __func__);
                }
            }
            else//B is sass, and A is non-SASS
            {
                gfps_sass_send_pause(sInfo);
            }
         }
         else
         {
             gfps_sass_update_active_info(sInfo);
         }
    }
    else
    {
        gfps_sass_update_active_info(sInfo);
    }
}

bool gfps_sass_select_steal_device(const bt_bdaddr_t *conn_req_addr, bt_bdaddr_t *steal_addr)
{
    SassBtInfo *sInfo= NULL;
    uint8_t activeId = gfps_sass_get_active_dev();
    bool ret = false;
    TRACE(2, "%s activeId:%d", __func__, activeId);
    if (activeId == SASS_INVALID_DEV_ID)
    {
        activeId = gfps_sass_get_last_active_dev();
    }

    if (activeId != SASS_INVALID_DEV_ID)
    {
        sInfo = gfps_sass_get_other_connected_dev(activeId);
        if (sInfo)
        {
            memcpy(steal_addr, sInfo->btAddr.address, sizeof(bt_bdaddr_t));
            ret = true;
        }
    }

    TRACE(1, "%s steal addr is:", __func__);
    DUMP8("%2x ", (uint8_t *)steal_addr, sizeof(bt_bdaddr_t));
    return ret;
}

bt_bdaddr_t *gfps_sass_select_disconnect_device(bt_bdaddr_t *incomming_addr)
{
    SassBtInfo *sInfo= NULL;
    uint8_t activeId = gfps_sass_get_active_dev();
    bt_bdaddr_t *stealAddr = NULL;

    if (activeId == SASS_INVALID_DEV_ID)
    {
        activeId = gfps_sass_get_last_active_dev();
    }

    if (activeId != SASS_INVALID_DEV_ID)
    {
        sInfo = gfps_sass_get_other_connected_dev(activeId);
        if (sInfo)
        {
            stealAddr = &(sInfo->btAddr);
            TRACE(1, "%s steal addr is:", __func__);
            DUMP8("%2x ", (uint8_t *)stealAddr, sizeof(bt_bdaddr_t));
        }
    }

    return stealAddr;
}

bool gfps_sass_is_dev_playing_state(SassBtInfo *sInfo)
{
    bool ret = false;
    if (!sInfo)
    {
        return ret;
    }

    if (IS_BT_DEVICE(sInfo->connId))
    {
        ret = GET_PROFILE_STATE(sInfo->audState, AUDIO, AVRCP);
    }
#if BLE_AUDIO_ENABLED
    else
    {
        AOB_MEDIA_INFO_T *p_media_info = ble_audio_earphone_info_get_media_info(sInfo->connId);
        if (p_media_info && (p_media_info->media_state = MCS_MEDIA_STATE_PLAYING) && \
          (GET_PROFILE_STATE(sInfo->audState, MUSIC, LEA) || GET_PROFILE_STATE(sInfo->audState, GAME, LEA)))
        {
            ret = true;
        }
    }
#endif
    return ret;
}

void gfps_sass_send_play(SassBtInfo *sInfo)
{
    if(IS_BT_DEVICE(sInfo->connId))
    {
        app_bt_ibrt_audio_play_a2dp_stream(GET_BT_ID(sInfo->connId));
    }
#if BLE_AUDIO_ENABLED
    else
    {
        bes_ble_aob_media_play(sInfo->connId);
    }
#endif
    return;
}

void gfps_sass_hf_connect_handler(SassBtInfo *sInfo, SassBtInfo* otherInfo)
{
    if (!sInfo)
    {
        return;
    }

    uint8_t oldHfpState = GET_PROFILE_STATE(sInfo->audState, AUDIO, HFP);

    SET_PROFILE_STATE(sInfo->audState, AUDIO, HFP, 1);
    if (GET_PROFILE_STATE(sInfo->audState, AUDIO, A2DP))
    {
        SET_PROFILE_STATE(sInfo->audState, AUDIO, A2DP, 0);
    }

    if (GET_PROFILE_STATE(sInfo->audState, AUDIO, AVRCP))
    {
        SET_PROFILE_STATE(sInfo->audState, AUDIO, AVRCP, 0);
    }

    if(otherInfo)
    {
        if(!gfps_sass_is_dev_calling(otherInfo))
        {
            if (!oldHfpState && gfps_sass_is_dev_media_ready(otherInfo))
            {
                gfps_sass_send_pause(otherInfo);
                if (gfps_sass_is_need_resume_after_call())
                {
                    if (gfps_sass_is_dev_music(otherInfo))
                    {
                        gfps_sass_set_resume_dev(otherInfo->connId, true);
                    }
                    else
                    {
                        gfps_sass_set_resume_dev(otherInfo->connId, false);
                    }
                }
            }
            gfps_sass_update_active_info(sInfo);
        }
        else
        {
            app_ibrt_if_hf_disc_audio_link(GET_BT_ID(sInfo->connId));
        }
    }
    else
    {
        gfps_sass_update_active_info(sInfo);
    }
}

void gfps_sass_hf_disconnect_handler(SassBtInfo *sInfo, SassBtInfo* otherInfo)
{
    uint8_t activeId = gfps_sass_get_active_dev();

    if (GET_PROFILE_STATE(sInfo->audState, AUDIO, HFP))
    {
        SET_PROFILE_STATE(sInfo->audState, AUDIO, HFP, 0);
        if (otherInfo)
        {
            bool isResumeMusic = gfps_sass_is_resume_dev_music();
            bool isOtherMusic = gfps_sass_is_dev_music(otherInfo);
            bool isOtherStreaming = gfps_sass_is_dev_media_streaming(otherInfo);

            if ((isResumeMusic && isOtherMusic && isOtherStreaming) ||
                (!isResumeMusic && isOtherStreaming))
            {
                gfps_sass_update_active_info(otherInfo);
                gfps_sass_set_resume_dev(SASS_INVALID_DEV_ID, false);
            }
            else
            {
                if (gfps_sass_is_need_resume_after_call() && \
                    gfps_sass_get_resume_dev() == otherInfo->connId)
                {
                    gfps_sass_send_play(otherInfo);
                }
            }
        }
        else
        {
            if (activeId == sInfo->connId)
            {
                gfps_sass_set_active_dev(SASS_INVALID_DEV_ID);
            }
        }
        sInfo->isCallsetup = false;
    }
}

#if BLE_AUDIO_ENABLED
void gfps_sass_lea_release_media(uint8_t conidx)
{
    uint8_t ase_lid_list[4] = {0};
    uint8_t nb_ase = 0;

    nb_ase = aob_media_get_ready_for_stream_ase_lid_list(conidx, ase_lid_list);
    for (int i = 0; i < nb_ase; i++)
    {
        //aob_media_release_stream(ase_lid_list[i]);
        aob_media_disable_stream(ase_lid_list[i]);
    }
}

void gfps_sass_lea_call_connect_handler(SassBtInfo *sInfo, SassBtInfo* otherInfo)
{
    if (!sInfo)
    {
        return;
    }

    uint8_t oldHfpState = GET_PROFILE_STATE(sInfo->audState, CALL, LEA);

    SET_PROFILE_STATE(sInfo->audState, CALL, LEA, 1);
    SET_PROFILE_STATE(sInfo->audState, CONNECTION, LEA, 1);
    if (GET_PROFILE_STATE(sInfo->audState, GAME, LEA))
    {
        SET_PROFILE_STATE(sInfo->audState, GAME, LEA, 0);
    }

    if (GET_PROFILE_STATE(sInfo->audState, MUSIC, LEA))
    {
        SET_PROFILE_STATE(sInfo->audState, MUSIC, LEA, 0);
    }

     if(otherInfo)
    {
        if(!gfps_sass_is_dev_calling(otherInfo))
        {
            if (!oldHfpState && gfps_sass_is_dev_media_ready(otherInfo))
            {
                gfps_sass_send_pause(otherInfo);
                if (gfps_sass_is_need_resume_after_call())
                {
                    if (gfps_sass_is_dev_music(otherInfo))
                    {
                        gfps_sass_set_resume_dev(otherInfo->connId, true);
                    }
                    else
                    {
                        gfps_sass_set_resume_dev(otherInfo->connId, false);
                    }
                }
            }
            gfps_sass_update_active_info(sInfo);
        }
        else
        {
            gfps_sass_lea_release_media(sInfo->connId);
        }
    }
    else
    {
        gfps_sass_update_active_info(sInfo);
    }
}

void gfps_sass_lea_call_disconnect_handler(SassBtInfo *sInfo, SassBtInfo* otherInfo)
{
    uint8_t activeId = gfps_sass_get_active_dev();

    if (GET_PROFILE_STATE(sInfo->audState, CALL, LEA))
    {
        SET_PROFILE_STATE(sInfo->audState, CALL, LEA, 0);
        if (otherInfo)
        {
            bool isResumeMusic = gfps_sass_is_resume_dev_music();
            bool isOtherMusic = gfps_sass_is_dev_music(otherInfo);
            bool isOtherStreaming = gfps_sass_is_dev_media_streaming(otherInfo);

            if ((isResumeMusic && isOtherMusic && isOtherStreaming) ||
                (!isResumeMusic && isOtherStreaming))
            {
                gfps_sass_update_active_info(otherInfo);
                gfps_sass_set_resume_dev(SASS_INVALID_DEV_ID, false);
            }
            else
            {
                if (gfps_sass_is_need_resume_after_call() && \
                    gfps_sass_get_resume_dev() == otherInfo->connId)
                {
                    gfps_sass_send_play(otherInfo);
                }
            }
        }
        else
        {
            if (activeId == sInfo->connId)
            {
                gfps_sass_set_active_dev(SASS_INVALID_DEV_ID);
            }
        }
    }
}
#endif

void gfps_sass_profile_event_handler(uint8_t pro, uint8_t devId, bt_bdaddr_t *btAddr, 
                                                   uint8_t event, uint8_t *param)
{
    bool needUpdate = true;
    bool needResume = false;
    bool devIssass = false;
    bool otherIssass = false;
    SASS_CONN_STATE_E tempState;
    SASS_CONN_STATE_E updateState;
    SassBtInfo *sInfo = NULL;
    uint8_t oldActive = gfps_sass_get_active_dev();
    SassPendingProc pending;
    bt_bdaddr_t zero_bdaddr = {{0}};
    bt_bdaddr_t invalid_bdaddr = {{0xFF}};
    bool isAddrMatch = true, isAddrValid = false;

    if (btAddr && memcmp(btAddr->address, zero_bdaddr.address, 6) &&
        memcmp(btAddr->address, invalid_bdaddr.address, 6))
    {
        sInfo = gfps_sass_get_connected_dev_by_addr(btAddr);
        isAddrValid = true;
    }

    //the address is error
    if (!sInfo)
    {
        isAddrMatch = false;
        sInfo = gfps_sass_get_connected_dev(devId);
        if (!sInfo) //for profile event comes first before connection event
        {
            gfps_sass_connect_handler(devId, (bt_bdaddr_t *)btAddr);
            gfps_sass_send_session_nonce(devId);
            gfps_sass_ntf_conn_status(devId, true, NULL);
        }
    }

    sInfo = gfps_sass_get_connected_dev(devId);
    if(!sInfo)
    {
        return;
    }
    else
    {
        if (isAddrValid && !isAddrMatch)
        {
            memcpy(sInfo->btAddr.address, btAddr->address, 6);
        }
    }

    TRACE(0,"sass_profile_event sInfo:%p, id:%d event:%d pro:%d avtiveId:%d, btAddr:%0x:%0x",
          sInfo, devId, event, pro, oldActive, btAddr->address[0], btAddr->address[5]);

    SassBtInfo *otherInfo = gfps_sass_get_other_connected_dev(devId);
    if (otherInfo)
    {
        otherIssass = gfps_sass_is_sass_dev(otherInfo);
    }
    needResume = gfps_sass_is_need_resume(&(sInfo->btAddr));
    devIssass = gfps_sass_is_sass_dev(sInfo);

    if (pro == SASS_PROFILE_A2DP)
    {
        //SassBtInfo *otherInfo = NULL;
        switch(event)
        {
            case BTIF_A2DP_EVENT_STREAM_OPEN:
            case BTIF_A2DP_EVENT_STREAM_OPEN_MOCK:
                SET_PROFILE_STATE(sInfo->audState, CONNECTION, A2DP, 1);
                if (gfps_sass_get_active_dev() == SASS_INVALID_DEV_ID || \
                    (otherInfo && !GET_PROFILE_STATE(otherInfo->audState, AUDIO, A2DP) && \
                    !GET_PROFILE_STATE(otherInfo->audState, AUDIO, HFP) && \
                    !GET_PROFILE_STATE(otherInfo->audState, AUDIO, AVRCP)))
                {
                    //gfps_sass_set_active_dev(devId);
                }
                break;

            case BTIF_A2DP_EVENT_STREAM_CLOSED:
                SET_PROFILE_STATE(sInfo->audState, AUDIO, A2DP, 0);
                SET_PROFILE_STATE(sInfo->audState, CONNECTION, A2DP, 0);
                gfps_sass_get_pending_proc(&pending);
                if (pending.activeId == devId)
                {
                    gfps_sass_clear_pending_proc();
                }

                if (oldActive == devId)
                {
                    gfps_sass_set_active_dev(SASS_INVALID_DEV_ID);
                }

                if (gfps_sass_get_resume_dev() == devId)
                {
                    gfps_sass_set_resume_dev(SASS_INVALID_DEV_ID, false);
                }
                break;

            case BTIF_A2DP_EVENT_STREAM_STARTED:
            case BTIF_A2DP_EVENT_STREAM_STARTED_MOCK:
                if(!GET_PROFILE_STATE(sInfo->audState, AUDIO, A2DP))
                {
                    SET_PROFILE_STATE(sInfo->audState, AUDIO, A2DP, 1);
                    gfps_sass_check_if_switch_media(sInfo, devIssass, otherInfo, otherIssass);
                }
                break;

            case BTIF_A2DP_EVENT_STREAM_SUSPENDED:
                SET_PROFILE_STATE(sInfo->audState, AUDIO, A2DP, 0);
                gfps_sass_get_pending_proc(&pending);
                if (pending.activeId == devId)
                {
                    gfps_sass_clear_pending_proc();
                }
                if (oldActive == devId)
                {
                    gfps_sass_set_active_dev(SASS_INVALID_DEV_ID);
                }
                break;
            default:
                needUpdate = false;
                break;
        }
    }
    else if (pro == SASS_PROFILE_HFP)
    {
        switch(event)
        {
            case BTIF_HF_EVENT_CALLSETUP_IND:
                TRACE(2, "%s hfp state:%d", __func__, param ? param[0] : 0);
                if (!param)
                {
                    needUpdate = false;
                }
                else if (param[0])
                {
                    if (!GET_PROFILE_STATE(sInfo->audState, AUDIO, HFP))
                    {
                        sInfo->isCallsetup = true;
                    }
                    gfps_sass_hf_connect_handler(sInfo, otherInfo);
                }
                else
                {
                    if (sInfo->isCallsetup && !sInfo->isCallInd)
                    {
                        gfps_sass_hf_disconnect_handler(sInfo, otherInfo);
                    }
                }
                break;

            case BTIF_HF_EVENT_AUDIO_CONNECTED:
                sInfo->isCallsetup = false;
                gfps_sass_hf_connect_handler(sInfo, otherInfo);
                break;

            case BTIF_HF_EVENT_CALL_IND:
            case BTIF_HF_EVENT_AUDIO_DISCONNECTED:
                if (event == BTIF_HF_EVENT_CALL_IND && param)
                {
                    sInfo->isCallInd = param[0];
                    if (param[0])
                    {
                        needUpdate = false;
                        break;
                    }
                }
                gfps_sass_hf_disconnect_handler(sInfo, otherInfo);
                break;

            case BTIF_HF_EVENT_SERVICE_DISCONNECTED:
                SET_PROFILE_STATE(sInfo->audState, CONNECTION, HFP, 0);
                if (oldActive == devId)
                {
                    gfps_sass_set_active_dev(SASS_INVALID_DEV_ID);
                }
                gfps_sass_clear_reject_hf_dev(devId);
                sInfo->isCallsetup = false;
                sInfo->isCallInd = false;
                break;

            case BTIF_HF_EVENT_SERVICE_CONNECTED:
            case BTIF_HF_EVENT_SERVICE_MOCK_CONNECTED:
                SET_PROFILE_STATE(sInfo->audState, CONNECTION, HFP, 1);
                sInfo->isCallsetup = false;
                sInfo->isCallInd = false;
                break;

            default:
            needUpdate = false;
            break;
        }
    }
    else if (pro == SASS_PROFILE_AVRCP)
    {
        switch(event)
        {
            case BTIF_AVCTP_CONNECT_EVENT:
            case BTIF_AVCTP_CONNECT_EVENT_MOCK:
                SET_PROFILE_STATE(sInfo->audState, CONNECTION, AVRCP, 1);
                sInfo->waitPauseDone = false;
                sInfo->isNeedResume = false;
                break;

            case BTIF_AVCTP_DISCONNECT_EVENT:
                SET_PROFILE_STATE(sInfo->audState, CONNECTION, AVRCP, 0);
                SET_PROFILE_STATE(sInfo->audState, AUDIO, AVRCP, 0);
                sInfo->waitPauseDone = false;
                sInfo->isNeedResume = false;
                break;

            case BTIF_AVRCP_EVENT_ADV_NOTIFY:
            case BTIF_AVRCP_EVENT_ADV_RESPONSE:
                TRACE(2, "%s avrcp state:%d", __func__, *param);
                if (*param == BTIF_AVRCP_MEDIA_PLAYING) {
                    if (!GET_PROFILE_STATE(sInfo->audState, AUDIO, AVRCP))
                    {
                        SET_PROFILE_STATE(sInfo->audState, AUDIO, AVRCP, 1);             
                        gfps_sass_check_if_switch_media(sInfo, devIssass, otherInfo, otherIssass);
                    }
                    sInfo->waitPauseDone = false;
                    sInfo->isNeedResume = false;
                } else if ((*param == BTIF_AVRCP_MEDIA_PAUSED || *param == BTIF_AVRCP_MEDIA_STOPPED)) {
                    SET_PROFILE_STATE(sInfo->audState, AUDIO, AVRCP, 0);
                    sInfo->waitPauseDone = false;
                    sInfo->isNeedResume = false;
                } else {
                    needUpdate = false;
                }
                break;

            default:
                needUpdate = false;
                break;
        }
    }
#if BLE_AUDIO_ENABLED
    else if (pro == SASS_PROFILE_LEA)
    {
        SassLeaParam *leaParam = (SassLeaParam *)param;
        AOB_MGR_CONTEXT_TYPE_BF_E contextType = aob_media_get_cur_context_type_by_ase_lid(leaParam->aseLid);
        TRACE(5, "%s event %d,leaParam->state:%d aseId:%d type 0x%0x", __func__, event,
                leaParam->state, leaParam->aseLid, contextType);
        switch(event)
        {
            case SASS_EVENT_LEA_PLAYBACK_STATE:
                sInfo->waitPauseDone = false;
                sInfo->isNeedResume = false;
                if (leaParam->state == AOB_MGR_PLAYBACK_STATE_PLAYING)
                {
                    //if(AOB_AUDIO_CONTEXT_TYPE_MEDIA & contextType)
                    {
                        SET_PROFILE_STATE(sInfo->audState, MUSIC, LEA, 1);
                        SET_PROFILE_STATE(sInfo->audState, CONNECTION, LEA, 1);
                        if(aob_media_get_cur_streaming_ase_lid(devId,AOB_MGR_DIRECTION_SINK) != SASS_INVALID_DEV_ID)
                        {
                             gfps_sass_check_if_switch_media(sInfo, devIssass, otherInfo, otherIssass);
                        }
                    }
                }
                else
                {
                    SET_PROFILE_STATE(sInfo->audState, MUSIC, LEA, 0);
                    SET_PROFILE_STATE(sInfo->audState, CONNECTION, LEA, 1);
                    TRACE(2, "oldActive is %d, devId is %d", oldActive, devId);
                    if (oldActive == devId)
                    {
                        gfps_sass_set_active_dev(SASS_INVALID_DEV_ID);
                    }
                }
                break;

            case SASS_EVENT_LEA_METADATA_UPDATA_STATE:
            case SASS_EVENT_LEA_STREAM_STATE:
#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
                if ((event == SASS_EVENT_LEA_STREAM_STATE) &&
                    (leaParam->state == AOB_MGR_STREAM_STATE_STREAMING ||
                     leaParam->state == AOB_MGR_STREAM_STATE_DISABLING ||
                     leaParam->state == AOB_MGR_STREAM_STATE_RELEASING) &&
                    (TWS_UI_SLAVE == app_ibrt_if_get_ui_role()))
                {
                    gfps_send_streaming_state_to_master(&(sInfo->btAddr), leaParam->state);
                }
#endif
                if (leaParam->state == AOB_MGR_STREAM_STATE_STREAMING && \
                    ((AOB_AUDIO_CONTEXT_TYPE_GAME | AOB_AUDIO_CONTEXT_TYPE_LIVE | \
                      AOB_AUDIO_CONTEXT_TYPE_IMMEDIATE_ALERT | AOB_AUDIO_CONTEXT_TYPE_INSTRUCTIONAL) & \
                     contextType))
                {
                    SET_PROFILE_STATE(sInfo->audState, GAME, LEA, 1);
                    SET_PROFILE_STATE(sInfo->audState, CONNECTION, LEA, 1);
                    gfps_sass_check_if_switch_media(sInfo, devIssass, otherInfo, otherIssass);
                }
                else if(leaParam->state == AOB_MGR_STREAM_STATE_STREAMING && \
                    (AOB_AUDIO_CONTEXT_TYPE_CONVERSATIONAL & contextType || \
                     AOB_AUDIO_CONTEXT_TYPE_EMERGENCY_ALERT == contextType || \
                     AOB_AUDIO_CONTEXT_TYPE_MAN_MACHINE == contextType))
                {
                    gfps_sass_lea_call_connect_handler(sInfo, otherInfo);
                }
                else if(leaParam->state == AOB_MGR_STREAM_STATE_STREAMING && \
                    (AOB_AUDIO_CONTEXT_TYPE_MEDIA & contextType))
                {
                    SET_PROFILE_STATE(sInfo->audState, MUSIC, LEA, 1);
                    SET_PROFILE_STATE(sInfo->audState, CONNECTION, LEA, 1);
                    gfps_sass_check_if_switch_media(sInfo, devIssass, otherInfo, otherIssass);
                }
                else if(leaParam->state == AOB_MGR_STREAM_STATE_STREAMING && \
                    (AOB_AUDIO_CONTEXT_TYPE_SOUND_EFFECT == contextType || \
                     AOB_AUDIO_CONTEXT_TYPE_ATTENTION_SEEKING == contextType ))
                {
                    needUpdate = false;
                }
                else if(leaParam->state == AOB_MGR_STREAM_STATE_STREAMING && \
                        GET_PROFILE_STATE(sInfo->audState, CALL, LEA) && \
                    ((AOB_AUDIO_CONTEXT_TYPE_CONVERSATIONAL & contextType) == 0))
                {
                    needUpdate = false;
                }
                else if(leaParam->state == AOB_MGR_STREAM_STATE_DISABLING || \
                        leaParam->state == AOB_MGR_STREAM_STATE_RELEASING)
                {
                    gfps_sass_get_pending_proc(&pending);
                    if ((pending.activeId == devId) && !(AOB_AUDIO_CONTEXT_TYPE_SOUND_EFFECT == contextType))
                    {
                        gfps_sass_clear_pending_proc();
                    }

                    if(GET_PROFILE_STATE(sInfo->audState, CALL, LEA))
                    {
                        gfps_sass_lea_call_disconnect_handler(sInfo, otherInfo);
                    }
                    else
                    {
                        if(GET_PROFILE_STATE(sInfo->audState, MUSIC, LEA))
                        {
                            SET_PROFILE_STATE(sInfo->audState, MUSIC, LEA, 0);
                        }
                        if(GET_PROFILE_STATE(sInfo->audState, GAME, LEA))
                        {
                            SET_PROFILE_STATE(sInfo->audState, GAME, LEA, 0);
                        }

                        if (oldActive == devId)
                        {
                            gfps_sass_set_active_dev(SASS_INVALID_DEV_ID);
                        }
                    }
                    sInfo->waitPauseDone = false;
                    sInfo->isNeedResume = false;
                }
                else
                {
                    TRACE(2,"leaParam->state is %d, media type is %d ", leaParam->state, contextType);
                    if (otherInfo && (!devIssass || !otherIssass) &&
                        leaParam->state == AOB_MGR_STREAM_STATE_ENABLING &&
                        gfps_sass_is_dev_streaming(otherInfo->connId))
                    {
                        if ((devIssass != otherIssass) || !gfps_sass_is_accept_new_media())
                        {
                            gfps_sass_send_pause(sInfo);
                        }
                        else // dev A is non-SASS and accept new media
                        {
                            gfps_sass_send_pause(otherInfo);
                        }
                    }

                   /* if(leaParam->state == AOB_MGR_STREAM_STATE_STREAMING)
                    {
                        //it is media
                        if( GET_PROFILE_STATE(sInfo->audState, MUSIC, LEA))
                        {
                            //check if switch to le a2dp
                            gfps_sass_check_if_switch_media(sInfo, devIssass, otherInfo, otherIssass);
                            sInfo->waitPauseDone = false;
                            sInfo->isNeedResume = false;
                        }
                        else
                        {
                            SET_PROFILE_STATE(sInfo->audState, MUSIC, LEA, 1);
                        }
                    }*/
                }
                break;

            case SASS_EVENT_LEA_CALL_STATE:
                if (leaParam->state != AOB_CALL_STATE_IDLE)
                {
                    gfps_sass_lea_call_connect_handler(sInfo, otherInfo);
                }
                else
                {
                    gfps_sass_lea_call_disconnect_handler(sInfo, otherInfo);
                    if (oldActive == devId)
                    {
                        gfps_sass_set_active_dev(SASS_INVALID_DEV_ID);
                    }
                }
                break;

            default:
                needUpdate = false;
                break;
        }
    }
#endif
    else
    {
        needUpdate = false;
        TRACE(1,"%s sass profile update error", __func__);
    }

    if (needUpdate)
    {
        uint8_t newActive;
        SassEvtParam evtParam;
        evtParam.devId = devId;

        tempState = gfps_sass_get_conn_state();
        updateState = gfps_sass_update_conn_state(sInfo);
        newActive = gfps_sass_get_active_dev();
        TRACE(4,"%s sass audState: 0x%0x, temp:0x%0x, state:0x%0x", __func__, sInfo->audState, tempState, updateState);

        if (tempState != updateState)
        {
            evtParam.event = SASS_EVT_UPDATE_CONN_STATE;
            evtParam.state.connState = updateState;
            gfps_sass_update_state(&evtParam);
        }
        else if ((tempState == updateState) && (oldActive != newActive))
        {
            evtParam.event = SASS_EVT_UPDATE_ACTIVE_DEV;
            gfps_sass_update_state(&evtParam);
        }
        else
        {
        }

    }

    if (needResume && GET_PROFILE_STATE(sInfo->audState, CONNECTION, A2DP) && \
        GET_PROFILE_STATE(sInfo->audState, CONNECTION, AVRCP))
    {
        app_bt_resume_music_player(GET_BT_ID(devId));
        memset(sassInfo.reconnInfo.reconnAddr.address, 0, sizeof(bt_bdaddr_t));
        sassInfo.reconnInfo.evt = 0xFF;
    }
}

uint8_t gfps_sass_switch_src_evt_hdl(uint8_t device_id, uint8_t evt)
{
    uint8_t reason = SASS_STATUS_OK;
    uint8_t awayId, switchId, currentId = SASS_INVALID_DEV_ID, otherId = SASS_INVALID_DEV_ID;
    SassBtInfo *info = gfps_sass_get_other_connected_dev(device_id);
    if (info)
    {
        otherId = info->connId;
    }

    TRACE(3,"gfps_sass_switch_src_evt_hdl, device_id:%d, evt:%x, otherId:%d", device_id, evt, otherId);

    if (evt & SASS_SWITCH_TO_CURR_DEV_BIT)
    {
        switchId = device_id;
        awayId = otherId;
    }
    else
    {
        switchId = otherId;
        awayId = device_id;
    }

    if(evt & SASS_DISCONN_ON_AWAY_DEV_BIT)
    {
        sassInfo.IfSendSwitchNTF = false;
    }
    else
    {
        if(!(evt & SASS_SWITCH_TO_CURR_DEV_BIT))
        {
            sassInfo.IfSendSwitchNTF = false;
        }
    }

    if (switchId != SASS_INVALID_DEV_ID)
    {
        SassBtInfo *tInfo = gfps_sass_get_connected_dev(switchId);
        currentId = SET_BT_ID(app_bt_audio_get_curr_playing_a2dp());
#if BLE_AUDIO_ENABLED
        if(currentId == BT_DEVICE_INVALID_ID)
        {
            currentId = app_ble_get_curr_play_bleaudio_id();
        }
#endif
        TRACE(3, "sass switch src to %d from %d, other:%d, evt:0x%0x curr id:%d", switchId, awayId, otherId, evt, currentId);

        if (currentId == switchId || \
            (tInfo && gfps_sass_is_disconnect_dev(&(tInfo->btAddr)))) {
            return SASS_STATUS_REDUNTANT;
        } 
        else
        {
            if (evt & SASS_DISCONN_ON_AWAY_DEV_BIT)
            {
                SassBtInfo *awayinfo = gfps_sass_get_connected_dev(awayId);
                if (awayinfo)
                {
                    gfps_sass_update_last_dev(&(awayinfo->btAddr));
                    gfps_sass_update_last_dev_connId(awayId);
                    gfps_sass_set_disconnecting_dev((bt_bdaddr_t *)&(awayinfo->btAddr));
#ifdef BT_SERVICE_ENABLE
                    app_dev_mgr_user_disconnect_dev((bt_bdaddr_t *)&(awayinfo->btAddr));
#else
                    app_ui_destroy_device((bt_bdaddr_t *)&(awayinfo->btAddr), false);
#endif
                }

                 if (!gfps_sass_is_dev_playing_state(tInfo) &&
                     (evt & SASS_RESUME_ON_SWITCH_DEV_BIT))
                 {
                     gfps_sass_send_play(tInfo);
                 }
            }
            else
            {
                // if (!gfps_sass_is_dev_playing_state(tInfo) &&
                //     (evt & SASS_RESUME_ON_SWITCH_DEV_BIT))
                // {
                //     gfps_sass_send_play(tInfo);
                // }

                if(gfps_sass_is_dev_streaming(switchId) &&
                   gfps_sass_is_profile_exchanged(tInfo) &&
                   (IS_BT_DEVICE(tInfo->connId) || (GET_PROFILE_STATE(tInfo->audState, MUSIC, LEA) || GET_PROFILE_STATE(tInfo->audState, GAME, LEA))))
                {
                    SassPendingProc pending;
                    gfps_sass_get_pending_proc(&pending);
                    if ((pending.proc == SASS_PROCESS_SWITCH_ACTIVE_SRC) &&
                        (pending.activeId == switchId))
                    {
                        gfps_sass_clear_pending_proc();
                    }
                    gfps_sass_switch_media(currentId, switchId, true);
                    goto set_hf;
                }
                else
                {
                    TRACE(0,"to set pending proc");
                    SassPendingProc pending;
                    pending.proc = SASS_PROCESS_SWITCH_ACTIVE_SRC;
                    pending.status = SASS_SWITCH_STATUS_REC_SWITCH;
                    pending.activeId = switchId;
                    gfps_sass_set_pending_proc(&pending);
                }
            }
       }
    }
    else
    {
        if(awayId != SASS_INVALID_DEV_ID)
        {
            if (IS_BT_DEVICE(awayId))
            {
                if (app_bt_is_a2dp_streaming(GET_BT_ID(awayId)))
                {
                    app_bt_audio_stop_a2dp_playing(GET_BT_ID(awayId));
                }
            }
#if BLE_AUDIO_ENABLED
            else
            {
                if (aob_media_is_device_any_ase_in_streamimg_state(awayId))
                {
                    bes_ble_aob_media_pause(awayId);
                }
            }
#endif
        }
    }

set_hf:
    if (evt & SASS_REJECT_SCO_ON_AWAY_DEV_BIT)
    {
        if(IS_BT_DEVICE(awayId))
        {
            app_bt_hf_set_reject_dev(GET_BT_ID(awayId));
        }
#if BLE_AUDIO_ENABLED
        else
        {
            app_bt_hf_set_reject_dev(awayId);
        }
#endif
    }
    return reason;
}

uint8_t gfps_sass_switch_back_evt_hdl(uint8_t device_id, uint8_t evt)
{
    bt_bdaddr_t currAddr, lastAddr;
    uint8_t lastconnId;
    uint8_t empty[6] = {0};
#if BLE_AUDIO_ENABLED
    uint8_t con_lid[AOB_COMMON_MOBILE_CONNECTION_MAX];
#endif
    sassInfo.IfSendSwitchNTF = false;

    if ((evt != SASS_EVT_SWITCH_BACK) && (evt != SASS_EVT_SWITCH_BACK_AND_RESUME))
    {
        return  SASS_STATUS_FAIL;
    }

    if(IS_BT_DEVICE(device_id))
    {
        app_bt_get_device_bdaddr(GET_BT_ID(device_id), currAddr.address);
    }
#if BLE_AUDIO_ENABLED
    else
    {
        ble_bdaddr_t GetPeerAddr = {{0}};
        app_ble_get_peer_solved_addr(device_id, &GetPeerAddr);
        memcpy(currAddr.address, GetPeerAddr.addr, 6);
    }
#endif
    gfps_sass_get_last_dev(&lastAddr);
    lastconnId = gfps_sass_get_last_dev_connId();

    if (!memcmp(lastAddr.address, empty, sizeof(bt_bdaddr_t)))
    {
        TRACE(0, "sass switch back hdl last dev is NULL!");
    }
    else if (memcmp(lastAddr.address, currAddr.address, sizeof(bt_bdaddr_t)))
    {
        TRACE(0, "sass switch back to dev:");
        DUMP8("%02x ", lastAddr.address, 6);
        if(IS_BT_DEVICE(lastconnId))
        {
            if (!app_bt_is_acl_connected_byaddr((bt_bdaddr_t *)&lastAddr))
            {
                uint8_t maxLink = gfps_sass_get_multi_status() ? SASS_MAX_DEVICE_NUM : 1;
#if BLE_AUDIO_ENABLED
                if (app_bt_count_connected_device() + ble_audio_get_mobile_connected_dev_lids(con_lid) >= maxLink)
#else
                if(app_bt_count_connected_device() >= maxLink)
#endif
                {
                    gfps_sass_set_reconnecting_dev(&lastAddr, evt);
                    gfps_sass_set_disconnecting_dev(&currAddr);
#ifdef BT_SERVICE_ENABLE
                    app_dev_mgr_user_disconnect_dev(&currAddr);
#else
                    app_ui_destroy_device((bt_bdaddr_t *)&(currAddr), false);
#endif
                }
                else
                {
                    app_ibrt_if_choice_mobile_connect((bt_bdaddr_t *)&lastAddr, SASS_CONNECT_COUNT);
                }
            }
            else if (evt == SASS_EVT_SWITCH_BACK_AND_RESUME && gfps_sass_is_profile_connected(&lastAddr))
            {
                SassBtInfo *lastInfo = gfps_sass_get_connected_dev_by_addr(&lastAddr);
                uint8_t lastId = lastInfo->connId;
                uint8_t currId;
                bool isExchanged = gfps_sass_is_profile_exchanged(lastInfo);

                currId = SET_BT_ID(app_bt_audio_get_curr_playing_a2dp());
#if BLE_AUDIO_ENABLED
                if(currId == BT_DEVICE_INVALID_ID)
                {
                    currId = app_ble_get_curr_play_bleaudio_id();
                }
#endif

                if (isExchanged && (currId != lastId && currId != SASS_INVALID_DEV_ID) && \
                    app_bt_is_a2dp_streaming(GET_BT_ID(lastId)) && \
                    GET_PROFILE_STATE(lastInfo->audState, AUDIO, AVRCP))
                {
                    gfps_sass_switch_media(currId, lastId, true);
                    lastInfo->isNeedResume = true;
                }
                else if (!isExchanged)
                {
                    TRACE(1, "%s waiting profile exchange", __func__);
                    SassPendingProc pending;
                    pending.proc = SASS_PROCESS_SWITCH_ACTIVE_SRC;
                    pending.activeId = lastId;
                    pending.status = SASS_SWITCH_STATUS_REC_SWITCH;
                    gfps_sass_set_pending_proc(&pending);
                }
                else if (!GET_PROFILE_STATE(lastInfo->audState, AUDIO, AVRCP))
                {
                    app_bt_ibrt_audio_play_a2dp_stream(GET_BT_ID(lastId));
                }
                else
                {
                    // do nothing.
                }
            }
            else
            {
                TRACE(0, "sass switch back already connect!!");
            }
        }
#if BLE_AUDIO_ENABLED
        else
        {
            if(!bes_ble_gap_is_remote_dev_connected((ble_bdaddr_t *)&lastAddr))
            {
                uint8_t maxLink = gfps_sass_get_multi_status() ? SASS_MAX_DEVICE_NUM : 1;
                if (app_bt_count_connected_device() + ble_audio_get_mobile_connected_dev_lids(con_lid) >= maxLink)
                {
                    gfps_sass_set_reconnecting_dev(&lastAddr, evt);
                    gfps_sass_set_disconnecting_dev(&currAddr);
#ifdef BT_SERVICE_ENABLE
                    app_dev_mgr_user_disconnect_dev(&currAddr);
#else
                    app_ui_destroy_device((bt_bdaddr_t *)&(currAddr), false);
#endif
                }
                else
                {
                    //phone connect the le device
                }
            }
            else if(evt == SASS_EVT_SWITCH_BACK_AND_RESUME && gfps_sass_is_profile_connected(&lastAddr))
            {
                SassBtInfo *lastInfo = gfps_sass_get_connected_dev_by_addr(&lastAddr);
                uint8_t lastId = lastInfo->connId;
                uint8_t currId;
                bool isExchanged = gfps_sass_is_profile_exchanged(lastInfo);

                currId = SET_BT_ID(app_bt_audio_get_curr_playing_a2dp());
                if(currId == BT_DEVICE_INVALID_ID)
                {
                    currId = app_ble_get_curr_play_bleaudio_id();
                }

                if (isExchanged && (currId != lastId && currId != SASS_INVALID_DEV_ID) && \
                    aob_media_is_device_any_ase_in_streamimg_state(lastId) && \
                    GET_PROFILE_STATE(lastInfo->audState, MUSIC, LEA))
                {
                    gfps_sass_switch_media(currId, lastId, true);
                    lastInfo->isNeedResume = true;
                }
                else if (!isExchanged && aob_media_is_device_any_ase_in_streamimg_state(lastId))
                {
                    TRACE(1, "%s waiting profile exchange", __func__);
                    SassPendingProc pending;
                    pending.proc = SASS_PROCESS_SWITCH_ACTIVE_SRC;
                    pending.activeId = lastId;
                    pending.status = SASS_SWITCH_STATUS_REC_SWITCH;
                    gfps_sass_set_pending_proc(&pending);
                }
                else
                {
                    bes_ble_aob_media_play(lastId);
                }
            }
            else
            {
                TRACE(0, "sass switch back already connect!!");
            }
        }
#endif
    }
    else
    {
        TRACE(0, "sass switch back hdl disconnect itself");
        gfps_sass_set_disconnecting_dev(&currAddr);
#ifdef BT_SERVICE_ENABLE
        app_dev_mgr_user_disconnect_dev(&currAddr);
#else
        app_ui_destroy_device((bt_bdaddr_t *)&(currAddr), false);
#endif
    }

    return SASS_STATUS_OK;
}

void gfps_sass_switch_src_hdl(uint8_t devId, uint8_t *data)
{
    uint8_t evt = data[0];
    uint8_t status = SASS_STATUS_OK;
#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if (TWS_UI_SLAVE != app_ibrt_if_get_ui_role()&& p_ibrt_ctrl->init_done)
#endif
    {
        status = gfps_sass_switch_src_evt_hdl(devId, evt);
    }

    if (status == SASS_STATUS_OK)
    {
        gfps_send_msg_ack(devId, FP_MSG_GROUP_SASS, FP_MSG_SASS_SWITCH_ACTIVE_SOURCE);
    }
    else if (status == SASS_STATUS_REDUNTANT)
    {
        gfps_send_msg_nak(devId, FP_MSG_NAK_REASON_REDUNDANT_ACTION, FP_MSG_GROUP_SASS,\
                                    FP_MSG_SASS_SWITCH_ACTIVE_SOURCE);
    }
    else
    {
        gfps_send_msg_nak(devId, FP_MSG_NAK_REASON_NOT_ALLOWED, FP_MSG_GROUP_SASS,\
                                    FP_MSG_SASS_SWITCH_ACTIVE_SOURCE);
    }
    TRACE(3,"%s evt:0x%0x, status:%d",__func__, evt, status);
}

void gfps_sass_switch_back_hdl(uint8_t devId, uint8_t *data)
{
    uint8_t evt = data[0];
    uint8_t status = SASS_STATUS_OK;
#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if(TWS_UI_SLAVE != app_ibrt_if_get_ui_role()&& p_ibrt_ctrl->init_done)
#endif
    {
        status = gfps_sass_switch_back_evt_hdl(devId, evt);
    }

    if (status == SASS_STATUS_OK)
    {
        gfps_send_msg_ack(devId, FP_MSG_GROUP_SASS, FP_MSG_SASS_SWITCH_BACK);
    }
    else
    {
        gfps_send_msg_nak(devId, FP_MSG_NAK_REASON_NOT_ALLOWED, FP_MSG_GROUP_SASS,\
                                    FP_MSG_SASS_SWITCH_BACK);
    }

    //gfps_sass_ntf_switch_evt(devId, SASS_REASON_UNSPECIFIED);
    TRACE(3,"%s evt:0x%0x, status:%d",__func__, evt, status);
}

void gfps_sass_remove_dev_handler(bt_bdaddr_t *addr, uint8_t deviceId)
{
    uint8_t maxLink = gfps_sass_get_multi_status() ? 2 : 1;
    uint8_t actDev = SASS_INVALID_DEV_ID;
    uint8_t *inUseKey = NULL;
    uint8_t *inUseAddr = NULL;
    SassBtInfo *sInfo = NULL;

    if (addr == NULL)
    {
        return;
    }

    if (!memcmp(sassInfo.reconnInfo.reconnAddr.address, (uint8_t *)addr, sizeof(bt_bdaddr_t)))
    {
        memset((uint8_t *)&(sassInfo.reconnInfo), 0, sizeof(sassInfo.reconnInfo));
    }

    if (!memcmp(sassInfo.disconectingAddr.address, (uint8_t *)addr, sizeof(bt_bdaddr_t)))
    {
        memset((uint8_t *)&(sassInfo.disconectingAddr), 0, sizeof(bt_bdaddr_t));
    }

    sInfo = gfps_sass_get_connected_dev_by_addr(addr);
    if (sInfo)
    {
        TRACE(2, "gfps_sass_remove_dev_handler, %d, deviceId %d", sInfo->connId, deviceId);
        if(sInfo->connId != deviceId)
        {
            return;
        }
        if (IS_BT_DEVICE(sInfo->connId))
        {
            gfps_sass_clear_reject_hf_dev(GET_BT_ID(sInfo->connId));
        }

        SassBtInfo *otherInfo = gfps_sass_get_other_connected_dev(sInfo->connId);
        
        if (sassInfo.activeId == sInfo->connId)
        {
            if (otherInfo)
            {
                if (IS_BT_DEVICE(otherInfo->connId))
                {
                    if (GET_PROFILE_STATE(otherInfo->audState, AUDIO, AVRCP) || \
                        GET_PROFILE_STATE(otherInfo->audState, AUDIO, A2DP) || \
                        GET_PROFILE_STATE(otherInfo->audState, AUDIO, HFP))
                    {
                        actDev = otherInfo->connId;
                    }
                    else
                    {
                        actDev = SASS_INVALID_DEV_ID;
                    }
                }
                else
                {
                    if (GET_PROFILE_STATE(otherInfo->audState, GAME, LEA) || \
                            GET_PROFILE_STATE(otherInfo->audState, MUSIC, LEA) || \
                            GET_PROFILE_STATE(otherInfo->audState, CALL, LEA))
                    {
                        actDev = otherInfo->connId;
                    }
                    else
                    {
                        actDev = SASS_INVALID_DEV_ID;
                    }
                }
            }
             gfps_sass_set_active_dev(actDev);
        }

        if (!memcmp(sassInfo.inUseAddr.address, (uint8_t *)addr, sizeof(bt_bdaddr_t)))
        {
            if (otherInfo && gfps_sass_check_sass_mode(otherInfo))
            {
                inUseKey = otherInfo->accKey;
                inUseAddr = otherInfo->btAddr.address;
            }
            gfps_sass_set_inuse_acckey(inUseKey, (bt_bdaddr_t *)inUseAddr);
        }

        if (sassInfo.pending.activeId == sInfo->connId)
        {
            gfps_sass_clear_pending_proc();
        }

        memset(sInfo, 0, sizeof(SassBtInfo));
        memset(sInfo->btAddr.address, 0xFF, sizeof(bt_bdaddr_t));
        sInfo->connId = SASS_INVALID_DEV_ID;
        sInfo->secondId = SASS_INVALID_DEV_ID;
        sassInfo.connNum--;
    }

    if (sassInfo.connNum >= maxLink)
    {
        sassInfo.connAvail = SASS_CONN_NONE_AVAILABLE;
    }
    else
    {
        sassInfo.connAvail = SASS_CONN_AVAILABLE;
        if (sassInfo.connNum == 0) {
            sassInfo.connState = SASS_STATE_NO_CONNECTION;
            sassInfo.focusMode = SASS_CONN_NO_FOCUS;
        }
    }
    TRACE(1, "%s acckey is:", __func__);
    DUMP8("%02x ", sassInfo.connInfo[0].accKey, 16);
    DUMP8("%02x ", sassInfo.connInfo[1].accKey, 16);
}

SASS_DEV_LINK_TYPE_E gfps_sass_check_dev_link(uint8_t devId, bt_bdaddr_t *addr)
{
    SASS_DEV_LINK_TYPE_E linkType;
    SassBtInfo *sInfo = gfps_sass_get_connected_dev_by_addr(addr);

    if (!sInfo || (devId == SASS_INVALID_DEV_ID))
    {
        return SASS_DEV_NEW_LINK;
    }

    if((sInfo->connId == SASS_INVALID_DEV_ID) &&
       (sInfo->secondId == SASS_INVALID_DEV_ID))
    {
        linkType = SASS_DEV_NEW_LINK;
    }
    else if((sInfo->connId != SASS_INVALID_DEV_ID) &&
            (sInfo->connId != devId))
    {
        sInfo->secondId = devId;
        linkType = SASS_DEV_DUAL_LINK;
        if (IS_BT_DEVICE(sInfo->connId))
        {
            // exchange dev id for dual mode devices
            uint8_t firstDevId = sInfo->connId;
            sInfo->connId = devId;
            sInfo->secondId = firstDevId;
        }
        TRACE(0,"%s update second Id:%d", __func__, devId);
    }
    else if ((sInfo->connId != SASS_INVALID_DEV_ID) &&
            (sInfo->secondId == SASS_INVALID_DEV_ID) &&
            (sInfo->connId == devId))
    {
        linkType = SASS_DEV_DUPLICATED;
    }
    else if ((sInfo->connId == SASS_INVALID_DEV_ID) &&
            (sInfo->secondId != SASS_INVALID_DEV_ID) &&
            (sInfo->secondId == devId))
    {
        linkType = SASS_DEV_DUPLICATED;
    }
    else
    {
        linkType = SASS_DEV_NEW_LINK;
    }
    return linkType;
}

void gfps_sass_add_dev_handler(uint8_t devId, bt_bdaddr_t *addr)
{
    uint8_t cod[3];
    uint8_t connNum = 0;
    SassBtInfo *btHdl;
    bool isSass = false;

    if (sassInfo.isMulti) {
        connNum = SASS_MAX_DEVICE_NUM;
    } else {
        connNum = 1;
    }

#ifdef BT_SERVICE_ENABLE
    app_dev_mgr_register_devices_limite_callback(gfps_sass_select_disconnect_device);
#endif
    DUMP8("%2x ", addr, 6);

    SASS_DEV_LINK_TYPE_E linkType = gfps_sass_check_dev_link(devId, addr);
    if (SASS_DEV_DUPLICATED == linkType) {
        gfps_sass_remove_dev_handler(addr, devId);
    }
    else if (SASS_DEV_DUAL_LINK == linkType) {
        return;
    }
    else {
    }

    btHdl = gfps_sass_get_free_handler();
    if (!btHdl)
    {
        TRACE(1,"%s has no res for new connection!!!", __func__);
        return ;
    }

    memset(btHdl, 0, sizeof(SassBtInfo));
    btHdl->connId = devId;
    btHdl->secondId = SASS_INVALID_DEV_ID;
    memcpy(btHdl->btAddr.address, addr, sizeof(bt_bdaddr_t));

    app_bt_get_remote_cod_by_addr(addr, cod);
    btHdl->devType = gfps_sass_get_dev_type_by_cod(cod);

    isSass = gfps_sass_check_sass_mode(btHdl);
    if (IS_BT_DEVICE(btHdl->connId))
    {
        gfps_sass_clear_reject_hf_dev(GET_BT_ID(btHdl->connId));
    }
    else
    {
        TRACE(0,"gfps_sass_add_dev_handler lea");
        SET_PROFILE_STATE(btHdl->audState, CONNECTION, LEA, 1);
        btHdl->state = SASS_STATE_NO_DATA;
    }
    sassInfo.connNum++;
    TRACE(5,"%s btHdl->devType:0x%0x num:%d isSass:%d devId:%d", 
            __func__, btHdl->devType, sassInfo.connNum, isSass, devId);

    if (sassInfo.connNum == 1) {
        sassInfo.connState = SASS_STATE_NO_DATA;
        sassInfo.focusMode = SASS_CONN_NO_FOCUS;
        gfps_sass_update_last_dev(addr);
        gfps_sass_update_last_dev_connId(devId);
        if (isSass)
        {
            gfps_sass_set_inuse_acckey(btHdl->accKey, &(btHdl->btAddr));
        }
    }

    if (sassInfo.connNum < connNum){
        sassInfo.connAvail = SASS_CONN_AVAILABLE;
    }else{
        sassInfo.connAvail = SASS_CONN_NONE_AVAILABLE;
    }

    if (!memcmp(sassInfo.reconnInfo.reconnAddr.address, addr, sizeof(bt_bdaddr_t)))
    {
        sassInfo.autoReconn = SASS_AUTO_RECONNECTED;
        if (sassInfo.reconnInfo.evt != SASS_EVT_SWITCH_BACK_AND_RESUME)
        {
            sassInfo.reconnInfo.evt = 0xFF;
        }
    }

    uint8_t zero[SESSION_NOUNCE_NUM] = {0};
    if(!memcmp(btHdl->session, zero, SESSION_NOUNCE_NUM))
    {
        for(int n = 0; n < SESSION_NOUNCE_NUM; n++)
        {
             btHdl->session[n] = (uint8_t)rand();
        }
    }
}

bool gfps_sass_check_if_remove_dev(bt_bdaddr_t *addr, uint8_t deviceId)
{
    bool ret = true;
#if BLE_AUDIO_ENABLED
    SassBtInfo *sInfo = gfps_sass_get_connected_dev_by_addr(addr);
    if (sInfo)
    {
        TRACE(0, "%s deviceId:%d connId:%d secondId:%d", __func__, deviceId, sInfo->connId, sInfo->secondId);
        if (deviceId == sInfo->connId &&
            sInfo->secondId != SASS_INVALID_DEV_ID)
        {
            sInfo->connId = sInfo->secondId;
            sInfo->secondId = SASS_INVALID_DEV_ID;
            ret = false;
        }
        else if (deviceId == sInfo->secondId &&
                 sInfo->connId != SASS_INVALID_DEV_ID)
        {
            sInfo->secondId = SASS_INVALID_DEV_ID;
            ret = false;
        }
        else
        {
        }
    }
#endif
    return ret;
}

bool gfps_sass_del_dev_handler(bt_bdaddr_t *addr, uint8_t reason, uint8_t deviceId)
{
    if (!gfps_sass_check_if_remove_dev(addr, deviceId))
    {
        return false;
    }
    uint8_t lastActId = gfps_sass_get_last_active_dev();

    gfps_sass_remove_dev_handler(addr, deviceId);
    if (reason == BTIF_BEC_MAX_CONNECTIONS || reason == BTIF_BEC_LOCAL_TERMINATED)
    {
        gfps_sass_update_last_dev(addr);
        gfps_sass_update_last_dev_connId(deviceId);
    }
    else if (!gfps_sass_is_any_dev_connected())
    {
        gfps_sass_clear_last_dev();
        gfps_sass_clear_last_dev_connId();
    }
    else
    {
        TRACE(1,"%s don't update last dev", __func__);
    }
    app_ui_update_scan_type_policy(SCAN_EV_ENABLE);

    if ((lastActId != SASS_INVALID_DEV_ID) &&
        !gfps_sass_get_connected_dev(lastActId))
    {
        gfps_sass_set_last_active_dev(SASS_INVALID_DEV_ID);
    }

    TRACE(2,"%s connNum: %d", __func__, sassInfo.connNum);
    return true;
}

void gfps_sass_connect_handler(uint8_t device_id, const bt_bdaddr_t *addr)
{
    SassEvtParam evtParam;
    evtParam.event = SASS_EVT_ADD_DEV;
    evtParam.devId = device_id;
    memcpy(evtParam.addr.address, addr, sizeof(bt_bdaddr_t));
    gfps_sass_update_state(&evtParam);
}

void gfps_sass_disconnect_handler(uint8_t device_id, const bt_bdaddr_t *addr, uint8_t errCode)
{
    if (device_id == SASS_INVALID_DEV_ID)
    {
        SassBtInfo *sInfo = gfps_sass_get_connected_dev_by_addr(addr);
        if (sInfo)
        {
            if ((sInfo->connId != SASS_INVALID_DEV_ID) &&
                IS_BT_DEVICE(sInfo->connId))
            {
                device_id = sInfo->connId;
            }
            else if ((sInfo->secondId != SASS_INVALID_DEV_ID) &&
                     IS_BT_DEVICE(sInfo->secondId))
            {
                device_id = sInfo->secondId;
            }
            else
            {
                TRACE(0,"%s devId:%d, secondId:%d", __func__, sInfo->connId, sInfo->secondId);
                return;
            }
        }
    }

    SassEvtParam evtParam;
    evtParam.event = SASS_EVT_DEL_DEV;
    evtParam.devId = device_id;
    evtParam.reason = errCode;
    memcpy(evtParam.addr.address, addr, sizeof(bt_bdaddr_t));
    gfps_sass_update_state(&evtParam);
}

void gfps_sass_update_state(SassEvtParam *evtParam)
{
    uint8_t devId = evtParam->devId;
    bool needUpdate = true;
    SassBtInfo *otherInfo = NULL, *sInfo = NULL;

    switch(evtParam->event)
    {
        case SASS_EVT_ADD_DEV:
        {
            gfps_sass_add_dev_handler(devId, &(evtParam->addr));
            break;
        }

        case SASS_EVT_DEL_DEV:
        {
            if (!gfps_sass_del_dev_handler(&(evtParam->addr), evtParam->reason, evtParam->devId))
            {
                return;
            }
            break;
        }

        case SASS_EVT_UPDATE_CONN_STATE:
        {
            gfps_sass_set_conn_state(evtParam->state.connState);
            break;
        }

        case SASS_EVT_UPDATE_HEAD_STATE:
        {
            SASS_HEAD_STATE_E headstate = evtParam->state.headState;
            gfps_sass_set_head_state(headstate);
            break;
        }

        case SASS_EVT_UPDATE_FOCUS_STATE:
        {
            SASS_FOCUS_MODE_E focusstate = evtParam->state.focusMode;
            gfps_sass_set_focus_mode(focusstate);
            break;
        }

        case SASS_EVT_UPDATE_RECONN_STATE:
        {
            SASS_AUTO_RECONN_E reconnstate = evtParam->state.autoReconn;
            gfps_sass_set_auto_reconn(reconnstate);
            break;
        }

        case SASS_EVT_UPDATE_CUSTOM_DATA:
        {
            gfps_sass_set_custom_data(evtParam->state.cusData);
            break;
        }

        case SASS_EVT_UPDATE_MULTI_STATE:
        {
            gfps_sass_set_conn_available(evtParam->state.connAvail);
            break;
        }

        case SASS_EVT_UPDATE_ACTIVE_DEV:
        {
            needUpdate = false;
            break;
        }

        case SASS_EVT_UPDATE_INUSE_ACCKEY:
        {
#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
            if (TWS_UI_MASTER == app_ibrt_if_get_ui_role())
            {
                gfps_sass_sync_info();
            }
#endif
            break;
        }
        default:
        break;
    }

    gfps_sass_update_adv_data();
    gfps_sass_ntf_conn_status(devId, true, NULL);

    otherInfo  = gfps_sass_get_other_connected_dev(devId);
    sInfo  = gfps_sass_get_connected_dev(devId);

    if (otherInfo && (otherInfo->connId != SASS_INVALID_DEV_ID))
    {
        if ((evtParam->event == SASS_EVT_UPDATE_HEAD_STATE) || \
            (evtParam->event == SASS_EVT_UPDATE_ACTIVE_DEV && (sInfo && sInfo->isSass && otherInfo->isSass && \
            !memcmp(sInfo->accKey, otherInfo->accKey, FP_ACCOUNT_KEY_SIZE))) || \
            (!sInfo)|| \
            (sInfo && !sInfo->isSass)|| \
            (sInfo && sInfo->isSass && otherInfo->isSass && \
            !memcmp(sInfo->accKey, otherInfo->accKey, FP_ACCOUNT_KEY_SIZE)))
        {
            gfps_sass_ntf_conn_status(otherInfo->connId, true, NULL);
        }
      /*  else if (sInfo->isSass && otherInfo->isSass && \
            memcmp(sInfo->accKey, otherInfo->accKey, FP_ACCOUNT_KEY_SIZE))
        {
            SASS_CONN_STATE_E providerState = gfps_sass_get_conn_state();
            if ((otherInfo->state != providerState) && (providerState > SASS_STATE_NO_AUDIO))
            {
                SassStateInfo seekerState;
                uint8_t len;
                gfps_sass_get_adv_data((uint8_t *)&seekerState, &len);
                SET_SASS_CONN_STATE(seekerState.state, CONN_STATE, otherInfo->state);
                gfps_sass_ntf_conn_status(otherInfo->connId, false, (uint8_t *)&seekerState);
            }
        }*/
    }

    if (needUpdate)
    {
        bes_ble_gap_refresh_adv_state(BLE_FASTPAIR_NORMAL_ADVERTISING_INTERVAL);
    }
}

void gfps_sass_ind_inuse_acckey(uint8_t devId, uint8_t *data)
{
    char str[8] = "in-use";
    uint8_t auth[8] = {0};
    uint8_t nonce[16] = {0};
    uint8_t keyCount = 0;
    uint8_t accKey[16] = {0};
    uint8_t lastDevKey[16] = {0};
    uint8_t output[8] = {0};
    if (memcmp(data, str, 6))
    {
        TRACE(1, "%s data error!", __func__);
        return;
    }

    gfps_sass_get_session_nonce(devId, nonce);
    memcpy(nonce + 8, data + 6, 8);
    memcpy(auth, data + 14, 8);

    keyCount = nv_record_fp_account_key_count();
    for (int i = 0; i < keyCount; i++)
    {
        nv_record_fp_account_key_get_by_index(i, accKey);
        gfps_encrypt_messasge(accKey, nonce, data, 6, output);

        if (!memcmp(output, auth, 8))
        {
            TRACE(2, "%s find account key index:%d", __func__, i);
            bt_bdaddr_t btAddr;
            uint8_t initAcc[FP_ACCOUNT_KEY_SIZE];
            gfps_sass_get_inuse_acckey(initAcc);
            gfps_sass_get_inuse_acckey_by_id(devId, lastDevKey);
            gfps_sass_set_inuse_acckey_by_dev(devId, accKey);
            if(IS_BT_DEVICE(devId))
            {
                if (app_bt_get_device_bdaddr(GET_BT_ID(devId), btAddr.address) && \
                    gfps_sass_is_truely_sass_dev(devId))
                {
                    nv_record_fp_update_addr(i, btAddr.address);
                    if (gfps_sass_get_active_dev() == SASS_INVALID_DEV_ID)
                    {
                        gfps_sass_set_inuse_acckey(accKey, &btAddr);
                    }
                }
            }
            else
            {
                ble_bdaddr_t GetPeerAddr = {{0}};
                if (app_ble_get_peer_solved_addr(devId, &GetPeerAddr) && \
                    gfps_sass_is_truely_sass_dev(devId))
                {
                    memcpy(btAddr.address, GetPeerAddr.addr, 6);
                    nv_record_fp_update_addr(i, btAddr.address);
                    if (gfps_sass_get_active_dev() == SASS_INVALID_DEV_ID)
                    {
                        gfps_sass_set_inuse_acckey(accKey, &btAddr);
                    }
                }
            }

            if (!gfps_sass_is_other_sass_dev(devId) || \
                memcmp(initAcc, accKey, FP_ACCOUNT_KEY_SIZE) || \
                memcmp(lastDevKey, accKey, FP_ACCOUNT_KEY_SIZE) || \
                gfps_sass_is_need_ntf_status(devId))
            {
                SassEvtParam evtParam;
                evtParam.devId = devId;
                evtParam.event = SASS_EVT_UPDATE_INUSE_ACCKEY;
                gfps_sass_update_state(&evtParam);
            }

            if (gfps_sass_is_need_ntf_switch(devId))
            {
                SASS_REASON_E reason = gfps_sass_get_switch_reason(gfps_sass_get_active_dev());
                gfps_sass_ntf_switch_evt(devId, reason);
            }
            break;
        }
    }
}

void gfps_sass_send_info_to_seeker(uint8_t devId)
{
    gfps_sass_send_session_nonce(devId);
    gfps_sass_get_capability(devId);
    gfps_sass_ntf_conn_status(devId, true, NULL);
}

void gfps_sass_handler(uint8_t devId, uint8_t evt, void *param)
{
    switch(evt)
    {
        case FP_MSG_SASS_GET_CAPBILITY:
        {
            gfps_sass_ntf_capability(devId);
            break;
        }

        case FP_MSG_SASS_NTF_CAPBILITY:
        {
            gfps_sass_set_capability(devId, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_SET_MULTIPOINT_STATE:
        {
            gfps_sass_set_multipoint_hdl(devId, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_SET_SWITCH_PREFERENCE:
        {
            gfps_sass_set_switch_pref_hdl(devId, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_GET_SWITCH_PREFERENCE:
        {
            gfps_sass_ntf_switch_pref(devId);
            break;
        }
        case FP_MSG_SASS_SWITCH_ACTIVE_SOURCE:
        {
            gfps_sass_switch_src_hdl(devId, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_SWITCH_BACK:
        {
            gfps_sass_switch_back_hdl(devId, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_GET_CONN_STATUS:
        {
            gfps_sass_get_conn_hdl(devId);
            break;
        }
        case FP_MSG_SASS_NTF_INIT_CONN:
        {
            gfps_sass_set_init_conn(devId, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_IND_INUSE_ACCOUNT_KEY:
        {
            gfps_sass_ind_inuse_acckey(devId, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_SEND_CUSTOM_DATA:
        {
            gfps_sass_set_custom_data(devId, (uint8_t *)param);
            break;
        }
        case FP_MSG_SASS_SET_DROP_TGT:
        {
            gfps_sass_set_drop_dev(devId, (uint8_t *)param);
            break;
        }
        default:
        break;
    }
}

#endif
