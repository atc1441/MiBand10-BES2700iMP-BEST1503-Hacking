/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#include <stdint.h>
#include <string.h>
#include "tz_ota.h"
#include "plat_types.h"
#include "hal_trace.h"
#include "hal_location.h"
#include "crc32_c.h"
#include "hal_cmu.h"
#include "hal_norflash.h"
#include "tz_flash.h"
#include "tz_ota_sec_aes_crypt.h"
#include "tz_ota_sec_mem.h"

#define NORMAL_BOOT		0xBE57EC1C
#define COPY_NEW_IMAGE	0x5a5a5a5a
#define BOOT_WORD_A     0xAAAAAAAA
#define BOOT_WORD_B     0xBBBBBBBB
#define BOOT_WORD_C     0xCCCCCCCC
#define BOOT_WORD_D     0xDDDDDDDD

#define CMSE_OTA_FLASH_ID   HAL_FLASH_ID_0

#ifdef FLASH_REMAP
#define OTA_NEW_IMAGE_START_ADDRESS     OTA_CODE_OFFSET
#define OTA_NEW_IMAGE_SIZE              OTA_REMAP_OFFSET
#else
#define OTA_NEW_IMAGE_START_ADDRESS     CMSE_OTA_BACKUP_START_ADDRESS
#define OTA_NEW_IMAGE_SIZE              CMSE_OTA_BACKUP_SIZE
#endif

#define LEN_OF_IMAGE_TAIL_TO_FIND_SANITY_CRC    512
static const char* image_info_sanity_crc_key_word = "CRC32_OF_IMAGE=0x";

#define PARSER_MAX_SIZE     (4*1024)
#define DECRYPT_LEN_ALIGN   (16)

#if (AES_CBC_SRC_BUF_MAX_SIZE < PARSER_MAX_SIZE)
#error "PARSER_MAX_SIZE should > PARSER_MAX_SIZE"
#endif

enum{
    SEC_CRYPT_FILE_OPERATION_TYPE_FLASH,
    SEC_CRYPT_FILE_OPERATION_TYPE_RAM
};

typedef struct{
    unsigned char * crypt_in_buf;
    unsigned int crypt_in_buf_len;
    unsigned char * crypt_out_buf;
    unsigned int crypt_out_buf_len;
}crypt_buf_type_t;

typedef struct
{
    uint32_t magicNumber;    // NORMAL_BOOT or COPY_NEW_IMAGE
    uint32_t imageSize;
    uint32_t imageCrc;
    uint32_t newImageFlashOffset;
    uint32_t boot_word;
    uint32_t tzImageSize;
    uint32_t tzImageCrc;
    uint32_t tzNewImageFlashOffset;
} FLASH_OTA_BOOT_INFO_T;


static uint32_t g_segment_crc = 0;
static uint32_t g_whole_crc = 0;
static uint32_t g_whole_crc_backup = 0;
static uint32_t g_decryption_crc = 0;
static uint32_t g_decryption_crc_backup = 0;
static uint32_t g_total_len = 0;
static uint32_t g_ota_rw_buf_length = 0;
static uint32_t g_flash_write_offset = 0;
static uint32_t g_raw_buf_length = 0;

static bool ota_check_flag = false;
static uint8_t cmse_ota_rw_buffer[FLASH_SECTOR_SIZE_IN_BYTES];
static uint8_t cmse_ota_decryption_buffer[FLASH_SECTOR_SIZE_IN_BYTES];
uint8_t cmse_ota_raw_buffer[FLASH_SECTOR_SIZE_IN_BYTES];

static crypt_buf_type_t decrypt_buf_node;



static uint8_t asciiToHex(uint8_t ascii_code)
{
    if ((ascii_code >= '0') && (ascii_code <= '9'))
    {
        return ascii_code - '0';
    }
    else if ((ascii_code >= 'a') && (ascii_code <= 'f'))
    {
        return ascii_code - 'a' + 10;
    }
    else if ((ascii_code >= 'A') && (ascii_code <= 'F'))
    {
        return ascii_code - 'A' + 10;
    }
    else
    {
        return 0xff;
    }
}

