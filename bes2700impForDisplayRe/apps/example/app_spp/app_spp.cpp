/***************************************************************************
 *
 * Copyright 2023-2033 BES.
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
#include <string.h>
#include "cmsis_os.h"
#include "plat_types.h"
#include "bluetooth.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "spp_api.h"
#include "app_bt.h"
#include "btapp.h"
#include "besbt.h"
#include "sdp_i.h"
#include "app_spp.h"


#define RFCOMM_CHANNEL_SERIAL_PORT                  27
#define RFCOMM_CHANNEL_CUSTOM_PORT_3                28
#define RFCOMM_CHANNEL_CUSTOM_PORT_2                29
#define RFCOMM_CHANNEL_CUSTOM_PORT_1                30

#define SERIA_PORT_SPP_CLIENT                       0
#define SERIAL_PORT_RX_BUFF_LEN                     (800*3)

static APP_SPP_CLIENT_CALLBACK_IND     g_app_spp_serial_port_client_callback  = NULL;
static APP_SPP_CLIENT_CALLBACK_IND     g_app_spp_serial_port_server_callback  = NULL;

static bt_spp_channel_t *_app_spp_serial_port_client_dev[BT_DEVICE_NUM];
static bool serial_port_client_is_connected[BT_DEVICE_NUM] = {false};

bt_spp_channel_t *g_app_spp_serial_port_device[BT_DEVICE_NUM];
bt_spp_channel_t *g_app_spp_serial_port_server_device[BT_DEVICE_NUM];

static const U8 SerialPortSppClassId[] = {
    SDP_ATTRIB_HEADER_8BIT(3),        /* Data Element Sequence, 6 bytes */
    SDP_UUID_16BIT(SC_SERIAL_PORT),     /* Hands-Free UUID in Big Endian */
};

/**********************************************************************************************
 *@brief           example make new sdp record pragrama of specifical spp port
 *
 *@note1:           RFCOMM channel number in SerialPortSppProtoDescList value can vary
 *@note2:           SPP uuid16 in SerialPortSppProfileDescList can vary
 *@note3:           length and name string in SerialPortSppServiceName1 can vary
 *********************************************************************************************/

static const U8 SerialPortSppProtoDescList[] = {
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
    SDP_UINT_8BIT(RFCOMM_CHANNEL_SERIAL_PORT)
};

/*
 * BluetoothProfileDescriptorList
 */
static const U8 SerialPortSppProfileDescList[] = {
    SDP_ATTRIB_HEADER_8BIT(8),        /* Data element sequence, 8 bytes */

    /* Data element sequence for ProfileDescriptor, 6 bytes */
    SDP_ATTRIB_HEADER_8BIT(6),

    SDP_UUID_16BIT(SC_SERIAL_PORT),   /* Uuid16 SPP */
    SDP_UINT_16BIT(0x0102)            /* As per errata 2239 */
};

/*
 * * OPTIONAL *  ServiceName
 */
static const U8 SerialPortSppServiceName1[] = {
    SDP_TEXT_8BIT(7),          /* Null terminated text string */
    's', 'e', 'r', 'i', 'a', 'l', '\0'
};

static bt_sdp_record_attr_t SerialPortSppSdpAttributes1[] =
{
    SDP_ATTRIBUTE(AID_SERVICE_CLASS_ID_LIST, SerialPortSppClassId),

    SDP_ATTRIBUTE(AID_PROTOCOL_DESC_LIST, SerialPortSppProtoDescList),

    SDP_ATTRIBUTE(AID_BT_PROFILE_DESC_LIST, SerialPortSppProfileDescList),

    SDP_ATTRIBUTE((AID_SERVICE_NAME + 0x0100), SerialPortSppServiceName1),
};


struct app_spp_port_record_t {
    struct list_node node;
    uint16_t uuid;
    uint16_t remote_rfcomm_port;
    uint16_t local_server_channel;
    bt_spp_callback_t app_spp_callback;
    bt_spp_callback_t client_spp_callback;
};

struct app_spp_port_t{
    struct list_node spp_port_list;
    bool spp_is_initialized;
};

static struct app_spp_port_t g_app_spp_global;

static struct app_spp_port_t *app_spp_get_global(void)
{
    return &g_app_spp_global;
}

