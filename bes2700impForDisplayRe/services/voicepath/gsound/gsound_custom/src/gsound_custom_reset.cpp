/***************************************************************************
*
*Copyright 2015-2019 BES.
*All rights reserved. All unpublished rights reserved.
*
*No part of this work may be used or reproduced in any form or by any
*means, or stored in a database or retrieval system, without prior written
*permission of BES.
*
*Use of this work is governed by a license granted by BES.
*This work contains confidential and proprietary information of
*BES. which is protected by copyright, trade secret,
*trademark and other intellectual property rights.
*
****************************************************************************/

/*****************************header include********************************/
#include "cmsis.h"
#include "gsound_custom_reset.h"
#include "gsound_dbg.h"
#include "gsound_target.h"
#include "crash_dump_section.h"
#include "norflash_api.h"

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declearation************************/
/*---------------------------------------------------------------------------
 *            get_crash_type_from_bes_dump
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get the libgsound defined crash type from BES internal crash dump data
 *
 * Parameters:
 *    pBuf - data buffer pointer of crash dump
 *
 * Return:
 *    see @GSoundResetReason to get more details
 */
static GSoundResetReason get_crash_type_from_bes_dump(struct CRASH_OUTPUT_BUF_T const *pBuf);

/************************private variable defination************************/
extern uint32_t __crash_dump_start;
extern uint32_t __crash_dump_end;

static const uint32_t gsound_dump_flash_start_addr = ( uint32_t )&__crash_dump_start;
static const uint32_t gsound_dump_flash_end_addr   = ( uint32_t )&__crash_dump_end;
static uint32_t gsound_dump_flash_len;

static uint32_t gsound_dump_flash_offs = 0;
static uint32_t gsound_dump_cur_dump_seqnum;
static bool gsound_dump_is_init = false;
static uint32_t gsound_dump_type;
static bool gsound_dump_is_happend           = false;
static CRASH_DATA_BUFFER *gsound_dump_buffer = NULL;

/****************************function defination****************************/
static GSoundResetReason get_crash_type_from_bes_dump(struct CRASH_OUTPUT_BUF_T const *pBuf)
{
    GSoundResetReason reason = GSOUND_RESET_TYPE_UNKNOWN;

    if (NULL != pBuf)
    {
        if (pBuf->hdr.magicCode == CRASH_DUMP_MAGIC_CODE)
        {
            if (CRASH_DUMP_ASSERT_CODE == pBuf->hdr.crashCode)
            {
                GLOG_I("CI: Reboot - Assert");
                reason = GSOUND_RESET_TYPE_ASSERT;
            }
            else if (CRASH_DUMP_EXCEPTION_CODE == pBuf->hdr.crashCode)
            {
                GLOG_I("CI: Reboot - Exception");
                reason = GSOUND_RESET_TYPE_EXCEPTION;
            }
        }
    }

    GLOG_I("CI: Reason: %d", reason);
    return reason;
}

void const *gsound_get_plateform_reset(void)
{
#if defined(DEBUG)
    if (gsound_is_crash_happened())
    {
        uint32_t crash_dump_address =
            gsound_get_latest_crash_dump_flash_addr();

        if (crash_dump_address > 0)
        {
            struct CRASH_OUTPUT_BUF_T *pBuf =
                ( struct CRASH_OUTPUT_BUF_T * )crash_dump_address;
            return pBuf;
        }
    }
#endif

    return NULL;
}

