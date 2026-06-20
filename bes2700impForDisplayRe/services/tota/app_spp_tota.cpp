/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include <stdio.h>
#include "cmsis_os.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "audioflinger.h"
#include "lockcqueue.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hal_chipid.h"
#include "analog.h"
#include "app_audio.h"
#include "app_status_ind.h"
#include "bluetooth_bt_api.h"
#include "app_bt_stream.h"
#include "nvrecord_bt.h"
#include "nvrecord_env.h"
#include "cqueue.h"
#include "app_spp_tota.h"
#include "app_tota_cmd_code.h"
#include "app_tota.h"
#include "app_tota_cmd_handler.h"
#include "app_tota_data_handler.h"
#include "plat_types.h"
#include "spp_api.h"
#include "sdp_api.h"
//#include "app_bt_conn_mgr.h"
#ifdef APP_ANC_TEST
#include "anc_parse_data.h"
#endif

#define SPP_RECV_BUFFER_SIZE (L2CAP_CFG_MTU*4)

static bool isTotaSppConnected = false;
static bt_spp_channel_t  *tota_spp_dev = NULL;

static app_spp_tota_tx_done_t app_spp_tota_tx_done_func = NULL;
static int app_spp_tota_client_callback(const bt_bdaddr_t *remote, bt_spp_event_t event, bt_spp_callback_param_t *param);

uint16_t app_spp_tota_tx_buf_size(void)
{
    return TOTA_SPP_TX_BUF_SIZE;
}

extern "C" APP_TOTA_CMD_RET_STATUS_E app_tota_data_received(uint8_t* ptrData, uint32_t dataLength);
extern "C" APP_TOTA_CMD_RET_STATUS_E app_tota_cmd_received(uint8_t* ptrData, uint32_t dataLength);


/****************************************************************************
 * TOTA SPP SDP Entries
 ****************************************************************************/

/*---------------------------------------------------------------------------
 *
 * ServiceClassIDList
 */
static const U8 TotaSppClassId[] = {
    SDP_ATTRIB_HEADER_8BIT(3),        /* Data Element Sequence, 6 bytes */
    SDP_UUID_16BIT(SC_SERIAL_PORT),     /* Hands-Free UUID in Big Endian */
};

static const U8 TotaSppProtoDescList[] = {
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
    SDP_UINT_8BIT(RFCOMM_CHANNEL_TOTA)
};

/*
 * BluetoothProfileDescriptorList
 */
static const U8 TotaSppProfileDescList[] = {
    SDP_ATTRIB_HEADER_8BIT(8),        /* Data element sequence, 8 bytes */

    /* Data element sequence for ProfileDescriptor, 6 bytes */
    SDP_ATTRIB_HEADER_8BIT(6),

    SDP_UUID_16BIT(SC_SERIAL_PORT),   /* Uuid16 SPP */
    SDP_UINT_16BIT(0x0102)            /* As per errata 2239 */
};

/*
 * * OPTIONAL *  ServiceName
 */
static const U8 TotaSppServiceName1[] = {
    SDP_TEXT_8BIT(5),          /* Null terminated text string */
    'S', 'p', 'p', '1', '\0'
};

static const U8 TotaSppServiceName2[] = {
    SDP_TEXT_8BIT(5),          /* Null terminated text string */
    'S', 'p', 'p', '2', '\0'
};

/* SPP attributes.
 *
 * This is a ROM template for the RAM structure used to register the
 * SPP SDP record.
 */
//static const SdpAttribute TotaSppSdpAttributes1[] = {
static bt_sdp_record_attr_t TotaSppSdpAttributes1[] = { // list attr id in ascending order

    SDP_ATTRIBUTE(AID_SERVICE_CLASS_ID_LIST, TotaSppClassId),

    SDP_ATTRIBUTE(AID_PROTOCOL_DESC_LIST, TotaSppProtoDescList),

    SDP_ATTRIBUTE(AID_BT_PROFILE_DESC_LIST, TotaSppProfileDescList),

    /* SPP service name*/
    SDP_ATTRIBUTE((AID_SERVICE_NAME + 0x0100), TotaSppServiceName1),
};

