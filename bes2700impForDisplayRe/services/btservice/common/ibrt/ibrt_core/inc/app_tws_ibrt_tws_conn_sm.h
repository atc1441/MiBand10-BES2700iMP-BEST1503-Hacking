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
#ifndef __APP_TWS_IBRT__TWS_CONN_SM__
#define __APP_TWS_IBRT__TWS_CONN_SM__

#if defined(IBRT) && defined(IBRT_UI)

#include "stdint.h"
#include "cmsis_os.h"
#include "app_ibrt_conn_evt.h"
#include "app_tws_ibrt_conn_api.h"

#define TWS_EVENT_BASE    0x1000

typedef enum
{
    //Messages for handling connect/disconnect requests.
    SIG_TWS_CONNECT = TWS_EVENT_BASE,
    SIG_TWS_SWITCH,
    SIG_TWS_DISCONNECT,
    SIG_SHARE_TWS_INFO,
    SIG_CANCEL_TWS_CONNECT_PAGE,

    // Messages for handling error conditions.
    EVT_TWS_CONNECT_FAILED,
    EVT_TWS_CONNECT_TIMEOUT,
    EVT_BESAUD_CONNECT_TIMEOUT,
    EVT_TWS_DISCONNECT_TIMEOUT,

    // Messages for feedback from custom API.
    EVT_TWS_CONNECTED,
    EVT_TWS_ENCRYPT,
    EVT_TWS_SIMPLE_PAIRING_COMPLETED,
    EVT_BESAUD_CONNECTED,
    EVT_TWS_BT_MSS_COMPLETE,
    EVT_TWS_DISCONNECTED,
    EVT_BESAUD_DISCONNECTED,
    EVT_SHARE_INFO_COMPLETED,

    IBRT_EVT_MAX_NUM,
} ibrt_tws_message_e;


#define IBRT_TWS_INITIATE_BY_PAIRING             1
#define IBRT_TWS_INITIATE_BY_CONNECT             2

#ifdef __cplusplus
extern "C" {
#endif

void app_tws_ibrt_tws_sm_start();
ibrt_conn_acl_state app_tws_ibrt_get_tws_connection_state();
bool app_tws_ibrt_is_tws_connecting();
bool app_tws_ibrt_is_besaud_connnected();
int app_tws_ibrt_tws_sm_send_msg(ibrt_tws_message_e evt,uint32_t param0=0,uint32_t param1=0,uint32_t param2=0);
int app_tws_ibrt_tws_sm_on_event(unsigned int event, uint32_t param0=0,uint32_t param1=0,uint32_t param2=0);
void app_tws_ibrt_register_share_info_cb(tws_share_info_func_t cb);
bool app_tws_ibrt_sm_is_in_tws_pairing(void);

#ifdef __cplusplus
}
#endif


#endif

#endif /*__APP_TWS_IBRT__TWS_CONN_SM__*/