void gsound_crash_convert(GSoundCrashFatalDump *gsound_crash,
                          void const *platform_reset)
{
    struct CRASH_OUTPUT_BUF_T *pBuf = ( struct CRASH_OUTPUT_BUF_T * )platform_reset;

    ASSERT(NULL != gsound_crash, "invalid ptr");

    memset(gsound_crash, 0, sizeof(*gsound_crash));
    GSoundResetReason reason = get_crash_type_from_bes_dump(pBuf);
    gsound_crash->reason     = reason;

    if (GSOUND_RESET_TYPE_ASSERT == reason)
    {
        gsound_crash->panic_file.data = &pBuf->buf[pBuf->hdr.liteDumpOffset];
        gsound_crash->panic_file.len  = pBuf->hdr.bufpos - pBuf->hdr.liteDumpOffset;
        // for (int i = ( int )pBuf->hdr.liteDumpOffset; i < ( int )pBuf->hdr.bufpos; i++)
        // {
        //     TR_DEBUG(1,"%c", pBuf->buf[i]);
        // }
    }
    else if (GSOUND_RESET_TYPE_EXCEPTION == reason)
    {
        /**
         * ------------------------
         * BES Crash Dump Format - Exceptions:
         *    - Registers:
         *      - Start: liteDumpOffset
         *      - Len  : CRASH_DUMP_NUM_REGSITERS * 4
         *
         *    - MSP:
         *      - Start:  CRASH_DUMP_NUM_REGSITERS * 4
         *      - Len  : (CRASH_DUMP_STACK_NUM_BYTES / 2)
         *
         *    - PSP:
         *      - Start:  CRASH_DUMP_STACK_NUM_BYTES
         *      - Len  : (CRASH_DUMP_STACK_NUM_BYTES / 2)
         *                            -
         * ------------------------
         **/
        gsound_crash->registers.data = &pBuf->buf[pBuf->hdr.liteDumpOffset];
        gsound_crash->registers.len  = CRASH_DUMP_REGISTERS_NUM_BYTES;
        gsound_crash->stack.data     = &pBuf->buf[pBuf->hdr.liteDumpOffset + CRASH_DUMP_REGISTERS_NUM_BYTES];
        gsound_crash->stack.len      = CRASH_DUMP_STACK_NUM_BYTES;

        // for (int i = ( int )pBuf->hdr.liteDumpOffset; i < ( int )pBuf->hdr.bufpos; i = i + 4)
        // {
        //     GLOG_I("%02X%02X%02X%02X",
        //           pBuf->buf[i + 3],
        //           pBuf->buf[i + 2],
        //           pBuf->buf[i + 1],
        //           pBuf->buf[i]);
        // }
    }
}

void gsound_reset_convert(GSoundResetReason *gsound_reset,
                          void const *platform_reset)
{
    GSoundResetReason reason        = GSOUND_RESET_TYPE_NORMAL;
#if defined(DEBUG)
    struct CRASH_OUTPUT_BUF_T *pBuf = ( struct CRASH_OUTPUT_BUF_T * )platform_reset;

    GLOG_I("CI: crash just happen: %d", gsound_is_crash_happened());

    if (gsound_is_crash_happened())
    {
        reason = get_crash_type_from_bes_dump(pBuf);
    }
#endif

    if (NULL != gsound_reset)
    {
        *gsound_reset = reason;
    }
}

static void _gsound_get_next_seqnum(uint32_t *dump_seqnum, uint32_t *sector_seqnum)
{
    uint32_t i;
    uint32_t count;
    static enum NORFLASH_API_RET_T result;
    CRASH_OUTPUT_BUF_HEADER_T gsound_crash_header;
    uint32_t max_dump_seqnum   = 0;
    uint32_t max_sector_seqnum = 0;
    bool is_existed            = false;

    count = (gsound_dump_flash_end_addr - gsound_dump_flash_start_addr) / CRASH_DUMP_BUFFER_LEN;
    for (i = 0; i < count; i++)
    {
        result = ( enum NORFLASH_API_RET_T )crash_dump_read(gsound_dump_flash_start_addr + i * CRASH_DUMP_BUFFER_LEN,
                                                            ( uint8_t * )&gsound_crash_header,
                                                            sizeof(CRASH_OUTPUT_BUF_HEADER_T));

        if (result == NORFLASH_API_OK)
        {
            if (gsound_crash_header.magicCode == CRASH_DUMP_MAGIC_CODE &&
                // gsound_crash_header.partnum == count &&
                (gsound_crash_header.crashCode == CRASH_DUMP_ASSERT_CODE ||
                 gsound_crash_header.crashCode == CRASH_DUMP_EXCEPTION_CODE))
            {
                is_existed = true;
                if (gsound_crash_header.seqnum > max_dump_seqnum)
                {
                    max_dump_seqnum   = gsound_crash_header.seqnum;
                    max_sector_seqnum = i;
                }
            }
        }
        else
        {
            ASSERT(0, "_gsound_get_seqnum: crash_dump_read failed!result = %d.", result);
        }
    }
    if (is_existed)
    {
        *dump_seqnum   = max_dump_seqnum + 1;
        *sector_seqnum = max_sector_seqnum + 1 >= count ? 0 : max_sector_seqnum + 1;
    }
    else
    {
        *dump_seqnum   = 0;
        *sector_seqnum = 0;
    }
}

