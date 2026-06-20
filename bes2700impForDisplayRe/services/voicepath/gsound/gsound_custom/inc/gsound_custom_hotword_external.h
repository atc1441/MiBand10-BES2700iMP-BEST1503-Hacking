/***************************************************************************
*
*Copyright 2015-2019 BES.
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


#ifndef __GSOUND_CUSTOM_HOTWORD_EXTERNAL_H__
#define __GSOUND_CUSTOM_HOTWORD_EXTERNAL_H__

#ifdef __cplusplus
extern "C"{
#endif

/*****************************header include********************************/
#include "gsound_target_hotword_external.h"
#include "cmsis_os.h"
#include "cp_accel.h"
#include "cp_queue.h"
#ifndef GSOUND_M55_ENABLED
#include "voice_sbc.h"
#endif
#include "app_audio.h"

/******************************macro defination*****************************/
#ifndef __ARRAY_EMPTY
#define __ARRAY_EMPTY 1
#endif

/// cp cache PCM data frame number
#define CP_PCM_DATA_CACHE_FRAME_NUM 4

/// cp cache PCM data buffer length
#define CP_PCM_DATA_CACHE_BUFFER_SIZE (CP_PCM_DATA_CACHE_FRAME_NUM * VOICE_SBC_PCM_DATA_SIZE_PER_HANDLE)

/// SBC compress ratio, this is a rough estimation
#define SBC_COMPRESS_RATIO 4

/// length of buffer used to cache the SBC encoded hotword data
#define HOTWORD_SBC_DATA_CACHE_BUF_MAX_LEN \
    ((GSOUND_TARGET_MAX_HOTWORD_BUF_LEN_MS * VOICE_SBC_SAMPLE_RATE_VALUE) / 1000) * \
    VOICE_SBC_SIZE_PER_SAMPLE / SBC_COMPRESS_RATIO / GSOUND_TARGET_MAX_SBC_FRAME_LEN * \
    GSOUND_TARGET_MAX_SBC_FRAME_LEN

/// valid platform architecture string stored in hotword model file
#define PLATEFORM_ARCHITECTURE "CM4_MB"

/// invalid hotword model ID
#define INVALID_MODEL_ID "FRED"

/// memory bank count number in hotword model file
#define MEMORY_BANK_CNT 2 //!< text bank and data bank

/// max version string length in hotword model file
#define VERSION_STRING_MAX_LEN 12

/// valid header version number for hotword model file
#define MODEL_HEADER_VERSION 0

/// length of architecture string stored in hotword model file
#define ARCHITECTURE_LENGTH 16

/// buffer size to store the data section of hotword model file
#define GSOUND_MAX_HOTWORD_MODEL_DATA_BUFFER_SIZE 23524

/// buffer size to store the text section of hotword model file
#define GSOUND_MAX_HOTWORD_MODEL_TEXT_BUFFER_SIZE 43500

/// max alignment edian
#define GSOUND_MAX_HOTWORD_MODEL_ALIGNMENT_EDIAN 4

#ifdef GSOUND_HOTWORD_EXTERNAL_M33
#define GSOUND_M33_EVENT_MCU_EVT_HANDLE      (0x10)
#define GSOUND_M33_EVENT_HW_DETECTED         (0x1 << 0 | GSOUND_M33_EVENT_MCU_EVT_HANDLE)
#define GSOUND_M33_EVENT_NEW_DATA_AVAILABLE  (0x1 << 1 | GSOUND_M33_EVENT_MCU_EVT_HANDLE)
#define GSOUND_M33_EVENT_UPSTREAM_DATA       (0x1 << 2 | GSOUND_M33_EVENT_MCU_EVT_HANDLE)
//#define GSOUND_M33_EVENT_START_VAD         (0x1 << 3 | GSOUND_M33_EVENT_MCU_EVT_HANDLE)
#define GSOUND_M33_EVENT_PROCESS_DONE        (0x1 << 4 | GSOUND_M33_EVENT_MCU_EVT_HANDLE)
#define GSOUND_M33_MANUAL_STOP               (0x1 << 5 | GSOUND_M33_EVENT_MCU_EVT_HANDLE)
#define GSOUND_M33_EVENT_HW_PROCESSING       (0x20)

