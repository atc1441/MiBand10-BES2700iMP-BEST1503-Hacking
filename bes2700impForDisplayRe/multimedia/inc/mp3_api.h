/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#ifndef __MP3_API_H__
#define __MP3_API_H__
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    int sample_rate;
    int8_t channels;
    int8_t layer;
    int bitrate_kbps;
    int frame_samples;
    int frame_size;
} mp3dec_info_t;

typedef struct
{
    int frame_bytes, channels, hz, layer, bitrate_kbps;
} mp3dec_frame_info_t;

typedef struct
{
    float mdct_overlap[2][9*32], qmf_state[15*2*32];
    int reserv, free_format_bytes;
    unsigned char header[4], reserv_buf[511];
    void* scratch;
} mp3dec_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int mp3dec_get_size();
bool mp3dec_init(mp3dec_t *dec);
bool mp3dec_deinit(mp3dec_t *dec);
bool mp3dec_get_info(mp3dec_t* dec, mp3dec_info_t*info);

int mp3dec_get_scratch_size();
bool mp3dec_set_scratch(mp3dec_t *dec,void* scratch);

int mp3dec_decode_frame(mp3dec_t *dec, const uint8_t *mp3, int mp3_bytes, int16_t *pcm, mp3dec_frame_info_t *info);
int mp3dec_decode_frame_float(mp3dec_t *dec, const uint8_t *mp3, int mp3_bytes, float *pcm, mp3dec_frame_info_t *info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
