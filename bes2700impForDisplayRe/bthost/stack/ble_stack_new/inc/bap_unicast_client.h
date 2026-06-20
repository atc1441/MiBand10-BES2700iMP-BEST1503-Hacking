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
#ifndef __BAP_UC_CLI_INC__
#define __BAP_UC_CLI_INC__
#include "bluetooth.h"

#include "gaf_cfg.h"
#include "gaf_log.h"
#include "generic_audio.h"

#include "gaf_prf.h"

#include "pacc.h"
#include "asc_common.h"

#if (BAP_ASCC_ENABLE)

#define BAP_UC_CLI_CONNECTION_MAX                     (GAF_CONNECTION_MAX)

#define BAP_UC_CLI_ASCC_WR_CP_TIMEOUT_MS              (gaf_prf_get_rx_ntf_ind_timeout_in_ms())

#define BAP_UC_CLI_MAX_SUPP_ASE_NUM_PER_CONN          (0b1111)

#define BAP_UC_CLI_MAX_SUPP_CIS_NUM                   (GAF_SUPP_MAX_CIS_NUM)

#define BAP_UC_CLI_MAX_CIS_NUM_PREPARE_IN_GRP         (BAP_UC_CLI_MAX_SUPP_CIS_NUM)

/// CIS EVENT
enum bap_uc_cli_cis_event
{
    // CIS connected
    BAP_UC_CLI_CIS_EVENT_CONNECTED = 0,
    // CIS establish fail
    BAP_UC_CLI_CIS_EVENT_FAILED,
    // CIS disconnected
    BAP_UC_CLI_CIS_EVENT_DISCONNECTED,
};

/// CIS EVENT
enum cig_event
{
    // CIG created
    BAP_UC_CLI_CIG_EVENT_CREATED = 0,
    // CIS removed
    BAP_UC_CLI_CIG_EVENT_REMOVED,
};
#endif /// (BAP_ASCC_ENABLE)

#if (BAP_PACC_ENABLE)
/*BAP ROLE STRUCT REDEFINE*/
typedef pacc_prf_svc_t bap_uc_cli_pacc_svc_info_t;
typedef pacc_evt_cb_t bap_uc_cli_pacc_evt_cb_t;
#endif /// (BAP_PACC_ENABLE)

#if (BAP_ASCC_ENABLE)
typedef struct bap_uc_cli_prf_svc_info bap_uc_cli_ascc_prf_svc_t;
typedef struct bap_uc_cli_ase_qos_cfg_param bap_uc_cli_ase_qos_cfg_t;
typedef struct bap_uc_cli_ase_qos_req_param bap_uc_cli_ase_qos_req_t;

/// Callback for ascs discovery cmp
typedef void (*bap_uc_cli_cb_discovery_cmp)(uint8_t con_lid, uint16_t err_code);
/// Callback for ascs bond data
typedef void (*bap_uc_cli_cb_bond_data)(uint8_t con_lid, const bap_uc_cli_ascc_prf_svc_t *p_svc_info);
/// Callback for ascc gatt set cfg cmp evt
typedef void (*bap_uc_cli_cb_set_cfg_cmp_evt)(uint8_t con_lid, uint8_t char_type, uint16_t err_code);
/// Callback for ascs ase state err occure
typedef void (*bap_uc_cli_cb_ase_err_state_ind)(uint8_t con_lid, uint8_t ase_id, uint8_t ase_state,
                                                uint16_t err_code);
/// Callback for ascs ase idle
typedef void (*bap_uc_cli_cb_ase_idle_state_ind)(uint8_t con_lid, uint8_t ase_id, uint8_t direction);
/// Callback for ascs ase codec configured
typedef void (*bap_uc_cli_cb_ase_codec_cfg_state_ind)(uint8_t con_lid, uint8_t ase_id,
                                                      const uint8_t *codec_id, const gen_aud_cc_ptr_t *p_codec_cfg, const bap_uc_cli_ase_qos_req_t *p_qos_req);
/// Callback for ascs ase qos configured
typedef void (*bap_uc_cli_cb_ase_qos_cfg_state_ind)(uint8_t con_lid, uint8_t ase_id, uint8_t cig_id,
                                                    uint8_t cis_id, const bap_uc_cli_ase_qos_cfg_t *p_qos_cfg);
/// Callback for ascs enabling/disabling/streaming/releasing
typedef void (*bap_uc_cli_cb_ase_state_metadata_ind)(uint8_t con_lid, uint8_t ase_id, uint8_t ase_state,
                                                     const gen_aud_metadata_t *p_metadata);
