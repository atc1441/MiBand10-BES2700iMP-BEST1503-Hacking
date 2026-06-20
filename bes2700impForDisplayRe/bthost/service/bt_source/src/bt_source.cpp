/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#if defined(BT_SOURCE)
#include <string.h>
#include "bt_source.h"
#include "app_a2dp_source.h"
#include "app_avrcp_target.h"
#include "app_hfp_ag.h"
#include "btapp.h"
#include "besbt_cfg.h"
#include "nvrecord_env.h"
#include "hal_trace.h"
#include "app_bt_func.h"
#include "audio_policy.h"
#include "app_bt.h"
#include "app_bt_media_manager.h"
#include "ddbif.h"
#include "audio_player_adapter.h"
#include "app_audio_active_device_manager.h"

struct BT_SOURCE_MANAGER_T bt_source_manager;

bt_source_callback_t g_bt_source_cb = NULL;

#define BT_SOURCE_CB(event,param) \
    do { \
        if (g_bt_source_cb != NULL) \
            g_bt_source_cb(event, param); \
    } while (0)

void bt_media_clear_media_type(uint16_t media_type, int device_id);

void app_bt_audio_stop_media_playing(uint8_t device_id);

uint32_t app_bt_audio_create_new_prio(void);

void app_bt_start_linkloss_reconnect(bt_bdaddr_t *remote, bool is_for_source_device);

void app_bt_device_reconnect_timehandler(void const *param);

osTimerDef (BT_SOURCE_DEVICE_CONNECT_TIMER0, app_bt_device_reconnect_timehandler);

#if BT_SOURCE_DEVICE_NUM > 1
osTimerDef (BT_SOURCE_DEVICE_CONNECT_TIMER1, app_bt_device_reconnect_timehandler);
#endif

#if defined(A2DP_SOURCE_TEST) || defined(HFP_AG_TEST)
#ifdef BT_SOURCE_SIMU
static const uint8_t source_test_bt_addr[6] = {0xcc, 0xaa, 0x99, 0x88, 0x77, 0x66};
#else
static const uint8_t source_test_bt_addr[6] = {0xf8, 0x3a, 0x6b, 0x25, 0x11, 0x11};
#endif
#endif

struct BT_SOURCE_DEVICE_T *app_bt_source_get_device(int i)
{
    if (i >= BT_SOURCE_DEVICE_ID_1 && i < BT_SOURCE_DEVICE_ID_N)
    {
        return bt_source_manager.devices + (i - BT_SOURCE_DEVICE_ID_1);
    }
    else
    {
        TRACE(2, "%s invalid source device id %d", __func__, i);
        return NULL;
    }
}

struct BT_SOURCE_DEVICE_T *app_bt_source_find_device(bt_bdaddr_t *remote)
{
    struct BT_SOURCE_DEVICE_T *device = NULL;

    for(int i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i++)
    {
        device = app_bt_source_get_device(i);
        if (device && memcmp(&device->base_device->remote, remote, sizeof(bt_bdaddr_t)) == 0)
        {
            return device;
        }
    }

    return NULL;
}

static void bt_source_config_init(struct bt_source_config* config)
{
    memset(config, 0, sizeof(struct bt_source_config));

#if defined(BT_SOURCE)
    config->bt_source_enable = true;
#else
    config->bt_source_enable = false;
#endif

#if defined(BT_HFP_AG_ROLE)
    config->ag_enable = true;
#else
    config->ag_enable = false;
#endif

    config->av_enable = config->bt_source_enable;
}

bool app_bt_source_nv_snk_or_src_enabled(void)
{
    return bt_source_manager.config.read_snk_or_src_from_nv;
}

extern void app_a2dp_source_init(void);
extern void app_avrcp_target_init(void);
extern void app_hfp_ag_init(void);
static void app_bt_source_init_global_handler(void);

void app_bt_source_init(void)
{
    struct BT_DEVICE_T *base_device = NULL;
    struct bt_source_config *config = NULL;
    struct BT_DEVICE_RECONNECT_T *reconnect_node = NULL;
    int i = 0;

    config = &bt_source_manager.config;
    memset(&bt_source_manager, 0, sizeof(struct BT_SOURCE_MANAGER_T));

    for(i = 0; i < BT_SOURCE_DEVICE_NUM; i += 1)
    {
        base_device = app_bt_manager.source_base_devices + i;
        memset(base_device, 0, sizeof(struct BT_DEVICE_T));
        bt_source_manager.devices[i].base_device = base_device;
        base_device->device_id = BT_SOURCE_DEVICE_ID_BASE + i;
        base_device->battery_level = 0xff;
    }

    bt_source_manager.curr_source_a2dp_id = BT_SOURCE_DEVICE_ID_1;
    bt_source_manager.curr_source_playing_a2dp = BT_SOURCE_DEVICE_INVALID_ID;

    bt_source_manager.curr_source_hfp_id = BT_SOURCE_DEVICE_ID_1;
    bt_source_manager.curr_source_playing_sco = BT_SOURCE_DEVICE_INVALID_ID;

    INIT_LIST_HEAD(&bt_source_manager.codec_packet_list);
    INIT_LIST_HEAD(&bt_source_manager.encoded_packet_list);

    bt_source_config_init(config);

    btif_me_register_is_creating_source_link(btif_me_is_creating_source_link);

#if defined(BT_SOURCE)
    if (bt_source_manager.config.read_snk_or_src_from_nv)
    {
        struct nvrecord_env_t *nvrecord_env;
        nv_record_env_get(&nvrecord_env);
        if (nvrecord_env->src_snk_flag.src_snk_mode)
        {
            config->bt_source_enable = true;
        }
        else
        {
            config->bt_source_enable = false;
        }
    }
#else
    bt_source_manager.config.read_snk_or_src_from_nv = false;
#endif

    if (!config->bt_source_enable)
    {
        return;
    }

    besbt_cfg.bt_source_enable = true;

    if (config->ag_enable && config->ag_standalone)
    {
        config->av_enable = false;
    }

    TRACE(0, "bt_sink_enable %d %d bt_source_enable %d ag_enable %d av_enable %d avdtp_cp %d",
        besbt_cfg.bt_sink_enable, besbt_cfg.a2dp_sink_enable, besbt_cfg.bt_source_enable,
        config->ag_enable, config->av_enable, besbt_cfg.avdtp_cp_enable);

    if (config->av_enable)
    {
#ifdef BT_A2DP_SRC_ROLE
        app_a2dp_source_init();
#endif
#ifdef BT_AVRCP_TG_ROLE
        app_avrcp_target_init();
#endif
    }

#ifdef BT_HFP_AG_ROLE
    if (config->ag_enable)
    {
        app_hfp_ag_init();
    }
#endif

#ifdef HFP_AG_TEST
    app_hfp_ag_uart_cmd_init();
#endif

    app_bt_source_init_global_handler();

    for (i = 0; i < BT_SOURCE_DEVICE_NUM; i += 1)
    {
        reconnect_node = app_bt_manager.reconnect_node + BT_DEVICE_NUM + i;
        reconnect_node->for_source_device = true;
        if (i == 0)
        {
            reconnect_node->acl_reconnect_timer = osTimerCreate(osTimer(BT_SOURCE_DEVICE_CONNECT_TIMER0), osTimerOnce, reconnect_node);
        }
#if BT_SOURCE_DEVICE_NUM > 1
        else if (i == 1)
        {
            reconnect_node->acl_reconnect_timer = osTimerCreate(osTimer(BT_SOURCE_DEVICE_CONNECT_TIMER1), osTimerOnce, reconnect_node);
        }
#endif
    }
}

