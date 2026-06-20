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
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "nvrecord_extension.h"
#include "nvrecord_ble.h"
#include "nvrecord_bt.h"
#include "hal_codec.h"
#include "hal_trace.h"
#include "tgt_hardware.h"
#include "hal_timer.h"
#include "bluetooth_bt_api.h"
#include "besbt.h"
#include "bt_drv_interface.h"
#include "cmac.h"
#include "ddbif.h"

#if BLE_AUDIO_ENABLED
#include "bluetooth_ble_api.h"
#include "bes_aob_api.h"
#endif

#define ble_nv_debug
#ifdef ble_nv_debug
#define ble_trace TRACE
#else
#define ble_trace
#endif

#define CRASH_DUMP_DEBUG_TRACE(attr, str, ...)  \
    TRACE(((attr) | TR_ATTR_NO_ID), str, ##__VA_ARGS__)

extern void srand (unsigned int seed);
extern int rand(void);

static NV_RECORD_PAIRED_BLE_DEV_INFO_T *nvrecord_ble_p = NULL;
static const uint8_t INVALID_ADDR[BTIF_BD_ADDR_SIZE] = {0x00};

uint8_t* nvrecord_find_arbitrary_peer_ble_device_address(void)
{
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *find_ptr = nvrecord_ble_p;

    if(NULL == find_ptr)
    {
        ble_trace(1, "%s", __func__);
        return NULL;
    }

    if (0 == find_ptr->saved_list_num)
    {
        TRACE(3,"%s Find Failed, ptr:%x, list_num:%d", __func__, (uint32_t)find_ptr, find_ptr->saved_list_num);
        return NULL;
    }

    uint8_t * bleDeviecAddress = NULL;
    for (uint8_t find_index = 0; find_index < find_ptr->saved_list_num; find_index++)
    {
        //find a arbitrary peer device address that is different from local device address in nvrecord.
        if (memcmp(find_ptr->ble_nv[find_index].pairingInfo.peer_addr.addr, find_ptr->self_info.ble_addr, BLE_ADDR_SIZE))
        {
            ble_trace(2,"%s Find Device Successfully", __func__);
            bleDeviecAddress = find_ptr->ble_nv[find_index].pairingInfo.peer_addr.addr;
            break;
        }
    }
    return (uint8_t*)bleDeviecAddress;
}

static void nvrecord_refresh_rand_seed(void)
{
    uint32_t generatedSeed = hal_sys_timer_get();
    for (uint8_t index = 0; index < sizeof(bt_global_addr); index++)
    {
        generatedSeed ^= (((uint32_t)(bt_global_addr[index])) << (hal_sys_timer_get() & 0xF));
    }
    srand(generatedSeed);
}

static void nvrecord_gen_local_random(uint8_t *random)
{
    for (int index = 0; index < BLE_IRK_SIZE; index++)
    {
        random[index] = (uint8_t)rand();
    }
}

void nvrecord_rebuild_paired_ble_dev_info(NV_RECORD_PAIRED_BLE_DEV_INFO_T *pPairedBtInfo)
{
    memset(( uint8_t * )pPairedBtInfo, 0, sizeof(NV_RECORD_PAIRED_BLE_DEV_INFO_T));
    pPairedBtInfo->saved_list_num = 0;  //init saved num
    memset(pPairedBtInfo->local_database_hash, 0, 16);
    nvrecord_refresh_rand_seed(); //avoid ble irk collision low probability
    nvrecord_gen_local_random(pPairedBtInfo->self_info.ble_irk);
    nvrecord_gen_local_random(pPairedBtInfo->self_info.ble_csrk);
    memset(pPairedBtInfo->gatt_cache, 0, sizeof(pPairedBtInfo->gatt_cache));
}

#define NV_BLE_DONT_UPDATE 0
#define NV_BLE_ADD_NEW_ITEM 1
#define NV_BLE_UPDATE_EXIST 2

static uint8_t blerec_specific_value_prepare(const BleDevicePairingInfo *param_rec)
{
    // Preparations before adding new ble record:
    // 1. If not existing. Check the record count. If it's BLE_RECORD_NUM,
    //     move 0-(BLE_RECORD_NUM-2) right side by one slot, to discard the last one and
    //     leave slot 0, decrease the record number.
    //     If it's smaller than BLE_RECORD_NUM, move 0-(count-1) right side by one slot, leave slot 0,
    //     don't change the record number.
    // 2. If existing already and the location is entryToFree , move 0-(entryToFree-1)
    //        right side by one slot, leave slot 0. Decrease the record number for adding the new one.

    bool isEntryExisting = false;
    uint8_t entryToFree  = 0;
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *dest_ptr;

    dest_ptr = nvrecord_ble_p;
    TRACE(3,"%s start search addr %p  list_num=%d", __func__, dest_ptr, dest_ptr->saved_list_num);

    if (dest_ptr->saved_list_num > 0)
    {
        for (uint8_t i = 0; i < dest_ptr->saved_list_num; i++)
        {
            if (0 == memcmp(dest_ptr->ble_nv[i].pairingInfo.peer_addr.addr, param_rec->peer_addr.addr, BLE_ADDR_SIZE))
            {
                if (!memcmp(( uint8_t * )param_rec, ( uint8_t * )&dest_ptr->ble_nv[i].pairingInfo, sizeof(BleDevicePairingInfo)))
                {
                    ble_trace(0,"The new coming BLE device info is the same as the recorded.");
                    return NV_BLE_DONT_UPDATE;
                }
                uint32_t lock = nv_record_pre_write_operation();
                memset(&(dest_ptr->ble_nv[i].pairingInfo), 0, sizeof(BleDevicePairingInfo));
                entryToFree = i;
                dest_ptr->saved_list_num--;
                isEntryExisting = true;
                nv_record_post_write_operation(lock);
                break;
            }
        }
    }
    else
    {
        return NV_BLE_ADD_NEW_ITEM;
    }

    uint32_t lock = nv_record_pre_write_operation();
    if (!isEntryExisting)
    {
        if (BLE_RECORD_NUM == dest_ptr->saved_list_num)
        {
            // [0,1,2,3,4] -> [x,0,1,2,3]
            for (uint8_t k = 0; k < BLE_RECORD_NUM - 1; k++)
            {
                memcpy(&(dest_ptr->ble_nv[BLE_RECORD_NUM - 1 - k]),
                       &(dest_ptr->ble_nv[BLE_RECORD_NUM - 2 - k]),
                       sizeof(BleDeviceinfo));
            }
            dest_ptr->saved_list_num--;
            memset(&(dest_ptr->ble_nv[0]), 0, sizeof(BleDeviceinfo));
        }
        else
        {
            // [0,1,2,.,.] -> [x,0,1,2,.]
            if (dest_ptr->saved_list_num > 0)
            {
                for (uint8_t k = 0; k < dest_ptr->saved_list_num; k++)
                {
                    memcpy(&(dest_ptr->ble_nv[dest_ptr->saved_list_num - k]),
                           &(dest_ptr->ble_nv[dest_ptr->saved_list_num - 1 - k]),
                           sizeof(BleDeviceinfo));
                }
                memset(&(dest_ptr->ble_nv[0]), 0, sizeof(BleDeviceinfo));
            }
        }
    }
    else
    {
        /**
         * entryToFree == 0:
         *      [x,1,2,3,4] -> [x,1,2,3,4]
         * entryToFree == 1:
         *      [0,x,2,3,4] -> [x,0,2,3,4]
         * entryToFree == 2:
         *      [0,1,x,3,4] -> [x,0,1,3,4]
         */
        BleDeviceinfo *exist = (BleDeviceinfo *)bes_bt_buf_malloc(sizeof(BleDeviceinfo));
        if (exist != NULL)
        {
            *exist = dest_ptr->ble_nv[entryToFree];
        }
        for (uint8_t list_updata = 0; list_updata < entryToFree; list_updata++)
        {
            memcpy(&(dest_ptr->ble_nv[entryToFree - list_updata]),
                   &(dest_ptr->ble_nv[entryToFree - list_updata - 1]),
                   sizeof(BleDeviceinfo));
        }
        if (exist)
        {
            dest_ptr->ble_nv[0] = *exist; // dont drop volume
            bes_bt_buf_free((uint8_t*)exist);
        }
    }
    nv_record_post_write_operation(lock);

    return isEntryExisting ? NV_BLE_UPDATE_EXIST : NV_BLE_ADD_NEW_ITEM;
}

void nv_record_blerec_init(void)
{
    uint32_t lock = nv_record_pre_write_operation();
    if (NULL == nvrecord_ble_p)
    {
        nvrecord_ble_p = &(nvrecord_extension_p->ble_pair_info);
        if (!memcmp(nvrecord_ble_p->self_info.ble_addr, INVALID_ADDR, BTIF_BD_ADDR_SIZE))
        {
            memcpy(nvrecord_ble_p->self_info.ble_addr, bt_get_ble_local_address(), BTIF_BD_ADDR_SIZE);
        }
    }
    nv_record_post_write_operation(lock);
}

NV_RECORD_PAIRED_BLE_DEV_INFO_T* nv_record_blerec_get_ptr(void)
{
   return nvrecord_ble_p;
}

uint8_t nv_record_get_paired_ble_dev_info_list_num(void)
{
    if (NULL == nvrecord_ble_p)
    {
        return 0;
    }

    return nvrecord_ble_p->saved_list_num;
}

bool nv_record_get_ble_pairing_info_through_list_index(uint8_t listIndex, BleDevicePairingInfo* blePairInfo)
{
    uint8_t nv_ble_list_num = nvrecord_ble_p->saved_list_num;
    if ((0 == nv_ble_list_num) || (NULL == nvrecord_ble_p) || (listIndex >= nv_ble_list_num))
    {
        return false;
    }

    memcpy(blePairInfo, &(nvrecord_ble_p->ble_nv[listIndex].pairingInfo), sizeof(BleDevicePairingInfo));

    return true;
}

void nv_record_blerec_get_local_irk(uint8_t *pIrk)
{
    memcpy(pIrk, nvrecord_ble_p->self_info.ble_irk, BLE_IRK_SIZE);
}

bool nv_record_blerec_get_bd_addr_from_irk(uint8_t *pBdAddr, uint8_t *pIrk)
{
    if (nvrecord_ble_p->saved_list_num > 0)
    {
        for (uint8_t index = 0; index < nvrecord_ble_p->saved_list_num; index++)
        {
            if (!memcmp(pIrk, nvrecord_ble_p->ble_nv[index].pairingInfo.IRK, BLE_IRK_SIZE))
            {
                memcpy(pBdAddr, nvrecord_ble_p->ble_nv[index].pairingInfo.peer_addr.addr, BLE_ADDR_SIZE);
                return true;
            }
        }
        return false;
    }
    else
    {
        return false;
    }
}

bool nv_record_blerec_get_irk_from_addr(uint8_t* pBdAddr, uint8_t* pIrk)
{
    if (NULL == nvrecord_ble_p)
    {
        return false;
    }
    if (nvrecord_ble_p->saved_list_num > 0)
    {
        for (uint8_t index = 0; index < nvrecord_ble_p->saved_list_num; index++)
        {
            if (!memcmp(pBdAddr, nvrecord_ble_p->ble_nv[index].pairingInfo.peer_addr.addr, BLE_ADDR_SIZE))
            {
                memcpy(pIrk, nvrecord_ble_p->ble_nv[index].pairingInfo.IRK, BLE_IRK_SIZE);
                return true;
            }
        }
        return false;
    }
    else
    {
        return false;
    }
}
/*
return:
    -1:     enum dev failure.
    0:      without paired dev.
    1:      only 1 paired dev,store@record1.
    2:      get 2 paired dev.notice:record1 is the latest record.
*/
int nv_record_blerec_enum_latest_two_paired_dev(BleDeviceinfo* record1, BleDeviceinfo* record2)
{
    if((NULL == record1) || (NULL == record2) || (NULL == nvrecord_ble_p))
    {
        return -1;
    }

    if (nvrecord_ble_p->saved_list_num > 0)
    {
        if (1 == nvrecord_ble_p->saved_list_num)
        {
            memcpy((uint8_t *)record1, (uint8_t *)&(nvrecord_ble_p->ble_nv[0]),
                   sizeof(BleDeviceinfo));
            return 1;
        }
        else
        {
            memcpy((uint8_t *)record1, (uint8_t *)&(nvrecord_ble_p->ble_nv[0]),
                   sizeof(BleDeviceinfo));
            memcpy((uint8_t *)record2, (uint8_t *)&(nvrecord_ble_p->ble_nv[1]),
                   sizeof(BleDeviceinfo));
            return 2;
        }
    }
    else
    {
        return 0;
    }
}

/*
return:
    -1:     enum dev failure.
    other:  the num of paired dev.
*/
uint8_t nv_record_blerec_enum_paired_dev_addr(BLE_ADDR_INFO_T *addr)
{
    if((NULL == addr) || (NULL == nvrecord_ble_p))
    {
        ASSERT(0, "%s", __func__);
    }

    for (uint8_t num = 0; num < nvrecord_ble_p->saved_list_num; num++)
    {
        memcpy(&addr[num],
               &nvrecord_ble_p->ble_nv[num].pairingInfo.peer_addr,
               sizeof(BLE_ADDR_INFO_T));
    }

    return nvrecord_ble_p->saved_list_num;
}

bool nv_record_blerec_get_paired_dev_from_addr(BleDevicePairingInfo* record, BLE_ADDR_INFO_T *pBdAddr)
{
    if((NULL == record) || (NULL == pBdAddr) || (NULL == nvrecord_ble_p))
    {
        return false;
    }

    for (uint8_t num = 0; num < nvrecord_ble_p->saved_list_num; num++)
    {
        if (!memcmp(&nvrecord_ble_p->ble_nv[num].pairingInfo.peer_addr, pBdAddr, sizeof(BLE_ADDR_INFO_T)))
        {
            memcpy(record, (uint8_t *)&(nvrecord_ble_p->ble_nv[num]),
                   sizeof(BleDevicePairingInfo));

            return true;
        }
    }

    return false;
}

bool nv_record_blerec_is_paired_from_addr(uint8_t *pBdAddr)
{
    if((NULL == pBdAddr) || (NULL == nvrecord_ble_p))
    {
        return false;
    }

    for (uint8_t num = 0; num < nvrecord_ble_p->saved_list_num; num++)
    {
        if (!memcmp(&nvrecord_ble_p->ble_nv[num].pairingInfo.peer_addr.addr, pBdAddr, BLE_ADDR_SIZE))
        {
            return true;
        }
    }

    return false;
}

void nv_recored_blerec_dump()
{
    for (uint8_t k = 0; k < nvrecord_ble_p->saved_list_num; k++)
    {
        TRACE(0,"=========================================");
        TRACE(1,"Num %d BLE record:", k);
        TRACE(0,"BLE addr:");
        DUMP8("%02x ", ( uint8_t * )nvrecord_ble_p->ble_nv[k].pairingInfo.peer_addr.addr, BT_ADDR_OUTPUT_PRINT_NUM);
        TRACE(1,"BLE addr type %d", nvrecord_ble_p->ble_nv[k].pairingInfo.peer_addr.addr_type);
        TRACE(0,"BLE bond bf %02x enc_key_size %d", nvrecord_ble_p->ble_nv[k].pairingInfo.bond_info_bf,
              nvrecord_ble_p->ble_nv[k].pairingInfo.enc_key_size);
        TRACE(1,"NV EDIV %d and random is:", nvrecord_ble_p->ble_nv[k].pairingInfo.EDIV);
        DUMP8("%02x ", ( uint8_t * )nvrecord_ble_p->ble_nv[k].pairingInfo.RANDOM, BLE_ENC_RANDOM_SIZE);
        TRACE(0,"NV LTK:");
        DUMP8("%02x ", ( uint8_t * )nvrecord_ble_p->ble_nv[k].pairingInfo.LTK, BLE_LTK_SIZE);
        TRACE(0,"NV irk:");
        DUMP8("%02x ", ( uint8_t * )nvrecord_ble_p->ble_nv[k].pairingInfo.IRK, BLE_IRK_SIZE);
    }
}

void nv_record_blerec_set_change_unaware(void)
{
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *dest_ptr = nvrecord_ble_p;
    BleDevicePairingInfo *pair_info = NULL;

    uint32_t lock = nv_record_pre_write_operation();

    for (uint8_t i = 0; i < dest_ptr->saved_list_num; i++)
    {
        pair_info = &dest_ptr->ble_nv[i].pairingInfo;
        pair_info->server_cache.service_change_unaware = true;
    }

    nv_record_post_write_operation(lock);

    nv_record_update_runtime_userdata();
    // nv_record_execute_async_flush();
}

void nv_record_blerec_update_database_hash(const uint8_t *hash)
{
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *dev_info = nv_record_blerec_get_ptr();
    uint32_t lock = nv_record_pre_write_operation();

    memcpy(dev_info->local_database_hash, hash, 16);

    nv_record_post_write_operation(lock);

    nv_record_update_runtime_userdata();
    // nv_record_execute_async_flush();
}

const gattc_nv_cache_t *nv_record_ble_gatt_cache_get(const BLE_ADDR_INFO_T *p_addr)
{
    if (NULL == nvrecord_ble_p || p_addr == NULL)
    {
        return NULL;
    }

    int i = 0;
    gattc_nv_cache_t *curr = NULL;
    gattc_nv_cache_t *found = NULL;

    for (i = 0; i < BLE_GATT_CACHE_NUM; i += 1)
    {
        curr = nvrecord_ble_p->gatt_cache + i;
        // Compare addr to match nv cache
        if (memcmp(&curr->peer_addr, p_addr, sizeof(*p_addr)) == 0)
        {
            found = curr;
            break;
        }
    }

    return found;
}

int nv_record_ble_gatt_cache_add(const BLE_ADDR_INFO_T *p_addr, uint8_t cache_seq, const void *client_cache)
{
    if (NULL == nvrecord_ble_p || p_addr == NULL || client_cache == NULL)
    {
        return -1;
    }

    int i = 0;
    uint8_t smallest_seqn = 0xff;
    gattc_nv_cache_t *curr = NULL;
    gattc_nv_cache_t *found = NULL;
    gattc_nv_cache_t *small = NULL;

    uint32_t lock = nv_record_pre_write_operation();

    if (client_cache)
    {
        for (i = 0; i < BLE_GATT_CACHE_NUM; i += 1)
        {
            // Go fetch seq first
            curr = nvrecord_ble_p->gatt_cache + i;
            if (curr->cache_seqn < smallest_seqn)
            {
                smallest_seqn = curr->cache_seqn;
                small = curr;
            }
            // Compare addr to match nv cache
            if (found == NULL && curr->cache_seqn &&
                memcmp(&curr->peer_addr, p_addr, sizeof(*p_addr)) == 0)
            {
                found = curr;
            }
        }

        if (found == NULL && small != NULL)
        {
            found = small;
            found->peer_addr = *p_addr;
            found->cache_seqn = cache_seq;
        }

        if (found != NULL)
        {
            // This call store service character cache
            found->client_cache = *((gattc_client_cache_t *)client_cache);
            TRACE(3,"%s add ble gatt cache success, seq:%d index:%d, list_num:%d",
                                            __func__, cache_seq, i, BLE_GATT_CACHE_NUM);
        }
    }

    nv_record_post_write_operation(lock);

    nv_record_update_runtime_userdata();
    // nv_record_execute_async_flush();

    return 0;
}

int nv_record_ble_gatt_cache_del(const BLE_ADDR_INFO_T *p_addr)
{
    if (NULL == nvrecord_ble_p || p_addr == NULL)
    {
        return -1;
    }

    int i = 0;
    gattc_nv_cache_t *curr = NULL;
    gattc_nv_cache_t *found = NULL;

    uint32_t lock = nv_record_pre_write_operation();

    for (i = 0; i < BLE_GATT_CACHE_NUM; i += 1)
    {
        curr = nvrecord_ble_p->gatt_cache + i;
        // Compare addr to match nv cache
        if (curr->cache_seqn &&
            memcmp(&curr->peer_addr, p_addr, sizeof(*p_addr)) == 0)
        {
            found = curr;
            break;
        }
    }

    if (found != NULL)
    {
        TRACE(3,"%s del ble gatt cache success, index:%d, list_num:%d", __func__,
                                                        i, BLE_GATT_CACHE_NUM);
        memset(found, 0, sizeof(*found));
    }

    nv_record_post_write_operation(lock);

    nv_record_update_runtime_userdata();
    // nv_record_execute_async_flush();

    return 0;
}

const gattc_server_cache_t *nv_record_ble_server_cache_get(const BLE_ADDR_INFO_T *p_addr)
{
    BleDevicePairingInfo *paired_info = NULL;

    if (NULL == nvrecord_ble_p || p_addr == NULL)
    {
        return NULL;
    }

    for (uint8_t num = 0; num < nvrecord_ble_p->saved_list_num; num++)
    {
        paired_info = &nvrecord_ble_p->ble_nv[num].pairingInfo;

        if (!memcmp(&paired_info->peer_addr, p_addr, sizeof(BLE_ADDR_INFO_T)))
        {
            return &paired_info->server_cache;
        }
    }

    return NULL;
}

int nv_record_ble_server_cache_add(const BLE_ADDR_INFO_T *p_addr, const void *server_cache)
{
    BleDevicePairingInfo *paired_info = NULL;

    if (NULL == nvrecord_ble_p || p_addr == NULL || server_cache == NULL)
    {
        return -1;
    }

    uint32_t lock = nv_record_pre_write_operation();

    for (uint8_t num = 0; num < nvrecord_ble_p->saved_list_num; num++)
    {
        paired_info = &nvrecord_ble_p->ble_nv[num].pairingInfo;

        if (!memcmp(&paired_info->peer_addr, p_addr, sizeof(BLE_ADDR_INFO_T)))
        {
            TRACE(3,"%s add ble server cache success, index:%d, list_num:%d", __func__,
                                                        num, nvrecord_ble_p->saved_list_num);
            paired_info->server_cache = *(gattc_server_cache_t *)server_cache;
            break;
        }
    }

    nv_record_post_write_operation(lock);

    nv_record_update_runtime_userdata();
    // nv_record_execute_async_flush();

    return 0;
}

int nv_record_ble_server_cache_del(const BLE_ADDR_INFO_T *p_addr)
{
    if (NULL == nvrecord_ble_p || p_addr == NULL)
    {
        return -1;
    }

    BleDevicePairingInfo *paired_info = NULL;
    gattc_server_cache_t empty_cache = {0};

    uint32_t lock = nv_record_pre_write_operation();

    for (uint8_t num = 0; num < nvrecord_ble_p->saved_list_num; num++)
    {
        paired_info = &nvrecord_ble_p->ble_nv[num].pairingInfo;

        if (!memcmp(&paired_info->peer_addr, p_addr, sizeof(BLE_ADDR_INFO_T)))
        {
            TRACE(3,"%s del ble server cache success, index:%d, list_num:%d", __func__,
                                                        num, nvrecord_ble_p->saved_list_num);
            paired_info->server_cache = empty_cache;
            break;
        }
    }

    nv_record_post_write_operation(lock);

    nv_record_update_runtime_userdata();
    // nv_record_execute_async_flush();

    return 0;
}

int nv_record_blerec_add(const BleDevicePairingInfo *param_rec)
{
    int nRet = 0;
    uint8_t isNeedToUpdateNv = blerec_specific_value_prepare(param_rec);

    if (isNeedToUpdateNv)
    {
        uint32_t lock = nv_record_pre_write_operation();
        BleDeviceinfo *record = &nvrecord_ble_p->ble_nv[0]; // slot 0 is emptied by blerec_specific_value_prepare

        memcpy(&record->pairingInfo, param_rec, sizeof(BleDevicePairingInfo));

        if (isNeedToUpdateNv == NV_BLE_ADD_NEW_ITEM)
        {
#if BLE_AUDIO_ENABLED
            record->volume = bes_ble_aob_convert_local_vol_to_le_vol(
                                    hal_codec_get_default_dac_volume_index());
#endif
        }

        nvrecord_ble_p->saved_list_num++; //updata saved num
        nv_record_post_write_operation(lock);

        nv_record_update_runtime_userdata();
        // nv_record_execute_async_flush();
        TRACE(2,"%s CURRENT BLE LIST NUM=%d", __func__, nvrecord_ble_p->saved_list_num);
    }

#ifdef ble_nv_debug
   nv_recored_blerec_dump();
#endif
    return nRet;
}

uint8_t nv_record_ble_fill_irk(uint8_t *irkToFill)
{
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *find_ptr = nvrecord_ble_p;

    if ((NULL == find_ptr) || (0 == find_ptr->saved_list_num))
    {
        return 0;
    }

    if (find_ptr->saved_list_num > 0)
    {
        for (uint8_t index = 0; index < find_ptr->saved_list_num; index++)
        {
            memcpy(irkToFill + index * BLE_IRK_SIZE, find_ptr->ble_nv[index].pairingInfo.IRK, BLE_IRK_SIZE);
        }
        return find_ptr->saved_list_num;
    }
    else
    {
        return 0;
    }
}

bool nv_record_ble_record_find_ltk(uint8_t *pBdAddr, uint8_t *ltk, uint16_t ediv)
{
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *find_ptr = nvrecord_ble_p;
    uint8_t *pFoundLtk = NULL;

    if(NULL == find_ptr)
    {
        ble_trace(1, "%s", __func__);
        return false;
    }

    if (0 == find_ptr->saved_list_num)
    {
        TRACE(3,"%s find LTK failed, ptr:%p, list_num:%d", __func__,
            find_ptr, find_ptr->saved_list_num);
        return false;
    }

    for (uint32_t find_index = 0; find_index < find_ptr->saved_list_num; find_index++)
    {
        BleDevicePairingInfo *pairingInfo = &find_ptr->ble_nv[find_index].pairingInfo;

        if (!memcmp(pairingInfo->peer_addr.addr, pBdAddr, BLE_ADDR_SIZE))
        {
            if ((ediv == 0) || ((ediv != 0) && (pairingInfo->EDIV == ediv))) {
                pFoundLtk = pairingInfo->LTK;
            } else if (pairingInfo->LOCAL_EDIV == ediv) {
                pFoundLtk = pairingInfo->LOCAL_LTK;
            }

            if (pFoundLtk != NULL) {
                ble_trace(2,"%s FIND LTK IN NV SUCCESS", __func__);
                DUMP8("%02x ", pFoundLtk, BLE_LTK_SIZE);
                memcpy(ltk, pFoundLtk, BLE_LTK_SIZE);
                return true;
            }
        }
    }
    return false;
}

uint8_t *nv_record_ble_record_find_device_security_info_through_static_bd_addr(uint8_t* pBdAddr)
{
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *find_ptr = nvrecord_ble_p;

    if(NULL == find_ptr)
    {
        ble_trace(1, "%s", __func__);
        return NULL;
    }

    if (0 == find_ptr->saved_list_num)
    {
        TRACE(3,"%s Find Failed, ptr:%x, list_num:%d", __func__, (uint32_t)find_ptr, find_ptr->saved_list_num);
        return NULL;
    }

    BleDevicePairingInfo * deviecSecurityInfo = NULL;
    for (uint8_t find_index = 0; find_index < find_ptr->saved_list_num; find_index++)
    {
        if (!memcmp(find_ptr->ble_nv[find_index].pairingInfo.peer_addr.addr, pBdAddr, BLE_ADDR_SIZE))
        {
            ble_trace(2,"%s Find Successfully %p", __func__, find_ptr->ble_nv[find_index].pairingInfo.LTK);
            deviecSecurityInfo = &(find_ptr->ble_nv[find_index].pairingInfo);
        }
    }
    return (uint8_t*)deviecSecurityInfo;
}

bool nv_record_ble_record_Once_a_device_has_been_bonded(void)
{
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *find_ptr = nvrecord_ble_p;

    if ((NULL == find_ptr) || (0 == find_ptr->saved_list_num))
    {
        return false;
    }

    for (uint8_t find_index = 0; find_index < find_ptr->saved_list_num; find_index++)
    {
        if (get_bit(find_ptr->ble_nv[find_index].pairingInfo.bond_info_bf, BONDED_STATUS_POS))
        {
            return true;
        }
    }
    return false;
}

bool nv_record_ble_read_addr_resolv_supp_via_bdaddr(uint8_t *pBdAddr, bool *issupport)
{
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *find_ptr = nvrecord_ble_p;

    if ((!find_ptr) || (0 == find_ptr->saved_list_num)) //BES intentional code.
    {
        TRACE(3, "%s find data failed, ptr:%p", __func__, find_ptr);
        return false;
    }

    ble_trace(2,"%s,ptr=%p,num=%d", __func__, find_ptr, find_ptr->saved_list_num);
    DUMP8("%02x ", pBdAddr, BLE_ADDR_SIZE);

    for (uint8_t find_index = 0; find_index < find_ptr->saved_list_num; find_index++)
    {
        if (!memcmp(find_ptr->ble_nv[find_index].pairingInfo.peer_addr.addr, pBdAddr, BLE_ADDR_SIZE))
        {
           *issupport = get_bit(find_ptr->ble_nv[find_index].pairingInfo.bond_info_bf, BONDED_WITH_IRK_DISTRIBUTED);
            ble_trace(2,"%s READ ADDR_RESOLV_SUPP IN NV SUCCESS support:%d", __func__, *issupport);
            return true;
        }
    }

    return false;
}

bool nv_record_ble_write_addr_resolv_supp_via_bdaddr(uint8_t *pBdAddr, bool issupport)
{
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *find_ptr = nvrecord_ble_p;

    if ((NULL == find_ptr) || (0 == find_ptr->saved_list_num))  //BES intentional code.
    {
        TRACE(3,"%s find data failed, ptr:%p", __func__, find_ptr);
        return false;
    }

    ble_trace(2,"%s,ptr=%p,num=%d,issupport=%d", __func__, find_ptr, find_ptr->saved_list_num, issupport);
    DUMP8("%02x ", pBdAddr, BLE_ADDR_SIZE);

    for (uint8_t find_index = 0; find_index < find_ptr->saved_list_num; find_index++)
    {
        if (!memcmp(find_ptr->ble_nv[find_index].pairingInfo.peer_addr.addr, pBdAddr, BLE_ADDR_SIZE))
        {
            uint32_t lock = nv_record_pre_write_operation();
            nv_record_update_runtime_userdata();
            if (issupport)
            {
                set_bit(find_ptr->ble_nv[find_index].pairingInfo.bond_info_bf, BONDED_WITH_IRK_DISTRIBUTED);
            }
            else
            {
                clr_bit(find_ptr->ble_nv[find_index].pairingInfo.bond_info_bf, BONDED_WITH_IRK_DISTRIBUTED);
            }
            nv_record_post_write_operation(lock);
            ble_trace(2,"%s WRITE ADDR_RESOLV_SUPP IN NV SUCCESS support:%d", __func__, issupport);
            // nv_record_execute_async_flush();
            return true;
        }
    }
    return false;
}


bool nv_record_ble_read_volume_via_bdaddr(uint8_t *pBdAddr, uint8_t *vol)
{
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *find_ptr = nvrecord_ble_p;

    if ((!find_ptr) || (0 == find_ptr->saved_list_num))
    {
        TRACE(3, "%s find data failed, ptr:%p", __func__, find_ptr);
        return false;
    }

    for (uint8_t find_index = 0; find_index < find_ptr->saved_list_num; find_index++)
    {
        if (!memcmp(find_ptr->ble_nv[find_index].pairingInfo.peer_addr.addr, pBdAddr, BLE_ADDR_SIZE))
        {
            *vol = find_ptr->ble_nv[find_index].volume;
            ble_trace(2,"%s READ VOLUME IN NV SUCCESS vol:%d", __func__, *vol);
            return true;
        }
    }

    return false;
}

bool nv_record_ble_write_volume_via_bdaddr(uint8_t *pBdAddr, uint8_t vol)
{
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *find_ptr = nvrecord_ble_p;

    if ((NULL == find_ptr) || (0 == find_ptr->saved_list_num))
    {
        TRACE(3,"%s find data failed, ptr:%p", __func__, find_ptr);
        return false;
    }

    for (uint8_t find_index = 0; find_index < find_ptr->saved_list_num; find_index++)
    {
        if (!memcmp(find_ptr->ble_nv[find_index].pairingInfo.peer_addr.addr, pBdAddr, BLE_ADDR_SIZE))
        {
            uint32_t lock = nv_record_pre_write_operation();
            find_ptr->ble_nv[find_index].volume = vol;
            ble_trace(2,"%s WRITE VOLUME IN NV SUCCESS vol:%d", __func__, vol);

            nv_record_post_write_operation(lock);
#ifndef FPGA
            nv_record_touch_cause_flush();
#endif
            return true;
        }
    }
    return false;
}

void nv_record_ble_delete_entry_by_index(uint32_t index_to_delete)
{
    uint32_t i = 0;
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *find_ptr = nvrecord_ble_p;
    uint32_t lock = 0;

    lock = nv_record_pre_write_operation();

    if (index_to_delete < find_ptr->saved_list_num)
    {
        for (i = index_to_delete; i < find_ptr->saved_list_num - 1; i++)
        {
            memcpy(&(find_ptr->ble_nv[i]),
                   &(find_ptr->ble_nv[i + 1]),
                   sizeof(BleDeviceinfo));
        }

        memset((uint8_t *)&(find_ptr->ble_nv[i]), 0, sizeof(BleDeviceinfo));
        find_ptr->saved_list_num--;
        nv_record_update_runtime_userdata();
    }

    nv_record_post_write_operation(lock);
}

void nv_record_ble_delete_entry(const uint8_t *pBdAddr)
{
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *find_ptr = nvrecord_ble_p;

    if ((NULL == find_ptr) || (0 == find_ptr->saved_list_num))
    {
        return;
    }

    int8_t indexToDelete = -1;

    for (uint8_t find_index = 0; find_index < find_ptr->saved_list_num; find_index++)
    {
        if (!memcmp(find_ptr->ble_nv[find_index].pairingInfo.peer_addr.addr, pBdAddr, BLE_ADDR_SIZE))
        {
            indexToDelete = find_index;
            break;
        }
    }

    if (indexToDelete >= 0)
    {
        nv_record_ble_delete_entry_by_index(indexToDelete);
    }
}

void nv_record_ble_delete_all_entry(void)
{
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *find_ptr = nvrecord_ble_p;
    uint8_t list_num = nv_record_get_paired_ble_dev_info_list_num();

    uint32_t lock = nv_record_pre_write_operation();

    for (uint8_t i = 0;i < list_num;i++)
    {
        memset(( uint8_t * )&(find_ptr->ble_nv[i]), 0, sizeof(BleDeviceinfo));
    }

    find_ptr->saved_list_num = 0;
    nv_record_update_runtime_userdata();

    nv_record_post_write_operation(lock);

}

#ifdef TWS_SYSTEM_ENABLED
static bool tws_use_same_ble_addr(uint8_t *peer_ble_addr)
{
    TRACE(1,"[%s] peer addr:", __func__);
    DUMP8("%x ", peer_ble_addr, BTIF_BD_ADDR_SIZE);

    NV_EXTENSION_RECORD_T *pNvExtRec = nv_record_get_extension_entry_ptr();

    if (!memcmp(pNvExtRec->ble_pair_info.self_info.ble_addr, peer_ble_addr, BTIF_BD_ADDR_SIZE))
    {
        return true;
    }

    return false;
}

void nv_record_extension_update_tws_ble_info(NV_RECORD_PAIRED_BLE_DEV_INFO_T *info)
{
    ASSERT(info, "null pointer received in [%s]", __func__);

    bool isNvExtentionPendingForUpdate = false;
    NV_EXTENSION_RECORD_T *pNvExtRec = nv_record_get_extension_entry_ptr();

    /// disable the MPU protection for write operation
    uint32_t lock = nv_record_pre_write_operation();
#if BLE_AUDIO_ENABLED
    if (tws_use_same_ble_addr(info->self_info.ble_addr))
#endif
    {
        if (memcmp(&pNvExtRec->ble_pair_info.self_info, &info->self_info, sizeof(BLE_BASIC_INFO_T)))
        {
            TRACE(0,"save the peer ble info to self_info");

            memcpy(&pNvExtRec->ble_pair_info.self_info, &info->self_info, sizeof(BLE_BASIC_INFO_T));
            nv_record_extension_update();
            isNvExtentionPendingForUpdate = true;
        }
    }

    if (memcmp(&pNvExtRec->tws_info.ble_info, &info->self_info, sizeof(BLE_BASIC_INFO_T)))
    {
        TRACE(0,"save the peer ble info to tws_info");

        memcpy(&pNvExtRec->tws_info.ble_info, &info->self_info, sizeof(BLE_BASIC_INFO_T));
        nv_record_extension_update();
        isNvExtentionPendingForUpdate = true;
    }

    /// enable the MPU protection after the write operation
    nv_record_post_write_operation(lock);

    if (isNvExtentionPendingForUpdate)
    {
        nv_record_flash_flush();
    }

    TRACE(0,"peer addr:");
    DUMP8("0x%02x ", pNvExtRec->tws_info.ble_info.ble_addr,BTIF_BD_ADDR_SIZE);
    TRACE(0,"peer irk");
    DUMP8("0x%02x ", pNvExtRec->tws_info.ble_info.ble_irk, BLE_IRK_SIZE);
}

void nv_record_tws_exchange_ble_info(void)
{
    TRACE(1,"[%s]+++", __func__);
#ifdef FPGA
    return;
#endif

    // for le audio case, no need to switch ble addr and irk
#if (BLE_AUDIO_ENABLED)
    return;
#endif

    NV_EXTENSION_RECORD_T *pNvExtRec = nv_record_get_extension_entry_ptr();

    if (tws_use_same_ble_addr(pNvExtRec->tws_info.ble_info.ble_addr))
    {
        TRACE(0,"tws use same ble addr");
        return;
    }

    if (!memcmp(pNvExtRec->tws_info.ble_info.ble_addr, INVALID_ADDR, BTIF_BD_ADDR_SIZE))
    {
        TRACE(0,"don't have tws ble addr");
        return;
    }

    /// disable the MPU protection for write operation
    uint32_t lock = nv_record_pre_write_operation();

#ifdef BLE_ADV_RPA_ENABLED
    uint8_t temp_ble_irk[BLE_IRK_SIZE];
    memcpy(temp_ble_irk, pNvExtRec->ble_pair_info.self_info.ble_irk, BLE_IRK_SIZE);
    memcpy(pNvExtRec->ble_pair_info.self_info.ble_irk, pNvExtRec->tws_info.ble_info.ble_irk, BLE_IRK_SIZE);
    memcpy(pNvExtRec->tws_info.ble_info.ble_irk, temp_ble_irk, BLE_IRK_SIZE);
    TRACE(0,"current local ble irk:");
    DUMP8("0x%02x ", pNvExtRec->ble_pair_info.self_info.ble_irk, BLE_IRK_SIZE);
#else
    uint8_t temp_ble_addr[BTIF_BD_ADDR_SIZE];
    memcpy(temp_ble_addr, pNvExtRec->ble_pair_info.self_info.ble_addr, BTIF_BD_ADDR_SIZE);
    memcpy(pNvExtRec->ble_pair_info.self_info.ble_addr, pNvExtRec->tws_info.ble_info.ble_addr, BTIF_BD_ADDR_SIZE);
    memcpy(pNvExtRec->tws_info.ble_info.ble_addr, temp_ble_addr, BTIF_BD_ADDR_SIZE);
    memcpy(bt_get_ble_local_address(), pNvExtRec->ble_pair_info.self_info.ble_addr, BTIF_BD_ADDR_SIZE);
    TRACE(0,"current local ble addr:");
    DUMP8("0x%02x ", pNvExtRec->ble_pair_info.self_info.ble_addr, BTIF_BD_ADDR_SIZE);

    bt_set_ble_local_address(pNvExtRec->ble_pair_info.self_info.ble_addr);
#endif

    nv_record_extension_update();

    /// enable the MPU protection after the write operation
    nv_record_post_write_operation(lock);

    TRACE(1,"[%s]---", __func__);
}

uint8_t *nv_record_tws_get_self_ble_info(void)
{
    TRACE(1,"[%s]+++", __func__);
    NV_EXTENSION_RECORD_T *pNvExtRec = nv_record_get_extension_entry_ptr();

    TRACE(0,"current local ble addr:");
    DUMP8("0x%02x ", pNvExtRec->ble_pair_info.self_info.ble_addr, BTIF_BD_ADDR_SIZE);

    TRACE(1,"[%s]---", __func__);
    return pNvExtRec->ble_pair_info.self_info.ble_addr;
}
#endif

#ifdef CTKD_ENABLE
static const uint8_t tmp1[TMP1LEN] = {0x74,0x6d,0x70,0x31};
static const uint8_t lebr[LEBRLEN] = {0x6c,0x65,0x62,0x72};
static const uint8_t salt[SALTLEN] =
        {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x74,0x6D,0x70,0x31};

extern void *app_bt_profile_active_store_ptr_get(uint8_t *bdAddr);
static void function_h6_calc_linkKey(uint8_t *ilk, uint8_t *pBdAddr) //BR/EDR link key = h6(ILK,"lebr")
{
    uint8_t LinkKey[LINKKEYLEN];
    uint8_t tmpLinkKey[LINKKEYLEN];

    aes_cal_cmac(lebr, LEBRLEN, LinkKey, ilk);

    for(uint8_t i = 0; i < LINKKEYLEN; i++)
    {
        tmpLinkKey[i] = LinkKey[LINKKEYLEN - (i + 1)];
    }

    TRACE(0, "tmpLinkKey:");
    DUMP8("%02x ", ( uint8_t * )tmpLinkKey, 16);

    btif_device_record_t record;
    bt_status_t status = BT_STS_FAILED;
    memcpy(record.bdAddr.address, pBdAddr, BLE_ADDR_SIZE);
    memcpy(record.linkKey, tmpLinkKey, LINKKEYLEN);
    record.keyType = BTIF_UNAUTH_COMBINATION_KEY;

    status = ddbif_add_record(&record);
    if (status != BT_STS_SUCCESS) {
        TRACE(0, "save link key fail");
    }
    else
    {
        btdevice_profile *btdevice_plf_p =
            (btdevice_profile *)app_bt_profile_active_store_ptr_get(record.bdAddr.address);
        nv_record_btdevicerecord_set_a2dp_profile_active_state(btdevice_plf_p, true);
        nv_record_btdevicerecord_set_hfp_profile_active_state(btdevice_plf_p, true);
    }
}

static void function_calc_ilk(uint8_t *ltk, uint8_t *ilk, bool ct2)
{
    uint8_t tmpLtk[BLE_LTK_SIZE];

    for(uint8_t i = 0; i < BLE_LTK_SIZE; i++)
    {
        tmpLtk[i]  = ltk[BLE_LTK_SIZE - (i + 1)];
    }

    TRACE(1, "%s,CT2:%d, tmpLtk:", __func__, ct2);
    DUMP8("%02x ", (uint8_t *)tmpLtk, BLE_LTK_SIZE);

    if (ct2) {
        // both devices set CT2 = 1, ILK = h7(SALT,LTK)
        aes_cal_cmac(tmpLtk, BLE_LTK_SIZE, ilk, salt);
    } else {
        // at least one device sets CT2 = 0, ILK = h6(LTK,"tmp1")
        aes_cal_cmac(tmp1, TMP1LEN, ilk, tmpLtk);
    }
}

void clac_linkKey(uint8_t *ltk, uint8_t *pBdAddr, bool ct2)
{
    uint8_t ilk[16];

    function_calc_ilk(ltk, ilk, ct2);
    function_h6_calc_linkKey(ilk, pBdAddr);
}
#endif

#if BLE_AUDIO_ENABLED
static NV_RECORD_BLE_AUDIO_DEV_INFO_T *nvrecord_bleaudio_p = NULL;

void nv_record_bleaudio_init(void)
{
    uint32_t lock = nv_record_pre_write_operation();
    uint8_t index = 0;
    uint8_t INVALID_SIRK[16] = {0,};
    uint8_t INVALID_SIRK_F[16] = {0xff,0xff,0xff,0xff,
                                  0xff,0xff,0xff,0xff,
                                  0xff,0xff,0xff,0xff,
                                  0xff,0xff,0xff,0xff};

    if (NULL == nvrecord_bleaudio_p)
    {
        nvrecord_bleaudio_p = &(nvrecord_extension_p->ble_audio_dev_info);
        bool use_custom_sirk = bes_ble_aob_csip_is_use_custom_sirk();
        bool sirk_full_ff =(!memcmp(nvrecord_bleaudio_p->sirk, INVALID_SIRK_F, 16));
        TRACE(0,"%s sirk:", __func__);
        DUMP8("%02x ", nvrecord_bleaudio_p->sirk, 16);
        if ((!memcmp(nvrecord_bleaudio_p->sirk, INVALID_SIRK, 16))||sirk_full_ff)
        {
            TRACE(0, "init csip sirk value");
            if (!use_custom_sirk)
            {
                uint32_t generatedSeed = hal_sys_timer_get();
                for (uint8_t index = 0; index < sizeof(bt_global_addr); index++)
                {
                    generatedSeed ^= (((uint32_t)(bt_global_addr[index])) << (hal_sys_timer_get()&0xF));
                }
                srand(generatedSeed);
                for (index = 0; index < 16; index++)
                {
                    nvrecord_bleaudio_p->sirk[index] = rand()%255 + 1;
                }
            }

            nvrecord_bleaudio_p->set_member = 0;
        }
    }
    nv_record_post_write_operation(lock);
}

NV_RECORD_BLE_AUDIO_DEV_INFO_T* nv_record_bleaudio_get_ptr(void)
{
   return nvrecord_bleaudio_p;
}

void nv_record_bleaudio_update_devinfo(uint8_t *info)
{
    ASSERT(info, "null pointer received in [%s]", __func__);
    if (!nvrecord_bleaudio_p) {
        return;
    }
    NV_RECORD_BLE_AUDIO_DEV_INFO_T* p_info = (NV_RECORD_BLE_AUDIO_DEV_INFO_T*)info;
    uint32_t lock = nv_record_pre_write_operation();
    nvrecord_bleaudio_p->set_member = p_info->set_member;
    memcpy((uint8_t*)nvrecord_bleaudio_p->sirk, p_info->sirk, 16);
    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
    // nv_record_execute_async_flush();
}
#endif

#ifdef TOTA_CRASH_DUMP_TOOL_ENABLE
void nv_record_blerec_crash_dump(void)
{
    uint8_t i = 0;

    NV_EXTENSION_RECORD_T *pNvExtRec = nv_record_get_extension_entry_ptr();

    CRASH_DUMP_DEBUG_TRACE(0,"tws peer ble addr:");
    DUMP8("0x%02x ", pNvExtRec->tws_info.ble_info.ble_addr,BTIF_BD_ADDR_SIZE);
    CRASH_DUMP_DEBUG_TRACE(0,"tws peer ble irk:");
    DUMP8("0x%02x ", pNvExtRec->tws_info.ble_info.ble_irk, BLE_IRK_SIZE);
    CRASH_DUMP_DEBUG_TRACE(0,"local ble irk:");
    DUMP8("0x%02x ", pNvExtRec->ble_pair_info.self_info.ble_addr,BTIF_BD_ADDR_SIZE);
    CRASH_DUMP_DEBUG_TRACE(0,"local ble irk:");
    DUMP8("0x%02x ", pNvExtRec->ble_pair_info.self_info.ble_irk, BLE_IRK_SIZE);

    if(pNvExtRec->ble_pair_info.saved_list_num > 0)
    {
        for(i = 0; i < pNvExtRec->ble_pair_info.saved_list_num; i++)
        {
            CRASH_DUMP_DEBUG_TRACE(0,"peer paired ble addr type %d addr:", pNvExtRec->ble_pair_info.ble_nv[i].peer_addr.addr_type);
            DUMP8("0x%02x ", pNvExtRec->ble_pair_info.ble_nv[i].pairingInfo.peer_bleAddr,BT_ADDR_OUTPUT_PRINT_NUM);
            CRASH_DUMP_DEBUG_TRACE(0,"peer paired ble LK:");
            DUMP8("0x%02x ", pNvExtRec->ble_pair_info.ble_nv[i].pairingInfo.LTK,BLE_LTK_SIZE);
            CRASH_DUMP_DEBUG_TRACE(0,"tws peer ble irk:");
            DUMP8("0x%02x ", pNvExtRec->ble_pair_info.ble_nv[i].pairingInfo.IRK, BLE_IRK_SIZE);
        }
    }

}

#endif
#endif  //#if defined(NEW_NV_RECORD_ENABLED)
