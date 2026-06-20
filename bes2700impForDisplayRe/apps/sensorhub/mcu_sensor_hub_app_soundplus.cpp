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
#include "mcu_sensor_hub_app_soundplus.h"
#include "crc32_c.h"
#include "app_utils.h"
#include "app_key.h"
#include "bluetooth_bt_api.h"
#include "btapp.h"
#include "app_bt_func.h"
#include "app_hfp.h"

#define CASE_ENUM(e) case e: return "["#e"]"

//#define MCU_SENSOR_HUB_AUDIO_DUMP_EN

#ifdef MCU_SENSOR_HUB_AUDIO_DUMP_EN
#include "audio_dump.h"
#endif

int app_sensor_hub_mcu_sndp_send_msg(uint16_t rpc_id,void * buf, uint16_t len);
int app_sensor_hub_mcu_sndp_reply_msg(uint32_t status,uint32_t rpc_id);

struct SNDP_RPC_REQ_MSG_T{
    struct SENS_MSG_HDR_T hdr;
    uint16_t rpc_id;
    uint16_t param_len;
    uint8_t param_buf[100];
};

typedef enum{
    SENSOR_HUB_SNDP_MSG_NONE = 0,     //none
}SENSOR_HUB_SNDP_MSG_TYPE_E;


static struct SNDP_RPC_REQ_MSG_T g_req_msg;
static struct SNDP_RPC_REQ_MSG_T g_received_req_msg;
static struct SNDP_RPC_REQ_MSG_T g_revceived_rsp_msg;
POSSIBLY_UNUSED static struct SNDP_RPC_REQ_MSG_T g_send_rsp_msg;

static bool mcu_sensor_hub_inited = false;
static const char *sens_ai_evt_to_str(SENSOR_HUB_SNDP_MSG_TYPE_E id)
{
    switch(id) {
        CASE_ENUM(SENSOR_HUB_SNDP_MSG_NONE);
        //CASE_ENUM(SENSOR_HUB_AI_MSG_TYPE_KWS);
        default:
            break;
    }
    return "";
}

void bt_key_handle_up_key(enum APP_KEY_EVENT_T event);
void bt_key_handle_down_key(enum APP_KEY_EVENT_T event);
extern void bt_key_handle_func_key(enum APP_KEY_EVENT_T event);
extern void app_ibrt_if_enable_hfp_voice_assistant(bool isEnable);

int virtual_key_handle(uint32_t id)
{
    TRACE(2,"%s, id=%d", __func__, id);
    switch (id)
    {
#ifdef BT_HFP_SUPPORT
        case 0:
            app_ibrt_if_enable_hfp_voice_assistant(TRUE);
            break;
#endif
        case 1:
            bes_bt_a2dp_key_handler(AVRCP_KEY_BACKWARD);
            break;
        case 2:
            bes_bt_a2dp_key_handler(AVRCP_KEY_FORWARD);
            break;
        case 3:
            bes_bt_a2dp_key_handler(AVRCP_KEY_VOLUME_DOWN);
            break;
        case 4:
            bes_bt_a2dp_key_handler(AVRCP_KEY_VOLUME_UP);
            break;
        case 5:
            bes_bt_a2dp_key_handler(AVRCP_KEY_PLAY);
            break;
        case 6:
            bes_bt_a2dp_key_handler(AVRCP_KEY_PAUSE);
            break;
        case 7:
        case 8:
            bt_key_handle_func_key(APP_KEY_EVENT_CLICK);
            break;
        default:
            break;
    }
    return 0;
}

static void app_sensor_hub_sndp_mcu_vad_event_handler(enum APP_VAD_EVENT_T event, uint8_t *param)
{
    uint32_t *param_s =  (uint32_t *)param;
    TRACE(0, "%s: event=%d,param:0x%x,0x%x,0x%x", __func__, event,param_s[0],param_s[1],param_s[2]);
    if (APP_VAD_EVT_VOICE_CMD == event) {
//        app_mcu_sensor_hub_request_vad_data(true,0);
        app_bt_start_custom_function_in_bt_thread(param_s[0]-1,0,(uint32_t)(uintptr_t)virtual_key_handle);
    }
}

