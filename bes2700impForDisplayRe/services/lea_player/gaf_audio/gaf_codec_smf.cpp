/**
 * @file aob_ux_stm.cpp
 * @author BES AI team
 * @version 0.1
 * @date 2020-08-31
 *
 * @copyright Copyright (c) 2015-2021 BES Technic.
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
 */

/*****************************header include********************************/
#include "bluetooth_bt_api.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "app_trace_rx.h"
#include "app_bt_func.h"
#include "app_utils.h"
#include "plat_types.h"
#include "cqueue.h"
#include "heap_api.h"
#include "audio_dump.h"
#include "audioflinger.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_interface.h"
#include "app_audio.h"
#include "app_overlay.h"
#include "list.h"
#include "hal_location.h"

#include "gaf_stream_dbg.h"
#include "gaf_media_pid.h"
#include "gaf_media_stream.h"

#include "smf_codec_lc3.h"
#include "smf_debug.h"
#include "SmfFCC.h"
#include "SmfExitDestroy.h"
#include "SmfHash.h"

#ifdef BLE_SMF
using namespace smf;
/*********************external function declaration*************************/


/************************private macro defination***************************/
#define AOB_CALCULATE_CODEC_MIPS       0
//#define LC3_AUDIO_UPLOAD_DUMP          0
//#define LC3_AUDIO_DOWNLOAD_DUMP        0
//#undef dbg_test_printf
//#define dbg_test_printf(...) 

/************************private type defination****************************/
#if AOB_CALCULATE_CODEC_MIPS
typedef struct
{
    uint32_t start_time_in_ms;
    uint32_t total_time_in_us;
    bool codec_started;
    uint32_t codec_mips;
} aob_codec_time_info_t;
#endif

typedef struct{
    const char* codec;
    GAF_AUDIO_STREAM_COMMON_INFO_T* info;
    void* handle;
    //void* pool_buff;
    int sample_align;
    void* vlc_frame_buff;
    int frame_samples;
    int pcm_frame_size;
    int vlc_frame_size;
    int vlc_frame_max;
} app_aob_smf_t;

/**********************private function declaration*************************/

/************************private variable defination************************/

/****************************function defination****************************/
static void* lc3_alloc(unsigned size){
    uint8_t *data=0;
    app_audio_mempool_get_buff(&data, size);
    dbgTestPXL("%p,%d",data,size);
    return data;
}

static void lc3_free(void* data){
    dbgTestPXL("%p",data);
    return ;
}

static void ctx_init(app_aob_smf_t *ctx){
    auto info = ctx->info;
    ctx->sample_align = info->bits_depth;
    if(info->bits_depth==24)
        ctx->sample_align=32;
    ctx->frame_samples = (int)(info->sample_rate*info->frame_ms/1000);
    if(info->sample_rate==44100)
        ctx->frame_samples = (int)(48000*info->frame_ms/1000);
    ctx->pcm_frame_size = (int)(ctx->frame_samples*info->num_channels*ctx->sample_align/8);
    ctx->vlc_frame_size = (int)(info->encoded_frame_size*info->num_channels);
    ctx->vlc_frame_max = ctx->vlc_frame_size<<1;
    //if(ctx->vlc_frame_max> LC3_MAX_FRAME_SIZE)
    //    ctx->vlc_frame_max= LC3_MAX_FRAME_SIZE;

    dbgLogPPL(ctx->codec);
    dbgLogPSL(ctx->codec);
    dbgLogPDL(ctx->sample_align);
    dbgLogPDL(ctx->frame_samples);
    dbgLogPDL(ctx->vlc_frame_size);
    dbgLogPDL(ctx->vlc_frame_max);
    dbgLogPDL(ctx->pcm_frame_size);
}