void app_spp_init(void)
{
    app_spp_port_t *app_spp_global = app_spp_get_global();
    if (app_spp_global->spp_is_initialized == false)
    {
        app_spp_global->spp_is_initialized = true;
        INIT_LIST_HEAD(&app_spp_global->spp_port_list);
    }
}

/**********************************************************************************************
 * @brief              Transfer variable length data to specified devices
 *
 * @param[in]          remote]       the specified devices
 * @param[in]          ptrData       addr of ptrData
 * @param[in]          length        Transfer variable length
 * @param[out]         bool          send status:true-success false-fail
 *********************************************************************************************/

bool app_spp_serial_port_send_data(bt_bdaddr_t *remote, const uint8_t* ptrData, uint16_t length)
{
    bt_status_t ret = BT_STS_SUCCESS;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    uint32_t rfcomm_handle = 0;
    curr_device = bt_adapter_get_connected_device_byaddr(remote);
    if (curr_device == NULL)
    {
        TRACE(0, "app_spp_uart_client_open: no connection");
        return false;
    }

    if (serial_port_client_is_connected[curr_device->device_id] == false)
    {
        TRACE(0, "app_spp_uart_client_open: no open");
        return false;
    }

    rfcomm_handle = _app_spp_serial_port_client_dev[curr_device->device_id]->rfcomm_handle;

    if (rfcomm_handle)
    {
        ret = bt_spp_write(rfcomm_handle, ptrData, length);
    }

    if ((BT_STS_SUCCESS != ret) && (BT_STS_PENDING != ret))
    {
        return false;
    }
    else
    {
        return true;
    }
}

/**********************************************************************************************
 * @brief              Close the corresponding SPP channel on the device
 *
 * @param[in]          remote]       the specified devices
 * @param[out]         NA
 *********************************************************************************************/

void app_spp_close_serial_port(const bt_bdaddr_t *remote)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    uint8_t device_id = BT_DEVICE_INVALID_ID;
    curr_device = bt_adapter_get_connected_device_byaddr(remote);

    if (curr_device == NULL)
    {
        TRACE(0, "app_spp_close_serial_port: no connection");
        return;
    }

    if (curr_device->device_id == BT_DEVICE_INVALID_ID)
    {
        TRACE(0,"warning close serial port fail,remote device already disconnected!");
        return;
    }

    device_id = curr_device->device_id;

    if (serial_port_client_is_connected[device_id] == false)
    {
        TRACE(0, "app_spp_close_serial_port: no open");
        return;
    }

    TRACE(1,"app_spp_close_serial_portt device_id[%d]", device_id);
    bt_spp_disconnect(_app_spp_serial_port_client_dev[device_id]->rfcomm_handle, BTIF_BEC_LOCAL_TERMINATED);
}

/**********************************************************************************************
 * @brief              Close the corresponding SPP channel on the device
 *
 * @param[in]          remote]       the specified devices
 * @param[out]         bool          connected status:true-connected false-no connected
 *********************************************************************************************/

bool app_spp_serial_port_is_connected(const bt_bdaddr_t *remote)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    curr_device = bt_adapter_get_connected_device_byaddr(remote);
    if (curr_device == NULL)
    {
        return false;
    }
    return serial_port_client_is_connected[curr_device->device_id];
}

/**********************************************************************************************
 * @brief              Callback to another location for process
 *
 * @param[in]          handle       handle function
 * @param[out]         NA
 *********************************************************************************************/

void app_spp_register_client_callback_handle(APP_SPP_CLIENT_CALLBACK_IND handle)
{
    g_app_spp_serial_port_client_callback = handle;
}

/**********************************************************************************************
 * @brief              Callback to another location for process
 *
 * @param[in]          handle       handle function
 * @param[out]         NA
 *********************************************************************************************/

void app_spp_register_server_callback_handle(APP_SPP_SERVER_CALLBACK_IND handle)
{
    g_app_spp_serial_port_server_callback = handle;
}

/**********************************************************************************************
 * @brief              serial port spp client Callback handle
 *
 * @param[in]          remote       Device address for report SPP events
 * @param[in]          event        report SPP events: OPENED/ CLOSED/ TX_DONE/ RX_DATA/ ACCEPT
 * @param[in]          param        bt_spp_callback_param_t
 *                                      typedef struct {
 *                                         uint8_t error_code;
 *                                         uint8_t device_id;
 *                                         uint16_t rx_data_len;
 *                                         const uint8_t *rx_data_ptr;
 *                                         bt_spp_channel_t *spp_chan;
 *                                     } bt_spp_callback_param_t;
 * @param[out]         NA
 *********************************************************************************************/

