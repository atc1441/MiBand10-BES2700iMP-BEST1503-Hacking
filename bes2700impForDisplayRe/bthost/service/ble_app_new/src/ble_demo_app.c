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
#include "bluetooth.h"

#include "app_ble.h"
#include "ble_demo_app.h"

#include "nvrecord_extension.h"
#include "nvrecord_ble.h"
#include "bluetooth_nv_mgr.h"

#include "gap_service.h"
#include "gaf_service.h"

#if defined(IBRT)
#include "app_tws_ctrl_thread.h"
#endif

#if (APP_BLE_DEMO_APP_ENABLED)

/* DEFINITIONS */

#define BLE_RS_CONTROLLER_SUPPORT (1)

#define APPLE_INC_MANUFACTURE_NAME_STR "Apple Inc."

#define IS_IOS_DEV(conidx)  (p_demo_app_info->ios_dev_bf & CO_BIT_MASK(gap_zero_based_conidx(conidx)))
#define IS_CTKD_SUPPORT()   (memcmp(p_demo_app_info->app_demo_addr.address, gap_hci_bt_address(), sizeof(bt_bdaddr_t)) == 0)

/* STRUCTURES */
typedef struct ble_demo_app_info
{
    uint8_t addr[6];
    uint8_t ltk[16];
    uint8_t irk[16];
    uint8_t local_rand[8];
    uint16_t local_ediv;
    uint8_t bond_info_bf;
    uint8_t padding[3];
} ble_demo_app_sync_t;

struct ble_demo_app_information
{
    // Adv enable
    bool adv_enable;
    // Address for app
    bt_bdaddr_t app_demo_addr;
    // IRK for app
    uint8_t irk[GAP_KEY_LEN];
    // Adv handle for demo app adv
    uint8_t adv_hdl;
    // Bitfield to distinguish ios device
    uint8_t ios_dev_bf;
    // Demo app profile lid
    uint8_t prf_lid;
    // Specific service databse hash
    uint8_t hash[GAP_KEY_LEN];
} *p_demo_app_info = NULL;

/* DECLARATIONS */
static void ble_demo_app_trans_addrress(uint8_t addr[6]);
static bool ble_demo_app_adv_activity_prepare(ble_adv_activity_t *adv);
static void ble_demo_app_conn_estb_thread_handler(uint8_t conidx);
static void ble_demo_app_visible_svc_database_hash_gen_cmp(void *priv, int error_code, const uint8_t *hash);
static void ble_demo_app_smp_requirements_modify(uint16_t connhdl, ble_smp_require_t *p_requirements);
static void ble_demo_app_add_ble_record_handler(uint16_t connhdl, BleDevicePairingInfo *p_record);
static void ble_demo_app_get_specific_irk_ia_handler(uint16_t connhdl, uint8_t **p_irk, bt_bdaddr_t **p_ia);
static bool ble_demo_app_get_specific_record_handler(uint16_t connhdl, const ble_bdaddr_t *peer_addr, BleDevicePairingInfo *pairing_info);
#if BLE_GATT_CLIENT_SUPPORT
static int ble_demo_app_gatt_client_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param);
#endif /* BLE_GATT_CLIENT_SUPPORT */
static void ble_demo_app_get_specific_hash_handler(uint16_t connhdl, uint8_t **pp_hash);

