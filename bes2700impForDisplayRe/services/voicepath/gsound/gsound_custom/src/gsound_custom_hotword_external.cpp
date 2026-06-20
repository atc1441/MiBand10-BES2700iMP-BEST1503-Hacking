/***************************************************************************
*
*Copyright 2015-2020 BES.
*All rights reserved. All unpublished rights reserved.
*
*No part of this work may be used or reproduced in any form or by any
*means, or stored in a database or retrieval system, without prior written
*permission of BES.
*
*Use of this work is governed by a license granted by BES.
*This work contains confidential and proprietary information of
*BES. which is protected by copyright, trade secret,
*trademark and other intellectual property rights.
*
****************************************************************************/

/*****************************header include********************************/
#include "stdio.h"
#include "hal_timer.h"
#include "gsound_dbg.h"
#include "gsound_custom_bt.h"
#include "gsound_custom_hotword_external.h"
#include "gsound_custom_audio.h"
#include "voice_sbc.h"
#include "hotword_dsp_multi_bank_api.h"
#include "nvrecord_gsound.h"
#include "audio_dump.h"
#include "app_ai_if_thirdparty.h"
#include "norflash_api.h"
#include "gsound_custom_hotword_common.h"

#ifdef VOICE_DETECTOR_EN
#include "app_voice_detector.h"
#endif
#ifdef VOICE_DETECTOR_SENS_EN
#include "sensor_hub_core_app_ai_ota.h"
#include "mcu_sensor_hub_app_ai.h"
#include "mcu_sensor_hub_app_ai_ota.h"
#include "app_ai_voice.h"
#endif
#include "app_ai_if.h"
#include "app_ai_voice.h"

#ifdef GSOUND_M55_ENABLED
#include "mcu_dsp_m55_app_ai.h"
extern AI_GSOUND_CMD_REQUEST_DATA_RSP_T gsound_cmd_rsp;
#endif
/*********************external function declearation************************/

/************************private macro defination***************************/
// #define HW_TRACE_CP_PROCESS_TIME (1)

/************************private type defination****************************/
typedef struct
{
    char modelId[GSOUND_HOTWORD_MODEL_ID_BYTES];
    int bankCnt;
    uint8_t dataIdx;
    uint32_t dataSize;
    uint8_t textIdx;
    uint32_t textSize;
    GSoundHotwordMmapType bankType;
    void *bankAddr[MEMORY_BANK_CNT];
#ifdef GSOUND_M55_ENABLED
    uint32_t hotwordVersion;
#endif
} MODEL_INFO_T;

/**********************private function declearation************************/
#ifndef GSOUND_M55_ENABLED
CP_TEXT_SRAM_LOC static unsigned int _cp_hw_main(uint8_t event);
static unsigned int _mcu_hw_evt_handler(uint8_t event);
/************************private variable defination************************/
static uint8_t emptyFrame[GSOUND_TARGET_MAX_SBC_FRAME_LEN] = {
    0x00, 0x00, 0x00, 0x00,
    0x6d, 0xb6, 0xdb, 0x6d, 0xb6, 0xdb, 0x6d, 0xb6, 0xdb, 0x6d, 0xb6, 0xdb,
    0x6d, 0xb6, 0xdb, 0x6d, 0xb6, 0xdb, 0x6d, 0xb6, 0xdb, 0x6d, 0xb6, 0xdb,
    0x6d, 0xb6, 0xdb, 0x6d, 0xb6, 0xdb, 0x6d, 0xb6, 0xdb, 0x6d, 0xb6, 0xdb,
    0x6d, 0xb6, 0xdb, 0x6d, 0xb6, 0xdb, 0x6d, 0xb6, 0xdb, 0x6d, 0xb6, 0xdb,
};
#endif

#ifdef HW_TRACE_CP_PROCESS_TIME
static CP_BSS_LOC uint32_t cp_last_process_time;
#endif

osMutexDef(hotwordDataCacheMutex);

HW_EXTERNAL_CONTEXT_T EHWctx;
#if !defined(VOICE_DETECTOR_SENS_EN) && !defined(GSOUND_M55_ENABLED)
#if defined(AI_KWS_ENGINE_OVERLAY)
#include "ai_manager.h"
void gsound_custom_hotword_external_model_reload(void);
void gsound_custom_hotword_external_model_resource_alloc(void);
void gsound_custom_hotword_external_model_resource_release(void);
extern uint8_t _get_work_mode(uint8_t user);

static uint8_t *cpPcmDataCacheBuf = NULL;
static uint8_t *sbcDataCacheBuf = NULL;

static uint8_t __attribute__((aligned(4))) *hwModelData = NULL;
static uint8_t __attribute__((aligned(4))) *hwModelText = NULL;

//static uint8_t cpPcmDataCacheBuf[CP_PCM_DATA_CACHE_BUFFER_SIZE] = {0,};
//static uint8_t sbcDataCacheBuf[HOTWORD_SBC_DATA_CACHE_BUF_MAX_LEN] = {0,};
//static uint8_t __attribute__((aligned(4))) hwModelData[GSOUND_MAX_HOTWORD_MODEL_DATA_BUFFER_SIZE] = {0,};
//static uint8_t __attribute__((aligned(4))) hwModelText[GSOUND_MAX_HOTWORD_MODEL_TEXT_BUFFER_SIZE] = {0,};

#else
static uint8_t cpPcmDataCacheBuf[CP_PCM_DATA_CACHE_BUFFER_SIZE] = {0,};
static uint8_t sbcDataCacheBuf[HOTWORD_SBC_DATA_CACHE_BUF_MAX_LEN] = {0,};
static uint8_t __attribute__((aligned(4))) hwModelData[GSOUND_MAX_HOTWORD_MODEL_DATA_BUFFER_SIZE] = {0,};
static uint8_t __attribute__((aligned(4))) hwModelText[GSOUND_MAX_HOTWORD_MODEL_TEXT_BUFFER_SIZE] = {0,};
#endif
#endif

static bool external_init_process_done = false;
static uint8_t cache_model_id[GSOUND_HOTWORD_MODEL_ID_BYTES*2] = {0};
static uint8_t cache_model_length = 0;

#if !defined(GSOUND_M55_ENABLED) && !defined(GSOUND_HOTWORD_EXTERNAL_M33)
static struct cp_task_desc TASK_DESC_HW = {CP_ACCEL_STATE_CLOSED, _cp_hw_main, NULL, _mcu_hw_evt_handler, NULL};
#endif
/****************************function defination****************************/

#ifdef GSOUND_HOTWORD_EXTERNAL_M33
void _update_read_offset_in_ai_voice_buf(uint32_t offset);

static osThreadId gsound_hotword_m33_tid = NULL;
static void gsound_hotword_m33_pro_thread(void const *argument);
osThreadDef(gsound_hotword_m33_pro_thread, osPriorityAboveNormal, 1, 2048, "gsound_hotword_m33");

static void gsound_hotword_m33_pro_thread_init()
{
    if (gsound_hotword_m33_tid == NULL)
    {
        gsound_hotword_m33_tid = osThreadCreate(osThread(gsound_hotword_m33_pro_thread), NULL);

        ASSERT(gsound_hotword_m33_tid, "create gsound hotword m33 thread fail");
    }
}

static void gsound_hotword_m33_pro_thread(void const *argument)
{
    osEvent evt;
    uint32_t signals = 0;

    while (1)
    {
        //wait any signal
        evt = osSignalWait(0x0, osWaitForever);
        if (evt.status == osEventSignal)
        {
            signals = evt.value.signals;
            if (signals & GSOUND_M33_EVENT_MCU_EVT_HANDLE)
            {
                _mcu_hw_evt_handler((uint8_t)signals);
            }

            if (signals & GSOUND_M33_EVENT_HW_PROCESSING)
            {
                _cp_hw_main(GSOUND_M33_EVENT_HW_PROCESSING);
            }
        }
    }
}
#endif

static uint32_t _read_uint32_big_endian_and_increment(const uint8_t **read_ptr)
{
    uint8_t i = 0;
    uint32_t ret = 0;
    for (i = 0; i < 4; i++)
    {
        ret |= **read_ptr << (8 * (3 - i));
        (*read_ptr)++;
    }
    return ret;
}

#ifdef VOICE_DETECTOR_SENS_EN
static void gsound_custom_hotword_kws_state_update(uint8_t* preambleMs,uint16_t len)
{
    if (HW_DETECT_STATUS_ONGOING == EHWctx.hotwordDetectStatus)
    {
        EHWctx.preambleMs = *(uint32_t *)preambleMs;
    }
    GLOG_D("%s status = %d preambleMs = %d %d",__func__,EHWctx.hotwordDetectStatus,EHWctx.preambleMs,*(uint32_t *)preambleMs);
}

