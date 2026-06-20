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
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "pmu.h"
#include "hal_codec.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "bluetooth_bt_api.h"
#include "app_a2dp.h"
#include "crc32_c.h"
#include "besbt.h"
#include "bt_drv_interface.h"
#include "norflash_api.h"
#include "nvrecord_bt.h"
#include "nvrecord_dev.h"
#include "factory_section.h"
#include "heap_api.h"
#include "besbt.h"

// #define nv_record_verbose_log

#define nvrec_trace TRACE
#define OS_DUMP8(x...)
#define CRASH_DUMP_DEBUG_TRACE(attr, str, ...)  \
    TRACE(((attr) | TR_ATTR_NO_ID), str, ##__VA_ARGS__)

#ifdef IBRT
extern bool app_ibrt_if_is_tws_addr(const uint8_t* pBdAddr);
#endif

#ifdef RAM_NV_RECORD
#define MAX_RECORD_NUM     3
nvrec_btdevicerecord g_fpga_ram_record[MAX_RECORD_NUM];
void ram_record_ddbrec_init(void)
{
    for(uint8_t i=0;i<MAX_RECORD_NUM;i++)
    {
        g_fpga_ram_record[i].record.trusted = false;
    }
}

bt_status_t ram_record_ddbrec_find(const bt_bdaddr_t* bd_ddr, nvrec_btdevicerecord **record)
{
    for(uint8_t i=0;i<MAX_RECORD_NUM;i++)
    {
        if (g_fpga_ram_record[i].record.trusted && \
            !memcmp(&g_fpga_ram_record[i].record.bdAddr.address[0], &bd_ddr->address[0], 6))
        {
            *record = &g_fpga_ram_record[i];
            return BT_STS_SUCCESS;
        }
    }
    return BT_STS_FAILED;
}

bt_status_t ram_record_ddbrec_add(const nvrec_btdevicerecord* param_rec)
{
    uint8_t i=0;
    for(i=0;i<MAX_RECORD_NUM;i++)
    {
        ///find the same address
        if (g_fpga_ram_record[i].record.trusted && \
            !memcmp(&g_fpga_ram_record[i].record.bdAddr.address[0], &param_rec->record.bdAddr.address[0], 6))
        {
            g_fpga_ram_record[i] = *param_rec;
            g_fpga_ram_record[i].record.trusted = true;
            return BT_STS_SUCCESS;
        }
    }
    for(i=0;i<MAX_RECORD_NUM;i++)
    {
        ///find the same address
        if (g_fpga_ram_record[i].record.trusted == false)
        {
            g_fpga_ram_record[i] = *param_rec;
            g_fpga_ram_record[i].record.trusted = true;
            return BT_STS_SUCCESS;
        }
    }

    return BT_STS_FAILED;
}

bt_status_t ram_record_ddbrec_clear(void)
{
    for(int i = 0; i < MAX_RECORD_NUM; i++)
    {
        g_fpga_ram_record[i].record.trusted = false;
    }
    return BT_STS_SUCCESS;
}

bt_status_t ram_record_ddbrec_delete(const bt_bdaddr_t *bdaddr)
{
    uint8_t i;
    for(i=0;i<MAX_RECORD_NUM;i++)
    {

        if (g_fpga_ram_record[i].record.trusted && \
            !memcmp(&g_fpga_ram_record[i].record.bdAddr.address[0], &bdaddr->address[0], 6))
        {
            g_fpga_ram_record[i].record.trusted = false;
        }
    }
    return BT_STS_SUCCESS;
}
#else
void ram_record_ddbrec_init(void)
{
}
#endif
void nvrecord_rebuild_paired_bt_dev_info(NV_RECORD_PAIRED_BT_DEV_INFO_T* pPairedBtInfo)
{
    memset((uint8_t *)pPairedBtInfo, 0, sizeof(NV_RECORD_PAIRED_BT_DEV_INFO_T));

    pPairedBtInfo->pairedDevNum = 0;
}

void nv_record_btdevicerecord_set_a2dp_profile_active_state(btdevice_profile* device_plf, bool isActive)
{
#ifndef RAM_NV_RECORD
    uint32_t lock = nv_record_pre_write_operation();
    if (isActive != device_plf->a2dp_act)
    {
        nv_record_update_runtime_userdata();
    }
    device_plf->a2dp_act = isActive;

    nv_record_post_write_operation(lock);
#else
    device_plf->a2dp_act = isActive;
#endif
}

void nv_record_btdevicerecord_set_hfp_profile_active_state(btdevice_profile* device_plf, bool isActive)
{
#ifndef RAM_NV_RECORD
    uint32_t lock = nv_record_pre_write_operation();
    if (isActive != device_plf->hfp_act)
    {
        nv_record_update_runtime_userdata();
    }
    device_plf->hfp_act = isActive;

    nv_record_post_write_operation(lock);
#else
    device_plf->hfp_act = isActive;
#endif
}

void nv_record_btdevicerecord_set_a2dp_profile_codec(btdevice_profile* device_plf, uint8_t a2dpCodec)
{
#ifndef RAM_NV_RECORD
    uint32_t lock = nv_record_pre_write_operation();
    if (a2dpCodec != device_plf->a2dp_codectype)
    {
        nv_record_update_runtime_userdata();
    }
    device_plf->a2dp_codectype = a2dpCodec;

    nv_record_post_write_operation(lock);
#else
    device_plf->a2dp_codectype = a2dpCodec;
#endif
}

int nv_record_get_paired_dev_count(void)
{
    if (NULL == nvrecord_extension_p)
    {
        return 0;
    }

    return nvrecord_extension_p->bt_pair_info.pairedDevNum;
}

/*
return:
    -1:     enum dev failure.
    0:      without paired dev.
    1:      only 1 paired dev,store@record1.
    2:      get 2 paired dev.notice:record1 is the latest record.
*/
int nv_record_enum_latest_two_paired_dev(btif_device_record_t* record1,btif_device_record_t* record2)
{
    if((NULL == record1) || (NULL == record2) || (NULL == nvrecord_extension_p))
    {
        return -1;
    }

    if (nvrecord_extension_p->bt_pair_info.pairedDevNum > 0)
    {
        if (1 == nvrecord_extension_p->bt_pair_info.pairedDevNum)
        {
            memcpy((uint8_t *)record1, (uint8_t *)&(nvrecord_extension_p->bt_pair_info.pairedBtDevInfo[0]),
                   sizeof(btif_device_record_t));
            return 1;
        }
        else
        {
            memcpy((uint8_t *)record1, (uint8_t *)&(nvrecord_extension_p->bt_pair_info.pairedBtDevInfo[0]),
                   sizeof(btif_device_record_t));
            memcpy((uint8_t *)record2, (uint8_t *)&(nvrecord_extension_p->bt_pair_info.pairedBtDevInfo[1]),
                   sizeof(btif_device_record_t));
            return 2;
        }
    }
    else
    {
        return 0;
    }
}

int nv_record_get_sink_paired_dev(btif_device_record_t *record, uint8_t max_num)
{
    uint32_t tempi = 0;
    if((NULL == record) || (NULL == nvrecord_extension_p))
    {
        return -1;
    }

    for(uint32_t i = 0; i < nvrecord_extension_p->bt_pair_info.pairedDevNum; i++)
    {
        if(nvrecord_extension_p->bt_pair_info.pairedBtDevInfo[i].record.for_bt_source == 1)
        {
            memcpy((uint8_t *)record + tempi * sizeof(btif_device_record_t), (uint8_t *)&(nvrecord_extension_p->bt_pair_info.pairedBtDevInfo[i]),
                   sizeof(btif_device_record_t));
            tempi ++;
        }
        if(tempi >= max_num)
        {
            break;
        }
    }
    return tempi;
}


/*
return:
    -1:     enum dev failure.
    0:      without paired dev.
    x:      paired device number.
*/
int nv_record_get_paired_dev_list(nvrec_btdevicerecord** record)
{
    if((NULL == record) || (NULL == nvrecord_extension_p))
    {
        return -1;
    }
    if (nvrecord_extension_p->bt_pair_info.pairedDevNum > 0)
    {
        *record = nvrecord_extension_p->bt_pair_info.pairedBtDevInfo;
        return nvrecord_extension_p->bt_pair_info.pairedDevNum;
    }
    else
    {
        return 0;
    }
}

static void nv_record_print_dev_record(const btif_device_record_t* record)
{
    TRACE(0,"nv record bdAddr = ");
    DUMP8("%02x ",record->bdAddr.address,BT_ADDR_OUTPUT_PRINT_NUM);
#ifdef nv_record_verbose_log
    TRACE(0,"record_trusted = ");
    DUMP8("%d ",&record->trusted,sizeof((uint8_t)record->trusted));
    TRACE(0,"record_for_bt_source = ");
    DUMP8("%d ",&record->for_bt_source,sizeof((uint8_t)record->for_bt_source));
#endif
    TRACE(0,"record_linkKey = ");
    DUMP8("%02x ",record->linkKey,sizeof(record->linkKey));
    TRACE(0, "remote device name: %s", record->remote_dev_name);
#ifdef nv_record_verbose_log
    TRACE(0,"record_keyType = ");
    DUMP8("%x ",&record->keyType,sizeof(record->keyType));
    TRACE(0,"record_pinLen = ");
    DUMP8("%x ",&record->pinLen,sizeof(record->pinLen));
#endif
}

void nv_record_all_ddbrec_print(void)
{
    if (NULL == nvrecord_extension_p)
    {
        TRACE(0,"No BT paired dev.");
        return;
    }

    if (nvrecord_extension_p->bt_pair_info.pairedDevNum > 0)
    {
        for(uint8_t tmp_i=0; tmp_i < nvrecord_extension_p->bt_pair_info.pairedDevNum; tmp_i++)
        {
            btif_device_record_t record;
            bt_status_t ret_status;
            ret_status = nv_record_enum_dev_records(tmp_i, &record);
            if (BT_STS_SUCCESS == ret_status)
            {
                nv_record_print_dev_record(&record);
            }
        }
    }
    else
    {
        TRACE(0,"No BT paired dev.");
    }
}

/*
this function should be surrounded by OS_LockStack and OS_UnlockStack when call.
*/
bt_status_t nv_record_enum_dev_records(unsigned short index,btif_device_record_t* record)
{
    btif_device_record_t *recaddr = NULL;

    if((index >= nvrecord_extension_p->bt_pair_info.pairedDevNum) || (NULL == nvrecord_extension_p))
    {
        return BT_STS_FAILED;
    }

    recaddr = (btif_device_record_t *)&(nvrecord_extension_p->bt_pair_info.pairedBtDevInfo[index].record);
    memcpy(record, recaddr, sizeof(btif_device_record_t));
    //nv_record_print_dev_record(record);
    return BT_STS_SUCCESS;
}

static int8_t nv_record_get_bt_pairing_info_index(const uint8_t* btAddr)
{
    NV_RECORD_PAIRED_BT_DEV_INFO_T* pBtDevInfo =
        (NV_RECORD_PAIRED_BT_DEV_INFO_T *)(&(nvrecord_extension_p->bt_pair_info));

    for (uint8_t index = 0; index < pBtDevInfo->pairedDevNum; index++)
    {
        if (!memcmp(pBtDevInfo->pairedBtDevInfo[index].record.bdAddr.address,
                    btAddr, BTIF_BD_ADDR_SIZE))
        {
            return (int8_t)index;
        }
    }

    return -1;
}

static uint8_t LatestNewlyPairedDeviceBtAddr[BTIF_BD_ADDR_SIZE];
static NEWDEVICE_PAIRED_CB_T new_device_paired_cb[NEW_DEVICE_CB_NUM] = {NULL};

static void nv_record_btdevice_new_device_paired_default_callback(const uint8_t* btAddr)
{
    TRACE(0, "New device paired:");
    DUMP8("%02x ", btAddr, BT_ADDR_OUTPUT_PRINT_NUM);
    memcpy(LatestNewlyPairedDeviceBtAddr, btAddr, sizeof(LatestNewlyPairedDeviceBtAddr));
}

uint8_t* nv_record_btdevice_get_latest_paired_device_bt_addr(void)
{
    return LatestNewlyPairedDeviceBtAddr;
}

void nv_record_bt_device_register_newly_paired_device_callback(NEW_DEVICE_CB_E index, nv_record_btdevice_new_device_paired_func_t func)
{
    new_device_paired_cb[index] = func;
}

void nvrecord_bt_init(void)
{
    new_device_paired_cb[NEW_DEVICE_CB_MOBILE] = NULL;
    new_device_paired_cb[NEW_DEVICE_CB_TWS] = NULL;
    new_device_paired_cb[NEW_DEVICE_CB_ANY_DEVICE] = nv_record_btdevice_new_device_paired_default_callback;
    new_device_paired_cb[NEW_DEVICE_CB_USER1] = NULL;
}

static void nv_record_bt_distribute_new_dev_paired_event(uint8_t* pBtAddr)
{
    if (new_device_paired_cb[NEW_DEVICE_CB_ANY_DEVICE])
    {
        new_device_paired_cb[NEW_DEVICE_CB_ANY_DEVICE](pBtAddr);
    }
    if (new_device_paired_cb[NEW_DEVICE_CB_USER1])
    {
        new_device_paired_cb[NEW_DEVICE_CB_USER1](pBtAddr);
    }

#ifdef IBRT
    if (app_ibrt_if_is_tws_addr(pBtAddr))
    {
        if (new_device_paired_cb[NEW_DEVICE_CB_TWS])
        {
            new_device_paired_cb[NEW_DEVICE_CB_TWS](pBtAddr);
        }
    }
    else
    {
        if (new_device_paired_cb[NEW_DEVICE_CB_MOBILE])
        {
            new_device_paired_cb[NEW_DEVICE_CB_MOBILE](pBtAddr);
        }
    }
#else
    if (new_device_paired_cb[NEW_DEVICE_CB_MOBILE])
    {
        new_device_paired_cb[NEW_DEVICE_CB_MOBILE](pBtAddr);
    }
#endif
}

/**********************************************
this function should be surrounded by OS_LockStack and OS_UnlockStack when call.
**********************************************/
static void nv_record_pairing_info_reset(nvrec_btdevicerecord* pPairingInfo)
{
    uint32_t default_dac_volume = hal_codec_get_default_dac_volume_index();
    pPairingInfo->device_vol.a2dp_vol = hal_codec_get_default_dac_volume_index();
    pPairingInfo->device_vol.hfp_vol = hal_codec_get_default_dac_volume_index();
    pPairingInfo->device_plf.hfp_act = false;
    pPairingInfo->device_plf.a2dp_abs_vol = a2dp_convert_local_vol_to_bt_vol(default_dac_volume);
    pPairingInfo->device_plf.a2dp_act = false;
}

static bt_status_t POSSIBLY_UNUSED nv_record_ddbrec_add(const btif_device_record_t* param_rec)
{
    if ((NULL == param_rec) || (NULL == nvrecord_extension_p))
    {
        return BT_STS_FAILED;
    }

    uint32_t lock = nv_record_pre_write_operation();

    bool isUpdateNv = false;

    // try to find the entry
    int8_t indexOfEntry = -1;
    NV_RECORD_PAIRED_BT_DEV_INFO_T* pBtDevInfo =
        (NV_RECORD_PAIRED_BT_DEV_INFO_T *)(&(nvrecord_extension_p->bt_pair_info));
    indexOfEntry = nv_record_get_bt_pairing_info_index(param_rec->bdAddr.address);

    if (-1 == indexOfEntry)
    {
        // don't exist,  need to add to the head of the entry list
        if (MAX_BT_PAIRED_DEVICE_COUNT == pBtDevInfo->pairedDevNum)
        {
            for (uint8_t k = 0; k < MAX_BT_PAIRED_DEVICE_COUNT - 1; k++)
            {
                memcpy((uint8_t *)&(pBtDevInfo->pairedBtDevInfo[MAX_BT_PAIRED_DEVICE_COUNT - 1 - k]),
                       (uint8_t *)&(pBtDevInfo->pairedBtDevInfo[MAX_BT_PAIRED_DEVICE_COUNT - 2 - k]),
                       sizeof(nvrec_btdevicerecord));
            }
            pBtDevInfo->pairedDevNum--;
        }
        else
        {
            for (uint8_t k = 0; k < pBtDevInfo->pairedDevNum; k++)
            {
                memcpy((uint8_t *)&(pBtDevInfo->pairedBtDevInfo[pBtDevInfo->pairedDevNum - k]),
                       (uint8_t *)&(pBtDevInfo->pairedBtDevInfo[pBtDevInfo->pairedDevNum - 1 - k]),
                       sizeof(nvrec_btdevicerecord));
            }
        }

        // fill the default value
        memset((void *)&(pBtDevInfo->pairedBtDevInfo[0]), 0, sizeof(nvrec_btdevicerecord));
        nvrec_btdevicerecord* nvrec_pool_record = &(pBtDevInfo->pairedBtDevInfo[0]);
        memcpy((uint8_t *)&(nvrec_pool_record->record), (uint8_t *)param_rec,
               sizeof(btif_device_record_t));
        nv_record_pairing_info_reset(nvrec_pool_record);
#ifdef BT_DIP_SUPPORT
        nvrec_pool_record->pnp_info.vend_id = 0;
        nvrec_pool_record->pnp_info.vend_id_source = 0;
#endif
        pBtDevInfo->pairedDevNum++;

        TRACE(0, "%s new added", __FUNCTION__);

        uint8_t zero_key[16] = {0};
        if (memcmp(param_rec->linkKey, zero_key, 16)) {
            nv_record_bt_distribute_new_dev_paired_event(nvrec_pool_record->record.bdAddr.address);
        }

        isUpdateNv = true;
    }
    else
    {
        // exist

        bool isRecordChanged = false;

        // check whether the record is changed, if so, do flush immediately
        if (memcmp((uint8_t *)&(pBtDevInfo->pairedBtDevInfo[indexOfEntry].record),
                       (uint8_t *)param_rec, sizeof(btif_device_record_t)))
        {

            TRACE(0, "%s used to be paired, link changed", __FUNCTION__);

            nv_record_bt_distribute_new_dev_paired_event(pBtDevInfo->pairedBtDevInfo[indexOfEntry].record.bdAddr.address);

            isRecordChanged = true;
        }

        // check whether it's already at the head
        // if not, move it to the head
        if (indexOfEntry > 0)
        {
            nvrec_btdevicerecord record;
            memcpy((uint8_t *)&record, (uint8_t *)&(pBtDevInfo->pairedBtDevInfo[indexOfEntry]),
                   sizeof(record));

            // if not, move it to the head
            for (uint8_t k = 0; k < indexOfEntry; k++)
            {
                memcpy((uint8_t *)&(pBtDevInfo->pairedBtDevInfo[indexOfEntry - k]),
                       (uint8_t *)&(pBtDevInfo->pairedBtDevInfo[indexOfEntry - 1 - k]),
                       sizeof(nvrec_btdevicerecord));
            }

            memcpy((uint8_t *)&(pBtDevInfo->pairedBtDevInfo[0]), (uint8_t *)&record,
                   sizeof(record));

            // update the link info
            memcpy((uint8_t *)&(pBtDevInfo->pairedBtDevInfo[0].record), (uint8_t *)param_rec,
                   sizeof(btif_device_record_t));

            // need to flush the nv record
            isUpdateNv = true;
        }
        // else, check whether the link info needs to be updated
        else
        {
            if (memcmp((uint8_t *)&(pBtDevInfo->pairedBtDevInfo[0].record),
                       (uint8_t *)param_rec, sizeof(btif_device_record_t)))
            {
                // update the link info
                memcpy((uint8_t *)&(pBtDevInfo->pairedBtDevInfo[0].record), (uint8_t *)param_rec,
                       sizeof(btif_device_record_t));

                // need to flush the nv record
                isUpdateNv = true;
            }
        }

        if (isRecordChanged)
        {
            nv_record_pairing_info_reset(&(pBtDevInfo->pairedBtDevInfo[0]));
        }
    }

    if (isUpdateNv)
    {
        nv_record_update_runtime_userdata();
    }

    nv_record_post_write_operation(lock);


    TRACE(1,"paired Bt dev:%d", pBtDevInfo->pairedDevNum);
    TRACE(1,"isUpdateNv: %d", isUpdateNv);
    nv_record_all_ddbrec_print();

    return BT_STS_SUCCESS;
}

/*
this function should be surrounded by OS_LockStack and OS_UnlockStack when call.
*/
bt_status_t nv_record_add(SECTIONS_ADP_ENUM type, void *record)
{
    bt_status_t retstatus = BT_STS_FAILED;

    if ((NULL == record) || (section_none == type))
    {
        return BT_STS_FAILED;
    }

    switch(type)
    {
        case section_usrdata_ddbrecord:
#ifndef RAM_NV_RECORD
            retstatus = nv_record_ddbrec_add(record);
#else
            retstatus = ram_record_ddbrec_add(record);
#endif
            break;
        default:
            break;
    }

    return retstatus;
}

/*
this function should be surrounded by OS_LockStack and OS_UnlockStack when call.
*/
bt_status_t nv_record_change_order(const btif_device_record_t *record, uint8_t target)
{
    bt_status_t status = BT_STS_FAILED;
    if((target >= nvrecord_extension_p->bt_pair_info.pairedDevNum) || (NULL == nvrecord_extension_p))
    {
        return status;
    }

    uint32_t lock = nv_record_pre_write_operation();

    bool isNVupdate = false;
    //find current index of record
    int8_t cur_index = -1;
    NV_RECORD_PAIRED_BT_DEV_INFO_T* pBtDevInfo =
        (NV_RECORD_PAIRED_BT_DEV_INFO_T *)(&(nvrecord_extension_p->bt_pair_info));
    cur_index = nv_record_get_bt_pairing_info_index(record->bdAddr.address);
    TRACE(2,"CUR_DEV:%d,TARGET:%d",cur_index,target);

    if (cur_index == -1)
    {
        TRACE(0,"current record is not in the list");
    }
    else
    {
        if(memcmp((uint8_t *)&(pBtDevInfo->pairedBtDevInfo[target]),
                  (uint8_t *)&(pBtDevInfo->pairedBtDevInfo[cur_index]),
                sizeof(nvrec_btdevicerecord)))
        {
            nvrec_btdevicerecord nvrec_tmp_record = pBtDevInfo->pairedBtDevInfo[cur_index];//caching the current index info

            if (cur_index < target)//when current index is less than the target
            {
                for(uint8_t i = cur_index; i < target; i++)
                {
                    memcpy((uint8_t *)&(pBtDevInfo->pairedBtDevInfo[i]),
                        (uint8_t *)&(pBtDevInfo->pairedBtDevInfo[i+1]),
                        sizeof(nvrec_btdevicerecord));
                }

            }
            else if (cur_index > target)//when current index is greater than the target
            {
                for(uint8_t i = cur_index; i > target; i--)
                {
                    memcpy((uint8_t *)&(pBtDevInfo->pairedBtDevInfo[i]),
                        (uint8_t *)&(pBtDevInfo->pairedBtDevInfo[i-1]),
                        sizeof(nvrec_btdevicerecord));
                }
            }

            memcpy((uint8_t *)&(pBtDevInfo->pairedBtDevInfo[target]),
                   (uint8_t *)&(nvrec_tmp_record),
                   sizeof(nvrec_btdevicerecord));

            isNVupdate = true;
            status = BT_STS_SUCCESS;
        }
        else
        {
            TRACE(0,"current record is already in target index position");
        }
    }

    if(isNVupdate)
    {
        nv_record_update_runtime_userdata();
    }

    nv_record_post_write_operation(lock);

    return status;
}

/*
this function should be surrounded by OS_LockStack and OS_UnlockStack when call.
*/
bt_status_t nv_record_ddbrec_find(const bt_bdaddr_t* bd_ddr, btif_device_record_t *record)
{
#ifndef RAM_NV_RECORD
    if ((NULL == bd_ddr) || (NULL == record) || (NULL == nvrecord_extension_p))
    {
        return BT_STS_FAILED;
    }

    int8_t indexOfEntry = -1;
    NV_RECORD_PAIRED_BT_DEV_INFO_T* pBtDevInfo =
        (NV_RECORD_PAIRED_BT_DEV_INFO_T *)(&(nvrecord_extension_p->bt_pair_info));
    indexOfEntry = nv_record_get_bt_pairing_info_index(bd_ddr->address);

    if (-1 == indexOfEntry)
    {
        return BT_STS_FAILED;
    }
    else
    {
        memcpy((uint8_t *)record, (uint8_t *)&(pBtDevInfo->pairedBtDevInfo[indexOfEntry].record),
               sizeof(btif_device_record_t));
        return BT_STS_SUCCESS;
    }
#else
    if ((NULL == bd_ddr) || (NULL == record) || (NULL == nvrecord_extension_p))
    {
        return BT_STS_FAILED;
    }
    nvrec_btdevicerecord *btrecord = NULL;
    bt_status_t status =  ram_record_ddbrec_find(bd_ddr, &btrecord);
    if(btrecord !=NULL)
    {
       // record = &btrecord->record;
       memcpy((uint8_t *)record, (uint8_t *)&(btrecord->record),
       sizeof(btif_device_record_t));
    }
    return status;
#endif
}

bool nv_record_get_pnp_info(bt_bdaddr_t *bdAddr, bt_dip_pnp_info_t *pPnpInfo)
{
#ifdef BT_DIP_SUPPORT
    nvrec_btdevicerecord *record = NULL;

    TRACE(1,"%s", __func__);
    DUMP8("%x ", bdAddr->address, BTIF_BD_ADDR_SIZE);

    if (!nv_record_btdevicerecord_find(bdAddr, &record))
    {
        if (record->pnp_info.vend_id)
        {
            TRACE(2, "has the vend_id 0x%x vend_id_source 0x%x",
                record->pnp_info.vend_id, record->pnp_info.vend_id_source);
            *pPnpInfo = record->pnp_info;
            return true;
        }
    }
#endif
    pPnpInfo->vend_id = 0;
    pPnpInfo->vend_id_source = 0;
    return false;
}

bt_status_t nv_record_ddbrec_clear(void)
{
#ifndef RAM_NV_RECORD
    if (NULL == nvrecord_extension_p)
    {
        return BT_STS_FAILED;
    }
    NV_RECORD_PAIRED_BT_DEV_INFO_T *pBtDevInfo =
        (NV_RECORD_PAIRED_BT_DEV_INFO_T *)(&(nvrecord_extension_p->bt_pair_info));

    uint32_t lock = nv_record_pre_write_operation();

    pBtDevInfo->pairedDevNum = 0;

    nv_record_update_runtime_userdata();

    nv_record_post_write_operation(lock);
#else
    ram_record_ddbrec_clear();
#endif
    return BT_STS_SUCCESS;
}

/*
this function should be surrounded by OS_LockStack and OS_UnlockStack when call.
*/
bt_status_t nv_record_ddbrec_delete(const bt_bdaddr_t *bdaddr)
{
#ifndef RAM_NV_RECORD
    if (NULL == nvrecord_extension_p)
    {
        return BT_STS_FAILED;
    }

    int8_t indexOfEntry = -1;
    NV_RECORD_PAIRED_BT_DEV_INFO_T* pBtDevInfo =
        (NV_RECORD_PAIRED_BT_DEV_INFO_T *)(&(nvrecord_extension_p->bt_pair_info));
    indexOfEntry = nv_record_get_bt_pairing_info_index(bdaddr->address);
    if (-1 == indexOfEntry)
    {
        return BT_STS_FAILED;
    }

    uint32_t lock = nv_record_pre_write_operation();

    for (uint8_t k = 0; k < pBtDevInfo->pairedDevNum - indexOfEntry - 1; k++)
    {
        memcpy((uint8_t *)&(pBtDevInfo->pairedBtDevInfo[indexOfEntry + k]),
               (uint8_t *)&(pBtDevInfo->pairedBtDevInfo[indexOfEntry + 1 + k]),
               sizeof(nvrec_btdevicerecord));
    }

    memset((uint8_t *)&(pBtDevInfo->pairedBtDevInfo[pBtDevInfo->pairedDevNum-1]), 0,
           sizeof(nvrec_btdevicerecord));
    pBtDevInfo->pairedDevNum--;

    nv_record_update_runtime_userdata();

    nv_record_post_write_operation(lock);
#else
    ram_record_ddbrec_delete(bdaddr);
#endif
    return BT_STS_SUCCESS;
}

int nv_record_btdevicerecord_find(const bt_bdaddr_t *bd_ddr, nvrec_btdevicerecord **record)
{
#ifndef RAM_NV_RECORD
    if ((NULL == bd_ddr) || (NULL == record) || (NULL == nvrecord_extension_p))
    {
        return -1;
    }

    int8_t indexOfEntry = -1;
    NV_RECORD_PAIRED_BT_DEV_INFO_T* pBtDevInfo =
        (NV_RECORD_PAIRED_BT_DEV_INFO_T *)(&(nvrecord_extension_p->bt_pair_info));
    indexOfEntry = nv_record_get_bt_pairing_info_index(bd_ddr->address);

    if (-1 == indexOfEntry)
    {
        return -1;
    }

    *record =
        (nvrec_btdevicerecord *)&(pBtDevInfo->pairedBtDevInfo[indexOfEntry]);
#else
    if ((NULL == bd_ddr) || (NULL == record) || (NULL == nvrecord_extension_p))
    {
        return -1;
    }
    ram_record_ddbrec_find(bd_ddr, record);
#endif
    return 0;
}

void nv_record_btdevicerecord_set_a2dp_vol(nvrec_btdevicerecord* pRecord, int8_t vol)
{
    uint32_t lock = nv_record_pre_write_operation();
    if (vol != pRecord->device_vol.a2dp_vol)
    {
        nv_record_update_runtime_userdata();
        pRecord->device_vol.a2dp_vol = vol;
    }

    nv_record_post_write_operation(lock);
}

void nv_record_btdevicerecord_set_a2dp_abs_vol(nvrec_btdevicerecord* pRecord, int8_t vol)
{
    uint32_t lock = nv_record_pre_write_operation();
    if (vol != pRecord->device_plf.a2dp_abs_vol)
    {
        nv_record_update_runtime_userdata();
        pRecord->device_plf.a2dp_abs_vol = vol;
    }

    nv_record_post_write_operation(lock);
}

void nv_record_btdevicerecord_set_hfp_vol(nvrec_btdevicerecord* pRecord, int8_t vol)
{
    uint32_t lock = nv_record_pre_write_operation();
    if (vol != pRecord->device_vol.hfp_vol)
    {
        nv_record_update_runtime_userdata();
        pRecord->device_vol.hfp_vol = vol;
    }

    nv_record_post_write_operation(lock);
}

void nv_record_btdevicevolume_set_a2dp_vol(btdevice_volume* device_vol, int8_t vol)
{
    uint32_t lock = nv_record_pre_write_operation();
    if (vol != device_vol->a2dp_vol)
    {
        nv_record_update_runtime_userdata();
        device_vol->a2dp_vol = vol;
    }

    nv_record_post_write_operation(lock);

}

void nv_record_btdevicevolume_set_hfp_vol(btdevice_volume* device_vol, int8_t vol)
{
    uint32_t lock = nv_record_pre_write_operation();
    if (vol != device_vol->hfp_vol)
    {
        nv_record_update_runtime_userdata();
        device_vol->hfp_vol = vol;
    }

    nv_record_post_write_operation(lock);

}

void nv_record_btdevicerecord_set_pnp_info(nvrec_btdevicerecord* pRecord, bt_dip_pnp_info_t* pPnpInfo)
{
#ifdef BT_DIP_SUPPORT
    TRACE(2, "%s vend id 0x%x", __func__, pPnpInfo->vend_id);
    uint32_t lock = nv_record_pre_write_operation();
    if (pPnpInfo->vend_id != pRecord->pnp_info.vend_id)
    {
        nv_record_update_runtime_userdata();
        pRecord->pnp_info = *pPnpInfo;
    }

    nv_record_post_write_operation(lock);
#endif
}

#ifdef TOTA_CRASH_DUMP_TOOL_ENABLE
void nv_record_btdevicerecord_crash_dump(void)
{
    if (NULL == nvrecord_extension_p)
    {
        return;
    }

    if (nvrecord_extension_p->bt_pair_info.pairedDevNum > 0)
    {
        for(uint8_t tmp_i=0; tmp_i < nvrecord_extension_p->bt_pair_info.pairedDevNum; tmp_i++)
        {
            btif_device_record_t record;
            bt_status_t ret_status;
            ret_status = nv_record_enum_dev_records(tmp_i, &record);
            if (BT_STS_SUCCESS == ret_status)
            {
                CRASH_DUMP_DEBUG_TRACE(0,"nv record bdAddr = \n\r");
                DUMP8("%02x ",record.bdAddr.address,BT_ADDR_OUTPUT_PRINT_NUM);
                CRASH_DUMP_DEBUG_TRACE(0,"record_trusted = ");
                DUMP8("%d ",&record.trusted,sizeof((uint8_t)record.trusted));
                CRASH_DUMP_DEBUG_TRACE(0,"record_linkKey = ");
                DUMP8("%02x ",record.linkKey,sizeof(record.linkKey));
                CRASH_DUMP_DEBUG_TRACE(0,"record_keyType = ");
                DUMP8("%x ",&record.keyType,sizeof(record.keyType));
                CRASH_DUMP_DEBUG_TRACE(0,"record_pinLen = ");
                DUMP8("%x ",&record.pinLen,sizeof(record.pinLen));
                CRASH_DUMP_DEBUG_TRACE(1,"record_a2dp_vol = %d",nvrecord_extension_p->bt_pair_info.pairedBtDevInfo[tmp_i].device_vol.a2dp_vol);
                CRASH_DUMP_DEBUG_TRACE(1,"record_hfp_vol = %d",nvrecord_extension_p->bt_pair_info.pairedBtDevInfo[tmp_i].device_vol.hfp_vol);
                CRASH_DUMP_DEBUG_TRACE(1,"record_a2dp_codec = %d",nvrecord_extension_p->bt_pair_info.pairedBtDevInfo[tmp_i].device_plf.a2dp_codectype);
                CRASH_DUMP_DEBUG_TRACE(1,"record_a2dp_act = %d",nvrecord_extension_p->bt_pair_info.pairedBtDevInfo[tmp_i].device_plf.a2dp_act);
                CRASH_DUMP_DEBUG_TRACE(1,"record_hfp_act = %d",nvrecord_extension_p->bt_pair_info.pairedBtDevInfo[tmp_i].device_plf.hfp_act);
            }
        }
    }
}
#endif

bt_status_t nv_record_ddbrec_clear_all_bt_paired_list()
{
#ifndef FPGA
    if (NULL == nvrecord_extension_p)
    {
        return BT_STS_FAILED;
    }

    NV_RECORD_PAIRED_BT_DEV_INFO_T* pBtDevInfo = NULL;

    uint32_t lock = nv_record_pre_write_operation();

    //clear real master paired list nvrecord
    pBtDevInfo = (NV_RECORD_PAIRED_BT_DEV_INFO_T *)(&(nvrecord_extension_p->bt_pair_info));
    for (uint8_t k = 0; k < pBtDevInfo->pairedDevNum; k++)
    {
        memset((uint8_t *)&(pBtDevInfo->pairedBtDevInfo[k]), 0,
               sizeof(nvrec_btdevicerecord));
    }
    pBtDevInfo->pairedDevNum = 0;

    nv_record_update_runtime_userdata();

    nv_record_post_write_operation(lock);
#endif
    return BT_STS_SUCCESS;
}

// factory section region
#define NVRECORD_CACHE_2_UNCACHE(addr)  \
    ((unsigned char *)((unsigned int)addr & ~(0x04000000)))

static bool dev_sector_valid = false;
uint8_t nv_record_dev_rev = NVREC_DEV_NEWEST_REV;
#define CLASSIC_BTNAME_LEN (BLE_NAME_LEN_IN_NV-5)
char classics_bt_name[CLASSIC_BTNAME_LEN]= "BES";
#ifdef RAM_NV_RECORD
uint32_t __factory_start[1024*2];
#else
extern uint32_t __factory_start[];
#endif

bool nvrec_dev_data_open(void)
{
    uint32_t dev_zone_crc,dev_zone_flsh_crc;
    uint32_t vermagic;

    vermagic = __factory_start[dev_version_and_magic];
    nvrec_trace(2, "%s,vermagic=0x%x", __func__, vermagic);
    if ((nvrec_dev_magic != (vermagic & 0xFFFF)) ||
        ((vermagic >> 16) > NVREC_DEV_NEWEST_REV))
    {
        dev_sector_valid = false;
        nvrec_trace(1, "%s,dev sector invalid.", __func__);
        return dev_sector_valid;
    }

    // following the nv rec version number programmed by the downloader tool,
    // to be backward compatible
    nv_record_dev_rev = vermagic >> 16;
    nvrec_trace(1, "Nv record dev version %d", nv_record_dev_rev);

    if (NVREC_DEV_VERSION_1 == nv_record_dev_rev)
    {
        dev_zone_flsh_crc = __factory_start[dev_crc];
        dev_zone_crc = crc32_c(0, (uint8_t *)(&__factory_start[dev_reserv1]), (dev_data_len - dev_reserv1) * sizeof(uint32_t));
    }
    else
    {
        // check the data length
        if ((rev2_dev_section_start_reserved * sizeof(uint32_t)) + __factory_start[rev2_dev_data_len] > 4096)
        {
            nvrec_trace(1, "nv rec dev data len %d has exceeds the facory sector size!.",
                        __factory_start[rev2_dev_data_len]);
            dev_sector_valid = false;
            return false;
        }

        // assure that in future, if the nv dev data structure is extended, the former tool
        // and former bin can still be workable
        dev_zone_flsh_crc = __factory_start[rev2_dev_crc];
        dev_zone_crc = crc32_c(0, (uint8_t *)(&__factory_start[rev2_dev_section_start_reserved]),
                               __factory_start[rev2_dev_data_len]);
    }

    nvrec_trace(4, "%s: data len 0x%x,dev_zone_flsh_crc=0x%x,dev_zone_crc=0x%x", __func__,
                __factory_start[rev2_dev_data_len], dev_zone_flsh_crc, dev_zone_crc);
    if (dev_zone_flsh_crc == dev_zone_crc)
    {
        dev_sector_valid = true;
    }

    if (dev_sector_valid)
    {
        nvrec_trace(1, "%s: nv rec dev is valid.", __func__);
    }
    else
    {
        nvrec_trace(1, "%s: nv rec dev is invalid.", __func__);
    }
    return dev_sector_valid;
}

bool nvrec_dev_localname_addr_init(dev_addr_name *dev)
{
    uint32_t *p_devdata_cache = __factory_start;
    size_t name_len = 0;

    if(true == dev_sector_valid)
    {
        nvrec_trace(1,"%s: nv dev data valid", __func__);
        if (NVREC_DEV_VERSION_1 == nv_record_dev_rev)
        {
            memcpy((void *) dev->btd_addr,(void *)&p_devdata_cache[dev_bt_addr],BTIF_BD_ADDR_SIZE);
            memcpy((void *) dev->ble_addr,(void *)&p_devdata_cache[dev_ble_addr],BTIF_BD_ADDR_SIZE);
            dev->localname = (char *)&p_devdata_cache[dev_name];

            // TODO: should init ble_name with default value
            // dev->ble_name = ;
        }
        else
        {
            memcpy((void *) dev->btd_addr,(void *)&p_devdata_cache[rev2_dev_bt_addr],BTIF_BD_ADDR_SIZE);
            memcpy((void *) dev->ble_addr,(void *)&p_devdata_cache[rev2_dev_ble_addr],BTIF_BD_ADDR_SIZE);
            dev->localname = (char *)&p_devdata_cache[rev2_dev_name];
            dev->ble_name = (char *)&p_devdata_cache[rev2_dev_ble_name];
        }

        if (strlen(dev->localname) < CLASSIC_BTNAME_LEN)
        {
            name_len = strlen(dev->localname);
        }
        else
        {
            name_len = CLASSIC_BTNAME_LEN - 1;
        }

        memcpy(classics_bt_name,dev->localname,name_len);
        classics_bt_name[name_len] = '\0';
        bt_set_local_name(classics_bt_name);
        bt_set_local_address(dev->btd_addr);

        bt_set_ble_local_name(dev->ble_name);
        bt_set_ble_local_address(dev->ble_addr);
    }
    else
    {
        nvrec_trace(1,"%s: nv dev data invalid", __func__);
    }

    nvrec_trace(0,"BT addr is:");
    DUMP8("%02x ", dev->btd_addr, BT_ADDR_OUTPUT_PRINT_NUM);
    nvrec_trace(0,"BLE addr is:");
    DUMP8("%02x ", dev->ble_addr, BT_ADDR_OUTPUT_PRINT_NUM);
    nvrec_trace(2,"localname=%s, namelen=%d", dev->localname, strlen(dev->localname));
    if (dev->ble_name)
    {
        nvrec_trace(2,"blename=%s, namelen=%d", dev->ble_name, strlen(dev->ble_name));
    }

    return dev_sector_valid;
}

int nvrec_dev_force_get_btaddress(unsigned char *btd_addr)
{
    uint32_t *p_devdata_cache = __factory_start;
    memcpy((void *) btd_addr,(void *)&p_devdata_cache[dev_bt_addr],BTIF_BD_ADDR_SIZE);
    return 0;
}

static void nvrec_dev_data_fill_xtal_fcap(uint32_t *mem_pool, uint32_t val)
{
    uint8_t *btaddr = NULL;
    uint8_t *bleaddr = NULL;

    assert(0 != mem_pool);
    if (!dev_sector_valid)
    {
        mem_pool[dev_version_and_magic] = ((nv_record_dev_rev << 16) | nvrec_dev_magic);
    }

    if (NVREC_DEV_VERSION_1 == nv_record_dev_rev)
    {
        if (dev_sector_valid)
        {
            memcpy((void *)mem_pool, (void *)__factory_start, 0x1000);
            mem_pool[dev_xtal_fcap] = val;
            mem_pool[dev_crc] = crc32_c(0, (uint8_t *)(&mem_pool[dev_reserv1]), (dev_data_len - dev_reserv1) * sizeof(uint32_t));
        }
        else
        {
            const char *localname = bt_get_local_name();
            unsigned int namelen = strlen(localname);

            btaddr = bt_get_local_address();
            bleaddr = bt_get_ble_local_address();

            mem_pool[dev_reserv1] = 0;
            mem_pool[dev_reserv2] = 0;
            memcpy((void *)&mem_pool[dev_name], (void *)localname, (size_t)namelen);
            nvrec_dev_rand_btaddr_gen(btaddr);
            nvrec_dev_rand_btaddr_gen(bleaddr);
            memcpy((void *)&mem_pool[dev_bt_addr], (void *)btaddr, BTIF_BD_ADDR_SIZE);
            memcpy((void *)&mem_pool[dev_ble_addr], (void *)bleaddr, BTIF_BD_ADDR_SIZE);
            memset((void *)&mem_pool[dev_dongle_addr], 0x0, BTIF_BD_ADDR_SIZE);
            mem_pool[dev_xtal_fcap] = val;
            mem_pool[dev_crc] = crc32_c(0, (uint8_t *)(&mem_pool[dev_reserv1]), (dev_data_len - dev_reserv1) * sizeof(uint32_t));
            nvrec_trace(2, "%s: mem_pool[dev_crc]=%x.\n", __func__, mem_pool[dev_crc]);
        }
    }
    else
    {
        if (dev_sector_valid)
        {
            memcpy((void *)mem_pool, (void *)__factory_start, 0x1000);
            mem_pool[rev2_dev_xtal_fcap] = val;
            mem_pool[rev2_dev_crc] = crc32_c(0,
                                             (uint8_t *)(&mem_pool[rev2_dev_section_start_reserved]), mem_pool[rev2_dev_data_len]);
        }
        else
        {
            const char *localname = bt_get_local_name();
            unsigned int namelen = strlen(localname);

            btaddr = bt_get_local_address();
            bleaddr = bt_get_ble_local_address();
            mem_pool[rev2_dev_section_start_reserved] = 0;
            mem_pool[rev2_dev_reserv2] = 0;
            memcpy((void *)&mem_pool[rev2_dev_name], (void *)localname, (size_t)namelen);
            memcpy((void *)&mem_pool[rev2_dev_ble_name], (void *)bt_get_ble_local_name(), BLE_NAME_LEN_IN_NV);
            nvrec_dev_rand_btaddr_gen(btaddr);
            nvrec_dev_rand_btaddr_gen(bleaddr);
            memcpy((void *)&mem_pool[rev2_dev_bt_addr], (void *)btaddr, BTIF_BD_ADDR_SIZE);
            memcpy((void *)&mem_pool[rev2_dev_ble_addr], (void *)bleaddr, BTIF_BD_ADDR_SIZE);
            memset((void *)&mem_pool[rev2_dev_dongle_addr], 0x0, BTIF_BD_ADDR_SIZE);
            mem_pool[rev2_dev_xtal_fcap] = val;
            mem_pool[rev2_dev_data_len] = (rev2_dev_section_end - rev2_dev_section_start_reserved) * sizeof(uint32_t);
            mem_pool[rev2_dev_crc] = crc32_c(0,
                                             (uint8_t *)(&mem_pool[rev2_dev_section_start_reserved]),
                                             mem_pool[rev2_dev_data_len]);
            nvrec_trace(2, "%s: mem_pool[rev2_dev_crc] = 0x%x.\n",
                        __func__, mem_pool[rev2_dev_crc]);
        }
    }
}

void nvrec_dev_flash_flush(unsigned char *mempool)
{
    uint32_t lock;
#ifdef nv_record_debug
    uint32_t devdata[dev_data_len] = {0,};
    uint32_t recrc;
#endif

    if(NULL == mempool)
        return;
    lock = int_lock_global();
    pmu_flash_write_config();

    enum NORFLASH_API_RET_T ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY,(uint32_t)__factory_start,FACTORY_SECTOR_SIZE,false);
    ASSERT(ret == NORFLASH_API_OK,"%s: erase failed! ret = %d.",__FUNCTION__,ret);
    ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY,(uint32_t)__factory_start,(uint8_t *)mempool,FACTORY_SECTOR_SIZE,false);
    ASSERT(ret == NORFLASH_API_OK,"%s: write failed! ret = %d.",__FUNCTION__,ret);

    pmu_flash_read_config();
    int_unlock_global(lock);

