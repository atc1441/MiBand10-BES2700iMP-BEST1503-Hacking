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
#include "hal_timer.h"
#include "a2dp_decoder_internal.h"
#include "app_a2dp.h"

typedef struct
{
    uint16_t sequenceNumber;
    uint32_t timestamp;
    uint16_t curSubSequenceNumber;
    uint16_t totalSubSequenceNumber;
    uint8_t *buffer;
    uint32_t buffer_len;
} a2dp_audio_lhdc_decoder_frame_t;

uint32_t *lhdcv5_ptr = NULL;

#define LHDC_MTU_LIMITER (120)
//#define BES_MEASURE_DECODE_TIME

#define A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES (lhdcv5_util_dec_get_sample_size(lhdcv5_ptr))

#define A2DP_LHDC_DEFAULT_LATENCY (1)

#define LHDC_READBUF_SIZE (626)
#define LHDC_READBUF_SIZE_MONO (314)

#define A2DP_LHDC_HDR_F_MSK 0x80
#define A2DP_LHDC_HDR_S_MSK 0x40
#define A2DP_LHDC_HDR_L_MSK 0x20
#define A2DP_LHDC_HDR_FLAG_MSK (A2DP_LHDC_HDR_F_MSK | A2DP_LHDC_HDR_S_MSK | A2DP_LHDC_HDR_L_MSK)

#define A2DP_LHDC_HDR_LATENCY_LOW   0x00
#define A2DP_LHDC_HDR_LATENCY_MID   0x01
#define A2DP_LHDC_HDR_LATENCY_HIGH  0x02
#define A2DP_LHDC_HDR_LATENCY_MASK  (A2DP_LHDC_HDR_LATENCY_MID | A2DP_LHDC_HDR_LATENCY_HIGH)

#define A2DP_LHDC_HDR_FRAME_NO_MASK 0xfc

typedef enum
{
    VERSION_2 = 200,
    VERSION_3 = 300,
    VERSION_4 = 400,
    VERSION_LLAC = 500,    //LLAC
    VERSION_5 = 550
} lhdc_ver_t;

typedef enum {
    LHDC_OUTPUT_STEREO = 0,
    LHDC_OUTPUT_LEFT_CAHNNEL,
    LHDC_OUTPUT_RIGHT_CAHNNEL,
} lhdc_channel_t;

/**
 * get lhdc frame header
 */

/**
    LHDC frame
*/
typedef struct _lhdc_frame_Info
{
    uint32_t frame_len;
    uint32_t isSplit;
    uint32_t isLeft;
} lhdc_frame_Info_t;

#ifdef A2DP_CP_ACCEL
struct A2DP_CP_LHDC_IN_FRM_INFO_T
{
    uint16_t sequenceNumber;
    uint32_t timestamp;
    uint16_t curSubSequenceNumber;
    uint16_t totalSubSequenceNumber;
    A2DP_AUDIO_CHANNEL_SELECT_E chnl_sel;
};

struct A2DP_CP_LHDC_OUT_FRM_INFO_T
{
    struct A2DP_CP_LHDC_IN_FRM_INFO_T in_info;
    uint16_t frame_samples;
    uint16_t decoded_frames;
    uint16_t frame_idx;
    uint16_t pcm_len;
};
#endif

extern "C"
{
    typedef struct bes_bt_local_info_t
    {
        uint8_t bt_addr[BTIF_BD_ADDR_SIZE];
        const char *bt_name;
        uint8_t bt_len;
        uint8_t ble_addr[BTIF_BD_ADDR_SIZE];
        const char *ble_name;
        uint8_t ble_len;
    } bes_bt_local_info;

    typedef void (*print_log_fp)(char*  msg);
    typedef int (*LHDC_GET_BT_INFO)(bes_bt_local_info *bt_info);

    int32_t lhdcv5_util_init_decoder(uint32_t *ptr, uint32_t bitPerSample, uint32_t sampleRate, uint32_t audioFormat, uint32_t frm_duration, uint32_t lossless_enable, lhdc_ver_t version);
    int32_t lhdcv5_util_dec_process(uint32_t *ptr, uint8_t * pOutBuf, uint8_t * pInput, uint32_t InLen, uint32_t *OutLen);
    bool lhdcv5_util_set_license(uint32_t *ptr, uint8_t * licTable, LHDC_GET_BT_INFO pFunc);
    int32_t lhdcv5_util_set_license_check_period (uint32_t *ptr, uint8_t period);
    char *lhdcv5_util_dec_get_version();
    int32_t lhdcv5_util_dec_destroy(uint32_t *ptr);
    void lhdcv5_util_dec_register_log_cb(print_log_fp cb);
    int32_t lhdcv5_util_dec_get_sample_size (uint32_t *ptr);
    int32_t lhdcv5_util_dec_fetch_frame_info(uint32_t *ptr, uint8_t *frameData, uint32_t frameDataLen, lhdc_frame_Info_t *frameInfo);
    int32_t lhdcv5_util_dec_channel_selsect(uint32_t *ptr, lhdc_channel_t channel_type);
    int32_t lhdcv5_util_dec_get_mem_req(lhdc_ver_t version, uint32_t sampleRate, uint32_t audioFormat, uint32_t lossless_enable, uint32_t *mem_req_bytes);
}

static A2DP_AUDIO_CONTEXT_T *a2dp_audio_context_p = NULL;
static A2DP_AUDIO_DECODER_LASTFRAME_INFO_T a2dp_audio_lhdc_lastframe_info;
static A2DP_AUDIO_OUTPUT_CONFIG_T a2dp_audio_lhdc_output_config;
static uint16_t lhdc_mtu_limiter = LHDC_MTU_LIMITER;

static uint8_t serial_no;

static uint32_t lhdc_total_frame_nb = 0;


extern const char testkey_bin[];
extern int bes_bt_local_info_get(bes_bt_local_info *local_info);

//Local API declare
void sav_lhdcv5_log_bytes_len(uint32_t bytes_len);
int a2dp_audio_lhdcv5_channel_select(A2DP_AUDIO_CHANNEL_SELECT_E chnl_sel);
int a2dp_audio_lhdcv5_is_left(void);

#define STATISTICS_UPDATE_INTERVAL   1000 // in ms
#define INTERVAL_TIMEBASE            1000 //in ms
typedef struct
{
    uint32_t sum_bytes;
    uint32_t last_times;        //in ms
    float last_avg_val;
    int32_t update_interval;    //in ms
} LHDC_TRACFIC_STATISTICS;

