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
#ifndef __APP_TWS_IBRT_AUDIO_ANALYSIS_H__
#define __APP_TWS_IBRT_AUDIO_ANALYSIS_H__

#define AUDIO_ANALYSIS_INTERVAL (25)
#define AUDIO_ANALYSIS_CHECKER_INTERVEL_INVALID (1000)

typedef struct {
    uint32_t timestamp;
    uint32_t decoded_frames;
    uint32_t sample_rate;
    uint16_t sequenceNumber;
    uint16_t frame_samples;
    uint8_t curSubSequenceNumber;
    uint8_t totalSubSequenceNumber;
    uint8_t undecode_frames;
    uint8_t undecode_min_frames;
    uint8_t undecode_max_frames;
    uint8_t average_frames;
#if A2DP_DECODER_CHECKER
    uint32_t check_sum;
#endif
}__attribute__((packed)) APP_TWS_IBRT_AUDIO_ANALYSIS_PLAYBACK_INFO_T;

#define AUDIO_SYNC_CUSTOM_DATA_SIZE    16

typedef struct
{
    uint8_t  custom_data[AUDIO_SYNC_CUSTOM_DATA_SIZE];
} AUDIO_SYNC_CUSTOM_DATA_T;

typedef struct {
    APP_TWS_IBRT_AUDIO_ANALYSIS_PLAYBACK_INFO_T playback_info;
    uint16_t mobile_cnt;
    uint16_t bt_cnt;
    uint32_t bt_clk;
    uint32_t mobile_clk;
    uint16_t handler_cnt;
    int8_t  compensate_cnt;
    bool updated;
    bt_bdaddr_t mobile_addr;
} __attribute__((packed)) APP_TWS_IBRT_AUDIO_ANALYSIS_INFO_T;

typedef struct {
    uint32_t tws_diff_clk;
    uint16_t tws_diff_bit;
    int32_t tws_diff_us;
    uint32_t mobile_diff_clk;
    uint16_t mobile_diff_cnt;
    int32_t mobile_diff_us;
} APP_TWS_IBRT_AUDIO_ANALYSIS_RESULT_T;

typedef enum {
    AUDIO_ANALYSIS_STATUS_STOP,
    AUDIO_ANALYSIS_STATUS_SUSPEND,
    AUDIO_ANALYSIS_STATUS_START,
    AUDIO_ANALYSIS_STATUS_DATA_VALID
}AUDIO_ANALYSIS_STATUS_E;

typedef enum {
    AUDIO_RETRIGGER_RET_TYPE_FAIL,
    AUDIO_RETRIGGER_RET_TYPE_SUCESS,
    AUDIO_RETRIGGER_RET_TYPE_IN_PROGRESS,
}AUDIO_ANALYSIS_RETRIGGER_RET_TYPE_E;

void  app_tws_ibrt_audio_mobile_clkcnt_get(uint8_t device_id, uint32_t btclk, uint16_t btcnt,
                                                     uint32_t *mobile_master_clk, uint16_t *mobile_master_cnt);
APP_TWS_IBRT_AUDIO_ANALYSIS_RESULT_T *app_tws_ibrt_audio_analysis_result_get(void);
APP_TWS_IBRT_AUDIO_ANALYSIS_INFO_T *app_tws_ibrt_audio_analysis_local_info_get(void);
APP_TWS_IBRT_AUDIO_ANALYSIS_INFO_T *app_tws_ibrt_audio_analysis_remote_info_get(void);
int app_tws_ibrt_audio_analysis_status_get(void);
int app_tws_ibrt_audio_analysis_init(void);
int app_tws_ibrt_audio_analysis_start(uint32_t start_offset, uint32_t checker_intervel);
int app_tws_ibrt_audio_analysis_stop(void);
int app_tws_ibrt_audio_analysis_suspend(void);
int app_tws_ibrt_audio_analysis_resume(void);
int app_tws_ibrt_audio_analysis_skipnext(void);
uint32_t app_tws_ibrt_audio_analysis_interval_get(void);
int app_tws_ibrt_audio_analysis_interval_set(uint32_t analysis_interval);
uint32_t app_tws_ibrt_audio_analysis_tick_get(void);
int8_t app_tws_ibrt_audio_analysis_get_refill_frames(void);
void app_tws_ibrt_audio_analysis_set_refill_frames(int8_t frames);
int8_t app_tws_ibrt_audio_analysis_update_refill_frames(int8_t delta);
int app_tws_ibrt_audio_analysis(APP_TWS_IBRT_AUDIO_ANALYSIS_INFO_T *local_info ,
                                APP_TWS_IBRT_AUDIO_ANALYSIS_INFO_T *remote_info);
void app_tws_ibrt_audio_analysis_interrupt_tick(void);
void app_tws_ibrt_audio_analysis_audiohandler_tick(uint8_t device_id);

AUDIO_ANALYSIS_RETRIGGER_RET_TYPE_E app_tws_ibrt_audio_retrigger(void);

#endif
