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

#include "app_tota_cmd_code.h"
#include "app_tota.h"
#include "app_tota_cmd_handler.h"
#include "hal_norflash.h"
#include "pmu.h"
#include "string.h"
#include "cmsis.h"
#include "crc32_c.h"
#include "cmsis_os.h"
#include "app_tota_flash_program.h"

#define TOTA_CACHE_2_UNCACHE(addr)      ((unsigned int *)((unsigned int)(addr) & ~(0x04000000)))

#if (TOTA_FLASH_SECTOR_BUF_USED)
static uint8_t sector_buffer[FLASH_SECTOR_SIZE_IN_BYTES];
#endif
static uint32_t sector_size = FLASH_SECTOR_SIZE_IN_BYTES;

/*
**  handle flash cmd
**  -> OP_TOTA_WRITE_FLASH_CMD
**  -> OP_TOTA_ERASE_FLASH_CMD
*/
static void _tota_flash_cmd_handle(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen);

/*
**  just use this function to write data to flash with proper address
**  already handle sector alignment
**  write flash success -> return true
**  write flash failed  -> return false
*/
static void _write_flash(uint32_t startAddr, uint8_t * dataBuf, uint32_t dataLen, uint32_t crc);

#if (TOTA_FLASH_SECTOR_BUF_USED)
static TOTA_FLASH_DATA_T * _read_flash_by_address(uint32_t flashAddr, uint16_t length);
#endif

/*
**  just use this function to erase flash with proper address and length
**  already handle sector alignment
*/
static void _erase_flash(uint32_t startAddr, uint32_t dataLen);


/* for debug and test */
void tota_show_flash_test(uint32_t startAddr, uint32_t len)
{
    uint8_t * pbuf = (uint8_t*)startAddr;
    for ( uint32_t i = 0; i < len; i ++ )
    {
       TOTA_LOG_DBG(3, "%d: [0x%2x] || [%c]", i, pbuf[i], (char)pbuf[i]);
    }
}

/* for debug and test */
void tota_write_flash_test(uint32_t startAddr, uint8_t * dataBuf, uint32_t dataLen)
{
    uint32_t lock;
    uint32_t s1, s2, s3, s4;
    hal_norflash_get_size(HAL_FLASH_ID_0, &s1, &s2, &s3, &s4);
    TOTA_LOG_DBG(0, "get norflash size:");
    TOTA_LOG_DBG(4,"-- %x %x %x %x --", s1, s2, s3, s4);
    TOTA_LOG_DBG(2,"flush %d bytes to flash addr 0x%x", dataLen, startAddr);
    TOTA_LOG_DBG(1,"data: %s", (char *)dataBuf);

    lock=int_lock_global();
    pmu_flash_write_config();
    hal_norflash_erase(HAL_FLASH_ID_0, (uint32_t)(startAddr), FLASH_SECTOR_SIZE_IN_BYTES);
    hal_norflash_write(HAL_FLASH_ID_0, (uint32_t)(startAddr), dataBuf, dataLen);
    pmu_flash_read_config();
    int_unlock_global(lock);

    uint8_t * pbuf = (uint8_t*)startAddr;
    for ( uint32_t i = 0; i < dataLen; i ++ )
    {
       TOTA_LOG_DBG(3, "%d: [0x%2x] || [0x%2x]", i, pbuf[i], dataBuf[i]);
    }
}

