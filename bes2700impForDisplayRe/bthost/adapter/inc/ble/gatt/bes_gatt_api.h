/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef __BES_GATT_API_H__
#define __BES_GATT_API_H__
#include "ble_gatt_common.h"

#ifdef BLE_HOST_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

#define BES_GATT_SUER_ID_INVALID 0xFF

/*
 * MACRO DEFINITIONS
 ****************************************************************************************
 */
/// Length of an handle
#define BES_GATT_HANDLE_LEN      (2)
/// Length of 16-bit UUID in octets
#define BES_GATT_UUID_16_LEN     (2)
/// Length of 32-bit UUID in octets
#define BES_GATT_UUID_32_LEN     (4)
/// Length of 128-bit UUID in octets
#define BES_GATT_UUID_128_LEN    (16)
/// Length of Database Hash in octets
#define BES_GATT_DB_HASH_LEN     (16)

/// helper macro to define an attribute property
/// @param prop Property see enum #gatt_prop_bf
#define BES_PROP(prop)          (BES_GATT_ATT_##prop##_BIT)
/// helper macro to define an attribute option bit
/// @param opt see enum #gatt_att_info_bf or see enum #gatt_att_ext_info_bf
#define BES_OPT(opt)            (BES_GATT_ATT_##opt##_BIT)

/// helper macro to set service security level
/// @param  lvl_name Security level see enum #gatt_sec_lvl
#define BES_SVC_SEC_LVL(lvl_name)    (((BES_GATT_SEC_##lvl_name) << (BES_GATT_SVC_AUTH_LSB)) & (BES_GATT_SVC_AUTH_MASK))
/// helper macro to set service security level
/// @param  lvl_val Security level value
#define BES_SVC_SEC_LVL_VAL(lvl_val)    (((lvl_val) << (BES_GATT_SVC_AUTH_LSB)) & (BES_GATT_SVC_AUTH_MASK))

/// helper macro to set attribute security level on a specific permission
/// @param  lvl_name Security level see enum #gatt_sec_lvl
/// @param  perm     Permission see enum #gatt_att_info_bf (only RP, WP, NIP authorized)
#define BES_SEC_LVL(perm, lvl_name)  (((BES_GATT_SEC_##lvl_name) << (BES_GATT_ATT_##perm##_LSB)) & (BES_GATT_ATT_##perm##_MASK))

/// helper macro to set attribute security level on a specific permission
/// @param  lvl_val  Security level value
/// @param  perm     Permission see enum #gatt_att_info_bf (only RP, WP, NIP authorized)
#define BES_SEC_LVL_VAL(perm, lvl_val)  (((lvl_val) << (BES_GATT_ATT_##perm##_LSB)) & (BES_GATT_ATT_##perm##_MASK))

/// helper macro to set attribute UUID type
/// @param uuid_type UUID type (16, 32, 128)
#define BES_ATT_UUID(uuid_type)      (((BES_GATT_UUID_##uuid_type) << (BES_GATT_ATT_UUID_TYPE_LSB)) & (BES_GATT_ATT_UUID_TYPE_MASK))

/// helper macro to set service UUID type
/// @param uuid_type UUID type (16, 32, 128)
#define BES_SVC_UUID(uuid_type)      (((BES_GATT_UUID_##uuid_type) << (BES_GATT_SVC_UUID_TYPE_LSB)) & (BES_GATT_SVC_UUID_TYPE_MASK))

#define BES_GATT_UUID_16_LSB(uuid)          (((uuid & 0xFF00) >> 8) | ((uuid & 0x00FF) << 8))

/*---------------- DECLARATIONS -----------------*/
/// Primary service Declaration
#define BES_GATT_DECL_PRIMARY_SERVICE          BES_GATT_UUID_16_LSB(0x2800) //!< Value: 0x2800
/// Secondary service Declaration
#define BES_GATT_DECL_SECONDARY_SERVICE        BES_GATT_UUID_16_LSB(0x2801) //!< Value: 0x2801
/// Include Declaration
#define BES_GATT_DECL_INCLUDE                  BES_GATT_UUID_16_LSB(0x2802) //!< Value: 0x2802
/// Characteristic Declaration
#define BES_GATT_DECL_CHARACTERISTIC           BES_GATT_UUID_16_LSB(0x2803) //!< Value: 0x2803

/*----------------- DESCRIPTORS -----------------*/
/// Characteristic extended properties
#define  BES_GATT_DESC_CHAR_EXT_PROPERTIES     BES_GATT_UUID_16_LSB(0x2900) //!< Value: 0x2900
/// Characteristic user description
#define  BES_GATT_DESC_CHAR_USER_DESCRIPTION   BES_GATT_UUID_16_LSB(0x2901) //!< Value: 0x2901
/// Client characteristic configuration
#define  BES_GATT_DESC_CLIENT_CHAR_CFG         BES_GATT_UUID_16_LSB(0x2902) //!< Value: 0x2902
/// Server characteristic configuration
#define  BES_GATT_DESC_SERVER_CHAR_CFG         BES_GATT_UUID_16_LSB(0x2903) //!< Value: 0x2903

enum bes_gatt_cli_conf
{
    /// Stop notification/indication
    BES_GATT_CLI_STOP_NTFIND = 0x0000,
    /// Start notification
    BES_GATT_CLI_START_NTF   = 0x0001,
    /// Start indication
    BES_GATT_CLI_START_IND   = 0x0002,
};

