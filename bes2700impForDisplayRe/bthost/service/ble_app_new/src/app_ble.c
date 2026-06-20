/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifdef BLE_HOST_SUPPORT
#undef MOUDLE
#define MOUDLE APP_BLE
#include "bluetooth_ble_api.h"
#include "apps.h"
#include "app_ble.h"
#include "app_a2dp.h"
#include "app_hfp.h"
#include "app_bt.h"
#include "nvrecord_ble.h"
#include "nvrecord_bt.h"
#include "bluetooth_nv_mgr.h"
#include "bt_drv_reg_op.h"
#include "hci_i.h"
#ifndef BLE_ONLY_ENABLED
#include "sdp_service.h"
#endif
#ifdef IBRT
#include "app_ibrt_internal.h"
#include "app_tws_ibrt.h"
#include "app_tws_ibrt_conn_api.h"
#if BLE_AUDIO_ENABLED
#include "aob_gaf_api.h"
#include "app_ui_api.h"
#include "gaf_service.h"
#endif
#endif
#ifdef GFPS_ENABLED
#include "gfps_ble.h"
#endif

#include "bes_gap_api.h"

#include "app_utils.h"

#ifdef BLE_WIRELESS_TRANS_SRV_ENABLED
#include "ble_wireless_trans_srv.h"
#endif

#ifdef BLE_WIRELESS_TRANS_CLI_ENABLED
#include "ble_wireless_trans_cli.h"
#endif

#ifdef BLE_MESH_ENABLE
#include "bt_mesh_ble_export.h"
#endif

#ifndef ADV_DATA_LEN
#define ADV_DATA_LEN                    (0x1F)
#endif
#ifndef BLE_ADV_FLAG_PART_LEN
#define BLE_ADV_FLAG_PART_LEN           (0x03)
#endif
#ifndef BLE_ADV_DATA_WITHOUT_FLAG_LEN
#define BLE_ADV_DATA_WITHOUT_FLAG_LEN   (ADV_DATA_LEN)
#endif
#ifndef BLE_ADV_DATA_WITH_FLAG_LEN
#define BLE_ADV_DATA_WITH_FLAG_LEN      (ADV_DATA_LEN - BLE_ADV_FLAG_PART_LEN)
#endif
#ifndef SCAN_RSP_DATA_LEN
#define SCAN_RSP_DATA_LEN                (0x1F)
#endif

/* EXTERN */
#if (APP_BLE_DEMO_APP_ENABLED)
extern int ble_demo_app_init(uint8_t adv_hdl_shared, const bt_bdaddr_t *p_app_ia_shared, const uint8_t *p_irk_shared);
extern uint32_t ble_demo_app_role_switch_handler(uint8_t conidx, bool is_restore, uint8_t *buf, uint16_t buf_len);
#endif /*APP_BLE_DEMO_APP_ENABLED*/

#ifdef CFG_APP_DATAPATH_SERVER
extern uint32_t ble_datapath_save_ctx(uint8_t conidx, uint8_t *buf, uint32_t buf_len);
extern uint32_t ble_datapath_restore_ctx(uint8_t conidx, uint8_t *buf, uint32_t buf_len);
#endif /* CFG_APP_DATAPATH_SERVER */

extern bool app_is_power_off_in_progress(void);
static void app_ble_impl_refresh_adv(void);
static void app_ble_check_load_server_cache(const gap_conn_item_t *conn);
static void app_ble_check_load_client_cache(const gap_conn_item_t *conn);
static void app_ble_gatt_server_cache(const gap_conn_item_t *conn, const gatt_server_cache_t *cache);
static void app_ble_gatt_client_cache(const gap_conn_item_t *conn, const gatt_client_cache_t *cache);
static void app_ble_global_handle(ble_event_t *event, void *output);

#if (BLE_AUDIO_ENABLED)
bool aob_conn_start_adv(bool, bool, bool);
#endif
ble_adv_activity_t *app_ble_get_advertising(uint8_t adv_handle);

/* DECLARATIONS */

static ble_global_t g_ble_global;

ble_global_t *ble_get_global(void)
{
    return &g_ble_global;
}

void app_ble_core_evt_cb_register(APP_BLE_CORE_EVENT_CALLBACK cb)
{
    ble_global_t *g = ble_get_global();
    g->ble_core_evt_cb = cb; // app_ble_core_evt_cb_p
}

void app_ble_core_register_global_handler_ind(APP_BLE_CORE_GLOBAL_HANDLER_FUNC handler)
{
    ble_global_t *g = ble_get_global();

    for (int i=0; i<BLE_MAX_CORE_EVT_CB; i++)
    {
        if (!g->ble_global_handler[i])
        {
            g->ble_global_handler[i] = handler; // app_ble_core_evt_cb_p
            return;
        }
    }

    CO_LOG_ERR_1(BT_STS_FAILED, handler);
}

void app_ble_core_unregister_global_handler_ind(APP_BLE_CORE_GLOBAL_HANDLER_FUNC handler)
{
    ble_global_t *g = ble_get_global();
    for (int i=0; i<BLE_MAX_CORE_EVT_CB; i++)
    {
        if (g->ble_global_handler[i] == handler)
        {
            g->ble_global_handler[i] = NULL; // app_ble_core_evt_cb_p
            return;
        }
    }
}

void app_sec_reg_dist_lk_bit_set_callback(set_rsp_dist_lk_bit_field_func callback)
{
    ble_global_t *g = ble_get_global();
    g->dist_lk_set_cb = callback;
}

void *app_sec_reg_dist_lk_bit_get_callback(void)
{
    ble_global_t *g = ble_get_global();
    return g->dist_lk_set_cb;
}

void app_ble_register_ia_exchanged_callback(smp_identify_addr_exch_complete callback)
{
    ble_global_t *g = ble_get_global();
    g->ble_smp_info_derived_from_bredr_complete = callback;
}

void app_ble_resolving_list_fill_callback_register(uint8_t (*resolving_list_fill_cb)(void))
{
    ble_global_t *g = ble_get_global();
    g->ble_resolving_list_fill_cb = resolving_list_fill_cb;
}

void app_ble_smp_require_modify_callback_register(void (*ble_smp_require_modify)(uint16_t, ble_smp_require_t *))
{
    ble_global_t *g = ble_get_global();
    g->ble_smp_require_modify = ble_smp_require_modify;
}

void app_ble_smp_get_specifc_irk_ia_callback_register(void (*ble_get_specific_irk_ia)(uint16_t, uint8_t **, bt_bdaddr_t **))
{
    ble_global_t *g = ble_get_global();
    g->ble_get_specific_irk_ia = ble_get_specific_irk_ia;
}

void app_ble_gap_add_record_modify_callback_register(void (*ble_add_record_modify)(uint16_t, BleDevicePairingInfo *))
{
    ble_global_t *g = ble_get_global();
    g->ble_add_record_modify = ble_add_record_modify;
}

void app_ble_gap_get_specifc_record_callback_register(bool (*ble_get_specific_record)(uint16_t, const ble_bdaddr_t *, BleDevicePairingInfo *))
{
    ble_global_t *g = ble_get_global();
    g->ble_get_specific_record = ble_get_specific_record;
}

void app_ble_gatt_get_specifc_hash_callback_register(void (*ble_get_specific_hash)(uint16_t, uint8_t **))
{
    ble_global_t *g = ble_get_global();
    g->ble_get_specific_hash = ble_get_specific_hash;
}

uint8_t app_ble_own_addr_type(void)
{
    gap_config_t cfg = gap_get_config();
    bt_addr_type_t own_addr_type = BT_ADDR_TYPE_PUBLIC;
    // Check whether local use random identity address
    if (cfg.use_random_identity_address)
    {
        own_addr_type = BT_ADDR_TYPE_RANDOM;
    }

    return own_addr_type;
}

static bt_addr_type_t app_ble_get_own_addr_type(uint8_t ia_rpa_npa)
{
    gap_config_t cfg = gap_get_config();
    bt_addr_type_t own_addr_type = app_ble_own_addr_type();

    CO_LOG_INFO_S_3(BT_STS_OWN_ADDRESS_TYPE, OWNT, cfg.use_random_identity_address, cfg.address_reso_support, ia_rpa_npa);

    if (ia_rpa_npa == APP_GAPM_GEN_RSLV_ADDR && cfg.address_reso_support)
    {
        own_addr_type = (bt_addr_type_t)(own_addr_type + 2);
    }

    return own_addr_type;
}

bool app_ble_get_peer_solved_addr(uint8_t conidx, ble_bdaddr_t* p_addr)
{
    ble_bdaddr_t peer_bdaddr;
    bt_bdaddr_t zero_bdaddr = {{0}};

    peer_bdaddr = gap_get_peer_resolved_address(gap_zero_based_ble_conidx_as_hdl(conidx));
    if (memcmp(peer_bdaddr.addr, &zero_bdaddr, sizeof(bt_bdaddr_t)) == 0)
    {
        return false;
    }

    if (p_addr)
    {
        *p_addr = peer_bdaddr;
    }

    return true;
}

const char *app_ble_get_peer_device_name(uint8_t conidx)
{
    ble_global_t *g = ble_get_global();

    if (conidx < BLE_CONNECTION_MAX && g != NULL)
    {
        return g->peer_dev_name[conidx];
    }

    return NULL;
}

void app_ble_set_phy_mode(uint8_t conidx, uint8_t tx_phy_bits, uint8_t rx_phy_bits, uint8_t phy_opt)
{
    gap_coded_phy_prefer_t coded_prefer = GAP_CODED_PHY_NO_PREFER_CODING;
    if (tx_phy_bits == GAP_PHY_BIT_LE_CODED || rx_phy_bits == GAP_PHY_BIT_LE_CODED)
    {
        coded_prefer = (gap_coded_phy_prefer_t)phy_opt;
    }

    gap_set_le_conn_phy(gap_zero_based_ble_conidx_as_hdl(conidx), tx_phy_bits, rx_phy_bits, coded_prefer);
}

ble_bdaddr_t app_get_current_ble_addr(void)
{
    return gap_local_identity_address(app_ble_own_addr_type());
}

ble_bdaddr_t app_ble_get_local_identity_addr(uint8_t conidx)
{
    ble_bdaddr_t ble_ia_addr;
    gap_conn_item_t *conn = NULL;

    conn = gap_get_conn_item(gap_zero_based_ble_conidx_as_hdl(conidx));
    if (conn)
    {
        ble_ia_addr = gap_conn_own_identity_address(conn);
    }
    else
    {
        ble_ia_addr = gap_local_identity_address(app_ble_own_addr_type());
    }

    return ble_ia_addr;
}

const uint8_t *app_ble_get_local_rpa_addr(uint8_t conidx)
{
    gap_conn_item_t *conn = gap_get_conn_item(gap_zero_based_ble_conidx_as_hdl(conidx));

    if (conn)
    {
        if (conn->own_addr_type == BT_ADDR_TYPE_PUB_IA || conn->own_addr_type == BT_ADDR_TYPE_RND_IA)
        {
            return conn->own_rpa.address;
        }
        else
        {
            CO_LOG_ERR_1(BT_STS_NOT_RANDOM_STATIC_ADDRESS, conidx);
        }
    }
    else
    {
        CO_LOG_ERR_1(BT_STS_NO_LINK, conidx);
    }

    return NULL;
}

const uint8_t *app_ble_get_local_rpa_by_adv_hdl(uint8_t adv_hdl)
{
    ble_adv_activity_t *p_actv = app_ble_get_advertising(adv_hdl);

    if (p_actv == NULL)
    {
        CO_LOG_ERR_1(BT_STS_INVALID_ADV_HANDLE, adv_hdl);
        return NULL;
    }

    if (p_actv->adv_param.own_addr_type != BT_ADDR_TYPE_PUB_IA &&
            p_actv->adv_param.own_addr_type != BT_ADDR_TYPE_RND_IA)
    {
        CO_LOG_ERR_2(BT_STS_INVALID_TYPE, adv_hdl,
                        p_actv->adv_param.own_addr_type);
    }

    return p_actv->local_rpa.address;
}

bt_status_t app_ble_set_public_address(const bt_bdaddr_t *public_addr)
{
    return gap_set_le_public_address(public_addr);
}

bt_status_t app_ble_set_le_tx_pwr(uint16_t connhdl, int8_t tx_pwr)
{
    return gap_set_le_tx_pwr(connhdl, tx_pwr);
}

#if defined (BLE_ADV_RPA_ENABLED)
static inline void app_ble_fake_resolving_list_item_using_self_info(void)
{
    CO_LOG_INFO_S_0(BT_STS_SET_FILTER_LIST, FRLS);
    // Always add local address into RPA list
    gap_resolving_list_add(BT_ADDR_TYPE_PUBLIC,
                           (bt_bdaddr_t *)bt_get_ble_local_address(),
                           gap_get_local_irk(), false, false);
}
#endif

static void app_ble_read_local_rpa_cmpl(uint16_t cmd_opcode, struct hci_cmd_evt_param_t *param)
{
    struct hci_le_read_local_rpa_cmpl *cmpl = (struct hci_le_read_local_rpa_cmpl *)param->cmpl_event;
    ble_adv_activity_t *p_actv = (ble_adv_activity_t *)param->priv;

    if (cmpl->status == HCI_ERROR_NO_ERROR && p_actv)
    {
        CO_LOG_INFO_S_3(*(cmpl->local_rpa), ARPA, *(cmpl->local_rpa + 1), *(cmpl->local_rpa + 3), *(cmpl->local_rpa + 5));
        memcpy(p_actv->local_rpa.address, cmpl->local_rpa, sizeof(cmpl->local_rpa));
    }
    else
    {
        CO_LOG_ERR_2(BT_STS_INVALID_STATUS, cmpl->status, p_actv);
        memset(&p_actv->local_rpa, 0, sizeof(p_actv->local_rpa));
    }
}

static void app_ble_read_local_rpa_addr_impl(const ble_adv_activity_t *p_actv)
{
    if (p_actv && p_actv->adv_is_started &&
            p_actv->adv_param.own_addr_type >= BT_ADDR_TYPE_PUB_IA)
    {
        gap_read_local_rpa(p_actv->adv_param.peer_type,
                            &p_actv->adv_param.peer_addr,
                            app_ble_read_local_rpa_cmpl, (void *)p_actv);
    }
    else if (p_actv == NULL || p_actv->adv_is_started == false)
    {
        CO_LOG_ERR_2(BT_STS_INVALID_ADV_HANDLE, p_actv ? p_actv->adv_is_started : 0,
                                                p_actv ? p_actv->adv_param.own_addr_type : 0xFF);
    }
}

void app_ble_read_local_rpa_by_adv_hdl(uint8_t adv_hdl)
{
    if (bt_defer_curr_func_1(app_ble_read_local_rpa_by_adv_hdl,
                             bt_fixed_param(adv_hdl)))
    {
        return;
    }

    app_ble_read_local_rpa_addr_impl(app_ble_get_advertising(adv_hdl));
}

bool app_ble_is_remote_dev_connected(const ble_bdaddr_t *p_addr)
{
    uint8_t conidx = 0;
    ble_bdaddr_t peer_addr = {0};
    gap_conn_item_t *conn = NULL;

    if (p_addr == NULL)
    {
        return false;
    }

    // First check acl address that use estb evt
    if (p_addr->addr_type != BT_ADDR_TYPE_PUBLIC)
    {
        conn = gap_get_conn_by_le_address((bt_addr_type_t)p_addr->addr_type,
                                            (bt_bdaddr_t *)p_addr->addr);
        
        return (conn != NULL);
    }
    // Second check solved addr
    for (conidx = 0; conidx < BLE_CONNECTION_MAX; conidx++)
    {
        if (app_ble_get_peer_solved_addr(conidx, &peer_addr) &&
                (memcmp(&peer_addr, p_addr, sizeof(peer_addr)) == 0))
        {
            return true;
        }
    }

    return false;
}

uint8_t app_ble_connection_count(void)
{
    return gap_count_ble_connections();
}

bool app_is_arrive_at_max_ble_connections(void)
{
    return app_ble_connection_count() >= BLE_CONNECTION_MAX;
}

bool app_ble_is_any_connection_exist(void)
{
    return app_ble_connection_count() > 0;
}

bool app_ble_is_connection_on(uint8_t index)
{
    return gap_get_conn_item(gap_zero_based_ble_conidx_as_hdl(index)) != NULL;
}

uint16_t app_ble_get_conhdl_from_conidx(uint8_t conidx)
{
    return gap_conn_hdl(gap_zero_based_conidx_to_ble_conidx(conidx));
}

static bool app_ble_check_device_conhdl(gap_conn_item_t *conn, void *priv)
{
    uint32_t *p_connhdl_conidx = (uint32_t *)priv;

    if (((*p_connhdl_conidx >> 16) == conn->connhdl) &&
            conn->conn_type == HCI_CONN_TYPE_LE_ACL)
    {
        *p_connhdl_conidx = gap_zero_based_conidx(conn->con_idx);
        return gap_end_loop;
    }

    return gap_continue_loop;
}

uint8_t app_ble_get_conidx_from_conhdl(uint16_t connhdl)
{
    uint32_t connhdl_conidx = (connhdl << 16) | GAP_INVALID_CONIDX;

    gap_conn_foreach(app_ble_check_device_conhdl, &connhdl_conidx);

    return (gap_zero_based_conidx(connhdl_conidx) >= BLE_CONNECTION_MAX)
                                        ? GAP_INVALID_CONIDX : connhdl_conidx;
}

void app_ble_set_white_list(BLE_WHITE_LIST_USER_E user, ble_bdaddr_t *bdaddr, uint8_t size)
{
    if (bt_defer_curr_func_3(app_ble_set_white_list, bt_fixed_param(user),
                                                     bt_alloc_param_size(bdaddr, size * sizeof(ble_bdaddr_t)),
                                                     bt_fixed_param(size)))
    {
        return;
    }

    gap_filter_list_clear();

    if (bdaddr == NULL || size == 0)
    {
        CO_LOG_INFO_S_0(BT_STS_SET_FILTER_LIST, CLFL);
        return;
    }

    CO_LOG_INFO_S_3(BT_STS_SET_FILTER_LIST, STFL, (user<<16)|size, bdaddr->addr_type,
        (bdaddr->addr[0]<<24)|(bdaddr->addr[1]<<16)|(bdaddr->addr[2]<<8)|bdaddr->addr[5]);
    gap_filter_list_add_user_item(user, bdaddr, size);
}

void app_ble_clear_white_list(BLE_WHITE_LIST_USER_E user)
{
    if (bt_defer_curr_func_1(app_ble_clear_white_list, bt_fixed_param(user)))
    {
        return;
    }

    gap_filter_list_remove_user_item(user);
}

void app_ble_clear_all_white_list(void)
{
    if (bt_defer_curr_func_0(app_ble_clear_all_white_list))
    {
        return;
    }

    gap_filter_list_clear();
}

void app_ble_add_dev_to_rpa_list_in_controller(const ble_bdaddr_t *ble_addr, const uint8_t *irk)
{
    if (bt_defer_curr_func_2(app_ble_add_dev_to_rpa_list_in_controller, bt_alloc_param(ble_addr),
                                                                        bt_alloc_param_size(irk, 16)))
    {
        return;
    }

    gap_resolving_list_add((bt_addr_type_t)ble_addr->addr_type, (bt_bdaddr_t *)ble_addr->addr, irk, true, false);

    app_ble_refresh_adv_state_generic();
}

void app_ble_add_devices_info_to_resolving(void)
{
    if (bt_defer_curr_func_0(app_ble_add_devices_info_to_resolving))
    {
        return;
    }

    ble_global_t *g = ble_get_global();
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *dev_info = NULL;
    BleDeviceinfo *record = NULL;
    uint8_t list_limit_num = BLE_RECORD_NUM;
    ble_bdaddr_t irk_ble_addr[BLE_RECORD_NUM];
    uint32_t total_ble_records = 0;
    uint32_t added_irk_records = 0;
    uint32_t i = 0;

    // Clear resolving list first to trigger AdvA regenerated
    gap_resolving_list_clear(false);
    // Other app want to add resolving list
    if (g->ble_resolving_list_fill_cb != NULL)
    {
        list_limit_num -= g->ble_resolving_list_fill_cb();
    }
#if defined (BLE_ADV_RPA_ENABLED)
    // Add an fake item that peer addr is device itself, seems like bonded with itself before,
    // it makes chance to start adv with local addr filled in peer addr field, then controller
    // can search resolving list and get this fake item to generate rpa with local irk filled.
    app_ble_fake_resolving_list_item_using_self_info();
    // No enough resolving list entry for devices
    list_limit_num -= 1;
#endif

    dev_info = nv_record_blerec_get_ptr();
    total_ble_records = dev_info->saved_list_num;

    for (i = 0; i < total_ble_records; i++)
    {
        record = dev_info->ble_nv + i;
        if (record->pairingInfo.bond_info_bf & CO_BIT_MASK(BONDED_WITH_IRK_DISTRIBUTED))
        {
            if (added_irk_records < list_limit_num)
            {
                irk_ble_addr[added_irk_records].addr_type = record->pairingInfo.peer_addr.addr_type;
                memcpy(irk_ble_addr[added_irk_records].addr, record->pairingInfo.peer_addr.addr, GAP_ADDR_LEN);
                gap_resolving_list_add((bt_addr_type_t)record->pairingInfo.peer_addr.addr_type,
                    (bt_bdaddr_t *)record->pairingInfo.peer_addr.addr, record->pairingInfo.IRK, false, false);
                added_irk_records += 1;
            }
            else
            {
                CO_LOG_ERR_2(BT_STS_REACH_UPPER_LIMIT, total_ble_records, added_irk_records);
            }
        }
    }

    CO_LOG_INFO_3(BT_STS_RESO_LIST_ADD_ALL, total_ble_records, added_irk_records, gap_address_reso_is_enabled());

    for (i = 0; i < added_irk_records; i += 1)
    {
        gap_set_device_privacy_mode((bt_addr_type_t)irk_ble_addr[i].addr_type,
            (bt_bdaddr_t *)irk_ble_addr[i].addr, false, false);
    }

    app_ble_refresh_adv_state_generic();
}

void app_ble_send_security_req(uint8_t conidx)
{
    gap_start_authentication(gap_zero_based_ble_conidx_as_hdl(conidx), GAP_AUTH_STARTED_BY_UPPER_APP);
}

bt_status_t app_ble_start_authentication(uint8_t conidx, const ble_smp_require_t *p_smp_req)
{
    bt_status_t status = BT_STS_SUCCESS;

    ble_global_t *g = ble_get_global();

    if (p_smp_req == NULL || conidx >= BLE_CONNECTION_MAX)
    {
        return BT_STS_INVALID_PARM;
    }

    if (g->p_smp_req[conidx] != NULL)
    {
        return BT_STS_IN_PROGRESS;
    }

    g->p_smp_req[conidx] = (ble_smp_require_t *)bes_bt_buf_malloc(sizeof(ble_smp_require_t));

    if (g->p_smp_req[conidx] == NULL)
    {
        return BT_STS_NO_RESOURCES;
    }

    *g->p_smp_req[conidx] = *p_smp_req;

    status = gap_start_authentication(gap_zero_based_ble_conidx_as_hdl(conidx), GAP_AUTH_STARTED_BY_UPPER_APP);

    if (status != BT_STS_SUCCESS)
    {
        bes_bt_buf_free(g->p_smp_req[conidx]);
        g->p_smp_req[conidx] = NULL;
    }

    return status;
}

