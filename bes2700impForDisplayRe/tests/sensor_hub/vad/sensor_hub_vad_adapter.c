/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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

#ifdef VOICE_DETECTOR_EN
#include "string.h"
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "analog.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hwtimer_list.h"
#include "audioflinger.h"
#include "sens_msg.h"
#include "sensor_hub_core.h"
#include "sensor_hub_core_app.h"
#include "sensor_hub_vad_core.h"
#include "sensor_hub_vad_adapter.h"
#ifdef VAD_ADPT_TEST_ENABLE
#include "sensor_hub_vad_adapter_tc.h"
#endif
#include "app_sensor_hub.h"
#include "app_thread.h"
#include "app_voice_detector.h"

#define VAD_ADPT_DEBUG 0

//#define VAD_TEST_PATT_EN

#define ADPT_USE_VAD_PRIVATE_DATA

#define SYSFREQ_USER_APP_VAD_ADPT HAL_SYSFREQ_USER_APP_2

#if 1
#define ADPT_LOG TRACE
#else
#define ADPT_LOG(...) do{}while(0)
#endif

static enum voice_detector_id act_vd_id = VOICE_DETECTOR_ID_0;
static enum APP_VAD_ADPT_ID_T act_adpt_id;
static app_vad_adpt_info_list_t app_vad_adpt_info_list;

#define to_vad_adpt_list()           (&app_vad_adpt_info_list)
#define to_vad_adpt_info(l,i)        (&((l)->d[(i)]))
#define to_vad_stream_handler(l,i,p) (&((l)->d[(i)].stream_handlers[p]))
#define to_vad_adpt_state(l,i)       (&((l)->d[(i)].state))
#define to_vad_stream_cfg(l,i)       (&((l)->d[(i)].itf->stream_cfg))

#define ALIGNED4 ALIGNED(4)
#define NON_EXP_ALIGN(val, exp)         (((val) + ((exp) - 1)) / (exp) * (exp))

//FIXME: malloc buffer dyn
static uint8_t ALIGNED4 app_vad_adpt_kw_buf[VAD_ADPT_REC_BUF_SIZE];

static app_vad_adpt_record_t app_vad_adpt_record;
#define to_vad_adpt_record()       (&app_vad_adpt_record)
#define to_vad_adpt_rec(type) (&(app_vad_adpt_record.d[type]))

typedef void (*vad_adpt_timer_cb_t)(uint32_t period);

POSSIBLY_UNUSED static HWTIMER_ID vad_timer;
POSSIBLY_UNUSED static bool vad_adpt_timer_started = false;
POSSIBLY_UNUSED static uint32_t vad_adpt_timer_period = 0;
POSSIBLY_UNUSED static vad_adpt_timer_cb_t vad_adpt_timer_cb;

static uint32_t sample_bit_to_size(enum AUD_BITS_T bit)
{
    uint32_t samp_size;
    if (bit == AUD_BITS_16) {
        samp_size = 2;
    } else {
        samp_size = 4;
    }
    return samp_size;
}

static uint32_t app_vad_adapter_calc_buf_size(uint32_t sr, uint32_t size, uint32_t chnum, uint32_t frm)
{
    return sr/1000*size*chnum*frm;
}

static int vad_detector_act_cmd = 0;
static void vad_adapter_set_detector_act_cmd(int cmd)
{
    vad_detector_act_cmd = cmd;
}

static bool vad_adapter_clr_detector_act_cmd(int state)
{
    int cmd;
    bool done = false;
    uint32_t lock;

    lock = int_lock();
    cmd = vad_detector_act_cmd;
    int_unlock(lock);

    switch (cmd) {
    case VOICE_DET_EVT_VAD_START:
        if (state == VOICE_DET_STATE_VAD_START) {
            done = true;
        }
        break;
    case VOICE_DET_EVT_AUD_CAP_START:
        if (state == VOICE_DET_STATE_VAD_CLOSE) {
            done = true;
        }
        break;
    case VOICE_DET_EVT_CLOSE:
        if (state == VOICE_DET_STATE_EXIT) {
            done = true;
        }
        break;
    default:
        break;
    }
    if (done) {
        osDelay(20);
        app_sensor_hub_core_vad_send_evt_msg(SENS_EVT_ID_VAD_CMD_DONE, cmd, 0, 0);
    }
    return done;
}

/*
 * ADAPTER TIMER
 ****************************************************************************
 */
POSSIBLY_UNUSED static void vad_adapter_timer_callback(uint32_t period)
{
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();
    app_vad_adpt_state_t *state = to_vad_adpt_state(l,act_adpt_id);
    app_vad_adpt_record_t *rec = to_vad_adpt_record();

    ADPT_LOG(0, "%s: period=%d, vad_active=%d", __func__, period, state->vad_active);
    if (state->vad_active) {
        /* timer is timed up: send event msg to MCU core */
        rec->request = false;
        vad_adapter_set_detector_act_cmd(VOICE_DET_EVT_VAD_START);
        app_voice_detector_send_event(act_vd_id,VOICE_DET_EVT_VAD_START);
    }
}

POSSIBLY_UNUSED static void app_vad_adapter_time_timeout(void *param)
{
    if (vad_adpt_timer_cb) {
        vad_adpt_timer_cb(vad_adpt_timer_period);
    }
    vad_adpt_timer_started = false;
}

