/**
 ****************************************************************************************
 *
 * @file app_bap_bc_deleg_msg.h
 *
 * @brief BLE Audio Broadcast Delegator
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

#ifndef APP_BAP_DELEG_MSG_H_
#define APP_BAP_DELEG_MSG_H_

#if BLE_AUDIO_ENABLED

#ifdef __cplusplus
extern "C" {
#endif
#include "app_bap.h"

#define APP_BAP_DFT_BC_DELEG_NB_SRCS                (1)
#define APP_BAP_DFT_BC_DELEG_ADV_INTV_MIN_SLOT      256 //100ms  *0.625ms
#define APP_BAP_DFT_BC_DELEG_ADV_INTV_MAX_SLOT      256 //100ms  *0.625ms
#define APP_BAP_DFT_BC_DELEG_CHNL_MAP               ADV_ALL_CHNLS_EN
#define APP_BAP_DFT_BC_DELEG_PHY_PRIM               APP_PHY_1MBPS_VALUE
#define APP_BAP_DFT_BC_DELEG_PHY_SECOND             APP_PHY_1MBPS_VALUE
#define APP_BAP_DFT_BC_DELEG_ADV_SID                1
#define APP_BAP_DFT_BC_DELEG_SKIP                   1
#define APP_BAP_DFT_BC_DELEG_REPORT_FILTER_BF       0x1F
#define APP_BAP_DFT_BC_DELEG_SYNC_TO_10MS           200
#define APP_BAP_DFT_BC_DELEG_SYNC_PA_RETRY_CNT      2
#define APP_BAP_DFT_BC_DELEG_WAIT_PAST_TO_10MS      100
#define APP_BAP_DFT_BC_DELEG_TIMEOUT_S              0 //Always scan periodic adv
#define APP_BAP_DFT_BC_DELEG_SYNC_BIG_TIMEOUT_10MS  100
#define APP_BAP_DFT_BC_DELEG_SYNC_BIG_MSE           5
#define APP_BAP_BC_DELEG_MAX_ADV_DATA_LEN           200
#define APP_BAP_DFT_BC_DELEG_SOLICITE_ADV_DATA      "bc_delegator_solicite_adv_data_test"

int app_bap_bc_deleg_source_add(app_gaf_bap_adv_id_t *addr, const app_gaf_bap_bcast_id_t *p_bcast_id,
                                uint8_t nb_subgroups, uint8_t *add_metadata, uint8_t add_metadata_len, uint16_t streaming_context_bf,
                                uint32_t bis_sync_bf_deprecated);
int app_bap_bc_deleg_restore_bond_data_req(uint8_t con_lid, uint8_t cfg_bf);
int app_bap_bc_deleg_restart_src_big_sync(uint8_t src_lid);
#ifdef __cplusplus
}
#endif

#endif
#endif // APP_BAP_DELEG_MSG_H_

/// @} APP_BAP_DELEG_MSG_H_