bt_status_t app_ble_enable_link_encryption(uint8_t conidx, const uint8_t *ediv, const uint8_t *rand, const uint8_t *ltk)
{
    gap_ltk_enc_info_t enc_data = {0};
    gap_conn_item_t *conn = gap_get_conn_item(gap_zero_based_ble_conidx_as_hdl(conidx));

    if (ltk == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    if (conn == NULL)
    {
        return BT_STS_NOT_FOUND;
    }

    if (ediv && rand)
    {
        memcpy(enc_data.ediv, ediv, sizeof(enc_data.ediv));
        memcpy(enc_data.rand, rand, sizeof(enc_data.rand));
    }

    return gap_start_enable_encryption(conn->connhdl, &enc_data, ltk);
}

bt_status_t app_ble_send_smp_pairing_response(uint8_t conidx, const ble_smp_require_t *p_smp_req)
{
    bt_status_t status = BT_STS_SUCCESS;
    gap_conn_item_t *conn = gap_get_conn_item(gap_zero_based_ble_conidx_as_hdl(conidx));

    ble_global_t *g = ble_get_global();

    if (p_smp_req == NULL || conidx >= BLE_CONNECTION_MAX || conn == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    if (conn->conn_flag.is_central == true)
    {
        return BT_STS_NOT_ALLOW;
    }

    if (g->p_smp_req[conidx] != NULL)
    {
        return BT_STS_IN_PROGRESS;
    }

    g->p_smp_req[conidx] = (ble_smp_require_t *)bes_bt_buf_malloc(sizeof(ble_smp_require_t));

    if (g->p_smp_req[conidx] == NULL)
    {
        return BT_STS_NO_RESOURCES;
    }

    *g->p_smp_req[conidx] = *p_smp_req;

    status = gap_reply_peer_pairing_request(conn->connhdl, false, 0);

    if (status != BT_STS_SUCCESS)
    {
        bes_bt_buf_free(g->p_smp_req[conidx]);
        g->p_smp_req[conidx] = NULL;
    }

    return status;
}

void app_ble_set_local_irk(const uint8_t *p_irk)
{
    if (p_irk != NULL)
    {
        gap_set_local_irk(p_irk);
    }
}

void app_ble_get_local_irk(uint8_t *p_irk)
{
    if (p_irk != NULL)
    {
        memcpy(p_irk, gap_get_local_irk(), GAP_KEY_LEN);
    }
}

void app_ble_data_fill_enable(BLE_ADV_USER_E user, bool enable)
{
    ble_global_t *g = ble_get_global();
    if (user < BLE_ADV_USER_NUM)
    {
        g->data_fill_enable[user] = enable;
    }
}

void app_ble_register_data_fill_handle(BLE_ADV_USER_E user, BLE_DATA_FILL_FUNC_T func, bool enable)
{
    ble_global_t *g = ble_get_global();
    if (user < BLE_ADV_USER_NUM)
    {
        g->data_fill_func[user] = func;
    }
}

bool ble_adv_is_allowed(void)
{
    if (!gap_stack_is_ready())
    {
        return false;
    }

    if (app_is_power_off_in_progress())
    {
        return false;
    }

    if (ble_get_global()->adv_force_disabled)
    {
        CO_LOG_ERR_1(BT_STS_ADV_FORCE_DISABLED, ble_get_global()->adv_force_disabled);
        return false;
    }

#if ((BLE_AUDIO_ENABLED == 0) && (!defined SASS_ENABLED)) && defined(BT_HFP_SUPPORT)
    if (btapp_hfp_is_sco_active())
    {
        return false;
    }
#endif

    if (app_is_arrive_at_max_ble_connections())
    {
        CO_LOG_WAR_0(BT_STS_REACH_MAX_NUMBER);
        return false;
    }

    return true;
}

void app_ble_dt_set_flags(gap_adv_param_t *adv_param, bool simu_bredr_support)
{
    uint8_t discoverable_mode = GAP_FLAGS_LE_NON_DISCOVERABLE_MODE;

    if (adv_param->connectable || adv_param->scannable)
    {
        discoverable_mode = adv_param->limited_discoverable_mode  ?
            GAP_FLAGS_LE_LIMITED_DISCOVERABLE_MODE :
            GAP_FLAGS_LE_GENERAL_DISCOVERABLE_MODE | GAP_FLAGS_SIMU_LE_BREDR_TO_SAME_DEVICE;
    }
    else
    {
        discoverable_mode = GAP_FLAGS_LE_NON_DISCOVERABLE_MODE;
    }

    if (discoverable_mode && adv_param->start_bg_advertising)
    {
        discoverable_mode = GAP_FLAGS_LE_GENERAL_DISCOVERABLE_MODE | GAP_FLAGS_SIMU_LE_BREDR_TO_SAME_DEVICE;
    }

    gap_dt_add_flags(&adv_param->adv_data, discoverable_mode, simu_bredr_support);
}

void app_ble_dt_set_local_name(gap_adv_param_t *adv_param, const char *cust_le_name)
{
    if (!gap_dt_buf_find_type(&adv_param->adv_data, GAP_DT_SHORT_LOCAL_NAME, GAP_DT_COMPLETE_LOCAL_NAME))
    {
        gap_dt_add_local_le_name(&adv_param->adv_data, adv_param->use_legacy_pdu, cust_le_name);
    }
}

static void app_ble_clean_adv_param(gap_adv_param_t *adv_param)
{
    gap_dt_buf_clear(&adv_param->adv_data);
    gap_dt_buf_clear(&adv_param->scan_rsp_data);
    memset(adv_param, 0, sizeof(gap_adv_param_t));
}

static ble_adv_activity_t *app_ble_find_adv_activity(uint8_t adv_handle)
{
    ble_global_t *g = ble_get_global();
    ble_adv_activity_t *adv = NULL;
    int activity_count = ARRAY_SIZE(g->adv);
    int i = 0;

    if (adv_handle == GAP_INVALID_ADV_HANDLE)
    {
        return NULL;
    }

    for (; i < activity_count; i += 1)
    {
        adv = g->adv + i;
        if (adv->adv_handle == adv_handle)
        {
            return adv;
        }
    }

    return NULL;
}

bool app_ble_check_device_master_role(void)
{
#if defined(IBRT)
#if defined(FREEMAN_ENABLED_STERO)
    return true;
#else
    if (app_ibrt_conn_is_freeman_mode())
    {
        return true;
    }
    return (app_ibrt_conn_get_ui_role() != TWS_UI_SLAVE);
#endif
#else
    return true;
#endif
}

// bit mask of the existing conn param modes
static uint32_t existingBleConnParamModes[BLE_CONNECTION_MAX] = {0};

// interval in the unit of 1.25ms
static const BLE_CONN_PARAM_CONFIG_T ble_conn_param_config[BLE_CONN_PARAM_MODE_NUM] =
{
    // default value: for the case of BLE just connected and the BT idle state
    {BLE_CONN_PARAM_MODE_DEFAULT, BLE_CONN_PARAM_PRIORITY_NORMAL, 36, 36, 0},

    {BLE_CONN_PARAM_MODE_AI_STREAM_ON, BLE_CONN_PARAM_PRIORITY_ABOVE_NORMAL1, 16, 16, 0},

    {BLE_CONN_PARAM_MODE_A2DP_ON, BLE_CONN_PARAM_PRIORITY_ABOVE_NORMAL0, 48, 48, 0},

    {BLE_CONN_PARAM_MODE_HFP_ON, BLE_CONN_PARAM_PRIORITY_ABOVE_NORMAL2, 48, 48, 0},

    {BLE_CONN_PARAM_MODE_OTA, BLE_CONN_PARAM_PRIORITY_HIGH, 12, 12, 0},

    {BLE_CONN_PARAM_MODE_OTA_SLOWER, BLE_CONN_PARAM_PRIORITY_HIGH, 20, 20, 0},

    {BLE_CONN_PARAM_MODE_SNOOP_EXCHANGE, BLE_CONN_PARAM_PRIORITY_HIGH, 8, 24, 0},

    // BAP Spec Table 8.3
    // To avoid the service discovery and encryption setup taking too long,
    // the Central should use the short connection intervals defined below
    {BLE_CONN_PARAM_MODE_SVC_DISC, BLE_CONN_PARAM_PRIORITY_ABOVE_NORMAL2, 8, 24, 0},

    // BAP Spec Table 8.4
    {BLE_CONN_PARAM_MODE_ISO_DATA, BLE_CONN_PARAM_PRIORITY_HIGH, 48, 48, 0},

    {BLE_CONN_PARAM_MODE_IDLE, BLE_CONN_PARAM_PRIORITY_NORMAL, 400, 400, 0},
    // TODO: add mode cases if needed
};

typedef struct {
    BLE_CONN_PARAM_MODE_E mode;
    bool enable;
} app_ble_updata_conn_param_t;

static bool app_ble_update_conn_foreach(gap_conn_item_t *conn, void *priv)
{
    app_ble_updata_conn_param_t *p = (app_ble_updata_conn_param_t *)priv;
    // Only update le acl
    if (conn->conn_type == HCI_CONN_TYPE_LE_ACL)
    {
        app_ble_update_conn_param_mode_of_specific_connection(conn->con_idx, p->mode, p->enable);
    }
    return gap_continue_loop;
}

void app_ble_update_conn_param_mode(BLE_CONN_PARAM_MODE_E mode, bool enable)
{
    app_ble_updata_conn_param_t param = {0};
    param.mode = mode;
    param.enable = enable;
    gap_conn_foreach(app_ble_update_conn_foreach, &param);
}

void app_ble_reset_conn_param_mode(uint8_t con_idx)
{
    uint8_t conidx = gap_zero_based_conidx(con_idx);
    existingBleConnParamModes[conidx] = 0;
}

void app_ble_update_conn_param_mode_of_specific_connection(uint8_t con_idx, BLE_CONN_PARAM_MODE_E mode, bool enable)
{
#ifdef IS_INITIATIVE_BLE_UPDATE_PARAMETER_DISABLED
    return;
#endif

    gap_update_params_t params = {0};
    const BLE_CONN_PARAM_CONFIG_T *pConfig = &ble_conn_param_config[mode];
    uint8_t conidx = gap_zero_based_conidx(con_idx);
    uint32_t index = 0;

    if (conidx > BLE_CONNECTION_MAX)
    {
        CO_LOG_ERR_1(BT_STS_INVALID_CONNECTION, conidx);
        return;
    }

    CO_LOG_INFO_4(BT_STS_CONN_STATUS, conidx, existingBleConnParamModes[conidx], mode, enable);

    if (enable)
    {
        if (existingBleConnParamModes[conidx] & (1 << mode))
        {
            // already existing, directly return
            return;
        } else {
            // update the bit-mask
            existingBleConnParamModes[conidx] |= (1 << mode);
            // not existing yet, need to go throuth the existing params to see whether
            // we need to update the param
            for (index = 0; index < ARRAY_SIZE(ble_conn_param_config); index++)
            {
                if (((uint32_t)1 << ble_conn_param_config[index].ble_conn_param_mode) &
                    existingBleConnParamModes[conidx])
                {
                    if (ble_conn_param_config[index].priority > pConfig->priority)
                    {
                        // one of the exiting param has higher priority than this one,
                        // so do nothing but update the bit-mask
                        return;
                    }
                }
            }
            // no higher priority conn param existing, so we need to apply this one
        }
    }
    else
    {
        // doesn't exist, directly return
        if (!(existingBleConnParamModes[conidx] & (1 << mode)))
        {
            return;
        }
        else
        {
            uint8_t priorityDisabled = pConfig->priority;
            // update the bit-mask
            existingBleConnParamModes[conidx] &= (~(1 << mode));

            if (0 == existingBleConnParamModes[conidx])
            {
                // restore to the default CI
                pConfig = &ble_conn_param_config[0];
                goto label_update;
            }

            pConfig = NULL;
            // existing, need to apply for the highest priority conn param
            for (index = 0; index < ARRAY_SIZE(ble_conn_param_config); index++)
            {
                if ((( uint32_t )1 << ( uint8_t )ble_conn_param_config[index].ble_conn_param_mode) &
                    existingBleConnParamModes[conidx])
                {
                    if (NULL != pConfig)
                    {
                        if (ble_conn_param_config[index].priority > pConfig->priority)
                        {
                            pConfig = &ble_conn_param_config[index];
                        }
                    }
                    else
                    {
                        pConfig = &ble_conn_param_config[index];
                    }
                }
            }

            if (pConfig && priorityDisabled < pConfig->priority) {
                // The priority of the disabled mode is less than that of the existing one,
                // indicating that it is already latest
                return;
            }
        }
    }

label_update:
    params.conn_interval_min_1_25ms = pConfig->conn_interval_min;
    params.conn_interval_max_1_25ms = pConfig->conn_interval_max;
    params.max_peripheral_latency = pConfig->conn_slave_latency_cnt;
    gap_update_le_conn_parameters(gap_zero_based_ble_conidx_as_hdl(conidx), &params);
}

#if GFPS_ENABLED
uint8_t delay_update_conidx = GAP_INVALID_CONIDX;
#define FP_DELAY_UPDATE_BLE_CONN_PARAM_TIMER_VALUE (10000)
osTimerId fp_update_ble_param_timer = NULL;
static void fp_update_ble_connect_param_timer_handler(void const *param);
osTimerDef (FP_UPDATE_BLE_CONNECT_PARAM_TIMER, (void (*)(void const *))fp_update_ble_connect_param_timer_handler);
extern uint8_t amgr_is_bluetooth_sco_on (void);

static void fp_update_ble_connect_param_timer_handler(void const *param)
{
    if (delay_update_conidx != GAP_INVALID_CONIDX)
    {
        if (amgr_is_bluetooth_sco_on())
        {
            app_ble_update_conn_param_mode_of_specific_connection(delay_update_conidx, BLE_CONN_PARAM_MODE_HFP_ON, true);
        }
        else
        {
            app_ble_update_conn_param_mode_of_specific_connection(delay_update_conidx, BLE_CONN_PARAM_MODE_DEFAULT, true);
        }

        delay_update_conidx = GAP_INVALID_CONIDX;
    }
}

void fp_update_ble_connect_param_start(uint8_t ble_conidx)
{
    if (delay_update_conidx != GAP_INVALID_CONIDX)
    {
        return;
    }

    if (fp_update_ble_param_timer == NULL)
    {
        fp_update_ble_param_timer = osTimerCreate(osTimer(FP_UPDATE_BLE_CONNECT_PARAM_TIMER), osTimerOnce, NULL);
        if (fp_update_ble_param_timer == NULL)
        {
            return;
        }
    }

    delay_update_conidx = ble_conidx;
    osTimerStart(fp_update_ble_param_timer, FP_DELAY_UPDATE_BLE_CONN_PARAM_TIMER_VALUE);
}

void fp_update_ble_connect_param_stop(uint8_t ble_conidx)
{
    if (delay_update_conidx == ble_conidx)
    {
        if (fp_update_ble_param_timer)
        {
            osTimerStop(fp_update_ble_param_timer);
        }
        delay_update_conidx = GAP_INVALID_CONIDX;
    }
}
#endif

static void app_ble_conn_param_update_req(gap_conn_update_req_t *update_req)
{
    bool accept = true;
    uint16_t conn_interval_min_1_25ms = update_req->params_req.conn_interval_min_1_25ms;
    uint32_t conn_interval_min_us = conn_interval_min_1_25ms * 1250;

    ble_event_t cb_event = {0};

    cb_event.evt_type = BLE_CONN_PARAM_UPDATE_REQ_EVENT;
    cb_event.p.conn_param_update_req_handled.conidx = gap_zero_based_conidx(update_req->con_idx);
    cb_event.p.conn_param_update_req_handled.intv_min = update_req->params_req.conn_interval_min_1_25ms;
    cb_event.p.conn_param_update_req_handled.intv_max = update_req->params_req.conn_interval_max_1_25ms;
    cb_event.p.conn_param_update_req_handled.latency = update_req->params_req.max_peripheral_latency;
    cb_event.p.conn_param_update_req_handled.time_out = update_req->params_req.superv_timeout_ms / 10;

    app_ble_global_handle(&cb_event, &accept);

    CO_LOG_INFO_0(accept);


#ifdef GFPS_ENABLED
    //make sure ble cnt interval is not less than 15ms, in order to prevent bt collision
    if (conn_interval_min_us < 15000)
    {
        fp_update_ble_connect_param_start(gap_zero_based_conidx(update_req->con_idx));
    }
    else
    {
        fp_update_ble_connect_param_stop(gap_zero_based_conidx(update_req->con_idx));
    }
#else
    bool music_ongoing = false;
    bool sco_active = false;
#ifdef BT_A2DP_SUPPORT
    music_ongoing = a2dp_is_music_ongoing();
#endif
#ifdef BT_HFP_SUPPORT
    sco_active = btapp_hfp_is_sco_active();
#endif
    if (music_ongoing || sco_active)
    {
        // when a2dp or sco streaming is on-going
        // make sure ble cnt interval is not less than 15ms, in order to prevent bt collision
        if (conn_interval_min_us < 15000)
        {
            accept = false;
        }
    }
#endif

    gap_accept_le_conn_parameters(update_req->connhdl, &update_req->params_req, accept);
}

static void app_ble_conn_param_update_cmpl(gap_conn_item_t *conn, uint16_t err_code)
{
    ble_event_t cb_event;

    if (err_code == BT_STS_SUCCESS)
    {
        cb_event.evt_type = BLE_CONN_PARAM_UPDATE_SUCCESSFUL_EVENT;
        cb_event.p.conn_param_update_successful_handled.conidx = gap_zero_based_conidx(conn->con_idx);
        cb_event.p.conn_param_update_successful_handled.con_interval = conn->timing.conn_interval_1_25ms;
        cb_event.p.conn_param_update_successful_handled.con_latency = conn->timing.peripheral_latency;
        cb_event.p.conn_param_update_successful_handled.sup_to = conn->timing.superv_timeout_ms / 10;
        app_ble_global_handle(&cb_event, NULL);
    }
    else
    {
        cb_event.evt_type = BLE_CONN_PARAM_UPDATE_FAILED_EVENT;
        cb_event.p.conn_param_update_failed_handled.conidx = gap_zero_based_conidx(conn->con_idx);
        cb_event.p.conn_param_update_failed_handled.err_code = err_code;
        app_ble_global_handle(&cb_event, NULL);
    }
}

static void app_ble_global_handle(ble_event_t *event, void *output)
{
    ble_global_t *g = ble_get_global();

    if (g->ble_core_evt_cb)
    {
        g->ble_core_evt_cb(event);
    }

    for (int i=0; i<BLE_MAX_CORE_EVT_CB; i++)
    {
        if (g->ble_global_handler[i])
        {
            g->ble_global_handler[i](event, output);
        }
    }
}

static void app_ble_stack_ready(void)
{
    CO_LOG_INFO_S_0(BT_STS_GLOBAL_STATUS, STKR);

    gap_set_le_host_feature_support(GAP_HOST_FEAT_BIT_CONN_SUBRAT_HOST_SUPP, true);

    gap_set_send_sec_error_rsp_directly(true);

    gap_set_report_sec_error_directly(true);

    bt_host_ready(STACK_READY_BLE);

#ifdef CUSTOMER_DEFINE_ADV_DATA
    app_ble_custom_init();
#endif

#ifdef BLE_MESH_ENABLE
    app_ble_mesh_init();
#endif

#if (BLE_AUDIO_ENABLED)
#if defined (IBRT)
    ble_audio_tws_sync_init();
#endif

#ifndef BLE_AUDIO_TEST_ENABLED
    ble_audio_tws_init();   //Normal init cfg
#endif

#elif BLE_ISO_ENABLED
    gap_set_le_host_feature_support(GAP_HOST_FEAT_BIT_CIS_HOST_SUPPORT, true);
#endif //end (BLE_AUDIO_ENABLED)

    app_ble_ready_and_init_done();
}

static void app_ble_reply_peer_ltk_request(gap_conn_item_t *conn, uint16_t ediv)
{
    ble_bdaddr_t ble_addr = {0};
    uint8_t ltk[GAP_KEY_LEN] = {0};
    bool positive_reply = false;
    // First fetch solved addr
    positive_reply = app_ble_get_peer_solved_addr(gap_zero_based_conidx(conn->con_idx), &ble_addr);
    positive_reply &= nv_record_ble_record_find_ltk(ble_addr.addr, ltk, ediv);
    // Just thread call to validate async call works well
    bt_thread_call_func_3(gap_reply_peer_ltk_request, bt_fixed_param(conn->connhdl),
                                                      bt_fixed_param(positive_reply == false),
                                                      bt_alloc_param_size(ltk, GAP_KEY_LEN));
}

static void app_ble_continue_recv_peer_ltk_req(void *priv, const gap_bond_sec_t *record)
{
    uint16_t connhdl = (uint16_t)((uint32_t)priv & 0xFFFF);
    uint16_t ediv = (uint16_t)(((uint32_t)priv & 0xFFFF0000) >> 16);
    gap_conn_item_t *conn = gap_get_conn_item(connhdl);

    /// TODO:connection may changed during resolving proc
    if (conn == NULL || record == NULL)
    {
        CO_LOG_ERR_2(BT_STS_RESOLVE_RPA_FAILED, connhdl, record);
        gap_reply_peer_ltk_request(connhdl, true, NULL);
        return;
    }

    // Update to new record
    conn->sec = *record;

    if (conn->sec.bonded_with_num_compare || conn->sec.bonded_with_passkey_entry || conn->sec.bonded_with_oob_method)
    {
        conn->conn_flag.auth_mitm_protection = true;
    }

    app_ble_reply_peer_ltk_request(conn, ediv);
}

static void app_ble_ltk_request_recv_handler(struct gap_conn_item_t *conn, const uint8_t *p_ediv)
{
    uint16_t ediv = CO_COMBINE_UINT16_LE(p_ediv);
    uint32_t ediv_connhdl = (conn->connhdl | (ediv << 16));
    // RPA need resolve using irk
    if (conn->peer_type == BT_ADDR_TYPE_RANDOM)
    {
        if (gap_resolve_rpa(&conn->peer_addr, app_ble_continue_recv_peer_ltk_req,
                                              (void *)ediv_connhdl) != BT_STS_PENDING)
        {
            gap_reply_peer_ltk_request(conn->connhdl, true, NULL);
        }

        return;
    }

    app_ble_reply_peer_ltk_request(conn, ediv);
}

void app_ble_enable_and_start_adv(void)
{
#if (BLE_AUDIO_ENABLED)
    aob_conn_start_adv(false, true, false);
#else
    ble_core_enable_stub_adv();
#endif
}

void app_ble_ready_and_init_done()
{
    uint8_t zero_irk[GAP_KEY_LEN] = {0};
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *dev_info = nv_record_blerec_get_ptr();

    CO_LOG_INFO_S_1(BT_STS_GLOBAL_STATUS, INTD, (sizeof(gatt_client_cache_t)<<16) |
#if BLE_AUDIO_ENABLED || defined(KEEP_BLE_AUDIO_IN_NV_RECORD)
        sizeof(GATTC_SRV_ATTR_t)
#else
        0x0000
#endif
        );

    // If local irk specifc by upper set is zero, use default
    if (memcmp(gap_get_local_irk(), zero_irk, sizeof(zero_irk)) == 0)
    {
        if (dev_info != NULL)
        {
            app_ble_set_local_irk(dev_info->self_info.ble_irk);
        }
    }

    app_ble_add_devices_info_to_resolving();

    gap_enable_address_resolution(true);

    app_ble_gap_update_local_database_hash();

#if (APP_BLE_DEMO_APP_ENABLED)
    bt_bdaddr_t test_addr = *gap_hci_bt_address();
    uint8_t test_irk[GAP_KEY_LEN] = {0};
    test_addr.address[5] = 0xFF;
    memcpy(test_irk, test_addr.address, sizeof(test_addr.address));
    /// Shall not using irk or ia that is same as nv section write for LEA using
    ble_demo_app_init(BLE_BASIC_ADV_HANDLE, &test_addr, test_irk);
#endif
}

static int app_ble_recv_stack_global_event(uintptr_t priv, gap_global_event_t event, gap_global_event_param_t param)
{
    switch (event)
    {
        case GAP_EVENT_RECV_STACK_READY:
        {
            app_ble_stack_ready();
            break;
        }
        case GAP_EVENT_REFRESH_ADVERTISING:
        {
            CO_LOG_INFO_S_1(BT_STS_REFRESH_ADV, RFSH, event);
            app_ble_refresh_adv_state_generic();
            break;
        }
        case GAP_EVENT_RECV_TX_POWER_LEVEL:
        {
            gap_le_tx_power_param_t *tx_power = param.tx_power;
            if (tx_power->type == GAP_LE_ADV_TX_POWER_LEVEL)
            {
                CO_LOG_INFO_S_2(BT_STS_GLOBAL_STATUS, ADPW,
                    tx_power->u.adv.adv_handle, tx_power->u.adv.curr_tx_power);
            }
            else
            {
                CO_LOG_INFO_S_2(BT_STS_GLOBAL_STATUS, PWRG,
                    tx_power->u.range.min_tx_power, tx_power->u.range.max_tx_power);
            }
            break;
        }
        case GAP_EVENT_RECV_DERIVED_BLE_LTK:
        {
            gap_recv_derived_ltk_t *p = param.recv_derived_ltk;

            CO_LOG_INFO_S_1(BT_STS_GLOBAL_STATUS, RLTK, p->ltk_generated_but_still_wait_peer_kdist);

            if (p->ltk_generated_but_still_wait_peer_kdist)
            {
                break;
            }

            app_ble_add_devices_info_to_resolving();

#if BLE_AUDIO_ENABLED
#ifdef IBRT // ble ltk derived from bredr, may prepare receive peer ble connection
            ble_global_t *g = ble_get_global();
            if (g->ble_smp_info_derived_from_bredr_complete != NULL)
            {
                ble_bdaddr_t ble_addr;
                ble_addr.addr_type = (p->peer_type & 0x01);
                memcpy(ble_addr.addr, &p->peer_addr, sizeof(bt_bdaddr_t));
                g->ble_smp_info_derived_from_bredr_complete(&ble_addr);
            }
#else
            app_ble_enable_and_start_adv();
#endif
#endif

            app_sysfreq_req(APP_SYSFREQ_USER_SMP_PAIRING, APP_SYSFREQ_32K);

            break;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

static void app_ble_connection_opened(gap_conn_item_t *p_conn)
{
    ble_global_t *g = ble_get_global();
    // Load gatt server cache
    app_ble_check_load_server_cache(p_conn);
    // Load gatt client cache
    app_ble_check_load_client_cache(p_conn);

    app_ble_reset_conn_param_mode(p_conn->con_idx);
    // Reset MTU size
    g->curr_mtu_size[gap_zero_based_conidx(p_conn->con_idx)] = L2CAP_LE_MIN_MTU;

#if BLE_GAP_CENTRAL_ROLE
    // Cancel all pending initiating in list
    if (app_ble_connection_count() + 1 >= BLE_CONNECTION_MAX)
    {
        gap_cancel_all_pend_initiating(app_is_arrive_at_max_ble_connections());
    }
#endif
}

static void app_ble_connection_encrypted(gap_conn_param_t *encrypted)
{
    if (app_ble_check_device_master_role())
    {
#if defined(ANCC_ENABLED)
        ble_ancc_start_discover(encrypted->connhdl);
#endif

#if defined(BLE_HID_HOST)
        ble_hid_host_start_discover(encrypted->connhdl);
#endif

#if defined (BLE_IAS_ENABLED)
        ble_iac_start_discover(encrypted->connhdl);
#endif
    }
}

static void app_ble_connection_paring_request(uint16_t connhdl)
{
    // Just thread call to validate async call works well
    bt_thread_call_func_3(gap_reply_peer_pairing_request, bt_fixed_param(connhdl),
                                                          bt_fixed_param(false), bt_fixed_param(0));

#if (BLE_AUDIO_ENABLED)
    ble_global_t *g = ble_get_global();
    // Slave earbuds clear db hash after first paring request
    if (bes_ble_audio_get_tws_nv_role() == BES_BLE_AUDIO_TWS_SLAVE)
    {
        TRACE(1, "Clear db hash after paring request");
        memset(g->local_database_hash, 0, 16);
    }
#endif /* BLE_AUDIO_ENABLED */
}

static int app_ble_conn_event_handle(uintptr_t connhdl, gap_conn_event_t event, gap_conn_callback_param_t param)
{
    ble_global_t *g = ble_get_global();
    ble_event_t cb_event;

    memset(&cb_event, 0, sizeof(ble_event_t));

    switch (event)
    {
        case GAP_CONN_EVENT_OPENED:
        {
            gap_conn_param_t *opened = param.opened;
            POSSIBLY_UNUSED const bt_bdaddr_t *la = &opened->conn->own_addr;
            POSSIBLY_UNUSED const bt_bdaddr_t *pa = &opened->peer_addr;

            CO_LOG_INFO_S_3(BT_STS_CONN_STATUS, CNNS,
                ((opened->con_idx) << 24) |
                ((opened->conn->conn_flag.is_central ? 0 : 1) << 16) | /* 0 central, 1 perpherial */
                ((opened->connhdl)),
                (la->address[0]<<24)|(la->address[1]<<16)|(la->address[5]<<8)|opened->own_addr_type,
                (pa->address[0]<<24)|(pa->address[1]<<16)|(pa->address[5]<<8)|opened->peer_type);

            app_ble_connection_opened(opened->conn);

            cb_event.evt_type = BLE_LINK_CONNECTED_EVENT;
            cb_event.p.connect_handled.conidx = gap_zero_based_conidx(opened->con_idx);
            cb_event.p.connect_handled.connhdl = opened->connhdl;
            cb_event.p.connect_handled.peer_bdaddr.addr_type = (opened->peer_type & 0x01);
            memcpy(cb_event.p.connect_handled.peer_bdaddr.addr, opened->peer_addr.address, sizeof(bt_bdaddr_t));
            app_ble_global_handle(&cb_event, NULL);

            app_ble_refresh_adv_state_generic();
            break;
        }
        case GAP_CONN_EVENT_FAILED:
        {
            gap_conn_failed_t *failed = param.conn_failed;

            CO_LOG_INFO_S_3(BT_STS_CONN_STATUS, CNNF,
                (failed->error_code<<24) |
                (failed->role << 16) | /* 0 central, 1 perpherial */
                (failed->peer_type),
                CO_COMBINE_UINT32_BE(failed->peer_addr.address),
                CO_COMBINE_UINT16_BE(failed->peer_addr.address+4));

            if (failed->role == 0) // central
            {
                cb_event.evt_type = BLE_CONNECTING_STOPPED_EVENT;
                cb_event.p.stopped_connecting_handled.peer_type = (failed->peer_type & 0x01);
                memcpy(cb_event.p.stopped_connecting_handled.peer_bdaddr, failed->peer_addr.address, sizeof(bt_bdaddr_t));
                app_ble_global_handle(&cb_event, NULL);
            }
            break;
        }
        case GAP_CONN_EVENT_CLOSED:
        {
            gap_conn_param_t *closed = param.closed;

            CO_LOG_INFO_S_2(BT_STS_CONN_STATUS, CNNE,
                ((closed->con_idx) << 24) |
                ((closed->conn->conn_flag.is_central ? 0 : 1) << 16) | /* 0 central, 1 perpherial */
                ((closed->connhdl)),
                closed->error_code);

            app_ble_reset_conn_param_mode(closed->con_idx);

            cb_event.evt_type = BLE_DISCONNECT_EVENT;
            cb_event.p.disconnect_handled.errCode = closed->error_code;
            cb_event.p.disconnect_handled.conidx = gap_zero_based_conidx(closed->con_idx);
            cb_event.p.disconnect_handled.connhdl = closed->connhdl;
            cb_event.p.disconnect_handled.peer_bdaddr.addr_type = (closed->peer_type & 0x01);
            memcpy(cb_event.p.disconnect_handled.peer_bdaddr.addr, closed->peer_addr.address, sizeof(bt_bdaddr_t));
            app_ble_global_handle(&cb_event, NULL);
            break;
        }
        case GAP_CONN_EVENT_CACHE:
        {
            gap_conn_cache_ind_t *p = param.conn_cache;

            CO_LOG_INFO_S_3(BT_STS_SUCCESS, CACH,
                ((p->con_idx) << 24) |
                ((p->conn->conn_flag.is_central ? 0 : 1) << 16) | /* 0 central, 1 perpherial */
                ((p->connhdl)),
                p->cli_cache, p->srv_cache);

            if (p->cli_cache)
            {
                app_ble_gatt_client_cache(p->conn, p->cli_cache);
            }

            if (p->srv_cache)
            {
                app_ble_gatt_server_cache(p->conn, p->srv_cache);
            }
            break;
        }
        case GAP_CONN_EVENT_MTU_EXCHANGED:
        {
            gap_conn_mtu_exchanged_t *p = param.mtu_exchanged;

            CO_LOG_INFO_S_2(BT_STS_CONN_STATUS, MTUE,
                ((p->con_idx) << 24) |
                ((p->conn->conn_flag.is_central ? 0 : 1) << 16) | /* 0 central, 1 perpherial */
                ((p->connhdl)),
                p->mtu);
            // Update mtu size
            g->curr_mtu_size[gap_zero_based_conidx(p->con_idx)] = p->mtu;
#if (mHDT_LE_SUPPORT)
            app_ble_mhdt_hci_le_rd_remote_proprietary_feat_cmd(p->con_idx);
#endif
            cb_event.evt_type = BLE_MTU_EXECHANGE_EVENT;
            cb_event.p.mtu_exec_handled.conidx = gap_zero_based_conidx(p->con_idx);
            cb_event.p.mtu_exec_handled.mtu = p->mtu;
            app_ble_global_handle(&cb_event, NULL);
            break;
        }
        case GAP_CONN_EVENT_USER_CONFIRM:
        {
            gap_user_confirm_t *confirm = param.user_confirm;

            CO_LOG_INFO_S_2(BT_STS_CONN_STATUS, USRC,
                ((confirm->con_idx) << 24) |
                ((confirm->conn->conn_flag.is_central ? 0 : 1) << 16) | /* 0 central, 1 perpherial */
                ((confirm->connhdl)),
                confirm->type);

            if (confirm->type == GAP_USER_NUMERIC_CONFIRM)
            {
#if defined(BLE_SEC_ACCEPT_BY_CUSTOMER) || defined (GFPS_ENABLED)
                cb_event.evt_type = BLE_CONNECT_NC_EXCH_EVENT;
                cb_event.p.connect_nc_exch_handled.conidx = gap_zero_based_conidx(confirm->con_idx);
                cb_event.p.connect_nc_exch_handled.connhdl = confirm->connhdl;
                cb_event.p.connect_nc_exch_handled.confirm_value = confirm->data.numeric_confirm_value;
                app_ble_global_handle(&cb_event, NULL);
#ifdef GFPS_ENABLED
                gap_input_numeric_confirm(confirm->connhdl, NULL, true);
#endif
#else
                gap_input_numeric_confirm(confirm->connhdl, NULL, true);
#endif
            }
            else if (confirm->type == GAP_USER_LTK_REQUEST_CONFIRM)
            {
                app_ble_ltk_request_recv_handler(confirm->conn, confirm->data.ltk_req.ediv);
            }
            break;
        }
        case GAP_CONN_EVENT_ENCRYPTED:
        {
            gap_conn_param_t *encrpt = param.encrypted;
            gap_conn_item_t *conn = encrpt->conn;
            uint8_t pairing_lvl = 0;

            CO_LOG_INFO_S_2(BT_STS_CONN_STATUS, ENCT,
                ((encrpt->con_idx) << 24) |
                ((encrpt->conn->conn_flag.is_central ? 0 : 1) << 16) | /* 0 central, 1 perpherial */
                ((encrpt->connhdl)),
                encrpt->error_code);

            if (encrpt->error_code)
            {
                if (encrpt->error_code == BES_ERROR_TERMINATED_MIC_FAILURE)
                {
                    bluetooth_nv_mgr_ble_record_del(BLE_NV_REC_DEL_LE_MIC_FAILURE, (uint8_t *)&conn->sec.peer_addr);
                }

                // If pairing failed due to key missing
                if (SMP_ERROR_ENABLE_ENC_FAILED == encrpt->error_code && conn->conn_flag.is_central == true)
                {
                    bluetooth_nv_mgr_ble_record_del(BLE_NV_REC_DEL_LE_ENC_FAILURE, (uint8_t *)&conn->sec.peer_addr);
                    // Start Authentication again to trigger SMP
                    bt_thread_call_func_2(gap_start_authentication, bt_fixed_param(conn->connhdl),
                                                                    bt_fixed_param(GAP_AUTH_STARTED_BY_UPPER_APP));
                }

                cb_event.evt_type = BLE_CONNECT_BOND_FAIL_EVENT;
                cb_event.p.connect_bond_handled.conidx = gap_zero_based_conidx(encrpt->con_idx);
                cb_event.p.connect_bond_handled.success = false;
                cb_event.p.connect_bond_handled.reason = encrpt->error_code;
                app_ble_global_handle(&cb_event, NULL);
            }
            else
            {
                app_ble_connection_encrypted(encrpt);

                app_ble_set_phy_mode(encrpt->con_idx, g->default_tx_pref_phy_bits, g->default_rx_pref_phy_bits, GAP_CODED_PHY_NO_PREFER_CODING);

                cb_event.evt_type = BLE_CONNECT_BOND_EVENT;
                cb_event.p.connect_bond_handled.conidx = gap_zero_based_conidx(encrpt->con_idx);
                cb_event.p.connect_bond_handled.success = true;
                cb_event.p.connect_bond_handled.reason = 0;
                // Only report bond event when in pairing
                if (conn->conn_flag.smp_pairing_ongoing != 0)
                {
                    app_ble_global_handle(&cb_event, NULL);
                }

                cb_event.evt_type = BLE_CONNECT_ENCRYPT_EVENT;
                cb_event.p.connect_encrypt_handled.conidx = gap_zero_based_conidx(encrpt->con_idx);
                cb_event.p.connect_encrypt_handled.addr_type = (encrpt->peer_type & 0x01);
                memcpy(cb_event.p.connect_encrypt_handled.addr, encrpt->peer_addr.address, sizeof(bt_bdaddr_t));
                if (conn->sec.device_bonded)
                {
                    if (conn->sec.secure_pairing)
                    {
                        pairing_lvl = GAP_PAIRING_BOND_SECURE_CON;
                    }
                    else
                    {
                        pairing_lvl = conn->conn_flag.auth_mitm_protection ?
                            GAP_PAIRING_BOND_AUTH : GAP_PAIRING_BOND_UNAUTH;
                    }
                }
                else
                {
                    if (conn->sec.secure_pairing)
                    {
                        pairing_lvl = GAP_PAIRING_SECURE_CON;
                    }
                    else
                    {
                        pairing_lvl = conn->conn_flag.auth_mitm_protection ?
                            GAP_PAIRING_AUTH : GAP_PAIRING_UNAUTH;
                    }
                }
                cb_event.p.connect_encrypt_handled.pairing_lvl = pairing_lvl;
                app_ble_global_handle(&cb_event, NULL);
            }

            app_sysfreq_req(APP_SYSFREQ_USER_SMP_PAIRING, APP_SYSFREQ_32K);
            break;
        }
        case GAP_CONN_EVENT_RECV_KEY_DIST:
        {
            gap_recv_key_dist_t *key_dist = param.recv_key_dist;

            CO_LOG_INFO_S_2(BT_STS_CONN_STATUS, KEYD,
                ((key_dist->con_idx) << 24) |
                ((key_dist->conn->conn_flag.is_central ? 0 : 1) << 16) | /* 0 central, 1 perpherial */
                ((key_dist->connhdl)),
                key_dist->key_type);

            if (key_dist->key_type == GAP_RECV_PEER_IRK_IDENTITY)
            {
                app_ble_add_devices_info_to_resolving();
            }
            else if (key_dist->key_type == GAP_RECV_DERIVED_BT_LINK_KEY)
            {
                gap_conn_item_t *conn = key_dist->conn;
                btif_device_record_t record = {{{0}},0};
                nv_record_ddbrec_find((const bt_bdaddr_t *)&key_dist->recv_ia_addr, &record);

                if (key_dist->link_key_generated_but_still_wait_peer_ia)
                {
                    break;
                }

                record.trusted = true;
                record.pinLen = key_dist->enc_key_size;
                record.bdAddr = key_dist->recv_ia_addr;
                memcpy(record.linkKey, key_dist->recv_key, GAP_KEY_LEN);

                if (conn->sec.bonded_with_num_compare ||
                    conn->sec.bonded_with_passkey_entry ||
                    conn->sec.bonded_with_oob_method)
                {
                    record.keyType = BTIF_AUTH_SC_COMBINATION_KEY;
                }
                else
                {
                    record.keyType = BTIF_UNAUTH_SC_COMBINATION_KEY;
                }

                bluetooth_nv_mgr_bt_record_add(BT_NV_REC_ADD_CTKD_OVER_LE, &record);
            }
            break;
        }
        case GAP_CONN_EVENT_UPDATE_REQ:
        {
            gap_conn_update_req_t *update_req = param.update_req;

            CO_LOG_INFO_S_3(BT_STS_CONN_STATUS, UPRQ,
                ((update_req->con_idx) << 24) |
                ((update_req->conn->conn_flag.is_central ? 0 : 1) << 16) | /* 0 central, 1 perpherial */
                ((update_req->connhdl)),
                (update_req->params_req.conn_interval_min_1_25ms << 16) |
                (update_req->params_req.conn_interval_max_1_25ms),
                (update_req->params_req.max_peripheral_latency << 16) |
                (update_req->params_req.superv_timeout_ms));

            app_ble_conn_param_update_req(update_req);
            break;
        }
        case GAP_CONN_EVENT_PARAMS_UPDATE:
        {
            gap_conn_param_t *update = param.params_update;
            gap_conn_item_t *conn = update->conn;

            CO_LOG_INFO_S_3(BT_STS_CONN_STATUS, UPED,
                ((update->con_idx) << 24) |
                ((update->conn->conn_flag.is_central ? 0 : 1) << 16) | /* 0 central, 1 perpherial */
                ((update->connhdl)),
                (conn->timing.subrate_factor << 16) |
                (conn->timing.conn_interval_1_25ms),
                (conn->timing.peripheral_latency << 16) |
                (conn->timing.superv_timeout_ms));

            app_ble_conn_param_update_cmpl(conn, update->error_code);
            break;
        }
        case GAP_CONN_EVENT_SUBRATE_CHANGE:
        {
            gap_conn_param_t *change = param.subrate_change;
            gap_conn_item_t *conn = change->conn;

            CO_LOG_INFO_S_3(BT_STS_CONN_STATUS, SBRC,
                ((change->con_idx) << 24) |
                ((change->conn->conn_flag.is_central ? 0 : 1) << 16) | /* 0 central, 1 perpherial */
                ((change->connhdl)),
                (conn->timing.subrate_factor << 16) |
                (conn->timing.conn_continuation_number),
                (conn->timing.peripheral_latency << 16) |
                (conn->timing.superv_timeout_ms));

            cb_event.evt_type = BLE_SUBRATE_CHANGE_EVENT;
            cb_event.p.subrate_change_handled.conidx = gap_zero_based_conidx(change->con_idx);
            cb_event.p.subrate_change_handled.sub_factor = conn->timing.subrate_factor;
            cb_event.p.subrate_change_handled.per_latency = conn->timing.peripheral_latency;
            cb_event.p.subrate_change_handled.cont_num = conn->timing.conn_continuation_number;
            cb_event.p.subrate_change_handled.timeout = conn->timing.superv_timeout_ms / 10;
            app_ble_global_handle(&cb_event, NULL);
            break;
        }
        case GAP_CONN_EVENT_PHY_UPDATE:
        {
            POSSIBLY_UNUSED gap_conn_phy_update_t *phy_update = param.phy_update;
            CO_LOG_INFO_S_3(BT_STS_CONN_STATUS, PHYU,
                ((phy_update->con_idx) << 24) |
                ((phy_update->conn->conn_flag.is_central ? 0 : 1) << 16) | /* 0 central, 1 perpherial */
                ((phy_update->connhdl)),
                phy_update->tx_phy,
                phy_update->rx_phy);
            break;
        }
        case GAP_CONN_EVENT_TX_POWER_REPORT:
        {
            gap_le_tx_power_report_t *tx_power = param.tx_power;
            POSSIBLY_UNUSED gap_le_tx_power_param_t *temp_param = &tx_power->param;
            CO_LOG_INFO_S_3(BT_STS_CONN_STATUS, TXPW,
                ((tx_power->con_idx) << 24) |
                ((tx_power->conn->conn_flag.is_central ? 0 : 1) << 16) | /* 0 central, 1 perpherial */
                ((tx_power->connhdl)),
                (temp_param->type << 24) | (temp_param->u.remote.phy << 16) |
                (temp_param->u.remote.is_at_min_level << 12) | (temp_param->u.remote.is_at_max_level << 8) |
                (temp_param->u.remote.delta),
                temp_param->u.remote.curr_tx_power);
            break;
        }
        case GAP_CONN_EVENT_RECV_KEY_METERIAL:
        {
            gap_recv_key_material_t *key = param.recv_key_material;
            if (key->error_code == BT_STS_SUCCESS)
            {
            }
            break;
        }
        case GAP_CONN_EVENT_RECV_SEC_REQUEST:
        {
            gap_recv_sec_request_t *sec_req = param.recv_security_req;

            CO_LOG_INFO_S_1(BT_STS_SMP_AUTHENCIATION, SECR, (sec_req->connhdl << 8 | sec_req->auth_req));

            gap_start_authentication(sec_req->connhdl, GAP_AUTH_STARTED_BY_UPPER_APP);
            break;
        }
        case GAP_CONN_EVENT_RECV_SMP_REQUIRE:
        {
            gap_recv_smp_requirements_t *smp_req = param.smp_requirements;

            CO_LOG_INFO_S_1(BT_STS_SMP_AUTHENCIATION, PARQ, (smp_req->connhdl << 8 | smp_req->pairing_req_or_rsp));

            if (smp_req->pairing_req_or_rsp == true)
            {
                app_ble_connection_paring_request(smp_req->connhdl);
            }
            break;
        }
        case GAP_CONN_EVENT_PAIRING_COMPLETE:
        {
            gap_smp_pairing_cmp_t *pairing_cmp = param.smp_pairing_cmp;

            CO_LOG_INFO_S_1(pairing_cmp->err_code, PACM, pairing_cmp->connhdl);

            cb_event.evt_type = BLE_SMP_PAIRING_CMP_EVENT;
            cb_event.p.pair_cmp_handled.conidx = pairing_cmp->con_idx;
            memcpy(cb_event.p.pair_cmp_handled.addr, 
                   (pairing_cmp->conn->sec.peer_addr.address), BTIF_BD_ADDR_SIZE);
            app_ble_global_handle(&cb_event, NULL);
            break;
        }
        case GAP_CONN_EVENT_FRAME_SPACE_UPDATE:
        {
            POSSIBLY_UNUSED gap_frame_space_update_t *fs_update = param.frame_space_update;

            CO_LOG_INFO_S_3(fs_update->error_code, FSUP, fs_update->connhdl,
                            (fs_update->initiator << 24 |
                             fs_update->spacing_types_bit << 8 |
                             fs_update->phy_bit),
                            fs_update->frame_space_us);
            break;
        }
        default:
        {
            break;
        }
    }

    return 0;
}

#if BLE_GAP_CENTRAL_ROLE
static int app_ble_client_callback(uintptr_t connhdl, gap_init_event_t event, gap_init_callback_param_t param)
{
    if (event < GAP_INIT_EVENT_CONN_OPENED)
    {
        ble_event_t cb_event;

        memset(&cb_event, 0, sizeof(ble_event_t));

        if (event == GAP_INIT_EVENT_SCAN_STARTED)
        {
            CO_LOG_INFO_S_0(BT_STS_SCAN_STATUS, SCNS);
            cb_event.evt_type = BLE_SCAN_STARTED_EVENT;
            app_ble_global_handle(&cb_event, NULL);
        }
        else if (event == GAP_INIT_EVENT_SCAN_STOPPED)
        {
            CO_LOG_INFO_S_0(BT_STS_SCAN_STATUS, SCNE);
            cb_event.evt_type = BLE_SCAN_STOPPED_EVENT;
            app_ble_global_handle(&cb_event, NULL);
        }
        else if (event == GAP_INIT_EVENT_SCAN_ADV_REPORT)
        {
            const gap_adv_report_t *adv_report = param.adv_report;
            uint16_t data_len = adv_report->data_length;
            cb_event.evt_type = BLE_SCAN_DATA_REPORT_EVENT;
            cb_event.p.scan_data_report_handled.trans_addr.addr_type = adv_report->peer_type;
            memcpy(cb_event.p.scan_data_report_handled.trans_addr.addr, &adv_report->peer_addr, sizeof(bt_bdaddr_t));
            cb_event.p.scan_data_report_handled.rssi = adv_report->rssi;
            if (data_len > EXT_ADV_DATA_LEN)
            {
                data_len = EXT_ADV_DATA_LEN;
            }
            cb_event.p.scan_data_report_handled.length = data_len;
            cb_event.p.scan_data_report_handled.adv = adv_report;
            cb_event.p.scan_data_report_handled.data = adv_report->data;
            if (adv_report->adv.scan_rsp)
            {
                cb_event.p.scan_data_report_handled.info = adv_report->adv.extended_adv ?
                    GAPM_REPORT_TYPE_SCAN_RSP_EXT : GAPM_REPORT_TYPE_SCAN_RSP_LEG;
            }
            else
            {
                cb_event.p.scan_data_report_handled.info = adv_report->adv.extended_adv ?
                    GAPM_REPORT_TYPE_ADV_EXT : GAPM_REPORT_TYPE_ADV_LEG;
            }
            if (adv_report->adv.cmpl_adv_data)
            {
                cb_event.p.scan_data_report_handled.info |= GAPM_REPORT_INFO_COMPLETE_BIT;
            }
            if (adv_report->adv.connectable)
            {
                cb_event.p.scan_data_report_handled.info |= GAPM_REPORT_INFO_CONN_ADV_BIT;
            }
            if (adv_report->adv.scannable)
            {
                cb_event.p.scan_data_report_handled.info |= GAPM_REPORT_INFO_SCAN_ADV_BIT;
            }
            if (adv_report->adv.directed)
            {
                cb_event.p.scan_data_report_handled.info |= GAPM_REPORT_INFO_DIR_ADV_BIT;
            }
            app_ble_global_handle(&cb_event, NULL);
        }
        else if (event == GAP_INIT_EVENT_INIT_STARTED)
        {
            CO_LOG_INFO_S_0(BT_STS_INIT_STATUS, INTS);
        }
        else if (event == GAP_INIT_EVENT_INIT_STOPPED)
        {
            CO_LOG_INFO_S_0(BT_STS_INIT_STATUS, INTE);
            gap_filter_list_remove_user_item(BLE_WHITE_LIST_USER_CENTRAL);
        }
    }
    else
    {
        app_ble_conn_event_handle(connhdl, (gap_conn_event_t)event, (gap_conn_callback_param_t){param.param_ptr});
    }

    return 0;
}
#endif /* BLE_GAP_CENTRAL_ROLE */

static uint16_t app_ble_impl_start_connection(const ble_bdaddr_t *peer_addr_or_list, uint32_t ListSize_IaRpaNpa, uint32_t connect_to_ms, bool is_ble_audio)
{
#if BLE_GAP_CENTRAL_ROLE
    uint16_t status = BT_STS_SUCCESS;
    uint16_t list_size = (uint16_t)(ListSize_IaRpaNpa >> 16);
    uint8_t ia_rpa_npa = (uint8_t)(ListSize_IaRpaNpa & 0xff);
    bt_addr_type_t own_addr_type = app_ble_get_own_addr_type(ia_rpa_npa);
    const ble_bdaddr_t *peer_addr = peer_addr_or_list;

    if (app_is_arrive_at_max_ble_connections() ||
            (list_size + app_ble_connection_count() > BLE_CONNECTION_MAX))
    {
        CO_LOG_ERR_2(BT_STS_REACH_MAX_NUMBER, list_size, app_ble_connection_count());
        return BT_STS_NOT_ALLOW;
    }

    gap_init_param_t init_param =
    {
        .own_addr_type = own_addr_type,
        // Default using 1M for scan and estb, 2M for estb
        .initiating_phys = (GAP_PHY_BIT_LE_1M | GAP_PHY_BIT_LE_2M),
        .has_custom_scan_timing = true,
        .has_custom_init_timing = true,
        .scan_timing = gap_default_scan_timing(),
        .init_timing = gap_default_init_timing(),
    };

    init_param.init_timing.init_proc_timeout_ms = connect_to_ms;

    if (list_size)
    {
        init_param.filter_policy = GAP_INIT_FILTER_POLICY_USE_FILTER_TO_ALL_PDUS;
        status = gap_filter_list_add_user_item(BLE_WHITE_LIST_USER_CENTRAL, peer_addr, list_size);
    }
    else
    {
        init_param.peer_type = (bt_addr_type_t)peer_addr->addr_type;
        init_param.peer_addr = *(bt_bdaddr_t *)peer_addr->addr;
    }

    if (status == BT_STS_SUCCESS)
    {
        status = gap_start_ext_initiating(&init_param, is_ble_audio, app_ble_client_callback);
    }

    return status;
#else
    return BT_STS_NOT_SUPPORTED;
#endif /*BLE_GAP_CENTRAL_ROLE*/
}

bt_status_t app_ble_start_auto_connect(const ble_bdaddr_t *addr_list, uint16_t list_size, uint8_t ia_rpa_npa, uint32_t conn_to_ms)
{
    return app_ble_impl_start_connection(addr_list, ((list_size)<<16)|((ia_rpa_npa)&0xff), conn_to_ms, false);
}

bt_status_t app_ble_start_connect(const ble_bdaddr_t *peer_addr, uint8_t ia_rpa_npa)
{
    return app_ble_impl_start_connection(peer_addr, ia_rpa_npa, 0, false);
}

bt_status_t app_ble_connect_ble_audio_device(const ble_bdaddr_t *peer_addr, uint8_t ia_rpa_npa, uint32_t conn_to_ms)
{
    return app_ble_impl_start_connection(peer_addr, ia_rpa_npa, conn_to_ms, true);
}

void app_ble_cancel_connecting(void)
{
    gap_cancel_initiating();
}

void app_ble_disconnect(uint16_t connhdl)
{
    gap_terminate_connection(connhdl, 0);
}

void app_ble_disconnect_all(void)
{
    gap_terminate_all_ble_connection();
}

void app_ble_start_disconnect(uint8_t conidx)
{
    app_ble_disconnect(gap_zero_based_ble_conidx_as_hdl(conidx));
}

static void app_ble_impl_start_scanning(BLE_SCAN_PARAM_T *scan_param)
{
#if BLE_GAP_CENTRAL_ROLE
    bt_addr_type_t own_addr_type = scan_param->ownAddrType;
    gap_scan_param_t param = {0};

    bool scan_use_rpa = (scan_param->scanFolicyType) >= 2;
    bool use_filter_list = (scan_param->scanFolicyType)&0x1;

    own_addr_type = app_ble_get_own_addr_type(scan_use_rpa ? APP_GAPM_GEN_RSLV_ADDR : APP_GAPM_STATIC_ADDR);

    param.filter_policy = use_filter_list ? GAP_SCAN_FILTER_POLICY_EXT_FILTERED : GAP_SCAN_FILTER_POLICY_EXT_UNFILTERED;
    param.active_scan = (scan_param->scanType&0x01 || scan_param->scanType_coded&0x01);
    param.filter_duplicated = scan_param->filter_duplicates;
    param.own_addr_type = own_addr_type;
    // Use scan timing from custom
    param.has_custom_scan_timing = true;
    // Use custom adv scan timing in fg scan
    param.scan_timing.fg_scan_window_ms = scan_param->scanWindowMs;
    param.scan_timing.fg_scan_window_coded_ms = scan_param->scanWindowMs_coded;
    param.scan_timing.fg_scan_interval_ms = scan_param->scanIntervalMs;
    param.scan_timing.fg_scan_interval_coded_ms = scan_param->scanIntervalMs_coded;
    param.scan_timing.fg_scan_time_ms = scan_param->scanDurationMs;
    // This api use FG scan by default

    param.phys = scan_param->phys;
    param.legacy = scan_param->legacy;
    if(scan_param->scan_callback)
    {
        gap_start_scanning(&param, scan_param->scan_callback);
    }
    else
    {
        gap_start_scanning(&param, (app_ble_scan_callback_t)app_ble_client_callback);
    }
#endif /*BLE_GAP_CENTRAL_ROLE*/
}

void app_ble_start_scan(BLE_SCAN_PARAM_T *param)
{
    app_ble_impl_start_scanning(param);
}

void app_ble_stop_scan(void)
{
#if BLE_GAP_CENTRAL_ROLE
    gap_disable_scanning();
#endif
}

static void app_ble_adv_started(ble_adv_activity_t *p_actv)
{
    p_actv->adv_is_started = true;
    // Read RPA addr when adv is started
    if (p_actv->adv_param.own_addr_type >= BT_ADDR_TYPE_PUB_IA)
    {
        app_ble_read_local_rpa_addr_impl(p_actv);
    }
    else
    {
        memset(&p_actv->local_rpa, 0, sizeof(p_actv->local_rpa));
    }
}

int app_ble_server_callback(uintptr_t connhdl, gap_adv_event_t event, gap_adv_callback_param_t param)
{
    if (event < GAP_ADV_EVENT_CONN_OPENED)
    {
        ble_adv_activity_t *adv = NULL;
        uint8_t adv_handle = (uint8_t)connhdl;
        ble_event_t cb_event;

        memset(&cb_event, 0, sizeof(ble_event_t));

        adv = app_ble_find_adv_activity(adv_handle);
        if (adv == NULL)
        {
            CO_LOG_ERR_2(BT_STS_INVALID_ADV_HANDLE, adv_handle, event);
            return 0;
        }
        if (event == GAP_ADV_EVENT_STARTED)
        {
            CO_LOG_INFO_S_1(BT_STS_ADV_STATUS, ADVS, adv_handle);

            app_ble_adv_started(adv);

            cb_event.evt_type = BLE_ADV_STARTED_EVENT;
            cb_event.p.adv_started_handled.actv_user = adv->user;
            app_ble_global_handle(&cb_event, NULL);
#if defined(BLE_MESH_ENABLE)
            bt_mesh_adv_callback(MESH_ADV_STARTED_EVT, adv_handle);
#endif
        }
        else if (event == GAP_ADV_EVENT_STOPPED)
        {
            CO_LOG_INFO_S_1(BT_STS_ADV_STATUS, ADVT, adv_handle);
            adv->adv_is_started = false;
            cb_event.evt_type = BLE_ADV_STOPPED_EVENT;
            cb_event.p.adv_stopped_handled.actv_user = adv->user;
            app_ble_global_handle(&cb_event, NULL);
#if defined(BLE_MESH_ENABLE)
            bt_mesh_adv_callback(MESH_ADV_STOPPED_EVT, adv_handle);
#endif
        }
    }
    else
    {
        if (event == GAP_ADV_EVENT_CONN_OPENED)
        {
            app_stop_fast_connectable_ble_adv_timer();
        }
        else if (event == GAP_ADV_EVENT_CONN_FAILED || event == GAP_ADV_EVENT_CONN_CLOSED)
        {
            CO_LOG_INFO_S_1(BT_STS_ADV_STATUS, RFSH, event);
            app_ble_refresh_adv_state_generic();
        }

        app_ble_conn_event_handle(connhdl, (gap_conn_event_t)event, (gap_conn_callback_param_t){param.param_ptr});
    }

    return 0;
}

POSSIBLY_UNUSED static bool app_ble_get_find_any_irk_distributed_record(ble_bdaddr_t *p_record_addr)
{
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *dev_info = nv_record_blerec_get_ptr();
    BleDeviceinfo *record = NULL;
    uint32_t count = dev_info->saved_list_num;
    int i = 0;

    for (; i < count; i++)
    {
        record = dev_info->ble_nv + i;
        if (record->pairingInfo.bond_info_bf & CO_BIT_MASK(BONDED_WITH_IRK_DISTRIBUTED))
        {
            if (p_record_addr)
            {
                *p_record_addr = *(ble_bdaddr_t *)&record->pairingInfo.peer_addr;
            }

            return true;
        }
    }

    return false;
}

static void app_ble_refresh_advertising(uint8_t adv_handle, gap_adv_param_t *adv_param)
{
    bt_bdaddr_t zero_addr = {{0}};
    POSSIBLY_UNUSED ble_bdaddr_t bonded_device_addr = {0};

    //------------------------------own addr type---------------------------
    APP_GAPM_OWN_ADDR_E addr_type = adv_param->own_addr_use_rpa ? APP_GAPM_GEN_RSLV_ADDR : APP_GAPM_STATIC_ADDR;

    adv_param->own_addr_type = app_ble_get_own_addr_type(addr_type);
    if (addr_type == APP_GAPM_GEN_RSLV_ADDR && adv_param->use_custom_local_addr)
    {
        if (memcmp(&adv_param->custom_local_addr, &zero_addr, sizeof(bt_bdaddr_t)) != 0)
        {
            adv_param->own_addr_type = BT_ADDR_TYPE_RANDOM;
        }
    }

    //----------------------------peer type/addr ---------------------------
    /*
     * Check rpa addr needed whenever peer addr not set, if peer addr is set:
     *
     * 1.Directed adv use peer addr to ensure target AdvA fill with peer addr, and
     *
     * 2.Adv use peer addr to specific resolving list item irk to generate rpa:
     *   If Own_Address_Type equals 0x02 or 0x03, the Peer_Address parameter
     *   contains the peer??s Identity Address and the Peer_Address_Type parameter
     *   contains the peer??s Identity Type (i.e., 0x00 or 0x01). These parameters are
     *   used to locate the corresponding local IRK in the resolving list; this IRK is used
     *   to generate their own address used in the advertisement.
     *
     */
    do
    {
        // Directed adv can not use fake rpa because should specifc peer addr and can not change
        if (adv_param->directed_adv == true)
        {
            break;
        }
        // Public or random addr use 01 72 fc or 01 35 20 to specific own addr, not use peer addr
        if (adv_param->own_addr_type < BT_ADDR_TYPE_PUB_IA)
        {
            break;
        }
        // If peer addr is not specifified and want to use fake rpa
        if (memcmp(&adv_param->peer_addr, &zero_addr, sizeof(bt_bdaddr_t)) == 0 &&
                adv_param->use_fake_btc_rpa_when_no_irk_exist == true)
        {
#if defined (BLE_ADV_RPA_ENABLED)
            // Fake rpa list item added, so use this to generate rpa
            adv_param->peer_type = BT_ADDR_TYPE_PUBLIC;
            adv_param->peer_addr = *(bt_bdaddr_t *)bt_get_ble_local_address();
#else
            if (app_ble_get_find_any_irk_distributed_record(&bonded_device_addr))
            {
                adv_param->peer_type = bonded_device_addr.addr_type;
                adv_param->peer_addr = *(bt_bdaddr_t *)bonded_device_addr.addr;
            }
            else
            {
                // No fake resolving list item and no bonded devices, fake rpa is not support
                CO_LOG_ERR_1(BT_STS_INVALID_PARM, adv_handle);
                adv_param->own_addr_type = (adv_param->own_addr_type & 0x01); // use public or random due to no item exist
            }
#endif /* BLE_ADV_RPA_ENABLED */
        }
    } while (0);

    gap_refresh_advertising(adv_handle, adv_param, app_ble_server_callback);
}

bool app_ble_is_in_advertising_state(void)
{
    ble_global_t *g = ble_get_global();
    ble_adv_activity_t *adv = NULL;
    int max_size = ARRAY_SIZE(g->adv);
    int i = 0;

    for (i = 0; i < max_size; i += 1)
    {
        adv = g->adv + i;
        if (adv->adv_is_started)
        {
            return true;
        }
    }

    return false;
}

ble_adv_activity_t *app_ble_get_advertising(uint8_t adv_handle)
{
    ble_global_t *g = ble_get_global();
    ble_adv_activity_t *adv = NULL;
    int max_size = ARRAY_SIZE(g->adv);
    int i = 0;

    if (adv_handle == GAP_INVALID_ADV_HANDLE)
    {
        return NULL;
    }

    for (i = 0; i < max_size; i += 1)
    {
        adv = g->adv + i;
        if (adv->adv_handle == adv_handle)
        {
            return adv;
        }
    }

    return NULL;
}

ble_adv_activity_t *app_ble_register_advertising(uint8_t adv_handle, app_ble_adv_activity_func adv_activity_func)
{
    ble_global_t *g = ble_get_global();
    ble_adv_activity_t *adv = NULL;
    ble_adv_activity_t *found = NULL;
    int max_size = ARRAY_SIZE(g->adv);
    int i = 0;

    if (adv_activity_func == NULL)
    {
        CO_LOG_ERR_0(BT_STS_INVALID_PARAMS);
        return NULL;
    }

    if (adv_handle >= BLE_MAX_FIXED_ADV_HANDLE)
    {
        CO_LOG_ERR_1(BT_STS_INVALID_ADV_HANDLE, adv_handle);
        return NULL;
    }

    adv = app_ble_get_advertising(adv_handle);
    if (adv)
    {
        adv->adv_activity_func = adv_activity_func;
        found = adv;
        goto label_found_and_return;
    }

    for (i = 0; i < max_size; i += 1)
    {
        adv = g->adv + i;
        if (adv->adv_handle == GAP_INVALID_ADV_HANDLE)
        {
            adv->adv_activity_func = adv_activity_func;
            adv->adv_handle = adv_handle;
            found = adv;
            break;
        }
    }

label_found_and_return:
    if (found)
    {
        CO_LOG_INFO_1(adv_handle, adv_activity_func);
        app_ble_clean_adv_param(&adv->adv_param);
        return found;
    }

    return NULL;
}

ble_adv_activity_t *app_ble_get_advertising_by_user(BLE_ADV_USER_E user)
{
    ble_global_t *g = ble_get_global();
    ble_adv_activity_t *adv = NULL;
    int max_size = ARRAY_SIZE(g->adv);
    int i = 0;

    for (i = 0; i < max_size; i += 1)
    {
        adv = g->adv + i;
        if (adv->adv_activity_func && adv->user == user)
        {
            return adv;
        }
    }

    return NULL;
}

void app_ble_param_set_adv_interval(BLE_ADV_INTERVALREQ_USER_E adv_intv_user,
                                    BLE_ADV_USER_E adv_user, uint32_t interval_ms)
{
    ble_adv_activity_t *adv = app_ble_get_advertising_by_user(adv_user);

    if (adv && (adv->custom_adv_interval_ms != interval_ms))
    {
        adv->custom_adv_interval_ms = interval_ms;
        // Adv is started and need to refresh its param
        if (adv->adv_is_started == true)
        {
            app_ble_refresh_adv_state_generic();
        }
    }
}

static void app_ble_set_adv_interval(ble_adv_activity_t *adv)
{   
    if (adv->custom_adv_interval_ms == 0)
    {
        return;
    }

    adv->adv_param.has_custom_adv_timing = true;

    if (adv->custom_adv_interval_ms < 100)
    {
        adv->adv_param.fast_advertising = true;
        adv->adv_param.adv_timing.min_adv_fast_interval_ms = adv->custom_adv_interval_ms;
        adv->adv_param.adv_timing.max_adv_fast_interval_ms = adv->custom_adv_interval_ms;
        adv->adv_param.adv_timing.min_adv_fast_interval_coded_ms = adv->custom_adv_interval_ms;
        adv->adv_param.adv_timing.max_adv_fast_interval_coded_ms = adv->custom_adv_interval_ms;
    }
    else
    {
        adv->adv_param.fast_advertising = false;
        adv->adv_param.adv_timing.min_adv_slow_interval_ms = adv->custom_adv_interval_ms;
        adv->adv_param.adv_timing.max_adv_slow_interval_ms = adv->custom_adv_interval_ms;
        adv->adv_param.adv_timing.min_adv_slow_interval_coded_ms = adv->custom_adv_interval_ms;
        adv->adv_param.adv_timing.max_adv_slow_interval_coded_ms = adv->custom_adv_interval_ms;
    }
}

void app_ble_set_adv_tx_power_dbm(ble_adv_activity_t *adv, int8_t tx_power_dbm)
{
    if (tx_power_dbm != -127 && tx_power_dbm != 127)
    {
        adv->adv_param.has_prefer_adv_tx_power = true;
        adv->adv_param.adv_tx_power = tx_power_dbm;
    }
    else
    {
        adv->adv_param.has_prefer_adv_tx_power = false;
        adv->adv_param.adv_tx_power = GAP_UNKNOWN_TX_POWER;
    }
}

void app_ble_set_adv_tx_power_level(ble_adv_activity_t *adv, BLE_ADV_TX_POWER_LEVEL_E tx_power_level)
{
    app_ble_set_adv_tx_power_dbm(adv, btdrv_reg_op_txpwr_idx_to_rssidbm(tx_power_level));
}

void app_ble_set_adv_txpwr_by_adv_user(BLE_ADV_USER_E user, int8_t txpwr_dbm)
{
    ble_adv_activity_t *adv = NULL;

    adv = app_ble_get_advertising_by_user(user);
    if (adv == NULL)
    {
        return;
    }

    app_ble_set_adv_tx_power_dbm(adv, txpwr_dbm);
}

static uint16_t app_ble_parse_out_service_uuid(uint8_t *data, uint16_t len, gap_dt_buf_t *out_uuid_16, gap_dt_buf_t *out_uuid_128)
{
    gap_dt_head_t *curr = (gap_dt_head_t *)data;
    const uint8_t *data_type_ptr = NULL;
    uint16_t removed_len = 0;
    uint16_t left = len;
    uint16_t item_data_len = 0;
    uint16_t data_type_len = 0;
    bool remove_data_type = false;

    if (data == NULL || len <= sizeof(gap_dt_head_t) || len > EXT_ADV_DATA_LEN)
    {
        return 0;
    }

    while (curr->length && left >= curr->length + 1)
    {
        item_data_len = curr->length + 1;
        data_type_len = curr->length - 1;
        data_type_ptr = ((uint8_t *)curr) + 2;
        remove_data_type = false;
        if (curr->ad_type == GAP_DT_SRVC_UUID_16_INCP_LIST || curr->ad_type == GAP_DT_SRVC_UUID_16_CMPL_LIST)
        {
            if (data_type_len && (data_type_len % 2) == 0)
            {
                gap_dt_add_raw_data(out_uuid_16, data_type_ptr, data_type_len);
            }
            else
            {
                CO_LOG_ERR_2(BT_STS_INVALID_ADV_DATA, curr->ad_type, data_type_len);
            }
            remove_data_type = true;
        }
        else if (curr->ad_type == GAP_DT_SRVC_UUID_128_INCP_LIST || curr->ad_type == GAP_DT_SRVC_UUID_128_CMPL_LIST)
        {
            if (data_type_len && (data_type_len % 16) == 0)
            {
                gap_dt_add_raw_data(out_uuid_128, data_type_ptr, data_type_len);
            }
            else
            {
                CO_LOG_ERR_2(BT_STS_INVALID_ADV_DATA, curr->ad_type, data_type_len);
            }
            remove_data_type = true;
        }
        else if (curr->ad_type == GAP_DT_FLAGS || curr->ad_type == GAP_DT_APPEARANCE)
        {
            remove_data_type = true;
        }
        if (remove_data_type)
        {
            removed_len += item_data_len;
            left -= item_data_len;
            if (left)
            {
                memmove(curr, ((uint8_t *)curr)+item_data_len, left);
            }
        }
        else
        {
            left -= item_data_len;
            curr = (gap_dt_head_t *)(((uint8_t *)curr) + item_data_len);
        }
    }

    return len - removed_len;
}

#if defined(IBRT)
bool app_ble_check_ibrt_allow_adv(BLE_ADV_USER_E user)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if (!p_ibrt_ctrl->init_done)
    {
        return false;
    }

#if defined(APP_SOUND_ENABLE)
    return true;
#endif

#if defined(FREEMAN_ENABLED_STERO)
    return true;
#else

    if (app_ibrt_conn_is_freeman_mode())
    {
        return true;
    }
    return (app_ibrt_conn_get_ui_role() != TWS_UI_SLAVE);
#endif
}
#endif

/**
 * old stack adv functions:
 *
 * app_ble_register_data_fill_handle
 * bes_ble_gap_register_data_fill_handle
 * ble_adv_fill_param
 * app_ble_start_adv
 * ble_adv_config_param
 * app_ble_adv_param_is_different
 * ble_adv_refreshing
 * BLE_ADV_RPA_ENABLED
 * app_ai_ble_data_fill_handler
 * app_ama_start_advertising
 * app_dma_start_advertising
 * app_gma_start_advertising
 * app_sv_start_advertising
 * app_tencent_start_advertising
 * voice_start_advertising
 * app_interconnection_ble_data_fill_handler
 * ota_ble_data_fill_handler
 * tile_ble_data_fill_handler
 * app_ble_stub_user_data_fill_handler
 * app_ble_custom_0_user_data_fill_handler
 * app_ble_custom_1_user_data_fill_handler
 * app_ble_custom_2_user_data_fill_handler
 * app_ble_custom_3_user_data_fill_handler
 * app_ble_demo0_user_data_fill_handler
 * app_ble_demo1_user_data_fill_handler
 * gfps_ble_data_fill_handler
 * spot_ble_data_fill_handler
 * swift_ble_data_fill_handler
 * app_ble_audio_user_data_fill_handler
 *
 * ble_start_adv
 * appm_start_advertising
 * gapm_activity_create_cmd_handler
 * gapm_adv_legacy_create/gapm_adv_ext_create/gapm_adv_periodic_create
 * gapm_adv_create
 * gapm_adv_create_transition
 * own_addr_type = gapm_le_actv_get_hci_own_addr_type(p_actv->own_addr_type)
 * gapm_adv_proc_start_transition
 *
 */

bool app_ble_get_user_adv_data(ble_adv_activity_t *adv, BLE_ADV_PARAM_T *param, int user_group)
{
    ble_global_t *g = ble_get_global();
    BLE_ADV_USER_E *consider_user = NULL; // ble_adv_fill_param
    BLE_ADV_USER_E user_group_0[] = {USER_STUB, USER_GSOUND, USER_AI, USER_INTERCONNECTION, USER_TILE, USER_OTA};
    uint32_t smallest_adv_interval = 0xFFFFFFFF;
    bool has_custom_adv_interval = false;
    bool has_user_enable_adv = false;
    BLE_ADV_USER_E user;
    int user_count = 0;
    int i = 0;

    if (user_group > BLE_ADV_ACTIVITY_USER_0)
    {
        return false; // currently no other group adv data
    }

    consider_user = user_group_0;
    user_count = ARRAY_SIZE(user_group_0);

    if (!ble_adv_is_allowed())
    {
        return false;
    }

#if defined(IBRT)
    if (!app_ble_check_ibrt_allow_adv(USER_INUSE))
    {
        return false;
    }
#endif

    for (i = 0; i < user_count; i += 1)
    {
        user = consider_user[i];
        if (g->data_fill_func[user])
        {
            g->data_fill_func[user](param);
            if (g->data_fill_enable[user])
            {
                has_user_enable_adv = true;
            }
            // Update to user specific interval
            param->advUserInterval[i] = param->advInterval;
        }
    }

    if (has_user_enable_adv)
    {
        for (i = 0; i < BLE_ADV_USER_NUM; i += 1)
        {
            if (param->advUserInterval[i] && param->advUserInterval[i] < smallest_adv_interval)
            {
                smallest_adv_interval = param->advUserInterval[i];
                has_custom_adv_interval = true;
            }
        }
    }

    if (has_custom_adv_interval)
    {
        adv->custom_adv_interval_ms = smallest_adv_interval;
    }

#if (BLE_APP_HID)
    memcpy(&param->advData[param->advDataLen], APP_HID_ADV_DATA_UUID, APP_HID_ADV_DATA_UUID_LEN);
    param->advDataLen += APP_HID_ADV_DATA_UUID_LEN;
    has_user_enable_adv = true;
#endif

    return has_user_enable_adv;
}

void app_ble_dt_add_adv_data(ble_adv_activity_t *adv, BLE_ADV_PARAM_T *a, const app_ble_adv_data_param_t *b)
{
    gap_dt_buf_t adv_data_uuid_16_list = {0};
    gap_dt_buf_t adv_data_uuid_128_list = {0};
    gap_dt_buf_t scan_rsp_uuid_16_list = {0};
    gap_dt_buf_t scan_rsp_uuid_128_list = {0};
    gap_dt_buf_t *dest_adv_data = &adv->adv_param.adv_data;
    gap_dt_buf_t *dest_scan_rsp = &adv->adv_param.scan_rsp_data;
    gap_dt_buf_t adv_buf_b = {0};
    gap_dt_buf_t scan_buf_b = {0};
    const uint8_t *left_adv_data_a = NULL;
    const uint8_t *left_adv_data_b = NULL;
    const uint8_t *left_scan_rsp_a = NULL;
    const uint8_t *left_scan_rsp_b = NULL;
    uint8_t left_adv_data_len_a = 0;
    uint8_t left_adv_data_len_b = 0;
    uint8_t left_scan_rsp_len_a = 0;
    uint8_t left_scan_rsp_len_b = 0;

    left_adv_data_len_a = app_ble_parse_out_service_uuid(a->advData, a->advDataLen, &adv_data_uuid_16_list, &adv_data_uuid_128_list);
    left_adv_data_a = a->advData;

    left_scan_rsp_len_a = app_ble_parse_out_service_uuid(a->scanRspData, a->scanRspDataLen, &scan_rsp_uuid_16_list, &scan_rsp_uuid_128_list);
    left_scan_rsp_a = a->scanRspData;

    if (b && b->adv_data && b->adv_data_len)
    {
        gap_dt_add_raw_data(&adv_buf_b, b->adv_data, b->adv_data_len);
        left_adv_data_len_b = app_ble_parse_out_service_uuid(ppb_get_data(adv_buf_b.ppb), adv_buf_b.ppb->len, &adv_data_uuid_16_list, &adv_data_uuid_128_list);
        left_adv_data_b = ppb_get_data(adv_buf_b.ppb);
    }

    if (b && b->scan_rsp_data && b->scan_rsp_len)
    {
        gap_dt_add_raw_data(&scan_buf_b, b->scan_rsp_data, b->scan_rsp_len);
        left_scan_rsp_len_b = app_ble_parse_out_service_uuid(ppb_get_data(scan_buf_b.ppb), scan_buf_b.ppb->len, &scan_rsp_uuid_16_list, &scan_rsp_uuid_128_list);
        left_scan_rsp_b = ppb_get_data(scan_buf_b.ppb);
    }

    /**
     * adv data
     *
     */
    if (adv_data_uuid_16_list.ppb && adv_data_uuid_16_list.ppb->len)
    {
        gap_dt_head_t head;
        head.length = adv_data_uuid_16_list.ppb->len + 1;
        head.ad_type = GAP_DT_SRVC_UUID_16_CMPL_LIST;
        gap_dt_add_raw_data(dest_adv_data, (uint8_t *)&head, sizeof(head));
        gap_dt_add_raw_data(dest_adv_data, ppb_get_data(adv_data_uuid_16_list.ppb), adv_data_uuid_16_list.ppb->len);
    }

    if (adv_data_uuid_128_list.ppb && adv_data_uuid_128_list.ppb->len)
    {
        gap_dt_head_t head;
        head.length = adv_data_uuid_128_list.ppb->len + 1;
        head.ad_type = GAP_DT_SRVC_UUID_128_CMPL_LIST;
        gap_dt_add_raw_data(dest_adv_data, (uint8_t *)&head, sizeof(head));
        gap_dt_add_raw_data(dest_adv_data, ppb_get_data(adv_data_uuid_128_list.ppb), adv_data_uuid_128_list.ppb->len);
    }

    if (left_adv_data_a && left_adv_data_len_a)
    {
        gap_dt_add_raw_data(dest_adv_data, left_adv_data_a, left_adv_data_len_a);
    }

    if (left_adv_data_b && left_adv_data_len_b)
    {
        gap_dt_add_raw_data(dest_adv_data, left_adv_data_b, left_adv_data_len_b);
    }

    /**
     * scan rsp data
     *
     */
    if (adv->adv_param.scannable || (adv->adv_param.use_legacy_pdu && adv->adv_param.connectable))
    {
        if (scan_rsp_uuid_16_list.ppb && scan_rsp_uuid_16_list.ppb->len)
        {
            gap_dt_head_t head;
            head.length = scan_rsp_uuid_16_list.ppb->len + 1;
            head.ad_type = GAP_DT_SRVC_UUID_16_CMPL_LIST;
            gap_dt_add_raw_data(dest_scan_rsp, (uint8_t *)&head, sizeof(head));
            gap_dt_add_raw_data(dest_scan_rsp, ppb_get_data(scan_rsp_uuid_16_list.ppb), scan_rsp_uuid_16_list.ppb->len);
        }

        if (scan_rsp_uuid_128_list.ppb && scan_rsp_uuid_128_list.ppb->len)
        {
            gap_dt_head_t head;
            head.length = scan_rsp_uuid_128_list.ppb->len + 1;
            head.ad_type = GAP_DT_SRVC_UUID_128_CMPL_LIST;
            gap_dt_add_raw_data(dest_scan_rsp, (uint8_t *)&head, sizeof(head));
            gap_dt_add_raw_data(dest_scan_rsp, ppb_get_data(scan_rsp_uuid_128_list.ppb), scan_rsp_uuid_128_list.ppb->len);
        }

        if (left_scan_rsp_a && left_scan_rsp_len_a)
        {
            gap_dt_add_raw_data(dest_scan_rsp, left_scan_rsp_a, left_scan_rsp_len_a);
        }

        if (left_scan_rsp_b && left_scan_rsp_len_b)
        {
            gap_dt_add_raw_data(dest_scan_rsp, left_scan_rsp_b, left_scan_rsp_len_b);
        }
    }

    gap_dt_buf_clear(&adv_data_uuid_16_list);
    gap_dt_buf_clear(&adv_data_uuid_128_list);
    gap_dt_buf_clear(&scan_rsp_uuid_16_list);
    gap_dt_buf_clear(&scan_rsp_uuid_128_list);
    gap_dt_buf_clear(&adv_buf_b);
    gap_dt_buf_clear(&scan_buf_b);
}

POSSIBLY_UNUSED static uint8_t app_ble_get_ad_flags(BLE_ADV_PARAM_T *adv_param)
{
    uint8_t ltv_pos = 0;
    // Fetch out ad flags
    if ((adv_param->advData[ltv_pos++] == 2)
            && (adv_param->advData[ltv_pos++] == GAP_DT_FLAGS))
    {
        return adv_param->advData[ltv_pos];
    }

    return 0;
}

bool app_ble_stub_adv_activity_prepare(ble_adv_activity_t *adv)
{
    gap_adv_param_t *adv_param = &adv->adv_param;
    BLE_ADV_PARAM_T legacy_param = {0};
    bool adv_legacy_enable = false;

    if (!ble_adv_is_allowed())
    {
        return false;
    }

#if defined(IBRT)
    if (!app_ble_check_ibrt_allow_adv(USER_STUB))
    {
        return false;
    }
#endif

#ifdef CTKD_ENABLE // ctkd needs ble adv no matter whether a mobile bt link has been established or not
    set_rsp_dist_lk_bit_field_func dist_lk_set_cb = app_sec_reg_dist_lk_bit_get_callback();
    if (dist_lk_set_cb && !dist_lk_set_cb())
    {
        CO_LOG_WAR_0(BT_STS_NOT_ALLOW);
        return false;
    }
#else
    ble_global_t *g = ble_get_global();
    if (!g->ble_stub_adv_enable)
    {
        return false;
    }
#endif

    adv_legacy_enable = app_ble_get_user_adv_data(adv, &legacy_param, BLE_ADV_ACTIVITY_USER_0);
    if (!adv_legacy_enable)
    {
        return false;
    }

    adv->user = USER_STUB;
    adv_param->connectable = true;
    adv_param->scannable = true;
    adv_param->use_legacy_pdu = true;
    adv_param->include_tx_power_data = true;

    app_ble_set_adv_tx_power_level(adv, BLE_ADV_TX_POWER_LEVEL_0);

    app_ble_dt_set_flags(adv_param, false);

    app_ble_dt_add_adv_data(adv, &legacy_param, NULL);

    app_ble_dt_set_local_name(adv_param, NULL);

    return true;
}

#if (BLE_AUDIO_ENABLED)
static bool app_ble_audio_adv_activity_prepare(ble_adv_activity_t *adv)
{
    ble_global_t *g = ble_get_global();
    gap_adv_param_t *adv_param = &adv->adv_param;
    BLE_ADV_PARAM_T extended_param = {0};
    bool adv_extended_enable = false;
    uint16_t appearance = 0;

    if (!ble_adv_is_allowed())
    {
        return false;
    }

    if (g->data_fill_func[USER_BLE_AUDIO])
    {
        g->data_fill_func[USER_BLE_AUDIO](&extended_param);
        if (g->data_fill_enable[USER_BLE_AUDIO])
        {
            adv_extended_enable = true;
        }
    }

    if (!adv_extended_enable)
    {
        return false;
    }

    adv->user = USER_BLE_AUDIO;
    // Use custom specifc adv interval
    adv->custom_adv_interval_ms = extended_param.advInterval;
    /// TODO:Add custom adv interval into timing
    // BLE AUDIO adv specification param
    adv_param->connectable = true;
    adv_param->scannable = true;
    adv_param->use_legacy_pdu = false;
    adv_param->include_tx_power_data = true;

    if ((gap_filter_list_user_item_exist(BLE_WHITE_LIST_USER_MOBILE)
        || gap_filter_list_user_item_exist(BLE_WHITE_LIST_USER_TWS))
        && gap_resolving_list_curr_size() != 0)
    {
        adv_param->policy = GAP_ADV_ACCEPT_ALL_CONN_SCAN_REQS_IN_LIST;
    }
    else
    {
        adv_param->policy = GAP_ADV_ACCEPT_ALL_CONN_SCAN_REQS;
    }

    app_ble_set_adv_tx_power_level(adv, BLE_ADV_TX_POWER_LEVEL_0);

    uint8_t ad_flags = app_ble_get_ad_flags(&extended_param);

    gap_dt_add_data_type(&adv_param->adv_data, GAP_DT_FLAGS, &ad_flags, sizeof(ad_flags));

    appearance = co_host_to_uint16_le(APPEARANCE_VALUE);
    if (appearance)
    {
        gap_dt_add_data_type(&adv_param->adv_data, GAP_DT_APPEARANCE, (uint8_t *)&appearance, sizeof(uint16_t));
    }

    app_ble_dt_add_adv_data(adv, &extended_param, NULL);

    app_ble_dt_set_local_name(adv_param, NULL);

    return true;
}
#endif /* BLE_AUDIO_ENABLED */

void app_ble_enable_advertising(uint8_t adv_handle)
{
    if (bt_defer_curr_func_1(app_ble_enable_advertising, bt_fixed_param(adv_handle)))
    {
        return;
    }

    POSSIBLY_UNUSED uint32_t ca = CO_LR_ADDRESS;
    ble_adv_activity_t *adv = NULL;

    if (adv_handle == GAP_INVALID_ADV_HANDLE)
    {
        return;
    }

    adv = app_ble_get_advertising(adv_handle);
    if (adv == NULL)
    {
        return;
    }

    if (ble_adv_is_allowed())
    {
        CO_LOG_INFO_S_2(BT_STS_START_ADV, SADV, adv_handle, ca);
        app_ble_set_adv_interval(adv);
        app_ble_refresh_advertising(adv_handle, &adv->adv_param);
    }
}

void app_ble_disable_advertising(uint8_t adv_handle)
{
    POSSIBLY_UNUSED uint32_t ca = CO_LR_ADDRESS;
    ble_adv_activity_t *adv = NULL;

    if (bt_defer_curr_func_1(app_ble_disable_advertising, bt_fixed_param(adv_handle)))
    {
        return;
    }

    if (adv_handle == GAP_INVALID_ADV_HANDLE)
    {
        return;
    }

    CO_LOG_INFO_S_2(BT_STS_TERMINATE, TADV, adv_handle, ca);

    gap_disable_advertising(adv_handle, true, GAP_ADV_DISABLE_BY_APP_BLE);

    adv = app_ble_get_advertising(adv_handle);
    if (adv)
    {
        adv->adv_is_started = false;
    }
}

uint16_t app_ble_adv_find_appearance_type(const uint8_t *data, uint16_t data_len)
{
    uint16_t appearance = 0;
    gap_dt_buf_t adv_data_buf = {0};
    const gap_dt_head_t *appearance_data = NULL;

    gap_dt_add_raw_data(&adv_data_buf, data, data_len);
    appearance_data = gap_dt_buf_find_type(&adv_data_buf, GAP_DT_APPEARANCE, 0);
    if (appearance_data)
    {
        appearance = *(uint16_t *)(appearance_data + 1);
    }
    gap_dt_buf_clear(&adv_data_buf);

    return appearance;
}


uint8_t app_ble_get_adv_hdl_by_user(BLE_ADV_USER_E user)
{
    uint8_t advHdl;
    switch (user)
    {
        case USER_GFPS:
        advHdl = BLE_GFPS_ADV_HANDLE;
        break;

        case USER_SWIFT:
        advHdl = BLE_SWIFT_ADV_HANDLE;
        break;

        case USER_BLE_AUDIO:
        advHdl = BLE_AUDIO_ADV_HANDLE;
        break;

        case USER_AI:
        case USER_INTERCONNECTION:
        case USER_TILE:
        advHdl = BLE_BASIC_ADV_HANDLE;
        break;

        case USER_BLE_CUSTOMER_0:
        advHdl = BLE_CUSTOMER0_ADV_HANDLE;
        break;
        case USER_BLE_CUSTOMER_1:
        advHdl = BLE_CUSTOMER1_ADV_HANDLE;
        break;
        case USER_BLE_CUSTOMER_2:
        advHdl = BLE_CUSTOMER2_ADV_HANDLE;
        break;
        case USER_BLE_CUSTOMER_3:
        advHdl = BLE_CUSTOMER3_ADV_HANDLE;
        break;

        default:
        advHdl = BLE_BASIC_ADV_HANDLE;
        break;
    }
    return advHdl;
}

BLE_ADV_USER_E app_ble_get_adv_user(uint32_t user)
{
    BLE_ADV_USER_E advUser = USER_INUSE;
    if(user & (1 << USER_GFPS)) {
        advUser = USER_GFPS;
    } else if(user & (1 <<USER_SWIFT)) {
        advUser = USER_SWIFT;
    } else if(user & (1 <<USER_AI)) {
        advUser = USER_AI;
    } else if(user & (1 <<USER_INTERCONNECTION)) {
        advUser = USER_INTERCONNECTION;
    } else if(user & (1 <<USER_TILE)) {
        advUser = USER_TILE;
    } else if(user & (1 <<USER_OTA)) {
        advUser = USER_OTA;
    } else if(user & (1 <<USER_BLE_AUDIO)) {
        advUser = USER_BLE_AUDIO;
    } else if(user & (1 <<USER_SPOT)) {
        advUser = USER_SPOT;
    } else if(user & (1 <<USER_BLE_CUSTOMER_0)) {
        advUser = USER_BLE_CUSTOMER_0;
    } else if(user & (1 <<USER_BLE_CUSTOMER_1)) {
        advUser = USER_BLE_CUSTOMER_1;
    } else if(user & (1 <<USER_BLE_CUSTOMER_2)) {
        advUser = USER_BLE_CUSTOMER_2;
    } else if(user & (1 <<USER_BLE_CUSTOMER_3)) {
        advUser = USER_BLE_CUSTOMER_3;
    } else {
        advUser = USER_INUSE;
    }
    return advUser;
}

uint8_t app_ble_filter_duplicate_data(uint8_t *ori, uint8_t oriLen, uint8_t *data, uint8_t dataLen)
{
    uint8_t leftLen = 0;
    uint8_t totalLen = dataLen;
    uint8_t tempOriLen = 0;
    uint8_t tempData[dataLen];
    gap_dt_head_t *oriHeader = NULL;
    gap_dt_head_t *dataHeader = NULL;
    uint8_t *oriPtr = ori;
    uint8_t *dataPtr = data;
    bool isFilter = false;

    while (dataLen)
    {
        dataHeader = (gap_dt_head_t *)dataPtr;
        oriPtr = ori;
        tempOriLen = oriLen;
        isFilter = false;
        while (tempOriLen)
        {
            oriHeader = (gap_dt_head_t *)oriPtr;
            if (!memcmp(oriPtr, dataPtr, dataHeader->length+1))
            {
                isFilter = true;
                break;
            }
            tempOriLen -= oriHeader->length+1;
            oriPtr += oriHeader->length+1;
        }

        if (!isFilter)
        {
            memcpy(tempData+leftLen, dataPtr, dataHeader->length+1);
            leftLen += dataHeader->length+1;
        }
        dataPtr += dataHeader->length+1;
        dataLen -= dataHeader->length+1;
    }

    if (totalLen != leftLen)
    {
        memcpy(data, tempData, leftLen);
    }
    return leftLen;
}

void app_ble_adv_set_param(BLE_ADV_PARAM_T *param, uint8_t user)
{
    if (!param || (param->adv_actv_user >= BLE_ADV_ACTIVITY_USER_NUM))
    {
        TRACE(0, "%s invalid param!!!", __func__);
        return;
    }

    BLE_ADV_PARAM_T tmp_param = {0};
    memcpy(&tmp_param, param, sizeof(BLE_ADV_PARAM_T));
    ble_global_t *g = ble_get_global();
    ble_adv_activity_t *defaultAdv = g->adv + app_ble_get_adv_user(tmp_param.advUser);;

    bool legacy_adv = false;
    bool connectable = false;
    bool scannable = false;
    bool directed = false;
    bt_bdaddr_t zero_addr = {{0}};

    tmp_param.advDataLen = app_ble_filter_duplicate_data((uint8_t *)gap_dt_buf_data(&defaultAdv->adv_param.adv_data),
                                                         gap_dt_buf_len(&defaultAdv->adv_param.adv_data),
                                                         tmp_param.advData, tmp_param.advDataLen);

    tmp_param.scanRspDataLen = app_ble_filter_duplicate_data((uint8_t *)gap_dt_buf_data(&defaultAdv->adv_param.scan_rsp_data),
                                                         gap_dt_buf_len(&defaultAdv->adv_param.scan_rsp_data),
                                                         tmp_param.scanRspData, tmp_param.scanRspDataLen);

    if (gap_filter_list_user_item_exist(BLE_WHITE_LIST_USER_MOBILE) || gap_filter_list_user_item_exist(BLE_WHITE_LIST_USER_TWS))
    {
        defaultAdv->adv_param.policy = GAP_ADV_ACCEPT_ALL_CONN_SCAN_REQS_IN_LIST;
    }
    else
    {
        defaultAdv->adv_param.policy = GAP_ADV_ACCEPT_ALL_CONN_SCAN_REQS;
    }

    legacy_adv = (tmp_param.advMode == ADV_MODE_LEGACY);
    connectable = (tmp_param.advType != ADV_TYPE_NON_CONN_SCAN && tmp_param.advType != ADV_TYPE_NON_CONN_NON_SCAN);
    scannable = (tmp_param.advType == ADV_TYPE_UNDIRECT || tmp_param.advType == ADV_TYPE_NON_CONN_SCAN || tmp_param.advType == ADV_TYPE_CONN_EXT_ADV);
    directed = (tmp_param.advType == ADV_TYPE_DIRECT_LDC || tmp_param.advType == ADV_TYPE_DIRECT_HDC);

#ifdef BLE_WATCH_ADAPTER
    defaultAdv->adv_from_adapter = true;
#endif
    defaultAdv->user = app_ble_get_adv_user(tmp_param.advUser);
    defaultAdv->adv_handle = app_ble_get_adv_hdl_by_user(defaultAdv->user);
    defaultAdv->adv_param.connectable = connectable;
    defaultAdv->adv_param.scannable = scannable;
    defaultAdv->adv_param.directed_adv = directed;
    defaultAdv->adv_param.high_duty_directed_adv = (tmp_param.advType == ADV_TYPE_DIRECT_HDC);
    defaultAdv->adv_param.use_legacy_pdu = legacy_adv;
    defaultAdv->adv_param.include_tx_power_data = true;
    defaultAdv->adv_param.own_addr_use_rpa = (tmp_param.localAddrType == GAPM_GEN_RSLV_ADDR);
    defaultAdv->adv_param.peer_type = (bt_addr_type_t)tmp_param.peerAddr.addr_type;
    defaultAdv->adv_param.peer_addr = *(bt_bdaddr_t *)tmp_param.peerAddr.addr;

    if (memcmp(&tmp_param.localAddr, &zero_addr, sizeof(bt_bdaddr_t)) != 0)
    {
        defaultAdv->adv_param.use_custom_local_addr = true;
        memcpy(&defaultAdv->adv_param.custom_local_addr, tmp_param.localAddr, 6);
    }

    if (!defaultAdv->custom_adv_interval_ms || tmp_param.advInterval < defaultAdv->custom_adv_interval_ms)
    {
        defaultAdv->custom_adv_interval_ms = tmp_param.advInterval;
    }
    app_ble_set_adv_tx_power_dbm(defaultAdv, btdrv_reg_op_txpwr_idx_to_rssidbm(tmp_param.advTxPwr));

    uint8_t ad_flags = app_ble_get_ad_flags(&tmp_param);
    if (tmp_param.isBleFlagsAdvDataConfiguredByAppLayer && ad_flags && \
                    !gap_dt_buf_find_type(&defaultAdv->adv_param.adv_data, GAP_DT_FLAGS, 0))
    {
        gap_dt_add_data_type(&defaultAdv->adv_param.adv_data, GAP_DT_FLAGS, &ad_flags, sizeof(ad_flags));
    }

    uint16_t appearance = 0;
    //find adv data appearance type
    appearance = app_ble_adv_find_appearance_type(tmp_param.advData, tmp_param.advDataLen);
    if (appearance)
    {
        gap_dt_add_data_type(&defaultAdv->adv_param.adv_data, GAP_DT_APPEARANCE, (uint8_t *)&appearance, sizeof(uint16_t));
        appearance = 0;
    }

    //find scan rsp data appearance type
    appearance = app_ble_adv_find_appearance_type(tmp_param.scanRspData, tmp_param.scanRspDataLen);
    if (appearance)
    {
        gap_dt_add_data_type(&defaultAdv->adv_param.scan_rsp_data, GAP_DT_APPEARANCE, (uint8_t *)&appearance, sizeof(uint16_t));
        appearance = 0;
    }

    app_ble_dt_add_adv_data(defaultAdv, &tmp_param, NULL);
    // app_ble_dt_set_local_name(&defaultAdv->adv_param, NULL);

    TRACE(0, "[ADV_LEN] %d [DATA]:", gap_dt_buf_len(&defaultAdv->adv_param.adv_data));
    DUMP8("%02x ", gap_dt_buf_data(&defaultAdv->adv_param.adv_data), gap_dt_buf_len(&defaultAdv->adv_param.adv_data));
    TRACE(0, "[SCAN_RSP_LEN] %d [DATA]:", gap_dt_buf_len(&defaultAdv->adv_param.scan_rsp_data));
    DUMP8("%02x ", gap_dt_buf_data(&defaultAdv->adv_param.scan_rsp_data), gap_dt_buf_len(&defaultAdv->adv_param.scan_rsp_data));

    if (legacy_adv)
    {
        ASSERT(BLE_ADV_DATA_WITH_FLAG_LEN >= gap_dt_buf_len(&defaultAdv->adv_param.adv_data), "[BLE][ADV]adv data exceed");
        ASSERT(SCAN_RSP_DATA_LEN >= gap_dt_buf_len(&defaultAdv->adv_param.scan_rsp_data), "[BLE][ADV]scan response data exceed");
    }
}

ble_adv_activity_t *app_ble_adv_get_param(BLE_ADV_ACTIVITY_USER_E user)
{
    ble_global_t *g = ble_get_global();
    ble_adv_activity_t *defaultAdv = g->adv + app_ble_get_adv_user(user);

    return defaultAdv;
}

void app_ble_process_adv_param(ble_adv_activity_t *src, ble_adv_activity_t *dst)
{
    app_ble_clean_adv_param(&dst->adv_param);

    gap_dt_buf_t adv_data = {0};
    gap_dt_buf_t scan_rsp_data = {0};

    gap_dt_add_raw_data(&adv_data, gap_dt_buf_data(&src->adv_param.adv_data), gap_dt_buf_len(&src->adv_param.adv_data));
    gap_dt_add_raw_data(&scan_rsp_data, gap_dt_buf_data(&src->adv_param.scan_rsp_data), gap_dt_buf_len(&src->adv_param.scan_rsp_data));

    gap_dt_buf_clear(&src->adv_param.adv_data);
    gap_dt_buf_clear(&src->adv_param.scan_rsp_data);

    memcpy(dst, src, sizeof(ble_adv_activity_t));

    if (!gap_dt_buf_find_type(&adv_data, GAP_DT_FLAGS, 0))
    {
        app_ble_dt_set_flags(&dst->adv_param, false);
    }

    gap_dt_add_raw_data(&src->adv_param.adv_data, gap_dt_buf_data(&adv_data), gap_dt_buf_len(&adv_data));
    gap_dt_add_raw_data(&src->adv_param.scan_rsp_data, gap_dt_buf_data(&scan_rsp_data), gap_dt_buf_len(&scan_rsp_data));
    gap_dt_add_raw_data(&dst->adv_param.adv_data, gap_dt_buf_data(&adv_data), gap_dt_buf_len(&adv_data));
    gap_dt_add_raw_data(&dst->adv_param.scan_rsp_data, gap_dt_buf_data(&scan_rsp_data), gap_dt_buf_len(&scan_rsp_data));

    gap_dt_buf_clear(&adv_data);
    gap_dt_buf_clear(&scan_rsp_data);
}

static void app_ble_impl_refresh_adv(void)
{
    ble_global_t *g = ble_get_global();
    ble_adv_activity_t *adv = NULL;
    int activity_count = ARRAY_SIZE(g->adv);
    int i = 0;
    POSSIBLY_UNUSED bool adv_enable = false;

    if (!ble_adv_is_allowed())
    {
        for (i = 0; i < activity_count; i += 1)
        {
            adv = g->adv + i;
            app_ble_disable_advertising(adv->adv_handle);
        }
        return;
    }

    // Check whether adding parsing list is in progress
    if (gap_is_refresh_adv_after_addr_reso_enable())
    {
        return;
    }

    for (i = 0; i < activity_count; i += 1)
    {
        adv = g->adv + i;
		
		if (adv->adv_activity_func == NULL)
        {
            continue;
        }

        if (adv->adv_param.own_addr_use_rpa && (!gap_address_reso_is_enabled()))
        {
            continue;
        }

#ifdef BLE_WATCH_ADAPTER
        if (adv->adv_from_adapter && (gap_dt_buf_len(&adv->adv_param.adv_data) || gap_dt_buf_len(&adv->adv_param.scan_rsp_data)))
        {
            CO_LOG_INFO_S_1(BT_STS_REFRESH_ADV, FAEN, adv->adv_handle);
            app_ble_set_adv_interval(adv);
            app_ble_refresh_advertising(adv->adv_handle, &adv->adv_param);
        }
        else
#endif
        {
            if (adv->adv_activity_func == NULL)
            {
                continue;
            }

            app_ble_clean_adv_param(&adv->adv_param);
            adv_enable = adv->adv_activity_func(adv);
            if (adv_enable)
            {
                CO_LOG_INFO_S_1(BT_STS_REFRESH_ADV, FAEN, adv->adv_handle);
                app_ble_set_adv_interval(adv);
                app_ble_refresh_advertising(adv->adv_handle, &adv->adv_param);
            }
            else
            {
                CO_LOG_INFO_S_1(BT_STS_ADV_STATUS, FDIS, adv->adv_handle);
                app_ble_disable_advertising(adv->adv_handle);
            }
        }
    }
}

void app_ble_impl_stop_adv(void)
{
    ble_global_t *g = ble_get_global();
    ble_adv_activity_t *adv = NULL;
    int i = 0;

    for (i = 0; i < ARRAY_SIZE(g->adv); i += 1)
    {
        adv = g->adv + i;
        if (adv->adv_is_started)
        {
            app_ble_disable_advertising(adv->adv_handle);
        }
    }
}

void app_ble_refresh_adv_state_generic(void)
{
    CO_LOG_INFO_S_1(BT_STS_REFRESH_ADV, FADV, CO_LR_ADDRESS);
    bt_defer_call_func_0(app_ble_impl_refresh_adv);
}

void app_ble_start_adv_generic(void)
{
    CO_LOG_INFO_S_1(BT_STS_START_ADV, FADV, CO_LR_ADDRESS);
    bt_defer_call_func_0(app_ble_impl_refresh_adv);
}

void app_ble_stop_adv_generic(void)
{
    CO_LOG_INFO_S_1(BT_STS_TERMINATE, TADV, CO_LR_ADDRESS);
    bt_defer_call_func_0(app_ble_impl_stop_adv);
}

BLE_ADV_ACTIVITY_USER_E app_ble_param_get_actv_user_from_adv_user(BLE_ADV_USER_E user)
{
    return user;
}

void app_ble_force_switch_adv(enum BLE_ADV_SWITCH_USER_E user, bool enable_adv)
{
    POSSIBLY_UNUSED uint32_t ca = CO_LR_ADDRESS;
    ble_global_t *g = ble_get_global();

    if (enable_adv)
    {
        g->adv_force_disabled &= ~(1 << user);
    }
    else
    {
        g->adv_force_disabled |= (1 << user);
    }

    CO_LOG_INFO_S_3(BT_STS_ADV_STATUS, SWTC, (user<<4)|enable_adv, g->adv_force_disabled, ca);

    app_ble_refresh_adv_state_generic();
}

void ble_core_enable_stub_adv(void)
{
    ble_global_t *g = ble_get_global();
    CO_LOG_INFO_S_0(BT_STS_ADV_STATUS, STBE);
    g->ble_stub_adv_enable = true;
    app_ble_refresh_adv_state_generic();
}

void ble_core_disable_stub_adv(void)
{
    ble_global_t *g = ble_get_global();
    CO_LOG_INFO_S_0(BT_STS_ADV_STATUS, STBD);
    g->ble_stub_adv_enable = false;
    app_ble_disable_advertising(BLE_BASIC_ADV_HANDLE);
    app_ble_refresh_adv_state_generic();
}

POSSIBLY_UNUSED static void app_ble_stub_user_data_fill_handler(void *param)
{
    TRACE(0, "%s", __func__);
    bool adv_enable = false;
    do {
#if (BLE_APP_HID)
        BLE_ADV_PARAM_T *cmd = (BLE_ADV_PARAM_T*)param;
        memcpy(&cmd->advData[cmd->advDataLen],
            APP_HID_ADV_DATA_UUID, APP_HID_ADV_DATA_UUID_LEN);
        cmd->advDataLen += APP_HID_ADV_DATA_UUID_LEN;
        memcpy(&cmd->advData[cmd->advDataLen],
            APP_HID_ADV_DATA_APPEARANCE, APP_ADV_DATA_APPEARANCE_LEN);
        cmd->advDataLen += APP_ADV_DATA_APPEARANCE_LEN;


        //cmd->advUserInterval[USER_BLE_DEMO0] = BLE_ADVERTISING_INTERVAL;
        adv_enable = true;
        break;
#endif //(BLE_APP_HID)

        // ctkd needs ble adv no matter whether a mobile bt link has been established or not
#ifdef CTKD_ENABLE
        set_rsp_dist_lk_bit_field_func dist_lk_set_cb = app_sec_reg_dist_lk_bit_get_callback();
        if ((dist_lk_set_cb && dist_lk_set_cb()) || (!dist_lk_set_cb))
        {
            adv_enable = true;
        }
#else

#ifdef CUSTOMER_DEFINE_ADV_DATA
        adv_enable = false;
#else
        adv_enable = true;
#endif
#endif
    } while(0);

    app_ble_data_fill_enable(USER_STUB, adv_enable);
}

void app_ble_stub_user_init(void)
{
    TRACE(0, "%s", __func__);
    app_ble_register_data_fill_handle(USER_STUB, (BLE_DATA_FILL_FUNC_T)app_ble_stub_user_data_fill_handler, false);
    ble_core_enable_stub_adv();
}

typedef struct {
    const uint8_t *peer_addr;
    bool connected;
} app_ble_foreach_param_t;

static bool app_ble_check_device_address(gap_conn_item_t *conn, void *priv)
{
    app_ble_foreach_param_t *param = (app_ble_foreach_param_t *)priv;
    if (conn->conn_type == HCI_CONN_TYPE_LE_ACL && memcmp(&conn->peer_addr, param->peer_addr, sizeof(bt_bdaddr_t)) == 0)
    {
        param->connected = true;
        return gap_end_loop;
    }
    return gap_continue_loop;
}

bool app_ble_is_connection_on_by_addr(uint8_t *addr)
{
    app_ble_foreach_param_t param = {addr, false};
    gap_conn_foreach(app_ble_check_device_address, &param);
    return param.connected;
}

int8_t app_ble_get_rssi(uint8_t con_idx)
{
    int8_t rssi = 127;
    rx_agc_t tws_agc = {0};
    uint16_t connHandle = gap_conn_hdl(con_idx);

    if (connHandle != GAP_INVALID_CONN_HANDLE)
    {
        if(bt_drv_reg_op_read_ble_rssi_in_dbm(connHandle, &tws_agc))
        {
            return tws_agc.rssi;
        }
    }

    return rssi;
}

uint16_t app_ble_get_connection_interval_1_25_ms(uint8_t conidx)
{
    gap_conn_item_t *conn = gap_get_conn_item(gap_zero_based_ble_conidx_as_hdl(conidx));

    if (conn)
    {
        return conn->timing.conn_interval_1_25ms;
    }
    else
    {
        return 0;
    }
}

ble_conn_timing_t app_ble_get_connection_curr_timing(uint8_t conidx)
{
    gap_conn_item_t *conn = gap_get_conn_item(gap_zero_based_ble_conidx_as_hdl(conidx));

    if (conn)
    {
        return *(ble_conn_timing_t *)&conn->timing;
    }

    return (ble_conn_timing_t){0};
}

uint16_t app_ble_get_connection_current_mtu_size(uint8_t conidx)
{
    ble_global_t *g = ble_get_global();

    if (conidx >= BLE_CONNECTION_MAX ||
            app_ble_is_connection_on(conidx) == false)
    {
        return 0;
    }

    return g->curr_mtu_size[conidx];
}

void app_ble_set_tx_rx_pref_phy(uint32_t tx_pref_phy, uint32_t rx_pref_phy)
{
    ble_global_t *g = ble_get_global();
    g->default_rx_pref_phy_bits = (uint8_t)tx_pref_phy;
    g->default_tx_pref_phy_bits = (uint8_t)rx_pref_phy;
}

uint8_t *app_ble_get_dev_name(void)
{
    return (uint8_t *)gap_local_le_name(NULL);
}

static void app_ble_get_nv_local_csrk(uint8_t *csrk)
{
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *dev_info = nv_record_blerec_get_ptr();
    memcpy(csrk, dev_info->self_info.ble_csrk, GAP_KEY_LEN);
}

static const uint8_t *app_ble_get_nv_local_database_hash(void)
{
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *dev_info = nv_record_blerec_get_ptr();
    return dev_info->local_database_hash;
}

static void app_ble_parse_nv_server_cache(const gattc_server_cache_t *p_input, gatt_server_cache_t *p_out)
{
    uint32_t min_size = sizeof(gattc_server_cache_t) > sizeof(gatt_server_cache_t) ?
                        sizeof(gatt_server_cache_t) : sizeof(gattc_server_cache_t);
    memcpy(p_out, p_input, min_size);
}

static void app_ble_build_nv_server_cache(const gatt_server_cache_t *p_input, gattc_server_cache_t *p_out)
{
    uint32_t min_size = sizeof(gattc_server_cache_t) > sizeof(gatt_server_cache_t) ?
                        sizeof(gatt_server_cache_t) : sizeof(gattc_server_cache_t);
    memcpy(p_out, p_input, min_size);
}

static void app_ble_parse_record_to_bond_device(const BleDevicePairingInfo *pair_info, gap_bond_sec_t *out)
{
    uint8_t peer_ediv_le[2] = {CO_SPLIT_UINT16_LE(pair_info->EDIV)};
    uint8_t local_ediv_le[2] = {CO_SPLIT_UINT16_LE(pair_info->LOCAL_EDIV)};
    out->device_paired = 1;
    out->device_bonded = (pair_info->bond_info_bf & CO_BIT_MASK(BONDED_STATUS_POS)) ? 1 : 0;
    out->secure_pairing = (pair_info->bond_info_bf & CO_BIT_MASK(BONDED_SECURE_CONNECTION)) ? 1 : 0;
    out->bonded_with_num_compare = (pair_info->bond_info_bf & CO_BIT_MASK(BONDED_WITH_NUM_COMPARE)) ? 1 : 0;
    out->bonded_with_passkey_entry = (pair_info->bond_info_bf & CO_BIT_MASK(BONDED_WITH_PASSKEY_ENTRY)) ? 1 : 0;
    out->bonded_with_oob_method = (pair_info->bond_info_bf & CO_BIT_MASK(BONDED_WITH_OOB_METHOD)) ? 1 : 0;
    out->peer_irk_distributed = (pair_info->bond_info_bf & CO_BIT_MASK(BONDED_WITH_IRK_DISTRIBUTED)) ? 1 : 0;
    out->peer_csrk_distributed = (pair_info->bond_info_bf & CO_BIT_MASK(BONDED_WITH_CSRK_DISTRIBUTED)) ? 1 : 0;
    out->peer_type = (bt_addr_type_t)(pair_info->peer_addr.addr_type & 0x01);
    out->peer_addr = *((bt_bdaddr_t *)pair_info->peer_addr.addr);
    out->enc_key_size = pair_info->enc_key_size;
    memcpy(out->ltk, pair_info->LTK, GAP_KEY_LEN);
    memcpy(out->peer_irk, pair_info->IRK, GAP_KEY_LEN);
    memcpy(out->peer_csrk, pair_info->CSRK, GAP_KEY_LEN);
    memcpy(out->local_ltk, pair_info->LOCAL_LTK, GAP_KEY_LEN);
    memcpy(out->rand, pair_info->RANDOM, sizeof(out->rand));
    memcpy(out->local_rand, pair_info->LOCAL_RANDOM, sizeof(out->local_rand));
    memcpy(out->ediv, peer_ediv_le, 2);
    memcpy(out->local_ediv, local_ediv_le, 2);
}

bool app_ble_get_nv_ble_device_by_index(uint32_t i, gap_bond_sec_t *out)
{
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *dev_info = nv_record_blerec_get_ptr();
    BleDeviceinfo *record = NULL;
    uint32_t count = dev_info->saved_list_num;
    if (count && count <= BLE_RECORD_NUM && i < count)
    {
        record = dev_info->ble_nv + i;
        if (out != NULL)
        {
            app_ble_parse_record_to_bond_device(&record->pairingInfo, out);
        }
    }
    return record != NULL;
}

static bool app_ble_get_nv_ble_device_by_addr(const gap_conn_item_t *conn, bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr, gap_bond_sec_t *out)
{
    ble_global_t *g = ble_get_global();
    BleDevicePairingInfo pair_info = {0};
    bool found = false;
    BLE_ADDR_INFO_T peer_le_addr = {.addr_type = (peer_type & 0x01)};
    memcpy(peer_le_addr.addr, peer_addr, sizeof(bt_bdaddr_t));

    if (g->ble_get_specific_record != NULL)
    {
        found = g->ble_get_specific_record(conn == NULL ? 0xFFFF : conn->connhdl, (ble_bdaddr_t *)&peer_le_addr, &pair_info);
    }
    else
    {
        found = nv_record_blerec_get_paired_dev_from_addr(&pair_info, &peer_le_addr);
    }

    if (found == true && out)
    {
        app_ble_parse_record_to_bond_device(&pair_info, out);
    }

    return found;
}

#ifdef BLE_GATT_CLIENT_CACHE
static bool app_ble_check_handle_in_range_of_128_service(uint16_t handle, const gattc_128_service_t *p_128_services, uint8_t services_128_size)
{
    uint8_t i = 0;

    for (i = 0; i < services_128_size; i++)
    {
        if (p_128_services[i].start_handle != 0 &&
            handle >= p_128_services[i].start_handle && handle <= p_128_services[i].end_handle)
        {
            return true;
        }
    }

    return false;
}

static void app_ble_parse_nv_client_cache(const gattc_client_cache_t *p_input, gatt_client_cache_t *p_out)
{
    uint8_t i = 0;
    bool cccd_present = false;
    gatt_char_cache_t *p_char_out = NULL;
    gatt_128_char_cache_t *p_char_128_out = NULL;
    const gattc_char_t *p_char_in = NULL;
    const gattc_128_char_t *p_char_128_in = NULL;
    // Parse databse hash
    memcpy(p_out->peer_database_hash, p_input->peer_database_hash, sizeof(p_input->peer_database_hash));
    // Parse services
    for (i = 0; i < ARRAY_SIZE(p_out->peer_service); i++)
    {
        p_out->peer_service[i] = *(gatt_service_cache_t *)&p_input->peer_service[i];
    }
    // Parse 128 services
    for (i = 0; i < ARRAY_SIZE(p_out->peer_128_service); i++)
    {
        p_out->peer_128_service[i] = *(gatt_128_service_cache_t *)&p_input->peer_128_service[i];
    }

    /// TODO: Assume that all character only have mo more than one cccd handle
    // Parse characters
    for (i = 0; i < ARRAY_SIZE(p_out->peer_character); i++)
    {
        p_char_in = &p_input->peer_character[i];
        p_char_out = &p_out->peer_character[i];
        // char prop indicate that cccd is present
        cccd_present = (p_char_in->char_prop & (GATT_NTF_PROP | GATT_IND_PROP));
        p_char_out->char_end_handle = p_char_in->char_value_handle + (cccd_present ? 1 : 0);
        p_char_out->char_cccd_handle = cccd_present ? p_char_out->char_end_handle : 0;
        p_char_out->char_value_handle = p_char_in->char_value_handle;
        p_char_out->service_seqn = p_char_in->service_seqn;
        p_char_out->reliable_write = p_char_in->reliable_write;
        p_char_out->writable_aux = p_char_in->writable_aux;
        p_char_out->char_prop = p_char_in->char_prop;
        p_char_out->char_uuid = p_char_in->char_uuid;
        p_char_out->in_128_service =
                app_ble_check_handle_in_range_of_128_service(p_char_out->char_value_handle,
                                                             p_input->peer_128_service,
                                                             ARRAY_SIZE(p_input->peer_128_service));
    }
    // Parse 128 characters
    for (i = 0; i < ARRAY_SIZE(p_out->peer_128_character); i++)
    {
        p_char_128_in = &p_input->peer_128_character[i];
        p_char_128_out = &p_out->peer_128_character[i];
        // char prop indicate that cccd is present
        cccd_present = (p_char_128_in->char_prop & (GATT_NTF_PROP | GATT_IND_PROP));
        p_char_128_out->char_end_handle = p_char_128_in->char_value_handle + (cccd_present ? 1 : 0);
        p_char_128_out->char_cccd_handle = cccd_present ? p_char_128_out->char_end_handle : 0;
        p_char_128_out->char_value_handle = p_char_128_in->char_value_handle;
        p_char_128_out->service_seqn = p_char_128_in->service_seqn;
        p_char_128_out->reliable_write = p_char_128_in->reliable_write;
        p_char_128_out->writable_aux = p_char_128_in->writable_aux;
        p_char_128_out->char_prop = p_char_128_in->char_prop;
        memcpy(p_char_128_out->uuid_128_le, p_char_128_in->uuid_128_le,
                                                sizeof(p_char_128_out->uuid_128_le));
        p_char_128_out->in_128_service =
                app_ble_check_handle_in_range_of_128_service(p_char_128_out->char_value_handle,
                                                             p_input->peer_128_service,
                                                             ARRAY_SIZE(p_input->peer_128_service));
    }
}
#endif /* BLE_GATT_CLIENT_CACHE */

static void app_ble_check_load_client_cache(const gap_conn_item_t *conn)
{
#ifdef BLE_GATT_CLIENT_CACHE
    BLE_ADDR_INFO_T addr = {.addr_type = (conn->sec.peer_type & 0x01)};
    memcpy(addr.addr, conn->sec.peer_addr.address, sizeof(bt_bdaddr_t));
    const gattc_nv_cache_t *p_nv_cache = nv_record_ble_gatt_cache_get(&addr);

    if (p_nv_cache == NULL)
    {
        CO_LOG_ERR_0(BT_STS_NOT_FOUND);
        DUMP8("%02x ", conn->sec.peer_addr.address, sizeof(bt_bdaddr_t));
        return;
    }

    // Means load cache needed
    gatt_client_cache_t *client_cache = (gatt_client_cache_t *)bes_bt_buf_malloc(sizeof(gatt_client_cache_t));

    if (client_cache != NULL)
    {
        client_cache->client_cache_seqn = p_nv_cache->cache_seqn;
        app_ble_parse_nv_client_cache(&p_nv_cache->client_cache, client_cache);
        gattc_cache_restore_handles(conn->connhdl, client_cache);

        bes_bt_buf_free(client_cache);
    }

#endif
}

static void app_ble_check_load_server_cache(const gap_conn_item_t *conn)
{
    ble_global_t *g = ble_get_global();
    BleDevicePairingInfo pair_info = {0};
    gatt_server_cache_t *server_cache = NULL;
    bool found = false;

    if(!conn)
    {
        TRACE(1, "%s:conn is null", __func__);
        return;
    }

    BLE_ADDR_INFO_T peer_le_addr = {.addr_type = (conn->sec.peer_type & 0x01)};
    memcpy(peer_le_addr.addr, conn->sec.peer_addr.address, sizeof(bt_bdaddr_t));

    if (g->ble_get_specific_record != NULL)
    {
        found = g->ble_get_specific_record(conn->connhdl, (ble_bdaddr_t *)&peer_le_addr, &pair_info);
    }
    else
    {
        found = nv_record_blerec_get_paired_dev_from_addr(&pair_info, &peer_le_addr);
    }

    if (found == true)
    {
        server_cache = (gatt_server_cache_t *)bes_bt_buf_malloc(sizeof(gatt_server_cache_t));

        if (server_cache != NULL)
        {
            app_ble_parse_nv_server_cache(&pair_info.server_cache, server_cache);
            gatts_server_cache_restore(conn->connhdl, server_cache);
            bes_bt_buf_free(server_cache);
        }
    }
}

static void app_ble_get_local_smp_requirements(const gap_conn_item_t *conn, smp_requirements_t *p_requirements)
{
    uint8_t conidx = gap_zero_based_conidx(conn->con_idx);
    ble_global_t *g = ble_get_global();

    // Only adjust LE SMP requirements
    if (conn->conn_type != HCI_CONN_TYPE_LE_ACL)
    {
        return;
    }

#if defined(GFPS_ENABLED) || defined(FINDMY_ENABLED) || defined(CTKD_ENABLE)
    p_requirements->auth_req |= SMP_AUTH_SC_SUPPORT;
#endif

#ifdef GFPS_ENABLED
    // Only in SMP paring start with peer request, we can set gfps flag
    if (gfps_ble_is_connected(conidx) || (conn->smp_conn != NULL && gfps_get_flag()))
    {
        p_requirements->auth_req |= SMP_AUTH_MITM_PROTECT;
        p_requirements->io_cap = SMP_IO_DISPLAY_YES_NO;
        gfps_set_flag(false);
    }
    else
#endif
    {
        p_requirements->auth_req &= ~SMP_AUTH_MITM_PROTECT;
        p_requirements->io_cap = SMP_IO_NO_INPUT_NO_OUTPUT;
    }

#if defined(CTKD_ENABLE)
    uint8_t key_dist = 0;

    if (g->dist_lk_set_cb == NULL)
    {
        key_dist = GAP_KDIST_LINKKEY; // default set LTK in distribution
    }
    else
    {
        key_dist |= g->dist_lk_set_cb();
    }

    if (key_dist & GAP_KDIST_LINKKEY)
    {
        p_requirements->init_key_dist |= GAP_KDIST_LINKKEY;
        p_requirements->resp_key_dist |= GAP_KDIST_LINKKEY;
    }
    else
    {
        p_requirements->init_key_dist &= ~GAP_KDIST_LINKKEY;
        p_requirements->resp_key_dist &= ~GAP_KDIST_LINKKEY;
    }
#endif

    if (g->ble_smp_require_modify != NULL)
    {
        g->ble_smp_require_modify(conn->connhdl, (ble_smp_require_t *)p_requirements);
    }

    // SMP is triggered by local api or pairing rsp
    if (conn != NULL && g->p_smp_req[conidx] != NULL)
    {
        *(ble_smp_require_t *)p_requirements = *g->p_smp_req[conidx];
        bes_bt_buf_free(g->p_smp_req[conidx]);
        g->p_smp_req[conidx] = NULL;
    }

    if (conn->smp_conn != NULL)
    {
        app_sysfreq_req(APP_SYSFREQ_USER_SMP_PAIRING, APP_SYSFREQ_208M);
    }
}

#ifdef BLE_GATT_CLIENT_CACHE
static void app_ble_build_nv_client_cache(const gatt_client_cache_t *p_input, gattc_client_cache_t *p_out)
{
    uint8_t i = 0;
    const gatt_char_cache_t *p_char_in = NULL;
    const gatt_128_char_cache_t *p_char_128_in = NULL;
    gattc_char_t *p_char_out = NULL;
    gattc_128_char_t *p_char_128_out = NULL;
    // Store databse hash
    memcpy(p_out->peer_database_hash, p_input->peer_database_hash, sizeof(p_input->peer_database_hash));
    // Store services
    for (i = 0; i < ARRAY_SIZE(p_out->peer_service); i++)
    {
        p_out->peer_service[i] = *(gattc_service_t *)&p_input->peer_service[i];
    }
    // Store 128 services
    for (i = 0; i < ARRAY_SIZE(p_out->peer_128_service); i++)
    {
        p_out->peer_128_service[i] = *(gattc_128_service_t *)&p_input->peer_128_service[i];
    }
    // Store characters
    for (i = 0; i < ARRAY_SIZE(p_out->peer_character); i++)
    {
        p_char_in = &p_input->peer_character[i];
        p_char_out = &p_out->peer_character[i];
        p_char_out->char_value_handle = p_char_in->char_value_handle;
        p_char_out->service_seqn = p_char_in->service_seqn;
        p_char_out->reliable_write = p_char_in->reliable_write;
        p_char_out->writable_aux = p_char_in->writable_aux;
        p_char_out->char_prop = p_char_in->char_prop;
        p_char_out->char_uuid = p_char_in->char_uuid;
    }
    // Store 128 characters
    for (i = 0; i < ARRAY_SIZE(p_out->peer_128_character); i++)
    {
        p_char_128_in = &p_input->peer_128_character[i];
        p_char_128_out = &p_out->peer_128_character[i];
        p_char_128_out->char_value_handle = p_char_128_in->char_value_handle;
        p_char_128_out->service_seqn = p_char_128_in->service_seqn;
        p_char_128_out->reliable_write = p_char_128_in->reliable_write;
        p_char_128_out->writable_aux = p_char_128_in->writable_aux;
        p_char_128_out->char_prop = p_char_128_in->char_prop;
        memcpy(p_char_128_out->uuid_128_le, p_char_128_in->uuid_128_le, sizeof(p_char_128_out->uuid_128_le));
    }
}
#endif /* BLE_GATT_CLIENT_CACHE */

static void app_ble_add_nv_ble_device(const gap_conn_item_t *conn, const gap_bond_sec_t *bond)
{
    ble_global_t *g = ble_get_global();
    BleDevicePairingInfo *p_record = NULL;

    // No public addr or irk distributed, random addr store in nv makes no sense
    if ((bond->peer_irk_distributed == false) &&
            ((conn != NULL && conn->peer_type == BT_ADDR_TYPE_PUB_IA) ||
                ((bond->peer_type & 0x01) != BT_ADDR_TYPE_PUBLIC)))
    {
        return;
    }

    p_record = (BleDevicePairingInfo *)bes_bt_buf_malloc(sizeof(BleDevicePairingInfo));

    if (p_record == NULL)
    {
        CO_LOG_ERR_0(BT_STS_NO_RESOURCES);
        return;
    }

    memset(p_record, 0, sizeof(*p_record));
    p_record->peer_addr.addr_type = (bond->peer_type & 0x01);
    memcpy(p_record->peer_addr.addr, bond->peer_addr.address, sizeof(bt_bdaddr_t));

    if (g->ble_add_record_modify != NULL)
    {
        g->ble_add_record_modify(conn ? conn->connhdl : 0xFFFF, p_record);
    }

    p_record->EDIV = CO_COMBINE_UINT16_LE(bond->ediv);
    p_record->LOCAL_EDIV = CO_COMBINE_UINT16_LE(bond->local_ediv);
    p_record->enc_key_size = bond->enc_key_size;
    p_record->bond_info_bf |= bond->device_bonded ? CO_BIT_MASK(BONDED_STATUS_POS) : 0;
    p_record->bond_info_bf |= bond->peer_irk_distributed ? CO_BIT_MASK(BONDED_WITH_IRK_DISTRIBUTED) : 0;
    p_record->bond_info_bf |= bond->peer_csrk_distributed ? CO_BIT_MASK(BONDED_WITH_CSRK_DISTRIBUTED) : 0;
    p_record->bond_info_bf |= bond->secure_pairing ? CO_BIT_MASK(BONDED_SECURE_CONNECTION) : 0;
    p_record->bond_info_bf |= bond->bonded_with_num_compare ? CO_BIT_MASK(BONDED_WITH_NUM_COMPARE) : 0;
    p_record->bond_info_bf |= bond->bonded_with_passkey_entry ? CO_BIT_MASK(BONDED_WITH_PASSKEY_ENTRY) : 0;
    memcpy(p_record->IRK, bond->peer_irk, GAP_KEY_LEN);
    memcpy(p_record->CSRK, bond->peer_csrk, GAP_KEY_LEN);
    memcpy(p_record->LTK, bond->ltk, GAP_KEY_LEN);
    memcpy(p_record->LOCAL_LTK, bond->local_ltk, GAP_KEY_LEN);
    memcpy(p_record->RANDOM, bond->rand, sizeof(bond->rand));
    memcpy(p_record->LOCAL_RANDOM, bond->local_rand, sizeof(bond->local_rand));

    if (conn != NULL)
    {
        bluetooth_nv_mgr_ble_record_add(BLE_NV_REC_ADD_SMP_NEW_PAIRED, p_record);
    }
    else
    {
        bluetooth_nv_mgr_ble_record_add(BLE_NV_REC_ADD_CTKD_OVER_BREDR, p_record);
    }

    bes_bt_buf_free(p_record);
}

static void app_ble_gatt_server_cache(const gap_conn_item_t *conn, const gatt_server_cache_t *cache)
{
    if (conn == NULL || cache == NULL)
    {
        return;
    }

    ble_global_t *g = ble_get_global();
    BleDevicePairingInfo *p_record = (BleDevicePairingInfo *)bes_bt_buf_malloc(sizeof(BleDevicePairingInfo));
    bool found = false;

    if (p_record == NULL)
    {
        return;
    }

    if (g->ble_get_specific_record != NULL)
    {
        found = g->ble_get_specific_record(conn->connhdl, (ble_bdaddr_t *)&p_record->peer_addr, p_record);
    }
    else
    {
        found = true;
    }

    if (found == true)
    {
        app_ble_build_nv_server_cache(cache, &p_record->server_cache);
        nv_record_ble_server_cache_add(&p_record->peer_addr, &p_record->server_cache);
    }

    bes_bt_buf_free(p_record);
}

static void app_ble_gatt_client_cache(const gap_conn_item_t *conn, const gatt_client_cache_t *cache)
{
#ifdef BLE_GATT_CLIENT_CACHE
    BLE_ADDR_INFO_T peer_addr = {0};
    peer_addr.addr_type = (conn->sec.peer_type & 0x01);
    memcpy(peer_addr.addr, conn->sec.peer_addr.address, sizeof(bt_bdaddr_t));

    gattc_client_cache_t *p_client_cache = NULL;

    if (cache != NULL)
    {
        p_client_cache = (gattc_client_cache_t *)bes_bt_buf_malloc(sizeof(*p_client_cache));

        if (p_client_cache == NULL)
        {
            CO_LOG_ERR_0(BT_STS_NO_RESOURCES);
        }
        else
        {
            app_ble_build_nv_client_cache(cache, p_client_cache);
            // Write to nv
            nv_record_ble_gatt_cache_add(&peer_addr,
                                         cache->client_cache_seqn,
                                         p_client_cache);
        }

        if (p_client_cache)
        {
            bes_bt_buf_free(p_client_cache);
        }
    }
#endif
}

bool app_ble_get_nv_bt_device_by_addr(const bt_bdaddr_t *bd_addr, gap_bredr_sec_t *out)
{
    nvrec_btdevicerecord *record = NULL;
    if (!nv_record_btdevicerecord_find(bd_addr, &record))
    {
        out->peer_addr = record->record.bdAddr;
        out->trusted = record->record.trusted;
        out->for_bt_source = record->record.for_bt_source;
        out->key_type = record->record.keyType;
        memcpy(out->link_key, record->record.linkKey, GAP_KEY_LEN);
        memcpy(out->cod, record->record.cod, sizeof(record->record.cod));
        return true;
    }
    return false;
}

static void app_ble_get_local_specific_irk_ia(const gap_conn_item_t *conn, uint8_t *irk, ble_bdaddr_t *ia)
{
    ble_global_t *g = ble_get_global();
    uint8_t *p_irk_specific = NULL;
    ble_bdaddr_t ia_addr;
    bt_bdaddr_t *p_ia_specific = NULL;

    if (irk)
    {
        if (conn != NULL && g->ble_get_specific_irk_ia != NULL)
        {
            g->ble_get_specific_irk_ia(conn->connhdl, &p_irk_specific, NULL);
        }

        memcpy(irk, p_irk_specific != NULL ? p_irk_specific : gap_get_local_irk(), GAP_KEY_LEN);
    }

    if (ia && conn)
    {
        if (conn->conn_type == HCI_CONN_TYPE_LE_ACL)
        {
            *ia = gap_conn_own_identity_address(conn);

            if (conn != NULL && g->ble_get_specific_irk_ia != NULL)
            {
                g->ble_get_specific_irk_ia(conn->connhdl, NULL, &p_ia_specific);
            }

            if (p_ia_specific != NULL)
            {
                memcpy(ia->addr, p_ia_specific, sizeof(ia_addr.addr));
            }
        }
        else
        {
            *ia = gap_ctkd_get_le_identity_address(conn);
        }
    }
}

bool app_ble_check_need_start_mtu_exchange(const gap_conn_item_t *conn)
{
    if (conn->conn_flag.is_central)
    {
        return true;
    }

    return false;
}

#ifdef CFG_SEC_CON
uint8_t ble_public_key[64];
uint8_t ble_private_key[32];
const uint8_t bes_demo_Public_key[64] = { //MSB->LSB
    0x3E,0x08,0x3B,0x0A,0x5C,0x04,0x78,0x84,0xBE,0x41,
    0xBE,0x7E,0x52,0xD1,0x0C,0x68,0x64,0x6C,0x4D,0xB6,
    0xD9,0x20,0x95,0xA7,0x32,0xE9,0x42,0x40,0xAC,0x02,
    0x54,0x48,0x99,0x49,0xDA,0xE1,0x0D,0x9C,0xF5,0xEB,
    0x29,0x35,0x7F,0xB1,0x70,0x55,0xCB,0x8C,0x8F,0xBF,
    0xEB,0x17,0x15,0x3F,0xA0,0xAA,0xA5,0xA2,0xC4,0x3C,
    0x1B,0x48,0x60,0xDA
};
const uint8_t bes_demo_private_key[32]= { //MSB->LSB
     0xCD,0xF8,0xAA,0xC0,0xDF,0x4C,0x93,0x63,0x2F,0x48,
     0x20,0xA6,0xD8,0xAB,0x22,0xF3,0x3A,0x94,0xBF,0x8E,
     0x4C,0x90,0x25,0xB3,0x44,0xD2,0x2E,0xDE,0x0F,0xB7,
     0x22,0x1F
};
#endif

#if defined(__GATT_OVER_BR_EDR__)
extern bool btif_is_gatt_over_br_edr_allowed_send(uint8_t conidx);
bool app_ble_is_gatt_allow_data_send(const gap_conn_item_t *conn)
{
#ifdef IBRT
#ifdef FREEMAN_ENABLED_STERO
    return true;
#else
    if (conn->conn_type == HCI_CONN_TYPE_BT_ACL)
    {
        return btif_is_gatt_over_br_edr_allowed_send(conn->con_idx);
    }
#endif
#else
    return true;
#endif

    return true;
}
#endif

const gap_ext_func_cbs_t gap_external_func_cbs =
{
    .nv_get_local_csrk = app_ble_get_nv_local_csrk,
    .nv_get_ble_device_by_index = app_ble_get_nv_ble_device_by_index,
    .nv_get_ble_device_by_addr = app_ble_get_nv_ble_device_by_addr,
    .nv_add_ble_device = app_ble_add_nv_ble_device,
    .smp_get_upper_requirements = app_ble_get_local_smp_requirements,
    .nv_get_bt_device_by_addr = app_ble_get_nv_bt_device_by_addr,
    .gap_get_upper_specific_irk_ia = app_ble_get_local_specific_irk_ia,
    .gatt_need_start_mtu_exchange = app_ble_check_need_start_mtu_exchange,
#if defined(__GATT_OVER_BR_EDR__)
    .gatt_allow_data_send = app_ble_is_gatt_allow_data_send,
#endif
};

/**
 * GAP Service
 * ===========
 *
 * The GATT Server shall contain the GAP Service. A device shall have only one instance
 * of the GAP Service in the GATT Server. The GAP Service is a GATT based service with
 * the service UUID as <<GAP Service>>.
 *
 * GAP Service shall be Mandatory support by LE Peripheral, LE Central, and BR/EDR/LE type
 * devices. It is optional for non BR/EDR/LE type devices. The suport for LE Broadcaster and
 * LE Observer devices is exclueded.
 *
 * The charactersitics requirements for the GAP Service in each of the GAP roles:
 *      Characteristics                         BR/EDR GAP Role     LE Peripheral       LE Central
 *      1) Device Name                          C1                  M                   M
 *      2) Appearance                           C1                  M                   M
 *      3) Peripheral Prefer Conn Prams         O                   O                   E
 *      4) Central Address Resolution           O                   C3                  C2
 *      5) Resolvable Private Address Only      O                   C3                  C2
 *      6) Encrypted Data Key Material          O                   O                   E
 *      7) LE GATT Security Levels              E                   O                   O
 *  C1 - Mandatory for BR/EDR/LE type devices, otherwise O
 *  C2 - Mandatory if LL privacy is supported, otherwise E
 *  C3 - Opitonal if LL privacy is supported, otherwise E
 *
 * A device that supports multiple GAP roles contains all the characteristics to meet the requirements
 * for the supported roles. The device must continue to expose the characteristics when the device
 * is operating in the role in which the characteristics are not valid.
 *
 * ## Device Name Characteristic (M)
 *
 * The Device Name shall contain the name of the device as an UTF-8 string. When the device is discoverable,
 * the Device Name shall be readable without authentication or authorization. When the device is not
 * discoverable, the Device Name should not be readable. The Device Name value may be writable. If writable,
 * authentication and authorization may be defined by a higher layer specification or be implementation
 * specific.
 *
 * The Device Name value shall be 0 to 248 octets in length. A device shall have only one instance of the
 * Device Name Charactersitic.
 *
 * Device Name Char Value Declaration
 * Attribute Handle     Attribute Type      Attribute Value     Attribute Permission
 * 0xMMMM               0x2A00              Device Name         Readable W/O authen or author when discoverable
 *                                                              Optionally writable, authen and author may be defined
 */

GATT_DECL_CHAR(g_gap_service_device_name,
    GATT_CHAR_UUID_DEVICE_NAME,
    GATT_RD_REQ|GATT_WR_REQ, // Readable only when discoverable, Optional writable
    ATT_SEC_NONE);

/**
 * ## Appearance Characteristic (M)
 *
 * The Apperance defines the representation of the external appearance of the device. This
 * enable the discovering device to represent the device to the user using an icon, or a
 * string, or similar. The Appearance value shall be readable without authentication or
 * authorization. The value may be writable, if writable, authen and author may be defined
 * by a higher layer specification or be implementation specific. A device shall have only
 * one instance of the Appearance characteristic.
 *
 * Appearance Char Value Declaration
 * Attribute Handle     Attribute Type      Attribute Value                 Attribute Permission
 * 0xMMMM               0x2A01              appearance enum value (2-byte)  Readable W/O authen or author
 *                                                                          Optionally writable, authen and author may be defined
 */

GATT_DECL_CHAR(g_gap_service_appearance,
    GATT_CHAR_UUID_APPEARANCE,
    GATT_RD_REQ|GATT_WR_REQ,
    ATT_SEC_NONE);

/**
 * ## Central Address Resolution Chacteristic (M)
 *
 * The Peripheral should check if the peer device supports address resolution by reading
 * this characteristic before using directed advertising where the target address is set
 * to RPA. This characterisitc defines whether the device supports privacy with address
 * resolution.
 *
 * The value shall be 1 octet in length. A device shall have only one instance of the
 * characteristic. If the characteristic is not present, then it shall be assumed that
 * the Central Address Resolution is not supported.
 *
 * Central Address Resolution Char Value Declaration
 * Attribute Handle     Attribute Type      Attribute Value     Attribute Permission
 * 0xMMMM               0x2AA6              Support (1-byte)    Read Only W/O authen and author
 *
 */

GATT_DECL_CHAR(g_gap_service_central_addr_reso_supp,
    GATT_CHAR_UUID_CENTRAL_ADDRESS_RESOLUTION,
    GATT_RD_REQ,
    ATT_SEC_NONE);

/**
 * ## Resolvable Private Address Only Characteristic (O)
 *
 * The device should check if the peer will only use RPA after bonding by reading this
 * characteristic, in order to determine if it will satisfy its privacy mode as defined
 * in Section GAP 10.7.
 *
 * This characteristic defines whether the device will only use RPAs as local addresses.
 * The value shall be 1 octet in length: 0x00 only RPAs will be used as local addresses
 * after bonding; 0x01 ~ 0xFF reserved for future use.
 *
 * A device shall have only one instance of this characteristic. If the characteristic
 * is not present, then it cannot be assumed that only RPAs will be used over the air.
 *
 * Resolvable Private Address Only Char Value Declaration
 * Attribute Handle     Attribute Type      Attribute Value         Attribute Permission
 * 0xMMMM               0x2AC9              RPA Only (1-byte)       Read Only W/O authen and author
 *
 */

GATT_DECL_OPTIONAL_CHAR_WITH_CONST_VALUE(g_gap_service_use_only_rpa_after_bonding,
    GATT_CHAR_UUID_RPA_ONLY,
    GATT_RD_REQ,
    ATT_SEC_NONE,
    0x00 /* 0 - only RPAs will be used as local addresses after bonding */ );

/**
 * ## Peripheral Preferred Connection Parameters (PPCP) Chacteristic (O)
 *
 * The PPCP characteristic contains the preferred connection parameters of the Peripheral.
 * The PPCP value shall be readable, authen or author may be defined by a higher layer
 * specification or be implementation specific. The PPCP value shall not be writable.
 *
 * The PPCP value shall be 8 octets in length. A device shall have oly one instance of the
 * PPCP characteristic. Each field of the value shall have the same meaning and requirements
 * as the field of LL_CONNECTION_PARAM_REQ PDU with the same name, or shall contain the
 * value 0xFFFF which indicates that no specific value is requested.
 *
 * PPCP Char Value Declaration
 * Attribute Handle     Attribute Type      Attribute Value             Attribute Permission
 * 0xMMMM               0x2A04              Parameters (8-byte)         Read Only, authen or author may be defined
 *
 */

GATT_DECL_OPTIONAL_CHAR(g_gap_service_periph_prefer_conn_params,
    GATT_CHAR_UUID_PERIPH_PREFER_CONN_PARAMS,
    GATT_RD_REQ,
    ATT_SEC_NONE);

/**
 * ## LE GATT Security Levels Characterisitc (O)
 *
 * This Char shall contian the highest security requirements of the GATT server when
 * operating on a LE connection. The value of the LE GATT Security Levels Char shall be
 * static during a connection. A device shall have at most one instance of a LE GATT
 * Security Levels Char.
 *
 * The Attribute Value is a sequence of Security Level Requirements, each with the type uint8[2].
 * Each security Level Requirement consits of a Security Mode field followed by a Secuirty Level
 * field. The Security Mode and Security Level shall be expressed as the same number as used in
 * ther definitions; e.g., mode 1 is represented as 0x01 and level 4 is represended as 0x04.
 *
 * Meeting any one of the security requirements provided for a given mode shall be sufficient for
 * the GATT server to allow the GATT client to use all the GATT procedures permitted by the
 * characteristic properties for all characteristics on the server operating in that mode.
 *
 * If any one of the security requirements specified in the LE GATT Security Levels is met, no GATT
 * procedure will fail for a secuirity-related reason. For example, the attribute value 0x01 0x04
 * for the LE GATT Security Levels characteristic means that the GATT server requires level 4
 * when operating in security mode 1 on a LE connection.
 *
 * LE GATT Security Levels Char Value Declaration
 * Attribute Handle     Attribute Type      Attribute Value         Attribute Permission
 * 0xMMMM               0x2BF5              one or more sec levels  Read Only No enc No Authen No Author
 *
 * LE security mode 1 levels:
 *      Level 1 - No security (no authentication and no encryption)
 *      Level 2 - Unauthenticated pairing with encryption
 *      Level 3 - Authenticated pairing with encryption
 *      Level 4 - Authenticated LE Secure Connections pairing with encryption using a 128-bit enc key
 * LE security mode 2 levels:
 *      shall not be used when a connection is operating in mode 1 level 2/3/4
 *      Level 1 - Unauthenticated pairing with data signing
 *      Level 2 - Authenticated pairing with data signing
 * LE security mode 3 levels:
 *      shall be used to broadcast a BIG in an ISO Broadcaster or receive a BIS in a Synced Receiver
 *      Level 1 - No security (no authentication and no encryption)
 *      Level 2 - Use of unauthenticated Broadcast_Code
 *      Level 3 - Use of authenticated Broadcast_Code
 *
 */

typedef struct {
    uint8_t mode;
    uint8_t level;
} gatt_security_level_t;

GATT_DECL_OPTIONAL_CHAR(g_gap_service_le_gatt_security_levels,
    GATT_CHAR_UUID_LE_GATT_SECURITY_LEVELS,
    GATT_RD_REQ,
    ATT_SEC_NONE);

/**
 * ## Encrypted Data Key Material (O)
 *
 * The Encrypted Data Key Material chanaracterisc allows advertising data associated with
 * the GAP service to be decrypted and authenticated using the key material. The character
 * shall not be writtable. It shall only be readable when authenticated and authorized.
 * When read, the key material is stored on the local device. The key material may be
 * discarded at any time on a local device.
 *
 * The Encrypted Data Key Material characteristic may support indications. When authenticated
 * and authorized, if the Encrypted Data Key characteristic value changes and the client
 * has configured the characteristic for indications, thne the characteristic shall be
 * indicated by the server to the client.
 *
 * This characteristic can be used by other services to allow those services to expose
 * separate key material for encrypted advertising data used by those services.
 *
 * Encrypted Data Key Material Char Value Declaration
 * Attribute Handle     Attribute Type      Attribute Value         Attribute Permission
 * 0xMMMM               0x2B88              Key Material            Read/Indicate when auth and author, Not writable
 *
 * The Key Material is composed of a 128-bit value that is used as the session key, and
 * a 64-bit vlaue that is used as the IV for encrypting and authenticating the Encrypted
 * Data. The server should update the Key Material periodically.
 *
 */

GATT_DECL_OPTIONAL_CHAR_WITH_FLAG(g_gap_service_enc_data_key_material,
    GATT_CHAR_UUID_ENCRYPTED_DATA_KEY_MATERIAL,
    GATT_RD_REQ|GATT_IND_PROP,
    ATT_RD_MITM_AUTH|ATT_RD_AUTHOR,
    ATT_FLAG_IND_AUTH);

// GAP Service Declaration

GATT_DECL_PRI_SERVICE(g_gap_service, GATT_UUID_GAP_SERVICE);

static const gatt_attribute_t g_gap_service_attr_list[] = {
    /* Service */
    gatt_attribute(g_gap_service),
    /* Characteristics */
    gatt_attribute(g_gap_service_device_name),
    gatt_attribute(g_gap_service_appearance),
    gatt_attribute(g_gap_service_central_addr_reso_supp),
    gatt_attribute(g_gap_service_le_gatt_security_levels),
#if (BLE_USE_ENC_DATA_KEY_MATERIAL_CHAR == 1)
    gatt_attribute(g_gap_service_enc_data_key_material),
#endif
#if (BLE_LOCAL_USE_RPA_ONLY_AFTER_BONDING == 1)
    gatt_attribute(g_gap_service_use_only_rpa_after_bonding),
#endif
    gatt_attribute(g_gap_service_periph_prefer_conn_params), // keep this as last characteristic
};

/**
 * Gatt Service
 * ============
 *
 * Only one instance of the Gatt Service shall be exposed on a Gatt Server. Below are
 * the characteristics may be present in the server and may be supported be the client.
 *      1) Service Changed
 *         - M if server can add/change/remove service, otherwise O
 *         - M for client
 *      2) Client Supported Features
 *         - M if server support Database Hash and Service Changed or
 *             if server support EATT or Multiple Variable Length Notification, otherwise excluded
 *         - O for client
 *      3) Database Hash
 *         - O for server
 *         - O for client
 *      4) Server Supported Features
 *         - M if server support any of the features in <server supported fetures>, otherwise O
 *         - O for client
 *
 * ## Service Changed Characteristic (M)
 *
 * The Service Changed characteristic is a control-point attribute that shall be used to indicate
 * to connected device that services have changed. It shall be used to indicate to clients that
 * have a trusted relationship (i.e. bond) with the server when GATT based services have changed
 * when they re-connect to the server.
 *
 * This Char Value shall be configured to be indicated using the CCCD by a client. Indications
 * caused by changes to this Char Value shall be considered lost if the client has erroneously
 * not enabled indications in the CCCD.
 *
 * The Char Value is two 16-bit Attribute Handles concatenated together indicating the beginning
 * and ending Attribute Handles affected by an addition, removeal, or modification to a service
 * on the server. A change to a characteristic value is not considered a modification of the
 * service. If a change has been made to any of the <<Gatt Service>> characteristic values other than
 * the Service Changed Char Value and the Client Supported Features Char Value, i.e., the Database Hash
 * Char Value or Server Supported Features Char Value is changed, the range shall also include the
 * beginning and ending Attribute Handle for the <<Gatt Service>>.
 *
 * A GATT based service is considered modified if the binding of the Attribute Handles to the
 * associated Attributes grouped within a server definition are changed. Any change to the <<Gatt
 * Service>> definition characteristic values other than the Service Changed Char Value and
 * the Client Supported Features Char Value themselves shall also be considered a modification.
 *
 * The Service Changed characteristic Attribute Handle on the server shall not change if the
 * server has a trusted relationship with any client.
 *
 * The Attribute information that shall be cached by a client is the Attribute Handles of all server
 * attributes and the <<GATT Service>> characteristics values.
 *
 * There shall be only one instance of the Service Changed within the Gatt Service definition.
 * A Service Changed Char Value shall exist for each client with a trusted relationship.
 *
 * If GATT based services on the server cannot be changed during the usable lifetime of the device,
 * the Service Changed characteristic shall not exist on the server and the client does not need
 * to need to ever perform service discovery after the initial service discovery for that server.
 *
 * If the Service Changed Char exists on the server, the Characteristic Value Indication support
 * on the server is mandatory. The client shall support Characteristic Value Indication of the
 * Service Changed Char.
 *
 * Service Changed Characteristic Declaration
 * Attribute Handle     Attribute Type      Attribute Value             Attribute Permission
 * 0xNNNN               0x2803              Char Prop = 0x20            Read Only
 *                                          0xMMMM Char Value Handle
 *                                          0x2A05 - <Service Changed>
 * Service Changed Char Value Declaration
 * Attribute Handle     Attribute Type      Attribute Value             Attribute Permission
 * 0xMMMM               0x2A05              0xSSSS Start Affected Range Not Readable, Not Writable
 *                                          0xTTTT End Affected Range
 */

GATT_DECL_CHAR(g_gatt_service_changed,
    GATT_CHAR_UUID_SERVICE_CHANGED,
    GATT_IND_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_gatt_service_changed_cccd,
    ATT_SEC_NONE);

typedef struct {
    uint16_t start_affected_handle;
    uint16_t end_affected_handle;
} gatt_service_changed_t;

/**
 * ## Client Supported Features Characteristic (M)
 *
 * The Client Support Features characteristic is used by the client to inform the server which
 * features are supported by the client. If the char exists on the server, the client may update
 * the Client Support Features bit field. If a client feature bit is set by a client and the
 * server supports that feature, the server shall fulfill all requirements associated with
 * this feature when communicating with this client. If a client feature bit is not set by the
 * client, then the server shall not use any of the features associated with that bit when
 * communicating with this client.
 *
 * Client Support Features Characteristic Declaration
 * Attribute Handle     Attribute Type      Attribute Value             Attribute Permission
 * 0xNNNN               0x2803              Char Prop = 0x0A            Read Only
 *                                          0xMMMM Char Value Handle
 *                                          0x2B29
 * Client Support Features Char Value Declaration
 * Attribute Handle     Attribute Type      Attribute Value             Attribute Permission
 * 0xMMMM               0x2B29              0xXX...XX -                 Readable, Writable
 *                                          Variable Length Client Features
 *
 * The Client Supported Features Char Value is an array of octets, each of which is a bit field.
 * Below is the allocation of these bits, all bits not listed are reserved for future use. The
 * array should not have any trailing zero octets. If any octet does not appear in the attrubte
 * value because it is too short, the server shall behave as if that octet were present with a
 * value of zero. The default value shall be all bits set to zero.
 *
 * There shall be only one instance of the Client Supported Features characteristic within the
 * Gatt Service definition. A Client Supported Features Char Value shall exist for each connected
 * client. For clients with a trusted relationship, the Char Value shall be persistent across
 * connections. For clients without a trusted relationship the Char Value shall be set to the
 * default value at each connection.
 *
 * The Attribute Handle of the Client Supported Features characteristic on the server shall
 * not change during a connection or if the server has a trusted relationship with any client.
 *
 * A client shall not clear any bits it has set. The server shall respond to any such request
 * with the Error Code of Value Not Allowed (0x13).
 *
 */

GATT_DECL_CHAR(g_gatt_client_supp_features,
    GATT_CHAR_UUID_CLIENT_SUPP_FEATURES,
    GATT_RD_REQ|GATT_WR_REQ,
    ATT_SEC_NONE);

/**
 * ## Server Supported Features Characteristic (M)
 *
 * The Server Supported features characteristic is a read-only characteristic that shall be used
 * to indicate support for server features. The server shall set a bit only if the corresponding
 * feature is supported. There shall be only one instance of the Server Supported Features
 * characteristic within the Gatt Service definition.
 *
 * Server Support Features Characteristic Declaration
 * Attribute Handle     Attribute Type      Attribute Value             Attribute Permission
 * 0xNNNN               0x2803              Char Prop = 0x02            Read Only
 *                                          0xMMMM Char Value Handle
 *                                          0x2B3A
 * Server Support Features Char Value Declaration
 * Attribute Handle     Attribute Type      Attribute Value             Attribute Permission
 * 0xMMMM               0x2B3A              0xUU - Features             Read Only
 *
 * The Server Supported Features Char Value is an array of octets, each of which is a bit field.
 * Below is the allocation of these bits, all bits not listed are reserved for future use. The
 * array should not have any trailing zero octets.
 *
 * If any octet does not appear in the attribute value because it is too short, the client shall
 * behave as if that octet were presnet with the value of zero.
 *
 */

GATT_DECL_CHAR(g_gatt_server_supp_features,
    GATT_CHAR_UUID_SERVER_SUPP_FEATURES,
    GATT_RD_REQ,
    ATT_SEC_NONE);

/**
 * Database Hash Characteristic (O)
 *
 * The Database Hash characteristic contains the result of a hash function applied to the service
 * definitions in the GATT database. The client may read the characteristic at any time to
 * determine if services have been added, removed, or modified. If any of the input fields to the
 * hash function have changed, the server shall calculate a new Database Hash and update the
 * characteristic value. The Database Hash characteristic is a read-only attribute.
 *
 * There is only one instance of the Database Hash characteristic within the Gatt Service
 * definition. The same Database Hash value is used for all clients, whether a trusted
 * relationship exists or not.
 *
 * In order to read the value of this characterisitc the client shall always use the
 * Gatt Read Using Char UUID. The Starting Handle should be set to 0x0001 and the Ending
 * Handle should be set to 0xFFFF. If a client reads the value of this characteristic
 * while the server is re-calculating the hash following a change to the database, the
 * server shall return the new hash, delaying its response until it is available.
 *
 * Database Hash Characteristic Declaration
 * Attribute Handle     Attribute Type      Attribute Value             Attribute Permission
 * 0xNNNN               0x2803              Char Prop = 0x02            Read Only
 *                                          0xMMMM Char Value Handle
 *                                          0x2B2A
 * Database Hash Char Value Declaration
 * Attribute Handle     Attribute Type      Attribute Value             Attribute Permission
 * 0xMMMM               0x2B2A              uint128 database hash       Read Only
 *
 * Database Hash Calaculation
 *
 * The Database Hash shall be cacluated according to RFC-4493. This RFC defines
 * the Cipher-based Message Authentication Code (CMAC) using AES-128 as the
 * block cipher function, also known as AES-CMAC.
 *
 * The inputs to AES-CMAC are:
 *      - m is the variable length data to be hashed
 *      - k is the 128-bit key, which shall be all zero
 *
 * The 128-bit databash hash is generated as AES-CMACk(m), where m is calculated as:
 *
 * In ascending order of attribute handles, starting with the first handle,
 * concatenate the fields Attribute Handle, Attribute Type, and Attribute Value
 * if the attribute has one of the following types: <<Primary Service>>, <<Secondary
 * Service>>, <<Included Service>>, <<Characteristic>>, or <<Characteristic Extended
 * Properties>>; concatenate the fields Attribute Handle and Attribute Type if the
 * attribute has one of the following types: <<Characteristic User Description>>,
 * <<Client Characteristic Configuration>>, <<Server Characteristic Configuration>>,
 * <<Characteristic Format>>, or <<Charactersitic Aggregate Format>>; and ignore the
 * attribute if it has any other type, such attributes are not parot of the concatenation.
 *
 * For each Attribute Handle, the fields shall be concatenated in the order given above.
 * The byte order used for each field or sub-field value shall be little-endian. If a
 * field contains sub-fields, the subfields shall be concatenated in the order they
 * appear. For instance, a characteristic declaration value of {0x02, 0x0005, 0x2A00}
 * is represented after contenation as 02 05 00 00 2A.
 *
 * If the length of m is not a multiple of the AES-CMAC block length of 128 bits,
 * padding shall be applied as specified in RFC-4493 Section 2.4.
 *
 */

GATT_DECL_OPTIONAL_CHAR(g_gatt_server_database_hash,
    GATT_CHAR_UUID_DATABASE_HASH,
    GATT_RD_REQ,
    ATT_SEC_NONE);

// Gatt Service Declaration

GATT_DECL_PRI_SERVICE(g_gatt_service, GATT_UUID_GATT_SERVICE);

static const gatt_attribute_t g_gatt_service_attr_list[] = {
    /* Service */
    gatt_attribute(g_gatt_service),
    /* Characteristics */
    gatt_attribute(g_gatt_service_changed),
        /* Descriptor */
        gatt_attribute(g_gatt_service_changed_cccd),
    gatt_attribute(g_gatt_client_supp_features),
    gatt_attribute(g_gatt_server_supp_features),
    gatt_attribute(g_gatt_server_database_hash), // keep this as last characteristic
};

#ifndef BLE_ONLY_ENABLED

/**
 * SDP Requirements - GAP Service
 *
 * A BR/EDR or BR/EDR/LE device that supports a GATT Server accessible over the
 * BR/EDR physical and that supports only one of ATT or EATT shall public the
 * SDP record shown below first table; if both ATT and EATT are supported, the
 * device shall publish the SDP record shown below second table.
 *
 * The GAP Service Start Handle shall be set to the attribute handle of the
 * <<GAP Service>> service declaration. The GAP Service End Handle shall be set
 * to the attribute handle of the last attribute within the <<GAP Service>>
 * service definition group.
 *
 * If a BR/EDR or BR/EDR/LE device supports a GATT-based service on the BR/EDR
 * transport, the service shall exist in the SDP Server and the GATT Server.
 *
 */

const uint8_t _gap_service_browse_group[] = {
    SDP_DE_DESEQ_8BITSIZE_H2_D(3),
        SDP_DE_UUID_H1_D2,
        0x10, 0x02,
};

const uint8_t _gap_service_class_id_list[] = {
    SDP_DE_DESEQ_8BITSIZE_H2_D(3),
        SDP_DE_UUID_H1_D2,
        SDP_SPLIT_16BITS_BE(GATT_UUID_GAP_SERVICE),
};

const uint8_t _gap_service_protocol_descriptor_list[] = {
    SDP_DE_DESEQ_8BITSIZE_H2_D(19),
        SDP_DE_DESEQ_8BITSIZE_H2_D(6),
            SDP_DE_UUID_H1_D2,
                SERV_UUID_L2CAP,
            SDP_DE_UINT_H1_D2,
                SDP_SPLIT_16BITS_BE(PSM_ATT),
        SDP_DE_DESEQ_8BITSIZE_H2_D(9),
            SDP_DE_UUID_H1_D2,
                SERV_UUID_ATT,
            SDP_DE_UINT_H1_D2,
                SDP_SPLIT_16BITS_BE(GAP_SERVICE_START_HANDLE), // GAP Service Start Handle
            SDP_DE_UINT_H1_D2,
                SDP_SPLIT_16BITS_BE(GAP_SERVICE_END_HANDLE), // GAP Service End Handle
};

#if (EATT_CHAN_SUPPORT == 1)
const uint8_t _gap_service_additional_protocol_descriptor_list[] = {
    SDP_DE_DESEQ_8BITSIZE_H2_D(21),
    SDP_DE_DESEQ_8BITSIZE_H2_D(19),
        SDP_DE_DESEQ_8BITSIZE_H2_D(6),
            SDP_DE_UUID_H1_D2,
                SERV_UUID_L2CAP,
            SDP_DE_UINT_H1_D2,
                SDP_SPLIT_16BITS_BE(PSM_EATT),
        SDP_DE_DESEQ_8BITSIZE_H2_D(9),
            SDP_DE_UUID_H1_D2,
                SERV_UUID_ATT,
            SDP_DE_UINT_H1_D2,
                SDP_SPLIT_16BITS_BE(GAP_SERVICE_START_HANDLE), // GAP Service Start Handle
            SDP_DE_UINT_H1_D2,
                SDP_SPLIT_16BITS_BE(GAP_SERVICE_END_HANDLE), // GAP Service End Handle
};
#endif

static const bt_sdp_record_attr_t _gap_service_sdp_attrs[] = { // list attr id in ascending order
    SDP_DEF_ATTRIBUTE(SERV_ATTRID_SERVICE_CLASS_ID_LIST, _gap_service_class_id_list),
    SDP_DEF_ATTRIBUTE(SERV_ATTRID_PROTOCOL_DESC_LIST, _gap_service_protocol_descriptor_list),
    SDP_DEF_ATTRIBUTE(SERV_ATTRID_BROWSE_GROUP_LIST, _gap_service_browse_group),
#if (EATT_CHAN_SUPPORT == 1)
    SDP_DEF_ATTRIBUTE(SERV_ATTRID_ADDITIONAL_PROT_DESC_LISTS, _gap_service_additional_protocol_descriptor_list),
#endif
};

/**
 * SDP Requirements - Gatt Service
 *
 * A device that supports Gatt Over BR/EDER and only one of ATT or EATT shall publish
 * the SDP record shown below frist table; if both ATT and EATT are supported, the device
 * shall publish the SDP record shown below the second table.
 *
 * The Gatt Service Start Handle shall be set to the attribute handle of the <<Gatt Service>>
 * service declaration. The Gatt Service End Handle shall be set to the attribute handle of
 * the last attribute within the <<Gatt Service>> service definition group.
 *
 */

const uint8_t _gatt_service_browse_group[] = {
    SDP_DE_DESEQ_8BITSIZE_H2_D(3),
        SDP_DE_UUID_H1_D2,
        0x10, 0x02,
};

const uint8_t _gatt_service_class_id_list[] = {
    SDP_DE_DESEQ_8BITSIZE_H2_D(3),
        SDP_DE_UUID_H1_D2,
        SDP_SPLIT_16BITS_BE(GATT_UUID_GATT_SERVICE),
};

const uint8_t _gatt_service_protocol_descriptor_list[] = {
    SDP_DE_DESEQ_8BITSIZE_H2_D(19),
        SDP_DE_DESEQ_8BITSIZE_H2_D(6),
            SDP_DE_UUID_H1_D2,
                SERV_UUID_L2CAP,
            SDP_DE_UINT_H1_D2,
                SDP_SPLIT_16BITS_BE(PSM_ATT),
        SDP_DE_DESEQ_8BITSIZE_H2_D(9),
            SDP_DE_UUID_H1_D2,
                SERV_UUID_ATT,
            SDP_DE_UINT_H1_D2,
                SDP_SPLIT_16BITS_BE(GATT_SERVICE_START_HANDLE), // GATT Service Start Handle
            SDP_DE_UINT_H1_D2,
                SDP_SPLIT_16BITS_BE(GATT_SERVICE_END_HANDLE), // GATT Service End Handle
};

#if (EATT_CHAN_SUPPORT == 1)
const uint8_t _gatt_service_additional_protocol_descriptor_list[] = {
    SDP_DE_DESEQ_8BITSIZE_H2_D(21),
    SDP_DE_DESEQ_8BITSIZE_H2_D(19),
        SDP_DE_DESEQ_8BITSIZE_H2_D(6),
            SDP_DE_UUID_H1_D2,
                SERV_UUID_L2CAP,
            SDP_DE_UINT_H1_D2,
                SDP_SPLIT_16BITS_BE(PSM_EATT),
        SDP_DE_DESEQ_8BITSIZE_H2_D(9),
            SDP_DE_UUID_H1_D2,
                SERV_UUID_ATT,
            SDP_DE_UINT_H1_D2,
                SDP_SPLIT_16BITS_BE(GATT_SERVICE_START_HANDLE), // GATT Service Start Handle
            SDP_DE_UINT_H1_D2,
                SDP_SPLIT_16BITS_BE(GATT_SERVICE_END_HANDLE), // GATT Service End Handle
};
#endif

static const bt_sdp_record_attr_t _gatt_service_sdp_attrs[] = { // list attr id in ascending order
    SDP_DEF_ATTRIBUTE(SERV_ATTRID_SERVICE_CLASS_ID_LIST, _gatt_service_class_id_list),
    SDP_DEF_ATTRIBUTE(SERV_ATTRID_PROTOCOL_DESC_LIST, _gatt_service_protocol_descriptor_list),
    SDP_DEF_ATTRIBUTE(SERV_ATTRID_BROWSE_GROUP_LIST, _gatt_service_browse_group),
#if (EATT_CHAN_SUPPORT == 1)
    SDP_DEF_ATTRIBUTE(SERV_ATTRID_ADDITIONAL_PROT_DESC_LISTS, _gatt_service_additional_protocol_descriptor_list),
#endif
};

#endif /* BLE_ONLY_ENABLED */

bt_status_t app_ble_gatt_server_send_service_change(uint32_t con_bfs)
{
    gatt_service_changed_t change;
    gatt_char_notify_t indicate = {NULL};

    change.start_affected_handle = co_host_to_uint16_le(0x0001);
    change.end_affected_handle = co_host_to_uint16_le(0xFFFF);

    indicate.character = g_gatt_service_changed;
    return gatts_send_value_indication(con_bfs, &indicate, (uint8_t *)&change, sizeof(change));
}

#if BLE_GATT_CLIENT_SUPPORT
static bt_status_t app_ble_gatt_send_local_client_supp_features(gatt_prf_t *prf, gatt_peer_character_t *c)
{
    if (prf == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    ble_global_t *g = ble_get_global();
    gap_conn_item_t *conn = gap_get_conn_item(prf->connhdl);

    /**
     * The Client Support Features characteristic is used by the client to inform the server which
     * features are supported by the client. If the char exists on the server, the client may update
     * the Client Support Features bit field.
     *
     */
    if (conn)
    {
        uint8_t local_client_supp_features = 0;
        local_client_supp_features |= g->gattc_supp_eatt_bearer ? GATT_CLIENT_FEAT_EATT_BEARER : 0;
        local_client_supp_features |= g->gattc_supp_robust_caching ? GATT_CLIENT_FEAT_ROBUST_CACHING : 0;
        local_client_supp_features |= g->gattc_supp_recv_multi_notify ? GATT_CLIENT_FEAT_MULT_VALUE_NOTIFY : 0;
        return gattc_write_character_value(prf, c, &local_client_supp_features, sizeof(uint8_t));
    }
    else
    {
        CO_LOG_ERR_1(BT_STS_INVALID_CONN_HANDLE, prf->connhdl);
        return BT_STS_FAILED;
    }
}
#endif

static void app_ble_gatt_cache_peer_client_aware_the_change(uint16_t connhdl)
{
    ble_global_t *g = ble_get_global();
    BleDevicePairingInfo pair_info = {0};
    bool found = false;
    gap_conn_item_t *conn = gap_get_conn_item(connhdl);
    if (conn == NULL)
    {
        return;
    }

    BLE_ADDR_INFO_T peer_le_addr = {.addr_type = (conn->sec.peer_type & 0x01)};
    memcpy(peer_le_addr.addr, conn->sec.peer_addr.address, sizeof(bt_bdaddr_t));

    if (g->ble_get_specific_record != NULL)
    {
        found = g->ble_get_specific_record(conn == NULL ? 0xFFFF : conn->connhdl, (ble_bdaddr_t *)&peer_le_addr, &pair_info);
    }
    else
    {
        found = nv_record_blerec_get_paired_dev_from_addr(&pair_info, &peer_le_addr);
    }

    if (found == true && conn->peer.gatt_service_change_unaware == true)
    {
        conn->peer.gatt_service_change_unaware = false;
        pair_info.server_cache.service_change_unaware = false;
        nv_record_ble_server_cache_add(&pair_info.peer_addr, &pair_info.server_cache);
    }
}

static void app_ble_gatt_cache_recv_service_changed_confirm(uint16_t connhdl)
{
    app_ble_gatt_cache_peer_client_aware_the_change(connhdl);
}

static void app_ble_gatt_cache_recv_databash_hash_request(uint16_t connhdl)
{
    app_ble_gatt_cache_peer_client_aware_the_change(connhdl);

    app_ble_gap_update_local_database_hash();
}

static int app_ble_gatt_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    gap_config_t cfg = gap_get_config();
    ble_global_t *g = ble_get_global();

    switch (event)
    {
        case GATT_SERV_EVENT_REGISTER_CMP:
        {
            CO_LOG_INFO_S_1(BT_STS_SVC_REGISTER, REGC, param.resgiter_cmp->status);
            break;
        }
        case GATT_SERV_EVENT_CONN_OPENED:
        {
            CO_LOG_INFO_S_3(BT_STS_GATT_SERVICE, GOPN, param.opened->service, g_gatt_service, g_gap_service);
            if (param.opened->service == g_gatt_service)
            {
                gatt_server_conn_opened_t *opened = param.opened;
                gap_conn_item_t *conn = opened->conn;
                if (g->gatts_supp_database_hash && conn->peer.gatt_service_change_unaware)
                {
                    app_ble_gatt_server_send_service_change(gap_conn_bf(conn->con_idx)); // server shall not send other ntf before recv cfm
                }
            }
            break;
        }
        case GATT_SERV_EVENT_CHAR_READ:
        {
            gatt_server_char_read_t *p = param.char_read;
            const uint8_t *c = p->character;
            if (c == g_gap_service_device_name)
            {
                uint8_t name_length = 0;
                const char *device_name = NULL;
                device_name = gap_local_le_name(&name_length);
                if (name_length == 0 || device_name == NULL)
                {
                    return true; // return empty name;
                }
                /* when the device is not discoverable, the device name characteristic
                 * should not be readable. */
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)device_name, name_length);
                return true;
            }
            else if (c == g_gap_service_appearance)
            {
                uint8_t appearance[2] = {CO_SPLIT_UINT16_LE(APPEARANCE_VALUE)};
                gatts_write_read_rsp_data(p->ctx, appearance, sizeof(uint16_t));
                return true;
            }
            else if (c == g_gap_service_central_addr_reso_supp)
            {
                uint8_t central_addr_reso_support = cfg.address_reso_support;
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&central_addr_reso_support, sizeof(uint8_t));
                return true;
            }
            else if (c == g_gap_service_use_only_rpa_after_bonding)
            {
                if (g->gap_only_use_rpa_after_bonding)
                {
                    uint8_t only_use_rpa_after_bonding = 0; /* 0 - only RPAs will be used as local addresses after bonding */
                    gatts_write_read_rsp_data(p->ctx, &only_use_rpa_after_bonding, sizeof(uint8_t));
                    return true;
                }
            }
            else if (c == g_gap_service_periph_prefer_conn_params)
            {
                gap_conn_prefer_params_t param_le;
                param_le.conn_interval_min_1_25ms = co_host_to_uint16_le(200);// 250ms, connection interal = interval * 1.25ms
                param_le.conn_interval_max_1_25ms = co_host_to_uint16_le(960);// 1200ms, connection interal = interval * 1.25ms
                param_le.max_peripheral_latency = co_host_to_uint16_le(0x00);// 0x00 to 0x01F3, max peripheral latency in units of subrated conn intervals
                param_le.superv_timeout_ms = co_host_to_uint16_le(30*1000);// 0x0A to 0x0C80 * 10ms, 100ms to 32s
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&param_le, sizeof(gap_conn_prefer_params_t));
                return true;
            }
            else if (c == g_gap_service_le_gatt_security_levels)
            {
                gatt_security_level_t sec[3];
                gap_security_levels_t sec_levels = 
                {
#if (BLE_AUDIO_ENABLED)
                    .link_sec_level = GAP_SEC_UNAUTHENTICATED,
#endif
                };
                int count = 0;
                if (sec_levels.link_sec_level)
                {
                    sec[count].mode = 1;
                    sec[count].level = sec_levels.link_sec_level;
                    count += 1;
                }
                if (sec_levels.data_sign_sec_exist)
                {
                    sec[count].mode = 2;
                    sec[count].level = sec_levels.data_sign_sec_level;
                    count += 1;
                }
                if (sec_levels.big_sec_level)
                {
                    sec[count].mode = 3;
                    sec[count].level = sec_levels.big_sec_level;
                    count += 1;
                }
                if (count)
                {
                    gatts_write_read_rsp_data(p->ctx, (uint8_t *)&sec, sizeof(gatt_security_level_t) * count);
                    return true;
                }
            }
            else if (c == g_gap_service_enc_data_key_material)
            {
                const gap_encrypted_data_key_material_t key = {.is_exist = false};
                if (key.is_exist)
                {
                    gatts_write_read_rsp_data(p->ctx, (uint8_t *)&key.data, sizeof(gap_key_material_t));
                    return true;
                }
            }
            else if (c == g_gatt_client_supp_features)
            {
                /**
                 * The Client Support Features characteristic is used by the client to inform the server which
                 * features are supported by the client. If the char exists on the server, the client may update
                 * the Client Support Features bit field.
                 *
                 * A Client Supported Features Char Value shall exist for each connected
                 * client. For clients with a trusted relationship, the Char Value shall be persistent across
                 * connections. For clients without a trusted relationship the Char Value shall be set to the
                 * default value at each connection.
                 *
                 */
                gap_conn_item_t *conn = p->conn;
                if (conn)
                {
                    uint8_t peer_client_supp_features = 0;
                    peer_client_supp_features |= conn->peer.gatt_client_supp_eatt_bearer ? GATT_CLIENT_FEAT_EATT_BEARER : 0;
                    peer_client_supp_features |= conn->peer.gatt_client_supp_robust_caching ? GATT_CLIENT_FEAT_ROBUST_CACHING : 0;
                    peer_client_supp_features |= conn->peer.gatt_client_supp_recv_multi_notify ? GATT_CLIENT_FEAT_MULT_VALUE_NOTIFY : 0;
                    gatts_write_read_rsp_data(p->ctx, &peer_client_supp_features, sizeof(uint8_t));
                    return true;
                }
            }
            else if (c == g_gatt_server_supp_features)
            {
                uint8_t features = g->gatts_supp_eatt_bearer ? GATT_SERVER_FEAT_EATT_BEARER : 0;
                gatts_write_read_rsp_data(p->ctx, &features, sizeof(uint8_t));
                return true;
            }
            else if (c == g_gatt_server_database_hash)
            {
                if (g->gatts_supp_database_hash)
                {
                    uint8_t *hash = g->local_database_hash;

                    if (g->ble_get_specific_hash != NULL)
                    {
                        g->ble_get_specific_hash(p->conn->connhdl, &hash);
                    }

                    gatts_write_read_rsp_data(p->ctx, hash, 16);
                    app_ble_gatt_cache_recv_databash_hash_request(svc->connhdl);
                    return true;
                }
            }
            break;
        }
        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            gatt_server_char_write_t *p = param.char_write;
            const uint8_t *c = p->character;
            if (c == g_gap_service_device_name)
            {
                // Device max name present is control by app layer
                uint8_t name_len_min = CO_MIN_VALUE(APP_DEVICE_NAME_MAX_LEN, p->value_len + 1);
                char *name_val = NULL;
                // Only valid to modify when value exist
                if (p->value_offset == 0 && p->value_len && p->value)
                {
                    CO_LOG_INFO_WITH_STR_1(BT_STS_GATT_DEVICE_NAME, (char *)p->value, p->value_len);
                    // Dynamic alloc name
                    name_val = (char *)bes_bt_buf_malloc(name_len_min);
                    if (name_val == NULL)
                    {
                        p->rsp_error_code = ATT_ERROR_INSUFF_RESOURCES;
                        return false;
                    }
                    memcpy(name_val, p->value, name_len_min - 1);
                    // Terminate character
                    name_val[name_len_min - 1] = '\0';
                    // Free previous name value
                    if (g->ble_dev_name != NULL)
                    {
                        bes_bt_buf_free(g->ble_dev_name);
                    }
                    // Record in app layer
                    g->ble_dev_name = name_val;
                    // Record and set ptr to btdrv
                    bt_set_ble_local_name(name_val);
                }
                else
                {
                    p->rsp_error_code = ATT_ERROR_VALUE_NOT_ALLOWED;
                    return false;
                }
            }
            else if (c == g_gap_service_appearance)
            {
                if (p->value_offset == 0 && p->value_len == sizeof(uint16_t))
                {
                    CO_LOG_INFO_S_1(BT_STS_GATT_APPEARANCE, WRAP, CO_COMBINE_UINT16_LE(p->value));
                }
                else
                {
                    p->rsp_error_code = (p->value_offset > sizeof(uint16_t )) ? ATT_ERROR_INVALID_OFFSET : ATT_ERROR_VALUE_NOT_ALLOWED;
                    return false;
                }
            }
            else if (c == g_gatt_client_supp_features)
            {
                gap_conn_item_t *conn = p->conn;
                if (conn && p->value_offset == 0 && p->value_len)
                {
                    /* A client shall not clear any bits it has set. The server shall respond to any such request
                     * with the Error Code of Value Not Allowed (0x13). */
                    uint8_t peer_client_supp_features = 0;
                    peer_client_supp_features |= conn->peer.gatt_client_supp_eatt_bearer ? GATT_CLIENT_FEAT_EATT_BEARER : 0;
                    peer_client_supp_features |= conn->peer.gatt_client_supp_robust_caching ? GATT_CLIENT_FEAT_ROBUST_CACHING : 0;
                    peer_client_supp_features |= conn->peer.gatt_client_supp_recv_multi_notify ? GATT_CLIENT_FEAT_MULT_VALUE_NOTIFY : 0;
                    if ((p->value[0] & peer_client_supp_features) == peer_client_supp_features)
                    {
                        peer_client_supp_features = p->value[0];
                        conn->peer.gatt_client_supp_eatt_bearer = (peer_client_supp_features & GATT_CLIENT_FEAT_EATT_BEARER) ? 1 : 0;
                        conn->peer.gatt_client_supp_robust_caching = (peer_client_supp_features & GATT_CLIENT_FEAT_ROBUST_CACHING) ? 1 : 0;
                        conn->peer.gatt_client_supp_recv_multi_notify = (peer_client_supp_features & GATT_CLIENT_FEAT_MULT_VALUE_NOTIFY) ? 1 : 0;
                        CO_LOG_INFO_S_2(BT_STS_GATT_CLIENT_SUPP_FEATURE, CFET, conn->connhdl, peer_client_supp_features);
                    }
                    else
                    {
                        p->rsp_error_code = ATT_ERROR_VALUE_NOT_ALLOWED;
                        return false;
                    }
                }
            }

            gatts_send_write_rsp(p->ctx, p->rsp_error_code);
            break;
        }
        case GATT_SERV_EVENT_INDICATE_CFM:
        {
            gatt_server_indicate_cfm_t *confirm = param.confirm;
            if (gatts_get_char_byte_16_bit_uuid(confirm->character) == GATT_CHAR_UUID_SERVICE_CHANGED)
            {
                app_ble_gatt_cache_recv_service_changed_confirm(svc->connhdl);
            }
            break;
        }
        case GATT_SERV_EVENT_DESC_READ:
        {
            gatt_server_desc_read_t *p = param.desc_read;
            uint16_t cccd_config = 0x0002;
            cccd_config = co_host_to_uint16_le(cccd_config);
            gatts_write_read_rsp_data(p->ctx, (uint8_t *)&cccd_config, sizeof(uint16_t));
            break;
        }
        case GATT_SERV_EVENT_DESC_WRITE:
        default:
        {
            break;
        }
    }

    return 0;
}