void app_bt_source_set_connectable_state(bool enable)
{
    TRACE(2, "%s %d\n", __func__, enable);

    btif_me_set_accepting_source_link(enable ? true : false);

    if (enable)
    {
        app_bt_set_access_mode(BTIF_BAM_CONNECTABLE_ONLY);
    }
    else
    {
        app_bt_set_access_mode(BTIF_BAM_NOT_ACCESSIBLE);
    }
}

void bt_source_register_callback(bt_source_callback_t cb)
{
    g_bt_source_cb = cb;
}

void bt_source_reconnect_a2dp_profile(const bt_bdaddr_t *remote)
{
    if (bt_source_manager.config.av_enable)
    {
        btif_me_set_creating_source_link(remote, true);

        app_bt_reconnect_a2dp_profile((bt_bdaddr_t *)remote);
    }
}

void bt_source_reconnect_avrcp_profile(const bt_bdaddr_t *remote)
{
    if (bt_source_manager.config.av_enable)
    {
        btif_me_set_creating_source_link(remote, true);

        app_bt_reconnect_avrcp_profile((bt_bdaddr_t *)remote);
    }
}

void bt_source_reconnect_hfp_profile(const bt_bdaddr_t *remote)
{
    if (bt_source_manager.config.ag_enable)
    {
        btif_me_set_creating_source_link(remote, true);

        app_bt_reconnect_hfp_profile((bt_bdaddr_t *)remote);
    }
}

#if defined(BT_HID_DEVICE)
void bt_source_reconnect_hid_profile(const bt_bdaddr_t *remote)
{
    btif_me_set_creating_source_link(remote, true);

    app_bt_hid_profile_connect_v1((bt_bdaddr_t *)remote);
}
#endif // BT_HID_DEVICE

void bt_source_create_audio_link(const bt_bdaddr_t *remote)
{
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;
    curr_device = app_bt_source_find_device((bt_bdaddr_t *)remote);

    if (!curr_device || !curr_device->base_device->acl_is_connected)
    {
        TRACE(1, "%s related acl is not exist", __func__);
        return;
    }

    if (curr_device->base_device->hf_audio_state == BT_HFP_AUDIO_CON)
    {
        TRACE(1, "%s already created", __func__);
        return;
    }

    app_hfp_ag_create_audio_link(curr_device->base_device->hf_channel);
}

void bt_source_disc_audio_link(const bt_bdaddr_t *remote)
{
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;
    curr_device = app_bt_source_find_device((bt_bdaddr_t *)remote);

    if (!curr_device || curr_device->base_device->hf_audio_state != BT_HFP_AUDIO_CON)
    {
        TRACE(1, "%s related sco link is not created", __func__);
        return;
    }

    app_hfp_ag_disc_audio_link(curr_device->base_device->hf_channel);
}

void bt_source_perform_profile_reconnect(const bt_bdaddr_t *remote)
{
    app_bt_source_set_connectable_state(false);

    bt_source_reconnect_a2dp_profile(remote);

    bt_source_reconnect_hfp_profile(remote);
}

void app_bt_report_source_link_connected(btif_remote_device_t *btm_conn, uint8_t errcode)
{
    uint8_t device_id = btif_me_get_device_id_from_rdev(btm_conn);
    struct BT_SOURCE_DEVICE_T* curr_device = NULL;

    TRACE(4, "%s errcode 0x%x device %x [%x]", __func__, errcode, device_id, BT_SOURCE_DEVICE_ID_N);

    if (device_id >= BT_SOURCE_DEVICE_ID_1 && device_id < BT_SOURCE_DEVICE_ID_N)
    {
        curr_device = app_bt_source_get_device(device_id);
        if (errcode == BTIF_BEC_NO_ERROR)
        {
            curr_device->base_device->acl_is_connected = true;
            curr_device->base_device->acl_conn_hdl = btif_me_get_remote_device_hci_handle(btm_conn);
            curr_device->base_device->remote = *btif_me_get_remote_device_bdaddr(btm_conn);
            curr_device->base_device->btm_conn = btm_conn;
            curr_device->base_device->profile_mgr.rmt_addr = curr_device->base_device->remote;
            app_bt_reset_profile_manager(&curr_device->base_device->profile_mgr);
        }
        else
        {
            curr_device->base_device->acl_is_connected = false;
        }
    }
}

void app_bt_report_source_link_disconnected(const bt_bdaddr_t *remote, uint8_t errcode)
{
    uint8_t device_id = app_bt_get_connected_device_id_byaddr(remote);
    struct BT_SOURCE_DEVICE_T* curr_device = NULL;

    TRACE(3, "%s errcode 0x%x device %x\n", __func__, errcode, device_id);

    if (device_id >= BT_SOURCE_DEVICE_ID_1 && device_id < BT_SOURCE_DEVICE_ID_N)
    {
        curr_device = app_bt_source_get_device(device_id);
        curr_device->base_device->acl_is_connected = false;
        curr_device->base_device->acl_conn_hdl = BT_INVALID_CONN_HANDLE;
        curr_device->base_device->btm_conn = NULL;
        memset(&curr_device->base_device->remote, 0, sizeof(bt_bdaddr_t));
        app_bt_reset_profile_manager(&curr_device->base_device->profile_mgr);
        curr_device->base_device->profile_mgr.rmt_addr = curr_device->base_device->remote;

        if (errcode == BTIF_BEC_CONNECTION_TIMEOUT || errcode == BTIF_BEC_LMP_RESPONSE_TIMEOUT)
        {
            app_bt_start_linkloss_reconnect(&curr_device->base_device->remote, true);
        }
    }
}

