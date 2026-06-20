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
#include "sensor_hub_core_app_ai_ota.h"

static osMutexId update_section_refer_mutex_id = NULL;
osMutexDef(update_section_refer_mutex);

static dataUpdateSectionOperatorStruct_T data_update_secton_operator;
static dataUpdateFramePayloadStruct_T data_update_frame_payload;
static dataUpdateOpHandlerTypeStruct_T data_update_op_handler_instant;
static uint8_t tx_buf[MSG_REPLY_PER_SESSION_SIZE]; 

int app_sensor_hub_core_ai_reply_msg(uint16_t rpc_id,void * buf, uint16_t len);


static dataUpdateSectionListStruct_T update_section_list = 
{
    .section_num = 0,
};

static inline void _update_section_infor_print(void)
{
    TRACE(2,"%s num = %d",__func__,update_section_list.section_num);
    uint32_t index ;
    for (index = 0; index < UPDATE_SECTION_LIST_NUM;index++){
        TRACE(4,"magicCode = %d state = %d section = %p size = %d",update_section_list.section[index].magicCode,
            update_section_list.section[index].state,update_section_list.section[index].dataBufSection,update_section_list.section[index].dataBufSectionSize);
    }
}

static inline uint32_t _update_section_list_search(uint32_t magicCode)
{
    uint32_t index ;
    for (index = 0; index < UPDATE_SECTION_LIST_NUM;index++){
        if(update_section_list.section[index].magicCode == magicCode){
            break;
        }
    }

    return index;
}

void app_sensor_hub_data_update_section_load_register(uint32_t magicCode,uint32_t start_addr,uint32_t size)
{
    uint32_t index;
    dataUpdateSectionStruct_T *section = NULL;

    index = _update_section_list_search(magicCode);
    if(UPDATE_SECTION_LIST_NUM == index){
        if(update_section_list.section_num < UPDATE_SECTION_LIST_NUM){
            section = &update_section_list.section[update_section_list.section_num];
            update_section_list.section_num++;
        }
    }else{
        section = &update_section_list.section[index];
    }
    TRACE(4,"%s index = %d section = %p num = %d ",__func__,
        index,section,update_section_list.section_num);
    if(section){
        section->dataBufSection = (uint8_t*)start_addr;
        section->dataBufSectionSize = size;
        section->state = SENSOR_HUB_UPDATE_SECTION_STATE_IDLE;
        section->magicCode = magicCode;
        TRACE(4,"section: %p size = %d state = %d code = %x",section->dataBufSection,section->dataBufSectionSize,
            section->state,section->magicCode);
    }
}

static void app_sensor_hub_data_update_op_handler_register(dataUpdateOpHandlerTypeStruct_T handler)
{
    data_update_op_handler_instant = handler;
}

void _update_section_state_update(uint32_t state)
{
    osMutexWait(update_section_refer_mutex_id, osWaitForever);
    data_update_secton_operator.section->state = state;
    osMutexRelease(update_section_refer_mutex_id);
}

uint32_t update_section_state_get(void)
{
    uint32_t section_state ; 
    osMutexWait(update_section_refer_mutex_id, osWaitForever);
    section_state = data_update_secton_operator.section->state;
    osMutexRelease(update_section_refer_mutex_id);

    return section_state;
}

bool section_state_allow_op(void)
{
    bool allow; 
    osMutexWait(update_section_refer_mutex_id, osWaitForever);
    allow = (SENSOR_HUB_UPDATE_SECTION_STATE_IDLE == data_update_secton_operator.section->state);
    osMutexRelease(update_section_refer_mutex_id);

    return allow;

}

