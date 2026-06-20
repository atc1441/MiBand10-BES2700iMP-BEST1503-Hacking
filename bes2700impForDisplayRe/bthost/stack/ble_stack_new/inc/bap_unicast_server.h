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
#ifndef __BAP_UC_SRV_INC__
#define __BAP_UC_SRV_INC__
#include "bluetooth.h"

#include "pacs.h"
#include "ascs_internal.h"

/*DEFINE*/
#define BAP_UC_SRV_SUPP_ASE_NUM_MAX_PER_DIRECTION   (ASCS_MAX_SUPP_ASE_NUM_PER_DIRECTION >> 1)

#define BAP_UC_SRV_CONNECTION_MAX                   (ASCS_CONNECTION_MAX)

/*ENUMERATIONS*/
#if (BAP_ASCS_ENABLE)
/// CIS EVENT
enum bap_uc_srv_cis_evt
{
    // CIS connected
    BAP_UC_SRV_CIS_EVENT_CONNECTED = 0,
    // CIS establish failed
    BAP_UC_SRV_CIS_EVENT_ESTB_FAILED,
    // CIS disconnected
    BAP_UC_SRV_CIS_EVENT_DISCONNECTED,
};
#endif /// (BAP_ASCS_ENABLE)

/*TYPE DEFINITIONS*/
#if (BAP_ASCS_ENABLE)
typedef struct
{
    /// ASE ID
    uint8_t  ase_id;
    /// Target latency
    uint8_t  latency;
    /// Target PHY
    uint8_t  phy;
    /// Codec ID
    uint8_t codec_id[GEN_AUD_CODEC_ID_LEN];
    /// Codec cfg
    gen_aud_cc_ptr_t codec_cfg;
} bap_uc_srv_codec_cfg_req_t;

typedef struct
{
    /// Framing Type
    uint8_t  framing_type;
    /// PHY
    uint8_t  phy_bf;
    /// Retransimission number
    uint8_t  rtn;
    /// Latency
    uint16_t max_trans_latency_ms;
    /// PresDelay MIN
    uint32_t pres_delay_max_us;
    /// PresDelay MAX
    uint32_t pres_delay_min_us;
    /// Prefer PresDelay MIN
    uint32_t prefer_pd_min_us;
    /// Prefer PresDelay MAX
    uint32_t prefer_pd_max_us;
} bap_uc_srv_codec_cfg_qos_req_t;

typedef struct
{
    /// CIG ID
    uint8_t  cig_id;
    /// CIS ID
    uint8_t  cis_id;
    /// SDU interval
    uint32_t sdu_interval;
    /// Framing Type
    uint8_t  framing;
    /// PHY
    uint8_t  phy;
    /// Max SDU size
    uint16_t max_sdu_size;
    /// Retranssmision number
    uint8_t  rtn;
    /// Max Transport Latency
    uint16_t max_trans_latency;
    /// Presentation Delay
    uint32_t pres_delay;
} bap_uc_srv_qos_cfg_ind_t;

typedef struct
{
    /// CIG ID
    uint8_t  cig_id;
    /// CIS ID
    uint8_t  cis_id;
    /// CIS handle
    uint16_t cis_hdl;
    /// CIS timing
    const void *cis_timing;
} bap_uc_srv_cis_info_t;

/// Callback for ascs bond data
typedef void (*bap_uc_srv_cb_bond_data)(uint8_t con_lid, uint8_t char_type, uint16_t cli_cfg_bf);

typedef void (*bap_uc_srv_cb_ase_codec_cfg_req)(uint8_t con_lid, uint8_t ase_num, const bap_uc_srv_codec_cfg_req_t *p_codec_cfg_req);

typedef void (*bap_uc_srv_cb_ase_enable_req)(uint8_t con_lid, uint8_t ase_num, const ascs_metadata_p_t *param);

typedef void (*bap_uc_srv_cb_ase_update_metadata_req)(uint8_t con_lid, uint8_t ase_num, const ascs_metadata_p_t *param);

typedef void (*bap_uc_srv_cb_ase_state_ind)(uint8_t con_lid, uint8_t ase_id, uint8_t ase_state, const bap_uc_srv_qos_cfg_ind_t *p_qos_cfg_ind);

typedef void (*bap_uc_srv_cb_ase_op_local_cmp)(uint8_t con_lid, uint8_t ase_id, uint8_t op_code, uint16_t status);

