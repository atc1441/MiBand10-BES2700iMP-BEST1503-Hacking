/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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

#ifndef __BLE_AUDIO_EARPHONE_INFO_H__
#define __BLE_AUDIO_EARPHONE_INFO_H__

#if (BLE_AUDIO_ENABLED)
/*****************************header include********************************/
#include "ble_aob_common.h"
#include "nvrecord_extension.h"
#include "ble_audio_define.h"

#include "bes_aob_api.h"

enum ADV_FLAG_BIT
{
    /// Is Support BR/EDR LE Simultaneous
    SIMU_BR_EDR_LE_BIT   = 0x01,
    /// Is Adv flag with BAP requiremenet with discoverable mode
    DISCOVERABLE_ADV_BIT = 0x02,
    /// Is Adv type Direct Connectable
    TARGET_ANNOUN_EN_BIT = 0x04,
};

/******************************macro defination*****************************/

#define BLE_AUDIO_CAPA_SINK_CONTEXT_BF \
    (BES_BLE_GAF_CONTEXT_TYPE_CONVERSATIONAL_BIT | \
     BES_BLE_GAF_CONTEXT_TYPE_MEDIA_BIT | \
     BES_BLE_GAF_CONTEXT_TYPE_UNSPECIFIED_BIT | \
     BES_BLE_GAF_CONTEXT_TYPE_LIVE_BIT | \
     BES_BLE_GAF_CONTEXT_TYPE_GAME_BIT)

#define BLE_AUDIO_CAPA_SRC_CONTEXT_BF \
    (BES_BLE_GAF_CONTEXT_TYPE_UNSPECIFIED_BIT | \
     BES_BLE_GAF_CONTEXT_TYPE_CONVERSATIONAL_BIT | \
     BES_BLE_GAF_CONTEXT_TYPE_MEDIA_BIT | \
     BES_BLE_GAF_CONTEXT_TYPE_MAN_MACHINE_BIT | \
     BES_BLE_GAF_CONTEXT_TYPE_LIVE_BIT | \
     BES_BLE_GAF_CONTEXT_TYPE_GAME_BIT)

/******************************type defination******************************/
typedef enum
{
    BLE_AUDIO_TWS_MASTER,
    BLE_AUDIO_TWS_SLAVE,
    BLE_AUDIO_MOBILE,
    BLE_AUDIO_ROLE_UNKNOW,
} BLE_AUDIO_TWS_ROLE_E;

typedef enum
{
    BLE_AUDIO_ACC_MCC,
    BLE_AUDIO_ACC_TBC,
} BLE_AUDIO_ACC_TYPE_E;

typedef struct
{
    bool                        connected;
    uint8_t                     conidx;
    uint8_t                     volume;
    bool                        muted;
    bool                        nv_vol_invalid;
    bool                        mcc_bond;
    bool                        tbc_bond;
    BLE_ADDR_INFO_T             peer_ble_addr;

    // TODO: use AOB_CALL_INFO_T* to map the g_aob_call_scb_info.call_info
    AOB_CALL_ENV_INFO_T         call_env_info;
    AOB_MEDIA_INFO_T            media_info;
} AOB_MOBILE_INFO_T;

