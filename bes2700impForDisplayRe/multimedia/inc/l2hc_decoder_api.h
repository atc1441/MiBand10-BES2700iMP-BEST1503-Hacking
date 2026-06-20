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
 * Description: l2hc_decoder_api.h
 * Notes: This file contains the definitions of structure and the declarations of functions in L2HC decoding algorithm.
 *        This file need to be included when using l2hc
 *        Structure List:
 *        Param Structure:                 -- AudioL2hcDecStruParam
 *        Data Structure:                -- AudioL2hcDecStruData
 *        Algorithm Version:                 -- AudioL2hcDecStruVersion
 *        Function List:
 *        AudioL2hcDecGetSize()      -- Obtaining the Memory Size of the Algorithm
 *        AudioL2hcDecInit()         -- Initializing 
 *        AudioL2hcDecSetParam()     -- Set algorithm parameters
 *        AudioL2hcDecGetParam()     -- Get algorithm parameters
 *        AudioL2hcDecApply()        -- Algorithm main processing function
 *        AudioL2hcDecGetVersion()   -- Get the algorithm version
*/
#ifndef __L2HC_DECODER_API_H__
#define __L2HC_DECODER_API_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ===================================================================
// the Return Codes and Macro Definitions
// ===================================================================
/* Parameter Macro Definitions */
#define AUDIO_L2HCDEC_FRAME_LEN_960 (960)        // input frame len, 960 points
#define AUDIO_L2HCDEC_FRAME_LEN_480 (480)        // input frame len, 480 points
#define AUDIO_L2HCDEC_FRAME_LEN_240 (240)        // input frame len, 240 points
#define AUDIO_L2HCDEC_CHANNEL_MONO (1)           // the number of channel，mono
#define AUDIO_L2HCDEC_CHANNEL_STEREO (2)         // the number of channel，stereo
#define AUDIO_L2HCDEC_SAMPLE_RATE_96000 (96000)  // input sample rate, 96000Hz
#define AUDIO_L2HCDEC_SAMPLE_RATE_88200 (88200)  // input sample rate, 88200Hz
#define AUDIO_L2HCDEC_SAMPLE_RATE_48000 (48000)  // input sample rate, 48000Hz
#define AUDIO_L2HCDEC_SAMPLE_RATE_44100 (44100)  // input sample rate, 44100Hz
#define AUDIO_L2HCDEC_BIT_DEPTH_16_FIX (16)      // input bit depth，16 bits，integer
#define AUDIO_L2HCDEC_BIT_DEPTH_24_FIX (24)      // input bit depth，24 bits，integer
#define AUDIO_L2HCDEC_BIT_DEPTH_32_FIX (32)      // input bit depth，32 bits，integer
#define AUDIO_L2HCDEC_BIT_DEPTH_32_FLOAT (-32)   // input bit depth，32 bits，floating-point
#define AUDIO_L2HCDEC_BITRATE_MONO_MAX   (960)       // the max value of target bitrate in mono
#define AUDIO_L2HCDEC_BITRATE_MONO_MIN   (64)        // the min value of target bitrate in mono
#define AUDIO_L2HCDEC_BITRATE_STEREO_MAX (1920)       // the max value of target bitrate in stereo
#define AUDIO_L2HCDEC_BITRATE_STEREO_MIN (128)        // the min value of target bitrate in stereo
#define AUDIO_L2HCDEC_BITS_BYTES_MAX     (2640)      // maximum number of bytes in bitstream

#define AUDIO_L2HCDEC_CHMODE_L           (0)         // dec left channel
#define AUDIO_L2HCDEC_CHMODE_R           (1)         // dec right channel
#define AUDIO_L2HCDEC_CHMODE_B           (2)         // dec both two channels

/* the Macro Definition of the Length of the Version String */
#define AUDIO_L2HCDEC_VERSION_LENGTH (48)  // length of the version string

