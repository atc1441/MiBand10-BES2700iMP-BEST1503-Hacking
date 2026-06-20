/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_bootmode.h"
#include "app_audio.h"
#include "app_utils.h"
#include "hal_aud.h"
#include "hal_norflash.h"
#include "pmu.h"
#include "string.h"
#include "stdio.h"
#include "stdarg.h"
#include "cmsis_os.h"
#include "app_tota.h"
#include "app_tota_cmd_code.h"
#include "app_tota_cmd_handler.h"
#include "app_spp_tota.h"
#include "cqueue.h"
#ifdef __IAG_BLE_INCLUDE__
#include "bluetooth_ble_api.h"
#endif
#include "bluetooth_bt_api.h"
#include "app_bt_audio.h"
#include "btapp.h"
#include "app_bt.h"
#include "apps.h"
#include "app_thread.h"
#include "cqueue.h"
#include "hal_location.h"
#include "app_hfp.h"
#include "bt_drv_reg_op.h"
#if defined(IBRT)
#include "app_tws_ibrt.h"
#include "app_tws_ctrl_thread.h"
#include "app_ibrt_ota_cmd.h"
#endif
#include "cmsis.h"
#include "app_battery.h"
#include "crc32_c.h"
#include "factory_section.h"
#include "tota_stream_data_transfer.h"
#if (BLE_APP_TOTA)
#include "app_tota_ble.h"
#endif
#if defined(OTA_OVER_TOTA_ENABLED)
#include "ota_control.h"
#include "ota_basic.h"
#include "app_ibrt_ota_cmd.h"
#include "app_tws_ctrl_thread.h"
#endif
#include "app_tota_encrypt.h"
#include "app_tota_flash_program.h"
#include "app_tota_audio_dump.h"
#include "app_tota_mic.h"
#include "app_tota_anc.h"
#include "app_tota_general.h"
#include "app_tota_custom.h"
#include "app_tota_conn.h"
#include "encrypt/aes.h"
#include "app_tota_audio_EQ.h"
#include "app_tota_common.h"
#include "audio_cfg.h"
#include "app_bt_cmd.h"

#include <map>

#include "beslib_info.h"

using namespace std;

#define DONGLE_DATA_FRAME 624

static APP_TOTA_CMD_PAYLOAD_T *payload = NULL;
/*call back sys for modules*/
static map<APP_TOTA_MODULE_E, const app_tota_callback_func_t *>     s_module_map;
static const app_tota_callback_func_t                             *s_module_func;
static APP_TOTA_MODULE_E                                    s_module;

#ifdef BLE_TOTA_ENABLED
/// health thermometer application environment structure
struct ble_tota_env_info
{
    uint8_t  connectionIndex;
    uint8_t  isNotificationEnabled;
    uint16_t connhdl;
    uint16_t mtu;
};

struct ble_tota_env_info ble_tota_env;
#endif

/* register callback module */
void app_tota_callback_module_register(APP_TOTA_MODULE_E module,
                                const app_tota_callback_func_t *tota_callback_func)
{
    map<APP_TOTA_MODULE_E, const app_tota_callback_func_t *>::iterator it = s_module_map.find(module);
    if ( it == s_module_map.end() )
    {
        TOTA_LOG_DBG(0, "add to map");
        s_module_map.insert(make_pair(module, tota_callback_func));
    }
    else
    {
        TOTA_LOG_DBG(0, "already exist, not add");
    }
}
/* set current module */
void app_tota_callback_module_set(APP_TOTA_MODULE_E module)
{
    map<APP_TOTA_MODULE_E, const app_tota_callback_func_t *>::iterator it = s_module_map.find(module);
    if ( it != s_module_map.end() )
    {
        s_module = module;
        s_module_func = it->second;
        TOTA_LOG_DBG(1, "set %d module success", module);
    }
    else
    {
        TOTA_LOG_DBG(0, "not find callback func by module");
    }
}

/* get current module */
APP_TOTA_MODULE_E app_tota_callback_module_get()
{
    return s_module;
}
/*-------------------------------------------------------------------*/

static void s_app_tota_connected();
static void s_app_tota_disconnected();
static void s_app_tota_tx_done();
static void s_app_tota_rx(uint8_t * cmd_buf, uint16_t len);


static void s_app_tota_connected()
{
    TOTA_LOG_DBG(0,"Tota connected.");
    app_tota_store_reset();
    tota_set_connect_status(TOTA_CONNECTED);
    tota_set_connect_path(APP_TOTA_VIA_SPP);
    if (s_module_func && s_module_func->connected_cb != NULL)
    {
        s_module_func->connected_cb();
    }
}

