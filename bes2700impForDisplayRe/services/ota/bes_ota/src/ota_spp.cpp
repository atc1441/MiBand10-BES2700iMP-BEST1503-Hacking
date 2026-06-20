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
 *
 *    Leonardo        2018/12/20
 ****************************************************************************/
#include "ota_spp.h"
#include "ota_basic.h"
#include "hal_trace.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "app_utils.h"
#include "bluetooth_bt_api.h"
#include "cqueue.h"
#include "ota_dbg.h"

OtaContext ota;

/****************************************************************************
 * OTA SPP SDP Entries
 ****************************************************************************/

static const uint8_t OTA_SPP_UUID_128[16] = BES_OTA_UUID_128;
/*---------------------------------------------------------------------------
 *
 * ServiceClassIDList
 */
static const U8 OtaSppClassId[] = {
#ifdef IS_BES_OTA_USE_CUSTOM_RFCOMM_UUID
    SDP_ATTRIB_HEADER_8BIT(17),        /* Data Element Sequence, 17 bytes */
    SDP_UUID_128BIT(OTA_SPP_UUID_128), /* 128 bit UUID in Big Endian */
#else
    SDP_ATTRIB_HEADER_8BIT(3),        /* Data Element Sequence, 6 bytes */
    SDP_UUID_16BIT(SC_SERIAL_PORT),     /* Hands-Free UUID in Big Endian */
#endif
};

static const U8 OtaSppProtoDescList[] = {
    SDP_ATTRIB_HEADER_8BIT(12), /* Data element sequence, 12 bytes */

    /* Each element of the list is a Protocol descriptor which is a
     * data element sequence. The first element is L2CAP which only
     * has a UUID element.
     */
    SDP_ATTRIB_HEADER_8BIT(3), /* Data element sequence for L2CAP, 3
                                  * bytes
                                  */

    SDP_UUID_16BIT(PROT_L2CAP), /* Uuid16 L2CAP */

    /* Next protocol descriptor in the list is RFCOMM. It contains two
     * elements which are the UUID and the channel. Ultimately this
     * channel will need to filled in with value returned by RFCOMM.
     */

    /* Data element sequence for RFCOMM, 5 bytes */
    SDP_ATTRIB_HEADER_8BIT(5),

    SDP_UUID_16BIT(PROT_RFCOMM), /* Uuid16 RFCOMM */

    /* Uint8 RFCOMM channel number - value can vary */
    SDP_UINT_8BIT(RFCOMM_CHANNEL_BES_OTA)
};

/*
 * BluetoothProfileDescriptorList
 */
static const U8 OtaSppProfileDescList[] = {
#ifdef IS_BES_OTA_USE_CUSTOM_RFCOMM_UUID
     SDP_ATTRIB_HEADER_8BIT(22), /* Data element sequence, 22 bytes */

    /* Data element sequence for ProfileDescriptor, 20 bytes */
    SDP_ATTRIB_HEADER_8BIT(20),

    SDP_UUID_128BIT(OTA_SPP_UUID_128), /* Uuid128 SPP */
#else
    SDP_ATTRIB_HEADER_8BIT(8),        /* Data element sequence, 8 bytes */

    /* Data element sequence for ProfileDescriptor, 6 bytes */
    SDP_ATTRIB_HEADER_8BIT(6),

    SDP_UUID_16BIT(SC_SERIAL_PORT),   /* Uuid16 SPP */
#endif
    SDP_UINT_16BIT(0x0102)          /* As per errata 2239 */
};

/*
 * * OPTIONAL *  ServiceName
 */
static const U8 OtaSppServiceName[] = {
    SDP_TEXT_8BIT(7), /* Null terminated text string */
    'B', 'E', 'S', 'O', 'T', 'A', '\0'};

/* SPP attributes.
 *
 * This is a ROM template for the RAM structure used to register the
 * SPP SDP record.
 */
static bt_sdp_record_attr_t OtaSppSdpAttributes[] = { // list attr id in ascending order

    SDP_ATTRIBUTE(AID_SERVICE_CLASS_ID_LIST, OtaSppClassId),

    SDP_ATTRIBUTE(AID_PROTOCOL_DESC_LIST, OtaSppProtoDescList),

    SDP_ATTRIBUTE(AID_BT_PROFILE_DESC_LIST, OtaSppProfileDescList),

    /* SPP service name*/
    SDP_ATTRIBUTE((AID_SERVICE_NAME + 0x0100), OtaSppServiceName),
};


static int spp_ota_callback(const bt_bdaddr_t *remote, bt_spp_event_t event, bt_spp_callback_param_t *param)
{
    bes_ota_event_param_t otaParam;
    otaParam.pathType = DATA_PATH_SPP;

    TRACE(0,"[%s] %x",__func__, event);
    switch(event) {
    case BT_SPP_EVENT_OPENED: {
        ota.ota_spp_dev = param->spp_chan;
        ota.isConnected = true;
        memcpy(otaParam.param.address, remote, sizeof(otaParam.param.address));
        otaParam.event = BES_OTA_CONN;
        app_ota_push_rx_data(SPP_RX_DATA_SELF_OTA, &otaParam);
        break;
    }
    case BT_SPP_EVENT_CLOSED: {
        ota.isConnected = false;
        otaParam.event = BES_OTA_DISCONN;
        app_ota_push_rx_data(SPP_RX_DATA_SELF_OTA, &otaParam);
        break;
    }
    case BT_SPP_EVENT_RX_DATA: {
        otaParam.event = BES_OTA_RECEVICE_DATA;
        otaParam.param.receive_data.data = (uint8_t *)param->rx_data_ptr;
        otaParam.param.receive_data.dataLen = param->rx_data_len;
        app_ota_push_rx_data(SPP_RX_DATA_SELF_OTA, &otaParam);
        break;
    }
    default: 
        break;
    }
    return 0;
}

void app_ota_spp_send_data(uint8_t *ptrData, uint16_t length)
{
    // TRACE(0,"[%s] %d",__func__, ota.isConnected);
    // DUMP8("%x ",ptrData,length);
    if (!ota.isConnected) {
        return;
    }

    bt_spp_write(ota.ota_spp_dev->rfcomm_handle, ptrData, length);
}

void ota_disconnect(void)
{
    LOG_D("%s %d ",__func__,__LINE__);

    if (DATA_PATH_SPP != app_ota_get_connected_type())
    {
        return;
    }

    if (ota.ota_spp_dev)
    {
        bt_spp_disconnect(ota.ota_spp_dev->rfcomm_handle, BTIF_BEC_LOCAL_TERMINATED);
    }

    return;
}

void app_ota_spp_init(void)
{
    TRACE(0,"[%s]",__func__);

    bt_spp_create_port(RFCOMM_CHANNEL_BES_OTA, OtaSppSdpAttributes, ARRAY_SIZE(OtaSppSdpAttributes));

    bt_spp_set_callback(RFCOMM_CHANNEL_BES_OTA, OTA_SPP_RECV_BUFFER_SIZE, spp_ota_callback, NULL);

    bt_spp_listen(RFCOMM_CHANNEL_BES_OTA, false, NULL);

    ota.ota_spp_dev = bt_spp_create_channel(BT_DEVICE_ID_1, RFCOMM_CHANNEL_BES_OTA);
}
