/**
 * @file aob_media_api.h
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


#ifndef __AOB_MEDIA_API_H__
#define __AOB_MEDIA_API_H__

/*****************************header include********************************/
#include "aob_mgr_gaf_evt.h"
#include "app_gaf_define.h"
#include "app_gaf_custom_api.h"

/******************************macro defination*****************************/

/******************************type defination******************************/

/**********************************************************************************
 * @brief Callback function for codec req ind - confirm use
 *
 * @param[in] direction     codec cfg req's ase direction
 * @param[in] codec_id      codec cfg req's codec id, @see LC3 and more
 * @param[in] tgt_latency   target latency to choose qos setting
 * @param[in] bap_cfg       bap configuration to choose qos setting
 *                      @see app_gaf_bap_cfg_t
 *
 * @param[out] app_gaf_bap_qos_req_t qos req for confirm use @see app_gaf_bap_qos_req_t
 **********************************************************************************/
typedef bool (*get_qos_req_cfg_info_cb)(uint8_t direction, const app_gaf_codec_id_t *codec_id, uint8_t tgt_latency,
                                        app_gaf_bap_cfg_t *codec_cfg_req, app_gaf_bap_qos_req_t *ntf_qos_req);
typedef void (*aob_mobile_ava_ctx_changed_cb)(uint8_t con_lid, uint16_t sink_ava_ctx, uint16_t src_ava_ctx);
/// Metadata data
typedef struct
{
    /// Length of Metadata value
    uint8_t metadata_len;
    /// Metadata value
    uint8_t metadata[__ARRAY_EMPTY];
} AOB_MEDIA_METADATA_CFG_T;