static int app_spp_serial_port_client_callback(const bt_bdaddr_t *remote, bt_spp_event_t event, bt_spp_callback_param_t *param)
{
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    curr_device = bt_adapter_get_connected_device_byaddr(remote);
    if (curr_device == NULL)
    {
        return 0;
    }

    switch (event)
    {
        case BT_SPP_EVENT_OPENED:
            serial_port_client_is_connected[curr_device->device_id] = true;
            TRACE(0, "app_spp_serial_port_client_callback ::SPP_EVENT_REMDEV_CONNECTED %02x", event);
            break;

        case BT_SPP_EVENT_CLOSED:
            serial_port_client_is_connected[curr_device->device_id] = false;
            TRACE(0, "app_spp_serial_port_client_callback ::SPP_EVENT_REMDEV_DISCONNECTED %02x", event);
            break;

        case BT_SPP_EVENT_TX_DONE:
            TRACE(0, "app_spp_serial_port_client_callback ::BT_SPP_EVENT_TX_DONE");
            break;

        case BT_SPP_EVENT_RX_DATA:
            TRACE(0, "app_spp_serial_port_client_callback ::BT_SPP_EVENT_RX_DATA");
            DUMP8("%02x ", param->rx_data_ptr, param->rx_data_len);
            break;

        default:
            break;
    }
    if (g_app_spp_serial_port_client_callback)
    {
        g_app_spp_serial_port_client_callback(SERIA_PORT_SPP_CLIENT, remote, event, param);
    }
    return 0;
}

/**********************************************************************************************
 * @brief              serial port spp server Callback handle
 *
 * @param[in]          remote       Device address for report SPP events
 * @param[in]          event        report SPP events: OPENED/ CLOSED/ TX_DONE/ RX_DATA/ ACCEPT
 * @param[in]          param        bt_spp_callback_param_t
 *                                      typedef struct {
 *                                         uint8_t error_code;
 *                                         uint8_t device_id;
 *                                         uint16_t rx_data_len;
 *                                         const uint8_t *rx_data_ptr;
 *                                         bt_spp_channel_t *spp_chan;
 *                                     } bt_spp_callback_param_t;
 * @param[out]         NA
 *********************************************************************************************/

static int app_spp_serial_port_server_callback(const bt_bdaddr_t *remote, bt_spp_event_t event, bt_spp_callback_param_t *param)
{
    if (param == NULL)
    {
        return 0;
    }

    switch (event)
    {
        case BT_SPP_EVENT_OPENED:
            TRACE(0, "spp_serial_port_server_callback ::BTIF_SPP_EVENT_REMDEV_CONNECTED");
            break;
        case BT_SPP_EVENT_CLOSED:
            TRACE(0, "spp_serial_port_server_callback ::BTIF_SPP_EVENT_REMDEV_DISCONNECTED");
            break;
        case BT_SPP_EVENT_TX_DONE:
            TRACE(0, "spp_serial_port_server_callback ::BT_SPP_EVENT_TX_DONE");
            break;
        case BT_SPP_EVENT_RX_DATA:
            TRACE(0, "spp_serial_port_server_callback ::BT_SPP_EVENT_RX_DATA");
            DUMP8("%02x ", param->rx_data_ptr, param->rx_data_len);
            break;
        default:
            break;
    }

    if (g_app_spp_serial_port_server_callback)
    {
        g_app_spp_serial_port_server_callback(SERIA_PORT_SPP_CLIENT, remote, event, param);
    }

    return 0;
}

/**********************************************************************************************
 * @brief           only register an SPP server service,Not creating an SPP server connection
 *
 * @param[in|       NA
 * @param[out]      NA
 *********************************************************************************************/