void bt_source_set_accessmode_by_sink(void)
{
    uint8_t  active_source_cons = btif_me_get_source_activeCons();
    if(active_source_cons < BT_SOURCE_DEVICE_NUM)
    {
        app_bt_source_set_connectable_state(true);
    }
    else
    {
        app_bt_source_set_connectable_state(false);
    }
}

extern void app_bt_set_accessible_by_source(void);
extern void app_bt_source_notify_a2dp_link_disconnected(bt_bdaddr_t *remote);

static void bt_source_global_callback_handler(const btif_event_t* Event)
{
    uint8_t etype = btif_me_get_callback_event_type(Event);
    uint8_t err_code = btif_me_get_callback_event_err_code(Event);
    btif_cmgr_handler_t *cmgrHandler = NULL;
    btif_remote_device_t *btm_conn = NULL;
    bt_bdaddr_t *remote_addr = NULL;
    uint16_t conn_handle = BT_INVALID_CONN_HANDLE;
    uint8_t active_source_cons = 0;

    if (etype != BTIF_BTEVENT_ACL_DATA_ACTIVE && etype != BTIF_BTEVENT_ACL_DATA_NOT_ACTIVE)
    {
        TRACE(2, "%s %d", __func__, etype);
    }
    app_bt_source_coex_global_event_ind(Event);
    switch(etype){
        case BTIF_BTEVENT_LINK_CONNECT_CNF:
        case BTIF_BTEVENT_LINK_CONNECT_IND:
            remote_addr = btif_me_get_callback_event_address(Event);
            conn_handle = btif_me_get_callback_event_handle(Event);
            btm_conn = btif_me_get_callback_event_rem_dev(Event);

            btif_me_set_creating_source_link(remote_addr, false);

            app_bt_report_source_link_connected(btm_conn, err_code);

            if (BTIF_BEC_NO_ERROR == err_code)
            {
                active_source_cons = btif_me_get_source_activeCons();

                TRACE(3, "%s ::BTEVENT_LINK_CONNECT_CNF/IND %d activeCons %d", __func__, etype, active_source_cons);

                btif_remote_device_t *p_remote_dev = app_bt_get_remote_dev_by_handle(conn_handle);
                btif_me_set_link_policy(p_remote_dev, BTIF_BLP_MASTER_SLAVE_SWITCH|BTIF_BLP_SNIFF_MODE);

                if (active_source_cons >= BT_SOURCE_DEVICE_NUM)
                {
                    if(besbt_cfg.a2dp_sink_enable == true)
                    {
                        app_bt_set_accessible_by_source();
                    }
                    else
                    {
                        app_bt_source_set_connectable_state(false);
                    }
                }
                else
                {
                    app_bt_source_set_connectable_state(true);
                }

                if (active_source_cons > BT_SOURCE_DEVICE_NUM)
                {
                    app_bt_MeDisconnectLink(conn_handle);
                }
            }
            else
            {
                TRACE(3, "%s ::BTEVENT_LINK_CONNECT_CNF/IND %d err_code %x", __func__, etype, err_code);
                app_bt_source_set_connectable_state(true);
            }
            break;
        case BTIF_BTEVENT_LINK_DISCONNECT:
            TRACE(2, "%s ::BTIF_BTEVENT_LINK_DISCONNECT err_code %x", __func__, err_code);

            remote_addr = btif_me_get_callback_event_address(Event);
            conn_handle = btif_me_get_callback_event_handle(Event);

            active_source_cons = btif_me_get_source_activeCons();
            if(active_source_cons < BT_SOURCE_DEVICE_NUM)
            {
                app_bt_source_set_connectable_state(true);
            }

            btif_me_set_creating_source_link(remote_addr, false);

#ifdef BT_A2DP_SRC_ROLE
            app_bt_source_notify_a2dp_link_disconnected(remote_addr);
#endif
            app_bt_report_source_link_disconnected(remote_addr, err_code);
            break;
        case BTIF_BTEVENT_SCO_CONNECT_IND:
            TRACE(2, "%s ::BTEVENT_SCO_CONNECT_IND %x", __func__, err_code);
            break;
        case BTIF_BTEVENT_SCO_DISCONNECT:
            TRACE(2, "%s ::BTEVENT_SCO_DISCONNECT %x", __func__, err_code);
            break;
        case BTIF_BTEVENT_NAME_RESULT:
            break;
        case BTIF_BTEVENT_AUTHENTICATED:
            {
                TRACE(2, "%s ::BTEVENT_AUTHENTICATED %x", __func__, err_code);
                bool delete_record_link_key_only = true;
                //after authentication completes, re-enable sniff mode.
                if (err_code == BTIF_BEC_AUTHENTICATE_FAILURE || err_code == BTIF_BEC_MISSING_KEY)
                {
                    //auth failed should clear nv record link key
                    bt_bdaddr_t *bd_ddr = btif_me_get_callback_event_address(Event);
                    btif_device_record_t record;
                    DUMP8("%02x ", bd_ddr->address, 6);
                    if (ddbif_find_record(bd_ddr, &record) == BT_STS_SUCCESS)
                    {
                        TRACE(0,"delete link key as authen fail");
                        ddbif_delete_record(bd_ddr);
                        if (delete_record_link_key_only)
                        {
                            TRACE(0, "app_bt_global_handle: delete link key");
                            memset(&record, 0, sizeof(record));
                            record.bdAddr = *bd_ddr;
                            ddbif_add_record(&record);
                        }
                        else
                        {
                            TRACE(0, "app_bt_global_handle: delete record");
                        }
                        nv_record_flash_flush();
                    }
                }
            }
            break;
        case BTIF_BTEVENT_SIMPLE_PAIRING_COMPLETE:
            TRACE(2, "%s ::BTEVENT_SIMPLE_PAIRING_COMPLETE %x", __func__, err_code);
            break;
        case BTIF_BTEVENT_ENCRYPTION_CHANGE:
#if defined(mHDT_SUPPORT)
            {
                btif_remote_device_t *p_remote_dev = btif_me_get_callback_event_rem_dev(Event);
                btif_me_mhdt_enter_mhdt_mode(p_remote_dev, 0x08, 0x08);
            }
#endif
            TRACE(2, "%s ::BTEVENT_ENCRYPTION_CHANGE %x", __func__, err_code);
            break;
        case BTIF_BTEVENT_ACL_DATA_NOT_ACTIVE:
            app_bt_conn_stop_sniff(btif_me_get_callback_event_handle(Event));
            break;
        case BTIF_BTEVENT_ACL_DATA_ACTIVE:
            cmgrHandler = btif_cmgr_get_acl_handler(btif_me_get_callback_event_handle(Event));
            if (cmgrHandler) {
                app_bt_CMGR_SetSniffTimer(cmgrHandler, NULL, BTIF_CMGR_SNIFF_TIMER);
            }
            break;
        case BTIF_BTEVENT_MAX_SLOT_CHANGED:
            TRACE(3, "%s ::BTEVENT_MAX_SLOT_CHANGED %x max_slot %d", __func__, err_code, btif_me_get_callback_event_max_slot(Event));
            break;
        case BTIF_BTEVENT_CONN_PACKET_TYPE_CHNG:
            TRACE(3, "%s ::BTEVENT_CONN_PACKET_TYPE_CHNG %x %04x", __func__, err_code, btif_me_get_callback_event_packet_type(Event));
            break;
        case BTIF_BTEVENT_ACCESSIBLE_CHANGE:
            TRACE(3, "%s ::BTEVENT_ACCESSIBLE_CHANGE %x accessible %d", __func__, err_code, btif_me_get_callback_event_a_mode(Event));
            app_bt_access_mode_change_event_handle(Event, err_code);
            break;
        case BTIF_BTEVENT_ROLE_CHANGE:
            btm_conn = btif_me_get_callback_event_rem_dev(Event);
            TRACE(3, "%s ::BTEVENT_ROLE_CHANGE %x role %d", __func__, err_code, btif_me_get_current_role(btm_conn));
            break;
        case BTIF_BTEVENT_MODE_CHANGE:
            btm_conn = btif_me_get_callback_event_rem_dev(Event);
            TRACE(3, "%s ::BTEVENT_MODE_CHANGE %x mode %d", __func__, err_code, btif_me_get_current_role(btm_conn));
            break;
        default:
            TRACE(2, "%s unknown event %d", __func__, etype);
            break;
    }
}

