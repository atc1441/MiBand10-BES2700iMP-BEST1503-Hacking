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
/*INCLUDE*/
#include "bt_common_define.h"
#include "nvrecord_extension.h"
#include "nvrecord_env.h"
#include "nvrecord_bt.h"
#include "nvrecord_ble.h"

#include "bluetooth_nv_mgr.h"

/*DEFINETIONS*/

/*DECLARTATION*/

/*EXTERN*/
int nv_record_blerec_add(const BleDevicePairingInfo *param_rec);
void nv_record_ble_delete_entry(const uint8_t *pBdAddr);

#define NV_MGR_HOOK_LIST_SIZE  (3)

typedef struct {
    bluetooth_nv_mgr_del_cb_t *elements[NV_MGR_HOOK_LIST_SIZE];
    int size;
} bluetooth_nv_mgr_hook_list_t;

static bluetooth_nv_mgr_hook_list_t     bluetooth_nv_mgr_hooks;

void bluetooth_nv_mgr_register_callback(bluetooth_nv_mgr_del_cb_t *cb)
{
    if (!cb)
    {
        TRACE(1, "Warning:input null pointer");
        return;
    }
    if (bluetooth_nv_mgr_hooks.size < NV_MGR_HOOK_LIST_SIZE)
    {
        bluetooth_nv_mgr_hooks.elements[bluetooth_nv_mgr_hooks.size] = cb;
        bluetooth_nv_mgr_hooks.size ++;
    }
}

static void bluetooth_nv_mgr_ble_record_del_hook(const uint8_t *p_addr)
{
    uint8_t size = bluetooth_nv_mgr_hooks.size;
    bluetooth_nv_mgr_del_cb_t *cbs = NULL;
    for (int i = 0; i < size; i ++)
    {
        cbs = bluetooth_nv_mgr_hooks.elements[i];
        if (cbs->nv_mgr_del_ble_record)
        {
            cbs->nv_mgr_del_ble_record(p_addr);
        }
    }
}

/*FUNCTIONS*/
int bluetooth_nv_mgr_ble_record_add(le_nv_rec_add_evt input_event, const bluetooth_nv_le_record_t *p_record)
{
    if (p_record == NULL || input_event >= BLE_NV_REC_ADD_EVENT_MAX)
    {
        return BT_STS_INVALID_PARM;
    }

    TRACE(1, "NV-LE-ADD input event:%d, ca:%p", input_event, __builtin_return_address(0));

    return nv_record_blerec_add(p_record);;
}

int bluetooth_nv_mgr_ble_record_del(le_nv_rec_del_evt input_event, const uint8_t *p_addr)
{
    if (p_addr == NULL || input_event >= BLE_NV_REC_DEL_EVENT_MAX)
    {
        return BT_STS_INVALID_PARM;
    }

    TRACE(1, "NV-LE-DEL input event:%d, ca:%p", input_event, __builtin_return_address(0));

    nv_record_ble_delete_entry(p_addr);
    bluetooth_nv_mgr_ble_record_del_hook(p_addr);
    return BT_STS_SUCCESS;
}

int bluetooth_nv_mgr_bt_record_add(bt_nv_rec_add_evt input_event, const bluetooth_nv_bt_record_t *p_record)
{
    if (p_record == NULL || input_event >= BT_NV_REC_ADD_EVENT_MAX)
    {
        return BT_STS_INVALID_PARM;
    }

    TRACE(1, "NV-BT-ADD input event:%d, ca:%p", input_event, __builtin_return_address(0));
    
    switch (input_event)
    {
        case BT_NV_REC_ADD_CTKD_OVER_LE:
            nv_record_ddbrec_delete((bt_bdaddr_t *)p_record);
        break;
        default:
        break;
    }

    return nv_record_add(section_usrdata_ddbrecord, (void *)p_record);
}

int bluetooth_nv_mgr_bt_record_del(bt_nv_rec_del_evt input_event, const uint8_t *p_addr)
{
    if (p_addr == NULL || input_event >= BT_NV_REC_DEL_EVENT_MAX)
    {
        return BT_STS_INVALID_PARM;
    }

    TRACE(1, "NV-BT-DEL input event:%d, ca:%p", input_event, __builtin_return_address(0));

    nv_record_ddbrec_delete((bt_bdaddr_t *)p_addr);

    return BT_STS_SUCCESS;
}
