/***************************************************************************
 *
 * Copyright 2024-2029 BES.
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
#undef MOUDLE
#define MOUDLE APP_BLE
#include "app_ble.h" 
#ifdef FINDMY_ENABLED // Apple find my network server
#include "ble_findmy.h"
#include "bt_drv_interface.h"

#define FINDMY_PAIRING_CTRL_POINT      0x7A, 0x42, 0x04, 0x03, 0x73, 0x2F, 0xD4, 0xBE, 0xEF, 0x49, 0x3B, 0x94, 0x01, 0x00, 0x86, 0x4F
#define FINDMY_CONFIG_CTRL_POINT       0x7A, 0x42, 0x04, 0x03, 0x73, 0x2F, 0xD4, 0xBE, 0xEF, 0x49, 0x3B, 0x94, 0x02, 0x00, 0x86, 0x4F
#define FINDMY_NON_OWNER_CTRL_POINT    0x7A, 0x42, 0x04, 0x03, 0x73, 0x2F, 0xD4, 0xBE, 0xEF, 0x49, 0x3B, 0x94, 0x03, 0x00, 0x86, 0x4F
#define FINDMY_OWNER_CTRL_POINT        0x7A, 0x42, 0x04, 0x03, 0x73, 0x2F, 0xD4, 0xBE, 0xEF, 0x49, 0x3B, 0x94, 0x04, 0x00, 0x86, 0x4F
#define FINDMY_DEBUG_CTRL_POINT        0x7A, 0x42, 0x04, 0x03, 0x73, 0x2F, 0xD4, 0xBE, 0xEF, 0x49, 0x3B, 0x94, 0x05, 0x00, 0x86, 0x4F

#define FINDMY_INFO_SRV                0x8B, 0x47, 0x38, 0xDC, 0xB9, 0x11, 0xA9, 0xA1, 0xB1, 0x43, 0x51, 0x3C, 0x02, 0x01, 0x29, 0x87
#define FINDMY_INFO_PROC_DATA          0x0B, 0xBB, 0x6F, 0x41, 0x3A, 0x00, 0xB4, 0xA7, 0x57, 0x4D, 0x52, 0x63, 0x01, 0x00, 0xA5, 0x6A
#define FINDMY_INFO_MANU_NAME          0x0B, 0xBB, 0x6F, 0x41, 0x3A, 0x00, 0xB4, 0xA7, 0x57, 0x4D, 0x52, 0x63, 0x02, 0x00, 0xA5, 0x6A
#define FINDMY_INFO_MODEL_NAME         0x0B, 0xBB, 0x6F, 0x41, 0x3A, 0x00, 0xB4, 0xA7, 0x57, 0x4D, 0x52, 0x63, 0x03, 0x00, 0xA5, 0x6A
#define FINDMY_INFO_RESERVED           0x0B, 0xBB, 0x6F, 0x41, 0x3A, 0x00, 0xB4, 0xA7, 0x57, 0x4D, 0x52, 0x63, 0x04, 0x00, 0xA5, 0x6A
#define FINDMY_INFO_CATEGORY           0x0B, 0xBB, 0x6F, 0x41, 0x3A, 0x00, 0xB4, 0xA7, 0x57, 0x4D, 0x52, 0x63, 0x05, 0x00, 0xA5, 0x6A
#define FINDMY_INFO_CAPABILITY         0x0B, 0xBB, 0x6F, 0x41, 0x3A, 0x00, 0xB4, 0xA7, 0x57, 0x4D, 0x52, 0x63, 0x06, 0x00, 0xA5, 0x6A
#define FINDMY_INFO_FW_VERSION         0x0B, 0xBB, 0x6F, 0x41, 0x3A, 0x00, 0xB4, 0xA7, 0x57, 0x4D, 0x52, 0x63, 0x07, 0x00, 0xA5, 0x6A
#define FINDMY_INFO_FD_VERSION         0x0B, 0xBB, 0x6F, 0x41, 0x3A, 0x00, 0xB4, 0xA7, 0x57, 0x4D, 0x52, 0x63, 0x08, 0x00, 0xA5, 0x6A
#define FINDMY_INFO_BATTERY_TYPE       0x0B, 0xBB, 0x6F, 0x41, 0x3A, 0x00, 0xB4, 0xA7, 0x57, 0x4D, 0x52, 0x63, 0x09, 0x00, 0xA5, 0x6A
#define FINDMY_INFO_BATTERY_LEVEL      0x0B, 0xBB, 0x6F, 0x41, 0x3A, 0x00, 0xB4, 0xA7, 0x57, 0x4D, 0x52, 0x63, 0x0A, 0x00, 0xA5, 0x6A

#define FINDMY_DEBUG

GATT_DECL_PRI_SERVICE(g_ble_findmy_service,
    FINDMY_SERVICE_UUID);

GATT_DECL_128_LE_CHAR(g_ble_findmy_pair_ctrl_character,
    FINDMY_PAIRING_CTRL_POINT,
    GATT_WR_REQ|GATT_IND_PROP|GATT_RD_REQ,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_findmy_pair_ctrl_cccd,
    ATT_SEC_NONE);

GATT_DECL_128_LE_CHAR(g_ble_findmy_config_ctrl_character,
    FINDMY_CONFIG_CTRL_POINT,
    GATT_WR_REQ|GATT_IND_PROP|GATT_RD_REQ,
    ATT_WR_ENC);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_findmy_config_ctrl_cccd,
    ATT_WR_ENC);

GATT_DECL_128_LE_CHAR(g_ble_findmy_nonowner_ctrl_character,
    FINDMY_NON_OWNER_CTRL_POINT,
    GATT_WR_REQ|GATT_IND_PROP|GATT_RD_REQ,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_findmy_nonowner_ctrl_cccd,
    ATT_SEC_NONE);

GATT_DECL_128_LE_CHAR(g_ble_findmy_owner_ctrl_character,
    FINDMY_OWNER_CTRL_POINT,
    GATT_WR_REQ|GATT_IND_PROP|GATT_RD_REQ,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_findmy_owner_ctrl_cccd,
    ATT_SEC_NONE);

#ifdef FINDMY_DEBUG
GATT_DECL_128_LE_CHAR(g_ble_findmy_debug_ctrl_character,
    FINDMY_DEBUG_CTRL_POINT,
    GATT_WR_REQ|GATT_IND_PROP|GATT_RD_REQ,
    ATT_WR_ENC);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_findmy_debug_ctrl_cccd,
    ATT_WR_ENC);
#endif

GATT_DECL_128_LE_PRI_SERVICE(g_ble_findmy_info_service,
    FINDMY_INFO_SRV);
GATT_DECL_128_LE_CHAR(g_ble_findmy_info_proc_data_character,
    FINDMY_INFO_PROC_DATA,
    GATT_RD_REQ,
    ATT_SEC_NONE);
GATT_DECL_128_LE_CHAR(g_ble_findmy_info_manu_name_character,
    FINDMY_INFO_MANU_NAME,
    GATT_RD_REQ,
    ATT_SEC_NONE);
GATT_DECL_128_LE_CHAR(g_ble_findmy_info_model_name_character,
    FINDMY_INFO_MODEL_NAME,
    GATT_RD_REQ,
    ATT_SEC_NONE);
GATT_DECL_128_LE_CHAR(g_ble_findmy_info_reserved_character,
    FINDMY_INFO_RESERVED,
    GATT_RD_REQ,
    ATT_SEC_NONE);
GATT_DECL_128_LE_CHAR(g_ble_findmy_info_category_character,
    FINDMY_INFO_CATEGORY,
    GATT_RD_REQ,
    ATT_SEC_NONE);
GATT_DECL_128_LE_CHAR(g_ble_findmy_info_capability_character,
    FINDMY_INFO_CAPABILITY,
    GATT_RD_REQ,
    ATT_SEC_NONE);
GATT_DECL_128_LE_CHAR(g_ble_findmy_info_fw_ver_character,
    FINDMY_INFO_FW_VERSION,
    GATT_RD_REQ,
    ATT_SEC_NONE);
GATT_DECL_128_LE_CHAR(g_ble_findmy_info_fd_ver_character,
    FINDMY_INFO_FD_VERSION,
    GATT_RD_REQ,
    ATT_SEC_NONE);
GATT_DECL_128_LE_CHAR(g_ble_findmy_info_battery_type_character,
    FINDMY_INFO_BATTERY_TYPE,
    GATT_RD_REQ,
    ATT_SEC_NONE);
GATT_DECL_128_LE_CHAR(g_ble_findmy_info_battery_lvl_character,
    FINDMY_INFO_BATTERY_LEVEL,
    GATT_RD_REQ,
    ATT_SEC_NONE);

static const gatt_attribute_t g_ble_findmy_attr_list[] = {
    /* Service */
    gatt_attribute(g_ble_findmy_service),
    /* Characteristics */
    gatt_attribute(g_ble_findmy_pair_ctrl_character),
    gatt_attribute(g_ble_findmy_pair_ctrl_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_findmy_config_ctrl_character),
    gatt_attribute(g_ble_findmy_config_ctrl_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_findmy_nonowner_ctrl_character),
    gatt_attribute(g_ble_findmy_nonowner_ctrl_cccd),
    /* Characteristics */
    gatt_attribute(g_ble_findmy_owner_ctrl_character),
    gatt_attribute(g_ble_findmy_owner_ctrl_cccd),
