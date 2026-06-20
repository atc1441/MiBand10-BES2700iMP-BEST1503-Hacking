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
 * @addtogroup GAF_NON_CODEC_STREAM
 * @{
 ****************************************************************************************
 */
#ifndef __GAF_BIS_EXTERNAL_MEDIA_STREAM_H__
#define __GAF_BIS_EXTERNAL_MEDIA_STREAM_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef void (*read_data_callback_t)(uint8_t stream_idx, uint8_t **data, uint16_t *data_len, uint8_t cache_num);
typedef void (*data_buf_free_callback_t)(uint8_t stream_id, uint8_t *data);

typedef struct
{
    uint8_t  stream_idx;
    uint16_t bis_hdl;
    uint16_t data_size;
    uint16_t send_interval_us;
    uint32_t big_bn;
    uint32_t big_sync_delay_us;
    read_data_callback_t read_data_cb;
    data_buf_free_callback_t data_buf_free_cb;
} gaf_bis_external_stream_param_t;

void gaf_bis_external_stream_start(gaf_bis_external_stream_param_t *stream_param);
void gaf_bis_external_stream_stop(uint8_t stream_idx);

#ifdef __cplusplus
}
#endif
#endif // __GAF_NON_CODEC_STREAM_H__
/// @} GAF_NON_CODEC_STREAM
