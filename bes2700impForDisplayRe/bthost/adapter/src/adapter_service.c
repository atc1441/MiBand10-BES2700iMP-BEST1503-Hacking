/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#include "bluetooth_bt_api.h"
#include "bt_callback_func.h"
#include "app_bt_audio.h"
#include "app_bt_func.h"
#include "intersyshci.h"
#include "bt_drv_interface.h"
#include "besbt_cfg.h"
#include "besbt.h"
#include "btapp.h"
#include "app_bt.h"
#include "app_bt_cmd.h"
#include "bt_if.h"
#include "sdp_i.h"
#include "spp_api.h"
#include "sdp_api.h"
#include "sco_api.h"
#include "l2cap_api.h"
#include "multi_heap.h"
#include "os_porting.h"
#include "app_bt_sync.h"
#ifdef BLE_HOST_SUPPORT
#include "app_ble.h"
#if BLE_AUDIO_ENABLED
#include "ble_audio_earphone_info.h"
#include "gaf_media_common.h"
#endif
#ifdef BLE_ISO_ENABLED
#include "ble_iso.h"
#endif
#endif
#ifdef BT_SOURCE
#include "bt_source.h"
#endif
#ifdef IBRT
#include "app_tws_ibrt.h"
#include "app_ibrt_ble_adv.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_vendor_cmd_evt.h"

#ifdef IBRT_UI
#include "app_tws_ibrt_conn_api.h"
void app_ibrt_conn_cmd_complete_callback(const uint8_t *para);
void app_ibrt_conn_cmd_status_callback(const void *para);
void app_ibrt_conn_stack_report_create_acl_failed(const bt_bdaddr_t *addr);
#endif
#endif

#ifdef BTH_IN_ROM
struct coheap_global_t *bt_rom_export_coheap_global(void);
#endif

#if 1
#define BT_ADAPTER_TRACE(...) TRACE(__VA_ARGS__)
#else
#define BT_ADAPTER_TRACE(...)
#endif

osMutexDef(g_bt_adapter_mutex_def);
static struct BT_ADAPTER_MANAGER_T g_bt_adapter_manager;
static int bt_adapter_event_callback(const bt_bdaddr_t *bd_addr, BT_EVENT_T event, BT_CALLBACK_PARAM_T param);

void bt_adapter_mutex_lock(void)
{
    if (g_bt_adapter_manager.adapter_lock)
    {
        osMutexWait(g_bt_adapter_manager.adapter_lock, osWaitForever);
    }
    else
    {
        TRACE(0, "bt_adapter_mutex_lock: invalid mutex");
    }
}

void bt_adapter_mutex_unlock(void)
{
    if (g_bt_adapter_manager.adapter_lock)
    {
        osMutexRelease(g_bt_adapter_manager.adapter_lock);
    }
}

static void bt_adapter_clear_device(struct BT_ADAPTER_DEVICE_T *curr_device)
{
    bt_adapter_mutex_lock();
    curr_device->acl_is_connected = false;
    curr_device->sco_is_connected = false;
    curr_device->hfp_ctx.hfp_is_connected = false;
    curr_device->a2dp_ctx.a2dp_is_connected = false;
    curr_device->a2dp_ctx.a2dp_is_streaming = false;
    curr_device->avrcp_ctx.avrcp_is_connected = false;
    curr_device->acl_conn_hdl = BT_INVALID_CONN_HANDLE;
    curr_device->sco_handle = BT_INVALID_CONN_HANDLE;
    curr_device->sco_codec_type = BT_HFP_SCO_CODEC_CVSD;
    bt_adapter_mutex_unlock();
}

void bt_adapter_manager_init(void)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    memset(&g_bt_adapter_manager, 0, sizeof(g_bt_adapter_manager));

    if (g_bt_adapter_manager.adapter_lock == NULL)
    {
        g_bt_adapter_manager.adapter_lock = osMutexCreate(osMutex(g_bt_adapter_mutex_def));
    }

    ASSERT(sizeof(BT_CALLBACK_PARAM_T) == sizeof(void *), "BT_CALLBACK_PARAM_T error define");
    ASSERT(BT_EVENT_ACL_OPENED == 0x1000 && BT_EVENT_HF_OPENED == 0x1100, "bt event group error define");

#ifndef BT_BUILD_WITH_CUSTOMER_HOST
    bt_add_event_callback(bt_adapter_event_callback,
        BT_EVENT_MASK_LINK_GROUP |
        BT_EVENT_MASK_HFP_HF_GROUP |
        BT_EVENT_MASK_HFP_AG_GROUP |
        BT_EVENT_MASK_A2DP_SNK_GROUP |
        BT_EVENT_MASK_A2DP_SRC_GROUP |
        BT_EVENT_MASK_AVRCP_GROUP);
#endif

    for (int i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_sink_device + i;
        bt_adapter_clear_device(curr_device);
        curr_device->device_id = i;
    }

    for (int i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_source_device + i;
        bt_adapter_clear_device(curr_device);
        curr_device->device_id = BT_SOURCE_DEVICE_ID_BASE + i;
    }

    curr_device = &g_bt_adapter_manager.bt_tws_device;
    bt_adapter_clear_device(curr_device);
    curr_device->device_id = BT_DEVICE_TWS_ID;
}

struct BT_ADAPTER_DEVICE_T *bt_adapter_get_device(int device_id)
{
    if (device_id < BT_ADAPTER_MAX_DEVICE_NUM)
    {
        return g_bt_adapter_manager.bt_sink_device + device_id;
    }
    else if (device_id >= BT_SOURCE_DEVICE_ID_BASE && device_id < BT_SOURCE_DEVICE_ID_BASE + BT_ADAPTER_MAX_DEVICE_NUM)
    {
        return g_bt_adapter_manager.bt_source_device + (device_id - BT_SOURCE_DEVICE_ID_BASE);
    }
    else if (device_id == BT_DEVICE_TWS_ID)
    {
        return &g_bt_adapter_manager.bt_tws_device;
    }
    else
    {
        TRACE(0, "bt_adapter_get_device: invalid device %x ca=%p", device_id, __builtin_return_address(0));
        return NULL;
    }
}

struct BT_ADAPTER_DEVICE_T *bt_adapter_get_connected_device_by_id(int device_id)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    struct BT_ADAPTER_DEVICE_T *connected_device = NULL;

    bt_adapter_mutex_lock();

    curr_device = bt_adapter_get_device(device_id);

    if (curr_device && curr_device->acl_is_connected)
    {
        connected_device = curr_device;
    }

    bt_adapter_mutex_unlock();

    return connected_device;
}

struct BT_ADAPTER_DEVICE_T *bt_adapter_get_connected_device_by_connhdl(uint16_t connhdl)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    int i = 0;

    bt_adapter_mutex_lock();

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_sink_device + i;
        if (curr_device->acl_is_connected && curr_device->acl_conn_hdl == connhdl)
        {
            bt_adapter_mutex_unlock();
            return curr_device;
        }
    }

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_source_device + i;
        if (curr_device->acl_is_connected && curr_device->acl_conn_hdl == connhdl)
        {
            bt_adapter_mutex_unlock();
            return curr_device;
        }
    }

    curr_device = &g_bt_adapter_manager.bt_tws_device;
    if (curr_device->acl_is_connected && curr_device->acl_conn_hdl == connhdl)
    {
        bt_adapter_mutex_unlock();
        return curr_device;
    }

    bt_adapter_mutex_unlock();
    return NULL;
}

struct BT_ADAPTER_DEVICE_T *bt_adapter_get_connected_device_byaddr(const bt_bdaddr_t *remote)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    int i = 0;

    if (remote == NULL)
    {
        return NULL;
    }

    bt_adapter_mutex_lock();

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_sink_device + i;
        if (curr_device->acl_is_connected && memcmp(&curr_device->remote, remote, sizeof(bt_bdaddr_t)) == 0)
        {
            bt_adapter_mutex_unlock();
            return curr_device;
        }
    }

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_source_device + i;
        if (curr_device->acl_is_connected && memcmp(&curr_device->remote, remote, sizeof(bt_bdaddr_t)) == 0)
        {
            bt_adapter_mutex_unlock();
            return curr_device;
        }
    }

    curr_device = &g_bt_adapter_manager.bt_tws_device;
    if (curr_device->acl_is_connected && memcmp(&curr_device->remote, remote, sizeof(bt_bdaddr_t)) == 0)
    {
        bt_adapter_mutex_unlock();
        return curr_device;
    }

    bt_adapter_mutex_unlock();
    return NULL;
}

