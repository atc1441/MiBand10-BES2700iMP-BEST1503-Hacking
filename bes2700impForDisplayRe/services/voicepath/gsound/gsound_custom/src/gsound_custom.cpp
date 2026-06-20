/***************************************************************************
*
*Copyright 2015-2019 BES.
*All rights reserved. All unpublished rights reserved.
*
*No part of this work may be used or reproduced in any form or by any
*means, or stored in a database or retrieval system, without prior written
*permission of BES.
*
*Use of this work is governed by a license granted by BES.
*This work contains confidential and proprietary information of
*BES. which is protected by copyright, trade secret,
*trademark and other intellectual property rights.
*
****************************************************************************/

/*****************************header include********************************/
#include "gsound_dbg.h"
#include "gsound_service.h"
#include "gsound_custom.h"
#include "gsound_custom_service.h"
#include "gsound_custom_bt.h"
#include "gsound_custom_audio.h"

#ifdef TWS_SYSTEM_ENABLED
#include "app_ibrt_internal.h"
#include "gsound_custom_tws.h"
#include "app_tws_ctrl_thread.h"
#endif

#ifdef IBRT
#include "app_tws_ibrt.h"
#include "app_ai_if.h"
#include "app_ibrt_customif_cmd.h"
#include "app_ai_tws.h"
#endif

#ifdef GSOUND_HOTWORD_EXTERNAL
#include "gsound_custom_hotword_external.h"
#endif

#include "bluetooth_bt_api.h"
#ifdef BT_DIP_SUPPORT
#include "nvrecord_bt.h"
#include "dip_api.h"
#endif

#include "ai_thread.h"

/************************private macro defination***************************/
#define GSOUND_CHANNEL_CONTROL_MASK (1 << GSOUND_CHANNEL_CONTROL)
#define GSOUND_CHANNEL_AUDIO_MASK (1 << GSOUND_CHANNEL_AUDIO)

#define CASES(prefix, item) \
    case prefix##item:      \
        str = #item;        \
        break;

/************************private type defination****************************/
typedef struct
{
    uint8_t rfcommInitState : 1;
    uint8_t rfcommConnectState : 2;
    uint8_t bleInitState : 1;
    uint8_t bleConnectState : 2;
    uint8_t serviceEnableState : 1;
    uint8_t roleSwitchState : 1;

    uint8_t btConnectState : 1;
    MOBILE_CONN_TYPE_E mobileType : 2;
    uint8_t reserved : 5;
    bool    isRxAvail;
    uint8_t connectedBdAddr[6];
    uint8_t connectedBleAddr[6];

#ifdef IBRT
    uint8_t role;
#endif
} GSOUND_ENV_T;

/**********************private function declearation************************/
/*---------------------------------------------------------------------------
 *            gsound_role_init
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    init gsound role for libgsound usage
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
static void gsound_role_init(void);

/************************private variable defination************************/
static GSOUND_ENV_T gsoundEnv;

/****************************function defination****************************/
void gsound_custom_init(bool isEnable)
{
    GLOG_I("[%s]+++", __func__);

    memset(&gsoundEnv, 0, sizeof(GSOUND_ENV_T));

#ifdef IBRT
    gsoundEnv.role = IBRT_UNKNOW;
#endif

    gsound_custom_audio_init();

#ifdef GSOUND_HOTWORD_ENABLED
#ifdef GSOUND_HOTWORD_EXTERNAL
    /// init external hotword env
    gsound_custom_hotword_external_init();

    /// enable external hotword
    GSoundServiceInitHotwordExternal();
#else
    /// enable internal hotword
    GSoundServiceInitHotwordInternal();
#endif
#endif

    gsound_service_init(isEnable);
    gsound_role_init();
    gsound_bt_init();
}

MOBILE_CONN_TYPE_E gsound_get_connected_mobile_type()
{
    return gsoundEnv.mobileType;
}


