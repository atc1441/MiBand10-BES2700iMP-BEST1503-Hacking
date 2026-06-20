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
#if defined(OTA_TZ_ENABLE) && !defined(SPA_AUDIO_SEC)

#include <stdlib.h>
#include <string.h>
#include "hal_trace.h"
#include "hal_timer.h"
#include "string.h"

#include "mbedtls/platform.h"

#include "mbedtls/memory_buffer_alloc.h"
#include "heap_api.h"

#include "mbedtls/aes.h"
#include "mbedtls/platform.h"
#include "mbedtls/platform_util.h"
#include "mbedtls/error.h"

#include "tz_ota_sec_aes_crypt.h"
#include "tz_ota_sec_mem.h"


#if defined(_WIN32)
#define bin_aes_crypt_dbg   printf
#define bin_aes_crypy_dump8(str,len) do{ \
                                        unsigned int block = len / 16; \
                                        unsigned int remain = len % 16; \
                                        unsigned int i =0; \
                                        unsigned int j = 0; \
                                        for(i = 0;i<block;i++) \
                                        {   \
                                            for(j=0;j<16;j++){ \
                                                bin_aes_crypt_dbg("0x%02x ",str[i*16+j]); \
                                            } \
                                            bin_aes_crypt_dbg("\n"); \
                                        } \
                                        for(j=0;j<remain;j++){ \
                                            bin_aes_crypt_dbg("0x%02x ",str[i*16+j]); \
                                        } \
                                         bin_aes_crypt_dbg("\n"); \
                                     }while(0);

#define BIN_AES_CRYPT_ASSERT(cond,str)                         \
    do {                                            \
        if( ! (cond) )                              \
        {                                           \
            bin_aes_crypt_dbg("ASSERT: line %d",__LINE__); \
            bin_aes_crypt_dbg("%s ",str); \
        }                                           \
    } while( 0 );

#else
#define bin_aes_crypt_dbg(str, ...)     TR_DEBUG(TR_MOD(SPA), str, ##__VA_ARGS__)
#define bin_aes_crypy_dump8(str,len)    DUMP8("0x%02x ",str,len)
#define BIN_AES_CRYPT_ASSERT(cond,str,...)  { if (!(cond)) { hal_trace_dummy(str, ##__VA_ARGS__); hal_trace_assert_dump(NULL); } }
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b) ((a)>(b)?(b):(a))
#endif

typedef struct{
    unsigned int key_len;
    unsigned int key_size;
    unsigned int iv_len;
    unsigned int iv_size;
    unsigned int src_buf_len;
    unsigned int src_buf_size;
    unsigned int dst_buf_len;
    unsigned int dst_buf_size;

    unsigned char * key;
    unsigned char * iv;
    unsigned char * src_buf;
    unsigned char * dst_buf;
}aes_cbc_info_t;

static aes_cbc_info_t aes_cbc_instant;
static mbedtls_aes_context ctx;
static char aes_cbc_key[AES_CBC_KEY_SIZE];
static char aes_cbc_iv[AES_CBC_IV_SIZE];

static unsigned int aes_cbc_key_len = 0;
static unsigned int aes_cbc_iv_len = 0;

static void _memcpy(void *dst, const void *src, int size) {
    int i;
    for (i = 0; i < size; i++) {
        ((unsigned char*)dst)[i] = ((unsigned char*)src)[i];
    }
}

static void _memset(void * dst, char val,int size)
{
    int i;
    for (i = 0; i < size; i++) {
        ((unsigned char*)dst)[i] = (unsigned char)val;
    }
}

static int bin_aes_crypt_mem_init(aes_cbc_info_t * cbc_info)
{
    if(cbc_info == NULL){
        return -2;
    }

    _memset((unsigned char *)cbc_info,0,sizeof(aes_cbc_info_t));

    sec_mem_heap_init(BIN_AES_CRYPT_HEAP_MAX_SIZE);

    cbc_info->key = sec_mem_heap_calloc(AES_CBC_KEY_SIZE);
    cbc_info->iv = sec_mem_heap_calloc(AES_CBC_IV_SIZE);
    cbc_info->src_buf = sec_mem_heap_calloc(AES_CBC_SRC_BUF_MAX_SIZE);
    cbc_info->dst_buf = sec_mem_heap_calloc(AES_CBC_DST_BUF_MAX_SIZE);

    cbc_info->key_size = AES_CBC_KEY_SIZE;
    cbc_info->iv_size = AES_CBC_IV_SIZE;
    cbc_info->src_buf_size = AES_CBC_SRC_BUF_MAX_SIZE;
    cbc_info->dst_buf_size = AES_CBC_DST_BUF_MAX_SIZE;

    if(!((cbc_info->key != NULL) &&
        (cbc_info->iv  != NULL) &&
        (cbc_info->src_buf != NULL) &&
        (cbc_info->dst_buf != NULL))){
        return -1;
    }


    return 0;
}

