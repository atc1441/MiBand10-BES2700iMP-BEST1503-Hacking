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
#include "mcu_sensor_hub_app_ai.h"
#include "sensor_hub_core_app_ai_ota.h"
#include "crc32_c.h"
#include "app_utils.h"
#include "mcu_sensor_hub_app_ai_ota.h"

#define CASE_ENUM(e) case e: return "["#e"]"

#define DATA_UPDATE_OP_HANDLER_PENDING_LIST_NUM  3

typedef struct{
    uint32_t magicCode;
    uint8_t *src;
    uint32_t size;
    sensorHubOtaOpCbType ota_cb;
}dataUpdateSectionOpHandlerInfo_T;

int app_sensor_hub_mcu_ai_send_msg(uint16_t rpc_id,void * buf, uint16_t len);
int app_sensor_hub_mcu_ai_reply_msg(uint32_t status,uint32_t rpc_id);

static sensorHubOtaOpCbType ota_cb;
static dataUpdateOpHandlerTypeStruct_T data_update_op_handler_instant;
static dataUpdateSectionStruct_T op_section_instant;
static dataUpdateSectionOperatorStruct_T data_update_secton_operator;
static uint8_t tx_msg_buf[MSG_REPLY_PER_SESSION_SIZE];
static uint8_t rx_msg_buf[MSG_REPLY_PER_SESSION_SIZE];
static uint8_t data_update_err_cnts = 0;

static dataUpdateSectionOpHandlerInfo_T pending_data_update_op_req[DATA_UPDATE_OP_HANDLER_PENDING_LIST_NUM];
static uint8_t pending_data_update_op_in_cursor = 0;
static uint8_t pending_data_update_op_out_cursor = 0;

static void app_print_pending_data_update_op_req(void)
{
    TRACE(0,"Pending data update op requests:");
    uint8_t index = pending_data_update_op_out_cursor;
    while (index != pending_data_update_op_in_cursor)
    {
        TRACE(4,"index %d magicCode %x src %p size %d", index,
            pending_data_update_op_req[index].magicCode,
            pending_data_update_op_req[index].src,
            pending_data_update_op_req[index].size);
        index++;
        if (DATA_UPDATE_OP_HANDLER_PENDING_LIST_NUM == index)
        {
            index = 0;
        }
    }
}

static void app_bt_push_pending_data_update_op(dataUpdateSectionOpHandlerInfo_T param)
{
    // go through the existing pending list to see if the remDev is already in
    uint8_t index = pending_data_update_op_out_cursor;
    while (index != pending_data_update_op_in_cursor)
    {
        if (param.magicCode == pending_data_update_op_req[index].magicCode)
        {
            pending_data_update_op_req[index].src = param.src;
            pending_data_update_op_req[index].size = param.size;
            pending_data_update_op_req[index].ota_cb = param.ota_cb;
            return;
        }
        index++;
        if (DATA_UPDATE_OP_HANDLER_PENDING_LIST_NUM == index)
        {
            index = 0;
        }
    }

    pending_data_update_op_req[pending_data_update_op_in_cursor].magicCode = param.magicCode;
    pending_data_update_op_req[pending_data_update_op_in_cursor].src = param.src;
    pending_data_update_op_req[pending_data_update_op_in_cursor].size = param.size;
    pending_data_update_op_req[pending_data_update_op_in_cursor].ota_cb = param.ota_cb;

    pending_data_update_op_in_cursor++;
    if (DATA_UPDATE_OP_HANDLER_PENDING_LIST_NUM == pending_data_update_op_in_cursor)
    {
        pending_data_update_op_in_cursor = 0;
    }
    app_print_pending_data_update_op_req();
}

dataUpdateSectionOpHandlerInfo_T* app_bt_pop_pending_data_update_op(void)
{
    if (pending_data_update_op_in_cursor == pending_data_update_op_out_cursor)
    {
        return NULL;
    }

    dataUpdateSectionOpHandlerInfo_T* ptReq = &pending_data_update_op_req[pending_data_update_op_out_cursor];
    pending_data_update_op_out_cursor++;
    if (DATA_UPDATE_OP_HANDLER_PENDING_LIST_NUM == pending_data_update_op_out_cursor)
    {
        pending_data_update_op_out_cursor = 0;
    }

    app_print_pending_data_update_op_req();
    return ptReq;
}

static inline void data_update_operator_err_reset(void)
{
    data_update_err_cnts = 0;
}

static inline void data_update_opeator_err_ticks(uint32_t status)
{
    if(status != SENSOR_HUB_DATA_OP_STATUS_OK){
        data_update_err_cnts++;
    }else{
        data_update_err_cnts = 0;
    }
}