typedef enum BES_GATT_ERR_CODE
{
    BES_GATT_NO_ERR = 0,
    /// Problem with ATTC protocol response
    BES_GATT_ERR_INVALID_ATT_LEN,
    /// Error in service search
    BES_GATT_ERR_INVALID_TYPE_IN_SVC_SEARCH,
    /// Invalid write data
    BES_GATT_ERR_WRITE,
    /// Signed write error
    BES_GATT_ERR_SIGNED_WRITE,
    /// No attribute client defined
    BES_GATT_ERR_ATTRIBUTE_CLIENT_MISSING,
    /// No attribute server defined
    BES_GATT_ERR_ATTRIBUTE_SERVER_MISSING,
    /// Permission set in service/attribute are invalid
    BES_GATT_ERR_INVALID_PERM,
    /// The Attribute bearer is closed
    BES_GATT_ERR_ATT_BEARER_CLOSE,
    /// No more Attribute bearer available
    BES_GATT_ERR_NO_MORE_BEARER,

    /// parameter Invalid
    BES_GATT_ERR_INVALID,
    /// parameter null pointer
    BES_GATT_ERR_POINTER_NULL,

} BES_GATT_ERR_CODE_E;

typedef enum BES_GATT_CMP_OP
{
    BES_GATT_OP_SVC_ADD=0,
    BES_GATT_OP_SVC_REMOVE,
    BES_GATT_OP_SVC_CTRL,
    BES_GATT_OP_SVC_GET_HASH,
    BES_GATT_OP_SVC_SEND_RELIABLE,
    BES_GATT_OP_SVC_SEND,
    BES_GATT_OP_SVC_SEND_MTP,
    BES_GATT_OP_SVC_SEND_MTP_CANCLE,
    BES_GATT_OP_CLI_DISCOVER,
    BES_GATT_OP_CLI_DISCOVER_INC,
    BES_GATT_OP_CLI_DISCOVER_CHAR,
    BES_GATT_OP_CLI_DISCOVER_DESC,
    BES_GATT_OP_CLI_DISCOVER_CANCEL,
    BES_GATT_OP_CLI_READ,
    BES_GATT_OP_CLI_TREA_BY_UUID,
    BES_GATT_OP_CLI_TREA_MULTIPLE,
    BES_GATT_OP_CLI_WRITE_RELIABLE,
    BES_GATT_OP_CLI_WEITE,
    BES_GATT_OP_CLI_WEITE_EXE,
    BES_GATT_OP_CLI_RECEIVE_CTRL,
    BES_GATT_OP_CLI_MTU_UPDATE,
} BES_GATT_CMP_OP_E;

/// BES GATT EVENT
typedef enum BES_GATT_EVENT_TYPE
{
    BES_GATT_CMP_EVENT = 0,

    /// when peer device requests to read an attribute.
    BES_GATT_SRV_READ_REQ_EVENT,
    /// When the user wants to send by reliable sending, get the event of sending data from the user layer
    BES_GATT_SRV_VAL_GET_REQ_EVENT,
    /// When the client writes reliably, it must first obtain the length of the value
    BES_GATT_SRV_GET_VAL_LEN_REQ_EVENT,
    /// Event triggered by GATT in order to inform GATT server user when an attribute value has been written by a
    /// peer device.
    BES_GATT_SRV_VAL_WRITE_REQ_EVENT,

    /// When the user wants to send by reliable write, get the event of sending data from the user layer
    BES_GATT_CLI_VAL_GET_REQ_EVENT,
    /// Inform GATT client user about reception of either a notification or an indication from peer device.
    BES_GATT_CLI_EVENT_REQ_EVENT,

    BES_GATT_CLI_DIS_SVC_EVENT,
    /// Event triggered when a service is found during service discovery procedure - Only Service information.
    BES_GATT_CLI_DIS_SVC_INFO_EVENT,
    /// Event triggered when an included service is found during discovery procedure.
    BES_GATT_CLI_DIS_INC_SVC_EVENT,
    /// Event triggered when a characteristic is found during discovery procedure.
    BES_GATT_CLI_DIS_CHAR_EVENT,
    /// Event triggered when a characteristic descriptor is found during discovery procedure.
    BES_GATT_CLI_DIS_DESC_EVENT,
    /// Event triggered when an attribute value has been read.
    BES_GATT_CLI_READ_RESP_EVENT,
    /// Event triggered when a service change has been received or if an attribute transaction triggers an
    /// out of sync error
    BES_GATT_CLI_SVC_CHANGED_EVENT,

    BES_GATT_EVENT_AMX,
} BES_GATT_EVENT_TYPE_E;

///  GATT UUID Type
typedef enum BES_GATT_UUID_TYPE
{
    /// 16-bit UUID value
    BES_GATT_UUID_16      = 0x00,
    /// 32-bit UUID value
    BES_GATT_UUID_32      = 0x01,
    /// 128-bit UUID value
    BES_GATT_UUID_128     = 0x02,
    /// Invalid UUID Type
    BES_GATT_UUID_INVALID = 0x03,
} BES_GATT_UUID_TYPE_E;

/// GATT attribute security level
typedef enum BES_GATT_SEC_LVL
{
    /// Attribute value is accessible on an unencrypted link.
    BES_GATT_SEC_NOT_ENC    = 0x00,
    /// Attribute value is accessible on an encrypted link or modified with using write signed procedure
    /// on unencrypted link if bonded using an unauthenticated pairing.
    BES_GATT_SEC_NO_AUTH    = 0x01,
    /// Attribute value is accessible on an encrypted link or modified with using write signed procedure
    /// on unencrypted link if bonded using an authenticated pairing.
    BES_GATT_SEC_AUTH       = 0x02,
    /// Attribute value is accessible on an encrypted link or modified with using write signed procedure
    /// on unencrypted link if bonded using a secure connection pairing.
    BES_GATT_SEC_SECURE_CON = 0x03,
} BES_GATT_SEC_LVL_E;

