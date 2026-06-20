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
#ifndef CHIP_SUBSYS_SENS
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_trace.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "app_sensor_hub.h"
#include "sensor_hub.h"
#include "mcu_sensor_hub_app.h"
#include "analog.h"
#include "string.h"
#include "sens_msg.h"

static app_vad_data_handler_t app_vad_data_handler = NULL;
static app_vad_event_handler_t app_vad_event_handler = NULL;
static struct ANA_RPC_REQ_MSG_T vad_rpc_msg;
static struct ANA_RPC_REPLY_MSG_T vad_rsp_msg;

#define CASE_ENUM(e) case e: return "["#e"]"
static const char *rpc_cmd_to_str(enum ANALOG_RPC_ID_T id)
{
    switch(id) {
    CASE_ENUM(ANALOG_RPC_FREQ_PLL_CFG);
    CASE_ENUM(ANALOG_RPC_OSC_CLK_ENABLE);
    CASE_ENUM(ANALOG_RPC_CODEC_OPEN);
    CASE_ENUM(ANALOG_RPC_CODEC_ADC_ENABLE);
    CASE_ENUM(ANALOG_RPC_VAD_ENABLE);
    CASE_ENUM(ANALOG_RPC_VAD_ADC_ENABLE);
    }
    return "";
}

static const char *vad_evt_to_str(enum SENS_VAD_EVT_ID_T evt)
{
    switch(evt) {
    CASE_ENUM(SENS_EVT_ID_VAD_IRQ_TRIG);   //vad FIND irq is triggerred
    CASE_ENUM(SENS_EVT_ID_VAD_VOICE_CMD);  //key word is recognized
    CASE_ENUM(SENS_EVT_ID_VAD_CMD_DONE);   //vad command execution done
    }
    return "";
}

#ifndef HAL_SYS_WAKE_LOCK_USER_VAD
#define HAL_SYS_WAKE_LOCK_USER_VAD HAL_SYS_WAKE_LOCK_USER_16
#endif

#ifndef HAL_SYSFREQ_SENS_VAD_USER
#define HAL_SYSFREQ_SENS_VAD_USER HAL_SYSFREQ_USER_APP_13
#endif

static void vad_lock_mcu_core_clock(bool lock)
{
    static int sys_lock = false;
    uint32_t l;

    if (lock == sys_lock) {
        return;
    }
    l = int_lock();
    if (lock) {
        hal_sysfreq_req(HAL_SYSFREQ_SENS_VAD_USER, HAL_CMU_FREQ_52M);
        hal_sys_wake_lock(HAL_SYS_WAKE_LOCK_USER_VAD);
        sys_lock = true;;
    } else {
        sys_lock = false;
        hal_sys_wake_unlock(HAL_SYS_WAKE_LOCK_USER_VAD);
        hal_sysfreq_req(HAL_SYSFREQ_SENS_VAD_USER, HAL_CMU_FREQ_32K);
    }
    int_unlock(l);
    TRACE(0, "%s: lock=%d", __func__, lock);
}

static int mcu_core_send_vad_msg(uint16_t cmd, uint32_t p0, uint32_t p1, uint32_t p2)
{
    int ret;

    if ((cmd == SENS_CMD_ID_VAD_START)
        |(cmd == SENS_CMD_ID_VAD_STOP)
        |((cmd == SENS_CMD_ID_VAD_REQ_DATA) && (!p0))) {
        vad_lock_mcu_core_clock(true);
    }
    TRACE(0, "%s: cmd=%d param=0x%X/0x%X/0x%X", __func__, cmd, p0, p1, p2);

    memset(&vad_rpc_msg, 0, sizeof(vad_rpc_msg));
    vad_rpc_msg.hdr.id   = SENS_MSG_ID_VAD_CMD;
    vad_rpc_msg.rpc_id   = cmd;
    vad_rpc_msg.param[0] = p0;
    vad_rpc_msg.param[1] = p1;
    vad_rpc_msg.param[2] = p2;

    ret = app_core_bridge_send_cmd(MCU_SENSOR_HUB_TASK_CMD_VAD, (uint8_t *)&vad_rpc_msg, sizeof(vad_rpc_msg));
    ASSERT(ret == 0, "%s: sensor_hub_send failed: %d", __func__, ret);
    return 0;
}

