/**
 ****************************************************************************************
 *
 * @file app_bap_uc_srv_msg.h
 *
 * @brief BLE Audio Audio Stream Control Service Server
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

#ifndef APP_BAP_UC_SRV_MSG_H_
#define APP_BAP_UC_SRV_MSG_H_

#if BLE_AUDIO_ENABLED
#include "app_bap.h"

#include "bap_unicast_server.h"
/*
 * DEFINES
 ****************************************************************************************
 */
#define APP_BAP_DFT_ASCS_NB_ASE_CHAR_SINK         (2)
#define APP_BAP_DFT_ASCS_NB_ASE_CHAR_SRC          (2)
#define APP_BAP_DFT_ASCS_NB_ASE_CHAR              (APP_BAP_DFT_ASCS_NB_ASE_CHAR_SINK + APP_BAP_DFT_ASCS_NB_ASE_CHAR_SRC)
#define APP_BAP_DFT_ASCS_NB_ASE_CFG               (APP_BAP_DFT_ASCS_NB_ASE_CHAR) * (BLE_CONNECTION_MAX)

/*
 * ENUMERATE
 ****************************************************************************************
 */
/// ASCS ASE Structure
typedef struct app_bap_ascs_ase
{
    /// ASE local index
    uint8_t ase_lid;
    /// Connection local index
    uint8_t con_lid;
    /// ASE Instance local index
    uint8_t ase_instance_idx;
    /// ASE Direction
    enum app_gaf_direction direction;
    /// ASE State
    uint8_t ase_state;
    /// Codec ID
    app_gaf_codec_id_t codec_id;
    /// Pointer to Codec Configuration structure
    app_gaf_bap_cfg_t *p_cfg;
    /// QoS Requirements
    app_gaf_bap_qos_req_t qos_req;
    /// CIG ID
    uint8_t cig_id;
    /// CIS ID
    uint8_t cis_id;
    /// CIS Connection Handle
    uint16_t cis_hdl;
    /// QoS Configuration structure
    app_gaf_bap_qos_cfg_t qos_cfg;
    /// Pointer to Metadata structure
    app_gaf_bap_cfg_metadata_t *p_metadata;
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
    /// context type bring by enable req
    uint16_t init_context_bf;
} app_bap_ascs_ase_t;

/// Structure for #bap_uc_srv_get_quality_cmp_evt_t
typedef struct
{
    ///Cmd return status
    uint16_t status;
    /// ASE local index
    uint8_t ase_lid;
    /// Number of packets transmitted and unacked
    uint32_t tx_unacked_packets;
    /// Number of flushed transmitted packets
    uint32_t tx_flushed_packets;
    /// Number of packets transmitted during last subevent
    uint32_t tx_last_subevent_packets;
    /// Number of retransmitted packets
    uint32_t retx_packets;
    /// Number of packets received with a CRC error
    uint32_t crc_error_packets;
    /// Number of unreceived packets
    uint32_t rx_unrx_packets;
    /// Number of duplicate packets received
    uint32_t duplicate_packets;
} __attribute__((__packed__)) app_bap_uc_srv_quality_rpt_evt_t;

typedef struct app_bap_uc_iso_tx_sync_dbg_info
{
    /// 0x00 - The HCI_LE_Read_ISO_TX_Sync command succeeded ; 0x01-0xFF Failed reason
    uint8_t  status;
    /// Connection handle of the CIS or BIS (Range: 0x0000-0x0EFF)
    uint16_t con_hdl;
    ///packet sequence number
    uint16_t packet_seq_num;
    ///The CIG reference point or BIG anchor point of a transmitted SDU
    ///derived using the Controller's free running reference clock (in microseconds).
    uint32_t tx_time_stamp;
    ///The time offset, in microseconds, that is associated with a transmitted SDU.
    uint32_t time_offset;
} __attribute__((__packed__)) app_bap_uc_iso_tx_sync_dbg_info_t;

#ifdef __cplusplus
extern "C" {
#endif
int app_bap_uc_srv_update_metadata_req(uint8_t ase_lid, app_gaf_bap_cfg_metadata_t *metadata);
int app_bap_uc_srv_iso_quality_ind_handler(uint16_t cisHdl, uint8_t *param);
int app_bap_uc_srv_restore_bond_data_req(uint8_t con_lid, uint8_t cli_cfg_bf, uint8_t ase_cli_cfg_bf);
int app_bap_uc_srv_store_codec_cfg_and_qos_req_for_ase(uint8_t ase_lid, app_gaf_bap_cfg_t *codec_cfg,
                                                       app_gaf_bap_qos_req_t *qos_req);
/**
 ****************************************************************************************
 * @brief configure codec
 *
 *
 * @param[in] ase_info          ase info
 *
 ****************************************************************************************
 */
int app_bap_uc_srv_configure_codec_ase_local(uint8_t ase_lid,
                                             const app_gaf_codec_id_t *p_codec_id,
                                             const app_gaf_bap_qos_req_t *p_qos_req,
                                             const app_gaf_bap_cfg_t *ntf_codec_cfg);

int app_bap_uc_srv_get_specifc_state_ase_lid_list(uint8_t con_lid, uint8_t ase_state, uint8_t *ase_lid_list);
#ifdef __cplusplus
}
#endif

#endif
#endif // APP_BAPS_MSG_H_

/// @} APP_BAP
