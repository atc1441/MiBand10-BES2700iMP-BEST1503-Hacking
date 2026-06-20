/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
/*
 * Description: l2hc_encoder_api.h
 * Notes: This file contains the definitions of structure and the declarations of functions in L2HC encoding algorithm.
 *        This file need to be included when using l2hc
 *        Structure List:
 *        Param Structure:                -- AudioL2hcEncStruParam
 *        Data Structure:                 -- AudioL2hcEncStruData
 *        Algorithm Version:              -- AudioL2hcEncStruVersion
 *        Function List:
 *        AudioL2hcEncGetSize()      -- Obtaining the Memory Size of the Algorithm
 *        AudioL2hcEncInit()         -- Initializing
 *        AudioL2hcEncSetParam()     -- Set algorithm parameters
 *        AudioL2hcEncGetParam()     -- Get algorithm parameters
 *        AudioL2hcEncApply()        -- Algorithm main processing function
 *        AudioL2hcEncGetVersion()   -- Get the algorithm version
 */
#ifndef __L2HC_ENCOCDER_API_H__
#define __L2HC_ENCOCDER_API_H__
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

/*
 * the Return Codes and Macro Definitions
 */
/* Parameter Macro Definitions */
#define AUDIO_L2HCENC_FRAME_LEN_960      (960)       // input frame len, 960 points
#define AUDIO_L2HCENC_FRAME_LEN_480      (480)       // input frame len, 480 points
#define AUDIO_L2HCENC_FRAME_LEN_240      (240)       // input frame len, 240 points
#define AUDIO_L2HCENC_CHANNEL_MONO       (1)         // the number of channel，mono
#define AUDIO_L2HCENC_CHANNEL_STEREO     (2)         // the number of channel，stereo
#define AUDIO_L2HCENC_SAMPLE_RATE_96000  (96000)     // input sample rate, 96000Hz
#define AUDIO_L2HCENC_SAMPLE_RATE_88200  (88200)     // input sample rate, 88200Hz
#define AUDIO_L2HCENC_SAMPLE_RATE_48000  (48000)     // input sample rate, 48000Hz
#define AUDIO_L2HCENC_SAMPLE_RATE_44100  (44100)     // input sample rate, 44100Hz
#define AUDIO_L2HCENC_BIT_DEPTH_16_FIX   (16)        // input bit depth，16 bits，integer
#define AUDIO_L2HCENC_BIT_DEPTH_24_FIX   (24)        // input bit depth，24 bits，integer
#define AUDIO_L2HCENC_BIT_DEPTH_32_FIX   (32)        // input bit depth，32 bits，integer
#define AUDIO_L2HCENC_BIT_DEPTH_32_FLOAT (-32)       // input bit depth，32 bits，floating-point
#define AUDIO_L2HCENC_BITRATE_MONO_MAX   (960)       // the max value of target bitrate in mono
#define AUDIO_L2HCENC_BITRATE_MONO_MIN   (64)        // the min value of target bitrate in mono
#define AUDIO_L2HCENC_BITRATE_STEREO_MAX (1920)       // the max value of target bitrate in stereo
#define AUDIO_L2HCENC_BITRATE_STEREO_MIN (128)        // the min value of target bitrate in stereo
#define AUDIO_L2HCENC_BITS_BYTES_MAX     (2640)      // maximum number of bytes in bitstream

/* the Macro Definition of the Length of the Version String */
#define AUDIO_L2HCENC_VERSION_LENGTH     (48)        // length of the version string

