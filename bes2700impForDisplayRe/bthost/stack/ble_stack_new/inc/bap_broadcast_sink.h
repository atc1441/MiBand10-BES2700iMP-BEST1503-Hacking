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
#ifndef __BAP_BC_SINK_INC__
#define __BAP_BC_SINK_INC__
#include "bluetooth.h"

#include "bap_broadcast_common.h"

#include "gap_service.h"

#define BAP_BC_SINK_GROUP_MAX_SUPP_NUM      (GAF_SUPP_MAX_BIG_NUM)

/*TYPEDEF*/
typedef enum bap_bc_stream_state
{
    /// ISO Datapath setup
    BAP_BC_SINK_STATE_ISO_DP_SETUP_SUCCESS_BIT          = CO_BIT(1),

    BAP_BC_SINK_STATE_ISO_DP_SETUP_RUNNING_BIT          = CO_BIT(2),

    BAP_BC_SINK_STATE_ISO_DP_REMOVE_RUNNING_BIT         = CO_BIT(3),

} bap_bc_stream_state_e;

typedef struct bap_bc_sink_stream
{
    /// BIS index
    uint8_t bis_idx;
    /// Stream state bitfiled
    uint8_t state_bf;
    /// BIS hdl
    uint16_t bis_hdl;
} bap_bc_sink_stream_t;

typedef void (*bap_bc_sink_big_sync_state)(uint16_t pa_sync_hdl, uint8_t big_id, uint8_t bis_cnt, const bap_bc_sink_stream_t *p_bis_stream_info, uint16_t err_code, const bap_bis_timing_t *p_bis_timing);

typedef void (*bap_bc_sink_big_sync_term)(uint8_t big_id, uint16_t err_code);

typedef void (*bap_bc_sink_iso_dp_state)(bool is_create, uint8_t big_id, uint8_t bis_idx, uint16_t err_code);

typedef struct bap_bc_sink_callbacks
{
    bap_bc_sink_big_sync_state cb_big_sync_state;

    bap_bc_sink_big_sync_term cb_big_sync_term;

    bap_bc_sink_iso_dp_state cb_bis_iso_dp_state;

} bap_bc_sink_callback_t;

#if (BAP_BROADCAST_SINK)
/*FUNCTIONS DECLARATION*/
/**
 * @brief BAP broadcast SINK initilization
 *
 * @return int         status
 */
int bap_bc_sink_init(void);

/**
 * @brief BAP broadcast SINK deinitilization
 *
 * @return int         status
 */
int bap_bc_sink_deinit(void);

/**
 * @brief BAP broadcast SINK callbacks register
 *
 * @param  user        Interbal USER @see bap_bc_common_user_e
 * @param  p_evt_cb    Event callbacks
 *
 * @return int         status
 */
int bap_bc_sink_callback_register(bap_bc_common_user_e user, const bap_bc_sink_callback_t *p_evt_cb);

/**
 * @brief BAP broadcast SINK callback deregister
 *
 * @param  user        Interbal USER @see bap_bc_common_user_e
 *
 * @return int         status
 */
int bap_bc_sink_callback_unregister(bap_bc_common_user_e user);


/**
 * @brief BAP broadcast SINK sync BIG
 *
 * @param  pa_sync_hdl PA sync handle
 * @param  timeout_10ms
 *                     Sync timeout 10ms
 * @param  mse         Max skip event
 * @param  bis_cnt     BIS count to create BIG sync
 * @param  bis_idx     BIS index list
 * @param  broadcast_code
 *                     Broadcast Code to decrypt BIG
 * @param  wait_big_sync_estb_to_10ms
 *                     Wait BIG sync estb timeout 10ms, 0 means no wait
 *
 * @return int         status
 */
int bap_bc_sink_sync_big(uint16_t pa_sync_hdl, uint16_t timeout_10ms, uint8_t mse, uint8_t bis_cnt, 
                         const uint8_t *bis_idx, const uint8_t *broadcast_code,
                         uint16_t wait_big_sync_estb_to_10ms);

/**
 * @brief BAP broadcast SINK BIG sync terminate
 *
 * @param  big_id      BIG ID
 *
 * @return int         status
 */
int bap_bc_sink_termiante_big_sync(uint8_t big_id);

/**
 * @brief BAP broadcast SINK BIG sync terminate by pa sync handle
 *
 * @param  sync_hdl    PA sync handle
 *
 * @return int         status
 */
int bap_bc_sink_termiante_big_sync_by_sync_hdl(uint16_t sync_hdl);

/**
 * @brief BAP broadcast SINK setup iso datapath
 *
 * @param  big_id      BIG ID
 * @param  bis_index   BIS index (>=1)
 * @param  p_codec_id  Codec ID value pointer
 * @param  p_codec_id  Codec Configuration
 *
 * @return int         status
 */
int bap_bc_sink_setup_iso_data_path(uint8_t big_id, uint8_t bis_index, const uint8_t *p_codec_id, const gen_aud_cc_t *p_codec_cfg);

/**
 * @brief BAP broadcast SINK remove iso datapath
 *
 * @param  big_id      BIG ID
 * @param  bis_index   BIS index (>=1)
 *
 * @return int         status
 */
int bap_bc_sink_remove_iso_data_path(uint8_t big_id, uint8_t bis_index);
#endif /// (BAP_BROADCAST_SINK)
#endif /// __BAP_BC_SINK_INC__