static inline bool data_update_operator_check_and_reusme(void)
{
    if(data_update_err_cnts > DATA_UPDATE_OP_ERR_STATUS_ALLOW_MAXIAM_CNTS){
        data_update_err_cnts = 0;
        return true;
    }
    return false;
}

static inline void app_sensor_hub_data_update_section_reset(void)
{
    memset((uint8_t*)&op_section_instant,0,sizeof(dataUpdateSectionStruct_T));
}

static inline void app_sensor_hub_data_update_operator_reset(void)
{
    memset((uint8_t*)&data_update_secton_operator,0,sizeof(dataUpdateSectionOperatorStruct_T));
    memset(tx_msg_buf,0,MSG_REPLY_PER_SESSION_SIZE);
    data_update_secton_operator.section = &op_section_instant;
}

static void app_sensor_hub_data_update_op_handler_register(dataUpdateOpHandlerTypeStruct_T handler)
{
    data_update_op_handler_instant = handler;
}

static uint32_t _data_update_op_write_req(uint8_t* rxBuf,uint32_t rx_len,uint8_t *txBuf,uint32_t * tx_len)
{
    dataUpdateOpWriteOpReplyMsg_T * header = (dataUpdateOpWriteOpReplyMsg_T *)rxBuf;
    SENSOR_HUB_DATA_OP_STATUS_E status = SENSOR_HUB_DATA_OP_STATUS_OK;

    uint32_t dst_pos = header->start_pos;
    uint32_t dst_size = header->size;

    data_update_secton_operator.section->dataBufSection = (uint8_t*)dst_pos;
    data_update_secton_operator.section->dataBufSectionSize = dst_size;

    TRACE(3,"%s dst = %p dst_size = %d",__func__,data_update_secton_operator.section->dataBufSection,data_update_secton_operator.section->dataBufSectionSize);

    if(data_update_secton_operator.op.dataLenExpectToOp > dst_size){
        TRACE(2,"%s dst buf size not enough. stop %d ",__func__,data_update_secton_operator.op.dataLenExpectToOp);
        status = SENSOR_HUB_DATA_OP_STATUS_TRANSFER_OVERFLOW;
    }

    return status;
}

static inline uint32_t _data_update_op_read_req(uint8_t* rxBuf,uint32_t rx_len,uint8_t *txBuf,uint32_t * tx_len)
{
    dataUpdateOpReadOpReplyMsg_T * header = (dataUpdateOpReadOpReplyMsg_T *)rxBuf;

    uint32_t src_pos = header->start_pos;
    uint32_t src_size = header->size;

    data_update_secton_operator.section->dataBufSection = (uint8_t*)src_pos;
    data_update_secton_operator.section->dataBufSectionSize = src_size;

    TRACE(3,"%s dst = %p dst_size = %d",__func__,data_update_secton_operator.section->dataBufSection,data_update_secton_operator.section->dataBufSectionSize);

    return SENSOR_HUB_DATA_OP_STATUS_OK;
}

/*
 * return : the real return write success len
*/
static inline uint32_t _data_update_op_write(uint8_t* buf,uint32_t len)
{
    uint8_t *dst = data_update_secton_operator.section->dataBufSection;
    uint32_t alreadyOp = data_update_secton_operator.op.dataLenAlreadyOp;
    uint32_t expectOp = data_update_secton_operator.op.dataLenExpectToOp;
    uint32_t crc = data_update_secton_operator.op.dataCrcVerify;

    TRACE(6,"%s len %d expect %d already %d dst %p buf %p crc %d",__func__,len,
        expectOp,alreadyOp,dst,buf,crc);
    len = MIN(len,expectOp - alreadyOp);

    memcpy(dst,buf,len);

    crc = crc32_c(crc, buf, len);

    TRACE(1,"crc = %x",crc);

    data_update_secton_operator.op.dataCrcVerify = crc;
    data_update_secton_operator.op.dataLenAlreadyOp += len;

    return len;
}

