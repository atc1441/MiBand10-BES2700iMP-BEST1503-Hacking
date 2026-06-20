/***************************************************************************
 *
 * Copyright (c) 2015-2023 BES Technic
 *
 * Authored by BES CD team (Blueelf Prj).
 *
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
#ifndef __TBS_COMMON__
#define __TBS_COMMON__
#include "bluetooth.h"
#include "gatt_service.h"
#include "gaf_cfg.h"

/// Length of Bearer Technology characteristic value
#define TBS_TECHNO_LEN               (1)
/// Length of Bearer Signal Strength characteristic value
#define TBS_SIGN_STRENGTH_LEN        (1)
/// Length of Bearer Signal Strength Reporting Interval characteristic value
#define TBS_SIGN_STRENGTH_INTV_LEN   (1)
/// Length of Call Control Point Optional Opcodes characteristic value
#define TBS_OPT_OPCODES_LEN          (2)
/// Length of Status Flags characteristic value
#define TBS_STATUS_FLAGS_LEN         (1)
/// Minimum number of Calls for Join procedure
#define TBS_JOIN_NB_CALL_MIN         (2)
/// Maximum Incoming or Outgoing URI length
#define TBS_URI_MAX_LEN              (252)
/// Maximal Signal Strength value
#define TBS_SIGN_STRENGTH_MAX        (100)
/// Signal Strength unavailable or meaningless
#define TBS_SIGN_STRENGTH_UNAVA      (255)

/// Characteristic type values
enum tbs_char_type
{
    /// Bearer Provider Name characteristic
    TBS_CHAR_TYPE_PROV_NAME = 0,
    /// Bearer Technology characteristic
    TBS_CHAR_TYPE_TECHNO,
    /// Bearer Signal Strength characteristic
    TBS_CHAR_TYPE_SIGN_STRENGTH,
    /// Bearer List Current Calls characteristic
    TBS_CHAR_TYPE_CURR_CALLS_LIST,
    /// Status Flags characteristic
    TBS_CHAR_TYPE_STATUS_FLAGS,
    /// Incoming Call Target Bearer URI characteristic
    TBS_CHAR_TYPE_IN_TGT_CALLER_ID,
    /// Call State characteristic
    TBS_CHAR_TYPE_CALL_STATE,
    /// Call Control Point characteristic
    TBS_CHAR_TYPE_CALL_CTL_PT,
    /// Termination Reason characteristic
    TBS_CHAR_TYPE_TERM_REASON,
    /// Incoming Call characteristic
    TBS_CHAR_TYPE_INCOMING_CALL,
    /// Call Friendly Name characteristic
    TBS_CHAR_TYPE_CALL_FRIENDLY_NAME,
    /// Bearer URI Schemes Supported List characteristic
    TBS_CHAR_TYPE_URI_SCHEMES_LIST,
    /// Bearer UCI characteristic
    TBS_CHAR_TYPE_UCI,
    /// Bearer Signal Strength Reporting Interval characteristic
    TBS_CHAR_TYPE_SIGN_STRENGTH_INTV,
    /// Content Control ID characteristic
    TBS_CHAR_TYPE_CCID,
    /// Call Control Point Optional Opcodes characteristic
    TBS_CHAR_TYPE_CALL_CTL_PT_OPT_OPCODES,

    TBS_CHAR_TYPE_MAX,
};

/// Descriptor type values
enum tbs_desc_type
{
    /// Client Characteristic Configuration descriptor for Bearer Provider Name characteristic
    TBS_DESC_TYPE_CCC_PROV_NAME = 0,
    /// Client Characteristic Configuration descriptor for Bearer Technology characteristic
    TBS_DESC_TYPE_CCC_TECHNO,
    /// Client Characteristic Configuration descriptor for Bearer Signal Strength characteristic
    TBS_DESC_TYPE_CCC_SIGN_STRENGTH,
    /// Client Characteristic Configuration descriptor for Bearer List Current Calls characteristic
    TBS_DESC_TYPE_CCC_CURR_CALLS_LIST,
    /// Client Characteristic Configuration descriptor for Status Flags characteristic
    TBS_DESC_TYPE_CCC_STATUS_FLAGS,
    /// Client Characteristic Configuration descriptor for Incoming Call Target Bearer
    /// URI characteristic
    TBS_DESC_TYPE_CCC_IN_TGT_CALLER_ID,
    /// Client Characteristic Configuration descriptor for Call State characteristic
    TBS_DESC_TYPE_CCC_CALL_STATE,
    /// Client Characteristic Configuration descriptor for Call Control Point characteristic
    TBS_DESC_TYPE_CCC_CALL_CTL_PT,
    /// Client Characteristic Configuration descriptor for Termination Reason characteristic
    TBS_DESC_TYPE_CCC_TERM_REASON,
    /// Client Characteristic Configuration descriptor for Incoming Call characteristic
    TBS_DESC_TYPE_CCC_INCOMING_CALL,
    /// Client Characteristic Configuration descriptor for Call Friendly Name characteristic
    TBS_DESC_TYPE_CCC_CALL_FRIENDLY_NAME,
    /// Client Characteristic Configuration descriptor for Bearer URI Schemes Supported
    /// List characteristic
    TBS_DESC_TYPE_CCC_URI_SCHEMES_LIST,

    TBS_DESC_TYPE_MAX,
};

/// Bearer technology values
enum tbs_techno
{
    /// 3G technology
    TBS_TECHNO_3G = 0,
    /// 4G technology
    TBS_TECHNO_4G,
    /// LTE technology
    TBS_TECHNO_LTE,
    /// Wi-Fi technology
    TBS_TECHNO_WIFI,
    /// 5G technology
    TBS_TECHNO_5G,
    /// GSM technology
    TBS_TECHNO_GSM,
    /// CDMA technology
    TBS_TECHNO_CDMA,
    /// 2G technology
    TBS_TECHNO_2G,
    /// WCDMA technology
    TBS_TECHNO_WCDMA,
    /// IP technology
    TBS_TECHNO_IP,

    TBS_TECHNO_MAX,
};

/// Call state values
enum tbs_call_state
{
    /// Incoming
    TBS_CALL_STATE_INCOMING = 0,
    /// Dialing
    TBS_CALL_STATE_DIALING,
    /// Alerting
    TBS_CALL_STATE_ALERTING,
    /// Active
    TBS_CALL_STATE_ACTIVE,
    /// Locally Held
    TBS_CALL_STATE_LOC_HELD,
    /// Remotely Held
    TBS_CALL_STATE_REMOTE_HELD,
    /// Locally and Remotely Held
    TBS_CALL_STATE_LOC_REMOTE_HELD,

    TBS_CALL_STATE_MAX,
};

/// Termination reason values
enum tbs_term_reason
{
    /// URI value improperly formed
    TBS_TERM_REASON_URI = 0,
    /// Call fail
    TBS_TERM_REASON_CALL_FAIL,
    /// Remote party ended Call
    TBS_TERM_REASON_REMOTE_END,
    /// Call ended from the Server
    TBS_TERM_REASON_SRV_END,
    /// Line Busy
    TBS_TERM_REASON_BUSY,
    /// Network Congestion
    TBS_TERM_REASON_CONGESTION,
    /// Call ended from the Client
    TBS_TERM_REASON_CLI_END,
    /// No service
    TBS_TERM_REASON_NO_SVC,
    /// No answer
    TBS_TERM_REASON_NO_ANSWER,
    /// Unspecified
    TBS_TERM_REASON_UNSPECIFIED,

    TBS_TERM_REASON_MAX,
};

/// Operation code values
enum tbs_opcode
{
    /// Accept
    TBS_OPCODE_ACCEPT = 0,
    /// Terminate
    TBS_OPCODE_TERMINATE,
    /// Local Hold
    TBS_OPCODE_HOLD,
    /// Local Retrieve
    TBS_OPCODE_RETRIEVE,
    /// Originate
    TBS_OPCODE_ORIGINATE,
    /// Join
    TBS_OPCODE_JOIN,

    TBS_OPCODE_MAX
};

/// Call action values
enum tbs_call_action
{
    /// Accept call
    TBS_ACTION_ACCEPT = 0,
    /// Terminate call
    TBS_ACTION_TERMINATE,
    /// Hold call (local)
    TBS_ACTION_HOLD_LOCAL,
    /// Retrieve call (local)
    TBS_ACTION_RETRIEVE_LOCAL,
    /// Hold call (remote)
    TBS_ACTION_HOLD_REMOTE,
    /// Retrieve call (remote)
    TBS_ACTION_RETRIEVE_REMOTE,
    /// Remote alert started for a call
    TBS_ACTION_ALERT_START,
    /// Remote answer for a call
    TBS_ACTION_ANSWER,

    TBS_ACTION_MAX,
};

/// Call action values
enum tbs_call_action_additional
{
    /// Incoming call (remote)
    TBS_ADD_ACTION_INCOMING = TBS_ACTION_MAX,
    /// Outgoing call (local)
    TBS_ADD_ACTION_OUTGOING,
    /// Join call (local)
    TBS_ADD_ACTION_JOIN,

    TBS_ADD_ACTION_MAX,
};

/// Direction values
enum tbs_direction
{
    /// Incoming
    TBS_DIRECTION_INCOMING = 0,
    /// Outgoing
    TBS_DIRECTION_OUTGOING,
};

/// Status type values
enum tbs_status_type
{
    /// Inband ringtone
    TBS_STATUS_TYPE_INBAND_RINGTONE = 0,
    /// Silent mode
    TBS_STATUS_TYPE_SILENT_MODE,

    TBS_STATUS_TYPE_MAX,
};

/// Call control point notification result code values
enum tbs_cp_ntf_result
{
    /// Opcode write was successful
    TBS_CP_NTF_RESULT_SUCCESS = 0,
    /// An invalid opcode was used for the Call Control Point write
    TBS_CP_NTF_RESULT_OPCODE_NOT_SUPPORTED,
    /// Requested operation cannot be completed
    TBS_CP_NTF_RESULT_OP_NOT_POSSIBLE,
    /// The Call Index used for the Call Control Point write is invalid
    TBS_CP_NTF_RESULT_INVALID_CALL_INDEX,
    /// The opcode written to the Call Control Point was received when the current Call State
    /// for the Call Index is not in the expected state
    TBS_CP_NTF_RESULT_STATE_MISMATCH,
    /// Lack of internal resources to complete the requested action
    TBS_CP_NTF_RESULT_LACK_OF_RESSOURCES,
    /// The Outgoing URI is incorrect or invalid when an Originate opcode is sent
    TBS_CP_NTF_RESULT_INVALID_URI,

    TBS_CP_NTF_RESULT_MAX,
};

/// Call Control Point Optional Opcodes bit field meaning
enum tbs_opt_opcodes_bf
{
    /// Indicate if Local Hold and Local Retrieve operation codes are supported (= 1) or not
    TBS_OPT_OPCODES_LOCAL_HOLD_BIT = CO_BIT(0),
    /// Indicate if Join operation code is supported (= 1) or not
    TBS_OPT_OPCODES_JOIN_BIT = CO_BIT(1),

    TBS_OPT_OPCODES_MASK = (TBS_OPT_OPCODES_LOCAL_HOLD_BIT | TBS_OPT_OPCODES_JOIN_BIT),
};

/// Status Flags bit field meaning
enum tbs_status_flags_bf
{
    /// Inband Ringtone
    TBS_STATUS_FLAGS_INBAND_RINGTONE_BIT = CO_BIT(0),
    /// Silent Mode
    TBS_STATUS_FLAGS_SILENT_BIT = CO_BIT(1),

    TBS_STATUS_FLAGS_MASK = (TBS_STATUS_FLAGS_INBAND_RINGTONE_BIT | TBS_STATUS_FLAGS_SILENT_BIT),
};

/// Call Flags bit field meaning
enum tbs_call_flags_bf
{
    /// Indicate if call is an outgoing call
    TBS_CALL_FLAGS_OUTGOING_BIT = CO_BIT(0),

    TBS_CALL_FLAGS_MASK = TBS_CALL_FLAGS_OUTGOING_BIT,
};

/// Bearer List Current Calls characteristic value description
enum tbs_curr_call_pos
{
    /// Position of List Item length field
    TBS_CURR_CALL_LIST_ITEM_LEN_POS = 0,
    /// Position of Call Index field
    TBS_CURR_CALL_INDEX_POS,
    /// Position of Call State field
    TBS_CURR_CALL_STATE_POS,
    /// Position of Call Flags field
    TBS_CURR_CALL_FLAGS_POS,
    /// Minimum length
    TBS_CURR_CALL_MIN_LEN,
    /// Minimum List Item length value
    TBS_CURR_CALL_MIN_LIST_ITEM_LEN = TBS_CURR_CALL_MIN_LEN - 1,
    /// Position of URI field
    TBS_CURR_CALL_URI_POS = TBS_CURR_CALL_MIN_LEN
};

/// Call State characteristic value description
enum tbs_call_state_pos
{
    /// Position of Call Index field
    TBS_CALL_STATE_INDEX_POS = 0,
    /// Position of Call State field
    TBS_CALL_STATE_STATE_POS,
    /// Position of Call Flags field
    TBS_CALL_STATE_FLAGS_POS,
    /// Minimum length
    TBS_CALL_STATE_MIN_LEN,
};

/// Incoming Call characteristic value description
enum tbs_incoming_call_pos
{
    /// Position of Call Index field
    TBS_INCOMING_CALL_INDEX_POS = 0,
    /// Minimum length
    TBS_INCOMING_CALL_MIN_LEN,
    /// Position of URI field
    TBS_INCOMING_CALL_URI_POS = TBS_INCOMING_CALL_MIN_LEN,
};

/// Call Control Point characteristic value description
enum tbs_cp_pos
{
    /// Position of Opcode field
    TBS_CP_OPCODE_POS = 0,
    /// Minimum length
    TBS_CP_MIN_LEN,
    /// Position of Parameter field
    TBS_CP_PARAM_POS = TBS_CP_MIN_LEN,
    /// Position of Call Index field for Accept, Terminate, Local Hold, Local Retrieve
    /// and Join operations
    TBS_CP_CALL_ID_POS = TBS_CP_PARAM_POS,
    /// Position of URI field for Originate field
    TBS_CP_URI_POS = TBS_CP_PARAM_POS,
    /// Length when only Call Index is provided
    TBS_CP_CALL_ID_LEN = TBS_CP_CALL_ID_POS + 1,
};

/// Termination Reason characteristic value description
enum tbs_term_reason_pos
{
    /// Position of Call Index field
    TBS_TERM_REASON_INDEX_POS = 0,
    /// Position of Reason Code field
    TBS_TERM_REASON_CODE_POS,
    /// Length
    TBS_TERM_REASON_LEN,
};

/// Call Control Point characteristic notification value description
enum tbs_cp_ntf_pos
{
    /// Requested opcode
    TBS_CP_NTF_OPCODE_POS = 0,
    /// Call index
    TBS_CP_NTF_CALL_INDEX_POS,
    /// Result
    TBS_CP_NTF_RESULT_POS,

    TBS_CP_NTF_LEN,
};

/// Application error codes for Telephone Bearer Service
enum tbs_app_err
{
    /// Value changed during Read Long
    TBS_APP_ERR_VALUE_CHANGED = 0x80,

    TBS_APP_ERR_UNEXPECTED,
};

typedef struct ccp_op_pdu_packed
{
    uint8_t op_code;
    uint8_t val[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ccp_op_p_t;

typedef struct ccp_list_curr_call_packed
{
    /// Total len of list item
    uint8_t list_item_len;
    /// Call index alloc by TBS
    uint8_t call_id;
    /// Call state
    uint8_t call_state;
    /// Call Flags
    uint8_t call_flags;
    /// Call URI(variable len)
    uint8_t call_uri[GAF_ARRAY_EMPTY];
} __attribute__((packed)) cpp_list_curr_call_p_t;

typedef struct ccp_call_state_packed
{
    /// Call index alloc by TBS
    uint8_t call_id;
    /// Call state
    uint8_t call_state;
    /// Call Flags
    uint8_t call_flags;
} __attribute__((packed)) ccp_call_state_p_t;

typedef struct ccp_incoming_call_packed
{
    /// Call index alloc by TBS
    uint8_t call_id;
    /// Call URI(variable len)
    uint8_t call_uri[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ccp_incoming_call_p_t;

typedef struct ccp_terminate_reason_packed
{
    /// Call index alloc by TBS
    uint8_t call_id;
    /// Call termination reason
    uint8_t term_reason;
} __attribute__((packed)) ccp_term_reason_p_t;

extern __CONST uint16_t tbs_char_uuid_list[TBS_CHAR_TYPE_MAX];

/**
 * @brief Telephone bearer service character uuid to character type
 *
 * @param  uuid        Character UUId
 *
 * @return uint8_t     Character type
 */
uint8_t tbs_char_uuid_to_char_type_enum(uint16_t uuid);

/**
 * @brief Telephone bearer service check uri value is valid
 *
 * @param  uri         URI value pointer
 * @param  uri_len     URI value length
 *
 * @return true        URI value is valid
 * @return false       URI value is invalid
 */
bool tbs_check_uri_invalid(const uint8_t *uri, uint8_t uri_len);
#endif /// __TBS_COMMON__