static uint32_t gsound_custom_hotword_kws_history_pcm_data_size_update(uint32_t raw_len)
{
    GLOG_D("%s size = %d",__func__,raw_len);
    EHWctx.kwsHistoryPcmDataSize = raw_len;
    return 0;
}

uint32_t gound_custom_hotword_get_kws_valid_raw_data_len(void)
{
    return (EHWctx.preambleMs * ((VOICE_SBC_SAMPLE_RATE_VALUE) / 1000));
}

static uint32_t gsound_custom_hotword_kws_valid_raw_pcm_sample_fetch(uint8_t * buf,uint32_t len)
{
    uint32_t valid_sample = gound_custom_hotword_get_kws_valid_raw_data_len();
    uint32_t raw_sample = EHWctx.kwsHistoryPcmDataSize;

    return (valid_sample>raw_sample)?(0):(raw_sample - valid_sample);
}

static uint32_t gsound_custom_hotword_model_update_user = 0;

static void gsound_custom_hotword_model_update_request(uint32_t magicCode)
{
    gsound_custom_hotword_model_update_user |= magicCode;
}

static void gsound_custom_hotword_model_update_finish_cb_handler(uint32_t magicCode)
{
    TRACE(3,"%s user = %x code = %x",__func__,gsound_custom_hotword_model_update_user,magicCode);

    if(magicCode & SENSOR_HUB_DATA_UPDATE_SECTION_GG_MAGIC_CODE){
        gsound_custom_hotword_model_update_user &= ~(SENSOR_HUB_DATA_UPDATE_SECTION_GG_MAGIC_CODE);
        gsound_custom_hotword_model_update_user &= ~(magicCode);
    }
    if(gsound_custom_hotword_model_update_user == 0){
        TRACE(0,"end");
        app_sensor_hub_ai_mcu_activate_ai_user(SENSOR_HUB_AI_USER_GG,1);
    }
}

static sensorHubOtaOpCbType sensor_hub_model_update_finish_cb = 
{
    gsound_custom_hotword_model_update_finish_cb_handler,
};

#endif

#if defined(AI_KWS_ENGINE_OVERLAY)
#define GSOUND_CUSTOM_HOTWORD_EXTERNAL_MODEL_YIELD_FOR_CP_CNTS  (20)
osMutexDef(gsound_custom_hotword_external_model_resource_op_mutex);
static osMutexId gsound_custom_hotword_external_model_resource_op_mutex_id = NULL;
static bool gsound_custom_hotword_external_model_run_allow = false;
static bool gsound_custom_hotword_external_model_run_on_cp = false;

SRAM_TEXT_LOC
static inline void gsound_custom_hotword_external_model_op_mutex_alloc(void)
{
    if (gsound_custom_hotword_external_model_resource_op_mutex_id == NULL) {
        gsound_custom_hotword_external_model_resource_op_mutex_id = osMutexCreate(osMutex(gsound_custom_hotword_external_model_resource_op_mutex));
    }
}

SRAM_TEXT_LOC
static inline void gsound_custom_hotword_external_model_op_mutex_req(void)
{
    osMutexWait(gsound_custom_hotword_external_model_resource_op_mutex_id, osWaitForever);
}

SRAM_TEXT_LOC
static inline void gsound_custom_hotword_external_model_op_mutex_rls(void)
{
    osMutexRelease(gsound_custom_hotword_external_model_resource_op_mutex_id);
}

SRAM_TEXT_LOC
static void gsound_custom_hotword_external_model_allow_update(bool op)
{
    uint32_t cnts = 0;
    gsound_custom_hotword_external_model_op_mutex_req();
    if(op == false){
        do{
            if(gsound_custom_hotword_external_model_run_on_cp == false){
                break;
            }
            osDelay(20);
        }while(cnts ++ <GSOUND_CUSTOM_HOTWORD_EXTERNAL_MODEL_YIELD_FOR_CP_CNTS);
    }
    if(cnts >= GSOUND_CUSTOM_HOTWORD_EXTERNAL_MODEL_YIELD_FOR_CP_CNTS){
        ASSERT(0,"%s cnts = %d op = %d",__func__,cnts,op);
    }
    gsound_custom_hotword_external_model_run_allow = op;
    gsound_custom_hotword_external_model_op_mutex_rls();
}


void gsound_custom_hotword_external_model_share_mem_update_notify(uint8_t user, uint8_t exist_user)
{
    bool allow = false;
    if(user != exist_user){
        if(user == AI_SPEC_GSOUND){
            allow = true;
        }else{
            allow = false;
            gsound_custom_hotword_external_model_resource_release();
        }
        gsound_custom_hotword_external_model_allow_update(allow);
        TRACE(4,"%s %d %d %d ",__func__,user,exist_user,allow);
    }
}

void gsound_custom_hotword_external_model_resource_alloc(void)
{
    TRACE(2,"%s %p",__func__,__builtin_return_address(0));
    bool fail = false;
    uint8_t force = 0;
    gsound_custom_hotword_external_model_op_mutex_req();

    uint8_t * buf = NULL;
    app_ai_share_mem_pool_monolize_force(AI_SPEC_INIT,&buf,0);

    if(hwModelData == NULL){
//        hwModelData = (uint8_t*)share_mem_pool_heap_total_malloc( GSOUND_MAX_HOTWORD_MODEL_DATA_BUFFER_SIZE);
        force += (uint8_t)app_ai_share_mem_pool_monolize_force(AI_SPEC_GSOUND,&hwModelData,GSOUND_MAX_HOTWORD_MODEL_DATA_BUFFER_SIZE);
        if(hwModelData == NULL){
            fail = true;
        }
    }
    if(hwModelText == NULL){
//        hwModelText = (uint8_t*)share_mem_pool_heap_total_malloc(GSOUND_MAX_HOTWORD_MODEL_TEXT_BUFFER_SIZE);
         force += (uint8_t)app_ai_share_mem_pool_monolize_force(AI_SPEC_GSOUND,&hwModelText,GSOUND_MAX_HOTWORD_MODEL_TEXT_BUFFER_SIZE);
        if(hwModelText == NULL){
            fail = true;
        }
    }

    if(cpPcmDataCacheBuf == NULL){
        /// force += app_ai_share_mem_pool_monolize_force(AI_SPEC_GSOUND,&cpPcmDataCacheBuf,CP_PCM_DATA_CACHE_BUFFER_SIZE);
        force += app_ai_share_mem_pool_monolize_force(AI_SPEC_GSOUND,(uint8_t**)&cpPcmDataCacheBuf,CP_PCM_DATA_CACHE_BUFFER_SIZE);
        if(cpPcmDataCacheBuf == NULL){
            fail = true;
        }
    }

    if(sbcDataCacheBuf == NULL){
        //force += app_ai_share_mem_pool_monolize_force(AI_SPEC_GSOUND,&sbcDataCacheBuf,HOTWORD_SBC_DATA_CACHE_BUF_MAX_LEN);
        force += app_ai_share_mem_pool_monolize_force(AI_SPEC_GSOUND,(uint8_t**)&sbcDataCacheBuf,HOTWORD_SBC_DATA_CACHE_BUF_MAX_LEN);
        if(sbcDataCacheBuf == NULL){
            fail = true;
        }
    }

    if (fail || force)
    {
        TRACE(3,"%s fail =%d force ?= %d",__func__,fail,force);
        app_ai_share_mem_pool_info_print();
        ASSERT(!fail,"need to check %d %d",fail,force);
    }

    gsound_custom_hotword_external_model_allow_update(true);

    gsound_custom_hotword_external_model_op_mutex_rls();

    TRACE(2,"ptr = %p %p",hwModelData,hwModelText);
}

void gsound_custom_hotword_external_model_resource_release(void)
{
    TRACE(2,"%s %p",__func__,__builtin_return_address(0));
    gsound_custom_hotword_external_model_op_mutex_req();
    if(hwModelData){
//        share_mem_pool_heap_total_free(hwModelData);
        hwModelData = NULL;
    }
    if(hwModelText){
//        share_mem_pool_heap_total_free(hwModelText);
        hwModelText = NULL;
    }
#if 0    
    if(cpPcmDataCacheBuf){
        cpPcmDataCacheBuf = NULL;
    }
    if(sbcDataCacheBuf){
        sbcDataCacheBuf = NULL;
    }
#endif    
//    share_mem_pool_heap_total_info_print();
    gsound_custom_hotword_external_model_allow_update(false);
    gsound_custom_hotword_external_model_op_mutex_rls();
    app_ai_share_mem_pool_info_print();
}
#endif