POSSIBLY_UNUSED static int app_vad_adapter_start_timer(uint32_t period, vad_adpt_timer_cb_t callback)
{
    if (!vad_adpt_timer_started) {
        if (vad_timer == NULL) {
            vad_timer = hwtimer_alloc(app_vad_adapter_time_timeout, NULL);
        }
        ADPT_LOG(0, "%s: period=%d", __func__, period);
        hwtimer_start(vad_timer, MS_TO_TICKS(period));
        vad_adpt_timer_cb = callback;
        vad_adpt_timer_period = period;
        vad_adpt_timer_started = true;
    }
    return 0;
}

POSSIBLY_UNUSED static int app_vad_adapter_stop_timer(void)
{
    if (vad_adpt_timer_started) {
        hwtimer_stop(vad_timer);
        vad_adpt_timer_started = false;
    }
    return 0;
}

POSSIBLY_UNUSED static void vad_adpt_codec_vad_hw_irq_handler(int find)
{
    ADPT_LOG(1, "%s: find=%d", __func__, find);

    app_sensor_hub_core_vad_send_evt_msg(SENS_EVT_ID_VAD_IRQ_TRIG, find, 0, 0);
}

POSSIBLY_UNUSED static void app_vad_adapter_init_vad_hw_cfg(struct AUD_VAD_CONFIG_T *c)
{
    c->type        = AUD_VAD_TYPE_DIG;
    c->sample_rate = AUD_SAMPRATE_16000;
    c->bits        = AUD_BITS_16;
    c->handler     = vad_adpt_codec_vad_hw_irq_handler;
    c->frame_len   = VAD_HW_PARAM_MIC_FRAME_LEN;
    c->mvad        = VAD_HW_PARAM_MVAD;
    c->sth         = VAD_HW_PARAM_STH;
    c->udc         = VAD_HW_PARAM_UDC;
    c->upre        = VAD_HW_PARAM_UPRE;
    c->dig_mode    = VAD_HW_PARAM_DIG_MODE;
    c->pre_gain    = VAD_HW_PARAM_PRE_GAIN;
    c->dc_bypass   = VAD_HW_PARAM_DC_BYPASS;
    c->frame_th[0] = VAD_HW_PARAM_FRAME_TH0;
    c->frame_th[1] = VAD_HW_PARAM_FRAME_TH0;
    c->frame_th[2] = VAD_HW_PARAM_FRAME_TH0;
    c->range[0]    = VAD_HW_PARAM_RANGE0;
    c->range[1]    = VAD_HW_PARAM_RANGE1;
    c->range[2]    = VAD_HW_PARAM_RANGE2;
    c->range[3]    = VAD_HW_PARAM_RANGE3;
    c->psd_th[0]   = VAD_HW_PARAM_PSD_TH0;
    c->psd_th[1]   = VAD_HW_PARAM_PSD_TH1;

    ADPT_LOG(0, "%s: frame_len=%d, mvad=%d, sth=%d", __func__, c->frame_len, c->mvad, c->sth);
}

/*
 * ADAPTER RECORD
 ****************************************************************************
 */
static int app_vad_adapter_record_init(void)
{
    app_vad_adpt_record_t *rec = to_vad_adpt_record();

    ADPT_LOG(0, "%s:", __func__);
    if (!rec->inited) {
        memset((void*)&rec->d, 0, sizeof(app_vad_adpt_record_data_t)*APP_VAD_ADPT_REC_QTY);
        rec->inited = true;
        rec->started = false;

        /* default: enable data transmition from sensor-hub to MCU core */
        rec->request     = false;
        rec->frm_period  = 0;
        rec->req_period  = 0;
        rec->data_size   = 0;
        rec->frm_counter = 0;
        rec->seek_size   = 0;
        rec->info = NULL;
    }
    return 0;
}

static int app_vad_adapter_record_setup(enum APP_VAD_ADPT_REC_T type, app_vad_adpt_info_t *info, uint8_t *buf, uint32_t size)
{
    app_vad_adpt_record_t *rec = to_vad_adpt_record();
    app_vad_adpt_record_data_t *pd = to_vad_adpt_rec(type);
    struct AF_STREAM_CONFIG_T *cfg = &info->itf->stream_cfg;

    ADPT_LOG(0, "%s: type=%d", __func__, type);

    rec->info = info;
    pd->buf   = buf;
    pd->size  = size;
    ADPT_LOG(0, "info(0x%x),record:buf(start=%x,end=%x),size=%d", (int)info,(int)buf,(int)(buf+size), size);

    // init record info
    uint32_t samp_size = sample_bit_to_size(cfg->bits);
    uint32_t frm_size = app_vad_adapter_calc_buf_size(cfg->sample_rate, samp_size, cfg->channel_num, 1);
    uint32_t frm_num = cfg->data_size / frm_size;
    uint32_t coef = VAD_ADPT_REC_REQ_PERIOD_COEF;

    ADPT_LOG(0, "samp_size=%d B, frm_size=%d B/ms, frm_num=%d ms", samp_size, frm_size, frm_num);

    rec->request = false;
    rec->frm_period = frm_num / 2;
    rec->req_period = rec->frm_period * coef;
    rec->data_size  = frm_size * rec->req_period;
    rec->req_counter = 0;
    rec->frm_counter = 0;
    rec->seek_size   = 0;

    ADPT_LOG(0, "frm_period=%d ms, req_period=%d ms, data_size=%d B, coef=%d",
        rec->frm_period, rec->req_period, rec->data_size, coef);
    return 0;
}

