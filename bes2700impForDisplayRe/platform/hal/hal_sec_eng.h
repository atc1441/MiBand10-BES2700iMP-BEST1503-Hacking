/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef __HAL_SEC_ENG_H__
#define __HAL_SEC_ENG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "stdbool.h"
#include "stdint.h"
#include "hal_dma.h"

enum HAL_SE_RET_T {
    HAL_SE_OK,
    HAL_SE_ERR,
    HAL_SE_ALREADY_OPENED,
    HAL_SE_NOT_OPENED,
    HAL_SE_ENG_BUSY,
    HAL_SE_DMA_BUSY,
    HAL_SE_CFG_NULL,
    HAL_SE_INPUT_NULL,
    HAL_SE_OUTPUT_NULL,
    HAL_SE_KEY_NULL,
    HAL_SE_IV_NULL,
    HAL_SE_KEY2_NULL,
    HAL_SE_BAD_INPUT_LEN,
    HAL_SE_BAD_OUTPUT_LEN,
    HAL_SE_BAD_KEY_LEN,
    HAL_SE_BAD_AES_MODE,
    HAL_SE_BAD_CBC_CS,
    HAL_SE_BAD_AES_MODULAR,
    HAL_SE_BAD_DONE_HDLR,
    HAL_SE_BAD_MODE,
    HAL_SE_BAD_OP,
    HAL_SE_BAD_SEG_OPER,
    HAL_SE_BAD_SEG_CTX_BUF,
    HAL_SE_BAD_SEG_CTX_LEN,
};

enum HAL_SE_AES_MODE_T {
    HAL_SE_AES_ECB                      = 0,
    HAL_SE_AES_CBC                      = 1,
    HAL_SE_AES_CTR                      = 2,
    HAL_SE_AES_XTS                      = 3,
    HAL_SE_AES_KEY_WRAP                 = 4,

    HAL_SE_AES_MODE_QTY,
};

enum HAL_SE_CBC_CS_MODE_T {
    HAL_SE_CBC_CS_NONE                  = 0,
    HAL_SE_CBC_CS1,
    HAL_SE_CBC_CS2,

    HAL_SE_CBC_CS_QTY,
};

enum HAL_SE_SEG_OPER_T {
    HAL_SE_SEG_NONE                     = 0,
    HAL_SE_SEG_INIT,
    HAL_SE_SEG_CONT,
    HAL_SE_SEG_TERM,

    HAL_SE_SEG_QTY,
};

enum HAL_SE_DONE_ERR_T {
    HAL_SE_DONE_OK                      = 0,
    HAL_SE_DONE_ERR_DMA_IN,
    HAL_SE_DONE_ERR_DMA_OUT,
    HAL_SE_DONE_ERR_DMA_IN_REMAIN,
    HAL_SE_DONE_ERR_DMA_OUT_REMAIN,
    HAL_SE_DONE_ERR_ENG_ERR1,
    HAL_SE_DONE_ERR_ENG_ERR2,
    HAL_SE_DONE_ERR_ENG_ERR1_ERR2,
    HAL_SE_DONE_ERR_GET_DIGEST,
};

enum HAL_SE_HASH_MODE_T {
    HAL_SE_HASH_SHA1                    = 0,
    HAL_SE_HASH_SHA256,
    HAL_SE_HASH_SHA224,
    HAL_SE_HASH_MD5,
    HAL_SE_HASH_SHA512,
    HAL_SE_HASH_SHA384,

    HAL_SE_HASH_QTY,
};

typedef void (*HAL_SE_DONE_HANDLER_T)(void *buf, uint32_t len, enum HAL_SE_DONE_ERR_T err);

struct HAL_SE_AES_CFG_T {
    enum HAL_SE_AES_MODE_T mode;
    enum HAL_SE_CBC_CS_MODE_T cbc_cs;
    enum HAL_SE_SEG_OPER_T seg_oper;
    const void *key;
    uint16_t key_len;
    uint8_t ctr_modular;
    const void *key2;
    const void *iv;
    const void *in;
    uint32_t in_len;
    void *out;
    uint32_t out_len;
    void *seg_ctx_buf;
    uint32_t seg_ctx_len;
    HAL_SE_DONE_HANDLER_T done_hdlr;
};