static int32_t find_key_word(uint8_t *target_array, uint32_t target_array_len,
                             uint8_t *key_word, uint32_t key_word_len)
{
    if ((key_word_len > 0) && (target_array_len >= key_word_len))
    {
        uint32_t index = 0, target_index = 0;
        for (target_index = 0;target_index < target_array_len;target_index++)
        {
            for (index = 0;index < key_word_len;index++)
            {
                if (target_array[target_index + index] != key_word[index])
                {
                    break;
                }
            }

            if (index == key_word_len)
            {
                return target_index;
            }
        }

        return -1;
    }
    else
    {
        return -1;
    }
}

static bool cmse_ota_check_image_data_sanity_crc(uint32_t total_image_size)
{
    // find the location of the CRC key word string
    uint8_t *last_4k_image = NULL;
    uint32_t sanity_crc32 = 0;
    uint32_t todo_check_length = 0;
    uint32_t check_done_length = 0;
    uint32_t check_now_length = 0;
    uint32_t calculated_crc32 = 0;
    uint32_t start_addr = FLASH_NC_BASE + OTA_NEW_IMAGE_START_ADDRESS;

    uint32_t sanity_start_addr = start_addr + ((total_image_size - LEN_OF_IMAGE_TAIL_TO_FIND_SANITY_CRC) / 512) * 512;
    uint32_t sanity_length = LEN_OF_IMAGE_TAIL_TO_FIND_SANITY_CRC + (total_image_size - LEN_OF_IMAGE_TAIL_TO_FIND_SANITY_CRC) % 512;
    TRACE(2, "cmse sanity_crc read addr 0x%x length %d", sanity_start_addr, sanity_length);
    sec_flash_read(CMSE_OTA_FLASH_ID, sanity_start_addr, cmse_ota_rw_buffer, sanity_length);
    last_4k_image = cmse_ota_rw_buffer;

    int32_t sanity_crc_location = find_key_word(last_4k_image, sanity_length,
                                                (uint8_t *)image_info_sanity_crc_key_word,
                                                strlen(image_info_sanity_crc_key_word));
    if (-1 == sanity_crc_location)
    {
        TRACE(0, "%s error: no crc_key_word", __func__);
        return false;
    }

    TRACE(0, "sanity_crc_location is %d", sanity_crc_location);

    uint8_t *crcString = (uint8_t *)(last_4k_image + sanity_crc_location + strlen(image_info_sanity_crc_key_word));

    for (uint8_t index = 0; index < 8; index++)
    {
        sanity_crc32 |= (asciiToHex(crcString[index]) << (28 - 4 * index));
    }

    TRACE(0, "sanity_crc32 is 0x%x", sanity_crc32);

    // generate the CRC from image data
    todo_check_length = sanity_crc_location + total_image_size -
                         sanity_length + strlen(image_info_sanity_crc_key_word);
    TRACE(0, "Bytes to generate crc32 is %d", todo_check_length);

    while (todo_check_length > check_done_length)
    {
        check_now_length = (todo_check_length - check_done_length) > FLASH_SECTOR_SIZE_IN_BYTES
                                ? FLASH_SECTOR_SIZE_IN_BYTES
                                : (todo_check_length - check_done_length);
        sec_flash_read(CMSE_OTA_FLASH_ID, start_addr+check_done_length, cmse_ota_rw_buffer, check_now_length);
        check_done_length += check_now_length;
        calculated_crc32 = crc32_c(calculated_crc32, cmse_ota_rw_buffer, check_now_length);
    }

    TRACE(0, "calculated_crc32 is 0x%x", calculated_crc32);

    return (sanity_crc32 == calculated_crc32);
}

POSSIBLY_UNUSED static void cmse_ota_update_magic_number(uint32_t newMagicNumber)
{
    uint32_t start_addr = FLASH_NC_BASE + OTA_NEW_IMAGE_START_ADDRESS;

    sec_flash_read(CMSE_OTA_FLASH_ID, start_addr, cmse_ota_rw_buffer, FLASH_SECTOR_SIZE_IN_BYTES);
    *(uint32_t *)cmse_ota_rw_buffer = newMagicNumber;
    sec_flash_erase(CMSE_OTA_FLASH_ID, start_addr, FLASH_SECTOR_SIZE_IN_BYTES, true);
    sec_flash_program(CMSE_OTA_FLASH_ID, start_addr, cmse_ota_rw_buffer, FLASH_SECTOR_SIZE_IN_BYTES, true);
}