#ifdef nv_record_debug
    if (NVREC_DEV_VERSION_1 == nv_record_dev_rev)
    {
        memset(devdata, 0x0, dev_data_len * sizeof(uint32_t));
        memcpy(devdata, __factory_start, dev_data_len * sizeof(uint32_t));
        recrc = crc32_c(0, (uint8_t *)(&devdata[dev_reserv1]), (dev_data_len - dev_reserv1) * sizeof(uint32_t));
        nvrec_trace(3, "%s: devdata[dev_crc]=%x.recrc=%x\n", __func__, devdata[dev_crc], recrc);

        if (devdata[dev_crc] != recrc)
        {
            assert(0);
        }
    }
#endif
}

void nvrec_dev_rand_btaddr_gen(uint8_t* bdaddr)
{
    unsigned int seed;
    int i;

    OS_DUMP8("%x ",bdaddr,6);
    seed = hal_sys_timer_get();
    for(i=0;i<BTIF_BD_ADDR_SIZE;i++)
    {
        unsigned int randval;
        srand(seed);
        randval = rand();
        bdaddr[i] = (randval&0xff);
        seed += rand();
    }
    OS_DUMP8("%x ",bdaddr,6);
}

void nvrec_dev_set_xtal_fcap(unsigned int xtal_fcap)
{
    uint8_t *mempool = NULL;
    uint32_t lock;
#ifdef nv_record_debug
    uint32_t devdata[dev_data_len] = {
        0,
    };
    uint32_t recrc;
#endif

    syspool_init();
    syspool_get_buff(&mempool, 0x1000);
    nvrec_dev_data_fill_xtal_fcap((uint32_t *)mempool, (uint32_t)xtal_fcap);
    lock = int_lock_global();
    norflash_api_flush_all(true); //Ensure that the flash is in an operational state
    pmu_flash_write_config();

    enum NORFLASH_API_RET_T ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY,(uint32_t)__factory_start,FACTORY_SECTOR_SIZE,false);
    ASSERT(ret == NORFLASH_API_OK,"%s: erase failed! ret = %d.",__FUNCTION__,ret);
    ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY,(uint32_t)__factory_start,(uint8_t *)mempool,FACTORY_SECTOR_SIZE,false);
    ASSERT(ret == NORFLASH_API_OK,"%s: write failed! ret = %d.",__FUNCTION__,ret);

    pmu_flash_read_config();
    int_unlock_global(lock);