struct HAL_SE_HASH_CFG_T {
    const void *in;
    uint32_t in_len;
    HAL_SE_DONE_HANDLER_T done_hdlr;
    enum HAL_SE_SEG_OPER_T seg_oper;
    void *seg_ctx_buf;
    uint32_t seg_ctx_len;
};

enum HAL_SE_RET_T hal_se_open(void);

enum HAL_SE_RET_T hal_se_close(void);

enum HAL_SE_RET_T hal_se_aes_encrypt(const struct HAL_SE_AES_CFG_T *cfg);

enum HAL_SE_RET_T hal_se_aes_decrypt(const struct HAL_SE_AES_CFG_T *cfg);

enum HAL_SE_RET_T hal_se_aes_save_seg_ctx(void *seg_ctx_buf, uint32_t seg_ctx_len);

int hal_se_aes_busy(void);

enum HAL_SE_RET_T hal_se_aes_reset(void);

uint32_t hal_se_hash_mode_ctx_len(enum HAL_SE_HASH_MODE_T mode);

uint32_t hal_se_hash_mode_digest_len(enum HAL_SE_HASH_MODE_T mode);

enum HAL_SE_RET_T hal_se_hash_init(enum HAL_SE_HASH_MODE_T mode);

enum HAL_SE_RET_T hal_se_hash_update(const struct HAL_SE_HASH_CFG_T *cfg);

enum HAL_SE_RET_T hal_se_hash_final(const struct HAL_SE_HASH_CFG_T *cfg, uint64_t total_in_len);

enum HAL_SE_RET_T hal_se_hash_save_seg_ctx(void *seg_ctx_buf, uint32_t seg_ctx_len);

enum HAL_SE_RET_T hal_se_hash(enum HAL_SE_HASH_MODE_T mode, const struct HAL_SE_HASH_CFG_T *cfg);

enum HAL_SE_RET_T hal_se_hash_get_digest(void *out, uint32_t out_len, uint32_t *real_len);

int hal_se_hash_busy(void);

enum HAL_SE_RET_T hal_se_hash_reset(void);

enum HAL_SE_RET_T hal_se_hmac_init(enum HAL_SE_HASH_MODE_T mode, const void *key, uint32_t key_len, const struct HAL_SE_HASH_CFG_T *cfg);

enum HAL_SE_RET_T hal_se_hmac_update(const struct HAL_SE_HASH_CFG_T *cfg);

enum HAL_SE_RET_T hal_se_hmac_final(const struct HAL_SE_HASH_CFG_T *cfg, uint64_t total_in_len);

enum HAL_SE_RET_T hal_se_hmac_save_seg_ctx(void *seg_ctx_buf, uint32_t seg_ctx_len);

enum HAL_SE_RET_T hal_se_hmac(enum HAL_SE_HASH_MODE_T mode, const void *key, uint32_t key_len, const struct HAL_SE_HASH_CFG_T *cfg);

enum HAL_SE_RET_T hal_se_hmac_get_digest(void *out, uint32_t out_len, uint32_t *real_len);

int hal_se_hmac_busy(void);

enum HAL_SE_RET_T hal_se_hmac_reset(void);

// ------------------------------------------------------------------------------
// ROM Functions
// ------------------------------------------------------------------------------

enum HAL_SE_RET_T hal_se_rom_open(void);

enum HAL_SE_RET_T hal_se_rom_close(void);

int hal_se_hash_rom_done(void);

int hal_se_hash_rom_busy(void);

int hal_se_hash_dma_rom_busy(void);

enum HAL_SE_RET_T hal_se_hash_rom_init(enum HAL_SE_HASH_MODE_T mode);

enum HAL_SE_RET_T hal_se_hash_rom_update(const struct HAL_SE_HASH_CFG_T *cfg);

enum HAL_SE_RET_T hal_se_hash_rom_final(const struct HAL_SE_HASH_CFG_T *cfg, uint64_t total_in_len);

enum HAL_SE_RET_T hal_se_hash_rom_get_digest(void *out, uint32_t out_len, uint32_t *real_len);

enum HAL_SE_RET_T hal_se_hash_rom(enum HAL_SE_HASH_MODE_T mode, const struct HAL_SE_HASH_CFG_T *cfg);

#ifdef __cplusplus
}
#endif

#endif