void gsound_mobile_type_get_callback(MOBILE_CONN_TYPE_E type)
{
    GLOG_I("[%s]+++ %d", __func__, type);
    gsoundEnv.mobileType =  type;

#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
    if (IBRT_SLAVE != app_ai_tws_get_local_role())
    {
        tws_ctrl_send_cmd(APP_TWS_CMD_BISTO_DIP_SYNC, (uint8_t *)&type, sizeof(MOBILE_CONN_TYPE_E));
    }
#endif
}

bool gsound_bt_is_rx_available()
{
    return gsoundEnv.isRxAvail;
}

void gsound_bt_set_rx_available(bool avail)
{
    gsoundEnv.isRxAvail = avail;
}

bool gsound_is_bt_init_done(void)
{
    return gsoundEnv.rfcommInitState;
}

void gsound_set_bt_init_state(bool state)
{
    gsoundEnv.rfcommInitState = state;
    GLOG_I("gsound bt init state is set to:%d", state);
}

uint8_t gsound_get_bt_connect_state(void)
{
    GLOG_I("gsound rfcommConnectState:%d", gsoundEnv.rfcommConnectState);
    return gsoundEnv.rfcommConnectState;
}

uint8_t gsound_get_rfcomm_connect_state(uint8_t chnl)
{
    return (gsoundEnv.rfcommConnectState & (1 << chnl));
}

void gsound_set_rfcomm_connect_state(uint8_t chnl, bool state)
{
    GLOG_I("[%s]+++", __func__);

    if (state)
    {
        gsoundEnv.rfcommConnectState |= (1 << chnl);
    }
    else
    {
        gsoundEnv.rfcommConnectState &= ~(1 << chnl);
    }

    GLOG_I("current connect state: 0x%02x", gsoundEnv.rfcommConnectState);
    GLOG_I("[%s]---", __func__);
}

bool gsound_is_ble_init_done(void)
{
    return gsoundEnv.bleInitState;
}

void gsound_set_ble_init_state(bool state)
{
    gsoundEnv.bleInitState = state;
}

uint8_t gsound_get_ble_connect_state(void)
{
    return gsoundEnv.bleConnectState;
}

uint8_t gsound_get_ble_channel_connect_state(uint8_t chnl)
{
    return (gsoundEnv.bleConnectState | (1 << chnl));
}

void gsound_set_ble_connect_state(uint8_t chnl, bool state)
{
    GLOG_I("BLE_%s_%s", gsound_chnl2str(chnl), state ? "CONNECTED" : "DISCONNECTED");

    if (state)
    {
        gsoundEnv.bleConnectState |= (1 << chnl);
    }
    else
    {
        gsoundEnv.bleConnectState &= ~(1 << chnl);
    }

    GLOG_I("%s current connect state: 0x%02x", __func__, gsoundEnv.bleConnectState);
}

uint8_t gsound_custom_get_connection_path(void)
{
    uint8_t path = CONNECTION_NULL;

    if (gsound_get_ble_connect_state())
    {
        path = CONNECTION_BLE;
    }
    else if (gsound_get_bt_connect_state())
    {
        path = CONNECTION_BT;
    }

    return path;
}

bool gsound_is_service_enabled(void)
{
    return gsoundEnv.serviceEnableState;
}

void gsound_set_service_enable_state(bool enable)
{
    gsoundEnv.serviceEnableState = enable;
}

bool gsound_is_role_switch_ongoing(void)
{
    return gsoundEnv.roleSwitchState;
}

void gsound_set_role_switch_state(bool state)
{
    gsoundEnv.roleSwitchState = state;
}

bool gsound_is_bt_connected(void)
{
    return gsoundEnv.btConnectState;
}

void gsound_set_bt_connection_state(bool state)
{
    GLOG_I("gsound_set_bt_connection_state %d", state);
    gsoundEnv.btConnectState = state;
}

