#pragma once
#ifndef __SMF_CODEC_SBC_H__
#define __SMF_CODEC_SBC_H__

#include "smf_api.h"
#include "smf_media_def.h"


typedef struct {
	uint32_t _bitrate;
	uint8_t _syncWord;//smf_sbc_syncwords_e
	uint8_t _channelMode;//sbc_channel_mode_e
	uint8_t _numSubBands;//sbc_subbands_e
	uint8_t _numBlocks;//sbc_blocks_e
	uint8_t _allocMethod;//sbc_alloc_method_e
	uint8_t _bitPool;
	uint8_t _sbcMode;//SbcMode_e
}smf_media_sbc_def_t;

typedef struct {
	smf_media_def_t head;
	smf_media_audio_def_t audio;
	smf_media_sbc_def_t sbc;
}smf_media_sbc_t;

enum smf_sbc_syncwords_e {
	SMF_SBC_SYNCWORDS_SBC = 0x9C,
	SMF_SBC_SYNCWORDS_MSBC = 0xAD,
	SMF_SBC_SYNCWORDS_GSBC = 0x8E,
	SMF_SBC_SYNCWORDS_ASBC = 0x7B,
};
enum smf_sbc_sample_rate_e{
	SMF_SBC_SAMPLE_RATE_16K=16000,
	SMF_SBC_SAMPLE_RATE_32K=32000,
	SMF_SBC_SAMPLE_RATE_44K1=44100,
	SMF_SBC_SAMPLE_RATE_48K=48000,

	SMF_MSBC_SAMPLE_RATE_16K=16000,
	SMF_GSBC_SAMPLE_RATE_48K=48000,

	SMF_ASBC_SAMPLE_RATE_32K = 32000,
	SMF_ASBC_SAMPLE_RATE_48K = 48000,
	SMF_ASBC_SAMPLE_RATE_88K2 = 88200,
	SMF_ASBC_SAMPLE_RATE_96K = 96000,
};
enum smf_sbc_alloc_method_e {
	SMF_SBC_ALLOC_METHOD_LOUDNESS=0,
	SMF_SBC_ALLOC_METHOD_SNR=1,
};
enum smf_sbc_channel_mode_e {
	SMF_SBC_CHNL_MODE_MONO          =0,
	SMF_SBC_CHNL_MODE_DUAL_CHNL     =1,
	SMF_SBC_CHNL_MODE_STEREO        =2,
	SMF_SBC_CHNL_MODE_JOINT_STEREO  =3,
};
enum smf_sbc_blocks_e{
	SMF_SBC_BLOCKS_4=4,
	SMF_SBC_BLOCKS_8=8,
	SMF_SBC_BLOCKS_12=12,
	SMF_SBC_BLOCKS_16=16,

	SMF_MSBC_BLOCKS_15=15,
	SMF_GSBC_BLOCKS_15=15,

	SMF_ASBC_BLOCKS_15 = 15,
	SMF_ASBC_BLOCKS_20 = 20,
	SMF_ASBC_BLOCKS_30 = 30,
	SMF_ASBC_BLOCKS_60 = 60,
};
enum smf_sbc_subbands_e{
	SMF_SBC_SUBBANDS_4=4,
	SMF_SBC_SUBBANDS_8=8,
};
enum smf_sbc_package_e{
	SMF_SBC_PACKAGE_NORMAL=0,
	SMF_SBC_PACKAGE_A2DP,
};

enum SbcMode_e {
	SMF_SBC_MODE_NONE = 0x00,
	SMF_SBC_MODE_NORMAL = 0x9C,
	SMF_SBC_MODE_MSBC = 0xAD,
	SMF_SBC_MODE_GSBC = 0x8E,
	SMF_SBC_MODE_ASBC = 0x7B,
};
typedef struct{
	smf_media_sbc_t media;
}smf_sbc_enc_open_param_t;

typedef struct {
	smf_media_sbc_t media;
}smf_sbc_dec_open_param_t;
///

/**
 * register sbc decoder
 */
EXTERNC void smf_sbc_decoder_register(void);

/**
 * register sbc encoder
 */
EXTERNC void smf_sbc_encoder_register(void);
//
enum{
	SMF_SBC_FRAME_FLAGS_MUTE_FRAME = 1<<1,
};
#endif