int bt_adapter_get_device_id_by_connhdl(uint16_t connhdl)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    int device_id = BT_DEVICE_INVALID_ID;

    bt_adapter_mutex_lock();

    curr_device = bt_adapter_get_connected_device_by_connhdl(connhdl);
    if (curr_device)
    {
        device_id = curr_device->device_id;
    }

    bt_adapter_mutex_unlock();

    return device_id;
}

int bt_adapter_get_device_id_byaddr(const bt_bdaddr_t *remote)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    int device_id = BT_DEVICE_INVALID_ID;

    bt_adapter_mutex_lock();

    curr_device = bt_adapter_get_connected_device_byaddr(remote);
    if (curr_device)
    {
        device_id = curr_device->device_id;
    }

    bt_adapter_mutex_unlock();

    return device_id;
}

uint8_t bt_adapter_get_hfp_sco_codec_type(int device_id)
{
    uint8_t sco_codec = BT_HFP_SCO_CODEC_CVSD;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_device(device_id);
    if (curr_device)
    {
        sco_codec = curr_device->sco_codec_type;
    }
    bt_adapter_mutex_unlock();

    return sco_codec;
}

void bt_adapter_set_hfp_sco_codec_type(int device_id, uint8_t sco_codec)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_device(device_id);
    if (curr_device)
    {
        curr_device->sco_codec_type = sco_codec;
        TRACE(0, "(d%x) bt_adapter_set_hfp_sco_codec_type: %d", device_id, sco_codec);
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_set_a2dp_codec_info(int device_id, uint8_t codec_type, uint8_t sample_rate, uint8_t sample_bit)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_device(device_id);
    if (curr_device)
    {
        curr_device->a2dp_ctx.a2dp_codec_type = codec_type;
        curr_device->a2dp_ctx.a2dp_sample_rate = sample_rate;
        TRACE(0, "(d%x) bt_adapter_set_a2dp_codec_info: type %d sample rate %02x bit %d",
            device_id, codec_type, sample_rate, sample_bit);
    }
    bt_adapter_mutex_unlock();
}

uint8_t bt_adapter_count_mobile_link(void)
{
    int i = 0;
    uint8_t count_device = 0;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_sink_device + i;
        if (curr_device->acl_is_connected)
        {
            count_device += 1;
        }
    }

    bt_adapter_mutex_unlock();

    return count_device;
}

uint8_t bt_adapter_count_connected_hfp(void)
{
    int i = 0;
    uint8_t count_device = 0;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_sink_device + i;
        if (curr_device->hfp_ctx.hfp_is_connected)
        {
            count_device += 1;
        }
    }

    bt_adapter_mutex_unlock();

    return count_device;
}

uint8_t bt_adapter_count_streaming_a2dp(void)
{
    int i = 0;
    uint8_t count_device = 0;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_sink_device + i;
        if (curr_device->a2dp_ctx.a2dp_is_streaming)
        {
            count_device += 1;
        }
    }

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_source_device + i;
        if (curr_device->a2dp_ctx.a2dp_is_streaming)
        {
            count_device += 1;
        }
    }

    bt_adapter_mutex_unlock();

    return count_device;
}

uint8_t bt_adapter_count_streaming_sco(void)
{
    int i = 0;
    uint8_t count_device = 0;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_sink_device + i;
        if (curr_device->sco_is_connected)
        {
            count_device += 1;
        }
    }

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_source_device + i;
        if (curr_device->sco_is_connected)
        {
            count_device += 1;
        }
    }

    bt_adapter_mutex_unlock();

    return count_device;
}

uint8_t bt_adapter_has_incoming_call(void)
{
    int i = 0;
    uint8_t count_device = 0;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_sink_device + i;
        if (curr_device->hfp_ctx.hfp_is_connected && curr_device->hfp_ctx.hfp_callsetup_state == BT_HFP_CALL_SETUP_IN)
        {
            count_device += 1;
        }
    }

    for (i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_source_device + i;
        if (curr_device->hfp_ctx.hfp_is_connected && curr_device->hfp_ctx.hfp_callsetup_state == BT_HFP_CALL_SETUP_IN)
        {
            count_device += 1;
        }
    }

    bt_adapter_mutex_unlock();

    return count_device;
}

bool bt_adapter_is_remote_tws_device(const bt_bdaddr_t *remote)
{
    return false;
}

uint8_t bt_adapter_count_connected_source_device(void)
{
    uint8_t count_device = 0;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();

    for (int i = 0; i < BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
    {
        curr_device = g_bt_adapter_manager.bt_source_device + i;
        if (curr_device->acl_is_connected)
        {
            count_device += 1;
        }
    }

    bt_adapter_mutex_unlock();

    return count_device;
}

uint8_t bt_adapter_create_new_device_id(const bt_bdaddr_t *remote, bool local_as_source)
{
    uint8_t device_id = BT_DEVICE_INVALID_ID;
    uint8_t count_device = 0;
    int i = 0;

    bt_adapter_mutex_lock();

    if (local_as_source)
    {
        count_device = bt_adapter_count_connected_source_device();

        if (count_device >= BT_ADAPTER_MAX_DEVICE_NUM)
        {
            device_id = BT_DEVICE_INVALID_ID;
            goto unlock_return;
        }

        for (i = BT_SOURCE_DEVICE_ID_BASE; i < BT_SOURCE_DEVICE_ID_BASE + BT_ADAPTER_MAX_DEVICE_NUM; i += 1)
        {
            if (!bt_adapter_get_device(i)->acl_is_connected)
            {
                device_id = i;
                goto unlock_return;
            }
        }

        device_id = count_device;
    }
    else
    {
        if (bt_adapter_is_remote_tws_device(remote))
        {
            device_id = BT_DEVICE_TWS_ID;
            goto unlock_return;
        }

        count_device = bt_adapter_count_mobile_link();

        if (count_device >= BT_ADAPTER_MAX_DEVICE_NUM)
        {
            device_id = BT_DEVICE_INVALID_ID;
            goto unlock_return;
        }

        for (i = 0; i < count_device; i += 1)
        {
            if (!bt_adapter_get_device(i)->acl_is_connected)
            {
                device_id = i;
                goto unlock_return;
            }
        }

        device_id = count_device;
    }

unlock_return:
    bt_adapter_mutex_unlock();
    return device_id;
}

uint8_t bt_adapter_reset_device_id(struct BT_ADAPTER_DEVICE_T *curr_device, bool reset_to_source)
{
    uint8_t new_device_id = BT_DEVICE_INVALID_ID;
    struct BT_ADAPTER_DEVICE_T temp_device;
    struct BT_ADAPTER_DEVICE_T *new_device = NULL;

    bt_adapter_mutex_lock();

    if (!curr_device->acl_is_connected)
    {
        new_device_id = curr_device->device_id;
    }
    else if (curr_device->device_id == BT_DEVICE_TWS_ID)
    {
        new_device_id = BT_DEVICE_TWS_ID;
    }
    else if (curr_device->device_id < BT_ADAPTER_MAX_DEVICE_NUM)
    {
        if (reset_to_source)
        {
            new_device_id = bt_adapter_create_new_device_id(&curr_device->remote, true);
            if (new_device_id != BT_DEVICE_INVALID_ID)
            {
                temp_device = *curr_device;
                bt_adapter_clear_device(curr_device);
                new_device = bt_adapter_get_device(new_device_id);
                *new_device = temp_device;
                new_device->device_id = new_device_id; // keep device id untouched
            }
            else
            {
                new_device_id = curr_device->device_id;
                TRACE(0, "bt_adapter_reset_device_id: reset to source failed %x", new_device_id);
            }
        }
        else
        {
            new_device_id = curr_device->device_id;
            TRACE(0, "bt_adapter_reset_device_id: local already sink device %x", new_device_id);
        }
    }
    else if (curr_device->device_id >= BT_SOURCE_DEVICE_ID_BASE && curr_device->device_id < BT_SOURCE_DEVICE_ID_BASE + BT_ADAPTER_MAX_DEVICE_NUM)
    {
        if (reset_to_source)
        {
            new_device_id = curr_device->device_id;
            TRACE(0, "bt_adapter_reset_device_id: local already source device %x", new_device_id);
        }
        else
        {
            new_device_id = bt_adapter_create_new_device_id(&curr_device->remote, false);
            if (new_device_id != BT_DEVICE_INVALID_ID)
            {
                temp_device = *curr_device;
                bt_adapter_clear_device(curr_device);
                new_device = bt_adapter_get_device(new_device_id);
                *new_device = temp_device;
                new_device->device_id = new_device_id; // keep device id untouched
            }
            else
            {
                new_device_id = curr_device->device_id;
                TRACE(0, "bt_adapter_reset_device_id: reset to sink failed %x", new_device_id);
            }
        }
    }
    else
    {
        new_device_id = curr_device->device_id;
        TRACE(0, "bt_adapter_reset_device_id: invalid device %x", new_device_id);
    }

    bt_adapter_mutex_unlock();

    return new_device_id;
}

void bt_adapter_report_acl_connected(const bt_bdaddr_t *bd_addr, const bt_adapter_acl_opened_param_t *acl_con)
{
    uint8_t gen_device_id = BT_DEVICE_INVALID_ID;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;

    bt_adapter_mutex_lock();

    if (acl_con->error_code == BT_STS_ACL_ALREADY_EXISTS)
    {
        goto unlock_return;
    }

    if (acl_con->error_code == BT_STS_SUCCESS)
    {
        if (acl_con->device_id != BT_DEVICE_INVALID_ID)
        {
            gen_device_id = acl_con->device_id;
        }
        else
        {
            gen_device_id = bt_adapter_create_new_device_id(bd_addr, acl_con->local_is_source);
        }

        if (gen_device_id == BT_DEVICE_INVALID_ID)
        {
            TRACE(0, "bt_adapter_report_acl_connected: gen new device id failed %x %d %02x:%02x:*:*:*:%02x",
                acl_con->device_id, acl_con->local_is_source, bd_addr->address[0],
                bd_addr->address[1], bd_addr->address[5]);
            goto unlock_return;
        }

        curr_device = bt_adapter_get_device(gen_device_id);
        bt_adapter_clear_device(curr_device);
        curr_device->remote = *bd_addr;
        curr_device->acl_is_connected = true;
        curr_device->acl_link_mode = 0;
        curr_device->acl_bt_role = acl_con->acl_bt_role;
        curr_device->acl_conn_hdl = acl_con->conn_handle;

        BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x connhdl %04x role %d d%x source %d",
            curr_device->device_id, __func__,
            bd_addr->address[0], bd_addr->address[1], bd_addr->address[5],
            acl_con->conn_handle, acl_con->acl_bt_role, acl_con->device_id, acl_con->local_is_source);
    }
    else
    {
        curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);

        BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x connhdl %04x error %02x",
            curr_device ? curr_device->device_id : 0xd, __func__,
            bd_addr->address[0], bd_addr->address[1], bd_addr->address[5],
            acl_con->conn_handle, acl_con->error_code);

        if (acl_con->error_code != BT_STS_BT_CANCEL_PAGE && curr_device)
        {
            bt_adapter_clear_device(curr_device);
        }
    }