static void s_app_tota_disconnected()
{
    TOTA_LOG_DBG(0,"Tota disconnected.");
    app_tota_store_reset();
    app_tota_ctrl_reset();
    tota_rx_queue_deinit();
#if (OTA_OVER_TOTA_ENABLED)
    Bes_exit_ota_state();
#ifdef IBRT
    if (APP_TOTA_VIA_NOTIFICATION == tota_get_connect_path())
    {
        uint8_t disconnected_parm = DATA_PATH_BLE;
        tws_ctrl_send_cmd(APP_TWS_CMD_OTA_MOBILE_DISC_CMD, &disconnected_parm, 1);
    }
#endif
#endif
    tota_set_connect_status(TOTA_DISCONNECTED);
    tota_set_connect_path(APP_TOTA_PATH_IDLE);

#if (OTA_OVER_TOTA_ENABLED)
    ota_control_set_datapath_type(NO_OTA_CONNECTION);
    Bes_exit_ota_state();
#ifdef IBRT
    if (APP_TOTA_VIA_NOTIFICATION == tota_get_connect_path())
    {
        uint8_t disconnected_parm = DATA_PATH_BLE;
        tws_ctrl_send_cmd(APP_TWS_CMD_OTA_MOBILE_DISC_CMD, &disconnected_parm, 1);
    }
#endif
#endif

    tota_set_connect_status(TOTA_DISCONNECTED);
    tota_set_connect_path(APP_TOTA_PATH_IDLE);

    if (s_module_func && s_module_func->disconnected_cb != NULL)
    {
        s_module_func->disconnected_cb();
    }
}

static void s_app_tota_tx_done()
{
    TOTA_LOG_DBG(0,"Tota tx done.");
    if (s_module_func && s_module_func->tx_done_cb != NULL)
    {
        s_module_func->tx_done_cb();
    }
}

#ifdef BUDSODIN2_TOTA
#define BUDSODIN2_VENDOR_CODE 0xfd
bool app_tota_if_customers_access_valid(uint8_t access_code)
{
    bool valid =  false;
    if(BUDSODIN2_VENDOR_CODE == access_code)
    {
        valid = true;
    }

    return valid;
}

#define BUDSODIN2_DATA_LEN_LIMIT 40
#define BUDSODIN2_RUBBISH_CODE_LEN 3
#define BUDSODIN2_HEADER_LEN 4
uint8_t g_custom_tota_data[BUDSODIN2_DATA_LEN_LIMIT]= {0};
uint8_t * app_tota_custom_refactor_tota_data(uint8_t* ptrData, uint32_t dataLength)
{
    TOTA_LOG_DBG(0,"TOTA custom hijack! data len=%d", dataLength);
    do{
        if(dataLength >= BUDSODIN2_DATA_LEN_LIMIT || dataLength < 4)
        {
            break;
        }
        memset(g_custom_tota_data, 0 , BUDSODIN2_DATA_LEN_LIMIT);
        //refacor tota data
        APP_TOTA_CMD_PAYLOAD_T* pPayload = (APP_TOTA_CMD_PAYLOAD_T *)&g_custom_tota_data;
        pPayload->cmdCode = OP_TOTA_STRING;
        pPayload->paramLen = dataLength - BUDSODIN2_HEADER_LEN - BUDSODIN2_RUBBISH_CODE_LEN;

        memcpy(pPayload->param, &ptrData[BUDSODIN2_HEADER_LEN], pPayload->paramLen);
    }while(0);

    return g_custom_tota_data;
}
#endif