int app_vad_adapter_get_unread_data_infor(uint32_t **src_start_addr, uint32_t **src_end_addr, uint32_t *len)
{
    app_vad_adpt_record_t *rec = to_vad_adpt_record();
    app_vad_adpt_record_data_t *pd = to_vad_adpt_rec(APP_VAD_ADPT_REC_KW);
    uint32_t data_size;

    if(!rec->started){
        return -1;
    }

    *src_start_addr = (uint32_t*)(pd->buf + pd->rpos);
    *src_end_addr = (uint32_t*)(pd->wpos);

    if (pd->wpos >= pd->rpos) {
        data_size = pd->wpos - pd->rpos;
    } else {
        data_size = pd->size - pd->rpos + pd->wpos;
    }

    *len = data_size;
    return 0;
}

int app_vad_adapter_request_record_process(enum APP_VAD_ADPT_REC_T type, uint8_t *buf, uint32_t len)
{
    bool send = false;
    app_vad_adpt_record_t *rec = to_vad_adpt_record();

    // return if record is stopped or not request
    if ((!rec->started) || (!rec->request)) {
        return -1;
    }

    // check frame counter
    rec->frm_counter++;
    if (rec->frm_counter*rec->frm_period >= rec->req_period) {
        rec->frm_counter = 0;
        send = true;
    }

    // send raw data to host
    if (send) {
        app_vad_adpt_record_data_t *pd = to_vad_adpt_rec(APP_VAD_ADPT_REC_KW);
        uint32_t start_addr, data_size, data_idx;
        uint32_t time = TICKS_TO_MS(hal_sys_timer_get());

        // set rpos to new position to discard some noise raw data
        if (rec->seek_size){
            pd->rpos += rec->seek_size;
            if (pd->rpos >= pd->size) {
                pd->rpos -= pd->size;
            }
            rec->seek_size = 0;
        }

        // transmit raw data by sending message
        data_idx   = rec->req_counter++;
        start_addr = (uint32_t)(pd->buf + pd->rpos);
        data_size  = rec->data_size;

        //update rpos
        pd->rpos += data_size;
        if (pd->rpos >= pd->size) {
            pd->rpos -= pd->size;
        }
        ADPT_LOG(0,"[%u] RD_REC: idx=%d,addr=%x,size=%d,rpos=%d",time,data_idx,start_addr,data_size,pd->rpos);
        app_sensor_hub_core_vad_send_data_msg(data_idx, start_addr, data_size);
    }
    return 0;
}

static int app_vad_adapter_record_clear(enum APP_VAD_ADPT_REC_T type)
{
    app_vad_adpt_record_data_t *pd = to_vad_adpt_rec(type);

    ADPT_LOG(0, "%s:", __func__);
    ASSERT((pd->size>0)&&(pd->buf!=NULL), "%s: bad size(%d) or buf(%x)", __func__, pd->size, (int)(pd->buf));
    pd->wpos = 0;
    pd->rpos = 0;
    memset(pd->buf, 0, pd->size);
    return 0;
}

static uint32_t app_vad_adapter_record_wpos_addr_get(enum APP_VAD_ADPT_REC_T type)
{
    app_vad_adpt_record_data_t *pd = to_vad_adpt_rec(type);
    return (uint32_t)(pd->buf + pd->wpos);
}

static int app_vad_adapter_record_write(enum APP_VAD_ADPT_REC_T type, uint8_t *buf, uint32_t len)
{
    app_vad_adpt_record_data_t *pd = to_vad_adpt_rec(type);

#if !defined(VAD_TEST_PATT_EN)
    int16_t *dst     = (int16_t *)(pd->buf + pd->wpos);
    int16_t *dst_end = (int16_t *)(pd->buf + pd->size);
    int16_t *src     = (int16_t *)(buf);
    int16_t *src_end = (int16_t *)(buf + len);

    ASSERT((pd->size>0)&&(pd->buf!=NULL), "%s: bad size(%d) or buf(%x)", __func__, pd->size, (int)(pd->buf));

    // dangerous memcpy !!
    while (src < src_end) {
        if (dst == dst_end) {
            dst = (int16_t *)(pd->buf);
        }
        *dst++ = *src++;
    }
#endif /* VAD_TEST_PATT_EN */

    pd->wpos += len;
    if (pd->wpos >= pd->size) {
        pd->wpos -= pd->size;
    }

#ifdef VAD_TEST_PATT_EN
    ADPT_LOG(0, "WR_REC: buf=%x,len=%d,wpos=%d",(int)buf,len,pd->wpos);
#endif
    return 0;
}