/*
static bt_sdp_record_attr_t TotaSppSdpAttributes2[] = { // list attr id in ascending order

    SDP_ATTRIBUTE(AID_SERVICE_CLASS_ID_LIST, TotaSppClassId),

    SDP_ATTRIBUTE(AID_PROTOCOL_DESC_LIST, TotaSppProtoDescList),

    SDP_ATTRIBUTE(AID_BT_PROFILE_DESC_LIST, TotaSppProfileDescList),


    SDP_ATTRIBUTE((AID_SERVICE_NAME + 0x0100), TotaSppServiceName2),
};
*/

extern "C" void reset_programmer_state(unsigned char **buf, size_t *len);
extern unsigned char *g_buf;
extern size_t g_len;

static int tota_spp_handle_data_event_func(const bt_bdaddr_t *remote, bt_spp_event_t event, bt_spp_callback_param_t *param)
{
    uint8_t *pData = (uint8_t *)param->rx_data_ptr;
    uint16_t dataLen = param->rx_data_len;

    TOTA_LOG_DBG(2,"[%s]data receive length = %d", __func__, dataLen);
    TOTA_LOG_DUMP("[0x%x]", pData, dataLen);
#if defined(APP_ANC_TEST)
    app_anc_tota_cmd_received(pData, (uint32_t)dataLen);
#else
    // the first two bytes of the data packet is the fixed value 0xFFFF
    app_tota_handle_received_data(pData, dataLen);
#endif

    return 0;
}

#if defined(APP_ANC_TEST)
static void app_synccmd_timehandler(void const *param);
osTimerDef (APP_SYNCCMD, app_synccmd_timehandler);
osTimerId app_check_send_synccmd_timer = NULL;
extern "C" void send_sync_cmd_to_tool();
static void app_synccmd_timehandler(void const *param)
{
    send_sync_cmd_to_tool();
}
#endif

static int spp_tota_callback(const bt_bdaddr_t *remote, bt_spp_event_t event, bt_spp_callback_param_t *param)
{
    if (BT_SPP_EVENT_OPENED == event)
    {
        TOTA_LOG_DBG(0, "spp_tota_callback ::BTIF_SPP_EVENT_REMDEV_CONNECTED %02x", event);
        isTotaSppConnected = true;
        tota_spp_dev = param->spp_chan;
        //app_spp_tota_create_read_thread();
        app_tota_connected(APP_TOTA_CONNECTED);
        app_tota_update_datapath(APP_TOTA_VIA_SPP);
        //conn_stop_connecting_mobile_supervising();
#if defined(APP_ANC_TEST)
        anc_data_buff_init();
        //add a send sync timer
        osTimerStop(app_check_send_synccmd_timer);
        osTimerStart(app_check_send_synccmd_timer, 2000);
#endif
    }
    else if (BT_SPP_EVENT_CLOSED == event)
    {
        TOTA_LOG_DBG(0, "spp_tota_callback ::BTIF_SPP_EVENT_REMDEV_DISCONNECTED %02x", event);
        isTotaSppConnected = false;
        //app_spp_tota_close_read_thread();
        app_tota_disconnected(APP_TOTA_DISCONNECTED);
        app_tota_update_datapath(APP_TOTA_PATH_IDLE);

#if defined(APP_ANC_TEST)
        anc_data_buff_deinit();
        osTimerStop(app_check_send_synccmd_timer);
#endif
        app_spp_tota_tx_done_func = NULL;
    }
    else if (BT_SPP_EVENT_TX_DONE == event)
    {
        if (app_spp_tota_tx_done_func)
        {
            app_spp_tota_tx_done_func();
        }
    }
    else if (BT_SPP_EVENT_RX_DATA == event)
    {
        tota_spp_handle_data_event_func(remote, event, param);
    }

    return 0;
}

static void app_spp_tota_send_data(uint8_t* ptrData, uint16_t length)
{
    if (!isTotaSppConnected)
    {
        return;
    }

    if (tota_spp_dev)
    {
        bt_spp_write(tota_spp_dev->rfcomm_handle, ptrData, length);
    }
    else
    {
        TRACE(0, "app_spp_tota_send_data: tota_spp_dev NULL");
    }
}

