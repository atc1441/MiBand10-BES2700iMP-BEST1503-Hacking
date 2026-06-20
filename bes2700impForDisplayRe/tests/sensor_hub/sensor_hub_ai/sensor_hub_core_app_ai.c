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
#include "plat_types.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "sens_msg.h"
#include "sensor_hub_core.h"
#include "sensor_hub_core_app.h"
#include "sensor_hub_vad_core.h"
#include "app_sensor_hub.h"
#include "audioflinger.h"
#include "sensor_hub_vad_adapter.h"
#include "sensor_hub_core_app_ai.h"
#include "sensor_hub_core_app_ai_voice.h"
#include "sensor_hub_core_app_ai_ota.h"

static struct AI_RPC_REQ_MSG_T g_req_msg;
static struct AI_RPC_REQ_MSG_T g_received_msg;
static struct AI_RPC_REQ_MSG_T g_send_rsp_msg;

int app_sensor_hub_core_ai_reply_msg(uint16_t rpc_id,void * buf, uint16_t len);

#define CASE_ENUM(e) case e: return "["#e"]"

static const char *sens_ai_evt_to_str(SENSOR_HUB_AI_MSG_TYPE_E id)
{
    switch(id) {
        CASE_ENUM(SENSOR_HUB_AI_MSG_TYPE_KWS);
        CASE_ENUM(SENSOR_HUB_AI_MSG_TYPE_ENV);
        CASE_ENUM(SENSOR_HUB_AI_MSG_TYPE_ACTIVATE);
        CASE_ENUM(SENSOR_HUB_AI_DATA_UPDATE_OP);
        default:
            break;
    }
    return "";
}

static void app_sensor_hub_core_ai_cmd_received_handler(uint8_t* ptr, uint16_t len)
{
    struct AI_RPC_REQ_MSG_T *req_msg = &g_received_msg;
    memcpy((uint8_t*)req_msg,ptr,len);

    TRACE(0, "%s: len=%d", __func__, len);
//    DUMP8("%02x ",(uint8_t*)req_msg,MIN(len,32));

    if (req_msg->hdr.reply) {
        ASSERT(false, "%s: Bad reply msg: id=%d", __func__, req_msg->hdr.id);
    } else {
        if (SENS_MSG_ID_AI_MSG == req_msg->hdr.id ) {
            TRACE(2,"%s %s",__func__,sens_ai_evt_to_str(req_msg->rpc_id));
            switch (req_msg->rpc_id)
            {
                case SENSOR_HUB_AI_MSG_TYPE_ACTIVATE:
                {
                    app_sensor_hub_core_ai_reply_msg(SENSOR_HUB_AI_MSG_TYPE_ACTIVATE,NULL,0);
                    AI_ACTIVATE_OP_T *env_op;
                    env_op = (AI_ACTIVATE_OP_T *)(req_msg->param_buf);
                    sensor_hub_voice_kws_ai_user_activate(env_op->user,env_op->op);
                }
                    break;
                case SENSOR_HUB_AI_MSG_TYPE_ENV:
                {
                    app_sensor_hub_core_ai_reply_msg(SENSOR_HUB_AI_MSG_TYPE_ENV,NULL,0);
                    sensor_hub_voice_kws_ai_env_setup(req_msg->param_buf,req_msg->param_len);
                }
                break;
                case SENSOR_HUB_AI_DATA_UPDATE_OP:
                    app_sensor_hub_core_updating_section_data_process((req_msg->param_buf),req_msg->param_len);
                    break;
                default:
                    break;
            }
        }
    }
}

static int app_sensor_hub_ai_core_send_msg(uint16_t hdr_id,uint16_t rpc_id,void * buf, uint16_t len)
{
    int ret = 0;
    uint16_t send_len = 0;
    struct AI_RPC_REQ_MSG_T *req = &g_req_msg;

    memset(req, 0, sizeof(*req));
    req->hdr.id = hdr_id;
    req->hdr.reply = 0;
    req->rpc_id = rpc_id;

    req->param_len = len;
    if(len){
        memcpy(req->param_buf,buf,len);
    }
    send_len = OFFSETOF(struct AI_RPC_REQ_MSG_T,param_buf);
    send_len += len;
    ret = app_core_bridge_send_cmd(MCU_SENSOR_HUB_TASK_CMD_AI, (uint8_t *)req, send_len);
    ASSERT(ret == 0, "%s: sensor_hub_send failed: %d", __func__, ret);

    return ret;

}

static int app_sensor_hub_core_ai_send_rsp_msg(enum SENS_MSG_ID_T msg_id, uint16_t rpc_id, void * buf, uint16_t len)
{
    int ret;
    uint16_t send_len = 0;
    struct AI_RPC_REQ_MSG_T *rsp = &g_send_rsp_msg;

    rsp->hdr.id    = msg_id;
    rsp->hdr.reply = 1;
    rsp->rpc_id    = rpc_id;
    rsp->param_len = len;
    if(len){
        memcpy(rsp->param_buf,buf,len);
    }
    send_len = OFFSETOF(struct AI_RPC_REQ_MSG_T,param_buf);
    send_len += len;
    ret = app_core_bridge_send_rsp(MCU_SENSOR_HUB_TASK_CMD_AI, (uint8_t *)rsp, send_len);
    ASSERT(ret == 0, "%s: sensor_hub_send failed: %d", __func__, ret);

    return ret;
}

int app_sensor_hub_core_ai_reply_msg(uint16_t rpc_id,void * buf, uint16_t len)
{
    app_sensor_hub_core_ai_send_rsp_msg(SENS_MSG_ID_AI_MSG,  rpc_id,buf,len);

    return 0;
}

int app_sensor_hub_core_ai_send_msg(uint16_t rpc_id,void * buf, uint16_t len)
{
    app_sensor_hub_ai_core_send_msg(SENS_MSG_ID_AI_MSG, rpc_id,buf,len);
    return 0;
}

static void app_sensor_hub_core_ai_cmd_trasmit_handler(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "%s: len=%d", __func__, len);
    app_core_bridge_send_data_with_waiting_rsp(MCU_SENSOR_HUB_TASK_CMD_AI, ptr, len);
}

static void app_sensor_hub_core_ai_cmd_wait_rsp_timeout(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "%s: len=%d", __func__, len);
    DUMP8("%02x ", ptr, len);
}

static void app_sensor_hub_core_ai_cmd_rsp_received_handler(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "%s: len=%d", __func__, len);
    DUMP8("%02x ", ptr, len);
}

static void app_sensor_hub_core_ai_cmd_tx_done_handler(uint16_t cmdCode, uint8_t* ptr, uint16_t len)
{
    TRACE(0, "%s: cmdCode=0x%x, len=%d", __func__, cmdCode, len);
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_AI,
                                "task cmd ai",
                                app_sensor_hub_core_ai_cmd_trasmit_handler,
                                app_sensor_hub_core_ai_cmd_received_handler,
                                APP_CORE_BRIDGE_DEFAULT_WAIT_RSP_TIMEOUT_MS,
                                app_sensor_hub_core_ai_cmd_wait_rsp_timeout,
                                app_sensor_hub_core_ai_cmd_rsp_received_handler,
                                app_sensor_hub_core_ai_cmd_tx_done_handler);

#endif