static void media_init(app_aob_smf_t *ctx,smf_media_info_t* media){
    dbgTestPPL(ctx);
    auto info = ctx->info;
    //
    media->codec = fcc64(ctx->codec);
    media->audio.sample_rate = info->sample_rate;
    media->audio.channels = info->num_channels;
    media->audio.sample_bits = info->bits_depth;
    media->audio.sample_width = (ctx->sample_align+7)>>3;
    media->frame_dms = (int)(info->frame_ms*10.f);
    media->frame_size = ctx->vlc_frame_size;
    media->audio.frame_samples = ctx->frame_samples;
    //media->is_non_interlace = false;
    
    dbgLogPDL(media->audio.sample_rate);
    dbgLogPDL(media->audio.channels);
    dbgLogPDL(media->audio.sample_bits);
    dbgLogPDL(media->audio.sample_width);
    dbgLogPDL(media->frame_dms);
    dbgLogPDL(media->frame_size);
    
}

static bool media_update(app_aob_smf_t *ctx,smf_media_info_t* media){
    dbgTestPPL(ctx);
    auto hd = ctx->handle;
    memset(media,0,sizeof(smf_media_info_t));
    returnIfErrC(false,!smf_get(hd,SMF_GET_MEDIA_INFO,media));    
    ctx->frame_samples = media->audio.frame_samples;
    ctx->pcm_frame_size = media->audio.frame_samples*media->audio.channels*media->audio.sample_width;
    ctx->vlc_frame_size = media->frame_size;
    return true;
}

static bool decoder_media_init(app_aob_smf_t *ctx,void* param){
    dbgTestPPL(ctx);
    media_init(ctx,(smf_media_info_t*)param);
    
    switch(fcc64(ctx->codec)){
    case fcc64("lc3"):{
        LOG_D("[LC3]decode");
        auto lc3 = (smf_lc3_dec_open_param_t*)param;
        lc3->plcMeth    = SMF_LC3_PLCMODE_ADVANCED;
        lc3->epmode     = SMF_LC3_EPMODE_OFF;
        lc3->hrmode     = false;
        break;
    }
    default:
        dbgErrPXL("unknown codec:%s",ctx->codec);
        break;
    }

    return true;
}

static bool decoder_media_update(app_aob_smf_t *ctx){
    dbgTestPPL(ctx);
    smf_media_info_t media;
    media_update(ctx,&media);
    return true;
}

static void decoder_stream_init(app_aob_smf_t *ctx){
    dbgTestPPL(ctx);
    //overlay load by smf
    //app_overlay_select(ctx->overlay);
    LOG_I("%s", __FUNCTION__);
    //AOB_CALCULATE_CODEC_MIPS
    //memset(&aob_decode_time_info, 0, sizeof(aob_codec_time_info_t));
    //
    if(ctx->handle){
        smf_destroy(ctx->handle);
        ctx->handle = 0;
    }
    //
    auto hd = smf_create_decoder(ctx->codec);
#ifdef GAF_DSP
    if(!smf_set(hd,smf::Hash("Wait"),(void*)FIFO_MODE))
    dbgTestPXL("smf set fifo_mode error");
#endif
    returnIfErrC0(!hd);
    ctx->handle = hd;
    //runtime pool
    smf_register_pool_with_callback(hd, &lc3_alloc, &lc3_free);
    /*int poolsize = 0;
    smf_get(hd,SMF_GET_DYNAMIC_MEMOEY_SIZE_MAX,&poolsize);
    if(poolsize){
        dbgTestPDL(poolsize);
        if(!ctx->pool_buff){
            app_audio_mempool_get_buff((uint8_t**)&ctx->pool_buff, poolsize);
            returnIfErrC0(!ctx->pool_buff);
        }
        dbgTestPPL(ctx->pool_buff);
        if(ctx->pool_buff){
            returnIfErrC0(!smf_register_pool_with_buffer(hd,ctx->pool_buff,poolsize));
        }
    }*/
    //open
    void* param = smf_alloc_open_param(hd);
    ExitFree ep(hd,param);
    returnIfErrC0(!param);
    returnIfErrC0(!decoder_media_init(ctx,param));
    returnIfErrC0(!smf_open(hd,param));
    //update media params
    decoder_media_update(ctx);
    
}

