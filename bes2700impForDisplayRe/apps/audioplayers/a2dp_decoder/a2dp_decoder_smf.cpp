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
 * porting from a2dp_decoder_aac_lc.cpp
 ****************************************************************************/
// Standard C Included Files
#include "cmsis.h"
#include "plat_types.h"
#include <string.h>
#include "heap_api.h"
#include "hal_location.h"
#include "hal_timer.h"
#include "a2dp_decoder_internal.h"
#include "cmsis_os.h"
#ifdef IBRT
#include "app_tws_ibrt_audio_analysis.h"
#endif
#include "smf_api.h"
#include "smf_debug.h"
#include <SmfFCC.h>
#include <SmfExitDestroy.h>
#include "smf_codec_sbc.h"
#include "smf_codec_aac.h"

using namespace smf;

static void* _smf = 0;
static smf_frame_t _output_frame={0};

typedef struct {
    uint16_t sequenceNumber;
    uint32_t timestamp;
    uint8_t *smf_buffer;
    uint32_t smf_buffer_len;
} a2dp_audio_smf_decoder_frame_t;

int a2dp_audio_smf_reorder_init(void);
int a2dp_audio_smf_reorder_deinit(void);

static A2DP_AUDIO_CONTEXT_T *a2dp_audio_context_p = NULL;

static A2DP_AUDIO_DECODER_LASTFRAME_INFO_T a2dp_audio_smf_lastframe_info;

static uint16_t smf_mtu_limiter = 0;

static a2dp_audio_smf_decoder_frame_t *smf_decoder_frame_reorder_p = NULL;
static a2dp_audio_smf_decoder_frame_t smf_decoder_last_valid_frame = {0,};
static bool smf_decoder_last_valid_frame_ready = false;

//////////////////////
static inline const char* a2dp_audio_smf_get_codec(){
    return a2dp_audio_context_p? (const char*)&a2dp_audio_context_p->audio_decoder.audio_decoder_fmt.codec: ""; 
}
static inline uint64_t a2dp_audio_smf_get_codec64(){
    return a2dp_audio_context_p?a2dp_audio_context_p->audio_decoder.audio_decoder_fmt.codec:0ull;
}

static inline int a2dp_audio_smf_get_list_max(){
    return a2dp_audio_context_p?a2dp_audio_context_p->audio_decoder.audio_decoder_fmt.frame_list_max:0; 
}

static inline int a2dp_audio_smf_get_samples(){
    return a2dp_audio_context_p?a2dp_audio_context_p->audio_decoder.audio_decoder_fmt.frame_samples:0; 
}

static inline int a2dp_audio_smf_get_frame_max(){
    return a2dp_audio_context_p?a2dp_audio_context_p->audio_decoder.audio_decoder_fmt.frame_bytes_max:0; 
}

static inline int a2dp_audio_smf_get_package(){
    return a2dp_audio_context_p?a2dp_audio_context_p->audio_decoder.audio_decoder_fmt.frame_package:0; 
}

static void *a2dp_audio_smf_frame_malloc(uint32_t packet_len)
{
    a2dp_audio_smf_decoder_frame_t *smf_decoder_frame_p = NULL;
    uint8_t *smf_buffer = NULL;

    smf_buffer = (uint8_t *)a2dp_audio_heap_malloc(a2dp_audio_smf_get_frame_max());
    smf_decoder_frame_p = (a2dp_audio_smf_decoder_frame_t *)a2dp_audio_heap_malloc(sizeof(a2dp_audio_smf_decoder_frame_t));
    smf_decoder_frame_p->smf_buffer = smf_buffer;
    smf_decoder_frame_p->smf_buffer_len = packet_len;
    return (void *)smf_decoder_frame_p;
}

static void a2dp_audio_smf_free(void *packet)
{
    if(packet){
        a2dp_audio_smf_decoder_frame_t *smf_decoder_frame_p = (a2dp_audio_smf_decoder_frame_t *)packet;
        a2dp_audio_heap_free(smf_decoder_frame_p->smf_buffer);
        a2dp_audio_heap_free(smf_decoder_frame_p);
    }
}

static void a2dp_audio_smf_decoder_init(void)
{
    dbgTestPXL("%s",a2dp_audio_smf_get_codec());
    auto context = a2dp_audio_context_p;    
    returnIfErrC0(!context);
    auto dec = _smf;
    if (dec == NULL){
        auto codec = a2dp_audio_smf_get_codec();
        dec = smf_create("dec",codec);
        returnIfErrC0(!dec);
        ExitDestroy eddec(dec);
        //
        returnIfErrC0(!smf_register_pool_with_callback(dec, a2dp_audio_heap_malloc, a2dp_audio_heap_free));
        //fill open params
        auto param = (smf_media_info_t*) smf_alloc_open_param(dec);
        returnIfErrC0(!param);
        ExitFree efparam(dec,param);
        param->package = a2dp_audio_smf_get_package();
        dbgTestPDL(param->package);
        dbgTestPSL(a2dp_audio_smf_get_codec());
        switch(a2dp_audio_smf_get_codec64()){
        case fcc64("aac"):
            break;
        case fcc64("sbc"):
            break;
        default:
            dbgErrPXL("unsupported codec:%s",codec);
            break;
        }
        returnIfErrC0(!smf_open(dec,param));
        //
        _smf = dec;
        eddec.Reset();
    }
}

static void a2dp_audio_smf_decoder_deinit(void)
{
    auto dec = _smf;
    if (_output_frame.buff != NULL){
        a2dp_audio_heap_free(_output_frame.buff);
        _output_frame.buff = NULL;
    }
    if(dec){
        smf_destroy(dec);
        _smf = 0;
    }
}