static uint32_t  cmse_ota_flush_data_to_flash(uint8_t *buf, uint32_t len, bool flush)
{
    uint32_t write_len = 0;
    uint32_t left_len = 0;

    TRACE(2, "cmse flush %d bytes to flash offset 0x%x", len, g_flash_write_offset);
    uint32_t start_addr = FLASH_NC_BASE + OTA_NEW_IMAGE_START_ADDRESS + g_flash_write_offset;

    if ((g_ota_rw_buf_length + len) >= FLASH_SECTOR_SIZE_IN_BYTES)
    {
        write_len = FLASH_SECTOR_SIZE_IN_BYTES - g_ota_rw_buf_length;
        left_len = len - write_len;
        memcpy(&cmse_ota_rw_buffer[g_ota_rw_buf_length], buf, write_len);
        sec_flash_erase(CMSE_OTA_FLASH_ID, start_addr, FLASH_SECTOR_SIZE_IN_BYTES, true);
        sec_flash_program(CMSE_OTA_FLASH_ID, start_addr, cmse_ota_rw_buffer, FLASH_SECTOR_SIZE_IN_BYTES, true);
        g_ota_rw_buf_length = 0;
        g_flash_write_offset += FLASH_SECTOR_SIZE_IN_BYTES;
        start_addr += FLASH_SECTOR_SIZE_IN_BYTES;
    }
    else
    {
        write_len = 0;
        left_len = len;
    }

    if (left_len)
    {
        memcpy(&cmse_ota_rw_buffer[g_ota_rw_buf_length], &buf[write_len], left_len);
        g_ota_rw_buf_length += left_len;
    }

    if (flush && g_ota_rw_buf_length)
    {
        sec_flash_erase(CMSE_OTA_FLASH_ID, start_addr, FLASH_SECTOR_SIZE_IN_BYTES, true);
        sec_flash_program(CMSE_OTA_FLASH_ID, start_addr, cmse_ota_rw_buffer, g_ota_rw_buf_length, true);
    }

    return len;
}

static void cmse_ota_write_packet_align(uint32_t offset)
{
    uint32_t start_addr = FLASH_NC_BASE + OTA_NEW_IMAGE_START_ADDRESS + 
        (offset/FLASH_SECTOR_SIZE_IN_BYTES) * FLASH_SECTOR_SIZE_IN_BYTES;
    uint32_t pre_bytes = offset % FLASH_SECTOR_SIZE_IN_BYTES;

    if (pre_bytes)
    {
        sec_flash_read(CMSE_OTA_FLASH_ID, start_addr, cmse_ota_rw_buffer, FLASH_SECTOR_SIZE_IN_BYTES);
        g_ota_rw_buf_length = pre_bytes;
    }
    else
    {
        g_ota_rw_buf_length = 0;
    }
    g_flash_write_offset = offset - pre_bytes;
}

static uint32_t cmse_ota_get_boot_word(void)
{
    if (sec_flash_is_remap_enabled())
    {
        FLASH_OTA_BOOT_INFO_T otaBootInfo;
        sec_flash_read(CMSE_OTA_FLASH_ID, OTA_SEC_BOOT_INFO_OFFSET, (uint8_t*)&otaBootInfo, sizeof(FLASH_OTA_BOOT_INFO_T));
        TRACE(0, "%s boot word 0x%x", __func__, otaBootInfo.boot_word);
        return otaBootInfo.boot_word;
    }
    else
    {
        return 0;
    }
}


static void cmse_ota_update_boot_info(uint32_t ns_offset, uint32_t ns_length, uint32_t ns_crc)
{
    uint32_t boot_word = 0x0;
    FLASH_OTA_BOOT_INFO_T boot_info;

    memset((uint8_t*)&boot_info, 0x0, sizeof(FLASH_OTA_BOOT_INFO_T));
    
    boot_info.magicNumber = COPY_NEW_IMAGE;
    if(sec_flash_is_remap_enabled())
    {
        boot_word = cmse_ota_get_boot_word();
        boot_info.newImageFlashOffset = ns_offset;
        boot_info.imageSize = ns_length;
        boot_info.imageCrc = ns_crc;
        boot_info.tzNewImageFlashOffset = OTA_NEW_IMAGE_START_ADDRESS;
        boot_info.tzImageSize = OTA_NEW_IMAGE_SIZE;
        boot_info.tzImageCrc = g_decryption_crc;
        if (boot_word == BOOT_WORD_D)
            boot_info.boot_word = BOOT_WORD_C;
        else
            boot_info.boot_word = BOOT_WORD_D;
    }
    else
    {
        boot_info.newImageFlashOffset = ns_offset;
        boot_info.imageSize = ns_length;
        boot_info.imageCrc = ns_crc;
        boot_info.tzNewImageFlashOffset = OTA_NEW_IMAGE_START_ADDRESS;
        boot_info.tzImageSize = g_total_len;
        boot_info.tzImageCrc = g_decryption_crc;
    }
    sec_flash_erase(CMSE_OTA_FLASH_ID, OTA_SEC_BOOT_INFO_OFFSET, FLASH_SECTOR_SIZE_IN_BYTES, true);
    sec_flash_program(CMSE_OTA_FLASH_ID, OTA_SEC_BOOT_INFO_OFFSET, 
            (uint8_t*)&boot_info, sizeof(FLASH_OTA_BOOT_INFO_T), true);
}


