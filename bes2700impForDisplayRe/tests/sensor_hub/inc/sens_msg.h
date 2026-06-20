/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef __SENS_MSG_H__
#define __SENS_MSG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define SENS_VAD_MSG_MAX_SIZE 32

/* The message ID for sensor-hub core or MCU core sending/receiving */
enum SENS_MSG_ID_T {
    SENS_MSG_ID_ANA_RPC,  //ANA rpc msg,   sensor-hub sends to MCU
    SENS_MSG_ID_AF_BUF,   //AF buf msg,    sensor-hub sends to MCU
    SENS_MSG_ID_VAD_CMD,  //VAD cmd msg,   MCU sends to sensor-hub
    SENS_MSG_ID_VAD_EVT,  //VAD event msg, sensor-hub sends to MCU
    SENS_MSG_ID_VAD_DATA, //VAD data msg,  sensor-hub sends to MCU
    SENS_MSG_ID_AI_MSG,   //AI signaling msg, MCU sends to sensor-hub
    SENS_MSG_ID_TEST_MSG,
};

/* The event ID for sensor-hub core sends to MCU core */
enum SENS_VAD_EVT_ID_T {
    SENS_EVT_ID_VAD_IRQ_TRIG,   //vad FIND irq is triggerred
    SENS_EVT_ID_VAD_VOICE_CMD,  //key word is recognized
    SENS_EVT_ID_VAD_CMD_DONE,   //vad command execution done
};

/* The command ID for MCU core sends to sensor-hub core */
enum SENS_VAD_CMD_ID_T {
    SENS_CMD_ID_VAD_START,
    SENS_CMD_ID_VAD_STOP,
    SENS_CMD_ID_VAD_TEST,
    SENS_CMD_ID_VAD_REQ_DATA,
    SENS_CMD_ID_VAD_SEEK_DATA,
    SENS_CMD_ID_VAD_SET_ACTIVE,
};

/* The VAD test sub-command ID for MCU core sends to sensor-hub core */
enum SENS_VAD_TEST_SCMD_ID_T {
    SENS_VAD_TEST_SCMD_NULL,
    SENS_VAD_TEST_SCMD_MEM_TEST, //memory rw test
    SENS_VAD_TEST_SCMD_CPU_TEST, //cpu test
    SENS_VAD_TEST_SCMD_KWS_TEST, //kws lib test
    SENS_VAD_TEST_SCMD_MIC_DATA, //mic data test
};

struct SENS_MSG_HDR_T {
    uint16_t id     : 15;
    uint16_t reply  : 1;
};

struct ANA_RPC_REQ_MSG_T {
    struct SENS_MSG_HDR_T hdr;
    uint16_t rpc_id;
    uint32_t param[3];
};

struct ANA_RPC_REPLY_MSG_T {
    struct SENS_MSG_HDR_T hdr;
    uint16_t rpc_id;
    uint32_t ret;
};

struct AF_RPC_REQ_MSG_T {
    struct SENS_MSG_HDR_T hdr;
    uint16_t stream_id;
    void *buf;
    uint32_t len;
};

/* The VAD data requesting msg for sensor-hub core sends to MCU core */
struct VAD_RPC_REQ_DATA_MSG_T {
    struct SENS_MSG_HDR_T hdr;
    uint16_t data_idx;
    uint32_t data_addr;
    uint32_t data_size;
};

#ifdef __cplusplus
}
#endif

#endif