typedef void (*bap_uc_srv_cb_cis_status_ind)(enum bap_uc_srv_cis_evt cis_evt, uint8_t con_lid, uint16_t err_code, const bap_uc_srv_cis_info_t *p_cis_info);

typedef void (*bap_uc_srv_cb_iso_dp_status_ind)(bool is_setup, uint8_t con_lid, uint8_t ase_id, uint8_t err_code);

typedef void (*bap_uc_srv_cb_cig_term_ind)(uint8_t con_lid, uint8_t cig_id);

/// Structure
typedef struct bap_uc_srv_ascs_evt_cb
{
    /// Callback function called when client configuration for PACS has been updated
    bap_uc_srv_cb_bond_data cb_bond_data;

    bap_uc_srv_cb_ase_codec_cfg_req cb_ase_codec_cfg_req;

    bap_uc_srv_cb_ase_enable_req cb_ase_enable_req;

    bap_uc_srv_cb_ase_update_metadata_req cb_ase_metadata_upd_req;

    bap_uc_srv_cb_ase_state_ind cb_ase_state_ind;

    bap_uc_srv_cb_ase_op_local_cmp cb_ase_op_local_cmp;

    bap_uc_srv_cb_cis_status_ind cb_cis_status_ind;

    bap_uc_srv_cb_iso_dp_status_ind cb_iso_dp_status_ind;

    bap_uc_srv_cb_cig_term_ind cb_cig_term_ind;
} bap_uc_srv_ascs_evt_cb_t;

typedef struct bap_uc_srv_init_cfg
{
    /// Preferred MTU size
    uint16_t pref_mtu;
    /// Max supported SINK ASE number
    uint8_t num_sink_ase_supp;
    /// Max supported SOURCE ASE number
    uint8_t num_src_ase_supp;
    /// Auto disconnect cis when stream is close
    bool auto_cis_disconn;
    /// Accept cis request when ase is in qos configured state
    bool accpet_qos_cis_req;
} bap_uc_srv_init_cfg_t;

#endif /// (BAP_ASCS_ENABLE)

#if (BAP_PACS_ENABLE)
/*STRUCT INFO REDIFINE*/
typedef pacs_evt_cb_t bap_uc_srv_capa_evt_cb_t;
typedef pacs_init_cfg_t bap_uc_srv_capa_init_cfg_t;
#endif /// (BAP_PACS_ENABLE)

/*INTERFACES*/
#if (BAP_ASCS_ENABLE)
/*BAP UC SRV ASCS INTERFACE*/
/**
 * @brief BAP unicast server initilization
 *
 * @param  bap_uc_srv_init_cfg
 *                 Initilization configuration
 * @param  bap_uc_srv_evt_cb
 *                 Event callbacks
 *
 * @return int         status
 */
int bap_uc_srv_ascs_init(const bap_uc_srv_init_cfg_t *bap_uc_srv_init_cfg,
                         const bap_uc_srv_ascs_evt_cb_t *bap_uc_srv_evt_cb);

/**
 * @brief BAP unicast client deinitilization
 *
 * @return int         status
 */
int bap_uc_srv_ascs_deinit(void);

/**
 * @brief BAP unicast client configure codec ASE local
 *
 * @param  con_lid     Connection local index
 * @param  ase_id      ASE ID (>=1)
 * @param  p_codec_id  Codec ID value pointer
 * @param  p_codec_cfg Codec Configuration
 * @param  p_qos_req   Qos request
 *
 * @return int         status
 */
int bap_uc_srv_configure_codec_ase_local(uint8_t con_lid, uint8_t ase_id, const uint8_t *p_codec_id,
                                         const gen_aud_cc_t *p_codec_cfg, const bap_uc_srv_codec_cfg_qos_req_t *p_qos_req);

/**
 * @brief BAP unicast client disable ASE local
 *
 * @param  con_lid     Connection local index
 * @param  ase_id      ASE ID (>=1)
 *
 * @return int         status
 */
int bap_uc_srv_disable_ase_local(uint8_t con_lid, uint8_t ase_id);

/**
 * @brief BAP unicast client release ASE local
 *
 * @param  con_lid     Connection local index
 * @param  ase_id      ASE ID (>=1)
 *
 * @return int         status
 */
int bap_uc_srv_release_ase_local(uint8_t con_lid, uint8_t ase_id);