/* the Macro Definition of Error Codes */
#define AUDIO_L2HCDEC_EOK (0)  // No error
/* AudioL2hcDecGetSize return codes */
#define AUDIO_L2HCDEC_GETSIZE_INV_CHANSIZE (-1)                // The channel size pointer is null
#define AUDIO_L2HCDEC_GETSIZE_4_BYTES_NOT_ALIGN_CHANSIZE (-2)  // The channel size pointer is not 4-byte aligned
/* AudioL2hcDecInit return codes */
#define AUDIO_L2HCDEC_INIT_INV_PHANDLE (-3)                // The handle is null
#define AUDIO_L2HCDEC_INIT_INV_CHANMEMBUF (-4)             // The channel memory buffer is null
#define AUDIO_L2HCDEC_INIT_4_BYTES_NOT_ALIGN_PHANDLE (-5)  // The handle is not 4-byte aligned
#define AUDIO_L2HCDEC_INIT_ERR_MEMSIZE (-6)                // The channel memory size is not enough
/* AudioL2hcDecSetParam return codes */
#define AUDIO_L2HCDEC_SETPARAM_INV_HANDLE (-7)                 // The handle is null
#define AUDIO_L2HCDEC_SETPARAM_8_BYTES_NOT_ALIGN_HANDLE (-8)   // The handle is not 8-byte aligned
#define AUDIO_L2HCDEC_SETPARAM_INV_PARAMS (-9)                 // The parameter memory pointer is null
#define AUDIO_L2HCDEC_SETPARAM_8_BYTES_NOT_ALIGN_PARAMS (-10)  // The parameter memory pointer is not 8-byte aligned
#define AUDIO_L2HCDEC_SETPARAM_UNINITIED (-11)                 // Wrong in calling sequence, not initialized
#define AUDIO_L2HCDEC_SETPARAM_ERR_PROTECTHEAD (-12)           // Wrong in protect head
#define AUDIO_L2HCDEC_SETPARAM_ERR_BITPERSAMPLE (-13)          // Input bit depth error
#define AUDIO_L2HCDEC_STEREOMODESET_ERR_CHMODE (-38)           // The decoding mode is incorrect
/* AudioL2hcDecGetParam return codes */
#define AUDIO_L2HCDEC_GETPARAM_INV_HANDLE (-14)                // The handle is null
#define AUDIO_L2HCDEC_GETPARAM_8_BYTES_NOT_ALIGN_HANDLE (-15)  // The handle is not 8-byte aligned
#define AUDIO_L2HCDEC_GETPARAM_INV_PARAMS (-16)                // The parameter memory pointer is null
#define AUDIO_L2HCDEC_GETPARAM_8_BYTES_NOT_ALIGN_PARAMS (-17)  // The parameter memory pointer is not 8-byte aligned
#define AUDIO_L2HCDEC_GETPARAM_UNINITIED (-18)                 // Wrong in calling sequence, not initialized
#define AUDIO_L2HCDEC_GETPARAM_ERR_PROTECTHEAD (-19)           // Wrong in protect head
/* AudioL2hcDecApply return codes */
#define AUDIO_L2HCDEC_APPLY_INV_HANDLE (-20)                          // The handle is null
#define AUDIO_L2HCDEC_APPLY_8_BYTES_NOT_ALIGN_HANDLE (-21)            // The handle is not 8-byte aligned
#define AUDIO_L2HCDEC_APPLY_UNINITIED_HANDLE (-22)                    // The handle is not initialized
#define AUDIO_L2HCDEC_APPLY_ERR_PROTECTHEAD (-23)                     // The protect head is overwritten
#define AUDIO_L2HCDEC_APPLY_INV_DATA (-24)                            // The data structure pointer is null
#define AUDIO_L2HCDEC_APPLY_8_BYTES_NOT_ALIGN_DATA (-25)              // The address of the data structure is not aligned
#define AUDIO_L2HCDEC_APPLY_INV_DATA_BITSTREAMIN (-26)                // The data input pointer is null
#define AUDIO_L2HCDEC_APPLY_8_BYTES_NOT_ALIGN_DATA_BITSTREAMIN (-27)  // The data input is not 8-byte aligned
#define AUDIO_L2HCDEC_APPLY_ERR_DATA_BITSTREAMINSIZEBYTE (-28)        // Invalid data input size
#define AUDIO_L2HCDEC_APPLY_INV_DATA_AUDIOOUT (-29)                   // The output data pointer is null
#define AUDIO_L2HCDEC_APPLY_8_BYTES_NOT_ALIGN_DATA_AUDIOOUT (-30)  // The output is not 8-byte aligned
#define AUDIO_L2HCDEC_APPLY_ERR_BITPERSAMPLE (-31)                 // Input bit depth error
#define AUDIO_L2HCDEC_APPLY_ERR_SMPRATE (-32)                      // Output sampling rate is incorrect
#define AUDIO_L2HCDEC_APPLY_ERR_CHANNELS (-33)                     // The number of output channels is incorrect
#define AUDIO_L2HCDEC_APPLY_ERR_FRLENGTH (-34)                     // Output frame length is incorrect
#define AUDIO_L2HCDEC_APPLY_ERR_CODECTYPE (-35)                    // The codec type is incorrect
/* AudioL2hcDecGetVersion() return codes */
#define AUDIO_L2HCDEC_VERSION_INV_PVERSION (-36)                // The version structure is empty
#define AUDIO_L2HCDEC_VERSION_4_BYTES_NOT_ALIGN_PVERSION (-37)  // The version structure is not 4-byte aligned