/// @verbatim
///    15   14    13  12 11 10  9  8   7    6    5   4   3    2    1    0
/// +-----+-----+---+---+--+--+--+--+-----+----+---+---+----+----+----+---+
/// | UUID_TYPE |  NIP  |  WP |  RP | EXT | WS | I | N | WR | WC | RD | B |
/// +-----+-----+---+---+--+--+--+--+-----+----+---+---+----+----+----+---+
///                                  <--------------- PROP -------------->
/// @endverbatim
/// GATT Attribute information Bit Field
typedef enum BES_GATT_ATT_INFO
{
    /// Broadcast descriptor present
    BES_GATT_ATT_B_BIT          = 0x0001,
    BES_GATT_ATT_B_POS          = 0,
    /// Read Access Mask
    BES_GATT_ATT_RD_BIT         = 0x0002,
    BES_GATT_ATT_RD_POS         = 1,
    /// Write Command Enabled attribute Mask
    BES_GATT_ATT_WC_BIT         = 0x0004,
    BES_GATT_ATT_WC_POS         = 2,
    /// Write Request Enabled attribute Mask
    BES_GATT_ATT_WR_BIT         = 0x0008,
    BES_GATT_ATT_WR_POS         = 3,
    /// Notification Access Mask
    BES_GATT_ATT_N_BIT          = 0x0010,
    BES_GATT_ATT_N_POS          = 4,
    /// Indication Access Mask
    BES_GATT_ATT_I_BIT          = 0x0020,
    BES_GATT_ATT_I_POS          = 5,
    /// Write Signed Enabled attribute Mask
    BES_GATT_ATT_WS_BIT         = 0x0040,
    BES_GATT_ATT_WS_POS         = 6,
    /// Extended properties descriptor present
    BES_GATT_ATT_EXT_BIT        = 0x0080,
    BES_GATT_ATT_EXT_POS        = 7,
    /// Read security level permission (see enum #BES_GATT_SEC_LVL).
    BES_GATT_ATT_RP_MASK        = 0x0300,
    BES_GATT_ATT_RP_LSB         = 8,
    /// Write security level permission (see enum #BES_GATT_SEC_LVL).
    BES_GATT_ATT_WP_MASK        = 0x0C00,
    BES_GATT_ATT_WP_LSB         = 10,
    /// Notify and Indication security level permission (see enum #BES_GATT_SEC_LVL).
    BES_GATT_ATT_NIP_MASK       = 0x3000,
    BES_GATT_ATT_NIP_LSB        = 12,
    /// Type of attribute UUID (see enum #BES_GATT_UUID_TYPE_E)
    BES_GATT_ATT_UUID_TYPE_MASK  = 0xC000,
    BES_GATT_ATT_UUID_TYPE_LSB   = 14,

    /// Attribute value property
    BES_GATT_ATT_PROP_MASK      = 0x00FF,
    BES_GATT_ATT_PROP_LSB       = 0,
} BES_GATT_ATT_INFO_E;

/// @verbatim
///       15     14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
/// +-----------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
/// | NO_OFFSET |               WRITE_MAX_SIZE               |
/// +-----------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
/// |                     INC_SVC_HANDLE                     |
/// +-----------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
/// |                     EXT_PROP_VALUE                     |
/// +-----------+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
/// @endverbatim
///
/// GATT Attribute extended information Bit Field
typedef enum BES_GATT_ATT_XET_INFO
{
    /// Maximum value authorized for an attribute write.
    /// Automatically reduce to Maximum Attribute value (GATT_MAX_VALUE) if greater
    BES_GATT_ATT_WRITE_MAX_SIZE_MASK  = 0x7FFF,
    BES_GATT_ATT_WRITE_MAX_SIZE_LSB   = 0,
    /// 1: Do not authorize peer device to read or write an attribute with an offset != 0
    /// 0: Authorize offset usage
    BES_GATT_ATT_NO_OFFSET_BIT        = 0x8000,
    BES_GATT_ATT_NO_OFFSET_POS        = 15,
    /// Include Service handle value
    BES_GATT_INC_SVC_HDL_BIT          = 0xFFFF,
    BES_GATT_INC_SVC_HDL_POS          = 0,
    /// Characteristic Extended Properties value
    BES_GATT_ATT_EXT_PROP_VALUE_MASK  = 0xFFFF,
    BES_GATT_ATT_EXT_PROP_VALUE_LSB   = 0,
} BES_GATT_ATT_XET_INFO_E;

/// @verbatim
///   7      6     5     4      3     2    1   0
/// +-----+-----+-----+------+-----+-----+---+---+
/// | RFU | UUID_TYPE | HIDE | DIS | EKS |  AUTH |
/// +-----+-----+-----+------+-----+-----+---+---+
/// @endverbatim
/// GATT information Bit Field
typedef enum BES_GATT_SVC_INFO
{
    /// Service minimum required security level (see enum #BES_GATT_SEC_LVL).
    BES_GATT_SVC_AUTH_MASK       = 0x03,
    BES_GATT_SVC_AUTH_LSB        = 0,
    /// If set, access to value with encrypted security requirement also requires a 128-bit encryption key size.
    BES_GATT_SVC_EKS_BIT         = 0x04,
    BES_GATT_SVC_EKS_POS         = 2,
    /// If set, service is visible but cannot be used by peer device
    BES_GATT_SVC_DIS_BIT         = 0x08,
    BES_GATT_SVC_DIS_POS         = 3,
    /// Hide the service
    BES_GATT_SVC_HIDE_BIT        = 0x10,
    BES_GATT_SVC_HIDE_POS        = 4,
    /// Type of service UUID (see enum #BES_GATT_UUID_TYPE_E)
    BES_GATT_SVC_UUID_TYPE_MASK  = 0x60,
    BES_GATT_SVC_UUID_TYPE_LSB   = 5,
}BES_GATT_SVC_INFO_E;

