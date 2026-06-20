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
#ifndef __ACC_OT_COMMON__
#define __ACC_OT_COMMON__

#include "bluetooth.h"

#include "gaf_cfg.h"

/*DEFINE*/

#define OTS_GATT_UUID_128_LEN           (16)

#define OTS_GATT_UUID_16_LEN            (2)

#define OTS_L2CAP_INVALID_CID           (GAF_INVALID_ANY_LID)

/// Length of Object First-Created characteristic or Last-Modified characteristic value
#define OTP_TIME_LEN                    (7)
/// Length of Object ID characteristic value
#define OTP_ID_LEN                      (6)
/// Length of Object Properties characteristic value
#define OTP_PROPERTIES_LEN              (4)
/// Maximum length of Object Name characteristic value
#define OTP_NAME_LEN_MAX                (120)

/// Instance number of Object List Filter characteristic
#define OTP_FILTER_NUM                  (3)
/// Invalid Connection/Tansfer/Filter/Object local index
#define OTP_INVALID_LID                 (0xFF)
/// Invalid Filter value
#define OTP_INVALID_FILTER_VALUE        (0xFF)

/// Simple Protocol and Service Multiplexers
#define OTS_SPSM                        (0x0025)

/// Object Action Control Point Read/Write timeout timer (30s)
#define OTP_OACP_TIMEOUT                (30000)
/// Object List Control Point Read/Write timeout timer (30s)
#define OTP_OLCP_TIMEOUT                (30000)

/*ENUMERATION*/
/// Characteristic type values for Object Transfer Service
enum otp_char_type
{
    /// Object Action Control Point characteristic
    OTP_CHAR_TYPE_OACP = 0,
    /// Object List Control Point characteristic
    OTP_CHAR_TYPE_OLCP,
    /// Object Changed characteristic
    OTP_CHAR_TYPE_CHANGED,

    OTP_CHAR_TYPE_IND_MAX,

    /// Object Feature characteristic
    OTP_CHAR_TYPE_FEATURE = OTP_CHAR_TYPE_IND_MAX,
    /// Object Name characteristic
    OTP_CHAR_TYPE_NAME,
    /// Object Type characteristic
    OTP_CHAR_TYPE_TYPE,
    /// Object Size characteristic
    OTP_CHAR_TYPE_SIZE,
    /// Object Properties characteristic
    OTP_CHAR_TYPE_PROPERTIES,
    /// Object First-Created characteristic
    OTP_CHAR_TYPE_FIRST_CREATED,
    /// Object Last-Modified characteristic
    OTP_CHAR_TYPE_LAST_MODIFIED,
    /// Object ID characteristic
    OTP_CHAR_TYPE_ID,
    /// Object List Filter characteristic
    OTP_CHAR_TYPE_LIST_FILTER,

    OTP_CHAR_TYPE_MAX,
};

/// Descriptor type values for Object Transfer Service
enum otp_desc_type
{
    /// Client Characteristic Configuration descriptor for Object Action Control Point characteristic
    OTP_DESC_TYPE_CCC_OACP = 0,
    /// Client Characteristic Configuration descriptor for Object List Control Point  characteristic
    OTP_DESC_TYPE_CCC_OLCP,
    /// Client Characteristic Configuration descriptor for Object Changed characteristic
    OTP_DESC_TYPE_CCC_CHANGED,

    OTP_DESC_TYPE_MAX,
};

/// Object Action Control Point Features bit field
enum otp_oacp_feat_bf
{
    /// Object Action Control Point Create Operation code Supported
    OTP_OACP_FEAT_CREATE_SUPP_POS = 0,
    OTP_OACP_FEAT_CREATE_SUPP_BIT = CO_BIT(OTP_OACP_FEAT_CREATE_SUPP_POS),

    /// Object Action Control Point Delete Operation code Supported
    OTP_OACP_FEAT_DELETE_SUPP_POS = 1,
    OTP_OACP_FEAT_DELETE_SUPP_BIT = CO_BIT(OTP_OACP_FEAT_DELETE_SUPP_POS),

