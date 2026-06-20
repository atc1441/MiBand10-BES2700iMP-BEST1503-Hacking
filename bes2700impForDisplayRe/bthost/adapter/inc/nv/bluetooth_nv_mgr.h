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
#ifndef __NVRECORD_MGR_H__
#define __NVRECORD_MGR_H__

#include "ble_device_info.h"
#include "bt_common_define.h"
#include "me_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef BleDevicePairingInfo bluetooth_nv_le_record_t;

typedef enum ble_nv_record_add_event
{
    BLE_NV_REC_ADD_EVENT_MIN = 0,
    // New paired devices from SMP
    BLE_NV_REC_ADD_SMP_NEW_PAIRED,
    // New paired devices from CTKD over BREDR
    BLE_NV_REC_ADD_CTKD_OVER_BREDR,
    // LE sync info received handler
    BLE_NV_REC_ADD_LE_SYNC_INFO_RECV,
    // LE demo app sync info recv
    BLE_NV_REC_ADD_LE_DEMO_APP_SYNC,
    // LE RS cmd nv record
    BLE_NV_REC_ADD_LE_SWITCH_SYNC,

    BLE_NV_REC_ADD_EVENT_MAX,
} le_nv_rec_add_evt;

typedef enum ble_nv_record_del_event
{
    BLE_NV_REC_DEL_EVENT_MIN = 0,
    BLE_NV_REC_DEL_LE_MIC_FAILURE,
    BLE_NV_REC_DEL_BT_NV_REC_CHG,
    BLE_NV_REC_DEL_LE_ENC_FAILURE,

    BLE_NV_REC_DEL_EVENT_MAX,
} le_nv_rec_del_evt;

typedef void bluetooth_nv_bt_record_t;

typedef enum bt_nv_record_add_event
{
    BT_NV_REC_ADD_EVENT_MIN = 0,
    // New paired devices lk generated
    BT_NV_REC_ADD_LINKKEY_GENERATED,
    // New paired devices enc changed
    BT_NV_REC_ADD_ENCRYPTION_CHANGED,
    // New paired devices from CTKD over LE
    BT_NV_REC_ADD_CTKD_OVER_LE,
    // DIP sync info received handler
    BT_NV_REC_ADD_DIP_SYNC_INFO_RECV,
    // RESTORE CONTEXT
    BT_NV_REC_ADD_CTX_RESTORE_SET,
    // Map share info dev to nv
    BT_NV_REC_ADD_REMAP_SHARE_DEV,
    // Merge share info dev to nv
    BT_NV_REC_ADD_UPD_SHARE_DEV_INFO,
    // Choice mobile connect
    BT_NV_REC_ADD_CHOICE_DEV_CONNECT,
    // Update device to nv top
    BT_NV_REC_ADD_UPD_DEV_TO_NV_TOP,
    // Config keeper resume
    BT_NV_REC_ADD_CFG_KEPPER_RESUME,
    // Auth fail only keep record with no sec
    BT_NV_REC_ADD_AUTH_FAIL_KEEP,
    // Name request result
    BT_NV_REC_ADD_NAME_RESULT_SAVE,

    BT_NV_REC_ADD_EVENT_MAX,
} bt_nv_rec_add_evt;

typedef enum bt_nv_record_del_event
{
    BT_NV_REC_DEL_EVENT_MIN = 0,
    BT_NV_REC_DEL_ALL_PAIRED_DEV,
    BT_NV_REC_DEL_ALL_PAIRED_MOB,
    BT_NV_REC_DEL_TWS_RECORD,
    BT_NV_REC_DEL_REMOVE_BOND,
    BT_NV_REC_DEL_PAIRING_FAILED,
    BT_NV_REC_DEL_AUTHEN_FAILED,
    BT_NV_REC_DEL_DISC_ERR_KEY,
    BT_NV_REC_DEL_DESTROY_DEV,

    BT_NV_REC_DEL_EVENT_MAX,
} bt_nv_rec_del_evt;

typedef struct {
    void (*nv_mgr_del_ble_record)(const uint8_t *p_addr);
    void (*nv_mgr_del_bt_record)(const uint8_t *p_addr);
} bluetooth_nv_mgr_del_cb_t;

/*DECLARATIONS*/
void bluetooth_nv_mgr_register_callback(bluetooth_nv_mgr_del_cb_t *cb);

int bluetooth_nv_mgr_ble_record_add(le_nv_rec_add_evt input_event, const bluetooth_nv_le_record_t *p_record);
int bluetooth_nv_mgr_ble_record_del(le_nv_rec_del_evt input_event, const uint8_t *p_addr);

int bluetooth_nv_mgr_bt_record_add(bt_nv_rec_add_evt input_event, const bluetooth_nv_bt_record_t *p_record);
int bluetooth_nv_mgr_bt_record_del(bt_nv_rec_del_evt input_event, const uint8_t *p_addr);


#ifdef __cplusplus
}
#endif

#endif /* __NVRECORD_MGR_H__ */