static inline uint32_t _data_update_op_verify(uint8_t* buf,uint32_t len)
{
    uint32_t verify_len,alreadyVerify,crc_val;
    uint8_t * verifyPosPtr;

    SENSOR_HUB_DATA_OP_STATUS_E status = SENSOR_HUB_DATA_OP_STATUS_OK;

    verify_len = data_update_secton_operator.op.dataLenExpectToOp;
    verifyPosPtr = data_update_secton_operator.section->dataBufSection;
    alreadyVerify = 0;
    crc_val = 0;

    while(verify_len - alreadyVerify> SENSOR_HUB_DATA_UPDATE_OP_MAXIAM_SIZE_PER_HANDLE){
        crc_val = crc32_c(crc_val,verifyPosPtr,SENSOR_HUB_DATA_UPDATE_OP_MAXIAM_SIZE_PER_HANDLE);

        alreadyVerify += SENSOR_HUB_DATA_UPDATE_OP_MAXIAM_SIZE_PER_HANDLE;
        verifyPosPtr += SENSOR_HUB_DATA_UPDATE_OP_MAXIAM_SIZE_PER_HANDLE;
    }

    if(verify_len - alreadyVerify){
        crc_val = crc32_c(crc_val,verifyPosPtr,verify_len - alreadyVerify);
    }

    TRACE(2,"%s computed crc = %x",__func__,crc_val);

    if(crc_val != data_update_secton_operator.op.dataCrcVerify){
        TRACE(1,"expect = %x",data_update_secton_operator.op.dataCrcVerify);
        status = SENSOR_HUB_DATA_OP_STATUS_VERIFY_FAIL;
    }

    return status;
}

static inline uint32_t _data_update_op_finish_handler(uint8_t* buf,uint32_t len)
{
    data_update_secton_operator.op.dataUpdateOpCode = SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_NONE;
    if(ota_cb.finish_cb){
        ota_cb.finish_cb(data_update_secton_operator.section->magicCode);
    }
    memset((uint8_t*)(&ota_cb),0,sizeof(sensorHubOtaOpCbType));
    app_sensor_hub_data_update_section_reset();
    app_sensor_hub_data_update_operator_reset();
    TRACE(1,"%s finished!",__func__);
    dataUpdateSectionOpHandlerInfo_T *info = app_bt_pop_pending_data_update_op();
    if(info){
        app_sensor_hub_mcu_data_update_write_start(info->magicCode,info->src,info->size,info->ota_cb);    
    }
    return SENSOR_HUB_DATA_OP_STATUS_OK;    
}

static inline void _data_update_op_transmitter(uint8_t*buf,uint32_t len)
{
    app_sensor_hub_mcu_ai_send_msg(SENSOR_HUB_AI_DATA_UPDATE_OP,buf,len);

}

static inline void _data_update_op_handler_init(void)
{
    dataUpdateOpHandlerTypeStruct_T hander = 
    {
        .writeReq   = _data_update_op_write_req,
        .readReq    = _data_update_op_read_req,
        .writeOp    = _data_update_op_write,
        .readOp     = NULL,
        .verifyOp   = _data_update_op_verify,
        .finishOp   = _data_update_op_finish_handler,
        .transmitter = _data_update_op_transmitter,
    };
    app_sensor_hub_data_update_op_handler_register(hander);
}