/* FUNCTIONS */
int ble_demo_app_init(uint8_t adv_hdl_shared, const bt_bdaddr_t *p_app_ia_shared, const uint8_t *p_irk_shared)
{
    TRACE(1, "%s using adv hdl: %d", __func__, adv_hdl_shared);

    if (p_app_ia_shared == NULL || p_irk_shared == NULL)
    {
        return BT_STS_INVALID_PARM;
    }

    if (p_demo_app_info != NULL)
    {
        return BT_STS_SUCCESS;
    }

    p_demo_app_info = (struct ble_demo_app_information *)bes_bt_buf_malloc(sizeof(struct ble_demo_app_information));

    if (p_demo_app_info == NULL)
    {
        return BT_STS_NO_RESOURCES;
    }

    memset(p_demo_app_info, 0, sizeof(*p_demo_app_info));
    // Adv handle for demo app adv
    p_demo_app_info->adv_hdl = adv_hdl_shared;
    // Shared IRK for demo app le smp
    memcpy(p_demo_app_info->irk, p_irk_shared, GAP_KEY_LEN);
    // Shared IA addr for demo app le smp and rpa generation
    p_demo_app_info->app_demo_addr = *p_app_ia_shared;

    p_demo_app_info->adv_enable = true;
    // Demo app adv
    app_ble_register_advertising(adv_hdl_shared, ble_demo_app_adv_activity_prepare);
#if BLE_GATT_CLIENT_SUPPORT
    // Demo app profile
    p_demo_app_info->prf_lid = gattc_register_profile(ble_demo_app_gatt_client_callback, NULL);
#endif /* BLE_GATT_CLIENT_SUPPORT */
    // SMP requirements modify
    app_ble_smp_require_modify_callback_register(ble_demo_app_smp_requirements_modify);
    // IRK/IA speicifc for handle connection
    app_ble_smp_get_specifc_irk_ia_callback_register(ble_demo_app_get_specific_irk_ia_handler);
    // Add ble record to nv before handler
    app_ble_gap_add_record_modify_callback_register(ble_demo_app_add_ble_record_handler);
    // Get nv record callback
    app_ble_gap_get_specifc_record_callback_register(ble_demo_app_get_specific_record_handler);
    // Hash value modify
    app_ble_gatt_get_specifc_hash_callback_register(ble_demo_app_get_specific_hash_handler);
    // Refresh resolving list
    app_ble_add_devices_info_to_resolving();

    return BT_STS_SUCCESS;
}

int ble_demo_app_deinit(void)
{
    if (p_demo_app_info == NULL)
    {
        return BT_STS_SUCCESS;
    }

#if BLE_GATT_CLIENT_SUPPORT
    // Demo app profile
    gattc_unregister_profile(p_demo_app_info->prf_lid);
#endif /* BLE_GATT_CLIENT_SUPPORT */
    // SMP requirements modify
    app_ble_smp_require_modify_callback_register(NULL);
    // IRK/IA speicifc for handle connection
    app_ble_smp_get_specifc_irk_ia_callback_register(NULL);
    // Add ble record to nv before handler
    app_ble_gap_add_record_modify_callback_register(NULL);
    // Get nv record callback
    app_ble_gap_get_specifc_record_callback_register(NULL);
    // Hash value modify
    app_ble_gatt_get_specifc_hash_callback_register(NULL);
    // Demo app adv deregister
    app_ble_register_advertising(p_demo_app_info->adv_hdl, NULL);

    bes_bt_buf_free(p_demo_app_info);
    p_demo_app_info = NULL;

    return BT_STS_SUCCESS;
}

void ble_demo_app_connection_established_handler(uint8_t conidx)
{
    if (p_demo_app_info == NULL)
    {
        return;
    }

    // Mark ios device not, waiting for read manufacture name done
    p_demo_app_info->ios_dev_bf &= ~(1 << gap_zero_based_conidx(conidx));

    bt_thread_call_func_1(ble_demo_app_conn_estb_thread_handler,
                          bt_fixed_param(conidx));
}