#ifdef nv_record_debug
    if (NVREC_DEV_VERSION_1 == nv_record_dev_rev)
    {
        memset(devdata, 0x0, dev_data_len * sizeof(uint32_t));
        memcpy(devdata, __factory_start, dev_data_len * sizeof(uint32_t));
        recrc = crc32_c(0, (uint8_t *)(&devdata[dev_reserv1]), (dev_data_len - dev_reserv1) * sizeof(uint32_t));
        nvrec_trace(4, "%s xtal_fcap=%d, devdata[dev_crc]=%x, recrc=%x\n",
                    __func__, devdata[dev_xtal_fcap], devdata[dev_crc], recrc);

        if (devdata[dev_crc] != recrc)
        {
            assert(0);
        }
    }
#endif
}

int nvrec_dev_get_xtal_fcap(unsigned int *xtal_fcap)
{
    unsigned int xtal_fcap_addr;
    int ret = 0;

    if (NVREC_DEV_VERSION_1 == nv_record_dev_rev)
    {
        xtal_fcap_addr = (unsigned int)(__factory_start + dev_xtal_fcap);
    }
    else
    {
        xtal_fcap_addr = (unsigned int)(__factory_start + rev2_dev_xtal_fcap);
    }

    unsigned int tmpval[1] = {
        0,
    };

    if ((false == dev_sector_valid) || (NULL == xtal_fcap))
    {
        ret = -1;
    }
    else
    {
        memcpy((void *)tmpval, (void *)xtal_fcap_addr, sizeof(unsigned int));
        *xtal_fcap = tmpval[0];
    }

    return ret;
}