/// GATT Send Type
typedef enum BES_GATT_SEND_TYPE
{
    /// Server initiated notification
    BES_GATT_NOTIFY     = 0x00,
    /// Server initiated indication
    BES_GATT_INDICATE   = 0x01,
} BES_GATT_SEND_TYPE_E;

/// GATT Discovery Type
typedef enum BES_GATT_DISCOVERY_TYPE
{
    /// Discover all primary services
    BES_GATT_DISCOVER_SVC_PRIMARY_ALL         = 0x00,
    /// Discover primary services using UUID value
    BES_GATT_DISCOVER_SVC_PRIMARY_BY_UUID     = 0x01,
    /// Discover all secondary services
    BES_GATT_DISCOVER_SVC_SECONDARY_ALL       = 0x02,
    /// Discover secondary services using UUID value
    BES_GATT_DISCOVER_SVC_SECONDARY_BY_UUID   = 0x03,
} BES_GATT_DISCOVERY_TYPE_E;

/// GATT Characteristic Discovery Type
typedef enum BES_GATT_DISCOVERY_CHAR_TYPE
{
    /// Discover all characteristics
    BES_GATT_DISCOVER_CHAR_ALL      = 0x00,
    /// Discover characteristics using UUID value
    BES_GATT_DISCOVER_CHAR_BY_UUID  = 0x01,
} BES_GATT_DISCOVERY_CHAR_TYPE_E;

/// GATT Write Type
typedef enum BES_GATT_WRITE_TYPE
{
    /// Write attribute
    BES_GATT_WRITE              = 0x00,
    /// Write attribute without response
    BES_GATT_WRITE_NO_RESP      = 0x01,
    /// Write attribute signed
    BES_GATT_WRITE_SIGNED       = 0x02,
} BES_GATT_WRITE_TYPE_E;

/// Write execution mode
typedef enum BES_GATT_WRITE_MODE
{
    /// Perform automatic write execution
    BES_GATT_WRITE_MODE_AUTO_EXECUTE    = 0x00,
    /// Force to use prepare write queue. Can be used to write multiple attributes
    BES_GATT_WRITE_MODE_QUEUE    = 0x01,
} BES_GATT_WRITE_MODE_E;

/// GATT service discovery information
typedef enum BES_GATT_SVC_DISC_INFO
{
    /// Complete service present in structure
    BES_GATT_SVC_CMPLT = 0x00,
    /// First service attribute present in structure
    BES_GATT_SVC_START = 0x01,
    /// Last service attribute present in structure
    BES_GATT_SVC_END   = 0x02,
    /// Following service attribute present in structure
    BES_GATT_SVC_CONT  = 0x03,
} BES_GATT_SVC_DISC_INFO_E;


/// Service Discovery Attribute type
typedef enum BES_GATT_ATT_TYPE
{
    /// No Attribute Information
    BES_GATT_ATT_NONE           = 0x00,
    /// Primary service attribute
    BES_GATT_ATT_PRIMARY_SVC    = 0x01,
    /// Secondary service attribute
    BES_GATT_ATT_SECONDARY_SVC  = 0x02,
    /// Included service attribute
    BES_GATT_ATT_INCL_SVC       = 0x03,
    /// Characteristic declaration
    BES_GATT_ATT_CHAR           = 0x04,
    /// Attribute value
    BES_GATT_ATT_VAL            = 0x05,
    /// Attribute descriptor
    BES_GATT_ATT_DESC           = 0x06,
} BES_GATT_ATT_TYPE_E;

/// Attribute Description structure
typedef struct bes_gatt_att_desc
{
    /// Attribute UUID (LSB First)
    uint8_t  uuid[16];
    /// Attribute information bit field (see enum #BES_GATT_ATT_INFO_E)
    uint16_t info;
    /// Attribute extended information bit field (see enum #BES_GATT_ATT_XET_INFO_E)
    /// Note:
    ///   - For Included Services and Characteristic Declarations, this field contains targeted handle.
    ///   - For Characteristic Extended Properties, this field contains 2 byte value
    ///   - For Client Characteristic Configuration and Server Characteristic Configuration, this field is not used.
    uint16_t ext_info;
} bes_gatt_att_desc_t;

typedef struct bes_gatt_add_server
{
    /// info:@verbatim, see @BES_GATT_SVC_INFO_E
    uint8_t               info ;
    uint8_t               p_uuid[16];
    uint8_t               nb_att;
    bes_gatt_att_desc_t * p_atts;
} bes_gatt_add_server_t;

/// Attribute structure
typedef struct bes_gatt_att
{
    /// Attribute handle
    uint16_t hdl;
    /// Value length
    uint16_t length;
} bes_gatt_att_t;