static void _tota_flash_cmd_handle(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen)
{
    TOTA_LOG_DBG(3,"[%s]: opCode:0x%x, paramLen:%d", __func__, funcCode, paramLen);

    APP_TOTA_CMD_RET_STATUS_E rsp_status = TOTA_NO_ERROR;

    switch (funcCode)
    {
        case OP_TOTA_WRITE_FLASH_CMD:
        {
            TOTA_WRITE_FLASH_STRUCT_T * pflash_write_struct = (TOTA_WRITE_FLASH_STRUCT_T *)ptrParam;
            rsp_status = TOTA_WRITE_FLASH_CRC_CHECK_FAILED;
            uint32_t crc = crc32_c(0, pflash_write_struct->dataBuf, pflash_write_struct->dataLen);
            TOTA_LOG_DBG(2, "write flash: %d bytes, crc: 0x%x", pflash_write_struct->dataLen, pflash_write_struct->dataCrc);
            TOTA_LOG_DBG(1, "crc 0x%x || 0x%x", crc, pflash_write_struct->dataCrc);
            if ( crc == pflash_write_struct->dataCrc )
            {
                _write_flash(pflash_write_struct->address, pflash_write_struct->dataBuf, pflash_write_struct->dataLen, pflash_write_struct->dataCrc);
                TOTA_LOG_DBG(0, "write flash success");
                rsp_status = TOTA_NO_ERROR;
            }
            else
            {
                TOTA_LOG_DBG(0, "crc error...");
            }
            break;
        }
        case OP_TOTA_ERASE_FLASH_CMD:
        {
            TOTA_ERASE_FLASH_STRUCT_T * pflash_erase_struct = (TOTA_ERASE_FLASH_STRUCT_T *)ptrParam;
            TOTA_LOG_DBG(2, "erase flash: 0x%x bytes, length: %u", pflash_erase_struct->address, pflash_erase_struct->length);
            _erase_flash(pflash_erase_struct->address, pflash_erase_struct->length);
            break;
        }
#if (TOTA_FLASH_SECTOR_BUF_USED)
        case OP_TOTA_READ_FLASH_CMD:
        {
            TOTA_READ_FLASH_STRUCT_T * pflash_read_struct = (TOTA_READ_FLASH_STRUCT_T *)ptrParam;
            TOTA_LOG_DBG(0,"read flash: addr: %x, len: %d", pflash_read_struct->address, pflash_read_struct->length);
            TOTA_FLASH_DATA_T * pflashData = _read_flash_by_address(pflash_read_struct->address, pflash_read_struct->length);
            app_tota_send_rsp(funcCode, rsp_status, (uint8_t*)pflashData, pflashData->length+6);
            return;
        }
#endif
        default:
            TOTA_LOG_DBG(0,"error function code");
    }
    app_tota_send_rsp(funcCode, rsp_status,NULL,0);
}

