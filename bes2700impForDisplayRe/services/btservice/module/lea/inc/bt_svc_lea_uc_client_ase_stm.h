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

#ifndef __BT_SVC_LEA_UC_CLIENT_ASE_STM_H__
#define __BT_SVC_LEA_UC_CLIENT_ASE_STM_H__
#include "ble_aob_common.h"
#include "bes_aob_api.h"

#ifdef AOB_MOBILE_ENABLED
typedef enum
{
    LEA_ASE_STATE_IDLE,
    LEA_ASE_STATE_CODEC_CONFIGUED,
    LEA_ASE_STATE_QOS_CONFIGUED,
    LEA_ASE_STATE_ENABLING,
    LEA_ASE_STATE_STREAMING,
    LEA_ASE_STATE_DISABLING, /// 5
    LEA_ASE_STATE_RELEASING,

    LEA_ASE_STATE_UNKNOW,
} LEA_ASE_STM_STATE_E;

typedef enum
{
    LEA_REQ_ASE_START,
    LEA_REQ_ASE_CODEC_CONFIGURE,
    LEA_REQ_ASE_QOS_CONFIGURE,
    LEA_REQ_ASE_ENABLE,
    LEA_REQ_ASE_DISABLE,
    LEA_REQ_ASE_RELEASE, /// 5

    LEA_EVT_ASE_CODEC_CONFIGURED,
    LEA_EVT_ASE_UC_GRP_CREATED,
    LEA_EVT_ASE_QOS_CONFIGURED,
    LEA_EVT_ASE_ENABLED,
    LEA_EVT_ASE_DISABLED, /// 10
    LEA_EVT_ASE_RELEASED,
    LEA_EVT_ASE_IDLE,
    LEA_EVT_ASE_UC_GRP_REMOVED,

    LEA_EVT_ASE_TIMEOUT,
    LEA_ASE_EVENT_MAX,
} LEA_ASE_STM_EVENT_E;

typedef struct
{
    uint16_t                     sample_rate;
    uint16_t                     frame_octet;
    bes_gaf_direction_t            direction;
    const bes_gaf_codec_id_t       *codec_id;
    uint16_t                    context_type;
} LEA_ASE_STM_CFG_INFO_T;

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

LEA_ASE_STM_STATE_E bt_svc_lea_uc_client_ase_stm_get_cur_state(uint8_t ase_lid);
void bt_svc_lea_uc_client_ase_stm_init(void);
void bt_svc_lea_uc_client_ase_stm_set_num_to_use(uint8_t con_lid, uint8_t ase_num);
void bt_svc_lea_uc_client_ase_stm_increase_num_to_use(uint8_t con_lid, uint8_t step);

void bt_svc_lea_uc_client_ase_stm_send_msg(uint8_t ase_lid, LEA_ASE_STM_EVENT_E event, uint32_t para0, uint32_t para1);
void bt_svc_lea_uc_client_ase_stm_send_msg_by_grp_lid(uint8_t grp_lid, LEA_ASE_STM_EVENT_E event, uint16_t status);

void bt_svc_lea_uc_client_ase_sm_start_auto_play_by_con_lid(uint8_t conn_lid);

uint8_t bt_svc_lea_uc_client_ase_have_sm_by_ase_lid(uint8_t ase_lid);

uint8_t bt_svc_lea_uc_client_ase_stream_start(uint8_t con_lid, uint8_t cis_id, uint8_t ase_lid,
            uint8_t biDirection, LEA_ASE_STM_CFG_INFO_T* pInfo_local);
#endif

#endif
