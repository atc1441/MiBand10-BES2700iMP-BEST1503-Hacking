/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#ifndef __SCO_API_H__
#define __SCO_API_H__

/*****************************header include********************************/
#include <stdint.h>
#include "bt_common_define.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************external declaration*************************/

/************************private macro defination***************************/

/************************private type defination****************************/
typedef struct
{
    // Callback when remote device initiates SCO connection request
    void (*connect_req)(uint8_t device_id, const uint8_t *remote);
    // Callback during SCO connection establishment
    void (*connected)(uint8_t device_id, const uint8_t *remote, uint16_t conn_hdl, uint8_t codec_type);
    // Callback during SCO connection establishment fail
    void (*connect_fail)(uint8_t device_id, const uint8_t *remote, uint8_t status);
    // Callback during SCO disconnection
    void (*disconnected)(uint8_t device_id, const uint8_t *remote, uint16_t conn_hdl);
    // Callback when receiving SCO data from the peer device
    void (*data_rx)(uint8_t dev_id, uint16_t sco_hdl, uint8_t flag, const uint8_t* data, uint16_t data_len);
} btif_sco_user_callbak_t;

/**********************private function declaration*************************/

/**
 ****************************************************************************************
 * @brief SCO initialization
 * @param[int] cb : user allcbak
 * @param[out]: none
 *
 * @return: none
 ****************************************************************************************
 */
void btif_sco_init(btif_sco_user_callbak_t* cb);

/**
 ****************************************************************************************
 * @brief SCO deinitialization
 * @param[int] cb : user allcbak
 * @param[out]: none
 *
 * @return: none
 ****************************************************************************************
 */
void btif_sco_deinit();

/**
 ****************************************************************************************
 * @brief Get the current number of SCO connections
 * @param[int]: none
 * @param[out]: none
 *
 * @return: number link
 ****************************************************************************************
 */
uint8_t btif_sco_get_conn_number();

/**
 ****************************************************************************************
 * @brief Get the maximum number of SCO link
 * @param[int]: none
 * @param[out]: none
 *
 * @return: maximum number link
 ****************************************************************************************
 */
uint8_t btif_sco_get_max_number();

/**
 ****************************************************************************************
 * @brief Get remote address by  sco connect handle
 * @param[int] sco_connhdl : sco connect handle
 * @param[out]: none
 *
 * @return: remote device bluetooth address
 ****************************************************************************************
 */
bt_bdaddr_t* btif_sco_get_btaddr_by_sco_handle(uint16_t handle);

/**
 ****************************************************************************************
 * @brief Send SCO connection request
 * @param[int] remote_bdaddr : remote device bluetooth address
 * @param[out]: none
 *
 * @return: status see@bt_status_t
 ****************************************************************************************
 */
int  btif_sco_connect_req(struct bdaddr_t *remote_bdaddr);

/**
 ****************************************************************************************
 * @brief Responding to SCO connection requests
 * @param[int] device_id : device index
 * @param[int] accept : [0] no accept,  [1] accept
 * @param[out]: none
 *
 * @return: none
 ****************************************************************************************
 */
void btif_sco_connect_resp(uint8_t device_id, uint8_t accept);

/**
 ****************************************************************************************
 * @brief Disconnect SCO link
 * @param[int] bdaddr : remote device bluetooth address
 * @param[int] reason : disconnect reason
 * @param[out]: none
 *
 * @return: status see@bt_status_t
 ****************************************************************************************
 */
int btif_sco_disconnect(struct bdaddr_t *bdaddr, uint8 reason);

/**
 ****************************************************************************************
 * @brief Send sco data
 * @param[int] sco_conhdl  : sco connect handle
 * @param[int] payload     : payload data
 * @param[int] payload_len : payload data length
 * @param[out]: none
 *
 * @return: status see@bt_status_t
 ****************************************************************************************
 */
int  btif_sco_send_data(uint16_t sco_conhdl, const uint8_t *payload, uint8_t payload_len);

/**
 ****************************************************************************************
 * @brief Send sco switch command to Bluetooth controller(Switching between master and slave in TWS)
 * @param[int] sco_handle : sco connect handle
 * @param[out]: none
 *
 * @return: none
 ****************************************************************************************
 */
bt_status_t btif_sco_tws_switch(uint16_t scohandle);

#ifdef __cplusplus
}
#endif

#endif /*__SCO_API_H__*/