void gsound_set_connected_bd_addr(uint8_t *addr)
{
    memcpy(gsoundEnv.connectedBdAddr, addr, 6);
}

uint8_t *gsound_get_connected_bd_addr(void)
{
    return gsoundEnv.connectedBdAddr;
}

void gsound_set_connected_ble_addr(uint8_t *addr)
{
    memcpy(gsoundEnv.connectedBleAddr, addr, 6);
}

uint8_t *gsound_get_connected_ble_addr(void)
{
    return gsoundEnv.connectedBleAddr;
}


#ifdef IBRT
uint8_t gsound_get_role(void)
{
    return gsoundEnv.role;
}

void gsound_set_role(uint8_t role)
{
    GLOG_I("gsound role is set to %d", role);
    gsoundEnv.role = role;
}

void gsound_on_system_role_switch_done(uint8_t newRole)
{
    uint8_t *mobileAddr = NULL;

    if (gsound_get_bt_connect_state())
    {
        gsound_custom_audio_role_switch_complete_handler(newRole);
    }

    // update the role in libgsound
    gsound_tws_inform_roleswitch_done();

    /// get connected mobile address
#ifdef IBRT_UI
    mobileAddr = app_ibrt_conn_get_mobile_info_ext()->mobile_addr.address;
#else
    mobileAddr = app_tws_ibrt_get_bt_ctrl_ctx()->mobile_addr.address;
#endif

    // update the bt connect status to gsound
    gsound_custom_bt_link_connected_handler(mobileAddr);

    if (newRole == IBRT_MASTER) {
        gsoundEnv.mobileType = app_bt_check_is_ios_device(mobileAddr) ? MOBILE_CONNECT_IOS : MOBILE_CONNECT_ANDROID;
        app_ai_mobile_connect_handle(gsoundEnv.mobileType, mobileAddr);
    }
}

void gsound_on_tws_role_updated(uint8_t newRole)
{
    gsound_tws_role_update(newRole);
}
#endif

static void gsound_role_init(void)
{
#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
    bool isMaster = false;
    uint8_t role = 0;

    GSoundServiceInitAsTws();

    role = app_ai_tws_get_local_role();
    isMaster = (TWS_UI_SLAVE != role);
    GLOG_I("ibrt role:%s", app_tws_ibrt_role2str(role));
    gsound_set_role(role);

    const GSoundTwsInterface *pGsoundTwsInterface = (const GSoundTwsInterface *)gsound_tws_get_target_handle();
    ASSERT(pGsoundTwsInterface, "gsound tws interface is NULL");

    if (pGsoundTwsInterface)
    {
        // pGsoundTwsInterface->gsound_tws_role_change_init_role(isMaster, earAssignment);
        pGsoundTwsInterface->gsound_tws_role_change_init_role(isMaster,
                                                              (app_ibrt_if_is_left_side())
                                                                  ? GSOUND_TARGET_EAR_ASSIGNMENT_LEFT
                                                                  : GSOUND_TARGET_EAR_ASSIGNMENT_RIGHT);
    }
#else
    GSoundServiceInitAsStereo();
#endif
}

const char *gsound_chnl2str(uint8_t chnl)
{
    const char *str = NULL;

    switch (chnl)
    {
        CASES(GSOUND_CHANNEL_, CONTROL);
        CASES(GSOUND_CHANNEL_, AUDIO);

    default:
        str = "INVALID";
        break;
    }

    return (char *)str;
}

void gsound_convert_bdaddr_to_gsound(void const *plat_addr,
                                     GSoundBTAddr *gsound_addr)
{
    if ((NULL != plat_addr) && (NULL != gsound_addr))
    {
        uint8_t *ptr = (uint8_t *)plat_addr;
        memcpy(&gsound_addr->nap, ptr, GSOUND_BT_NAP_SIZE);
        ptr += GSOUND_BT_NAP_SIZE;
        memcpy(&gsound_addr->uap, ptr, GSOUND_BT_UAP_SIZE);
        ptr += GSOUND_BT_UAP_SIZE;
        memcpy(&gsound_addr->lap, ptr, GSOUND_BT_LAP_SIZE);
    }
}