static void _write_flash(uint32_t startAddr, uint8_t * dataBuf, uint32_t dataLen, uint32_t crc)
{
    hal_norflash_disable_protection(HAL_FLASH_ID_0);

    uint32_t lock;
    TOTA_LOG_DBG(2,"flush %d bytes to flash addr 0x%x", dataLen, startAddr);
    //TOTA_LOG_DBG(1,"data: %s", (char *)dataBuf);

#if (TOTA_FLASH_SECTOR_BUF_USED)
    uint32_t pre_bytes = 0, middle_bytes = 0, post_bytes = 0;
    uint32_t base1 = startAddr - startAddr%sector_size;
    uint32_t base2 = startAddr + dataLen - (startAddr+dataLen)%sector_size;
    uint32_t exist_bytes;
    uint32_t middle_start_addr;
    uint32_t buffer_offset;
    if ( startAddr % sector_size == 0 )
    {
        pre_bytes    = 0;
        middle_bytes = dataLen - dataLen%sector_size;
        post_bytes   = dataLen - middle_bytes;
    }
    else
    {
        if ( base1 == base2 )
        {
            pre_bytes = dataLen;
        }
        else
        {
            pre_bytes    = sector_size - startAddr%sector_size;
            middle_bytes = base2 - base1 - sector_size;
            post_bytes   = startAddr + dataLen - base2;
        }
    }
    TOTA_LOG_DBG(3, "pre:%u, middle:%u, post:%u", pre_bytes, middle_bytes, post_bytes);

    if ( pre_bytes != 0 )
    {
        memcpy(sector_buffer, (uint8_t*)base1, sector_size);
        if ( base1 == base2 )
            memcpy(sector_buffer+(startAddr-base1), dataBuf, dataLen);
        else
            memcpy(sector_buffer+(startAddr-base1), dataBuf, sector_size-(startAddr-base1));
        lock=int_lock_global();
        pmu_flash_write_config();
        hal_norflash_erase(HAL_FLASH_ID_0, base1, sector_size);
        hal_norflash_write(HAL_FLASH_ID_0, base1, sector_buffer, sector_size);
        pmu_flash_read_config();
        int_unlock_global(lock);
    }
    if ( middle_bytes != 0 )
    {
        if ( base1 != startAddr )
            middle_start_addr = base1+sector_size;
        else
            middle_start_addr = base1;
        buffer_offset = middle_start_addr - startAddr;
        lock=int_lock_global();
        pmu_flash_write_config();
        for ( uint32_t i = 0; i < middle_bytes/sector_size; i++ )
        {
            hal_norflash_erase(HAL_FLASH_ID_0, middle_start_addr+(i*sector_size), sector_size);
            hal_norflash_write(HAL_FLASH_ID_0, middle_start_addr+(i*sector_size), dataBuf+buffer_offset+(i*sector_size), sector_size);
        }
        pmu_flash_read_config();
        int_unlock_global(lock);
    }
    if ( post_bytes != 0 )
    {
        exist_bytes = sector_size - post_bytes;
        memcpy(sector_buffer, (uint8_t*)(base2+post_bytes), exist_bytes);

        lock=int_lock_global();
        pmu_flash_write_config();

        hal_norflash_erase(HAL_FLASH_ID_0, base2, FLASH_SECTOR_SIZE_IN_BYTES);
        hal_norflash_write(HAL_FLASH_ID_0, base2, dataBuf+(base2-startAddr), post_bytes);
        hal_norflash_write(HAL_FLASH_ID_0, base2+post_bytes, sector_buffer, exist_bytes);

        pmu_flash_read_config();
        int_unlock_global(lock);
    }

#else
    uint32_t sector_num = 0;

    if ( startAddr % sector_size != 0 || dataLen == 0)
    {
        TRACE(0,"[%s] TOTA WRITE FLASH ERR",__func__);
        return;
    }

    sector_num = dataLen/sector_size;
    lock=int_lock_global();
    pmu_flash_write_config();
    for ( uint32_t i = 0; i < sector_num; i++ )
    {
        hal_norflash_erase(HAL_FLASH_ID_0, startAddr+(i*sector_size), sector_size);
        hal_norflash_write(HAL_FLASH_ID_0, startAddr+(i*sector_size), dataBuf+(i*sector_size), sector_size);
    }
    if(dataLen - sector_num*sector_size)
    {
        hal_norflash_erase(HAL_FLASH_ID_0, startAddr+(sector_num*sector_size), sector_size);
        hal_norflash_write(HAL_FLASH_ID_0, startAddr+(sector_num*sector_size), dataBuf+(sector_num*sector_size), (dataLen - sector_num*sector_size));
    }
    pmu_flash_read_config();
    int_unlock_global(lock);
#endif
    return;
}

