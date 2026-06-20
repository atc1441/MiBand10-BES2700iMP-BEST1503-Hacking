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
#include "cmsis.h"
#include "plat_types.h"
#include "tgt_hardware.h"
#include "string.h"
#include "crc32_c.h"
#include "factory_section.h"
#include "pmu.h"
#include "hal_trace.h"
#include "hal_norflash.h"
#include "norflash_api.h"
#include "heap_api.h"

extern uint32_t __factory_start[];
extern uint32_t __factory_end[];

static factory_section_t *factory_section_p = NULL;
static uint8_t nv_record_dev_rev = nvrec_current_version;

#ifndef RAM_NV_RECORD
static void factory_callback(void* param)
{
    NORFLASH_API_OPERA_RESULT *opera_result;

    opera_result = (NORFLASH_API_OPERA_RESULT*)param;

    TRACE(5,"%s:type = %d, addr = 0x%x,len = 0x%x,result = %d,suspend_num=%d.",
                __func__,
                opera_result->type,
                opera_result->addr,
                opera_result->len,
                opera_result->result,
                opera_result->suspend_num);
}
#endif

void factory_section_init(void)
{
#ifndef RAM_NV_RECORD
    enum NORFLASH_API_RET_T result;
    enum HAL_FLASH_ID_T flash_id;
    uint32_t sector_size = 0;
    uint32_t block_size = 0;
    uint32_t page_size = 0;

    flash_id = norflash_api_get_dev_id_by_addr((uint32_t)__factory_start);
    hal_norflash_get_size(flash_id,
               NULL,
               &block_size,
               &sector_size,
               &page_size);
    result = norflash_api_register(NORFLASH_API_MODULE_ID_FACTORY,
                    flash_id,
                    (uint32_t)__factory_start,
                    (uint32_t)__factory_end - (uint32_t)__factory_start,
                    block_size,
                    sector_size,
                    page_size,
                    FACTORY_SECTOR_SIZE,
                    factory_callback
                    );
    ASSERT(result == NORFLASH_API_OK,"nv_record_init: module register failed! result = %d.",result);
#endif
}

int factory_section_open(void)
{
    factory_section_p = (factory_section_t *)__factory_start;

#ifdef RAM_NV_RECORD
    factory_section_set_bt_address(bt_global_addr);
    factory_section_set_ble_address(ble_global_addr);
#else
    if (factory_section_p->head.magic != nvrec_dev_magic)
    {
        factory_section_p = NULL;
        return -1;
    }
    if ((factory_section_p->head.version < nvrec_mini_version) ||
        (factory_section_p->head.version > nvrec_current_version))
    {
        factory_section_p = NULL;
        return -1;
    }

    nv_record_dev_rev = factory_section_p->head.version;

    if (1 == nv_record_dev_rev)
    {
        if (factory_section_p->head.crc !=
            crc32_c(0,(unsigned char *)(&(factory_section_p->head.reserved0)),
            sizeof(factory_section_t)-2-2-4-(5+63+2+2+2+1+8)*sizeof(int))){
            factory_section_p = NULL;
            return -1;
        }

        memcpy(bt_global_addr, factory_section_p->data.bt_address, 6);
        memcpy(ble_global_addr, factory_section_p->data.ble_address, 6);
        TRACE(2,"%s sucess btname:%s", __func__, factory_section_p->data.device_name);
    }
    else
    {
        // check the data length
        if (((uint32_t)(&((factory_section_t *)0)->data.rev2_reserved0)+
            factory_section_p->data.rev2_data_len) > 4096)
        {
            TRACE(1,"nv rec dev data len %d has exceeds the facory sector size!.",
                factory_section_p->data.rev2_data_len);
            return -1;
        }

        if (factory_section_p->data.rev2_crc !=
            crc32_c(0,(unsigned char *)(&(factory_section_p->data.rev2_reserved0)),
            factory_section_p->data.rev2_data_len)){
            factory_section_p = NULL;
            return -1;
        }


        memcpy(bt_global_addr, factory_section_p->data.rev2_bt_addr, 6);
        memcpy(ble_global_addr, factory_section_p->data.rev2_ble_addr, 6);
        TRACE(2,"%s sucess btname:%s", __func__, (char *)factory_section_p->data.rev2_bt_name);
    }
#endif
    DUMP8("%02x ", bt_global_addr, BT_ADDR_OUTPUT_PRINT_NUM);
    DUMP8("%02x ", ble_global_addr, BT_ADDR_OUTPUT_PRINT_NUM);
    return 0;
}