static void ble_demo_app_add_ble_record_handler(uint16_t connhdl, BleDevicePairingInfo *p_record)
{
    gap_conn_item_t *conn = gap_get_conn_item(connhdl);
    bool found = nv_record_blerec_get_paired_dev_from_addr(p_record, &p_record->peer_addr);

    if (found == true)
    {
        TRACE(1, "%s exist ble record", __func__);
    }

    if (conn != NULL && conn->conn_type == HCI_CONN_TYPE_BT_ACL)
    {
        return;
    }

    if (conn != NULL)
    {
        if (IS_IOS_DEV(conn->con_idx) || conn->adv_handle == p_demo_app_info->adv_hdl)
        {
            // LEA paired irk resolved this addr, but we should store this record due to it is from app
            // And if APP paired irk resolved this addr, repaired with mobile, we should store this
            // record but not to update addr
            if ((found == true && (p_record->bond_info_bf & CO_BIT_MASK(BONDED_WITH_DEMO_APP_ADV)) == 0) ||
                    (conn->peer_type == 0x01 && (p_record->peer_addr.addr_type == 0)))
            {
                p_record->bond_info_bf |= CO_BIT_MASK(BONDED_WITH_DEMO_APP_ADV);
                ble_demo_app_trans_addrress(p_record->peer_addr.addr);
            }
        }
    }

    // Only sync after smp irk/ia exec and is for app using, and push this behind all of this
    if ((conn != NULL && conn->smp_conn != NULL) &&
            (p_record->peer_addr.addr_type == 0 &&
             (p_record->bond_info_bf & CO_BIT_MASK(BONDED_WITH_DEMO_APP_ADV)) != 0))
    {
        TRACE(1, "update connection[%d] solved addr:", conn->con_idx);
        DUMP8("%02x ", conn->peer_addr.address, sizeof(bt_bdaddr_t));
        DUMP8("%02x ", p_record->peer_addr.addr, sizeof(bt_bdaddr_t));
        // Update to record addr
        conn->sec.peer_addr = *(bt_bdaddr_t *)p_record->peer_addr.addr;
#if (BLE_RS_CONTROLLER_SUPPORT == 0)
        bt_thread_call_func_0(ble_demo_app_tws_sync_info);
#endif
    }
}

void ble_demo_app_tws_sync_info(void)
{
    ble_demo_app_sync_t sync_info[BLE_RECORD_NUM] = {0};
    memset(sync_info, 0, sizeof(sync_info));
    uint8_t i = 0;
    uint8_t j = 0;
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *dev_info = nv_record_blerec_get_ptr();
    BleDeviceinfo *record = NULL;
    uint32_t count = dev_info->saved_list_num;
    for (i = 0; i < count; i++)
    {
        record = dev_info->ble_nv + i;

        if ((record->pairingInfo.bond_info_bf & CO_BIT_MASK(BONDED_WITH_DEMO_APP_ADV)) &&
                record->pairingInfo.peer_addr.addr_type == 0 &&
                record->pairingInfo.LTK[0] != 0 &&
                record->pairingInfo.IRK[0] != 0)
        {
            memcpy(sync_info[j].addr, record->pairingInfo.peer_addr.addr, 6);
            if (record->pairingInfo.bond_info_bf & CO_BIT_MASK(BONDED_SECURE_CONNECTION))
            {
                memcpy(sync_info[j].ltk, record->pairingInfo.LTK, GAP_KEY_LEN);
            }
            else
            {
                memcpy(sync_info[j].ltk, record->pairingInfo.LOCAL_LTK, GAP_KEY_LEN);
            }
            memcpy(sync_info[j].irk, record->pairingInfo.IRK, GAP_KEY_LEN);
            sync_info[j].bond_info_bf = record->pairingInfo.bond_info_bf;
            sync_info[j].local_ediv = record->pairingInfo.LOCAL_EDIV;
            memcpy(sync_info[j].local_rand, record->pairingInfo.LOCAL_RANDOM, 8);
            j++;
        }
    }

    tws_ctrl_send_cmd(APP_TWS_CMD_SEND_DEMO_APP_INFO, (uint8_t *)sync_info, j * sizeof(ble_demo_app_sync_t));
}

static void ble_demo_app_gatt_read_peer_manufacture_name(gap_conn_item_t *conn)
{
#if BLE_GATT_CLIENT_SUPPORT
    gatt_prf_t prf = {0};

    prf.prf_id = p_demo_app_info->prf_lid;
    prf.connhdl = conn->connhdl;
    prf.con_idx = conn->con_idx;

    gattc_read_character_by_uuid_of_service(&prf, 0x0001, 0xFFFF, GATT_CHAR_UUID_MANUFACTURER_NAME_STRING, NULL);
#endif
}

