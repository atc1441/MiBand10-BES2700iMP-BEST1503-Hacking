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
#include <stdio.h>
#include "hal_trace.h"
#include "aud_section.h"
#include "crc32_c.h"
#include "tgt_hardware.h"
#include "string.h"
#include "hal_timer.h"
#include "hal_norflash.h"
#include "norflash_api.h"
#include "cmsis.h"
#ifdef __ARMCC_VERSION
#include "link_sym_armclang.h"
#endif

#ifdef AUDIO_SECTION_DEBUG
#define AUD_FLASH_TRACE TRACE
#else
#define AUD_FLASH_TRACE(level,...)
#endif

extern uint32_t __aud_start[];
extern uint32_t __aud_end[];
extern uint32_t __anc_start[];
extern uint32_t __anc_end[];

#ifndef ANC_COEF_LIST_NUM
#define ANC_COEF_LIST_NUM                   0
#endif

#define MAGIC_NUMBER            0xBE57
#define USERDATA_VERSION        0x0001

static uint32_t section_device_length[AUDIO_SECTION_DEVICE_NUM] = {
    AUDIO_SECTION_LENGTH_ANC,
    AUDIO_SECTION_LENGTH_AUDIO,
    AUDIO_SECTION_LENGTH_SPEECH,
};

void audio_section_callback(void* param)
{
    NORFLASH_API_OPERA_RESULT *opera_result;
    opera_result = (NORFLASH_API_OPERA_RESULT*)param;

    AUD_FLASH_TRACE(6,"%s:type = %d, addr = 0x%x,len = 0x%x,remain = %d,result = %d,suspend_num= %d.",
                __func__,
                opera_result->type,
                opera_result->addr,
                opera_result->len,
                opera_result->remain_num,
                opera_result->result,
                opera_result->suspend_num);
}

static uint8_t audio_section_inited = false;
static void audio_section_init(void)
{
    if(audio_section_inited)
        return;

#ifndef FPGA
    enum NORFLASH_API_RET_T result;
    enum HAL_FLASH_ID_T flash_id;
    uint32_t sector_size = 0;
    uint32_t block_size = 0;
    uint32_t page_size = 0;

    flash_id = norflash_api_get_dev_id_by_addr((uint32_t)__aud_start);
    hal_norflash_get_size(flash_id,
                          NULL,
                          &block_size,
                          &sector_size,
                          &page_size);
    result = norflash_api_register(NORFLASH_API_MODULE_ID_AUDIO,
                                   flash_id,
                                   ((uint32_t)__aud_start),
                                   ((uint32_t)__aud_end - (uint32_t)__aud_start),
                                   block_size,
                                   sector_size,
                                   page_size,
                                   ((uint32_t)__aud_end - (uint32_t)__aud_start),
                                   audio_section_callback);
    ASSERT(result == NORFLASH_API_OK, "audio_section_init: module register failed! result = %d.", result);

    audio_section_inited = true;
#endif
}

static uint8_t anc_section_inited = false;
static void anc_section_init(void)
{
    if(anc_section_inited)
        return;

#ifndef FPGA
    enum NORFLASH_API_RET_T result;
    enum HAL_FLASH_ID_T flash_id;
    uint32_t sector_size = 0;
    uint32_t block_size = 0;
    uint32_t page_size = 0;

    flash_id = norflash_api_get_dev_id_by_addr((uint32_t)__anc_start);
    hal_norflash_get_size(flash_id,
                          NULL,
                          &block_size,
                          &sector_size,
                          &page_size);
    result = norflash_api_register(NORFLASH_API_MODULE_ID_ANC,
                                   flash_id,
                                   ((uint32_t)__anc_start),
                                   ((uint32_t)__anc_end - (uint32_t)__anc_start),
                                   block_size,
                                   sector_size,
                                   page_size,
                                   ((uint32_t)__anc_end - (uint32_t)__anc_start),
                                   audio_section_callback);
    ASSERT(result == NORFLASH_API_OK, "anc_section_init: module register failed! result = %d.", result);

    anc_section_inited = true;
#endif
}

enum NORFLASH_API_RET_T audio_section_erase(uint32_t start_addr, uint32_t len, bool is_async)
{
    //erase audio device[x] section
    enum NORFLASH_API_RET_T ret = 0;
    uint32_t t_size = 0;
    uint32_t b_size = 0;
    uint32_t s_size = 0;
    uint32_t p_size = 0;
    enum HAL_FLASH_ID_T flash_id;

