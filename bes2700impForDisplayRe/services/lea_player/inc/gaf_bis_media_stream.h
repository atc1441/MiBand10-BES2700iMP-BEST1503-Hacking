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
/**
 ****************************************************************************************
 * @addtogroup APP_BAP
 * @{
 ****************************************************************************************
 */

#ifndef APP_BAP_STREAM_H_
#define APP_BAP_STREAM_H_

/*****************************header include********************************/
#include "gaf_media_stream.h"
#include "audioflinger.h"

/******************************macro defination*****************************/

/******************************type defination******************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t  grp_lid;
    uint16_t conhdl;
    uint32_t big_sync_delay;
    uint32_t big_trans_latency;
} gaf_bis_src_param_t;

void gaf_bis_src_audio_stream_stop_handler(uint8_t grp_lid);
void gaf_bis_src_audio_stream_start_handler(gaf_bis_src_param_t *src_para);
void gaf_bis_src_audio_stream_init(void);

void gaf_bis_audio_stream_stop_handler(uint8_t grp_lid);
void gaf_bis_audio_stream_start_handler(uint8_t grp_lid);
void gaf_bis_audio_stream_set_stream_volume(int8_t vol);
void gaf_bis_audio_stream_init(void);
void gaf_bis_stream_dump_dma_trigger_status(void);

#ifdef __cplusplus
}
#endif

#endif // APP_BAP_STREAM_H_

/// @} APP_BAP