POSSIBLY_UNUSED static void ble_demo_app_disconnect_ble_by_adv_hdl(uint8_t adv_hdl)
{
    uint8_t i = 0;

    gap_conn_item_t *conn = NULL;

    for (i = 0; i < BLE_CONNECTION_MAX; i++)
    {
        conn = gap_get_conn_item(gap_zero_based_ble_conidx_as_hdl(i));

        if (conn != NULL && conn->adv_handle == adv_hdl)
        {
            gap_terminate_connection(conn->connhdl, 0);
            break;
        }
    }
}

void ble_demo_app_tws_role_switch_start(uint8_t curr_ui_role)
{
    if (0x01 != curr_ui_role)
    {
#if (BLE_RS_CONTROLLER_SUPPORT == 0)
        ble_demo_app_tws_sync_info();
#endif
    }

    p_demo_app_info->adv_enable = false;

    app_ble_refresh_adv_state_generic();
}

void ble_demo_app_tws_role_switch_cmp(uint8_t curr_ui_role)
{
    p_demo_app_info->adv_enable = true;
    app_ble_refresh_adv_state_generic();

    if (curr_ui_role == 0x01)
    {
#if (BLE_RS_CONTROLLER_SUPPORT == 0)
        ble_demo_app_disconnect_ble_by_adv_hdl(p_demo_app_info->adv_hdl);
#endif
    }
}

void ble_demo_app_tws_role_update(uint8_t curr_ui_role)
{
    p_demo_app_info->adv_enable = true;
    app_ble_refresh_adv_state_generic();

    if (curr_ui_role == 0x01)
    {
#if (BLE_RS_CONTROLLER_SUPPORT == 0)
        ble_demo_app_disconnect_ble_by_adv_hdl(p_demo_app_info->adv_hdl);
#endif
    }
}

void ble_demo_app_tws_sync_info_receive_handler(uint8_t *p_info, uint16_t len)
{
    BleDevicePairingInfo record;

    uint8_t size = len / sizeof(ble_demo_app_sync_t);
    uint8_t i = 0;

    ble_demo_app_sync_t *p_info_list = (ble_demo_app_sync_t *)p_info;

    while (i < size)
    {
        memset(&record, 0, sizeof(record));

        record.peer_addr.addr_type = 0;
        record.enc_key_size = GAP_KEY_LEN;
        record.bond_info_bf |= p_info_list[i].bond_info_bf;
        memcpy(record.peer_addr.addr, p_info_list[i].addr, sizeof(bt_bdaddr_t));
        memcpy(record.IRK, p_info_list[i].irk, GAP_KEY_LEN);
        if (record.bond_info_bf & CO_BIT_MASK(BONDED_SECURE_CONNECTION))
        {
            memcpy(record.LTK, p_info_list[i].ltk, GAP_KEY_LEN);
        }
        else
        {
            memcpy(record.LOCAL_LTK, p_info_list[i].ltk, GAP_KEY_LEN);
        }
        record.LOCAL_EDIV = p_info_list[i].local_ediv;
        memcpy(record.LOCAL_RANDOM, p_info_list[i].local_rand, 8);
        bluetooth_nv_mgr_ble_record_add(BLE_NV_REC_ADD_CTKD_OVER_BREDR, &record);
        i++;
    }

    if (size > 0)
    {
        app_ble_add_devices_info_to_resolving();
    }
}

static void ble_demo_app_smp_requirements_modify(uint16_t connhdl, ble_smp_require_t *p_requirements)
{
    gap_conn_item_t *conn = gap_get_conn_item(connhdl);

    if (conn == NULL)
    {
        return;
    }

    // If device is not ios device, using no ctkd to ensure that LEA can bond with ctkd in another method
    // And not support ctkd over bredr 
    if ((conn->adv_handle == p_demo_app_info->adv_hdl) &&
                (IS_IOS_DEV(conn->con_idx) == 0) && (IS_CTKD_SUPPORT() == 0))
    {
        TRACE(1, "adv hdl:%d smp reurements modify", conn->adv_handle);
        p_requirements->init_key_dist &= ~GAP_KDIST_LINKKEY;
        p_requirements->resp_key_dist &= ~GAP_KDIST_LINKKEY;
    }
}