    /// Object Action Control Point Calculate Checksum Operation code Supported
    OTP_OACP_FEAT_CALCULATE_CHECKSUM_SUPP_POS = 2,
    OTP_OACP_FEAT_CALCULATE_CHECKSUM_SUPP_BIT = CO_BIT(OTP_OACP_FEAT_CALCULATE_CHECKSUM_SUPP_POS),

    /// Object Action Control Point Execute Operation code Supported
    OTP_OACP_FEAT_EXECUTE_SUPP_POS = 3,
    OTP_OACP_FEAT_EXECUTE_SUPP_BIT = CO_BIT(OTP_OACP_FEAT_EXECUTE_SUPP_POS),

    /// Object Action Control Point Read Operation code Supported
    OTP_OACP_FEAT_READ_SUPP_POS = 4,
    OTP_OACP_FEAT_READ_SUPP_BIT = CO_BIT(OTP_OACP_FEAT_READ_SUPP_POS),

    /// Object Action Control Point Write Operation code Supported
    OTP_OACP_FEAT_WRITE_SUPP_POS = 5,
    OTP_OACP_FEAT_WRITE_SUPP_BIT = CO_BIT(OTP_OACP_FEAT_WRITE_SUPP_POS),

    /// Appending Additional Data to Objects Supported
    OTP_OACP_FEAT_APPENDING_SUPP_POS = 6,
    OTP_OACP_FEAT_APPENDING_SUPP_BIT = CO_BIT(OTP_OACP_FEAT_APPENDING_SUPP_POS),

    /// Truncation of Objects Supported
    OTP_OACP_FEAT_TRUNCATION_SUPP_POS = 7,
    OTP_OACP_FEAT_TRUNCATION_SUPP_BIT = CO_BIT(OTP_OACP_FEAT_TRUNCATION_SUPP_POS),

    /// Patching of Objects Supported
    OTP_OACP_FEAT_PATCHING_SUPP_POS = 8,
    OTP_OACP_FEAT_PATCHING_SUPP_BIT = CO_BIT(OTP_OACP_FEAT_PATCHING_SUPP_POS),

    /// Object Action Control Point Abort Operation code Supported
    OTP_OACP_FEAT_ABORT_SUPP_POS = 9,
    OTP_OACP_FEAT_ABORT_SUPP_BIT = CO_BIT(OTP_OACP_FEAT_ABORT_SUPP_POS),

    /// RFU bits
    OTP_OACP_FEAT_RFU_LSB = 10,
    OTP_OACP_FEAT_RFU_MASK = 0xFFFFFC00,
};

/// Object List Control Point Features bit field
enum otp_olcp_feat_bf
{
    /// Object List Control Point Go To Operation code Supported
    OTP_OLCP_FEAT_GOTO_SUPP_POS = 0,
    OTP_OLCP_FEAT_GOTO_SUPP_BIT = CO_BIT(OTP_OLCP_FEAT_GOTO_SUPP_POS),

    /// Object List Control Point Order Operation code Supported
    OTP_OLCP_FEAT_ORDER_SUPP_POS = 1,
    OTP_OLCP_FEAT_ORDER_SUPP_BIT = CO_BIT(OTP_OLCP_FEAT_ORDER_SUPP_POS),

    /// Object List Control Point Request Number of Objects Operation code Supported
    OTP_OLCP_FEAT_REQ_NUM_OBJECTS_SUPP_POS = 2,
    OTP_OLCP_FEAT_REQ_NUM_OBJECTS_SUPP_BIT = CO_BIT(OTP_OLCP_FEAT_REQ_NUM_OBJECTS_SUPP_POS),

    /// Object List Control Point Clear Marking Operation code Supported
    OTP_OLCP_FEAT_CLEAR_MARKING_SUPP_POS = 3,
    OTP_OLCP_FEAT_CLEAR_MARKING_SUPP_BIT = CO_BIT(OTP_OLCP_FEAT_CLEAR_MARKING_SUPP_POS),

    /// RFU bits
    OTP_OLCP_FEAT_RFU_LSB = 4,
    OTP_OLCP_FEAT_RFU_MASK = 0xFFFFFFF0,
};

