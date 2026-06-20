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
#include "app_sensor_hub.h"
#include "sensor_hub.h"
#include "mcu_sensor_hub_app.h"
#include "analog.h"
#include "string.h"
#include "sens_msg.h"
#include "sensor_hub_core_app_ai_ota.h"
#include "mcu_sensor_hub_app_ai.h"
#include "mcu_sensor_hub_app_ai_ota.h"
#include "crc32_c.h"
#include "app_utils.h"

#define CASE_ENUM(e) case e: return "["#e"]"

#define MCU_SENSOR_HUB_VAD_AI_VOICE_ADAPT_ID    APP_VAD_ADPT_ID_AI_KWS
//#define MCU_SENSOR_HUB_AUDIO_DUMP_EN

#ifdef MCU_SENSOR_HUB_AUDIO_DUMP_EN
#include "audio_dump.h"
#endif

int app_sensor_hub_mcu_ai_send_msg(uint16_t rpc_id,void * buf, uint16_t len);
int app_sensor_hub_mcu_ai_reply_msg(uint32_t status,uint32_t rpc_id);


static struct AI_RPC_REQ_MSG_T g_req_msg;
static struct AI_RPC_REQ_MSG_T g_received_req_msg;
static struct AI_RPC_REQ_MSG_T g_revceived_rsp_msg;
POSSIBLY_UNUSED static struct AI_RPC_REQ_MSG_T g_send_rsp_msg;


static SENSOR_HUB_AI_OPERATOR_MANAGER_T sensor_hub_ai_operator_manager_instant;

static bool mcu_sensor_hub_inited = false;
static const char *sens_ai_evt_to_str(SENSOR_HUB_AI_MSG_TYPE_E id)
{
    switch(id) {
        CASE_ENUM(SENSOR_HUB_AI_DATA_UPDATE_OP);
        CASE_ENUM(SENSOR_HUB_AI_MSG_TYPE_KWS);
        default:
            break;
    }
    return "";
}

static inline bool sensor_hub_ai_mcu_find_user(SENSOR_HUB_AI_USER_E user, uint16_t *index)
{
    uint8_t i =0;
    bool found = false;

    if(SENSOR_HUB_AI_USER_NONE == user){
        return found;
    }

    for (i = 0; i <SENSOR_HUB_MAXIAM_AI_USER;i++){
        if(sensor_hub_ai_operator_manager_instant.ai_operator[i].ai_user == user){
            found = true;
            *index = i;
            break;
        }
    }

    return found;
}

static inline bool sensor_hub_ai_mcu_allocate_user(uint16_t *index)
{
    uint8_t i =0;
    bool found = false;

    for (i = 0; i <SENSOR_HUB_MAXIAM_AI_USER;i++){
        if(SENSOR_HUB_AI_USER_NONE == sensor_hub_ai_operator_manager_instant.ai_operator[i].ai_user){
            *index = i;
            found = true;
            break;
        }
    }

    return found;
}

void sensor_hub_ai_mcu_register_ai_user(SENSOR_HUB_AI_USER_E user,SENSOR_HUB_AI_OPERATOR_T ai_operator)
{
    if(SENSOR_HUB_AI_USER_NONE == user){
        TRACE(1,"%s role invalid",__func__);
        return ;
    }

    uint16_t index =0;
    bool found = false;

    if(sensor_hub_ai_mcu_find_user(user,&index) == true){
        found = true;
    }

    if(found == true){
        sensor_hub_ai_operator_manager_instant.ai_operator[index] = ai_operator;
    }else{
        if(SENSOR_HUB_MAXIAM_AI_USER == sensor_hub_ai_operator_manager_instant.ai_user_num){
            found = sensor_hub_ai_mcu_allocate_user(&index);
            if(found == false){
                TRACE(0,"already have the maxiam ai user . can not add");
            }else{
                sensor_hub_ai_operator_manager_instant.ai_operator[index].ai_user = user;
                sensor_hub_ai_operator_manager_instant.ai_operator[index] = ai_operator;
            }
            return ;
        }else{
            sensor_hub_ai_operator_manager_instant.ai_operator[index].ai_user = user;
            sensor_hub_ai_operator_manager_instant.ai_operator[sensor_hub_ai_operator_manager_instant.ai_user_num] = ai_operator;
            sensor_hub_ai_operator_manager_instant.ai_user_num++;
        }
    }

    TRACE(5,"%s user = %d update ?= %d num = %d current = %d",__func__,user,found,sensor_hub_ai_operator_manager_instant.ai_user_num,sensor_hub_ai_operator_manager_instant.ai_current_user);
}