    norflash_api_get_dev_id(NORFLASH_API_MODULE_ID_AUDIO, &flash_id);
    hal_norflash_get_size(flash_id, &t_size, &b_size, &s_size, &p_size);

    // judge start addr and len sector size alignment
    ASSERT(((start_addr & (s_size - 1)) == 0 &&
            (len & (s_size - 1)) == 0),
           "%s: No sec size alignment! start_addr = 0x%x, len = 0x%x",
           __func__,
           start_addr,
           len);

    do
    {
        hal_trace_pause();
        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_AUDIO, start_addr,
                                 s_size, is_async);
        hal_trace_continue();
        if(ret != NORFLASH_API_OK)
        {
            AUD_FLASH_TRACE(3, "%s:offset = 0x%x,ret = %d.", __func__, start_addr, ret);
            return ret;
        }
        start_addr += s_size;
        len -= s_size;
        if (len == 0)
        {
            break;
        }
    } while (1);

    return ret;
}

void audio_section_pending_op(enum NORFLASH_API_MODULE_ID_T module,
                              enum NORFLASH_API_OPRATION_TYPE type)
{
    do
    {
        hal_trace_pause();
        norflash_api_flush();
        hal_trace_continue();

        if (NORFLASH_API_ALL != type)
        {
            if (0 == norflash_api_get_used_buffer_count(module, type))
            {
                break;
            }
        }
        else
        {
            if (norflash_api_buffer_is_free(module))
            {
                break;
            }
        }
        hal_sys_timer_delay(MS_TO_HWTICKS(10));
    } while (1);

    // AUD_FLASH_TRACE(1,"%s: module:%d, type:%d done", __func__, module, type);
}

enum NORFLASH_API_RET_T audio_section_write(uint32_t start_addr, uint8_t* ptr, uint32_t len, bool is_async)
{
    enum NORFLASH_API_RET_T ret = NORFLASH_API_OK;
    uint32_t sec_num;
    uint32_t write_len;
    uint32_t written_len = 0;
    uint32_t t_size = 0;
    uint32_t b_size = 0;
    uint32_t s_size = 0;
    uint32_t p_size = 0;
    enum HAL_FLASH_ID_T flash_id;

    norflash_api_get_dev_id(NORFLASH_API_MODULE_ID_AUDIO, &flash_id);
    hal_norflash_get_size(flash_id, &t_size, &b_size, &s_size, &p_size);

    // judge start addr and len sector size alignment
    ASSERT((start_addr & (s_size - 1)) == 0,
           "%s: No sec size alignment! start_addr = 0x%x",
           __func__,
           start_addr);

    AUD_FLASH_TRACE(1,"%s: write: 0x%x,0x%x", __func__, start_addr, len);
    sec_num = (len + s_size - 1) / s_size;
    for (int i = 0; i < sec_num; i++)
    {
        if (len - written_len > s_size)
        {
            write_len = s_size;
        }
        else
        {
            write_len = len - written_len;
        }

        do
        {
            ret = norflash_api_write(NORFLASH_API_MODULE_ID_AUDIO,
                                     start_addr + written_len,
                                     ptr + written_len,
                                     write_len,
                                     is_async);

            if (NORFLASH_API_OK == ret)
            {
                AUD_FLASH_TRACE(1, "%s: norflash_api_write ok!", __func__);
                written_len += write_len;
                break;
            }
            else if (NORFLASH_API_BUFFER_FULL == ret)
            {
                AUD_FLASH_TRACE(1, "%s: buffer full! To flush it.", __func__);
                audio_section_pending_op(NORFLASH_API_MODULE_ID_AUDIO, NORFLASH_API_WRITTING);
            }
            else
            {
                ASSERT(0, "%s: norflash_api_write failed. ret = %d", __FUNCTION__, ret);
            }
        } while (1);
    }
    // AUD_FLASH_TRACE(1,"%s: write: 0x%x,0x%x done.", __func__, start_addr, len);
    return NORFLASH_API_OK;
}