static void a2dp_audio_smf_decoder_reinit(void)
{
    auto dec = _smf;
    if(dec){
        a2dp_audio_smf_decoder_deinit();
    }
    a2dp_audio_smf_decoder_init();
}

static int a2dp_audio_smf_list_checker(void)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    a2dp_audio_smf_decoder_frame_t *smf_decoder_frame_p = NULL;
    int cnt = 0;

    do {
        smf_decoder_frame_p = (a2dp_audio_smf_decoder_frame_t *)a2dp_audio_smf_frame_malloc(0);
        if (smf_decoder_frame_p){
            a2dp_audio_list_append(list, smf_decoder_frame_p);
        }
        cnt++;
    }while(smf_decoder_frame_p && cnt < smf_mtu_limiter);

    do {
        if ((node = a2dp_audio_list_begin(list)) != NULL){
            smf_decoder_frame_p = (a2dp_audio_smf_decoder_frame_t *)a2dp_audio_list_node(node);
            a2dp_audio_list_remove(list, smf_decoder_frame_p);
        }
    }while(node);

    TRACE_A2DP_DECODER_I("[%s][INIT] cnt:%d list:%d", a2dp_audio_smf_get_codec(), cnt, a2dp_audio_list_length(list));

    return 0;
}

int a2dp_audio_smf_channel_select(A2DP_AUDIO_CHANNEL_SELECT_E chnl_sel)
{
    auto smf_decoder_channel_select = SMF_CHANNEL_STEREO;
    switch(chnl_sel)
    {
        case A2DP_AUDIO_CHANNEL_SELECT_STEREO:
            smf_decoder_channel_select = SMF_CHANNEL_STEREO;
            break;
        case A2DP_AUDIO_CHANNEL_SELECT_LRMERGE:
            smf_decoder_channel_select = SMF_CHANNEL_LRMERGE;
            break;
        case A2DP_AUDIO_CHANNEL_SELECT_LCHNL:
            smf_decoder_channel_select = SMF_CHANNEL_L;
           break;
        case A2DP_AUDIO_CHANNEL_SELECT_RCHNL:
            smf_decoder_channel_select = SMF_CHANNEL_R;
        default:
            break;
    }
    //aacDecoder_DecodeFrame_Config(smf_decoder_channel_select);
    auto dec = _smf;
    if(dec){
        smf_set(dec,SMF_SET_CHANNEL_SELECT, (void*)smf_decoder_channel_select);
    }
    return 0;
}

int a2dp_audio_smf_init(A2DP_AUDIO_OUTPUT_CONFIG_T *config, void *context)
{
    dbgTestPXL("[%s] init", a2dp_audio_smf_get_codec());
    smf_sbc_decoder_register();
    smf_aac_decoder_register();

    a2dp_audio_context_p = (A2DP_AUDIO_CONTEXT_T *)context;
    smf_mtu_limiter = a2dp_audio_smf_get_list_max();//a2dp_audio_context_p->audio_decoder.audio_decoder_fmt.mtu_max;
    dbgTestPDL(smf_mtu_limiter);

    memset(&a2dp_audio_smf_lastframe_info, 0, sizeof(A2DP_AUDIO_DECODER_LASTFRAME_INFO_T));
    a2dp_audio_smf_lastframe_info.stream_info = *config;
    a2dp_audio_smf_lastframe_info.frame_samples = a2dp_audio_smf_get_samples();// AAC_OUTPUT_FRAME_SAMPLES;
    a2dp_audio_smf_lastframe_info.list_samples = a2dp_audio_smf_get_samples();//AAC_OUTPUT_FRAME_SAMPLES;
    a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_smf_lastframe_info);

    ASSERT_A2DP_DECODER(a2dp_audio_context_p->dest_packet_mut < smf_mtu_limiter, "%s MTU OVERFLOW:%u/%u", __func__, a2dp_audio_context_p->dest_packet_mut, smf_mtu_limiter);

    smf_init();
    //a2dp_audio_smf_decoder_init();
    a2dp_audio_smf_decoder_reinit();
    a2dp_audio_smf_reorder_init();
    a2dp_audio_smf_list_checker();

    a2dp_audio_smf_channel_select(a2dp_audio_context_p->chnl_sel);

    return A2DP_DECODER_NO_ERROR;
}


int a2dp_audio_smf_deinit(void)
{
    a2dp_audio_smf_decoder_deinit();
    a2dp_audio_smf_reorder_deinit();

    //size_t total = 0, used = 0, max_used = 0;

    //heap_memory_info(smf_memhandle, &total, &used, &max_used);
    //TRACE_A2DP_DECODER_I("[AAC] deinit MEM: total - %d, used - %d, max_used - %d.",
    //    total, used, max_used);
    smf_pool_print();

    return A2DP_DECODER_NO_ERROR;
}
#if 1
static uint32_t _output_frame_stamp = 0;
static uint32_t _output_frame_index = 0;