uint8_t* factory_section_get_bt_address(void)
{
    if (factory_section_p)
    {
        if (1 == nv_record_dev_rev)
        {
            return (uint8_t *)&(factory_section_p->data.bt_address);
        }
        else
        {
            return (uint8_t *)&(factory_section_p->data.rev2_bt_addr);
        }
    }
    else
    {
        return NULL;
    }
}

uint8_t* factory_section_get_ble_address(void)
{
    if (factory_section_p)
    {
        if (1 == nv_record_dev_rev)
        {
            return (uint8_t *)&(factory_section_p->data.ble_address);
        }
        else
        {
            return (uint8_t *)&(factory_section_p->data.rev2_ble_addr);
        }
    }
    else
    {
        return NULL;
    }
}

uint8_t* factory_section_get_default_peer_bt_address(void)
{
    if (factory_section_p)
    {
        if (1 == nv_record_dev_rev)
        {
            return (uint8_t *)&(factory_section_p->data.default_peer_bt_address);
        }
        else
        {
            return (uint8_t *)&(factory_section_p->data.rev2_default_peer_bt_address);
        }
    }
    else
    {
        return NULL;
    }
}

uint8_t factory_section_get_default_bt_nv_role(void)
{
    uint8_t bt_nv_role = NV_ROLE_IBRT_UNKNOWN;

    if (factory_section_p) {
        if (1 == nv_record_dev_rev) {
            TRACE(0, "WARNING! bt nv role only used in rev 2");
        } else {
            bt_nv_role = factory_section_p->data.rev2_default_bt_nv_role;

            if ((bt_nv_role != NV_ROLE_IBRT_MASTER) && (bt_nv_role != NV_ROLE_IBRT_SLAVE)) {
                TRACE(0, "WARNING! nvrecord bt role invalid: %d!", bt_nv_role);
                bt_nv_role = NV_ROLE_IBRT_UNKNOWN;
            }
        }
    }

    return bt_nv_role;
}

int factory_section_set_bt_address(uint8_t* btAddr)
{
    uint32_t lock;

    if (factory_section_p){
        TRACE(0, "Update bt addr as:");
        DUMP8("%02x ", btAddr, BT_ADDR_OUTPUT_PRINT_NUM);

        uint32_t heap_size = syspool_original_size();
        uint8_t* tmpBuf =
            (uint8_t *)(((uint32_t)syspool_start_addr()+heap_size-0x3000+FACTORY_SECTOR_SIZE-1)/FACTORY_SECTOR_SIZE*FACTORY_SECTOR_SIZE);

        lock = int_lock_global();
        memcpy(tmpBuf, factory_section_p, FACTORY_SECTOR_SIZE);
        if (1 == nv_record_dev_rev)
        {
            memcpy(((factory_section_t *)tmpBuf)->data.bt_address,
                btAddr, 6);
            ((factory_section_t *)tmpBuf)->head.crc =
                crc32_c(0,(unsigned char *)(&(((factory_section_t *)tmpBuf)->head.reserved0)),sizeof(factory_section_t)-2-2-4);
        }
        else
        {
            memcpy((uint8_t *)&(((factory_section_t *)tmpBuf)->data.rev2_bt_addr),
                btAddr, 6);
            ((factory_section_t *)tmpBuf)->data.rev2_crc =
                crc32_c(0,(unsigned char *)(&(((factory_section_t *)tmpBuf)->data.rev2_reserved0)),
                factory_section_p->data.rev2_data_len);
        }
#ifdef RAM_NV_RECORD
        memcpy(factory_section_p, tmpBuf, FACTORY_SECTOR_SIZE);
#else
        enum NORFLASH_API_RET_T ret;
        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY,(uint32_t)(__factory_start)&0x00FFFFFF,FACTORY_SECTOR_SIZE,false);
        ASSERT(ret == NORFLASH_API_OK,"%s: erase failed! ret = %d.",__FUNCTION__,ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY,(uint32_t)(__factory_start)&0x00FFFFFF,(uint8_t *)tmpBuf,FACTORY_SECTOR_SIZE,false);
        ASSERT(ret == NORFLASH_API_OK,"%s: write failed! ret = %d.",__FUNCTION__,ret);
#endif
        int_unlock_global(lock);

        return 0;
    }else{
        return -1;
    }
}