#ifdef CP_TEXT_SRAM_LOC
#undef CP_TEXT_SRAM_LOC
#define CP_TEXT_SRAM_LOC
#endif
#endif

/******************************type defination******************************/
typedef struct
{
    /**
     * RO/RW/RW uninitialized memory. This is 32-bits big endian
     */
    GSoundHotwordMmapType type;

    /**
     * Bytes from the beginning of the model to start of mmap section
     */
    uint32_t offset_bytes;

    /**
     * Number of bytes this memory occupies. For the TEXT and DATA types this
     * length indicates the size of initialized data. For the BSS type this length
     * indicates the memory that must be allocated.
     */
    uint32_t length_bytes;
} GSoundHotwordMmapHeader;

/**
 * This is a representation of the header stored in flash. Assume no padding in
 * the structure. All fields are 32-bits stored in big-endian format.
 * This struct will be at the beginning of a hotword model OTA'd file. Use it to
 * parse and load the models on GSoundTargetSetModel
 */
typedef struct
{
    /** Version of the format for this header. This format is version 0 */
    uint32_t header_version;

    /** Version of the compatible hotword library. Note: int not uint */
    int32_t library_version;

    /** Length of the architecture string */
    uint32_t architecture_length;

    /** Architecture string. Only the first architecture_length bytes are valid */
    uint8_t architecture[16];

    /** Count of valid mmap_headers */
    uint32_t mmap_count;

    /** Array of headers for parsing the mmaps for a model */
    GSoundHotwordMmapHeader mmap_headers[MEMORY_BANK_CNT];
} GSoundHotwordModelHeader;

/**
 * This is a representation of the model stored in flash. It consists of a header 
 * followed by a byte array for each mmap. The header specifies any padding between
 * the mmaps.
 */
typedef struct
{
    GSoundHotwordModelHeader model_header;
    uint8_t model_data[__ARRAY_EMPTY];
}hw_model;

enum
{
    HW_DETECT_STATUS_IDLE = 0,
    HW_DETECT_STATUS_ONGOING = 1,
    HW_DETECT_STATUS_START_UPSTREAM = 2,
    HW_DETECT_STATUS_UPSTREAMING = 3,

    HW_DETECT_STATUS_NUM,
};

enum
{
    HW_BUF_STATUS_IDLE = 0,
    HW_BUF_STATUS_CACHING = 1,
    HW_BUF_STATUS_PROCESSING = 2,

    HW_BUF_STATUS_NUM,
};

enum
{
    HW_MIC_DATA_IDLE = 0,
    HW_MIC_DATA_CACHED = 1,

    HW_MIC_DATA_STATUS_NUM,
};

typedef struct
{
    uint8_t hotwordDetectStatus;

    uint8_t hotwordBufStatus;
    bool isEmptyLastTime;
    uint8_t *hotwordDataCacheBuf;
    CQueue hotwordDataCacheQueue;
    osMutexId hotwordDataCacheMutexId;

    // mark the incoming mic data(raw PCM data) info
    uint8_t micDataCacheIdx;
    uint8_t micDataProcessIdx;
    uint8_t micDataStatus[CP_PCM_DATA_CACHE_FRAME_NUM];
    uint8_t *micDataInputBuf[CP_PCM_DATA_CACHE_FRAME_NUM];
    uint32_t micDataInputLen[CP_PCM_DATA_CACHE_FRAME_NUM];
    uint8_t *micDataOutputBuf;

    uint32_t newAvailableDataLen;

    char libVersionStr[VERSION_STRING_MAX_LEN];
    char activeModelId[GSOUND_HOTWORD_MODEL_ID_BYTES];
    uint8_t *hotwordModelData;
    uint8_t *hotwordModelText;
    void *mHandle;
#ifdef VOICE_DETECTOR_SENS_EN
    uint32_t kwsHistoryPcmDataSize;
    uint32_t rawInputLenPerSession;
#endif    
    const GSoundHotwordInterface *hotwordInterface;
    GSoundTargetHotwordConfig hwConfig;

    int preambleMs;
    int maxPreambleMs;
} HW_EXTERNAL_CONTEXT_T;

/****************************function declearation**************************/
/**
 * @brief init external hotword
 * 
 */