uint32_t app_sensor_hub_mcu_updating_section_data_process(uint8_t * buf, uint32_t len)
{
    memcpy(rx_msg_buf,buf,len);
    dataUpdateFramePayloadStruct_T * payloadPtr =(dataUpdateFramePayloadStruct_T*)rx_msg_buf;
    uint32_t cmd = payloadPtr->cmd;
    uint32_t status = payloadPtr->status;
    dataUpdateFramePayloadStruct_T *payload = (dataUpdateFramePayloadStruct_T *)tx_msg_buf;

    uint16_t next_cmd = SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_NONE;
    uint32_t send_len = 0;
    uint8_t * send_ptr = (uint8_t*)&(payload->data);
    bool need_update = false;

    TRACE(3,"%s cmd = %x cmdLen = %d %d ",__func__,cmd,payloadPtr->dataLen,status);


    switch(cmd)
    {
        case SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_NONE:
            {
                app_sensor_hub_data_update_section_reset();
                app_sensor_hub_data_update_operator_reset();
            }
            break;
        case SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_REQ_WRITE:
        {
            if(SENSOR_HUB_DATA_OP_STATUS_OK == status){
                if(data_update_op_handler_instant.writeReq){
                    status = data_update_op_handler_instant.writeReq((uint8_t*)&(payloadPtr->data),(payloadPtr->dataLen),send_ptr,&send_len);
                    if(SENSOR_HUB_DATA_OP_STATUS_OK == status){
                        next_cmd = SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_DATA_TRANSFERING;
                    }
                }
            }            
            need_update = true;
        }
        break;
        case SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_DATA_TRANSFERING:
        {
            if(SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_REQ_WRITE == data_update_secton_operator.op.dataUpdateOpCode){
                if(data_update_op_handler_instant.writeOp){
                    data_update_op_handler_instant.writeOp((uint8_t*)data_update_secton_operator.op.dataStartOpPos,data_update_secton_operator.op.dataLenExpectToOp);
                }
                next_cmd = SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_DATA_TRANSFERING;
                TRACE(2,"expect = %d already = %d",data_update_secton_operator.op.dataLenExpectToOp,data_update_secton_operator.op.dataLenAlreadyOp);
                if(data_update_secton_operator.op.dataLenAlreadyOp == data_update_secton_operator.op.dataLenExpectToOp){
                    //next state : verify
                    next_cmd = SENSOR_HUB_DATA_UPDATE_SIGNALING_CMD_VERIFY;
                }

            }
            need_update = true;

        }
        break;
        case SENSOR_HUB_DATA_UPDATE_SIGNALING_CMD_VERIFY:
        {
            //next stage : finish            
            if(data_update_op_handler_instant.verifyOp){
                status = data_update_op_handler_instant.verifyOp(NULL,0);
            }
            if(SENSOR_HUB_DATA_OP_STATUS_OK == status){
                next_cmd = SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_DATA_FINISH;
            }
            need_update = true;
        }
        break;
        case SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_DATA_FINISH:
        {
            //stage end.
            if(data_update_op_handler_instant.finishOp){
                data_update_op_handler_instant.finishOp(NULL,0);
            }
            need_update = false;
        }
        break;
        default:
            break;
    }

    data_update_opeator_err_ticks(status);

    if(status != SENSOR_HUB_DATA_OP_STATUS_OK){
        TRACE(2,"status = %d cmd = %x",status,cmd);
        bool reset = data_update_operator_check_and_reusme();
        if(reset){
            need_update = true;
            next_cmd = SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_NONE;
            status = SENSOR_HUB_DATA_OP_STATUS_OK;
            send_len = 0;
        }
    }
    if(need_update){
        if(data_update_op_handler_instant.transmitter){
            payload->cmd = next_cmd;
            payload->status = status;
            payload->dataLen = send_len;
            data_update_op_handler_instant.transmitter((uint8_t*)payload,OFFSETOF(dataUpdateFramePayloadStruct_T, data)+send_len);
        }
    }

    return need_update;
}

void app_sensor_hub_mcu_data_update_init(void)
{
    app_sensor_hub_data_update_section_reset();
    app_sensor_hub_data_update_operator_reset();
    _data_update_op_handler_init();
    data_update_operator_err_reset();
}

bool app_sensor_hub_mcu_data_update_write_allow(void)
{
    if(SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_NONE == data_update_secton_operator.op.dataUpdateOpCode){
        return true;
    }
    return false;    
}

uint32_t app_sensor_hub_mcu_data_update_write_start(uint32_t magicCode,uint8_t* src,uint32_t size,sensorHubOtaOpCbType cb)
{
    if(SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_NONE == data_update_secton_operator.op.dataUpdateOpCode){
        data_update_secton_operator.op.dataUpdateOpCode = SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_REQ_WRITE;
        data_update_secton_operator.op.dataLenExpectToOp = size;
        data_update_secton_operator.op.dataStartOpPos = (uint32_t)src;
        data_update_secton_operator.section->magicCode = magicCode;
        ota_cb = cb;

        uint16_t len = 0;
        dataUpdateFramePayloadStruct_T *payload = (dataUpdateFramePayloadStruct_T *)tx_msg_buf;
        dataUpdateOpWriteReqHeader_T *writeReq = (dataUpdateOpWriteReqHeader_T *)&(payload->data);
        payload->cmd = SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_REQ_WRITE;
        payload->serviceId = 0xff;
        payload->status = 0;
        payload->dataLen = sizeof(dataUpdateOpWriteReqHeader_T);

        writeReq->magicCode = magicCode;
        writeReq->dataLenExpectToWrite = size;
        writeReq->dataCrcVerify = 0;
        len = OFFSETOF(dataUpdateFramePayloadStruct_T, data);
        len += sizeof(dataUpdateOpWriteReqHeader_T);
        TRACE(1,"%s",__func__);
        TRACE(5,"len = %d magicCode = %x src = %x expect = %d crc = %d",len,writeReq->magicCode,data_update_secton_operator.op.dataStartOpPos,writeReq->dataLenExpectToWrite,writeReq->dataCrcVerify);
        app_sensor_hub_mcu_ai_send_msg(SENSOR_HUB_AI_DATA_UPDATE_OP,(uint8_t*)payload,len);
        return 0;
    }else{
        dataUpdateSectionOpHandlerInfo_T op;
        op.magicCode = magicCode;
        op.src = src;
        op.size = size;
        op.ota_cb = cb;
        app_bt_push_pending_data_update_op(op);
    }

    return 1;
}

#endif