static void _erase_flash(uint32_t startAddr, uint32_t dataLen)
{
    uint32_t lock;
    TOTA_LOG_DBG(2,"erase %d bytes from flash addr 0x%x", dataLen, startAddr);

#if (TOTA_FLASH_SECTOR_BUF_USED)
    uint32_t pre_bytes = 0, middle_bytes = 0, post_bytes = 0;
    uint32_t base1 = startAddr - startAddr%sector_size;
    uint32_t base2 = startAddr + dataLen - (startAddr+dataLen)%sector_size;
    uint32_t exist_bytes;
    uint32_t middle_start_addr;
    if ( startAddr % sector_size == 0 )
    {
        pre_bytes    = 0;
        middle_bytes = dataLen - dataLen%sector_size;
        post_bytes   = dataLen - middle_bytes;
    }
    else
    {
        if ( base1 == base2 )
        {
            pre_bytes = dataLen;
        }
        else
        {
            pre_bytes    = sector_size - startAddr%sector_size;
            middle_bytes = base2 - base1 - sector_size;
            post_bytes   = startAddr + dataLen - base2;
        }
    }
    TOTA_LOG_DBG(3, "pre:%u, middle:%u, post:%u", pre_bytes, middle_bytes, post_bytes);

    if ( pre_bytes != 0 )
    {
        memcpy(sector_buffer, (uint8_t*)base1, sector_size);
        if ( base1 == base2 )
            memset(sector_buffer+(startAddr-base1), 0xff, dataLen);
        else
            memset(sector_buffer+(startAddr-base1), 0xff, sector_size-(startAddr-base1));
        lock=int_lock_global();
        pmu_flash_write_config();
        hal_norflash_erase(HAL_FLASH_ID_0, base1, sector_size);
        hal_norflash_write(HAL_FLASH_ID_0, base1, sector_buffer, sector_size);
        pmu_flash_read_config();
        int_unlock_global(lock);
    }
    if ( middle_bytes != 0 )
    {
        if ( base1 != startAddr )
            middle_start_addr = base1+sector_size;
        else
            middle_start_addr = base1;
        lock=int_lock_global();
        pmu_flash_write_config();
        for ( uint32_t i = 0; i < middle_bytes/sector_size; i++ )
        {
            hal_norflash_erase(HAL_FLASH_ID_0, middle_start_addr+(i*sector_size), sector_size);
        }
        pmu_flash_read_config();
        int_unlock_global(lock);
    }
    if ( post_bytes != 0 )
    {
        exist_bytes = sector_size - post_bytes;
        memcpy(sector_buffer, (uint8_t*)(base2+post_bytes), exist_bytes);

        lock=int_lock_global();
        pmu_flash_write_config();

        hal_norflash_erase(HAL_FLASH_ID_0, base2, FLASH_SECTOR_SIZE_IN_BYTES);
        hal_norflash_write(HAL_FLASH_ID_0, base2+post_bytes, sector_buffer, exist_bytes);

        pmu_flash_read_config();
        int_unlock_global(lock);
    }
#else
    uint32_t sector_num = 0;

    if ( startAddr % sector_size != 0 || dataLen == 0)
    {
        TRACE(0,"[%s] TOTA ERASE FLASH ERR",__func__);
        return;
    }

    sector_num = dataLen/sector_size;
    lock=int_lock_global();
    pmu_flash_write_config();
    for ( uint32_t i = 0; i < sector_num; i++ )
    {
        hal_norflash_erase(HAL_FLASH_ID_0, startAddr+(i*sector_size), sector_size);
    }
    if(dataLen - sector_num*sector_size)
    {
        hal_norflash_erase(HAL_FLASH_ID_0, startAddr+(sector_num*sector_size), sector_size);
    }
    pmu_flash_read_config();
    int_unlock_global(lock);
#endif
}

#if (TOTA_FLASH_SECTOR_BUF_USED)
static TOTA_FLASH_DATA_T * _read_flash_by_address(uint32_t readAddr, uint16_t length)
{
    TOTA_FLASH_DATA_T *flashData = (TOTA_FLASH_DATA_T *) sector_buffer;

    flashData->address = readAddr;
    flashData->length = length;
    memcpy(flashData->dataBuf, (uint8_t*)readAddr, length);

    return flashData;
}
#endif

#if (TOTA_DATA_TRANSCEIVER)

TOTA_FLASH_DATA_T * tota_read_flash(uint32_t readAddr, uint16_t length)
{
    TOTA_FLASH_DATA_T *flashData = (TOTA_FLASH_DATA_T *) sector_buffer;

    flashData->address = readAddr;
    flashData->length = length;
    memcpy(flashData->dataBuf, (uint8_t*)readAddr, length);

    return flashData;
}