void app_spp_serial_port_server_init(void)
{
    struct list_node *node = NULL;
    struct list_node *head = NULL;
    app_spp_port_record_t *p_app_spp_port_record = NULL;
    struct app_spp_port_record_t *spp_port_record = NULL;

    app_spp_init();

    head = &g_app_spp_global.spp_port_list;
    colist_iterate(node, head)
    {
        if (node == NULL)
            break;
        p_app_spp_port_record = colist_structure(node, app_spp_port_record_t, node);
        if (p_app_spp_port_record != NULL &&
            (p_app_spp_port_record->local_server_channel == RFCOMM_CHANNEL_SERIAL_PORT))
            break;
        p_app_spp_port_record = NULL;
    }

    if (p_app_spp_port_record)
    {
        spp_port_record = p_app_spp_port_record;
        spp_port_record->local_server_channel = RFCOMM_CHANNEL_SERIAL_PORT;

        if ((spp_port_record->client_spp_callback == NULL) && \
            (spp_port_record->app_spp_callback != app_spp_serial_port_server_callback))
        {
            bt_spp_set_callback(RFCOMM_CHANNEL_SERIAL_PORT, SERIAL_PORT_RX_BUFF_LEN,
                                app_spp_serial_port_server_callback, spp_port_record->client_spp_callback);
            spp_port_record->app_spp_callback = app_spp_serial_port_server_callback;
        }
    }
    else
    {
        spp_port_record = (struct app_spp_port_record_t *)bes_bt_buf_malloc(sizeof(struct app_spp_port_record_t));
        if (spp_port_record)
        {
            memset(spp_port_record, 0, sizeof(struct app_spp_port_record_t));
            spp_port_record->local_server_channel = RFCOMM_CHANNEL_SERIAL_PORT;
            spp_port_record->app_spp_callback = app_spp_serial_port_server_callback;
            spp_port_record->client_spp_callback = NULL;
            colist_addto_tail(&spp_port_record->node, &g_app_spp_global.spp_port_list);
        }

        bt_spp_create_port(RFCOMM_CHANNEL_SERIAL_PORT, SerialPortSppSdpAttributes1, ARRAY_SIZE(SerialPortSppSdpAttributes1));
        bt_spp_set_callback(RFCOMM_CHANNEL_SERIAL_PORT, SERIAL_PORT_RX_BUFF_LEN, app_spp_serial_port_client_callback, NULL);
        bt_spp_listen(RFCOMM_CHANNEL_SERIAL_PORT, true, NULL);
    }

    for (uint8_t device_id = 0; device_id < BT_DEVICE_NUM; device_id++)
    {
        g_app_spp_serial_port_server_device[device_id] = bt_spp_create_channel(device_id, RFCOMM_CHANNEL_SERIAL_PORT);
    }
}

/**********************************************************************************************
 * @brief           spp client init tx buff
 *
 * @param[in|       ptr                      init buffer ptr
 * @param[out]      reserved
 *********************************************************************************************/

static int app_spp_client_init_tx_buff(uint8_t *ptr)
{
    return 0;
}

/**********************************************************************************************
 * @brief           creat an SPP serial port clien connection by the UUID of the application
 *
 * @param[in|       remote_addr                   mobile device address
 * @param[in|       uuid                          Connect the UUID of the application
 * @param[in]       snoop_spp                     true: slave map restore spp connection
 *                                                false: master creat spp connection
 *********************************************************************************************/

