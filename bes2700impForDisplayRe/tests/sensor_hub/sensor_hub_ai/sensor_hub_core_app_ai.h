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
#ifndef __SENSOR_HUB_CORE_APP_AI_H__
#define __SENSOR_HUB_CORE_APP_AI_H__
#include "sens_msg.h"

#define MODEL_FILE_BUF_SIZE (120*1024)      //consider the most difficult gsound lib model file

#define SENSOR_HUB_MAXIAM_AI_USER    5

typedef enum{
    SENSOR_HUB_AI_USER_NONE = 0,
    SENSOR_HUB_AI_USER_BES = 1,
    SENSOR_HUB_AI_USER_GG,
    SENSOR_HUB_AI_USER_SS,
    SENSOR_HUB_AI_USER_ALEXA,
    SENSOR_HUB_MAXIAM_AI_CNTS = SENSOR_HUB_MAXIAM_AI_USER,
}SENSOR_HUB_AI_USER_E;

typedef unsigned short sensor_hub_ai_user_type;

typedef enum{
    SENSOR_HUB_AI_MSG_NONE = 0,     //none
    SENSOR_HUB_AI_MSG_TYPE_KWS,
    SENSOR_HUB_AI_MSG_TYPE_ENV,
    SENSOR_HUB_AI_MSG_TYPE_ACTIVATE,
    SENSOR_HUB_AI_DATA_UPDATE_OP,
}SENSOR_HUB_AI_MSG_TYPE_E;

/* ai layer msg struction defination*/
struct AI_RPC_REQ_MSG_T{
    struct SENS_MSG_HDR_T hdr;
    uint16_t rpc_id;
    uint16_t param_len;
    uint8_t param_buf[100];
};

/*ai kws info struction defination*/
typedef struct GG_KWS_Result{
    int preambleMs;
}GG_KWS_Result_T;

#if 0
typedef struct PryonLiteMetadataBlob
{
    long int blobSize; // in bytes
    const char *blob;
} PryonLiteMetadataBlob;

typedef struct PryonLiteWakewordResult
{
    long long beginSampleIndex;      ///< Identifies the sample index in the client's source of audio at which the
                                    /// speech for the enumResult begins.

    long long endSampleIndex;        ///< Identifies the sample index in the client's source of audio at which the
                                    /// wakeword ends.  The number of samples in the audio for the
                                    /// wakeword is given by endSampleIndex - beginSampleIndex.

    const char* keyword;            ///< The keyword that was detected
    long int confidence;              ///< The confidence of the detection, from 0 (lowest) to 1000 (highest)

    PryonLiteMetadataBlob metadataBlob;  ///< Auxiliary information


    void* reserved;             ///< reserved for future use
    void* userData;             ///< userData passed in via PryonLiteWakewordConfig during wakeword initialization

} PryonLiteWakewordResult;
#else
typedef struct {
    int      score;
    uint32_t start_index;
    uint32_t end_index;
}ALEXA_KWS_Result_T;
#endif

typedef struct{
    int word;
    int score;
}SS_KWS_Result_T;

typedef struct{
    sensor_hub_ai_user_type ai_user;
    union{
        GG_KWS_Result_T gg_kws_info;
        ALEXA_KWS_Result_T alexa_kws_info;
        SS_KWS_Result_T ss_kws_info;
    }p;
}AI_KWS_INFOR_PAYLOAD_T;

typedef struct{
    uint32_t history_pcm_data_len;
}AI_KWS_INFOR_ENV_INFOR_T;

typedef struct{
    AI_KWS_INFOR_ENV_INFOR_T env_info;
    AI_KWS_INFOR_PAYLOAD_T kws_infor;
}AI_KWS_INFOR_T;

/*ai activate struction defination*/
typedef struct{
    SENSOR_HUB_AI_USER_E user;
    uint32_t op;
}AI_ACTIVATE_OP_T;

/*ai env setup struction defination*/
typedef struct{
    SENSOR_HUB_AI_USER_E user;
    void * info;
}AI_ENV_SETUP_INFO_T;

/* function interface declaration  */

int app_sensor_hub_core_ai_send_msg(uint16_t rpc_id,void * buf, uint16_t len);

#endif