#ifdef FINDMY_DEBUG
    /* Characteristics */
    gatt_attribute(g_ble_findmy_debug_ctrl_character),
    gatt_attribute(g_ble_findmy_debug_ctrl_cccd),
#endif
};

static const gatt_attribute_t g_ble_findmy_info_attr_list[] = {
gatt_attribute(g_ble_findmy_info_service),
gatt_attribute(g_ble_findmy_info_proc_data_character),
gatt_attribute(g_ble_findmy_info_manu_name_character),
gatt_attribute(g_ble_findmy_info_model_name_character),
gatt_attribute(g_ble_findmy_info_reserved_character),
gatt_attribute(g_ble_findmy_info_category_character),
gatt_attribute(g_ble_findmy_info_capability_character),
gatt_attribute(g_ble_findmy_info_fw_ver_character),
gatt_attribute(g_ble_findmy_info_fd_ver_character),
gatt_attribute(g_ble_findmy_info_battery_type_character),
gatt_attribute(g_ble_findmy_info_battery_lvl_character),
};

typedef struct {
    ble_findmy_event_cb callback;
} ble_findmy_global_t;

static ble_findmy_dev_info_t g_findmy_devInfo;
static ble_findmy_global_t g_ble_findmy_global;
ble_findmy_global_t *ble_finmy_get_global(void)
{
    return &g_ble_findmy_global;
}