#if BLE_GATT_CLIENT_SUPPORT
static void app_ble_client_check_discover_basic_service(gatt_prf_t *prf)
{
    const gatt_peer_service_t *srv = NULL;
    uint8_t uuid_16_le_gap[sizeof(uint16_t)] = {CO_SPLIT_UINT16_LE(GATT_UUID_GAP_SERVICE)};
    uint8_t uuid_16_le_gatt[sizeof(uint16_t)] = {CO_SPLIT_UINT16_LE(GATT_UUID_GATT_SERVICE)};

    if (BT_STS_SUCCESS != gattc_get_service(prf, sizeof(uint16_t), uuid_16_le_gap, &srv))
    {
        gattc_discover_service(prf, GATT_UUID_GAP_SERVICE, NULL);
    }
    else
    {
        app_ble_gatt_read_peer_character_value(prf->connhdl, GATT_CHAR_UUID_DEVICE_NAME);
    }

    if (BT_STS_SUCCESS != gattc_get_service(prf, sizeof(uint16_t), uuid_16_le_gatt, &srv))
    {
        gattc_discover_service(prf, GATT_UUID_GATT_SERVICE, NULL);
    }
}

static void app_ble_peer_enc_data_key_material_received(gap_conn_item_t *conn, const gap_key_material_t *key, uint8_t error_code)
{
    gap_recv_key_material_t param = {0};

    param.connhdl = conn->connhdl;
    param.con_idx = conn->con_idx;
    param.own_addr_type = conn->own_addr_type;
    param.peer_type = conn->peer_type;
    param.peer_addr = conn->peer_addr;
    param.conn = conn;

    if (key && error_code == ATT_ERROR_NO_ERROR)
    {
        param.error_code = BT_STS_SUCCESS;
        param.key_material = key;
    }
    else
    {
        param.error_code = error_code ? error_code : BT_STS_FAILED;
        param.key_material = NULL;
    }

    if (conn->conn_callback)
    {
        conn->conn_callback(conn->connhdl, GAP_CONN_EVENT_RECV_KEY_METERIAL, (gap_conn_callback_param_t){&param});
    }
}