static bool a2dp_audio_smf_mcu_decode_one_frame(int buffer_bytes){
    returnIfErrC(false,!a2dp_audio_context_p);
    //
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    returnIfErrC(false,!list);
    //
    list_node_t *node = a2dp_audio_list_begin(list);
    returnIfErrCS(false,!node,"[%s]cache underflow", a2dp_audio_smf_get_codec());
    //
    auto dec = _smf;
    if(!dec){
        a2dp_audio_smf_decoder_reinit();
        dec = _smf;
        returnIfErrC(0,!dec);
    }    
    //
    auto frm = &_output_frame;
    if(!frm->buff){
        frm->max = buffer_bytes*3;
        frm->buff = a2dp_audio_heap_malloc(frm->max);
        frm->size = 0;
        frm->offset = 0;
    }
    returnIfErrC(-1,!frm->buff);
    //
    auto frame_p = (a2dp_audio_smf_decoder_frame_t *)a2dp_audio_list_node(node);
    int bufferSize = frame_p->smf_buffer_len;
    
    /* decode one frame */
    smf_frame_t ifrm;
    ifrm.buff = frame_p->smf_buffer;
    ifrm.offset = 0;
    ifrm.max = bufferSize;
    ifrm.size = bufferSize;
    ifrm.flags = 0;
    
    smf_frame_t ofrm;
    ofrm.buff = (char*)frm->buff+frm->offset+frm->size;
    ofrm.offset = 0;
    ofrm.max = frm->max-frm->size-frm->offset;
    ofrm.size = 0;
    ofrm.flags = 0;

    auto rst = smf_decode(dec,&ifrm,&ofrm);
    if(!rst){
        smf_print_error(dec);
        auto err = smf_get_error(dec);
        TRACE_A2DP_DECODER_W("[MCU][%s]smf_decode failed:0x%08x-%08x",a2dp_audio_smf_get_codec(), err->err32[1], err->err32[0]);
        bool isopen = true;
        smf_get(dec,SMF_GET_IS_OPEN,&isopen);
        if(!isopen){
            a2dp_audio_smf_decoder_reinit();
            TRACE_A2DP_DECODER_I("[MCU][%s]smf_decode reinin codec \n", a2dp_audio_smf_get_codec() );
        }
        return false;
    }
    _output_frame_stamp = frame_p->timestamp - frm->size;//(frm->size*1000000ull/2/2/44100);
    _output_frame_index = frame_p->sequenceNumber;
    //dbgTestPXL("%u",smf_decoder_frame_p->timestamp);
    frm->size+=ofrm.size;
    //dbgTestPDL(frm->size);
    a2dp_audio_list_remove(list, frame_p);

    return true;
}

//static a2dp_audio_smf_decoder_frame_t _last_frame;
static int a2dp_audio_smf_mcu_decode_frame(uint8_t *buffer, uint32_t buffer_bytes)
{//dbgTestPXL("%p,%d",buffer,buffer_bytes);
    auto frm = &_output_frame;
    if(!frm->buff){
        frm->max = buffer_bytes*3;
        frm->buff = a2dp_audio_heap_malloc(frm->max);
        frm->size = 0;
        frm->offset = 0;
    }
    returnIfErrC(-1,!frm->buff);
    //
    auto dec = _smf;
    if(!dec){
        a2dp_audio_smf_decoder_reinit();
        dec = _smf;
        returnIfErrC(0,!dec);
    }    
    //        
    //bool cache_underflow = false;
    int output_byte = 0;
    unsigned stamp = 0;
    unsigned index  =0;

    dbgTestPXL("%d->%d,%u,%u",frm->size,buffer_bytes,_output_frame_stamp,_output_frame_index);
    if(frm->size >= (int)buffer_bytes){
        frm->size-=buffer_bytes;
        memcpy(buffer,(char*)frm->buff,buffer_bytes);
        if(frm->size)
            memmove((char*)frm->buff,(char*)frm->buff+buffer_bytes,frm->size);
        output_byte = buffer_bytes;
        stamp = _output_frame_stamp;
        index = _output_frame_index;
        _output_frame_stamp += buffer_bytes;
    }
    if(frm->size<(int)buffer_bytes){
        a2dp_audio_smf_mcu_decode_one_frame(buffer_bytes);
    }
    if(a2dp_audio_smf_get_samples() == output_byte/4){
        list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
        a2dp_audio_smf_lastframe_info.sequenceNumber = index;//plast->sequenceNumber;
        a2dp_audio_smf_lastframe_info.timestamp = stamp;//_output_frame_stamp - output_byte;//plast->timestamp;
        a2dp_audio_smf_lastframe_info.curSubSequenceNumber = 0;
        a2dp_audio_smf_lastframe_info.totalSubSequenceNumber = 0;
        a2dp_audio_smf_lastframe_info.frame_samples = output_byte;
        a2dp_audio_smf_lastframe_info.decoded_frames++;
        a2dp_audio_smf_lastframe_info.undecode_frames = a2dp_audio_list_length(list)-1;
        a2dp_audio_smf_lastframe_info.check_sum= 0;//a2dp_audio_decoder_internal_check_sum_generate(plast->smf_buffer, plast->smf_buffer_len);
        a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_smf_lastframe_info);
    }
    
    /*if(cache_underflow){
        a2dp_audio_smf_lastframe_info.undecode_frames = 0;
        a2dp_audio_smf_lastframe_info.check_sum = 0;
        a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_smf_lastframe_info);
        output_byte = A2DP_DECODER_CACHE_UNDERFLOW_ERROR;
    }*/
    //dbgTestPDL(output_byte);
    return output_byte;
}

