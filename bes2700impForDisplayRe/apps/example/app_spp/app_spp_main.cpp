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

#include "hal_trace.h"
#include "plat_types.h"
#include "bluetooth.h"
#include "app_spp.h"
//#include "app_tws_ibrt.h"

typedef  uint8_t   spp_error_type;
#define SPP_NO_ERROR              0
#define SPP_NO_CONNECTION         1
#define SPP_STACK_ERROR           2

typedef struct
{
    const uint8_t *data;
    uint16_t length;
    uint8_t  port;
    uint8_t  busniss_type;
    uint8_t  remote_type;
    bt_bdaddr_t    remote_addr;
} spp_data_sent_s;

void app_spp_example_serial_port_client_open(const bt_bdaddr_t *addr)
{
    /*
    * For example, Creating spp client services through UUID retrieval
    * the remote device has an SPP service with a UUID of 0xfe35
    */
    app_spp_serial_port_client_open_by_uuid((bt_bdaddr_t *)addr, 0xfe35, false);
    /*
    * For example, Creating spp client services through prot number retrieval
    * the remote device has an SPP service with a prot number of 5
    */
    //app_spp_serial_port_client_open_by_port((bt_bdaddr_t *)addr, 5, false);
}

void app_spp_example_serial_port_client_close(const bt_bdaddr_t *addr)
{
    if (app_spp_serial_port_is_connected(addr))
    {
        app_spp_close_serial_port(addr);
    }
}

spp_error_type app_spp_example_serial_port_client_send_data(const void *input)
{
    spp_data_sent_s send_data = *(spp_data_sent_s *)input;

    if (app_spp_serial_port_send_data(&send_data.remote_addr, send_data.data, send_data.length))
    {
        return SPP_NO_ERROR;
    }
    else
    {
        return SPP_STACK_ERROR;
    }
}

void app_spp_example_client_spp_event_ind(uint8_t client_type,
                const bt_bdaddr_t *remote, bt_spp_event_t event, bt_spp_callback_param_t *param)
{
    /*
     *client_type:
     *          0---->spp serial port
     *          1---->spp other port
     *through client_type judge serial port or other port
     */

    /*
     *get spp_event_type
     *typedef struct
     *  {
     *      bt_bdaddr_t  remote_addr;
     *      uint8_t      business_type; // XXXX_BUSNISS--->1; XXXX_BUSNISS--->2
     *      uint8_t      link_type; //fixed value--->0 mobile_link
     *      uint16_t     port;
     *      uint16_t     state;
     *      uint8_t      reserved1[STRUCT_RESERVED_SIZE_4];
     *      uint8_t      reserved2[STRUCT_RESERVED_SIZE_4];
     *      uint8_t      reserved3[STRUCT_RESERVED_SIZE_4];
     *  } spp_state_change_s;
     */
    switch (event)
    {
        case BT_SPP_EVENT_OPENED:
            break;

        case BT_SPP_EVENT_CLOSED:
            break;

        case BT_SPP_EVENT_TX_DONE:
            break;

        case BT_SPP_EVENT_RX_DATA:
            /*
             *get spp_data_type
             *typedef struct
             *  {
             *      uint16_t       length;
             *      const uint8_t  *data;
             *      uint8_t        link_type; //fixed value--->0 mobile_link
             *      uint8_t        port;
             *      uint32_t       business_type;
             *      bt_bdaddr_t    remote_addr;
             *  } rcv_spp_data_s;
             */
            {
                spp_data_sent_s send_data = {0};
                send_data.remote_addr = param->spp_chan->remote;
                send_data.data = param->rx_data_ptr;
                send_data.length = param->rx_data_len;
                app_spp_example_serial_port_client_send_data((void *)&send_data);
            }
            break;
        default:
            break;
    }
}

void app_spp_main(const bt_bdaddr_t *addr)
{
    /*
    *
    *  as client:
    *      Call interface app_spp_register_client_callback_handle function to register spp client handle
    *      Usually, after the HFP profile is established, interface app_spp_example_serial_port_client_open
    * is called to establish the spp client service;Or during the connection period, it is necessary to use
    * the SPP clientservice to establish the SPP client service
    *
    */
    if (addr)
    {
        app_spp_register_client_callback_handle(app_spp_example_client_spp_event_ind);
        app_spp_example_serial_port_client_open(addr);
    }
    /*
    *  as server:
    *      Call interface app_spp_serial_port_server_init() function during bth initialization
    * to register spp server service
    */
    //app_spp_serial_port_server_init();
}