unlock_return:
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_acl_disconnected(const bt_bdaddr_t *bd_addr, const bt_adapter_acl_closed_param_t *acl_dis)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x error %02x reason %02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5], acl_dis->error_code, acl_dis->disc_reason);
    if (curr_device)
    {
        bt_adapter_clear_device(curr_device);
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_sco_connected(const bt_bdaddr_t *bd_addr, const bt_adapter_sco_opened_param_t *sco_con)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x scohdl %04x codec %d", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5], sco_con->sco_handle, sco_con->codec);
    if (curr_device && sco_con->error_code == BT_STS_SUCCESS)
    {
        curr_device->sco_is_connected = true;
        curr_device->sco_handle = sco_con->sco_handle;
        curr_device->sco_codec_type = sco_con->codec;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_sco_disconnected(const bt_bdaddr_t *bd_addr, const bt_adapter_sco_closed_param_t *sco_dis)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x error %02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5], sco_dis->error_code);
    if (curr_device)
    {
        curr_device->sco_is_connected = false;
        curr_device->sco_handle = BT_INVALID_CONN_HANDLE;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_access_change(const bt_bdaddr_t *bd_addr, const bt_adapter_access_change_param_t *access_change)
{
    BT_ADAPTER_TRACE(0, " %s %d", __func__, access_change->access_mode);
    g_bt_adapter_manager.access_mode = access_change->access_mode;
}

void bt_adapter_report_role_discover(const bt_bdaddr_t *bd_addr, const bt_adapter_role_discover_param_t *role_discover)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x role %d error %02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5],
        role_discover->acl_bt_role, role_discover->error_code);
    if (curr_device && role_discover->error_code == BT_STS_SUCCESS)
    {
        curr_device->acl_bt_role = role_discover->acl_bt_role;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_role_change(const bt_bdaddr_t *bd_addr, const bt_adapter_role_change_param_t *role_change)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x role %d error %02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5],
        role_change->acl_bt_role, role_change->error_code);
    if (curr_device && role_change->error_code == BT_STS_SUCCESS)
    {
        curr_device->acl_bt_role = role_change->acl_bt_role;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_mode_change(const bt_bdaddr_t *bd_addr, const bt_adapter_mode_change_param_t *mode_change)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x M %d T %d errno %02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5],
        mode_change->acl_link_mode, mode_change->sniff_interval, mode_change->error_code);
    if (curr_device && mode_change->error_code == BT_STS_SUCCESS)
    {
        curr_device->acl_link_mode = mode_change->acl_link_mode;
        if (curr_device->acl_link_mode == BT_MODE_SNIFF_MODE)
        {
            curr_device->sniff_interval = mode_change->sniff_interval;
        }
        else
        {
            curr_device->sniff_interval = 0;
        }
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_authenticated(const bt_bdaddr_t *bd_addr, const bt_adapter_authenticated_param_t *auth)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    if (curr_device)
    {
        BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x error %02x", curr_device->device_id, __func__,
            bd_addr->address[0], bd_addr->address[1], bd_addr->address[5], auth->error_code);
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_enc_change(const bt_bdaddr_t *bd_addr, const bt_adapter_enc_change_param_t *enc_change)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    if (curr_device)
    {
        BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x error %02x", curr_device->device_id, __func__,
            bd_addr->address[0], bd_addr->address[1], bd_addr->address[5],
            enc_change->error_code);
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_inquiry_result(const bt_bdaddr_t *bd_addr, const bt_adapter_inquiry_result_param_t *inq_result)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(&inq_result->remote);
    if (curr_device)
    {
        BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device->device_id, __func__,
            inq_result->remote.address[0], inq_result->remote.address[1], inq_result->remote.address[5]);
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_inquiry_complete(const bt_bdaddr_t *bd_addr, const bt_adapter_inquiry_complete_param_t *inq_complete)
{

}

void bt_adapter_reconfig_hci_trace(uint8_t cfg_type, bool enable)
{
    bt_export_reconfig_hci_trace(cfg_type, enable);
}

void bt_adapter_echo_register(void(*echo_req)(uint8_t device_id, uint16_t conhdl, uint8_t id, uint16_t len, uint8_t *data),
                           void(*echo_rsp)(uint8_t device_id, uint16_t conhdl, uint8_t *rxdata, uint16_t rxle))
{
    btif_custom_l2cap_echo_init(echo_req, echo_rsp);
}

void bt_adapter_echo_req_send(uint8_t device_id, void *conn, uint8_t *data, uint16_t data_len)
{
    btif_l2cap_fill_in_echo_req_data(device_id, conn, data, data_len);
}

void bt_adapter_echo_rsp_send(uint8_t device_id, uint16 conn_handle, uint8 sigid, uint16 len, const uint8* data)
{
    btif_l2cap_fill_in_enco_rsp_data(device_id, conn_handle, sigid, len, data);
}

#ifdef BT_HFP_SUPPORT

void bt_adapter_report_hfp_connected(const bt_bdaddr_t *bd_addr, const bt_hf_opened_param_t *hfp_conn)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5]);
    if (curr_device)
    {
        curr_device->hfp_ctx.hfp_is_connected = true;
        curr_device->hfp_ctx.hfp_call_state = hfp_conn->call_status;
        curr_device->hfp_ctx.hfp_callsetup_state = hfp_conn->callsetup_status;
        curr_device->hfp_ctx.hfp_callhold_state = hfp_conn->callhold_status;
        TRACE(2, "%s curr_device: %p", __func__, curr_device);
        TRACE(3, "call: %x, callSetup:%x, callheld:%x",
            curr_device->hfp_ctx.hfp_call_state, curr_device->hfp_ctx.hfp_callsetup_state, curr_device->hfp_ctx.hfp_callhold_state);
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_hfp_disconnected(const bt_bdaddr_t *bd_addr, const bt_hf_closed_param_t *hfp_disc)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5]);
    if (curr_device)
    {
        curr_device->hfp_ctx.hfp_is_connected = false;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_hfp_ring(const bt_bdaddr_t *bd_addr, const void *param)
{

}

void bt_adapter_report_hfp_clip_ind(const bt_bdaddr_t *bd_addr, const bt_hf_clip_ind_param_t *hfp_caller_ind)
{

}

void bt_adapter_report_hfp_call_state(const bt_bdaddr_t *bd_addr, const bt_hf_call_ind_param_t *hfp_call)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x call %d", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5], hfp_call->call);
    if (curr_device)
    {
        curr_device->hfp_ctx.hfp_call_state = hfp_call->call;
        if(curr_device->hfp_ctx.hfp_call_state == BT_HF_CALL_NO_CALLS_IN_PROGRESS)
        {
            /*when pre_state is callheld=2,hungup this call,ios device only report call = 0;
                ios device not send callheld none to HF*/
            curr_device->hfp_ctx.hfp_callhold_state = BT_HF_CALLHELD_NONE;
        }
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_hfp_callsetup_state(const bt_bdaddr_t *bd_addr, const bt_hf_callsetup_ind_param_t *hfp_callsetup)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x callsetup %d", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5], hfp_callsetup->callsetup);
    if (curr_device)
    {
        curr_device->hfp_ctx.hfp_callsetup_state = hfp_callsetup->callsetup;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_hfp_callhold_state(const bt_bdaddr_t *bd_addr, const bt_hf_callheld_ind_param_t *hfp_callhold)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x callhold %d", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5], hfp_callhold->callheld);
    if (curr_device)
    {
        curr_device->hfp_ctx.hfp_callhold_state = hfp_callhold->callheld;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_hfp_volume_change(const bt_bdaddr_t *bd_addr, const bt_hf_volume_change_param_t *volume_change)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x type %d vol %d", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5], volume_change->type, volume_change->volume);
    if (curr_device && volume_change->type == BT_HF_VOLUME_TYPE_SPK)
    {
        curr_device->hfp_ctx.hfp_speak_vol = volume_change->volume;
    }
    bt_adapter_mutex_unlock();
}

#endif /* BT_HFP_SUPPORT */

#ifdef BT_A2DP_SUPPORT

void bt_adapter_report_a2dp_connected(const bt_bdaddr_t *bd_addr, const bt_a2dp_opened_param_t *a2dp_conn)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x codec %d len %d", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5],
        a2dp_conn->codec_type, a2dp_conn->codec_info_len);
    if (curr_device)
    {
        curr_device->a2dp_ctx.a2dp_is_connected = true;
        curr_device->a2dp_ctx.a2dp_codec_type = a2dp_conn->codec_type;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_a2dp_disconnected(const bt_bdaddr_t *bd_addr, const bt_a2dp_closed_param_t *a2dp_disc)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5]);
    if (curr_device)
    {
        curr_device->a2dp_ctx.a2dp_is_connected = false;
        curr_device->a2dp_ctx.a2dp_is_streaming = false;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_a2dp_stream_start(const bt_bdaddr_t *bd_addr, const bt_a2dp_stream_start_param_t *a2dp_stream_start)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x codec %d", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5],
        curr_device ? curr_device->a2dp_ctx.a2dp_codec_type : 0xdd);
    if (curr_device)
    {
        curr_device->a2dp_ctx.a2dp_is_streaming = true;
        curr_device->a2dp_ctx.rsv_avdtp_start_signal = true;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_a2dp_stream_reconfig(const bt_bdaddr_t *bd_addr, const bt_a2dp_stream_reconfig_param_t *a2dp_stream_reconfig)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x codec %d len %d", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5],
        a2dp_stream_reconfig->codec_type, a2dp_stream_reconfig->codec_info_len);
    if (curr_device)
    {
        curr_device->a2dp_ctx.a2dp_codec_type = a2dp_stream_reconfig->codec_type;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_a2dp_stream_suspend(const bt_bdaddr_t *bd_addr, const bt_a2dp_stream_suspend_param_t *a2dp_stream_suspend)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5]);
    if (curr_device)
    {
        curr_device->a2dp_ctx.a2dp_is_streaming = false;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_a2dp_stream_close(const bt_bdaddr_t *bd_addr, const bt_a2dp_stream_close_param_t *a2dp_stream_close)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5]);
    if (curr_device)
    {
        curr_device->a2dp_ctx.a2dp_is_streaming = false;
    }
    bt_adapter_mutex_unlock();
}