int factory_section_set_ble_address(uint8_t* bleAddr)
{
    uint32_t lock;

    if (factory_section_p){
        TRACE(0, "Update ble addr as:");
        DUMP8("%02x ", bleAddr, BT_ADDR_OUTPUT_PRINT_NUM);

        uint32_t heap_size = syspool_original_size();
        uint8_t* tmpBuf =
            (uint8_t *)(((uint32_t)syspool_start_addr()+heap_size-0x3000+FACTORY_SECTOR_SIZE-1)/FACTORY_SECTOR_SIZE*FACTORY_SECTOR_SIZE);

        lock = int_lock_global();
        memcpy(tmpBuf, factory_section_p, FACTORY_SECTOR_SIZE);
        if (1 == nv_record_dev_rev)
        {
            memcpy(((factory_section_t *)tmpBuf)->data.ble_address,
                bleAddr, 6);
            ((factory_section_t *)tmpBuf)->head.crc =
                crc32_c(0,(unsigned char *)(&(((factory_section_t *)tmpBuf)->head.reserved0)),sizeof(factory_section_t)-2-2-4);
        }
        else
        {
            memcpy((uint8_t *)&(((factory_section_t *)tmpBuf)->data.rev2_ble_addr),
                bleAddr, 6);
            ((factory_section_t *)tmpBuf)->data.rev2_crc =
                crc32_c(0,(unsigned char *)(&(((factory_section_t *)tmpBuf)->data.rev2_reserved0)),
                factory_section_p->data.rev2_data_len);
        }
#ifdef RAM_NV_RECORD
        memcpy(factory_section_p, tmpBuf, FACTORY_SECTOR_SIZE);
#else
        enum NORFLASH_API_RET_T ret;
        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY,(uint32_t)(__factory_start)&0x00FFFFFF,FACTORY_SECTOR_SIZE,false);
        ASSERT(ret == NORFLASH_API_OK,"%s: erase failed! ret = %d.",__FUNCTION__,ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY,(uint32_t)(__factory_start)&0x00FFFFFF,(uint8_t *)tmpBuf,FACTORY_SECTOR_SIZE,false);
        ASSERT(ret == NORFLASH_API_OK,"%s: write failed! ret = %d.",__FUNCTION__,ret);
#endif
        int_unlock_global(lock);

        return 0;
    }else{
        return -1;
    }
}

uint8_t* factory_section_get_bt_name(void)
{
    if (factory_section_p)
    {
        if (1 == nv_record_dev_rev)
        {
            return (uint8_t *)&(factory_section_p->data.device_name);
        }
        else
        {
            return (uint8_t *)&(factory_section_p->data.rev2_bt_name);
        }
    }
    else
    {
        return (uint8_t *)BT_LOCAL_NAME;
    }
}

int factory_section_set_bt_name(const char *name,int len)
{
    uint8_t *mempool = NULL;
    if (factory_section_p)
    {
        syspool_init();
        syspool_get_buff((uint8_t **)&mempool, 0x1000);
        memcpy(mempool, factory_section_p, 0x1000);

        if (1 == nv_record_dev_rev)
        {
            TRACE(2,"%s [OLD] %s -> [NEW1] %s", __func__, factory_section_p->data.device_name,name);
            memcpy(((factory_section_t *)mempool)->data.device_name, name, len);
            ((factory_section_t *)mempool)->head.crc = crc32_c(0,(unsigned char *)(&(((factory_section_t *)mempool)->head.reserved0)),sizeof(factory_section_t)-2-2-4);
        }
        else
        {
            TRACE(2,"%s [OLD] %s -> [NEW2] %s", __func__, (char *)factory_section_p->data.rev2_bt_name,name);
            memcpy(((factory_section_t *)mempool)->data.rev2_bt_name, name, len);
            ((factory_section_t *)mempool)->data.rev2_crc =
                crc32_c(0,(unsigned char *)(&(((factory_section_t *)mempool)->data.rev2_reserved0)),
                ((factory_section_t *)mempool)->data.rev2_data_len);
        }
#ifdef RAM_NV_RECORD
        memcpy(factory_section_p, mempool, FACTORY_SECTOR_SIZE);
#else
        uint32_t lock;
        enum NORFLASH_API_RET_T ret;
        lock = int_lock_global();
        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY,(uint32_t)(__factory_start)&0x00FFFFFF ,FACTORY_SECTOR_SIZE,false);
        ASSERT(ret == NORFLASH_API_OK,"factory_section_xtal_fcap_set: erase failed! ret = %d.",ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY,(uint32_t)(__factory_start)&0x00FFFFFF ,(uint8_t *)mempool,FACTORY_SECTOR_SIZE,false);
        ASSERT(ret == NORFLASH_API_OK,"factory_section_xtal_fcap_set: write failed! ret = %d.",ret);
        int_unlock_global(lock);
#endif
        return 0;
    }
    else
    {
        return -1;
    }
}