// Note: For decoder, the user can only set the output bit depth. The sampling rate, number of audio channels, and frame length can only be read from bitstreams and cannot be set separately
typedef struct {
    // smpRate: sample rate;valid range:{44100,48000,88200,96000};default:96000;reserved;
    int32_t smpRate;
    // bitPerSample
    // bit depth;valid range:{16,24,32,-32};default:24;error code:-13;note:-32 stand for float;
    int16_t bitPerSample;
    // channels: the number of input 's channel;valid range:{1,2};default:2;reserved;
    int16_t channels;
    // frLength: frame length;valid range:{480,960},
    // when sample rate is 44100/48000, frLength could be 240/480，when sample rate is 88200/96000, frLength could be 480/960;default:960;reserved;
    int16_t frLength;
    // chMode dec mode when input is stereo;valid range:{0, 1, 2};default:2；error code：-38；Only with the stereo input it is valid
    uint16_t chMode;
} AudioL2hcDecStruParam;

typedef struct {
    // bitstreamIn: input bitstream，need 8-byte aligned
    uint8_t *bitstreamIn;
    // bitstreamInSizeByte: Number of bytes in the bitstream
    uint32_t bitstreamInSizeByte;
    // audioOut: output pcm data，need 8-byte aligned, In the case of stereo-channel audio, the LRs need to be arranged alternately
    uint8_t *audioOut;
    // audioOutSizeByte: size of an output frame of PCM data，unit: byte, it should be equals to frLength * channels * bitPerSample
    uint32_t audioOutSizeByte;
} AudioL2hcDecStruData;

typedef struct {
    // algorithm version info string
    int8_t l2hcVersionChar[AUDIO_L2HCDEC_VERSION_LENGTH];
} AudioL2hcDecStruVersion;

// ============================================================
// Function Name: AudioL2hcDecGetSize
// Description: get channel size
// Input Argument: chanSize -- channel size，correspond to chanMemBuf and memSize，to be assigned
// Output Argument: chanSize -- channel size，correspond to chanMemBuf and memSize，assigned
// Return Argument: ret -- return code, AUDIO_L2HC_EOK stand for success，other return codes indicate failure
// =============================================================
int32_t AudioL2hcDecGetSize(uint32_t *chanSize);

// =========================================================================
// Function Name: AudioL2hcDecInit
// Description: initializes an algorithm instance (channel) and returns its handle
// Input Argument: pHandle        -- handle
//          chanMemBuf     -- memory first address
//          memSize        -- the size of chanMemBuff
// Output Argument: pHandle        -- object handle
// Return Argument: ret  -- return code AUDIO_L2HC_EOK stand for success，other return codes indicate failure
// =========================================================================
int32_t AudioL2hcDecInit(uintptr_t **pHandle, uintptr_t *chanMemBuf, uint32_t memSize);

// ============================================================
// Function Name: AudioL2hcDecSetParam
// Description: set parameters of decoder
// Input Argument: handle   -- handle
//          params   -- pointer to the structure with decoding parameters
// Output Argument: handle   -- handle after parameter updates
// Return Argument: ret -- return code AUDIO_L2HC_EOK stand for success，other return codes indicate failure
// =============================================================
int32_t AudioL2hcDecSetParam(uintptr_t *handle, AudioL2hcDecStruParam *params);

// ============================================================
// Function Name: AudioL2hcDecGetParam
// Description: get parameters of decoder
// Input Argument: handle   -- handle
//          params   -- pointer to the parameter structure that is not assigned
// Output Argument: params   -- pointer to the parameter structure that is assigned
// Return Argument: ret -- return code  AUDIO_L2HC_EOK stand for success，other return codes indicate failure
// =============================================================
int32_t AudioL2hcDecGetParam(uintptr_t *handle, AudioL2hcDecStruParam *params);

// =========================================================================
// Function Name: AudioL2hcDecApply
// Description: decode and return PCM data
// Input Argument: handle               -- handle
//          data                 -- pointer to the input data structure
// Output Argument: data                 -- pointer to the output data structure
// Return Argument: ret -- return code AUDIO_L2HC_EOK stand for success，other return codes indicate failure
// =========================================================================
int32_t AudioL2hcDecApply(uintptr_t *handle, AudioL2hcDecStruData *data);

// ============================================================
// Function Name: AudioL2hcDecGetParam
// Description: get the version information of the algorithm
// Input Argument: pVersion   -- pointer to the version structure that is not assigned
// Output Argument: pVersion   -- pointer to the version structure to which is assigned
// Return Argument: ret -- return code AUDIO_L2HC_EOK stand for success，other return codes indicate failure
// =============================================================
int32_t AudioL2hcDecGetVersion(AudioL2hcDecStruVersion *pVersion);

#ifdef __cplusplus
}
#endif

#endif
