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

#ifndef _PRF_TYPES_INC_
#define _PRF_TYPES_INC_

/**
 ****************************************************************************************
 * @addtogroup PROFILE_TYPES_API Enumerations and Structures
 * @ingroup PROFILE_API
 * @brief Definitions of shared profiles types
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <stdint.h>
#include <stdbool.h>

/*
 * DEFINES
 ****************************************************************************************
 */

/// Characteristic Presentation Format Descriptor Size
#define PRF_CHAR_PRES_FMT_SIZE          (7)
/// Client Characteristic Configuration descriptor value length
#define PRF_CCC_DESC_LEN                (2)
/// Minimum number of attributes for a characteristic (description + value attributes)
#define PRF_CHAR_NB_ATT_MIN             (2)
/// Offset of Characteristic Value attribute in a local characteristic
#define PRF_ATT_OFFSET_CHAR_VAL         (1)
/// Offset of first descriptor in a local characteristic
#define PRF_ATT_OFFSET_FIRST_DESC       (2)

/*
 * MACROS
 ****************************************************************************************
 */

/// Provide attribute requirement value
#define ATT_REQ(req_type, val) \
            ((PRF_ATT_REQ_ ## req_type ## _ ## val) << PRF_ATT_REQ_ ## req_type ## _LSB)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Attribute Instantiation requirement values
enum prf_att_req_inst
{
    /// Mono-instantiated
    PRF_ATT_REQ_INST_MONO = 0,
    /// Multi-instantiated
    PRF_ATT_REQ_INST_MULTI,
};

/// Attribute presence requirement values
enum prf_att_req_pres
{
    /// Attribute is optional
    PRF_ATT_REQ_PRES_OPT = 0,
    /// Attribute is mandatory
    PRF_ATT_REQ_PRES_MAND,
};

/// Attribute requirements bit field meaning
/// @verbatim
/// +-----+----------+----------+
/// | 7-2 |    1     |     0    |
/// +-----+----------+----------+
/// | RFU | INSTANCE | PRESENCE |
/// +-----+----------+----------+
/// @endverbatim
enum prf_att_req_bf
{
    /// Indicate if presence of attribute is optional or mandatory
    PRF_ATT_REQ_PRES_LSB = 0,
    PRF_ATT_REQ_PRES_MASK = 0x01,

    /// Indicate if attribute is mono-instantiated or multi-instantiated
    PRF_ATT_REQ_INST_LSB = 1,
    PRF_ATT_REQ_INST_MASK = 0x02,
};

/// Possible values for setting client configuration characteristics
enum prf_cli_conf
{
    /// Stop notification/indication
    PRF_CLI_STOP_NTFIND = 0x0000,
    /// Start notification
    PRF_CLI_START_NTF   = 0x0001,
    /// Start indication
    PRF_CLI_START_IND   = 0x0002,
};

/// Policy for sending of events
enum prf_evt_policy
{
    /// Send an event to all Client devices that have registered for reception of events
    PRF_EVT_POLICY_ALL = 0,
    /// Send an event to a single Client device
    PRF_EVT_POLICY_SINGLE,
    /// Send an event to all Client devices that hae registered for reception of events
    /// except one
    PRF_EVT_POLICY_ALL_BUT,
    /// Send an event to a set of devices
    PRF_EVT_POLICY_LIST,
};

/// Possible values for setting server configuration characteristics
enum prf_srv_conf
{
    /// Stop Broadcast
    PRF_SRV_STOP_BCST  = 0x0000,
    /// Start Broadcast
    PRF_SRV_START_BCST = 0x0001,
};

/// Connection type
enum prf_con_type
{
    /// Discovery type connection
    PRF_CON_DISCOVERY = 0x00,
    /// Normal type connection
    PRF_CON_NORMAL    = 0x01,
};

/// Service type
enum prf_svc_type
{
    /// Primary service
    PRF_PRIMARY_SERVICE   = 0x00,
    /// Secondary service
    PRF_SECONDARY_SERVICE = 0x01
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/**
 * Characteristic Presentation Format Descriptor structure
 * Packed size is PRF_CHAR_PRES_FMT_SIZE
 */
typedef struct prf_char_pres_fmt
{
    /// Unit (The Unit is a UUID)
    uint16_t unit;
    /// Description
    uint16_t description;
    /// Format
    uint8_t  format;
    /// Exponent
    uint8_t  exponent;
    /// Name space
    uint8_t  name_space;
} prf_char_pres_fmt_t;

/**
 * date and time structure
 * size = 7 bytes
 */
/// Time profile information
typedef struct prf_date_time
{
    /// year time element
    uint16_t year;
    /// month time element
    uint8_t  month;
    /// day time element
    uint8_t  day;
    /// hour time element
    uint8_t  hour;
    /// minute time element
    uint8_t  min;
    /// second time element
    uint8_t  sec;
} prf_date_time_t;

/// Date profile information - 4 bytes
typedef struct prf_date
{
    /// year date element
    uint16_t year;
    /// month date element
    uint8_t month;
    /// day date element
    uint8_t day;
} prf_date_t;

/**
 *  SFLOAT: Short Floating Point Type
 *
 * @verbatim
 *        +----------+----------+---------+
 *        | Exponent | Mantissa |  Total  |
 * +------+----------+----------+---------+
 * | size |  4 bits  | 12 bits  | 16 bits |
 * +------+----------+----------+---------+
 * @endverbatim
 */
typedef uint16_t prf_sfloat;

/// utf8_s string
typedef struct prf_utf_8
{
    /// value length
    uint16_t length;
    /// Value string in UTF8 format
    uint8_t  str[0];
} prf_utf_8_t;

/// Service information structure
typedef struct prf_svc
{
    /// start handle
    uint16_t shdl;
    /// end handle
    uint16_t ehdl;
} prf_svc_t;

/// Included Service information structure
typedef struct prf_incl_svc
{
    /// attribute handle
    uint16_t handle;
    /// included service start handle
    uint16_t start_hdl;
    /// included service  end handle
    uint16_t end_hdl;
    /// UUID length
    uint8_t uuid_len;
    /// UUID
    uint8_t uuid[16];
} prf_incl_svc_t;

/// Characteristic information structure
typedef struct prf_char
{
    /// Value handle
    uint16_t val_hdl;
    /// Characteristic properties
    uint8_t prop;
} prf_char_t;

/// Descriptor information structure
typedef struct prf_desc
{
    /// Descriptor handle
    uint16_t desc_hdl;
} prf_desc_t;

/// Characteristic definition
typedef struct prf_char_def
{
    /// Characteristic UUID
    uint16_t uuid;
    /// Requirement bit field
    uint8_t req_bf;
    /// Mandatory Properties
    uint8_t prop_mand;
} prf_char_def_t;

/// 128-bit UUID Characteristic definition
typedef struct prf_char128_def
{
    /// 128-bit Characteristic UUID
    uint8_t uuid[16];
    /// Requirement bit field
    uint8_t req_bf;
    /// Mandatory Properties
    uint8_t prop_mand;
} prf_char128_def_t;

/// Characteristic Descriptor definition
typedef struct prf_desc_def
{
    /// Characteristic Descriptor uuid
    uint16_t uuid;
    /// Requirement bit field
    uint8_t req_bf;
    /// Corresponding characteristic code
    uint8_t char_code;
} prf_desc_def_t;

/*
 * CALLBACK FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Callback function to call when an included service attribute is found
 *
 * @param[in] con_lid   Connection local index
 * @param[in] uuid      Service UUID
 * @param[in] shdl      Service start handle
 * @param[in] ehdl      Service end handle
 ****************************************************************************************
 */
typedef void (*prf_inc_svc_cb)(uint8_t con_lid, uint16_t uuid, uint16_t shdl, uint16_t ehdl);

/**
 ****************************************************************************************
 * @brief Callback function to call when a multi-instantiated attribute is found
 *
 * @param[in] con_lid   Connection local index
 * @param[in] hdl       Characteristic value handle or Descriptor handle
 * @param[in] desc      Indicate if found attribute is a descriptor (True) or a characteristic
 * @param[in] idx       Index
 * @param[in] prop      Characteristic properties
 ****************************************************************************************
 */
typedef void (*prf_multi_att_cb)(uint8_t con_lid, uint16_t hdl, bool desc, uint8_t idx,
                                 uint8_t prop);

/// @} PROFILE_TYPES_API

#endif /* _PRF_TYPES_INC_ */