void gsound_custom_hotword_external_init(void)
{
    GLOG_D("[%s]", __func__);

    memset(&EHWctx, 0, sizeof(EHWctx));

#ifndef GSOUND_M55_ENABLED
    if (EHWctx.hotwordDataCacheMutexId == NULL)
    {
        EHWctx.hotwordDataCacheMutexId = osMutexCreate((osMutex(hotwordDataCacheMutex)));
    }

    EHWctx.hotwordDetectStatus = HW_DETECT_STATUS_IDLE;
    EHWctx.hotwordBufStatus = HW_BUF_STATUS_IDLE;
    EHWctx.isEmptyLastTime = true;
    memcpy(EHWctx.activeModelId, INVALID_MODEL_ID, GSOUND_HOTWORD_MODEL_ID_BYTES);
    memcpy(cache_model_id, INVALID_MODEL_ID, GSOUND_HOTWORD_MODEL_ID_BYTES);
    cache_model_length = 0;

#ifndef VOICE_DETECTOR_SENS_EN
#if defined(AI_KWS_ENGINE_OVERLAY)
    gsound_custom_hotword_external_model_op_mutex_alloc();
    gsound_custom_hotword_external_model_resource_alloc();
    memset(hwModelData, 0, GSOUND_MAX_HOTWORD_MODEL_DATA_BUFFER_SIZE);
    memset(hwModelText, 0, GSOUND_MAX_HOTWORD_MODEL_TEXT_BUFFER_SIZE);
    EHWctx.hotwordModelData = hwModelData;
    EHWctx.hotwordModelText = hwModelText;
    app_ai_share_mem_pool_monolized_info_update_notify_callback_reg(AI_SPEC_GSOUND,(uint32_t)gsound_custom_hotword_external_model_share_mem_update_notify);
#else
    memset(hwModelData, 0, GSOUND_MAX_HOTWORD_MODEL_DATA_BUFFER_SIZE);
    memset(hwModelText, 0, GSOUND_MAX_HOTWORD_MODEL_TEXT_BUFFER_SIZE);
    EHWctx.hotwordModelData = hwModelData;
    EHWctx.hotwordModelText = hwModelText;
#endif
#else
    SENSOR_HUB_AI_OPERATOR_T gsound_operator = 
    {
        {SENSOR_HUB_AI_USER_GG,},
        {0,},
        {
            {gsound_custom_hotword_kws_history_pcm_data_size_update,},
            {gsound_custom_hotword_kws_valid_raw_pcm_sample_fetch,},
            {gound_custom_hotword_get_kws_valid_raw_data_len,},
            {gsound_custom_hotword_kws_state_update,},
            {(sensor_hub_ai_mcu_mic_data_come_handler_t)(app_ai_voice_interface_of_raw_input_handler_get()),},
        },
    };
    sensor_hub_ai_mcu_register_ai_user(SENSOR_HUB_AI_USER_GG,gsound_operator);
#endif
#else
    app_dsp_m55_mcu_ai_send_msg(DSP_M55_AI_USER_GSOUND, DSP_M55_AI_GSOUND_CMD_INIT, NULL, 0);
#endif
}

void gsound_custom_hotword_external_init_buffer(void)
{
    GLOG_D("%s hotword queue len:%d", __func__, HOTWORD_SBC_DATA_CACHE_BUF_MAX_LEN);
#if defined(AI_KWS_ENGINE_OVERLAY)
    gsound_custom_hotword_external_model_resource_alloc();
#endif

    memset(cpPcmDataCacheBuf, 0, CP_PCM_DATA_CACHE_BUFFER_SIZE);
    for (uint8_t i = 0; i < CP_PCM_DATA_CACHE_FRAME_NUM; i++)
    {
        EHWctx.micDataInputBuf[i] = cpPcmDataCacheBuf + (uint32_t)(i * VOICE_SBC_PCM_DATA_SIZE_PER_HANDLE);
    }

    osMutexWait(EHWctx.hotwordDataCacheMutexId, osWaitForever);
    EHWctx.hotwordDataCacheBuf = sbcDataCacheBuf;

    // init mic data cache queue
    InitCQueue(&(EHWctx.hotwordDataCacheQueue),
               HOTWORD_SBC_DATA_CACHE_BUF_MAX_LEN,
               EHWctx.hotwordDataCacheBuf);
    osMutexRelease(EHWctx.hotwordDataCacheMutexId);
}

void gsound_custom_hotword_external_deinit_buffer(void)
{
    GLOG_D("%s", __func__);

    for (uint8_t i = 0; i < CP_PCM_DATA_CACHE_FRAME_NUM; i++)
    {
        EHWctx.micDataInputBuf[i] = NULL;
    }

    osMutexWait(EHWctx.hotwordDataCacheMutexId, osWaitForever);
    EHWctx.hotwordDataCacheBuf = NULL;

    // reset mic data cache queue
    ResetCQueue(&(EHWctx.hotwordDataCacheQueue));
    osMutexRelease(EHWctx.hotwordDataCacheMutexId);
}

void *gsound_custom_hotword_external_get_env(void)
{
    return (void *)&EHWctx;
}

static void _read_model_file(void *modelAddr, MODEL_INFO_T *info)
{
    ASSERT(modelAddr, "invalid model address");
    ASSERT(info, "invalid info pointer");

    uint32_t readValue;
    GSoundHotwordMmapType mType;
    const uint8_t *pRead = (const uint8_t *)modelAddr;

    // model file header version check
    readValue = _read_uint32_big_endian_and_increment(&pRead);
    GLOG_D("model header version:%d", readValue);
    ASSERT(MODEL_HEADER_VERSION == readValue,
           "model file header version parse error");

    // model file library version check
    readValue = _read_uint32_big_endian_and_increment(&pRead);
    GLOG_D("hotword version:%d", readValue);

#ifdef GSOUND_M55_ENABLED
    //m55 call api 'GoogleHotwordVersion'
    info->hotwordVersion = readValue;
#else
    ASSERT(((uint32_t)GoogleHotwordVersion()) == readValue,
           "model file library version parse error");
#endif

    // length of architecture string length
    readValue = _read_uint32_big_endian_and_increment(&pRead);
    GLOG_D("arch len%d", readValue);
    GLOG_D("arch string:");
    DUMP8("0x%02x ", pRead, readValue);
    ASSERT((readValue == strlen(PLATEFORM_ARCHITECTURE) &&
            memcmp(pRead, PLATEFORM_ARCHITECTURE, readValue) == 0),
           "arch info parse error");
    pRead += ARCHITECTURE_LENGTH;

    // memory bank num check
    info->bankCnt = (uint8_t)_read_uint32_big_endian_and_increment(&pRead);
    ASSERT(MEMORY_BANK_CNT >= info->bankCnt,
           "model file bank cnt parse error");
    GLOG_D("bankCnt:%x", info->bankCnt);

    for (uint8_t i = 0; i < info->bankCnt; i++)
    {
        // memory bank type
        mType = (GSoundHotwordMmapType)_read_uint32_big_endian_and_increment(&pRead);
        GLOG_D("mtype %d", mType);

        // memory bank offset
        readValue = _read_uint32_big_endian_and_increment(&pRead);
        info->bankAddr[i] = (void *)(((uint8_t *)modelAddr) + readValue);
        GLOG_D("memBank address: %p", info->bankAddr[i]);

        // memory bank length
        readValue = _read_uint32_big_endian_and_increment(&pRead);

        switch (mType)
        {
        case GSOUND_HOTWORD_MMAP_TEXT:
            // For TEXT section don't copy to RAM
            info->textIdx = i;
            info->textSize = readValue;
            ASSERT(GSOUND_MAX_HOTWORD_MODEL_TEXT_BUFFER_SIZE >= info->textSize,
                   "model file text bank length error");
            break;
        case GSOUND_HOTWORD_MMAP_DATA:
            info->dataIdx = i;
            info->dataSize = readValue;
            ASSERT(GSOUND_MAX_HOTWORD_MODEL_DATA_BUFFER_SIZE >= info->dataSize,
                   "model file data bank length error");
            break;
        case GSOUND_HOTWORD_MMAP_BSS:
            // Not supported yet. Need to allocate scratch space
            ASSERT(0, "BSS not implemented");
            break;
        default:
            ASSERT(0, "Bad hw mmap");
        }
    }
}