bool tota_write_flash(uint32_t startAddr, uint8_t * dataBuf, uint32_t dataLen)
{
    uint32_t lock;
    TOTA_LOG_DBG(2,"flush %d bytes to flash addr 0x%x", dataLen, startAddr);
    //TOTA_LOG_DBG(1,"data: %s", (char *)dataBuf);

    uint32_t pre_bytes = 0, middle_bytes = 0, post_bytes = 0;
    uint32_t base1 = startAddr - startAddr%sector_size;
    uint32_t base2 = startAddr + dataLen - (startAddr+dataLen)%sector_size;
    uint32_t exist_bytes;
    uint32_t middle_start_addr;
    uint32_t buffer_offset;
    if ( startAddr % sector_size == 0 )
    {
        pre_bytes    = 0;
        middle_bytes = dataLen - dataLen%sector_size;
        post_bytes   = dataLen - middle_bytes;
    }
    else
    {
        if ( base1 == base2 )
        {
            pre_bytes = dataLen;
        }
        else
        {
            pre_bytes    = sector_size - startAddr%sector_size;
            middle_bytes = base2 - base1 - sector_size;
            post_bytes   = startAddr + dataLen - base2;
        }
    }
    TOTA_LOG_DBG(3, "pre:%u, middle:%u, post:%u", pre_bytes, middle_bytes, post_bytes);

    if ( pre_bytes != 0 )
    {
        memcpy(sector_buffer, (uint8_t*)base1, sector_size);
        if ( base1 == base2 )
            memcpy(sector_buffer+(startAddr-base1), dataBuf, dataLen);
        else
            memcpy(sector_buffer+(startAddr-base1), dataBuf, sector_size-(startAddr-base1));
        lock=int_lock_global();
        pmu_flash_write_config();
        hal_norflash_erase(HAL_FLASH_ID_0, base1, sector_size);
        hal_norflash_write(HAL_FLASH_ID_0, base1, sector_buffer, sector_size);
        pmu_flash_read_config();
        int_unlock_global(lock);
    }
    if ( middle_bytes != 0 )
    {
        if ( base1 != startAddr )
            middle_start_addr = base1+sector_size;
        else
            middle_start_addr = base1;
        buffer_offset = middle_start_addr - startAddr;
        lock=int_lock_global();
        pmu_flash_write_config();
        for ( uint32_t i = 0; i < middle_bytes/sector_size; i++ )
        {
            hal_norflash_erase(HAL_FLASH_ID_0, middle_start_addr+(i*sector_size), sector_size);
            hal_norflash_write(HAL_FLASH_ID_0, middle_start_addr+(i*sector_size), dataBuf+buffer_offset+(i*sector_size), sector_size);
        }
        pmu_flash_read_config();
        int_unlock_global(lock);
    }
    if ( post_bytes != 0 )
    {
        exist_bytes = sector_size - post_bytes;
        memcpy(sector_buffer, (uint8_t*)(base2+post_bytes), exist_bytes);

        lock=int_lock_global();
        pmu_flash_write_config();

        hal_norflash_erase(HAL_FLASH_ID_0, base2, FLASH_SECTOR_SIZE_IN_BYTES);
        hal_norflash_write(HAL_FLASH_ID_0, base2, dataBuf+(base2-startAddr), post_bytes);
        hal_norflash_write(HAL_FLASH_ID_0, base2+post_bytes, sector_buffer, exist_bytes);

        pmu_flash_read_config();
        int_unlock_global(lock);
    }

    // read back and check...
    uint8_t * pflash = (uint8_t*)startAddr;
    for ( uint32_t i = 0; i < dataLen; i ++)
    {
        if ( dataBuf[i] != pflash[i] )
        {
            TOTA_LOG_DBG(0, "write flash check error!");
            return false;
        }
    }

    return true;
}
#endif

TOTA_COMMAND_TO_ADD(OP_TOTA_WRITE_FLASH_CMD, _tota_flash_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_ERASE_FLASH_CMD, _tota_flash_cmd_handle, false, 0, NULL );
#if (TOTA_FLASH_SECTOR_BUF_USED)
TOTA_COMMAND_TO_ADD(OP_TOTA_READ_FLASH_CMD, _tota_flash_cmd_handle, false, 0, NULL );
#endif