void sensor_hub_ai_mcu_unregister_ai_user(SENSOR_HUB_AI_USER_E user)
{
    if(SENSOR_HUB_AI_USER_NONE == user){
        TRACE(1,"%s role invalid",__func__);
        return ;
    }

    uint16_t index =0;
    bool found = false;

    if(sensor_hub_ai_mcu_find_user(user,&index) == true){
        found = true;
    }

    if(found){
        sensor_hub_ai_operator_manager_instant.ai_operator[index].ai_user = SENSOR_HUB_AI_USER_NONE;
        if(sensor_hub_ai_operator_manager_instant.ai_current_user == user){
            sensor_hub_ai_operator_manager_instant.ai_current_user = SENSOR_HUB_AI_USER_NONE;
        }
    }
}

uint32_t sensor_hub_ai_mcu_get_kws_history_data_size(uint16_t user)
{
    return sensor_hub_ai_operator_manager_instant.ai_operator[user].history_pcm_data_size;
}

void sensor_hub_ai_mcu_kws_history_data_size_set(uint16_t user,uint32_t size)
{
    sensor_hub_ai_operator_manager_instant.ai_operator[user].history_pcm_data_size = size;
}

void sensor_hub_ai_mcu_operator_init(void)
{
    memset((uint8_t*)&sensor_hub_ai_operator_manager_instant,0,sizeof(SENSOR_HUB_AI_OPERATOR_MANAGER_T));
}

static inline void sensor_hub_ai_mcu_kws_infor_process_handler(uint8_t * buf,uint16_t len)
{
    AI_KWS_INFOR_T *kws_info = (AI_KWS_INFOR_T *)buf;
    SENSOR_HUB_AI_USER_E user = (SENSOR_HUB_AI_USER_E)kws_info->kws_infor.ai_user;
    uint32_t seek_size =0;
    uint16_t index = 0;

    len -= OFFSETOF(AI_KWS_INFOR_T, kws_infor);
    len -= OFFSETOF(AI_KWS_INFOR_PAYLOAD_T,p);

    TRACE(3,"%s user = %d historyBytes = %d ",__func__,user,kws_info->env_info.history_pcm_data_len);
    app_sysfreq_req(APP_SYSFREQ_USER_AI_VOICE, SENSOR_HUB_MCU_AI_REQUEST_FREQ);

    if(sensor_hub_ai_mcu_find_user(user,&index) == true){
        sensor_hub_ai_operator_manager_instant.ai_current_user = user;

        sensor_hub_ai_mcu_kws_word_info_handler_t kws_handler = sensor_hub_ai_operator_manager_instant.ai_operator[index].handler.kws_info_handler;
        if(kws_handler){
            kws_handler((uint8_t*)&(kws_info->kws_infor.p),len);
        }

        sensor_hub_ai_mcu_kws_history_pcm_data_size_update history_data_handler = sensor_hub_ai_operator_manager_instant.ai_operator[index].handler.history_pcm_size_update_handler;
        if(history_data_handler){
            history_data_handler(kws_info->env_info.history_pcm_data_len);
        }

        sensor_hub_ai_mcu_kws_valid_raw_pcm_sample_check valid_sample_check_handler = sensor_hub_ai_operator_manager_instant.ai_operator[index].handler.kws_raw_pcm_sample_check_handler;
        if(valid_sample_check_handler){
            seek_size = valid_sample_check_handler(buf,len);
        }

        app_mcu_sensor_hub_request_vad_data(true,seek_size);
    }

}

static void app_sensor_hub_ai_mcu_vad_event_handler(enum APP_VAD_EVENT_T event, uint8_t *param)
{
    TRACE(0, "%s: event=%d", __func__, event);
    if (APP_VAD_EVT_VOICE_CMD == event) {
//        app_mcu_sensor_hub_request_vad_data(true,0);
        TRACE(1, "%s",__func__);
    }
}

