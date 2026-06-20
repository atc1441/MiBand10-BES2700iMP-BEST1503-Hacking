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
// Standard C Included Files
#include "cmsis.h"
#include "plat_types.h"
#include <string.h>
#include "heap_api.h"
#include "hal_location.h"
#include "a2dp_decoder_internal.h"
#include "lc3_process.h"
#include "bluetooth_bt_api.h"
#include "btapp.h"
#include "app_overlay.h"

#define LC3_READBUF_SIZE (800)
#define LC3_MTU_LIMITER (30)
#define LC3_OUTPUT_FRAME_SAMPLES (480)
#define A2DP_LC3_OUTPUT_FRAME_SAMPLES   (480)

static A2DP_AUDIO_CONTEXT_T *a2dp_audio_context_p = NULL;
static A2DP_AUDIO_DECODER_LASTFRAME_INFO_T a2dp_audio_lc3_lastframe_info;
static A2DP_AUDIO_OUTPUT_CONFIG_T a2dp_audio_lc3_output_config;

typedef struct {
    A2DP_COMMON_MEDIA_FRAME_HEADER_T header;
} a2dp_audio_lc3_decoder_frame_t;

LC3_Dec_Info* lc3_dec_info = NULL;
uint32_t lc3_pcm_frame_len = 0;
static void* lc3_alloc(void* pool,unsigned size){
    return a2dp_audio_heap_malloc(size);
}
static void lc3_free(void* pool, void*ptr){
    a2dp_audio_heap_free(ptr);
}
static void lc3_overlay(LC3_Enc_Info* info){
    //APP_OVERLAY_ID_T id = info->is_lc3plus ? APP_OVERLAY_LC3PLUS : APP_OVERLAY_LC3;
    //app_overlay_select(id);
}

int a2dp_audio_lc3_decoder_init(uint32_t sampleRate, uint8_t channelNum, uint8_t bits)
{
    LC3_API_PlcMode plcMeth = LC3_API_PLC_ADVANCED;
    uint8_t frame_dms = 100;	
    uint32_t epmode = 0;
    TRACE(3, "lc3_dec_init sample Rate=%d, channel_mode = %d, bits = %d\n", sampleRate, channelNum, bits);

    LC3_Dec_Info*info = (LC3_Dec_Info*)a2dp_audio_heap_malloc(sizeof(LC3_Dec_Info));
    lc3_dec_info = info;
    memset((void*)lc3_dec_info, 0x0, sizeof(LC3_Dec_Info));
    info->sample_rate = sampleRate;
    info->channels = channelNum;
    info->bitwidth = bits;
    info->frame_dms = frame_dms;
    info->is_interlaced = true;
    info->plcMeth = plcMeth;
    info->epmode= (LC3_API_EpMode)epmode;
    info->pool = 0;
    info->cb_alloc = &lc3_alloc;
    info->cb_free = &lc3_free;
    info->cb_overlay = &lc3_overlay;
    lc3_api_decoder_init(info);

    lc3_pcm_frame_len = info->frame_samples*channelNum*bits/8;

    /* Print info */
    TRACE(1, "Decoder size:     %i", info->instance_size);
    TRACE(1, "Scratch size:     %i", info->scratch_size);
    TRACE(1, "Sample rate:      %i", sampleRate);
    TRACE(1, "Channels:         %i", channelNum);
    TRACE(1, "Frame length:     %i", info->frame_samples);
    TRACE(1, "PLC mode:         %i", plcMeth);

    return 0;
}

static void a2dp_audio_lc3_decoder_deinit(void)
{
    LC3_Dec_Info*info = lc3_dec_info;
    if(info){
        if (info->cb_uninit){
            info->cb_uninit(info);
        }
        a2dp_audio_heap_free(info);
        lc3_dec_info = NULL;
    }
}