/* the Macro Definition of Error Codes */
#define AUDIO_L2HCENC_EOK                                       (0)   // No error
/* AudioL2hcEncGetSize return codes */
#define AUDIO_L2HCENC_GETSIZE_INV_CHANSIZE                      (-1)  // The channel size pointer is null
#define AUDIO_L2HCENC_GETSIZE_4_BYTES_NOT_ALIGN_CHANSIZE        (-2)  // The channel size pointer is not 4-byte aligned
/* AudioL2hcEncInit return codes */
#define AUDIO_L2HCENC_INIT_INV_PHANDLE                          (-3)  // The handle is null
#define AUDIO_L2HCENC_INIT_INV_CHANMEMBUF                       (-4)  // The channel memory buffer is null
#define AUDIO_L2HCENC_INIT_4_BYTES_NOT_ALIGN_PHANDLE            (-5)  // The handle is not 4-byte aligned
#define AUDIO_L2HCENC_INIT_ERR_MEMSIZE                          (-6)  // The channel memory size is not enough
/* AudioL2hcEncSetParam return codes */
#define AUDIO_L2HCENC_SETPARAM_INV_HANDLE                       (-7)  // The handle is null
#define AUDIO_L2HCENC_SETPARAM_8_BYTES_NOT_ALIGN_HANDLE         (-8)  // The handle is not 8-byte aligned
#define AUDIO_L2HCENC_SETPARAM_INV_PARAMS                       (-9)  // The parameter memory pointer is null
#define AUDIO_L2HCENC_SETPARAM_8_BYTES_NOT_ALIGN_PARAMS         (-10) // The parameter memory pointer is not 8-byte aligned
#define AUDIO_L2HCENC_SETPARAM_UNINITIED                        (-11) // Wrong in calling sequence, not initialized
#define AUDIO_L2HCENC_SETPARAM_ERR_PROTECTHEAD                  (-12) // Wrong in protect head
#define AUDIO_L2HCENC_SETPARAM_ERR_SMPRATE                      (-13) // Wrong in sample rate
#define AUDIO_L2HCENC_SETRARAM_ERR_BITPERSAMPLE                 (-14) // Input bit depth error
#define AUDIO_L2HCENC_SETPARAM_ERR_CHANNELS                     (-15) // Input channel number error
#define AUDIO_L2HCENC_SETPARAM_ERR_FRLENGTH                     (-16) // Input frame len error
#define AUDIO_L2HCENC_SETPARAM_ERR_BITRATE                      (-17) // Input bitrate error
/* AudioL2hcEncGetParam return codes */
#define AUDIO_L2HCENC_GETPARAM_INV_HANDLE                       (-18) // The handle is null
#define AUDIO_L2HCENC_GETPARAM_8_BYTES_NOT_ALIGN_HANDLE         (-19) // The handle is not 8-byte aligned
#define AUDIO_L2HCENC_GETPARAM_INV_PARAMS                       (-20) // The parameter memory pointer is null
#define AUDIO_L2HCENC_GETPARAM_8_BYTES_NOT_ALIGN_PARAMS         (-21) // The parameter memory pointer is not 8-byte aligned
#define AUDIO_L2HCENC_GETPARAM_UNINITIED                        (-22) // Wrong in calling sequence, not initialized
#define AUDIO_L2HCENC_GETPARAM_ERR_PROTECTHEAD                  (-23) // Wrong in protect head
/* AudioL2hcEncApply return codes */
#define AUDIO_L2HCENC_APPLY_INV_HANDLE                          (-24) // The handle is null
#define AUDIO_L2HCENC_APPLY_8_BYTES_NOT_ALIGN_HANDLE            (-25) // The handle is not 8-byte aligned
#define AUDIO_L2HCENC_APPLY_UNINITIED_HANDLE                    (-26) // The handle is not initialized
#define AUDIO_L2HCENC_APPLY_ERR_PROTECTHEAD                     (-27) // The protect head is overwritten
#define AUDIO_L2HCENC_APPLY_INV_DATA                            (-28) // The data structure pointer is null
#define AUDIO_L2HCENC_APPLY_8_BYTES_NOT_ALIGN_DATA              (-29) // The address of the data structure is not aligned
#define AUDIO_L2HCENC_APPLY_INV_DATA_AUDIOIN                    (-30) // The data input pointer is null
#define AUDIO_L2HCENC_APPLY_8_BYTES_NOT_ALIGN_DATA_AUDIOIN      (-31) // The data input is not 8-byte aligned
#define AUDIO_L2HCENC_APPLY_ERR_DATA_AUDIOINSIZEBYTE            (-32) // Invalid data input size
#define AUDIO_L2HCENC_APPLY_INV_DATA_BITSTREAMOUT               (-33) // The output data pointer is null
#define AUDIO_L2HCENC_APPLY_8_BYTES_NOT_ALIGN_DATA_BITSTREAMOUT (-34) // The output is not 8-byte aligned
#define AUDIO_L2HCENC_APPLY_ERR_SMPRATE	                        (-35) // Input sample rate error
#define AUDIO_L2HCENC_APPLY_ERR_BITPERSAMPLE                    (-36) // Input bit depth error
#define AUDIO_L2HCENC_APPLY_ERR_CHANNELS                        (-37) // Input channel number error
#define AUDIO_L2HCENC_APPLY_ERR_FRLENGTH                        (-38) // Input frame len error
#define AUDIO_L2HCENC_APPLY_ERR_BITRATE	                        (-39) // Input bitrate error
#define AUDIO_L2HCENC_APPLY_ERR_CODECTYPE                       (-40) // The codec type is incorrect
/* AudioL2hcEncGetVersion() return codes */
#define AUDIO_L2HCENC_VERSION_INV_PVERSION                      (-41) // The version structure is empty
#define AUDIO_L2HCENC_VERSION_4_BYTES_NOT_ALIGN_PVERSION        (-42) // The version structure is not 4-byte aligned