POSSIBLY_UNUSED static uint32_t app_vad_adapter_record_read(enum APP_VAD_ADPT_REC_T type, uint8_t *buf, uint32_t len)
{
    app_vad_adpt_record_t *rec = to_vad_adpt_record();
    struct AF_STREAM_CONFIG_T *cfg = &(rec->info->itf->stream_cfg);
    app_vad_adpt_record_data_t *pd = to_vad_adpt_rec(type);
    uint32_t avail = 0, zlen = 0;
    uint32_t samp_size;

    ASSERT((pd->size>0)&&(pd->buf!=NULL), "%s: bad size(%d) or buf(%x)", __func__, pd->size, (int)(pd->buf));

    samp_size = sample_bit_to_size(cfg->bits);

    if (pd->wpos >= pd->rpos) {
        avail = pd->wpos - pd->rpos;
    } else {
        avail = pd->size - pd->rpos + pd->wpos;
    }
    if (avail < len) {
        zlen = len - avail;
        len = avail;
    }
    if (samp_size == 4) {
        int32_t *src     = (int32_t *)(pd->buf + pd->rpos);
        int32_t *src_end = (int32_t *)(pd->buf + pd->size);
        int32_t *dst     = (int32_t *)buf;
        int32_t *dst_end = (int32_t *)(buf + len);
        while(dst < dst_end) {
            *dst++ = *src++;
            if (src == src_end) {
                src = (int32_t *)(pd->buf);
            }
        }
        pd->rpos = (uint32_t)src - (uint32_t)(pd->buf);
    } else {
        int16_t *src     = (int16_t *)(pd->buf + pd->rpos);
        int16_t *src_end = (int16_t *)(pd->buf + pd->size);
        int16_t *dst     = (int16_t *)buf;
        int16_t *dst_end = (int16_t *)(buf + len);
        while(dst < dst_end) {
            *dst++ = *src++;
            if (src == src_end) {
                src = (int16_t *)(pd->buf);
            }
        }
        pd->rpos = (uint32_t)src - (uint32_t)(pd->buf);
    }
    if (zlen > 0) {
        memset(buf+len, 0, zlen);
    }
    return len;
}

POSSIBLY_UNUSED static uint32_t vad_get_tail_raw_data(uint8_t *buf, uint32_t total_len, uint32_t tail_len)
{
    int16_t *dst     = (int16_t *)(buf);
    int16_t *src     = (int16_t *)(buf+total_len-tail_len);
    int16_t *src_end = (int16_t *)(buf+total_len);
    uint32_t real_len = tail_len;

    ADPT_LOG(0, "%s:dst=%x,src=%x,src_end=%x,total_len=%d,tail_len=%d", __func__,
        (uint32_t)dst, (uint32_t)src, (uint32_t)src_end, total_len, tail_len);
    if (total_len <= tail_len) {
        real_len = total_len;
        goto _exit;
    }
    while (src < src_end) {
        *dst++ = *src++;
    }
_exit:
    return real_len;
}

POSSIBLY_UNUSED static uint32_t app_vad_adapter_load_vad_private_buffer(enum APP_VAD_ADPT_ID_T id)
{
    struct CODEC_VAD_BUF_INFO_T vad_info;
    app_vad_adpt_record_data_t *pd = to_vad_adpt_rec(APP_VAD_ADPT_REC_KW);
    uint8_t *rec_buf = pd->buf;
    uint32_t rec_buf_len = pd->size;
    uint32_t vad_buf_len, rlen, reqlen;

    ADPT_LOG(0, "%s:rec_buf=%x,rec_buf_len=%d", __func__,(int)rec_buf,rec_buf_len);

    // get vad buffer info
    af_vad_get_data_info(&vad_info);
    vad_buf_len = vad_info.buf_size;
    ADPT_LOG(0, "vad_buf=%x,vad_buf_len=%d,data_cnt=%d,addr_cnt=%d",
        vad_info.base_addr,vad_buf_len,vad_info.data_count,vad_info.addr_count);
    ASSERT(rec_buf_len>=vad_buf_len, "rec_buf_len(%d)<vad_buf_len(%d)",rec_buf_len,vad_buf_len);

    // get total vad buffer data
    reqlen = vad_info.data_count;
    rlen = af_vad_get_data(rec_buf, reqlen);

    // get tail vad buffer data
    rlen = vad_get_tail_raw_data(rec_buf, rlen, VAD_PRIV_BUFF_TAIL_DATA_LEN);

    // update wpos
    pd->wpos += rlen;
    if (pd->wpos >= pd->size) {
        pd->wpos -= pd->size;
    }
#ifdef VAD_ZERO_PRIVATE_DATA
    memset(rec_buf, 0, VAD_PRIV_BUFF_TAIL_DATA_LEN);
    pd->wpos = 0;
#endif
    //update rpos to zero, read from first position
    pd->rpos = 0;

    ADPT_LOG(0, "reqlen=%d,rlen=%d,rpos=%d, wpos=%d",reqlen,rlen,pd->rpos,pd->wpos);
    return rlen;
}

/*
 * ADAPTER RECORD APIs
 ****************************************************************************
 */