void app_bt_source_disconnect_all_connections(bool power_off)
{
    int i = 0;
    struct BT_SOURCE_DEVICE_T *device = NULL;

    TRACE(2, "%s %d", __func__, power_off);

    for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1)
    {
        device = app_bt_source_get_device(i);
        if (device->base_device->a2dp_conn_flag)
        {
            app_bt_disconnect_a2dp_profile(device->base_device->a2dp_connected_stream);
        }
        if (device->base_device->avrcp_conn_flag)
        {
            app_bt_disconnect_avrcp_profile(device->base_device->avrcp_channel);
        }
        if (device->base_device->hf_conn_flag)
        {
            app_bt_disconnect_hfp_profile(device->base_device->hf_channel);
        }
    }
}

void app_bt_source_disconnect_mobile_connections(void)
{
    struct BT_DEVICE_T *curr_device = NULL;

    TRACE(1, "%s", __func__);
    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->a2dp_conn_flag)
        {
            app_bt_disconnect_a2dp_profile(curr_device->a2dp_connected_stream);
        }
        if (curr_device->avrcp_conn_flag)
        {
            app_bt_disconnect_avrcp_profile(curr_device->avrcp_channel);
        }
        if (curr_device->hf_conn_flag)
        {
            app_bt_disconnect_hfp_profile(curr_device->hf_channel);
        }
    }
}

static btif_handler g_app_bt_source_handler;
void app_bt_source_init_global_handler(void)
{
    g_app_bt_source_handler.callback = bt_source_global_callback_handler;

    btif_me_set_event_mask(&g_app_bt_source_handler,
            BTIF_BEM_LINK_CONNECT_CNF|
            BTIF_BEM_LINK_CONNECT_IND|
            BTIF_BEM_LINK_DISCONNECT|
            BTIF_BEM_MAX_SLOT_CHANGED |
            BTIF_BEM_CONN_PACKET_TYPE_CHNG |
            BTIF_BEM_SCO_CONNECT_CNF|
            BTIF_BEM_SCO_CONNECT_IND|
            BTIF_BEM_SCO_DISCONNECT|
            BTIF_BEM_ACCESSIBLE_CHANGE|
            BTIF_BEM_ROLE_CHANGE|
            BTIF_BEM_MODE_CHANGE|
            BTIF_BEM_AUTHENTICATED|
            BTIF_BEM_SIMPLE_PAIRING_COMPLETE|
            BTIF_BEM_ENCRYPTION_CHANGE);

    btif_me_set_bt_source_event_handler(&g_app_bt_source_handler);
}

static void bt_source_device_search_complete_callback(bt_bdaddr_t *remote_addr)
{
    bt_source_event_param_t param;
    uint8_t connected_source_count = app_bt_source_count_connected_device();

    if (remote_addr == NULL)
    {
        TRACE(2, "%s device not found, connected source %d", __func__, connected_source_count);
    }
    else
    {
        TRACE(7, "connected source %d, start conn %02x:%02x:%02x:%02x:%02x:%02x", connected_source_count,
                remote_addr->address[0], remote_addr->address[1], remote_addr->address[2],
                remote_addr->address[3], remote_addr->address[4], remote_addr->address[5]);

        bt_source_perform_profile_reconnect(remote_addr);
    }

    BT_SOURCE_CB(BT_SOURCE_EVENT_SEARCH_COMPLETE, &param);
}

static void bt_source_device_search_result_callback(app_bt_search_result_t *result)
{
    bt_source_event_param_t param;
    param.p.search_result.result = result;
    BT_SOURCE_CB(BT_SOURCE_EVENT_SEARCH_RESULT, &param);
}

