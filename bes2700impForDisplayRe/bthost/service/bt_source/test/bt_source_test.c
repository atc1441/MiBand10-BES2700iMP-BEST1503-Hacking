/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#if defined(BT_SOURCE) && defined(A2DP_SOURCE_TEST) && defined(UTILS_ESHELL_EN)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_a2dp_source.h"
#include "app_avrcp_target.h"
#include "app_hfp_ag.h"
#include "btapp.h"
#include "besbt_cfg.h"
#include "nvrecord_env.h"
#include "hal_trace.h"
#include "app_bt_func.h"
#include "app_bt.h"
#include "app_bt_media_manager.h"

#include "bt_source.h"
#include "eshell.h"

static int bt_source_event_callback(bt_source_event_t event, bt_source_event_param_t *param)
{
    bt_bdaddr_t *addr = NULL;
    switch (event) {
        case BT_SOURCE_EVENT_SEARCH_RESULT:
            addr = param->p.search_result.result->addr;
            eshell_putstring_nl("bt_source_search : found device, addr=0x%x 0x%x 0x%x 0x%x 0x%x 0x%x,name=%s",
                addr->address[0], addr->address[1], addr->address[2], addr->address[3], addr->address[4], addr->address[5],
                param->p.search_result.result->name);
        break;
        case BT_SOURCE_EVENT_SEARCH_COMPLETE:
            eshell_putstring_nl("bt_source_search : complete");
        break;
        case BT_SOURCE_EVENT_A2DP_SOURCE_CONNECT_FAIL:
            eshell_putstring_nl("bt_source: connect fail");
        break;
        case BT_SOURCE_EVENT_A2DP_SOURCE_STREAM_OPEN:
            eshell_putstring_nl("bt_source: stream open");
        break;
        case BT_SOURCE_EVENT_A2DP_SOURCE_STREAM_CLOSE:
            eshell_putstring_nl("bt_source: stream close");
        break;
    }
    return 0;
}

// search device
static void bts_bt_source_search_cmd(int argc, char *argv[])
{
    eshell_putstring_nl("search bt device...");
    bt_source_register_callback(bt_source_event_callback);
    app_bt_source_search_device();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_search_dev", "bt_search_dev", bts_bt_source_search_cmd);

// set device discoverable/conncetable mode
static void bts_bt_set_access_mode_cmd(int argc, char *argv[])
{
    uint32_t access_mode = atoi(argv[1]);

    app_bt_set_access_mode(access_mode);

    eshell_putstring_nl("bts_bt_set_access_mode_cmd scan_mode=%d", access_mode);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_set_access_mode", "bt_set_access_mode [scan_mode]", bts_bt_set_access_mode_cmd);


// connect device
static void bts_bt_connect_device_cmd(int argc, char *argv[])
{
    bt_bdaddr_t addr;
    if (argc != 2) {
        eshell_putstring_nl("input parameter error!!!");
        return;
    }
    char *param = argv[1];
    eshell_param_macstr_get_array(param, 0, (unsigned char *)&addr);

    eshell_putstring_nl("bts_bt_connect_device_cmd 0x%02x %02x %02x %02x %02x %02x", \
                        addr.address[0], addr.address[1], addr.address[2], addr.address[3], addr.address[4], addr.address[5]);

    //bt_source_reconnect_a2dp_profile(&addr);
     bt_source_perform_profile_reconnect(&addr);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_connect_dev", "bt_connect_dev [mac_addr]", bts_bt_connect_device_cmd);


// disconnect device
static void bts_bt_disconnect_device_cmd(int argc, char *argv[])
{
    eshell_putstring_nl("disconnect bt device...");
    bt_bdaddr_t addr;
    if (argc != 2) {
        eshell_putstring_nl("input parameter error!!!");
        return;
    }
    char *param = argv[1];
    eshell_param_macstr_get_array(param, 0, (unsigned char *)&addr);

    eshell_putstring_nl("bts_bt_disconnect_device_cmd 0x%02x %02x %02x %02x %02x %02x", \
                        addr.address[0], addr.address[1], addr.address[2], addr.address[3], addr.address[4], addr.address[5]);

    app_bt_source_disconnect_all_connections(false);
    app_bt_report_source_link_disconnected(&addr, BTIF_BEC_NO_ERROR);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_disconnect_dev", "bt_disconnect_dev", bts_bt_disconnect_device_cmd);


static void bts_bt_source_count_connected_cmd(int argc, char *argv[])
{
    uint8_t count = 0;
    count = app_bt_source_count_connected_device();
    eshell_putstring_nl("bt source device connected count: %d", count);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_source_count", "bt_source_count", bts_bt_source_count_connected_cmd);


static void bts_bt_source_disconnected_mobile_link_cmd(int argc, char *argv[])
{
    bt_bdaddr_t addr;
    if (argc != 2) {
        eshell_putstring_nl("input parameter error!!!");
        return;
    }
    char *param = argv[1];
    eshell_param_macstr_get_array(param, 0, (unsigned char *)&addr);
    eshell_putstring_nl("bts_disconn_mobilecmd 0x%02x %02x %02x %02x %02x %02x", \
                        addr.address[0], addr.address[1], addr.address[2], addr.address[3], addr.address[4], addr.address[5]);
    app_bt_source_disconnect_mobile_connections();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_src_disconn_mobile", "bt_src_disconn_mobile", bts_bt_source_disconnected_mobile_link_cmd);


#endif /* BT_SOURCE */