void ble_findmy_register_event_callback(ble_findmy_event_cb func)
{
    ble_findmy_global_t *g = ble_finmy_get_global();
    if (g)
    {
        g->callback = func;
    }
}

void ble_findmy_init_dev_info(ble_findmy_dev_info_t *devInfo)
{
    if (devInfo)
    {
        memcpy((uint8_t *)&g_findmy_devInfo, devInfo, sizeof(ble_findmy_dev_info_t));
    }
}

void ble_findmy_set_service_visible(ble_findmy_services_e service, bool visible, uint16_t connHdl)
{
    gatt_attribute_t *srvPtr = NULL;
    switch (service)
    {
        case BLE_FINDMY_SRV_PAIRING_CTRL:
        srvPtr = (gatt_attribute_t *)g_ble_findmy_attr_list + 1;
        break;

        case BLE_FINDMY_SRV_CONFIG_CTRL:
        srvPtr = (gatt_attribute_t *)g_ble_findmy_attr_list + 3;
        break;

        case BLE_FINDMY_SRV_NONOWNER_CTRL:
        srvPtr = (gatt_attribute_t *)g_ble_findmy_attr_list + 5;
        break;

        case BLE_FINDMY_SRV_OWNER_CTRL:
        srvPtr = (gatt_attribute_t *)g_ble_findmy_attr_list + 7;
        break;

        case BLE_FINDMY_SRV_DEBUG_CTRL:
        srvPtr = (gatt_attribute_t *)g_ble_findmy_attr_list + 9;
        break;

        default:
        break;
    }

    if (srvPtr)
    {
        gatts_control_service(srvPtr, 0, visible, connHdl);
    }
}