static void _load_data_and_init_lib_op(MODEL_INFO_T *info)
{
#if !defined(VOICE_DETECTOR_SENS_EN) && !defined(GSOUND_M55_ENABLED)
    static void *memBank[MEMORY_BANK_CNT];
#endif
    // current model is not the incoming model
        // update the local saved active model info
        memcpy(EHWctx.activeModelId, info->modelId, GSOUND_HOTWORD_MODEL_ID_BYTES);
        memcpy(cache_model_id, info->modelId, GSOUND_HOTWORD_MODEL_ID_BYTES);
        cache_model_length = GSOUND_HOTWORD_MODEL_ID_BYTES;

#if !defined(VOICE_DETECTOR_SENS_EN) && !defined(GSOUND_M55_ENABLED)
        // copy model data into ram
        memcpy((uint8_t *)EHWctx.hotwordModelData, (uint8_t *)info->bankAddr[info->dataIdx], info->dataSize);
        memcpy((uint8_t *)EHWctx.hotwordModelText, (uint8_t *)info->bankAddr[info->textIdx], info->textSize);

        memBank[info->dataIdx] = (void *)EHWctx.hotwordModelData;
        memBank[info->textIdx] = (void *)EHWctx.hotwordModelText;

        for (uint8_t i = 0; i < info->bankCnt; i++)
        {
            GLOG_D("bank Addr:%p", memBank[i]);
        }

        // init hotword detection engine
    {
        EHWctx.mHandle = GoogleHotwordDspMultiBankInit((void **)memBank, info->bankCnt);
    }
        ASSERT(EHWctx.mHandle, "libgsound init error");
#endif
}

static bool _load_data_and_init_lib(MODEL_INFO_T *info,bool reload)
{
    bool updated = false;
    // current model is not the incoming model
    if ((memcmp(info->modelId, EHWctx.activeModelId, GSOUND_HOTWORD_MODEL_ID_BYTES)) ||(reload))
    {
        _load_data_and_init_lib_op(info);
        updated = true;
    }
    else
    {
        GLOG_W("model already inited before");
    }
    return updated;
}

#if defined(AI_KWS_ENGINE_OVERLAY)
SRAM_TEXT_LOC
POSSIBLY_UNUSED void gsound_custom_hotword_external_model_reload(void)
{
    TRACE(2,"%s %p done ?= %d",__func__,__builtin_return_address(0),external_init_process_done);
    if(external_init_process_done == true && !gsound_custom_hotword_external_model_run_allow){
        gsound_custom_hotword_external_init_process((const char *)cache_model_id,cache_model_length,true);
        GoogleHotwordDspMultiBankReset(EHWctx.mHandle);
    }
}
#endif

void gsound_custom_hotword_external_init_process(const char *model_id,
                                                 uint8_t model_length,bool reload)
{
    void *modelAddr = NULL;
    MODEL_INFO_T modelInfo;
    memset((uint8_t *)&modelInfo, 0, sizeof(modelInfo));

    if (0 == model_length || NULL == model_id)
    {
        GLOG_W("all model will be disabled");

        /// clear out its active model disabling hotword until a valid model is provided
        memcpy(EHWctx.activeModelId, INVALID_MODEL_ID, GSOUND_HOTWORD_MODEL_ID_BYTES);
        memcpy(cache_model_id, INVALID_MODEL_ID, GSOUND_HOTWORD_MODEL_ID_BYTES);
        cache_model_length = 0;

#ifdef GSOUND_M55_ENABLED
        memcpy(modelInfo.modelId, INVALID_MODEL_ID, GSOUND_HOTWORD_MODEL_ID_BYTES);
        app_dsp_m55_mcu_ai_send_msg(DSP_M55_AI_USER_GSOUND, DSP_M55_AI_GSOUND_CMD_HW_MODEL_ACTIVATE, &modelInfo, sizeof(MODEL_INFO_T));
#endif        
    }
    else
    {
#if defined(AI_KWS_ENGINE_OVERLAY)
        gsound_custom_hotword_external_model_resource_alloc();
#endif
        modelAddr = (void *)nv_record_gsound_rec_get_hotword_model_addr(model_id, false, 0);
        GLOG_D("modelHeader addr:%p", modelAddr);
        ASSERT(modelAddr, "INVALID model address");

        memcpy(modelInfo.modelId, model_id, GSOUND_HOTWORD_MODEL_ID_BYTES);
        _read_model_file(modelAddr, &modelInfo);
        if(_load_data_and_init_lib(&modelInfo,reload) == true)
        {
#ifndef GSOUND_M55_ENABLED
            external_init_process_done = true;
            memcpy(cache_model_id, model_id, GSOUND_HOTWORD_MODEL_ID_BYTES);
            cache_model_length = model_length;
#ifdef VOICE_DETECTOR_SENS_EN
//        app_sensor_hub_mcu_data_update_write_start(SENSOR_HUB_DATA_UPDATE_SECTION_GG_MAGIC_CODE,(uint8_t*)modelAddr,MODEL_FILE_BUF_SIZE);
        app_sensor_hub_ai_mcu_env_setup(SENSOR_HUB_AI_USER_GG,(uint8_t*)&modelInfo,sizeof(MODEL_INFO_T));
        gsound_custom_hotword_model_update_request(SENSOR_HUB_DATA_UPDATE_SECTION_GG_DATA_MAGIC_CODE);
        gsound_custom_hotword_model_update_request(SENSOR_HUB_DATA_UPDATE_SECTION_GG_TEXT_MAGIC_CODE);
        app_sensor_hub_mcu_data_update_write_start(SENSOR_HUB_DATA_UPDATE_SECTION_GG_DATA_MAGIC_CODE,(uint8_t *)modelInfo.bankAddr[modelInfo.dataIdx], modelInfo.dataSize,sensor_hub_model_update_finish_cb);
        app_sensor_hub_mcu_data_update_write_start(SENSOR_HUB_DATA_UPDATE_SECTION_GG_TEXT_MAGIC_CODE,(uint8_t *)modelInfo.bankAddr[modelInfo.textIdx], modelInfo.textSize,sensor_hub_model_update_finish_cb);
#endif
#else
        app_dsp_m55_mcu_ai_send_msg(DSP_M55_AI_USER_GSOUND, DSP_M55_AI_GSOUND_CMD_HW_MODEL_ACTIVATE, &modelInfo, sizeof(MODEL_INFO_T));
#endif
        }
    }
}

void gsound_custom_hotword_external_handle_mic_data(uint8_t *ptrBuf, uint32_t length, uint8_t *outputPtr)
{
    ASSERT(VOICE_SBC_PCM_DATA_SIZE_PER_HANDLE >= length, "invalid length:%d", length);

#if defined(AI_KWS_ENGINE_OVERLAY)
    if(_get_work_mode(AIV_USER_GVA) == AIV_WORK_MODE_HW_DETECTING)
    {
        bool op_allow = false;
        gsound_custom_hotword_external_model_op_mutex_req();
        op_allow = gsound_custom_hotword_external_model_run_allow;
        if(op_allow == false){
            gsound_custom_hotword_external_model_op_mutex_rls();
    //        TRACE(0,"not allowed [%d]",__LINE__);
            return;
        }
    }
#endif
#ifndef GSOUND_M55_ENABLED
    // configure the global param which will be used in co-processor
    uint8_t cacheIdx = EHWctx.micDataCacheIdx;
    if (EHWctx.micDataInputBuf[cacheIdx])
    {
        memcpy(EHWctx.micDataInputBuf[cacheIdx], ptrBuf, length);
        EHWctx.micDataInputLen[cacheIdx] = length;
        EHWctx.micDataOutputBuf = outputPtr;
        EHWctx.micDataStatus[cacheIdx] = HW_MIC_DATA_CACHED;
        EHWctx.micDataCacheIdx++;

        if (EHWctx.micDataCacheIdx >= CP_PCM_DATA_CACHE_FRAME_NUM)
        {
            EHWctx.micDataCacheIdx = 0;
        }

        GLOG_V("data cached, cacheIdx:%d, processIdx:%d", cacheIdx, EHWctx.micDataProcessIdx);

#ifdef GSOUND_HOTWORD_EXTERNAL_M33
        osSignalSet(gsound_hotword_m33_tid, GSOUND_M33_EVENT_HW_PROCESSING);
#else
        cp_accel_send_event_mcu2cp(CP_BUILD_ID(CP_TASK_HW, CP_EVENT_HW_PROCESSING));
#endif
    }
    else
    {
        EHWctx.micDataInputLen[cacheIdx] = 0;
        GLOG_W("mic data cache buffer is NULL, will drop the data");
    }
#else
    GSOUND_PCM_TO_M55_T gsound_pcm;
    gsound_pcm.in_ptr = ptrBuf;
    gsound_pcm.out_ptr = outputPtr;
    gsound_pcm.length = length;
    app_dsp_m55_mcu_ai_send_msg_with_waiting_rsp(DSP_M55_AI_USER_GSOUND, DSP_M55_AI_GSOUND_CMD_MIC_DATA, &gsound_pcm, sizeof(gsound_pcm));
#endif
#if defined(AI_KWS_ENGINE_OVERLAY)
    gsound_custom_hotword_external_model_op_mutex_rls();
#endif
}