POSSIBLY_UNUSED static void sec_crypt_buf_node_init(void)
{
    decrypt_buf_node.crypt_in_buf_len = PARSER_MAX_SIZE;
    decrypt_buf_node.crypt_out_buf_len = PARSER_MAX_SIZE;
}

POSSIBLY_UNUSED static void sec_crypt_file_operation_read(char type, char * in_buf, unsigned int in_len,char ** out_buf)
{
    if(type == SEC_CRYPT_FILE_OPERATION_TYPE_FLASH){
        hal_norflash_read(HAL_FLASH_ID_0,(uint32_t)in_buf,decrypt_buf_node.crypt_in_buf,in_len);
        *out_buf = (char *)decrypt_buf_node.crypt_in_buf;
    }else{
        *out_buf = in_buf;
    }
}

POSSIBLY_UNUSED static void sec_crypt_file_operation_write(char type, char * in_buf, unsigned int in_len,char * out_buf,unsigned int out_len)
{
    if(type == SEC_CRYPT_FILE_OPERATION_TYPE_FLASH){

    }else{
        unsigned int op_len = MIN(in_len,out_len);
        memcpy(out_buf,in_buf,op_len);
    }
}

POSSIBLY_UNUSED static int sec_decypt_operation_handler(char * infile,
                                        uint32_t infile_len,char * outfile,uint32_t *outfile_len)
{
    uint32_t decrypt_len = 0;

    uint32_t i = 0;
    uint32_t crypt_iter_op_in_len = 0;
    uint32_t crypt_iter_op_size = 0;
    uint32_t crypt_iter_op_remain = 0;

    unsigned char *outbuf;

    char * op_file_pos = (char *)infile;
    char * op_file_in_buf = NULL;

    outbuf = decrypt_buf_node.crypt_out_buf;

    if(outbuf == NULL){
        ASSERT(0,"%s buf error",__func__);
    }

    crypt_iter_op_size = decrypt_buf_node.crypt_out_buf_len;
    crypt_iter_op_remain = infile_len;

    for(i = 0; i < infile_len; i+= crypt_iter_op_in_len){
        decrypt_len = 0;

        crypt_iter_op_in_len = crypt_iter_op_size;

        if(crypt_iter_op_remain < PARSER_MAX_SIZE){
            crypt_iter_op_in_len = crypt_iter_op_remain;
        }

        sec_crypt_file_operation_read(SEC_CRYPT_FILE_OPERATION_TYPE_RAM,op_file_pos,crypt_iter_op_in_len,&op_file_in_buf);

        bin_aes_crypt_handler_if(MODE_DECRYPT,op_file_in_buf,crypt_iter_op_in_len,(char*)outbuf,&decrypt_len);

        sec_crypt_file_operation_write(SEC_CRYPT_FILE_OPERATION_TYPE_RAM,(char*)outbuf,decrypt_len,(char *)(outfile + i),decrypt_len);

        op_file_pos += crypt_iter_op_in_len;

        crypt_iter_op_remain -= crypt_iter_op_in_len;
    }

    if(outfile_len){
        *outfile_len = infile_len;
    }

    return 0;
}

POSSIBLY_UNUSED static int cmse_ota_decypt_handler(const char * infile,
                                        uint32_t infile_len,const char * outfile,uint32_t *outfile_len)
{
    TRACE(0,"%s start",__func__);

    sec_decypt_operation_handler((char *)infile,infile_len,(char *)outfile,outfile_len);

    TRACE(1,"%s success",__func__);

    return 0;
}

