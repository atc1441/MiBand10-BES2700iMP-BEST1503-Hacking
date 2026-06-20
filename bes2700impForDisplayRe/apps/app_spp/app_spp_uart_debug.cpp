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
#include "hal_trace.h"
#include "bluetooth_bt_api.h"
#include "app_spp_uart_debug.h"
#if (CHIP_CAPSENSOR_VER < 1)
#include "app_sensor_hub.h"
#endif
#include "btapp.h"
#include "app_tws_ibrt.h"
#include "app_tws_ibrt_conn.h"
#include "app_ibrt_middleware.h"

#define APP_SPP_SERVICE_ENABLE

#ifdef APP_SPP_SERVICE_ENABLE

#define MAX_SPP_PACKET_SIZE          664
#define MAX_SPP_PACKET_NUM           2

typedef struct{
    bool isBusy;
    bool isConnected;
    bt_spp_channel_t *pSppDevice;
}bth_spp_ctl_t;

static bth_spp_ctl_t bth_spp_ctl = {
    .isBusy         = false,
    .isConnected    = false,
    .pSppDevice     = NULL,
};

app_spp_server_callback_t *app_spp_server_cb = NULL;

uint8_t capsensor_app_uuid[16] =
{ 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99 };
/*---------------------------------------------------------------------------
 *
 * ServiceClassIDList
 */
static const U8 AppSppClassId[] = {
    SDP_ATTRIB_HEADER_8BIT(17),
    SDP_UUID_128BIT(capsensor_app_uuid),
};

static const U8 AppSppProtoDescList[] = {
    SDP_ATTRIB_HEADER_8BIT(12),  /* Data element sequence, 12 bytes */

    /* Each element of the list is a Protocol descriptor which is a
     * data element sequence. The first element is L2CAP which only
     * has a UUID element.
     */
    SDP_ATTRIB_HEADER_8BIT(3),   /* Data element sequence for L2CAP, 3
                                  * bytes
                                  */

    SDP_UUID_16BIT(PROT_L2CAP),  /* Uuid16 L2CAP */

    /* Next protocol descriptor in the list is RFCOMM. It contains two
     * elements which are the UUID and the channel. Ultimately this
     * channel will need to filled in with value returned by RFCOMM.
     */

    /* Data element sequence for RFCOMM, 5 bytes */
    SDP_ATTRIB_HEADER_8BIT(5),

    SDP_UUID_16BIT(PROT_RFCOMM), /* Uuid16 RFCOMM */

    /* Uint8 RFCOMM channel number - value can vary */
    SDP_UINT_8BIT(RFCOMM_CHANNEL_CUSTOM_2)
};

/*
 * BluetoothProfileDescriptorList
 */
static const U8 AppSppProfileDescList[] = {
    SDP_ATTRIB_HEADER_8BIT(8),        /* Data element sequence, 8 bytes */

    /* Data element sequence for ProfileDescriptor, 6 bytes */
    SDP_ATTRIB_HEADER_8BIT(6),

    SDP_UUID_16BIT(SC_SERIAL_PORT),   /* Uuid16 SPP */
    SDP_UINT_16BIT(0x0102)            /* As per errata 2239 */
};

/*
 * * OPTIONAL *  ServiceName
 */
static const U8 AppSppServiceName1[] = {
    SDP_TEXT_8BIT(6),          /* Null terminated text string */
    'W', 'A', 'T', 'C', 'H', '\0'
};

/* SPP attributes.
 *
 * This is a ROM template for the RAM structure used to register the
 * SPP SDP record.
 */
static bt_sdp_record_attr_t AppSppSdpAttributes[] = { // list attr id in ascending order

    SDP_ATTRIBUTE(AID_SERVICE_CLASS_ID_LIST, AppSppClassId),

    SDP_ATTRIBUTE(AID_PROTOCOL_DESC_LIST, AppSppProtoDescList),

    SDP_ATTRIBUTE(AID_BT_PROFILE_DESC_LIST, AppSppProfileDescList),

    /* SPP service name*/
    SDP_ATTRIBUTE((AID_SERVICE_NAME + 0x0100), AppSppServiceName1),
};

#if (CHIP_CAPSENSOR_VER < 1)
void app_send_spp_capsensor_test_data(uint8_t cmd)
{
    //TRACE(2, "%s  send capsensor test %d", __func__, cmd);
    app_core_bridge_send_cmd(MCU_SENSOR_HUB_TASK_CMD_SPP_CAPSENSOR_TEST, &cmd, 1);
}

static void app_sensor_hub_spp_capsensor_test_cmd_received_handler(uint8_t* ptr, uint16_t len)
{
    //TRACE(2, "%s receive sensor hub data len %d", __func__, len);
    app_spp_server_send_data(ptr, len);
}

static void app_sensor_hub_spp_capsensor_test_cmd_handler(uint8_t* ptr, uint16_t len)
{
    //TRACE(2, "%s len %d", __func__, len);
    app_core_bridge_send_data_without_waiting_rsp(MCU_SENSOR_HUB_TASK_CMD_SPP_CAPSENSOR_TEST, ptr, len);
}

static void app_sensor_hub_spp_capsensor_test_tx_done_handler(uint16_t cmdCode, uint8_t* ptr, uint16_t len)
{
    //TRACE(2, "%s cmdCode 0x%x", __func__, cmdCode);
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_SPP_CAPSENSOR_TEST,
                                "spp receive no rsp req from sensor",
                                app_sensor_hub_spp_capsensor_test_cmd_handler,
                                app_sensor_hub_spp_capsensor_test_cmd_received_handler,
                                0,
                                NULL,
                                NULL,
                                app_sensor_hub_spp_capsensor_test_tx_done_handler);