void app_bt_source_search_device(void)
{
    bool enabled = app_bt_source_is_enabled();
    uint8_t connected_source_count = app_bt_source_count_connected_device();
    POSSIBLY_UNUSED struct BT_SOURCE_DEVICE_T *curr_device = NULL;

    TRACE(2,"app_bt_source_search_device %d connected %d", enabled, connected_source_count);

    if (enabled && connected_source_count < BT_SOURCE_DEVICE_NUM)
    {
        app_bt_source_set_connectable_state(false);

#if (BT_SOURCE_DEVICE_NUM > 1)
        app_bt_clear_search_except_device_list();
        for (int i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1)
        {
            curr_device = app_bt_source_get_device(i);
            if (curr_device->base_device->acl_is_connected)
            {
                app_bt_add_search_except_device(&curr_device->base_device->remote);
            }
        }
#endif
        app_bt_start_search_with_callback(bt_source_device_search_complete_callback, bt_source_device_search_result_callback);
    }
}

#if defined(A2DP_SOURCE_TEST) || defined(HFP_AG_TEST)
void app_bt_source_set_source_addr(uint8_t *addr)
{
    memcpy(addr,&source_test_bt_addr[0],6);
}
#endif

bool app_bt_source_check_sink_audio_activity(void)
{
    uint8_t sink_streaming_sco_count = app_bt_audio_count_connected_sco();
    uint8_t sink_streaming_a2dp_count = app_bt_audio_count_streaming_a2dp();

    TRACE(2,"sink_streaming_sco_count %d sink_streaming_a2dp_count %d", sink_streaming_sco_count, sink_streaming_a2dp_count);

    return sink_streaming_sco_count != 0;
}

bool app_bt_source_has_streaming_a2dp(void)
{
    struct BT_SOURCE_DEVICE_T *device = NULL;

    for(int i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i++)
    {
        device = app_bt_source_get_device(i);
        if (device->base_device->a2dp_streamming)
        {
            return true;
        }
    }

    return false;
}

bool app_bt_source_has_streaming_sco(void)
{
    struct BT_SOURCE_DEVICE_T *device = NULL;

    for(int i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i++)
    {
        device = app_bt_source_get_device(i);
        if (device->base_device->hf_audio_state == BT_HFP_AUDIO_CON)
        {
            return true;
        }
    }

    return false;
}

uint8_t app_bt_source_count_streaming_sco(void)
{
    struct BT_SOURCE_DEVICE_T *device = NULL;
    uint8_t count = 0;

    for(int i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i++)
    {
        device = app_bt_source_get_device(i);
        if (device->base_device->hf_audio_state == BT_HFP_AUDIO_CON)
        {
            count += 1;
        }
    }

    TRACE(2,"%s %d", __func__, count);

    return count;
}

uint8_t app_bt_source_count_streaming_a2dp(void)
{
    struct BT_SOURCE_DEVICE_T *device = NULL;
    uint8_t count = 0;

    for(int i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i++)
    {
        device = app_bt_source_get_device(i);
        if (device->base_device->a2dp_streamming)
        {
            count += 1;
        }
    }

    return count;
}

uint8_t app_bt_source_count_streaming_aac(void)
{
#ifdef A2DP_SOURCE_AAC_ON
    struct BT_SOURCE_DEVICE_T *device = NULL;
    uint8_t count = 0;

    for(int i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i++)
    {
        device = app_bt_source_get_device(i);
        if (device->base_device->a2dp_streamming && device->base_device->codec_type == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC)
        {
            count += 1;
        }
    }

    return count;
#else
    return 0;
#endif
}

uint8_t app_bt_source_count_streaming_lhdc(void)
{
#ifdef A2DP_SOURCE_LHDC_ON
    struct BT_SOURCE_DEVICE_T *device = NULL;
    uint8_t count = 0;

    for(int i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i++)
    {
        device = app_bt_source_get_device(i);
        if (device->base_device->a2dp_streamming && \
            (device->base_device->codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP) && \
            (device->base_device->a2dp_non_type == A2DP_SOURCE_NON_TYPE_LHDC))
        {
            count += 1;
        }
    }

    return count;
#else
    return 0;
#endif
}

uint8_t app_bt_source_count_streaming_lhdcv5(void)
{
#ifdef A2DP_SOURCE_LHDCV5_ON
    struct BT_SOURCE_DEVICE_T *device = NULL;
    uint8_t count = 0;

    for(int i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i++)
    {
        device = app_bt_source_get_device(i);
        if (device->base_device->a2dp_streamming && \
            (device->base_device->codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP) && \
            (device->base_device->a2dp_non_type == A2DP_SOURCE_NON_TYPE_LHDCV5))
        {
            count += 1;
        }
    }

    return count;
#else
    return 0;
#endif
}

uint8_t app_bt_source_count_streaming_ldac(void)
{
#ifdef A2DP_SOURCE_LDAC_ON
    struct BT_SOURCE_DEVICE_T *device = NULL;
    uint8_t count = 0;

    for(int i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i++)
    {
        device = app_bt_source_get_device(i);
        if (device->base_device->a2dp_streamming && \
            (device->base_device->codec_type == BT_A2DP_CODEC_TYPE_LDAC) && \
            (device->base_device->a2dp_non_type == A2DP_SOURCE_NON_TYPE_LDAC))
        {
            count += 1;
        }
    }

    return count;
#else
    return 0;
#endif
}

uint8_t app_bt_source_count_streaming_sbc(void)
{
    struct BT_SOURCE_DEVICE_T *device = NULL;
    uint8_t count = 0;

    for(int i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i++)
    {
        device = app_bt_source_get_device(i);
        if (device->base_device->a2dp_streamming && device->base_device->codec_type == BT_A2DP_CODEC_TYPE_SBC)
        {
            count += 1;
        }
    }

    return count;
}

uint8_t app_bt_source_count_connected_device(void)
{
    struct BT_SOURCE_DEVICE_T *device = NULL;
    uint8_t count = 0;

    for(int i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i++)
    {
        device = app_bt_source_get_device(i);
        if (device->base_device->acl_is_connected)
        {
            count += 1;
        }
    }

    return count;
}

static void app_bt_source_start_a2dp_playing(uint8_t device_id)
{
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;

    curr_device = app_bt_source_get_device(device_id);

    if (!curr_device->base_device->a2dp_streamming)
    {
        app_a2dp_source_start_stream(device_id);
    }
    else
    {
        a2dp_source_start_pcm_capture(device_id);
    }
}