static uint32_t cmse_ota_decypt_and_write_flash(uint8_t *buf, uint32_t len, bool flush)
{
    uint32_t decrypt_buf_len = 0;

    memset(cmse_ota_decryption_buffer, 0, sizeof(cmse_ota_decryption_buffer));
    cmse_ota_decypt_handler((const char *)buf, len, (const char *)cmse_ota_decryption_buffer, &decrypt_buf_len);

    g_decryption_crc = crc32_c(g_decryption_crc, cmse_ota_decryption_buffer, decrypt_buf_len);
    g_total_len += decrypt_buf_len;

    return cmse_ota_flush_data_to_flash(cmse_ota_decryption_buffer, decrypt_buf_len, flush);
}

POSSIBLY_UNUSED static void cmse_ota_sec_crypt_init(void)
{
    static bool initFlag = false;
    if (initFlag)
    {
        bin_aes_crypt_deinit_if();
        initFlag = false;
    }

    uint8_t demo_aes_cbc_key[] = {0x1F, 0x81, 0x99, 0x00, 0x4C, 0xB7, 0xFB, 0xAF, 0x07, 0x0C, 0x48, 0x64, 0x49, 0x53, 0x99, 0xA0};
    uint8_t demo_aes_cbc_iv[] = {0x3F, 0xA8, 0xDC, 0x14, 0x9F, 0x54, 0xD3, 0x69, 0xF7, 0x1E, 0x73, 0x86, 0xBD, 0x37, 0xEE, 0x50};

    uint32_t demo_aes_cbc_key_len = sizeof(demo_aes_cbc_key);
    uint32_t demo_aes_cbc_iv_len = sizeof(demo_aes_cbc_iv);

    sec_crypt_buf_node_init();

    sec_mem_total_init();

    sec_mem_buf_alloc(&(decrypt_buf_node.crypt_in_buf),decrypt_buf_node.crypt_in_buf_len);

    sec_mem_buf_alloc(&(decrypt_buf_node.crypt_out_buf),decrypt_buf_node.crypt_out_buf_len);

    bin_aes_crypt_init_if((const char *)demo_aes_cbc_key,demo_aes_cbc_key_len,(const char *)demo_aes_cbc_iv,demo_aes_cbc_iv_len);

    initFlag = true;

}

POSSIBLY_UNUSED static void cmse_ota_reset_env(void)
{
    g_segment_crc = 0;
    g_whole_crc = 0;
    g_whole_crc_backup = 0;
    g_decryption_crc = 0;
    g_decryption_crc_backup = 0;
    g_total_len = 0;
    g_ota_rw_buf_length = 0;
    g_flash_write_offset = 0;
    g_raw_buf_length = 0;
}

static void cmse_ota_calculate_crc(uint32_t offset, uint8_t *buf, uint32_t len)
{
    static uint32_t offset_backup = 0;

    if (g_segment_crc == 0)
    {
        g_whole_crc_backup = g_whole_crc;
        g_decryption_crc_backup = g_decryption_crc;
    }
    if (offset_backup > offset)
    {
        //segment error, retry send.
        g_whole_crc = g_whole_crc_backup;
        g_decryption_crc = g_decryption_crc_backup;
    }
    offset_backup = offset;
    g_segment_crc = crc32_c(g_segment_crc, buf, len);
    g_whole_crc = crc32_c(g_whole_crc, buf, len);
}

CMSE_API void cmse_ota_start(void)
{
    g_segment_crc = 0;
    g_whole_crc = 0;
    g_total_len = 0;
    g_decryption_crc = 0;
}

CMSE_API void cmse_ota_set_segment_crc(uint32_t crc_origin)
{
    cmse_ota_sec_crypt_init();
    cmse_ota_reset_env();
    g_segment_crc = crc_origin;
}

CMSE_API uint32_t cmse_ota_get_segment_crc(void)
{
    uint32_t crc = g_segment_crc;
    g_segment_crc = 0;
    return crc;
}