void gsound_convert_bdaddr_to_plateform(GSoundBTAddr const *gsound_addr,
                                        void *plat_addr)
{
    if ((NULL != plat_addr) && (NULL != gsound_addr))
    {
        uint8_t *ptr = (uint8_t *)plat_addr;
        memcpy(ptr, &gsound_addr->nap, GSOUND_BT_NAP_SIZE);
        ptr += GSOUND_BT_NAP_SIZE;
        memcpy(ptr, &gsound_addr->uap, GSOUND_BT_UAP_SIZE);
        ptr += GSOUND_BT_UAP_SIZE;
        memcpy(ptr, &gsound_addr->lap, GSOUND_BT_LAP_SIZE);
    }
}

void gsound_custom_connection_state_received_handler(uint8_t *buf)
{
#ifdef IBRT
    AI_CONNECTION_STATE_T *state = (AI_CONNECTION_STATE_T *)buf;

    if (CONNECTION_BLE == state->connPathType)
    {
        gsound_custom_bt_link_connected_handler(state->connBdAddr);
        gsoundEnv.bleConnectState = state->connPathState;
    }
    else if (CONNECTION_BT == state->connPathType)
    {
        gsound_custom_bt_link_connected_handler(state->connBdAddr);
        gsound_bt_update_channel_connection_state(GSOUND_CHANNEL_AUDIO, state->connBdAddr, (state->connPathState & GSOUND_CHANNEL_AUDIO_MASK));
        gsound_bt_update_channel_connection_state(GSOUND_CHANNEL_CONTROL,state->connBdAddr, (state->connPathState & GSOUND_CHANNEL_CONTROL_MASK));
        gsound_set_connected_bd_addr(state->connBdAddr);

        if (gsound_get_bt_connect_state() != state->connPathState)
        {
            GLOG_I("Update bisto rfcomm connection state.");

            if ((state->connPathState & GSOUND_CHANNEL_AUDIO_MASK) && (!gsound_get_rfcomm_connect_state(GSOUND_CHANNEL_AUDIO)))
            {
                gsound_custom_bt_on_channel_connected_ind(state->connBdAddr, GSOUND_CHANNEL_AUDIO);
                gsound_custom_bt_on_channel_connected(state->connBdAddr, GSOUND_CHANNEL_AUDIO);
            }
            else
            {
                gsound_custom_bt_on_channel_disconnected(GSOUND_CHANNEL_AUDIO);
            }

            if ((state->connPathState & GSOUND_CHANNEL_CONTROL_MASK) && (!gsound_get_rfcomm_connect_state(GSOUND_CHANNEL_CONTROL)))
            {
                gsound_custom_bt_on_channel_connected_ind(state->connBdAddr, GSOUND_CHANNEL_CONTROL);
                gsound_custom_bt_on_channel_connected(state->connBdAddr, GSOUND_CHANNEL_CONTROL);
            }
            else
            {
                gsound_custom_bt_on_channel_disconnected(GSOUND_CHANNEL_CONTROL);
            }
        }
    }
    else
    {
        gsoundEnv.bleConnectState = 0;
        gsoundEnv.rfcommConnectState = 0;
        gsound_custom_bt_link_disconnected_handler(state->connBdAddr);
    }

    GLOG_I("[%s] type:%d, lstate:%d, rstate:%d|%d",
           __func__,
           state->connPathType,
           state->connPathState,
           gsoundEnv.rfcommConnectState,
           gsoundEnv.rfcommConnectState);
#endif
}

void gsound_on_bt_link_disconnected(uint8_t *addr)
{
    gsound_custom_bt_link_disconnected_handler(addr);
}
