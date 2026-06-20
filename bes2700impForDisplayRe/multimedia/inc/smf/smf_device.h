#pragma once
#ifndef __SMF_DEVICE_H__
#define __SMF_DEVICE_H__
#include "smf_common.h"
//enum CODEC_TYPE {
//    CODEC_USE_INTCODEC = 0,
//    CODEC_USE_I2S = 1
//};
//typedef struct smf_microphone_open_param_t {
//	smf_source_param_t media;
//}smf_microphone_open_param_t;
//
//typedef struct smf_speaker_open_param_t {
//	smf_sink_param_t media;
//}smf_speaker_open_param_t;

/**
 * register speaker
 */
//EXTERNC void smf_speaker_register(void);

/**
 * register microphone
 */

//EXTERNC void smf_microphone_register(void);


//enum SMF_DMA_DEVICE_ID {
//    SMF_DMA_DEVICE_CODEC = 0xa0,
//    SMF_DMA_DEVICE_I2S0,
//    SMF_DMA_DEVICE_I2S1,
//    SMF_DMA_DEVICE_BT
//};
//EXTERNC void smf_i2s_source_register(void);
//EXTERNC void smf_i2s_sink_register(void);
//EXTERNC void smf_speakerX_register(void);
//EXTERNC void smf_microphoneN_register(void);
//EXTERNC void smf_ivX_source_register(void);
//EXTERNC void smf_speaker_register(void);
//EXTERNC void smf_microphone_register(void);
//EXTERNC void smf_iv_source_register(void);

EXTERNC void smf_dma_source_register(void);
EXTERNC void smf_dma_sink_register(void);
#endif