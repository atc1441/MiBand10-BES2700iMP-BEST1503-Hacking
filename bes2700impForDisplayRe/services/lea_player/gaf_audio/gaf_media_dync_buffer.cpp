/**
 *
 * @copyright Copyright (c) 2015-2023 BES Technic.
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

#ifdef DYNAMIC_SET_PB_TIME

#include "cmsis.h"
#include "cmsis_os.h"

#include "app_gaf_dbg.h"
#include "gaf_media_common.h"
#include "gaf_media_stream.h"
#include "gaf_media_dync_buffer.h"

#ifdef DYNAMIC_SET_PB_TIME
#define PLAYBACK_MAX_LATENCY                   (20)
#define PLAYBACK_MIN_LATENCY                   (2)
#define PLAYBACK_BUFF_DEPTH                    (2)    // unit of FT
#define NUM_CONSECUTIVE_PLC_PKTS               (20)
#define ADJUST_PLAYBACK_FT_STEP                (2)
#define STABLE_STREAM_TIMEOUT                 (10 * 1000)
#endif

// only for test
bool interference_start = false;

/// Start simulating the interference environment
void gaf_media_stream_start_interference()
{
    LOG_I("%s",__func__);
    interference_start = true;
}

/// Stop simulating the interference environment
void gaf_media_stream_stop_interference()
{
    LOG_I("%s",__func__);
    interference_start = false;
}

bool gaf_dync_is_interference_start()
{
    return interference_start;
}

static inline void gaf_stream_playback_inc_ft(GAF_AUDIO_DYNC_BUFFER_T* dync_buffer)
{
    if (dync_buffer->playback_buf_depth == PLAYBACK_MAX_LATENCY)
    {
        return;
    }
    uint8_t inc_ft = dync_buffer->playback_buf_depth + ADJUST_PLAYBACK_FT_STEP;

    if (inc_ft >= PLAYBACK_MAX_LATENCY)
    {
        dync_buffer->playback_buf_depth = PLAYBACK_MAX_LATENCY;
    }
    else
    {
        dync_buffer->playback_buf_depth = inc_ft;
    }

    LOG_I("%s,ft = %d",__func__,dync_buffer->playback_buf_depth);
}

static inline void gaf_stream_playback_dec_ft(GAF_AUDIO_DYNC_BUFFER_T* dync_buffer)
{
    LOG_I("%s start,ft = %d",__func__,dync_buffer->playback_buf_depth);
    if (dync_buffer->playback_buf_depth == PLAYBACK_MIN_LATENCY)
    {
        LOG_I("%s,already set min ft",__func__);
        return;
    }

    dync_buffer->playback_buf_depth -= ADJUST_PLAYBACK_FT_STEP;
    LOG_I("%s,ft = %d",__func__,dync_buffer->playback_buf_depth);
}

static void gaf_stream_playback_stream_status_checker_timer_cb(const void* ctx)
{
    LOG_I("%s",__func__);
    GAF_AUDIO_DYNC_BUFFER_T* dync_buffer = (GAF_AUDIO_DYNC_BUFFER_T*)ctx;
    gaf_stream_playback_dec_ft(dync_buffer);
    dync_buffer->stable_check_ongoing = false;
}

void gaf_dync_monitor_recv_pkt_status(GAF_AUDIO_DYNC_BUFFER_T* dync_buffer, gaf_media_data_t* decoder_frame_p)
{
    if (decoder_frame_p->data_len == 0)
    {
        if ((dync_buffer->playback_buf_depth != PLAYBACK_MAX_LATENCY)
             && dync_buffer->plc_counter < NUM_CONSECUTIVE_PLC_PKTS)
        {
            ++dync_buffer->plc_counter;
        }
        else
        {
           gaf_stream_playback_inc_ft(dync_buffer);
           // reset counter
           dync_buffer->plc_counter = 0;
        }

        if (dync_buffer->stable_check_ongoing)
        {
            osTimerStop(dync_buffer->stream_status_checker_tid);
            dync_buffer->stable_check_ongoing = false;
        }
    }
    else
    {
        if (dync_buffer->plc_counter != 0)
        {
            LOG_I("reset plc counter");
            dync_buffer->plc_counter = 0;
        }

        // LOG_I("%s stable depth = %d",__func__,pStreamEnv->stream_info.playbackInfo.playback_buf_depth);
        if ((dync_buffer->playback_buf_depth != PLAYBACK_MIN_LATENCY)
            && !dync_buffer->stable_check_ongoing)
        {
            LOG_I("%s start monitor stream",__func__);
            dync_buffer->stable_check_ongoing = true;
            osTimerStart(dync_buffer->stream_status_checker_tid,STABLE_STREAM_TIMEOUT);
        }
    }
}

void gaf_dync_set_timestamp_by_variable_ft(
    gaf_media_data_t *pkt, GAF_AUDIO_DYNC_BUFFER_T* dync_buffer, uint32_t iso_interval)
{
    pkt->time_stamp -= (PLAYBACK_MAX_LATENCY - dync_buffer->playback_buf_depth) * iso_interval;
}

void gaf_dync_buffer_info_reset(GAF_AUDIO_DYNC_BUFFER_T* dync_buffer)
{
    dync_buffer->plc_counter = 0;
    dync_buffer->playback_irq_counter = 0;
    dync_buffer->playback_buf_depth = PLAYBACK_BUFF_DEPTH;
    dync_buffer->stable_check_ongoing = false;
}

void gaf_dync_buffer_init(GAF_AUDIO_DYNC_BUFFER_T *dync_buffer)
{
    dync_buffer->plc_counter = 0;
    dync_buffer->playback_irq_counter = 0;
    dync_buffer->playback_buf_depth = PLAYBACK_BUFF_DEPTH;
    osTimerInit(dync_buffer->stream_status_checker_tdef,gaf_stream_playback_stream_status_checker_timer_cb);
    dync_buffer->stream_status_checker_tid= \
        osTimerCreate(&dync_buffer->stream_status_checker_tdef.os_timer_def,osTimerOnce, dync_buffer);
    dync_buffer->stable_check_ongoing = false;
}

#endif