static uint32_t audio_section_get_device_addr_offset(uint32_t device)
{
    ASSERT(device < AUDIO_SECTION_DEVICE_NUM, "[%s] device(%d) >= AUDIO_SECTION_DEVICE_NUM", __func__, device);

    uint32_t addr_offset = 0;

    for (uint32_t i=0; i<device; i++)
    {
        addr_offset += section_device_length[i];
    }

    return addr_offset;
}

static audio_section_t *audio_section_get_device_ptr(uint32_t device)
{
    uint8_t *section_ptr = (uint8_t *)__aud_start;
    section_ptr += audio_section_get_device_addr_offset(device);

    return (audio_section_t *)section_ptr;
}

int audio_section_store_cfg(uint32_t device, uint8_t *cfg, uint32_t len)
{
    audio_section_t *section_ptr = (audio_section_t *)cfg;
    uint32_t addr_start = 0;
    uint32_t crc = 0;
    enum NORFLASH_API_RET_T flash_opt_res;
    bool is_async = false;
    uint32_t t_size = 0;
    uint32_t b_size = 0;
    uint32_t s_size = 0;
    uint32_t p_size = 0;
    uint32_t flashOffset;
    enum HAL_FLASH_ID_T flash_id;

    audio_section_init();

    norflash_api_get_dev_id(NORFLASH_API_MODULE_ID_AUDIO, &flash_id);
    hal_norflash_get_size(flash_id, &t_size, &b_size, &s_size, &p_size);

    section_ptr->head.magic = MAGIC_NUMBER;
    section_ptr->head.version = USERDATA_VERSION;
    section_ptr->device = device;
    section_ptr->cfg_len = len;

    // calculate crc
    crc = crc32_c(0, (unsigned char *)section_ptr + AUDIO_SECTION_HEAD_LEN, len - AUDIO_SECTION_HEAD_LEN);
    // crc = crc32_c(0, (unsigned char *)section_ptr + AUDIO_SECTION_CFG_RESERVED_LEN, len - AUDIO_SECTION_CFG_RESERVED_LEN);
    section_ptr->head.crc = crc;

    // get flash offset
    flashOffset = audio_section_get_device_addr_offset(device);
    ASSERT((flashOffset & (s_size - 1)) == 0,
            "%s: No sec size alignment! offset = 0x%x",
            __func__,
            flashOffset);

    /// get logic address to write
    flash_opt_res = norflash_api_get_base_addr(NORFLASH_API_MODULE_ID_AUDIO, &addr_start);
    if (flash_opt_res)
    {
        AUD_FLASH_TRACE(2,"[%s] ERROR: get base address res = %d", __func__, flash_opt_res);
        return flash_opt_res;
    }

    addr_start = addr_start + flashOffset;

    AUD_FLASH_TRACE(2,"[%s] len = %d", __func__, len);
    AUD_FLASH_TRACE(2,"[%s] addr_start = 0x%x", __func__, addr_start);
    AUD_FLASH_TRACE(2,"[%s] block length = 0x%x", __func__, section_device_length[device]);

    flash_opt_res = audio_section_erase(addr_start, section_device_length[device], is_async);
    if (flash_opt_res)
    {
        AUD_FLASH_TRACE(2,"[%s] ERROR: erase flash res = %d", __func__, flash_opt_res);
        return flash_opt_res;
    }

    flash_opt_res = audio_section_write(addr_start, (uint8_t *)section_ptr, len, is_async);
    if (flash_opt_res)
    {
        AUD_FLASH_TRACE(2,"[%s] ERROR: write flash res = %d", __func__, flash_opt_res);
        return flash_opt_res;
    }

    AUD_FLASH_TRACE(1,"********************[%s]********************", __func__);
    AUD_FLASH_TRACE(1,"magic:      0x%x", section_ptr->head.magic);
    AUD_FLASH_TRACE(1,"version:    0x%x", section_ptr->head.version);
    AUD_FLASH_TRACE(1,"crc:        0x%x", section_ptr->head.crc);
    AUD_FLASH_TRACE(1,"device:     %d", section_ptr->device);
    AUD_FLASH_TRACE(1,"cfg_len:    %d", section_ptr->cfg_len);
    AUD_FLASH_TRACE(0,"********************END********************");

    // audio_section_t *section_read_ptr = audio_section_get_device_ptr(device);
    // check

    return 0;
}