void ble_findmy_enter_paired_mode(uint8_t conidx)
{
    uint16_t connHdl = gap_conn_hdl(gap_zero_based_conidx_to_ble_conidx(conidx));
    ble_findmy_set_service_visible(BLE_FINDMY_SRV_PAIRING_CTRL, false, connHdl);
    ble_findmy_set_service_visible(BLE_FINDMY_SRV_CONFIG_CTRL, false, connHdl);
}

void ble_findmy_notify_pair_complete_handler(uint8_t conidx, uint8_t *addr)
{
    ble_findmy_event_param_t entry;
    ble_findmy_global_t *g = ble_finmy_get_global();
    
    entry.conidx = conidx;
    entry.event = BLE_FINDMY_EVT_LE_PAIR_CMP;
    memcpy(entry.peerAddr.address, addr, sizeof(bt_bdaddr_t));
    if (g && g->callback)
    {
        g->callback(&entry);
    }
}

static int ble_findmy_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    ble_findmy_event_param_t entry;
    ble_findmy_global_t *g = ble_finmy_get_global();

    switch (event)
    {
        case GATT_SERV_EVENT_CONN_OPENED:
        {
            gatt_server_conn_opened_t *p = param.opened;
            entry.conidx = p->con_idx;
            entry.event = BLE_FINDMY_EVT_LE_CONNECTED;
            memcpy(entry.peerAddr.address, p->conn->peer_addr.address, sizeof(bt_bdaddr_t));
            break;
        }

        case GATT_SERV_EVENT_CONN_CLOSED:
        {
            gatt_server_conn_closed_t *p = param.closed;
            entry.conidx = p->con_idx;
            entry.reason = p->error_code;
            entry.event = BLE_FINDMY_EVT_LE_DISCONNECTED;
            break;
        }

        case GATT_SERV_EVENT_CONN_ENCRYPTED:
        {
            gatt_server_conn_encrypted_t *p = param.conn_encrypted;
            entry.conidx = p->con_idx;
            entry.event = BLE_FINDMY_EVT_LE_ENCRYPTED;
            memcpy(entry.peerAddr.address, p->conn->sec.peer_addr.address, sizeof(bt_bdaddr_t));
            memcpy(entry.ltk, p->conn->sec.ltk, 16);
            break;
        }

        case GATT_SERV_EVENT_MTU_CHANGED:
        {
            gatt_server_mtu_changed_t *p = param.mtu_changed;
            entry.conidx = p->con_idx;
            entry.mtu = p->mtu;
            entry.event = BLE_FINDMY_EVT_LE_MTU_EXC_DONE;
            break;
        }

        case GATT_SERV_EVENT_INDICATE_CFM:
        {
            gatt_server_indicate_cfm_t *p = param.confirm;
            entry.conidx = p->con_idx;
            if (param.confirm->character == g_ble_findmy_pair_ctrl_character) {
                entry.event = BLE_FINDMY_EVT_PAIR_CTRL_IND_CFM;
            } else if (param.confirm->character == g_ble_findmy_config_ctrl_character) {
                entry.event = BLE_FINDMY_EVT_CONFIG_CTRL_IND_CFM;
            } else if (param.confirm->character == g_ble_findmy_nonowner_ctrl_character) {
                entry.event = BLE_FINDMY_EVT_NONOWNER_CTRL_IND_CFM;
            } else if (param.confirm->character == g_ble_findmy_owner_ctrl_character) {
                entry.event = BLE_FINDMY_EVT_OWNER_CTRL_IND_CFM;
            } else if (param.confirm->character == g_ble_findmy_debug_ctrl_character) {
                entry.event = BLE_FINDMY_EVT_DEBUG_CTRL_IND_CFM;
            }
            else {
                TRACE(0, "%s indicate cfm is error!!!", __func__);
            }
            break;
        }

        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            gatt_server_char_write_t *p = param.char_write;
            entry.conidx = p->con_idx;
            entry.pBuf = (uint8_t *)p->value;
            entry.len = p->value_len;

            if (p->value_offset != 0 || p->value_len == 0 || p->value == NULL)
            {
                return false;
            }
            if (p->character == g_ble_findmy_pair_ctrl_character)
            {
                entry.event = BLE_FINDMY_EVT_RX_REC_PAIR_CTRL;
            }
            else if (p->character == g_ble_findmy_pair_ctrl_cccd)
            {
                entry.event = BLE_FINDMY_EVT_RX_REC_PAIR_CTRL_CCCD;
            }
            else if (p->character == g_ble_findmy_config_ctrl_character)
            {
                entry.event = BLE_FINDMY_EVT_RX_REC_CFG_CTRL;
            }
            else if (p->character == g_ble_findmy_config_ctrl_cccd)
            {
                entry.event = BLE_FINDMY_EVT_RX_REC_CFG_CTRL_CCCD;
            }
            else if (p->character == g_ble_findmy_nonowner_ctrl_character)
            {
                entry.event = BLE_FINDMY_EVT_RX_REC_NON_OWNER_CTRL;
            }
            else if (p->character == g_ble_findmy_nonowner_ctrl_cccd)
            {
                entry.event = BLE_FINDMY_EVT_RX_REC_NON_OWNER_CTRL_CCCD;
            }
            else if (p->character == g_ble_findmy_owner_ctrl_character)
            {
                entry.event = BLE_FINDMY_EVT_RX_REC_OWNER_CTRL;
            }
            else if (p->character == g_ble_findmy_owner_ctrl_cccd)
            {
                entry.event = BLE_FINDMY_EVT_RX_REC_OWNER_CTRL_CCCD;
            }
            else if (p->character == g_ble_findmy_debug_ctrl_character)
            {
                entry.event = BLE_FINDMY_EVT_RX_REC_DEBUG_CTRL;
            }
            else if (p->character == g_ble_findmy_debug_ctrl_cccd)
            {
                entry.event = BLE_FINDMY_EVT_RX_REC_DEBUG_CTRL_CCCD;
            }
            else
            {
                return false;
            }
            p->rsp_error_code = ATT_ERROR_NO_ERROR;
            gatts_send_write_rsp(p->ctx, p->rsp_error_code);
            break;
        }

        default:
        {
            return false;
        }
    }

    if (g && g->callback)
    {
        g->callback(&entry);
    }
    return true;
}