static void mcu_core_send_vad_reply_msg(uint16_t msg_id, uint16_t rpc_id, uint32_t result)
{
    int ret;

    vad_rsp_msg.hdr.id    = msg_id;
    vad_rsp_msg.hdr.reply = 1;
    vad_rsp_msg.rpc_id    = rpc_id;
    vad_rsp_msg.ret       = result;
    ret = app_core_bridge_send_rsp(MCU_SENSOR_HUB_TASK_CMD_VAD, (uint8_t *)&vad_rsp_msg, sizeof(vad_rsp_msg));
    ASSERT(ret == 0, "%s: sensor_hub_send failed: %d", __func__, ret);
}

static void mcu_core_handle_vad_ana_rpc_msg(uint32_t *data, uint16_t len)
{
    struct ANA_RPC_REQ_MSG_T *ana_req;
    enum ANALOG_RPC_ID_T ana_id;

    ana_req = (struct ANA_RPC_REQ_MSG_T *)data;
    ana_id = (enum ANALOG_RPC_ID_T)(ana_req->rpc_id);

    TRACE(0, "%s: %s [%d]", __func__, rpc_cmd_to_str(ana_id), ana_id);

    switch (ana_id) {
    case ANALOG_RPC_FREQ_PLL_CFG:
// FIXME:vad not currently supported by 1502p
#if !defined(CHIP_BEST1502P)
        analog_rpcsvr_freq_pll_config(ana_req->param[0], ana_req->param[1]);
#endif
        break;
    case ANALOG_RPC_OSC_CLK_ENABLE:
#if !defined(CHIP_BEST1502P)
        analog_rpcsvr_osc_clk_enable(ana_req->param[0]);
#endif
        break;
    case ANALOG_RPC_CODEC_OPEN:
#if !defined(CHIP_BEST1502P)
        analog_rpcsvr_codec_open(ana_req->param[0]);
#endif
        break;
    case ANALOG_RPC_CODEC_ADC_ENABLE:
#if !defined(CHIP_BEST1502P)
        analog_rpcsvr_codec_adc_enable((enum AUD_IO_PATH_T)(ana_req->param[0]),
            (enum AUD_CHANNEL_MAP_T)(ana_req->param[1]), (bool)(ana_req->param[2]));
#endif
        break;
    case ANALOG_RPC_VAD_ENABLE:
#if !defined(CHIP_BEST1502P)
        analog_rpcsvr_vad_enable((enum AUD_VAD_TYPE_T)(ana_req->param[0]), (bool)(ana_req->param[1]));
#endif
        break;
    case ANALOG_RPC_VAD_ADC_ENABLE:
#if !defined(CHIP_BEST1502P)
        analog_rpcsvr_vad_adc_enable(ana_req->param[0]);
#endif
        break;
    default:
        ASSERT(false, "%s: Bad ana rpc id: %d", __func__, ana_id);
        break;
    }
    mcu_core_send_vad_reply_msg(SENS_MSG_ID_ANA_RPC, ana_id, 0);
}

static void mcu_core_handle_vad_data_msg(uint32_t *data, uint16_t len)
{
    struct VAD_RPC_REQ_DATA_MSG_T *msg = (struct VAD_RPC_REQ_DATA_MSG_T *)data;

    mcu_core_send_vad_reply_msg(SENS_MSG_ID_VAD_DATA, msg->data_idx, 0);

    TRACE(0, "%s:idx=%d,addr=0x%x,size=%d", __func__, msg->data_idx, msg->data_addr, msg->data_size);

    if (app_vad_data_handler) {
        app_vad_data_handler(msg->data_idx, (uint8_t *)(msg->data_addr), msg->data_size);
    }
}

static void mcu_core_handle_vad_evt_msg(uint32_t *data, uint16_t len)
{
    struct ANA_RPC_REQ_MSG_T *rpc_msg = (struct ANA_RPC_REQ_MSG_T *)data;
    enum SENS_VAD_EVT_ID_T evt = (enum SENS_VAD_EVT_ID_T)rpc_msg->rpc_id;
    uint8_t *param = (uint8_t *)&rpc_msg->param;

    TRACE(0, "%s:evt=%s[%d],param:%d/%d/%d", __func__,vad_evt_to_str(evt),evt,param[0],param[1],param[2]);

    if (evt == SENS_EVT_ID_VAD_CMD_DONE) {
        vad_lock_mcu_core_clock(false);
    }
    mcu_core_send_vad_reply_msg(SENS_MSG_ID_VAD_EVT, evt, 0);
    if (app_vad_event_handler) {
        app_vad_event_handler((enum APP_VAD_EVENT_T)evt, param);
    }
}

static void mcu_core_handle_vad_af_buf_msg(uint32_t *data, uint16_t len)
{
    struct AF_RPC_REQ_MSG_T *af_req;
    af_req = (struct AF_RPC_REQ_MSG_T *)data;

    TRACE(0, "%s: Rx af buf msg: stream=%d", __func__, af_req->stream_id);
    mcu_core_send_vad_reply_msg(SENS_MSG_ID_VAD_EVT, af_req->stream_id, 0);
}