#endif /* BT_A2DP_SUPPORT */

#ifdef BT_AVRCP_SUPPORT
void bt_adapter_report_avrcp_browsing_connected(const bt_bdaddr_t *bd_addr, const bt_avrcp_opened_t *avrcp_conn)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5]);
}

void bt_adapter_report_avrcp_browsing_disconnected(const bt_bdaddr_t *bd_addr, const bt_avrcp_closed_t *avrcp_disc)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5]);
}

void bt_adapter_report_avrcp_browsing_rsp(const bt_bdaddr_t *bd_addr, const bt_avrcp_browsing_rsp_t *browsing_rsp)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5]);
    DUMP8("%02x ", browsing_rsp->data, (browsing_rsp->data_len> 20) ? 20 : browsing_rsp->data_len);
}

void bt_adapter_report_avrcp_obex_connected(const bt_bdaddr_t *bd_addr, const bt_avrcp_opened_t *avrcp_conn)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5]);
}

void bt_adapter_report_avrcp_connected(const bt_bdaddr_t *bd_addr, const bt_avrcp_opened_t *avrcp_conn)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5]);
    if (curr_device)
    {
        curr_device->avrcp_ctx.avrcp_is_connected = true;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_avrcp_disconnected(const bt_bdaddr_t *bd_addr, const bt_avrcp_closed_t *avrcp_disc)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s %02x:%02x:***:%02x", curr_device ? curr_device->device_id : 0xd, __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[5]);
    if (curr_device)
    {
        curr_device->avrcp_ctx.avrcp_is_connected = false;
    }
    bt_adapter_mutex_unlock();
}

void bt_adapter_report_avrcp_status_changed(const bt_bdaddr_t *bd_addr, const bt_avrcp_play_status_change_t *avrcp_changed)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    bt_adapter_mutex_lock();
    curr_device = bt_adapter_get_connected_device_byaddr(bd_addr);
    BT_ADAPTER_TRACE(0, "(d%x) %s state:%d", curr_device ? curr_device->device_id : 0xd, __func__,
        avrcp_changed->play_status);
    if (curr_device)
    {
        curr_device->avrcp_ctx.avrcp_playback_status = avrcp_changed->play_status;
    }
    bt_adapter_mutex_unlock();
}
#endif /* BT_AVRCP_SUPPORT */