/****************************function declaration***************************/
#ifdef __cplusplus
extern "C" {
#endif

/* MUSIC APIs (Earbuds)*/
void aob_media_mcs_discovery(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Media control get value of specific characteristic.
 *
 * @param[in] device_id     device id
 * @param[in] char_type     characteristic type
 * char_type:
 * char_type < AOB_MC_CHAR_TYPE_MAX)
 * char_type != AOB_MC_CHAR_TYPE_TRACK_CHANGED
 * char_type != AOB_MC_CHAR_TYPE_MEDIA_CP
 * char_type != AOB_MC_CHAR_TYPE_SEARCH_CP
 *
 ****************************************************************************************
 */
void aob_mc_char_type_get(uint8_t device_id, AOB_MGR_MC_CHAR_TYPE_E char_type);

/**
 ****************************************************************************************
 * @brief Media control set value of specific characteristic.
 *
 * @param[in] device_id     device id
 * @param[in] char_type     characteristic type
 * char_type:
 * char_type == ACC_MC_CHAR_TYPE_TRACK_POSITION
 * char_type == ACC_MC_CHAR_TYPE_PLAYBACK_SPEED
 * char_type == ACC_MC_CHAR_TYPE_PLAYING_ORDER
 *
 ****************************************************************************************
 */
void aob_mc_char_type_set(uint8_t device_id, AOB_MGR_MC_CHAR_TYPE_E char_type, int32_t val);

/**
 ****************************************************************************************
 * @brief Media control get configuration for notification.
 *
 * @param[in] device_id     device id
 * @param[in] char_type     characteristic type
 * char_type:
 * char_type < AOB_MC_NTF_CHAR_TYPE_MAX
 *
 ****************************************************************************************
 */
void aob_mc_get_cfg(uint8_t device_id, AOB_MGR_MC_CHAR_TYPE_E char_type);

/**
 ****************************************************************************************
 * @brief Media control configure characteristic for notification.
 *
 * @param[in] device_id     device id
 * @param[in] char_type     characteristic type
 * char_type:
 * char_type < AOB_MC_NTF_CHAR_TYPE_MAX
 * @param[in] enable        Indicate if sending of notifications must be enabled (!=0) or disabled
 *
 ****************************************************************************************
 */
void aob_mc_set_cfg(uint8_t device_id, AOB_MGR_MC_CHAR_TYPE_E char_type, uint8_t enable);

/**
 ****************************************************************************************
 * @brief Media control set object id.
 *
 * @param[in] device_id     device id
 * @param[in] char_type     characteristic type
 * char_type:
 * char_type == ACC_MC_CHAR_TYPE_CUR_TRACK_OBJ_ID
 * char_type == ACC_MC_CHAR_TYPE_NEXT_TRACK_OBJ_ID
 * char_type == ACC_MC_CHAR_TYPE_CUR_GROUP_OBJ_ID
 * @param[in] obj_id        object id
 *
 ****************************************************************************************
 */
void aob_mc_set_obj_id(uint8_t device_id, AOB_MGR_MC_CHAR_TYPE_E char_type, uint8_t *obj_id);

/**
 ****************************************************************************************
 * @brief Media control for client.
 *
 * @param[in] con_lid       connection local id
 * @param[in] media_lid     media player local id
 * @param[in] opcode        media control operation code
 * @param[in] val           operation value
 *
 ****************************************************************************************
 */
void aob_media_mcc_control(uint8_t con_lid, uint8_t media_lid, AOB_MGR_MC_OPCODE_E opcode, uint32_t val);

/**
 ****************************************************************************************
 * @brief Media set Character value for client.
 *
 * @param[in] con_lid       connection local id
 * @param[in] media_lid     media player local id
 * @param[in] char_type     character type
 * @param[in] val           value
 *
 ****************************************************************************************
 */
void aob_media_mcc_set_val(uint8_t con_lid, uint8_t media_lid, uint8_t char_type, uint32_t val);

/**
 ****************************************************************************************
 * @brief Media set objects id for client.
 *
 * @param[in] con_lid       connection local id
 * @param[in] media_lid     media player local id
 * @param[in] char_type     character type
 * @param[in] obj_id        Object ID
 *
 ****************************************************************************************
 */
void aob_media_mcc_set_obj_id(uint8_t con_lid, uint8_t media_lid, uint8_t char_type, uint8_t *obj_id);

/**
 ****************************************************************************************
 * @brief Media control.
 *
 * @param[in] device_id     device id
 * @param[in] opcode        media control operation code
 * @param[in] val           operation value
 *
 ****************************************************************************************
 */
void aob_media_control(uint8_t con_lid, AOB_MGR_MC_OPCODE_E opcode, uint32_t val);

/**
 ****************************************************************************************
 * @brief Media play.
 *
 * @param[in] device_id     device id
 *
 ****************************************************************************************
 */
void aob_media_play(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Media pause.
 *
 * @param[in] device_id     device id
 *
 ****************************************************************************************
 */
void aob_media_pause(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Media stop.
 *
 * @param[in] device_id     device id
 *
 ****************************************************************************************
 */
void aob_media_stop(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Media play next song.
 *
 * @param[in] device_id     device id
 *
 ****************************************************************************************
 */
void aob_media_next(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Media play previous song.
 *
 * @param[in] device_id     device id
 *
 ****************************************************************************************
 */
void aob_media_prev(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Media fast forward.
 *
 * @param[in] device_id     device id
 *
 ****************************************************************************************
 */
void aob_media_fast_fw(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Media rewind.
 *
 * @param[in] device_id     device id
 *
 ****************************************************************************************
 */
void aob_media_fast_rw(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Media control search for media for player.
 *
 * @param[in] con_lid       connection local id
 * @param[in] media_lid     media local id
 * @param[in] param_len     param len
 * @param[in] param         param
 *
 ****************************************************************************************
 */
void aob_media_search_player(uint8_t con_lid, uint8_t media_lid, uint8_t param_len, uint8_t *param);

/**
 ****************************************************************************************
 * @brief Media control search for media.
 *
 * @param[in] device_id     device id
 * @param[in] param_len     param len
 * @param[in] param         param
 *
 ****************************************************************************************
 */
void aob_media_search(uint8_t device_id, uint8_t param_len, uint8_t *param);

/**
 ****************************************************************************************
 * @brief get the current media state
 *
 * @param[in] ase_lid       ASE local index
 *
 * @return AOB_MGR_STREAM_STATE_E
 ****************************************************************************************
 */
AOB_MGR_STREAM_STATE_E aob_media_get_cur_ase_state(uint8_t ase_lid);

/**
 ****************************************************************************************
 * @brief get the ase direction
 *
 * @param ase_lid
 * @return AOB_MGR_DIRECTION_E
 ****************************************************************************************
 */
AOB_MGR_DIRECTION_E aob_media_get_ase_direction(uint8_t ase_lid);

/**
 ****************************************************************************************
 * @brief Send enable cfm to peer device
 *
 * @param[in] ase_lid   ASE local index
 * @param[in] accept    Accept or not
 *
 ****************************************************************************************
 */
void aob_media_send_enable_rsp(uint8_t ase_lid, bool accept);

/**
 ****************************************************************************************
 * @brief Send enable cfm to peer device with stream ctx type rejetced
 *
 * @param[in] ase_lid   ASE local index
 *
 ****************************************************************************************
 */
void aob_media_send_enable_reject_stream_ctx_rsp(uint8_t ase_lid);

/**
 ****************************************************************************************
 * @brief Send update metadata cfm to peer device with reason
 *
 * @param[in] ase_lid   ASE local index
 * @param[in] accept    Accept or not
 * @param[in] reason    reason for only if @param[in] accept == false
 *
 ****************************************************************************************
 */
void aob_media_send_upd_metadata_rsp(uint8_t ase_lid, bool accept, uint8_t reason);

/**
 ****************************************************************************************
 * @brief Release a CIS Stream(Stream stop and CIS disconnected)..
 *
 * @param[in] ase_lid          ASE local index
 * @param[in] switchToIdle
 *                  0   switch to CODEC CONFIGURED state
 *                  1   switch to Idle state
 ****************************************************************************************
 */
void aob_media_release_stream(uint8_t ase_lid);

/**
 ****************************************************************************************
 * @brief Read a iso link quality info.
 *
 * @param[in] ase_lid          ASE local index
 *
 ****************************************************************************************
 */
void aob_media_read_iso_link_quality(uint8_t ase_id);

/**
 ****************************************************************************************
 * @brief Disable a CIS Stream(Stream stop and CIS disconnected).
 *
 * @param[in] ase_lid          ASE local index
 ****************************************************************************************
 */
void aob_media_disable_stream(uint8_t ase_lid);

/**
 ****************************************************************************************
 * @brief Update metadata.
 *
 * @param[in] ase_lid          ASE local index
 * @param[in] meta_data          Metadata for Codec Configuration see @app_bap_metadata_cfg_t
 ****************************************************************************************
 */
void aob_media_update_metadata(uint8_t ase_lid, app_gaf_bap_cfg_metadata_t *meta_data);

/**
 ****************************************************************************************
 * @brief get stream type
 *
 * @param[in] ase_lid             ASE local index
 * @return stream type
 ****************************************************************************************
 */
AOB_MGR_CONTEXT_TYPE_BF_E aob_media_get_cur_context_type(uint8_t ase_lid);

/**
 ****************************************************************************************
 * @brief get ASE local id.
 *
 * @param[in] con_lid              connection local index
 * @param[in] direction            ASE Direction, @see AOB_MGR_DIRECTION_E
 *
 * @return ASE local index
 ****************************************************************************************
 */
uint8_t aob_media_get_cur_streaming_ase_lid(uint8_t con_lid, AOB_MGR_DIRECTION_E direction);

/**
 ****************************************************************************************
 * @brief get streaming ASE local id list
 *
 * @param con_lid
 * @param ase_lid_list
 * @return uint8_t
 ****************************************************************************************
 */
uint8_t aob_media_get_curr_streaming_ase_lid_list(uint8_t con_lid, uint8_t *ase_lid_list);

/**
 ****************************************************************************************
 * @brief get enabling and streaming ASE local id list
 *
 * @param con_lid
 * @param ase_lid_list
 * @return uint8_t
 ****************************************************************************************
 */
uint8_t aob_media_get_ready_for_stream_ase_lid_list(uint8_t con_lid, uint8_t *ase_lid_list);

/**
 ****************************************************************************************
 * @brief Check is any srtc ase in streaming state
 *
 * @param con_lid
 * @return bool
 ****************************************************************************************
 */

uint8_t aob_media_get_enableing_ase_lid_list(uint8_t con_lid, uint8_t *ase_lid_list);
/**
 ****************************************************************************************
 * @brief get enableing ase lid list
 *
 * @param con_lid
 * @return count_cnt
 ****************************************************************************************
 */

bool aob_media_is_device_any_ase_in_streamimg_state(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief get device connection local index that src ASE is in streaming
 *
 * @param p_con_lid_list
 * @return uint8_t
 ****************************************************************************************
 */
uint8_t aob_media_get_src_ase_streaming_device_list(uint8_t *p_con_lid_list);

/**
 ****************************************************************************************
 * @brief get current media state.
 *
 * @param[in] con_lid              connection local index
 *
 * @return media state
 ****************************************************************************************
 */
AOB_MGR_PLAYBACK_STATE_E aob_media_get_state(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief get current MIC state.
 *
 * @param[in] con_lid              connection local index
 *
 * @return MIC state, 1:muted 0:unmute
 ****************************************************************************************
 */
uint8_t aob_media_get_mic_state(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Configure codec.
 *
 * @param[in] ntf_codec_cfg        app_gaf_bap_cfg_t
 * @param[in] ase_lid              ASE local index
 * @param[in] ntf_qos_req          ASE app_gaf_bap_qos_req_t
 * @param[in] codec_id             Codec ID value, @see app_gaf_codec_id_t
 *
 ****************************************************************************************
 */
void aob_media_ascs_srv_set_codec(uint8_t ase_lid, const app_gaf_codec_id_t *codec_id,
                                  app_gaf_bap_qos_req_t *ntf_qos_req, app_gaf_bap_cfg_t *ntf_codec_cfg);

/**
 ****************************************************************************************
 * @brief set ASE qos req params.
 *
 *
 * @param[in] ase_lid              ASE local index
 * @param[in] qos_req              QoS Requirement params send from server to client when
 *                                 ASE enter Codec Confiured State @see app_gaf_bap_qos_req_t
 *
 ****************************************************************************************
 */
void aob_media_set_qos_info(uint8_t ase_lid, app_gaf_bap_qos_req_t *qos_info);

/**
 ****************************************************************************************
 * @brief Microphone Control set mute for mobile
 *
 * @param[in] mute              mute/unmute
 *
 ****************************************************************************************
 */
void aob_media_mics_set_mute(uint8_t mute);

/**
 ****************************************************************************************
 * @brief Microphone Control read mute for mobile
 *
 * @param[in] mute              mute/unmute
 *
 ****************************************************************************************
 */
void aob_media_mobile_micc_read_mute(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Get the codec type of the specified ASE
 *
 * @param[in] ase_lid              ASE local index
 *
 * @return codec type see@APP_GAF_CODEC_TYPE_T
 ****************************************************************************************
 */
APP_GAF_CODEC_TYPE_T aob_media_get_codec_type(uint8_t ase_lid);

/**
 ****************************************************************************************
 * @brief Get the sample rate of the specified ASE
 *
 * @param[in] ase_lid              ASE local index
 *
 * @return sample rate see@gaf_bap_sampling_freq
 ****************************************************************************************
 */
GAF_BAP_SAMLLING_REQ_T aob_media_get_sample_rate(uint8_t ase_lid);

/**
 ****************************************************************************************
 * @brief Get the metadata of the specified ASE
 *
 * @param[in] ase_lid              ASE local index
 *
 * @return metadata see@AOB_MEDIA_METADATA_CFG_T
 ****************************************************************************************
 */
AOB_MEDIA_METADATA_CFG_T *aob_media_get_metadata(uint8_t ase_lid);

/**
 ****************************************************************************************
 * @brief Check if is there an qos conigure ASE
 *
 * @param[in] null
 *
 * @return bool
 *          true: There is an qos configured ASE
 *          false: There is not an qos configured ASE
 ****************************************************************************************
 */
bool aob_media_is_exist_qos_configured_ase(void);

/**
 ****************************************************************************************
 * @brief Configure the report threshold of iso quality (Two methods)
 *          1. Configuring any parameter to reach the threshold;
 *          2. Configuring an interger multiple of iso interval.
 *
 * @param[in] ase_lid   ASE local index
 * @param[in] qlty_rep_evt_cnt_thr   interger multiple of iso interval
 *
 * @return none
 ****************************************************************************************
 */
void aob_media_set_iso_quality_rep_thr(uint8_t ase_lid, uint16_t qlty_rep_evt_cnt_thr,
                                       uint16_t tx_unack_pkts_thr, uint16_t tx_flush_pkts_thr, uint16_t tx_last_subevent_pkts_thr, uint16_t retrans_pkts_thr,
                                       uint16_t crc_err_pkts_thr, uint16_t rx_unreceived_pkts_thr, uint16_t duplicate_pkts_thr);

void aob_media_ascs_register_codec_req_handler_cb(get_qos_req_cfg_info_cb cb_func);

void aob_media_api_init(void);

uint8_t aob_media_get_src_ase_streaming_device_id(void);

uint8_t aob_media_get_sink_ase_streaming_device_id(void);

uint8_t aob_media_get_ase_streaming_device_id(void);

bool aob_media_is_device_any_ase_in_streamimg_state(uint8_t con_lid);

AOB_MGR_CONTEXT_TYPE_BF_E aob_media_get_cur_context_type_by_ase_lid(uint8_t ase_lid);

#ifdef AOB_MOBILE_ENABLED
/****************************for client(mobile)*****************************/
/******************************macro defination*****************************/

/******************************type defination******************************/
typedef struct
{
    uint16_t sample_rate;
    uint16_t frame_octet;
    app_gaf_direction_t direction;
    const app_gaf_codec_id_t *codec_id;
    uint16_t context_type;
} AOB_MEDIA_ASE_CFG_INFO_T; //see @BLE_ASE_CFG_INFO_T

typedef enum
{
    /// Media start
    AOB_MOBILE_MEDIA_START_STREAM_DJOB = 0,

} aob_mobile_media_djob_type_e;

typedef struct aob_media_info
{
    bool biDirection;
    uint8_t delayCnt;
    bool djobStarted;
    aob_mobile_media_djob_type_e djob_type;
    AOB_MEDIA_ASE_CFG_INFO_T pCfgInfo[AOB_MGR_DIRECTION_MAX];
} AOB_MOBILE_MEDIA_INFO_T;

typedef struct
{
    uint8_t djobStatusBf;
    bool djobTimerStarted;
    AOB_MOBILE_MEDIA_INFO_T info[BLE_AUDIO_CONNECTION_CNT];
} AOB_MOBILE_MEDIA_ENV_T;

/****************************function declaration***************************/
/**
 ****************************************************************************************
 * @brief start ASE dicovery procedure
 *
 * @param[in] con_lid      Connection local index
 *
 ****************************************************************************************
*/
void aob_media_mobile_start_ase_disvovery(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief start ASE cfg codec procedure
 *
 * @param[in] ase_lid      ASE local index
 * @param[in] cis_id       CIS ID
 * @param[in] codec_id     Codec ID
 * @param[in] sampleRate   Sample Rat
 * @param[in] frame_octet  Frame Octs
 *
 ****************************************************************************************
*/
void aob_media_mobile_cfg_codec(uint8_t ase_lid, uint8_t cis_id,
                                const app_gaf_codec_id_t *codec_id,
                                uint8_t sampleRate, uint16_t frame_octet);

/**
 ****************************************************************************************
 * @brief start ASE cfg codec with cfg procedure
 *
 * @param[in] ase_lid      ASE local index
 * @param[in] cis_id       CIS ID
 * @param[in] codec_id     Codec ID
 * @param[in] p_cfg        Codec Configurations
 *
 ****************************************************************************************
*/
void aob_media_mobile_configure_codec_with_cfg(uint8_t ase_lid, uint8_t cis_id,
                                                const app_gaf_codec_id_t *codec_id,
                                                const app_gaf_bap_cfg_t *p_cfg);

/**
 ****************************************************************************************
 * @brief start ASE cfg qos procedure
 *
 * @param[in] ase_lid      ASE local index
 * @param[in] cig_id       CIG ID
 * @param[in] max_sdu_size Max SDU size
 *
 ****************************************************************************************
*/
void aob_media_mobile_cfg_qos(uint8_t ase_lid, uint8_t cig_id, uint16_t max_sdu_size);

/**
 ****************************************************************************************
 * @brief Set all ase sdu interval
 *
 * @param[in] sdu_intv_m2s_us      SDU interval c 2 p in us
 * @param[in] sdu_intv_s2m_us      SDU interval p 2 c in us
 *
 ****************************************************************************************
*/
void aob_media_mobile_set_sdu_interval(uint32_t sdu_intv_m2s_us, uint32_t sdu_intv_s2m_us);

/**
 ****************************************************************************************
 * @brief Set cis number in cig whatever it is prepare to use or not
 *
 * @param[in] cis_num      CIS count in CIG
 *
 ****************************************************************************************
*/
void aob_media_mobile_set_cis_count_in_cig(uint8_t cis_num);

/**
 ****************************************************************************************
 * @brief Prepare cig param for cig created
 *
 * @param[in] cig_param    CIG param
 *
 ****************************************************************************************
*/
void aob_media_mobile_prepare_cig_param(const AOB_BAP_CIG_PARAM_T *cig_param);

/**
 ****************************************************************************************
 * @brief start ASE enable procedure
 *
 * @param[in] ase_lid      ASE local index
 * @param[in] stream_context_bf
 *                         Stream context bitfiled
 * @param[in] ccid_num     Number of CCID
 * @param[in] p_ccid_list  CCID list
 *
 ****************************************************************************************
*/
void aob_media_mobile_enable(uint8_t ase_lid, uint16_t stream_context_bf,
                             uint8_t ccid_num, const uint8_t *p_ccid_list);

/**
 ****************************************************************************************
 * @brief start ASE disable procedure
 *
 * @param[in] ase_lid      ASE local index
 *
 ****************************************************************************************
*/
void aob_media_mobile_disable(uint8_t ase_lid);

/**
 ****************************************************************************************
 * @brief start ASE release procedure
 *
 * @param[in] ase_lid      ASE local index
 *
 ****************************************************************************************
*/
void aob_media_mobile_release(uint8_t ase_lid);

/**
 ****************************************************************************************
 * @brief Update metadata.
 *
 * @param[in] ase_lid       ASE local index
 * @param[in] meta_data     Metadata for Codec Configuration see @app_bap_metadata_cfg_t
 ****************************************************************************************
 */
void aob_media_mobile_update_metadata(uint8_t ase_lid, app_gaf_bap_cfg_metadata_t *meta_data);

/**
 ****************************************************************************************
 * @brief create CIS streaming
 *
 * @param[in] pInfo    CIS info see @AOB_MEDIA_ASE_CFG_INFO_T
 * @param[in] con_lid   Connection local index
 * @param[in] biDirection bi-direction CIS stream
 *
 ****************************************************************************************
*/
void aob_media_mobile_start_stream(AOB_MEDIA_ASE_CFG_INFO_T *pInfo, uint8_t con_lid, bool biDirection);

/**
 ****************************************************************************************
 * @brief Disable a CIS Stream(Stream stop and CIS disconnected)..
 *
 * @param[in] ase_lid              ase local index
 ****************************************************************************************
 */
void aob_media_mobile_disable_stream(uint8_t ase_lid);

/**
 * @brief Disable all unicast stream for specific conn and direction
 *
 * @param con_lid connection local index
 * @param direction bap direction
 */
void aob_media_mobile_disable_all_stream_for_specifc_direction(uint8_t con_lid, uint8_t direction);

/**
 ****************************************************************************************
 * @brief Release a CIS Stream(Stream stop and CIS disconnected)..
 *
 * @param[in] ase_lid              ase local index
 ****************************************************************************************
 */
void aob_media_mobile_release_stream(uint8_t ase_lid);

/**
 * @brief Release all unicast stream for specific conn and direction
 *
 * @param con_lid connection local index
 * @param direction bap direction
 */
void aob_media_mobile_release_all_stream_for_specifc_direction(uint8_t con_lid, uint8_t direction);

/**
 ****************************************************************************************
 * @brief enable a CIS Stream
 *
 * @param[in] ase_lid       ase local index
 ****************************************************************************************
 */
void aob_media_mobile_enable_stream(uint8_t ase_lid);

/**
 ****************************************************************************************
 * @brief get ASE local id.
 *
 * @param[in] con_lid              connection local index
 * @param[in] direction            ASE Direction, @see AOB_MGR_DIRECTION_E
 *
 * @return ASE local index
 ****************************************************************************************
 */
uint8_t aob_media_mobile_get_cur_streaming_ase_lid(uint8_t con_lid, AOB_MGR_DIRECTION_E direction);

/**
 ****************************************************************************************
 * @brief Microphone Control for Mobile
 *
 * @param[in] con_lid           connection local index
 * @param[in] mute              mute/unmute
 *
 ****************************************************************************************
 */
void aob_media_mobile_micc_set_mute(uint8_t con_lid, uint8_t mute);

/**
 ****************************************************************************************
 * @brief get the current media state
 *
 * @param[in] ase_lid       ASE local index
 * @return AOB_MGR_STREAM_STATE_E
 ****************************************************************************************
 */
AOB_MGR_STREAM_STATE_E aob_media_mobile_get_cur_ase_state(uint8_t ase_lid);

/**
 ****************************************************************************************
 * @brief Media control.
 *
 * @param[in] media_lid     media id
 * @param[in] action        media action
 *
 ****************************************************************************************
 */
void aob_media_mobile_action_control(uint8_t media_lid, uint8_t action);

/**
 ****************************************************************************************
 * @brief Media control.
 *
 * @param[in] media_lid     media id
 * @param[in] action        media action
 * @param[in] track_pos     track position
 * @param[in] seeking_speed seeking speed
 *
 ****************************************************************************************
 */
void aob_media_mobile_position_ctrl(uint8_t media_lid, uint8_t action, int32_t track_pos, int8_t seeking_speed);

/**
 ****************************************************************************************
 * @brief Media control.
 *
 * @param[in] media_lid     media id
 * @param[in] name          player name
 * @param[in] name_len      player name length
 *
 ****************************************************************************************
 */
void aob_media_mobile_set_player_name(uint8_t media_lid, uint8_t *name, uint8_t name_len);

/**
 ****************************************************************************************
 * @brief Media Track changed.
 *
 * @param[in] media_lid     media id
 * @param[in] duration      duration
 * @param[in] title         title
 * @param[in] title_len     title_len
 *
 ****************************************************************************************
 */
void aob_media_mobile_change_track(uint8_t media_lid, uint32_t duration, uint8_t *title, uint8_t title_len);

void aob_media_mobile_api_init(void);

void aob_mobile_ava_ctx_changed_cb_init(aob_mobile_ava_ctx_changed_cb cb);
#endif

AOB_MGR_CONTEXT_TYPE_BF_E aob_media_get_cur_context_type_by_ase_lid(uint8_t ase_lid);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __AOB_MUSIC_API_H__ */