int audio_section_load_cfg(uint32_t device, uint8_t *cfg, uint32_t len)
{
    audio_section_t *section_ptr = audio_section_get_device_ptr(device);
    uint32_t crc = 0;

    norflash_api_flash_operation_start((uint32_t) __aud_start);
    AUD_FLASH_TRACE(1,"********************[%s]********************", __func__);
    AUD_FLASH_TRACE(1,"magic:      0x%x", section_ptr->head.magic);
    AUD_FLASH_TRACE(1,"version:    0x%x", section_ptr->head.version);
    AUD_FLASH_TRACE(1,"crc:        0x%x", section_ptr->head.crc);
    AUD_FLASH_TRACE(1,"device:     %d", section_ptr->device);
    AUD_FLASH_TRACE(1,"cfg_len:    %d", section_ptr->cfg_len);
    AUD_FLASH_TRACE(0,"********************END********************");

    if(section_ptr->head.magic != MAGIC_NUMBER)
    {
        norflash_api_flash_operation_end((uint32_t) __aud_start);
        AUD_FLASH_TRACE(3,"[%s] WARNING: Different magic number (%x != %x)", __func__, section_ptr->head.magic, MAGIC_NUMBER);
        return -1;
    }

    // Calculate crc and check crc value
    crc = crc32_c(0, (unsigned char *)section_ptr + AUDIO_SECTION_HEAD_LEN, len - AUDIO_SECTION_HEAD_LEN);
    // crc = crc32_c(0, (unsigned char *)section_ptr + AUDIO_SECTION_CFG_RESERVED_LEN, len - AUDIO_SECTION_CFG_RESERVED_LEN);

    if(section_ptr->head.crc != crc)
    {
        norflash_api_flash_operation_end((uint32_t) __aud_start);
        AUD_FLASH_TRACE(3,"[%s] WARNING: Different crc (%x != %x)", __func__, section_ptr->head.crc, crc);
        return -2;
    }

    if(section_ptr->device != device)
    {
        norflash_api_flash_operation_end((uint32_t) __aud_start);
        AUD_FLASH_TRACE(3,"[%s] WARNING: Different device (%d != %d)", __func__, section_ptr->device, device);
        return -3;
    }

    if(section_ptr->cfg_len != len)
    {
        norflash_api_flash_operation_end((uint32_t) __aud_start);
        AUD_FLASH_TRACE(3,"[%s] WARNING: Different length (%d != %d)", __func__, section_ptr->cfg_len, len);
        return -4;
    }

    memcpy(cfg, section_ptr, len);
    norflash_api_flash_operation_end((uint32_t) __aud_start);

    return 0;
}

int anccfg_loadfrom_audsec(const struct_anc_cfg *list[], const struct_anc_cfg *list_44p1k[], uint32_t count)
{
#ifdef PROGRAMMER

    return 1;

#else // !PROGRAMMER

#ifdef CHIP_BEST1000
    ASSERT(0, "[%s] Can not support anc load in this branch!!!", __func__);
#else
    unsigned int re_calc_crc,i;
    const pctool_aud_section *audsec_ptr;

    AUD_FLASH_TRACE(1,"%s:PCTOOL_AUDSEC_RESERVED_LEN:%d", __func__,PCTOOL_AUDSEC_RESERVED_LEN);
    AUD_FLASH_TRACE(1,"%s:sizeof(pctool_anc_config_t):%d", __func__,sizeof(pctool_anc_config_t));

    norflash_api_flash_operation_start((uint32_t) __anc_start);
    audsec_ptr = (pctool_aud_section *)__anc_start;
    AUD_FLASH_TRACE(3,"0x%x,0x%x,0x%x",audsec_ptr->sec_head.magic,audsec_ptr->sec_head.version,audsec_ptr->sec_head.crc);
    if (audsec_ptr->sec_head.magic != aud_section_magic) {
        norflash_api_flash_operation_end((uint32_t) __anc_start);
        AUD_FLASH_TRACE(0,"Invalid aud section - magic");
        return 1;
    }
    re_calc_crc = crc32_c(0,(unsigned char *)&(audsec_ptr->sec_body),sizeof(audsec_body)-4);
    if (re_calc_crc != audsec_ptr->sec_head.crc){
        norflash_api_flash_operation_end((uint32_t) __anc_start);
        AUD_FLASH_TRACE(0,"crc verify failure, invalid aud section.");
        return 1;
    }
    AUD_FLASH_TRACE(0,"Valid aud section.");
    for(i=0;i<ANC_COEF_LIST_NUM;i++)
        list[i] = (struct_anc_cfg *)&(audsec_ptr->sec_body.anc_config.anc_config_arr[i].anc_cfg[PCTOOL_SAMPLERATE_48X8K]);
    for(i=0;i<ANC_COEF_LIST_NUM;i++)
        list_44p1k[i] = (struct_anc_cfg *)&(audsec_ptr->sec_body.anc_config.anc_config_arr[i].anc_cfg[PCTOOL_SAMPLERATE_44_1X8K]);
    norflash_api_flash_operation_end((uint32_t) __anc_start);
#endif

    return 0;

#endif // !PROGRAMMER
}