static inline uint32_t _data_update_op_write_req(uint8_t* rxBuf,uint32_t rx_len,uint8_t *txBuf,uint32_t * tx_len)
{
    dataUpdateOpWriteReqHeader_T * header = (dataUpdateOpWriteReqHeader_T *)rxBuf;
    uint32_t status = SENSOR_HUB_DATA_OP_STATUS_ERR;
    uint32_t index = UPDATE_SECTION_LIST_NUM;

    memset((uint8_t*)&data_update_secton_operator,0,sizeof(dataUpdateSectionOperatorStruct_T));
    data_update_secton_operator.op.dataLenExpectToOp = header->dataLenExpectToWrite;

    index = _update_section_list_search(header->magicCode);
    TRACE(2,"%s index = %d",__func__,index);
    DUMP8("%x ",rxBuf,rx_len);
    if(index < UPDATE_SECTION_LIST_NUM){
        dataUpdateOpWriteOpReplyMsg_T *rely_msg = (dataUpdateOpWriteOpReplyMsg_T*)txBuf;
        osMutexWait(update_section_refer_mutex_id, osWaitForever);
        data_update_secton_operator.section = &update_section_list.section[index];

        if(SENSOR_HUB_UPDATE_SECTION_STATE_IDLE == data_update_secton_operator.section->state){
            data_update_secton_operator.section->state = SENSOR_HUB_UPDATE_SECTION_STATE_WRITTING;
            status = SENSOR_HUB_DATA_OP_STATUS_OK;
        }
        osMutexRelease(update_section_refer_mutex_id);

        rely_msg->start_pos = (uint32_t)data_update_secton_operator.section->dataBufSection;
        rely_msg->size = data_update_secton_operator.section->dataBufSectionSize;
        *tx_len = sizeof(dataUpdateOpWriteOpReplyMsg_T);

    }else{
        _update_section_infor_print();
    }

    return status;
}

static inline uint32_t _data_update_op_read_req(uint8_t* rxBuf,uint32_t rx_len,uint8_t *txBuf,uint32_t * tx_len)
{
    dataUpdateOpReadReqHeader_T * header = (dataUpdateOpReadReqHeader_T *)rxBuf;
    uint32_t status = SENSOR_HUB_DATA_OP_STATUS_ERR;
    uint32_t index = UPDATE_SECTION_LIST_NUM;

    memset((uint8_t*)&data_update_secton_operator,0,sizeof(dataUpdateSectionStruct_T));
    data_update_secton_operator.op.dataLenExpectToOp = header->dataLenExpectToRead;
    data_update_secton_operator.op.dataStartOpPos = header->dataStartToReadPos;

    index = _update_section_list_search(header->magicCode);

    TRACE(2,"%s index = %d",__func__,index);

    if(index < UPDATE_SECTION_LIST_NUM){
        dataUpdateOpReadOpReplyMsg_T *rely_msg = (dataUpdateOpReadOpReplyMsg_T*)txBuf;

        osMutexWait(update_section_refer_mutex_id, osWaitForever);
        data_update_secton_operator.section = &update_section_list.section[index];

        if(SENSOR_HUB_UPDATE_SECTION_STATE_IDLE == data_update_secton_operator.section->state){
            data_update_secton_operator.section->state = SENSOR_HUB_UPDATE_SECTION_STATE_READING;
            status = SENSOR_HUB_DATA_OP_STATUS_OK;
        }
        osMutexRelease(update_section_refer_mutex_id);

        rely_msg->start_pos = (uint32_t)data_update_secton_operator.section->dataBufSection;
        rely_msg->size = data_update_secton_operator.section->dataBufSectionSize;
        *tx_len = sizeof(dataUpdateOpReadOpReplyMsg_T);
    }else{
        _update_section_infor_print();
    }
    return status;
}

static inline uint32_t _data_update_op_write(uint8_t* buf,uint32_t len)
{
#if 0
    uint32_t alreadyOpLen = data_update_secton_operator.op.dataLenAlreadyOp;
    uint32_t expectOpLen  = data_update_secton_operator.op.dataLenExpectToOp;
    uint32_t bufferLimit = (uint32_t)data_update_secton_operator->section.dataBufSection + data_update_secton_operator->section.dataBufSectionSize;
    uint8_t * opPosPtr = (uint8_t*)((uint32_t)data_update_secton_operator->section.dataBufSection + alreadyOpLen);

    if((uint32_t)opPosPtr + len) > bufferLimit{
        TRACE(1,"%s overflow !!",__func__);
        return SENSOR_HUB_DATA_OP_STATUS_TRANSFER_OVERFLOW;
    }

    memcpy(opPosPtr,buf,len);

    return SENSOR_HUB_DATA_OP_STATUS_OK;
#else
    return SENSOR_HUB_DATA_OP_STATUS_OK;
#endif
}