/// Object Properties bit field
enum otp_prop_bf
{
    /// Deletion of this object is permitted
    OTP_PROP_DELETE_PERM_POS = 0,
    OTP_PROP_DELETE_PERM_BIT = CO_BIT(OTP_PROP_DELETE_PERM_POS),

    /// Execution of this object is permitted
    OTP_PROP_EXECUTE_PERM_POS = 1,
    OTP_PROP_EXECUTE_PERM_BIT = CO_BIT(OTP_PROP_EXECUTE_PERM_POS),

    /// Reading this object is permitted
    OTP_PROP_READ_PERM_POS = 2,
    OTP_PROP_READ_PERM_BIT = CO_BIT(OTP_PROP_READ_PERM_POS),

    /// Writing data to this object is permitted
    OTP_PROP_WRITE_PERM_POS = 3,
    OTP_PROP_WRITE_PERM_BIT = CO_BIT(OTP_PROP_WRITE_PERM_POS),

    /// Appending data to this object that increases its Allocated Size is permitted
    OTP_PROP_APPEND_PERM_POS = 4,
    OTP_PROP_APPEND_PERM_BIT = CO_BIT(OTP_PROP_APPEND_PERM_POS),

    /// Truncation of this object is permitted
    OTP_PROP_TRUNCATE_PERM_POS = 5,
    OTP_PROP_TRUNCATE_PERM_BIT = CO_BIT(OTP_PROP_TRUNCATE_PERM_POS),

    /// Patching this object by overwriting some of the object's existing contents is permitted
    OTP_PROP_PATCH_PERM_POS = 6,
    OTP_PROP_PATCH_PERM_BIT = CO_BIT(OTP_PROP_PATCH_PERM_POS),

    /// This object is a marked object
    OTP_PROP_MARK_PERM_POS = 7,
    OTP_PROP_MARK_PERM_BIT = CO_BIT(OTP_PROP_MARK_PERM_POS),

    /// RFU bits
    OTP_PROP_RFU_LSB = 8,
    OTP_PROP_RFU_MASK = 0xFFFFFF00,
};

/// Filter type values
enum otp_filter_type
{
    /// No Filter (everything passes)
    OTP_FILTER_TYPE_NO_FILTER = 0,
    /// Name Starts With
    OTP_FILTER_TYPE_NAME_STARTS_WITH,
    /// Name Ends With
    OTP_FILTER_TYPE_NAME_ENDS_WITH,
    /// Name Contains
    OTP_FILTER_TYPE_NAME_CONTAINS,
    /// Name is Exactly
    OTP_FILTER_TYPE_NAME_IS_EXACTLY,
    /// Object Type (UUID)
    OTP_FILTER_TYPE_OBJECT_TYPE,
    /// Created between (inclusive)
    OTP_FILTER_TYPE_CREATED_BETW,
    /// Modified between (inclusive)
    OTP_FILTER_TYPE_MODIFIED_BETW,
    /// Current Size between (inclusive)
    OTP_FILTER_TYPE_CURRENT_SIZE_BETW,
    /// Allocated Size between (inclusive)
    OTP_FILTER_TYPE_ALLOCATED_SIZE_BETW,
    /// Marked Objects
    OTP_FILTER_TYPE_MARKED_OBJECTS,
};

/// Object Action Control Point Operation codes
enum otp_oacp_opcode
{
    /// Create a new, empty object
    OTP_OACP_OPCODE_CREATE = 0x01,
    /// Delete the Current Object
    OTP_OACP_OPCODE_DELETE = 0x02,
    /// Calculate checksum
    OTP_OACP_OPCODE_CALCULATE_CHECKSUM = 0x03,
    /// Perform an operation on the Server device
    OTP_OACP_OPCODE_EXECUTE = 0x04,
    /// Read object data from the Server device
    OTP_OACP_OPCODE_READ = 0x05,
    /// Write object data from the Server device
    OTP_OACP_OPCODE_WRITE = 0x06,
    /// Cease sending previously requested data
    OTP_OACP_OPCODE_ABORT = 0x07,
    /// Response from the Server device
    OTP_OACP_OPCODE_RESPONSE = 0x60,
};

