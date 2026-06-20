/**
 ****************************************************************************************
 *
 * @file app_bap_uc_cli_msg.h
 *
 * @brief BLE Audio Audio Stream Control Service Client
 *
 * Copyright 2015-2019 BES.
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
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP_BAP
 * @{
 ****************************************************************************************
 */

#ifndef APP_BAP_UC_CLI_MSG_H_
#define APP_BAP_UC_CLI_MSG_H_
#if BLE_AUDIO_ENABLED
#include "app_bap.h"

#define APP_BAP_DFT_ASCC_NB_SINK_ASE              (2)
#define APP_BAP_DFT_ASCC_NB_SRC_ASE               (2)

#define APP_BAP_DFT_ASCC_NB_ASE_CFG               (APP_BAP_DFT_ASCC_NB_SINK_ASE + APP_BAP_DFT_ASCC_NB_SRC_ASE)
#define APP_BAP_DFT_ASCC_TOTAL_NB_ASE_CFG         (APP_BAP_DFT_ASCC_NB_ASE_CFG * BLE_AUDIO_CONNECTION_CNT)
#define APP_BAP_DFT_ASCC_SUPP_PHY_BF              APP_PHY_ALL
#define APP_BAP_DFT_ASCC_PRFE_PHY                 APP_PHY_2MBPS_VALUE
#define APP_BAP_DFT_ASCC_DP_ID                    0
#define APP_BAP_DFT_ASCC_CTL_DELAY_US             0x0102
#define APP_BAP_DFT_ASCC_PACKING_TYPE             APP_ISO_PACKING_SEQUENTIAL
#define APP_BAP_DFT_ASCC_FRAMING_TYPE             APP_ISO_UNFRAMED_MODE
#define APP_BAP_DFT_ASCC_SCA                      4
/// we prepare two cis in cig for rejoin situation
#define APP_BAP_DFT_ASCC_CIS_NUM                  2
#define APP_BAP_DFT_ASCC_SDU_INTERVAL_US          10000

typedef enum
{
    APP_BAP_UC_CLI_DISCOVER,
    APP_BAP_UC_CLI_CONFIGURE_CODEC,
    APP_BAP_UC_CLI_CONFIGURE_QOS,
    APP_BAP_UC_CLI_ENABLE,
    APP_BAP_UC_CLI_UPDATE_METADATA,
    APP_BAP_UC_CLI_DISABLE,
    APP_BAP_UC_CLI_RELEASE,
    APP_BAP_UC_CLI_GET_QUALITY,
    APP_BAP_UC_CLI_VS_SET_TRIGGER,
    APP_BAP_UC_CLI_GET_CFG,
    APP_BAP_UC_CLI_SET_CFG,
    APP_BAP_UC_CLI_GET_STATE,
} app_bap_uc_cli_cmd_code_t; // see @bap_uc_cli_cmd_code;

/// QoS Configuration structure (short)
typedef struct app_bap_uc_cli_qos_cfg
{
    /// PHY
    uint8_t phy;
    /// Maximum number of retransmissions for each CIS Data PDU
    /// From 0 to 15
    uint8_t retx_nb;
    /// Maximum SDU size
    /// From 0 to 4095 bytes (0xFFF)
    uint16_t max_sdu_size;
    /// Presentation delay in microseconds
    uint32_t pres_delay_us;
} app_bap_uc_cli_qos_cfg_t;

/// CIG params structure
typedef struct app_bap_uc_cli_grp_param
{
    /// SDU interval from Master to Slave in microseconds
    /// From 0xFF (255us) to 0xFFFF (1.048575s)
    uint32_t sdu_intv_m2s_us;
    /// SDU interval from Slave to Master in microseconds
    /// From 0xFF (255us) to 0xFFFF (1.048575s)
    uint32_t sdu_intv_s2m_us;
    /// Maximum time (in milliseconds) for an SDU to be transported from master controller to slave
    /// controller. From 0x5 (5ms) to 0xFA0 (4s)
    uint16_t tlatency_m2s_ms;
    /// Maximum time (in milliseconds) for an SDU to be transported from slave controller to master
    /// controller. From 0x5 (5ms) to 0xFA0 (4s)
    uint16_t tlatency_s2m_ms;
    /// Sequential or interleaved scheduling
    uint8_t packing;
    /// Unframed or framed mode
    uint8_t framing;
    /// Worst slow clock accuracy of slaves
    uint8_t sca;
#ifdef BLE_AUDIO_IS_ALWAYS_USE_TEST_MODE_CIG_BIG_CREATING
    /// Flush timeout in milliseconds for each payload sent from Master to Slave
    uint16_t ft_m2s_ms;
    /// Flush timeout in milliseconds for each payload sent from Slave to Master
    uint16_t ft_s2m_ms;
    /// ISO interval in milliseconds
    uint16_t iso_intv_ms;
    /// cis numbles
    uint16_t cis_num;
#endif
} app_bap_uc_cli_grp_param_t;