static int app_ble_gatt_client_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param)
{
    uint8_t conidx = gap_zero_based_conidx(prf->con_idx);
    ble_global_t *g = ble_get_global();

    switch (event)
    {
        case GATT_PROF_EVENT_OPENED:
        {
            // Bonded device do not start service discovery
            // Cause service cache for feature is nv cached
            if (param.opened->conn->sec.device_bonded == true)
            {
                CO_LOG_INFO_S_0(param.opened->conn->con_idx, DFSD);
                break;
            }
        }
        // FALLTHROUGH
        case GATT_PROF_EVENT_CONN_ENCRYPTED:
        {
            // Check start mtu exec and basic service discovery,if gfps enable, do not mtu ecchange
#ifndef GFPS_ENABLED
            gatt_start_att_mtu_exchange(param.opened->conn->connhdl, 0);
#endif
            app_ble_client_check_discover_basic_service(prf);
            break;
        }
        case GATT_PROF_EVENT_CLOSED:
        {
            break;
        }
        case GATT_PROF_EVENT_SERVICE_CHANGED:
        {
            CO_LOG_INFO_S_0(BT_STS_PEER_SERVICE_CHANGED, PSHG);
            app_ble_clr_gatt_cli_cache_by_connhdl(prf->connhdl);
            break;
        }
        case GATT_PROF_EVENT_SERVICE:
        {
            gatt_profile_service_t *p = param.service;
            gatt_peer_service_t *s = p->service;
            if (p->error_code != ATT_ERROR_NO_ERROR)
            {
                CO_LOG_ERR_2(BT_STS_SERVICE_NOT_FOUND, p->service_uuid, p->error_code);
                break;
            }
            if (s->service_uuid == GATT_UUID_GAP_SERVICE)
            {
                uint16_t gap_chars[] = {
                        GATT_CHAR_UUID_DEVICE_NAME,
                        GATT_CHAR_UUID_APPEARANCE,
                        GATT_CHAR_UUID_CENTRAL_ADDRESS_RESOLUTION,
                        GATT_CHAR_UUID_RPA_ONLY,
                        GATT_CHAR_UUID_PERIPH_PREFER_CONN_PARAMS,
                        GATT_CHAR_UUID_LE_GATT_SECURITY_LEVELS,
                        GATT_CHAR_UUID_ENCRYPTED_DATA_KEY_MATERIAL,
                    };
                gattc_discover_multi_characters(prf, s, gap_chars, sizeof(gap_chars)/sizeof(uint16_t));
                break;
            }
            if (s->service_uuid == GATT_UUID_GATT_SERVICE)
            {
                uint16_t gatt_chars[] = {
                        GATT_CHAR_UUID_SERVICE_CHANGED,
                        GATT_CHAR_UUID_CLIENT_SUPP_FEATURES,
                        GATT_CHAR_UUID_SERVER_SUPP_FEATURES,
                        GATT_CHAR_UUID_DATABASE_HASH};
                gattc_discover_multi_characters(prf, s, gatt_chars, sizeof(gatt_chars)/sizeof(uint16_t));
            }
            break;
        }
        case GATT_PROF_EVENT_CHARACTER:
        {
            gatt_profile_character_t *p = param.character;
            gatt_peer_service_t *s = p->service;
            gatt_peer_character_t *c = p->character;
            gap_conn_item_t *conn = p->conn;
            if (p->error_code != ATT_ERROR_NO_ERROR)
            {
                CO_LOG_ERR_2(BT_STS_CHARACTER_NOT_FOUND, p->char_uuid, p->error_code);
                break;
            }
            if (s->service_uuid == GATT_UUID_GAP_SERVICE)
            {
                if (p->char_uuid != GATT_CHAR_UUID_ENCRYPTED_DATA_KEY_MATERIAL)
                {
                    gattc_read_character_value(prf, c);
                }
                break;
            }
            if (s->service_uuid == GATT_UUID_GATT_SERVICE)
            {
                if (p->char_uuid == GATT_CHAR_UUID_SERVICE_CHANGED)
                {
                    CO_LOG_INFO_S_2(BT_STS_SERVICE_CHANGED, HSHG, conn->connhdl, c->char_value_handle);
                    conn->peer.gatt_server_has_service_changed_char = true;
                    gattc_write_cccd_descriptor(prf, c, false, true);
                }
                else if (p->char_uuid == GATT_CHAR_UUID_CLIENT_SUPP_FEATURES)
                {
                    app_ble_gatt_send_local_client_supp_features(prf, c);
                }
                else if (p->char_uuid == GATT_CHAR_UUID_DATABASE_HASH)
                {
                    uint8_t zero_hash[16] = {0};
                    CO_LOG_INFO_S_2(BT_STS_DATABASE_HASH, HDSH, conn->connhdl, c->char_value_handle);
                    conn->peer.gatt_server_has_database_hash_char = true;
                    if (memcmp(conn->peer.service_database_hash, zero_hash, 16) == 0)
                    {
                        gattc_read_character_value(prf, c);
                    }
                }
                else
                {
                    gattc_read_character_value(prf, c);
                }
            }
            break;
        }
        case GATT_PROF_EVENT_CHAR_READ_RSP:
        {
            gatt_profile_char_read_rsp_t *p = param.char_read_rsp;
            bool read_success = (p->error_code == ATT_ERROR_NO_ERROR) && p->value_len;
            uint16_t char_uuid = gatt_char_16_uuid_le(p->character);
            gap_conn_item_t *conn = p->conn;
            if (char_uuid == GATT_CHAR_UUID_ENCRYPTED_DATA_KEY_MATERIAL)
            {
                gap_key_material_t *key = (gap_key_material_t *)p->value;
                app_ble_peer_enc_data_key_material_received(conn, read_success ? key : NULL, p->error_code);
                break;
            }
            if (!read_success)
            {
                break;
            }
            // Peer device name store in app
            if (char_uuid == GATT_CHAR_UUID_DEVICE_NAME)
            {
                uint8_t value_len = 0;

                if (p->value && p->value_len)
                {
                    if (g->peer_dev_name[conidx])
                    {
                        bes_bt_buf_free(g->peer_dev_name[conidx]);
                        g->peer_dev_name[conidx] = NULL;
                    }
                    // Max len up to GAP_MAX_DEVICE_NAME_LEN
                    value_len = (p->value_len <= GAP_MAX_DEVICE_NAME_LEN) ?
                                            p->value_len : GAP_MAX_DEVICE_NAME_LEN;
                    g->peer_dev_name[conidx] = (char *)bes_bt_buf_malloc(value_len + 1);
                    // Tail with 0
                    if (g->peer_dev_name[conidx])
                    {
                        memcpy((void *)g->peer_dev_name[conidx], p->value, value_len);
                        *(char *)&(g->peer_dev_name[conidx][value_len]) = 0;
                    }

                    CO_LOG_INFO_WITH_STR_2(BT_STS_GATT_DEVICE_NAME, (char *)g->peer_dev_name[conidx],
                        p->value_len, prf->connhdl);
                }
            }
            else if (char_uuid == GATT_CHAR_UUID_APPEARANCE)
            {
                CO_LOG_INFO_S_2(BT_STS_GATT_APPEARANCE, APRA, prf->connhdl, CO_COMBINE_UINT16_LE(p->value));
            }
            else if (char_uuid == GATT_CHAR_UUID_CENTRAL_ADDRESS_RESOLUTION)
            {
                CO_LOG_INFO_S_2(BT_STS_GATT_CENTRAL_ADDR_RESOLUTION, CRES, prf->connhdl, p->value[0]);
                if (conn)
                {
                    conn->peer.central_addr_reso_support = p->value[0] ? true : false;
                }
            }
            else if (char_uuid == GATT_CHAR_UUID_RPA_ONLY)
            {
                CO_LOG_INFO_S_2(BT_STS_ONLY_USE_RPA_AFTER_BONDING, ORPA, prf->connhdl, p->value[0]);
                if (conn)
                {
                    conn->peer.only_use_rpa_after_bonding = p->value[0] ? true : false;
                }
            }
            else if (char_uuid == GATT_CHAR_UUID_PERIPH_PREFER_CONN_PARAMS)
            {
                POSSIBLY_UNUSED gap_conn_prefer_params_t *conn_params = (gap_conn_prefer_params_t *)p->value;
                POSSIBLY_UNUSED uint16_t interval_min = co_uint16_le_to_host(conn_params->conn_interval_min_1_25ms);
                POSSIBLY_UNUSED uint16_t interval_max = co_uint16_le_to_host(conn_params->conn_interval_max_1_25ms);
                POSSIBLY_UNUSED uint16_t latency = co_uint16_le_to_host(conn_params->max_peripheral_latency);
                POSSIBLY_UNUSED uint16_t timeout_ms = co_uint16_le_to_host(conn_params->superv_timeout_ms) * 10;
                CO_LOG_INFO_S_3(BT_STS_PERIPH_PERFERRED_CONN_PARAMS, PPAR, (p->value_len<<16)|prf->connhdl,
                    (interval_min<<16)|interval_max, (latency<<16)|timeout_ms);
            }
            else if (char_uuid == GATT_CHAR_UUID_LE_GATT_SECURITY_LEVELS)
            {
                gap_security_levels_t sec_levels = {0};
                gatt_security_level_t *sec = (gatt_security_level_t *)p->value;
                uint8_t mode = 0, level = 0;
                int count = p->value_len / sizeof(gatt_security_level_t);
                int i = 0;
                for (; i < count; i += 1)
                {
                    mode = sec[i].mode;
                    level = sec[i].level;

                    CO_LOG_INFO_S_3(BT_STS_GATT_PEER_SEC_LEVEL, SLVL, (i<<16)|count, mode, level);

                    if (mode == 1)
                    {
                        sec_levels.link_sec_level = level;
                    }
                    else if (mode == 2)
                    {
                        sec_levels.data_sign_sec_exist = true;
                        sec_levels.data_sign_sec_level = level;
                    }
                    else if (mode == 3)
                    {
                        sec_levels.big_sec_level = level;
                    }

                    gap_peer_security_levels_received(conn, sec_levels);
                }
            }
            else if (char_uuid == GATT_CHAR_UUID_SERVER_SUPP_FEATURES)
            {
                CO_LOG_INFO_S_2(BT_STS_GATT_SERVER_SUPP_FEATURE, SFET, prf->connhdl, p->value[0]);
                if (conn)
                {
                    conn->peer.gatt_server_supp_eatt_bearer =
                        (p->value[0] & GATT_SERVER_FEAT_EATT_BEARER) ? true : false;
                }
            }
            else if (char_uuid == GATT_CHAR_UUID_DATABASE_HASH)
            {
                gattc_recv_peer_database_hash(conn->connhdl, p->value,
                                                conn->peer.gatt_server_has_service_changed_char);
            }
            break;
        }
        case GATT_PROF_EVENT_NOTIFY:
        {
            gatt_profile_recv_notify_t *notify = param.notify;
            if (notify->service->service_uuid == GATT_UUID_GATT_SERVICE && gatt_char_16_uuid_le(notify->character) == GATT_CHAR_UUID_SERVICE_CHANGED)
            {
                const gatt_service_changed_t *value = (gatt_service_changed_t *)notify->value;
                uint16_t start_handle = co_uint16_le_to_host(value->start_affected_handle);
                uint16_t end_handle = co_uint16_le_to_host(value->end_affected_handle);
                if (notify->value_len != sizeof(gatt_service_changed_t))
                {
                    CO_LOG_ERR_1(BT_STS_INVALID_LENGTH, notify->value_len);
                    break;
                }
                CO_LOG_INFO_S_3(BT_STS_PEER_SERVICE_CHANGED, SCHG, (prf->prf_id<<16)|notify->character->char_value_handle, start_handle, end_handle);

                if (notify->conn->peer.gatt_server_has_database_hash_char == false)
                {
                    gattc_recv_peer_service_changed(prf->connhdl, start_handle, end_handle);
                }
                else
                {
                    app_ble_gatt_read_peer_character_value(prf->connhdl, GATT_CHAR_UUID_DATABASE_HASH);

                }
            }
            break;
        }
        case GATT_PROF_EVENT_INCLUDE:
        case GATT_PROF_EVENT_DESC_READ_RSP:
        case GATT_PROF_EVENT_CHAR_WRITE_RSP:
        case GATT_PROF_EVENT_DESC_WRITE_RSP:
        default:
        {
            break;
        }
    }

    return 0;
}
#endif /* BLE_GATT_CLIENT_SUPPORT */