#if defined(PSAP_APP)
int psapcfg_loadfrom_audsec(const struct_psap_cfg *list[], const struct_psap_cfg *list_44p1k[], uint32_t count)
{
#ifdef PROGRAMMER

    return 1;

#else // !PROGRAMMER

    unsigned int re_calc_crc,i;
    const pctool_aud_section *audsec_ptr;

    AUD_FLASH_TRACE(1,"%s", __func__);

    norflash_api_flash_operation_start((uint32_t) __anc_start);
    audsec_ptr = (pctool_aud_section *)__anc_start;
    AUD_FLASH_TRACE(3,"0x%x,0x%x,0x%x",audsec_ptr->sec_head.magic,audsec_ptr->sec_head.version,audsec_ptr->sec_head.crc);
    if (audsec_ptr->sec_head.magic != aud_section_magic) {
        norflash_api_flash_operation_end((uint32_t) __anc_start);
        AUD_FLASH_TRACE(0,"Invalid aud section - magic");
        return 1;
    }
    re_calc_crc = crc32_c(0,(unsigned char *)&(audsec_ptr->sec_body),sizeof(audsec_body)-4);
    if (re_calc_crc != audsec_ptr->sec_head.crc){
        norflash_api_flash_operation_end((uint32_t) __anc_start);
        AUD_FLASH_TRACE(0,"crc verify failure, invalid aud section.");
        return 1;
    }
    AUD_FLASH_TRACE(0,"Valid aud section.");
    for(i=0;i<PSAP_COEF_LIST_NUM;i++)
        list[i] = (struct_psap_cfg *)&(audsec_ptr->sec_body.psap_config.psap_config_arr[i].psap_cfg[PCTOOL_SAMPLERATE_48X8K]);
    for(i=0;i<PSAP_COEF_LIST_NUM;i++)
        list_44p1k[i] = (struct_psap_cfg *)&(audsec_ptr->sec_body.psap_config.psap_config_arr[i].psap_cfg[PCTOOL_SAMPLERATE_44_1X8K]);
    norflash_api_flash_operation_end((uint32_t) __anc_start);

    return 0;

#endif // !PROGRAMMER
}
#endif

#if defined(AUDIO_ANC_SPKCALIB_HW)
int spkcalibcfg_loadfrom_audsec(const struct_spkcalib_cfg *list[], const struct_spkcalib_cfg *list_44p1k[], uint32_t count)
{
#ifdef PROGRAMMER

    return 1;

#else // !PROGRAMMER

    unsigned int re_calc_crc,i;
    const pctool_aud_section *audsec_ptr;

    AUD_FLASH_TRACE(1,"%s", __func__);

    norflash_api_flash_operation_start((uint32_t) __aud_start);
    audsec_ptr = (pctool_aud_section *)__aud_start;
    AUD_FLASH_TRACE(3,"0x%x,0x%x,0x%x",audsec_ptr->sec_head.magic,audsec_ptr->sec_head.version,audsec_ptr->sec_head.crc);
    if (audsec_ptr->sec_head.magic != aud_section_magic) {
        norflash_api_flash_operation_end((uint32_t) __aud_start);
        AUD_FLASH_TRACE(0,"Invalid aud section - magic");
        return 1;
    }
    re_calc_crc = crc32_c(0,(unsigned char *)&(audsec_ptr->sec_body),sizeof(audsec_body)-4);
    if (re_calc_crc != audsec_ptr->sec_head.crc){
        norflash_api_flash_operation_end((uint32_t) __aud_start);
        AUD_FLASH_TRACE(0,"crc verify failure, invalid aud section.");
        return 1;
    }
    AUD_FLASH_TRACE(0,"Valid aud section.");
    for(i=0;i<SPKCALIB_COEF_LIST_NUM;i++)
        list[i] = (struct_spkcalib_cfg *)&(audsec_ptr->sec_body.spkcalib_config.spkcalib_config_arr[i].spkcalib_cfg[PCTOOL_SAMPLERATE_48X8K]);
    for(i=0;i<SPKCALIB_COEF_LIST_NUM;i++)
        list_44p1k[i] = (struct_spkcalib_cfg *)&(audsec_ptr->sec_body.spkcalib_config.spkcalib_config_arr[i].spkcalib_cfg[PCTOOL_SAMPLERATE_44_1X8K]);
    norflash_api_flash_operation_end((uint32_t) __aud_start);

    return 0;

#endif // !PROGRAMMER
}
#endif

