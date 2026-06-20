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
#include "string.h"
#include "hal_trace.h"
#include "ohos_walkie_talkie_app_msg.h"
#include "kfifo.h"
#include "bth_bt_if_client.h"

#define OHOS_MEDIA_WALKIE_TALKIE_CONFIG
#define OHOS_MEDIA_WALKIE_TALKIE_NUMBER     6
struct kfifo    *walkie_talkie_fifo[OHOS_MEDIA_WALKIE_TALKIE_NUMBER];

static walkie_talkie_msg_send_func  send_walkie_talkie_stream_msg = NULL;
int walkie_talkie_register_ohos_msg_send_func(walkie_talkie_msg_send_func p_func)
{
    send_walkie_talkie_stream_msg = p_func;
}

void ohos_walkie_talkie_init(void)
{
   return;
}

void ohos_walkie_talkie_deinit(void)
{
    return;
}

void ohos_walkie_talkie_stream_start(uint8_t device_id, void* fifo)
{
    walkie_talkie_stream_t  pcm_msg;
    walkie_talkie_stream_info *p_pcm_info;

    TRACE(3,"%s  %d device id %d", __FUNCTION__, __LINE__, device_id);

    if(device_id >= OHOS_MEDIA_WALKIE_TALKIE_NUMBER)
    {
        TRACE(3, "%s device id to large id %d > max %d ", __func__, device_id, OHOS_MEDIA_WALKIE_TALKIE_NUMBER);
        return;
    }
    if (send_walkie_talkie_stream_msg) 
    {
        walkie_talkie_fifo[device_id] = (struct kfifo *)fifo;
        p_pcm_info = (walkie_talkie_stream_info *)pcm_msg.msg_data;
        p_pcm_info->sample_rate = (uint32_t)8000;
        p_pcm_info->channel_num = (uint8_t)1;
        p_pcm_info->bits_per_sample = (uint8_t)16;
        p_pcm_info->bit_rate = 11000;
        p_pcm_info->one_buffer_size = 83;
        p_pcm_info->length = 0;
        p_pcm_info->device_id = device_id;
        pcm_msg.msg_type = WALKIE_TALKIE_PCM_STREAM_START;
        pcm_msg.msg_arg_len = sizeof(walkie_talkie_stream_info);
        send_walkie_talkie_stream_msg(&pcm_msg);
        TRACE(4, "%s %d device id %d, kfifo id = %p\n", __func__, __LINE__, device_id, walkie_talkie_fifo[device_id]);
    }
    return;
}

void ohos_walkie_talkie_stream_stop(uint8_t device_id)
{
    walkie_talkie_stream_t  pcm_msg;

    TRACE(3,"%s  %d device id %d", __FUNCTION__, __LINE__, device_id);

    if(device_id >= OHOS_MEDIA_WALKIE_TALKIE_NUMBER)
    {
        TRACE(3, "%s device id to large id %d > max %d ", __func__, device_id, OHOS_MEDIA_WALKIE_TALKIE_NUMBER);
        return;
    }
    if (send_walkie_talkie_stream_msg) 
    {
        pcm_msg.msg_data[0] = device_id;
        pcm_msg.msg_type = WALKIE_TALKIE_PCM_STREAM_STOP;
        pcm_msg.msg_arg_len = 1;
        send_walkie_talkie_stream_msg(&pcm_msg);
    }
    return;
}

void walkie_talkie_write_data_to_ohos(uint8_t device_id)
{
    walkie_talkie_stream_t  pcm_msg;
    walkie_talkie_write_info  *p_wr_info;

    if(device_id >= OHOS_MEDIA_WALKIE_TALKIE_NUMBER)
    {
        TRACE(3, "%s device id to large id %d > max %d ", __func__, device_id, OHOS_MEDIA_WALKIE_TALKIE_NUMBER);
        return;
    }
    TRACE(3,"%s  %d device id %d", __FUNCTION__, __LINE__, device_id);
    if (send_walkie_talkie_stream_msg) 
    {
        pcm_msg.msg_data[0] = device_id;
        pcm_msg.msg_arg_len = 1;
        pcm_msg.msg_type = WALKIE_TALKIE_PCM_STREAM_WRITE;
        send_walkie_talkie_stream_msg(&pcm_msg);
    }
    return;
}

#if  1
static const ble_client_walkie_talkie_callback_t ohos_walkie_talkie_stream_func_cb_list =
{
    .ble_client_walkie_talkie_init_cb = ohos_walkie_talkie_init,
    .ble_client_walkie_talkie_deinit_cb = ohos_walkie_talkie_deinit,
    .ble_client_walkie_talkie_playback_stream_start_cb = ohos_walkie_talkie_stream_start,
    .ble_client_walkie_talkie_playback_cb = walkie_talkie_write_data_to_ohos,
    .ble_client_walkie_talkie_playback_stream_stop_cb = ohos_walkie_talkie_stream_stop,
};
#endif

uint32_t ohos_walkie_talkie_updata_stream(uint8_t device_id, uint8_t *pcm_buf, uint32_t len)
{
    uint32_t real_len = 0;
    uint32_t data_len = 0;

    if (walkie_talkie_fifo[device_id] == NULL)
    {
        TRACE(2,"%s  line %d ",__func__, __LINE__);
        return 0;
    }

    data_len = kfifo_len(walkie_talkie_fifo[device_id]);
    // TRACE(4,"%s  line %d data_len %d, len %d ",__func__, __LINE__,data_len, len);
    // TRACE(4, "%s kfifo = %p in %d out %d", __func__, walkie_talkie_fifo[device_id], walkie_talkie_fifo[device_id]->in, walkie_talkie_fifo[device_id]->out);
    // DUMP8("%02x", (uint8_t *)walkie_talkie_fifo[device_id], 16);
    if (len <= data_len)
    {
        kfifo_get(walkie_talkie_fifo[device_id], pcm_buf, len);
        real_len = len;
    }
    else if (data_len > 0)
    {
        //TRACE(2,"kfifo data_len %d  len %d", data_len, len);
        kfifo_get(walkie_talkie_fifo[device_id], pcm_buf, data_len);
        real_len = data_len;
    }

    return real_len;
}


void ohos_walkie_talkie_stream_config(void)
{
#ifdef OHOS_MEDIA_WALKIE_TALKIE_CONFIG
    btif_client_register_walkie_talkie_callback(&ohos_walkie_talkie_stream_func_cb_list);
#endif
}

