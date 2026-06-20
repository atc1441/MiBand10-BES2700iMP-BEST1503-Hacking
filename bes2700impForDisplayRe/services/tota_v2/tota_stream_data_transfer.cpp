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
#ifdef STREAM_DATA_OVER_TOTA
#include "string.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "apps.h"
#include "stdbool.h"
#include "app_tota.h"
#include "app_tota_cmd_code.h"
#include "app_tota_cmd_handler.h"
#include "app_utils.h"
#include "app_spp_tota.h"
#include "tota_buffer_manager.h"
#include "tota_stream_data_transfer.h"
#include "app_tota_conn.h"
#include "app_tota_common.h"

/*
**  stream control strcut
**  stream packet format:
**  # header + body #
**  > header: 2   bytes
**  > body  : 664 bytes
**
**  note: stream will never be encrypted!
*/
typedef struct{
    uint32_t flush_bytes;
    bool     is_streaming;
    uint8_t  module;
}stream_control_t;

stream_control_t stream_control;

/* static function */
static void _tota_stream_data_init();


// define stream data transfer thread
#define TOTA_STREAM_DATA_STACK_SIZE             2048
static void tota_stream_data_transfer_thread(void const *argument);
osThreadId tota_stream_thread_tid;
osThreadDef(tota_stream_data_transfer_thread, osPriorityHigh, 1, TOTA_STREAM_DATA_STACK_SIZE, "TOTA_STREAM_DATA_THREAD");

static void tota_stream_data_transfer_thread(void const *argument)
{
    uint8_t buf[MAX_SPP_PACKET_SIZE] = {0xff, 0xff};
    TOTA_LOG_DBG(0, "[STREAMING THREAD] +++");
    while ( true )
    {
        app_sysfreq_req(APP_SYSFREQ_USER_TOTA, APP_SYSFREQ_32K);
        osSignalWait(0x0001, osWaitForever);
        app_sysfreq_req(APP_SYSFREQ_USER_TOTA, APP_SYSFREQ_104M);
        while ( tota_stream_buffer_read(buf + STREAM_HEADER_SIZE, MAX_SPP_PACKET_SIZE - STREAM_HEADER_SIZE) )
        {
            app_tota_send(buf, MAX_SPP_PACKET_SIZE,OP_TOTA_NONE);
        }
    }
}

/*-----------------------------------------------------------------------------------------------*/

void app_tota_stream_data_transfer_init()
{
    tota_stream_thread_tid = osThreadCreate(osThread(tota_stream_data_transfer_thread), NULL);
    _tota_stream_data_init();
}

// stream start
void app_tota_stream_data_start(uint16_t set_module)
{
    stream_control.module       = set_module;
    stream_control.is_streaming = true;
}

// stream end
void app_tota_stream_data_end()
{
    stream_control.is_streaming = false;
}

// stream send data while stream is start
bool app_tota_send_stream_data(uint8_t * pdata, uint32_t dataLen)
{
    if ( !stream_control.is_streaming )
    {
        TOTA_LOG_DBG(0, "error: data stream not start.");
        return false;
    }
    if ( tota_stream_buffer_write(pdata, dataLen) )
    {
        TOTA_LOG_DBG(0, "send to stream buffer ok.");
        return true;
    }
    else
    {
        TOTA_LOG_DBG(0, "send to stream buffer error.");
        return false;
    }
}

// stream flush
void app_tota_stream_data_flush()
{
    tota_stream_buffer_flush();
}

// stream clean
void app_tota_stream_data_clean()
{
    tota_stream_buffer_clean();
}

bool is_stream_data_running()
{
    return stream_control.is_streaming;
}

static void _tota_stream_data_init()
{
    tota_stream_buffer_init(tota_stream_thread_tid);
    stream_control.module       = 0;
    stream_control.is_streaming = false;
}
#endif