static void app_sensor_hub_ai_mcu_vad_data_handler(uint32_t pkt_id, uint8_t *payload, uint32_t bytes)
{
    TRACE(4, "%s: pkt_id=%d, payload = %p bytes=%d", __func__, pkt_id, payload,bytes);
    uint16_t user = sensor_hub_ai_operator_manager_instant.ai_current_user;
    uint16_t index ;

#ifdef MCU_SENSOR_HUB_AUDIO_DUMP_EN
    audio_dump_add_channel_data(0, (void *)payload, bytes/2);
    audio_dump_run();
#endif

    if(sensor_hub_ai_mcu_find_user((SENSOR_HUB_AI_USER_E)user,&index) == true){
        sensor_hub_ai_mcu_mic_data_come_handler_t handler = sensor_hub_ai_operator_manager_instant.ai_operator[index].handler.mic_come_handler;
        if(handler){
            handler(payload,bytes);
        }
    }
}

static void app_sensor_hub_mcu_ai_msg_handler(uint8_t* buf, uint16_t len)
{
    struct AI_RPC_REQ_MSG_T *rpc_msg = (struct AI_RPC_REQ_MSG_T *)buf;
    SENSOR_HUB_AI_MSG_TYPE_E evt = (SENSOR_HUB_AI_MSG_TYPE_E)rpc_msg->rpc_id;

    uint16_t param_len = rpc_msg->param_len;
    
    uint8_t *param = (uint8_t *)rpc_msg->param_buf;
    TRACE(0, "%s: %s", __func__, sens_ai_evt_to_str(evt));

    app_sensor_hub_mcu_ai_reply_msg(0, evt);

    switch (evt) {
        case SENSOR_HUB_AI_MSG_TYPE_KWS:
            sensor_hub_ai_mcu_kws_infor_process_handler(param,param_len);
            break;
        default:
            break;
    }

}

static void app_sensor_hub_mcu_ai_cmd_received_handler(uint8_t* ptr, uint16_t len)
{
    struct AI_RPC_REQ_MSG_T *req_msg = &g_received_req_msg;

    memcpy((uint8_t*)req_msg,ptr,len);

    TRACE(0, "%s: len=%d", __func__, len);
    DUMP8("%02x ",(uint8_t*)req_msg,MIN(len,32));

    if (req_msg->hdr.reply) {
        ASSERT(false, "%s: Bad reply msg: id=%d", __func__, req_msg->hdr.id);
    } else {
        switch(req_msg->hdr.id)
        {
            case SENS_MSG_ID_AI_MSG:
                app_sensor_hub_mcu_ai_msg_handler(ptr,len);
                break;
            default:
                ASSERT(false, "%s: Bad msg: id=%d", __func__, req_msg->hdr.id);
                break;
        }
    }
}

static int app_sensor_hub_mcu_send_msg(uint16_t hdr_id,uint16_t rpc_id,void * buf, uint16_t len)
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

int app_sensor_hub_mcu_ai_reply_msg(uint32_t status,uint32_t rpc_id)
{
    app_sensor_hub_core_ai_send_rsp_msg(SENS_MSG_ID_AI_MSG,rpc_id,(void*)&status,sizeof(uint32_t));

    return 0;
}


int app_sensor_hub_mcu_ai_send_msg(uint16_t rpc_id,void * buf, uint16_t len)
{
    app_sensor_hub_mcu_send_msg(SENS_MSG_ID_AI_MSG,rpc_id,buf,len);
    return 0;
}

static void app_sensor_hub_mcu_ai_cmd_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_core_bridge_send_data_with_waiting_rsp(MCU_SENSOR_HUB_TASK_CMD_AI, ptr, len);
}

static void app_sensor_hub_mcu_ai_cmd_wait_rsp_timeout(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "%s: len=%d", __func__, len);
}

static void app_sensor_hub_mcu_ai_cmd_rsp_received_handler(uint8_t* ptr, uint16_t len)
{
    struct AI_RPC_REQ_MSG_T *rsp_msg = &g_revceived_rsp_msg;

    memcpy((uint8_t*)rsp_msg,ptr,len);

    TRACE(0, "%s: len=%d", __func__, len);
    DUMP8("%02x ",(uint8_t*)rsp_msg,MIN(len,32));

    if (rsp_msg->hdr.reply == 0) {
        ASSERT(false, "%s: Bad reply msg: id=%d", __func__, rsp_msg->hdr.id);
    }else{
        if(SENS_MSG_ID_AI_MSG == rsp_msg->hdr.id){
            if(SENSOR_HUB_AI_DATA_UPDATE_OP == rsp_msg->rpc_id){
                uint16_t param_len = rsp_msg->param_len;
                uint8_t *param = (uint8_t *)rsp_msg->param_buf;
                app_sensor_hub_mcu_updating_section_data_process(param,param_len);

            }
        }
    }
}