/**
 * @brief BAP unicast client confirm codec cfg request
 *
 * @param  con_lid     Connection local index
 * @param  ase_id      ASE ID (>=1)
 * @param  accept      Accpet or not
 * @param  rsp_code    Upper specifc rsp code when rejected
 * @param  reason      Upper specifc reason when rejected
 * @param  p_qos_req   Qos request
 *
 * @return int         status
 */
int bap_uc_srv_configure_codec_req_upper_cfm(uint8_t con_lid, uint8_t ase_id, bool accept,
                                             uint8_t rsp_code, uint8_t reason,
                                             const bap_uc_srv_codec_cfg_qos_req_t *p_qos_req);

/**
 * @brief BAP unicast client confirm enable request
 *
 * @param  con_lid     Connection local index
 * @param  accept      Accpet or not
 * @param  rsp_code    Upper specifc rsp code when rejected
 * @param  reason      Upper specifc reason when rejected
 * @param  ase_num     Number of ASE ID
 * @param  ase_id_list ASE ID list to be confirmed
 *
 * @return int         status
 */
int bap_uc_srv_enable_req_upper_cfm(uint8_t con_lid, bool accept,
                                    uint8_t rsp_code, uint8_t reason,
                                    uint8_t ase_num, const uint8_t *ase_id_list);

/**
 * @brief BAP unicast client confirm update metadata request
 *
 * @param  con_lid     Connection local index
 * @param  accept      Accpet or not
 * @param  rsp_code    Upper specifc rsp code when rejected
 * @param  reason      Upper specifc reason when rejected
 * @param  ase_num     Number of ASE ID
 * @param  ase_id_list ASE ID list to be confirmed
 *
 * @return int         status
 */
int bap_uc_srv_update_metadata_req_upper_cfm(uint8_t con_lid, bool accept,
                                             uint8_t rsp_code, uint8_t reason,
                                             uint8_t ase_num, const uint8_t *ase_id_list);
#if 0
int bap_uc_srv_disonnect_ase_cis(uint8_t con_lid, uint8_t ase_id);
#endif
#endif /// (BAP_ASCS_ENABLE)

#if (BAP_PACS_ENABLE)
/*FUNCTIONS DECLARATION*/
#define bap_uc_srv_pacs_init(pacs_init_cfg, pacs_evt_cb)\
                                            pacs_init((pacs_init_cfg_t *)pacs_init_cfg, (pacs_evt_cb_t *)pacs_evt_cb)

#define bap_uc_srv_pacs_deinit()            pacs_deinit()

#if (GAF_USE_CACHE_GATT_CCCD)
#define bap_uc_srv_restore_pacs_cli_cfg_cache(con_lid, cli_cfg_bf)\
                                            pacs_restore_cli_cfg_cache((uint8_t)con_lid, (uint16_t)cli_cfg_bf)
#define bap_uc_srv_restore_ascs_cli_cfg_cache(con_lid, cli_cfg_bf)\
                                            ascs_restore_cli_cfg_cache((uint8_t)con_lid, (uint16_t)cli_cfg_bf)
#endif /// (GAF_USE_CACHE_GATT_CCCD)

#define bap_uc_srv_set_ava_audio_context(con_lid, direction, ava_audio_context_bf)\
                                            pacs_set_ava_audio_context((uint8_t)con_lid, \
                                                                (uint8_t)direction, (uint16_t)ava_audio_context_bf)

#define bap_uc_srv_set_supp_audio_context(direction, supp_audio_context_bf)\
                                            pacs_set_supp_audio_context((uint8_t)direction, (uint32_t)supp_audio_context_bf)

#define bap_uc_srv_set_audio_location(direction, audio_location_bf)\
                                            pacs_set_audio_location((uint8_t)direction, (uint32_t)audio_location_bf)

#define bap_uc_srv_add_pac_record(pac_lid, record_lid, p_codec_id, p_capa_codec, p_metadata)\
                                            pacs_add_pac_record((uint8_t)pac_lid, (uint8_t)record_lid, \
                                                        (uint8_t *)p_codec_id, (gen_aud_capa_t *)p_capa_codec, \
                                                                                    (gen_aud_metadata_t *)p_metadata)

#define bap_uc_srv_del_pac_record(record_lid)\
                                            pacs_del_pac_record((uint8_t)record_lid)
#endif /// (BAP_PACS_ENABLE)

#endif /// __BAP_UC_SRV_INC__