static void decoder_stream_deinit(app_aob_smf_t *ctx){
    dbgTestPPL(ctx);
    void*& hd = ctx->handle;
    if(hd){
        smf_destroy(hd);
        ctx->handle = 0;
    }
}

static void decoder_buf_init(void* _pStreamEnv, uint8_t alg_context_cnt)
{
    //GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    dbgTestPDL(alg_context_cnt);
}

static void decoder_init(void* _pStreamEnv, uint8_t alg_context_cnt)
{
    LOG_D("%s", __func__);
    dbgTestPL();
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    for (uint8_t i = 0; i < alg_context_cnt; i++) {
        app_aob_smf_t* ctx = (app_aob_smf_t*)pStreamEnv->stream_context.codec_alg_context[i].lc3_codec_context.lc3_dec_info;
        memset(ctx,0,sizeof(app_aob_smf_t));
        ctx->codec = pStreamEnv->stream_info.codec;
        ctx->info = &pStreamEnv->stream_info.playbackInfo;
        ctx_init(ctx);
        decoder_stream_init(ctx);
    }
    LOG_D("%s end", __func__);
}

static void decoder_deinit(void)
{
    dbgTestPL();
    app_overlay_unloadall();

    LOG_D("%s", __FUNCTION__);
}

static int decode(bool isPLC, uint32_t inputDataLength, void* input,
    gaf_codec_algorithm_context_t *algo_context, void* output)
{
    app_aob_smf_t* ctx = (app_aob_smf_t*)&algo_context->lc3_codec_context.lc3_dec_info;
    //dbgTestPXL("%d->%d",inputDataLength,ctx->pcm_frame_size);
    void* hd = ctx->handle;
    returnIfErrC(-1,!hd);
    smf_frame_t ifrm;
    memset(&ifrm, 0, sizeof(smf_frame_t));
    ifrm.buff = input;
    ifrm.max = inputDataLength;
    ifrm.offset = 0;
    ifrm.size = inputDataLength;
    ifrm.flags = 0;
    if(isPlc || !inputDataLength || !input){
        ifrm.flags |= SMF_FRAME_IS_FAIL;
    }
    smf_frame_t ofrm;
    memset(&ofrm, 0, sizeof(smf_frame_t));
    ofrm.buff = output;
    ofrm.max = ctx->pcm_frame_size;
    ofrm.offset = 0;
    ofrm.size = 0;
    ofrm.flags = 0;
    if(!smf_decode(hd,&ifrm,&ofrm)){
        dbgErrPPL(ctx->codec);
        dbgErrPSL(ctx->codec);
        dbgErrPPL(ctx->vlc_frame_buff);
        dbgErrPDL(ctx->frame_samples);
        dbgErrPDL(ctx->pcm_frame_size);
        dbgErrPDL(ctx->vlc_frame_size);
        smf_print_error(hd);
        return smf_get_err32(hd);
    }
    return 0;
}

static void decoder_buf_deinit(void* _pStreamEnv, uint8_t alg_context_cnt)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    dbgTestPDL(alg_context_cnt);
    for (uint8_t i = 0; i < alg_context_cnt; i++) {	
        app_aob_smf_t* ctx = (app_aob_smf_t*)pStreamEnv->stream_context.codec_alg_context[i].lc3_codec_context.lc3_dec_info;
        decoder_stream_deinit(ctx);
    }
}

static const GAF_AUDIO_DECODER_FUNC_LIST_T gaf_audio_smf_decoder_func_list =
{
    .decoder_init_buf_func = decoder_buf_init,
    .decoder_init_func = decoder_init,
    .decoder_deinit_func = decoder_deinit,
    .decoder_decode_frame_func = decode,
    .decoder_deinit_buf_func = decoder_buf_deinit,
};

