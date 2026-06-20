/**
 * @file aob_call_info_define.h
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

#ifndef __AOB_CALL_INFO_DEFINE_H__
#define __AOB_CALL_INFO_DEFINE_H__

/*****************************header include********************************/
#include "ble_aob_common.h"

/******************************macro defination*****************************/

#define CALL_SIGNAL_STRENGTH_REPORT_INTERVAL        (10) //unit seconds

#define CALL_INVALID_IDS 0xFF

#ifndef INVALID_CALL_ID
#define INVALID_CALL_ID (0)
#endif
/******************************type defination******************************/

/****************************function declaration***************************/
#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    /// Bearer Provider Name characteristic
    AOB_CALL_CHAR_TYPE_PROV_NAME                = 0,
    /// Bearer Technology characteristic
    AOB_CALL_CHAR_TYPE_TECHNO                   = 1,
    /// Bearer Signal Strength characteristic
    AOB_CALL_CHAR_TYPE_SIGN_STRENGTH            = 2,
    /// Bearer List Current Calls characteristic
    AOB_CALL_CHAR_TYPE_CURR_CALLS_LIST          = 3,
    /// Status Flags characteristic
    AOB_CALL_CHAR_TYPE_STATUS_FLAGS             = 4,
    /// Incoming Call Target Bearer URI characteristic
    AOB_CALL_CHAR_TYPE_IN_TGT_CALLER_ID         = 5,
    /// Call State characteristic
    AOB_CALL_CHAR_TYPE_CALL_STATE               = 6,
    /// Call Control Point characteristic
    AOB_CALL_CHAR_TYPE_CALL_CTL_PT              = 7,
    /// Termination Reason characteristic
    AOB_CALL_CHAR_TYPE_TERM_REASON              = 8,
    /// Incoming Call characteristic
    AOB_CALL_CHAR_TYPE_INCOMING_CALL            = 9,
    /// Call Friendly Name characteristic
    AOB_CALL_CHAR_TYPE_CALL_FRIENDLY_NAME       = 10,
    /// Bearer URI Schemes Supported List characteristic
    AOB_CALL_CHAR_TYPE_URI_SCHEMES_LIST         = 11,

    /// Bearer UCI characteristic
    AOB_CALL_CHAR_TYPE_UCI                      = 12,
    /// Bearer Signal Strength Reporting Interval characteristic
    AOB_CALL_CHAR_TYPE_SIGN_STRENGTH_INTV       = 13,
    /// Content Control ID characteristic
    AOB_CALL_CHAR_TYPE_CCID                     = 14,
    /// Call Control Point Optional Opcodes characteristic
    AOB_CALL_CHAR_TYPE_CALL_CTL_PT_OPT_OPCODES  = 15,

    AOB_CALL_CHAR_TYPE_MAX                      = 16,
} AOB_CALL_CHARACTERISTIC_TYPE_E;

/// Call state event type
typedef enum
{
    //local originates a call dialing request
    AOB_CALL_EVENT_LOCAL_OUTGOING_REQ_CMP           = 0,
    //peer device originates a call dialing request
    AOB_CALL_EVENT_REMOTE_OUTGOING_REQ_IND          = 1,
    //local request a incoming call(only TBS role, such as mobile)
    AOB_CALL_EVENT_INCOMING_REQ_CMP                 = 2,
    AOB_CALL_EVENT_LOCAL_ACTION_ACCEPT_CMP          = 3,
    AOB_CALL_EVENT_LOCAL_ACTION_TERMINATE_CMP       = 4,
    AOB_CALL_EVENT_LOCAL_ACTION_HOLD_LOCAL_CMP      = 5,
    AOB_CALL_EVENT_LOCAL_ACTION_RETRIEVE_LOCAL_CMP  = 6,
    AOB_CALL_EVENT_LOCAL_ACTION_HOLD_REMOTE_CMP     = 7,
    AOB_CALL_EVENT_LOCAL_ACTION_RETRIEVE_REMOTE_CMP = 8,
    AOB_CALL_EVENT_LOCAL_ACTION_ALTER_CMP           = 9,
    AOB_CALL_EVENT_LOCAL_ACTION_ANSWER_CMP          = 10,
    AOB_CALL_EVENT_LOCAL_JOIN_REQ_CMP               = 11,
    AOB_CALL_EVENT_REMOTE_ACTION_ACCEPT_IND         = 12,
    AOB_CALL_EVENT_REMOTE_ACTION_TERMINATE_IND      = 13,
    AOB_CALL_EVENT_REMOTE_ACTION_HOLD_IND           = 14,
    AOB_CALL_EVENT_REMOTE_ACTION_RETRIEVE_IND       = 15,
    AOB_CALL_EVENT_REMOTE_ACTION_ORIGINATE_IND      = 16,
    AOB_CALL_EVENT_REMOTE_ACTION_JOIN_IND           = 17,
    // access externl telephone service timeout event, such as: remote answer timeout or establish telephone service
    // timeout
    AOB_CALL_EVENT_ACCESS_EXTERNAL_SERVICE_TIMEOUT_IND = 18,

    AOB_CALL_EVENT_MAX_NUM,
} AOB_CALL_STATE_EVENT_E;