/// GATT_SRV_EVENT_RELIABLE_SEND Command structure definition
typedef struct bes_gatt_srv_send_reliable
{
    /// Connection index
    uint8_t             conidx;
    /// Event type to trigger (see enum #BES_GATT_SEND_TYPE_E)
    uint8_t             evt_type;
    /// Number of attribute
    uint8_t             nb_att;
    /// List of attribute
    bes_gatt_att_t  *atts;
} bes_gatt_srv_send_reliable_t;

/// GATT_SRV_EVENT_SEND Command structure definition
typedef struct bes_gatt_srv_send
{
    /// Connection index
    uint8_t         conidx;
    /// Event type to trigger (see enum #BES_GATT_SEND_TYPE_E)
    uint8_t         evt_type;
    /// Attribute handle
    uint16_t        hdl;
    /// Value length
    uint16_t        value_length;
    /// Value to transmit
    uint8_t         *value;
} bes_gatt_srv_send_t;

typedef struct bes_gatt_srv_send_mtp
{
    /// Connection index bit field
    uint32_t        conidx_bf;
    /// Event type to trigger (see enum #BES_GATT_SEND_TYPE_E)
    uint8_t         evt_type;
    /// Attribute handle
    uint16_t        hdl;
    /// Value length
    uint16_t        value_length;
    /// Value to transmit
    uint8_t         *value;
} bes_gatt_srv_send_mtp_t;

typedef struct bes_gatt_cli_discover_general
{
    /// Connection index
    uint8_t         conidx;
    /// Search start handle
    uint16_t        start_hdl;
    /// Search end handle
    uint16_t        end_hdl;
} bes_gatt_cli_discover_general_t;

typedef struct bes_gatt_cli_discover
{
    /// Connection index
    uint8_t         conidx;
    /// GATT Service discovery type (see enum #BES_GATT_DISCOVERY_TYPE_E)
    uint8_t         disc_type;
    /// Perform discovery of all information present in the service  (1: enable, 0: disable)
    uint8_t         full;
    /// Search start handle
    uint16_t        start_hdl;
    /// Search end handle
    uint16_t        end_hdl;
    /// UUID Type (see enum #BES_GATT_UUID_TYPE_E)
    uint8_t         uuid_type;
    /// Searched Service UUID (meaningful only for discovery by UUID)
    uint8_t         uuid[16];
} bes_gatt_cli_discover_t;

typedef struct bes_gatt_cli_discover_char
{
    /// Connection index
    uint8_t         conidx;
    /// GATT characteristic discovery type (see enum #BES_GATT_DISCOVERY_CHAR_TYPE_E)
    uint8_t         disc_type;
    /// Search start handle
    uint16_t        start_hdl;
    /// Search end handle
    uint16_t        end_hdl;
    /// UUID Type (see enum #BES_GATT_UUID_TYPE_E)
    uint8_t         uuid_type;
    /// Searched UUID (meaningful only for discovery by UUID)
    uint8_t         uuid[16];
} bes_gatt_cli_discover_char_t;

typedef struct bes_gatt_cli_read
{
    /// Connection index
    uint8_t         conidx;
    /// Attribute handle
    uint16_t        hdl;
    /// Value offset
    uint16_t        offset;
    /// Value length to read (0 = read complete value)
    uint16_t        length;
} bes_gatt_cli_read_t;

typedef struct bes_gatt_cli_read_by_uuid
{
    /// Connection index
    uint8_t         conidx;
    /// Search start handle
    uint16_t        start_hdl;
    /// Search end handle
    uint16_t        end_hdl;
    /// UUID Type (see enum #BES_GATT_UUID_TYPE_E)
    uint8_t         uuid_type;
    /// Searched UUID
    uint8_t         uuid[16];
} bes_gatt_cli_read_by_uuid_t;

typedef struct bes_gatt_cli_read_multiple
{
    /// Connection index
    uint8_t             conidx;
    /// Number of attribute
    uint8_t             nb_att;
    /// List of attribute
    /// If Attribute length is zero (consider length unknown):
    ///     - Attribute protocol read multiple variable length procedure used
    bes_gatt_att_t  *atts;
} bes_gatt_cli_read_multiple_t;

typedef struct bes_gatt_cli_write_reliable
{
    /// Connection index
    uint8_t         conidx;
    /// GATT write type (see enum #BES_GATT_WRITE_TYPE_E)
    uint8_t         write_type;
    /// Write execution mode (see enum #BES_GATT_WRITE_MODE_E). Valid only for GATT_WRITE.
    uint8_t         write_mode;
    /// Attribute handle
    uint16_t        hdl;
    /// Value offset, valid only for GATT_WRITE
    uint16_t        offset;
    /// Value length to write
    uint16_t        length;
} bes_gatt_cli_write_reliable_t;

typedef struct bes_gatt_cli_write
{
    /// Connection index
    uint8_t         conidx;
    /// GATT write type (see enum #BES_GATT_WRITE_TYPE_E)
    uint8_t         write_type;
    /// Attribute handle
    uint16_t        hdl;
    /// Value offset, valid only for GATT_WRITE
    uint16_t        offset;
    /// Value length to write
    uint16_t        value_length;
    /// Attribute value
    uint8_t         *value;
} bes_gatt_cli_write_t;

typedef struct bes_gatt_cli_rec_ctrl
{
    /// Connection index
    uint8_t         conidx;
    /// GATT client receive control, 0:disenable receive, 1:receive
    uint8_t         en;
    /// Search start handle
    uint16_t        start_hdl;
    /// Search end handle
    uint16_t        end_hdl;
} bes_gatt_cli_rec_ctrl_t;