static void app_sensor_hub_sndp_mcu_vad_data_handler(uint32_t pkt_id, uint8_t *payload, uint32_t bytes)
{
    TRACE(4, "%s: pkt_id=%d, payload = %p bytes=%d", __func__, pkt_id, payload,bytes);

#ifdef MCU_SENSOR_HUB_AUDIO_DUMP_EN
    audio_dump_add_channel_data(0, (void *)payload, bytes/2);
    audio_dump_run();
#endif

}

static void app_sensor_hub_mcu_sndp_msg_handler(uint8_t* buf, uint16_t len)
{
    struct SNDP_RPC_REQ_MSG_T *rpc_msg = (struct SNDP_RPC_REQ_MSG_T *)buf;
    SENSOR_HUB_SNDP_MSG_TYPE_E evt = (SENSOR_HUB_SNDP_MSG_TYPE_E)rpc_msg->rpc_id;

    //uint16_t param_len = rpc_msg->param_len;

    //uint8_t *param = (uint8_t *)rpc_msg->param_buf;
    TRACE(0, "%s: %s", __func__, sens_ai_evt_to_str(evt));

    app_sensor_hub_mcu_sndp_reply_msg(0, evt);

    switch (evt) {
        #if 0
        case SENSOR_HUB_AI_MSG_TYPE_KWS:
            sensor_hub_sndp_mcu_kws_infor_process_handler(param,param_len);
            break;
        #endif
        default:
            break;
    }

}

static void app_sensor_hub_mcu_sndp_cmd_received_handler(uint8_t* ptr, uint16_t len)
{
    struct SNDP_RPC_REQ_MSG_T *req_msg = &g_received_req_msg;

    memcpy((uint8_t*)req_msg,ptr,len);

    TRACE(0, "%s: len=%d", __func__, len);
    DUMP8("%02x ",(uint8_t*)req_msg,MIN(len,32));

    if (req_msg->hdr.reply) {
        ASSERT(false, "%s: Bad reply msg: id=%d", __func__, req_msg->hdr.id);
    } else {
        switch(req_msg->hdr.id)
        {
            case SENS_MSG_ID_TEST_MSG:
                app_sensor_hub_mcu_sndp_msg_handler(ptr,len);
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
    struct SNDP_RPC_REQ_MSG_T *req = &g_req_msg;

    memset(req, 0, sizeof(*req));
    req->hdr.id = hdr_id;
    req->hdr.reply = 0;
    req->rpc_id = rpc_id;

    req->param_len = len;
    if(len){
        memcpy(req->param_buf,buf,len);
    }
    send_len = OFFSETOF(struct SNDP_RPC_REQ_MSG_T,param_buf);
    send_len += len;
    ret = app_core_bridge_send_cmd(MCU_SENSOR_HUB_TASK_CMD_SNDP, (uint8_t *)req, send_len);
    ASSERT(ret == 0, "%s: sensor_hub_send failed: %d", __func__, ret);

    return ret;

}

static int app_sensor_hub_core_ai_send_rsp_msg(enum SENS_MSG_ID_T msg_id, uint16_t rpc_id, void * buf, uint16_t len)
{
    int ret;
    uint16_t send_len = 0;
    struct SNDP_RPC_REQ_MSG_T *rsp = &g_send_rsp_msg;

    rsp->hdr.id    = msg_id;
    rsp->hdr.reply = 1;
    rsp->rpc_id    = rpc_id;
    rsp->param_len = len;
    if(len){
        memcpy(rsp->param_buf,buf,len);
    }
    send_len = OFFSETOF(struct SNDP_RPC_REQ_MSG_T,param_buf);
    send_len += len;
    ret = app_core_bridge_send_rsp(MCU_SENSOR_HUB_TASK_CMD_SNDP, (uint8_t *)rsp, send_len);
    ASSERT(ret == 0, "%s: sensor_hub_send failed: %d", __func__, ret);

    return ret;
}

int app_sensor_hub_mcu_sndp_reply_msg(uint32_t status,uint32_t rpc_id)
{
    app_sensor_hub_core_ai_send_rsp_msg(SENS_MSG_ID_TEST_MSG,rpc_id,(void*)&status,sizeof(uint32_t));

    return 0;
}


int app_sensor_hub_mcu_sndp_send_msg(uint16_t rpc_id,void * buf, uint16_t len)
{
    app_sensor_hub_mcu_send_msg(SENS_MSG_ID_TEST_MSG,rpc_id,buf,len);
    return 0;
}

static void app_sensor_hub_mcu_sndp_cmd_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_core_bridge_send_data_with_waiting_rsp(MCU_SENSOR_HUB_TASK_CMD_SNDP, ptr, len);
}

static void app_sensor_hub_mcu_sndp_cmd_wait_rsp_timeout(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "%s: len=%d", __func__, len);
}

