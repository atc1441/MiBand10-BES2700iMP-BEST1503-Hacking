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
#if defined(FPGA)

#include <string.h>
#include "hal_trace.h"
#include "bluetooth_bt_api.h"
#include "app_bt.h"
#include "app_bt_func.h"
#include "app_fpga_test_proxy.h"
#include "app_tws_ibrt_conn_api.h"

#if defined(IBRT_UI)
#include "app_ibrt_internal.h"
#include "app_ui_queues.h"
#endif

bt_bdaddr_t mobile_addr_1 ={
        .address = {0x6C, 0x04, 0x5E, 0x6C, 0x66, 0x5C}
    };

bt_bdaddr_t mobile_addr_2 ={
        .address = {0x3E, 0x44, 0x82, 0x73, 0x4D, 0x6C}
    };

bt_bdaddr_t mobile_addr_3 ={
        .address = {0x26, 0x88, 0x2D, 0x43, 0xE1, 0xBC}
    };

void app_fpga_proxy_tws_master_pairing(void)
{
    app_ibrt_conn_fpga_start_tws_pairing(0);
}

void app_fpga_proxy_tws_slave_pairing(void)
{
    app_ibrt_conn_fpga_start_tws_pairing(1);
}

void app_fpga_proxy_set_access_mode(void)
{
    app_ibrt_conn_set_discoverable_connectable(true,true);
}

void app_fpga_proxy_tws_connect(void)
{
    app_ibrt_conn_tws_connect_request(false, 0);
}

void app_fpga_proxy_tws_disconnect(void)
{
    app_ibrt_conn_tws_disconnect();
}

void app_fpga_proxy_w4_mobile_connect(void)
{
    if(app_ibrt_conn_is_nv_master())
    {
        app_bt_set_access_mode(BTIF_BAM_GENERAL_ACCESSIBLE);
    }

    if(app_ibrt_conn_is_nv_slave())
    {
        app_bt_set_access_mode(BTIF_BAM_NOT_ACCESSIBLE);
    }
}

void app_fpga_proxy_mobile_link_connect(uint8_t link_idx)
{
    if(link_idx == 1)
    {
        app_ibrt_conn_remote_dev_connect_request(&mobile_addr_1,OUTGOING_CONNECTION_REQ, true, 0);
    }
    else if(link_idx == 2)
    {
        app_ibrt_conn_remote_dev_connect_request(&mobile_addr_2,OUTGOING_CONNECTION_REQ, true, 0);
    }
    else if(link_idx == 3)
    {
        app_ibrt_conn_remote_dev_connect_request(&mobile_addr_3,OUTGOING_CONNECTION_REQ, true, 0);
    }
}

void app_fpga_proxy_mobile_link_disconnect(uint8_t link_idx)
{
    if(link_idx == 1)
    {
       app_ibrt_conn_remote_dev_disconnect_request(&mobile_addr_1,NULL);
    }
    else if(link_idx == 2)
    {
        app_ibrt_conn_remote_dev_disconnect_request(&mobile_addr_2,NULL);
    }
    else if(link_idx == 3)
    {
        app_ibrt_conn_remote_dev_disconnect_request(&mobile_addr_3,NULL);
    }
}

void app_fpga_proxy_start_ibrt_link(uint8_t link_idx)
{
    if(link_idx == 1)
    {
       app_ibrt_conn_connect_ibrt(&mobile_addr_1);
    }
    else if(link_idx == 2)
    {
        app_ibrt_conn_connect_ibrt(&mobile_addr_2);
    }
    else if(link_idx == 3)
    {
        app_ibrt_conn_connect_ibrt(&mobile_addr_3);
    }
}

void app_fpga_proxy_stop_ibrt_link(uint8_t link_idx)
{
    if(link_idx == 1)
    {
       app_ibrt_conn_disconnect_ibrt(&mobile_addr_1);
    }
    else if(link_idx == 2)
    {
        app_ibrt_conn_disconnect_ibrt(&mobile_addr_2);
    }
    else if(link_idx == 3)
    {
        app_ibrt_conn_disconnect_ibrt(&mobile_addr_3);
    }

}

void app_fpga_proxy_link1_role_switch(uint8_t link_idx)
{
    ibrt_status_t status = IBRT_STATUS_SUCCESS;

    if(link_idx == 1)
    {
       status = app_ibrt_conn_tws_role_switch(&mobile_addr_1, false);
    }
    else if(link_idx == 2)
    {
        status = app_ibrt_conn_tws_role_switch(&mobile_addr_2, false);
    }
    else if(link_idx == 3)
    {
        status = app_ibrt_conn_tws_role_switch(&mobile_addr_3, false);
    }

    LOG_I("%s status=%d",__func__,status);
}

void app_fpga_proxy_dump_ibrt_conn_info(void)
{
    LOG_I("%s",__func__);
    app_ibrt_conn_dump_ibrt_info();
}

void app_fpga_proxy_cmd_request(fpga_test_cmd_type_t cmd_type,uint32_t param)
{
    LOG_I("fpga_proxy_request cmd:0x%x,param = %d",cmd_type,param);

    switch(cmd_type)
    {
        case TWS_MASTER_PAIRING:
            app_fpga_proxy_tws_master_pairing();
            break;
        case TWS_SLAVE_PAIRING:
            app_fpga_proxy_tws_slave_pairing();
            break;
        case SET_ACCESS_MODE:
            app_fpga_proxy_set_access_mode();
            break;
        case TWS_CONNECT:
            app_fpga_proxy_tws_connect();
            break;
        case TWS_DISCONNECT:
            app_fpga_proxy_tws_disconnect();
            break;
        case W4_MOBILE_CONNECT:
            app_fpga_proxy_w4_mobile_connect();
            break;
        case MOBILE_LINK_CONNECT:
            app_fpga_proxy_mobile_link_connect(param);
            break;
        case MOBILE_LINK_DISCONNECT:
            app_fpga_proxy_mobile_link_disconnect(param);
            break;
        case START_LINK_IBRT:
            app_fpga_proxy_start_ibrt_link(param);
            break;
        case STOP_LINK_IBRT:
            app_fpga_proxy_stop_ibrt_link(param);
            break;
        case MOBILE_LINK_RW:
            app_fpga_proxy_link1_role_switch(param);
            break;
        case DUMP_IBRT_CONN_INFO:
            app_fpga_proxy_dump_ibrt_conn_info();
            break;
#if defined(IBRT_UI)
        case DUMP_IBRT_MGR_INFO:
            app_ibrt_if_dump_ui_status();
            break;
        case IBRT_MGR_UNIT_TEST:
            //app_ui_queue_test();
            break;
#endif
        default:
            break;
    }
}

void app_fpga_test_proxy_entry(fpga_test_cmd_type_t cmd_type,uint32_t param)
{
    app_bt_start_custom_function_in_bt_thread(cmd_type,
            param,
            ( uint32_t )app_fpga_proxy_cmd_request);
}

#endif