#ifdef BLE_TOTA_ENABLED
static void tota_ble_event_ccc_change_handler(bes_ble_tota_event_param_t *param, uint8_t ntf_en)
{
    if (ntf_en)
    {
        if (!ble_tota_env.isNotificationEnabled)
        {
            ble_tota_env.isNotificationEnabled = ntf_en;
            ble_tota_env.connectionIndex = param->conidx;
            ble_tota_env.connhdl = param->connhdl;
            app_tota_store_reset();
            tota_set_connect_status(TOTA_CONNECTED);
            tota_set_connect_path(APP_TOTA_VIA_NOTIFICATION);
            TOTA_LOG_DBG(3,"[%s], condix = %d connectionIndex = %d",__func__,param->conidx, ble_tota_env.connectionIndex);
#if defined(OTA_OVER_TOTA_ENABLED)
            bes_ota_event_param_t otaParam;
            otaParam.pathType = DATA_PATH_BLE;
            otaParam.event = BES_OTA_CONN;
            app_ota_push_rx_data(BLE_RX_DATA_SELF_OTA_OVER_TOTA, &otaParam);
            bes_ble_gap_update_conn_param_mode(BLE_CONN_PARAM_MODE_OTA, true);
#endif
            if (ble_tota_env.mtu)
            {
                tota_set_trans_MTU(ble_tota_env.mtu);
#if defined(OTA_OVER_TOTA_ENABLED)
                ota_basic_mtu_set((ble_tota_env.mtu - TOTA_PACKET_VERIFY_SIZE)/16*16);
#endif
            }
        }
    }
    else
    {
        if(ble_tota_env.isNotificationEnabled)
        {
            ble_tota_env.isNotificationEnabled = ntf_en;
            tota_set_connect_path(APP_TOTA_PATH_IDLE);
            app_tota_ble_disconnected();
        }
    }
}

static void tota_ble_event_callback(bes_ble_tota_event_param_t *param)
{
    switch(param->event_type) {
    case BES_BLE_TOTA_CCC_CHANGED:
        TOTA_LOG_DBG(3,"[%s], condix = %d  %p",__func__, param->conidx,(void *)param);
        tota_ble_event_ccc_change_handler(param, param->param.ntf_en);
        break;
    case BES_BLE_TOTA_DIS_CONN_EVENT:
        TOTA_LOG_DBG(3,"[%s], condix = %d Index = %d",__func__, param->conidx, ble_tota_env.connectionIndex);
        if (param->conidx == ble_tota_env.connectionIndex)
        {
            ble_tota_env.connectionIndex = INVALID_CONNECTION_INDEX;
            ble_tota_env.isNotificationEnabled = false;
            ble_tota_env.mtu = 0;
            tota_set_connect_path(APP_TOTA_PATH_IDLE);
            app_tota_ble_disconnected();
        }
        break;
    case BES_BLE_TOTA_RECEVICE_DATA:
        app_tota_handle_received_data(param->param.receive_data.data, param->param.receive_data.data_len);
        break;
    case BES_BLE_TOTA_MTU_UPDATE:
        TOTA_LOG_DBG(1,"[%s]TOTA",__func__);
        if (param->conidx == ble_tota_env.connectionIndex)
        {
            ble_tota_env.mtu = param->param.mtu;
            TRACE(1,"updated data packet size is %d", ble_tota_env.mtu);
        }
        else
        {
            ble_tota_env.mtu = param->param.mtu;
        }
        break;
    case BES_BLE_TOTA_SEND_DONE:
        break;
    default:
        TOTA_LOG_DBG(0,"TOTA ble not find event %x", param->event_type);
        break;
    }
}

uint8_t tota_ble_get_conidx(void)
{
    return ble_tota_env.connectionIndex;
}
static void tota_ble_init(void)
{
    ble_tota_env.connectionIndex =  INVALID_CONNECTION_INDEX;
    ble_tota_env.isNotificationEnabled = false;
    ble_tota_env.mtu = 0;

    bes_ble_tota_event_reg(tota_ble_event_callback);
}

#endif


static void s_app_tota_rx(uint8_t * cmd_buf, uint16_t len)
{
    TOTA_LOG_DBG(0,"Tota rx.");
    uint8_t * buf = cmd_buf;

    //sanity check
    if(buf == NULL)
    {
        TOTA_LOG_DBG(0,"[%s] cmd_buf is null", __func__);
        return;
    }

    int ret = 0;
#ifdef BUDSODIN2_TOTA
    //hijack the customers tota data
    if(app_tota_if_customers_access_valid(buf[0]))
    {
        buf = app_tota_custom_refactor_tota_data(buf, len);
    }
#endif
    ret = tota_rx_queue_push(buf, len);
    ASSERT(ret == 0, "tota rx queue FULL!!!!!!");

    uint16_t queueLen= tota_rx_queue_length();
    while (TOTA_PACKET_VERIFY_SIZE < queueLen)
    {
        TOTA_LOG_DBG(1,"queueLen = 0x%x", queueLen);
        ret = app_tota_rx_unpack(buf, queueLen);
        if (ret)
        {
            break;
        }
        queueLen= tota_rx_queue_length();

        TOTA_LOG_DBG(1,"queueLen = 0x%x",queueLen);
    }

}