/// Callback for ascs operation cmp evt
typedef void (*bap_uc_cli_cb_ase_cp_op_cmp_evt)(uint8_t con_lid, uint8_t op_code, uint8_t ase_id,
                                                uint16_t err_code, uint8_t cp_rsp_code, uint8_t reason);
/// Callback for ascs set cig parameters operation cmp evt
typedef void (*bap_uc_cli_cb_set_cig_status_evt)(uint8_t cig_evt, uint8_t grp_lid, uint8_t cig_id,
                                                 uint16_t err_code);
/// Callback for ascs iso dp status evt
typedef void (*bap_uc_cli_cb_iso_dp_status_evt)(bool is_setup, uint8_t con_lid, uint8_t ase_id,
                                                uint16_t err_code);
/// Callback for ascs cis status evt
typedef void (*bap_uc_cli_cb_cis_status_evt)(enum bap_uc_cli_cis_event cis_evt, uint8_t cig_id, uint8_t cis_id,
                                             uint16_t cis_hdl, const bap_cis_timing_t *p_cis_timing, uint16_t err_code);
/// Callback function called when prf status event generated
typedef gatt_prf_status_event_callback bap_uc_cli_cb_prf_status_evt;

typedef struct bap_uc_cli_evt_callback
{
    /*ASCC GATT callback*/
    bap_uc_cli_cb_discovery_cmp cb_discovery_cmp;
    bap_uc_cli_cb_bond_data cb_bond_data;
    bap_uc_cli_cb_set_cfg_cmp_evt cb_set_cfg_cmp;
    /*ASE err callback*/
    bap_uc_cli_cb_ase_err_state_ind cb_ase_state_err;
    /*ASE CP op cmp callback*/
    bap_uc_cli_cb_ase_cp_op_cmp_evt cb_ase_cp_op_cmp;
    /*ASE state callback*/
    bap_uc_cli_cb_ase_idle_state_ind cb_ase_idle;
    bap_uc_cli_cb_ase_codec_cfg_state_ind cb_ase_codec_cfg;
    bap_uc_cli_cb_ase_qos_cfg_state_ind cb_ase_qos_cfg;
    bap_uc_cli_cb_ase_state_metadata_ind cb_ase_metadata;
    /*ISO callback*/
    bap_uc_cli_cb_set_cig_status_evt cb_cig_status;
    bap_uc_cli_cb_cis_status_evt cb_cis_status;
    bap_uc_cli_cb_iso_dp_status_evt cb_iso_dp_status;
    /// Callback function called when prf status event generated
    bap_uc_cli_cb_prf_status_evt cb_prf_status_event;
} bap_uc_cli_ascc_evt_cb_t;

struct bap_uc_cli_ase_qos_req_param
{
    uint8_t  framing;
    uint8_t  pref_phy;
    uint8_t  pref_rtn;
    uint16_t max_trans_latency_ms;
    uint32_t pres_delay_min_us;
    uint32_t pres_delay_max_us;
    uint32_t prefer_pd_min_us;
    uint32_t prefer_pd_max_us;
};

struct bap_uc_cli_ase_qos_cfg_param
{
    /// SDU interval
    uint32_t sdu_interval;
    /// QoS Framing
    uint8_t framing;
    /// QoS PHY
    uint8_t phy;
    /// QoS Max SDU size*/
    uint16_t max_sdu_size;
    /// QoS Retransmission Number
    uint8_t rtn;
    /// QoS Transport Latency
    uint16_t max_trans_latency;
    /// QoS Presentation Delay
    uint32_t pres_delay;
};

typedef struct bap_uc_cli_create_cig_test_param
{
    /// BN
    uint8_t bn;
    /// NSE
    uint8_t nse;
    /// MAX PDU SIZE
    uint16_t max_pdu_size;
} bap_uc_cli_cig_test_param_t;

typedef struct bap_uc_cli_ase_op_param
{
    /// Connection Local Index
    uint8_t con_lid;
    /// ASE ID
    uint8_t ase_id;
    union
    {
        struct
        {
            /* Codec Target Latency*/
            uint8_t tgt_latency;
            /* Codec Target PHY*/
            uint8_t tgt_phy;
            /* Codec Codec ID*/
            uint8_t codec_id[GEN_AUD_CODEC_ID_LEN];
            /* Codec Specific Parameters*/
            gen_aud_cc_t codec_cfg_param;
        } cfg_codec;
        struct
        {
            /// Group Local ID
            uint8_t grp_lid;
            /// Assign cis id if needed, 0xFF means auto choose
            uint8_t cis_id;
            /// Qos cfg parameters
            bap_uc_cli_ase_qos_cfg_t qos_cfg_param;
            /// Create CIG test param
            bap_uc_cli_cig_test_param_t cig_test_param;
        } cfg_qos;
        struct
        {
            /* Metadata*/
            gen_aud_metadata_t metadata;
        } enable;
        struct
        {
            /* Metadata*/
            gen_aud_metadata_t metadata;
        } update_metadata;
    } u_param;
} bap_uc_cli_ase_op_param_t;

