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
#ifndef __SENSOR_HUB_CORE_APP_AI_OTA_H__
#define __SENSOR_HUB_CORE_APP_AI_OTA_H__
#include "sens_msg.h"

#define MODEL_FILE_BUF_SIZE (120*1024)      //consider the most difficult gsound lib model file
#define SENSOR_HUB_DATA_UPDATE_OP_MAXIAM_SIZE_PER_HANDLE    (4096)
#define UPDATE_SECTION_LIST_NUM (2)
#define MSG_REPLY_PER_SESSION_SIZE  (256)
#define DATA_UPDATE_OP_ERR_STATUS_ALLOW_MAXIAM_CNTS (3)

/*data update struction defination*/

typedef enum{
    SENSOR_HUB_DATA_UPDATE_SECTION_GG_MAGIC_CODE = 0x80,
    SENSOR_HUB_DATA_UPDATE_SECTION_GG_DATA_MAGIC_CODE = 0x81,
    SENSOR_HUB_DATA_UPDATE_SECTION_GG_TEXT_MAGIC_CODE = 0x82,
}SENSOR_HUB_DATA_UPDATE_SECTION_MAGIC_CODE_E;

typedef enum{
    SENSOR_HUB_UPDATE_SECTION_STATE_IDLE= 0,
    SENSOR_HUB_UPDATE_SECTION_STATE_WORK = 1,
    SENSOR_HUB_UPDATE_SECTION_STATE_WRITTING = 2,
    SENSOR_HUB_UPDATE_SECTION_STATE_READING = 3,
    SENSOR_HUB_UPDATE_SECTION_STATE_BUSY = 4,
}SENSOR_HUB_UPDATE_SECTION_STATE_E;

typedef enum{
    SENSOR_HUB_DATA_OP_STATUS_OK = 0,
    SENSOR_HUB_DATA_OP_STATUS_ERR = 1,
    SENSOR_HUB_DATA_OP_STATUS_SESSION_ERR = 2,
    SENSOR_HUB_DATA_OP_STATUS_OP_ERR    = 3,
    SENSOR_HUB_DATA_OP_STATUS_PARAM_ERR = 4,
    SENSOR_HUB_DATA_OP_STATUS_VERIFY_FAIL = 5,
    SENSOR_HUB_DATA_OP_STATUS_TRANSFER_OVERFLOW = 6,
}SENSOR_HUB_DATA_OP_STATUS_E;

typedef enum{
    SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_NONE       = 0,
    SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_REQ_WRITE  = 0x81,
    SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_REQ_READ  = 0x82,
    SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_DATA_TRANSFERING  = 0x84,
    SENSOR_HUB_DATA_UPDATE_SINGALING_CMD_DATA_FINISH  = 0x85,
    SENSOR_HUB_DATA_UPDATE_SIGNALING_CMD_VERIFY       = 0X86,
}SENSOR_HUB_DATA_UPDATE_SIGNALING_CMD_TYPE_E;

typedef struct{
    uint32_t dataUpdateOpCode;
    uint32_t dataLenExpectToOp;
    uint32_t dataStartOpPos;
    uint32_t dataLenAlreadyOp;
    uint32_t dataCrcVerify;
}dataUpdateSectionOpStruct_T;

typedef struct{
    uint32_t magicCode;
    uint32_t state;
    uint8_t * dataBufSection;
    uint32_t  dataBufSectionSize;
}dataUpdateSectionStruct_T;

typedef struct{
    uint32_t section_num ;
    dataUpdateSectionStruct_T section[UPDATE_SECTION_LIST_NUM];
}dataUpdateSectionListStruct_T;

typedef struct{
    dataUpdateSectionOpStruct_T op;
    dataUpdateSectionStruct_T *section;
//    uint32_t dataBufSection[MODEL_FILE_BUF_SIZE/sizeof(uint32_t)];
}dataUpdateSectionOperatorStruct_T;

typedef struct{
    uint32_t magicCode;
    uint32_t dataLenExpectToWrite;
    uint32_t dataCrcVerify;
}dataUpdateOpWriteReqHeader_T;

typedef struct{
    uint32_t magicCode;
    uint32_t dataLenExpectToRead;
    uint32_t dataStartToReadPos;
}dataUpdateOpReadReqHeader_T;

typedef struct{
    uint32_t dataLenNeedToVerify;
    uint32_t dataStartPosNeedToVerify;
    uint32_t dataCRC;
}dataUpdateOpVerifyHeader_T;

typedef struct{
    uint32_t serviceId;
    uint32_t cmd;
    uint32_t status;
    uint32_t dataLen;
    uint8_t * data; 
}dataUpdateFramePayloadStruct_T;

typedef uint32_t(*dataUpdateOpWriteReqType)(uint8_t* rxBuf,uint32_t rx_len,uint8_t *txBuf,uint32_t * tx_len);
typedef uint32_t(*dataUpdateOpReadReqType)(uint8_t* rxBuf,uint32_t rx_len,uint8_t *txBuf,uint32_t * tx_len);
typedef uint32_t(*dataUpdateOpWriteHandlerType)(uint8_t*buf,uint32_t len);
typedef uint32_t(*dataUpdateOpReadHandlerType)(uint8_t*buf,uint32_t len);
typedef uint32_t(*dataUpdateOpVerifyHandlerType)(uint8_t*buf,uint32_t len);
typedef uint32_t(*dataUpdateOpFinishHandlerType)(uint8_t*buf,uint32_t len);
typedef void(*dataUpdateOpTransmitterHandlerType)(uint8_t*buf,uint32_t len);
typedef struct{
    dataUpdateOpWriteReqType writeReq;
    dataUpdateOpReadReqType readReq;
    dataUpdateOpWriteHandlerType writeOp;
    dataUpdateOpReadHandlerType readOp;
    dataUpdateOpVerifyHandlerType verifyOp;
    dataUpdateOpFinishHandlerType finishOp;
    dataUpdateOpTransmitterHandlerType transmitter;
}dataUpdateOpHandlerTypeStruct_T;


typedef struct{
    uint32_t start_pos;
    uint32_t size;
}dataUpdateOpReadOpReplyMsg_T;

typedef struct{
    uint32_t start_pos;
    uint32_t size;
}dataUpdateOpWriteOpReplyMsg_T;


/* 
 *  the dynamic data section need to  be registered then can be updated
 *  magicCode to identify the registed data section
 *  start_addr : of dynamic data updated section
 *  size : of dynamic data updated section
*/
void app_sensor_hub_data_update_section_load_register(uint32_t magicCode,uint32_t start_addr,uint32_t size);

void app_sensor_hub_core_data_update_init(void);

/* 
 *  need to check whether the data section be in visit 
 *  true, can be visit
 *  false, under async state
*/
bool section_state_allow_op(void);

uint32_t app_sensor_hub_core_updating_section_data_process(uint8_t * buf, uint32_t len);

#endif