static int bt_adapter_event_callback(const bt_bdaddr_t *bd_addr, BT_EVENT_T event, BT_CALLBACK_PARAM_T param)
{
    switch (event)
    {
        case BT_EVENT_ACL_OPENED:
            bt_adapter_report_acl_connected(bd_addr, param.bt.acl_opened);
            break;
        case BT_EVENT_ACL_CLOSED:
            bt_adapter_report_acl_disconnected(bd_addr, param.bt.acl_closed);
            break;
        case BT_EVENT_SCO_OPENED:
            bt_adapter_report_sco_connected(bd_addr, param.bt.sco_opened);
            break;
        case BT_EVENT_SCO_CLOSED:
            bt_adapter_report_sco_disconnected(bd_addr, param.bt.sco_closed);
            break;
        case BT_EVENT_ACCESS_CHANGE:
            bt_adapter_report_access_change(bd_addr, param.bt.access_change);
            break;
        case BT_EVENT_ROLE_DISCOVER:
            bt_adapter_report_role_discover(bd_addr, param.bt.role_discover);
            break;
        case BT_EVENT_ROLE_CHANGE:
            bt_adapter_report_role_change(bd_addr, param.bt.role_change);
            break;
        case BT_EVENT_MODE_CHANGE:
            bt_adapter_report_mode_change(bd_addr, param.bt.mode_change);
            break;
        case BT_EVENT_AUTHENTICATED:
            bt_adapter_report_authenticated(bd_addr, param.bt.authenticated);
            break;
        case BT_EVENT_ENC_CHANGE:
            bt_adapter_report_enc_change(bd_addr, param.bt.enc_change);
            break;
        case BT_EVENT_INQUIRY_RESULT:
            bt_adapter_report_inquiry_result(bd_addr, param.bt.inq_result);
            break;
        case BT_EVENT_INQUIRY_COMPLETE:
            bt_adapter_report_inquiry_complete(bd_addr, param.bt.inq_complete);
            break;
#ifdef BT_HFP_SUPPORT
        case BT_EVENT_HF_OPENED:
            bt_adapter_report_hfp_connected(bd_addr, param.hf.opened);
            break;
        case BT_EVENT_HF_CLOSED:
            bt_adapter_report_hfp_disconnected(bd_addr, param.hf.closed);
            break;
        case BT_EVENT_HF_RING_IND:
            bt_adapter_report_hfp_ring(bd_addr, NULL);
            break;
        case BT_EVENT_HF_CLIP_IND:
            bt_adapter_report_hfp_clip_ind(bd_addr, param.hf.clip_ind);
            break;
        case BT_EVENT_HF_CALL_IND:
            bt_adapter_report_hfp_call_state(bd_addr,param.hf.call_ind);
            break;
        case BT_EVENT_HF_CALLSETUP_IND:
            bt_adapter_report_hfp_callsetup_state(bd_addr, param.hf.callsetup_ind);
            break;
        case BT_EVENT_HF_CALLHELD_IND:
            bt_adapter_report_hfp_callhold_state(bd_addr, param.hf.callheld_ind);
            break;
        case BT_EVENT_HF_VOLUME_CHANGE:
            bt_adapter_report_hfp_volume_change(bd_addr, param.hf.volume_change);
            break;
#endif /* BT_HFP_SUPPORT */
#ifdef BT_A2DP_SUPPORT
        case BT_EVENT_A2DP_OPENED:
            bt_adapter_report_a2dp_connected(bd_addr, param.av.opened);
            break;
        case BT_EVENT_A2DP_CLOSED:
            bt_adapter_report_a2dp_disconnected(bd_addr, param.av.closed);
            break;
        case BT_EVENT_A2DP_STREAM_START:
            bt_adapter_report_a2dp_stream_start(bd_addr, param.av.stream_start);
            break;
        case BT_EVENT_A2DP_STREAM_RECONFIG:
            bt_adapter_report_a2dp_stream_reconfig(bd_addr, param.av.stream_reconfig);
            break;
        case BT_EVENT_A2DP_STREAM_SUSPEND:
            bt_adapter_report_a2dp_stream_suspend(bd_addr, param.av.stream_suspend);
            break;
        case BT_EVENT_A2DP_STREAM_CLOSE:
            bt_adapter_report_a2dp_stream_close(bd_addr, param.av.stream_close);
            break;
#endif /* BT_A2DP_SUPPORT */
#ifdef BT_AVRCP_SUPPORT
        case BT_EVENT_AVRCP_OPENED:
            bt_adapter_report_avrcp_connected(bd_addr, param.ar.opened);
            break;
        case BT_EVENT_AVRCP_CLOSED:
            bt_adapter_report_avrcp_disconnected(bd_addr, param.ar.closed);
            break;
        case BT_EVENT_AVRCP_PLAY_STATUS_CHANGE:
            bt_adapter_report_avrcp_status_changed(bd_addr,param.ar.play_status_change);
            break;
        case BT_EVENT_AVRCP_BROSING_OPENED:
            bt_adapter_report_avrcp_browsing_connected(bd_addr, param.ar.opened);
            break;
        case BT_EVENT_AVRCP_BROSING_CLOSED:
            bt_adapter_report_avrcp_browsing_disconnected(bd_addr, param.ar.closed);
            break;
        case BT_EVENT_AVRCP_BROSING_RSP_INFO:
            bt_adapter_report_avrcp_browsing_rsp(bd_addr, param.ar.browsing_rsp);
            break;
        case BT_EVENT_AVRCP_OBEX_EVENT_INFO:
            bt_adapter_report_avrcp_obex_connected(bd_addr, param.ar.opened);
            break;
#endif /* BT_AVRCP_SUPPORT */
        default:
            break;
    }

    return 0;
}

void bt_adapter_local_volume_up(void)
{
    app_audio_manager_ctrl_volume(APP_AUDIO_MANAGER_VOLUME_CTRL_UP, 0);
}

void bt_adapter_local_volume_down(void)
{
    app_audio_manager_ctrl_volume(APP_AUDIO_MANAGER_VOLUME_CTRL_DOWN, 0);
}

void bt_adapter_local_volume_up_with_callback(void (*cb)(uint8_t device_id))
{
     app_audio_manager_ctrl_volume_with_callback(APP_AUDIO_MANAGER_VOLUME_CTRL_UP, 0, cb);
}

void bt_adapter_local_volume_down_with_callback(void (*cb)(uint8_t device_id))
{
    app_audio_manager_ctrl_volume_with_callback(APP_AUDIO_MANAGER_VOLUME_CTRL_DOWN, 0, cb);
}

U32 be_to_host32(const U8* ptr)
{
    return (U32)( ((U32) *ptr << 24)   | \
                  ((U32) *(ptr+1) << 16) | \
                  ((U32) *(ptr+2) << 8)  | \
                  ((U32) *(ptr+3)) );
}

const char *DebugMask2Prefix(uint8_t mask)
{
    const char *prefix = "";

    switch (mask)
    {
        case GAP_ERROR:
            prefix = "[ERROR][BLE GAPM] ";
            break;
        case GAP_OUT:
            prefix = "[BLE GAP] ";
            break;
        case HCI_ERROR:
            prefix = "[ERROR][BLE HCI] ";
            break;
        case HCI_OUT:
            prefix = "[BLE HCI] ";
            break;
        case GATT_ERROR:
            prefix = "[ERROR][BLE GATT] ";
            break;
        case GATT_OUT:
            prefix = "[BLE GATT] ";
            break;
        case ATT_ERROR:
            prefix = "[ERROR][BLE ATT] ";
            break;
        case ATT_OUT:
            prefix = "[BLE ATT] ";
            break;
        case L2C_ERROR:
            prefix = "[ERROR][BLE L2C] ";
            break;
        case L2C_OUT:
            prefix = "[BLE L2C] ";
            break;
        case SMP_ERROR:
            prefix = "[ERROR][BLE SMP] ";
            break;
        case SMP_OUT:
            prefix = "[BLE SMP] ";
            break;
        case APP_ERROR:
            prefix = "[ERROR][BLE APP] ";
            break;
        case APP_OUT:
            prefix = "[BLE APP] ";
            break;
        case PRF_HT_ERROR:
            prefix = "[ERROR][BLE HP] ";
            break;
        case PRF_HT_OUT:
            prefix = "[BLE HP] ";
            break;
        case BLE_ERROR:
            prefix = "[ERROR][BLE] ";
            break;
        case BLE_OUT:
            prefix = "[BLE] ";
            break;
        default:
            prefix = "";
            break;
    }

    return prefix;
}