static const tota_callback_func_t app_tota_cb = {
    s_app_tota_connected,
    s_app_tota_disconnected,
    s_app_tota_tx_done,
    s_app_tota_rx
};

#ifdef TOTA_FACTORY_USED
static bool in_tota_mode = false;
bool is_in_tota_mode(void)
{
    return in_tota_mode;
}

void set_in_tota_mode(bool en)
{
    in_tota_mode = en;
}

void reboot_to_enter_tota_mode(void)
{
    TRACE(0,"Enter tota mode !!!");
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT|HAL_SW_BOOTMODE_TOTA_REBOOT);
    pmu_reboot();
}
#endif

void app_tota_init(void)
{
#ifdef TOTA_FACTORY_USED
    if (!is_in_tota_mode()) {
        TOTA_LOG_DBG(0, "Not in tota mode");
        return;
    }
#endif

    TOTA_LOG_DBG(0, "tota %s", BESLIB_INFO_STR);
    TOTA_LOG_DBG(0, "Init application test over the air.");

#ifdef BT_SPP_SUPPORT
    app_spp_tota_init(&app_tota_cb);
#endif

#ifdef STREAM_DATA_OVER_TOTA    
    /* initialize stream thread */
    app_tota_stream_data_transfer_init();
#endif

    // app_tota_cmd_handler_init();
    /* register callback modules */

#ifdef DATA_DUMP_TOTA    
    app_tota_audio_dump_init();
#endif

    /* set module to access spp callback */
    app_tota_callback_module_set(APP_TOTA_AUDIO_DUMP);

    tota_common_init();

#if defined(BLE_TOTA_ENABLED)
    tota_ble_init();
#if defined(OTA_OVER_TOTA_ENABLED)
    app_ota_rx_thread_init();
#endif
#endif

}

void app_tota_payload_set(APP_TOTA_CMD_PAYLOAD_T* buf)
{
    payload = buf;
}

void app_tota_handle_received_data(uint8_t* buffer, uint16_t maxBytes)
{
    TOTA_LOG_DBG(2,"[%s]data receive data length = %d",__func__,maxBytes);
    TOTA_LOG_DUMP("[0x%x]",buffer,(maxBytes>20 ? 20 : maxBytes));
    s_app_tota_rx(buffer,maxBytes);
}

void app_tota_ble_disconnected(void)
{
    s_app_tota_disconnected();
#if defined(OTA_OVER_TOTA_ENABLED)
    bes_ota_event_param_t otaParam;
    otaParam.pathType = DATA_PATH_BLE;
    otaParam.event = BES_OTA_DISCONN;
    app_ota_push_rx_data(BLE_RX_DATA_SELF_OTA_OVER_TOTA, &otaParam);
#endif
}

static bool app_tota_send_via_datapath(uint8_t * pdata, uint16_t dataLen)
{
    dataLen = app_tota_tx_pack(pdata, dataLen);
    if (0 == dataLen)
    {
        return false;
    }

    switch (tota_get_connect_path())
    {
#ifdef BT_SPP_SUPPORT
        case APP_TOTA_VIA_SPP:
            return app_spp_tota_send_data(pdata, dataLen);
#endif
#ifdef BLE_TOTA_ENABLED
        case APP_TOTA_VIA_NOTIFICATION:
            return bes_ble_tota_send_notification(ble_tota_env.connectionIndex, pdata, dataLen);
#endif
        default:
            return false;
    }
}