void gsound_custom_hotword_external_init(void);

/**
 * @brief Init the hotword related buffer.
 * 
 */
void gsound_custom_hotword_external_init_buffer(void);

/**
 * @brief Deinit the hotword related buffer.
 * 
 */
void gsound_custom_hotword_external_deinit_buffer(void);

/**
 * @brief get the external hotword environment
 * 
 * @return void* pointer of external hotword environment
 */
void *gsound_custom_hotword_external_get_env(void);

/**
 * @brief init the external hotword process environment
 * 
 */
void gsound_custom_hotword_external_init_process(const char *model_id,
                                                 uint8_t model_length,bool reload);

/**
 * @brief processing mic data in co-processor
 * 
 * @param ptrBuf input data pointer
 * @param length input data length
 * @param outputPtr output data pointer
 * 
 */
void gsound_custom_hotword_external_handle_mic_data(uint8_t *ptrBuf, uint32_t length, uint8_t *outputPtr);

/**
 * @brief add mic data processing data into co-processor task
 * 
 * @return int result
 */
int gsound_custom_hotword_external_start_cp_process(void);

/**
 * @brief remove mic data processing data from co-processor task
 * 
 * @return int result
 */
int gsound_custom_hotword_external_stop_cp_process(void);

/**
 * @brief store hotword external interface and hotword external configurations comes from libgsound
 * and start/restart external hotword detection
 * 
 * @param audio_config pointer of audio settings
 * @param hw_config pointer of hotword external configurations come from libgsound
 */
void gsound_custom_hotword_external_start_detection(const GSoundTargetAudioInSettings *audio_config,
                                                    const GSoundTargetHotwordConfig *hw_config);

/**
 * @brief handler of libgsound data request
 * 
 * @param max_num_bytes max bytes to pass to libgsound
 * @param bytes_per_frame number of data per frame
 * @param sync_hw_data_out output hotword data info 
 * @param ready_now_out mark if hotword data is put in @see sync_hw_data_out
 */
void gsound_custom_hotword_external_request_data(uint32_t max_num_bytes,
                                                 uint32_t bytes_per_frame,
                                                 GSoundTargetHotwordDataMsg *sync_hw_data_out,
                                                 bool *ready_now_out);

/**
 * @brief callback of hotword data consumed by libgsound
 * 
 * @param hw_data consumed data info
 */
void gsound_custom_hotword_data_consumed(GSoundTargetHotwordDataMsg *hw_data);

/**
 * @brief stop stream data to libgsound
 * 
 * @return int result
 */
int gsound_custom_hotword_external_stop_stream(void);

/**
 * @brief stop external hotword detection
 * 
 * @param voiceReady @see GSoundTargetHotwordStopDetection to get more info
 */
void gsound_custom_hotword_external_stop_detection(bool voiceReady);

/**
 * @brief get hotword library info
 * 
 * @param info_out hotword library info
 *
 * @return int result
 */
int gsound_custom_hotword_get_lib_info(const GSoundHotwordLibInfo *const);

/**
 * @brief store active model info comes from libgsound
 * 
 * @param model_id active model ID string
 * @param model_length length of active model ID string
 */
void gsound_custom_hotword_external_store_active_model(const char *model_id,
                                                       uint8_t model_length);

/**
 * @brief store interface comes from libgsound
 * 
 * @param handlers pointer of libgsound handlers
 */
void gsound_custom_hotword_external_store_interface(const GSoundHotwordInterface *handlers);

/**
 * @brief processing raw data and cache the sbc-encoded mic data into hotword cache data buffer
 * 
 * @param rawDataBuf incoming raw data pointer
 * @param rawDataLen incoming raw data length
 * @param encodedDataBuf incoming sbc-encoded data pointer to cache
 * @param encodedDataLen incoming sbc-encoded data length to cache
 * @param upstreamFlag if upstream needed
 */
CP_TEXT_SRAM_LOC
void gsound_custom_hotword_external_process_data(uint8_t *rawDataBuf,
                                                 uint32_t rawDataLen,
                                                 uint8_t *encodedDataBuf,
                                                 uint32_t encodedDataLen,
                                                 bool upstreamFlag);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __GSOUND_CUSTOM_HOTWORD_EXTERNAL_H__ */