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
#include "hal_trace.h"
#include "string.h"
#include "hearing_det_cmd.h"
#include "app_trace_rx.h"
#include "app_key.h"
#include "app_thread.h"
#include <stdlib.h>

#define HEARING_LOG_I(str, ...)      TR_INFO(TR_MOD(TEST), "[HEARING DET]" str, ##__VA_ARGS__)

/**
 * Usage:
 *  1. CMD Format: e.g. [hearing_det,stream_switch]
 **/

typedef void (*func_handler_t)(void);

typedef struct {
    const char *name;
    func_handler_t handler;
} hearing_det_func_t;

enum HEAR_COMP_MSG_T {
    HEAR_COMP_MSG_OPEN = 0,
    HEAR_COMP_MSG_CLOSE,

    HEAR_COMP_MSG_NUM,
};

static int32_t hearing_det_cmd_process(enum HEAR_COMP_MSG_T action);

static bool hearing_det_stream_status = true;
int32_t hearing_det_stream(void)
{
    TRACE(0, "[%s] status = %d", __func__, hearing_det_stream_status);

    //hearing_det_stream_start(hearing_det_stream_status);

    if (hearing_det_stream_status) {
        hearing_det_cmd_process(HEAR_COMP_MSG_OPEN);
    } else {
        hearing_det_cmd_process(HEAR_COMP_MSG_CLOSE);
    }

    hearing_det_stream_status = !hearing_det_stream_status;

    return 0;
}

static void hearing_det_stream_switch(void)
{
    HEARING_LOG_I(" %s...", __func__);

    hearing_det_stream();
}

const hearing_det_func_t hearing_det_func[]= {
    {"stream_switch",       hearing_det_stream_switch},
};

extern void audio_get_eq_para(int *para_buf, uint8_t cfg_flag);

int eq_para_buff[7] = {0}; // 0:5 six test result of hearing test; 6:eq_update_flg
static void test_tmp(void)
{
    for(int i=0;i<7;i++)
    {
        TRACE(2,"eq_para_buff[%d]=%d",i,eq_para_buff[i]);
    }
    TRACE(0,"test_tmp_func!!");
    uint8_t cfg_flag = eq_para_buff[6];
    audio_get_eq_para(eq_para_buff,cfg_flag);
}

func_handler_t  eq_handle = test_tmp;

static void set_amp_fun(void)
{
    TRACE(0,"Set amp only!!!");
}
func_handler_t  set_amp_handle = set_amp_fun;

extern void set_hearing_para(int uart_freq, int uart_amp_dB);
extern void set_hearing_amp(int uart_amp_dB);

static uint32_t hearing_det_cmd_callback(uint8_t *buf, uint32_t len)
{
    uint8_t *func_name = buf;
    func_handler_t func_handler = NULL;

    HEARING_LOG_I("[%s] len = %d", __func__, len);

    char *str1 = (char *)func_name;
    const char *str2 = hearing_det_func[0].name;
    int str_comp_result = 0;
    if(13<=len)
    {
        for(int i=0;i<13;i++)
        {
            if(str1[i]!=str2[i])
            {
                str_comp_result = 1;
                //TRACE(1,"i=%d",i);
            }
        }
        if(0==str_comp_result)
        {
            func_handler = hearing_det_func[0].handler;

            int uart_get_fs = 0;
            int uart_get_amp = 0;
            if(','==str1[13]  && ','==str1[18])
            {
                uart_get_fs = atoi(&str1[14]);
                uart_get_amp = atoi(&str1[19]);
            }
            TRACE(2,"uart_get_fs=%d,uart_get_amp=%d",uart_get_fs,uart_get_amp);

            set_hearing_para(uart_get_fs,uart_get_amp);
        }
    }

//Set test tone level
    const char *str4 = "Set_test_amp";
    int str_comp_result3 = 0;

    if(12<=len)
    {
        for(int i=0;i<12;i++)
        {
            if(str1[i]!=str4[i])
            {
                str_comp_result3 = 1;
                //TRACE(1,"i=%d",i);
            }
        }
        if(0==str_comp_result3)
        {
            int uart_get_amp = 0;
            uart_get_amp = atoi(&str1[13]);
            set_hearing_amp(uart_get_amp);
            TRACE(1,"uart_get_amp=%d",uart_get_amp);
            func_handler = set_amp_handle;
        }
    }

//Set EQ
    const char *str3 = "Set_EQ";
    int str_comp_result2 = 0;

    int eq_idx = 0;
    if(6<=len)
    {
        for(int i=0;i<6;i++)
        {
            if(str1[i]!=str3[i])
            {
                str_comp_result2 = 1;
                //TRACE(1,"i=%d",i);
            }
        }
        if(0==str_comp_result2)
        {
            for(int i=0;i<len;i++)
            {
                if( ','==str1[i] && i!=(len-1) && 6>=eq_idx)
                {
                    eq_para_buff[eq_idx] = atoi(&str1[i+1]);
                    eq_idx++;
                }
            }
            func_handler = eq_handle;
        }
    }


    if (func_handler) {
        func_handler();
    } else {
        HEARING_LOG_I(" Can not found cmd: %s", func_name);
    }

    return 0;
}

static int32_t hearing_det_cmd_process(enum HEAR_COMP_MSG_T action)
{
    TRACE(1, "[%s] ", __func__);

    APP_MESSAGE_BLOCK msg;
    msg.mod_id = APP_MODULE_HEAR_COMP;
#if defined(USE_BASIC_THREADS)
    msg.mod_level = APP_MOD_LEVEL_1;
#endif 
    msg.msg_body.message_id = action;
    app_mailbox_put(&msg);

    return 0;
}

extern int hearing_det_stream_start(bool on);

static int hearing_det_cmd_thread_handler(APP_MESSAGE_BODY *msg_body)
{
    uint32_t id = msg_body->message_id;

    TRACE(0, "[%s] id :%d", __func__, id);

    switch (id) {
        case HEAR_COMP_MSG_OPEN:
            hearing_det_stream_start(true);
            break;
        case HEAR_COMP_MSG_CLOSE:
            hearing_det_stream_start(false);
            break;
        default:
            ASSERT(0, "[%s] id(%d) is invalid", __func__, id);
            break;
    }

    return 0;
}

int32_t hearing_det_cmd_init(void)
{
    app_trace_rx_register("hearing_det", hearing_det_cmd_callback);

    app_set_threadhandle(APP_MODULE_HEAR_COMP, hearing_det_cmd_thread_handler);

    return 0;
}