static void ble_demo_app_get_specific_irk_ia_handler(uint16_t connhdl, uint8_t **p_irk, bt_bdaddr_t **p_ia)
{
    gap_conn_item_t *conn = gap_get_conn_item(connhdl);

    if (conn == NULL)
    {
        return;
    }

    // Only if from bt acl or le acl with adv using demo app adv
    if (conn->adv_handle == p_demo_app_info->adv_hdl || conn->conn_type == HCI_CONN_TYPE_BT_ACL)
    {
        // Always send specific irk to app adv lead connection smp
        if (p_irk && ((conn->conn_type == HCI_CONN_TYPE_LE_ACL) || IS_CTKD_SUPPORT()))
        {
            // If address is same as bt set, means customer want this adv as ctkd adv using same bt/le addr
            // If not, means this adv just using for single le, not for ctkd over bredr
            // If same bt/le addr or from le connhdl, this app is used for ctkd and using provided irk to start rpa adv
            *p_irk = p_demo_app_info->irk;
        }

        // If not ios device and support ctkd, we can use demo app address as ctkd or signle le using addr
        if (p_ia && ((IS_IOS_DEV(conn->con_idx) == 0) || IS_CTKD_SUPPORT()))
        {
            *p_ia = &p_demo_app_info->app_demo_addr;
        }
    }
}

static bool ble_demo_app_get_specific_record_handler(uint16_t connhdl, const ble_bdaddr_t *peer_addr, BleDevicePairingInfo *pairing_info)
{
    gap_conn_item_t *conn = gap_get_conn_item(connhdl);
    bool found = false;
    uint8_t loop_time = 0;
    ble_bdaddr_t loop_addr = {0};
    memcpy(&loop_addr, peer_addr, sizeof(ble_bdaddr_t));

    while (loop_time < 2)
    {
        found = nv_record_blerec_get_paired_dev_from_addr(pairing_info, (BLE_ADDR_INFO_T *)&loop_addr);

        if (found && conn != NULL)
        {
            if ((conn->adv_handle != p_demo_app_info->adv_hdl &&
                    (pairing_info->bond_info_bf & CO_BIT_MASK(BONDED_WITH_DEMO_APP_ADV))) ||
                    (conn->adv_handle == p_demo_app_info->adv_hdl &&
                     (pairing_info->bond_info_bf & CO_BIT_MASK(BONDED_WITH_DEMO_APP_ADV)) == 0))
            {
                found = false;
            }
        }

        if (found == true)
        {
            break;
        }

        ble_demo_app_trans_addrress(loop_addr.addr);
        loop_time++;
    }

    if (found == true && conn != NULL)
    {
        TRACE(1, "update connection[%d] solved addr:", conn->con_idx);
        DUMP8("%02x ", conn->peer_addr.address, sizeof(bt_bdaddr_t));
        DUMP8("%02x ", loop_addr.addr, sizeof(bt_bdaddr_t));
        // Update to record addr
        conn->sec.peer_addr = *(bt_bdaddr_t *)loop_addr.addr;
    }

    return found;
}

static void ble_demo_app_get_specific_hash_handler(uint16_t connhdl, uint8_t **pp_hash)
{
    gap_conn_item_t *conn = gap_get_conn_item(connhdl);

    if (conn != NULL && conn->adv_handle == p_demo_app_info->adv_hdl)
    {
        *pp_hash = p_demo_app_info->hash;
    }
}