int nvrec_dev_get_dongleaddr(bt_bdaddr_t *dongleaddr)
{
    unsigned int dongle_addr_pos;
    int ret = 0;

    if (NVREC_DEV_VERSION_1 == nv_record_dev_rev)
    {
        dongle_addr_pos = (unsigned int)(__factory_start + dev_dongle_addr);
    }
    else
    {
        dongle_addr_pos = (unsigned int)(__factory_start + rev2_dev_dongle_addr);
    }

    if ((false == dev_sector_valid) || (NULL == dongleaddr))
    {
        ret = -1;
    }
    else
    {
        memcpy((void *)dongleaddr, (void *)dongle_addr_pos, BTIF_BD_ADDR_SIZE);
    }

    return ret;
}

int nvrec_dev_get_btaddr(char *btaddr)
{
    unsigned int bt_addr_pos;
    int ret = 1;

    if (NVREC_DEV_VERSION_1 == nv_record_dev_rev)
    {
        bt_addr_pos = (unsigned int)(__factory_start + dev_bt_addr);
    }
    else
    {
        bt_addr_pos = (unsigned int)(__factory_start + rev2_dev_bt_addr);
    }

    if ((false == dev_sector_valid) || (NULL == btaddr))
    {
        ret = 0;
    }
    else
    {
        memcpy((void *)btaddr, (void *)bt_addr_pos, BTIF_BD_ADDR_SIZE);
    }

    return ret;
}