#endif

static int app_spp_server_receive_data(const bt_bdaddr_t *remote, bt_spp_callback_param_t *param)
{
    uint8_t *pData = (uint8_t *)param->rx_data_ptr;
#if (CHIP_CAPSENSOR_VER > 1)
    uint16_t dataLen = param->rx_data_len;
#endif

    // TRACE(2,"%s rx:%d", __func__, dataLen);
    // DUMP8("%x", pData, dataLen);

    if(app_spp_server_cb && app_spp_server_cb->spp_server_recv_data_cb)
    {
#if (CHIP_CAPSENSOR_VER < 1)
        app_send_spp_capsensor_test_data(pData[15]);
#else
        app_spp_server_cb->spp_server_recv_data_cb(pData, dataLen);
#endif
    }

    return 0;
}

static int app_spp_server_callback(const bt_bdaddr_t *remote, bt_spp_event_t event, bt_spp_callback_param_t *param)
{
    struct BT_DEVICE_T *curr_device = NULL;
    ibrt_mobile_info_t *p_mobile_info = NULL;
    bt_bdaddr_t *mobile_addr = NULL;

    switch (event)
    {
    case BT_SPP_EVENT_OPENED:
        TRACE(1, "%s ::BTIF_SPP_EVENT_REMDEV_CONNECTED", __func__);
        for (uint8_t i = 0; i < BT_DEVICE_NUM; ++i)
        {
            curr_device = app_bt_get_device(i);
            mobile_addr = &curr_device->remote;
            p_mobile_info = (ibrt_mobile_info_t *)app_ibrt_conn_get_mobile_sm_by_addr(mobile_addr);

            if ((NULL != p_mobile_info) && (p_mobile_info->mobile_mode == IBRT_SNIFF_MODE)) {
                app_tws_ibrt_exit_sniff_with_mobile(mobile_addr);
                break;
            }
            app_ibrt_if_prevent_sniff_set((uint8_t*)mobile_addr, OTA_ONGOING);
        }

        bth_spp_ctl.isConnected = true;
        bth_spp_ctl.pSppDevice = param->spp_chan;
        if(app_spp_server_cb && app_spp_server_cb->spp_server_connection_state_cb)
        {
            app_spp_server_cb->spp_server_connection_state_cb(APP_SPP_CONNECTION_STATE_CONNECTED);
        }
        break;
    case BT_SPP_EVENT_CLOSED:
        TRACE(1, "%s ::BTIF_SPP_EVENT_REMDEV_DISCONNECTED", __func__);
        for (uint8_t i = 0; i < BT_DEVICE_NUM; ++i)
        {
            curr_device = app_bt_get_device(i);
            mobile_addr = &curr_device->remote;
            p_mobile_info = (ibrt_mobile_info_t *)app_ibrt_conn_get_mobile_sm_by_addr(mobile_addr);

            if ((NULL != p_mobile_info) && (p_mobile_info->mobile_mode == IBRT_SNIFF_MODE)) {
                app_tws_ibrt_exit_sniff_with_mobile(mobile_addr);
                break;
            }
            app_ibrt_if_prevent_sniff_clear((uint8_t*)mobile_addr, OTA_ONGOING);
        }

        bth_spp_ctl.isConnected = false;
        if(app_spp_server_cb && app_spp_server_cb->spp_server_connection_state_cb)
        {
            app_spp_server_cb->spp_server_connection_state_cb(APP_SPP_CONNECTION_STATE_DISCONNECTED);
        }
        break;
    case BT_SPP_EVENT_TX_DONE:
        // TRACE(1, "%s ::BTIF_SPP_EVENT_DATA_SENT", __func__);
        if(app_spp_server_cb && app_spp_server_cb->spp_server_tx_done)
        {
            app_spp_server_cb->spp_server_tx_done();
        }
        break;
    case BT_SPP_EVENT_RX_DATA:
        app_spp_server_receive_data(remote, param);
        break;
    default:
        break;
    }
    return 0;
}

void app_spp_server_init(app_spp_server_callback_t *cb)
{
    TRACE(1, "%s", __func__);
    app_spp_server_cb = cb;
    bt_spp_create_port(RFCOMM_CHANNEL_CUSTOM_2, AppSppSdpAttributes, ARRAY_SIZE(AppSppSdpAttributes));
    bt_spp_set_callback(RFCOMM_CHANNEL_CUSTOM_2, MAX_SPP_PACKET_SIZE*MAX_SPP_PACKET_NUM, app_spp_server_callback, NULL);
    bt_spp_listen(RFCOMM_CHANNEL_CUSTOM_2, false, NULL);
    bth_spp_ctl.pSppDevice = bt_spp_create_channel(BT_DEVICE_ID_1, RFCOMM_CHANNEL_CUSTOM_2);
}


bool app_spp_server_send_data(uint8_t* ptrData, uint16_t length)
{
    bt_status_t ret = BT_STS_SUCCESS;

    if (!bth_spp_ctl.isConnected)
    {
        return false;
    }

    // TRACE(1, "app spp tx:%d", length);
    ret = bt_spp_write(bth_spp_ctl.pSppDevice->rfcomm_handle, ptrData, length);

    if (BT_STS_SUCCESS != ret)
    {
        // TRACE(1, "BT_STS fail");
        return false;
    }
    else
    {
        // TRACE(1, "BT_STS success");
        return true;
    }
}

bool app_spp_server_is_connected(void)
{
    return bth_spp_ctl.isConnected;
}

#endif