static void *a2dp_audio_lc3_frame_malloc(uint32_t packet_len)
{
    a2dp_audio_lc3_decoder_frame_t *decoder_frame_p = NULL;
    uint8_t *buffer = NULL;

    buffer = (uint8_t *)a2dp_audio_heap_malloc(packet_len);
    decoder_frame_p = (a2dp_audio_lc3_decoder_frame_t *)a2dp_audio_heap_malloc(sizeof(a2dp_audio_lc3_decoder_frame_t));
    decoder_frame_p->header.ptrData = buffer;
    decoder_frame_p->header.dataLen = packet_len;
    decoder_frame_p->header.totalSubSequenceNumber = 0;
    decoder_frame_p->header.curSubSequenceNumber = 0;    
    return (void *)decoder_frame_p;
}


static int a2dp_audio_lc3_list_checker(void)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    a2dp_audio_lc3_decoder_frame_t *lc3_decoder_frame_p = NULL;
    int cnt = 0;

    do {
        lc3_decoder_frame_p = (a2dp_audio_lc3_decoder_frame_t *)a2dp_audio_lc3_frame_malloc(LC3_READBUF_SIZE);
        if (lc3_decoder_frame_p){
            a2dp_audio_list_append(list, lc3_decoder_frame_p);
        }
        cnt++;
    }while(lc3_decoder_frame_p && cnt < LC3_MTU_LIMITER);

    do {
        node = a2dp_audio_list_begin(list);
        if (node){
            lc3_decoder_frame_p = (a2dp_audio_lc3_decoder_frame_t *)a2dp_audio_list_node(node);
            a2dp_audio_list_remove(list, lc3_decoder_frame_p);
        }
    }while(node);

    TRACE(3,"%s cnt:%d list:%d", __func__, cnt, a2dp_audio_list_length(list));

    return 0;
}

//int16_t lc3_noninterleave_buf[960] = {0};

static int a2dp_audio_lc3_mcu_decode_frame(uint8_t * buffer, uint32_t buffer_bytes)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    LC3_Dec_Info*info = lc3_dec_info;
    A2DP_AUDIO_DECODER_LASTFRAME_INFO_T *lstframes = &a2dp_audio_lc3_lastframe_info;
    list_node_t *node = NULL;
    a2dp_audio_lc3_decoder_frame_t* lc3_decoder_frame_p = NULL;
    bool cache_underflow = false;
    int output_byte = 0;
    int err     = LC3_API_OK;

    node = a2dp_audio_list_begin(list);    
    if (!node){
        TRACE(0,"lc3_decode cache underflow");
        cache_underflow = true;
        goto exit;
    }else{
        lc3_decoder_frame_p = (a2dp_audio_lc3_decoder_frame_t *)a2dp_audio_list_node(node);
        do {
            err = info->cb_decode_interlaced(info, info->scratch,
                lc3_decoder_frame_p->header.ptrData, lc3_decoder_frame_p->header.dataLen, buffer, 0);
            if (err == LC3_API_OK){
                output_byte += lc3_pcm_frame_len;
            }else{
                TRACE(1, "lc3 decode err %d", err);
                break;
            }            
            lstframes->sequenceNumber = lc3_decoder_frame_p->header.sequenceNumber;
            lstframes->timestamp = lc3_decoder_frame_p->header.timestamp;
            lstframes->curSubSequenceNumber = lc3_decoder_frame_p->header.curSubSequenceNumber;
            lstframes->totalSubSequenceNumber = lc3_decoder_frame_p->header.totalSubSequenceNumber;
            lstframes->frame_samples = A2DP_LC3_OUTPUT_FRAME_SAMPLES;
            lstframes->decoded_frames++;
            lstframes->undecode_frames = a2dp_audio_list_length(list)-1;
            a2dp_audio_decoder_internal_lastframe_info_set(lstframes);  
            a2dp_audio_list_remove(list, lc3_decoder_frame_p);
        }while(output_byte < (int)buffer_bytes);
    }
