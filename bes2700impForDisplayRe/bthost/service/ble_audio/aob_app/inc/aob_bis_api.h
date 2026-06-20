/**
 * @file aob_bis_api.h
 * @author BES AI team
 * @version 0.1
 * @date 2020-08-31
 *
 * @copyright Copyright (c) 2015-2021 BES Technic.
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
 */

#ifndef __AOB_BIS_API_H__
#define __AOB_BIS_API_H__

#ifdef __cplusplus
extern "C" {
#endif
/*****************************header include********************************/
#include "ble_audio_define.h"
#include "aob_mgr_gaf_evt.h"

/******************************macro defination*****************************/

/******************************type defination******************************/
typedef struct
{
    uint8_t                    bcast_id[3];
    // Indicate if streams are encrypted (!= 0) or not
    uint8_t                    encrypted;
    // Broadcast code. Meaningful only if encrypted parameter indicates that streams are encrypted
    uint8_t                    bcast_code[16];
    /// Number of Streams in the Broadcast Group. Cannot be 0
    uint8_t                    nb_streams;
    /// Number of Subgroups in the Broadcast Group. Cannot be 0
    uint8_t                    nb_subgroups;
    /// SDU interval in microseconds
    /// From 256us (0x00000100) to 1.048575s (0x000FFFFF)
    uint32_t                   sdu_intv_us;
    /// Maximum size of an SDU
    /// From 1 to 4095 bytes
    uint16_t                   max_sdu;
    // Audio output presentation delay in microseconds
    uint32_t                   pres_delay_us;
    // creat test big hci cmd, 0:creat BIG cmd, 1:creat BIG test cmd
    uint8_t                    test;

    //// test = 0, set this param
    struct
    {
        /// Maximum time (in milliseconds) between the first transmission of an SDU to the end of the last transmission
        /// of the same SDU
        /// From 0ms to 4.095s (0x0FFF)
        uint16_t                   max_tlatency_ms;
        /// Number of times every PDU should be transmitted
        /// From 0 to 15
        uint8_t                    rtn;
    } big_param;

    //// test = 1, set this param
    struct
    {
        //// test = 1, set this param
        /// ISO interval in multiple of 1.25ms. From 0x4 (5ms) to 0xC80 (4s)
        uint16_t                   iso_intv_frame;
        /// Number of subevents in each interval of each stream in the group
        uint8_t                    nse;
        /// Maximum size of a PDU
        uint8_t                    max_pdu;
        /// Burst number (number of new payload in each interval). From 1 to 7
        uint8_t                    bn;
        /// Number of times the scheduled payload is transmitted in a given event. From 0x1 to 0xF
        uint8_t                    irc;
        /// Isochronous Interval spacing of payloads transmitted in the pre-transmission subevents.
        /// From 0x00 to 0x0F
        uint8_t                    pto;
    } test_big_param;

    uint8_t adv_data_len;
    uint8_t *adv_data;
} aob_bis_src_big_param_t;

typedef struct
{
    /// Subgroup local identifier
    uint8_t                  sgrp_lid;
    /// Codec ID
    uint8_t                  codec_id[5];

    ///LTV cfg info
    uint32_t                 location_bf;
    /// Length of a codec frame in octets
    uint16_t                 frame_octet;
    /// Sampling Frequency (see #bap_sampling_freq enumeration)
    uint8_t                  sampling_freq;
    /// Frame Duration (see #bap_frame_dur enumeration)
    uint8_t                  frame_dur;
    /// Number of blocks of codec frames that shall be sent or received in a single SDU
    uint8_t                  frames_sdu;

    ///LTV media data
    /// Streaming Audio Contexts bit field (see #enum bap_context_type_bf enumeration)
    uint16_t                 context_bf;

    uint8_t                  add_metadata_len;
    uint8_t*                 add_metadata_data;
} aob_bis_src_subgrp_param_t;

typedef struct
{
    /// Stream local identifier
    uint8_t                  stream_lid;
    /// Subgroup local identifier
    uint8_t                  sgrp_lid;
    /// LTV cfg param
    /// When received, 0 shall be interpreted as a single channel with no specified Audio Location
    uint32_t                 location_bf;
    /// Length of a codec frame in octets
    uint16_t                 frame_octet;
    /// Sampling Frequency (see #bap_sampling_freq enumeration)
    uint8_t                  sampling_freq;
    /// Frame Duration (see #bap_frame_dur enumeration)
    uint8_t                  frame_dur;
    /// Number of blocks of codec frames that shall be sent or received in a single SDU
    uint8_t                  frames_sdu;
} aob_bis_src_stream_param_t;

typedef struct
{
    uint16_t sampling_freq;
    uint16_t frame_octet;
} AOB_BIS_MEDIA_INFO_T;

typedef struct
{
    uint32_t big_trans_latency;
} aob_bis_src_started_param_t;


/// sync see@aob_bis_src_stream_param_t
typedef struct
{
    void (*bis_stream_read_data_cb)(uint8_t stream_id, uint8_t **data, uint16_t *data_len, uint8_t cache_num);
    void (*bis_stream_get_buf_free)(uint8_t stream_id, uint8_t *data);
    void (*bis_stream_start_ind)(uint8_t stream_id, aob_bis_src_started_param_t *start_bis_info);
    void (*bis_stream_stop_ind)(uint8_t stream_id);
} aob_bis_src_started_info_t;

/// sync see@aob_bis_src_stream_param_t
typedef struct
{
    uint32_t ch_bf;
    uint8_t *bc_id;
    uint8_t *bc_code;
    struct
    {
        void (*bis_sink_scan_state_cb)(bool scan_or_pa_sync, bool started, uint32_t param);
        bool (*bis_sink_select_source)(ble_bdaddr_t *addr, uint8_t adv_sid, uint8_t *bcast_id,
                                            uint8_t *adv_data, uint8_t adv_data_len, int8_t rssi);
        void (*bis_sink_started_callback)(uint8_t grp_lid);
        void (*bis_sink_stoped_callback)(uint8_t grp_lid, uint16_t err_code);
        void (*bis_sink_metadata_callback)(uint8_t subgrp_lid, uint8_t *buf, uint8_t buf_len);
    } event_callback;
} aob_bis_sink_start_param_t;


/****************************function declaration***************************/

/*BAP Broadcast Source APIs*/
/**
 ****************************************************************************************
 * @brief Set BIG encrypt configuration.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] is_encrypted     0:Not encrypted, !=0:encrypted
 * @param[in] bcast_code       Broadcast Code, @see app_gaf_bc_code_t, only meaningful when is_encrypted != 0
 *
 ****************************************************************************************
 */
void aob_bis_src_set_id_key(uint8_t big_idx, uint8_t *bcast_id, uint8_t *bcast_code);

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
void aob_bis_src_set_stream_codec_cfg(uint8_t big_idx, uint8_t stream_lid, uint16_t frame_octet, uint8_t sampling_freq);

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
void aob_bis_src_update_stream_codec_cfg(uint8_t big_idx, uint8_t stream_lid, app_gaf_bap_cfg_t *p_cfg);

/**
 ****************************************************************************************
 * @brief Update Group info specified by big_idx
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 * @param[in] nb_subgrp        Number of subgrps
 * @param[in] nb_stream        Number of streams
 * @param[in] sdu_intvl        SDU interval
 * @param[in] max_sdu_size     Max sdu size
 *
 ****************************************************************************************
 */
void aob_bis_src_update_src_group_info(uint8_t big_idx, uint8_t nb_subgrp, uint8_t nb_stream, uint32_t sdu_intvl, uint16_t max_sdu_size);

void aob_bis_src_set_big_param(uint8_t big_idx, aob_bis_src_big_param_t *p_big_param);

void aob_bis_src_set_subgrp_param(uint8_t big_idx, aob_bis_src_subgrp_param_t *p_subgrp_param);

void aob_bis_src_set_stream_param(uint8_t big_idx, aob_bis_src_stream_param_t *p_stream_param);

void aob_bis_src_write_bis_data(uint8_t big_idx, uint8_t stream_lid, uint8_t *data, uint16_t data_len);

uint32_t aob_bis_src_get_anchor_time(uint8_t big_idx, uint8_t stream_idx);

void aob_bis_src_start(uint8_t big_idx, aob_bis_src_started_info_t *start_info);

void aob_bis_src_stop(uint8_t big_idx);

uint16_t aob_bis_src_get_bis_hdl_by_big_idx(uint8_t big_idx);

const AOB_CODEC_ID_T *aob_bis_src_get_codec_id_by_big_idx(uint8_t big_idx, uint8_t subgrp_idx);

const AOB_BAP_CFG_T *aob_bis_src_get_codec_cfg_by_big_idx(uint8_t big_idx);

uint32_t aob_bis_src_get_iso_interval_ms_by_big_idx(uint8_t big_idx);

/**
 ****************************************************************************************
 * @brief Enable Periodic Advertising for a Broadcast Group.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 *
 ****************************************************************************************
 */
void aob_bis_src_enable_pa(uint8_t big_idx);

/**
 ****************************************************************************************
 * @brief Disable Periodic Advertising for a Broadcast Group.
 *
 * @param[in] grp_lid         Group local index
 *
 ****************************************************************************************
 */
void aob_bis_src_disable_pa(uint8_t grp_lid);

/**
 ****************************************************************************************
 * @brief Enable a Broadcast Group.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 *
 ****************************************************************************************
 */
void aob_bis_src_enable(uint8_t big_idx);

/**
 ****************************************************************************************
 * @brief Disable a Broadcast Group.
 *
 * @param[in] grp_lid         Group local index
 *
 ****************************************************************************************
 */
void aob_bis_src_disable(uint8_t grp_lid);

/**
 ****************************************************************************************
 * @brief Add a BIG to BIS source.
 *
 * @param[in] big_idx          BIS Group(BIG) local index
 *
 ****************************************************************************************
 */
void aob_bis_src_add_group_req(uint8_t big_idx);

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
void aob_bis_src_start_streaming(uint8_t big_idx);

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
void aob_bis_src_update_metadata(uint8_t grp_lid, uint8_t sgrp_lid, app_gaf_bap_cfg_metadata_t *metadata);

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
void aob_bis_src_stop_streaming(uint8_t big_idx);

void aob_bis_src_api_init(void);

/*BAP Broadcast Sink APIs*/
const app_gaf_bap_adv_id_t *aob_bis_sink_get_pa_addr_info(uint8_t pa_lid);

uint16_t aob_bis_sink_get_pa_sync_hdl_by_pa_lid(uint8_t pa_lid);

void aob_bis_sink_set_player_channel(uint32_t channel_bf);

void aob_bis_sink_start(aob_bis_sink_start_param_t *param);

void aob_bis_sink_stop();

void aob_bis_restart_deleg_bis_sink(void);

/**
 ****************************************************************************************
 * @brief Enable a group of sink streams.
 *
 * @param[in]  pa_lid          Periodic Advertising local index
 * @param[in]  mse             Maximum number of subevents the controller should use to receive data payloads in each interval
 * @param[in]  stream_pos_bf   Stream position bit field indicating streams to synchronize with.
 * @param[in]  timeout_10ms    Timeout duration (10ms unit) before considering synchronization lost (Range 100 ms to 163.84 s).
 * @param[in]  encrypted       Indicate if streams are encrypted (!= 0) or not
 * @param[in]  bcast_code      Broadcast Code value
 * @param[in]  bcast_id        Broadcast ID
 *
 ****************************************************************************************
 */
void aob_bis_sink_enable(uint8_t pa_lid, uint8_t mse, uint8_t stream_pos_bf,
                         uint16_t timeout_10ms, uint8_t encrypted, uint8_t *bcast_code,
                         uint8_t *bcast_id);

/**
 ****************************************************************************************
 * @brief Disable a group of sink streams.
 * @param[in] grp_lid         Group local index
 *
 ****************************************************************************************
 */
void aob_bis_sink_disable(uint8_t grp_lid);

/**
 ****************************************************************************************
 * @brief Set src id or bcast code of sink streams.
 * @param[in] bcast_id        Broadcast ID
 * @param[in] bcast_code      Broadcast Code value
 *
 ****************************************************************************************
 */
void aob_bis_sink_set_src_id_key(uint8_t *bcast_id, uint8_t *bcast_code);

/**
 ****************************************************************************************
 * @brief Media start sink scan.
 *
 ****************************************************************************************
 */
void aob_bis_start_scan(void);

/**
 ****************************************************************************************
 * @brief Set the broadcast id of scan
 * @param[in] bcast_id         broadcast id, length=3
 * @param[in] addr             broadcast source addr, length=6
 ****************************************************************************************
 */
void aob_bis_scan_set_src_info(uint8_t *bcast_id, uint8_t *addr);

/**
 ****************************************************************************************
 * @brief Start periodic advertising synchronize.
 *          Expected callback event: APP_GAF_SCAN_PA_ESTABLISHED_IND.
 * @param[in] addr         BD Address of device
 * @param[in] addr_type    Address type of the device 0=public/1=private random
 * @param[in] adv_sid      Advertising SID
 * @param[in] to_s         sync timeout in second
 ****************************************************************************************
 */
void aob_bis_scan_pa_sync_with_to(uint8_t *addr, uint8_t addr_type, uint8_t adv_sid, uint16_t sync_to_s);

/**
 ****************************************************************************************
 * @brief Stop periodic advertising synchronize.
 ****************************************************************************************
 */
void aob_bis_scan_pa_sync_stop(void);

/**
 ****************************************************************************************
 * @brief Start periodic advertising synchronize.
 *          Expected callback event: APP_GAF_SCAN_PA_ESTABLISHED_IND.
 * @param[in] addr         BD Address of device
 * @param[in] addr_type    Address type of the device 0=public/1=private random
 * @param[in] adv_sid      Advertising SID
 ****************************************************************************************
 */
void aob_bis_scan_pa_sync(uint8_t *addr, uint8_t addr_type, uint8_t adv_sid);

/**
 ****************************************************************************************
 * @brief Cancel periodic advertising synchronize.
 ****************************************************************************************
 */
void aob_bis_scan_pa_sync_cancel(void);

/**
 ****************************************************************************************
 * @brief Enable or disable periodic advertising report.
 * @param[in] pa_lid         Periodic advertising local index
 * @param[in] enable         Enable or disable report
 *
 ****************************************************************************************
 */
void aob_bis_scan_pa_report_ctrl(uint8_t pa_lid, bool enable);

/**
 ****************************************************************************************
 * @brief Terminate periodic advertising synchronize.
 *          Expected callback event: APP_GAF_SCAN_PA_TERMINATED_IND.
 * @param[in] pa_lid         Periodic advertising local index
 *
 ****************************************************************************************
 */
void aob_bis_sink_scan_pa_terminate(uint8_t pa_lid);

/**
 ****************************************************************************************
 * @brief Start one of a group of sink streams.
 *          Expected callback event: APP_GAF_SINK_BIS_STREAM_STARTED_IND.
 * @param[in] grp_lid         Group local index
 * @param[in] stream_pos_bf   Stream position bit field indicating streams to synchronize with
 * @param[in] codec_type      Codec ID value
 * @param[in] media_info      @see AOB_BIS_MEDIA_INFO_T
 ****************************************************************************************
 */
void aob_bis_sink_start_streaming(uint8_t grp_lid, uint32_t stream_pos_bf,
                                  uint8_t codec_type, AOB_BIS_MEDIA_INFO_T *media_info);

/**
 ****************************************************************************************
 * @brief Stop one of a group of sink streams.
 *          Expected callback event: APP_GAF_SINK_BIS_STREAM_STOPPED_IND.
 * @param[in] grp_lid         Sink streams group local index
 * @param[in] stream_pos      Stream position in group
 *
 ****************************************************************************************
 */
void aob_bis_sink_stop_streaming(uint8_t grp_lid, uint8_t stream_pos);

/**
 ****************************************************************************************
 * @brief set bis sink objective source ID and CODE
 * @param[in] bcast_id        objective source ID
 * @param[in] bcast_code      objective source CODE
 *
 ****************************************************************************************
 */
void aob_bis_sink_set_src_id_key(uint8_t *bcast_id, uint8_t *bcast_code);

/**
 ****************************************************************************************
 * @brief Init sink status cb
 *
 ****************************************************************************************
 */
void aob_bis_sink_api_init(void);

/*BAP Broadcast Scan APIs*/
/**
 ****************************************************************************************
 * @brief Media set scan param.
 *
 * @param[in] intv_1m_slot      Scan interval for LE 1M PHY in multiple of 0.625ms - Must be higher than 2.5ms
 * @param[in] intv_coded_slot   Scan interval for LE Codec PHY in multiple of 0.625ms - Must be higher than 2.5ms
 * @param[in] wd_1m_slot        Scan window for LE 1M PHY in multiple of 0.625ms - Must be higher than 2.5ms
 * @param[in] wd_coded_slot     Scan window for LE Codec PHY in multiple of 0.625ms - Must be higher than 2.5ms
 ****************************************************************************************
 */
void aob_bis_scan_set_scan_param(uint16_t scan_timeout_s, uint16_t intv_1m_slot,
                                 uint16_t intv_coded_slot, uint16_t wd_1m_slot, uint16_t wd_coded_slot);

/**
 ****************************************************************************************
 * @brief Media stop scan.
 *
 ****************************************************************************************
 */
void aob_bis_stop_scan(void);

/**
 ****************************************************************************************
 * @brief Send TWS sync bis state request
 *
 ****************************************************************************************
 */
void aob_bis_tws_sync_state_req(void);

/**
 ****************************************************************************************
 * @brief Process bis sync requests
 *
 ****************************************************************************************
 */
void aob_bis_tws_sync_state_req_handler(uint8_t *buf);

/**
 ****************************************************************************************
 * @brief Init scan status cb
 *
 ****************************************************************************************
 */
void aob_bis_scan_api_init(void);

/*BAP Broadcast Deleg APIs*/
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
void aob_bis_deleg_start_solicite(uint16_t timeout_s, uint32_t context_bf);

/**
 ****************************************************************************************
 * @brief Stop solicite advertising.
 *          Expected callback event: APP_GAF_DELEG_SOLICITE_STOPPED_IND.
 *
 ****************************************************************************************
 */
void aob_bis_deleg_stop_solicite(void);

/**
 ****************************************************************************************
 * @brief Confirm bis sync pref.
 *
 *
 ****************************************************************************************
 */
void aob_bis_deleg_cfm_bis_sync_pref(uint8_t src_lid, uint32_t bis_sync_pref);

/**
 ****************************************************************************************
 * @brief Add source local
 *
 *
 ****************************************************************************************
 */
void aob_bis_deleg_add_source(app_gaf_bap_adv_id_t *addr,
                              const app_gaf_bap_bcast_id_t *p_bcast_id,
                              uint8_t nb_subgroups, uint8_t *add_metadata,
                              uint8_t add_metadata_len, uint16_t streaming_context_bf,
                              uint32_t bis_sync_bf_deprecated);

/**
 ****************************************************************************************
 * @brief Terminate periodic advertising synchronize req.
 *          Expected callback event: BAP_BC_SCAN_PA_SYNCHRONIZE_RI.
 * @param[in] pa_lid         Periodic advertising local index
 * @param[in] accept         false: not accept, true: accept
 *
 ****************************************************************************************
 */
void aob_bis_deleg_pa_sync_ri(uint8_t pa_lid, bool accept);

/**
 ****************************************************************************************
 * @brief Terminate periodic advertising synchronize req.
 *          Expected callback event: BAP_BC_SCAN_PA_TERMINATE_RI.
 * @param[in] pa_lid         Periodic advertising local index
 * @param[in] accept         false: not accept, true: accept
 *
 ****************************************************************************************
 */
void aob_bis_deleg_pa_terminate_ri(uint8_t pa_lid, bool accept);

void aob_bis_deleg_api_init(void);

/*BAP Broadcast Assist APIs*/
/**
 ****************************************************************************************
 * @brief Media start assitant scan.
 *
 * @param[in] con_lid         Connection local index
 ****************************************************************************************
 */
void aob_bis_assist_scan_bc_src(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Start scanning for solicitation.
 *          Expected callback event: APP_GAF_SCAN_REPORT_IND and APP_GAF_SCAN_PA_ESTABLISHED_IND.
 * @param[in] timeout_s         Scanning time duration, Unit:s
 *
 ****************************************************************************************
 */
void aob_bis_assist_start_scan(uint16_t timeout_s);

/**
 ****************************************************************************************
 * @brief Stop scanning for solicitation.
 *          Expected callback event: APP_GAF_SCAN_STOPPED_IND.
 *
 ****************************************************************************************
 */
void aob_bis_assist_stop_scan(void);

/**
 ****************************************************************************************
 * @brief Assist finds BASS service of the peer device
 *
 * @param[in] con_lid         Connection local index
 ****************************************************************************************
 */
void aob_bis_assist_discovery(uint8_t con_lid);


/**
 ****************************************************************************************
 * @brief Assist initiate add source operation
 *
 * @param[in] con_lid          con_lid
 * @param[in] p_adv_id         p_adv_id
 * @param[in] p_bcast_id       p_bcast_id
 * @param[in] pa_sync          pa_sync
 * @param[in] nb_subgrp        nb_subgrp
 * @param[in] bis_sync_bf      bis_sync_bf
 ****************************************************************************************
 */
void aob_bis_assist_add_source(uint8_t con_lid, const app_gaf_bap_adv_id_t *p_adv_id, const uint8_t *p_bcast_id,
                               uint8_t pa_sync, uint8_t nb_subgrp, uint32_t bis_sync_bf);

/**
 ****************************************************************************************
 * @brief Assist initiate remove source operation
 *
 * @param[in] src_id            Broadcast ID
 * @param[in] src_key           Broadcast Code
 ****************************************************************************************
 */
void aob_bis_assist_set_src_id_key(uint8_t *src_id, uint8_t *src_key);

/**
 ****************************************************************************************
 * @brief Assist initiate remove source operation
 *
 * @param[in] con_lid          con_lid
 * @param[in] src_lid          src_lid
 ****************************************************************************************
 */
void aob_bis_assist_remove_source(uint8_t con_lid, uint8_t src_lid);


void aob_bis_assist_api_init(void);

#ifdef __cplusplus
}
#endif

#endif