typedef struct
{
    /// Group local index
    uint8_t             grp_lid;
    /// Position of the stream in the group (range 1 to 32)
    uint8_t             stream_pos;
    /// Codec ID value
    AOB_CODEC_ID_T      codec_id;
    /// Select which audio channel to play when a bis contains multiple audio channels
    uint8_t             audio_chan;
    /// Length of Codec Configuration value - in bytes
    uint8_t             cfg_len;
    /// Parameters structure
    AOB_BAP_CFG_T       cfg_param;
    /// Additional Codec Configuration (in LTV format)
    AOB_CFG_LTV_T       add_cfg;
} AOB_BIS_STREAM_INFO_T;
typedef struct
{
    /// Group local index
    uint8_t grp_lid;
    /// Broadcast ID earphone wants to recv
    uint8_t bcast_id[AOB_COMMON_BC_ID_LEN];
    /// Broadcast Code earphone use to decrypt
    uint8_t bcast_code[AOB_COMMON_GAP_KEY_LEN];
    /// Periodic Advertising local index
    uint8_t pa_lid;
    /// Delegator source local index
    uint8_t src_lid;
    /// Stream position in group
    uint32_t stream_pos_bf;
    /// BIG info recv
    bes_ble_bap_bc_big_info_t group_info;
    /// Synced Stream(BIS) position bf
    uint32_t stream_sink_pos_bf;
    /// BIS hdl (synced bis)
    uint16_t bis_hdl[AOB_COMMON_IAP_NB_STREAMS];

    uint32_t pres_delay;
    /// BIS recving play
    /// Select which audio channel to play when a bis contains multiple audio channels
    uint8_t play_stream_num;
    uint8_t play_ch_num;
    struct
    {
        uint8_t  stream_lid;
        uint8_t  select_ch_bf;
        uint16_t bis_hdl;
        uint16_t blocks_size;
        AOB_BIS_STREAM_INFO_T stream_info;
    } play_stream_info[2];
} AOB_BIS_GROUP_INFO_T;

typedef struct
{
    bool init_done;

    bool aob_enable_adv;

    BLE_AUDIO_TWS_ROLE_E nv_role;
    BLE_AUDIO_TWS_ROLE_E current_role;
    aob_audio_cfg_e audio_cfg_select;

    bool tws_connected;
    uint8_t tws_conidx;

    uint8_t local_ble_addr[BLE_ADDR_SIZE];
    uint8_t peer_ble_addr[BLE_ADDR_SIZE];
    uint8_t bis_src_ble_addr[BLE_ADDR_SIZE];

    AOB_MOBILE_INFO_T mobile_info[AOB_COMMON_MOBILE_CONNECTION_MAX];

    AOB_BIS_GROUP_INFO_T bis_group_info;
    AOB_BIS_STREAM_INFO_T bis_stream_info;
} AOB_EARPHONE_INFO_T;