static bool encoder_media_init(app_aob_smf_t *ctx,void* param){
    dbgTestPPL(ctx);
    media_init(ctx,(smf_media_info_t*)param);
        
    switch(fcc64(ctx->codec)){
    case fcc64("lc3"):{
        LOG_I("[lc3]encoder");
        auto lc3 = (smf_lc3_enc_open_param_t*)param;
        lc3->epmode     = SMF_LC3_EPMODE_OFF;
        lc3->hrmode     = false;
        break;
    }
    default:
        dbgErrPXL("unknown codec:%s",ctx->codec);
        break;
    }
    
    return true;
}
static bool encoder_media_update(app_aob_smf_t *ctx){
    dbgTestPPL(ctx);
    smf_media_info_t media;
    media_update(ctx,&media);

    return true;
}

static void encoder_stream_init(app_aob_smf_t *ctx)
{
    dbgTestPPL(ctx);    
    //overlay load by smf
    //app_overlay_select(ctx->overlay);
    LOG_I("%s", __FUNCTION__);
    //AOB_CALCULATE_CODEC_MIPS
    //memset(&aob_decode_time_info, 0, sizeof(aob_codec_time_info_t));
    if(ctx->handle){
        smf_destroy(ctx->handle);
        ctx->handle = 0;
    }

    auto hd = smf_create_encoder(ctx->codec);
#ifdef GAF_DSP
    if(!smf_set(hd,smf::Hash("Wait"),(void*)FIFO_MODE))
    dbgTestPXL("smf set fifo_mode error");
#endif
    returnIfErrC0(!hd);
    ctx->handle = hd;
    //encode vlc buff
    ctx->vlc_frame_buff = lc3_alloc(ctx->vlc_frame_max);
    returnIfErrC0(!ctx->vlc_frame_buff);
    memset(ctx->vlc_frame_buff,0,ctx->vlc_frame_max);
    //runtime pool
    smf_register_pool_with_callback(hd, &lc3_alloc, &lc3_free);
    /*int poolsize = 0;
    smf_get(hd,SMF_GET_DYNAMIC_MEMOEY_SIZE_MAX,&poolsize);
    if(poolsize){
        if(!ctx->pool_buff){
            app_audio_mempool_get_buff((uint8_t**)&ctx->pool_buff, poolsize);
            returnIfErrC0(!ctx->pool_buff);
        }
        dbgTestPPL(ctx->pool_buff);
        if(ctx->pool_buff){
            returnIfErrC0(!smf_register_pool_with_buffer(hd,ctx->pool_buff,poolsize));
        }
    }*/

    //open
    void* param = ctx->vlc_frame_buff;//smf_alloc_open_param(hd);
    //ExitFree ep(hd,param);
    returnIfErrC0(!param);
    returnIfErrC0(!encoder_media_init(ctx,param));
    returnIfErrC0(!smf_open(hd,param));    
    //update media params
    encoder_media_update(ctx);

}

static void encoder_stream_deinit(app_aob_smf_t *ctx)
{
    dbgTestPPL(ctx);
    void*& hd = ctx->handle;
    if(hd){
        smf_destroy(hd);
        ctx->handle = 0;
    }
    if(ctx->vlc_frame_buff){
        lc3_free(ctx->vlc_frame_buff);
        ctx->vlc_frame_buff = 0;
    }
}

static void encoder_buf_init(void* _pStreamEnv)
{
    dbgTestPL();
    //GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    //Capture should always be less than 1

}

static void encoder_init(void* _pStreamEnv)
{
    dbgTestPL();
    //Capture should always be less than 1
    LOG_D("%s", __func__);
    
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    int i=0;
    app_aob_smf_t* ctx = (app_aob_smf_t*)pStreamEnv->stream_context.codec_alg_context[i].lc3_codec_context.lc3_enc_info;
    memset(ctx,0,sizeof(app_aob_smf_t));
    ctx->codec = pStreamEnv->stream_info.codec;
    ctx->info = &pStreamEnv->stream_info.captureInfo;
    ctx_init(ctx);
    encoder_stream_init(ctx);
    LOG_D("%s end", __func__);

}