static inline uint32_t _data_update_op_read(uint8_t* buf,uint32_t len)
{
#if 0
    uint32_t alreadyOpLen = data_update_secton_operator.op.dataLenAlreadyOp;
    uint32_t expectOpLen  = data_update_secton_operator.op.dataLenExpectToOp;
    uint32_t bufferLimit = (uint32_t)data_update_secton_operator->section.dataBufSection + data_update_secton_operator->section.dataBufSectionSize;
    uint8_t * opPosPtr = (uint8_t*)((uint32_t)data_update_secton_operator->section.dataBufSection + alreadyOpLen);

    len = MIN(expectOpLen - alreadyOpLen,len);

    memcpy(buf,opPosPtr,len);

    return len;
#else
    return SENSOR_HUB_DATA_OP_STATUS_OK;
#endif
}

static inline uint32_t _data_update_op_verify(uint8_t* buf,uint32_t len)
{
#if 0
    dataUpdateOpVerifyHeader_T * header = (dataUpdateOpVerifyHeader_T *)buf; 
    uint32_t src_pos,verify_len,alreadyVerify,crc_val;
    uint8_t * verifyPosPtr;
    uint32_t bufferLimit = (uint32_t)data_update_secton_operator->section.dataBufSection + data_update_secton_operator->section.dataBufSectionSize;

    SENSOR_HUB_DATA_OP_STATUS_E status;

    src_pos = header->dataStartPosNeedToVerify;
    verify_len = header->dataLenNeedToVerify;

    verifyPosPtr = (uint8_t*)((uint32_t)data_update_secton_operator->section.dataBufSection + src_pos);

    alreadyVerify = 0;
    crc_val = 0;

    if((verify_len > data_update_secton_operator->section.dataBufSectionSize) || 
        ((src_pos + verify_len ) > bufferLimit)){
        status = SENSOR_HUB_DATA_OP_STATUS_PARAM_ERR;
    }else{
        while(verify_len - alreadyVerify> SENSOR_HUB_DATA_UPDATE_OP_MAXIAM_SIZE_PER_HANDLE){
            crc_val = crc32_c(crc_val,verifyPosPtr,SENSOR_HUB_DATA_UPDATE_OP_MAXIAM_SIZE_PER_HANDLE);

            alreadyVerify += SENSOR_HUB_DATA_UPDATE_OP_MAXIAM_SIZE_PER_HANDLE;
            verifyPosPtr += SENSOR_HUB_DATA_UPDATE_OP_MAXIAM_SIZE_PER_HANDLE;
        }
        if(verify_len - alreadyVerify){
            crc_val = crc32_c(crc_val,verifyPosPtr,verify_len - alreadyVerify);
        }
        if(crc_val != data_update_secton_operator.op.dataCrcVerify){
            TRACE(2,"crc_val = %x expect = %x",crc_val,data_update_secton_operator.op.dataCrcVerify);
            status = SENSOR_HUB_DATA_OP_STATUS_VERIFY_FAIL;
        }
    }

    return status;
#else
    return SENSOR_HUB_DATA_OP_STATUS_OK;
#endif
}

static inline uint32_t _data_update_op_finish_handler(uint8_t* buf,uint32_t len)
{
    _update_section_state_update(SENSOR_HUB_UPDATE_SECTION_STATE_IDLE);

    TRACE(1,"%s finished",__func__);

    return SENSOR_HUB_DATA_OP_STATUS_OK;
}

static inline void _data_update_op_transmitter(uint8_t*buf,uint32_t len)
{
    app_sensor_hub_core_ai_reply_msg(SENSOR_HUB_AI_DATA_UPDATE_OP,(void*)buf,len);
}

static inline void _data_update_op_handler_init(void)
{
    dataUpdateOpHandlerTypeStruct_T hander = 
    {
        .writeReq   = _data_update_op_write_req,
        .readReq    = _data_update_op_read_req,
        .writeOp    = NULL,
        .readOp     = NULL,
        .verifyOp   = NULL,
        .finishOp   = _data_update_op_finish_handler,
        .transmitter = _data_update_op_transmitter,
    };
    app_sensor_hub_data_update_op_handler_register(hander);
}

static inline void _data_updata_frame_payload_reset(void)
{
    memset((uint8_t*)&data_update_frame_payload,0,sizeof(dataUpdateFramePayloadStruct_T));
    data_update_frame_payload.serviceId = 0xffffffff;
}

static inline void _data_update_instant_op_state_reset(void)
{
    memset((uint8_t*)&(data_update_secton_operator.op),0,sizeof(dataUpdateSectionOpStruct_T));
}