static void app_bt_source_stop_a2dp_playing(uint8_t device_id)
{
    app_a2dp_source_suspend_stream(device_id);

    a2dp_source_stop_pcm_capture(device_id);
}

static uint8_t app_bt_source_get_playing_a2dp(void)
{
    return bt_source_manager.curr_source_playing_a2dp;
}

static uint8_t app_bt_source_select_streaming_a2dp(void)
{
    struct BT_SOURCE_DEVICE_T* curr_device = NULL;
    uint32_t stream_prio = 0;
    uint8_t device_id = BT_SOURCE_DEVICE_INVALID_ID;
    int i = 0;

    for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; ++i)
    {
        curr_device = app_bt_source_get_device(i);
        if (curr_device->base_device->a2dp_streamming)
        {
            if (curr_device->base_device->a2dp_audio_prio > stream_prio)
            {
                stream_prio = curr_device->base_device->a2dp_audio_prio;
                device_id = i;
            }
        }
    }

    return device_id;
}

uint8_t app_bt_source_get_streaming_a2dp(void)
{
    uint8_t device_id = BT_SOURCE_DEVICE_INVALID_ID;

    device_id = app_bt_source_get_playing_a2dp();

    if (device_id == BT_SOURCE_DEVICE_INVALID_ID)
    {
        device_id = app_bt_source_select_streaming_a2dp();
    }

    return device_id;
}

uint8_t app_bt_source_get_current_a2dp(void)
{
    uint8_t device_id = BT_SOURCE_DEVICE_INVALID_ID;

    device_id = app_bt_source_get_streaming_a2dp();

    if (device_id == BT_SOURCE_DEVICE_INVALID_ID)
    {
        device_id = bt_source_manager.curr_source_a2dp_id;
    }

    return device_id;
}

struct BT_SOURCE_DEVICE_T *app_bt_source_get_current_a2dp_device(void)
{
    uint8_t device_id = app_bt_source_get_current_a2dp();
    return app_bt_source_get_device(device_id);
}

static uint8_t app_bt_source_select_connected_a2dp(void)
{
    struct BT_SOURCE_DEVICE_T* curr_device = NULL;
    uint32_t stream_prio = 0;
    uint8_t device_id = BT_SOURCE_DEVICE_INVALID_ID;
    int i = 0;

    for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; ++i)
    {
        curr_device = app_bt_source_get_device(i);
        if (curr_device->base_device->a2dp_conn_flag)
        {
            if (curr_device->base_device->a2dp_conn_prio > stream_prio)
            {
                stream_prio = curr_device->base_device->a2dp_conn_prio;
                device_id = i;
            }
        }
    }

    return device_id;
}

static uint8_t app_bt_source_get_playing_sco(void)
{
    return bt_source_manager.curr_source_playing_sco;
}

static uint8_t app_bt_source_select_streaming_sco(void)
{
    struct BT_SOURCE_DEVICE_T* curr_device = NULL;
    uint32_t stream_prio = 0;
    uint8_t device_id = BT_SOURCE_DEVICE_INVALID_ID;
    int i = 0;

    for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; ++i)
    {
        curr_device = app_bt_source_get_device(i);
        if (curr_device->base_device->hf_audio_state == BT_HFP_AUDIO_CON)
        {
            if (curr_device->base_device->sco_audio_prio > stream_prio)
            {
                stream_prio = curr_device->base_device->sco_audio_prio;
                device_id = i;
            }
        }
    }

    return device_id;
}

static uint8_t app_bt_source_select_connected_hfp(void)
{
    struct BT_SOURCE_DEVICE_T* curr_device = NULL;
    uint8_t device_id = BT_SOURCE_DEVICE_INVALID_ID;
    uint32_t hfp_prio = 0;
    int i = 0;

    for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; ++i)
    {
        curr_device = app_bt_source_get_device(i);
        if (curr_device->base_device->hf_conn_flag)
        {
            if (curr_device->base_device->hfp_conn_prio > hfp_prio)
            {
                hfp_prio = curr_device->base_device->hfp_conn_prio;
                device_id = i;
            }
        }
    }

    return device_id;
}

uint8_t app_bt_source_get_streaming_sco(void)
{
    uint8_t device_id = BT_SOURCE_DEVICE_INVALID_ID;

    device_id = app_bt_source_get_playing_sco();

    if (device_id == BT_SOURCE_DEVICE_INVALID_ID)
    {
        device_id = app_bt_source_select_streaming_sco();
    }

    return device_id;
}

uint8_t app_bt_source_get_current_hfp(void)
{
    uint8_t device_id = BT_SOURCE_DEVICE_INVALID_ID;

    device_id = app_bt_source_get_streaming_sco();

    if (device_id == BT_SOURCE_DEVICE_INVALID_ID)
    {
        device_id = bt_source_manager.curr_source_hfp_id;
    }

    return device_id;
}

struct BT_SOURCE_DEVICE_T *app_bt_source_get_current_hfp_device(void)
{
    uint8_t device_id = app_bt_source_get_current_hfp();
    return app_bt_source_get_device(device_id);
}

static void app_bt_source_set_ag_curr_stream(uint8_t device_id)
{
    if (device_id == BT_DEVICE_AUTO_CHOICE_ID)
    {
        device_id = app_bt_source_get_playing_sco();
        if (device_id == BT_SOURCE_DEVICE_INVALID_ID)
        {
            device_id = app_bt_source_select_streaming_sco();
            if (device_id == BT_SOURCE_DEVICE_INVALID_ID)
            {
                device_id = app_bt_source_select_connected_hfp();
                if (device_id == BT_SOURCE_DEVICE_INVALID_ID)
                {
                    device_id = BT_SOURCE_DEVICE_ID_1;
                }
            }
        }
    }

    TRACE(2, "%s as %x", __func__, device_id);

    if (device_id == BT_DEVICE_INVALID_ID)
    {
        return;
    }

    bt_source_manager.curr_source_hfp_id = device_id;
}