static int ble_findmy_info_server_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
    switch (event)
    {
        case GATT_SERV_EVENT_CONN_OPENED:
        case GATT_SERV_EVENT_CONN_CLOSED:
        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            break;
        }
        case GATT_SERV_EVENT_CHAR_READ:
        {
            gatt_server_char_read_t *p = param.char_read;
            if (p->character == g_ble_findmy_info_proc_data_character)
            {
                gatts_write_read_rsp_data(p->ctx, g_findmy_devInfo.productData, 8);
            }
            else if(p->character == g_ble_findmy_info_manu_name_character)
            {
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)g_findmy_devInfo.manuName, 64);
            }
            else if(p->character == g_ble_findmy_info_model_name_character)
            {
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)g_findmy_devInfo.modelName, 64);
            }
            else if(p->character == g_ble_findmy_info_reserved_character)
            {
                gatts_write_read_rsp_data(p->ctx, NULL, 0);
            }
            else if(p->character == g_ble_findmy_info_category_character)
            {
                gatts_write_read_rsp_data(p->ctx, g_findmy_devInfo.category, 8);
            }
            else if(p->character == g_ble_findmy_info_capability_character)
            {
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&(g_findmy_devInfo.capability), sizeof(uint32_t));
            }
            else if(p->character == g_ble_findmy_info_fw_ver_character)
            {
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&(g_findmy_devInfo.fwVer), sizeof(uint32_t));
            }
            else if(p->character == g_ble_findmy_info_fd_ver_character)
            {
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&(g_findmy_devInfo.findmyVer), sizeof(uint32_t));
            }
            else if(p->character == g_ble_findmy_info_battery_type_character)
            {
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&(g_findmy_devInfo.batteryType), sizeof(uint8_t));
            }
            else if(p->character == g_ble_findmy_info_battery_lvl_character)
            {
                gatts_write_read_rsp_data(p->ctx, (uint8_t *)&(g_findmy_devInfo.batteryLevel), sizeof(uint8_t));
            }
            else
            {
                return false;
            }
            break;
        }
        default:
        {
            return false;
        }
    }

    return true;
}

