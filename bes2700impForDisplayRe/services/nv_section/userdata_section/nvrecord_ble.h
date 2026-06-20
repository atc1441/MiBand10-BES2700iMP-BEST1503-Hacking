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
#if defined(NEW_NV_RECORD_ENABLED)

#ifndef NVRECORD_BLE_H
#define NVRECORD_BLE_H

#include "nvrecord_extension.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t* nvrecord_find_arbitrary_peer_ble_device_address(void);
void nv_record_blerec_set_change_unaware(void);
void nv_record_blerec_update_database_hash(const uint8_t *hash);
bool nv_record_ble_record_find_ltk(uint8_t *pBdAddr, uint8_t *ltk, uint16_t ediv);
uint8_t *nv_record_ble_record_find_device_security_info_through_static_bd_addr(uint8_t* pBdAddr);
bool nv_record_ble_record_Once_a_device_has_been_bonded(void);
bool nv_record_ble_read_addr_resolv_supp_via_bdaddr(uint8_t *pBdAddr, bool *issupport);
bool nv_record_ble_write_addr_resolv_supp_via_bdaddr(uint8_t *pBdAddr, bool issupport);
bool nv_record_ble_read_volume_via_bdaddr(uint8_t *pBdAddr, uint8_t *vol);
bool nv_record_ble_write_volume_via_bdaddr(uint8_t *pBdAddr, uint8_t vol);
uint8_t nv_record_ble_fill_irk(uint8_t* ltkToFill);
void nv_record_blerec_init(void);
NV_RECORD_PAIRED_BLE_DEV_INFO_T* nv_record_blerec_get_ptr(void);
void nv_record_blerec_get_local_irk(uint8_t* pIrk);
uint8_t nv_record_get_paired_ble_dev_info_list_num(void);
bool nv_record_get_ble_pairing_info_through_list_index(uint8_t listIndex, BleDevicePairingInfo* blePairInfo);
bool nv_record_blerec_get_bd_addr_from_irk(uint8_t* pBdAddr, uint8_t* pIrk);
bool nv_record_blerec_get_irk_from_addr(uint8_t* pBdAddr, uint8_t* pIrk);
void nvrecord_rebuild_paired_ble_dev_info(NV_RECORD_PAIRED_BLE_DEV_INFO_T* pPairedBtInfo);
int nv_record_blerec_enum_latest_two_paired_dev(BleDeviceinfo* record1, BleDeviceinfo* record2);
uint8_t nv_record_blerec_enum_paired_dev_addr(BLE_ADDR_INFO_T *addr);
bool nv_record_blerec_get_paired_dev_from_addr(BleDevicePairingInfo* record, BLE_ADDR_INFO_T *pBdAddr);
bool nv_record_blerec_is_paired_from_addr(uint8_t *pBdAddr);
void nv_recored_blerec_dump();
int nv_record_ble_gatt_cache_add(const BLE_ADDR_INFO_T *p_addr, uint8_t cache_seq, const void *client_cache);
int nv_record_ble_gatt_cache_del(const BLE_ADDR_INFO_T *p_addr);
const gattc_nv_cache_t *nv_record_ble_gatt_cache_get(const BLE_ADDR_INFO_T *p_addr);
int nv_record_ble_server_cache_add(const BLE_ADDR_INFO_T *p_addr, const void *server_cache);
int nv_record_ble_server_cache_del(const BLE_ADDR_INFO_T *p_addr);
const gattc_server_cache_t *nv_record_ble_server_cache_get(const BLE_ADDR_INFO_T *p_addr);

#if BLE_AUDIO_ENABLED
void nv_record_bleaudio_init(void);
NV_RECORD_BLE_AUDIO_DEV_INFO_T* nv_record_bleaudio_get_ptr(void);
void nv_record_bleaudio_update_devinfo(uint8_t *info);
#endif

#ifdef TWS_SYSTEM_ENABLED
void nv_record_extension_update_tws_ble_info(NV_RECORD_PAIRED_BLE_DEV_INFO_T *info);
void nv_record_tws_exchange_ble_info(void);
uint8_t *nv_record_tws_get_self_ble_info(void);
#endif

#ifdef TOTA_CRASH_DUMP_TOOL_ENABLE
void nv_record_blerec_crash_dump(void);
#endif


#ifdef CTKD_ENABLE
#define TMP1LEN          (4)
#define LEBRLEN          (4)
#define SALTLEN          (16)
#define LINKKEYLEN       (16)

void clac_linkKey(uint8_t *ltk, uint8_t *pBdAddr, bool ct2);
#endif

#ifdef __cplusplus
}
#endif

#endif
#endif // #if defined(NEW_NV_RECORD_ENABLED)