/// Object Action Control Point Write Operation code Mode bit field
enum otp_oacp_mode_bf
{
    /// Indicate if truncation of object is enabled (= 1) or disabled for OACP Write Operation code
    OTP_OACP_MODE_TRUNCATE_POS = 1,
    OTP_OACP_MODE_TRUNCATE_BIT = CO_BIT(OTP_OACP_MODE_TRUNCATE_POS),

    /// RFU bits
    OTP_OACP_MODE_RFU_LSB = 0,
    OTP_OACP_MODE_RFU_MASK = 0xFD,
};

/// Object Action Control Point Result Codes of Response
enum otp_oacp_result_code
{
    /// Response for successful operation
    OTP_OACP_RESULT_SUCCESS = 1,
    /// Response if unsupported Operation code is received
    OTP_OACP_RESULT_OPCODE_NOT_SUPP = 2,
    /// Response if Parameter received does not meet the   requirements of the service
    OTP_OACP_RESULT_INVALID_PARAM = 3,
    /// Response if the number of octets requested via the value of the Length parameter
    /// or Size parameter (as applicable) exceeds the available memory or processing capabilities of the Server
    OTP_OACP_RESULT_INSUFFICIENT_RESOURCES = 4,
    /// Response if the requested OACP procedure cannot be performed
    /// because the Current Object is an Invalid Object
    OTP_OACP_RESULT_INVALID_OBJECT = 5,
    /// Response if the requested procedure could not be performed
    /// because an Object Transfer Channel was not available for use
    OTP_OACP_RESULT_CHANNEL_UNAVAILABLE = 6,
    /// Response if the object type specified in the OACP procedure Type parameter
    /// is not supported by the Server
    OTP_OACP_RESULT_UNSUPP_TYPE = 7,
    /// Response if the requested procedure is not permitted
    /// according to the properties of the Current Object
    OTP_OACP_RESULT_PROC_NOT_PERMITTED = 8,
    /// Response if the Current Object is temporarily locked by the Server
    OTP_OACP_RESULT_OBJECT_LOCAKED = 9,
    /// Response if the requested procedure failed for any reason
    /// other than those enumerated in this table
    OTP_OACP_RESULT_OPERATION_FAILED = 10,
};

/// Object List Control Point Operation codes
enum otp_olcp_opcode
{
    /// Make the first object in the list become the Current Object
    OTP_OLCP_OPCODE_FIRST = 0x01,
    /// Make the last object in the list become the Current Object
    OTP_OLCP_OPCODE_LAST = 0x02,
    /// Make the object immediately preceding the Current Object in the list become the Current Object
    OTP_OLCP_OPCODE_PREVIOUS = 0x03,
    /// Make the object immediately after the Current Object in the list become the Current Object
    OTP_OLCP_OPCODE_NEXT = 0x04,
    /// Enable a Client to select an object directly by specifying its Object ID
    OTP_OLCP_OPCODE_GOTO = 0x05,
    ///   Arrange the list of objects in order according to the parameter
    OTP_OLCP_OPCODE_ORDER = 0x06,
    /// Report the total number of objects found in the list of objects
    OTP_OLCP_OPCODE_REQ_NUM_OBJECTS = 0x07,
    /// Make all objects in the list become unmarked
    OTP_OLCP_OPCODE_CLEAR_MARKING = 0x08,
    /// Response from the Server device
    OTP_OLCP_OPCODE_RESPONSE = 0x70,
};

/// Object List Control Point List Sort Order values
enum otp_olcp_order
{
    /// Order the list by object name, ascending
    OTP_OLCP_ORDER_ASCEND_NAME = 0x01,
    /// Order the list by object type, ascending
    OTP_OLCP_ORDER_ASCEND_TYPE = 0x02,
    /// Order the list by object current size, ascending
    OTP_OLCP_ORDER_ASCEND_CURRENT_SIZE = 0x03,
    /// Order the list by object first-created timestamp, ascending
    OTP_OLCP_ORDER_ASCEND_FIRST_CREATED = 0x04,
    /// Order the list by object last-modified timestamp, ascending
    OTP_OLCP_ORDER_ASCEND_LAST_MODIFIED = 0x05,
    /// Order the list by object name, descending
    OTP_OLCP_ORDER_DESCEND_NAME = 0x11,
    /// Order the list by object type, descending
    OTP_OLCP_ORDER_DESCEND_TYPE = 0x12,
    /// Order the list by object current size, descending
    OTP_OLCP_ORDER_DESCEND_CURRENT_SIZE = 0x13,
    /// Order the list by object first-created timestamp, descending
    OTP_OLCP_ORDER_DESCEND_FIRST_CREATED = 0x14,
    /// Order the list by object last-modified timestamp, descending
    OTP_OLCP_ORDER_DESCEND_LAST_MODIFIED = 0x15,
};