bt_status_t app_ble_gatt_update_enc_data_key_material(const gap_key_material_t *key_material)
{
    return gatts_send_indication(GAP_ALL_CONNS, g_gap_service_enc_data_key_material,
                                 (uint8_t *)key_material, sizeof(gap_key_material_t));
}

bt_status_t app_ble_gatt_read_peer_character_value(uint16_t connhdl, uint16_t uuid)
{
#if BLE_GATT_CLIENT_SUPPORT
    if (bt_defer_curr_func_2(app_ble_gatt_read_peer_character_value,
            bt_fixed_param(connhdl), bt_fixed_param(uuid)))
    {
        return BT_STS_SUCCESS;
    }

    ble_global_t *g = ble_get_global();
    gap_conn_item_t *conn = NULL;
    gatt_prf_t prf = {0};

    conn = gap_get_conn_item(connhdl);
    if (conn == NULL)
    {
        CO_LOG_ERR_1(BT_STS_INVALID_CONN_HANDLE, connhdl);
        return BT_STS_INVALID_CONN_HANDLE;
    }

    prf.prf_id = g->gattc_profile_id;
    prf.connhdl = conn->connhdl;
    prf.con_idx = conn->con_idx;

    return gattc_read_character_by_uuid_of_service(&prf, 0x0001, 0xFFFF, uuid, NULL);
#else
    return BT_STS_NOT_SUPPORT;
#endif
}

