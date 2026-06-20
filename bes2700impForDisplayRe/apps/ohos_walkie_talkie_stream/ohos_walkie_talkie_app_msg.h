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

#ifndef __WALKIE_TALKIE_APP_MSG_H__
#define __WALKIE_TALKIE_APP_MSG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_MSG_DATA_LEN  (64)

typedef enum
{
    WALKIE_TALKIE_PCM_STREAM_REQ = 0,
    WALKIE_TALKIE_PCM_STREAM_START,
    WALKIE_TALKIE_PCM_STREAM_WRITE,
    WALKIE_TALKIE_PCM_STREAM_STOP,
    WALKIE_TALKIE_PCM_STREAM_CLOSE,
    WALKIE_TALKIE_MSG_MAX,
}WALKIE_TALKIE_MSG_TYPE;

typedef struct
{
    WALKIE_TALKIE_MSG_TYPE msg_type;
    uint16_t msg_arg_len;
    uint8_t msg_data[MAX_MSG_DATA_LEN];
}walkie_talkie_stream_t;

typedef struct{
    uint32_t length;
    uint32_t sample_rate;
    uint32_t bit_rate;
    uint8_t channel_num;
    uint8_t bits_per_sample;
    uint16_t one_buffer_size;
    uint8_t device_id;
}walkie_talkie_stream_info;

typedef struct
{
    uint8_t device_id;
    uint8_t *pcm_buff;
    uint32_t once_put_len;
}walkie_talkie_write_info;

typedef int (* walkie_talkie_msg_send_func)(walkie_talkie_stream_t *pMsg);

int walkie_talkie_register_ohos_msg_send_func(walkie_talkie_msg_send_func p_func);
void ohos_walkie_talkie_stream_config(void);
uint32_t ohos_walkie_talkie_updata_stream(uint8_t device_id, uint8_t *pcm_buf, uint32_t len);

#ifdef __cplusplus
}
#endif
#endif // __WALKIE_TALKIE_APP_MSG_H__