static LHDC_TRACFIC_STATISTICS statistic = {0, 0, 0, STATISTICS_UPDATE_INTERVAL};

//static void print_log_cb(char*  msg){
    //TRACE_A2DP_DECODER_I("%s", msg);
//}

void sav_lhdcv5_log_bytes_len(uint32_t bytes_len)
{
    uint32_t time_current = GET_CURRENT_MS();
    float time_diff = time_current - statistic.last_times;
    statistic.sum_bytes += bytes_len;
    if (time_diff >= statistic.update_interval)
    {

        statistic.last_avg_val = ((float)(statistic.sum_bytes * 8) / 1000) / (time_diff / INTERVAL_TIMEBASE);
        TRACE_A2DP_DECODER_I("Avarage rate about %d kbps", (int)statistic.last_avg_val);

        statistic.sum_bytes = 0;
        statistic.last_times = time_current;
    }
}

static void *a2dp_audio_lhdc_frame_malloc(uint32_t packet_len)
{
    a2dp_audio_lhdc_decoder_frame_t *decoder_frame_p = NULL;
    uint8_t *buffer = NULL;

    buffer = (uint8_t *)a2dp_audio_heap_malloc(packet_len);
    decoder_frame_p = (a2dp_audio_lhdc_decoder_frame_t *)a2dp_audio_heap_malloc(sizeof(a2dp_audio_lhdc_decoder_frame_t));
    decoder_frame_p->buffer = buffer;
    decoder_frame_p->buffer_len = packet_len;
    return (void *)decoder_frame_p;
}

static void initial_lhdc_assemble_packet(bool splitFlg)
{
    serial_no = 0xff;
}

/**
 * A2DP packet 组包
 */
int assemble_lhdcv5_packet(uint8_t *input, uint32_t input_len, uint8_t **pLout, uint32_t *pLlen)
{
    uint8_t hdr = 0, seqno = 0xff;
    int ret = -1;
    uint32_t status = 0;

    hdr = (*input);
    input++;
    seqno = (*input);
    input++;
    input_len -= 2;

    //Check latency and update value when changed.
    status = hdr & A2DP_LHDC_HDR_LATENCY_MASK;

    //Get number of frame in packet.
    status = (hdr & A2DP_LHDC_HDR_FRAME_NO_MASK) >> 2;
    if (status <= 0)
    {
        TRACE_A2DP_DECODER_I("No any frame in packet.");
        return 0;
    }
    lhdc_total_frame_nb = status;
    if (seqno != serial_no)
    {
        TRACE_A2DP_DECODER_I("Packet lost! now(%d), expect(%d)", seqno, serial_no);
    }
    serial_no = seqno + 1;

    sav_lhdcv5_log_bytes_len(input_len);

    if (pLlen && pLout)
    {
        *pLlen = input_len;
        *pLout = input;
    }
    ret = lhdc_total_frame_nb;

    return ret;
}

#if 0
static int parse_lhdc_info(uint8_t *in, lhdc_frame_Info_t *h)
{
#define LHDC_HDR_LEN 4
    uint32_t hdr = 0;
    int ret = -1;
    memcpy(&hdr, in, LHDC_HDR_LEN);
    h->frame_len = (int)((hdr >> 8) & 0x1fff);
    h->isSplit = ((hdr & 0x00600000) == 0x00600000);
    h->isLeft = ((hdr & 0xf) == 0);

    if ((hdr & 0xff000000) != 0x4c000000)
    {
    }
    else
    {
        ret = 0;
    }
    return ret;
}
#endif

#ifdef A2DP_CP_ACCEL
static bool cp_codec_reset;
extern "C" uint32_t get_in_cp_frame_cnt(void);
extern "C" unsigned int set_cp_reset_flag(uint8_t evt);
extern uint32_t app_bt_stream_get_dma_buffer_samples(void);

//LHDC V4 Ext function
#define LHDC_EXT_FLAGS_LLAC   0x04
#define LHDC_EXT_FLAGS_V4     0x40

#define LHDC_EXT_FLAGS_JAS    0x01
#define LHDC_EXT_FLAGS_AR     0x02
#define LHDC_EXT_FLAGS_MQA    0x08
#define LHDC_EXT_FLAGS_MBR    0x10
#define LHDC_EXT_FLAGS_LARC   0x20
extern bool a2dp_lhdc_get_ext_flags(uint32_t flags);

int a2dp_cp_lhdcv5_cp_decode(void);

TEXT_LHDCV5_LOC
static int a2dp_cp_lhdc_after_cache_underflow(void)
{
    int ret = 0;
#ifdef A2DP_CP_ACCEL
    cp_codec_reset = true;
#endif
    return ret;
}