/// Object List Control Point Result Codes of Response
enum otp_olcp_result_code
{
    /// Response for successful operation
    OTP_OLCP_RESULT_SUCCESS = 0x01,
    /// Response if unsupported Operation code is received
    OTP_OLCP_RESULT_OPCODE_NOT_SUPP = 0x02,
    /// Response if Parameter received does not meet the requirements of the service
    OTP_OLCP_RESULT_INVALID_PARAM = 0x03,
    /// Response if the requested procedure failed for a reason other than those enumerated below
    OTP_OLCP_RESULT_OPERATION_FAILED = 0x04,
    /// Response if the requested procedure attempted to select an object
    /// beyond the first object or beyond the last object in the current list
    OTP_OLCP_RESULT_OUT_OF_BOUNDS = 0x05,
    /// Response if the requested procedure failed due to too many objects in the current list
    OTP_OLCP_RESULT_TOO_MANY_OBJECTS = 0x06,
    /// Response if the requested procedure failed due to there being zero objects in the current list
    OTP_OLCP_RESULT_NO_OBJECT = 0x07,
    /// Response if the requested procedure failed due to there being no object with the requested Object ID
    OTP_OLCP_RESULT_ID_NOT_FOUND = 0x08,
};

/// Object Changed Characteristic Flags bit field
enum otp_changed_flags_bf
{
    /// Indicate if source of change is Serve (=0) or another Client
    OTP_CHANGE_FLAGS_SOURCE_POS = 0,
    OTP_CHANGE_FLAGS_SOURCE_BIT = CO_BIT(OTP_CHANGE_FLAGS_SOURCE_POS),

    /// Indicate if change occurred to the object contents (= 1) or not
    OTP_CHANGE_FLAGS_OBJECT_CONTENTS_POS = 1,
    OTP_CHANGE_FLAGS_OBJECT_CONTENTS_BIT = CO_BIT(OTP_CHANGE_FLAGS_OBJECT_CONTENTS_POS),

    /// Indicate if change occurred to the object metadata  (= 1) or not
    OTP_CHANGE_FLAGS_OBJECT_METADATA_POS = 2,
    OTP_CHANGE_FLAGS_OBJECT_METADATA_BIT = CO_BIT(OTP_CHANGE_FLAGS_OBJECT_METADATA_POS),

    /// Indicate if change occurred to the object creation (= 1) or not
    OTP_CHANGE_FLAGS_OBJECT_CREATION_POS = 3,
    OTP_CHANGE_FLAGS_OBJECT_CREATION_BIT = CO_BIT(OTP_CHANGE_FLAGS_OBJECT_CREATION_POS),

    /// Indicate if change occurred to the object deletion (= 1) or not
    OTP_CHANGE_FLAGS_OBJECT_DELETION_POS = 4,
    OTP_CHANGE_FLAGS_OBJECT_DELETION_BIT = CO_BIT(OTP_CHANGE_FLAGS_OBJECT_DELETION_POS),

    /// RFU bits
    OTP_CHANGE_RFU_LSB = 5,
    OTP_CHANGE_RFU_MASK = 0xE0,
};

/// UUID Type
enum otp_uuid_type
{
    /// 16-bit UUID
    OTP_UUID_TYPE_16_BIT = 0,
    /// 128-bit UUID
    OTP_UUID_TYPE_128_BIT = 1,
};

/// Position/length of fields in Object Changed characteristic value
enum otp_changed_pos_len
{
    /// Position of Flags field
    OTP_CHANGED_POS_FLAGS = 0,
    /// Position of Object ID field
    OTP_CHANGED_POS_ID,