void app_bt_source_set_a2dp_curr_stream(uint8_t device_id)
{
    if (device_id == BT_DEVICE_AUTO_CHOICE_ID)
    {
        device_id = app_bt_source_get_playing_a2dp();
        if (device_id == BT_SOURCE_DEVICE_INVALID_ID)
        {
            device_id = app_bt_source_select_streaming_a2dp();
            if (device_id == BT_SOURCE_DEVICE_INVALID_ID)
            {
                device_id = app_bt_source_select_connected_a2dp();
                if (device_id == BT_SOURCE_DEVICE_INVALID_ID)
                {
                    device_id = BT_SOURCE_DEVICE_ID_1;
                }
            }
        }
    }

    TRACE(2, "%s as %x", __func__, device_id);

    if (device_id == BT_SOURCE_DEVICE_INVALID_ID)
    {
        return;
    }

    bt_source_manager.curr_source_a2dp_id = device_id;
}

static void app_bt_source_stop_sco_playing(uint8_t device_id)
{
    if (bt_source_manager.curr_source_playing_sco == device_id)
    {
        bt_source_manager.curr_source_playing_sco = BT_SOURCE_DEVICE_INVALID_ID;
        Audio_device_t audio_device;
        audio_device.audio_device.device_id = device_id;
        audio_device.audio_device.device_type = AUDIO_TYPE_BT;
        audio_device.aud_id = MAX_RECORD_NUM;
        audio_player_playback_stop(AUDIO_STREAM_VOICE, &audio_device);
    }
    else
    {
        //bt_media_clear_media_type(BT_STREAM_VOICE, (enum BT_DEVICE_ID_T)device_id);
        //todo remove to bthost
    }

    app_bt_source_set_ag_curr_stream(BT_DEVICE_AUTO_CHOICE_ID);
}

static void app_bt_source_start_sco_playing(uint8_t device_id)
{
    app_bt_source_set_ag_curr_stream(device_id);

#ifndef AUDIO_PROMPT_USE_DAC2ORDAC3_ENABLED
    for (int i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1) // stop media if any
    {
        if (app_audio_adm_media_stream_is_active())
        {
            app_bt_audio_stop_media_playing(i);
        }
    }
#endif

    if (bt_source_manager.curr_source_playing_sco != device_id)
    {
        bt_source_manager.curr_source_playing_sco = device_id;

        Audio_device_t audio_device;
        audio_device.audio_device.device_id = device_id;
        audio_device.audio_device.device_type = AUDIO_TYPE_BT;
        audio_device.aud_id = MAX_RECORD_NUM;
        audio_player_play(AUDIO_STREAM_VOICE, &audio_device);
    }
}

static void app_bt_source_swap_sco_playing(uint8_t device_id)
{
    app_bt_source_set_ag_curr_stream(device_id);

    bt_source_manager.curr_source_playing_sco = device_id;

    audio_player_swap_sco(AUDIO_STREAM_VOICE, device_id);
}

static void app_bt_source_select_sco_stream_to_play(uint8_t device_id)
{
    // TODO: check sink audio activity

    uint8_t select_sco_stream = app_bt_source_select_streaming_sco();
    uint8_t curr_playing_sco = app_bt_source_get_playing_sco();
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;

    TRACE(4,"(d%x) %s curr_playing_sco %x select_sco_stream %x", device_id, __func__, curr_playing_sco, select_sco_stream);

    if (select_sco_stream == curr_playing_sco)
    {
        // current selected sco is playing
        app_bt_source_start_sco_playing(select_sco_stream);
        return;
    }

    if (curr_playing_sco != BT_DEVICE_INVALID_ID) // sco is playing
    {
        app_bt_source_swap_sco_playing(select_sco_stream);
        TRACE(3,"(d%x) %s disconnect ag sco %x", device_id, __func__, curr_playing_sco);
        curr_device = app_bt_source_get_device(curr_playing_sco);
        app_bt_HF_DisconnectAudioLink(curr_device->base_device->hf_channel);
    }
    else
    {
        app_bt_source_start_sco_playing(select_sco_stream);
    }
}

void app_bt_source_audio_event_handler(uint8_t device_id, enum app_bt_source_audio_event_t event,
                                    struct app_bt_source_audio_event_param_t *event_param)
{
    int i = 0;
    bt_source_event_param_t param;
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;
    struct BT_SOURCE_DEVICE_T *source_device = NULL;

