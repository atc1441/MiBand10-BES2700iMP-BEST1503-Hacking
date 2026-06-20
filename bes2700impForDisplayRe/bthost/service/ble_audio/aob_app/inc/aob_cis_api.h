/**
 * @file aob_cis_api.h
 * @author BES AI team
 * @version 0.1
 * @date 2022-04-18
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

#ifndef __AOB_CIS_API_H__
#define __AOB_CIS_API_H__

#ifdef __cplusplus
extern "C" {
#endif
/*****************************header include********************************/
#include "ble_audio_define.h"
#include "aob_mgr_gaf_evt.h"

/******************************macro defination*****************************/

/******************************type defination******************************/
/// Structure for cis max pdu size info
typedef struct
{
    /// Connection local index of LE connection the CIS is bound with
    uint8_t con_lid;
    /// Maximum size, in octets, of the payload from master to slave (Range: 0x00-0xFB)
    uint16_t max_pdu_m2s;
    /// Maximum size, in octets, of the payload from slave to master (Range: 0x00-0xFB)
    uint16_t max_pdu_s2m;
} aob_cis_pdu_size_info_t;
/****************************function declaration***************************/

void aob_cis_api_init(void);

aob_cis_pdu_size_info_t *aob_cis_get_pdu_size_info(uint8_t con_lid);
uint8_t aob_get_ase_lid_by_ase_id(uint8_t con_lid, uint8_t ase_id);

#ifdef AOB_MOBILE_ENABLED
void aob_cis_mobile_api_init(void);
aob_cis_pdu_size_info_t *aob_cis_mobile_get_pdu_size_info(uint8_t con_lid);
#endif

#ifdef __cplusplus
}
#endif

#endif