void audio_section_nv_register(void)
{
    audio_section_init();
}

void anc_section_nv_register(void)
{
    anc_section_init();
}


int anc_section_nv_erase(uint32_t start_addr, uint32_t len, bool is_async)
{
    enum NORFLASH_API_RET_T ret = 0;
    uint32_t t_size = 0;
    uint32_t b_size = 0;
    uint32_t s_size = 0;
    uint32_t p_size = 0;
    enum HAL_FLASH_ID_T flash_id;

    norflash_api_get_dev_id(NORFLASH_API_MODULE_ID_ANC, &flash_id);
    hal_norflash_get_size(flash_id, &t_size, &b_size, &s_size, &p_size);

    len = ((len + (s_size-1))/s_size) * s_size;

    // judge start addr and len sector size alignment
    ASSERT(((start_addr & (s_size - 1)) == 0 &&
            (len & (s_size - 1)) == 0),
           "%s: No sec size alignment! start_addr = 0x%x, len = 0x%x",
           __func__,
           start_addr,
           len);

    do
    {
        if (len == 0)
        {
            break;
        }
        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_ANC, start_addr,
                                 s_size, is_async);
        if(ret != NORFLASH_API_OK)
        {
            AUD_FLASH_TRACE(3, "%s:offset = 0x%x,ret = %d.", __func__, start_addr, ret);
            return ret;
        }
        start_addr += s_size;
        if(len >= s_size)
            len -= s_size;
    } while (1);

    return (int)ret;
}


int anc_section_nv_write(uint32_t start_addr, uint8_t* ptr, uint32_t len, bool is_async)
{
    enum NORFLASH_API_RET_T ret = NORFLASH_API_OK;
    uint32_t sec_num;
    uint32_t write_len;
    uint32_t written_len = 0;
    uint32_t t_size = 0;
    uint32_t b_size = 0;
    uint32_t s_size = 0;
    uint32_t p_size = 0;
    enum HAL_FLASH_ID_T flash_id;

    norflash_api_get_dev_id(NORFLASH_API_MODULE_ID_ANC, &flash_id);
    hal_norflash_get_size(flash_id, &t_size, &b_size, &s_size, &p_size);

    // judge start addr and len sector size alignment
    ASSERT((start_addr & (s_size - 1)) == 0,
           "%s: No sec size alignment! start_addr = 0x%x",
           __func__,
           start_addr);

    AUD_FLASH_TRACE(1,"%s: write: 0x%x,0x%x", __func__, start_addr, len);
    sec_num = (len + s_size - 1) / s_size;
    for (int i = 0; i < sec_num; i++)
    {
        if (len - written_len > s_size)
        {
            write_len = s_size;
        }
        else
        {
            write_len = len - written_len;
        }

        do
        {
            ret = norflash_api_write(NORFLASH_API_MODULE_ID_ANC,
                                     start_addr + written_len,
                                     ptr + written_len,
                                     write_len,
                                     is_async);

            if (NORFLASH_API_OK == ret)
            {
                AUD_FLASH_TRACE(1, "%s: norflash_api_write ok!", __func__);
                written_len += write_len;
                break;
            }
            else if (NORFLASH_API_BUFFER_FULL == ret)
            {
                AUD_FLASH_TRACE(1, "%s: buffer full! To flush it.", __func__);
                audio_section_pending_op(NORFLASH_API_MODULE_ID_ANC, NORFLASH_API_WRITTING);
            }
            else
            {
                ASSERT(0, "%s: norflash_api_write failed. ret = %d", __FUNCTION__, ret);
            }
        } while (1);
    }
    // AUD_FLASH_TRACE(1,"%s: write: 0x%x,0x%x done.", __func__, start_addr, len);
    return 0;
}