static bool _gsound_get_lastest_seqnum(uint32_t *dump_seqnum, uint32_t *sector_seqnum)
{
    uint32_t i;
    uint32_t count;
    static enum NORFLASH_API_RET_T result;
    CRASH_OUTPUT_BUF_HEADER_T gsound_crash_header;
    uint32_t max_dump_seqnum   = 0;
    uint32_t max_sector_seqnum = 0;
    bool is_existed            = false;

    count = (gsound_dump_flash_end_addr - gsound_dump_flash_start_addr) / CRASH_DUMP_BUFFER_LEN;
    for (i = 0; i < count; i++)
    {
        result = ( enum NORFLASH_API_RET_T )crash_dump_read(gsound_dump_flash_start_addr + i * CRASH_DUMP_BUFFER_LEN,
                                                            ( uint8_t * )&gsound_crash_header,
                                                            sizeof(CRASH_OUTPUT_BUF_HEADER_T));

        if (result == NORFLASH_API_OK)
        {
            if (gsound_crash_header.magicCode == CRASH_DUMP_MAGIC_CODE &&
                // gsound_crash_header.partnum == count&&
                (gsound_crash_header.crashCode == CRASH_DUMP_ASSERT_CODE ||
                 gsound_crash_header.crashCode == CRASH_DUMP_EXCEPTION_CODE))
            {
                is_existed = true;
                if (gsound_crash_header.seqnum > max_dump_seqnum)
                {
                    max_dump_seqnum   = gsound_crash_header.seqnum;
                    max_sector_seqnum = i;
                }
            }
        }
        else
        {
            ASSERT(0, "_gsound_get_seqnum: crash_dump_read failed!result = %d.", result);
        }
    }
    if (is_existed)
    {
        *dump_seqnum   = max_dump_seqnum;
        *sector_seqnum = max_sector_seqnum;
        return true;
    }
    else
    {
        *dump_seqnum   = 0;
        *sector_seqnum = 0;
        return false;
    }
}

static void gsound_dump_notify_handler(enum HAL_TRACE_STATE_T state)
{
    uint32_t lock;
    CRASH_DATA_BUFFER *data_buff;

    if (!gsound_dump_is_init)
    {
        return;
    }
    if ((state == HAL_TRACE_STATE_CRASH_ASSERT_START) ||
        (state == HAL_TRACE_STATE_CRASH_FAULT_START))
    {
        gsound_dump_buffer         = crash_dump_get_buffer();
        gsound_dump_buffer->offset = 0;
    }
    if (state == HAL_TRACE_STATE_CRASH_ASSERT_START)
    {
        gsound_dump_type = CRASH_DUMP_ASSERT_CODE;
    }
    else if (state == HAL_TRACE_STATE_CRASH_FAULT_START)
    {
        gsound_dump_type = CRASH_DUMP_EXCEPTION_CODE;
    }
    CRASH_DUMP_TRACE(2,"__CRASH_DUMP:%s: state = %d.", __func__, state);
    CRASH_DUMP_TRACE(3,"__CRASH_DUMP:%s: start_addr = 0x%x,end_addr = 0x%x.",
                     __func__,
                     gsound_dump_flash_start_addr,
                     gsound_dump_flash_end_addr);
    CRASH_DUMP_TRACE(3,"__CRASH_DUMP:%s: dump_seqnum = 0x%x,flash_offset = 0x%x.",
                     __func__,
                     gsound_dump_cur_dump_seqnum,
                     gsound_dump_flash_offs);

    if (HAL_TRACE_STATE_CRASH_END == state)
    {
        data_buff = crash_dump_get_buffer();
        CRASH_OUTPUT_BUF_HEADER_T *pHeader =
            ( CRASH_OUTPUT_BUF_HEADER_T * )(data_buff->buffer);
        pHeader->bufpos = data_buff->offset;
        uint32_t sector = 0;
        for (; sector < (data_buff->offset + CRASH_DUMP_SECTOR_SIZE - 1) / CRASH_DUMP_SECTOR_SIZE;
             sector++)
        {
            lock = int_lock_global();
            crash_dump_write(gsound_dump_flash_start_addr + gsound_dump_flash_offs + CRASH_DUMP_SECTOR_SIZE * sector,
                             data_buff->buffer + CRASH_DUMP_SECTOR_SIZE * sector,
                             CRASH_DUMP_SECTOR_SIZE);
            int_unlock_global(lock);
        }

        gsound_dump_flash_offs += (sector * CRASH_DUMP_SECTOR_SIZE);
    }
}