static int bin_aes_crypt_deinit(aes_cbc_info_t * cbc_info)
{
    if(cbc_info != NULL){
        sec_mem_heap_free(cbc_info->key);
        sec_mem_heap_free(cbc_info->iv);
        sec_mem_heap_free(cbc_info->src_buf);
        sec_mem_heap_free(cbc_info->dst_buf);
        _memset((char *)cbc_info,0,sizeof(aes_cbc_info_t));
    }
    return 0;
}

static int bin_aes_crypt_key_iv_fill(const char * key, unsigned int key_len , const char * iv, unsigned int iv_len)
{
    /* fill key text */
    if(key){
        _memcpy(aes_cbc_instant.key,key,key_len);
        aes_cbc_instant.key_len = key_len;
    }

    /* fill iv text */
    if(iv){
        _memcpy(aes_cbc_instant.iv,iv,iv_len);
        aes_cbc_instant.iv_len = iv_len;
    }
    return 0;
}

static int bin_aes_encrypt_handler(aes_cbc_info_t* aes_cbc,const char * in_buf , unsigned int in_buf_len,char * out_buf,
                                            unsigned int * out_buf_len)
{
    bin_aes_crypt_dbg("+++++ bin_aes_encrypt_handler +++++ \n");

//    mbedtls_aes_context ctx;
    int block = 0;
    int ret = 0;
//    unsigned int out_requre_len = *out_buf_len;

    if(!((in_buf != NULL) && 
        (aes_cbc != NULL) &&
        (out_buf != NULL) &&
        (out_buf_len != NULL))){
        BIN_AES_CRYPT_ASSERT(0,"bin_aes_encrypt_handler");
        return -1;
    }

    block = in_buf_len %16;
    if(block){
        in_buf_len += 16 - block;
    }

    aes_cbc->src_buf_len = in_buf_len;
    _memcpy(aes_cbc->src_buf,in_buf,in_buf_len);

//    mbedtls_aes_init( &ctx );

    /*plain text*/
//    bin_aes_crypt_dbg("++ plain text ++ \n");
//    bin_aes_crypt_dbg("%s \n",aes_cbc->src_buf);

    /* encrypt */
    bin_aes_crypt_dbg("++ start encrypt ++ \n");

    mbedtls_aes_setkey_enc( &ctx, (const unsigned char *)(aes_cbc->key), aes_cbc->key_len  * 8 );

    ret = mbedtls_aes_crypt_cbc( &ctx, MBEDTLS_AES_ENCRYPT, aes_cbc->src_buf_len, aes_cbc->iv, aes_cbc->src_buf, aes_cbc->dst_buf);

    if(ret != 0){
        bin_aes_crypt_dbg("mbedtls_aes_crypt_cbc MBEDTLS_AES_ENCRYPT fail %d \n",ret);
        return -1;
    }

    aes_cbc->dst_buf_len = in_buf_len;

    *out_buf_len = aes_cbc->dst_buf_len;

//    bin_aes_crypt_dbg("%s \n",aes_cbc->dst_buf);
//    bin_aes_crypt_dbg("in len %d out len %d wants : %d \n",in_buf_len,*out_buf_len,out_requre_len);

    _memcpy(out_buf,aes_cbc->dst_buf,aes_cbc->dst_buf_len);

//    bin_aes_crypy_dump8(aes_cbc->dst_buf,*out_buf_len);

    bin_aes_crypt_dbg("----- bin_aes_encrypt_handler end ----- \n");

//    mbedtls_aes_free( &ctx );
    return 0;
}