int audio_section_nv_erase(uint32_t start_addr, uint32_t len, bool is_async)
{
    enum NORFLASH_API_RET_T ret = 0;
    uint32_t t_size = 0;
    uint32_t b_size = 0;
    uint32_t s_size = 0;
    uint32_t p_size = 0;
    enum HAL_FLASH_ID_T flash_id;

    norflash_api_get_dev_id(NORFLASH_API_MODULE_ID_AUDIO, &flash_id);
    hal_norflash_get_size(flash_id, &t_size, &b_size, &s_size, &p_size);

    len = ((len + (s_size-1))/s_size) * s_size;

    // judge start addr and len sector size alignment
    ASSERT(((start_addr & (s_size - 1)) == 0 &&
            (len & (s_size - 1)) == 0),
           "%s: No sec size alignment! start_addr = 0x%x, len = 0x%x",
           __func__,
           start_addr,
           len);

    do
    {
        if (len == 0)
        {
            break;
        }
        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_AUDIO, start_addr,
                                 s_size, is_async);
        if(ret != NORFLASH_API_OK)
        {
            AUD_FLASH_TRACE(3, "%s:offset = 0x%x,ret = %d.", __func__, start_addr, ret);
            return ret;
        }
        start_addr += s_size;
        if(len >= s_size)
            len -= s_size;
    } while (1);

    return (int)ret;
}

int audio_section_nv_write(uint32_t start_addr, uint8_t* ptr, uint32_t len, bool is_async)
{
    enum NORFLASH_API_RET_T ret = NORFLASH_API_OK;
    uint32_t sec_num;
    uint32_t write_len;
    uint32_t written_len = 0;
    uint32_t t_size = 0;
    uint32_t b_size = 0;
    uint32_t s_size = 0;
    uint32_t p_size = 0;
    enum HAL_FLASH_ID_T flash_id;

    norflash_api_get_dev_id(NORFLASH_API_MODULE_ID_AUDIO, &flash_id);
    hal_norflash_get_size(flash_id, &t_size, &b_size, &s_size, &p_size);

    // judge start addr and len sector size alignment
    ASSERT((start_addr & (s_size - 1)) == 0,
           "%s: No sec size alignment! start_addr = 0x%x",
           __func__,
           start_addr);

    AUD_FLASH_TRACE(1,"%s: write: 0x%x,0x%x", __func__, start_addr, len);
    sec_num = (len + s_size - 1) / s_size;
    for (int i = 0; i < sec_num; i++)
    {
        if (len - written_len > s_size)
        {
            write_len = s_size;
        }
        else
        {
            write_len = len - written_len;
        }

        do
        {
            ret = norflash_api_write(NORFLASH_API_MODULE_ID_AUDIO,
                                     start_addr + written_len,
                                     ptr + written_len,
                                     write_len,
                                     is_async);

            if (NORFLASH_API_OK == ret)
            {
                AUD_FLASH_TRACE(1, "%s: norflash_api_write ok!", __func__);
                written_len += write_len;
                break;
            }
            else if (NORFLASH_API_BUFFER_FULL == ret)
            {
                AUD_FLASH_TRACE(1, "%s: buffer full! To flush it.", __func__);
                audio_section_pending_op(NORFLASH_API_MODULE_ID_AUDIO, NORFLASH_API_WRITTING);
            }
            else
            {
                ASSERT(0, "%s: norflash_api_write failed. ret = %d", __FUNCTION__, ret);
            }
        } while (1);
    }
    // AUD_FLASH_TRACE(1,"%s: write: 0x%x,0x%x done.", __func__, start_addr, len);
    return 0;
}

uint32_t audio_section_get_device_size(uint8_t device)
{
    if(AUDIO_SECTION_DEVICE_NUM > device){
        return section_device_length[device];
    }else{
        return 0;
    }
}