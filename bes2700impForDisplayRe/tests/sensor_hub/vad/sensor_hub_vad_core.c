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
#ifdef VOICE_DETECTOR_EN
#include "string.h"
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "analog.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hwtimer_list.h"
#include "audioflinger.h"
#include "sens_msg.h"
#include "sensor_hub_core.h"
#include "sensor_hub_core_app.h"
#include "sensor_hub_vad_core.h"
#include "sensor_hub_vad_adapter.h"
#ifdef VAD_ADPT_TEST_ENABLE
#include "sensor_hub_vad_adapter_tc.h"
#endif
#include "app_sensor_hub.h"
#include "app_thread.h"

static struct ANA_RPC_REQ_MSG_T vad_rpc_req;
POSSIBLY_UNUSED static struct ANA_RPC_REPLY_MSG_T vad_rsp_msg;
POSSIBLY_UNUSED static struct VAD_RPC_REQ_DATA_MSG_T vad_req_data_msg;

static int app_sensor_hub_core_vad_send_rsp_msg(enum SENS_MSG_ID_T msg_id, uint16_t rpc_id, uint32_t result)
{
#ifndef NOAPP
    int ret;
    struct ANA_RPC_REPLY_MSG_T *rsp = &vad_rsp_msg;

    rsp->hdr.id    = msg_id;
    rsp->hdr.reply = 1;
    rsp->rpc_id    = rpc_id;
    rsp->ret       = result;
    ret = app_core_bridge_send_rsp(MCU_SENSOR_HUB_TASK_CMD_VAD, (uint8_t *)rsp, sizeof(*rsp));
    ASSERT(ret == 0, "%s: sensor_hub_send failed: %d", __func__, ret);
#endif
    return 0;
}

static int app_sensor_hub_core_send_req_msg(uint16_t hdr_id, uint16_t rpc_id,
    uint32_t p0, uint32_t p1, uint32_t p2)
{
    int ret = 0;
    struct ANA_RPC_REQ_MSG_T *req = &vad_rpc_req;

    memset(req, 0, sizeof(*req));
    req->hdr.id   = hdr_id;
    req->rpc_id   = rpc_id;
    req->param[0] = p0;
    req->param[1] = p1;
    req->param[2] = p2;
    ret = app_core_bridge_send_cmd(MCU_SENSOR_HUB_TASK_CMD_VAD, (uint8_t *)req, sizeof(*req));
    ASSERT(ret == 0, "%s: sensor_hub_send failed: %d", __func__, ret);

    return ret;
}

int app_sensor_hub_core_vad_send_data_msg(uint32_t data_idx, uint32_t data_addr, uint32_t data_size)
{
    int ret = 0;
    struct VAD_RPC_REQ_DATA_MSG_T *req = &vad_req_data_msg;

    memset(req, 0, sizeof(*req));
    req->hdr.id = SENS_MSG_ID_VAD_DATA;
    req->data_idx  = data_idx;
    req->data_addr = data_addr;
    req->data_size = data_size;
    ret = app_core_bridge_send_cmd(MCU_SENSOR_HUB_TASK_CMD_VAD, (uint8_t *)req, sizeof(*req));
    ASSERT(ret == 0, "%s: sensor_hub_send failed: %d", __func__, ret);

    return ret;
}

static bool vad_adpt_opened = false;
static enum APP_VAD_ADPT_ID_T vad_adpt_id = APP_VAD_ADPT_ID_NONE;

static void app_sensor_hub_core_handle_vad_cmd(uint32_t *data, uint16_t len)
{
    struct ANA_RPC_REQ_MSG_T *req;
    enum SENS_VAD_CMD_ID_T cmd;

    bool active_state = false;
    bool request_state = false;
    uint32_t data_size = 0;
    uint32_t seek_size = 0;

    req = (struct ANA_RPC_REQ_MSG_T *)data;
    cmd = (enum ANALOG_RPC_ID_T)(req->rpc_id);

    TRACE(0, "%s: cmd=[%d]", __func__, cmd);

    (void)active_state;
    (void)request_state;
    (void)data_size;
    (void)seek_size;

    if ((cmd == SENS_CMD_ID_VAD_START)
        || (cmd == SENS_CMD_ID_VAD_STOP)) {
        vad_adpt_id = req->param[0];
        TRACE(0, "%s: set vad_adpt_id = %d", __func__, vad_adpt_id);
    } else if (cmd == SENS_CMD_ID_VAD_REQ_DATA) {
        request_state = req->param[0];
        data_size  = req->param[1];
        seek_size = req->param[2];
        TRACE(4, "%s: request_state=%d, data_size=%d seek = %d", __func__, request_state, data_size,seek_size);
    } else if(cmd == SENS_CMD_ID_VAD_SEEK_DATA){
        seek_size = req->param[0];
    } else if (cmd == SENS_CMD_ID_VAD_SET_ACTIVE) {
        active_state = req->param[0];
        TRACE(0, "%s: set active_state = %d", __func__, active_state);
#ifdef VAD_ADPT_TEST_ENABLE
    } else if (cmd == SENS_CMD_ID_VAD_TEST) {
        if (!vad_adpt_opened) {
            TRACE(0, "%s:VAD is not opened, ignore test cmd", __func__);
            goto _exit;
        }
#endif /* VAD_ADPT_TEST_ENABLE  */
    } else {
        goto _exit;
    }

#ifndef VD_TEST
    // only open adapter once
    if (!vad_adpt_opened) {
        app_vad_adapter_open(vad_adpt_id);
        vad_adpt_opened = true;
    }
    // start/stop vad adapter
    switch (cmd) {
    case SENS_CMD_ID_VAD_START:
        TRACE(0, "SENS_CMD_ID_VAD_START");
        app_vad_adapter_open(vad_adpt_id);
        app_vad_adapter_start(vad_adpt_id);
        break;
    case SENS_CMD_ID_VAD_STOP:
        TRACE(0, "SENS_CMD_ID_VAD_STOP");
        app_vad_adapter_stop(vad_adpt_id);
//        app_vad_adapter_close(vad_adpt_id);
        break;
    case SENS_CMD_ID_VAD_REQ_DATA:
        TRACE(0, "SENS_CMD_ID_VAD_REQ_DATA");
        app_vad_adapter_set_vad_request(vad_adpt_id, request_state, data_size,seek_size);
        break;
    case SENS_CMD_ID_VAD_SEEK_DATA:
        app_vad_adapter_seek_data_update(seek_size);
        break;
    case SENS_CMD_ID_VAD_SET_ACTIVE:
        TRACE(0, "SENS_CMD_ID_VAD_SET_ACTIVE");
        app_vad_adapter_set_vad_active(vad_adpt_id, active_state);
        break;
#ifdef VAD_ADPT_TEST_ENABLE
    case SENS_CMD_ID_VAD_TEST:
    {
        uint32_t scmd = req->param[0];
        uint32_t arg1 = req->param[1];
        uint32_t arg2 = req->param[2];

        TRACE(0, "SENS_CMD_ID_VAD_TEST");
        app_vad_adapter_test_handler(vad_adpt_id, scmd, arg1, arg2);
    }
        break;
#endif /* VAD_ADPT_TEST_ENABLE  */
    default:
        ASSERT(false, "%s: Bad rpc id: %d", __func__, cmd);
        break;
    }
#endif

_exit:
    app_sensor_hub_core_vad_send_rsp_msg(SENS_MSG_ID_VAD_CMD, cmd, 0);
}