typedef union bes_gatt_cmp_param
{
    /// operation = BES_GATT_OP_SVC_ADD
    //  Service start handle associated to created service.
    uint16_t        start_hdl;
    /// operation = BES_GATT_OP_SVC_GET_HASH
    /// Database Hash.
    uint8_t         hash[16];
    /// operation = BES_GATT_OP_SVC_SEND_RELIABLE,
    /// operation = BES_GATT_OP_SVC_SEND,
    /// operation = BES_GATT_OP_SVC_SEND_MTP,
    /// operation = BES_GATT_OP_SVC_SEND_MTP_CANCLE
    /// operation = BES_GATT_OP_CLI_DISCOVER,
    /// operation = BES_GATT_OP_CLI_DISCOVER_INC,
    /// operation = BES_GATT_OP_CLI_DISCOVER_CHAR,
    /// operation = BES_GATT_OP_CLI_DISCOVER_DESC,
    /// operation = BES_GATT_OP_CLI_DISCOVER_CANCEL
    /// operation = BES_GATT_OP_CLI_WEITE,
    /// operation = BES_GATT_OP_CLI_WEITE_EXE,
    /// operation = BES_GATT_OP_CLI_RECEIVE_CTRL
    /// Connection index
    uint8_t         conidx;
}bes_gatt_cmp_param_u;

typedef struct bes_gatt_cmp_event
{
    /// operation complete , see@BES_GATT_CMP_OP_E
    uint8_t               operation;
    /// complete status, see@BES_GATT_ERR_CODE_E
    uint16_t              status;
    /// complete operation parameter
    bes_gatt_cmp_param_u  param;
}bes_gatt_cmp_event_t;

typedef struct bes_gatt_svc_read_req_event
{
    /// Token provided by GATT module that must be used into message
    uint16_t        token;
    /// Connection index
    uint8_t         conidx;
    /// Attribute handle
    uint16_t        hdl;
    /// Value offset
    uint16_t        offset;
    /// Maximum value length to return
    uint16_t        max_length;
} bes_gatt_svc_read_req_event_t;

typedef struct bes_gatt_srv_val_get_event
{
    /// Token provided by GATT module that must be used into message
    uint16_t        token;
    /// Connection index
    uint8_t         conidx;
    /// Attribute handle
    uint16_t        hdl;
    /// Maximum value length to return
    uint16_t        max_length;
} bes_gatt_srv_val_get_event_t;

typedef struct bes_gatt_svc_val_basic_resp
{
    /// Token provided by GAT module into message
    uint16_t        token;
    /// Connection index
    uint8_t         conidx;
    /// Status of the request by GATT user, see@BES_GATT_ERR_CODE_E
    uint16_t        status;
    /// Complete Length of the attribute value
    uint16_t        att_length;
    /// Value length
    uint16_t        value_length;
    /// Attribute value (starting from data offset)
    uint8_t         *value;
} bes_gatt_svc_val_basic_resp_t;

typedef struct bes_gatt_srv_get_val_len_event
{
    /// Token provided by GATT module that must be used into the GAT_CFM message
    uint16_t        token;
    /// Connection index
    uint8_t         conidx;
    /// Attribute handle
    uint16_t        hdl;
} bes_gatt_srv_get_val_len_event_t;

typedef struct bes_gatt_srv_get_val_len_resp
{
    /// Token provided by GATT module into message
    uint16_t        token;
    /// Connection index
    uint8_t         conidx;
    /// Status of the request by GATT user,see@BES_GATT_ERR_CODE_E
    uint16_t        status;
    /// Attribute Value length
    uint16_t        att_length;
} bes_gatt_srv_get_val_len_resp_t;

typedef struct bes_gatt_srv_write_req_event
{
    /// Token provided by GATT module that must be used into message
    uint16_t        token;
    /// Connection index
    uint8_t         conidx;
    /// Attribute handle
    uint16_t        hdl;
    /// Value offset
    uint16_t        offset;
    /// Value length to write
    uint16_t        value_length;
    /// Attribute value to update (starting from offset)
    uint8_t         *value;
} bes_gatt_srv_write_req_event_t;

typedef struct bes_gatt_srv_write_resp
{
    /// Token provided by GAT module into message
    uint16_t        token;
    /// Connection index
    uint8_t         conidx;
    /// Status of the request by GATT user, see@BES_GATT_ERR_CODE_E
    uint16_t        status;
} bes_gatt_srv_write_resp_t;

typedef struct bes_gatt_cli_get_val_req_event
{
    /// Token provided by GATT module that must be used into  message
    uint16_t        token;
    /// Connection index
    uint8_t         conidx;
    /// Attribute handle
    uint16_t        hdl;
    /// Data offset
    uint16_t        offset;
    /// Maximum value length to return
    uint16_t        max_length;
} bes_gatt_cli_get_val_req_event_t;

typedef struct bes_gatt_cli_get_val_resp
{
    /// Token provided by GAT module into message
    uint16_t        token;
    /// Connection index
    uint8_t         conidx;
    /// Status of the request by GATT user, see@BES_GATT_ERR_CODE_E
    uint16_t        status;
    /// Value length
    uint16_t        value_length;
    /// Attribute value (starting from data offset)
    uint8_t         *value;
} bes_gatt_cli_get_val_resp_t;