static void app_sensor_hub_mcu_sndp_cmd_rsp_received_handler(uint8_t* ptr, uint16_t len)
{
    struct SNDP_RPC_REQ_MSG_T *rsp_msg = &g_revceived_rsp_msg;

    memcpy((uint8_t*)rsp_msg,ptr,len);

    TRACE(0, "%s: len=%d", __func__, len);
    DUMP8("%02x ",(uint8_t*)rsp_msg,MIN(len,32));

    if (rsp_msg->hdr.reply == 0) {
        ASSERT(false, "%s: Bad reply msg: id=%d", __func__, rsp_msg->hdr.id);
    }else{
        if(SENS_MSG_ID_TEST_MSG == rsp_msg->hdr.id){
            #if 0
            if(SENSOR_HUB_AI_DATA_UPDATE_OP == rsp_msg->rpc_id){
                uint16_t param_len = rsp_msg->param_len;
                uint8_t *param = (uint8_t *)rsp_msg->param_buf;

            }
            #endif
        }
    }
}

static void app_sensor_hub_mcu_sndp_cmd_tx_done_handler(uint16_t cmdCode, uint8_t* ptr, uint16_t len)
{
    TRACE(0, "%s: cmdCode 0x%x tx done", __func__, cmdCode);
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_SNDP,
                                "task cmd ai",
                                app_sensor_hub_mcu_sndp_cmd_transmit_handler,
                                app_sensor_hub_mcu_sndp_cmd_received_handler,
                                APP_CORE_BRIDGE_DEFAULT_WAIT_RSP_TIMEOUT_MS,
                                app_sensor_hub_mcu_sndp_cmd_wait_rsp_timeout,
                                app_sensor_hub_mcu_sndp_cmd_rsp_received_handler,
                                app_sensor_hub_mcu_sndp_cmd_tx_done_handler);

void app_sensor_hub_sndp_mcu_request_vad_start(void)
{
    if(!mcu_sensor_hub_inited){
        return ;
    }
    //app_sysfreq_req(APP_SYSFREQ_USER_AI_VOICE, SENSOR_HUB_MCU_SNDP_REQUEST_FREQ);
    app_mcu_sensor_hub_start_vad(APP_VAD_ADPT_ID_SNDP);
}

void app_sensor_hub_sndp_mcu_request_vad_data(bool onoff)
{
    if(!mcu_sensor_hub_inited){
        return ;
    }
    app_sysfreq_req(APP_SYSFREQ_USER_AI_VOICE, SENSOR_HUB_MCU_SNDP_RELEASE_FREQ);
    app_mcu_sensor_hub_request_vad_data(onoff,0);
}

void app_sensor_hub_sndp_mcu_request_vad_stop(void)
{
    if(!mcu_sensor_hub_inited){
        return ;
    }
    app_sysfreq_req(APP_SYSFREQ_USER_AI_VOICE, SENSOR_HUB_MCU_SNDP_RELEASE_FREQ);
    app_mcu_sensor_hub_stop_vad(APP_VAD_ADPT_ID_SNDP);
}

void sensor_hub_soundplus_mcu_app_init(void)
{
    if(!mcu_sensor_hub_inited){
        app_mcu_sensor_hub_setup_vad_event_handler(app_sensor_hub_sndp_mcu_vad_event_handler);
        app_mcu_sensor_hub_setup_vad_data_handler(app_sensor_hub_sndp_mcu_vad_data_handler);
    }
#ifdef MCU_SENSOR_HUB_AUDIO_DUMP_EN
    audio_dump_init(APP_VAD_DATA_PKT_SIZE/2, 2, 1);
#endif
    mcu_sensor_hub_inited = true;
}


#endif