static void app_sensor_hub_core_vad_cmd_trasmit_handler(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "%s: len=%d", __func__, len);
#ifndef NOAPP
    app_core_bridge_send_data_with_waiting_rsp(MCU_SENSOR_HUB_TASK_CMD_VAD, ptr, len);
#endif
}

static void app_sensor_hub_core_vad_cmd_received_handler(uint8_t* ptr, uint16_t len)
{
    uint32_t data[SENS_VAD_MSG_MAX_SIZE/4] = {0};
    struct SENS_MSG_HDR_T *hdr;

    //DUMP8("%02x ", ptr, len);

    ASSERT(len<sizeof(data), "%s: invalid len=%d >= %d", __func__, len, sizeof(data));
    memcpy(data, ptr, len);
    hdr = (struct SENS_MSG_HDR_T *)data;
    if (hdr->reply) {
        ASSERT(false, "%s: Bad reply msg: id=%d", __func__, hdr->id);
    } else {
        if (hdr->id == SENS_MSG_ID_VAD_CMD) {
            app_sensor_hub_core_handle_vad_cmd(data, len);
        }
    }
}

static void app_sensor_hub_core_vad_cmd_wait_rsp_timeout(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "%s: len=%d", __func__, len);
//    DUMP8("%02x ", ptr, len);
}

static void app_sensor_hub_core_vad_cmd_rsp_received_handler(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "%s: len=%d", __func__, len);
//    DUMP8("%02x ", ptr, len);
}

static void app_sensor_hub_core_vad_cmd_tx_done_handler(uint16_t cmdCode, uint8_t* ptr, uint16_t len)
{
    TRACE(0, "%s: cmdCode=0x%x, len=%d", __func__, cmdCode, len);
}

int app_sensor_hub_core_vad_send_evt_msg(enum SENS_VAD_EVT_ID_T id, uint32_t param0, uint32_t param1, uint32_t param2)
{
    TRACE(0, "%s: opened = %d evt=%d param=0x%X/0x%X/0x%X", __func__, vad_adpt_opened,id, param0, param1, param2);
    if (vad_adpt_opened) {
        app_vad_adapter_event_handler(vad_adpt_id, id, param0, param1, param2);
    }
    return app_sensor_hub_core_send_req_msg(SENS_MSG_ID_VAD_EVT, id, param0, param1, param2);
}

static int app_sensor_hub_core_vad_rpc_cb(enum ANALOG_RPC_ID_T id, uint32_t param0, uint32_t param1, uint32_t param2)
{
    TRACE(0, "%s: id=%d param=0x%X/0x%X/0x%X", __func__, id, param0, param1, param2);
    return app_sensor_hub_core_send_req_msg(SENS_MSG_ID_ANA_RPC, id, param0, param1, param2);
}

void app_sensor_hub_core_vad_init(void)
{
    TRACE(0, "%s:", __func__);
    analog_aud_register_rpc_callback(app_sensor_hub_core_vad_rpc_cb);
    app_vad_adapter_init();
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_VAD,
                                "task cmd vad",
                                app_sensor_hub_core_vad_cmd_trasmit_handler,
                                app_sensor_hub_core_vad_cmd_received_handler,
                                APP_CORE_BRIDGE_DEFAULT_WAIT_RSP_TIMEOUT_MS,
                                app_sensor_hub_core_vad_cmd_wait_rsp_timeout,
                                app_sensor_hub_core_vad_cmd_rsp_received_handler,
                                app_sensor_hub_core_vad_cmd_tx_done_handler);
#endif /* VOICE_DETECTOR_EN */