#ifndef GSOUND_M55_ENABLED
CP_TEXT_SRAM_LOC
static unsigned int _cp_hw_main(uint8_t event)
{
    uint8_t processIdx;

    GLOG_V("cache idx:%d", EHWctx.micDataCacheIdx);
    GLOG_V("process idx:%d", EHWctx.micDataProcessIdx);

#ifdef HW_TRACE_CP_PROCESS_TIME
    uint32_t stime;
    uint32_t etime;

    stime = hal_fast_sys_timer_get();
#endif

#if defined(AI_KWS_ENGINE_OVERLAY)
    if(_get_work_mode(AIV_USER_GVA) == AIV_WORK_MODE_HW_DETECTING)
    {
#ifdef GSOUND_HOTWORD_EXTERNAL_M33
        gsound_custom_hotword_external_model_op_mutex_req();
#endif
        if(gsound_custom_hotword_external_model_run_allow == false){
#ifdef GSOUND_HOTWORD_EXTERNAL_M33
            gsound_custom_hotword_external_model_op_mutex_rls();
#endif
            return 0;
        }
        //GLOG_I("enter %d status = %d",__LINE__,gsound_custom_hotword_external_model_run_allow);
        gsound_custom_hotword_external_model_run_on_cp = true;
    }
#endif

    switch (event)
    {
#ifdef GSOUND_HOTWORD_EXTERNAL_M33
    case GSOUND_M33_EVENT_HW_PROCESSING:
#else
    case CP_EVENT_HW_PROCESSING:
#endif    
        for (uint8_t idx = 0; idx < CP_PCM_DATA_CACHE_FRAME_NUM; idx++)
        {
            processIdx = EHWctx.micDataProcessIdx;
            if (HW_MIC_DATA_CACHED == EHWctx.micDataStatus[processIdx])
            {
                gsound_custom_audio_handle_mic_data(EHWctx.micDataInputBuf[processIdx],
                                                    EHWctx.micDataInputLen[processIdx],
                                                    EHWctx.micDataOutputBuf);

                EHWctx.micDataStatus[processIdx] = HW_MIC_DATA_IDLE;
                EHWctx.micDataProcessIdx++;

                if (EHWctx.micDataProcessIdx >= CP_PCM_DATA_CACHE_FRAME_NUM)
                {
                    EHWctx.micDataProcessIdx = 0;
                }
            }
        }

        /// inform mcu data process done
#ifdef GSOUND_HOTWORD_EXTERNAL_M33
        osSignalSet(gsound_hotword_m33_tid, GSOUND_M33_EVENT_PROCESS_DONE);
#else
        cp_accel_send_event_cp2mcu(CP_BUILD_ID(CP_TASK_HW, MCU_EVENT_HW_CP_PROCESS_DONE));
#endif
        break;
    default:
        break;
    }

#if defined(AI_KWS_ENGINE_OVERLAY)
        gsound_custom_hotword_external_model_run_on_cp = false;
#ifdef GSOUND_HOTWORD_EXTERNAL_M33
        gsound_custom_hotword_external_model_op_mutex_rls();
#endif
        //GLOG_I("enter %d status = %d",__LINE__,gsound_custom_hotword_external_model_run_allow);
#endif
#ifdef HW_TRACE_CP_PROCESS_TIME
    etime = hal_fast_sys_timer_get();
    GLOG_I("[%d] cp_decode: %5u us in %5u us",__LINE__,
           FAST_TICKS_TO_US(etime - stime),
           FAST_TICKS_TO_US(etime - cp_last_process_time));
    cp_last_process_time = etime;
#endif

    return 0;
}

static unsigned int _mcu_hw_evt_handler(uint8_t msg)
{
#ifdef GSOUND_HOTWORD_EXTERNAL_M33
    uint8_t event = 0;
    msg = (msg & 0x0F);
    while(!(msg & 0x01))
    {
        msg = msg >> 1;
        event++;
    }
    GLOG_V("received event:%d", event);
#else
    uint8_t event = CP_EVENT_GET(msg);
    GLOG_V("received cp event:%d", event);
#endif

    switch (event)
    {
    case MCU_EVENT_HW_DETECTED:
        // inform libgsound hotword detected, and wait for libgsound call @see GSoundTargetHotwordRequestForData
        EHWctx.hotwordInterface->gsound_on_hotword_detected(gsound_custom_audio_get_bitpool());
        break;

    case MCU_EVENT_HW_NEW_DATA_AVAILABLE:
        // inform libgsound that new data is coming in, and wait for libgsound call @see GSoundTargetHotwordRequestForData
        EHWctx.hotwordInterface->gsound_on_hotword_data_available(EHWctx.newAvailableDataLen);
        break;

    case MCU_EVENT_HW_UPSTREAM_DATA:
        gsound_custom_audio_transmit_data_to_libgsound();
        break;

    case MCU_EVENT_HW_CP_PROCESS_DONE:
        gsound_custom_audio_trigger_data_process();
        break;

#ifdef VOICE_DETECTOR_EN
    case MCU_EVENT_HW_START_VAD:
        if (app_voicepath_get_vad_audio_capture_start_flag())
        {
            app_voicepath_set_vad_audio_capture_start_flag(0);
            app_voice_detector_capture_enable_vad(VOICE_DETECTOR_ID_0, VOICE_DET_USER_AI);
        }
        break;
#endif

#ifdef GSOUND_HOTWORD_EXTERNAL_M33
    case MCU_EVENT_M33_MANUAL_STOP:
        {
            _update_read_offset_in_ai_voice_buf(0);
        }
        break;
#endif

    default:
        GLOG_E("invalid event received:%d", event);
        break;
    }

    return 0;
}

int gsound_custom_hotword_external_start_cp_process(void)
{
#if defined(AI_KWS_ENGINE_OVERLAY)
    if(ai_manager_get_current_spec() >AI_SPEC_GSOUND ){
        return 0;
    }
    gsound_custom_hotword_external_model_reload();
#endif

#ifdef GSOUND_HOTWORD_EXTERNAL_M33
    gsound_hotword_m33_pro_thread_init();
#else
    norflash_api_flush_disable(NORFLASH_API_USER_CP,(uint32_t)cp_accel_init_done, false);
    cp_accel_open(CP_TASK_HW, &TASK_DESC_HW);
    while (cp_accel_init_done() == false)
    {
        TRACE(1, "[%s] Delay...", __func__);
        osDelay(1);
    }
    norflash_api_flush_enable(NORFLASH_API_USER_CP);
#ifdef HW_TRACE_CP_PROCESS_TIME
    cp_last_process_time = hal_fast_sys_timer_get();
#endif
#endif
    GLOG_D("[%s]", __func__);

    return 0;
}

int gsound_custom_hotword_external_stop_cp_process(void)
{
    GLOG_I("external hotword stop the mic data processing in cp");
#if defined(AI_KWS_ENGINE_OVERLAY)
    if(ai_manager_get_current_spec() >AI_SPEC_GSOUND ){
        return 0;
    }
#endif
#ifndef GSOUND_HOTWORD_EXTERNAL_M33
    cp_accel_close(CP_TASK_HW);
#else
    osSignalSet(gsound_hotword_m33_tid, MCU_EVENT_M33_MANUAL_STOP);
#endif
#if defined(AI_KWS_ENGINE_OVERLAY)
    gsound_custom_hotword_external_model_resource_release();
#endif
    return 0;
}
#endif