static int bin_aes_decrypt_handler(aes_cbc_info_t* aes_cbc,const char * in_buf , unsigned int in_buf_len,char * out_buf,
                                            unsigned int * out_buf_len)
{
    bin_aes_crypt_dbg("+++++ bin_aes_decrypt_handler +++++ \n");

//    mbedtls_aes_context ctx;
    int ret = 0;
//    unsigned int out_requre_len = *out_buf_len;

    if(!((in_buf != NULL) && 
        (aes_cbc != NULL) &&
        (out_buf != NULL) &&
        (out_buf_len != NULL))){
        BIN_AES_CRYPT_ASSERT(0,"bin_aes_decrypt_handler");
        return -1;
    }

    aes_cbc->src_buf_len = in_buf_len;
    _memcpy(aes_cbc->src_buf,in_buf,in_buf_len);

//    mbedtls_aes_init( &ctx );

    /* decrypt */
    bin_aes_crypt_dbg("++ start decrypt ++ \n");
//    bin_aes_crypt_dbg("%s \n",aes_cbc->src_buf);

//    bin_aes_crypy_dump8(aes_cbc->src_buf,in_buf_len);


    mbedtls_aes_setkey_dec( &ctx, (const unsigned char *)(aes_cbc->key), aes_cbc->key_len * 8 );

    ret = mbedtls_aes_crypt_cbc( &ctx, MBEDTLS_AES_DECRYPT, aes_cbc->src_buf_len, aes_cbc->iv, aes_cbc->src_buf, aes_cbc->dst_buf);

    if(ret != 0){
        bin_aes_crypt_dbg("mbedtls_aes_crypt_cbc MBEDTLS_AES_DECRYPT fail %d \n",ret);
        return -1;
    }

//    bin_aes_crypt_dbg("%s \n",aes_cbc->dst_buf);

    aes_cbc->dst_buf_len = in_buf_len;

    *out_buf_len = aes_cbc->dst_buf_len;

//    bin_aes_crypt_dbg("in len %d out len %d wants : %d \n",in_buf_len,*out_buf_len,out_requre_len);

    _memcpy(out_buf,aes_cbc->dst_buf,aes_cbc->dst_buf_len);

//    bin_aes_crypy_dump8(demo_dec_buf,aes_cbc_instant.src_buf_len);

    bin_aes_crypt_dbg("----- bin_aes_decrypt_handler end ----- \n");

//    mbedtls_aes_free( &ctx );

    return 0;

}

#define DEMO_AES_CBC_ENC_BUF_MAX_SIZE   (1024*4)
#define DEMO_AES_CBC_DEC_BUF_MAX_SIZE   (1024*4)

static const char * POSSIBLY_UNUSED demo_aes_cbc_plain = "1234567890_hello_world";
static const char * POSSIBLY_UNUSED demo_aes_cbc_passphrase = "drjom_MOJRD";
static const char * POSSIBLY_UNUSED demo_aes_cbc_salt = "84BF10B7760E4034";
static const char * POSSIBLY_UNUSED demo_aes_cbc_key = "1F8199004CB7FBAF070C4864495399A0";
static const char * POSSIBLY_UNUSED demo_aes_cbc_iv = "3FA8DC149F54D369F71E7386BD37EE50";
static const char * POSSIBLY_UNUSED demo_aes_cbc_enc = "笅綂?>箃[会鼦棝S?1鴹匏??LESW9犞?:vV?环諑|?瞱??をP<G?:0d膄!鯾F";

static char POSSIBLY_UNUSED demo_enc_buf[DEMO_AES_CBC_ENC_BUF_MAX_SIZE] = {0};
static char POSSIBLY_UNUSED demo_dec_buf[DEMO_AES_CBC_DEC_BUF_MAX_SIZE] = {0};

int demo_bin_aes_crypt_main(void)
{
    bin_aes_crypt_dbg("++++++++ %s ++++++++ \n",__func__);

    int ret = 0;
    unsigned int enc_cipher_len = 0;
    unsigned int dec_plain_len = 0;

    sec_mem_total_init();

    ret = bin_aes_crypt_mem_init(&aes_cbc_instant);

    if(ret != 0){
        bin_aes_crypt_dbg("bin_aes_crypt_mem_init fail errno = %d \n",ret);
        return -1;
    }

    mbedtls_aes_init( &ctx );

    /*fill the key & iv*/
    bin_aes_crypt_key_iv_fill(demo_aes_cbc_key,strlen(demo_aes_cbc_key),demo_aes_cbc_iv,strlen(demo_aes_cbc_iv));

    /* encrypt*/
    bin_aes_encrypt_handler(&aes_cbc_instant,demo_aes_cbc_plain,strlen(demo_aes_cbc_plain),demo_enc_buf,&enc_cipher_len);

    /*fill the key & iv*/
    bin_aes_crypt_key_iv_fill(demo_aes_cbc_key,strlen(demo_aes_cbc_key),demo_aes_cbc_iv,strlen(demo_aes_cbc_iv));

    /* deccrypt*/
    bin_aes_decrypt_handler(&aes_cbc_instant,demo_enc_buf,enc_cipher_len,demo_dec_buf,&dec_plain_len);

    demo_dec_buf[dec_plain_len] = '\0';

    ret = strcmp( (char *) demo_dec_buf, demo_aes_cbc_plain );

    if(ret){
        bin_aes_crypt_dbg("%s idx = %d \n",demo_aes_cbc_plain,ret);
    }

    mbedtls_aes_free( &ctx );

    bin_aes_crypt_deinit(&aes_cbc_instant);

    bin_aes_crypt_dbg("-------- %s end -------- \n",__func__);

    return 0;
}

