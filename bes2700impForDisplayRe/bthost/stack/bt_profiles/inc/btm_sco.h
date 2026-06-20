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
#ifndef __BTM_SCO_H__
#define __BTM_SCO_H__

/*****************************header include********************************/
#include <stdint.h>
#include "btm_i.h"

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
    void (*disconnected)(uint8_t device_id, const uint8_t *remote, uint16_t conn_hdl, uint8 status, uint8 reason);
    // Callback when receiving SCO data from the peer device
    void (*data_rx)(uint8_t dev_id, uint16_t sco_hdl, uint8_t flag, const uint8_t* data, uint16_t data_len);
} btm_sco_user_callbak_t;

/**********************private function declaration*************************/

/************************private variable defination************************/
/**
 ****************************************************************************************
 * @brief SCO initialization
 * @param[int] cb: user allcbak
 * @param[out]: none
 *
 * @return: none
 ****************************************************************************************
 */
void btm_sco_init(btm_sco_user_callbak_t* cb);

/**
 ****************************************************************************************
 * @brief SCO deinitialization
 * @param[int]: none
 * @param[out]: none
 *
 * @return: none
 ****************************************************************************************
 */
void btm_sco_deinit();

/**
 ****************************************************************************************
 * @brief Registered codec type support checking callback functions
 * @param[int] cb: callback function, codec_type see@btm_check_hfp_codec_enable_type
 * @param[out]: none
 *
 * @return: none
 ****************************************************************************************
 */
void btm_sco_reg_codec_check(bool (*cb)(uint8_t codec_type, uint8_t dev_id, struct bdaddr_t *bdaddr));

/**
 ****************************************************************************************
 * @brief Get the number of connected SCO
 * @param[int] num: max number
 * @param[out]: none
 *
 * @return: connected number
 ****************************************************************************************
 */
uint8_t btm_sco_get_conn_number();

/**
 ****************************************************************************************
 * @brief Get the maximum number of SCO link
 * @param[int]: none
 * @param[out]: none
 *
 * @return: maximum number link
 ****************************************************************************************
 */
uint8_t btm_sco_get_max_number();

/**
 ****************************************************************************************
 * @brief Search for SCO  by  ACL
 * @param[int] conn : acl object
 * @param[out]: none
 *
 * @return: sco object
 ****************************************************************************************
 */
struct btm_sco_conn_item_t* btm_sco_find_by_acl( struct btm_conn_item_t *conn);

/**
 ****************************************************************************************
 * @brief Search for SCO  by  sco connect handle
 * @param[int] sco_connhdl : sco connect handle
 * @param[out]: none
 *
 * @return: sco object
 ****************************************************************************************
 */
struct btm_sco_conn_item_t* btm_sco_find_by_sco_handle( uint16 sco_connhdl);

/**
 ****************************************************************************************
 * @brief Search for ACL  by  sco connect handle
 * @param[int] sco_connhdl : sco connect handle
 * @param[out]: none
 *
 * @return: acl object
 ****************************************************************************************
 */
struct btm_conn_item_t* btm_sco_find_acl_by_sco_handle(uint16_t sco_handle);

/**
 ****************************************************************************************
 * @brief Initiate a connection request
 * @param[int] remote_bdaddr : remote device bluetooth address
 * @param[out]: none
 *
 * @return: status, see@bt_status_t
 ****************************************************************************************
 */
int8 btm_sco_connect_req (struct bdaddr_t *remote_bdaddr);

/**
 ****************************************************************************************
 * @brief Responding to connection requests from remote devices
 * @param[int] device_id : device index
 * @param[int] accept    : [0]reject    [1]accept
 * @param[out]: none
 *
 * @return: acl object
 ****************************************************************************************
 */
void btm_sco_connect_resp(uint8_t device_id, bool accept);

/**
 ****************************************************************************************
 * @brief Disconnect the SCO link
 * @param[int] remote_bdaddr : remote device bluetooth address
 * @param[int] reason        : disconnect reason, see@btif_error_code_t
 * @param[out]: none
 *
 * @return: acl object
 ****************************************************************************************
 */
int8 btm_sco_conn_disconnect(struct bdaddr_t *remote_bdaddr, uint8 reason);

