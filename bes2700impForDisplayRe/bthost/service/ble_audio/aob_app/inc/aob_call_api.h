/**
 * @file aob_call_api.h
 * @author BES AI team
 * @version 0.1
 * @date 2021-05-24
 *
 * @copyright Copyright (c) 2015-2021 BES Technic.
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
 */
/**
 ****************************************************************************************
 * @addtogroup AOB_APP
 * @{
 ****************************************************************************************
 */

#ifndef __AOB_CALL_API_H__
#define __AOB_CALL_API_H__

/*****************************header include********************************/
#include "aob_call_info_define.h"
#include "aob_stream_handler.h"

/******************************macro defination*****************************/

/******************************type defination******************************/
typedef void (*aob_call_report_ui_value_handler_t)(uint8_t con_lid, uint8_t value_len, uint8_t *value_table);

/****************************function declaration***************************/
#ifdef __cplusplus
extern "C" {
#endif

void aob_call_tbs_discovery(uint8_t con_lid);

/**
 * @brief Join call API for APP layer
 *
 * @param conidx        Local saved ble connected index
 * @param callNb        Number of call to join
 * @param callIds       Pointer of call ID to join
 */
void aob_call_if_join_call(uint8_t conidx, uint8_t callNb, uint8_t *callIds);

/**
 * @brief Outgoing Dial API for APP layer
 *
 * @param conidx        Local saved ble connected index
 * @param uri           Pointer of URI string
 * @param uriLen        Length of URI string
 */
void aob_call_if_outgoing_dial(uint8_t conidx, uint8_t *uri, uint8_t uriLen);

/**
 * @brief Retrieve call to active status API for APP layer
 *
 * @param conidx        Local saved ble connected index
 * @param call_id       Local saved call index
 */
void aob_call_if_retrieve_call(uint8_t conidx, uint8_t call_id);

/**
 * @brief Hold up call API for APP layer
 *
 * @param conidx        Local saved ble connected index
 * @param call_id       Local saved call index
 */
void aob_call_if_hold_call(uint8_t conidx, uint8_t call_id);

/**
 * @brief Terminate call API for APP layer
 *
 * @param conidx        Local saved ble connected index
 * @param call_id       Local saved call index
 */
void aob_call_if_terminate_call(uint8_t conidx, uint8_t call_id);

/**
 * @brief Accept call API for APP layer
 *
 * @param conidx        Local saved ble connected ID
 * @param call_id       Local saved call index
 */
void aob_call_if_accept_call(uint8_t conidx, uint8_t call_id);

void aob_call_if_action_call(uint8_t conidx, uint8_t call_id, uint8_t opcode);

void aob_call_if_free_pending_actions(struct list_node *action_list);

/**
 * @brief Get single call info
 *
 * @param conidx        Local saved ble connected ID
 * @param call_id       Local saved call index
 * @return Call state
 */
uint8_t aob_call_if_call_state_get(uint8_t conidx, uint8_t call_id);

/**
 * @brief Init call client context information
 *
 */
void aob_call_if_init(void);

/**
 * @brief call server control block information
 *
 */
void aob_call_scb_init();

bool aob_call_is_device_call_active(uint8_t conidx);

uint8_t aob_call_if_get_not_idle_call_by_conidx(uint8_t conidx, uint8_t *call_id);

///////////////////////////////////////////////////////////////////////////////////////////
#ifdef AOB_MOBILE_ENABLED
void aob_call_mobile_if_remote_alert_start(uint8_t conidx, uint8_t call_id);

void aob_call_mobile_if_remote_answer_call(uint8_t conidx, uint8_t call_id);

void aob_call_mobile_if_outgoing_dial(uint8_t conidx, uint8_t *uri, uint8_t uriLen, uint8_t username_len);

void aob_call_mobile_if_incoming_call(uint8_t bearer_lid, const char *from_uri, uint8_t from_urilen, const char *to_uri,
                                      uint8_t to_urilen, const char *call_username, uint8_t username_len);

void aob_call_mobile_if_accept_call(uint8_t conidx, uint8_t call_id);

void aob_call_mobile_if_terminate_call(uint8_t bearer_lid, uint8_t call_id);

void aob_call_mobile_if_hold_local_call(uint8_t bearer_lid, uint8_t call_id);

void aob_call_mobile_if_retrieve_local_call(uint8_t bearer_lid, uint8_t call_id);

void aob_call_mobile_join_req(uint8_t bearer_lid,  uint8_t nb_calls, uint8_t *call_ids);

void aob_call_mobile_set_char_val(uint8_t bearer_lid, uint8_t char_type, uint8_t *val, uint8_t len);

void aob_call_mobile_set_call_status(uint8_t bearer_lid, uint8_t status_type, bool enable);

void aob_call_mobile_if_hold_remote_call(uint8_t conidx, uint8_t call_id);

void aob_call_mobile_if_init(void);
#endif

void aob_acc_tbc_set_rpt_intv(uint8_t con_lid, uint8_t bearer_lid, uint8_t interval, uint8_t reliable);
#ifdef __cplusplus
}
#endif

#endif /* #ifndef __AOB_CALL_API_H__ */

/// @} AOB_APP