static int app_vad_adapter_open_record(enum APP_VAD_ADPT_ID_T id)
{
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();
    app_vad_adpt_info_t *info = to_vad_adpt_info(l,id);
    struct AF_STREAM_CONFIG_T *cfg = to_vad_stream_cfg(l,id);
    uint32_t samp_rate, samp_size, frm_num, chnum, len;
    uint8_t *buf = app_vad_adpt_kw_buf;

    ADPT_LOG(0, "%s:id=%d", __func__,id);

    samp_size = sample_bit_to_size(cfg->bits);
    samp_rate = cfg->sample_rate;
    chnum     = cfg->channel_num;
    frm_num   = VAD_ADPT_REC_LEN_MS;
    len = app_vad_adapter_calc_buf_size(samp_rate, samp_size, chnum, frm_num);
    ADPT_LOG(0, "%s: id=%d,sr=%d,ch=%d,frm=%d,len=%d", __func__,id,samp_rate,chnum,frm_num,len);

#ifdef VAD_TEST_PATT_EN
    // initialize record buffer with data pattern:1,2,3 ...
    uint32_t i;
    int16_t *p16 = (int16_t *)buf;
    for (i = 0; i < len/2; i++) {
        p16[i] = i+1;
    }
#endif
    app_vad_adapter_record_setup(APP_VAD_ADPT_REC_KW, info, buf, len);
    return 0;
}

static int app_vad_adapter_start_record(enum APP_VAD_ADPT_ID_T id)
{
    app_vad_adpt_record_t *rec = to_vad_adpt_record();

    ADPT_LOG(0, "%s:id=%d", __func__,id);
    if (!rec->started) {
        rec->started = true;
        app_vad_adapter_record_clear(APP_VAD_ADPT_REC_KW);
    }
    return 0;
}

static int app_vad_adapter_stop_record(enum APP_VAD_ADPT_ID_T id)
{
    app_vad_adpt_record_t *rec = to_vad_adpt_record();

    ADPT_LOG(0, "%s:id=%d", __func__,id);
    if (rec->started) {
        rec->started = false;
    }
    return 0;
}

static int app_vad_adapter_close_record(enum APP_VAD_ADPT_ID_T id)
{
    app_vad_adpt_record_t *rec = to_vad_adpt_record();

    if (rec->started) {
        app_vad_adapter_stop_record(id);
    }
    ADPT_LOG(0, "%s:id=%d", __func__,id);
    return 0;
}

int app_vad_adapter_write_record(enum APP_VAD_ADPT_REC_T type, uint8_t *buf, uint32_t len)
{
    if (type == APP_VAD_ADPT_REC_KW) {
        app_vad_adpt_record_t *rec = to_vad_adpt_record();
        if (rec->started) {
            app_vad_adapter_record_write(type, buf, len);
            return 0;
        }
    }
    return -1;
}

/*
 * VAD ADAPTER APIs
 ****************************************************************************
 */
int app_vad_adapter_init(void)
{
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();

    ADPT_LOG(0, "%s:", __func__);

    if (!l->inited) {
        // init adapter info
        memset((void*)&(l->d[0]),0,sizeof(app_vad_adpt_info_t)*APP_VAD_ADPT_ID_QTY);

        // init adapter record
        app_vad_adapter_record_init();

        // init vad apps
        app_voice_detector_init();

        l->inited = true;
    }
    return 0;
}

#if 0
int app_vad_adapter_register_stream_handler(enum APP_VAD_ADPT_ID_T id, app_vad_adpt_stream_handler_t *h)
{
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();
    app_vad_adpt_stream_handler_t *sh = to_vad_stream_handler(l,id,0);
    ASSERT(l->inited==true, "%s:no inited", __func__);

    ADPT_LOG(0, "%s:id=%d, h=%x", __func__, id, (int)h);
    *sh = *h;
    return 0;
}

int app_vad_adapter_unregister_stream_handler(enum APP_VAD_ADPT_ID_T id, app_vad_adpt_stream_handler_t *h)
{
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();
    app_vad_adpt_stream_handler_t *sh = to_vad_stream_handler(l,id,0);
    ASSERT(l->inited==true, "%s:no inited", __func__);

    ADPT_LOG(0, "%s:id=%d, h=%x", __func__, id, (int)h);
    sh->active = false;
    sh->stream_handler = NULL;
    sh->priv = NULL;
    return 0;
}

int app_vad_adapter_enable_stream_handler(enum APP_VAD_ADPT_ID_T id)
{
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();
    app_vad_adpt_stream_handler_t *sh = to_vad_stream_handler(l,id,0);
    ASSERT(l->inited==true, "%s:no inited", __func__);

    ADPT_LOG(0, "%s:id=%d", __func__,id);
    sh->active = true;
    return 0;
}

int app_vad_adapter_disable_stream_handler(enum APP_VAD_ADPT_ID_T id)
{
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();
    app_vad_adpt_stream_handler_t *sh = to_vad_stream_handler(l,id,0);
    ASSERT(l->inited==true, "%s:no inited", __func__);

    ADPT_LOG(0, "%s:id=%d", __func__,id);
    sh->active = false;
    return 0;
}

int app_vad_adapter_register_word_handler(enum APP_VAD_ADPT_ID_T id, app_vad_adpt_word_handler_t *h)
{
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();
    app_vad_adpt_word_handler_t *wh = to_vad_word_handler(l,id,0);
    ASSERT(l->inited==true, "%s:no inited", __func__);

    ADPT_LOG(0, "%s:id=%d, h=%x", __func__, id, (int)h);
    *wh = *h;
    return 0;
}

int app_vad_adapter_unregister_word_handler(enum APP_VAD_ADPT_ID_T id, app_vad_adpt_word_handler_t *h)
{
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();
    app_vad_adpt_word_handler_t *wh = to_vad_word_handler(l,id,0);
    ASSERT(l->inited==true, "%s:no inited", __func__);

    ADPT_LOG(0, "%s:id=%d, h=%x", __func__, id, (int)h);
    wh->active = false;
    wh->word_handler = NULL;
    return 0;
}
#endif