static const char * const aud_id_str[] =
{
    "[POWER_ON]",
    "[POWER_OFF]",
    "[LANGUAGE_SWITCH]",

    "[NUM_0]",
    "[NUM_1]",
    "[NUM_2]",
    "[NUM_3]",
    "[NUM_4]",
    "[NUM_5]",
    "[NUM_6]",
    "[NUM_7]",
    "[NUM_8]",
    "[NUM_9]",

    "[BT_PAIR_ENABLE]",
    "[BT_PAIRING]",
    "[BT_PAIRING_SUC]",
    "[BT_PAIRING_FAIL]",
    "[BT_CALL_REFUSE]",
    "[BT_CALL_OVER]",
    "[BT_CALL_ANSWER]",
    "[BT_CALL_HUNG_UP]",
    "[BT_CALL_INCOMING_CALL]",
    "[BT_CALL_INCOMING_NUMBER]",
    "[BT_CHARGE_PLEASE]",
    "[BT_CHARGE_FINISH]",
    "[BT_CLEAR_SUCCESS]",
    "[BT_CLEAR_FAIL]",
    "[BT_CONNECTED]",
    "[BT_DIS_CONNECT]",
    "[BT_WARNING]",
    "[BT_ALEXA_START]",
    "[FIND_MY_BUDS]",
    "[TILE FIND]",
    "[BT_ALEXA_STOP]",
    "[BT_GSOUND_MIC_OPEN]",
    "[BT_GSOUND_MIC_CLOSE]",
    "[BT_GSOUND_NC]",
    "[BT_MUTE]",
    "[RING_WARNING]",
#ifdef __INTERACTION__
    "[BT_FINDME]",
#else
    "[UNKNOWN]",
#endif
    "[ANC_PROMPT]",
    "[ANC_MUTE]",
    "[ANC_MODE0]",
    "[ANC_MODE1]",
#if BLE_AUDIO_ENABLED
    "[LE_AUD_INCOMING_CALL]",
#else
    "[UNKNOWN]",
#endif
};

const char *aud_id2str(uint16_t aud_id)
{
    const char *str = NULL;

    if (aud_id < ARRAY_SIZE(aud_id_str))
    {
        str = aud_id_str[aud_id];
    }
    else
    {
        str = "[UNKNOWN]";
    }

    return str;
}

/**
 * Register bt stack callback functions
 *
 */

static struct BT_CALLBACK_FUNC_T fn = {0};

static struct bt_hf_custom_id_t g_hf_custom_id;

static struct bt_hf_custom_id_t *app_bt_get_hf_custom_id(void)
{
    return &g_hf_custom_id;
}

#ifndef BT_REDUCE_CALLBACK_FUNC
static bool besbt_cfg_lhdc_v3(void)
{
    return besbt_cfg.lhdc_v3;
}

static bool besbt_cfg_a2dp_sink_enable(void)
{
    return besbt_cfg.a2dp_sink_enable;
}

static bool besbt_cfg_bt_source_enable(void)
{
    return besbt_cfg.bt_source_enable;
}

static bool besbt_cfg_bt_sink_enable(void)
{
    return besbt_cfg.bt_sink_enable;
}

static bool besbt_cfg_avdtp_cp_enable(void)
{
    return besbt_cfg.avdtp_cp_enable;
}

static bool besbt_cfg_bt_source_48k(void)
{
    return besbt_cfg.bt_source_48k;
}

static bool besbt_cfg_source_unknown_cmd_flag(void)
{
    return besbt_cfg.source_unknown_cmd_flag;
}

static bool besbt_cfg_source_suspend_err_flag(void)
{
    return besbt_cfg.source_suspend_err_flag;
}

static bool besbt_cfg_source_get_all_cap_flag(void)
{
    return besbt_cfg.source_get_all_cap_flag;
}

static bool besbt_cfg_mark_some_code_for_fuzz_test(void)
{
    return besbt_cfg.mark_some_code_for_fuzz_test;
}

static bool besbt_cfg_use_page_scan_repetition_mode_r1(void)
{
    return besbt_cfg.use_page_scan_repetition_mode_r1;
}

static bool besbt_cfg_bt_hid_cod_enable(void)
{
    return besbt_cfg.bt_hid_cod_enable;
}

static bool besbt_cfg_le_audio_enabled(void)
{
    return besbt_cfg.le_audio_enabled;
}

static bool besbt_cfg_hsp_enable(void)
{
    return besbt_cfg.hsp_enable;
}

static bool besbt_cfg_disc_acl_after_auth_key_missing(void)
{
    return besbt_cfg.disc_acl_after_auth_key_missing;
}

static bool besbt_cfg_hfp_support_lc3_swb_en(void)
{
    return besbt_cfg.hfp_support_lc3_swb_en;
}

static bool besbt_cfg_vendor_codec_en(void)
{
    return besbt_cfg.vendor_codec_en;
}

static bool besbt_cfg_normal_test_mode_switch(void)
{
    return besbt_cfg.normal_test_mode_switch;
}

static bool besbt_cfg_hfp_hf_pts_acs_bv_09_i(void)
{
    return besbt_cfg.hfp_hf_pts_acs_bv_09_i;
}

static bool besbt_cfg_sniff(void)
{
    return besbt_cfg.sniff;
}

static uint16_t besbt_cfg_dip_vendor_id(void)
{
    return besbt_cfg.dip_vendor_id;
}

static uint16_t besbt_cfg_dip_product_id(void)
{
    return besbt_cfg.dip_product_id;
}

static uint16_t besbt_cfg_dip_product_version(void)
{
    return besbt_cfg.dip_product_version;
}

static uint16_t besbt_cfg_dip_vendor_id_source(void)
{
    return besbt_cfg.dip_vendor_id_source;
}

static bool besbt_cfg_apple_hf_at_support(void)
{
    return besbt_cfg.apple_hf_at_support;
}

static bool besbt_cfg_hf_support_hf_ind_feature(void)
{
    return besbt_cfg.hf_support_hf_ind_feature;
}

static bool besbt_cfg_hf_dont_support_cli_feature(void)
{
    return besbt_cfg.hf_dont_support_cli_feature;
}

static bool besbt_cfg_hf_dont_support_enhanced_call(void)
{
    return besbt_cfg.hf_dont_support_enhanced_call;
}

static bool besbt_cfg_hf_dont_support_3way_call(void)
{
    return besbt_cfg.hf_dont_support_3way_call;
}

static bool besbt_cfg_force_use_cvsd(void)
{
    return besbt_cfg.force_use_cvsd;
}

static bool besbt_cfg_hfp_ag_pts_ecs_02(void)
{
    return besbt_cfg.hfp_ag_pts_ecs_02;
}

static bool besbt_cfg_hfp_ag_pts_ecc(void)
{
    return besbt_cfg.hfp_ag_pts_ecc;
}

static bool besbt_cfg_hfp_ag_pts_ecs_01(void)
{
    return besbt_cfg.hfp_ag_pts_ecs_01;
}

static bool besbt_cfg_hfp_ag_pts_enable(void)
{
    return besbt_cfg.hfp_ag_pts_enable;
}

static bool besbt_cfg_send_l2cap_echo_req(void)
{
    return besbt_cfg.send_l2cap_echo_req;
}

static bool besbt_cfg_support_enre_mode(void)
{
    return besbt_cfg.support_enre_mode;
}
#endif /* BT_REDUCE_CALLBACK_FUNC */

bt_iocap_requirement_t *bt_adapter_iocap_request_callback(uint8_t device_id, uint16_t connhdl,
        const bt_bdaddr_t *remote, const bt_iocap_requirement_t *remote_initiate)
{
    // bt_iocap_requirement_t *iocap = &g_bt_adapter_manager.iocap_requirement;
    return NULL;
}