void gsound_custom_hotword_external_start_detection(const GSoundTargetAudioInSettings *audio_config,
                                                    const GSoundTargetHotwordConfig *hw_config)
{
    // parameter validity check
    ASSERT(hw_config->audio_backlog_ms <= GSOUND_TARGET_MAX_HOTWORD_BUF_LEN_MS, "audio_backlog_ms exceed.");

    // allow bt enter sniff mode(since hotword detection is locally, no need to transmit audio data to APP)
    gsound_custom_bt_allow_sniff();

    memcpy((void *)&EHWctx.hwConfig,
           (void *)hw_config,
           sizeof(GSoundTargetHotwordConfig));
    GLOG_I("audio_backlog_ms is set to:%d", EHWctx.hwConfig.audio_backlog_ms);

#ifndef GSOUND_M55_ENABLED
    while (HW_BUF_STATUS_IDLE != EHWctx.hotwordBufStatus)
    {
        hal_sys_timer_delay_us(10);
    }
    EHWctx.hotwordBufStatus = HW_BUF_STATUS_PROCESSING;
    osMutexWait(EHWctx.hotwordDataCacheMutexId, osWaitForever);
    // Queue size is calculated according to EHWctx.hwConfig.audio_backlog_ms
    ResetCQueue(&(EHWctx.hotwordDataCacheQueue));
    osMutexRelease(EHWctx.hotwordDataCacheMutexId);
    EHWctx.hotwordBufStatus = HW_BUF_STATUS_IDLE;

    EHWctx.micDataCacheIdx = 0;
    EHWctx.micDataProcessIdx = 0;
    EHWctx.micDataOutputBuf = NULL;

#ifdef VOICE_DETECTOR_SENS_EN
    EHWctx.rawInputLenPerSession = 0;
    EHWctx.kwsHistoryPcmDataSize = 0;
#endif

    for (uint8_t i = 0; i < CP_PCM_DATA_CACHE_FRAME_NUM; i++)
    {
        EHWctx.micDataStatus[i] = HW_MIC_DATA_IDLE;
    }

    // set hotword status to DETECT_ONGOING
    // start hotword detection progress
    EHWctx.hotwordDetectStatus = HW_DETECT_STATUS_ONGOING;

#ifndef VOICE_DETECTOR_SENS_EN
    GoogleHotwordDspMultiBankReset(EHWctx.mHandle);
#endif

    // store audio_config
    gsound_custom_audio_store_audio_in_settings(audio_config);
#else
    app_dsp_m55_mcu_ai_send_msg(DSP_M55_AI_USER_GSOUND, DSP_M55_AI_GSOUND_CMD_HW_START_DETECTION, (void*)audio_config, sizeof(audio_config));
#endif

    // open mic and start capture stream
    gsound_custom_audio_open_mic(audio_config->sbc_bitpool);
}

void gsound_custom_hotword_external_request_data(uint32_t max_num_bytes,
                                                 uint32_t bytes_per_frame,
                                                 GSoundTargetHotwordDataMsg *sync_hw_data_out,
                                                 bool *ready_now_out)
{
    GLOG_D("[%s] bytes:%d, bytes_per_frame:%d", __func__, max_num_bytes, bytes_per_frame);
    uint32_t frameLen = gsound_custom_audio_get_output_sbc_frame_len();

    GLOG_V("max required data len:%d", max_num_bytes);
    ASSERT(bytes_per_frame == frameLen,
           "SBC frame size is not expected:%d/%d",
           frameLen,
           bytes_per_frame);
#if defined(AI_KWS_ENGINE_OVERLAY)
    bool op_allow = false;
    gsound_custom_hotword_external_model_op_mutex_req();
    op_allow = gsound_custom_hotword_external_model_run_allow;
    if(op_allow == false){
        EHWctx.hotwordBufStatus = HW_BUF_STATUS_IDLE;

        EHWctx.hotwordDetectStatus = HW_DETECT_STATUS_UPSTREAMING;
        gsound_custom_hotword_external_model_op_mutex_rls();
        // TRACE(0,"not allowed");
        return;
    }
#endif

#ifndef GSOUND_M55_ENABLED
    int maxOutput = max_num_bytes / bytes_per_frame;
    maxOutput *= bytes_per_frame;
    unsigned int len1, len2;

    while (HW_BUF_STATUS_IDLE != EHWctx.hotwordBufStatus)
    {
        hal_sys_timer_delay_us(10);
    }
    EHWctx.hotwordBufStatus = HW_BUF_STATUS_PROCESSING;
    osMutexWait(EHWctx.hotwordDataCacheMutexId, osWaitForever);

    // libgsound ask for first packet data
    if (HW_DETECT_STATUS_START_UPSTREAM == EHWctx.hotwordDetectStatus)
    {
        // calculate the libgsound required data length
        double frameCntPerMs = gsound_custom_audio_get_sbc_frame_cnt_per_ms();
        uint32_t dataCnt = ((uint32_t)(frameCntPerMs * EHWctx.preambleMs)) * frameLen;
        GLOG_I("%d bytes data is required from libgsound, cached data length:%d",
               dataCnt,
               LengthOfCQueue(&(EHWctx.hotwordDataCacheQueue)));

        // make sure cached data is enough for libgsound required
        if (dataCnt > (uint32_t)LengthOfCQueue(&(EHWctx.hotwordDataCacheQueue)))
        {
            dataCnt -= LengthOfCQueue(&(EHWctx.hotwordDataCacheQueue));
            uint16_t emptyFrameNum = dataCnt / ARRAY_SIZE(emptyFrame);
            if (dataCnt % ARRAY_SIZE(emptyFrame))
            {
                emptyFrameNum++;
            }

            GLOG_D("%d frames empty data enqueued", emptyFrameNum);
            for (uint16_t i = 0; i < emptyFrameNum; i++)
            {
                // enqueue empty data to head of the queue
                ASSERT(CQ_OK == EnCQueueFront(&(EHWctx.hotwordDataCacheQueue), (CQItemType *)&emptyFrame, ARRAY_SIZE(emptyFrame)),
                       "%s EnCQueueFront error, avaliable:%d",
                       __func__, AvailableOfCQueue(&(EHWctx.hotwordDataCacheQueue)));
            }
        }
        else
        {
            // calculate the redundant data length and drop them
            dataCnt = LengthOfCQueue(&(EHWctx.hotwordDataCacheQueue)) - dataCnt;
            DeCQueue(&(EHWctx.hotwordDataCacheQueue), NULL, dataCnt);
            GLOG_D("dropped %d cached data, left data length:%d",
                   dataCnt,
                   LengthOfCQueue(&(EHWctx.hotwordDataCacheQueue)));
        }
    }

    sync_hw_data_out->bytes_per_frame = frameLen;

    // data cache buffer is empty
    if (CQ_OK == IsEmptyCQueue(&(EHWctx.hotwordDataCacheQueue)))
    {
        GLOG_W("hotword data cache buffer is empty");
        sync_hw_data_out->data1 = NULL;
        sync_hw_data_out->data2 = NULL;
        sync_hw_data_out->num_bytes1 = 0;
        sync_hw_data_out->num_bytes2 = 0;
        *ready_now_out = false;

        // mark buffer status, so that we can inform libgsound when new data come in
        EHWctx.isEmptyLastTime = true;
    }
    // data cache buffer is not empty
    else
    {
        maxOutput = (maxOutput < LengthOfCQueue(&(EHWctx.hotwordDataCacheQueue)))
                        ? maxOutput
                        : LengthOfCQueue(&(EHWctx.hotwordDataCacheQueue));

        // send data to libgsound
        ASSERT(CQ_ERR != PeekCQueue(&(EHWctx.hotwordDataCacheQueue),
                                    maxOutput,
                                    (CQItemType **)&(sync_hw_data_out->data1),
                                    &len1,
                                    (CQItemType **)&(sync_hw_data_out->data2),
                                    &len2),
               "peek queue error");

        sync_hw_data_out->num_bytes1 = len1;
        sync_hw_data_out->num_bytes2 = len2;
        *ready_now_out = true;

        GLOG_V("num_bytes1:%d, num_bytes2:%d, bytes_per_frame:%d",
               sync_hw_data_out->num_bytes1,
               sync_hw_data_out->num_bytes2,
               sync_hw_data_out->bytes_per_frame);
    }

    osMutexRelease(EHWctx.hotwordDataCacheMutexId);
    EHWctx.hotwordBufStatus = HW_BUF_STATUS_IDLE;

    EHWctx.hotwordDetectStatus = HW_DETECT_STATUS_UPSTREAMING;
#else
    AI_GSOUND_CMD_REQUEST_DATA_REQ_T req_data;
    req_data.max_num_bytes = max_num_bytes;
    req_data.bytes_per_frame = bytes_per_frame;

    memset(&gsound_cmd_rsp, 0x00, sizeof(gsound_cmd_rsp));
    app_dsp_m55_mcu_ai_send_msg_with_waiting_rsp(DSP_M55_AI_USER_GSOUND, DSP_M55_AI_GSOUND_CMD_REQUEST_DATA, (void*)&req_data, sizeof(req_data));

    if (gsound_cmd_rsp.valid)
    {
        memcpy(sync_hw_data_out, &gsound_cmd_rsp.sync_hw_data_out, sizeof(gsound_cmd_rsp.sync_hw_data_out));
        *ready_now_out = true;
    }
    else
    {
        TRACE(1, "hotword data cache buffer is empty");
        sync_hw_data_out->data1 = NULL;
        sync_hw_data_out->data2 = NULL;
        sync_hw_data_out->num_bytes1 = 0;
        sync_hw_data_out->num_bytes2 = 0;
        *ready_now_out = false;
    }
#endif

#if defined(AI_KWS_ENGINE_OVERLAY)
    gsound_custom_hotword_external_model_op_mutex_rls();
#endif

}