static inline void vad_adpt_codec_stream_prepare_start_callback_handler(uint8_t *buf, uint32_t len)
{
    uint32_t i, j;
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();

    for (i = 0; i < APP_VAD_ADPT_ID_QTY; i++) {
        for (j = 0; j < APP_VAD_ADPT_STREAM_PRIO_QTY; j++) {
            app_vad_adpt_stream_handler_t *h = to_vad_stream_handler(l,i,j);
            if (h->active && h->prepare_start_handler) {
                h->prepare_start_handler(buf, len);
            }
        }
    }
}

static inline void vad_adpt_codec_stream_prepare_stop_callback_handler(uint8_t *buf, uint32_t len)
{
    uint32_t i, j;
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();

    for (i = 0; i < APP_VAD_ADPT_ID_QTY; i++) {
        for (j = 0; j < APP_VAD_ADPT_STREAM_PRIO_QTY; j++) {
            app_vad_adpt_stream_handler_t *h = to_vad_stream_handler(l,i,j);
            if (h->active && h->prepare_stop_handler) {
                h->prepare_stop_handler(buf, len);
            }
        }
    }
}

static inline void vad_adpt_codec_data_callback_handler(uint8_t *buf, uint32_t len)
{
    uint32_t i, j;
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();

    for (i = 0; i < APP_VAD_ADPT_ID_QTY; i++) {
        app_vad_adpt_state_t *state = to_vad_adpt_state(l,i);
        if (state->start) {
            for (j = 0; j < APP_VAD_ADPT_STREAM_PRIO_QTY; j++) {
                app_vad_adpt_stream_handler_t *h = to_vad_stream_handler(l,i,j);
                if (h->active && h->stream_handler) {
                    h->stream_handler(buf, len);
                }
            }
        }
    }
}

static int app_vad_adapter_check_bad_adc_data(uint8_t *buf, uint32_t len,
    uint32_t samp_size, uint32_t check_cnt)
{
    int err = 0;

    if (samp_size == 2) {
        int16_t *pcm = (int16_t *)buf;
        uint32_t i;

        for (i = 0; i < len / 2; i++) {
            if (i >= check_cnt) {
                break;
            }
            if (ABS(pcm[i]) > 32700) {
                ADPT_LOG(0, "=====> WARNING: bad adc data[%d]=%d",i,pcm[i]);
                err++;
            }
        }
    }
    if (err) {
//        ASSERT(0, "%s: stoped, err=%d", __func__,err);
    }
    return err;
}

static uint32_t vad_adpt_codec_cap_data_handler(uint8_t *buf, uint32_t len)
{
#if (VAD_ADPT_DEBUG == 1)
    uint32_t curtime = TICKS_TO_MS(hal_sys_timer_get());
#endif

    // check bad adc data
    app_vad_adapter_check_bad_adc_data(buf, len, 2, 32);

    // save data into record buffer
    app_vad_adapter_write_record(APP_VAD_ADPT_REC_KW, buf, len);

    // invoke stream data handler
    vad_adpt_codec_data_callback_handler(buf,len);

    // record data request process
    app_vad_adapter_request_record_process(APP_VAD_ADPT_REC_KW, buf, len);

#if (VAD_ADPT_DEBUG == 1)
    ADPT_LOG(0, "[%d] ADPT CODEC_CAP: buf=%x, len=%d", curtime, (int)buf, len);
#endif
    return 0;
}

static int app_vad_adapter_init_interface(enum APP_VAD_ADPT_ID_T id)
{
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();
    app_vad_adpt_info_t *info = to_vad_adpt_info(l,id);
    app_vad_adpt_stream_handler_t *sh = to_vad_stream_handler(l,id,APP_VAD_ADPT_STREAM_PRIO_L1);
    app_vad_adpt_if_t *pif = app_vad_adapter_get_if(id);

    // init adapter interface data
    ASSERT(pif->init_func!=NULL, "%s: null init_func", __func__);
    if (pif->init_func) {
        pif->init_func(&pif->stream_cfg);
    }
    if (pif->stream_cfg.handler != vad_adpt_codec_cap_data_handler) {
        pif->stream_cfg.handler = vad_adpt_codec_cap_data_handler;
    }
    app_vad_adapter_init_vad_hw_cfg(&pif->vad_cfg);

    // setup adapter interface
    info->itf = pif;

    // setup stream handler
    sh->prepare_start_handler = pif->stream_prepare_start_handler;
    sh->prepare_stop_handler = pif->stream_prepare_stop_handler;
    sh->stream_handler = pif->stream_handler;
    sh->active = true;

    ADPT_LOG(0, "%s:done, id=%d", __func__, id);
    return 0;
}

static enum HAL_CMU_FREQ_T app_vad_adapter_get_vad_freq(void)
{
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();
    app_vad_adpt_info_t *info = to_vad_adpt_info(l,act_adpt_id);

    ADPT_LOG(0, "%s:freq=%d", __func__, info->itf->cap_freq);
    return info->itf->cap_freq;
}