void app_spp_serial_port_client_open_by_uuid(const bt_bdaddr_t *remote_addr, uint16_t uuid, bool snoop_spp)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    uint8_t remote_uuid[] = {BT_SDP_SPLIT_16BITS_BE(uuid)};
    struct app_spp_port_record_t *spp_port_record = NULL;
    struct list_node *node = NULL;
    struct list_node *head = NULL;
    app_spp_port_record_t *p_app_spp_port_record = NULL;
    uint8_t device_id = BT_DEVICE_INVALID_ID;

    app_spp_init();

    curr_device = bt_adapter_get_connected_device_byaddr(remote_addr);
    if (curr_device == NULL)
    {
        TRACE(0, "app_spp_serial_port_client_open: fail,no acl connection!");
        return;
    }
    if (curr_device->device_id == BT_DEVICE_INVALID_ID)
    {
        TRACE(0,"warning serial port fail,remote device is disconnected!");
        return;
    }
    device_id = curr_device->device_id;
    head = &g_app_spp_global.spp_port_list;
    colist_iterate(node, head)
    {
        if (node == NULL)
            break;
        p_app_spp_port_record = colist_structure(node, app_spp_port_record_t, node);
        if (p_app_spp_port_record != NULL &&
            (p_app_spp_port_record->local_server_channel == RFCOMM_CHANNEL_SERIAL_PORT))
            break;
        p_app_spp_port_record = NULL;
    }
    if (p_app_spp_port_record)
    {
        spp_port_record = p_app_spp_port_record;
        spp_port_record->uuid = uuid;
        spp_port_record->local_server_channel = RFCOMM_CHANNEL_SERIAL_PORT;

        if ((spp_port_record->client_spp_callback == NULL) && \
            (spp_port_record->app_spp_callback != app_spp_serial_port_client_callback))
        {
            bt_spp_set_callback(RFCOMM_CHANNEL_SERIAL_PORT, SERIAL_PORT_RX_BUFF_LEN,
                                spp_port_record->app_spp_callback, app_spp_serial_port_client_callback);
            spp_port_record->client_spp_callback = app_spp_serial_port_client_callback;
        }
    }
    else
    {
        spp_port_record = (struct app_spp_port_record_t *)bes_bt_buf_malloc(sizeof(struct app_spp_port_record_t));
        if (!spp_port_record)
        {
            TRACE(0,"warning serial port malloc fail!");
            return;
        }
        memset(spp_port_record, 0, sizeof(struct app_spp_port_record_t));
        INIT_LIST_HEAD(&spp_port_record->node);
        spp_port_record->uuid = uuid;
        spp_port_record->local_server_channel = RFCOMM_CHANNEL_SERIAL_PORT;
        spp_port_record->client_spp_callback = app_spp_serial_port_client_callback;
        spp_port_record->app_spp_callback = NULL;
        colist_addto_tail(&spp_port_record->node, &g_app_spp_global.spp_port_list);
        app_spp_client_init_tx_buff(NULL);
        bt_spp_create_port(RFCOMM_CHANNEL_SERIAL_PORT, SerialPortSppSdpAttributes1, ARRAY_SIZE(SerialPortSppSdpAttributes1));
        bt_spp_set_callback(RFCOMM_CHANNEL_SERIAL_PORT, SERIAL_PORT_RX_BUFF_LEN, app_spp_serial_port_client_callback, NULL);
        bt_spp_listen(RFCOMM_CHANNEL_SERIAL_PORT, true, NULL);
    }
    _app_spp_serial_port_client_dev[device_id] = bt_spp_create_channel(device_id, RFCOMM_CHANNEL_SERIAL_PORT);
    if (!snoop_spp)
    {
        status = bt_spp_connect(&curr_device->remote, RFCOMM_CHANNEL_SERIAL_PORT, remote_uuid, sizeof(remote_uuid));
    }
    else
    {
        //IBRT SLAVE restore profile,NO need call btif open
        bt_spp_callback_param_t spp_param = {0};
        spp_param.error_code = 0;
        spp_param.device_id = device_id;
        spp_param.spp_chan = _app_spp_serial_port_client_dev[device_id];
        serial_port_client_is_connected[device_id] = true;
        app_spp_serial_port_client_callback(remote_addr, BT_SPP_EVENT_OPENED, &spp_param);
        status = BT_STS_SUCCESS;
    }
    TRACE(1,"serial port open return status is %d", status);
}

/**********************************************************************************************
 * @brief           creat an SPP serial port clien connection by the rfcomm port of the application
 *
 * @param[in|       remote_addr                   mobile device address
 * @param[in|       rfcomm_port                   Connect the rfcomm port of the application
 * @param[in]       snoop_spp                     true: slave map restore spp connection
 *                                                false: master creat spp connection
 *********************************************************************************************/