bool app_tota_send(uint8_t * pdata, uint16_t dataLen, APP_TOTA_CMD_CODE_E opCode)
{
    if ( opCode == OP_TOTA_NONE )
    {
        TOTA_LOG_DBG(0, "Send pure data");
        /* send pure data */
        return app_tota_send_via_datapath(pdata, dataLen);
    }

    APP_TOTA_CMD_PAYLOAD_T payload;
    /* sanity check: opcode is valid */
    if (opCode >= OP_TOTA_COMMAND_COUNT)
    {
        TOTA_LOG_DBG(0, "Warning: opcode not found");
        return false;
    }
    /* sanity check: data length */
    if (dataLen > sizeof(payload.param))
    {
        TOTA_LOG_DBG(0, "Warning: the length of the data is too lang");
        return false;
    }
    /* sanity check: opcode entry */
    // becase some cmd only for one side
    uint16_t entryIndex = app_tota_cmd_handler_get_entry_index_from_cmd_code(opCode);
    if (INVALID_TOTA_ENTRY_INDEX == entryIndex)
    {
        TOTA_LOG_DBG(0, "Warning: cmd not registered");
        return false;
    }

    payload.cmdCode = opCode;
    payload.paramLen = dataLen;
    memcpy(payload.param, pdata, dataLen);

    /* if is string, direct send */
    if ( opCode == OP_TOTA_STRING )
    {
        return app_tota_send_via_datapath((uint8_t*)&payload, dataLen+4);
    }
#if TOTA_ENCODE
    /* cmd filter */
    if ((TOTA_SHAKE_HANDED  == tota_get_connect_status()) && (opCode > OP_TOTA_CONN_CONFIRM))
    {
        // encode here
        TOTA_LOG_DBG(0, "do encode");
        uint16_t len = tota_encrypt(totaEnv.pcodeBuf, (uint8_t*)&payload, dataLen+4);
        if (app_tota_send_via_datapath(totaEnv.pcodeBuf, len))
        {
            APP_TOTA_CMD_INSTANCE_T* pInstance = TOTA_COMMAND_PTR_FROM_ENTRY_INDEX(entryIndex);
            if (pInstance->isNeedResponse)
            {
                app_tota_cmd_handler_add_waiting_rsp_timeout_supervision(entryIndex);
            }
            return true;
        }
        else
        {
            return false;
        }
    }
#endif
    TOTA_LOG_DBG(0, "send normal cmd");
    if (app_tota_send_via_datapath((uint8_t*)&payload, dataLen+4))
    {
        APP_TOTA_CMD_INSTANCE_T* pInstance = TOTA_COMMAND_PTR_FROM_ENTRY_INDEX(entryIndex);
        if (pInstance->isNeedResponse)
        {
            app_tota_cmd_handler_add_waiting_rsp_timeout_supervision(entryIndex);
        }
    }

    return true;
}

bool app_tota_send_rsp(APP_TOTA_CMD_CODE_E rsp_opCode, APP_TOTA_CMD_RET_STATUS_E rsp_status, uint8_t * pdata, uint16_t dataLen)
{
    TOTA_LOG_DBG(3,"[%s] opCode=0x%x, status=%d",__func__, rsp_opCode, rsp_status);
    // check responsedCmdCode's validity
    if ( rsp_opCode >= OP_TOTA_COMMAND_COUNT || rsp_opCode < OP_TOTA_STRING)
    {
        return false;
    }
    APP_TOTA_CMD_RSP_T* pResponse = (APP_TOTA_CMD_RSP_T *)(payload->param);

    // check parameter length
    if (dataLen > sizeof(pResponse->rspData))
    {
        return false;
    }
    pResponse->cmdCodeToRsp = rsp_opCode;
    pResponse->cmdRetStatus = rsp_status;
    pResponse->rspDataLen   = dataLen;
    memcpy(pResponse->rspData, pdata, dataLen);

    payload->cmdCode = OP_TOTA_RESPONSE_TO_CMD;
    payload->paramLen = 3*sizeof(uint16_t) + dataLen;

#if TOTA_ENCODE
    uint16_t len = tota_encrypt(totaEnv.pcodeBuf, (uint8_t*)payload, payload->paramLen+4);
    return app_tota_send_via_datapath(totaEnv.pcodeBuf, len);
#else
    return app_tota_send_via_datapath((uint8_t*)payload, payload->paramLen+4);
#endif
}