#if defined(_WIN32)
static int bin_aes_file_op_open_param_read(FILE** file,char * dir)
{
    if ((*file = fopen(dir, "rb")) == NULL)
    {
        bin_aes_crypt_dbg("%s fal! \n",__func__);
        return(-1);
    }

    return 0;
}

static int bin_aes_file_op_open_param_write(FILE** file,char * dir)
{
    if ((*file = fopen(dir, "wb")) == NULL)
    {
        bin_aes_crypt_dbg("%s fal! \n",__func__);
        return(-1);
    }

    return 0;
}

static int bin_aes_file_op_get_file_len(FILE* file , unsigned int *file_len)
{
    fseek(file, 0, SEEK_END);
    if ((*file_len = ftell(file)) == -1)
    {
        fclose(file);
        file = NULL;
        bin_aes_crypt_dbg("%s fail! \n",__func__);
        return(-1);
    }
    fseek(file, 0, SEEK_SET);

    return 0;
}

static int bin_aes_file_op_read_out(FILE* file,char * buf,unsigned int buf_len)
{
    if (fread(buf, 1, buf_len, file) != buf_len)
    {
        bin_aes_crypt_dbg("\n  file read fail line %d! \n",__LINE__);
        free(buf);
        fclose(file);
        buf = NULL;
        file = NULL;
        return -1;
    }
//    fseek(file, buf_len, SEEK_CUR);

    return 0;
}

static int bin_aes_file_op_read_begin(FILE* file)
{
    fseek(file, 0, SEEK_SET);
    return 0;
}

static int bin_aes_file_op_write_append(FILE* file,char * buf,unsigned int buf_len)
{
    if(fwrite(buf,1,buf_len,file) != buf_len){
        bin_aes_crypt_dbg("\n  file write fail line %d! \n",__LINE__);
        free(buf);
        fclose(file);
        buf = NULL;
        file = NULL;
        return -1;
    }

    return 0;
}

static int bin_aes_file_op_close(FILE* file)
{
    fclose(file);
    file = NULL;
    return 0;
}
#endif

static int bin_aes_crypt_key_iv_input(const char * key, unsigned int key_len , const char * iv, unsigned int iv_len)
{
    /* fill key text */
    if(key){
        if( key_len > (int) sizeof( aes_cbc_key) )
            key_len = (int) sizeof( aes_cbc_key );
        
        _memcpy( aes_cbc_key, key, key_len );
        aes_cbc_key_len = key_len;
    }

    /* fill iv text */
    if(iv){
        if( iv_len > (int) sizeof( aes_cbc_iv) )
            iv_len = (int) sizeof( aes_cbc_iv );
        
        _memcpy( aes_cbc_iv, iv, iv_len );
        aes_cbc_iv_len = iv_len;
    }
    return 0;
}

int bin_aes_crypt_init_if(const char * key, unsigned int key_len , const char * iv, unsigned int iv_len)
{
    int ret  = 0;
    bin_aes_crypt_key_iv_input(key,key_len,iv,iv_len);


    ret = bin_aes_crypt_mem_init(&aes_cbc_instant);

    if(ret != 0){
        bin_aes_crypt_dbg("bin_aes_crypt_mem_init fail errno = %d \n",ret);
        return -1;
    }

    mbedtls_aes_init( &ctx );

    /*fill plain text*/
    bin_aes_crypt_key_iv_fill((const char *)aes_cbc_key,aes_cbc_key_len,(const char *)aes_cbc_iv,aes_cbc_iv_len);

//    bin_aes_crypt_dbg("key len %d  %s \n",aes_cbc_key_len,aes_cbc_key);
//    bin_aes_crypt_dbg("iv  len %d %s \n",aes_cbc_iv_len,aes_cbc_iv);

    return 0;
}

void bin_aes_crypt_key_iv_reset(void)
{
    /*fill plain text*/
    bin_aes_crypt_key_iv_fill((const char *)aes_cbc_key,aes_cbc_key_len,(const char *)aes_cbc_iv,aes_cbc_iv_len);
}

int bin_aes_crypt_deinit_if(void)
{
    mbedtls_aes_free( &ctx );

    bin_aes_crypt_deinit(&aes_cbc_instant);

    return 0;
}

