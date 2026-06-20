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

#ifndef __OTA_SEC_AES_CRYPT_H__
#define __OTA_SEC_AES_CRYPT_H__

#ifndef MODE_ENCRYPT
#define MODE_ENCRYPT    0
#endif

#ifndef MODE_DECRYPT
#define MODE_DECRYPT    1
#endif

#define AES_CBC_KEY_SIZE    (100)
#define AES_CBC_IV_SIZE     (100)

#if defined(_WIN32)
#define AES_CBC_SRC_BUF_MAX_SIZE    (1024*1024)
#define AES_CBC_DST_BUF_MAX_SIZE    (1024*1024)
#define AES_CBC_CRYPT_OP_ITERATE_SIZE   (1024*1024)  /*pc iterate 10MB*/
#else
#define AES_CBC_SRC_BUF_MAX_SIZE    (1024*4)
#define AES_CBC_DST_BUF_MAX_SIZE    (1024*4)
#define AES_CBC_CRYPT_OP_ITERATE_SIZE   (1024*4)        /*mbed iterate 4KB*/
#endif

#define BIN_AES_CRYPT_HEAP_MAX_SIZE                (AES_CBC_SRC_BUF_MAX_SIZE + AES_CBC_DST_BUF_MAX_SIZE + 2*1024)

#ifdef __cplusplus
extern "C" {
#endif

int bin_aes_crypt_init_if(const char * key, unsigned int key_len , const char * iv, unsigned int iv_len);
void bin_aes_crypt_key_iv_reset(void);
int bin_aes_crypt_deinit_if(void);
int bin_aes_crypt_handler_if(char mode ,const char * in_buf , unsigned int in_buf_len,char * out_buf,unsigned int * out_buf_len);


#ifdef __cplusplus
}
#endif

#endif