static void app_ble_gatt_cache_server_service_changed(void)
{
    ble_global_t *g = ble_get_global();

    CO_LOG_INFO_S_0(BT_STS_SERVICE_CHANGED, LSHG);

    if (g->gatts_supp_database_hash == false)
    {
        return;
    }

    nv_record_blerec_set_change_unaware();

    app_ble_gatt_server_send_service_change(GAP_ALL_CONNS);
}

static void app_ble_gatt_gen_local_hash_complete(void *priv, int error_code, const uint8_t *hash)
{
    ble_global_t *g = ble_get_global();
    struct pp_buff *ppb = (struct pp_buff *)priv;
    const uint8_t *nv_database_hash = NULL;
    bool local_service_changed = false;
    uint8_t zero_hash[16] = {0};

    if (error_code || hash == NULL)
    {
        CO_LOG_ERR_1(BT_STS_GEN_DATABASE_HASH_FAILED, error_code);
        goto label_free_ppb;
    }

    nv_record_blerec_update_database_hash(hash);

    nv_database_hash = app_ble_get_nv_local_database_hash();

    CO_LOG_INFO_S_3(BT_STS_GATT_DATABASE_HASH_VALUE, LDHS, CO_COMBINE_UINT32_BE(hash),
        CO_COMBINE_UINT32_BE(hash+4), CO_COMBINE_UINT32_BE(hash+12));

    if (memcmp(g->local_database_hash, zero_hash, 16) != 0 &&
        memcmp(g->local_database_hash, nv_database_hash, 16) != 0)
    {
        local_service_changed = true;
    }

    memcpy(g->local_database_hash, nv_database_hash, 16);

    if (local_service_changed)
    {
        app_ble_gatt_cache_server_service_changed();
    }

label_free_ppb:
    ppb_free(ppb);
}