#else
int a2dp_audio_smf_mcu_decode_frame(uint8_t *buffer, uint32_t buffer_bytes)
{//dbgTestPXL("%p,%d",buffer,buffer_bytes);
    auto dec = _smf;
    if(!dec){
        a2dp_audio_smf_decoder_reinit();
        returnIfErrC(0,!dec);
    }
    //
    a2dp_audio_smf_decoder_frame_t *smf_decoder_frame_p = NULL;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    uint32_t bufferSize = 0;

    bool cache_underflow = false;
    int output_byte = 0;

    /*if(buffer_bytes < DECODE_AAC_PCM_FRAME_LENGTH){
        TRACE_A2DP_DECODER_W("[MCU][%s] pcm_len = %d \n", a2dp_audio_smf_get_codec(), buffer_bytes);
        return A2DP_DECODER_NO_ERROR;
    }*/
    if(!dec){
        TRACE_A2DP_DECODER_W("[MCU][%s] smf_decode not ready", a2dp_audio_smf_get_codec());
        return A2DP_DECODER_NO_ERROR;
    }

    node = a2dp_audio_list_begin(list);
    if (!node){
        TRACE_A2DP_DECODER_W("[MCU][%s] cache underflow", a2dp_audio_smf_get_codec());
        cache_underflow = true;
        goto exit;
    }else{
        //auto last = *smf_decoder_frame_p;
        //if(smf_decoder_frame_p){
            //dbgTestPXL("input:%p,%d",smf_decoder_frame_p->smf_buffer,smf_decoder_frame_p->smf_buffer_len);
            //dbgTestDump(smf_decoder_frame_p->smf_buffer,32);
            //a2dp_audio_list_remove(list, smf_decoder_frame_p);
        //}
        smf_decoder_frame_p = (a2dp_audio_smf_decoder_frame_t *)a2dp_audio_list_node(node);

        /*if (smf_decoder_frame_p->smf_buffer_len < 64)
            smf_maxreadBytes = 64;
        else if (smf_decoder_frame_p->smf_buffer_len < 128)
            smf_maxreadBytes = 128;
        else if (smf_decoder_frame_p->smf_buffer_len < 256)
            smf_maxreadBytes = 256;
        else if (smf_decoder_frame_p->smf_buffer_len < 512)
            smf_maxreadBytes = 512;
        else if (smf_decoder_frame_p->smf_buffer_len < 1024)
            smf_maxreadBytes = 1024;*/
        bufferSize = smf_decoder_frame_p->smf_buffer_len;
        //bufferSize = smf_maxreadBytes;
        //bytesValid = smf_maxreadBytes;       

        /* decode one frame */
        smf_frame_t ifrm;
        ifrm.buff = smf_decoder_frame_p->smf_buffer;
        ifrm.offset = 0;
        ifrm.max = bufferSize;
        ifrm.size = bufferSize;
        ifrm.flags = 0;
        
        smf_frame_t ofrm;
        ofrm.buff = buffer;
        ofrm.offset = 0;
        ofrm.max = buffer_bytes;
        ofrm.size = 0;
        ofrm.flags = 0;
        
        //dbgTestPXL("input:%p,%d",ifrm.buff,bufferSize);
        //dbgTestDump(ifrm.buff,32);
        auto rst = smf_decode(dec,&ifrm,&ofrm);
        //dbgTestPXL("input:%p,%d",ifrm.buff,bufferSize);
        //dbgTestDump(ifrm.buff,32);

        if(!rst){
            smf_print_error(dec);
            auto err = smf_get_error(dec);
            TRACE_A2DP_DECODER_W("[MCU][%s]smf_decode failed:0x%08x-%08x",a2dp_audio_smf_get_codec(), err->err32[1], err->err32[0]);
            bool isopen = true;
            smf_get(dec,SMF_GET_IS_OPEN,&isopen);
            //if(!smf_is_open(dec)){
            if(!isopen){
                a2dp_audio_smf_decoder_reinit();
                TRACE_A2DP_DECODER_I("[MCU][%s]smf_decode reinin codec \n", a2dp_audio_smf_get_codec() );
            }
            goto end_decode;
        }
      
        output_byte = ofrm.size;
        /*ASSERT_A2DP_DECODER(a2dp_audio_smf_get_samples() == output_byte/4
            , "smf_decode output mismatch samples:%d,%d"
            , output_byte/4
            ,a2dp_audio_smf_get_samples()
            );*/
end_decode:
        //dbgTestPXL("output:%p,%d",buffer,output_byte);
        //dbgTestDump(buffer,16);
        if(a2dp_audio_smf_get_samples() == output_byte/4){
            a2dp_audio_smf_decoder_frame_t* plast = smf_decoder_frame_p;
            a2dp_audio_smf_lastframe_info.sequenceNumber = plast->sequenceNumber;
            a2dp_audio_smf_lastframe_info.timestamp = plast->timestamp;
            a2dp_audio_smf_lastframe_info.curSubSequenceNumber = 0;
            a2dp_audio_smf_lastframe_info.totalSubSequenceNumber = 0;
            a2dp_audio_smf_lastframe_info.frame_samples = output_byte;
            a2dp_audio_smf_lastframe_info.decoded_frames++;
            a2dp_audio_smf_lastframe_info.undecode_frames = a2dp_audio_list_length(list)-1;
            a2dp_audio_smf_lastframe_info.check_sum= a2dp_audio_decoder_internal_check_sum_generate(plast->smf_buffer, plast->smf_buffer_len);
            a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_smf_lastframe_info);
        }
        a2dp_audio_list_remove(list, smf_decoder_frame_p);
    }
exit:
    if(cache_underflow){
        a2dp_audio_smf_lastframe_info.undecode_frames = 0;
        a2dp_audio_smf_lastframe_info.check_sum = 0;
        a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_smf_lastframe_info);
        output_byte = A2DP_DECODER_CACHE_UNDERFLOW_ERROR;
    }
    dbgTestPDL(output_byte);
    return output_byte;
}
#endif
int a2dp_audio_smf_decode_frame(uint8_t *buffer, uint32_t buffer_bytes)
{
    return a2dp_audio_smf_mcu_decode_frame(buffer, buffer_bytes);
}