typedef struct bes_gatt_cli_req_event
{
    /// Token provided by GATT module that must be used into the GAT_CFM message
    uint16_t        token;
    /// Connection index
    uint8_t         conidx;
    /// Event type triggered (see enum #BES_GATT_SEND_TYPE_E)
    uint8_t         evt_type;
    /// 1: if event value if complete value has been received
    /// 0: if data received is equals to maximum attribute protocol value. In such case GATT Client User should
    ///    perform a read procedure.
    uint8_t         complete;
    /// Attribute handle
    uint16_t        hdl;
    /// Value length
    uint16_t        value_length;
    /// Attribute value
    uint8_t         *value;
} bes_gatt_cli_req_event_t;

typedef struct bes_gatt_cli_event_resp
{
    /// Token provided by GAT module into the GATT_REQ_IND message
    uint16_t        token;
    /// Connection index
    uint8_t         conidx;
    /// Status of the request by GATT user, see@BES_GATT_ERR_CODE_E
    uint16_t        status;
} bes_gatt_cli_event_resp_t;

typedef struct bes_gatt_svc_att
{
    /// Attribute Type (see enum #BES_GATT_ATT_TYPE_E)
    uint8_t  att_type;
    /// UUID type (see enum #BES_GATT_UUID_TYPE_E)
    uint8_t  uuid_type;
    /// UUID - LSB first (0 for GATT_ATT_NONE)
    uint8_t  uuid[16];

    /// Information about Service attribute union
    union bes_gatt_info
    {
        /// Service info structure
        //#BES_GATT_ATT_PRIMARY_SVC
        //#BES_GATT_ATT_SECONDARY_SVC
        //#BES_GATT_ATT_INCL_SVC)
        struct bes_gatt_svc_info
        {
            /// Service start handle
            uint16_t start_hdl;
            /// Service end handle
            uint16_t end_hdl;
        } svc;

        /// Characteristic info structure
        //#BES_GATT_ATT_CHAR
        struct bes_gatt_char_info
        {
            /// Value handle
            uint16_t val_hdl;
            /// Characteristic properties (see enum #BES_GATT_ATT_INFO_E - bits [0-7])
            uint8_t  prop;
        } charac;
    } info; //!< Information about Service attribute
} bes_gatt_svc_att_t;

typedef struct bes_gatt_cli_dis_svc_event
{
    /// Connection index
    uint8_t         conidx;
    /// First handle value of following list
    uint16_t        hdl;
    /// Discovery information (see enum #BES_GATT_SVC_DISC_INFO_E)
    uint8_t         disc_info;
    /// Number of attributes
    uint8_t         nb_att;
    /// Attribute information present in a service
    bes_gatt_svc_att_t *atts;
} bes_gatt_cli_dis_svc_event_t;

typedef struct bes_gatt_cli_dis_svc_basic_event
{
    /// Connection index
    uint8_t         conidx;
    /// Service start handle
    uint16_t        start_hdl;
    /// Service end handle
    uint16_t        end_hdl;
    /// UUID Type (see enum #BES_GATT_UUID_TYPE_E)
    uint8_t         uuid_type;
    /// Service UUID (LSB first)
    uint8_t         uuid[16];
} bes_gatt_cli_dis_svc_basic_event_t;

typedef struct bes_gatt_cli_dis_char_event
{
    /// Connection index
    uint8_t         conidx;
    /// Characteristic attribute handle
    uint16_t        char_hdl;
    /// Value handle
    uint16_t        val_hdl;
    /// Characteristic properties (see enum #BES_GATT_ATT_INFO_E - bits [0-7])
    uint8_t         prop;
    /// UUID Type (see enum #BES_GATT_UUID_TYPE_E)
    uint8_t         uuid_type;
    /// Characteristic value UUID - LSB first
    uint8_t         uuid[16];
}bes_gatt_cli_dis_char_event_t;

typedef struct bes_gatt_cli_dis_desc_event
{
    /// Connection index
    uint8_t         conidx;
    /// Characteristic descriptor attribute handle
    uint16_t        desc_hdl;
    /// UUID Type (see enum #BES_GATT_UUID_TYPE_E)
    uint8_t         uuid_type;
    /// Attribute UUID - LSB first
    uint8_t         uuid[16];
} bes_gatt_cli_dis_desc_event_t;

/// GATT_CLI_ATT_VAL Indication structure definition
typedef struct bes_gatt_cli_read_resp_event
{
    /// Connection index
    uint8_t         conidx;
    /// Attribute handle
    uint16_t        hdl;
    /// Data offset
    uint16_t        offset;
    /// Value length
    uint16_t        value_length;
    /// Attribute value starting from offset
    uint8_t         *value;
} bes_gatt_cli_read_resp_event_t;

typedef struct bes_gatt_cli_svc_changed_event
{
    /// Connection index
    uint8_t         conidx;
    /// True if an out of sync error has been received
    uint8_t         out_of_sync;
    /// Service start handle
    uint16_t        start_hdl;
    /// Service end handle
    uint16_t        end_hdl;
} bes_gatt_cli_svc_changed_event_t;


typedef union bes_gatt_event_param
{
    bes_gatt_cmp_event_t               cmp;
    bes_gatt_svc_read_req_event_t      svc_read_req;
    bes_gatt_srv_val_get_event_t       svc_val_get_req;
    bes_gatt_srv_get_val_len_event_t   svc_get_val_len_req;
    bes_gatt_srv_write_req_event_t     svc_write_req;
    bes_gatt_cli_get_val_req_event_t   cli_get_val_req;
    bes_gatt_cli_req_event_t           cli_event_req;
    bes_gatt_cli_dis_svc_event_t       cli_dis_svc;
    bes_gatt_cli_dis_svc_basic_event_t cli_dis_svc_info;
    bes_gatt_cli_dis_svc_basic_event_t cli_dis_svc_inc;
    bes_gatt_cli_dis_char_event_t      cli_dis_char;
    bes_gatt_cli_dis_desc_event_t      cli_dis_desc;
    bes_gatt_cli_read_resp_event_t     cli_read_resp;
    bes_gatt_cli_svc_changed_event_t   cli_svc_change;
}bes_gatt_event_param_u;