//uint32_t demoTimer = 0;
static int a2dp_cp_lhdc_mcu_decode(uint8_t *buffer, uint32_t buffer_bytes)
{
    a2dp_audio_lhdc_decoder_frame_t *lhdc_decoder_frame_p = NULL;
    list_node_t *node = NULL;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    int ret, dec_ret;
    struct A2DP_CP_LHDC_IN_FRM_INFO_T in_info;
    struct A2DP_CP_LHDC_OUT_FRM_INFO_T *p_out_info;

    uint8_t *out;
    uint32_t out_len = 0;
    uint32_t out_frame_len = 0;
    uint32_t cp_buffer_frames_max = 0;

    cp_buffer_frames_max = app_bt_stream_get_dma_buffer_samples() / 2;

    if (cp_buffer_frames_max % (a2dp_audio_lhdc_lastframe_info.frame_samples))
    {
        cp_buffer_frames_max = cp_buffer_frames_max / (a2dp_audio_lhdc_lastframe_info.frame_samples) + 1;
    }
    else
    {
        cp_buffer_frames_max = cp_buffer_frames_max / (a2dp_audio_lhdc_lastframe_info.frame_samples);
    }

    out_frame_len = sizeof(*p_out_info) + buffer_bytes;

    ret = a2dp_cp_decoder_init(out_frame_len, cp_buffer_frames_max * 8);
    if (ret)
    {
        TRACE_A2DP_DECODER_I("%s: a2dp_cp_decoder_init() failed: ret=%d", __func__, ret);
        set_cp_reset_flag(true);
        return A2DP_DECODER_DECODE_ERROR;
    }

    while ((node = a2dp_audio_list_begin(list)) != NULL)
    {
        lhdc_decoder_frame_p = (a2dp_audio_lhdc_decoder_frame_t *)a2dp_audio_list_node(node);
        in_info.sequenceNumber = lhdc_decoder_frame_p->sequenceNumber;
        in_info.timestamp = lhdc_decoder_frame_p->timestamp;
        in_info.curSubSequenceNumber = lhdc_decoder_frame_p->curSubSequenceNumber;
        in_info.totalSubSequenceNumber = lhdc_decoder_frame_p->totalSubSequenceNumber;
        in_info.chnl_sel = a2dp_audio_context_p->chnl_sel;

        ret = a2dp_cp_put_in_frame(&in_info, sizeof(in_info), lhdc_decoder_frame_p->buffer, lhdc_decoder_frame_p->buffer_len);
        if (ret)
        {
            //TRACE_A2DP_DECODER_I("%s  piff  !!!!!!ret: %d ",__func__, ret);
            break;
        }

        a2dp_audio_list_remove(list, lhdc_decoder_frame_p);
        node = a2dp_audio_list_begin(list);
    }

    ret = a2dp_cp_get_full_out_frame((void **)&out, &out_len);

    if (ret && ret != 1)
    {
            TRACE_A2DP_DECODER_I("%s %d cp find cache underflow(%d)", __func__, __LINE__, ret);
            TRACE_A2DP_DECODER_I("aud_list_len:%d. cp_get_in_frame:%d", a2dp_audio_list_length(list), get_in_cp_frame_cnt());
            a2dp_cp_lhdc_after_cache_underflow();
            return A2DP_DECODER_CACHE_UNDERFLOW_ERROR;
    }

    if (out_len == 0)
    {
        memset(buffer, 0, buffer_bytes);
        a2dp_cp_consume_full_out_frame();
        TRACE_A2DP_DECODER_I("%s  olz!!!%d ", __func__, __LINE__);
        return A2DP_DECODER_NO_ERROR;
    }
    if (out_len != out_frame_len)
    {
        TRACE_A2DP_DECODER_I("%s: Bad out len %u (should be %u)", __func__, out_len, out_frame_len);
        set_cp_reset_flag(true);
        return A2DP_DECODER_DECODE_ERROR;
    }
    p_out_info = (struct A2DP_CP_LHDC_OUT_FRM_INFO_T *)out;
    if (p_out_info->pcm_len)
    {
        a2dp_audio_lhdc_lastframe_info.chnl_sel =  p_out_info->in_info.chnl_sel;
        a2dp_audio_lhdc_lastframe_info.sequenceNumber = p_out_info->in_info.sequenceNumber;
        a2dp_audio_lhdc_lastframe_info.timestamp = p_out_info->in_info.timestamp;
        a2dp_audio_lhdc_lastframe_info.curSubSequenceNumber = p_out_info->in_info.curSubSequenceNumber;
        a2dp_audio_lhdc_lastframe_info.totalSubSequenceNumber = p_out_info->in_info.totalSubSequenceNumber;
        a2dp_audio_lhdc_lastframe_info.frame_samples = p_out_info->frame_samples;
        a2dp_audio_lhdc_lastframe_info.decoded_frames += p_out_info->decoded_frames;
        a2dp_audio_lhdc_lastframe_info.undecode_frames =
            a2dp_audio_list_length(list) + a2dp_cp_get_in_frame_cnt_by_index(p_out_info->frame_idx) - 1;
        a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_lhdc_lastframe_info);
#if 0
        TRACE_A2DP_DECODER_I("lhdc_decoder seq:%d cursub:%d ttlsub:%d decoded:%d/%d",
                                    a2dp_audio_lhdc_lastframe_info.sequenceNumber,
                                    a2dp_audio_lhdc_lastframe_info.curSubSequenceNumber,
                                    a2dp_audio_lhdc_lastframe_info.totalSubSequenceNumber,
                                    a2dp_audio_lhdc_lastframe_info.decoded_frames,
                                    a2dp_audio_lhdc_lastframe_info.undecode_frames);
#endif
    }

    if (p_out_info->pcm_len == buffer_bytes)
    {
        memcpy(buffer, p_out_info + 1, p_out_info->pcm_len);
        dec_ret = A2DP_DECODER_NO_ERROR;
    }
    else
    {
        TRACE_A2DP_DECODER_I("%s  %d cp decoder error  !!!!!!", __func__, __LINE__);
        set_cp_reset_flag(true);
        return A2DP_DECODER_DECODE_ERROR;
    }

    ret = a2dp_cp_consume_full_out_frame();
    if (ret)
    {

        TRACE_A2DP_DECODER_I("%s: a2dp_cp_consume_full_out_frame() failed: ret=%d", __func__, ret);
        set_cp_reset_flag(true);
        return A2DP_DECODER_DECODE_ERROR;
    }
    return dec_ret;
}

#ifdef __CP_EXCEPTION_TEST__
static bool _cp_assert = false;
int cp_assert(void)
{
    _cp_assert = true;
    return 0;
}
#endif

#define LHDCV5_DECODED_FRAME_SIZE (lhdcv5_util_dec_get_sample_size(lhdcv5_ptr))