int a2dp_audio_smf_preparse_packet(btif_media_header_t * header, uint8_t *buffer, uint32_t buffer_bytes)
{
    a2dp_audio_smf_lastframe_info.sequenceNumber = header->sequenceNumber;
    a2dp_audio_smf_lastframe_info.timestamp = header->timestamp;
    a2dp_audio_smf_lastframe_info.curSubSequenceNumber = 0;
    a2dp_audio_smf_lastframe_info.totalSubSequenceNumber = 0;
    a2dp_audio_smf_lastframe_info.frame_samples = a2dp_audio_smf_get_samples();
    a2dp_audio_smf_lastframe_info.list_samples = a2dp_audio_smf_get_samples();
    a2dp_audio_smf_lastframe_info.decoded_frames = 0;
    a2dp_audio_smf_lastframe_info.undecode_frames = 0;
    a2dp_audio_smf_lastframe_info.check_sum = 0;
    a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_smf_lastframe_info);

    TRACE_A2DP_DECODER_I("[AAC][PRE] seq:%d timestamp:%08x", header->sequenceNumber, header->timestamp);

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_smf_reorder_init(void)
{
    smf_decoder_frame_reorder_p = NULL;
    smf_decoder_last_valid_frame_ready = false;
    smf_decoder_last_valid_frame.sequenceNumber = 0;
    smf_decoder_last_valid_frame.timestamp = 0;
    smf_decoder_last_valid_frame.smf_buffer = NULL;
    smf_decoder_last_valid_frame.smf_buffer_len = 0;

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_smf_reorder_deinit(void)
{
    smf_decoder_frame_reorder_p = NULL;
    smf_decoder_last_valid_frame_ready = false;
    smf_decoder_last_valid_frame.sequenceNumber = 0;
    smf_decoder_last_valid_frame.timestamp = 0;
    smf_decoder_last_valid_frame.smf_buffer = NULL;
    smf_decoder_last_valid_frame.smf_buffer_len = 0;
    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_smf_reorder_store_packet(a2dp_audio_smf_decoder_frame_t *smf_decoder_frame_p,
                                                              btif_media_header_t * header, uint8_t *buffer, uint32_t buffer_bytes)
{
    smf_decoder_frame_p->sequenceNumber = header->sequenceNumber;
    smf_decoder_frame_p->timestamp = header->timestamp;
    memcpy(smf_decoder_frame_p->smf_buffer, buffer, buffer_bytes);
    smf_decoder_frame_p->smf_buffer_len = buffer_bytes;
    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_smf_reorder_proc(a2dp_audio_smf_decoder_frame_t *smf_decoder_frame_p,
                                                        btif_media_header_t * header, uint8_t *buffer, uint32_t buffer_bytes)
{
    uint8_t *dest_buf = NULL;
    if ((smf_decoder_frame_p->smf_buffer_len + buffer_bytes) > (uint32_t)a2dp_audio_smf_get_frame_max()){
        return A2DP_DECODER_NO_ERROR;
    }
    TRACE_A2DP_DECODER_W("[AAC][INPUT][REORDER] proc enter seq:%d len:%d", smf_decoder_frame_p->sequenceNumber, smf_decoder_frame_p->smf_buffer_len);
    dest_buf = &smf_decoder_frame_p->smf_buffer[smf_decoder_frame_p->smf_buffer_len];
    memcpy(dest_buf, buffer, buffer_bytes);
    smf_decoder_frame_p->smf_buffer_len += buffer_bytes;
    TRACE_A2DP_DECODER_W("[AAC][INPUT][REORDER] proc exit seq:%d len:%d", smf_decoder_frame_p->sequenceNumber, smf_decoder_frame_p->smf_buffer_len);

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_smf_packet_recover_save_last(a2dp_audio_smf_decoder_frame_t *smf_decoder_frame)
{
    smf_decoder_last_valid_frame_ready = true;
    smf_decoder_last_valid_frame.sequenceNumber = smf_decoder_frame->sequenceNumber;
    smf_decoder_last_valid_frame.timestamp = smf_decoder_frame->timestamp;
    return 0;
}

int a2dp_audio_smf_packet_recover_find_missing(a2dp_audio_smf_decoder_frame_t *smf_decoder_frame)
{
    uint16_t diff_seq = 0;
    uint32_t diff_timestamp = 0;
    uint32_t diff = 0;
    float tmp_pkt_cnt = 0;
    uint32_t need_recover_pkt = 0;

    if (!smf_decoder_last_valid_frame_ready){
        return need_recover_pkt;
    }

    diff_seq = a2dp_audio_get_passed(smf_decoder_frame->sequenceNumber, smf_decoder_last_valid_frame.sequenceNumber, UINT16_MAX);
    diff_timestamp = a2dp_audio_get_passed(smf_decoder_frame->timestamp, smf_decoder_last_valid_frame.timestamp, UINT32_MAX);

    if (diff_seq > 1){
        TRACE_A2DP_DECODER_W("[%s][INPUT][PLC] seq:%d/%d stmp:%d/%d"
            , a2dp_audio_smf_get_codec()
            , smf_decoder_frame->sequenceNumber
            , smf_decoder_last_valid_frame.sequenceNumber
            ,smf_decoder_frame->timestamp, smf_decoder_last_valid_frame.timestamp
            );
        diff = diff_timestamp/diff_seq;
        if (diff%a2dp_audio_smf_get_samples() == 0){
            tmp_pkt_cnt = (float)diff_timestamp/a2dp_audio_smf_get_samples();
        }else{
            tmp_pkt_cnt = (float)diff_timestamp/
            ((1000.f/(float)a2dp_audio_context_p->output_cfg.sample_rate)*(float)a2dp_audio_smf_get_samples());
        }
        need_recover_pkt = (uint32_t)(tmp_pkt_cnt+0.5f);
        TRACE_A2DP_DECODER_W("[%s][INPUT][PLC] diff_seq:%d diff_stmp:%d diff:%d missing:%d", a2dp_audio_smf_get_codec(),diff_seq, diff_timestamp, diff, need_recover_pkt);
        if (need_recover_pkt == diff_seq){
            need_recover_pkt--;
            TRACE_A2DP_DECODER_W("[%s][INPUT][PLC] need_recover_pkt seq:%d", a2dp_audio_smf_get_codec(),need_recover_pkt);
        }else{
            need_recover_pkt = 0;
        }
    }
    return need_recover_pkt;
}

int a2dp_audio_smf_packet_recover_proc(a2dp_audio_smf_decoder_frame_t *smf_decoder_frame)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    int missing_pkt_cnt = 0;
    missing_pkt_cnt = a2dp_audio_smf_packet_recover_find_missing(smf_decoder_frame);
    if (missing_pkt_cnt <= 4 && a2dp_audio_list_length(list) + missing_pkt_cnt < smf_mtu_limiter){
        for (uint8_t i=0; i<missing_pkt_cnt; i++){
#if AAC_MUTE_FRAME
            a2dp_audio_smf_decoder_frame_t *smf_decoder_frame_p = (a2dp_audio_smf_decoder_frame_t *)a2dp_audio_smf_frame_malloc(AAC_MUTE_FRAME_MAX);
            smf_decoder_frame_p->sequenceNumber = UINT16_MAX;
            smf_decoder_frame_p->timestamp = UINT32_MAX;    
            A2DP_AUDIO_OUTPUT_CONFIG_T*info=&a2dp_audio_smf_lastframe_info.stream_info;
            smf_decoder_frame_p->smf_buffer_len = aacCreateMuteFrame(smf_decoder_frame_p->smf_buffer
                ,info->sample_rate
                ,info->num_channels
                ,AAC_DEC_TRANSPORT_TYPE
                );
#else
            a2dp_audio_smf_decoder_frame_t *smf_decoder_frame_p = (a2dp_audio_smf_decoder_frame_t *)a2dp_audio_smf_frame_malloc(smf_decoder_frame->smf_buffer_len);
            smf_decoder_frame_p->sequenceNumber = UINT16_MAX;
            smf_decoder_frame_p->timestamp = UINT32_MAX;
            memcpy(smf_decoder_frame_p->smf_buffer, smf_decoder_frame->smf_buffer, smf_decoder_frame->smf_buffer_len);
            smf_decoder_frame_p->smf_buffer_len = smf_decoder_frame->smf_buffer_len;
#endif
            a2dp_audio_list_append(list, smf_decoder_frame_p);
        }
    }

    return A2DP_DECODER_NO_ERROR;
}

int inline a2dp_audio_smf_packet_append(a2dp_audio_smf_decoder_frame_t *smf_decoder_frame)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    a2dp_audio_smf_packet_recover_proc(smf_decoder_frame);
    a2dp_audio_smf_packet_recover_save_last(smf_decoder_frame);
    a2dp_audio_list_append(list, smf_decoder_frame);

    return 0;
}

#if 1
int a2dp_audio_smf_store_packet(btif_media_header_t * header, uint8_t *buffer, uint32_t buffer_bytes)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    int nRet = A2DP_DECODER_NO_ERROR;
    a2dp_audio_smf_decoder_frame_t *smf_decoder_frame_p = NULL;

    if (a2dp_audio_list_length(list) < smf_mtu_limiter &&
        buffer_bytes <= (uint32_t)a2dp_audio_smf_get_frame_max()){
        if (smf_decoder_frame_reorder_p == NULL){
            smf_decoder_frame_p = (a2dp_audio_smf_decoder_frame_t *)a2dp_audio_smf_frame_malloc(buffer_bytes);
            smf_decoder_frame_reorder_p = smf_decoder_frame_p;
            TRACE_A2DP_DECODER_W("[AAC][INPUT][REORDER] start seq:%d len:%d", header->sequenceNumber, buffer_bytes);
            a2dp_audio_smf_reorder_store_packet(smf_decoder_frame_p, header, buffer, buffer_bytes);
        }else{
            if (smf_decoder_frame_reorder_p->smf_buffer[2] == buffer[2] &&
                smf_decoder_frame_reorder_p->smf_buffer[1] == buffer[1]){
                dbgTestPXL("%d,%d,%d",smf_decoder_frame_reorder_p->sequenceNumber,smf_decoder_frame_reorder_p->timestamp,smf_decoder_frame_reorder_p->smf_buffer_len);
                a2dp_audio_smf_packet_append(smf_decoder_frame_reorder_p);
                smf_decoder_frame_p = (a2dp_audio_smf_decoder_frame_t *)a2dp_audio_smf_frame_malloc(buffer_bytes);
                smf_decoder_frame_reorder_p = smf_decoder_frame_p;
                a2dp_audio_smf_reorder_store_packet(smf_decoder_frame_p, header, buffer, buffer_bytes);
            }else{
                dbgTestDump(smf_decoder_frame_reorder_p->smf_buffer,8);
                dbgTestDump(buffer,8);
                smf_decoder_frame_p = smf_decoder_frame_reorder_p;
                a2dp_audio_smf_reorder_proc(smf_decoder_frame_p, header, buffer, buffer_bytes);
                a2dp_audio_smf_packet_append(smf_decoder_frame_p);
                smf_decoder_frame_reorder_p = NULL;
            }
        }
        nRet = A2DP_DECODER_NO_ERROR;
    }
    else{        
        TRACE_A2DP_DECODER_W("[AAC][INPUT] list full current list_len:%d buff_len:%d", a2dp_audio_list_length(list), buffer_bytes);
        nRet = A2DP_DECODER_MTU_LIMTER_ERROR;
    }

    return nRet;
}
#else
int a2dp_audio_smf_store_packet(btif_media_header_t * header, uint8_t *buffer, uint32_t buffer_bytes)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    int nRet = A2DP_DECODER_NO_ERROR;

    if (!a2dp_audio_smf_reorder_valid(header, buffer, buffer_bytes)){
        return A2DP_DECODER_NO_ERROR;
    }

    if (a2dp_audio_list_length(list) < smf_mtu_limiter &&
        buffer_bytes <= AAC_READBUF_SIZE){
        a2dp_audio_smf_decoder_frame_t *smf_decoder_frame_p = (a2dp_audio_smf_decoder_frame_t *)a2dp_audio_smf_frame_malloc(buffer_bytes);
        TRACE_A2DP_DECODER_D("[AAC][INPUT]seq:%d len:%d", header->sequenceNumber, buffer_bytes);
        smf_decoder_frame_p->sequenceNumber = header->sequenceNumber;
        smf_decoder_frame_p->timestamp = header->timestamp;
        memcpy(smf_decoder_frame_p->smf_buffer, buffer, buffer_bytes);
        smf_decoder_frame_p->smf_buffer_len = buffer_bytes;
        a2dp_audio_smf_reorder_history(smf_decoder_frame_p);
        a2dp_audio_list_append(list, smf_decoder_frame_p);
        nRet = A2DP_DECODER_NO_ERROR;
    }else{
        TRACE_A2DP_DECODER_I("[AAC][INPUT] list full current list_len:%d buff_len:%d", a2dp_audio_list_length(list), buffer_bytes);
        nRet = A2DP_DECODER_MTU_LIMTER_ERROR;
    }

    return nRet;
}
#endif