static void app_mcu_sensor_hub_vad_cmd_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_core_bridge_send_data_with_waiting_rsp(MCU_SENSOR_HUB_TASK_CMD_VAD, ptr, len);
}

static void app_mcu_sensor_hub_vad_cmd_received_handler(uint8_t* ptr, uint16_t len)
{
    uint32_t data[SENS_VAD_MSG_MAX_SIZE/sizeof(uint32_t)] = {0};
    struct SENS_MSG_HDR_T *hdr;

    TRACE(0, "%s:", __func__);

    ASSERT(len<sizeof(data), "%s: invalid len %d>%d", __func__, len, sizeof(data));
    memcpy(data, ptr, len);
    hdr = (struct SENS_MSG_HDR_T *)data;
    if (hdr->reply) {
        ASSERT(false, "%s: Bad reply msg: id=%d", __func__, hdr->id);
    } else {
        if (hdr->id == SENS_MSG_ID_ANA_RPC) {
            mcu_core_handle_vad_ana_rpc_msg(data, len);
        } else if (hdr->id == SENS_MSG_ID_AF_BUF) {
            mcu_core_handle_vad_af_buf_msg(data, len);
        } else if (hdr->id == SENS_MSG_ID_VAD_EVT) {
            mcu_core_handle_vad_evt_msg(data, len);
        } else if (hdr->id == SENS_MSG_ID_VAD_DATA) {
            mcu_core_handle_vad_data_msg(data, len);
        } else {
            ASSERT(false, "%s: Bad msg: id=%d", __func__, hdr->id);
        }
    }
//    DUMP8("%02x ", ptr, len);
}

static void app_mcu_sensor_hub_vad_cmd_wait_rsp_timeout(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "%s: len=%d", __func__, len);
}

static void app_mcu_sensor_hub_vad_cmd_rsp_received_handler(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "%s: len=%d", __func__, len);
}

static void app_mcu_sensor_hub_vad_cmd_tx_done_handler(uint16_t cmdCode, uint8_t* ptr, uint16_t len)
{
    TRACE(0, "%s: cmdCode=0x%x", __func__, cmdCode);
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_VAD,
                                "task cmd vad",
                                app_mcu_sensor_hub_vad_cmd_transmit_handler,
                                app_mcu_sensor_hub_vad_cmd_received_handler,
                                APP_CORE_BRIDGE_DEFAULT_WAIT_RSP_TIMEOUT_MS,
                                app_mcu_sensor_hub_vad_cmd_wait_rsp_timeout,
                                app_mcu_sensor_hub_vad_cmd_rsp_received_handler,
                                app_mcu_sensor_hub_vad_cmd_tx_done_handler);

int app_mcu_sensor_hub_start_vad(uint8_t vad_adaptor_id)
{
    return mcu_core_send_vad_msg(SENS_CMD_ID_VAD_START, vad_adaptor_id, 0, 0);
}

int app_mcu_sensor_hub_stop_vad(uint8_t vad_adaptor_id)
{
    return mcu_core_send_vad_msg(SENS_CMD_ID_VAD_STOP, vad_adaptor_id, 0, 0);
}

int app_mcu_sensor_hub_setup_vad_event_handler(app_vad_event_handler_t handler)
{
    app_vad_event_handler = handler;
    return 0;
}

int app_mcu_sensor_hub_setup_vad_data_handler(app_vad_data_handler_t handler)
{
    app_vad_data_handler = handler;
    return 0;
}

int app_mcu_sensor_hub_request_vad_data(bool request,uint32_t seek_size)
{
    return mcu_core_send_vad_msg(SENS_CMD_ID_VAD_REQ_DATA, request, APP_VAD_DATA_PKT_SIZE, seek_size);
}

int app_mcu_sensor_hub_seek_vad_data(uint32_t size)
{
    return mcu_core_send_vad_msg(SENS_CMD_ID_VAD_SEEK_DATA, size, 0, 0);
}

int app_mcu_sensor_hub_bypass_vad(bool bypass)
{
    bool active = bypass ? false : true;
    return mcu_core_send_vad_msg(SENS_CMD_ID_VAD_SET_ACTIVE, active, 0, 0);
}

int app_mcu_sensor_hub_send_test_msg(uint32_t scmd, uint32_t arg1, uint32_t arg2)
{
    return mcu_core_send_vad_msg(SENS_CMD_ID_VAD_TEST, scmd, arg1, arg2);
}

#endif

