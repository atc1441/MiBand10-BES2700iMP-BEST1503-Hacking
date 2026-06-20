/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
/**
 ****************************************************************************************
 * @addtogroup APP_ACC

 * @{
 ****************************************************************************************
 */

#ifndef APP_ATC_CSISM_MSG_H_
#define APP_ATC_CSISM_MSG_H_
#if BLE_AUDIO_ENABLED

#define APP_ATC_CSIS_SETS_NUM           (1)            //group set number, it means one earbud can belongs to multi groups.

#define APP_ATC_CSIS_SIRK_LEN           (16)
#define APP_ATC_CSIS_RSI_LEN            (6)

#define APP_ATC_CSIS_SETS_INVALID       (0xFF)
typedef struct
{
    uint8_t     sirk_value[APP_ATC_CSIS_SIRK_LEN];
    uint8_t     group_include_dev_nb;         //set_size
    uint8_t     lock_timeout;                 //second unit
    uint8_t     rank_index;
} CSISM_SET_INFO_CONFIG_T;

typedef struct
{
    uint8_t      con_lid;            //0xFF is invalid
    bool         auth_done;
    bool         rsi_avaliable;
    uint8_t      rsi_value[APP_ATC_CSIS_RSI_LEN];
} CSISM_SET_INFO_VALUE_T;

typedef struct app_atc_csism_info
{
    uint8_t                 active_set_lid;         //0xFF is invalid
    CSISM_SET_INFO_CONFIG_T config_info[APP_ATC_CSIS_SETS_NUM];
    CSISM_SET_INFO_VALUE_T  value_info[APP_ATC_CSIS_SETS_NUM];
} APP_ATC_CSISM_ENV_T;

typedef bool (*csism_info_user_config_cb)(CSISM_SET_INFO_CONFIG_T *info);

#ifdef _cplusplus
extern "C" {
#endif

int app_atc_csism_set_rank(uint8_t set_lid, uint8_t rank);
int app_atc_csism_restore_bond_data_req(uint8_t con_lid, uint8_t set_lid, uint8_t is_locked,
                                        uint8_t cli_cfg_bf);

#ifdef _cplusplus
}
#endif

#endif
#endif // APP_ATC_CSISM_MSG_H_

/// @} APP_ATC_CSISM_MSG_H_