static void app_vad_adapter_cb_cmd_run_done(int state, void *param)
{
    ADPT_LOG(0, "%s: state=%d", __func__, state);

    /* this callback can moniters the voice detector runing process
     */
    if (state == VOICE_DET_STATE_AUD_CAP_START) {
#ifdef ADPT_USE_VAD_PRIVATE_DATA
        uint32_t vad_record_len = 0;
#endif
        /* start one timer to moniter codec capture stream.
         * when it's timed out, capture stream will be stoppted automatically,
         * the VAD will be restarted again at the same time.
         */
        app_vad_adapter_start_timer(VAD_CAP_STREAM_PERIOD_MS, vad_adapter_timer_callback);

        /* When adapter starts codec capture stream, the record will be started at
         * the same time;
         */
        app_vad_adapter_start_record(act_adpt_id);

#ifdef ADPT_USE_VAD_PRIVATE_DATA
        /* load vad private buffer */
        vad_record_len = app_vad_adapter_load_vad_private_buffer(act_adpt_id);
        vad_adpt_codec_stream_prepare_start_callback_handler((uint8_t*)(app_vad_adapter_record_wpos_addr_get(APP_VAD_ADPT_REC_KW) - vad_record_len),vad_record_len);
#endif
    }
    if (state == VOICE_DET_STATE_AUD_CAP_STOP) {
        /* When adapter stops codec capture stream, the record will be stopped at
         * the same time;
         */
#ifdef ADPT_USE_VAD_PRIVATE_DATA
        vad_adpt_codec_stream_prepare_stop_callback_handler(NULL,0);
#endif
        app_vad_adapter_stop_record(act_adpt_id);

        /* capture stream has been stopped, decrease system frequency
         */
        hal_sysfreq_req(SYSFREQ_USER_APP_VAD_ADPT, HAL_CMU_FREQ_32K);
    }
    vad_adapter_clr_detector_act_cmd(state);
}

static void app_vad_adapter_cb_vad_find(int state, void *param)
{
    enum HAL_CMU_FREQ_T freq = app_vad_adapter_get_vad_freq();

    /* VAD Find IRQ is triggered, means the CPU is waked up now.
     * The sensor hub core will send RPC cmd to MCU core;
     * The sensor hub core will start capture stream for listening voice cmd;
     */
    ADPT_LOG(0, "%s: state=%d", __func__, state);

    /* increase system frequency */
    hal_sysfreq_req(SYSFREQ_USER_APP_VAD_ADPT, freq);

    /* start capture stream */
    vad_adapter_set_detector_act_cmd(VOICE_DET_EVT_AUD_CAP_START);
    app_voice_detector_send_event(act_vd_id,VOICE_DET_EVT_AUD_CAP_START);

    /* send event msg to MCU core */
    app_sensor_hub_core_vad_send_evt_msg(SENS_EVT_ID_VAD_IRQ_TRIG, 1, 0, 0);
}

static int app_vad_adapter_start_interface(enum APP_VAD_ADPT_ID_T id, struct AF_STREAM_CONFIG_T *cfg)
{
    enum voice_detector_id vd_id = act_vd_id;

    ADPT_LOG(0, "%s:adpt_id=%d,vd_id=%d", __func__, id, vd_id);

    app_voice_detector_open(vd_id, AUD_VAD_TYPE_DIG);
    //app_voice_detector_setup_vad(vd_id, &vad_cfg);
    app_voice_detector_setup_stream(vd_id, AUD_STREAM_CAPTURE, cfg);
    app_voice_detector_setup_callback(vd_id, VOICE_DET_CB_RUN_DONE, app_vad_adapter_cb_cmd_run_done, NULL);
    app_voice_detector_setup_callback(vd_id, VOICE_DET_FIND_APP, app_vad_adapter_cb_vad_find, NULL);

//    app_voice_detector_send_event(vd_id,VOICE_DET_EVT_OPEN);

    vad_adapter_set_detector_act_cmd(VOICE_DET_EVT_VAD_START);
    app_voice_detector_send_event(vd_id,VOICE_DET_EVT_VAD_START);
    //app_voice_detector_send_event(vd_id,VOICE_DET_EVT_AUD_CAP_START);

    return 0;
}

static int app_vad_adapter_stop_interface(enum APP_VAD_ADPT_ID_T id)
{
    /* stop voice detector Apps
     */
    enum voice_detector_id vdid = VOICE_DETECTOR_ID_0;

    vad_adapter_set_detector_act_cmd(VOICE_DET_EVT_CLOSE);
    //app_voice_detector_send_event(vdid, VOICE_DET_EVT_CLOSE);
    app_voice_detector_close(vdid);
    return 0;
}

int app_vad_adapter_open(enum APP_VAD_ADPT_ID_T id)
{
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();
    app_vad_adpt_state_t *ps = to_vad_adpt_state(l,id);
    ASSERT(l->inited==true, "%s:no inited", __func__);

    ADPT_LOG(0, "%s:id=%d open=%d", __func__, id,ps->open);
    if (!ps->open) {
        app_vad_adapter_init_interface(id);
        app_vad_adapter_open_record(id);
        ps->open = 1;
    }
    return 0;
}