exit:
    if(cache_underflow){
        lstframes->undecode_frames = 0;
        a2dp_audio_decoder_internal_lastframe_info_set(lstframes);
        output_byte = A2DP_DECODER_CACHE_UNDERFLOW_ERROR;
    }
    return output_byte;
}

int a2dp_audio_lc3_decode_frame(uint8_t *buffer, uint32_t buffer_bytes)
{
    return a2dp_audio_lc3_mcu_decode_frame(buffer, buffer_bytes);
}

int a2dp_audio_lc3_preparse_packet(btif_media_header_t * header, uint8_t *buffer, uint32_t buffer_bytes)
{
    A2DP_AUDIO_DECODER_LASTFRAME_INFO_T *lstframes = &a2dp_audio_lc3_lastframe_info;
    lstframes->sequenceNumber = header->sequenceNumber;
    lstframes->timestamp = header->timestamp;
    lstframes->curSubSequenceNumber = 0;
    lstframes->totalSubSequenceNumber = 0;
    lstframes->frame_samples = A2DP_LC3_OUTPUT_FRAME_SAMPLES;
    lstframes->list_samples = A2DP_LC3_OUTPUT_FRAME_SAMPLES;
    lstframes->decoded_frames = 0;
    lstframes->undecode_frames = 0;
    a2dp_audio_decoder_internal_lastframe_info_set(lstframes);

    TRACE(3,"%s seq:%d timestamp:%08x", __func__, header->sequenceNumber, header->timestamp);

    return A2DP_DECODER_NO_ERROR;
}

void a2dp_audio_lc3_free(void *packet)
{
    a2dp_audio_lc3_decoder_frame_t *decoder_frame_p = (a2dp_audio_lc3_decoder_frame_t *)packet; 
    a2dp_audio_heap_free(decoder_frame_p->header.ptrData);
    a2dp_audio_heap_free(decoder_frame_p);
}

int a2dp_audio_lc3_store_packet(btif_media_header_t * header, uint8_t *buffer, uint32_t buffer_bytes)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    int nRet = A2DP_DECODER_NO_ERROR;
    uint16_t frame_len = 0;

    frame_len = (buffer[0]) | (buffer[1] << 8);
    //TRACE(2,"%s, frame_len:%d %d, %d", __func__, frame_len, buffer_bytes, a2dp_audio_list_length(list));
    if (a2dp_audio_list_length(list) < LC3_MTU_LIMITER) {	
        a2dp_audio_lc3_decoder_frame_t *decoder_frame_p = (a2dp_audio_lc3_decoder_frame_t *)a2dp_audio_lc3_frame_malloc(frame_len);
        //TRACE(3,"%s seq:%d len:%d", __func__, header->sequenceNumber, frame_len);
        decoder_frame_p->header.sequenceNumber = header->sequenceNumber;
        decoder_frame_p->header.timestamp = header->timestamp;
        memcpy(decoder_frame_p->header.ptrData, buffer+2, frame_len);
        decoder_frame_p->header.dataLen = frame_len;
        a2dp_audio_list_append(list, decoder_frame_p);       
        return A2DP_DECODER_NO_ERROR;
    }else{
        TRACE(3,"%s list full current list_len:%d buff_len:%d", __func__, a2dp_audio_list_length(list), frame_len);
        nRet = A2DP_DECODER_MTU_LIMTER_ERROR;
    }

    return nRet;
}

int a2dp_audio_lc3_discards_packet(uint32_t packets)
{
    int nRet = A2DP_DECODER_MEMORY_ERROR;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    a2dp_audio_lc3_decoder_frame_t* lc3_decoder_frame_p = NULL;

    if (packets <= a2dp_audio_list_length(list))
    {
        for (uint8_t i=0; i<packets; i++)
        {
            node = a2dp_audio_list_begin(list);
            lc3_decoder_frame_p = (a2dp_audio_lc3_decoder_frame_t *)a2dp_audio_list_node(node);
            a2dp_audio_list_remove(list, lc3_decoder_frame_p);
        }
        nRet = A2DP_DECODER_NO_ERROR;
    }
    TRACE(3,"%s packets:%d nRet:%d", __func__, packets, nRet);
    return nRet;

}