void app_ble_gap_update_local_database_hash(void)
{
    if (bt_defer_curr_func_0(app_ble_gap_update_local_database_hash))
    {
        return;
    }

    ble_global_t *g = ble_get_global();

    const uint8_t *nv_database_hash = app_ble_get_nv_local_database_hash();
    memcpy(g->local_database_hash, nv_database_hash, 16);
    gatts_gen_local_database_hash(app_ble_gatt_gen_local_hash_complete);
}

bt_status_t app_ble_clr_gatt_cli_cache_by_connhdl(uint16_t connhdl)
{
    BLE_ADDR_INFO_T peer_addr = {0};
    gap_conn_item_t *conn = NULL;

    conn = gap_get_conn_item(connhdl);
    if (conn)
    {
        peer_addr.addr_type = (conn->sec.peer_type & 0x01);
        memcpy(peer_addr.addr, conn->sec.peer_addr.address, sizeof(bt_bdaddr_t));
        nv_record_ble_gatt_cache_del(&peer_addr);
        return BT_STS_SUCCESS;
    }

    return BT_STS_FAILED;
}

bt_status_t app_ble_gap_set_local_ecdh_key_pair(const uint8_t *p_sec_key_256, const uint8_t *p_pub_key_256)
{
    return gap_set_local_secret_public_key_pair(p_sec_key_256, p_pub_key_256);
}