uint8_t* factory_section_get_ble_name(void)
{
    if (factory_section_p)
    {
        if (1 == nv_record_dev_rev)
        {
            return (uint8_t *)BLE_DEFAULT_NAME;
        }
        else
        {
            return (uint8_t *)&(factory_section_p->data.rev2_ble_name);
        }
    }
    else
    {
        return (uint8_t *)BLE_DEFAULT_NAME;
    }
}

uint32_t factory_section_get_version(void)
{
    if (factory_section_p)
    {
        return nv_record_dev_rev;
    }

    return 0;
}

int factory_section_xtal_fcap_get(unsigned int *xtal_fcap)
{
#ifdef IS_USE_DEFAULT_XTAL_FCAP
    return DEFAULT_XTAL_FCAP;
#endif
    if (factory_section_p){
        if (1 == nv_record_dev_rev)
        {
            *xtal_fcap = factory_section_p->data.xtal_fcap;
        }
        else
        {
            *xtal_fcap = factory_section_p->data.rev2_xtal_fcap;
        }
        return 0;
    }else{
        return -1;
    }
}

int factory_section_xtal_fcap_set(unsigned int xtal_fcap)
{
    uint8_t *mempool = NULL;

    if (factory_section_p){
        TRACE(1,"factory_section_xtal_fcap_set:%d", xtal_fcap);
        syspool_init();
        syspool_get_buff((uint8_t **)&mempool, 0x1000);
        memcpy(mempool, factory_section_p, 0x1000);
        if (1 == nv_record_dev_rev)
        {
            ((factory_section_t *)mempool)->data.xtal_fcap = xtal_fcap;
            ((factory_section_t *)mempool)->head.crc = crc32_c(0,(unsigned char *)(&(((factory_section_t *)mempool)->head.reserved0)),sizeof(factory_section_t)-2-2-4);
        }
        else
        {
            ((factory_section_t *)mempool)->data.rev2_xtal_fcap = xtal_fcap;
            ((factory_section_t *)mempool)->data.rev2_crc =
                crc32_c(0,(unsigned char *)(&(((factory_section_t *)mempool)->data.rev2_reserved0)),
                factory_section_p->data.rev2_data_len);
        }
#ifdef RAM_NV_RECORD
        memcpy(factory_section_p, mempool, FACTORY_SECTOR_SIZE);
#else
        uint32_t lock;
        enum NORFLASH_API_RET_T ret;
        lock = int_lock_global();

        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY,(uint32_t)(__factory_start)&0x00FFFFFF,0x1000,false);
        ASSERT(ret == NORFLASH_API_OK,"factory_section_xtal_fcap_set: erase failed! ret = %d.",ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY,(uint32_t)(__factory_start)&0x00FFFFFF,(uint8_t *)mempool,0x1000,false);
        ASSERT(ret == NORFLASH_API_OK,"factory_section_xtal_fcap_set: write failed! ret = %d.",ret);

        int_unlock_global(lock);
#endif
        return 0;
    }else{
        return -1;
    }
}

void factory_section_original_btaddr_get(uint8_t *btAddr)
{
    if(factory_section_p){
        TRACE(0,"get factory_section_p");
        if (1 == nv_record_dev_rev)
        {
            memcpy(btAddr, factory_section_p->data.bt_address, 6);
        }
        else
        {
            memcpy(btAddr, factory_section_p->data.rev2_bt_addr, 6);
        }
    }else{
        TRACE(0,"get bt_addr");
        memcpy(btAddr, bt_global_addr, 6);
    }
}