bool app_tota_send_data(APP_TOTA_CMD_CODE_E opCode, uint8_t * data, uint32_t dataLen)
{
    uint16_t trans_MTU = tota_get_trans_MTU();
    TOTA_LOG_DBG(1, "trans_MTU:%d",trans_MTU);

    /* sanity check: opcode is valid */
    if (opCode >= OP_TOTA_COMMAND_COUNT)
    {
        TOTA_LOG_DBG(0, "Warning: opcode not found");
        return false;
    }

    /* sanity check: opcode entry */
    // becase some cmd only for one side
    uint16_t entryIndex = app_tota_cmd_handler_get_entry_index_from_cmd_code(opCode);
    if (INVALID_TOTA_ENTRY_INDEX == entryIndex)
    {
        TOTA_LOG_DBG(0, "Warning: cmd not registered");
        return false;
    }

    uint8_t *pData = data;
    uint8_t *sendBuf = (uint8_t *)payload;
    uint32_t sendBytes = 0;
    uint32_t leftBytes = dataLen;
    payload->cmdCode = opCode;
    payload->paramLen = dataLen;
    sendBytes = 4;
    leftBytes += 4;

    do
    {
        TOTA_LOG_DBG(2, "leftBytes=%d,sendBytes=%d", leftBytes, sendBytes);

        if (leftBytes <= DONGLE_DATA_FRAME)
        {
            memcpy(sendBuf+sendBytes, pData, (leftBytes-sendBytes));
            pData += (leftBytes-sendBytes);
            sendBytes = leftBytes;
        }
        else
        {
            memcpy(sendBuf+sendBytes, pData, DONGLE_DATA_FRAME-sendBytes);
            pData += DONGLE_DATA_FRAME-sendBytes;
            sendBytes = DONGLE_DATA_FRAME;
        }

        leftBytes -= sendBytes;

        TOTA_LOG_DBG(2, "leftBytes=%d,sendBytes=%d",leftBytes,sendBytes);
#if TOTA_ENCODE
        if (TOTA_SHAKE_HANDED == tota_get_connect_status())
        {
            // encode here
            TOTA_LOG_DBG(0, "do encode");
            sendBytes = tota_encrypt(totaEnv.pcodeBuf, sendBuf, sendBytes);
            sendBuf = totaEnv.pcodeBuf;
        }
        else
        {
            return false;
        }
#else
        TOTA_LOG_DBG(0, "send normal cmd");
#endif

        app_tota_send_via_datapath(sendBuf, sendBytes);

        APP_TOTA_CMD_INSTANCE_T* pInstance = TOTA_COMMAND_PTR_FROM_ENTRY_INDEX(entryIndex);
        if (pInstance->isNeedResponse)
        {
            app_tota_cmd_handler_add_waiting_rsp_timeout_supervision(entryIndex);
        }

        sendBytes = 0;
        sendBuf = (uint8_t *)payload;

    }while(leftBytes > 0);

    return true;
}


#if defined(OTA_OVER_TOTA_ENABLED)
void app_ota_over_tota_receive_data(uint8_t* ptrParam, uint32_t paramLen)
{
    TOTA_LOG_DBG(1,"[%s] datapath %d", __func__, tota_get_connect_path());

    bes_ota_event_param_t otaParam = {.conidx = INVALID_CONNECTION_INDEX};

    otaParam.event = BES_OTA_RECEVICE_DATA;
    otaParam.param.receive_data.dataLen = (uint16_t)paramLen;
    otaParam.param.receive_data.data = ptrParam;

    switch (tota_get_connect_path())
    {
#ifdef BT_SPP_SUPPORT
        case APP_TOTA_VIA_SPP:
            otaParam.pathType = DATA_PATH_SPP;
            app_ota_push_rx_data(SPP_RX_DATA_SELF_OTA_OVER_TOTA, &otaParam);
            break;
#endif
#ifdef BLE_TOTA_ENABLED
        case APP_TOTA_VIA_NOTIFICATION:
            otaParam.pathType = DATA_PATH_BLE;
            otaParam.conidx = tota_ble_get_conidx();
            app_ota_push_rx_data(BLE_RX_DATA_SELF_OTA_OVER_TOTA, &otaParam);
            break;
#endif
        default:
            break;
    }
}

#if (BLE_TOTA_ENABLED)
void ota_tota_send_notification(uint8_t* ptrData, uint16_t length)
{
    app_tota_send_data(OP_TOTA_OTA, ptrData, length);
}
#endif

void ota_spp_tota_send_data(uint8_t* ptrData, uint16_t length)
{
    app_tota_send_data(OP_TOTA_OTA, ptrData, length);
}
#endif

/*---------------------------------------------------------------------------------------------------------------------------*/
#ifdef IS_TOTA_LOG_PRINT_ENABLED
static char strBuf[MAX_SPP_PACKET_SIZE-4];

char *tota_get_strbuf(void)
{
    return strBuf;
}

void tota_printf(const char * format, ...)
{
#ifdef BT_SPP_SUPPORT
    app_spp_tota_send_data((uint8_t*)format, strlen(format));
#endif
}

void tota_print(const char * format, ...)
{
    app_tota_send((uint8_t*)format, strlen(format), OP_TOTA_STRING);
}
#else
void tota_printf(const char * format, ...)
{

}

void tota_print(const char * format, ...)
{

}
#endif

static void app_tota_demo_cmd_handler(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen)
{
    switch(funcCode)
    {
        case OP_TOTA_STRING:

            break;
        default:
            break;
    }
}

TOTA_COMMAND_TO_ADD(OP_TOTA_STRING, app_tota_demo_cmd_handler, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_DEMO_CMD, app_tota_demo_cmd_handler, false, 0, NULL );
