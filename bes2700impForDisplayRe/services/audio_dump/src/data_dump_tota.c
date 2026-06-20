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
#include "data_dump_tota.h"
#include "app_tota_data_handler.h"
#include "hal_trace.h"
#include "string.h"

#define _SEND_DATA_USE_THREAD

// ------------------------------ TOTA API Wrap ------------------------------
extern bool app_tota_is_connected(void);
extern bool app_spp_tota_tx_buf_is_available(uint32_t dataLenToSend);

APP_TOTA_START_DATA_XFER_T tota_audio_dump_start_req = {
	.isHasCrcCheck = 0,
};

APP_TOTA_STOP_DATA_XFER_T tota_audio_dump_stop_req = {
	.isHasWholeCrcCheck = 0,
};

static bool _tota_is_connected(void)
{
    return app_tota_is_connected();
}

static bool _tota_buf_is_available(uint32_t dataLenToSend)
{
    return app_spp_tota_tx_buf_is_available(dataLenToSend);
}

static void _tota_tx_data_start(void)
{
	app_tota_start_data_xfer(APP_TOTA_VIA_SPP, &tota_audio_dump_start_req);
}

static void _tota_tx_data_stop(void)
{
	app_tota_stop_data_xfer(APP_TOTA_VIA_SPP, &tota_audio_dump_stop_req);	
}

static void _tota_tx_data_run(uint8_t *buf, uint32_t len)
{
	app_tota_send_data(APP_TOTA_VIA_SPP, buf, len);	
}

// ------------------------------ Cache data ------------------------------
#include "cmsis.h"
#define TOTA_TX_BUF_SIZE        (512)
uint8_t g_tota_tx_buf[TOTA_TX_BUF_SIZE];

// sample rate, bytes, channel number, time
#define DUMP_BUF_SIZE           (16000 * 2 * 1 * 1)
uint8_t g_dump_buf[DUMP_BUF_SIZE];
uint32_t g_write_pos = 0;

static uint32_t _data_len(void)
{
    return g_write_pos;
}

// static int32_t _write_data(uint8_t *buf, uint32_t len)
int32_t _write_data(uint8_t *buf, uint32_t len)
{
    // ASSERT(g_write_pos + len <= DUMP_BUF_SIZE, "[%s] g_write_pos(%d) + len(%d) > DUMP_BUF_SIZE(%d)", __func__, g_write_pos, len, DUMP_BUF_SIZE);
    if (g_write_pos + len > DUMP_BUF_SIZE) {
        ASSERT(1, "[%s] WARNING: Data buffer is full. Clean buffer...", __func__);
        g_write_pos = 0;
    }

    // TRACE(4, "[%s] g_write_pos: %d; len: %d; DUMP_BUF_SIZE: %d", __func__, g_write_pos, len, DUMP_BUF_SIZE);

    uint32_t lock = int_lock();
    memcpy(&g_dump_buf[g_write_pos], buf, len);
    g_write_pos += len;
    int_unlock(lock);

    return 0;
}

static int32_t _read_data(uint8_t *buf, uint32_t len)
{
    int32_t ret = 0;

    // TRACE(3, "[%s] g_write_pos: %d; len: %d", __func__, g_write_pos, len);

    if (len <= g_write_pos) {
        uint32_t lock = int_lock();
        memcpy(buf, g_dump_buf, len);
        // TODO: Use read pos to delete move operation
        memcpy(g_dump_buf, &g_dump_buf[len], g_write_pos - len);
        g_write_pos -= len;
        int_unlock(lock);
        ret = 1;
    } else {
        // TRACE(1, "[%s] SKIP...", __func__);
    }

    return ret;
}