    /// Length of Flags field
    OTP_CHANGED_LEN_FLAGS = 1,
    /// Length of Object Changed characteristic value
    OTP_CHANGED_LEN = 7,
};

/// Position/length of fields in OTS Feature characteristic value
enum otp_feature_pos_len
{
    /// Position of OACP Features field
    OTP_FEATURE_POS_OACP = 0,
    /// Position of OLCP Features field
    OTP_FEATURE_POS_OLCP = 4,

    /// Length of OACP Features field
    OTP_FEATURE_LEN_OACP = 4,
    /// Length of OLCP Features field
    OTP_FEATURE_LEN_OLCP = 4,

    /// Length of Feature characteristic value
    OTP_FEATURE_LEN = OTP_FEATURE_LEN_OACP + OTP_FEATURE_LEN_OLCP,
};

/// Position/length of fields in Object Size characteristic value
enum otp_size_pos_len
{
    /// Position of Current Size field
    OTP_SIZE_POS_CURRENT = 0,
    /// Position of Allocated Size field
    OTP_SIZE_POS_ALLOCATED = 4,

    /// Length of Current Size field
    OTP_SIZE_LEN_CURRENT = 4,
    /// Length of Allocated Size field
    OTP_SIZE_LEN_ALLOCATED = 4,

    /// Length of Object Size characteristic value
    OTP_SIZE_LEN = OTP_SIZE_LEN_CURRENT + OTP_SIZE_LEN_ALLOCATED,
};

/// Position/length of fields in Object Action Control Point characteristic value
enum otp_oacp_pos_len
{
    /// Position of Opcode field
    OTP_OACP_POS_OPCODE = 0,
    /// Position of first parameter
    OTP_OACP_POS_PARAM = 1,
    /// Position of Size field
    OTP_OACP_POS_SIZE = OTP_OACP_POS_PARAM,

    /// Length of opcode
    OTP_OACP_LEN_OPCODE = 1,
    /// Length of mode
    OTP_OACP_LEN_MODE = 1,

    /// Position of Offset field
    OTP_OACP_POS_OFFSET = OTP_OACP_POS_PARAM,

    /// Length of Size
    OTP_OACP_LEN_SIZE = 4,
    /// Length of Offset
    OTP_OACP_LEN_OFFSET = 4,
    /// Length of Length
    OTP_OACP_LEN_LENGTH = 4,

    /// Position of Type field
    OTP_OACP_POS_TYPE = 5,
    /// Position of Length field
    OTP_OACP_POS_LENGTH = OTP_OACP_POS_TYPE,
    /// Position of Mode field
    OTP_OACP_POS_MODE = 9,

    /// Length of Object Action Control Point characteristic value
    OTP_OACP_LEN = 21,
};

/// Position/length of fields in Object Action Control Point characteristic response value
enum otp_oacp_rsp_pos_len
{
    /// Position of Response code field
    OTP_OACP_RSP_POS_RESPONSE = 0,
    /// Position of Request Opcode field
    OTP_OACP_RSP_POS_OPCODE = 1,
    /// Position of Result code field
    OTP_OACP_RSP_POS_RESULT = 2,
    /// Position of Response parameter
    OTP_OACP_RSP_POS_PARAM = 3,

    /// Length of response code
    OTP_OACP_RSP_LEN_RESPONSE = 1,
    /// Length of opcode
    OTP_OACP_RSP_LEN_OPCODE = 1,
    /// Length of result code
    OTP_OACP_RSP_LEN_RESULT = 1,
    /// Minimum length of Object Action Control Point characteristic response value
    OTP_OACP_RSP_LEN_MIN = 3,
    /// Length of checksum
    OTP_OACP_RSP_LEN_CHECHSUM = 4,
};

/// Position/length of fields in Object List Control Point characteristic value
enum otp_olcp_pos_len
{
    /// Position of Opcode field
    OTP_OLCP_POS_OPCODE = 0,
    /// Position of Object ID field
    OTP_OLCP_POS_ID = 1,
    /// Position of List Sort Order field
    OTP_OLCP_POS_ORDER = 1,