void gsound_custom_hotword_data_consumed(GSoundTargetHotwordDataMsg *hw_data)
{
    GLOG_V("data1 consumed:%d, data2 consumed:%d, bytes_per_frame:%d",
           hw_data->num_bytes1, hw_data->num_bytes2, hw_data->bytes_per_frame);

    // ASSERT(0 == ((hw_data->num_bytes1 + hw_data->num_bytes2) % hw_data->bytes_per_frame),
    //        "consumed bytes should be integer multiples of bytes_per_frame");

#ifndef GSOUND_M55_ENABLED
    while (HW_BUF_STATUS_IDLE != EHWctx.hotwordBufStatus)
    {
        hal_sys_timer_delay_us(10);
    }
    EHWctx.hotwordBufStatus = HW_BUF_STATUS_PROCESSING;
    osMutexWait(EHWctx.hotwordDataCacheMutexId, osWaitForever);
    DeCQueue(&(EHWctx.hotwordDataCacheQueue), NULL, hw_data->num_bytes1 + hw_data->num_bytes2);
    osMutexRelease(EHWctx.hotwordDataCacheMutexId);
    EHWctx.hotwordBufStatus = HW_BUF_STATUS_IDLE;
#else
    app_dsp_m55_mcu_ai_send_msg(DSP_M55_AI_USER_GSOUND, DSP_M55_AI_GSOUND_CMD_DATA_CONSUMED, (void*)hw_data, sizeof(*hw_data));
#endif
}

int gsound_custom_hotword_get_lib_info(const GSoundHotwordLibInfo *const info_out)
{
    int ret = 0;
#ifndef GSOUND_M55_ENABLED
    int version = GoogleHotwordVersion();
    sprintf(EHWctx.libVersionStr, "%d", version);

    ASSERT(strlen(EHWctx.libVersionStr) < info_out->version_length,
        "version length exceeded:%d|%d", strlen(EHWctx.libVersionStr), info_out->version_length);
    strcpy(info_out->hotword_library_version, EHWctx.libVersionStr);
    GLOG_D("version is %s, strLen is %d", info_out->hotword_library_version, info_out->version_length);
#endif
    ASSERT(strlen(PLATEFORM_ARCHITECTURE) < info_out->arch_length,
        "arch length exceeded:%d|%d", strlen(PLATEFORM_ARCHITECTURE), info_out->arch_length);
    strcpy(info_out->hotword_library_architecture, PLATEFORM_ARCHITECTURE);

    uint8_t length = info_out->supported_model_length;
    ret = nv_record_gsound_rec_get_supported_model_id(info_out->supported_models, &length);
    ASSERT(length < info_out->supported_model_length,
        "supported model length exceeded:%d|%d", length, info_out->supported_model_length);

    ASSERT((GSOUND_HOTWORD_MODEL_ID_BYTES) < info_out->active_model_length,
        "active model length exceeded:%d|%d", GSOUND_HOTWORD_MODEL_ID_BYTES, info_out->active_model_length);
    if (memcmp(EHWctx.activeModelId, INVALID_MODEL_ID, GSOUND_HOTWORD_MODEL_ID_BYTES))
    {
        const char* terminator = STRING_TERMINATOR_NULL;
        memcpy(info_out->active_model, EHWctx.activeModelId, GSOUND_HOTWORD_MODEL_ID_BYTES);
        memcpy((info_out->active_model + GSOUND_HOTWORD_MODEL_ID_BYTES), terminator, STRING_TERMINATOR_SIZE);
        GLOG_D("activated model: %s", info_out->active_model);
    }
    else
    {
        memset(info_out->active_model, 0, info_out->active_model_length);
        GLOG_D("no activated model");
    }

    return ret;
}

int gsound_custom_hotword_external_stop_stream(void)
{
    int ret = 0;

#ifndef GSOUND_M55_ENABLED
    GLOG_I("current hotword detection status:%d", EHWctx.hotwordDetectStatus);

    if (HW_DETECT_STATUS_UPSTREAMING == EHWctx.hotwordDetectStatus)
    {
        // enable the local mic data processing again
        EHWctx.hotwordDetectStatus = HW_DETECT_STATUS_ONGOING;

        while (HW_BUF_STATUS_IDLE != EHWctx.hotwordBufStatus)
        {
            osDelay(5);
        }
        EHWctx.hotwordBufStatus = HW_BUF_STATUS_PROCESSING;
        osMutexWait(EHWctx.hotwordDataCacheMutexId, osWaitForever);
        // clear the hotword data cache buffer
        DeCQueue(&(EHWctx.hotwordDataCacheQueue), NULL, LengthOfCQueue(&(EHWctx.hotwordDataCacheQueue)));
        GLOG_I("hotword data cache queue cleared, current len:%d", LengthOfCQueue(&(EHWctx.hotwordDataCacheQueue)));
        osMutexRelease(EHWctx.hotwordDataCacheMutexId);
        EHWctx.hotwordBufStatus = HW_BUF_STATUS_IDLE;

        EHWctx.micDataCacheIdx = 0;
        EHWctx.micDataProcessIdx = 0;
        EHWctx.micDataOutputBuf = NULL;
#ifdef VOICE_DETECTOR_SENS_EN
        EHWctx.kwsHistoryPcmDataSize = 0;
        EHWctx.rawInputLenPerSession = 0;
#endif    
        for (uint8_t i = 0; i < CP_PCM_DATA_CACHE_FRAME_NUM; i++)
        {
            EHWctx.micDataStatus[i] = HW_MIC_DATA_IDLE;
        }
    }
    else
    {
        ret = -1;
        ASSERT(0, "hotword detect status error.");
    }
#else
    app_dsp_m55_mcu_ai_send_msg(DSP_M55_AI_USER_GSOUND, DSP_M55_AI_GSOUND_CMD_HW_STOP_STREAM, NULL, 0);
#endif
    return ret;
}

void gsound_custom_hotword_external_stop_detection(bool voiceReady)
{
    // just encode the data and wait libgsound call
    // @see GSoundTargetAudioInOpen pass data to libgsound
    if (voiceReady)
    {
        GLOG_I("stop detection but not stop the stream");
    }
    // close the mic data capture
    else
    {
        gsound_custom_audio_close_mic();
    }
#ifndef GSOUND_M55_ENABLED 
    while (HW_BUF_STATUS_IDLE != EHWctx.hotwordBufStatus)
    {
        hal_sys_timer_delay_us(10);
    }
    EHWctx.hotwordBufStatus = HW_BUF_STATUS_PROCESSING;
    osMutexWait(EHWctx.hotwordDataCacheMutexId, osWaitForever);
    // clear the hotword data cache buffer
    DeCQueue(&(EHWctx.hotwordDataCacheQueue), NULL, LengthOfCQueue(&(EHWctx.hotwordDataCacheQueue)));
    osMutexRelease(EHWctx.hotwordDataCacheMutexId);
    EHWctx.hotwordBufStatus = HW_BUF_STATUS_IDLE;

    EHWctx.hotwordDetectStatus = HW_DETECT_STATUS_IDLE;

    EHWctx.micDataCacheIdx = 0;
    EHWctx.micDataProcessIdx = 0;
    EHWctx.micDataOutputBuf = NULL;
#ifdef VOICE_DETECTOR_SENS_EN
    EHWctx.kwsHistoryPcmDataSize = 0;
    EHWctx.rawInputLenPerSession = 0;

    app_sensor_hub_ai_mcu_request_vad_data_close();
#endif    
    for (uint8_t i = 0; i < CP_PCM_DATA_CACHE_FRAME_NUM; i++)
    {
        EHWctx.micDataStatus[i] = HW_MIC_DATA_IDLE;
    }
#else
    app_dsp_m55_mcu_ai_send_msg(DSP_M55_AI_USER_GSOUND, DSP_M55_AI_GSOUND_CMD_HW_STOP_DETECTION, NULL, 0);
#endif
}

