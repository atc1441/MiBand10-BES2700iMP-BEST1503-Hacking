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
#ifndef __SIGNAL_GENERATOR_H__
#define __SIGNAL_GENERATOR_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum  {
    SG_TYPE_TONE_SIN_1K,
    SG_TYPE_TONE_SIN_100,
    SG_TYPE_TONE_SIN_500,
    SG_TYPE_TONE_SIN_1600,
    SG_TYPE_NUM
} SG_TYPE_T;

void signal_generator_init(SG_TYPE_T type, uint32_t sample_rate, uint32_t bits, uint32_t channel_num);
void signal_generator_deinit(void);
void signal_generator_loop_get_data(void *pcm_buf, uint32_t frame_len);

#ifdef __cplusplus
}
#endif

#endif