int app_vad_adapter_start(enum APP_VAD_ADPT_ID_T id)
{
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();
    app_vad_adpt_state_t *ps = to_vad_adpt_state(l,id);

    ASSERT(l->inited==true, "%s:no inited", __func__);
    ASSERT(ps->open != 0, "%s:no opened", __func__);

    ADPT_LOG(0, "%s:id=%d start?= %d", __func__,id,ps->start);
    if (!ps->start) {
        struct AF_STREAM_CONFIG_T *cfg = to_vad_stream_cfg(l,id);

        app_vad_adapter_start_interface(id, cfg);
        ps->start = 1;
        ps->vad_active = 1;
        act_adpt_id = id;
    }
    return 0;
}

int app_vad_adapter_stop(enum APP_VAD_ADPT_ID_T id)
{
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();
    app_vad_adpt_state_t *ps = to_vad_adpt_state(l,id);

    ASSERT(l->inited==true, "%s:no inited", __func__);
    ASSERT(ps->open != 0, "%s:no opened", __func__);

    ADPT_LOG(0, "%s:id=%d", __func__,id);
    if (ps->start) {
        app_vad_adapter_set_vad_active(id, false);
        app_vad_adapter_stop_interface(id);
        ps->start = 0;
    }
    return 0;
}

int app_vad_adapter_close(enum APP_VAD_ADPT_ID_T id)
{
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();
    app_vad_adpt_state_t *ps = to_vad_adpt_state(l,id);
    ASSERT(l->inited==true, "%s:no inited", __func__);

    ADPT_LOG(0, "%s:id=%d", __func__,id);
    if (ps->open) {
        ps->open = 0;
        app_vad_adapter_close_record(id);
    }
    return 0;
}

int app_vad_adapter_event_handler(enum APP_VAD_ADPT_ID_T id, enum SENS_VAD_EVT_ID_T evt_id,
        uint32_t p0, uint32_t p1, uint32_t p2)
{
    ADPT_LOG(0, "%s:evt=%d", __func__, evt_id);
    /*
     * When key word is recognized, vad will not active again, this means
     * the codec capture stream will always on
     */
    if (evt_id == SENS_EVT_ID_VAD_VOICE_CMD) {
#if !defined(VAD_ADPT_TEST_ENABLE)
        app_vad_adapter_set_vad_active(id, false);
#endif
    }
    return 0;
}

int app_vad_adapter_set_vad_active(enum APP_VAD_ADPT_ID_T id, bool active)
{
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();
    app_vad_adpt_state_t *s = to_vad_adpt_state(l,id);

    if(s->start == 0){
        ADPT_LOG(0,"%s:not start",__func__);
    }

    ADPT_LOG(0, "%s:id=%d,active=%d", __func__, id, active);
#if 0
    if (active == (!!s->vad_active)) {
        ADPT_LOG(0, "%s: already active", __func__);
        return 0;
    }
#endif
    s->vad_active = active ? 1 : 0;
    if (active) {
        // send Apps event cmd to make vad be active
        vad_adapter_set_detector_act_cmd(VOICE_DET_EVT_VAD_START);
        app_voice_detector_send_event(act_vd_id, VOICE_DET_EVT_VAD_START);
    }
    return 0;
}

int app_vad_adapter_set_vad_request(enum APP_VAD_ADPT_ID_T id, bool request, uint32_t data_size,uint32_t seek_size)
{
    app_vad_adpt_record_t *rec = to_vad_adpt_record();
    app_vad_adpt_info_list_t *l = to_vad_adpt_list();
    struct AF_STREAM_CONFIG_T *cfg = to_vad_stream_cfg(l,id);

    ADPT_LOG(0, "%s:id=%d,request=%d,data_size=%d seek_size = %d", __func__, id, request, data_size,seek_size);

    rec->request = request;
    if (request) {
        uint32_t samp_size = sample_bit_to_size(cfg->bits);
        uint32_t frm_size = app_vad_adapter_calc_buf_size(cfg->sample_rate, samp_size,cfg->channel_num, 1);
        uint32_t req_period = data_size / frm_size;

        ASSERT((data_size%frm_size) == 0, "%s: bad data_size %d/%d != 0", __func__, data_size, frm_size);
        ASSERT((req_period >= rec->frm_period), "%s: bad req_period(%d), frm_period=%d", __func__, req_period, rec->frm_period);
        ADPT_LOG(0, "update data_size: %d --> %d", rec->data_size, data_size);
        ADPT_LOG(0, "update req_period : %d --> %d", rec->req_period, req_period);
        rec->data_size = data_size;
        rec->req_period = req_period;
        rec->seek_size = NON_EXP_ALIGN(seek_size,data_size);
    } else {
        /*
         * This subroutine's behaviour is complicated when request=false.
         * When mcu don't need receiving VAD data from SensorHub,
         * force to set VAD active again;
         */
        app_vad_adapter_record_clear(APP_VAD_ADPT_REC_KW);
        app_vad_adapter_set_vad_active(id, true);
    }
    return 0;
}

int app_vad_adapter_seek_data_update(uint32_t size)
{
    app_vad_adpt_record_t *rec = to_vad_adpt_record();
    app_vad_adpt_record_data_t *pd = to_vad_adpt_rec(APP_VAD_ADPT_REC_KW);

    if (!rec->started) {
        return -1;
    }

    if(rec->data_size){
        size = NON_EXP_ALIGN(size,rec->data_size);
    }

    pd->rpos += size;
    if (pd->rpos >= pd->size) {
        pd->rpos -= pd->size;
    }

    return 0;
}

#endif

