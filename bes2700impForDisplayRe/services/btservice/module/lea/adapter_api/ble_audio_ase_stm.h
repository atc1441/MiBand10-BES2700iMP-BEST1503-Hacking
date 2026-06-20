/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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

#ifndef __BLE_AUDIO_ASE_STM_H__
#define __BLE_AUDIO_ASE_STM_H__
#ifdef AOB_MOBILE_ENABLED
#include "ble_audio_define.h"
#include "ble_audio_core_api.h"
#include "ble_aob_common.h"
#include "bes_aob_api.h"
#include "app_bap_uc_cli_msg.h"

#ifdef __cplusplus
extern "C"{
#endif

#define LEA_ASE_STM_NUM                     (APP_BAP_DFT_ASCC_TOTAL_NB_ASE_CFG)
#define LEA_ASE_STM_INVALID_LID             (0xFF)
#define LEA_ASE_STM_MAXIMUM_QUEUED_CNT      (APP_BAP_DFT_ASCC_TOTAL_NB_ASE_CFG)
#define LEA_ASE_STM_LAST_IN_USE             (1)
#define LEA_ASE_STM_TIMEOUT                 (5000)
#define LEA_ASE_STM_RETRY_MAX_CNT           (3)

#define LEA_ASE_STM_DEBUG_LEVEL
#define LEA_ASE_STM_INFO_LEVEL
#ifdef LEA_ASE_STM_DEBUG_LEVEL
#define LEA_ASE_STM_TRACE                   TRACE
#else
#define LEA_ASE_STM_TRACE(...)              do{} while(0)
#endif

#ifdef LEA_ASE_STM_INFO_LEVEL
#define LEA_ASE_STM_INFO                    TRACE
#else
#define LEA_ASE_STM_INFO(...)               do{} while(0)
#endif
#define LEA_ASE_STM_WARN                    TRACE
#define LEA_ASE_STM_ERROR                   TRACE

typedef enum
{
    LEA_ASE_ELEMENT_CON_LID,
    LEA_ASE_ELEMENT_ASE_LID,
    LEA_ASE_ELEMENT_ASE_LID_W_EVT,
} LEA_ASE_STM_FIND_TYPE_E;

typedef enum
{
    LEA_ASE_STM_STATUS_READY,
    LEA_ASE_STM_STATUS_BUSY,
    LEA_ASE_STM_STATUS_DUPLICATE,

    LEA_ASE_STM_STATUS_UNKNOW,
} LEA_ASE_STM_STATUS_E;

typedef struct
{
    LEA_ASE_STM_FIND_TYPE_E             type;
    uint8_t                         element1;
    uint8_t                         element2;
} LEA_ASE_STM_FIND_INFO_T;

typedef struct
{
    uint8_t ase_lid;
    uint8_t cis_id;
    LEA_ASE_STM_CFG_INFO_T ase_cfg_info;
} BLE_ASE_ESTABLISH_INFO_T;

typedef struct
{
    void *stm;
    LEA_ASE_STM_EVENT_E evt;
} LEA_ASE_STM_PENDING_INFO_T;

LEA_ASE_STM_STATE_E lea_ase_stm_get_cur_state(uint8_t ase_lid);
void lea_ase_stm_init(void);
void lea_ase_stm_start_auto_play(void);
void ble_ase_stm_set_ase_stm_num_to_use(uint8_t con_lid, uint8_t ase_num);
void ble_ase_stm_increase_ase_stm_num_to_use(uint8_t con_lid, uint8_t step);

#ifdef __cplusplus
}
#endif
#endif

#endif