int a2dp_audio_smf_discards_packet(uint32_t packets)
{
    int nRet = A2DP_DECODER_MEMORY_ERROR;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    a2dp_audio_smf_decoder_frame_t *smf_decoder_frame_p = NULL;

    if (packets <= a2dp_audio_list_length(list)){
        for (uint8_t i=0; i<packets; i++){
            if ((node = a2dp_audio_list_begin(list)) != NULL){
                smf_decoder_frame_p = (a2dp_audio_smf_decoder_frame_t *)a2dp_audio_list_node(node);
                a2dp_audio_list_remove(list, smf_decoder_frame_p);
            }
        }
        nRet = A2DP_DECODER_NO_ERROR;
    }

    TRACE_A2DP_DECODER_I("[AAC][DISCARDS] packets:%d nRet:%d", packets, nRet);
    return nRet;
}

int a2dp_audio_smf_headframe_info_get(A2DP_AUDIO_HEADFRAME_INFO_T* headframe_info)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    a2dp_audio_smf_decoder_frame_t *smf_decoder_frame = NULL;

    if (a2dp_audio_list_length(list) && ((node = a2dp_audio_list_begin(list)) != NULL)){
        smf_decoder_frame = (a2dp_audio_smf_decoder_frame_t *)a2dp_audio_list_node(node);
        headframe_info->sequenceNumber = smf_decoder_frame->sequenceNumber;
        headframe_info->timestamp = smf_decoder_frame->timestamp;
        headframe_info->curSubSequenceNumber = 0;
        headframe_info->totalSubSequenceNumber = 0;
    }else{
        memset(headframe_info, 0, sizeof(A2DP_AUDIO_HEADFRAME_INFO_T));
    }

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_smf_info_get(void *info)
{
    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_smf_synchronize_packet(A2DP_AUDIO_SYNCFRAME_INFO_T *sync_info, uint32_t mask)
{
    int nRet = A2DP_DECODER_SYNC_ERROR;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    int list_len;
    a2dp_audio_smf_decoder_frame_t *smf_decoder_frame_p = NULL;

    list_len = a2dp_audio_list_length(list);

    for (uint16_t i=0; i<list_len; i++){
        node = a2dp_audio_list_begin(list);
        if (node){
            smf_decoder_frame_p = (a2dp_audio_smf_decoder_frame_t *)a2dp_audio_list_node(node);
            TRACE_A2DP_DECODER_D("[synchronize_packet]%d/%d %x/%x", smf_decoder_frame_p->sequenceNumber, sync_info->sequenceNumber,
                                  smf_decoder_frame_p->timestamp, sync_info->timestamp);
            if (A2DP_AUDIO_SYNCFRAME_CHK(smf_decoder_frame_p->sequenceNumber  >= sync_info->sequenceNumber, A2DP_AUDIO_SYNCFRAME_MASK_SEQ,       mask)&&
                A2DP_AUDIO_SYNCFRAME_CHK(smf_decoder_frame_p->timestamp       >= sync_info->timestamp,      A2DP_AUDIO_SYNCFRAME_MASK_TIMESTAMP, mask)){
                nRet = A2DP_DECODER_NO_ERROR;
                break;
            }
            a2dp_audio_list_remove(list, smf_decoder_frame_p);
        }
    }

    node = a2dp_audio_list_begin(list);
    if (node){
        smf_decoder_frame_p = (a2dp_audio_smf_decoder_frame_t *)a2dp_audio_list_node(node);
        TRACE_A2DP_DECODER_I("[MCU][SYNC][AAC] sync pkt nRet:%d SEQ:%d timestamp:%d", nRet, smf_decoder_frame_p->sequenceNumber, smf_decoder_frame_p->timestamp);
    }else{
        TRACE_A2DP_DECODER_I("[MCU][SYNC][AAC] sync pkt nRet:%d",  nRet);
        if(smf_decoder_frame_reorder_p) {
            smf_decoder_frame_p = smf_decoder_frame_reorder_p;
            TRACE_A2DP_DECODER_D("[synchronize_packet]%d/%d %x/%x", smf_decoder_frame_p->sequenceNumber, sync_info->sequenceNumber,
                                  smf_decoder_frame_p->timestamp, sync_info->timestamp);
            if (A2DP_AUDIO_SYNCFRAME_CHK(smf_decoder_frame_p->sequenceNumber  >= sync_info->sequenceNumber, A2DP_AUDIO_SYNCFRAME_MASK_SEQ,       mask)&&
                A2DP_AUDIO_SYNCFRAME_CHK(smf_decoder_frame_p->timestamp       >= sync_info->timestamp,      A2DP_AUDIO_SYNCFRAME_MASK_TIMESTAMP, mask)){
                nRet = A2DP_DECODER_NO_ERROR;
                TRACE_A2DP_DECODER_I("[MCU][SYNC][AAC] sync pkt reorder_p nRet:%d SEQ:%d timestamp:%d", nRet, smf_decoder_frame_p->sequenceNumber, smf_decoder_frame_p->timestamp);
            }
        }
    }

    return nRet;
}

int a2dp_audio_smf_synchronize_dest_packet_mut(uint16_t packet_mut)
{
    list_node_t *node = NULL;
    uint32_t list_len = 0;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    a2dp_audio_smf_decoder_frame_t *smf_decoder_frame_p = NULL;

    list_len = a2dp_audio_list_length(list);
    if (list_len > packet_mut){
        do{
            node = a2dp_audio_list_begin(list);
            if (node){
                smf_decoder_frame_p = (a2dp_audio_smf_decoder_frame_t *)a2dp_audio_list_node(node);
                a2dp_audio_list_remove(list, smf_decoder_frame_p);
            }
        }while(a2dp_audio_list_length(list) > packet_mut);
    }

    if (packet_mut == 0){
        if (smf_decoder_frame_reorder_p){
            a2dp_audio_smf_free(smf_decoder_frame_reorder_p);
            a2dp_audio_smf_reorder_init();
        }
    }

    TRACE_A2DP_DECODER_I("[MCU][SYNC][AAC] dest pkt list:%d", a2dp_audio_list_length(list));
    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_smf_convert_list_to_samples(uint32_t *samples)
{
    uint32_t list_len = 0;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;

    list_len = a2dp_audio_list_length(list);
    *samples = a2dp_audio_smf_get_samples()*list_len;

    //TRACE_A2DP_DECODER_I("AUD][DECODER][MCU][AAC] list:%d samples:%d", list_len, *samples);

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_smf_discards_samples(uint32_t samples)
{
    int nRet = A2DP_DECODER_SYNC_ERROR;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    a2dp_audio_smf_decoder_frame_t *smf_decoder_frame_p = NULL;
    list_node_t *node = NULL;
    int need_remove_list = 0;
    uint32_t list_samples = 0;
    ASSERT_A2DP_DECODER(!(samples%a2dp_audio_smf_get_samples()), "%s samples err:%d", __func__, samples);

    a2dp_audio_smf_convert_list_to_samples(&list_samples);
    if (list_samples >= samples){
        need_remove_list = samples/a2dp_audio_smf_get_samples();
        for (int i=0; i<need_remove_list; i++){
            node = a2dp_audio_list_begin(list);
            if (node){
                smf_decoder_frame_p = (a2dp_audio_smf_decoder_frame_t *)a2dp_audio_list_node(node);
                a2dp_audio_list_remove(list, smf_decoder_frame_p);
            }
        }
        nRet = A2DP_DECODER_NO_ERROR;
    }

    return nRet;
}

#if 1
extern const A2DP_AUDIO_DECODER_T a2dp_audio_aac_lc_decoder_config = {
    {44100, 2, 16},
    1,{fcc64("aac"), 25, 1024, 900,  SMF_AAC_PACKAGE_MCP1},
    a2dp_audio_smf_init,
    a2dp_audio_smf_deinit,
    a2dp_audio_smf_decode_frame,
    a2dp_audio_smf_preparse_packet,
    a2dp_audio_smf_store_packet,
    a2dp_audio_smf_discards_packet,
    a2dp_audio_smf_synchronize_packet,
    a2dp_audio_smf_synchronize_dest_packet_mut,
    a2dp_audio_smf_convert_list_to_samples,
    a2dp_audio_smf_discards_samples,
    a2dp_audio_smf_headframe_info_get,
    a2dp_audio_smf_info_get,
    a2dp_audio_smf_free,
    a2dp_audio_smf_channel_select,
} ;
#endif

#if 1
extern const A2DP_AUDIO_DECODER_T a2dp_audio_sbc_decoder_config = {
    {44100, 2, 16},
    //1,{fcc64("sbc"), 250, 128, 96},
    1,{fcc64("sbc"), 51, 128*5, 96*8, SMF_SBC_PACKAGE_A2DP},
    a2dp_audio_smf_init,
    a2dp_audio_smf_deinit,
    a2dp_audio_smf_mcu_decode_frame,
    a2dp_audio_smf_preparse_packet,
    a2dp_audio_smf_store_packet,
    a2dp_audio_smf_discards_packet,
    a2dp_audio_smf_synchronize_packet,
    a2dp_audio_smf_synchronize_dest_packet_mut,
    a2dp_audio_smf_convert_list_to_samples,
    a2dp_audio_smf_discards_samples,
    a2dp_audio_smf_headframe_info_get,
    a2dp_audio_smf_info_get,
    a2dp_audio_smf_free,
    a2dp_audio_smf_channel_select,
} ;
#endif