static void ble_demo_app_conn_estb_thread_handler(uint8_t conidx)
{
    const uint8_t empty_hash[GAP_KEY_LEN] = {0};
    gap_conn_item_t *conn = gap_get_conn_item(app_ble_get_conhdl_from_conidx(conidx));

    ble_demo_app_gatt_read_peer_manufacture_name(conn);

    if (conn->adv_handle == p_demo_app_info->adv_hdl)
    {
        // Control LEA service no visible
        gaf_prf_control_all_service(conn->connhdl, false);
        // Generate database hash when control service done
        if (memcpy(p_demo_app_info->hash, empty_hash, GAP_KEY_LEN) == 0)
        {
            gatts_gen_visible_svc_database_hash(conn->connhdl,
                                                ble_demo_app_visible_svc_database_hash_gen_cmp);
        }
    }
}

static void ble_demo_app_visible_svc_database_hash_gen_cmp(void *priv, int error_code, const uint8_t *hash)
{
    struct pp_buff *ppb = (struct pp_buff *)priv;
    uint8_t *raw_data = ppb_get_data(ppb);
    gap_conn_item_t *conn = NULL;

    if (error_code || hash == NULL)
    {
        TRACE(1, "%s %d", __func__, error_code);
        ppb_free(ppb);
        return;
    }

    conn = gap_get_conn_item(CO_COMBINE_UINT16_LE(&raw_data[ppb->len - 2]));

    if (conn != NULL)
    {
        DUMP8("%02x ", hash, 16);
        memcpy(p_demo_app_info->hash, hash, 16);
    }

    ppb_free(ppb);
}

static void ble_demo_app_trans_addrress(uint8_t addr[6])
{
    uint8_t i = 0;

    for (i = 0; i < 6; i++)
    {
        addr[i] = ~addr[i];
    }
}

uint8_t ble_demo_app_add_resolving_list_item(void)
{
    if (p_demo_app_info == NULL)
    {
        return 0;
    }

    return (gap_resolving_list_add_item(BT_ADDR_TYPE_PUBLIC,
                                        &p_demo_app_info->app_demo_addr,
                                        p_demo_app_info->irk,
                                        p_demo_app_info->irk,
                                        false, false) == BT_STS_SUCCESS);
}

static bool ble_demo_app_adv_activity_prepare(ble_adv_activity_t *adv)
{
    gap_adv_param_t *adv_param = &adv->adv_param;
    uint8_t len = 0;
    char *p_demo_adv_name = NULL;
    const char tail_name[] = "_app_adv";
    const char *local_le_name = NULL;

    if (!ble_adv_is_allowed())
    {
        return false;
    }

    if (IS_CTKD_SUPPORT())
    {
        // Disable LE Audio adv
        app_ble_disable_advertising(BLE_AUDIO_ADV_HANDLE);
    }

#if defined(IBRT)
    if (!app_ble_check_ibrt_allow_adv(USER_BLE_DEMO0))
    {
        return false;
    }
#endif

    if (p_demo_app_info->adv_enable == false)
    {
        return false;
    }

    adv->user = USER_BLE_DEMO0;
    adv_param->connectable = true;
    adv_param->scannable = true;
    adv_param->use_legacy_pdu = true;
    adv_param->include_tx_power_data = true;
    adv_param->own_addr_use_rpa = true;
    adv_param->use_fake_btc_rpa_when_no_irk_exist = true;
    adv_param->peer_type = BT_ADDR_TYPE_PUBLIC;
    adv_param->peer_addr = p_demo_app_info->app_demo_addr;

    app_ble_set_adv_tx_power_level(adv, BLE_ADV_TX_POWER_LEVEL_0);

    gap_dt_add_flags(&adv_param->adv_data, GAP_FLAGS_LE_NON_DISCOVERABLE_MODE, false);

    local_le_name = gap_local_le_name(&len);

    if (local_le_name)
    {
        p_demo_adv_name = (char *)bes_bt_buf_malloc(len + sizeof(tail_name));

        if (p_demo_adv_name)
        {
            memcpy(p_demo_adv_name, local_le_name, len);
            memcpy(p_demo_adv_name + len, tail_name, sizeof(tail_name));
        }
    }

    app_ble_dt_set_local_name(adv_param, p_demo_adv_name);

    if (p_demo_adv_name)
    {
        bes_bt_buf_free(p_demo_adv_name);
    }
    return true;
}