static int32_t _send_data(void)
{
    int32_t ret = 0;

#if 1
    while (1) {
        if (_tota_buf_is_available(TOTA_TX_BUF_SIZE) == false) {
            TRACE(1, "[%s] tota tx buffer is not available!!!", __func__);
            break;
        } else if (_data_len() >= TOTA_TX_BUF_SIZE) {
            ret = _read_data(g_tota_tx_buf, TOTA_TX_BUF_SIZE);
            if (ret) {
                _tota_tx_data_run(g_tota_tx_buf, TOTA_TX_BUF_SIZE);
                // TRACE(3, "[%s] %d, %d", __func__, g_tota_tx_buf[0], g_tota_tx_buf[1]);
            }
        } else {
            // TRACE(1, "[%s] No data to send...", __func__);
            break;
        }
    }
#else
    ret = _read_data(g_tota_tx_buf, TOTA_TX_BUF_SIZE);
    if (ret) {
        _tota_tx_data_run(g_tota_tx_buf, TOTA_TX_BUF_SIZE);
        TRACE(3, "[%s] %d, %d", __func__, g_tota_tx_buf[0], g_tota_tx_buf[1]);
    }
#endif

    return 0;
}

// ------------------------------ Send data thread ------------------------------
#ifdef _SEND_DATA_USE_THREAD
#include "cmsis_os.h"
static osThreadId _dump_thread_tid;

#define TOTA_AUDIO_DUMP_STACK_SIZE          (1024 * 2)
static void _dump_thread(void const *argument);
osThreadDef(_dump_thread, osPriorityNormal, 1, TOTA_AUDIO_DUMP_STACK_SIZE, "DATA_DUMP_TOTA");
// osPriorityAboveNormal
// osPriorityNormal
// osPriorityBelowNormal

// osMutexId voice_tx_mutex_id = NULL;
// osMutexDef(voice_tx_mutex);

#define TOTA_AUDIO_DUMP_SIGNAL_READY   1

static void _dump_thread_trigger(void)
{
    osSignalSet(_dump_thread_tid, 0x01 << TOTA_AUDIO_DUMP_SIGNAL_READY);
}

static void _dump_thread(void const *argument)
{
    osEvent evt;
    uint32_t signals = 0;

    while (1) {
        //wait any signal
        evt = osSignalWait(0x0, osWaitForever);
        signals = evt.value.signals;
        //TRACE(3, "[%s] status = %x, signals = %d", __func__, evt.status, evt.value.signals);

        if (evt.status == osEventSignal) {
            // TRACE(1, "signal = %d", signals);
            if (signals & (1 << TOTA_AUDIO_DUMP_SIGNAL_READY)) {
                _send_data();
            }
        } else {
            ASSERT(0, "[%s] ERROR: evt.status = %d", __func__, evt.status);
            continue;
        }
    }
}

void _dump_thread_open(void)
{
    TRACE(1, "[%s] ......", __func__);

    // if (voice_tx_mutex_id == NULL) {
    //     voice_tx_mutex_id = osMutexCreate((osMutex(voice_tx_mutex)));
    // }

    g_write_pos = 0;

    _dump_thread_tid = osThreadCreate(osThread(_dump_thread), NULL);
    osSignalSet(_dump_thread_tid, 0x0);
}

void _dump_thread_close(void)
{
    TRACE(1, "[%s] ......", __func__);

    osThreadTerminate(_dump_thread_tid);
}
#endif

// ------------------------------ API ------------------------------
static bool connected_flag = false;
static bool started_flag = false;

void data_dump_tota_open(void)
{
    connected_flag = false;
    started_flag = false;

#ifdef _SEND_DATA_USE_THREAD
    _dump_thread_open();
#endif
}

void data_dump_tota_close(void)
{
#ifdef _SEND_DATA_USE_THREAD
    _dump_thread_close();
#endif

    connected_flag = false;
    started_flag = false;
}

void data_dump_tota_start(void)
{
    // If this function can not start successfully, do not dump data
    connected_flag = _tota_is_connected();
    if (connected_flag == false) {
        TRACE(1, "[%s] TOTA is disconnected!!!", __func__);
        return;
    }

    _tota_tx_data_start();

    started_flag = true;
}

void data_dump_tota_stop(void)
{
    started_flag = false;

    if (connected_flag == false) {
        return;
    }

    _tota_tx_data_stop();
}

void data_dump_tota_run(uint8_t *buf, uint32_t len)
{
    // TODO: Check tota status and start or stop
    if (connected_flag == false) {
        return;
    }

    if (started_flag == false) {
        return;
    }

    _write_data(buf, len);

#ifdef _SEND_DATA_USE_THREAD
    _dump_thread_trigger();
#else
    _send_data();
#endif
}