char *nvrec_dev_get_bt_name(void)
{
    return classics_bt_name;
}

const char *nvrec_dev_get_ble_name(void)
{
    if ((NVREC_DEV_VERSION_1 == nv_record_dev_rev) || (!dev_sector_valid))
    {
        return BLE_DEFAULT_NAME;
    }
    else
    {
        return (const char *)(&__factory_start[rev2_dev_ble_name]);
    }
}

#ifdef APP_SOUND_ENABLE
int nv_record_stash_original_bt_record(void)
{
    if (NULL == nvrecord_extension_p) {
        return BT_STS_FAILED;
    }
#ifndef RAM_NV_RECORD
    uint32_t lock = nv_record_pre_write_operation();
    memcpy(&(nvrecord_extension_p->bt_pair_info_cache), &(nvrecord_extension_p->bt_pair_info), sizeof(NV_RECORD_PAIRED_BT_DEV_INFO_T));
    memset((NV_RECORD_PAIRED_BT_DEV_INFO_T *)&(nvrecord_extension_p->bt_pair_info), 0, sizeof(NV_RECORD_PAIRED_BT_DEV_INFO_T));
    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
#else
    memcpy(&(nvrecord_extension_p->bt_pair_info_cache), &(nvrecord_extension_p->bt_pair_info), sizeof(NV_RECORD_PAIRED_BT_DEV_INFO_T));
    memset((NV_RECORD_PAIRED_BT_DEV_INFO_T *)&(nvrecord_extension_p->bt_pair_info), 0, sizeof(NV_RECORD_PAIRED_BT_DEV_INFO_T));
#endif
    TRACE(1, "%s", __func__);
    return BT_STS_SUCCESS;
}

int nv_record_pop_original_bt_record(void)
{
    if (NULL == nvrecord_extension_p) {
        return BT_STS_FAILED;
    }
#ifndef RAM_NV_RECORD
    uint32_t lock = nv_record_pre_write_operation();
    memcpy(&(nvrecord_extension_p->bt_pair_info), &(nvrecord_extension_p->bt_pair_info_cache), sizeof(NV_RECORD_PAIRED_BT_DEV_INFO_T));
    nv_record_update_runtime_userdata();
    nv_record_post_write_operation(lock);
#else
    memcpy(&(nvrecord_extension_p->bt_pair_info), &(nvrecord_extension_p->bt_pair_info_cache), sizeof(NV_RECORD_PAIRED_BT_DEV_INFO_T));
#endif
    TRACE(1, "%s", __func__);
    return BT_STS_SUCCESS;
}
#endif

#endif