/// ASCC ASE Information structure
typedef struct app_bap_ascc_ase
{
    /// ASE local index
    uint8_t ase_lid;
    /// ASE State
    uint8_t ase_state;
    /// Connection local index
    uint8_t con_lid;
    /// ASE instance index
    uint8_t ase_instance_idx;
    /// ASE Direction
    enum app_gaf_direction direction;
    /// Codec ID
    app_gaf_codec_id_t codec_id;
    /// Pointer to Codec Configuration
    app_gaf_bap_cfg_t *p_cfg;
    /// QoS configuration
    app_bap_uc_cli_qos_cfg_t qos_cfg;
    /// Pointer to Metadata structure
    app_gaf_bap_cfg_metadata_t *p_metadata;
    /// CIS index
    uint8_t cis_id;
    /// CIS Connection Handle
    uint16_t cis_hdl;
    /// current audio stream type, @see gaf_bap_context_type_bf
    //uint16_t context_bf;
    /// CIG sync delay in us
    uint32_t cig_sync_delay;
    /// CIS sync delay in us
    uint32_t cis_sync_delay;
    /// iso interval in us
    uint32_t iso_interval_us;
    /// bn count from master to slave
    uint32_t bn_m2s;
    /// bn count from slave to master
    uint32_t bn_s2m;
} app_bap_ascc_ase_t;

/// ASCC CIG Information structure
typedef struct app_bap_ascc_grp
{
    /// CIG local index
    uint8_t cig_grp_lid;
    /// Group parameters
    app_bap_uc_cli_grp_param_t grp_params;
} app_bap_ascc_grp_t;

/// Audio Stream Control Service Client environment structure
typedef struct app_bap_ascc_env
{
    /// Preferred Mtu
    uint8_t preferred_mtu;
    /// Number of ASE configurations that can be maintained
    uint8_t nb_ases_cfg;
    /// Supported PHY Bitfield
    uint8_t phy_bf;
    /// Preferred PHY
    uint8_t phy;
    /// Group Information
    app_bap_ascc_grp_t grp_info;
    /// ASE Information
    app_bap_ascc_ase_t *ase_info;
    /// test mode enable flag
    bool tm_enable;
} app_bap_ascc_env_t;

typedef struct
{
    uint16_t op_code;
    uint16_t status;
    uint8_t con_lid;
    uint8_t ase_lid;
} app_bap_ascc_cmd_cmp_t;

/// Structure for response message
typedef struct
{
    bool isCreate;
    /// Status
    uint16_t status;
    /// ASE local index
    uint8_t ase_lid;
    /// CIG local index
    uint8_t cig_grp_lid;
} app_bap_ascc_grp_state_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef AOB_MOBILE_ENABLED
uint8_t app_bap_uc_cli_iso_send_data_to_all_channel(uint8_t **payload, uint16_t payload_len, uint32_t ref_time);
int app_bap_uc_cli_update_metadata(uint8_t ase_lid, app_gaf_bap_cfg_metadata_t *metadata);
int app_bap_uc_cli_set_sdu_intv(uint32_t sdu_intv_m2s_us, uint32_t sdu_intv_s2m_us);
int app_bap_uc_cli_set_cis_num_in_cig(uint8_t cis_num);
int app_bap_uc_cli_increase_cis_num_in_cig(uint8_t step);

int app_bap_uc_cli_discovery_start(uint8_t con_lid);
int app_bap_uc_cli_stream_establish_stm(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
#endif // APP_BAP_UC_CLI_MSG_H_

/// @} APP_BAP