    /// Length of opcode
    OTP_OLCP_LEN_OPCODE = 1,
    /// Length of order
    OTP_OLCP_LEN_ORDER = 1,
    /// Length of Object List Control Point characteristic value
    OTP_OLCP_LEN = 7,
};

/// Position of fields in Object List Filter characteristic value
enum otp_filter_pos_len
{
    /// Position of Filter value field
    OTP_FILTER_POS_VALUE = 0,
    /// Position of first parameter
    OTP_FILTER_POS_PARAM = 1,
    /// Position of start time
    OTP_FILTER_POS_TIME_START = OTP_FILTER_POS_PARAM,
    /// Position of minimum size
    OTP_FILTER_POS_SIZE_MIN = OTP_FILTER_POS_PARAM,
    /// Position of maximum size field
    OTP_FILTER_POS_SIZE_MAX = 5,
    /// Position of end time field
    OTP_FILTER_POS_TIME_END = 8,

    /// Length of filter value
    OTP_FILTER_LEN_VALUE = 1,
    /// Length of current/allocated size
    OTP_FILTER_LEN_SIZE = 4,
};

/// Position/lengths of fields in Object List Control Point characteristic response value
enum otp_olcp_rsp_pos
{
    /// Position of Response code field
    OTP_OLCP_RSP_POS_RESPONSE = 0,
    /// Position of Opcode field
    OTP_OLCP_RSP_POS_OPCODE = 1,
    /// Position of Result code field
    OTP_OLCP_RSP_POS_RESULT = 2,
    /// Position of response parameter field
    OTP_OLCP_RSP_POS_PARAM = 3,

    /// Length of response code
    OTP_OLCP_RSP_LEN_RESPONSE = 1,
    /// Length of opcode
    OTP_OLCP_RSP_LEN_OPCODE = 1,
    /// Length of result code
    OTP_OLCP_RSP_LEN_RESULT = 1,
    /// Minimum length of Object List Control Point characteristic response value
    OTP_OLCP_RSP_LEN_MIN = 3,
    /// Length of Object number
    OTP_OLCP_RSP_LEN_NUM_OBJECTS = 4,
};

/// Position of date and time in Coordinated Universal Time(UTC)
enum otp_date_time_pos
{
    /// Position of year
    OTP_DATE_TIME_POS_YEAR = 0,
    /// Position of month
    OTP_DATE_TIME_POS_MONTH = 2,
    /// Position of day
    OTP_DATE_TIME_POS_DAY = 3,
    /// Position of hour
    OTP_DATE_TIME_POS_HOUR = 4,
    /// Position of minute
    OTP_DATE_TIME_POS_MIN = 5,
    /// Position of second
    OTP_DATE_TIME_POS_SEC = 6,
};

/// Specific error codes for Object Transfer Profile
enum otp_err
{
    /// No error
    OTP_ERR_NO_ERROR = 0x00,
    /// Write an invalid or not supported value
    OTP_ERR_WRITE_REQUEST_REJECTED = 0x80,
    /// Read/Write an Object Metadata while the Current Object was Invalid
    OTP_ERR_OBJECT_NOT_SELECTED,
    /// Exceed the concurrency limit of the service
    OTP_ERR_CONCURRENCY_LIMIT_EXCEEDED,
    /// Name was already in use by an existing object on the Server
    OTP_ERR_NAME_ALREADY_EXISTS,

    OTP_ERR_UNEXPECTED,
};

/// Object ID
typedef struct ot_object_id
{
    /// Object ID
    uint8_t object_id[OTP_ID_LEN];
} ot_object_id_t;

/*FUNCTIONS*/
/**
 * @brief Objects transfer profile check object ID value is valid
 *
 * @param  p_object_id Object ID value pointer
 *
 * @return true        Object ID value is valid
 * @return false       Object ID value is invalid
 */
bool otp_is_object_id_valid(const ot_object_id_t *p_object_id);

/**
 * @brief Objects transfer profile character uuid to Character type
 *
 * @param  uuid        Character uuid
 *
 * @return uint8_t     Character type
 */
uint8_t otp_char_uuid_to_char_type_enum(uint16_t uuid);

#endif /// __ACC_OT_COMMON__