int bin_aes_crypt_handler_if(char mode ,const char * in_buf , unsigned int in_buf_len,char * out_buf,
                                            unsigned int * out_buf_len)
{
    if(mode == MODE_ENCRYPT){
        bin_aes_encrypt_handler(&aes_cbc_instant,in_buf,in_buf_len,out_buf,out_buf_len);
    }else if(mode == MODE_DECRYPT){
        bin_aes_decrypt_handler(&aes_cbc_instant,in_buf,in_buf_len,out_buf,out_buf_len);
    }else{
        BIN_AES_CRYPT_ASSERT(0,"bin_aes_crypt_main fail \n");
        return -1;
    }

    return 0;
}

int bin_aes_crypt_main(int argc, char* argv[])
{
    int ret = 0;

#if !defined(_WIN32)
    demo_bin_aes_crypt_main();
#else
    unsigned int i = 0;
    unsigned int crypt_iter_op_size = 0;
    unsigned int crypt_iter_op_remain = 0;
    int keylen = 0;
    int ivlen = 0;
    int mode = 0;

    unsigned int infile_len = 0;
    unsigned int outfile_len = 0;

    char * file_in_dir = NULL;
    char * file_out_dir = NULL;

    FILE* f_in = NULL;
    FILE* f_out = NULL;

    unsigned char * read_buf = NULL;
    unsigned char * write_buf = NULL;

    unsigned char * aes_cbc_crypt_buf = NULL;
    unsigned int crypt_iter_op_in_len = 0;
    unsigned int crypt_iter_op_out_len = 0;

    /*Start of Fetch input parameters*/

    if(argc != 6){
        bin_aes_crypt_dbg("usage: \n");
        bin_aes_crypt_dbg("bin_aes_crypt_main.exe <mode> 0:encrypt 1:decrypt  <in_file> <key> <iv> <out_file> \n");
        return -1;
    }

    mode = atoi( argv[1] );

    bin_aes_crypt_dbg("mode = %d \n",mode);

    /*get input file dir*/
    file_in_dir = argv[2];

    bin_aes_crypt_dbg("file_ind_dir = %s \n",file_in_dir);


    /*get key & iv */
    keylen = strlen( argv[3] );
    ivlen = strlen( argv[4] );

    sec_mem_total_init();

    bin_aes_crypt_init_if(argv[3],keylen,argv[4],ivlen);

    /*get output file dir*/
    file_out_dir = argv[5];

    /*End of Fetch input parameters*/

    bin_aes_file_op_open_param_read(&f_in,file_in_dir);

    bin_aes_file_op_open_param_write(&f_out,file_out_dir);

    bin_aes_file_op_get_file_len(f_in,&infile_len);

    read_buf = sec_mem_heap_calloc(AES_CBC_CRYPT_OP_ITERATE_SIZE);
    aes_cbc_crypt_buf = sec_mem_heap_calloc(AES_CBC_CRYPT_OP_ITERATE_SIZE);
    bin_aes_file_op_read_begin(f_in);


    crypt_iter_op_size = (infile_len > AES_CBC_CRYPT_OP_ITERATE_SIZE)?(AES_CBC_CRYPT_OP_ITERATE_SIZE):(infile_len);
    crypt_iter_op_remain = infile_len;

    for(i = 0;i < infile_len; i += crypt_iter_op_in_len){
        crypt_iter_op_out_len = 0;

        crypt_iter_op_in_len = crypt_iter_op_size;

        if(crypt_iter_op_remain < AES_CBC_CRYPT_OP_ITERATE_SIZE){
            crypt_iter_op_in_len = crypt_iter_op_remain;
        }

        bin_aes_crypt_dbg(" in_len %d  remain %d iter_len %d \n",crypt_iter_op_in_len,crypt_iter_op_remain,AES_CBC_CRYPT_OP_ITERATE_SIZE);

        bin_aes_file_op_read_out(f_in,read_buf,crypt_iter_op_in_len);
        bin_aes_crypy_dump8(read_buf,16);

        bin_aes_crypt_handler_if(mode,read_buf,crypt_iter_op_in_len,aes_cbc_crypt_buf,&crypt_iter_op_out_len);
        bin_aes_file_op_write_append(f_out,aes_cbc_crypt_buf,crypt_iter_op_out_len);

        crypt_iter_op_remain -= crypt_iter_op_in_len;
    }

    bin_aes_crypt_deinit_if();

    sec_mem_heap_free(read_buf);

    sec_mem_heap_free(aes_cbc_crypt_buf);

    bin_aes_file_op_close(f_out);

    bin_aes_file_op_close(f_in);

#endif

    return(ret);
}

#endif
