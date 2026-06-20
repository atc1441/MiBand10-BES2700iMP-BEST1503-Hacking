/**
 ****************************************************************************************
 *
 * @file app_bap_bc_scan_msg.h
 *
 * @brief BLE Audio Broadcast Scan
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

#ifndef APP_BAP_BC_SACN_MSG_H_
#define APP_BAP_BC_SACN_MSG_H_

#if BLE_AUDIO_ENABLED
#include "app_gaf_define.h"

#define APP_BAP_DFT_BC_SCAN_CACHE_SIZE                 2
#define APP_BAP_DFT_BC_SCAN_NB_SYNC                    1
#define APP_BAP_DFT_BC_SCAN_SCAN_TIMEOUT_S             600
#define APP_BAP_DFT_BC_SCAN_INTV_1M_SLOT               0x20
#define APP_BAP_DFT_BC_SCAN_WD_1M_SLOT                 0x20
#define APP_BAP_DFT_BC_SCAN_INTV_CODED_SLOT            0
#define APP_BAP_DFT_BC_SCAN_WD_CODED_SLOT              0
#define APP_BAP_DFT_BC_SCAN_SKIP                       1
#define APP_BAP_DFT_BC_SCAN_REPORT_FILTER_BF           0x1F
#define APP_BAP_DFT_BC_SCAN_SYNC_TO_10MS               1000
#define APP_BAP_DFT_BC_SCAN_TIMEOUT_S                  2 //Always scan periodic adv

/// Content of Broadcast Scan environment
typedef struct app_bap_bc_scan_env
{
    /// Number of Broadcast Source information that may be stored in the cache
    uint8_t cache_size;
    /// Number of Periodic Synchronizations that may be established in parallel
    uint8_t nb_sync;
    // Scanning trigger method
    uint8_t scan_trigger_method;
    // Scanning State
    uint8_t scan_state;
    /// Timeout duration in seconds
    uint16_t scan_timeout_s;
    /// Broadcast ID
    app_gaf_bap_bcast_id_t bcast_id;
    // Set scan parameter done
    bool scan_para_set_done;
    /// Scan parameters
    app_gaf_bap_bc_scan_param_t scan_param;
    /// PA Sync Params
    app_gaf_bap_bc_scan_sync_param_t sync_param;
    // PA Report Information
    app_gaf_bap_bc_scan_pa_report_info_t pa_info[APP_BAP_DFT_BC_SCAN_NB_SYNC];
} app_bap_bc_scan_env_t;

#ifdef __cplusplus
extern "C" {
#endif

app_bap_bc_scan_env_t *app_bap_bc_scan_get_scan_env(void);

uint8_t app_bap_bc_scan_get_scan_state(void);
uint8_t app_bap_bc_scan_get_sync_state(void);

#ifdef __cplusplus
}
#endif

#endif
#endif // APP_BAP_BC_SACN_MSG_H_

/// @} APP_BAP