/**
 ****************************************************************************************
 * @brief Send sco data
 * @param[int] conhdl      : sco connect handle
 * @param[int] payload     : payload data
 * @param[int] payload_len : payload length
 * @param[out]: none
 *
 * @return: none
 ****************************************************************************************
 */
void btm_sco_tx_data_send(uint16_t conhdl, const uint8_t *payload, uint8_t payload_len);

/**
 ****************************************************************************************
 * @brief Notify SCO processing function after ACL connection is successful
 * @param[int] conn   : ACL connection object for remote devices
 * @param[out]: none
 *
 * @return: none
 ****************************************************************************************
 */
void btm_sco_rx_data_handle(uint16_t connhdl, hci_sco_ps_flag_t sco_ps_flag, const uint8_t *sco_data, uint8_t data_len);

/**
 ****************************************************************************************
 * @brief Processing function after receiving a remote device SCO connection request
 * @param[int] pkt : event parameter
 * @param[int] conn: ACL connection object for remote devices
 * @param[out]: none
 *
 * @return: none
 ****************************************************************************************
 */
void btm_sco_conn_req_event_handle(struct hci_evt_packet_t *pkt, struct btm_conn_item_t * conn);

/**
 ****************************************************************************************
 * @brief SCO connection completion processing function
 * @param[int] param  : event parameter
 * @param[int] conn   : ACL connection object for remote devices
 * @param[int] status : Connection status, see@conn_sco_stat_enum
 * @param[out]: none
 *
 * @return: none
 ****************************************************************************************
 */
void btm_sco_conn_cmp_event_handle(struct hci_ev_conn_complete *param, struct btm_conn_item_t *conn, uint8 status);

/**
 ****************************************************************************************
 * @brief SCO disconnection processing function
 * @param[int] sco_conn : SCO connection object for remote devices
 * @param[int] status   : Connection status, see@conn_sco_stat_enum
 * @param[int] reason   : Disconnection reason, see@BT_ECODE_NO_ERROR
 * @param[out]: none
 *
 * @return: none
 ****************************************************************************************
 */
void btm_sco_disconn_event_handle(struct btm_sco_conn_item_t *sco_conn, uint8 status, uint8 reason);

/**
 ****************************************************************************************
 * @brief Notify SCO processing function after ACL connection is successful
 * @param[int] conn : ACL connection object for remote devices
 * @param[out]: none
 *
 * @return: none
 ****************************************************************************************
 */
void btm_sco_ntf_acl_conn_event_handle(struct btm_conn_item_t *conn);

/**
 ****************************************************************************************
 * @brief Create SCO completed
 * @param[int] pkt : Packet parameters
 * @param[out]: none
 *
 * @return: none
 ****************************************************************************************
 */
void btm_sco_cmp_event_handle(struct hci_evt_packet_t *pkt);

/**
 ****************************************************************************************
 * @brief Send sco switch command to Bluetooth controller(Switching between master and slave in TWS)
 * @param[int] sco_handle : sco connect handle
 * @param[out]: none
 *
 * @return: none
 ****************************************************************************************
 */
int8 btm_sco_hci_set_sco_switch (const uint16 sco_handle);

/**
 ****************************************************************************************
 * @brief Send TX silent command to Bluetooth controller
 * @param[int] sco_handle : sco connect handle
 * @param[out]: none
 *
 * @return: none
 ****************************************************************************************
 */
int8 btm_sco_hci_sco_tx_silence (const uint16 connhandle, const uint8 slience_on);

/**
 ****************************************************************************************
 * @brief SCO connected frome tws sync
 * @param[int] bdAddr      : remote device bluetooth address
 * @param[int] codec       : audio code type
 * @param[int] conn_handle : sco connect handle
 * @param[out]: none
 *
 * @return: SUCCESS or FAILURE
 ****************************************************************************************
 */
int8 btm_sco_tws_sync_connected(struct bdaddr_t *bdAddr, uint8 codec, uint16 conn_handle);

/**
 ****************************************************************************************
 * @brief SCO connected frome tws sync
 * @param[int] bdAddr      : remote device bluetooth address
 * @param[int] conn_handle : sco connect handle
 * @param[out]: none
 *
 * @return: none
 ****************************************************************************************
 */
void btm_sco_tws_sync_disconnected(struct bdaddr_t *bdAddr, uint16_t conn_handle);

#ifdef __cplusplus
}
#endif

#endif