static void app_sensor_hub_mcu_ai_cmd_tx_done_handler(uint16_t cmdCode, uint8_t* ptr, uint16_t len)
{
    TRACE(0, "%s: cmdCode 0x%x tx done", __func__, cmdCode);
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_AI,
                                "task cmd ai",
                                app_sensor_hub_mcu_ai_cmd_transmit_handler,
                                app_sensor_hub_mcu_ai_cmd_received_handler,
                                APP_CORE_BRIDGE_DEFAULT_WAIT_RSP_TIMEOUT_MS,
                                app_sensor_hub_mcu_ai_cmd_wait_rsp_timeout,
                                app_sensor_hub_mcu_ai_cmd_rsp_received_handler,
                                app_sensor_hub_mcu_ai_cmd_tx_done_handler);

void app_sensor_hub_ai_mcu_env_setup(SENSOR_HUB_AI_USER_E user,uint8_t * info,uint32_t len)
{
    static uint8_t env_info_buf[128];
    AI_ENV_SETUP_INFO_T *env_info_ptr = (AI_ENV_SETUP_INFO_T *)env_info_buf;
    env_info_ptr->user = user;
    len = MIN(sizeof(env_info_buf),len);
    memcpy((uint8_t*)&(env_info_ptr->info),info,len);
    len += OFFSETOF(AI_ENV_SETUP_INFO_T,info);
    app_sensor_hub_mcu_ai_send_msg(SENSOR_HUB_AI_MSG_TYPE_ENV,(void*)(env_info_ptr),len);
}

void app_sensor_hub_ai_mcu_activate_ai_user(SENSOR_HUB_AI_USER_E user, bool op)
{
    AI_ACTIVATE_OP_T env_op;
    env_op.user = user = user;
    env_op.op = op;
    app_sensor_hub_mcu_ai_send_msg(SENSOR_HUB_AI_MSG_TYPE_ACTIVATE,(void*)(&env_op),sizeof(env_op));
}

void app_sensor_hub_ai_mcu_request_vad_data_start(void)
{
    if(!mcu_sensor_hub_inited){
        return ;
    }
    app_sysfreq_req(APP_SYSFREQ_USER_AI_VOICE, SENSOR_HUB_MCU_AI_REQUEST_FREQ);
    app_mcu_sensor_hub_start_vad(MCU_SENSOR_HUB_VAD_AI_VOICE_ADAPT_ID);
}

void app_sensor_hub_ai_mcu_request_vad_data_stop(void)
{
    if(!mcu_sensor_hub_inited){
        return ;
    }
    app_sysfreq_req(APP_SYSFREQ_USER_AI_VOICE, SENSOR_HUB_MCU_AI_RELEASE_FREQ);
    app_mcu_sensor_hub_request_vad_data(false,0);
}

void app_sensor_hub_ai_mcu_request_vad_data_close(void)
{
    if(!mcu_sensor_hub_inited){
        return ;
    }
    app_sysfreq_req(APP_SYSFREQ_USER_AI_VOICE, SENSOR_HUB_MCU_AI_RELEASE_FREQ);
    app_mcu_sensor_hub_stop_vad(MCU_SENSOR_HUB_VAD_AI_VOICE_ADAPT_ID);
}

void sensor_hub_ai_mcu_app_init(void)
{
    if(!mcu_sensor_hub_inited){
        sensor_hub_ai_mcu_operator_init();
        app_sensor_hub_mcu_data_update_init();
        app_mcu_sensor_hub_setup_vad_event_handler(app_sensor_hub_ai_mcu_vad_event_handler);
        app_mcu_sensor_hub_setup_vad_data_handler(app_sensor_hub_ai_mcu_vad_data_handler);
    }
#ifdef MCU_SENSOR_HUB_AUDIO_DUMP_EN
    audio_dump_init(APP_VAD_DATA_PKT_SIZE/2, 2, 1);
#endif

    mcu_sensor_hub_inited = true;
}


#endif