TEXT_LHDCV5_LOC
int a2dp_cp_lhdcv5_cp_decode(void)
{
    int ret;
    enum CP_EMPTY_OUT_FRM_T out_frm_st;
    uint8_t *out;
    uint32_t out_len;
    uint8_t *dec_start;
    uint32_t dec_len;
    struct A2DP_CP_LHDC_IN_FRM_INFO_T *p_in_info;
    struct A2DP_CP_LHDC_OUT_FRM_INFO_T *p_out_info;
    uint8_t *in_buf;
    uint32_t in_len;

    int32_t dec_sum;
    uint32_t lhdc_out_len = 0;
    
    if (cp_codec_reset) {
        cp_codec_reset = false;
    }

#ifdef __CP_EXCEPTION_TEST__
    if (_cp_assert)
    {
        _cp_assert = false;
        *(int *)0 = 1;
        //ASSERT(0, "ASSERT  %s %d", __func__, __LINE__);
    }
#endif

//    TRACE_A2DP_DECODER_I("Run %s", __func__);
    out_frm_st = a2dp_cp_get_emtpy_out_frame((void **)&out, &out_len);

    if (out_frm_st != CP_EMPTY_OUT_FRM_OK && out_frm_st != CP_EMPTY_OUT_FRM_WORKING)
    {
        return out_frm_st;
    }
    ASSERT(out_len > sizeof(*p_out_info), "%s: Bad out_len %u (should > %u)", __func__, out_len, sizeof(*p_out_info));

    p_out_info = (struct A2DP_CP_LHDC_OUT_FRM_INFO_T *)out;
    if (out_frm_st == CP_EMPTY_OUT_FRM_OK)
    {
        p_out_info->pcm_len = 0;
        p_out_info->decoded_frames = 0;
    }

    ASSERT(out_len > sizeof(*p_out_info) + p_out_info->pcm_len, "%s: Bad out_len %u (should > %u + %u)", __func__, out_len, sizeof(*p_out_info), p_out_info->pcm_len);

    dec_start = (uint8_t *)(p_out_info + 1) + p_out_info->pcm_len;
    dec_len = out_len - (dec_start - (uint8_t *)out);

    dec_sum = 0;

    while (dec_sum < (int32_t)dec_len)
    {
        uint32_t lhdc_decode_temp = 0;

        ret = a2dp_cp_get_in_frame((void **)&in_buf, &in_len);

        if (ret)
        {
            TRACE_A2DP_DECODER_I("cp_get_int_frame fail, ret=%d", ret);
            return 4;
        }

        ASSERT(in_len > sizeof(*p_in_info), "%s: Bad in_len %u (should > %u)", __func__, in_len, sizeof(*p_in_info));

        p_in_info = (struct A2DP_CP_LHDC_IN_FRM_INFO_T *)in_buf;
        in_buf += sizeof(*p_in_info);
        in_len -= sizeof(*p_in_info);

        //TRACE_A2DP_DECODER_I("IN");
        //DUMP8("%02x ", in_buf, 16);
        //lhdcPutData(in_buf, in_len);
        //TRACE_A2DP_DECODER_I("%s:put length=%d, dec_len=%d", __func__, in_len, dec_len);

        #if 0
        uint32_t loop_cnt = 0;

        do
        {
            //TRACE_A2DP_DECODER_I("loop %d , dec_start %p, dec_sum %d, lhdc_decode_temp %d", loop_cnt, dec_start, dec_sum, lhdc_decode_temp);
            if (lhdc_out_len > 0)
            {
                lhdc_decode_temp += lhdc_out_len;
                loop_cnt++;
            }
            else
            {
                //TRACE_A2DP_DECODER_I("decodeProcess error!!! ret=%d", lhdc_out_len);
                break;
            }
        } while (lhdc_decode_temp < dec_len && lhdc_decode_temp > 0);
        #endif

        //lhdc_out_len = lhdcDecodeProcess(dec_start + dec_sum);
#ifdef BES_MEASURE_DECODE_TIME
        uint32_t start_time = hal_sys_timer_get();
#endif
        int32_t ret = 0;
        ret = lhdcv5_util_dec_process(lhdcv5_ptr, dec_start + dec_sum, in_buf, in_len, &lhdc_out_len);

#ifdef BES_MEASURE_DECODE_TIME
        uint32_t end_time = hal_sys_timer_get();
        uint32_t deltaMs = TICKS_TO_US(end_time - start_time);
        TRACE_A2DP_DECODER_I("%s: LHDC Decode time %d uS", __func__, deltaMs);
#endif
        //lhdc_decode_temp = dec_len;
        //memset(dec_start, 0, lhdc_decode_temp);

        //TRACE_A2DP_DECODER_I("%s:decode loop run times=%d, lhdc_decode_temp=%d", __func__, loop_cnt, lhdc_decode_temp);
        //TRACE_A2DP_DECODER_I("OUT");
        //DUMP8("%02x ", dec_start + dec_sum, 8);
        dec_sum += lhdc_out_len;
#if 0
        TRACE_A2DP_DECODER_I("lhdc_cp_decode seq:%d len:%d:%d err:%d", p_in_info->sequenceNumber,
                                                       dec_sum, dec_len,
                                                       lhdc_out_len);
#endif
        if ((lhdc_decode_temp % LHDCV5_DECODED_FRAME_SIZE))
        {
            TRACE_A2DP_DECODER_I("error!!! dec_sum: %d decode_temp: %d", dec_sum, lhdc_decode_temp);
            return -1;
        }

        ret = a2dp_cp_consume_in_frame();

        if (ret != 0)
        {
            TRACE_A2DP_DECODER_I("%s: a2dp_cp_consume_in_frame() failed: ret=%d", __func__, ret);
        }
        ASSERT(ret == 0, "%s: a2dp_cp_consume_in_frame() failed: ret=%d", __func__, ret);

        memcpy(&p_out_info->in_info, p_in_info, sizeof(*p_in_info));
        p_out_info->decoded_frames++;
        p_out_info->frame_samples = A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES;
        p_out_info->frame_idx = a2dp_cp_get_in_frame_index();
    }

    if ((dec_sum % LHDCV5_DECODED_FRAME_SIZE))
    {
        TRACE_A2DP_DECODER_I("error!!! dec_sum:%d  != dec_len:%d", dec_sum, dec_len);
        ASSERT(0, "%s", __func__);
    }

    p_out_info->pcm_len += dec_sum;

    if (out_len <= sizeof(*p_out_info) + p_out_info->pcm_len)
    {
        ret = a2dp_cp_consume_emtpy_out_frame();
        ASSERT(ret == 0, "%s: a2dp_cp_consume_emtpy_out_frame() failed: ret=%d", __func__, ret);
    }

    return 0;
}
#endif

#if 1
static int a2dp_audio_lhdc_list_checker(void)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    a2dp_audio_lhdc_decoder_frame_t *lhdc_decoder_frame_p = NULL;
    int cnt = 0;

    uint32_t lhdc_readbuf_size = LHDC_READBUF_SIZE;
    if(a2dp_audio_lhdcv5_is_left() == 1 || a2dp_audio_lhdcv5_is_left() == 2)//LEFT or RIGHT channel
    {
        lhdc_readbuf_size = LHDC_READBUF_SIZE_MONO;
    }
    do
    {
        lhdc_decoder_frame_p = (a2dp_audio_lhdc_decoder_frame_t *)a2dp_audio_lhdc_frame_malloc(lhdc_readbuf_size);
        if (lhdc_decoder_frame_p)
        {
            a2dp_audio_list_append(list, lhdc_decoder_frame_p);
        }
        cnt++;
    } while (lhdc_decoder_frame_p && cnt < LHDC_MTU_LIMITER);

    do
    {
        node = a2dp_audio_list_begin(list);
        if (node)
        {
            lhdc_decoder_frame_p = (a2dp_audio_lhdc_decoder_frame_t *)a2dp_audio_list_node(node);
            a2dp_audio_list_remove(list, lhdc_decoder_frame_p);
        }
    } while (node);

    TRACE_A2DP_DECODER_I("%s cnt:%d list:%d", __func__, cnt, a2dp_audio_list_length(list));

    return 0;
}
#endif