bool ble_demo_app_is_connection_support_le_rs(uint16_t connhdl)
{
    if (p_demo_app_info == NULL)
    {
        return false;
    }

    gap_conn_item_t *conn = gap_get_conn_item(connhdl);

    if (conn == NULL || conn->adv_handle != p_demo_app_info->adv_hdl)
    {
        return false;
    }

    return true;
}

uint32_t ble_demo_app_role_switch_handler(uint8_t conidx, bool is_restore, uint8_t *buf, uint16_t buf_len)
{
    if (buf_len < sizeof(bt_bdaddr_t) + GAP_KEY_LEN)
    {
        TRACE(1, "%s no more ctx buf:%d", __func__, buf_len);
        return 0;
    }

    BTIF_CTX_INIT(buf);

    if (is_restore == true)
    {
        BTIF_CTX_LDR_BUF(p_demo_app_info->app_demo_addr.address, sizeof(bt_bdaddr_t));
        BTIF_CTX_LDR_BUF(p_demo_app_info->irk, GAP_KEY_LEN);
        TRACE(1, "demo app restore ctx here");
        DUMP8("%02x ", p_demo_app_info->app_demo_addr.address, sizeof(bt_bdaddr_t));
        DUMP8("%02x ", p_demo_app_info->irk, GAP_KEY_LEN);

        // Refresh resolving list
        bt_thread_call_func_0(app_ble_add_devices_info_to_resolving);
    }
    else
    {
        DUMP8("%02x ", p_demo_app_info->app_demo_addr.address, sizeof(bt_bdaddr_t));
        DUMP8("%02x ", p_demo_app_info->irk, GAP_KEY_LEN);
        BTIF_CTX_STR_BUF(p_demo_app_info->app_demo_addr.address, sizeof(bt_bdaddr_t));
        BTIF_CTX_STR_BUF(p_demo_app_info->irk, GAP_KEY_LEN);
        BTIF_CTX_SAVE_UPDATE_DATA_LEN();

        TRACE(1, "demo app save ctx here");
    }

    return BTIF_CTX_GET_TOTAL_LEN();
}

#if BLE_GATT_CLIENT_SUPPORT
static int ble_demo_app_gatt_client_callback(gatt_prf_t *prf, gatt_profile_event_t event, gatt_profile_callback_param_t param)
{
    if (p_demo_app_info == NULL)
    {
        return false;
    }

    switch (event)
    {
        case GATT_PROF_EVENT_OPENED:
        case GATT_PROF_EVENT_CLOSED:
        case GATT_PROF_EVENT_SERVICE:
        case GATT_PROF_EVENT_CHARACTER:
            break;
        case GATT_PROF_EVENT_CHAR_READ_RSP:
        {
            gatt_profile_char_read_rsp_t *p = param.char_read_rsp;
            bool read_success = (p->error_code == ATT_ERROR_NO_ERROR) && p->value_len;
            uint16_t char_uuid = gatt_char_16_uuid_le(p->character);
            if (!read_success)
            {
                break;
            }
            if (char_uuid == GATT_CHAR_UUID_MANUFACTURER_NAME_STRING)
            {
                DUMP8("%02x ", p->value, p->value_len);

                if (memcmp(p->value, APPLE_INC_MANUFACTURE_NAME_STR, p->value_len >= 10 ? 10 : p->value_len) == 0)
                {
                    p_demo_app_info->ios_dev_bf |= (1 << gap_zero_based_conidx(p->conn->con_idx));
                }
                else
                {
                    p_demo_app_info->ios_dev_bf &= ~(1 << gap_zero_based_conidx(p->conn->con_idx));
                }

                TRACE(1, "demo app ios dev bf: 0x%x", p_demo_app_info->ios_dev_bf);
            }
            break;
        }
        case GATT_PROF_EVENT_NOTIFY:
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

#endif
