/**
 ****************************************************************************************
 *
 * @file app_gaf_custom_api.h
 *
 * @brief BLE Audio APIs For Customer
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
 * @addtogroup APP_GAF_CUSTOM_API
 * @{
 ****************************************************************************************
 */
#ifndef APP_GAF_CUSTOM_API_H_
#define APP_GAF_CUSTOM_API_H_

#if BLE_AUDIO_ENABLED

#ifdef __cplusplus
extern "C" {
#endif

#include "prf_types.h"
#include "app_bap_uc_srv_msg.h"
#include "app_bap_uc_cli_msg.h"
#include "app_bap_capa_srv_msg.h"
#include "app_bap_capa_cli_msg.h"
#include "app_bap_bc_src_msg.h"
#include "app_bap_bc_scan_msg.h"
#include "app_bap_bc_sink_msg.h"
#include "app_bap_bc_assist_msg.h"
#include "app_bap_bc_deleg_msg.h"
#include "app_atc_csism_msg.h"
#include "app_atc_csisc_msg.h"

/* Enable/Disable ACC/BAP/ARC/ATC/TMAP/IAP/HAP */
#ifdef BLE_USB_AUDIO_OPTIMIZE_CON_FLOW
#define APP_GAF_BAP_ENABLE      (1)
#define APP_GAF_ARC_ENABLE      (1)
#define APP_GAF_ACC_ENABLE      (0)
#define APP_GAF_ATC_ENABLE      (0)
#define APP_GAF_TMAP_ENABLE     (0)
#define APP_GAF_HAP_ENABLE      (0)
#define APP_GAF_CAP_ENABLE      (0)
#define APP_GAF_GMAP_ENABLE     (0)
#else
#define APP_GAF_BAP_ENABLE      (1)
#define APP_GAF_ARC_ENABLE      (1)
#define APP_GAF_ACC_ENABLE      (1)
#define APP_GAF_ATC_ENABLE      (1)
#define APP_GAF_TMAP_ENABLE     (1)
#define APP_GAF_CAP_ENABLE      (1)
#define APP_GAF_HAP_ENABLE      (0)
#if defined (AOB_GMAP_ENABLED)
#define APP_GAF_GMAP_ENABLE     (1)
#endif
#endif

//For Broadcast Stream(BIS)
#define APP_GAF_BC_SDU_INTERVAL_US          10000
#define APP_GAF_BC_MAX_SDU_SIZE             0xFFF
#define APP_GAF_BC_MAX_TRANS_LATENCY_MS     10

typedef void (*GAF_EVT_REPORT_E)(uint16_t evt, void *param, uint32_t len);

typedef struct
{
    GAF_EVT_REPORT_E earbud_report;
#ifdef AOB_MOBILE_ENABLED
    GAF_EVT_REPORT_E mobile_report;
#endif
} GAF_EVT_REPORT_BUNDLE_T;

/*****************************Custom APIs For Audio Stream Control******************************************/
/* NOTICE: ASC&PAC&BAS services are init on earbuds, earbuds are GATT Server, mobile is GATT Client*/
/**
 ****************************************************************************************
 * @brief Start gaf service discovery
 *
 * @param[in] con_lid              Connection local index
 *
 ****************************************************************************************
 */
void app_gaf_mobile_start_discovery(uint8_t con_lid);

/*BAP PACS APIs (Earbuds)*/
/**
 ****************************************************************************************
 * @brief Get record list of specific PAC.
 *
 * @param[in] pac_lid              PAC local index
 * @param[out] record_list         Record list of specific PAC
 *
 ****************************************************************************************
 */
int app_bap_capa_srv_get_pac_record_list(uint8_t pac_lid, uint8_t *record_list);

/**
 ****************************************************************************************
 * @brief Get record info of specific PAC.
 *
 * @param[in] pac_lid           PAC local index
 * @param[in] record_id         Record ID
 *
 * @return Record Info, @see app_bap_capa_srv_record_t
 *
 ****************************************************************************************
 */
app_bap_capa_srv_record_t *app_bap_capa_srv_get_pac_record_info(uint8_t pac_lid, uint8_t record_id);

/**
 ****************************************************************************************
 * @brief Update record info of specific PAC.
 *
 * @param[in] pac_lid           PAC local index
 * @param[in] record_id         Record ID
 * @param[in] codec_id          Codec ID, @see app_gaf_codec_id_t
 * @param[in] p_capa            Codec Capabilities, @see app_gaf_bap_capa_t
 * @param[in] p_metadata        Codec Capabilities Metadata, @see app_gaf_bap_capa_metadata_t
 *
 ****************************************************************************************
 */
int app_bap_capa_srv_update_pac_record_info(uint8_t pac_lid, uint8_t record_id, app_gaf_codec_id_t *codec_id,
                                            app_gaf_bap_capa_t *p_capa, app_gaf_bap_capa_metadata_t *p_metadata);

/**
 ****************************************************************************************
 * @brief Add a record to specific PAC.
 *
 * @param[in] pac_lid              PAC local index
 * @param[in] codec_id             Codec ID, @see app_gaf_codec_id_t
 * @param[in] p_capa               Codec Capabilities, @see app_gaf_bap_capa_t
 * @param[in] p_metadata           Codec Capabilities Metadata, @see app_gaf_bap_capa_metadata_t
 *
 ****************************************************************************************
 */
int app_bap_capa_srv_add_pac_record(uint8_t pac_lid, app_gaf_codec_id_t *codec_id,
                                    app_gaf_bap_capa_t *p_capa, app_gaf_bap_capa_metadata_t *p_metadata);

/**
 ****************************************************************************************
 * @brief Delete a record of specific PAC.
 *
 * @param[in] pac_lid              PAC local index
 * @param[in] record_id            Record ID
 *
 ****************************************************************************************
 */
int app_bap_capa_srv_delete_pac_record(uint8_t pac_lid, uint8_t record_id);
/**
 ****************************************************************************************
 * @brief clear all records of specific PAC.
 *
 * @param[in] pac_lid              PAC local index
 *
 ****************************************************************************************
 */
int app_bap_capa_srv_clear_pac_record(uint8_t pac_lid);

/**
 ****************************************************************************************
 * @brief set supported location_bf (for each direction).
 *          Expected callback event: APP_GAF_PACS_LOCATION_SET_IND.
 *
 * @param[in] direction              ASE Direction, @see app_gaf_direction
 * @param[in] location_bf            Supported location bitfield, @see app_gaf_loc_bf
 *
 ****************************************************************************************
 */
int app_bap_capa_srv_set_supp_location_bf(enum app_gaf_direction direction, uint32_t location_bf);

/**
 ****************************************************************************************
 * @brief get location_bf (for each direction).
 *          Expected callback event: APP_GAF_PACS_LOCATION_SET_IND.
 *
 * @param[in] direction              ASE Direction, @see app_gaf_direction
 *
 ****************************************************************************************
 */
int app_bap_capa_srv_get_location_bf(enum app_gaf_direction direction);

/**
 ****************************************************************************************
 * @brief set supported Audio Contexts Bitfield (for each direction).
 *
 * @param[in] con_lid                Connection local index
 * @param[in] direction              ASE Direction, @see app_gaf_direction
 * @param[in] context_bf_supp        Supported context bitfield, @see app_bap_context_type_bf
 *
 ****************************************************************************************
 */
int app_bap_capa_srv_set_supp_context_bf(uint8_t con_lid, enum app_gaf_direction direction, uint16_t context_bf_supp);

/**
 ****************************************************************************************
 * @brief set Available Audio Contexts Bitfield (one for each connection, and with both direction).
 *
 * @param[in] con_lid              connection local index
 * @param[in] context_bf_ava_sink       Available Audio Contexts bitfield, @see app_bap_context_type_bf
 * @param[in] context_bf_ava_src       Available Audio Contexts bitfield, @see app_bap_context_type_bf
 *
 ****************************************************************************************
 */
int app_bap_capa_srv_set_ava_context_bf(uint8_t con_lid, uint16_t context_bf_ava_sink, uint16_t context_bf_ava_src);

/**
 ****************************************************************************************
 * @brief set Available Audio Contexts Bitfield (one for each connection, and with both direction).
 *
 * @param[in] con_lid               connection local index
 * @param[out] context_bf_ava_sink  Available Audio Contexts bitfield, @see app_bap_context_type_bf
 * @param[out] context_bf_ava_src   Available Audio Contexts bitfield, @see app_bap_context_type_bf
 *
 ****************************************************************************************
 */
int app_bap_capa_srv_get_ava_context_bf(uint8_t con_lid, uint16_t *context_bf_ava_sink, uint16_t *context_bf_ava_src);

/**
 ****************************************************************************************
 * @brief get peer capa srv available context
 *
 * @param con_lid
 * @param direction
 * @return uint16_t
 ****************************************************************************************
 */
uint16_t app_bap_capa_cli_get_ava_context_bf(uint8_t con_lid, uint8_t direction);

/*BAP ASCS APIs (Earbuds)*/
/**
 ****************************************************************************************
 * @brief Get the number of instances of the ASE characteristic.
 *
 * @return ASE number
 *
 ****************************************************************************************
 */
int app_bap_uc_srv_get_nb_ase_chars(void);

/**
 ****************************************************************************************
 * @brief Get the number of ASE configurations that can be maintained.
 *
 * @return ASE Number
 *
 ****************************************************************************************
 */
int app_bap_uc_srv_get_nb_ases_cfg(void);

/**
 ****************************************************************************************
 * @brief set ASE qos req params.
 *          Expected callback event: APP_GAF_CIS_SRV_STREAM_STATE_UPDATED.
 *
 * @param[in] ase_lid              ASE local index
 * @param[in] qos_req              QoS Requirement params send from server to client when
 *                                 ASE enter Codec Confiured State @see app_gaf_bap_qos_req_t
 *
 ****************************************************************************************
 */
int app_bap_uc_srv_set_ase_qos_req(uint8_t ase_lid, app_gaf_bap_qos_req_t *qos_req);

/**
 ****************************************************************************************
 * @brief get ASE info.
 *
 * @param[in] ase_lid              ASE local index
 *
 * @return ASE Information @see app_bap_ascs_ase_t
 ****************************************************************************************
 */
app_bap_ascs_ase_t *app_bap_uc_srv_get_ase_info(uint8_t ase_lid);

/**
 * @brief Get ASE info by cis handle
 *
 * @param direction
 * @param cis_hdl
 * @return app_bap_ascs_ase_t*
 */
app_bap_ascs_ase_t *app_bap_uc_srv_get_ase_info_from_cis_hdl(enum app_gaf_direction direction, uint16_t cis_hdl);

/**
 ****************************************************************************************
 * @brief get ASE state.
 *
 * @param[in] ase_lid              ASE local index
 *
 * @return ASE state
 ****************************************************************************************
 */
int app_bap_uc_srv_get_ase_state(uint8_t ase_lid);

/**
 ****************************************************************************************
 * @brief get ASE local id.
 *
 * @param[in] con_lid              connection local index
 * @param[in] direction            ASE Direction, @see app_gaf_direction
 *
 * @return ASE local index
 ****************************************************************************************
 */
int app_bap_uc_srv_get_streaming_ase_lid(uint8_t con_lid, enum app_gaf_direction direction);

/**
 ****************************************************************************************
 * @brief get streaming ase list
 *
 * @param con_lid
 * @param ase_lid_list
 * @return uint8_t
 ****************************************************************************************
 */
int app_bap_uc_srv_get_streaming_ase_lid_list(uint8_t con_lid, uint8_t *ase_lid_list);

/**

 ****************************************************************************************
 * @brief Send codec cfg cfm to peer device
 *
 ****************************************************************************************
 */
int app_bap_uc_srv_send_configure_codec_rsp(bool accept, uint8_t ase_lid,
                                            const app_gaf_codec_id_t *p_codec_id,
                                            const app_gaf_bap_qos_req_t *p_qos_req,
                                            const app_gaf_bap_cfg_t *p_codec_cfg);

/**
 ****************************************************************************************
 * @brief Send enable cfm to peer device with reason
 *
 ****************************************************************************************
 */
int app_bap_uc_srv_send_enable_rsp_with_reason(uint8_t ase_lid, bool accept, uint8_t reason);

/**
 ****************************************************************************************
 * @brief Send update metadata cfm to peer device
 *
 ****************************************************************************************
 */
int app_bap_uc_srv_send_update_metadata_rsp(uint8_t ase_lid, bool accpet, uint8_t reason);

/**
 ****************************************************************************************
 * @brief Release a CIS Stream(Stream stop and CIS disconnected).
 *          Expected callback event: APP_GAF_CIS_SRV_STREAM_STATE_UPDATED and APP_GAF_ASCS_CIS_DISCONNETED_IND.
 *
 * @param[in] ase_lid          ASE local index
 * @param[in] idle
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>0  <td> return CODEC CONFIGURED state
 * <tr><td>!0  <td> return to IDLE state
 * </table>
 *
 ****************************************************************************************
 */
int app_bap_uc_srv_stream_release(uint8_t ase_lid, uint8_t idle);

#if 0
/**
 ****************************************************************************************
 * @brief Disconnect CIS coupled with an ase
 *
 * @param[in] ase_lid          ASE local index
 *
 * @return success or other err
 *
 ****************************************************************************************
 */
int app_bap_uc_srv_check_disconnect_exist_cis(uint8_t ase_lid);
#endif

/**
 ****************************************************************************************
 * @brief Read a ISO link quality.
 *
 * @param[in] ase_lid          ASE local index
 *
 * @return a iso link quality info(info structure is hci_le_rd_iso_link_quality_cmd_cmp_evt)
 *
 ****************************************************************************************
 */
int app_bap_uc_srv_read_iso_link_quality(uint8_t ase_lid);

/**
 ****************************************************************************************
 * @brief Disable a CIS Stream(Stream stop but CIS still connect).
 *          Expected callback event: APP_GAF_CIS_SRV_STREAM_STATE_UPDATED and APP_GAF_ASCS_CIS_STREAM_STOPPED_IND.
 *
 * @param[in] ase_lid          ASE local index
 *
 ****************************************************************************************
 */
int app_bap_uc_srv_stream_disable(uint8_t ase_lid);

/**
 ****************************************************************************************
 * @brief HAP HAC restore bond data request
 *
 *
 * @param[in] con_lid     Connection index
 *
 ****************************************************************************************
 */
void app_hap_hac_restore_bond_data_req(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief HAP HAC message discover command
 *
 *
 * @param[in] con_lid     Connection index
 *
 ****************************************************************************************
 */
void app_hap_hac_msg_discover_cmd(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief HAP HAC message get command
 *
 *
 * @param[in] con_lid               Connection index
 * @param[in] char_type             Characteristic type
 * @param[in] presets_instance_idx  Presets characteristic instance index
 *
 ****************************************************************************************
 */
void app_hap_hac_msg_get_cmd(uint8_t con_lid, uint8_t char_type, uint8_t presets_instance_idx);

/**
 ****************************************************************************************
 * @brief HAP HAC message get configure
 *
 *
 * @param[in] con_lid               Connection index
 * @param[in] char_type             Characteristic type
 * @param[in] presets_instance_idx  Presets characteristic instance index
 *
 ****************************************************************************************
 */
void app_hap_hac_msg_get_cfg_cmd(uint8_t con_lid, uint8_t char_type, uint8_t presets_instance_idx);

/**
 ****************************************************************************************
 * @brief HAP HAC message set configure
 *
 *
 * @param[in] con_lid               Connection index
 * @param[in] char_type             Characteristic type
 * @param[in] presets_instance_idx  Presets characteristic instance index
 * @param[in] enable                Enable or disable
 *
 ****************************************************************************************
 */
void app_hap_hac_msg_set_cfg_cmd(uint8_t con_lid, uint8_t char_type,
                                 uint8_t presets_instance_idx, uint8_t enable);

/**
 ****************************************************************************************
 * @brief HAP HAC message set preset name command
 *
 *
 * @param[in] con_lid       Connection index
 * @param[in] preset_idx    Preset index
 * @param[in] length        Length of Preset name
 * @param[in] name          Preset name
 *
 ****************************************************************************************
 */
void app_hap_hac_msg_set_preset_name_cmd(uint8_t con_lid, uint8_t preset_idx,
                                         uint8_t length, char *name);

/**
 ****************************************************************************************
 * @brief HAP HAC message set active preset command
 *
 *
 * @param[in] con_lid       Connection index
 * @param[in] set_type      Set type
 * @param[in] coordinated   Coordinated
 * @param[in] preset_idx    Preset index
 *
 ****************************************************************************************
 */
void app_hap_hac_msg_set_active_preset_cmd(uint8_t con_lid, uint8_t set_type,
                                           uint8_t coordinated, uint8_t preset_idx);

/**
 ****************************************************************************************
 * @brief HAP HAS message restore bond data request
 *
 *
 * @param[in] con_lid               Connection index
 * @param[in] cli_cfg_bf            Client configuration bit field
 * @param[in] presets_cli_cfg_bf    Client configuration bit field for Presets
 * @param[in] evt_cfg_bf            Event configuration bit field
 * @param[in] presets_evt_cfg_bf    Event configuration bit field for Presets
 *
 ****************************************************************************************
 */
void app_hap_has_msg_restore_bond_data_req(uint8_t con_lid, uint8_t cli_cfg_bf,
                                           uint8_t presets_cli_cfg_bf, uint8_t evt_cfg_bf, uint8_t presets_evt_cfg_bf);

/**
 ****************************************************************************************
 * @brief HAP HAS message configure preset request
 *
 *
 * @param[in] preset_lid                Preset local index
 * @param[in] presets_instance_idx      Presets characteristic instance index
 * @param[in] read_only                 Read only or not
 * @param[in] length                    Length of Preset name
 * @param[in] name                      Preset name
 *
 ****************************************************************************************
 */
void app_hap_has_msg_configure_preset_req(uint8_t preset_lid, uint8_t presets_instance_idx,
                                          uint8_t read_only, uint8_t length, char *name);

/**
 ****************************************************************************************
 * @brief HAP HAS message set active preset request
 *
 *
 * @param[in] preset_lid                Preset local index
 *
 ****************************************************************************************
 */
void app_hap_has_msg_set_active_preset_req(uint8_t preset_lid);

/**
 ****************************************************************************************
 * @brief HAP HAS message set coordination support request
 *
 *
 * @param[in] supported     Indicate if Preset Coordination is supported or not
 *
 ****************************************************************************************
 */
void app_hap_has_msg_set_coordination_support_req(uint8_t supported);

/**
 ****************************************************************************************
 * @brief HAP HAS message set preset active confirmation
 *
 *
 * @param[in] status     Status
 *
 ****************************************************************************************
 */
void app_hap_has_msg_set_preset_active_cfm(uint16_t status);

/**
 ****************************************************************************************
 * @brief start IAP test mode
 *
 *
 * @param[in] stream_lid     stream index
 * @param[in] transmit
 * @param[in] payload_type
 *
 ****************************************************************************************
 */
void app_iap_tm_start_cmd(uint8_t stream_lid, uint8_t transmit, uint8_t payload_type);

/**
 ****************************************************************************************
 * @brief read iso tx sync cmd
 *
 *
 * @param[in] stream_lid     stream index
 *
 ****************************************************************************************
 */
void app_iap_read_iso_tx_sync_cmd(uint8_t stream_lid);

/**
 ****************************************************************************************
 * @brief get IAP test mode cnt
 *
 *
 * @param[in] stream_lid     stream index
 *
 ****************************************************************************************
 */
void app_gaf_iap_msg_tm_cnt_get_cmd(uint8_t stream_lid);

/**
 ****************************************************************************************
 * @brief end IAP test mode
 *
 *
 * @param[in] stream_lid     stream index
 *
 ****************************************************************************************
 */
void app_gaf_iap_msg_test_mode_stop_cmd(uint8_t stream_lid);

#ifdef AOB_MOBILE_ENABLED
/**
 ****************************************************************************************
 * @brief Get characteristic info of PAC Server.
 *
 * @param[in] con_lid          Connection local index
 * @param[in] char_type        Characteristic type
 * @param[in] pac_lid          PAC local index(only used when get PAC characteristic info)
 *
 ****************************************************************************************
 */
int app_bap_capa_cli_get(uint8_t con_lid, uint8_t char_type, uint8_t pac_lid);

/**
 ****************************************************************************************
 * @brief Set location_bf of PAC Server.
 *
 * @param[in] con_lid          Connection local index
 * @param[in] direction        ASE Direction, @see app_gaf_direction
 * @param[in] location_bf      ASE Location Bitfield
 *
 ****************************************************************************************
 */
int app_bap_capa_cli_set_remote_location_bf(uint8_t con_lid, uint8_t direction, uint32_t location_bf);

/**
 ****************************************************************************************
 * @brief Get supported capa infomation of specific direction.
 *
 * @param[in] direction        ASE Direction, @see app_gaf_direction
 *
 * @return Capa infomation of specific direction, @see app_bap_capa_cli_supp_t
 *
 ****************************************************************************************
 */
app_bap_capa_cli_supp_t *app_bap_capa_cli_get_capa_info(enum app_gaf_direction direction);

/**
 ****************************************************************************************
 * @brief Set Codec Capabilities and Codec Capabilities Metadata of specific direction.
 *
 * @param[in] direction        ASE Direction, @see app_gaf_direction
 * @param[in] codec_id         Codec Id LC3 or Vendor Specific Codec @see app_gaf_codec_id_t
 * @param[in] p_capa           Codec Capabilities, @see app_gaf_bap_capa_t
 * @param[in] p_metadata       Codec Capabilities Metadata, @see app_gaf_bap_capa_metadata_t
 *
 ****************************************************************************************
 */
int app_bap_capa_cli_set_add_capa_info(enum app_gaf_direction direction, app_gaf_codec_id_t *codec_id,
                                       app_gaf_bap_capa_t *p_capa, app_gaf_bap_capa_metadata_t *p_metadata);

/**
 ****************************************************************************************
 * @brief Set supported location Bitfield.
 *
 * @param[in] direction        ASE Direction, @see app_gaf_direction
 * @param[in] location_bf      Supported Location Bitfield, @see app_gaf_loc_bf
 *
 ****************************************************************************************
 */
int app_bap_capa_cli_set_supp_location_bf(enum app_gaf_direction direction, uint32_t location_bf);

/**
 ****************************************************************************************
 * @brief Set supported context Bitfield.
 *
 * @param[in] direction         ASE Direction, @see app_gaf_direction
 * @param[in] context_bf_supp   Supported context Bitfield, @see app_bap_context_type_bf
 *
 ****************************************************************************************
 */
int app_bap_capa_cli_set_supp_context_bf(enum app_gaf_direction direction, uint16_t context_bf_supp);

/*BAP ASCC APIs (Mobile)*/
/**
 ****************************************************************************************
 * @brief Disable a CIS Stream(Stream stop but CIS still connect).
 *          Expected callback event: APP_GAF_CIS_CLI_STREAM_STATE_UPDATED and APP_GAF_ASCC_CIS_STREAM_STOPPED_IND.
 *
 * @param[in] ase_lid          ASE local index
 *
 ****************************************************************************************
 */
int app_bap_uc_cli_stream_disable(uint8_t ase_lid);

#if 0
/**
 ****************************************************************************************
 * @brief Disconnect a CIS Stream
 *
 *
 * @param[in] ase_lid          ASE local index
 *
 ****************************************************************************************
 */
int app_bap_uc_cli_check_disonnect_cis(uint8_t ase_lid);
#endif

/**
 ****************************************************************************************
 * @brief Release a CIS Stream(Stream stop and CIS disconnected).
 *          Expected callback event: APP_GAF_CIS_CLI_STREAM_STATE_UPDATED and APP_GAF_ASCC_CIS_DISCONNETED_IND.
 *
 * @param[in] ase_lid          ASE local index
 *
 ****************************************************************************************
 */
int app_bap_uc_cli_stream_release(uint8_t ase_lid);

int app_bap_uc_cli_link_remove_group_cmd(uint8_t grp_lid);

/**
 ****************************************************************************************
 * @brief Config ase codec using pac record
 *
 * @param[in] ase_lid              ASE local index
 * @param[in] direction            ASE Direction, @see app_gaf_direction
 * @param[in] cis_id               cis index
 *
 ****************************************************************************************
 */
int app_bap_uc_cli_configure_codec(uint8_t ase_lid, uint8_t cis_id, const app_gaf_codec_id_t *codec_id,
                                   uint8_t sampleRate_enum, uint16_t frame_octet);

/**
 ****************************************************************************************
 * @brief Configure ase codec using external provided codec cfg parameters
 *
 * @param[in] ase_lid              ASE local index
 * @param[in] direction            ASE Direction, @see app_gaf_direction
 * @param[in] cis_id               cis index
 *
 ****************************************************************************************
 */
int app_bap_uc_cli_configure_codec_with_cfg(uint8_t ase_lid, uint8_t cis_id,
                                            const app_gaf_codec_id_t *codec_id, const app_gaf_bap_cfg_t *p_cfg);

/**
 ****************************************************************************************
 * @brief get ASE info.
 *
 * @param[in] ase_lid              ASE local index
 *
 * @return ASE Information @see app_bap_ascc_ase_t
 ****************************************************************************************
 */
app_bap_ascc_ase_t *app_bap_uc_cli_get_ase_info_by_ase_lid(uint8_t ase_lid);

/**
 ****************************************************************************************
 * @brief get ASE list of specific BLE connection.
 *
 * @param[in] con_lid              connection local index
 *
 * @return number of ASE
 * @param[out] ase_lid_list
 ****************************************************************************************
 */
uint8_t app_bap_uc_cli_get_ase_lid_list(uint8_t con_lid, uint8_t *ase_lid_list);

/**
 ****************************************************************************************
 * @brief get ASE list of specific ase state and connection.
 *
 * @param[in] con_lid              connection local index
 * @param[in] direction            ase direction
 * @param[in] ase_state            ase state
 *
 * @return number of ASE
 * @param[out] ase_lid_list
 ****************************************************************************************
 */
uint8_t app_bap_uc_cli_get_specific_state_ase_lid_list(uint8_t con_lid, uint8_t direction,
                                                       uint8_t ase_state,
                                                       uint8_t *ase_lid_list);

/**
 ****************************************************************************************
 * @brief Get the number of ASE configurations that can be maintained.
 *
 * @return ASE Number
 *
 ****************************************************************************************
 */
uint8_t app_bap_uc_cli_get_nb_ases_cfg();

uint8_t app_bap_uc_cli_get_ava_ase_lid_by_ase_instance_idx(uint8_t con_lid, uint8_t instance_idx, bool start);

/**
 ****************************************************************************************
 * @brief Get the available ASE lid that state is under enabling
 *
 * @param[in] con_lid              connection local index
 * @param[in] instance_idx         ase instant idx
 * @return ASE lid
 *
 ****************************************************************************************
 */
uint8_t app_bap_uc_cli_get_ase_under_enabling_by_ase_id(uint8_t con_lid, uint8_t instance_idx);

/**
 ****************************************************************************************
 * @brief Get the ase state by ase_id
 *
 * @return ase state
 *
 ****************************************************************************************
 */
uint8_t app_bap_uc_get_ase_state_by_ase_id(uint8_t ase_id);

/**
 ****************************************************************************************
 * @brief Get the ASE lid
 *
 * @return ASE lid
 *
 ****************************************************************************************
 */
uint8_t app_bap_uc_cli_get_ase_lid(uint8_t con_lid, uint8_t ase_state, uint8_t direction);

/**
 ****************************************************************************************
 * @brief Get the available ASE lid
 *
 * @return True: Bonded False: Not bonded
 *
 ****************************************************************************************
 */
bool app_bap_uc_cli_is_already_bonded(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief get streaming ASE local id.
 *
 * @param[in] con_lid              connection local index
 * @param[in] direction            ASE Direction, @see app_gaf_direction
 *
 * @return ASE local index
 ****************************************************************************************
 */
uint8_t app_bap_uc_cli_get_streaming_ase_lid(uint8_t con_lid, enum app_gaf_direction direction);

int app_bap_uc_cli_link_create_group_req(uint8_t cig_id);

/**
 ****************************************************************************************
 * @brief configure ASE's QoS
 *
 * @param[in] ase_lid   ASE local index
 * @param[in] grp_lid   Group local index
 * @param[in] max_sdu_size
 *                      Max SDU size
 *
 ****************************************************************************************
 */
int app_bap_uc_cli_configure_qos(uint8_t ase_lid, uint8_t grp_lid, uint16_t max_sdu_size);

/**
 ****************************************************************************************
 * @brief Enable streaming
 *
 * @param[in] ase_lid       ASE local index
 * @param[in] context_bf    Context type, @see gaf_bap_context_type_bf
 * @param[in] ccid_num      ccid list length
 * @param[in] p_ccid_list   ccid list
 *
 ****************************************************************************************
 */
int app_bap_uc_cli_enable_stream_with_ccid(uint8_t ase_lid, uint16_t context_bf,
                                           uint8_t ccid_num, const uint8_t *p_ccid_list);

/**
 ****************************************************************************************
 * @brief Enable streaming
 *
 * @param[in] ase_lid   ASE local index
 * @param[in] context_bf   Context type, @see gaf_bap_context_type_bf
 *
 ****************************************************************************************
 */
int app_bap_uc_cli_enable_stream(uint8_t ase_lid, uint16_t context_bf);
#endif

/*BAP Broadcast Src APIs*/
/**
 ****************************************************************************************
 * @brief Start a Broadcase Source Audio Stream.
 *          Expected callback event: APP_GAF_SRC_BIS_STREAM_STARTED_IND.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 *
 ****************************************************************************************
 */
int app_bap_bc_src_start(uint8_t big_idx);

/**
 ****************************************************************************************
 * @brief Stop a Broadcase Source Audio Stream.
 *          Expected callback event: APP_GAF_SRC_BIS_STREAM_STOPPED_IND.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 *
 ****************************************************************************************
 */
int app_bap_bc_src_stop(uint8_t big_idx);

/**
 ****************************************************************************************
 * @brief get BIG group params.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[out] grp_param        BIG Group params, @see app_bap_bc_src_grp_info_t
 *
 ****************************************************************************************
 */
app_bap_bc_src_grp_info_t *app_bap_bc_src_get_big_info(uint8_t big_idx);

/**
 ****************************************************************************************
 * @brief Set BIG group params.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] grp_param        BIG Group params, @see app_bap_bc_src_grp_param_t
 *
 ****************************************************************************************
 */
int app_bap_bc_src_set_big_params(uint8_t big_idx, app_bap_bc_src_grp_param_t *grp_param);

/**
 ****************************************************************************************
 * @brief Set BIG subgrp params.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] subgrp_param     BIG Subgroup params, @see app_bap_bc_src_grp_param_t
 *
 ****************************************************************************************
 */
int app_bap_bc_src_set_subgrp_params(uint8_t big_idx, app_bap_bc_src_subgrp_info_t *subgrp_param);

/**
 ****************************************************************************************
 * @brief Set BIG stream params.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] stream_param     BIG stream params, @see app_bap_bc_src_grp_param_t
 *
 ****************************************************************************************
 */
int app_bap_bc_src_set_stream_params(uint8_t big_idx, app_bap_bc_src_stream_info_t *stream_param);

/**
 ****************************************************************************************
 * @brief Set BIG advertising params.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] adv_param        BIG Advertising params, @see app_gaf_bap_bc_adv_param_t
 *
 ****************************************************************************************
 */
int app_bap_bc_src_set_adv_params(uint8_t big_idx, app_gaf_bap_bc_adv_param_t *adv_param);

/**
 ****************************************************************************************
 * @brief Set BIG periodic advertising params.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] per_adv_param    BIG Periodic Advertising params, @see app_gaf_bap_bc_per_adv_param_t
 *
 ****************************************************************************************
 */
int app_bap_bc_src_set_per_adv_params(uint8_t big_idx, app_gaf_bap_bc_per_adv_param_t *per_adv_param);

/**
 ****************************************************************************************
 * @brief Set BIG presentation delay in us.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] pres_delay_us    Presentation Delay(Unit: us)
 *
 ****************************************************************************************
 */
int app_bap_bc_src_set_pres_delay_us(uint8_t big_idx, uint32_t pres_delay_us);

/**
 ****************************************************************************************
 * @brief Src start tx stream.
 *
 * @param[in] bis_hdl          BIS Handle
 *
 ****************************************************************************************
 */
int app_bap_bc_src_tx_stream_start(uint16_t bis_hdl);

/**
 ****************************************************************************************
 * @brief Src stop tx stream.
 *
 * @param[in] bis_hdl          BIS Handle
 *
 ****************************************************************************************
 */
int app_bap_bc_src_tx_stream_stop(uint16_t bis_hdl);

/**
 ****************************************************************************************
 * @brief Set BIG Codec configuration.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] sgrp_lid         Subgroup local index
 * @param[in] codec_id         Codec Id LC3 or Vendor Specific Codec @see app_gaf_codec_id_t
 * @param[in] frame_octet      Length of a codec frame in octets
 * @param[in] sampling_freq    Sampling Frequency (see #bap_sampling_freq enumeration)
 *
 ****************************************************************************************
 */
int app_bap_bc_src_set_subgrp_codec_cfg(uint8_t big_idx, uint8_t sgrp_lid,
                                        app_gaf_codec_id_t *codec_id, uint16_t frame_octet, uint8_t sampling_freq);
/**
 ****************************************************************************************
 * @brief Set Codec configuration of specific BIS stream.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] stream_lid       Stream local index
 * @param[in] frame_octet      Length of a codec frame in octets
 * @param[in] sampling_freq    Sampling Frequency (see #bap_sampling_freq enumeration)
 *
 ****************************************************************************************
 */
int app_bap_bc_src_set_stream_codec_cfg(uint8_t big_idx, uint8_t stream_lid, uint16_t frame_octet, uint8_t sampling_freq);

/**
 ****************************************************************************************
 * @brief Update Codec configuration of specific BIS stream.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] stream_lid       Stream local index
 * @param[in] p_cfg            Codec Configuration
 *
 ****************************************************************************************
 */
int app_bap_bc_src_update_stream_codec_cfg(uint8_t big_idx, uint8_t stream_lid, app_gaf_bap_cfg_t *p_cfg);

/**
 ****************************************************************************************
 * @brief Set BIG broadcast id
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] bcast_id         broadcast id
 * @param[in] bcast_id_len     Broadcast id length
 *
 ****************************************************************************************
 */
int app_bap_bc_src_set_bcast_id(uint8_t big_idx, uint8_t *bcast_id, uint8_t bcast_id_len);

/**
 ****************************************************************************************
 * @brief Set BIG encrypt configuration.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] is_encrypted     0:Not encrypted, !=0:encrypted
 * @param[in] bcast_code       Broadcast Code, @see app_gaf_bcast_code_t, only meaningful when is_encrypted != 0
 *
 ****************************************************************************************
 */
int app_bap_bc_src_set_encrypt(uint8_t big_idx, uint8_t is_encrypted, app_gaf_bc_code_t *bcast_code);

/**
 ****************************************************************************************
 * @brief Set BIG adv data and periodic adv data.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] adv_len          adv data length
 * @param[in] adv_data         adv data
 *
 ****************************************************************************************
 */
int app_bap_bc_src_set_adv_data(uint8_t big_idx, uint8_t adv_len, uint8_t *adv_data);

/**
 ****************************************************************************************
 * @brief Set BIG adv data and periodic adv data.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] per_adv_len      periodic adv data length
 * @param[in] per_adv_data     periodic adv data
 *
 ****************************************************************************************
 */
int app_bap_bc_src_set_per_adv_data(uint8_t big_idx, uint8_t per_adv_len, uint8_t *per_adv_data);

/**
 ****************************************************************************************
 * @brief Src remove group req
 *
 * @param[in] grp_lid         Group local index
 *
 ****************************************************************************************
 */
int app_bap_bc_src_remove_group_cmd(uint8_t grp_lid);

/**
 ****************************************************************************************
 * @brief Start src steaming
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] stream_lid_bf    Stream local index bit field indicating for which stream streaming must be started
 *                             0xFFFFFFFF means that streaming must be started for all BISes
 *
 ****************************************************************************************
 */
int app_bap_bc_src_start_streaming(uint8_t big_idx, uint8_t stream_lid_bf);

/**
 ****************************************************************************************
 * @brief Update additional adv data request, pa or ea additional adv data
 *
 * @param[in] grp_lid          BIS Group(BIG) local index
 * @param[in] adv_type         0: Extended adv, 1: Periodic adv
 * @param[in] data_len         data length
 * @param[in] data             data pointer
 *
 ****************************************************************************************
 */
int app_bap_bc_src_update_adv_data_req(uint8_t grp_lid, uint8_t adv_type, uint8_t data_len, uint8_t *data);

/**
 ****************************************************************************************
 * @brief Update metadeta request
 *
 * @param[in] grp_lid          BIS Group(BIG) local index
 * @param[in] sgrp_lid         BIS SubGroup local index
 * @param[in] metadata         Metadata for Codec Configuration see @app_gaf_bap_cfg_metadata_t
 *
 ****************************************************************************************
 */
int app_bap_bc_src_update_metadata_req(uint8_t grp_lid, uint8_t sgrp_lid, app_gaf_bap_cfg_metadata_t *metadata);

/**
 ****************************************************************************************
 * @brief Stop src steaming
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] stream_lid_bf    Stream local index bit field indicating for which stream streaming must be stopped
 *                             0xFFFFFFFF means that streaming must be stopped for all BISes
 *
 ****************************************************************************************
 */
int app_bap_bc_src_stop_streaming(uint8_t big_idx, uint8_t stream_lid_bf);

/*BAP Broadcast Scan APIs*/
/**
 ****************************************************************************************
 * @brief Get scan report info according to pa lid.
 *
 * @param[in] pa_lid         Periodic advertising local index
 * @return Point of pa info
 *
 ****************************************************************************************
 */
app_gaf_bap_bc_scan_pa_report_info_t *app_bap_bc_scan_get_exist_pa_info_by_pa_lid(uint8_t pa_lid);

/**
 ****************************************************************************************
 * @brief Start scanning.
 *          Expected callback event: APP_GAF_SCAN_REPORT_IND.
 *
 * @param[in] scan_trigger_method         Trigger method, @see enum app_gaf_bap_bc_scan_method
 *
 ****************************************************************************************
 */
int app_bap_bc_scan_start(enum app_gaf_bap_bc_scan_method scan_method);

/**
 ****************************************************************************************
 * @brief Stop scanning.
 *          Expected callback event: APP_GAF_SCAN_STOPPED_IND.
 *
 ****************************************************************************************
 */
int app_bap_bc_scan_stop(void);

/**
 ****************************************************************************************
 * @brief Set scan parameters.
 * @param[in] scan_timeout_s     Scan timeout.Unit:s
 * @param[in] scan_param         Scan parameters, @see app_gaf_bap_bc_scan_param_t
 *
 ****************************************************************************************
 */
int app_bap_bc_scan_set_scan_param(uint16_t scan_timeout_s, app_gaf_bap_bc_scan_param_t *scan_param);

/**
 ****************************************************************************************
 * @brief Set synchronize parameters.
 * @param[in] sync_param         Synchronize parameters, @see app_gaf_bap_bc_scan_sync_param_t
 *
 ****************************************************************************************
 */
int app_bap_bc_scan_set_sync_param(app_gaf_bap_bc_scan_sync_param_t *sync_param);

/**
 ****************************************************************************************
 * @brief Start periodic advertising synchronize.
 * @param[in] adv_id         Address info about the device to synchronize, @see app_gaf_bap_adv_id_t
 * @param[in] sync_to_10ms   Sync timeout to cancel sync when controller can not sync pa
 *
 ****************************************************************************************
 */
int app_bap_bc_scan_pa_sync_with_to(app_gaf_bap_adv_id_t *adv_id, uint16_t sync_to_10ms);

/**
 ****************************************************************************************
 * @brief Start periodic advertising synchronize.
 * @param[in] adv_id         Address info about the device to synchronize, @see app_gaf_bap_adv_id_t
 *
 ****************************************************************************************
 */
int app_bap_bc_scan_pa_sync(app_gaf_bap_adv_id_t *adv_id);

/**
 ****************************************************************************************
 * @brief Cancel periodic advertising synchronize.
 *
 ****************************************************************************************
 */
void app_bap_bc_scan_pa_sync_cancel(void);

/**
 ****************************************************************************************
 * @brief Set the periodic advertising information reported by stack.
 * @param[in] pa_lid                Periodic advertising local index
 * @param[in] report_filter_bf      Report filter Bitfield, @see app_bap_bc_scan_report_filter_bf
 *                                  Bit set to 1 means report, 0 means Not report
 *
 ****************************************************************************************
 */
int app_bap_bc_scan_pa_report_ctrl(uint8_t pa_lid, uint8_t report_filter_bf);

/**
 ****************************************************************************************
 * @brief Terminate periodic advertising synchronize.
 *          Expected callback event: APP_GAF_SCAN_PA_TERMINATED_IND.
 * @param[in] pa_lid         Periodic advertising local index
 *
 ****************************************************************************************
 */
int app_bap_bc_scan_pa_terminate(uint8_t pa_lid);

/**
 ****************************************************************************************
 * @brief Terminate periodic advertising synchronize req.
 *          Expected callback event: BAP_BC_SCAN_PA_SYNCHRONIZE_RI.
 * @param[in] pa_lid         Periodic advertising local index
 * @param[in] accept         false: not accept, true: accept
 *
 ****************************************************************************************
 */
int app_bap_bc_scan_dele_pa_sync_ri(uint8_t pa_lid, bool accept);

/**
 ****************************************************************************************
 * @brief Terminate periodic advertising synchronize req.
 *          Expected callback event: BAP_BC_SCAN_PA_TERMINATE_RI.
 * @param[in] pa_lid         Periodic advertising local index
 * @param[in] accept         false: not accept, true: accept
 *
 ****************************************************************************************
 */
int app_bap_bc_scan_dele_pa_terminate_ri(uint8_t pa_lid, bool accept);

/*BAP Broadcast Sink APIs*/
/**
 ****************************************************************************************
 * @brief Enable a group of sink streams.
 * @param[in] p_sink_enable         Sink Stream group information, @see app_gaf_bap_bc_sink_enable_t
 *
 ****************************************************************************************
 */
int app_bap_bc_sink_enable(app_gaf_bap_bc_sink_enable_t *p_sink_enable);

/**
 ****************************************************************************************
 * @brief Disable a group of sink streams.
 * @param[in] grp_lid         Group local index
 *
 ****************************************************************************************
 */
int app_bap_bc_sink_disable(uint8_t grp_lid);

/**
 ****************************************************************************************
 * @brief Sink stop rx stream.
 *
 * @param[in] bis_hdl          BIS Handle
 * @param[in] frame_octet      Length of a codec frame in octets
 * @param[in] grp_lid          Group local index
 *
 ****************************************************************************************
 */
int app_bap_bc_sink_rx_stream_start(uint16_t bis_hdl, uint16_t frame_octet, uint8_t grp_lid);

/**
 ****************************************************************************************
 * @brief Sink stop rx stream.
 *
 * @param[in] bis_hdl          BIS Handle
 * @param[in] grp_lid         Group local index
 *
 ****************************************************************************************
 */
int app_bap_bc_sink_rx_stream_stop(uint16_t bis_hdl, uint8_t grp_lid);

/**
 ****************************************************************************************
 * @brief Sink get bis hdl.
 *
 * @param[in] stream_lid          stream local index
 * @return BIS Handle
 *
 ****************************************************************************************
 */
uint16_t app_bap_bc_sink_get_bis_hdl(uint8_t stream_lid);

/**
 ****************************************************************************************
 * @brief Config one of a group of sink stream.
 * @param[in] grp_lid         Group local index
 * @param[in] stream_pos_bf   Stream position bit field in broadcast group to receive
 * @param[in] codec_type      Codec type
 * @param[in] frame_octet     Length of a codec frame in octets
 * @param[in] sampling_freq   Sampling Frequency (see #bap_sampling_freq enumeration)
 * @param[out] app_gaf_bap_bc_sink_audio_streaming_t   Point of streaming info
 ****************************************************************************************
 */
int app_bap_bc_sink_streaming_config(uint8_t grp_lid, uint32_t stream_pos_bf, uint8_t codec_type,
                                     app_gaf_bap_bc_scan_subgrp_info_t *subgroup_info,
                                     app_gaf_bap_bc_scan_stream_info_t *select_stream_info,
                                     app_gaf_bap_bc_sink_audio_streaming_t *stream_info);

/**
 ****************************************************************************************
 * @brief Start one of a group of sink streams.
 *          Expected callback event: APP_GAF_SINK_BIS_STREAM_STARTED_IND.
 * @param[in] stream         The information of stream to start, @see app_gaf_bap_bc_sink_audio_streaming_t
 *
 ****************************************************************************************
 */
int app_bap_bc_sink_start_streaming(app_gaf_bap_bc_sink_audio_streaming_t *stream);

/**
 ****************************************************************************************
 * @brief Stop one of a group of sink streams.
 *          Expected callback event: APP_GAF_SINK_BIS_STREAM_STOPPED_IND.
 * @param[in] grp_lid         Sink streams group local index
 * @param[in] stream_pos      Stream position in group
 *
 ****************************************************************************************
 */
int app_bap_bc_sink_stop_streaming(uint8_t grp_lid, uint8_t stream_pos);

/*BAP Broadcast Delegator APIs*/
/**
 ****************************************************************************************
 * @brief Remove Broadcast Source from upper layer
 * @param[in] src_lid        Broadcast local index
 *
 ****************************************************************************************
 */
int app_bap_bc_deleg_source_remove(uint8_t src_lid);

/**
 ****************************************************************************************
 * @brief Confirm bis sync prefered from upper layer
 * @param[in] src_lid        Broadcast local index
 *
 ****************************************************************************************
 */
int app_bap_bc_deleg_pref_bis_sync_cfm(uint8_t src_lid, uint32_t upper_pref_bis_sync);

/**
 ****************************************************************************************
 * @brief Set advertising data of solicite advertising.
 * @param[in] adv_data        Advertising data
 * @param[in] adv_data_len    Advertising data length
 *
 ****************************************************************************************
 */
int app_bap_bc_deleg_set_solicite_adv_data(char *adv_data, uint8_t adv_data_len);

/**
 ****************************************************************************************
 * @brief Set advertising params of solicite advertising.
 * @param[in] adv_param        Advertising params, @see app_gaf_bap_bc_adv_param_t
 *
 ****************************************************************************************
 */
int app_bap_bc_deleg_set_adv_params(app_gaf_bap_bc_adv_param_t *adv_param);

/**
 ****************************************************************************************
 * @brief Start solicite advertising.
 *          Expected callback event: APP_GAF_DELEG_SOLICITE_STARTED_IND.
 *
 * @param[in] timeout_s         Timeout duration of adv, Unit:s
 * @param[in] context_bf        Available audio contexts bit field in adv data, , @see enum gaf_bap_context_type_bf
 *
 ****************************************************************************************
 */
int app_bap_bc_deleg_start_solicite(uint16_t timeout_s, uint32_t context_bf);

/**
 ****************************************************************************************
 * @brief Sync to periodic advertising.
 *          Expected callback event: APP_GAF_SCAN_PA_ESTABLISHED_IND.
 * @param[in] src_lid         Broadcast source local index
 *
 ****************************************************************************************
 */
int app_bap_bc_deleg_pa_sync(uint8_t src_lid);

/**
 ****************************************************************************************
 * @brief Sync to big.
 * @param[in] p_sink_enable         Infomation of BIG to sync, @see app_gaf_bap_bc_deleg_sink_enable_t
 *
 ****************************************************************************************
 */
int app_bap_bc_deleg_sink_enable(app_gaf_bap_bc_deleg_sink_enable_t *p_sink_enable);

/**
 ****************************************************************************************
 * @brief Stop solicite advertising.
 *          Expected callback event: APP_GAF_DELEG_SOLICITE_STOPPED_IND.
 *
 ****************************************************************************************
 */
int app_bap_bc_deleg_stop_solicite(void);

#ifdef AOB_MOBILE_ENABLED
/*BAP Broadcast Assistant APIs*/
/**
 ****************************************************************************************
 * @brief Send the information regarding a Broadcast Source to Delegator.
 * @param[in] p_src_add         Broadcast Source Information, @see app_gaf_bap_bc_assist_add_src_t
 *
 ****************************************************************************************
 */
int app_bap_bc_assist_source_add(app_gaf_bap_bc_assist_add_src_t *p_src_add);

/**
 ****************************************************************************************
 * @brief Request the Delegator to remove information for a Broadcast Source.
 * @param[in] con_lid         Connection local index
 * @param[in] src_lid         Broadcast local index
 *
 ****************************************************************************************
 */
int app_bap_bc_assist_source_remove(uint8_t con_lid, uint8_t src_lid);

/**
 ****************************************************************************************
 * @brief Get the Broadcast Receive State of Delegator.
 * @param[in] con_lid         Connection local index
 * @param[in] src_lid         Broadcast local index
 *
 ****************************************************************************************
 */
int app_bap_bc_assist_get_state(uint8_t con_lid, uint8_t src_lid);

/**
 ****************************************************************************************
 * @brief Start scanning for solicitation.
 *          Expected callback event: APP_GAF_SCAN_REPORT_IND and APP_GAF_SCAN_PA_ESTABLISHED_IND.
 * @param[in] timeout_s         Scanning time duration, Unit:s
 *
 ****************************************************************************************
 */
int app_bap_bc_assist_start_scan(uint16_t timeout_s);

/**
 ****************************************************************************************
 * @brief Stop scanning for solicitation.
 *          Expected callback event: APP_GAF_SCAN_STOPPED_IND.
 *
 ****************************************************************************************
 */
int app_bap_bc_assist_stop_scan(void);

/**
 ****************************************************************************************
 * @brief Get the Broadcast Receive State of Delegator.
 * @param[in] con_lid         Connection local index
 * @param[in] src_lid         Source local index
 * @param[in] bcast_code      Bcast code set by user
 *
 ****************************************************************************************
 */
int app_bap_bc_assist_send_bcast_code(uint8_t con_lid, uint8_t src_lid, uint8_t *bcast_code);

/*****************************Custom APIs For Audio Rendering Control******************************************/
/* NOTICE: ARC services are init on earbuds, earbuds are GATT Server, mobile is GATT Client*/
/* Audio Input Control APIs for mobile*/
int app_arc_aicc_set_desc(uint8_t con_lid, uint8_t input_lid,
                          char *desc, uint16_t desc_len);
int app_arc_aicc_set_cfg(uint8_t con_lid, uint8_t input_lid,
                         uint8_t char_type, uint8_t enable);
int app_arc_aicc_control(uint8_t con_lid, uint8_t input_lid,
                         uint8_t opcode, int16_t value);
int app_arc_aicc_read(uint8_t con_lid, uint8_t input_lid, uint8_t char_type);
#endif

/* Audio Input Control APIs for earbuds*/
int app_arc_aics_set(uint8_t input_lid, uint8_t set_type, uint32_t value);
int app_arc_aics_set_desc(uint8_t input_lid, uint8_t *desc, uint8_t desc_len);

#ifdef AOB_MOBILE_ENABLED
/* Microphone Control APIs for mobile */
int app_arc_micc_set_cfg(uint8_t con_lid, uint8_t enable);
int app_arc_micc_set_mute(uint8_t con_lid, uint8_t mute);
int app_arc_micc_read_mute(uint8_t con_lid);
#endif

/* Microphone Control API for earbuds */
int app_arc_mics_set_mute(uint8_t mute);

#ifdef AOB_MOBILE_ENABLED
/* Volume Control API for mobile */
/**
 * @brief API for volume control on mobile
 *
 * @param con_lid       Connection ID to change volume
 * @param opcode        Opcode, @see enum arc_vc_opcode to get more info
 * @param volume        Volume to set, in range of 0-255; valid only when opcode is ARC_VC_OPCODE_VOL_SET_ABS
 */
int app_arc_vcc_control(uint8_t con_lid, uint8_t opcode, uint8_t volume);

/* Volume Offset Control APIs for mobile */
int app_arc_vocc_set_desc(uint8_t con_lid, uint8_t output_lid,
                          char *desc, uint16_t desc_len);
int app_arc_vocc_set_cfg(uint8_t con_lid, uint8_t output_lid,
                         uint8_t char_type, uint8_t enable);
int app_arc_vocc_control(uint8_t con_lid, uint8_t output_lid,
                                    uint8_t opcode, uint32_t value);
int app_arc_vocc_read(uint8_t con_lid, uint8_t output_lid, uint8_t char_type);
#endif

/* Volume Control API for earbuds */
/**
 * @brief API for volume control
 *
 * @param opcode        Opcode, @see enum arc_vc_opcode to get more info
 * @param volume        Volume to set, in range of 0-255; valid only when opcode is ARC_VC_OPCODE_VOL_SET_ABS
 */
int app_arc_vcs_control(uint8_t opcode, uint8_t volume, bool no_changed_cb);

/**
 * @brief API for send notification
 *
 * @param con_lid       Local connection ID
 * @param char_type     Characteristic type
 */
int app_arc_vcs_send_ntf(uint8_t con_lid, uint8_t char_type);

/**
 * @brief API for volume info update
*/
int app_arc_vcs_update_info_req(uint8_t bit_map, uint8_t volume, bool mute);

/* Volume Offset Control APIs for earbuds */
int app_arc_vocs_set(uint8_t output_lid, uint8_t set_type, uint32_t value);
int app_arc_vocs_set_desc(uint8_t output_lid, uint8_t *desc, uint8_t desc_len);

/*****************************Custom APIs For Audio Topology Control******************************************/
/* NOTICE: ATC services are init on earbuds, earbuds are GATT Server, mobile is GATT Client*/
#ifdef AOB_MOBILE_ENABLED
/* Coordinated Set Identification APIs for mobile */
int app_atc_csisc_resolve(uint8_t *rsi);
int app_atc_csisc_lock(uint8_t con_lid, uint8_t set_lid, uint8_t lock);
int app_atc_csisc_get(uint8_t con_lid, uint8_t set_lid, uint8_t char_type);
int app_atc_csisc_get_cfg(uint8_t con_lid, uint8_t set_lid, uint8_t char_type);
int app_atc_csisc_set_cfg(uint8_t con_lid, uint8_t set_lid, uint8_t char_type, uint8_t enable);
int app_atc_csisc_add_sirk(uint8_t *sirk);
int app_atc_csisc_remove_sirk(uint8_t key_lid);
APP_ATC_CSISC_ENV_T *app_atc_csisc_get_env(void);
#endif

/* Coordinated Set Identification APIs for earbuds */
APP_ATC_CSISM_ENV_T *app_atc_csism_get_env(void);
int app_atc_csism_register_sets_config_cb(csism_info_user_config_cb func_cb);
int app_atc_csism_set_sirk(uint8_t set_lid, uint8_t *sirk);
int app_atc_csism_update_rsi(uint8_t set_lid);
int app_atc_csism_set_size(uint8_t set_lid, uint8_t size);
void app_atc_csism_set_lock(uint8_t con_lid, bool lock);
/**
 ****************************************************************************************
 * @brief Restore bond data after reconnection with a trusted device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] set_lid       Coordinated Set local index
 * @param[in] locked        Indicate if the peer device is the device for which lock has been granted
 *
 * @return none
 ****************************************************************************************
 */
int app_atc_csism_restore_bond_data(uint8_t con_lid, uint8_t set_lid, bool locked);

/*****************************Custom APIs For Audio Content Control******************************************/
/* NOTICE: ACC services are init on mobile phone, mobile is GATT Server, earbuds are GATT Client*/
/* Media Control APIs for earbuds*/
int app_acc_mcc_get(uint8_t con_lid, uint8_t media_lid, uint8_t char_type);
int app_acc_mcc_get_cfg(uint8_t con_lid, uint8_t media_lid, uint8_t char_type);
int app_acc_mcc_set_cfg(uint8_t con_lid, uint8_t media_lid, uint8_t char_type, uint8_t enable);
int app_acc_mcc_set(uint8_t con_lid, uint8_t media_lid, uint8_t char_type, int32_t val);
int app_acc_mcc_set_obj_id(uint8_t con_lid, uint8_t media_lid, uint8_t char_type, uint8_t *obj_id);
int app_acc_mcc_control(uint8_t con_lid, uint8_t media_lid, uint8_t opcode, uint32_t val);
int app_acc_mcc_search(uint8_t con_lid, uint8_t media_lid, uint8_t param_len, uint8_t *param);

#ifdef AOB_MOBILE_ENABLED
/* Media Control APIs for mobile*/
int app_acc_mcs_control_req_cfm(uint16_t status, uint8_t media_lid,
                                uint8_t action, int32_t track_pos, int8_t seeking_speed);
int app_acc_mcs_val_get_req_cfm(uint16_t status, uint8_t con_lid, uint8_t media_lid,
                                const uint8_t *p_buf_prepare, uint16_t buf_len);
int app_acc_mcs_val_set_req_cfm(uint16_t status, uint8_t media_lid, uint32_t val);
int app_acc_mcs_set_req(uint8_t media_lid, uint8_t char_type, int32_t val);
int app_acc_mcs_set_obj_id_req(uint8_t media_lid, uint8_t *obj);
int app_acc_mcs_set_player_name_req(uint8_t media_lid, uint8_t *name, uint8_t name_len);
int app_acc_mcs_action_req(uint8_t media_lid, uint8_t action,
                           int32_t track_pos, int8_t seeking_speed);

/// callback event APP_GAF_MCC_TRACK_CHANGED_IND
int app_acc_mcs_track_change_req(uint8_t media_lid, int32_t track_dur,
                                 uint8_t *title, uint8_t title_len);
#endif

/* Object Transfer APIs for earbuds*/
int app_acc_otc_get(uint8_t con_lid, uint8_t transfer_lid, uint8_t get_type, uint8_t char_type);
int app_acc_otc_get_cfg(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type);
int app_acc_otc_set_cfg(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type, uint8_t enable);
int app_acc_otc_set_name(uint8_t con_lid, uint8_t transfer_lid, uint8_t *name, uint8_t name_len);
int app_acc_otc_set_time(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type, app_gaf_prf_date_time_t *time);
int app_acc_otc_set_properties(uint8_t con_lid, uint8_t transfer_lid, uint32_t properties);
int app_acc_otc_object_create(uint8_t con_lid, uint8_t transfer_lid, uint8_t size, uint16_t uuid);
int app_acc_otc_object_control(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode);
int app_acc_otc_object_manipulate(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode,
                                  uint32_t offset, uint32_t length, uint8_t mode);
int app_acc_otc_object_execute(uint8_t con_lid, uint8_t transfer_lid, uint8_t param_len, uint8_t *param);
int app_acc_otc_list_control(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode, uint8_t order);
int app_acc_otc_list_goto(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode, uint8_t *object_id);
int app_acc_otc_list_filter_set(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid, uint8_t filter_val);
int app_acc_otc_list_filter_set_time(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid,
                                     uint8_t filter_val, app_gaf_prf_date_time_t *time_start, app_gaf_prf_date_time_t *time_end);
int app_acc_otc_list_filter_set_size(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid,
                                     uint8_t filter_val, uint32_t size_min, uint32_t size_max);
int app_acc_otc_list_filter_set_name(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid,
                                     uint8_t filter_val, uint32_t *name, uint32_t name_len);
int app_acc_otc_list_filter_set_type(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid, uint16_t uuid);
int app_acc_otc_coc_connect(uint8_t con_lid, uint16_t local_max_sdu);
int app_acc_otc_coc_disconnect(uint8_t con_lid);
int app_acc_otc_coc_send(uint8_t con_lid, uint8_t length, uint8_t *sdu);
int app_acc_otc_coc_release(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Transfer Object content data through a LE Credit Based Connection Oriented Link
 *
 * @param[in] con_lid       Connection local index
 * @param[in] spsm          Simplified Protocol/Service Multiplexer
 * @param[in] length        SDU length
 * @param[in] p_sdu         Pointer to SDU to be transferred to the peer device
 *
 * @return An error status
 ****************************************************************************************
 */
void app_acc_dts_coc_send(uint8_t con_lid, uint16_t spsm, uint16_t length, const uint8_t *sdu);

/**
 ****************************************************************************************
 * @brief Disconnect a LE Credit Based Connection Oriented Link
 *
 * @param[in] con_lid       Connection local index
 * @param[in] spsm          Simplified Protocol/Service Multiplexer
 *
 * @return An error status
 ****************************************************************************************
 */
void app_acc_dts_coc_disconnect(uint8_t con_lid, uint16_t spsm);

/**
 ****************************************************************************************
 * @brief Release buffers which have been consumed by preceding data reception triggered
 * by OTS_DATA indication.
 *
 * @param[in] con_lid       Connection local index
 * @param[in] spsm          Simplified Protocol/Service Multiplexer
 *
 * @return An error status
 ****************************************************************************************
 */
void app_acc_dts_coc_release(uint8_t con_lid, uint16_t spsm);

/**
 ****************************************************************************************
 * @brief Configure spsm value.
 *
 * @param[in] spsm             Simplified Protocol/Service Multiplexer
 * @param[in] initial_credits  Initial credits
 *
 * @return An error status
 ****************************************************************************************
 */
void app_acc_dts_coc_register_spsm(uint16_t spsm, uint16_t initial_credits);

#ifdef AOB_MOBILE_ENABLED
/**
 ****************************************************************************************
 * @brief Establish a LE Credit Based Connection Oriented Link
 *
 * @param[in] con_lid       Connection local index
 * @param[in] local_max_sdu Maximum SDU size that the local device can receive
 * @param[in] spsm          Simplified Protocol/Service Multiplexer
 *
 * @return An error status
 ****************************************************************************************
 */
void app_acc_dtc_coc_connect(uint8_t con_lid, uint16_t local_max_sdu, uint16_t spsm);

/**
 ****************************************************************************************
 * @brief Disconnect a LE Credit Based Connection Oriented Link
 *
 * @param[in] con_lid       Connection local index
 * @param[in] spsm          Simplified Protocol/Service Multiplexer
 *
 * @return An error status
 ****************************************************************************************
 */
void app_acc_dtc_coc_disconnect(uint8_t con_lid, uint16_t spsm);

/**
 ****************************************************************************************
 * @brief Transfer data through a LE Credit Based Connection Oriented Link
 *
 * @param[in] con_lid       Connection local index
 * @param[in] spsm          Simplified Protocol/Service Multiplexer
 * @param[in] length        SDU length
 * @param[in] sdu           Pointer to SDU to be transferred to the peer device
 *
 * @return An error status
 * ****************************************************************************************
 */
void app_acc_dtc_coc_send(uint8_t con_lid, uint16_t spsm, uint16_t length, uint8_t *sdu);

/**
 ****************************************************************************************
 * @brief Release buffers which have been consumed by preceding data reception triggered
 * by DTC_DATA indication
 *
 * @param[in] con_lid       Connection local index
 * @param[in] spsm          Simplified Protocol/Service Multiplexer
 *
 * @return An error status
 ****************************************************************************************
 */
void app_acc_dtc_coc_release(uint8_t con_lid, uint16_t spsm);

/* Object Transfer APIs for mobile*/
int app_acc_ots_coc_disconnect(uint8_t con_lid);
int app_acc_ots_coc_send(uint8_t con_lid, uint8_t length, uint8_t *sdu);
int app_acc_ots_coc_release(uint8_t con_lid);
int app_acc_ots_obj_add(uint8_t mc_obj_idx, uint32_t allocated_size, uint16_t uuid);
int app_acc_ots_obj_remove(uint8_t object_lid);
int app_acc_ots_obj_change(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid);
int app_acc_ots_obj_changed(uint8_t flags, uint8_t *object_id);
int app_acc_ots_set(uint8_t object_lid, uint8_t set_type, uint32_t val);
int app_acc_ots_set_time(uint8_t object_lid, app_gaf_prf_date_time_t *time);
#endif

/* Call Control APIs for earbuds */
/**
 * @brief Read characteristic value
 *
 * @param con_lid       BLE connection local ID
 * @param bearer_lid    Bearer local ID
 * @param char_type     char_type < ACC_TB_CHAR_TYPE_MAX)
 *                      char_type != ACC_TB_CHAR_TYPE_CALL_CTL_PT
 *                      char_type != ACC_TB_CHAR_TYPE_TERM_REASON
 */
int app_acc_tbc_get_char_value(uint8_t con_lid, uint8_t bearer_lid, uint8_t char_type);

/**
 * @brief Configure characteristic for notification
 *
 * @param con_lid       BLE connection local ID
 * @param bearer_lid    Bearer local ID
 * @param char_type     Characteristic type, @see enum acc_tb_char_type
 *                      char_type < ACC_TB_NTF_CHAR_TYPE_MAX
 *                      char_type != ACC_TB_CHAR_TYPE_TERM_REASON
 * @param enable        Enable/disable
 */
int app_acc_tbc_set_cfg(uint8_t con_lid, uint8_t bearer_lid, uint8_t char_type, uint8_t enable);

/**
 * @brief Get configuration for notification
 *
 * @param con_lid       BLE connection local ID
 * @param bearer_lid    Bearer local ID
 * @param char_type     Characteristic type, @see enum acc_tb_char_type
 */
int app_acc_tbc_get_cfg(uint8_t con_lid, uint8_t bearer_lid, uint8_t char_type);

/**
 * @brief Set report interval
 *
 * @param con_lid       BLE connection local ID
 * @param bearer_lid    Bearer local ID
 * @param interval      Interval in second
 */
int app_acc_tbc_set_rpt_intv(uint8_t con_lid, uint8_t bearer_lid, uint8_t interval, uint8_t reliable);

/**
 * @brief Originate a call
 *
 * @param con_lid       BLE connection local ID
 * @param bearer_lid    Bearer local ID
 * @param uri           Pointer of URI
 * @param uri_len       Length of URI
 */
int app_acc_tbc_call_outgoing(uint8_t con_lid, uint8_t bearer_lid, uint8_t *uri, uint8_t uri_len);

/**
 * @brief Handle a incoming call
 *
 * @param con_lid       BLE connection local ID
 * @param bearer_lid    Bearer local ID
 * @param call_id       Call ID
 * @param opcode        ACC_TB_CHAR_TYPE_CALL_CTL_PT:
 *                      opcode != ACC_TB_OPCODE_ORIGINATE
 *                      opcode != ACC_TB_OPCODE_JOIN
 */
int app_acc_tbc_call_action(uint8_t con_lid, uint8_t bearer_lid, uint8_t call_id, uint8_t opcode);

/**
 * @brief Join two or more calls
 *
 * @param con_lid       BLE connection local ID
 * @param bearer_lid    Bearer local ID
 * @param nb_calls      Number of calls to join
 * @param call_ids      Pointer of call IDs
 */
int app_acc_tbc_call_join(uint8_t con_lid, uint8_t bearer_lid, uint8_t nb_calls, uint8_t *call_ids);

#ifdef AOB_MOBILE_ENABLED
/**
 * @brief Add instance for telephone bearer service
 *
 * @param ccid          CCID to add
 * @param uci           UCI pointer
 * @param uci_len       Length of UCI string
 */
int app_acc_tbs_add_req(uint8_t ccid, uint8_t *uci, uint8_t uci_len);

/**
 * @brief Enable telephone bearer service
 *
 */
int app_acc_tbs_enable(void);

/* Call Control APIs for mobile */
/* app_acc_tbs_set:
char_type:
ACC_TB_CHAR_TYPE_SIGN_STRENGTH (< ACC_TB_SIGN_STRENGTH_MAX)
ACC_TB_CHAR_TYPE_TECHNO: see (enum acc_tb_techno)
*/
int app_acc_tbs_set_req(uint8_t bearer_lid, uint8_t char_type, uint8_t val);

/* app_acc_tbs_set_status: set ACC_TB_CHAR_TYPE_STATUS_FLAGS
status_type:
/// Inband ringtone
ACC_TBS_STATUS_TYPE_INBAND_RINGTONE = 0,
/// Silent mode
ACC_TBS_STATUS_TYPE_SILENT_MODE,
*/
int app_acc_tbs_set_status_req(uint8_t bearer_lid, uint8_t status_type, uint8_t val);

/* char_type:
ACC_TB_CHAR_TYPE_PROV_NAME
ACC_TB_CHAR_TYPE_URI_SCHEMES_LIST
*/
int app_acc_tbs_set_long_req(uint8_t bearer_lid, uint8_t char_type, uint8_t *val, uint8_t len);

/**
 * @brief Inform local stack and peer device(ex. earbuds) the incoming call information
 *
 * @param bearer_lid    bearer local ID
 * @param val           pointer of incoming call information, sequentially include URI, TGT_URI, FRIENDLY_NAME
 * @param uri_len       URI length
 * @param tgt_uri_len   TGT_URI length
 * @param friendly_name_len FRIENDLY_NAME length
 */
int app_acc_tbs_call_incoming_req(uint8_t bearer_lid,  uint8_t *val, uint8_t uri_len,
                                  uint8_t tgt_uri_len, uint8_t friendly_name_len);

/**
 * @brief Inform local stack and peer device(ex. earbuds) the outgoing call information
 *
 * @param bearer_lid    bearer local ID
 * @param val           pointer of incoming call information, sequentially include URI, FRIENDLY_NAME
 * @param uri_len       URI length
 * @param friendly_name_len FRIENDLY_NAME length
 */
int app_acc_tbs_call_outgoing_req(uint8_t bearer_lid,  uint8_t *val,
                                  uint8_t uri_len, uint8_t friendly_name_len);

/**
 * @brief Inform an call related action to local stack and peer device
 *
 * @param bearer_lid    bearer local ID, cmd response value of command @see ACC_TBS_ADD
 * @param call_id       call ID, cmd response value of command @see ACC_TBS_CALL_INCOMING
 * @param action        call action, @see enum acc_tbs_call_action
 * @param reason        reason of action, @see enum acc_tb_term_reason, only useful when action is ACC_TBS_ACTION_TERMINATE
 */
int app_acc_tbs_call_action_req(uint8_t bearer_lid,  uint8_t call_id,
                                uint8_t action, uint8_t reason);

/**
 * @brief Inform local stack and peer device that join the call
 *
 * @param bearer_lid    bearer local ID, cmd response value of command @see ACC_TBS_ADD
 * @param nb_calls      number of call
 * @param call_ids      Pointer of the call ID
 */
int app_acc_tbs_call_join_req(uint8_t bearer_lid,  uint8_t nb_calls, uint8_t *call_ids);
#endif

/**
 * @brief Register GAF event report handler
 *
 * @param handler       GAF event report to register, must be the pointer of structure @see GAF_EVT_REPORT_BUNDLE_T
 */
void app_gaf_evt_report_register(void *handler);

/**
 * @brief Test mode setup of client
 *
 * @param enable       Test mode enable flag
 */
int app_bap_uc_cli_set_test_mode(bool enable);

/**
 * @brief Test mode setup of server
 *
 * @param enable       Test mode enable flag
 */
int app_bap_uc_srv_set_test_mode(bool enable);

#ifdef __cplusplus
}
#endif

#endif
#endif // APP_GAF_CUSTOM_API_H_

/// @} APP_GAF_CUSTOM_API_H_