CMSE_API uint32_t cmse_ota_get_whole_crc(void)
{
   uint32_t transfer_crc = g_whole_crc;

    if (g_raw_buf_length)
    {
        cmse_ota_decypt_and_write_flash(cmse_ota_raw_buffer, g_raw_buf_length, true);
    }

    if (sec_flash_is_remap_enabled())
    {
        ota_check_flag = sec_flash_check_signature(CMSE_OTA_FLASH_ID, OTA_NEW_IMAGE_START_ADDRESS, g_total_len);
    }
    else
    {
        ota_check_flag = cmse_ota_check_image_data_sanity_crc(g_total_len);
    }

    if (ota_check_flag)
    {
        transfer_crc = g_whole_crc;
    }
    else
    {
        transfer_crc = 0;
    }
    TRACE(0, "%s check_flag %d whole_crc 0x%x", __func__, ota_check_flag, g_whole_crc);
    g_whole_crc = 0;

   return transfer_crc;
}

CMSE_API uint32_t cmse_ota_write_packet(uint32_t offset, uint8_t *buf, uint32_t len)
{
    uint32_t total_len = 0;
    uint32_t write_len = 0;
    uint32_t left_len = 0;
    uint32_t raw_len = 0;
    static uint32_t last_offset = 0;

    TRACE(2, "cmse write %d bytes to flash offset 0x%x", len, offset);
    if ((g_total_len + len) > OTA_NEW_IMAGE_SIZE)
    {
        TRACE(0, "%s error: total len %d > %d", __func__, g_total_len, OTA_NEW_IMAGE_SIZE);
        return 0;
    }

    cmse_ota_calculate_crc(offset, buf, len);
    if(last_offset >= offset)
    {
        //4k align
        cmse_ota_write_packet_align(offset);
    }
    last_offset = offset;

    total_len = 0;
    write_len = 0;
    left_len = len;
    raw_len = g_raw_buf_length;
    while (left_len > 0)
    {
        if (raw_len + left_len > FLASH_SECTOR_SIZE_IN_BYTES)
        {
            write_len = FLASH_SECTOR_SIZE_IN_BYTES - raw_len;
            memcpy(&cmse_ota_raw_buffer[raw_len], &buf[total_len], write_len);
            cmse_ota_decypt_and_write_flash(cmse_ota_raw_buffer, FLASH_SECTOR_SIZE_IN_BYTES, false);
        }
        else
        {
            write_len = left_len;
            memcpy(&cmse_ota_raw_buffer[raw_len], &buf[total_len], write_len);
            g_raw_buf_length = raw_len + write_len;
        }
        left_len -= write_len;
        total_len += write_len;
        raw_len = 0;
    }

    return len;
}

CMSE_API uint32_t cmse_ota_apply(uint32_t ns_offset, uint32_t ns_length, uint32_t ns_crc)
{
    uint32_t ret = 0;

    if (ota_check_flag)
    {
        cmse_ota_update_magic_number(NORMAL_BOOT);
        //update boot info
        cmse_ota_update_boot_info(ns_offset, ns_length, ns_crc);
        ret = 0;
    }
    else
    {
        ret = 1;
    }
    return ret;
}

CMSE_API uint32_t cmse_ota_decypt_handler_test(void)
{
    static const char* buf_plain = "1234567887654321";
    // static const uint8_t buf_encryption[] = {0x47,0x42,0x33,0x7D,0x24,0x62,0x86,0x66,0xCB,0x62,0x02,
    //                                         0x4B,0xA1,0x72,0x42,0x56,0x86,0x03,0x31,0x95,0x72,0xF9,0x3D,
    //                                         0xDA,0xF2,0x49,0xAA,0x26,0xD8,0x2E,0x3D,0x2E};

    static const uint8_t buf_encryption[] = {0xa1,0xd4,0x3f,0x62,0xbe,0x6c,0x8e,0x6d,0x16,0xb2,0xfa,
                                            0x93,0x37,0xc4,0x37,0x41,0xc7,0x4f,0x6c,0x60,0x73,0x5D,0x28,
                                            0x05,0x3D,0x3A,0x05,0xAB,0x52,0x0B,0x6B,0xC4};

    uint32_t decrypt_buf_len = 0;
    memset(cmse_ota_decryption_buffer, 0, sizeof(cmse_ota_decryption_buffer));
    cmse_ota_decypt_handler((const char *)buf_encryption, sizeof(buf_encryption), (const char *)cmse_ota_decryption_buffer, &decrypt_buf_len);
    TRACE(0, "DECRYPT:");
    DUMP8("%02x ", cmse_ota_decryption_buffer, decrypt_buf_len);
    TRACE(0, "PLAIN:");
    DUMP8("%02x ", buf_plain, strlen(buf_plain));

    return 0;
}