void app_ble_gatt_standard_service_init(void)
{
    ble_global_t *g = ble_get_global();
    uint16_t gap_service_attrs = 0;
    uint16_t gatt_service_attrs = 0;
    uint8_t user_lid = gatts_register_server_user(app_ble_gatt_server_callback);

#if (EATT_CHAN_SUPPORT == 1)
    g->gatts_supp_eatt_bearer = true;
    g->gattc_supp_eatt_bearer = true;
#endif
    g->gattc_supp_recv_multi_notify = true;

    gap_service_attrs = ARRAY_SIZE(g_gap_service_attr_list);

    gatt_service_attrs = ARRAY_SIZE(g_gatt_service_attr_list);

    if (g->gatts_supp_database_hash == false)
    {
        gatt_service_attrs -= 1;
    }

    gatts_register_user_service(user_lid, g_gap_service_attr_list, gap_service_attrs, NULL);

    gatts_register_user_service(user_lid, g_gatt_service_attr_list, gatt_service_attrs, NULL);

#if BLE_GATT_CLIENT_SUPPORT
    g->gattc_profile_id = gattc_register_self_manage_profile(app_ble_gatt_client_callback, NULL);
#endif

#ifndef BLE_ONLY_ENABLED
    if (bt_l2cap_get_config().gatt_over_br_edr && bt_l2cap_get_config().stack_new_design)
    {
        uint8_t btgatt_add_std_gap_service_sdp = false;
        uint8_t btgatt_add_std_gatt_service_sdp = false;
        int gap_sdp_attrs = ARRAY_SIZE(_gap_service_sdp_attrs);
        int gatt_sdp_attrs = ARRAY_SIZE(_gatt_service_sdp_attrs);

#if (EATT_CHAN_SUPPORT == 1)
        if (!bt_l2cap_get_config().eatt_over_br_edr)
        {
            gap_sdp_attrs -= 1;

            gatt_sdp_attrs -= 1;
        }
#endif
        if (btgatt_add_std_gap_service_sdp)
        {
            bt_sdp_create_record(_gap_service_sdp_attrs, gap_sdp_attrs);
        }

        if (btgatt_add_std_gatt_service_sdp)
        {
            bt_sdp_create_record(_gatt_service_sdp_attrs, gatt_sdp_attrs);
        }
    }
#endif
}

void app_ble_init(void)
{
    ble_global_t *g = ble_get_global();
    ble_adv_activity_t *adv = NULL;
    int max_size = ARRAY_SIZE(g->adv);
    int i = 0;

    gap_config_t gap_cfg = 
    {
        .cfg =
        {
#if defined(__GATT_OVER_BR_EDR__)
            .gatt_over_br_edr = true,
#endif

#if defined(__HFP_ACS_BV17_I__) || (defined(CTKD_ENABLE) && defined(IS_CTKD_OVER_BR_EDR_ENABLED))
            .ctkd_over_br_edr = true,
#endif
        },

#if defined (BLE_ADV_RPA_ENABLED)
        .address_reso_support = true,
#endif
    };

    gap_init(&gap_cfg, app_ble_recv_stack_global_event, &gap_external_func_cbs);

#if BLE_AUDIO_ENABLED
    gaf_prf_cfg_t gaf_cfg =
    {
#ifdef USB_AUDIO_APP
        /// USB Audio enable will need more tolerence time
        .rx_ntf_ind_timeout_s = 2,
#else
        .rx_ntf_ind_timeout_s = 1,
#endif /* USB_AUDIO_APP */
    };

    gaf_prf_init(&gaf_cfg);
#endif /* BLE_AUDIO_ENABLED */

    memset(g, 0, sizeof(ble_global_t));

    for (; i < max_size; i += 1)
    {
        adv = g->adv + i;
        memset(adv, 0, sizeof(ble_adv_activity_t));
        adv->adv_handle = GAP_INVALID_ADV_HANDLE;
    }

#if (mHDT_LE_SUPPORT)
    g->default_tx_pref_phy_bits = GAP_PHY_BIT_LE_MHDT;
    g->default_rx_pref_phy_bits = GAP_PHY_BIT_LE_MHDT;
#else
    g->default_tx_pref_phy_bits = GAP_PHY_BIT_LE_2M;
    g->default_rx_pref_phy_bits = GAP_PHY_BIT_LE_2M;
#endif

    app_ble_gatt_standard_service_init();

    app_ble_customif_init();

#ifdef CFG_SEC_CON
    // Note: for the product that needs secure connection feature while google
    // fastpair is not included, the ble private/public key should be
    // loaded from custom parameter section
    memcpy(ble_public_key, bes_demo_Public_key, sizeof(ble_public_key));
    memcpy(ble_private_key, bes_demo_private_key, sizeof(ble_private_key));
#endif

#if (BLE_AUDIO_ENABLED)
    app_ble_register_advertising(BLE_AUDIO_ADV_HANDLE, app_ble_audio_adv_activity_prepare);
#else
    app_ble_register_advertising(BLE_BASIC_ADV_HANDLE, app_ble_stub_adv_activity_prepare);
#endif

#ifndef BLE_WATCH_ADAPTER
#if defined(BLE_DIP_ENABLE)
    ble_dip_init();
#endif

#if defined(BLE_BATT_ENABLE)
    ble_batt_init();
#endif

#if defined(BLE_HID_ENABLE)
    ble_hid_device_init();
#endif

#if defined(ANCC_ENABLED)
    ble_ancc_init();
#endif

#if (defined(BES_OTA) || defined(BES_OTA_BASIC))&& !defined(OTA_OVER_TOTA_ENABLED)
    ble_ota_init();
#endif

#if defined(__AMA_VOICE__)
    ble_ai_ama_init();
#endif

#if defined(__DMA_VOICE__)
    ble_ai_dma_init();
#endif

#if defined(__GMA_VOICE__)
    ble_ai_gma_init();
#endif

#if defined(__SMART_VOICE__)
    ble_ai_smart_voice_init();
#endif

#if defined(__TENCENT_VOICE__)
    ble_ai_tencent_voice_init();
#endif

#if defined(DUAL_MIC_RECORDING)
    ble_ai_recording_init();
#endif

#if defined(__CUSTOMIZE_VOICE__)
    ble_ai_customize_init();
#endif

#ifdef CFG_APP_DATAPATH_SERVER
    ble_datapath_server_init();
#endif

#ifdef CFG_APP_DATAPATH_CLIENT
    ble_datapath_client_init();
#endif

#ifdef TILE_DATAPATH
    ble_tile_init();
#endif

#endif   /* BLE_WATCH_ADAPTER */

#if defined(BLE_HID_HOST)
    ble_hid_host_init();
#endif

#if (BLE_AHP_SERVER_SUPPORT)
    ble_ahp_init();
#endif

#ifdef ANCS_ENABLED
    ble_ancs_init();
#endif

#if defined(BLE_IAS_ENABLED)
    ble_iac_init();
#endif

#if defined(CFG_APP_SAS_SERVER)
    ble_sass_init();
#endif

#if defined(BES_MOBILE_SAS)
    ble_sasc_init();
#endif

#ifdef SWIFT_ENABLED
    app_swift_init();
#endif

#ifdef BLE_TOTA_ENABLED
    ble_tota_init();
#endif

#ifdef BLE_IAS_ENABLED
    ble_ias_init();
#endif

#ifdef BLE_WIRELESS_TRANS_SRV_ENABLED
    ble_wireless_trans_srv_init();
#endif

#ifdef BLE_WIRELESS_TRANS_CLI_ENABLED
    ble_wireless_trans_cli_init();
#endif
}

uint32_t app_ble_save_ctx(uint8_t conidx, uint8_t *buf, uint16_t buf_len)
{
    uint16_t offset = 0;

#if (APP_BLE_DEMO_APP_ENABLED)
    offset += ble_demo_app_role_switch_handler(conidx, false, buf + offset, buf_len - offset);
#endif

#ifdef CFG_APP_DATAPATH_SERVER
    offset += ble_datapath_save_ctx(conidx, buf + offset, buf_len - offset);
#endif

    return offset;
}

uint32_t app_ble_restore_ctx(uint8_t conidx, uint8_t *buf, uint16_t buf_len)
{
    uint16_t offset = 0;

#if (APP_BLE_DEMO_APP_ENABLED)
    offset += ble_demo_app_role_switch_handler(conidx, true, buf + offset, buf_len - offset);
#endif

#ifdef CFG_APP_DATAPATH_SERVER
    offset += ble_datapath_restore_ctx(conidx, buf + offset, buf_len - offset);
#endif

    return offset;
}

#if (mHDT_LE_SUPPORT)
void app_ble_mhdt_hci_le_rd_local_proprietary_feat_cmd()
{
    gap_mhdt_hci_le_rd_local_proprietary_feat_cmd();
}

void app_ble_mhdt_hci_le_rd_remote_proprietary_feat_cmd(uint8_t conidx)
{
    gap_mhdt_hci_le_rd_remote_proprietary_feat_cmd(conidx);
}
#endif
#endif /* BLE_HOST_SUPPORT */