int a2dp_audio_lc3_headframe_info_get(A2DP_AUDIO_HEADFRAME_INFO_T* headframe_info)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    a2dp_audio_lc3_decoder_frame_t *decoder_frame_p = NULL;

    if (a2dp_audio_list_length(list)){
        node = a2dp_audio_list_begin(list);                
        decoder_frame_p = (a2dp_audio_lc3_decoder_frame_t *)a2dp_audio_list_node(node);
        headframe_info->sequenceNumber = decoder_frame_p->header.sequenceNumber;
        headframe_info->timestamp = decoder_frame_p->header.timestamp;
        headframe_info->curSubSequenceNumber = 0;
        headframe_info->totalSubSequenceNumber = 0;
    }else{
        memset(headframe_info, 0, sizeof(A2DP_AUDIO_HEADFRAME_INFO_T));
    }

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_lc3_info_get(void *info)
{
    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_lc3_init(A2DP_AUDIO_OUTPUT_CONFIG_T *config, void *context)
{
    TRACE(4,"%s ch:%d freq:%d bits:%d", __func__, config->num_channels, config->sample_rate, config->bits_depth);

    a2dp_audio_context_p = (A2DP_AUDIO_CONTEXT_T *)context;
    A2DP_AUDIO_DECODER_LASTFRAME_INFO_T *lstframes = &a2dp_audio_lc3_lastframe_info;
    memset(lstframes, 0, sizeof(A2DP_AUDIO_DECODER_LASTFRAME_INFO_T));    
    memcpy(&a2dp_audio_lc3_output_config, config, sizeof(A2DP_AUDIO_OUTPUT_CONFIG_T));
    lstframes->stream_info = a2dp_audio_lc3_output_config;
    lstframes->frame_samples = LC3_OUTPUT_FRAME_SAMPLES;
    lstframes->list_samples = LC3_OUTPUT_FRAME_SAMPLES;
    a2dp_audio_decoder_internal_lastframe_info_set(lstframes);

    a2dp_audio_lc3_decoder_init(config->sample_rate, config->num_channels, config->bits_depth);

    a2dp_audio_lc3_list_checker();

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_lc3_deinit(void)
{

    a2dp_audio_lc3_decoder_deinit();

    return A2DP_DECODER_NO_ERROR;
}

int  a2dp_audio_lc3_synchronize_packet(A2DP_AUDIO_SYNCFRAME_INFO_T *sync_info, uint32_t mask)
{
    int nRet = A2DP_DECODER_SYNC_ERROR;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    int list_len;
    a2dp_audio_lc3_decoder_frame_t* lc3_decoder_frame = NULL;

    list_len = a2dp_audio_list_length(list);

    for (uint16_t i=0; i<list_len; i++){
        node = a2dp_audio_list_begin(list);
        lc3_decoder_frame = (a2dp_audio_lc3_decoder_frame_t *)a2dp_audio_list_node(node);
        if  (A2DP_AUDIO_SYNCFRAME_CHK(lc3_decoder_frame->header.sequenceNumber         == sync_info->sequenceNumber,        A2DP_AUDIO_SYNCFRAME_MASK_SEQ,        mask)&&
             A2DP_AUDIO_SYNCFRAME_CHK(lc3_decoder_frame->header.curSubSequenceNumber   == sync_info->curSubSequenceNumber,  A2DP_AUDIO_SYNCFRAME_MASK_CURRSUBSEQ, mask)&&
             A2DP_AUDIO_SYNCFRAME_CHK(lc3_decoder_frame->header.totalSubSequenceNumber == sync_info->totalSubSequenceNumber,A2DP_AUDIO_SYNCFRAME_MASK_TOTALSUBSEQ,mask)){
            nRet = A2DP_DECODER_NO_ERROR;
            break;
        }
        a2dp_audio_list_remove(list, lc3_decoder_frame);
    }

    node = a2dp_audio_list_begin(list);
    if (node){
        lc3_decoder_frame = (a2dp_audio_lc3_decoder_frame_t *)a2dp_audio_list_node(node);
        TRACE(6,"%s nRet:%d SEQ:%d timestamp:%d %d/%d", __func__, nRet, lc3_decoder_frame->header.sequenceNumber, lc3_decoder_frame->header.timestamp,
                                                      lc3_decoder_frame->header.curSubSequenceNumber, lc3_decoder_frame->header.totalSubSequenceNumber);
    }else{
        TRACE(1,",%s nRet:%d", __func__, nRet);
    }

    return nRet;
}


int a2dp_audio_lc3_synchronize_dest_packet_mut(uint16_t packet_mut)
{
    list_node_t *node = NULL;
    uint32_t list_len = 0;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    a2dp_audio_lc3_decoder_frame_t *lc3_decoder_frame_p = NULL;

    list_len = a2dp_audio_list_length(list);
    if (list_len > packet_mut){
        do{        
            node = a2dp_audio_list_begin(list);            
            lc3_decoder_frame_p = (a2dp_audio_lc3_decoder_frame_t *)a2dp_audio_list_node(node);
            a2dp_audio_list_remove(list, lc3_decoder_frame_p);
        }while(a2dp_audio_list_length(list) > packet_mut);
    }

    TRACE(2,"%s list:%d", __func__, a2dp_audio_list_length(list));

    return A2DP_DECODER_NO_ERROR;
}


int a2dp_audio_lc3_convert_list_to_samples(uint32_t *samples)
{
    uint32_t list_len = 0;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;

    list_len = a2dp_audio_list_length(list);
    *samples = LC3_OUTPUT_FRAME_SAMPLES*list_len;

    TRACE(3, "%s list:%d samples:%d", __func__, list_len, *samples);

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_lc3_discards_samples(uint32_t samples)
{
    int nRet = A2DP_DECODER_SYNC_ERROR;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    a2dp_audio_lc3_decoder_frame_t *lc3_decoder_frame_p = NULL;
    list_node_t *node = NULL;
    int need_remove_list = 0;
    uint32_t list_samples = 0;
    ASSERT(!(samples%LC3_OUTPUT_FRAME_SAMPLES), "%s samples err:%d", __func__, samples);

    a2dp_audio_lc3_convert_list_to_samples(&list_samples);
    if (list_samples >= samples){
        need_remove_list = samples/LC3_OUTPUT_FRAME_SAMPLES;
        for (int i=0; i<need_remove_list; i++){
            node = a2dp_audio_list_begin(list);
            lc3_decoder_frame_p = (a2dp_audio_lc3_decoder_frame_t *)a2dp_audio_list_node(node);
            a2dp_audio_list_remove(list, lc3_decoder_frame_p);
        }
        nRet = A2DP_DECODER_NO_ERROR;
    }
    return nRet;
}

extern const A2DP_AUDIO_DECODER_T a2dp_audio_lc3_decoder_config = {
    {44100, 2, 16},
    1,{0},
    a2dp_audio_lc3_init,
    a2dp_audio_lc3_deinit,
    a2dp_audio_lc3_decode_frame,
    a2dp_audio_lc3_preparse_packet,
    a2dp_audio_lc3_store_packet,
    a2dp_audio_lc3_discards_packet,
    a2dp_audio_lc3_synchronize_packet,
    a2dp_audio_lc3_synchronize_dest_packet_mut,
    a2dp_audio_lc3_convert_list_to_samples,
    a2dp_audio_lc3_discards_samples,
    a2dp_audio_lc3_headframe_info_get,
    a2dp_audio_lc3_info_get,
    a2dp_audio_lc3_free,
} ;