void bt_adapter_callback_func_init(void)
{
    g_hf_custom_id.hf_custom_vendor_id = 0x0000;
    g_hf_custom_id.hf_custom_product_id = 0x0000;
    g_hf_custom_id.hf_custom_version_id = 0x0100;
    g_hf_custom_id.hf_custom_feature_id = 0x03;
#ifndef BT_REDUCE_CALLBACK_FUNC
    fn.lhdc_v3 = besbt_cfg_lhdc_v3;
    fn.a2dp_sink_enable = besbt_cfg_a2dp_sink_enable;
    fn.bt_source_enable = besbt_cfg_bt_source_enable;
    fn.bt_sink_enable = besbt_cfg_bt_sink_enable;
    fn.avdtp_cp_enable = besbt_cfg_avdtp_cp_enable;
    fn.bt_source_48k = besbt_cfg_bt_source_48k;
    fn.source_unknown_cmd_flag = besbt_cfg_source_unknown_cmd_flag;
    fn.source_suspend_err_flag = besbt_cfg_source_suspend_err_flag;
    fn.source_get_all_cap_flag = besbt_cfg_source_get_all_cap_flag;
    fn.mark_some_code_for_fuzz_test = besbt_cfg_mark_some_code_for_fuzz_test;
    fn.use_page_scan_repetition_mode_r1 = besbt_cfg_use_page_scan_repetition_mode_r1;
    fn.bt_hid_cod_enable = besbt_cfg_bt_hid_cod_enable;
    fn.disc_acl_after_auth_key_missing = besbt_cfg_disc_acl_after_auth_key_missing;
    fn.hfp_support_lc3_swb_en = besbt_cfg_hfp_support_lc3_swb_en;
    fn.vendor_codec_en = besbt_cfg_vendor_codec_en;
    fn.normal_test_mode_switch = besbt_cfg_normal_test_mode_switch;
    fn.hfp_hf_pts_acs_bv_09_i = besbt_cfg_hfp_hf_pts_acs_bv_09_i;
    fn.sniff = besbt_cfg_sniff;
    fn.dip_vendor_id = besbt_cfg_dip_vendor_id;
    fn.dip_product_id = besbt_cfg_dip_product_id;
    fn.dip_product_version = besbt_cfg_dip_product_version;
    fn.dip_vendor_id_source = besbt_cfg_dip_vendor_id_source;
    fn.apple_hf_at_support = besbt_cfg_apple_hf_at_support;
    fn.hf_support_hf_ind_feature = besbt_cfg_hf_support_hf_ind_feature;
    fn.hf_dont_support_cli_feature = besbt_cfg_hf_dont_support_cli_feature;
    fn.hf_dont_support_enhanced_call = besbt_cfg_hf_dont_support_enhanced_call;
    fn.hf_dont_support_3way_call = besbt_cfg_hf_dont_support_3way_call;
    fn.force_use_cvsd = besbt_cfg_force_use_cvsd;
    fn.hfp_ag_pts_ecs_02 = besbt_cfg_hfp_ag_pts_ecs_02;
    fn.hfp_ag_pts_ecc = besbt_cfg_hfp_ag_pts_ecc;
    fn.hfp_ag_pts_ecs_01 = besbt_cfg_hfp_ag_pts_ecs_01;
    fn.hfp_ag_pts_enable = besbt_cfg_hfp_ag_pts_enable;
    fn.send_l2cap_echo_req = besbt_cfg_send_l2cap_echo_req;
    fn.support_enre_mode = besbt_cfg_support_enre_mode;
    fn.le_audio_enabled = besbt_cfg_le_audio_enabled;
    fn.hsp_enable = besbt_cfg_hsp_enable;
#endif /*BT_REDUCE_CALLBACK_FUNC*/

    fn.bt_get_address = bt_get_local_address;
    fn.bt_get_ble_address = bt_get_ble_local_address;
    fn.bt_get_pts_address = app_bt_get_pts_address;
    fn.bt_set_ble_local_name = bt_set_ble_local_name;
    fn.bt_get_ble_local_name = bt_get_ble_local_name;
#if BLE_AUDIO_ENABLED
    fn.iso_rx_buf_malloc = gaf_stream_heap_malloc;
    fn.iso_rx_buf_free = gaf_stream_heap_free;
#elif BLE_ISO_ENABLED
    fn.iso_rx_buf_malloc = app_iso_rx_malloc;
    fn.iso_rx_buf_free = app_iso_rx_free;
#endif
    fn.call_func_in_bt_thread = app_bt_call_func_in_bt_thread;
    fn.defer_call_in_bt_thread = app_bt_defer_call_in_bt_thread;
    fn.btdrv_reconn = btdrv_reconn;
    fn.load_sleep_config = btdrv_load_sleep_config;
    fn.is_support_multipoint_ibrt = bt_drv_is_support_multipoint_ibrt;
    fn.is_bt_thread = app_bt_is_besbt_thread;
    fn.bt_get_hf_custom_id = app_bt_get_hf_custom_id;
#ifndef BLE_ONLY_ENABLED
    fn.bt_get_class_of_device = app_bt_get_class_of_device_headset;
    fn.read_le_host_chnl_map_cmpl= app_bt_process_cmd_complete_read_le_host_chnl_map;
    fn.bt_get_device = app_bt_get_device;
    fn.get_device_id_byaddr = app_bt_get_device_id_byaddr;
    fn.get_remote_dev_by_address = app_bt_get_remote_dev_by_address;
    fn.get_remote_dev_by_handle = app_bt_get_remote_dev_by_handle;
    fn.iocap_request_callback = bt_adapter_iocap_request_callback;
    fn.gather_uuid_func_lst[0] = sdp_gather_global_service_uuids;
    fn.eir_fill_manufactrue_data_func = NULL;
#if BLE_AUDIO_ENABLED
    fn.set_device_support_le_audio = app_bt_set_device_support_le_audio;
#endif

#ifdef BTH_IN_ROM
    fn.coheap_get_global = bt_rom_export_coheap_global;
#endif

#ifdef BT_SOURCE
    fn.bt_source_find_device = app_bt_source_find_device;
    fn.bt_report_source_link_connected = app_bt_report_source_link_connected;
#endif
#ifdef IBRT
    fn.l2cap_sdp_disconnect_cb = app_tws_ibrt_sdp_disconnect_callback;
    fn.tws_get_ibrt_role = app_tws_get_ibrt_role;
    fn.get_ibrt_role_cb = app_tws_ibrt_role_get_callback;
    fn.ibrt_io_capbility_callback = app_tws_ibrt_key_already_exist;
    fn.ibrt_profile_callback = app_tws_ibrt_profile_callback;
    fn.middleware_get_ui_role = app_ibrt_conn_get_ui_role;
    fn.sniff_timeout_handler_ext_fn = app_tws_ibrt_sniff_timeout_handler;
    fn.conn_remote_is_mobile = app_tws_ibrt_remote_is_mobile;
    fn.ibrt_spp_app_callback = app_tws_ibrt_spp_callback;
    fn.tws_ibrt_disconnect_callback = app_tws_ibrt_disconnect_callback;
    fn.get_ibrt_hci_handle_callback = app_tws_ibrt_get_ibrt_handle_callback;
    fn.avrcp_register_notify_send_check_callback = app_tws_ibrt_avrcp_register_notify_send_callback;
    fn.avrcp_register_notify_resp_check_callback = app_tws_ibrt_avrcp_register_notify_resp_callback;
    if (bt_drv_is_esco_auto_accept_support())
    {
        fn.hci_sync_airmode_check_ind_callback = app_tws_ibrt_sync_airmode_check_ind_handler;
    }
#ifdef BLE_HOST_SUPPORT
    fn.hci_cmd_complete_callback[HCI_CMD_COMPLETE_USER_BLE] = app_ibrt_slave_ble_cmd_complete_callback;
#endif
    fn.hci_cmd_complete_callback[HCI_CMD_COMPLETE_USER_IBRT_CMD] = app_ibrt_conn_cmd_complete_callback;
    fn.get_ui_role_cb = app_ibrt_conn_get_ui_role;
    fn.ibrt_cmd_status_callback = app_ibrt_conn_cmd_status_callback;
    fn.hci_vendor_event_handler = app_hci_vendor_event_handler_v2;
    fn.avdtp_stream_command_accept_pack = app_ibrt_pack_a2dp_command_accept_event;
    fn.hf_sco_codec_info_sync = app_tws_ibrt_sync_sco_codec_info_v2;
    fn.spp_report_close_to_ibrt_slave = app_tws_ibrt_report_spp_close_to_slave;
    fn.me_stack_create_acl_failed_cb = app_ibrt_conn_stack_report_create_acl_failed;
    fn.extra_acl_conn_req_update = app_ui_incoming_extra_conn_req_callback;
    fn.ibrt_rx_data_filter_cb = app_tws_ibrt_rx_data_filter_callback;
#endif
#endif /* BLE_ONLY_ENABLED */

#ifdef IBRT
    hci_register_cmd_filter_handler_callback(app_tws_ibrt_slave_cmd_filter);
#endif

    TRACE(0, "bt_register_callback_func");
    bt_register_callback_func(&fn);
}

uint8_t bes_bt_a2dp_get_curr_sample_bit(void)
{
#ifdef BLE_ONLY_ENABLED
    return 16;
#else
#ifdef BT_A2DP_SUPPORT
    return bt_sbc_player_get_sample_bit();
#else
    return 16;
#endif
#endif
}