void app_spp_serial_port_client_open_by_port(const bt_bdaddr_t *remote_addr, uint8_t rfcomm_port, bool snoop_spp)
{
    bt_status_t status = BT_STS_FAILED;
    struct BT_ADAPTER_DEVICE_T *curr_device = NULL;
    struct app_spp_port_record_t *spp_port_record = NULL;
    struct list_node *node = NULL;
    struct list_node *head = NULL;
    app_spp_port_record_t *p_app_spp_port_record = NULL;

    app_spp_init();

    curr_device = bt_adapter_get_connected_device_byaddr(remote_addr);
    if (curr_device == NULL)
    {
        TRACE(0, "app_serial_port_client_open: fail,because of no acl connection");
        return;
    }

    head = &g_app_spp_global.spp_port_list;
    colist_iterate(node, head)
    {
        if (node == NULL)
            break;
        p_app_spp_port_record = colist_structure(node, app_spp_port_record_t, node);
        if (p_app_spp_port_record != NULL &&
            (p_app_spp_port_record->local_server_channel == RFCOMM_CHANNEL_SERIAL_PORT))
            break;
        p_app_spp_port_record = NULL;
    }

    if (p_app_spp_port_record)
    {
        spp_port_record = p_app_spp_port_record;
        spp_port_record->remote_rfcomm_port = rfcomm_port;
        spp_port_record->local_server_channel = RFCOMM_CHANNEL_SERIAL_PORT;

        if ((spp_port_record->client_spp_callback == NULL) && \
            (spp_port_record->app_spp_callback != app_spp_serial_port_client_callback))
        {
            bt_spp_set_callback(RFCOMM_CHANNEL_SERIAL_PORT, SERIAL_PORT_RX_BUFF_LEN,
                                spp_port_record->app_spp_callback, app_spp_serial_port_client_callback);
            spp_port_record->client_spp_callback = app_spp_serial_port_client_callback;
        }
    }
    else
    {
        spp_port_record = (struct app_spp_port_record_t *)bes_bt_buf_malloc(sizeof(struct app_spp_port_record_t));

        if (!spp_port_record)
        {
            TRACE(0,"warning serial port malloc fail!");
            return;
        }

        memset(spp_port_record, 0, sizeof(struct app_spp_port_record_t));
        spp_port_record->remote_rfcomm_port = rfcomm_port;
        spp_port_record->local_server_channel = RFCOMM_CHANNEL_SERIAL_PORT;
        spp_port_record->client_spp_callback = app_spp_serial_port_client_callback;
        spp_port_record->app_spp_callback = NULL;
        colist_addto_tail(&spp_port_record->node, &g_app_spp_global.spp_port_list);
        app_spp_client_init_tx_buff(NULL);
        bt_spp_create_port(RFCOMM_CHANNEL_SERIAL_PORT, SerialPortSppSdpAttributes1, ARRAY_SIZE(SerialPortSppSdpAttributes1));
        bt_spp_set_callback(RFCOMM_CHANNEL_SERIAL_PORT, SERIAL_PORT_RX_BUFF_LEN, app_spp_serial_port_client_callback, NULL);
        bt_spp_listen(RFCOMM_CHANNEL_SERIAL_PORT, true, NULL);
    }

    _app_spp_serial_port_client_dev[curr_device->device_id] = bt_spp_create_channel(curr_device->device_id, RFCOMM_CHANNEL_SERIAL_PORT);
    if (!snoop_spp)
    {
        status = bt_spp_connect_server_channel(&curr_device->remote, RFCOMM_CHANNEL_SERIAL_PORT, rfcomm_port);
    }
    else
    {
        //IBRT SLAVE restore profile,NO need call btif open
        bt_spp_callback_param_t spp_param = {0};
        spp_param.error_code = 0;
        spp_param.device_id = curr_device->device_id;
        spp_param.spp_chan = _app_spp_serial_port_client_dev[curr_device->device_id];
        serial_port_client_is_connected[curr_device->device_id] = true;
        app_spp_serial_port_client_callback(remote_addr, BT_SPP_EVENT_OPENED, &spp_param);
        status = BT_STS_SUCCESS;
    }
    TRACE(1,"serial port open return status is %d", status);
}

/**********************************************************************************************
 * @brief           slave restore creat spp profile
 *
 * @param[in|       *bdaddr_p                   mobile device address
 * @param[in|       *buf                        reseved
 * @param[in|       buf_len                     reseved
 * @param[in]       app_id                      Identify spp channel types
 *
 * @note                                               false: master creat spp connection
 *********************************************************************************************/

void app_spp_restore_serial_port_ctx(bt_bdaddr_t *bdaddr_p, uint64_t app_id)
{
    struct list_node *node = NULL;
    struct list_node *head = NULL;
    app_spp_port_record_t *p_app_spp_port_record = NULL;

    if (bt_spp_get_app_id_from_port(RFCOMM_CHANNEL_SERIAL_PORT) == app_id)
    {
        colist_iterate(node, head)
        {
            if (node == NULL)
                break;
            p_app_spp_port_record = colist_structure(node, app_spp_port_record_t, node);
            if (p_app_spp_port_record != NULL &&
                (p_app_spp_port_record->local_server_channel == RFCOMM_CHANNEL_SERIAL_PORT))
                break;
            p_app_spp_port_record = NULL;
        }

        if (p_app_spp_port_record)
        {
            app_spp_serial_port_client_open_by_uuid((const bt_bdaddr_t *)bdaddr_p, p_app_spp_port_record->uuid, true);
        }
    }
}