uint32_t app_sensor_hub_core_updating_section_data_process(uint8_t * buf, uint32_t len)
{
    dataUpdateFramePayloadStruct_T * payloadPtr =(dataUpdateFramePayloadStruct_T*)buf;
    dataUpdateFramePayloadStruct_T * replyMsg = (dataUpdateFramePayloadStruct_T *)tx_buf;
//    uint32_t service_id = payloadPtr->serviceId;
    uint32_t cmd = payloadPtr->cmd;
    uint32_t dataLen =0;
    SENSOR_HUB_DATA_OP_STATUS_E status = SENSOR_HUB_DATA_OP_STATUS_OK;

    memcpy((uint8_t*)replyMsg,(uint8_t*)payloadPtr,len);

    TRACE(2,"%s cmd = %x",__func__,cmd);
#if 0
    if(data_update_frame_payload.serviceId == 0xffffffff){
        _data_updata_frame_payload_reset();
        data_update_frame_payload.serviceId = service_id;
    }else if (data_update_frame_payload.serviceId != payloadPtr->serviceId){
        TRACE(0,"not one same session op");
        status = SENSOR_HUB_DATA_OP_STATUS_SESSION_ERR;
    }
#endif

    switch(cmd)
    {
        case SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_NONE:
            //do reset
            {
                _data_updata_frame_payload_reset();
                _data_update_instant_op_state_reset();
            }
            break;
        case SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_REQ_WRITE:
            {
                if(data_update_op_handler_instant.writeReq){
                    status = data_update_op_handler_instant.writeReq((uint8_t*)&(payloadPtr->data),payloadPtr->dataLen,
                        (uint8_t*)&(replyMsg->data),&dataLen);
                }
                data_update_secton_operator.op.dataUpdateOpCode = cmd;
            }
            break;
        case SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_REQ_READ:
            {
                if(data_update_op_handler_instant.readReq){
                    status = data_update_op_handler_instant.readReq((uint8_t*)&(payloadPtr->data),payloadPtr->dataLen,
                        (uint8_t*)&(replyMsg->data),&dataLen);
                }
                data_update_secton_operator.op.dataUpdateOpCode = cmd;
            }
            break;
        case SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_DATA_TRANSFERING:
            {
                if(SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_REQ_READ == data_update_secton_operator.op.dataUpdateOpCode){
                    if(data_update_op_handler_instant.readOp){
                        dataLen = data_update_op_handler_instant.readOp((uint8_t*)&(replyMsg->data),payloadPtr->dataLen);
                    }
                }else if(SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_REQ_WRITE == data_update_secton_operator.op.dataUpdateOpCode ){
                    if(data_update_op_handler_instant.writeOp){
                        status = data_update_op_handler_instant.writeOp((uint8_t*)&(payloadPtr->data),payloadPtr->dataLen);
                    }
                }else{
                    status = SENSOR_HUB_DATA_OP_STATUS_OP_ERR;
                }
            }
            break;
        case SENSOR_HUB_DATA_UPDATE_SIGNALING_CMD_VERIFY:
            {
                if(data_update_op_handler_instant.verifyOp){
                    status = data_update_op_handler_instant.verifyOp((uint8_t*)&(payloadPtr->data),payloadPtr->dataLen);
                }
            }
            break;
        case SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_DATA_FINISH:
            _data_updata_frame_payload_reset();
            _data_update_instant_op_state_reset();

            if(data_update_op_handler_instant.finishOp){
                status = data_update_op_handler_instant.finishOp((uint8_t*)&(payloadPtr->data),payloadPtr->dataLen);
            }
            
            TRACE(0,"finished");
            break;
        default:
            break;
    }

    if(status != SENSOR_HUB_DATA_OP_STATUS_OK){
        TRACE(2,"%s op err opcode = %d ",__func__,status);
    }

    replyMsg->dataLen = dataLen;
    replyMsg->status = status;
    if(data_update_op_handler_instant.transmitter){
        data_update_op_handler_instant.transmitter(tx_buf,replyMsg->dataLen + OFFSETOF(dataUpdateFramePayloadStruct_T, data));
    }

    return status;
}

void app_sensor_hub_core_data_update_init(void)
{
    if (update_section_refer_mutex_id == NULL) {
        update_section_refer_mutex_id = osMutexCreate(osMutex(update_section_refer_mutex));
    }

    _data_updata_frame_payload_reset();
    _data_update_instant_op_state_reset();

    _data_update_op_handler_init();
}

#endif