static void encoder_deinit(void* _pStreamEnv)
{
    LOG_D("%s", __FUNCTION__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    int i = 0;
    app_aob_smf_t* ctx = (app_aob_smf_t*)pStreamEnv->stream_context.codec_alg_context[i].lc3_codec_context.lc3_enc_info;
    encoder_stream_deinit(ctx);
}

static void encode(void* pStreamEnv, uint32_t timeStamp,
    uint32_t inputDataLength, void *input, gaf_codec_algorithm_context_t *algo_context,
    GAF_AUDIO_ENCODER_SEND_FRAME_FUNC cbsend)
{
    returnIfErrC0(!algo_context);
    returnIfErrC0(!input);
    returnIfErrC0(!inputDataLength);
    app_aob_smf_t* ctx = (app_aob_smf_t*)algo_context->lc3_codec_context.lc3_enc_info;    
    ctx->vlc_frame_size = ctx->info->encoded_frame_size*ctx->info->num_channels;    
    //dbgTestPXL("%u->%u[%ums]%ums",inputDataLength,ctx->vlc_frame_size,TICKS_TO_MS(hal_sys_timer_get()),timeStamp/1000);
    void* hd = ctx->handle;
    returnIfErrC0(!hd);

    smf_frame_t ifrm;
    memset(&ifrm, 0, sizeof(smf_frame_t));
    ifrm.buff = input;
    ifrm.max = inputDataLength;
    ifrm.offset = 0;
    ifrm.size = inputDataLength;
    ifrm.flags = 0;//SMF_FRAME_IS_PCM_NONINTERLACE;
    smf_frame_t ofrm;
    memset(&ofrm, 0, sizeof(smf_frame_t));
    ofrm.buff = ctx->vlc_frame_buff;
    ofrm.max = ctx->vlc_frame_size;
    ofrm.offset = 0;
    ofrm.size = 0;
    ofrm.flags = 0;
    //dbgTestDump(input[0],16);
    //dbgTestDump(input[1],16);
    if(!smf_encode(hd,&ifrm,&ofrm)){
        dbgErrPL();
        smf_print_error(hd);
    }
    //
    //dbgTestPXL("%d,%d,%p",inputDataLength,ofrm.size,ofrm.buff);
    if(cbsend){
        cbsend(pStreamEnv,ofrm.buff,ofrm.size,timeStamp);
    }
}

static void encoder_buf_deinit(void* _pStreamEnv)
{
    //GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    //Capture should always be less than 1
}

static const GAF_AUDIO_ENCODER_FUNC_LIST_T gaf_audio_smf_encoder_func_list =
{
    .encoder_init_buf_func = encoder_buf_init,
    .encoder_init_func = encoder_init,
    .encoder_deinit_func = encoder_deinit,
    .encoder_encode_frame_func = encode,
    .encoder_deinit_buf_func = encoder_buf_deinit,
};

//++enable lc3 with smf
EXTERNC void gaf_audio_lc3_update_codec_func_list(void* _pStreamEnv)
{
    LOG_I("[LC3]");
    smf_init();
    smf_lc3_decoder_register();
    smf_lc3_encoder_register();
    dbgTestPL();
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    pStreamEnv->func_list->decoder_func_list = &gaf_audio_smf_decoder_func_list;
    pStreamEnv->func_list->encoder_func_list = &gaf_audio_smf_encoder_func_list;
    pStreamEnv->stream_info.codec = "lc3";
}

EXTERNC int gaf_audio_lc3_encoder_get_max_frame_size(){
    int framesize_max = 870;
    dbgTestPDL(framesize_max);
    return framesize_max;
}
//--enable lc3 with smf


#endif