bt_status_t ble_findmy_send_pairing_data(uint8_t conidx, const uint8_t *value, uint16_t len)
{
    return gatts_send_indication(gap_conn_bf(conidx), g_ble_findmy_pair_ctrl_character, value, len);
}

bt_status_t ble_findmy_send_config_data(uint8_t conidx, const uint8_t *value, uint16_t len)
{
    return gatts_send_indication(gap_conn_bf(conidx), g_ble_findmy_config_ctrl_character, value, len);
}

bt_status_t ble_findmy_send_nonowner_data(uint8_t conidx, const uint8_t *value, uint16_t len)
{
    return gatts_send_indication(gap_conn_bf(conidx), g_ble_findmy_nonowner_ctrl_character, value, len);
}

bt_status_t ble_findmy_send_paired_owner_data(uint8_t conidx, const uint8_t *value, uint16_t len)
{
    return gatts_send_indication(gap_conn_bf(conidx), g_ble_findmy_owner_ctrl_character, value, len);
}

bt_status_t ble_findmy_send_debug_data(uint8_t conidx, const uint8_t *value, uint16_t len)
{
    return gatts_send_indication(gap_conn_bf(conidx), g_ble_findmy_debug_ctrl_character, value, len);
}

void ble_findmy_init(app_ble_adv_activity_func advFunc, ble_findmy_event_cb evtCb, ble_findmy_dev_info_t *devInfo)
{
    ble_findmy_init_dev_info(devInfo);
    ble_findmy_register_event_callback(evtCb);
    app_ble_register_advertising(BLE_FINDMY_ADV_HANDLE, advFunc);

    gatts_cfg_t cfg = {0};
    cfg.svc_size = sizeof(gatt_svc_t);
    cfg.preferred_mtu = FINDMY_PREF_MTU;
    cfg.eatt_preferred = false;
    gatts_register_service(g_ble_findmy_attr_list, ARRAY_SIZE(g_ble_findmy_attr_list), ble_findmy_server_callback, &cfg);
    gatts_register_service(g_ble_findmy_info_attr_list, ARRAY_SIZE(g_ble_findmy_info_attr_list), ble_findmy_info_server_callback, &cfg);
}

#endif /* FINDMY_ENABLED */
