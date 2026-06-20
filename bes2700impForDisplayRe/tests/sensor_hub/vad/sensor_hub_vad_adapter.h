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
#ifndef __SENSOR_HUB_VAD_ADAPTER_H__
#define __SENSOR_HUB_VAD_ADAPTER_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "sens_msg.h"
#include "audioflinger.h"
/*
 * This source file is used to implement an adatper for VAD applications; The VAD
 * adapter is one layer which connects application layer and sensor hub core layer;
 * The adapter layer is used control the VAD application and thirdparty's AI voice
 * library for voice recoginition; The VAD adapter initiailizes the VAD Apps by matched
 * stream configuration which depends on different thirdparty library.
 * Now We define the APP_VAD_ADPT_ID_T to specify the supported thirdparty libraries
 * for the VAD adatper.
 */

#define VAD_HW_PARAM_UDC         0x1//0xa
#define VAD_HW_PARAM_UPRE        0x4
#define VAD_HW_PARAM_SENSOR_FRAME_LEN    8       // 1.6K sample rate
#define VAD_HW_PARAM_MIC_FRAME_LEN       0x50    // 16K sample rate
#define VAD_HW_PARAM_MVAD        0x7
#define VAD_HW_PARAM_DIG_MODE    0x1
#define VAD_HW_PARAM_PRE_GAIN    0x4
#define VAD_HW_PARAM_STH         0x10
#define VAD_HW_PARAM_DC_BYPASS   0x0
#define VAD_HW_PARAM_FRAME_TH0   0x32
#define VAD_HW_PARAM_FRAME_TH1   0x1f4
#define VAD_HW_PARAM_FRAME_TH2   0x1388
#define VAD_HW_PARAM_RANGE0      0xf
#define VAD_HW_PARAM_RANGE1      0x32
#define VAD_HW_PARAM_RANGE2      0x96
#define VAD_HW_PARAM_RANGE3      0x12c
#define VAD_HW_PARAM_PSD_TH0     0x0
#define VAD_HW_PARAM_PSD_TH1     0x07ffffff

// The period value of adatper timer for codec capture stream working
#define VAD_CAP_STREAM_PERIOD_MS 20000 //ms

// The vad adapter record maximum buffer size
#define VAD_ADPT_REC_SAMPRATE     16000
#define VAD_ADPT_REC_CHNUM     1
#define VAD_ADPT_REC_SSIZE     2
#define VAD_ADPT_REC_LEN_MS    3072

#define VAD_ADPT_REC_BUF_SIZE ((VAD_ADPT_REC_SAMPRATE/1000)* \
    VAD_ADPT_REC_CHNUM*VAD_ADPT_REC_SSIZE*VAD_ADPT_REC_LEN_MS)

// The request record data info
#define VAD_ADPT_REC_REQ_PERIOD_COEF (2)

// The vad private buffer tail data length
#define VAD_PRIV_BUFF_TAIL_LEN_MS (32*20)
#define VAD_PRIV_BUFF_TAIL_DATA_LEN ((VAD_ADPT_REC_SAMPRATE/1000)* \
    VAD_ADPT_REC_CHNUM*VAD_ADPT_REC_SSIZE*VAD_PRIV_BUFF_TAIL_LEN_MS)

enum APP_VAD_ADPT_ID_T {
    APP_VAD_ADPT_ID_NONE,
    APP_VAD_ADPT_ID_DEMO,
    APP_VAD_ADPT_ID_AI_KWS,
    APP_VAD_ADPT_ID_SNDP,

    APP_VAD_ADPT_ID_QTY,
};

enum APP_VAD_ADPT_STREAM_PRIO_T {
    APP_VAD_ADPT_STREAM_PRIO_L0 = 0, //highest
    APP_VAD_ADPT_STREAM_PRIO_L1,     //higher
    APP_VAD_ADPT_STREAM_PRIO_QTY,
};

enum APP_VAD_ADPT_WORD_PRIO_T {
    APP_VAD_ADPT_WORD_PRIO_L0 = 0, //for extern user
    APP_VAD_ADPT_WORD_PRIO_L1, //default
    APP_VAD_ADPT_WORD_PRIO_QTY,
};

/* vad adapter codec capture stream data handler */
typedef uint32_t (*vad_adpt_stream_prepare_start_handler_t)(uint8_t *buf, uint32_t len);

typedef uint32_t (*vad_adpt_stream_prepare_stop_handler_t)(uint8_t *buf, uint32_t len);

/* vad adapter codec capture stream data handler */
typedef uint32_t (*vad_adpt_stream_data_handler_t)(uint8_t *buf, uint32_t len);

/* vad adapter voice word handler */
typedef void (*vad_adpt_word_handler_t)(void *kws_info,uint16_t len);