int a2dp_audio_lhdcv5_mcu_decode_frame(uint8_t *buffer, uint32_t buffer_bytes)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    a2dp_audio_lhdc_decoder_frame_t *lhdc_decoder_frame_p = NULL;

    bool cache_underflow = false;
    int output_byte = 0;

    uint32_t lhdc_out_len = 0;

    node = a2dp_audio_list_begin(list);
    if (!node)
    {
        TRACE_A2DP_DECODER_I("lhdc_decode cache underflow");
        cache_underflow = true;
        goto exit;
    }
    else
    {
        lhdc_decoder_frame_p = (a2dp_audio_lhdc_decoder_frame_t *)a2dp_audio_list_node(node);

        #if 0
        do
        {
            lhdc_out_len = lhdcDecodeProcess(buffer + output_byte);
            if (lhdc_out_len > 0)
            {
                output_byte += lhdc_out_len;
            }
            else
            {
                break;
            }
        } while (output_byte < (int)buffer_bytes && output_byte > 0);
        #endif

        if (output_byte != (int)buffer_bytes)
        {
            TRACE_A2DP_DECODER_I("[warning] lhdc_decode_frame output_byte:%d lhdc_out_len:%d buffer_bytes:%d", output_byte, lhdc_out_len, buffer_bytes);
            TRACE_A2DP_DECODER_I("[warning] lhdc_decode_frame frame_len:%d rtp seq:%d timestamp:%d decoder_frame:%d/%d ",
                      lhdc_decoder_frame_p->buffer_len,
                      lhdc_decoder_frame_p->sequenceNumber,
                      lhdc_decoder_frame_p->timestamp,
                      lhdc_decoder_frame_p->curSubSequenceNumber,
                      lhdc_decoder_frame_p->totalSubSequenceNumber);
            output_byte = buffer_bytes;
            int32_t dump_byte = lhdc_decoder_frame_p->buffer_len;
            int32_t dump_offset = 0;
            while (1)
            {
                uint32_t dump_byte_output = 0;
                dump_byte_output = dump_byte > 32 ? 32 : dump_byte;
                DUMP8("%02x ", lhdc_decoder_frame_p->buffer + dump_offset, dump_byte_output);
                dump_offset += dump_byte_output;
                dump_byte -= dump_byte_output;
                if (dump_byte <= 0)
                {
                    break;
                }
            }
            ASSERT(0, "%s", __func__);
        }
        a2dp_audio_lhdc_lastframe_info.chnl_sel = a2dp_audio_context_p->chnl_sel;
        a2dp_audio_lhdc_lastframe_info.sequenceNumber = lhdc_decoder_frame_p->sequenceNumber;
        a2dp_audio_lhdc_lastframe_info.timestamp = lhdc_decoder_frame_p->timestamp;
        a2dp_audio_lhdc_lastframe_info.curSubSequenceNumber = lhdc_decoder_frame_p->curSubSequenceNumber;
        a2dp_audio_lhdc_lastframe_info.totalSubSequenceNumber = lhdc_decoder_frame_p->totalSubSequenceNumber;
        a2dp_audio_lhdc_lastframe_info.frame_samples = A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES;
        a2dp_audio_lhdc_lastframe_info.decoded_frames++;
        a2dp_audio_lhdc_lastframe_info.undecode_frames = a2dp_audio_list_length(list) - 1;
        a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_lhdc_lastframe_info);
        a2dp_audio_list_remove(list, lhdc_decoder_frame_p);
    }
exit:
    if (cache_underflow)
    {
        a2dp_audio_lhdc_lastframe_info.undecode_frames = 0;
        a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_lhdc_lastframe_info);
        output_byte = A2DP_DECODER_CACHE_UNDERFLOW_ERROR;
    }
    return output_byte;
}

int a2dp_audio_lhdcv5_decode_frame(uint8_t *buffer, uint32_t buffer_bytes)
{
#ifdef A2DP_CP_ACCEL
    return a2dp_cp_lhdc_mcu_decode(buffer, buffer_bytes);
#else
    return a2dp_audio_lhdcv5_mcu_decode_frame(buffer, buffer_bytes);
#endif
}

int a2dp_audio_lhdcv5_preparse_packet(btif_media_header_t *header, uint8_t *buffer, uint32_t buffer_bytes)
{
    a2dp_audio_lhdc_lastframe_info.sequenceNumber = header->sequenceNumber;
    a2dp_audio_lhdc_lastframe_info.timestamp = header->timestamp;
    a2dp_audio_lhdc_lastframe_info.curSubSequenceNumber = 0;
    a2dp_audio_lhdc_lastframe_info.totalSubSequenceNumber = 0;
    a2dp_audio_lhdc_lastframe_info.frame_samples = A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES;
    a2dp_audio_lhdc_lastframe_info.list_samples = A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES;
    a2dp_audio_lhdc_lastframe_info.decoded_frames = 0;
    a2dp_audio_lhdc_lastframe_info.undecode_frames = 0;
    a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_lhdc_lastframe_info);

    TRACE_A2DP_DECODER_I("%s seq:%d timestamp:%08x", __func__, header->sequenceNumber, header->timestamp);

    return A2DP_DECODER_NO_ERROR;
}

void a2dp_audio_lhdcv5_free(void *packet)
{
    a2dp_audio_lhdc_decoder_frame_t *decoder_frame_p = (a2dp_audio_lhdc_decoder_frame_t *)packet;
    a2dp_audio_heap_free(decoder_frame_p->buffer);
    a2dp_audio_heap_free(decoder_frame_p);
}