typedef struct
{
    uint8_t         con_lid;
    uint8_t         bearer_id;
    uint8_t         uri_len;
    uint8_t         *uri;
} AOB_CALL_CLI_OUTGOING_T;

typedef struct
{
    uint8_t         con_lid;
    uint8_t         bearer_id;
    uint8_t         call_id;
    uint8_t         action_opcode;
} AOB_CALL_CLI_ACTION_T;

typedef struct
{
    uint8_t         con_lid;
    uint8_t         bearer_id;
    uint8_t         nb_calls;
    uint8_t         call_ids[AOB_COMMON_CALL_MAX_NB_IDS];
} AOB_CALL_CLI_JOIN_T;

typedef struct
{
    uint8_t         con_lid;
    uint8_t         bearer_id;
    uint8_t         call_id;
    uint8_t         call_flags;
    uint8_t         uri_len;
    uint8_t         *uri;
} AOB_CALL_SRV_ACTION_IND_T;

typedef struct
{
    uint8_t         con_lid;
    uint8_t         bearer_id;
    uint8_t         call_id;
    uint8_t         terminate_reason;
} AOB_CALL_SRV_TERMINATE_IND_T;

typedef struct
{
    uint8_t         con_lid;
    uint8_t         bearer_id;
    uint8_t         call_id;
    uint8_t         action_opcode;
    uint8_t         result;
} AOB_CALL_CLI_ACTION_RESULT_IND_T;


/* union of all event datatypes */
typedef union
{
    AOB_CALL_CLI_OUTGOING_T   api_call_outgoing;
    AOB_CALL_CLI_ACTION_T     api_call_action;
    AOB_CALL_CLI_JOIN_T       api_call_join;
    AOB_CALL_SRV_ACTION_IND_T     api_call_srv_action_ind;
    AOB_CALL_SRV_TERMINATE_IND_T  api_call_srv_terminate_ind;
} AOB_CALL_CLI_DATA_T;

/* earbud side stm trigger event */
typedef enum
{
    AOB_CALL_CLIENT_OUTGOING_USER_EVT       = 0,
    AOB_CALL_CLIENT_ACCEPT_USER_EVT         = 1,
    AOB_CALL_CLIENT_TERMINATE_USER_EVT      = 2,
    AOB_CALL_CLIENT_HOLD_USER_EVT           = 3,
    AOB_CALL_CLIENT_RETRIEVE_USER_EVT       = 4,
    AOB_CALL_CLIENT_JOIN_USER_EVT           = 5,
    AOB_CALL_SERVER_INCOMING_IND_EVT        = 6,
    AOB_CALL_SERVER_OUTGOING_IND_EVT        = 7,
    AOB_CALL_SERVER_ALTER_IND_EVT           = 8,
    AOB_CALL_SERVER_ANSWER_IND_EVT          = 9,
    AOB_CALL_SERVER_ACCEPT_IND_EVT          = 10,
    AOB_CALL_SERVER_TERMINATE_IND_EVT       = 11,
    AOB_CALL_SERVER_HELD_LOCAL_IND_EVT      = 12,
    AOB_CALL_SERVER_HELD_REMOTE_IND_EVT     = 13,
    AOB_CALL_SERVER_RETRIEVE_LOCAL_IND_EVT  = 14,
    AOB_CALL_SERVER_RETRIEVE_REMOTE_IND_EVT = 15,
    AOB_CALL_SERVER_JOIN_IND_EVT            = 16,
    AOB_CALL_EARPHONE_MAX_NUM_EVT           = 17,
} AOB_CALL_STATE_EARPHONE_EVENT_E;

/* mobile side stm trigger event */
typedef enum
{
    AOB_CALL_API_SERVER_OUTGOING_USER_EVT           = 0,
    AOB_CALL_API_ALERT_USER_EVT                     = 1,
    AOB_CALL_API_ANSWER_USER_EVT                    = 2,
    AOB_CALL_API_INCOMING_USER_EVT                  = 3,
    AOB_CALL_API_SERVER_ACCEPT_USER_EVT             = 4,
    AOB_CALL_API_SERVER_TERMINATE_USER_EVT          = 5,
    AOB_CALL_API_SERVER_HOLD_LOCAL_USER_EVT         = 6,
    AOB_CALL_API_SERVER_RETRIEVE_LOCAL_USER_EVT     = 7,
    AOB_CALL_API_SERVER_HOLD_REMOTE_USER_EVT        = 8,
    AOB_CALL_API_SERVER_RETRIEVE_REMOTE_USER_EVT    = 9,
    AOB_CALL_API_SERVER_JOIN_USER_EVT               = 10,
    AOB_CALL_CLIENT_ACCEPT_IND_EVT                  = 11,
    AOB_CALL_CLIENT_TERMINATE_IND_EVT               = 12,
    AOB_CALL_CLIENT_HOLD_IND_EVT                    = 13,
    AOB_CALL_CLIENT_RETRIEVE_IND_EVT                = 14,
    AOB_CALL_CLIENT_ORIGINATE_IND_EVT               = 15,
    AOB_CALL_CLIENT_JOIN_IND_EVT                    = 16,
    AOB_CALL_MOBILE_MAX_NUM_EVT                     = 17,
} AOB_CALL_STATE_MOBILE_EVENT_E;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __AOB_CALL_INFO_DEFINE_H__ */

/// @} AOB_APP

