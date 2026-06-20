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
#ifndef __BAP_ASC_COMMON__
#define __BAP_ASC_COMMON__

#include "bluetooth.h"

#include "gaf_cfg.h"
#include "generic_audio.h"

/// Maximum Presentation Delay for Unicast (in microseconds)
#define ASCS_MAX_PRES_DELAY_US               (0x00FFFFFF)

#define ASCS_ASE_ID_MIN                      (1)

#define ASCS_ASE_ID_MAX                      (16)

#define ASCS_INVALID_ASE_INDEX               (GAF_INVALID_ANY_LID)

#define ASCS_INVALID_GRP_LID                 (GAF_INVALID_ANY_LID)
#define ASCS_INVALID_STREAM_LID              (GAF_INVALID_ANY_LID)
#define ASCS_INVALID_CIS_HDL                 (GAF_INVALID_CON_HDL)

#define ASCS_ASE_UUID_TO_DIRECTION(ase_uuid) (ase_uuid == GATT_CHAR_UUID_SINK_ASE ? BAP_DIRECTION_SINK :\
                                                (ase_uuid == GATT_CHAR_UUID_SOURCE_ASE ? BAP_DIRECTION_SRC : BAP_DIRECTION_MAX))

#define ASCS_ASE_CP_WR_BUF_LEN_MIN           (sizeof(ascs_ase_cp_t) + sizeof(uint8_t))
/*************************ASE STATUS PACKED*********************************/
typedef struct ascs_ase_status
{
    uint8_t  ase_id;
    uint8_t  state;
    uint8_t  params[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ascs_ase_status_t;

typedef struct ascs_codec_config_ltv_packed
{
    uint8_t len;
    uint8_t type;
    uint8_t data[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ascs_codec_cfg_ltv_t;

/// ASE_State = 0x01 (Codec Configured), defined in Table 4.3.
typedef struct ascs_ase_status_config
{
    uint8_t  framing;
    uint8_t  phy;
    uint8_t  rtn;
    uint16_t latency;
    uint8_t  pd_min[3];
    uint8_t  pd_max[3];
    uint8_t  prefer_pd_min[3];
    uint8_t  prefer_pd_max[3];
    uint8_t  codec_id[GEN_AUD_CODEC_ID_LEN];
    uint8_t  codec_cfg_len;
    /// LTV-formatted Codec-Specific Configuration
    ascs_codec_cfg_ltv_t codec_cfg[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ascs_ase_status_codec_cfg_t;

/// ASE_State = 0x02 (QoS Configured), defined in Table 4.4.
typedef struct ascs_ase_status_qos
{
    uint8_t  cig_id;
    uint8_t  cis_id;
    uint8_t  interval[3];
    uint8_t  framing;
    uint8_t  phy;
    uint16_t sdu;
    uint8_t  rtn;
    uint16_t latency;
    uint8_t  pd[3];
} __attribute__((packed)) ascs_ase_status_qos_cfg_t;

typedef struct ascs_ase_status_metadata
{
    uint8_t  metadata_len;
    /// LTV-formatted Metadata
    uint8_t  metadata[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ascs_metadata_t;

/// ASE_Status = 0x03 (Enabling) defined in Table 4.5.

typedef struct ascs_ase_status_enabling
{
    uint8_t  cig_id;
    uint8_t  cis_id;
    ascs_metadata_t metadata;
} __attribute__((packed)) ascs_ase_status_enabling_t;

/// ASE_Status =  0x04 (Streaming) defined in Table 4.5.

typedef struct ascs_ase_status_streaming
{
    uint8_t  cig_id;
    uint8_t  cis_id;
    ascs_metadata_t metadata;
} __attribute__((packed)) ascs_ase_status_streaming_t;

/// ASE_Status = 0x05 (Disabling) as defined in Table 4.5.

typedef struct ascs_ase_status_disabling
{
    uint8_t  cig_id;
    uint8_t  cis_id;
    ascs_metadata_t metadata;
} __attribute__((packed)) ascs_ase_status_disabling_t;

/***************************ASE CP OPCODE********************************/
/// ASE Control Point Protocol
typedef struct ascs_ase_cp
{
    /// Request/Notification opcode
    uint8_t  op;
    uint8_t  pdu[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ascs_ase_cp_t;

typedef struct ascs_codec_config_packed
{
    /// ASE ID
    uint8_t  ase_id;
    /// Target latency
    uint8_t  tgt_latency;
    /// Target PHY
    uint8_t  tgt_phy;
    /// Codec ID
    uint8_t codec_id[GEN_AUD_CODEC_ID_LEN];
    /// Codec Specific Config Length
    uint8_t  codec_cfg_len;
    /// LTV-formatted Codec-Specific Configuration
    ascs_codec_cfg_ltv_t codec_cfg[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ascs_codec_cfg_p_t;

typedef struct ascs_op_config_op
{
    /// Number of ASEs
    uint8_t  num_ases;
    /// Config Parameters
    ascs_codec_cfg_p_t cfg[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ascs_op_codec_cfg_t;

typedef struct ascs_op_qos_cfg_packed
{
    /// ASE ID
    uint8_t  ase_id;
    /// CIG ID
    uint8_t  cig;
    /// CIG ID
    uint8_t  cis;
    /// SDU interval
    uint8_t  interval[3];
    /// Frame framing
    uint8_t  framing;
    /// PHY
    uint8_t  phy;
    /// Maximum SDU Size
    uint16_t sdu;
    /// Retransmission Effort
    uint8_t  rtn;
    /// Transport Latency
    uint16_t latency;
    /// Presentation Delay
    uint8_t  pd[3];
} __attribute__((packed)) ascs_qos_cfg_p_t;

typedef struct ascs_op_qos_cfg
{
    /// Number of ASEs
    uint8_t  num_ases;
    /// QoS Parameters
    ascs_qos_cfg_p_t qos_cfg[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ascs_op_qos_cfg_t;

typedef struct ascs_metadata_packed
{
    /// ASE ID
    uint8_t  ase_id;
    /// Metadata length
    uint8_t  len;
    /// LTV-formatted Metadata
    uint8_t  data[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ascs_metadata_p_t;

typedef struct ascs_op_enable
{
    /// Number of ASEs
    uint8_t  num_ases;
    /// Metadata
    ascs_metadata_p_t metadata[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ascs_op_enable_t;

typedef struct ascs_op_metadata_upd
{
    /// Number of ASEs
    uint8_t  num_ases;
    /// Metadata
    ascs_metadata_p_t metadata[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ascs_op_upd_md_t;

typedef struct ascs_op_rx_start
{
    /// Number of ASEs
    uint8_t  num_ases;
    /// ASE IDs
    uint8_t  ase_id[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ascs_op_rx_start_t;

typedef struct ascs_op_disable
{
    /// Number of ASEs
    uint8_t  num_ases;
    /// ASE IDs
    uint8_t  ase_id[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ascs_op_disable_t;

typedef struct ascs_op_rx_stop
{
    /// Number of ASEs
    uint8_t  num_ases;
    /// ASE IDs
    uint8_t  ase_id[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ascs_op_rx_stop_t;

typedef struct ascs_op_release
{
    /// Number of ASEs
    uint8_t  num_ases;
    /// Ase IDs
    uint8_t  ase_id[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ascs_op_release_t;

typedef struct ascs_cp_ase_rsp_packed
{
    /// ASE ID
    uint8_t  ase_id;
    /// Response code
    uint8_t  rsp_code;
    /// Response reason
    uint8_t  reason;
} __attribute__((packed)) ase_cp_ase_rsp_p_t;

typedef struct ascs_ase_cp_rsp
{
    /// Opcode
    uint8_t  op;
    /// Number of ASEs
    uint8_t  num_ase;
    /// ASE response
    ase_cp_ase_rsp_p_t ase_rsp[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ascs_ase_cp_rsp_t;

typedef struct ascs_op_ase_metadata
{
    /// Number of ASEs
    uint8_t  num_ases;
    /// Metadata
    ascs_metadata_p_t metadata[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ascs_op_ase_md_t;

typedef struct ascs_op_ase_id
{
    /// Number of ASEs
    uint8_t  num_ases;
    /// ASE IDs
    uint8_t  ase_id[GAF_ARRAY_EMPTY];
} __attribute__((packed)) ascs_op_ase_id_t;

/// ASCS character type
enum ascs_char
{
    /// ASE Control Point characteristic
    ASCS_CHAR_TYPE_CP = 0,
    /// Sink ASE characteristic
    ASCS_CHAR_TYPE_SINK_ASE,
    /// Src ASE characteristic
    ASCS_CHAR_TYPE_SRC_ASE,
    /// ASCS char type max
    ASCS_CHAR_TYPE_MAX,
};

/// Operation code values for ASE Control Point characteristic
enum ascs_opcode
{
    ASCS_OPCODE_MIN = 0,
    /// Configure Codec
    ASCS_OPCODE_CFG_CODEC,
    /// Configure QoS
    ASCS_OPCODE_CFG_QOS,
    /// Enable
    ASCS_OPCODE_ENABLE,
    /// Receiver Start Ready
    ASCS_OPCODE_RX_START_READY,
    /// Disable
    ASCS_OPCODE_DISABLE,
    /// Receiver Stop Ready
    ASCS_OPCODE_RX_STOP_READY,
    /// Update Metadata
    ASCS_OPCODE_UPDATE_METADATA,
    /// Release
    ASCS_OPCODE_RELEASE,

    ASCS_OPCODE_MAX,
};

/// ASE State values
enum ascs_ase_state
{
    /// Idle
    ASCS_ASE_STATE_IDLE = 0,
    /// Codec configured
    ASCS_ASE_STATE_CODEC_CONFIGURED,
    /// QoS configured
    ASCS_ASE_STATE_QOS_CONFIGURED,
    /// Enabling
    ASCS_ASE_STATE_ENABLING,
    /// Streaming
    ASCS_ASE_STATE_STREAMING,
    /// Disabling
    ASCS_ASE_STATE_DISABLING,
    /// Releasing
    ASCS_ASE_STATE_RELEASING,

    ASCS_ASE_STATE_MAX,
};

/// Response Code values for ASE Control Point characteristic
enum ascs_cp_rsp_code
{
    /// Success
    ASCS_CP_RSP_CODE_SUCCESS = 0,
    /// Unsupported Opcode
    ASCS_CP_RSP_CODE_UNSUPPORTED_OPCODE,
    /// Invalid Length
    ASCS_CP_RSP_CODE_INVALID_LENGTH,
    /// Invalid ASE ID
    ASCS_CP_RSP_CODE_INVALID_ASE_ID,
    /// Invalid ASE State Machine Transition
    ASCS_CP_RSP_CODE_INVALID_TRANSITION,
    /// Invalid ASE direction
    ASCS_CP_RSP_CODE_INVALID_ASE_DIRECTION,
    /// Unsupported Audio Capabilities
    ASCS_CP_RSP_CODE_UNSUPPORTED_AUDIO_CAPA,
    /// Unsupported Configuration Parameter value
    ASCS_CP_RSP_CODE_UNSUPPORTED_CFG_PARAM,
    /// Rejected Configuration Parameter value
    ASCS_CP_RSP_CODE_REJECTED_CFG_PARAM,
    /// Invalid Configuration Parameter value
    ASCS_CP_RSP_CODE_INVALID_CFG_PARAM,
    /// Unsupported Metadata
    ASCS_CP_RSP_CODE_UNSUPPORTED_METADATA,
    /// Rejected Metadata
    ASCS_CP_RSP_CODE_REJECTED_METADATA,
    /// Invalid Metadata
    ASCS_CP_RSP_CODE_INVALID_METADATA,
    /// Insufficient Resources
    ASCS_CP_RSP_CODE_INSUFFICIENT_RESOURCES,
    /// Unspecified Error
    ASCS_CP_RSP_CODE_UNSPECIFIED_ERROR,

    ASCS_CP_RSP_CODE_MAX,
};

/// Reason values for ASE Control Point characteristic
enum ascs_cp_reason
{
    ASCS_CP_REASON_MIN = 1,
    /// Codec ID
    ASCS_CP_REASON_CODEC_ID = ASCS_CP_REASON_MIN,
    /// Codec Specific Configuration
    ASCS_CP_REASON_CODEC_SPEC_CFG,
    /// SDU Interval
    ASCS_CP_REASON_SDU_INTERVAL,
    /// Framing
    ASCS_CP_REASON_FRAMING,
    /// PHY
    ASCS_CP_REASON_PHY,
    /// Maximum SDU Size
    ASCS_CP_REASON_MAX_SDU_SIZE,
    /// Retransmission Number
    ASCS_CP_REASON_RETX_NB,
    /// Maximum Transport Latency
    ASCS_CP_REASON_MAX_TRANS_LATENCY,
    /// Presentation Delay
    ASCS_CP_REASON_PRES_DELAY,
    /// Invalid ASE CIS Mapping
    ASCS_CP_REASON_INVALID_ASE_CIS_MAPPING,
    /// APP layer rejected
    ASCS_CP_REASON_APP_REJECTED,
    /// OPCODE not match
    ASCS_CP_REASON_OPCODE_NOT_MATCH,
    /// Parameter check failed
    ASCS_CP_REASON_PARAM_ERR,
    /// ASE lid not match
    ASCS_CP_REASON_ASE_LID_NOT_MATCH,
    /// ASE lid invalid
    ASCS_CP_REASON_ASE_LID_INTVALID,
    /// Additional info error
    ASCS_CP_REASON_ADD_INFO_ERR,
    /// ASE ENV invalid
    ASCS_CP_REASON_ASE_ENV_INVALID,

    ASCS_CP_REASON_MAX,
};

/// Target Latency values
enum ascs_tgt_latency
{
    ASCS_TGT_LATENCY_MIN = 1,

    /// Target lower latency
    ASCS_TGT_LATENCY_LOWER = ASCS_TGT_LATENCY_MIN,
    /// Target balanced latency and reliability
    ASCS_TGT_LATENCY_BALENCED,
    /// Target higher reliability
    ASCS_TGT_LATENCY_RELIABLE,

    ASCS_TGT_LATENCY_MAX,
};

/**
 * @brief ASCS ASE state transition check
 *
 * @param  opcode      Operation code @see enum ascs_opcode
 * @param  direction   BAP direction @see enum bap_direction
 * @param  old_state   Old ASE state @see enum ascs_ase_state
 * @param  new_state   New ASE state @see enum ascs_ase_state
 *
 * @return true        Check success
 * @return false       Check failed
 */
bool asc_common_ase_state_changes_check(uint8 opcode, uint8_t direction, uint8_t old_state, uint8_t new_state);
#endif /// __BAP_ASC_COMMON__
