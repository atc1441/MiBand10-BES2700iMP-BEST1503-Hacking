/***************************************************************************
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
/**
 ****************************************************************************************
 * @addtogroup AOB_APP
 * @{
 ****************************************************************************************
 */
#ifndef __AOB_GATT_CACHE_H__
#define __AOB_GATT_CACHE_H__
#include "nvrecord_extension.h"

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
 * Function         aob_gattc_delete nv cache
 * Description      delete gatt cache rec in nv using pee_addr or rec_lid
 * @param[in]       peer_addr the peer_mobile's public addr
 * @param[in]       svc_uuid indicate the svc record that should be deleted
 * Returns          void.
******************************************************************************/
void aob_gattc_delete_nv_cache(uint8_t *peer_addr, uint32_t svc_uuid);

/******************************************************************************
 * Function         aob_gattc_del_all_nv_cache nv cache
 * Description      delete all nv gatt caching
 * Returns          void.
******************************************************************************/
void aob_gattc_del_all_nv_cache(void);

/******************************************************************************
 * Function         aob_gattc_cache_reset
 * Description      This callout function is executed by GATTC to reset cache in
 *                  application
 * Parameter        server_bda: server bd address of this cache belongs to
 * Returns          void.
******************************************************************************/
void aob_gattc_rebuild_cache(GATTC_NV_SRV_ATTR_t *record);

/******************************************************************************
 * Function         aob_gattc_fill_nv_attr
 * Description      fill a NV attribute entry value
 * Returns          void.
******************************************************************************/
void aob_gattc_fill_nv_attr();

/******************************************************************************
 * Function         aob_gattc_cache_save
 * Description      save the server cache into NV
 * @param[in]       peer_addr the peer_mobile's public addr
 * @param[in]       svc_uuid indicate the svc record that should be save in nv
 * @param[in]       gatt_data svc bon_data that should be saved in nv
 * Returns          void.
******************************************************************************/
bool aob_gattc_cache_save(uint8_t *peer_addr, uint32_t svc_uuid, void *gatt_data);

/******************************************************************************
 * Function         aob_gattc_rebuild_new_cache
 * Description      generate a new gatt caching,if caching is already exist,return directly.
 * @param[in]       peer_addr the peer_mobile's public addr
 * @param[in]       svc_uuid indicate the svc record that should be save in nv
 * @param[in]       gatt_data svc bon_data that should be saved in nv
 * Returns          void.
******************************************************************************/

bool aob_gattc_rebuild_new_cache(uint8_t *peer_addr, uint32_t svc_uuid, void *gatt_data);

/******************************************************************************
 * Function         aob_gattc_cache_load
 * Description      Load GATT cache from storage for server.
 * @param[in]       con_lid the peer_mobile's ble connectin local id
 * @param[in]       peer_addr the peer_mobile's public addr
 * @param[in]       svc_uuid indicate the svc record that should be save in nv
 * Returns          true on success, false otherwise
******************************************************************************/
bool aob_gattc_cache_load(uint8_t con_lid, uint8_t *peer_addr, uint32_t svc_uuid);

/******************************************************************************
 * Function         aob_gattc_is_item_exist
 * Description      Check GATT cache item from storage for server is exist.
 * @param[in]       peer_addr the peer_mobile's public addr
 * @param[in]       svc_uuid indicate the svc record that should be save in nv
 * Returns          true on success, false otherwise
******************************************************************************/
bool aob_gattc_is_cache_item_exist(uint8_t *peer_addr, uint32_t svc_uuid);

GATTC_SRV_ATTR_t *aob_gattc_find_valid_cache(uint8_t *peer_addr, bool alloc);

void aob_gattc_display_cache_server();

void aob_gattc_caching_unit_test();


#ifdef __cplusplus
}
#endif


#endif /* __AOB_GATT_CACHE_H__ */