void app_tota_send_cmd_via_spp(uint8_t* ptrData, uint32_t length)
{
    app_spp_tota_send_data(ptrData, (uint16_t)length);
}

void app_tota_send_data_via_spp(uint8_t* ptrData, uint32_t length)
{
    TOTA_LOG_DBG(2,"[%s]tota send data length = %d",__func__,length);
    app_spp_tota_send_data(ptrData, (uint16_t)length);
}

void app_spp_tota_register_tx_done(app_spp_tota_tx_done_t callback)
{
    app_spp_tota_tx_done_func = callback;
}

void app_spp_tota_init(void)
{
    bt_spp_create_port(RFCOMM_CHANNEL_TOTA, TotaSppSdpAttributes1, ARRAY_SIZE(TotaSppSdpAttributes1));

    bt_spp_set_callback(RFCOMM_CHANNEL_TOTA, SPP_RECV_BUFFER_SIZE, spp_tota_callback, app_spp_tota_client_callback);

    bt_spp_listen(RFCOMM_CHANNEL_TOTA, false, NULL);

    tota_spp_dev = bt_spp_create_channel(BT_DEVICE_ID_1, RFCOMM_CHANNEL_TOTA);

#if defined(APP_ANC_TEST)
    reset_programmer_state(&g_buf, &g_len);
#endif

#if defined(APP_ANC_TEST)
    if (app_check_send_synccmd_timer == NULL)
        app_check_send_synccmd_timer = osTimerCreate (osTimer(APP_SYNCCMD), osTimerPeriodic, NULL);
#endif
}

//tota client
static bt_spp_channel_t *_app_spp_tota_client_dev;
static bool isTotaClientSppConnected = false;

static void app_spp_tota_client_send_data(const uint8_t* ptrData, uint16_t length)
{
    if (!isTotaClientSppConnected)
    {
        return;
    }

    if (_app_spp_tota_client_dev)
    {
        bt_spp_write(_app_spp_tota_client_dev->rfcomm_handle, ptrData, length);
    }
    else
    {
        TRACE(0, "_app_spp_tota_client_dev: tota_spp_dev NULL");
    }
}

static int app_spp_tota_client_callback(const bt_bdaddr_t *remote, bt_spp_event_t event, bt_spp_callback_param_t *param)
{
    static const uint8_t data_buf[8] = {0x23, 0x67, 0x87, 0x45, 0x11, 0x22, 0x33, 0x44};
    TRACE(0,"spp client event.");
    switch (event)
    {
        case BT_SPP_EVENT_OPENED:
        {
            isTotaClientSppConnected = true;
            TRACE(0, "app_spp_tota_client_callback ::SPP_EVENT_REMDEV_CONNECTED %02x", event);
            app_spp_tota_client_send_data(data_buf, 8);
            break;
        }

        case BT_SPP_EVENT_CLOSED:
        {
            isTotaClientSppConnected = false;
            TRACE(0, "app_spp_tota_client_callback ::SPP_EVENT_REMDEV_DISCONNECTED %02x", event);
            break;
        }

        case BT_SPP_EVENT_TX_DONE:
        {
            break;
        }

        case BT_SPP_EVENT_RX_DATA:
        {
            tota_spp_handle_data_event_func(remote, event, param);
            break;
        }

        default:
        {
            break;
        }
    }
    return 0;
}

#define APP_TOTA_PORT_SPP 0x1101

void app_spp_tota_client_open(bt_bdaddr_t *remote)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    uint8_t remote_uuid[] = {BT_SDP_SPLIT_16BITS_BE(APP_TOTA_PORT_SPP)};

    curr_device = bt_adapter_get_connected_device_byaddr(remote);
    if (curr_device == NULL)
    {
        TRACE(0, "app_spp_tota_client_open: no connection");
        return;
    }

    _app_spp_tota_client_dev = bt_spp_create_channel(curr_device->device_id, RFCOMM_CHANNEL_TOTA);

    bt_spp_connect(remote, RFCOMM_CHANNEL_TOTA, remote_uuid, sizeof(remote_uuid));
}

