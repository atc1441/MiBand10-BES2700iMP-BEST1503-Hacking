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
#ifndef __APP_IBRT_VOICE_REPORT_H__
#define __APP_IBRT_VOICE_REPORT_H__

#include "hal_aud.h"

typedef struct
{
    uint32_t aud_id;
    uint32_t aud_pram;
    uint32_t tg_tick;
    uint32_t vol;
} __attribute__((packed)) app_ibrt_voice_report_info_t;

typedef struct
{
    uint32_t aud_id;
    uint32_t aud_pram;
    uint32_t vol;
} __attribute__((packed)) app_ibrt_voice_report_request_t;

typedef struct
{
    enum AUD_STREAM_USE_DEVICE_T device;
    uint8_t trigger_channel;
} app_ibrt_voice_resport_trigger_device_t;

typedef uint8_t voice_report_role_t;
#define VOICE_REPORT_MASTER   0x00
#define VOICE_REPORT_SLAVE    0x01
#define VOICE_REPORT_LOCAL    0x02

typedef uint32_t (*voice_report_se_join_handler_t)(uint32_t curr_ticks, uint32_t tgt_ticks);

#ifdef __cplusplus
extern "C" {
#endif

int app_ibrt_voice_report_trigger_checker(void);
int app_ibrt_voice_report_trigger_init(uint32_t aud_id, uint32_t aud_pram, app_ibrt_voice_resport_trigger_device_t *device);
int app_ibrt_voice_report_trigger_deinit(void);
void app_ibrt_send_voice_report_request_req(uint8_t *p_buff, uint16_t length);
void app_ibrt_voice_report_request_req_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_send_voice_report_start_req(uint8_t *p_buff, uint16_t length);
void app_ibrt_voice_report_request_start_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
int app_ibrt_if_voice_report_handler(uint32_t aud_id, uint16_t aud_pram, uint8_t device_id);
bool app_ibrt_voice_report_is_me(uint32_t voice_chnlsel);
voice_report_role_t app_ibrt_voice_report_get_role(void);

void app_ibrt_if_register_retrigger_prompt_id(uint16_t prompt_id);

void app_ibrt_voice_report_register_se_join_callback(voice_report_se_join_handler_t handler);

#ifdef __cplusplus
}
#endif

#endif