int a2dp_audio_lhdcv5_store_packet(btif_media_header_t *header, uint8_t *buffer, uint32_t buffer_bytes)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    int nRet = A2DP_DECODER_NO_ERROR;
    uint32_t frame_num = 0;
    uint32_t frame_cnt = 0;
    uint32_t lSize = 0;
    uint8_t *lPTR = NULL;
    lhdc_frame_Info_t lhdc_frame_Info;
    uint32_t ptr_offset = 0;

    //TRACE_A2DP_DECODER_I("run %s", __func__);
    if ((frame_num = assemble_lhdcv5_packet(buffer, buffer_bytes, &lPTR, &lSize)) > 0)
    {
        if (lPTR != NULL && lSize != 0)
        {
            ptr_offset = 0;
            //TRACE_A2DP_DECODER_I("%s: There are %d frames in packet, packet size %d", __func__, frame_num, lSize);
            while((lhdcv5_util_dec_fetch_frame_info(lhdcv5_ptr, lPTR + ptr_offset, lSize, &lhdc_frame_Info)==0) && ptr_offset < lSize && frame_cnt < frame_num)
            {
                //TRACE_A2DP_DECODER_I("%s: Save frame %d, frame len %d", __func__, frame_cnt, lhdc_frame_Info.frame_len);
                a2dp_audio_lhdc_decoder_frame_t *decoder_frame_p = NULL;
                if (!lhdc_frame_Info.frame_len)
                {
                    DUMP8("%02x ", lPTR + ptr_offset, 32);
                    ASSERT(0, "lhdc_frame_Info error frame_len:%d offset:%d ptr:%08x/%08x", lhdc_frame_Info.frame_len, ptr_offset, (uint32_t)buffer, (uint32_t)lPTR);
                }
                ASSERT(lhdc_frame_Info.frame_len <= (lSize - ptr_offset), "%s frame_len:%d ptr_offset:%d buffer_bytes:%d",
                       __func__, lhdc_frame_Info.frame_len, ptr_offset, lSize);
                if(a2dp_audio_lhdcv5_is_left() == 1)//LEFT channel
                {
                    if (a2dp_audio_list_length(list) < lhdc_mtu_limiter)
                    {
                        decoder_frame_p = (a2dp_audio_lhdc_decoder_frame_t *)a2dp_audio_lhdc_frame_malloc(lhdc_frame_Info.frame_len);
                    }
                    else
                    {
                        nRet = A2DP_DECODER_MTU_LIMTER_ERROR;
                        break;
                    }
                    frame_cnt++;

                    decoder_frame_p->sequenceNumber = header->sequenceNumber;
                    decoder_frame_p->timestamp = header->timestamp;
                    decoder_frame_p->curSubSequenceNumber = frame_cnt;
                    decoder_frame_p->totalSubSequenceNumber = frame_num;
                    memcpy(decoder_frame_p->buffer, lPTR + ptr_offset, lhdc_frame_Info.frame_len);
                    decoder_frame_p->buffer_len = lhdc_frame_Info.frame_len;
                    a2dp_audio_list_append(list, decoder_frame_p);
                }
                else if(a2dp_audio_lhdcv5_is_left() == 2)//RIGHT channel
                {
                    if (a2dp_audio_list_length(list) < lhdc_mtu_limiter)
                    {
                        decoder_frame_p = (a2dp_audio_lhdc_decoder_frame_t *)a2dp_audio_lhdc_frame_malloc(lhdc_frame_Info.frame_len);
                    }
                    else
                    {
                        nRet = A2DP_DECODER_MTU_LIMTER_ERROR;
                        break;
                    }
                    frame_cnt++;

                    decoder_frame_p->sequenceNumber = header->sequenceNumber;
                    decoder_frame_p->timestamp = header->timestamp;
                    decoder_frame_p->curSubSequenceNumber = frame_cnt;
                    decoder_frame_p->totalSubSequenceNumber = frame_num;
                    memcpy(decoder_frame_p->buffer, lPTR + ptr_offset, 2);//frame header
                    memcpy(decoder_frame_p->buffer+2, lPTR + ptr_offset+2, lhdc_frame_Info.frame_len-2);//R ch data
                    decoder_frame_p->buffer_len = lhdc_frame_Info.frame_len;
                    a2dp_audio_list_append(list, decoder_frame_p);
                }
                else//DO NOT SPLIT CHANNEL
                {
                    ASSERT(0, "error : DO NOT SPLIT CHANNEL");

                    //if (a2dp_audio_list_length(list) < lhdc_mtu_limiter)
                    //{
                    //    decoder_frame_p = (a2dp_audio_lhdc_decoder_frame_t *)a2dp_audio_lhdc_frame_malloc(lhdc_frame_Info.frame_len);
                    //}
                    //else
                    //{
                    //    nRet = A2DP_DECODER_MTU_LIMTER_ERROR;
                    //    break;
                    //}
                    //frame_cnt++;

                    //decoder_frame_p->sequenceNumber = header->sequenceNumber;
                    //decoder_frame_p->timestamp = header->timestamp;
                    //decoder_frame_p->curSubSequenceNumber = frame_cnt;
                    //decoder_frame_p->totalSubSequenceNumber = frame_num;
                    //memcpy(decoder_frame_p->buffer, lPTR + ptr_offset, lhdc_frame_Info.frame_len);
                    //decoder_frame_p->buffer_len = lhdc_frame_Info.frame_len;
                    //a2dp_audio_list_append(list, decoder_frame_p);
                }
                ptr_offset += (lhdc_frame_Info.frame_len-2)*2+2;
#if 0
                TRACE_A2DP_DECODER_I("lhdc_store_packet save seq:%d timestamp:%d len:%d lSize:%d frame_len:%d Split:%d/%d",
                                                                                                             header->sequenceNumber,
                                                                                                             header->timestamp,
                                                                                                             buffer_bytes,
                                                                                                             lSize,
                                                                                                             lhdc_frame_Info.frame_len,
                                                                                                             lhdc_frame_Info.isSplit,
                                                                                                             lhdc_frame_Info.isLeft);
#endif
            }
        }
    }
    else
    {
        //        TRACE_A2DP_DECODER_I("lhdc_store_packet skip seq:%d timestamp:%d len:%d l:%d", header->sequenceNumber, header->timestamp,buffer_bytes, lSize);
    }

    return nRet;
}

int a2dp_audio_lhdcv5_discards_packet(uint32_t packets)
{
#ifdef A2DP_CP_ACCEL
    a2dp_cp_reset_frame();
#endif

    int nRet = a2dp_audio_context_p->audio_decoder.audio_decoder_synchronize_dest_packet_mut(a2dp_audio_context_p->dest_packet_mut);

    return nRet;
}

