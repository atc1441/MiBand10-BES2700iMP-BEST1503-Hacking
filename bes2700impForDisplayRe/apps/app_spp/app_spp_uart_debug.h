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
#ifndef __APP_SPP_UART_DEBUG_H__
#define __APP_SPP_UART_DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
    APP_SPP_CONNECTION_STATE_DISCONNECTED = 0,
    APP_SPP_CONNECTION_STATE_CONNECTED,
} bt_spp_connection_state_t;

typedef void (*app_spp_server_connection_state_callback)(bt_spp_connection_state_t state);
typedef void (*app_spp_server_receive_data_callback)(uint8_t *data, uint16_t data_len);
typedef void (*app_spp_server_tx_done_callback)(void);

typedef struct {
    app_spp_server_connection_state_callback spp_server_connection_state_cb;
    app_spp_server_receive_data_callback spp_server_recv_data_cb;
    app_spp_server_tx_done_callback      spp_server_tx_done;
}app_spp_server_callback_t;


void app_spp_server_init(app_spp_server_callback_t *cb);
bool app_spp_server_send_data(uint8_t* ptrData, uint16_t length);
bool app_spp_server_is_connected(void);

#ifdef __cplusplus
}
#endif
#endif