static void gsound_output_crash_handler(const unsigned char *buf, unsigned int buf_len)
{
    uint32_t uint_len;
    uint32_t written_len;
    uint32_t remain_len;
    CRASH_OUTPUT_BUF_HEADER_T log_header;
    CRASH_DATA_BUFFER *data_buff;

    if (!gsound_dump_is_init)
    {
        return;
    }

    if (strstr(( char * )buf, CRASH_DUMP_PREFIX) != NULL)
    {
        return;
    }

    data_buff   = gsound_dump_buffer;
    remain_len  = buf_len;
    written_len = 0;

    do
    {
        if (data_buff->offset == 0)
        {
            CRASH_DUMP_TRACE(4,"__CRASH_DUMP:%s:%d offset = 0x%x,dump_seqnum = %d.",
                             __func__,
                             __LINE__,
                             gsound_dump_flash_offs,
                             gsound_dump_cur_dump_seqnum);

            memset(( uint8_t * )&log_header, 0, sizeof(log_header));
            log_header.magicCode = CRASH_DUMP_MAGIC_CODE;

            log_header.seqnum         = gsound_dump_cur_dump_seqnum;
            log_header.partnum        = (gsound_dump_flash_end_addr - gsound_dump_flash_start_addr) / CRASH_DUMP_BUFFER_LEN;
            log_header.reserved       = 0x0;
            log_header.bufpos         = sizeof(CRASH_OUTPUT_BUF_HEADER_T);
            log_header.liteDumpOffset = sizeof(CRASH_OUTPUT_BUF_HEADER_T);
            log_header.crashCode      = gsound_dump_type;
            memcpy(data_buff->buffer + data_buff->offset,
                   ( uint8_t * )&log_header,
                   sizeof(log_header));

            data_buff->offset += sizeof(log_header);
            gsound_dump_cur_dump_seqnum++;
        }

        uint32_t leftSpaceInCurrentFlashSector;
        leftSpaceInCurrentFlashSector = CRASH_DUMP_SECTOR_SIZE - (data_buff->offset % CRASH_DUMP_SECTOR_SIZE);
        if (remain_len > leftSpaceInCurrentFlashSector)
        {
            uint_len = leftSpaceInCurrentFlashSector;
        }
        else
        {
            uint_len = remain_len;
        }
        if (uint_len > 0)
        {
            memcpy(data_buff->buffer + data_buff->offset,
                   buf + written_len,
                   uint_len);
            data_buff->offset += uint_len;
            written_len += uint_len;
        }

        remain_len -= uint_len;
    } while (remain_len > 0);
}

void gsound_crash_dump_init(void)
{
    uint32_t dump_seqnum   = 0;
    uint32_t sector_seqnum = 0;

    crash_dump_register(gsound_dump_notify_handler, gsound_output_crash_handler, gsound_output_crash_handler);
    _gsound_get_next_seqnum(&dump_seqnum, &sector_seqnum);
    CRASH_DUMP_TRACE(3,"__CRASH_DUMP:%s: dump_seqnum = 0x%x, sector_seqnum = 0x%x.",
                     __func__,
                     dump_seqnum,
                     sector_seqnum);
    gsound_dump_cur_dump_seqnum = dump_seqnum;
    gsound_dump_flash_len       = gsound_dump_flash_end_addr - gsound_dump_flash_start_addr;
    gsound_dump_flash_offs      = sector_seqnum * CRASH_DUMP_BUFFER_LEN;
    gsound_dump_is_init         = true;
}

void gsound_dump_set_flag(uint8_t is_happend)
{
    gsound_dump_is_happend = is_happend;
}

bool gsound_is_crash_happened(void)
{
    return gsound_dump_is_happend;
}

uint32_t gsound_get_latest_crash_dump_flash_addr(void)
{
    uint32_t dump_seqnum  = 0;
    uint32_t flash_seqnum = 0;

    _gsound_get_lastest_seqnum(&dump_seqnum, &flash_seqnum);
    return gsound_dump_flash_start_addr + flash_seqnum * CRASH_DUMP_BUFFER_LEN;
}