struct bap_uc_cli_prf_svc_info
{
    /// Service handler range
    gatt_prf_svc_range_t svc_range;
    /// Service UUID
    uint16_t uuid;
};

typedef struct bap_uc_cli_ascc_init_cfg
{
    /// Supported ASE number
    uint8_t ase_num_supp;
    /// CP write reliable
    bool cp_write_reliable;
    /// CIS auto disconnect after stream close
    bool auto_cis_disconn;
    /// Preferred MTU size
    uint16_t pref_mtu;
} bap_uc_cli_ascc_init_cfg_t;
#endif /// (BAP_ASCC_ENABLE)

/*FUCNTION DECLARATION*/
#if (BAP_ASCC_ENABLE)
/**
 * @brief BAP unicast client initilization
 *
 * @param  p_init_cfg  Initilization configuration
 * @param  p_cb        Event callbacks
 *
 * @return int         status
 */
int bap_uc_cli_ascc_init(const bap_uc_cli_ascc_init_cfg_t *p_init_cfg, const bap_uc_cli_ascc_evt_cb_t *p_cb);

/**
 * @brief BAP unicast client deinitilization
 *
 * @return int         status
 */
int bap_uc_cli_ascc_deinit(void);

/**
 * @brief BAP unicast client ascs service discovery
 *
 * @param  con_lid     Connection local index
 *
 * @return int         status
 */
int bap_uc_cli_ascs_discovery(uint8_t con_lid);

/**
 * @brief BAP unicast client create unicast stream group with cis prepared
 *
 * @param  grp_lid     Unicast group local index upper specific
 * @param  grp_prepare_cis_num
 *                     Prepare CIS num in group
 * @param  p_cig_info  CIG info to create CIG
 *
 * @return int         status
 */
int bap_uc_cli_create_uc_stream_group(uint8_t grp_lid, uint8_t grp_prepare_cis_num, const bap_cig_param_t *p_cig_info);

/**
 * @brief BAP unicast client delete unicast stream group
 *
 * @param  grp_lid     Group local index upper specific
 *
 * @return int         status
 */
int bap_uc_cli_delete_uc_stream_group(uint8_t grp_lid);

/**
 * @brief BAP unicast client ASE operation
 *
 * @param  op_code     Operation code @see enum ascs_opcode
 * @param  p_ase_param Operation parameters
 *
 * @return int         status
 */
int bap_uc_cli_ase_operation(uint8_t op_code,  const bap_uc_cli_ase_op_param_t *p_ase_param);
#if 0
int bap_uc_cli_disconnect_ase_cis(uint8_t con_lid, uint8_t ase_id);
int bap_uc_cli_update_configuration(bool cp_write_reliable, bool auto_disconn_cis);
#endif
#endif /// (BAP_ASCC_ENABLE)

/*FUCNTION DECLARATION REDEFINE*/
#define bap_uc_cli_pacc_init(p_init_cfg, p_cb)          pacc_init((pacc_init_cfg_t *)p_init_cfg, (pacc_evt_cb_t *)p_cb)
#define bap_uc_cli_pacc_deinit()                        pacc_deinit()
#define bap_uc_cli_pacs_discovery(con_lid)              pacc_service_discovery((uint8_t)con_lid)
#define bap_uc_cli_write_pacs_cccd(con_lid, char_type, pac_lid, enable_ntf)\
                                                        pacc_character_cccd_write((uint8_t)con_lid, (uint8_t)char_type, \
                                                                                (uint8_t)pac_lid, (bool)enable_ntf)
#define bap_uc_cli_read_ava_audio_context(con_lid)\
                                                        pacc_ava_context_read((uint8_t)con_lid)
#define bap_uc_cli_read_supp_audio_context(con_lid)\
                                                        pacc_supp_context_read((uint8_t)con_lid)
#define bap_uc_cli_read_audio_location(con_lid, direction)\
                                                        pacc_audio_location_value_read((uint8_t)con_lid, (uint8_t)direction)
#define bap_uc_cli_read_pac_value(con_lid, direction, pac_lid)\
                                                        pacc_pac_value_read((uint8_t)con_lid, (uint8_t)direction, (uint8_t)pac_lid)
#define bap_uc_cli_write_audio_location(con_lid, direction, location_bf)\
                                                        pacc_audio_location_write((uint8_t)con_lid, (uint8_t)direction, (uint32_t)location_bf)

#endif /// __BAP_UC_CLI_INC__