typedef void (*bes_gatt_event_cb)(BES_GATT_EVENT_TYPE_E type, bes_gatt_event_param_u *param_p);


/// bes gatt event callback register and unregister api
uint8_t bes_ble_gatt_user_register(bes_gatt_event_cb cb);
void bes_ble_gatt_user_unreg(uint8_t gatt_user_id);

void bes_ble_gatt_set_mtu(uint8_t conidx, uint16_t mtu);

/// get connect device att mtu
BES_GATT_ERR_CODE_E bes_ble_gatt_mtu_get(uint8_t conidx, uint16_t *mtu);

/// bes gatt service api
BES_GATT_ERR_CODE_E bes_ble_gatt_svc_add(uint8_t gatt_user_id, bes_gatt_add_server_t *svc_info);
BES_GATT_ERR_CODE_E bes_ble_gatt_svc_remove(uint8_t gatt_user_id, uint16_t start_hdl);
BES_GATT_ERR_CODE_E bes_ble_gatt_svc_ctrl(uint8_t gatt_user_id, uint8_t enable, uint8_t visible, uint16_t start_hdl);
BES_GATT_ERR_CODE_E bes_ble_gatt_svc_get_hash(uint8_t gatt_user_id);
BES_GATT_ERR_CODE_E bes_ble_gatt_svc_send_reliable(uint8_t gatt_user_id, bes_gatt_srv_send_reliable_t *param);
BES_GATT_ERR_CODE_E bes_ble_gatt_svc_send(uint8_t gatt_user_id, bes_gatt_srv_send_t *param);
BES_GATT_ERR_CODE_E bes_ble_gatt_svc_send_mtp(uint8_t gatt_user_id, bes_gatt_srv_send_mtp_t *param);
BES_GATT_ERR_CODE_E bes_ble_gatt_svc_send_mtp_cancel(uint8_t gatt_user_id);
BES_GATT_ERR_CODE_E bes_ble_gatt_svc_read_resp(bes_gatt_svc_val_basic_resp_t *param);
BES_GATT_ERR_CODE_E bes_ble_gatt_svc_val_get_resp(bes_gatt_svc_val_basic_resp_t *param);
BES_GATT_ERR_CODE_E bes_ble_gatt_svc_get_val_len_resp(bes_gatt_srv_get_val_len_resp_t *param);
BES_GATT_ERR_CODE_E bes_ble_gatt_svc_write_resp(bes_gatt_srv_write_resp_t *param);

/// bes gatt client api
BES_GATT_ERR_CODE_E bes_ble_gatt_cli_discover(uint8_t gatt_user_id, bes_gatt_cli_discover_t *param);
BES_GATT_ERR_CODE_E bes_ble_gatt_cli_discover_included(uint8_t gatt_user_id, bes_gatt_cli_discover_general_t *param);
BES_GATT_ERR_CODE_E bes_ble_gatt_cli_discover_char(uint8_t gatt_user_id, bes_gatt_cli_discover_char_t *param);
BES_GATT_ERR_CODE_E bes_ble_gatt_cli_discover_desc(uint8_t gatt_user_id, bes_gatt_cli_discover_general_t *param);
BES_GATT_ERR_CODE_E bes_ble_gatt_cli_discover_cancel(uint8_t gatt_user_id, uint8_t conidx);
BES_GATT_ERR_CODE_E bes_ble_gatt_cli_read(uint8_t gatt_user_id, bes_gatt_cli_read_t *param);
BES_GATT_ERR_CODE_E bes_ble_gatt_cli_read_by_uuid(uint8_t gatt_user_id, bes_gatt_cli_read_by_uuid_t *param);
BES_GATT_ERR_CODE_E bes_ble_gatt_cli_read_multiple(uint8_t gatt_user_id, bes_gatt_cli_read_multiple_t *param);
BES_GATT_ERR_CODE_E bes_ble_gatt_cli_write_reliable(uint8_t gatt_user_id, bes_gatt_cli_write_reliable_t *param);
BES_GATT_ERR_CODE_E bes_ble_gatt_cli_write(uint8_t gatt_user_id, bes_gatt_cli_write_t *param);
BES_GATT_ERR_CODE_E bes_ble_gatt_cli_write_exe(uint8_t gatt_user_id, uint8_t conidx, uint8_t execute);
BES_GATT_ERR_CODE_E bes_ble_gatt_cli_receive_ctrl(uint8_t gatt_user_id, bes_gatt_cli_rec_ctrl_t *param);
BES_GATT_ERR_CODE_E bes_ble_gatt_cli_mtu_update(uint8_t gatt_user_id, uint8_t conidx);
BES_GATT_ERR_CODE_E bes_ble_gatt_cli_get_val_resp(bes_gatt_cli_get_val_resp_t *param);
BES_GATT_ERR_CODE_E bes_ble_gatt_cli_event_resp(bes_gatt_cli_event_resp_t *param);
void bes_ble_gatt_cli_create_bearer(uint8_t conidx);
void bes_ble_gatt_cli_disconnect_bearer(uint8_t conidx);

#ifdef __cplusplus
}
#endif
#endif
#endif /* __BES_GATT_API_H__ */