// unit: 1/10ms
uint8_t bes_bt_a2dp_get_curr_duration(void)
{
#ifdef BLE_ONLY_ENABLED
    return 10;  // need check
#else
#ifdef BT_A2DP_SUPPORT
    return bt_sbc_player_get_duration();
#else
    return 10;
#endif
#endif
}

bool bes_bt_a2dp_is_first_packet_after_stream_start(int device_id)
{
#ifdef BT_A2DP_SUPPORT
    bool is_first_packet = false;
#ifdef BLE_ONLY_ENABLED
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    curr_device = bt_adapter_get_device(device_id);
    if (curr_device && curr_device->a2dp_ctx.rsv_avdtp_start_signal)
    {
        curr_device->a2dp_ctx.rsv_avdtp_start_signal = false;
        is_first_packet = true;
    }
#else
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device && curr_device->rsv_avdtp_start_signal) {
        curr_device->rsv_avdtp_start_signal = false;
        is_first_packet = true;
    }
#endif
    return is_first_packet;
#else
    return false;
#endif
}

bool bes_bt_avrcp_channel_is_open(int device_id)
{
#ifdef BLE_ONLY_ENABLED
    return false;
#else
    return bt_export_avrcp_channel_is_open(device_id);
#endif
}

void bes_bt_me_acl_disc_all_bt_links(bool power_off_flag)
{
#ifndef BLE_ONLY_ENABLED
    app_disconnect_all_bt_connections(power_off_flag);
#endif
}

void bes_bt_me_transfer_pairing_to_connectable(void)
{
#ifndef BLE_ONLY_ENABLED
    PairingTransferToConnectable();
#endif
}

int8_t bes_bt_me_get_bt_rssi(void)
{
#ifdef BLE_ONLY_ENABLED
    return 0;
#else
    return app_bt_get_rssi();
#endif
}

uint8_t bes_bt_me_count_link(BT_COUNT_LINK_ENUM_T type)
{
#ifdef BLE_ONLY_ENABLED
    return 0;
#else
    return bt_export_count_link(type);
#endif
}

uint8_t bes_bt_me_get_max_sco_number(void)
{
#ifdef BLE_ONLY_ENABLED
    return 0;
#else
    return btif_sco_get_max_number();
#endif
}

bt_acl_state_t bes_bt_me_acl_get_state(int device_id)
{
#ifdef BLE_ONLY_ENABLED
    bt_acl_state_t state = {{{0}},0};
    state.acl_conn_hdl = BT_INVALID_CONN_HANDLE;
    return state;
#else
    return bt_export_acl_get_state_by_rdev(app_bt_get_remote_dev_by_device_id(device_id));
#endif
}

uint16_t app_bt_get_conhandle_by_device_id(uint8_t device_id)
{
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
    return 0xFFFF;
#else
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    return (curr_device && curr_device->acl_is_connected) ? curr_device->acl_conn_hdl : 0xFFFF;
#endif
}

uint8_t bes_bt_me_select_device(BT_SELECT_DEVICE_ENUM_T op)
{
#ifdef BLE_ONLY_ENABLED
    return 0;
#else
    return bt_export_select_device(op);
#endif
}

uint8_t bes_bt_me_select_another_device(BT_SELECT_ANOTHER_DEVICE_ENUM_T op, int curr_device_id)
{
#ifdef BLE_ONLY_ENABLED
    return BT_DEVICE_INVALID_ID;
#else
    return bt_export_select_another_device(op, curr_device_id);
#endif
}

#ifdef BLE_ONLY_ENABLED
uint8_t app_bt_sync_get_available_trigger_channel(uint32_t opcode, uint8_t policy)
{
    return 0xff;
}

void app_bt_sync_release_trigger_channel(uint8_t chnl)
{

}

void app_bt_sync_register_report_info_callback(APP_BT_SYNC_INFO_REPORT_T cb)
{

}

void aud_set_stay_active_mode(bool keep_active)
{

}

uint8_t aud_get_curr_a2dp_device(void)
{
    return 0;
}

uint8_t aud_get_curr_sco_device(void)
{
    return 0;
}

bool aud_is_sco_prompt_play_mode(void)
{
    return true;
}

uint8_t aud_get_a2dp_codec_type(int device_id)
{
    return BT_A2DP_CODEC_TYPE_SBC;
}

int aud_switch_sco(uint16_t  scohandle)
{
    return 0;
}

void aud_report_hfp_speak_gain(void)
{

}

void aud_report_a2dp_speak_gain(void)
{

}

bool aud_hfp_mic_need_skip_frame(void)
{
    return false;
}

uint8_t aud_hfp_need_mute(void)
{
    return false;
}

void aud_hfp_set_local_vol(int id, uint8_t vol)
{

}

uint8_t aud_adjust_hfp_volume(uint8_t device_id, bool up, bool adjust_local_vol_level)
{
    return 0;
}

uint8_t aud_adjust_a2dp_volume(uint8_t device_id, bool up, bool adjust_local_vol_level)
{
    return 0;
}

bool ignore_ring_and_play_tone_self(int device_id)
{
    return false;
}

static const bes_aud_bt_t g_bes_aud_bt = {
    .aud_set_stay_active_mode = aud_set_stay_active_mode,
    .aud_get_curr_a2dp_device = aud_get_curr_a2dp_device,
    .aud_get_curr_sco_device = aud_get_curr_sco_device,
    .aud_is_sco_prompt_play_mode = aud_is_sco_prompt_play_mode,
    .aud_get_a2dp_codec_type = aud_get_a2dp_codec_type,
    .aud_switch_sco = aud_switch_sco,
    .aud_report_hfp_speak_gain = aud_report_hfp_speak_gain,
    .aud_report_a2dp_speak_gain = aud_report_a2dp_speak_gain,
    .aud_hfp_mic_need_skip_frame = aud_hfp_mic_need_skip_frame,
    .aud_hfp_need_mute = aud_hfp_need_mute,
    .aud_hfp_set_local_vol = aud_hfp_set_local_vol,
    .aud_adjust_hfp_volume = aud_adjust_hfp_volume,
    .aud_adjust_a2dp_volume = aud_adjust_a2dp_volume,
    .ignore_ring_and_play_tone_self = ignore_ring_and_play_tone_self,
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
    .a2dp_lhdc_get_ext_flags = NULL,
    .a2dp_lhdc_config_llc_get = NULL,
#endif
#if defined(BT_A2DP_SUPPORT)
    .a2dp_get_non_type_by_device_id = NULL,
#endif // BT_A2DP_SUPPORT
};

const bes_aud_bt_t * const bes_aud_bt= &g_bes_aud_bt;
#endif /* BLE_ONLY_ENABLED */

#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)

uint8_t bt_sbc_player_get_sample_bit(void)
{
    return 16;
}

uint8_t app_bt_get_curr_a2dp_sample_rate(uint8_t curr_a2dp_device_id)
{
    return 0x20; // A2D_SBC_IE_SAMP_FREQ_44;
}

uint16_t a2dp_Get_curr_a2dp_conhdl(void)
{
    return 0xFFFF;
}

bool app_bt_is_curr_a2dp_streaming(uint8_t curr_a2dp_device_id)
{
    return false;
}

uint16_t app_bt_get_curr_sco_hci_handle(uint8_t curr_sco_device_id)
{
    return 0xFFFF;
}

uint8_t btapp_hfp_incoming_calls(void)
{
    return 0;
}

bool btapp_hfp_is_call_active(void)
{
    return false;
}

uint8_t app_bt_audio_count_straming_mobile_links(void)
{
    return 0;
}

uint8_t app_bt_audio_count_streaming_a2dp(void)
{
    return 0;
}

uint8_t btif_me_get_activeCons(void)
{
    return 0;
}

uint8_t app_bt_audio_get_curr_a2dp_device(void)
{
    return 0; // device 0
}

#endif /* BT_BUILD_WITH_CUSTOMER_HOST || BLE_ONLY_ENABLED */

void bt_adapter_register_hci_log_report_callback(uint16_t max_len,
                int (*tx_cb)(const uint8_t *buf, uint16_t len),
                int (*rx_cb)(const uint8_t *buf, uint16_t len))
{
    hci_register_log_report_callback(max_len, tx_cb, rx_cb);
}