void gsound_custom_hotword_external_store_active_model(const char *model_id,
                                                       uint8_t model_length)
{
    GLOG_I("%s", __func__);

    if (0 == model_length || NULL == model_id)
    {
        GLOG_I("disable all models");

        // disable all hotword models
        memcpy(EHWctx.activeModelId, INVALID_MODEL_ID, GSOUND_HOTWORD_MODEL_ID_BYTES);
        memcpy(cache_model_id, INVALID_MODEL_ID, GSOUND_HOTWORD_MODEL_ID_BYTES);
        cache_model_length = 0;
        EHWctx.hotwordDetectStatus = HW_DETECT_STATUS_IDLE;
    }
    else
    {
        GLOG_I("activate model id(len %d): %s", model_length, model_id);

        // TODO: parse the incoming param, save the glib configuration and enable corresponding models
    }
}

void gsound_custom_hotword_external_store_interface(const GSoundHotwordInterface *handlers)
{
    EHWctx.hotwordInterface = handlers;
}

CP_TEXT_SRAM_LOC
void gsound_custom_hotword_encoded_data_fill_in_target_cache(uint8_t * encodedDataBuf,uint32_t encodedDataLen)
{
    uint32_t availableCacheQueue = 0;
    uint32_t removeLen = 0;
    uint32_t frameLen = gsound_custom_audio_get_output_sbc_frame_len();

    availableCacheQueue = AvailableOfCQueue(&(EHWctx.hotwordDataCacheQueue));

    // throw the earliest cached data, if the queue is not enough
    if (availableCacheQueue < encodedDataLen)
    {
        if (HW_DETECT_STATUS_UPSTREAMING == EHWctx.hotwordDetectStatus)
        {
            GLOG_W("hotword data cache buff exceed!");
        }

        removeLen = (encodedDataLen - availableCacheQueue) / frameLen * frameLen;
        if (removeLen != (encodedDataLen - availableCacheQueue))
        {
            removeLen += frameLen;
        }

        GLOG_V("will remove %d bytes", encodedDataLen - availableCacheQueue);
#ifdef GSOUND_HOTWORD_EXTERNAL
        DeCQueue(&(EHWctx.hotwordDataCacheQueue), NULL, encodedDataLen - availableCacheQueue);
#else
        DeCpQueue(&(EHWctx.hotwordDataCacheQueue), NULL, encodedDataLen - availableCacheQueue);
#endif
    }

#ifdef GSOUND_HOTWORD_EXTERNAL
    EnCQueue(&(EHWctx.hotwordDataCacheQueue), encodedDataBuf, encodedDataLen);
#else
    EnCpQueue(&(EHWctx.hotwordDataCacheQueue), encodedDataBuf, encodedDataLen);
#endif
}

CP_TEXT_SRAM_LOC
void gsound_custom_hotword_external_detect_kws_process(uint8_t * rawDataBuf,uint32_t rawDataLen)
{
#ifndef VOICE_DETECTOR_SENS_EN
    static uint32_t hotwordIdleCycle = 0;

    //// detecet hotword
    // local detection
    if (HW_DETECT_STATUS_ONGOING == EHWctx.hotwordDetectStatus)
    {
        if (EHWctx.mHandle)
        {
            // hotword local detected
            // NOTE: should use raw PCM data according to BISTO team
            if (GoogleHotwordDspMultiBankProcess((const void *)rawDataBuf,
                                                 rawDataLen / VOICE_SBC_SIZE_PER_SAMPLE,
                                                 &EHWctx.preambleMs,
                                                 EHWctx.mHandle))
            {
                GLOG_D("external hotword detected, preableMs:%d", EHWctx.preambleMs);

                // update the hotword detect status
                EHWctx.hotwordDetectStatus = HW_DETECT_STATUS_START_UPSTREAM;
                hotwordIdleCycle = 0;
#ifdef GSOUND_HOTWORD_EXTERNAL_M33
                osSignalSet(gsound_hotword_m33_tid, GSOUND_M33_EVENT_HW_DETECTED);
#else
                cp_accel_send_event_cp2mcu(CP_BUILD_ID(CP_TASK_HW, MCU_EVENT_HW_DETECTED));
#endif
            }
            else
            {
                hotwordIdleCycle++;

#ifdef VOICE_DETECTOR_EN
                if (hotwordIdleCycle % 300 == 0)
                {
                    ResetCpQueue(&(EHWctx.hotwordDataCacheQueue));
                    cp_accel_send_event_cp2mcu(CP_BUILD_ID(CP_TASK_HW, MCU_EVENT_HW_START_VAD));
                }
#endif
            }
        }
        else
        {
            GLOG_W("glib memory handle is null");
        }
    }
#else
    EHWctx.rawInputLenPerSession += rawDataLen;
    uint32_t expect_len = gound_custom_hotword_get_kws_valid_raw_data_len();
    if(EHWctx.rawInputLenPerSession >= expect_len){
        if(EHWctx.hotwordDetectStatus == HW_DETECT_STATUS_ONGOING){
            EHWctx.hotwordDetectStatus = HW_DETECT_STATUS_START_UPSTREAM;
            cp_accel_send_event_cp2mcu(CP_BUILD_ID(CP_TASK_HW, MCU_EVENT_HW_DETECTED));
        }
    }
#endif
}

CP_TEXT_SRAM_LOC
void gsound_custom_hotword_external_notify_glib_data_avaliable(uint8_t *encodedDataBuf,
                                                 uint32_t encodedDataLen)
{
    if (HW_DETECT_STATUS_UPSTREAMING == EHWctx.hotwordDetectStatus)
    {
        if (EHWctx.isEmptyLastTime)
        {
            // update the cache buffer status
            EHWctx.isEmptyLastTime = false;

            EHWctx.newAvailableDataLen = encodedDataLen;
#ifdef GSOUND_HOTWORD_EXTERNAL_M33
            osSignalSet(gsound_hotword_m33_tid, GSOUND_M33_EVENT_NEW_DATA_AVAILABLE);
#else
            cp_accel_send_event_cp2mcu(CP_BUILD_ID(CP_TASK_HW, MCU_EVENT_HW_NEW_DATA_AVAILABLE));
#endif
        }
    }
}
CP_TEXT_SRAM_LOC
void gsound_custom_hotword_external_process_data(uint8_t *rawDataBuf,
                                                 uint32_t rawDataLen,
                                                 uint8_t *encodedDataBuf,
                                                 uint32_t encodedDataLen,
                                                 bool upstreamFlag)
{
    GLOG_V("[%s]+++", __func__);

    // just transmit data to libgsound
    if (upstreamFlag)
    {
        GLOG_I("upstream data to libgsound");
#ifdef GSOUND_HOTWORD_EXTERNAL_M33
        osSignalSet(gsound_hotword_m33_tid, GSOUND_M33_EVENT_UPSTREAM_DATA);
#else
        cp_accel_send_event_cp2mcu(CP_BUILD_ID(CP_TASK_HW, MCU_EVENT_HW_UPSTREAM_DATA));
#endif
    }
    else
    {
        if (HW_DETECT_STATUS_IDLE != EHWctx.hotwordDetectStatus)
        {
            while (HW_BUF_STATUS_IDLE != EHWctx.hotwordBufStatus)
            {
                hal_sys_timer_delay_us(10);
            }

            EHWctx.hotwordBufStatus = HW_BUF_STATUS_CACHING;


            gsound_custom_hotword_encoded_data_fill_in_target_cache(encodedDataBuf,encodedDataLen);
            gsound_custom_hotword_external_detect_kws_process(rawDataBuf,rawDataLen);
            gsound_custom_hotword_external_notify_glib_data_avaliable(encodedDataBuf,encodedDataLen);

            EHWctx.hotwordBufStatus = HW_BUF_STATUS_IDLE;
            GLOG_V("hotword detect status:%d", EHWctx.hotwordDetectStatus);
        }
        else
        {
            GLOG_I("will not cache hotword data because detect status is idle");
        }
    }

    GLOG_V("[%s]---", __func__);
}