static int a2dp_audio_lhdcv5_headframe_info_get(A2DP_AUDIO_HEADFRAME_INFO_T *headframe_info)
{
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    a2dp_audio_lhdc_decoder_frame_t *decoder_frame_p = NULL;

    if (a2dp_audio_list_length(list))
    {
        node = a2dp_audio_list_begin(list);
        decoder_frame_p = (a2dp_audio_lhdc_decoder_frame_t *)a2dp_audio_list_node(node);
        headframe_info->sequenceNumber = decoder_frame_p->sequenceNumber;
        headframe_info->timestamp = decoder_frame_p->timestamp;
        headframe_info->curSubSequenceNumber = 0;
        headframe_info->totalSubSequenceNumber = 0;
    }
    else
    {
        memset(headframe_info, 0, sizeof(A2DP_AUDIO_HEADFRAME_INFO_T));
    }

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_lhdcv5_info_get(void *info)
{
    return A2DP_DECODER_NO_ERROR;
}
extern uint8_t __lhdc_license_start[];

uint32_t a2dp_lhdc_config_bitrate_get(void);
int a2dp_audio_lhdcv5_init(A2DP_AUDIO_OUTPUT_CONFIG_T *config, void *context)
{
    uint8_t* pLHDC_lic = (uint8_t*)__lhdc_license_start;
    pLHDC_lic = pLHDC_lic + 0x98;
    TRACE_A2DP_DECODER_I("%s: lic = 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X", __func__, pLHDC_lic[0], pLHDC_lic[1], pLHDC_lic[2], pLHDC_lic[3], pLHDC_lic[4], pLHDC_lic[5]);
    TRACE_A2DP_DECODER_I("%s %s ch:%d freq:%d bits:%d bitrate:%d", __func__, lhdcv5_util_dec_get_version(),
          config->num_channels,
          config->sample_rate,
          config->bits_depth,
          a2dp_lhdc_config_bitrate_get());

    a2dp_audio_context_p = (A2DP_AUDIO_CONTEXT_T *)context;

    lhdc_ver_t version = VERSION_5;
    uint32_t size = 0;
    //lhdcv5_util_dec_register_log_cb(&print_log_cb);
    lhdcv5_util_dec_get_mem_req(version, config->sample_rate, 2, 0, &size);
    lhdcv5_ptr = (uint32_t *)a2dp_audio_heap_malloc(size);
    lhdcv5_util_set_license(lhdcv5_ptr, pLHDC_lic, bes_bt_local_info_get);
    TRACE_A2DP_DECODER_I("%s: LHDC V5 heap size %d", __func__, size);
    if (lhdcv5_ptr)
    {
        lhdcv5_util_init_decoder(lhdcv5_ptr, config->bits_depth, config->sample_rate, 2, 50, 0, version);
        lhdcv5_util_set_license_check_period(lhdcv5_ptr, 1);
    }


    memset(&a2dp_audio_lhdc_lastframe_info, 0, sizeof(A2DP_AUDIO_DECODER_LASTFRAME_INFO_T));
    memcpy(&a2dp_audio_lhdc_output_config, config, sizeof(A2DP_AUDIO_OUTPUT_CONFIG_T));
    a2dp_audio_lhdc_lastframe_info.stream_info = a2dp_audio_lhdc_output_config;
    a2dp_audio_lhdc_lastframe_info.frame_samples = A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES;
    a2dp_audio_lhdc_lastframe_info.list_samples = A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES;
    a2dp_audio_lhdc_lastframe_info.chnl_sel = a2dp_audio_context_p->chnl_sel;
    a2dp_audio_decoder_internal_lastframe_info_set(&a2dp_audio_lhdc_lastframe_info);

    initial_lhdc_assemble_packet(false);

#ifdef A2DP_CP_ACCEL
    int ret;
    cp_codec_reset = true;
    ret = a2dp_cp_init(a2dp_cp_lhdcv5_cp_decode, CP_PROC_DELAY_2_FRAMES);
    ASSERT(ret == 0, "%s: a2dp_cp_init() failed: ret=%d", __func__, ret);
#endif
    a2dp_audio_lhdc_list_checker();

    a2dp_audio_lhdcv5_channel_select(a2dp_audio_context_p->chnl_sel);
    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_lhdcv5_deinit(void)
{
#ifdef A2DP_CP_ACCEL
    a2dp_cp_deinit();
#endif

    lhdcv5_util_dec_destroy(lhdcv5_ptr);

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_lhdcv5_is_left(void){
    if(!a2dp_audio_context_p){
        return -1;
    }
    if(a2dp_audio_context_p->chnl_sel == A2DP_AUDIO_CHANNEL_SELECT_LCHNL){
        return 1;
    }
    if(a2dp_audio_context_p->chnl_sel == A2DP_AUDIO_CHANNEL_SELECT_RCHNL){
        return 2;
    }
    return 0;
}

int a2dp_audio_lhdcv5_synchronize_packet(A2DP_AUDIO_SYNCFRAME_INFO_T *sync_info, uint32_t mask)
{
    int nRet = A2DP_DECODER_SYNC_ERROR;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    list_node_t *node = NULL;
    int list_len;
    a2dp_audio_lhdc_decoder_frame_t *lhdc_decoder_frame;

#ifdef A2DP_CP_ACCEL
    a2dp_cp_reset_frame();
#endif

    list_len = a2dp_audio_list_length(list);

    for (uint16_t i = 0; i < list_len; i++)
    {
        node = a2dp_audio_list_begin(list);
        lhdc_decoder_frame = (a2dp_audio_lhdc_decoder_frame_t *)a2dp_audio_list_node(node);
        if (A2DP_AUDIO_SYNCFRAME_CHK(lhdc_decoder_frame->sequenceNumber == sync_info->sequenceNumber, A2DP_AUDIO_SYNCFRAME_MASK_SEQ, mask) &&
            A2DP_AUDIO_SYNCFRAME_CHK(lhdc_decoder_frame->curSubSequenceNumber == sync_info->curSubSequenceNumber, A2DP_AUDIO_SYNCFRAME_MASK_CURRSUBSEQ, mask) &&
            A2DP_AUDIO_SYNCFRAME_CHK(lhdc_decoder_frame->totalSubSequenceNumber == sync_info->totalSubSequenceNumber, A2DP_AUDIO_SYNCFRAME_MASK_TOTALSUBSEQ, mask))
        {
            nRet = A2DP_DECODER_NO_ERROR;
            break;
        }
        a2dp_audio_list_remove(list, lhdc_decoder_frame);
    }

    node = a2dp_audio_list_begin(list);
    if (node)
    {
        lhdc_decoder_frame = (a2dp_audio_lhdc_decoder_frame_t *)a2dp_audio_list_node(node);
        TRACE_A2DP_DECODER_I("%s nRet:%d SEQ:%d timestamp:%d %d/%d", __func__, nRet, lhdc_decoder_frame->sequenceNumber, lhdc_decoder_frame->timestamp,
              lhdc_decoder_frame->curSubSequenceNumber, lhdc_decoder_frame->totalSubSequenceNumber);
    }
    else
    {
        TRACE_A2DP_DECODER_I("%s nRet:%d", __func__, nRet);
    }

    return nRet;
}

int a2dp_audio_lhdcv5_synchronize_dest_packet_mut(uint16_t packet_mut)
{
    list_node_t *node = NULL;
    uint32_t list_len = 0;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    a2dp_audio_lhdc_decoder_frame_t *lhdc_decoder_frame_p = NULL;

    list_len = a2dp_audio_list_length(list);
    if (list_len > packet_mut)
    {
        do
        {
            node = a2dp_audio_list_begin(list);
            lhdc_decoder_frame_p = (a2dp_audio_lhdc_decoder_frame_t *)a2dp_audio_list_node(node);
            a2dp_audio_list_remove(list, lhdc_decoder_frame_p);
        } while (a2dp_audio_list_length(list) > packet_mut);
    }

    TRACE_A2DP_DECODER_I("%s list:%d", __func__, a2dp_audio_list_length(list));
    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_lhdcv5_convert_list_to_samples(uint32_t *samples)
{
    uint32_t list_len = 0;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;

    list_len = a2dp_audio_list_length(list);

    *samples = A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES * list_len;

    TRACE_A2DP_DECODER_I("%s list:%d samples:%d", __func__, list_len, *samples);

    return A2DP_DECODER_NO_ERROR;
}

int a2dp_audio_lhdcv5_discards_samples(uint32_t samples)
{
    int nRet = A2DP_DECODER_SYNC_ERROR;
    list_t *list = a2dp_audio_context_p->audio_datapath.input_raw_packet_list;
    a2dp_audio_lhdc_decoder_frame_t *lhdc_decoder_frame_p = NULL;
    list_node_t *node = NULL;
    int need_remove_list = 0;
    uint32_t list_samples = 0;

    ASSERT(!(samples % A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES), "%s samples err:%d", __func__, samples);

    a2dp_audio_lhdcv5_convert_list_to_samples(&list_samples);
    if (list_samples >= samples)
    {
        need_remove_list = samples / A2DP_LHDCV5_OUTPUT_FRAME_SAMPLES;
        for (int i = 0; i < need_remove_list; i++)
        {
            node = a2dp_audio_list_begin(list);
            lhdc_decoder_frame_p = (a2dp_audio_lhdc_decoder_frame_t *)a2dp_audio_list_node(node);
            a2dp_audio_list_remove(list, lhdc_decoder_frame_p);
        }
        nRet = A2DP_DECODER_NO_ERROR;

        node = a2dp_audio_list_begin(list);
        if (node) {
            lhdc_decoder_frame_p = (a2dp_audio_lhdc_decoder_frame_t *)a2dp_audio_list_node(node);
            TRACE_A2DP_DECODER_I("%s discard %d sample cur seq:%d", __func__, samples, lhdc_decoder_frame_p->sequenceNumber);
        }
    }

    return nRet;
}
int a2dp_audio_lhdcv5_channel_select(A2DP_AUDIO_CHANNEL_SELECT_E chnl_sel){
    //LHDC_OUTPUT_STEREO
    //LHDC_OUTPUT_LEFT_CAHNNEL
    //LHDC_OUTPUT_RIGHT_CAHNNEL
    lhdc_channel_t  channel_type;

    switch(chnl_sel){
        case A2DP_AUDIO_CHANNEL_SELECT_STEREO:
        channel_type = LHDC_OUTPUT_STEREO;
        TRACE_A2DP_DECODER_I("channel select stereo.");
        break;
        case A2DP_AUDIO_CHANNEL_SELECT_LCHNL:
        channel_type = LHDC_OUTPUT_LEFT_CAHNNEL;
        TRACE_A2DP_DECODER_I("channel select L channel.");
        break;
        case A2DP_AUDIO_CHANNEL_SELECT_RCHNL:
        channel_type = LHDC_OUTPUT_RIGHT_CAHNNEL;
        TRACE_A2DP_DECODER_I("channel select R channel.");
        break;

        default:
        case A2DP_AUDIO_CHANNEL_SELECT_LRMERGE:
        TRACE_A2DP_DECODER_I("Unsupported channel config(%d).", chnl_sel);
        return A2DP_DECODER_NOT_SUPPORT;
    }

    lhdcv5_util_dec_channel_selsect(lhdcv5_ptr, channel_type);

    return A2DP_DECODER_NO_ERROR;
}

extern const A2DP_AUDIO_DECODER_T a2dp_audio_lhdcv5_decoder_config = {
                                                        {96000, 2, 24},
                                                        1,{0},
                                                        a2dp_audio_lhdcv5_init,
                                                        a2dp_audio_lhdcv5_deinit,
                                                        a2dp_audio_lhdcv5_decode_frame,
                                                        a2dp_audio_lhdcv5_preparse_packet,
                                                        a2dp_audio_lhdcv5_store_packet,
                                                        a2dp_audio_lhdcv5_discards_packet,
                                                        a2dp_audio_lhdcv5_synchronize_packet,
                                                        a2dp_audio_lhdcv5_synchronize_dest_packet_mut,
                                                        a2dp_audio_lhdcv5_convert_list_to_samples,
                                                        a2dp_audio_lhdcv5_discards_samples,
                                                        a2dp_audio_lhdcv5_headframe_info_get,
                                                        a2dp_audio_lhdcv5_info_get,
                                                        a2dp_audio_lhdcv5_free,
                                                        a2dp_audio_lhdcv5_channel_select,
                                                     } ;