typedef struct {
    // smpRate: sample rate;valid range:{44100,48000,88200,96000};default:96000;error code:-13;unit:Hz;
    int32_t smpRate;
    // bitPerSample: bit depth;valid range:{16,24,32,-32};default:24;error code:-14;note:-32 stand for float;
    int16_t bitPerSample;
    // channels:  the number of input 's channel;valid range:{1,2};default:2;error code:-15;
    int16_t channels;
    // frLength: frame length;valid range:{240,480,960};default:960;error code:-16;
    // when sample rate is 44100/48000, frLength could be 240/480，when sample rate is 88200/96000, frLength could be 480/960
    int16_t frLength;
    // bitRate: target bitrate;valid range:mono[64, 960];stereo[128, 1920];default:500;error code:-17;unit:kbps;
    int16_t bitRate;
} AudioL2hcEncStruParam;

typedef struct {
    // audioIn: input pcm data，need 8-byte aligned, In the case of stereo-channel audio, the LRs need to be arranged alternately
    uint8_t *audioIn;
    // audioInSizeByte: the size of input of a frame，unit: byte。it should be equal to frLength * channels * bitPerSample / 8
    uint32_t audioInSizeByte;
    // bitstreamOut: output bitstream，need 8-byte aligned
    uint8_t *bitstreamOut;
    // bitstreamOutSizeByte: Number of bytes in the output bitstream
    uint32_t bitstreamOutSizeByte;
} AudioL2hcEncStruData;

typedef struct {
    // algorithm version info string
    int8_t l2hcVersionChar[AUDIO_L2HCENC_VERSION_LENGTH];
} AudioL2hcEncStruVersion;

/*
 * Description: get channel size
 * Input Argument:
 * chanSize -- channel size，pointer can not be null。correspond to AudioL2hcEncInit's chanMemBuf and memSize，to be assigned
 * Output Argument:
 * chanSize -- channel size，pointer can not be null。correspond to AudioL2hcEncInit's chanMemBuf and memSize，assigned
 * Return Argument:
 * ret -- return code AUDIO_L2HC_EOK stand for success，other return codes indicate failure
 */
int32_t AudioL2hcEncGetSize(uint32_t *chanSize);

/*
 * Description: initializes an algorithm instance (channel) and returns its handle
 * Input Argument:
 * pHandle    -- handle
 * chanMemBuf -- memory first address
 * memSize    -- chanMemBuf length
 * Output Argument:
 * pHandle    -- object handle
 * Return Argument:
 * ret -- return code AUDIO_L2HC_EOK stand for success，other return codes indicate failure
 */
int32_t AudioL2hcEncInit(uintptr_t **pHandle, uintptr_t *chanMemBuf, uint32_t memSize);

/*
 * Description: set parameters of enocder
 * Input Argument:
 * handle   -- handle
 * params   -- pointer to the structure with encoding parameters
 * Output Argument:
 * handle   -- handle after parameter updates
 * Return Argument:
 * ret -- return code AUDIO_L2HC_EOK stand for success，other return codes indicate failure
 */
int32_t AudioL2hcEncSetParam(uintptr_t *handle, AudioL2hcEncStruParam *params);

/*
 * Description: get parameters of enocder
 * Input Argument:
 * handle   -- handle
 * params   -- pointer to the parameter structure that is not assigned
 * Output Argument:
 * params   -- pointer to the parameter structure that is assigned
 * Return Argument:
 * ret -- return code AUDIO_L2HC_EOK stand for success，other return codes indicate failure
 */
int32_t AudioL2hcEncGetParam(uintptr_t *handle, AudioL2hcEncStruParam *params);

/*
 * Description: encode and return bitstream
 * Input Argument:
 * handle    -- handle
 * data      -- pointer to the input data structure
 * Output Argument:
 * data      -- pointer to the output data structure
 * Return Argument:
 * ret -- return code AUDIO_L2HC_EOK stand for success，other return codes indicate failure
 */
int32_t AudioL2hcEncApply(uintptr_t *handle, AudioL2hcEncStruData *data);

/*
 * Description: get the version information of the algorithm
 * Input Argument:
 * pVersion -- pointer to the version structure that is not assigned
 * Output Argument:
 * pVersion -- pointer to the version structure to which is assigned
 * Return Argument:
 * ret -- return code AUDIO_L2HC_EOK stand for success，other return codes indicate failure
 */
int32_t AudioL2hcEncGetVersion(AudioL2hcEncStruVersion *pVersion);

#ifdef __cplusplus
}
#endif

#endif