/****************************function declaration***************************/
#ifdef __cplusplus
extern "C" {
#endif

void ble_audio_earphone_info_init(void);

bool ble_audio_is_ux_master(void);

bool ble_audio_is_ux_slave(void);

void ble_audio_set_tws_nv_role(uint8_t role);

void ble_audio_update_tws_nv_role(uint8_t role);

void ble_audio_set_tws_nv_role_via_nv_addr(void);

uint8_t ble_audio_get_tws_nv_role(void);

void ble_audio_update_tws_current_role(uint8_t role);

uint8_t ble_audio_request_tws_current_role(void);

void ble_audio_set_tws_local_ble_addr(uint8_t *addr);

uint8_t *ble_audio_get_tws_local_ble_addr(void);

void ble_audio_set_tws_peer_ble_addr(const uint8_t *addr);

uint8_t *ble_audio_get_tws_peer_ble_addr(void);

void ble_audio_set_bis_src_ble_addr(const uint8_t *addr);

uint8_t *ble_audio_get_bis_src_ble_addr(void);

bool app_ble_audio_adv_enable(void);

bool app_ble_audio_adv_disable(void);

void app_ble_audio_dump_adv_state(void);

uint8_t ble_audio_get_tws_conidx(void);

bool ble_audio_earphone_info_connected_set(uint8_t conidx, ble_bdaddr_t *mobile_addr);

bool ble_audio_earphone_info_check_conn_exited(uint8_t conidx, ble_bdaddr_t *mobile_addr);

bool ble_audio_earphone_info_disconnected_clear(uint8_t conidx);

void ble_audio_advData_prepare(BLE_ADV_PARAM_T *adv_param, uint8_t adv_flag);

/**
 * @brief Set nv vol info flag valid for next read vol procedure
 *
 * @param[in] con_lid
 */
void ble_audio_earphone_info_cache_vol_to_nv(uint8_t con_lid);

/**
 * @brief Get available monbile info that has not been connected
 *
 * @return AOB_MOBILE_INFO_T*
 */
AOB_MOBILE_INFO_T *ble_audio_earphone_info_get_ava_mobile_info(void);

/**
 * @brief Get local mobile info according to the given connection index
 *
 * @param conidx        BLE connection index
 * @return AOB_MOBILE_INFO_T* Pointer of the mobile info
 */
AOB_MOBILE_INFO_T *ble_audio_earphone_info_get_mobile_info(uint8_t conidx);

/**
 * @brief Get a new call info according to the given connection index when call incoming or outgoing
 *
 * @param conidx        Connection index
 * @param call_id       Call index
 * @return AOB_SINGLE_CALL_INFO_T* Pointer of the call info
 */
AOB_SINGLE_CALL_INFO_T *ble_audio_earphone_info_make_call_info(uint8_t conidx, uint8_t call_id, uint8_t uriLen);

/**
* @brief Get the call index according to the given connection index and bearer index
*
* @param conidx        Connection index
* @param bearer_lid    Bearer local index
* @return uint8_t      Call index
*/
uint8_t ble_audio_bearer_id_get_call_id(uint8_t conidx, uint8_t bearer_lid);


/**
 * @brief Update the call info of given mobile connection
 *
 * @param conidx        BLE connection index
 * @param callInfo      Pointer of the call environment info
 */
void ble_audio_earphone_info_update_call_info(uint8_t conidx, AOB_CALL_ENV_INFO_T  *callInfo);

/**
 * @brief Clear the call info when call terminate
 *
 * @param conidx        BLE connection index
 * @param call_id       Call index
 */
void ble_audio_earphone_info_clear_call_info(uint8_t conidx, uint8_t call_id);

/**
 * @brief Set bear_id
 *
 * @param conidx        BLE connection index
 * @param call_id       Call index
 * @param bearer_lid     Bearer local index
 */
bool ble_audio_earphone_info_set_bearer_lid(uint8_t conidx, uint8_t call_id, uint8_t bearer_lid);

/**
 * @brief Get the media info according to the given connection index
 *
 * @param[in] con_lid        Connection local index
 * @return AOB_MEDIA_INFO_T* Pointer of the media info
 */
AOB_MEDIA_INFO_T *ble_audio_earphone_info_get_media_info(uint8_t con_lid);

/**
 * @brief Get the bis group info
 *
 * @return AOB_BIS_GROUP_INFO_T* Pointer of the group info
 */
AOB_BIS_GROUP_INFO_T *ble_audio_earphone_info_get_bis_group_info(void);

/**
 * @brief Get the bis group info
 *
 * @return AOB_BIS_GROUP_INFO_T* Pointer of the group info
 */
void ble_audio_earphone_info_clear_bis_group_info(void);
/**
 * @brief Set bis grp_lid
 *
 * @param[in] grp_lid        Group local index
 */
void ble_audio_earphone_info_set_bis_grp_lid(uint8_t grp_lid);

/**
 * @brief Get bis grp_lid
 *
 * @return uint8_t
 */
uint8_t ble_audio_earphone_info_get_bis_grp_lid(void);

/**
 * @brief Set bis broadcast ID that earphone wants to recv
 *
 * @param[in] bcast_id  Broadcast ID
 */
void ble_audio_earphone_info_set_bis_bcast_id(uint8_t *bcast_id_p);
/**
 * @brief Get bis bcast ID that earphone wants to recv
 *
 * @return uint8_t*
 */
uint8_t *ble_audio_earphone_info_get_bis_bcast_id(void);

/**
 * @brief Set bis bcast_code for decrypt
 *
 * @param bcast_code_p
 */
void ble_audio_earphone_info_set_bis_bcast_code(uint8_t *bcast_code_p);

/**
 * @brief Get bis bcast_code for decrypt
 *
 */
uint8_t *ble_audio_earphone_info_get_bis_bcast_code(void);

/**
 * @brief Set bis pa_lid
 *
 * @param[in] pa_lid        Periodic Advertising local index
 */
void ble_audio_earphone_info_set_bis_pa_lid(uint8_t pa_lid);

/**
 * @brief Get bis pa_lid
 *
 * @return uint8_t
 */
uint8_t ble_audio_earphone_info_get_bis_pa_lid(void);

/**
 * @brief Get bis src_lid
 *
 * @return src_lid        Source local index
 */
uint8_t ble_audio_earphone_info_get_bis_src_lid(void);

/**
 * @brief Set bis src_lid
 *
 * @param[in] src_lid        Source local index
 */
void ble_audio_earphone_info_set_bis_src_lid(uint8_t src_lid);

/**
 * @brief Record bis stream position in BIG Group Air tansimitted
 *
 * @param[in] stream_pos_bf        Stream position in group
 */
void ble_audio_earphone_info_set_bis_stream_pos_bf(uint32_t stream_pos_bf);

/**
 * @brief Record bis stream sink position in Local Synced BIG
 *
 * @param stream_pos_bf     Stream position in group
 */
void ble_audio_earphone_info_set_bis_stream_sink_pos_bf(uint32_t stream_pos_bf);

/**
 * @brief Update real time volume info
 *
 */
bool ble_audio_earphone_info_set_vol_info(uint8_t con_lid, uint8_t vol, bool muted);

/**
 * @brief Get the real time volume info
 *
 */
bool ble_audio_earphone_info_get_vol_info(uint8_t con_lid, uint8_t *vol, bool *muted);

void ble_audio_fill_eir_manufacture_data(uint8_t *buff, uint32_t* offset);

uint8_t ble_audio_earphone_info_bis_stream_pos_2_stream_lid(uint8_t stream_pos);

AOB_SINGLE_CALL_INFO_T *ble_audio_earphone_info_find_call_info(uint8_t conidx, uint8_t call_id);

AOB_SINGLE_CALL_INFO_T *ble_audio_earphone_info_find_call_info_by_uri(uint8_t conidx, uint8_t uri_len, const uint8_t *p_uri);

/**
 * @brief Get call environment
 *
 */
AOB_CALL_ENV_INFO_T * ble_audio_earphone_info_get_call_env_info(uint8_t con_lid);

uint8_t ble_audio_earphone_info_get_call_id_by_conidx(uint8_t conidx);

uint8_t ble_audio_earphone_info_get_calling_call_id_by_conidx(uint8_t conidx);

uint8_t ble_audio_earphone_info_get_incoming_call_id_by_conidx(uint8_t conidx);

uint8_t ble_audio_earphoe_info_get_call_id_by_conidx_and_type(uint8_t conidx, AOB_CALL_STATE_E call_state);

uint8_t ble_audio_earphone_info_get_another_valid_call_id(uint8_t call_id, AOB_CALL_ENV_INFO_T *call_info);

bool ble_audio_earphone_info_get_acc_bond_status(uint8_t conidx, uint8_t type);

void ble_audio_earphone_info_set_acc_bond_status(uint8_t conidx, uint8_t type, bool status);

void ble_audio_earphone_info_set_target_announcement_en(bool enable);
/**
 * @brief Set bap audio configuration selection
 *
 * @param aud_cfg 
 */
void ble_audio_earphone_info_set_audido_cfg_select(aob_audio_cfg_e aud_cfg);

/**
 * @brief Get bap audio configuration selection
 *
 * @return aob_audio_cfg_e 
 */
aob_audio_cfg_e ble_audio_earphone_info_get_audido_cfg_select(void);

void ble_audio_earphone_dump_conn_state(void);
#ifdef __cplusplus
}
#endif

#endif
#endif


