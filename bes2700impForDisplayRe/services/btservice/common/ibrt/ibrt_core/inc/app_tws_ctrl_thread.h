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
#ifndef __APP_TWS_CTRL_THREAD__
#define __APP_TWS_CTRL_THREAD__

#include "app_tws_ibrt.h"
#include "app_tws_ibrt_cmd_handler.h"

typedef void (*app_tws_cmd_send_function_t)(uint8_t*, uint16_t);
typedef void (*TWS_CTRL_STATUS_NOTIFY_T)(uint16_t ,uint8_t* ,uint16_t);

typedef struct
{
    uint32_t                        cmdcode;
    app_tws_cmd_send_function_t     tws_cmd_send;
} __attribute__((packed)) app_tws_cmd_send_t;

typedef enum
{
    PARA_NONE,
    PARA_CODEC_TYPE,
    PARA_PLAY_BACK,
    PARA_BUFFER,
    PARA_SYNC_TRIGGER,
} mail_para_type_e;

typedef struct
{
    uint8_t *cmd_buffer;
    uint16_t cmd_len;
} cmd_buffer_t;

typedef struct
{
    uint32_t evt;
    uint32_t arg;
    uint32_t arg1;
    uint32_t system_time;
    cmd_buffer_t cmd_buff;
} TWS_MSG_BLOCK;

#ifdef __cplusplus
extern "C" {
#endif
void tws_ctrl_thread_init(void);
void *tws_ctrl_mailbox_heap_malloc(uint32_t size);
void tws_ctrl_mailbox_heap_free(void *rmem);
int tws_ctrl_send_cmd(uint32_t cmd_code, uint8_t *p_buff, uint16_t length);
int tws_ctrl_send_rsp(uint16_t rsp_code, uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void tws_ctrl_free_cmd(uint32_t cmd_code);
void tws_ctrl_mailbox_clear(osMailQId tws_mailbox);
int tws_ctrl_mailbox_free(osMailQId tws_mailbox, TWS_MSG_BLOCK* msg_p);
void tws_ctrl_mailbox_dump_log(void);
uint32_t tws_ctrl_get_mtu_size(void);
bool app_tws_curr_thread_is_tws_ctrl(void);

void tws_ctrl_register_send_status_callback(TWS_CTRL_STATUS_NOTIFY_T cb);

#ifdef __cplusplus
}
#endif
#endif
