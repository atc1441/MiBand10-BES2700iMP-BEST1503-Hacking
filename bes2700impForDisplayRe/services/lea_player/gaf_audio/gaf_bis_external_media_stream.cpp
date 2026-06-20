/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#if BLE_AUDIO_ENABLED
#include "bluetooth_bt_api.h"
#include "app_bt_func.h"
#include "app_utils.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hwtimer_list.h"
#include "rwble_config.h"

#include "gaf_stream_dbg.h"
#include "gaf_bis_external_media_stream.h"
#include "gaf_media_common.h"
#include "gaf_media_stream.h"
#include "gaf_media_sync.h"

#include "bt_drv_reg_op.h"

#include "bes_aob_api.h"

#define GAF_BIS_EXTERNAL_STREAM_MAX         3
#define BIS_SEND_DATA_WINDOW                2000    //BIS Send Data Window

typedef struct{
    bool       started;
    int        btc_buf_num_max;
    uint32_t   data_seq_num;
    uint32_t   prev_timestamp;
    gaf_bis_external_stream_param_t stream_param;
} gaf_bis_external_stream_info_t;

typedef struct
{
    uint8_t stream_num;
    HWTIMER_ID timer_id;
    gaf_bis_external_stream_info_t stream_info[GAF_BIS_EXTERNAL_STREAM_MAX];
} gaf_bis_external_stream_env_t;

static gaf_bis_external_stream_env_t bis_env = {0};


static uint32_t gaf_bis_external_stream_calculate_trigger(gaf_bis_external_stream_info_t *stream_info)
{
    uint32_t time_delay = 0;
    uint32_t latest_iso_bt_time = 0;
#ifndef WIFI_DONGLE
    latest_iso_bt_time = btdrv_reg_op_big_anchor_timestamp(
        BLE_ISOHDL_TO_ACTID(stream_info->stream_param.bis_hdl)); 
#endif
    uint32_t current_bt_time = gaf_media_sync_get_curr_time();

    if (latest_iso_bt_time > current_bt_time)
    {
        latest_iso_bt_time -= ((int32_t)(stream_info->stream_param.send_interval_us));
    }

    uint32_t trigger_bt_time = latest_iso_bt_time + stream_info->stream_param.big_sync_delay_us;
    if (trigger_bt_time > current_bt_time)
    {
        time_delay = trigger_bt_time - current_bt_time;
    }
    else if (trigger_bt_time+BIS_SEND_DATA_WINDOW < current_bt_time)
    {
        time_delay = (trigger_bt_time + stream_info->stream_param.send_interval_us) - current_bt_time;
    }
    LOG_I("last=%d us, current=%d us,trigger=%d us, delay %d us",
        latest_iso_bt_time, current_bt_time, trigger_bt_time, time_delay);

    return time_delay;
}

static void gaf_bis_external_stream_timer_handler_in_bt_thread(uint32_t param0, uint32_t param1)
{
    uint16_t data_len = 0;
    uint8_t *send_data_buf = NULL;
    int free_num=0;
    gaf_bis_external_stream_info_t *stream_info = NULL;

    for (int i=0; i<4; ++i)
    {
        free_num = bes_ble_bap_get_free_iso_packet_num();
        if (free_num < 2)
        {
            return;
        }

        for (int stream_idx=0; stream_idx < bis_env.stream_num; ++stream_idx)
        {
            stream_info = &bis_env.stream_info[stream_idx];
            if (!stream_info->data_seq_num)
            {
                stream_info->btc_buf_num_max = free_num;
            }
            /// read data from user layer, and send data to BTC
            if (stream_info->stream_param.read_data_cb)
            {
                stream_info->stream_param.read_data_cb(stream_info->stream_param.stream_idx,
                    &send_data_buf, &data_len, stream_info->btc_buf_num_max-free_num);
            }

            if (send_data_buf)
            {
                bes_ble_bap_iso_dp_send_data(stream_info->stream_param.bis_hdl,
                    stream_info->data_seq_num++, send_data_buf,
                    data_len,  stream_info->prev_timestamp);

                if (stream_info->stream_param.data_buf_free_cb)
                {
                    stream_info->stream_param.data_buf_free_cb(stream_info->stream_param.stream_idx, send_data_buf);
                }
            }
            else
            {
                return;
            }
        }
    }
}

static void gaf_bis_external_stream_timer_handler(void *param)
{
    gaf_bis_external_stream_info_t *stream_info = (gaf_bis_external_stream_info_t *)param;

    app_bt_start_custom_function_in_bt_thread(0, 0,
        (uint32_t)gaf_bis_external_stream_timer_handler_in_bt_thread);

    hwtimer_start(bis_env.timer_id, US_TO_TICKS(stream_info->stream_param.send_interval_us));
}

void gaf_bis_external_stream_start(gaf_bis_external_stream_param_t *stream_param)
{
    uint8_t stream_idx = stream_param->stream_idx;
    uint32_t trigger_delay = 0;
    gaf_bis_external_stream_info_t *stream_info = NULL;

    if (bis_env.stream_info[stream_idx].started)
    {
        return;
    }

    if (!bis_env.stream_num)
    {
        app_sysfreq_req(APP_SYSFREQ_USER_AOB_CAPTURE, APP_SYSFREQ_26M);
        bt_adapter_write_sleep_enable(0);
    }

    stream_info = &bis_env.stream_info[stream_idx];
    stream_info->started = true;
    stream_info->data_seq_num = 0;
    stream_info->prev_timestamp = 0;
    memcpy(&stream_info->stream_param, stream_param, sizeof(gaf_bis_external_stream_param_t));
    if (!bis_env.stream_num)
    {

        bis_env.timer_id = hwtimer_alloc(gaf_bis_external_stream_timer_handler, stream_info);
        trigger_delay = gaf_bis_external_stream_calculate_trigger(stream_info);
        if (!trigger_delay)
        {
            gaf_bis_external_stream_timer_handler_in_bt_thread(0,0);
            trigger_delay = stream_info->stream_param.send_interval_us;
        }
        hwtimer_start(bis_env.timer_id, US_TO_TICKS(trigger_delay));
    }
    bis_env.stream_num++;
}

void gaf_bis_external_stream_stop(uint8_t stream_idx)
{
    gaf_bis_external_stream_info_t *stream_info = NULL;

    if (!bis_env.stream_info[stream_idx].started)
    {
        return;
    }

    bis_env.stream_num--;
    stream_info = &bis_env.stream_info[stream_idx];
    stream_info->started = false;
    stream_info->data_seq_num = 0;
    stream_info->prev_timestamp = 0;
    memset(&stream_info->stream_param, 0, sizeof(gaf_bis_external_stream_param_t));

    if (!bis_env.stream_num)
    {
        hwtimer_stop(bis_env.timer_id);
        hwtimer_free(bis_env.timer_id);
        bt_adapter_write_sleep_enable(1);
        app_sysfreq_req(APP_SYSFREQ_USER_AOB_CAPTURE, APP_SYSFREQ_32K);
    }
}
#endif
/// @} APP
