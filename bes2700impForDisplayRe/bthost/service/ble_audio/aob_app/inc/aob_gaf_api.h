/**
 * @file aob_ux_stm.h
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


#ifndef __AOB_GAF_API_H__
#define __AOB_GAF_API_H__

/*****************************header include********************************/
#include "aob_mgr_gaf_evt.h"

/******************************macro defination*****************************/

/******************************type defination******************************/
/// Capability information structure
typedef struct
{
    /// Supported Audio Locations for the direction
    uint32_t sink_location_bf;
    uint32_t src_location_bf;
    /// Available Context Type Bitfield
    uint16_t sink_ava_bf;
    uint16_t src_ava_bf;
    /// Supported Audio Contexts for the direction
    uint16_t sink_context_bf_supp;
    uint16_t src_context_bf_supp;
    /// Number of PACs for the direction
    uint8_t sink_nb_pacs;
    uint8_t src_nb_pacs;
} aob_gaf_capa_info_t;

/****************************function declaration***************************/
#ifdef __cplusplus
extern "C" {
#endif

/**
 ****************************************************************************************
 * @brief earbuds initialize gaf.
 *
 * @param[in] capa_info     the information of gaf capability
 * @param[in] role_bf       the role bit field with BAP
 ****************************************************************************************
 */
void aob_gaf_earbuds_init(aob_gaf_capa_info_t *capa_info, uint32_t role_bf);

/**
 ****************************************************************************************
 * @brief earbuds deinitialize gaf.
 *
 ****************************************************************************************
 */
void aob_gaf_earbuds_deinit(void);

void aob_gaf_bis_init(void);

#if AOB_MOBILE_ENABLED
/**
 ****************************************************************************************
 * @brief mobile initialize gaf.
 *
 *
 ****************************************************************************************
 */
void aob_gaf_mobile_init(void);
#endif

void ble_audio_tws_init(void);

void ble_audio_tws_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __AOB_GAF_API_H__ */