typedef void (*vad_adpt_init_cap_stream_func_t)(struct AF_STREAM_CONFIG_T *cfg);

/* app vad adapter stream handler type */
typedef struct {
    bool active; //enable or disable
    vad_adpt_stream_prepare_start_handler_t prepare_start_handler;
    vad_adpt_stream_prepare_stop_handler_t prepare_stop_handler;
    vad_adpt_stream_data_handler_t stream_handler; //codec capture stream handler
} app_vad_adpt_stream_handler_t;

/* app vad adapter interface type */
typedef struct {
    enum APP_VAD_ADPT_ID_T id;
    enum HAL_CMU_FREQ_T cap_freq;
    struct AF_STREAM_CONFIG_T stream_cfg;
    struct AUD_VAD_CONFIG_T vad_cfg;
    vad_adpt_stream_prepare_start_handler_t stream_prepare_start_handler;
    vad_adpt_stream_prepare_stop_handler_t stream_prepare_stop_handler;
    vad_adpt_stream_data_handler_t stream_handler;
    vad_adpt_init_cap_stream_func_t init_func;
} app_vad_adpt_if_t;

/* app vad adapter state */
typedef struct {
    uint32_t open          :1;
    uint32_t start         :1;
    uint32_t stream_enable :1;
    uint32_t vad_active    :1;
    uint32_t evt_id        :4;
    uint32_t evt_cnt       :24;
} app_vad_adpt_state_t;

/* app vad adapter info data */
typedef struct {
    app_vad_adpt_state_t state;
    app_vad_adpt_if_t *itf;
    app_vad_adpt_stream_handler_t stream_handlers[APP_VAD_ADPT_STREAM_PRIO_QTY];
} app_vad_adpt_info_t;

/* app vad adapter info list */
typedef struct {
    bool inited;
    app_vad_adpt_info_t d[APP_VAD_ADPT_ID_QTY];
} app_vad_adpt_info_list_t;

/* app vad adapter record type */
enum APP_VAD_ADPT_REC_T {
    APP_VAD_ADPT_REC_PRELOAD,
    APP_VAD_ADPT_REC_KW,
    APP_VAD_ADPT_REC_QTY,
};

/* app vad adapter record data */
typedef struct {
    uint8_t *buf;
    uint32_t wpos;
    uint32_t rpos;
    uint32_t size;
} app_vad_adpt_record_data_t;

/* app vad adapter record */
typedef struct {
    bool inited;
    bool started;
    bool request;
    uint32_t frm_period;
    uint32_t frm_counter;
    uint32_t req_period;
    uint32_t req_counter;
    uint32_t data_size;
    uint32_t seek_size;
    app_vad_adpt_info_t *info;
    app_vad_adpt_record_data_t d[APP_VAD_ADPT_REC_QTY];
} app_vad_adpt_record_t;

/* initialize vad adapter */
int app_vad_adapter_init(void);

/* open vad adapter by id */
int app_vad_adapter_open(enum APP_VAD_ADPT_ID_T id);

/* vad adapter starts work */
int app_vad_adapter_start(enum APP_VAD_ADPT_ID_T id);

/* vad adapter stop work */
int app_vad_adapter_stop(enum APP_VAD_ADPT_ID_T id);

/* close vad adatper by id */
int app_vad_adapter_close(enum APP_VAD_ADPT_ID_T id);

/* vad adapter get interface data */
app_vad_adpt_if_t *app_vad_adapter_get_if(enum APP_VAD_ADPT_ID_T id);

/* vad adapter event msg handler from core layer */
int app_vad_adapter_event_handler(enum APP_VAD_ADPT_ID_T id, enum SENS_VAD_EVT_ID_T evt_id,
    uint32_t p0, uint32_t p1, uint32_t p2);

/* app_vad_adpter_set_vad_active - vad adapter set vad active state after
 * key word is recognized.
 *
 * After the key word is recognized, if active is false, the VAD will keep
 * inactive state and the codec capture stream will be always on at the same time;
 *
 * When this subroutine is invoked, if active is true, the Adapter will active VAD and close
 * codec capture stream.
 */
int app_vad_adapter_set_vad_active(enum APP_VAD_ADPT_ID_T id, bool active);

/* set data requesting state for capture stream data transmition */
int app_vad_adapter_set_vad_request(enum APP_VAD_ADPT_ID_T id, bool request, uint32_t size,uint32_t seek_size);

/* get the gap data infor of read and write on the record buf*/
int app_vad_adapter_get_unread_data_infor(uint32_t **src_start_addr,uint32_t **src_end_addr,uint32_t *len);

/* mcu can request to seek the offset of sensor-hub's recording */
int app_vad_adapter_seek_data_update(uint32_t size);

#ifdef __cplusplus
}
#endif

#endif