    if (event >= APP_BT_SOURCE_AUDIO_EVENT_AG_SERVICE_CONNECTED)
    {
        curr_device = app_bt_source_get_device(device_id);
    }
    if (curr_device)
    {
        app_bt_source_coex_audio_event_ind(&curr_device->base_device->remote, event, event_param);
    }
    switch (event)
    {
        case APP_BT_SOURCE_AUDIO_EVENT_HF_SCO_CONNECTED:
            if (app_bt_source_has_streaming_a2dp())
            {
                for(i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i++)
                {
                    source_device = app_bt_source_get_device(i);

                    if (source_device->base_device->a2dp_streamming)
                    {
                        TRACE(2, "(d%x) hf sco pause a2dp source d%x", device_id, i);

                        source_device->a2dp_paused_by_sink = true;

                        app_bt_source_stop_a2dp_playing(i);
                    }
                }
            }
            break;
        case APP_BT_SOURCE_AUDIO_EVENT_HF_SCO_DISCONNECTED:
            for(i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i++)
            {
                source_device = app_bt_source_get_device(i);

                if (source_device->a2dp_paused_by_sink)
                {
                    TRACE(4, "(d%x) hf sco resume a2dp source d%x conn_flag %d strming %d", device_id, i,
                            source_device->base_device->a2dp_conn_flag, source_device->base_device->a2dp_streamming);

                    source_device->a2dp_paused_by_sink = false;

                    if (source_device->base_device->a2dp_conn_flag)
                    {
                        if (app_bt_source_count_streaming_sco())
                        {
                            source_device->a2dp_paused_by_ag_sco = true;
                        }
                        else
                        {
                            app_bt_source_start_a2dp_playing(i);
                        }
                    }
                }
            }
            break;
        case APP_BT_SOURCE_AUDIO_EVENT_AG_SERVICE_CONNECTED:
            curr_device->base_device->sco_audio_prio = 0;
            curr_device->base_device->hfp_conn_prio = app_bt_audio_create_new_prio();
            app_bt_source_set_ag_curr_stream(BT_DEVICE_AUTO_CHOICE_ID);
            break;
        case APP_BT_SOURCE_AUDIO_EVENT_AG_SCO_CONNECTED:
            if (app_bt_source_has_streaming_a2dp())
            {
                for(i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i++)
                {
                    source_device = app_bt_source_get_device(i);

                    if (source_device->base_device->a2dp_streamming)
                    {
                        TRACE(2, "(d%x) ag sco pause a2dp source d%x", device_id, i);

                        source_device->a2dp_paused_by_ag_sco = true;

                        app_bt_source_stop_a2dp_playing(i);
                    }
                }
            }
            curr_device->base_device->sco_audio_prio = app_bt_audio_create_new_prio();
            app_bt_source_select_sco_stream_to_play(device_id);
            break;
        case APP_BT_SOURCE_AUDIO_EVENT_AG_SERVICE_DISCONNECTED:
            curr_device->base_device->hfp_conn_prio = 0;
            /* fall through */
        case APP_BT_SOURCE_AUDIO_EVENT_AG_SCO_DISCONNECTED:
            curr_device->base_device->sco_audio_prio = 0;
            app_bt_source_stop_sco_playing(device_id);
            if (!app_bt_source_count_streaming_sco())
            {
                for(i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i++)
                {
                    source_device = app_bt_source_get_device(i);

                    if (source_device->a2dp_paused_by_ag_sco)
                    {
                        TRACE(4, "(d%x) ag sco resume a2dp source d%x conn_flag %d strming %d", device_id, i,
                                source_device->base_device->a2dp_conn_flag, source_device->base_device->a2dp_streamming);

                        source_device->a2dp_paused_by_ag_sco = false;

                        if (source_device->base_device->a2dp_conn_flag)
                        {
                            if (app_bt_source_check_sink_audio_activity())
                            {
                                source_device->a2dp_paused_by_sink = true;
                            }
                            else
                            {
                                app_bt_source_start_a2dp_playing(i);
                            }
                        }
                    }
                }
            }
            else
            {
                app_bt_source_select_sco_stream_to_play(device_id);
            }
            break;
        case APP_BT_SOURCE_AUDIO_EVENT_SRC_CONNECT_FAIL:
            param.p.a2dp_source_connect_fail.addr = event_param->p.a2dp_source_connect_fail.addr;
            BT_SOURCE_CB(BT_SOURCE_EVENT_A2DP_SOURCE_CONNECT_FAIL, &param);
            break;
        case APP_BT_SOURCE_AUDIO_EVENT_SRC_STREAM_OPEN:
            curr_device->base_device->a2dp_audio_prio = 0;
            curr_device->base_device->a2dp_conn_prio = app_bt_audio_create_new_prio();
            app_bt_source_set_a2dp_curr_stream(BT_DEVICE_AUTO_CHOICE_ID);

            param.p.a2dp_source_stream_open.addr = &curr_device->base_device->remote;
            param.p.a2dp_source_stream_open.device_id = device_id;
            BT_SOURCE_CB(BT_SOURCE_EVENT_A2DP_SOURCE_STREAM_OPEN, &param);
            break;
        case APP_BT_SOURCE_AUDIO_EVENT_SRC_STREAM_START:
            curr_device->base_device->a2dp_audio_prio = app_bt_audio_create_new_prio();
            if (app_bt_source_check_sink_audio_activity())
            {
                curr_device->a2dp_paused_by_sink = true;
            }
            else if (app_bt_source_count_streaming_sco())
            {
                curr_device->a2dp_paused_by_ag_sco = true;
            }
            else
            {
                a2dp_source_start_pcm_capture(device_id);
            }
            app_bt_source_set_a2dp_curr_stream(device_id);
            break;
        case APP_BT_SOURCE_AUDIO_EVENT_SRC_STREAM_CLOSE:
            curr_device->base_device->a2dp_conn_prio = 0;

            param.p.a2dp_source_stream_close.addr = &curr_device->base_device->remote;
            param.p.a2dp_source_stream_close.device_id = device_id;
            BT_SOURCE_CB(BT_SOURCE_EVENT_A2DP_SOURCE_STREAM_CLOSE, &param);
            /* fall through */
        case APP_BT_SOURCE_AUDIO_EVENT_SRC_STREAM_SUSPEND:
            curr_device->base_device->a2dp_audio_prio = 0;
            a2dp_source_stop_pcm_capture(device_id);
            app_bt_source_set_a2dp_curr_stream(BT_DEVICE_AUTO_CHOICE_ID);
            break;
        default:
            TRACE(2, "%s invalid event %d", __func__, event);
            break;
    }
}

void app_bt_source_set_hfp_ag_pts_enable(bool enable)
{
    besbt_cfg.hfp_ag_pts_enable = enable;
}

void app_bt_source_set_hfp_ag_pts_esc_01_enable(bool enable)
{
    besbt_cfg.hfp_ag_pts_ecs_01 = enable;
}

void app_bt_source_set_hfp_ag_pts_esc_02_enable(bool enable)
{
    besbt_cfg.hfp_ag_pts_ecs_02 = enable;
}

void app_bt_source_set_hfp_ag_pts_ecc_enable(bool enable)
{
    besbt_cfg.hfp_ag_pts_ecc = enable;
}

void app_bt_source_set_source_pts_get_all_cap_flag(bool enable)
{
    besbt_cfg.source_get_all_cap_flag = enable;
}

void app_bt_source_set_source_pts_suspend_err_flag(bool enable)
{
    besbt_cfg.source_suspend_err_flag = enable;
}

void app_bt_source_set_source_pts_unknown_cmd_flag(bool enable)
{
    besbt_cfg.source_unknown_cmd_flag = enable;
}

static bt_source_event_global_cb_t bt_source_event_global_callback = NULL;
static bt_source_audio_coex_event_cb_t bt_source_audio_coex_event_callback = NULL;

void app_bt_source_register_coex_global_event_handle(bt_source_event_global_cb_t func)
{
    bt_source_event_global_callback = func;
}

void app_bt_source_register_coex_audio_event_handler(bt_source_audio_coex_event_cb_t func)
{
    bt_source_audio_coex_event_callback = func;
}

void app_bt_source_coex_global_event_ind(const btif_event_t * event)
{
    if (bt_source_event_global_callback)
    {
        bt_source_event_global_callback(event);
    }
}

void app_bt_source_coex_audio_event_ind(bt_bdaddr_t *addr, enum app_bt_source_audio_event_t event,
                                    struct app_bt_source_audio_event_param_t *event_param)
{
    if (bt_source_audio_coex_event_callback)
    {
        bt_source_audio_coex_event_callback(addr, event, event_param);
    }
}

#endif /* BT_SOURCE */

