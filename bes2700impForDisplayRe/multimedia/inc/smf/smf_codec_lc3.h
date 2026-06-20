#pragma once
#ifndef __SMF_CODEC_LC3_H__
#define __SMF_CODEC_LC3_H__

#include "smf_api.h"
#include "smf_media_def.h"
typedef struct {
	uint8_t _sampleBits;
	uint8_t _sampleWidth;
	bool _isNonInterlace;
	bool _isFloat;
	uint32_t _bitrate;
	uint32_t _bandWidth;
	uint8_t _frameDMs;
	uint8_t _epmode;
	uint8_t _epmr;
	uint8_t _hrmode;
	uint8_t _lfemode;
	uint8_t _plcMeth;
	uint16_t _frameSize;
	uint32_t _quanPrec;
	uint32_t _ltpfFlag;
	void* _other;
}smf_media_lc3_def_t;

typedef struct {
	smf_media_def_t head;
	smf_media_audio_def_t audio;
	smf_media_lc3_def_t lc3;
}smf_media_lc3_t;

/*! Decoder packet loss concealment mode */
enum lc3_plcmode_e
{
	SMF_LC3_PLCMODE_STANDARD = 0, /*!< Less complex than advanced method */
	SMF_LC3_PLCMODE_ADVANCED = 1  /*!< Enhanced concealment method */
};

/*! Error protection mode. LC3_EP_ZERO differs to LC3_EP_OFF in that
 *  errors can be detected but not corrected. */
enum lc3_epmode_e
{
	SMF_LC3_EPMODE_OFF = 0, /*!< Error protection is disabled */
	SMF_LC3_EPMODE_ZERO = 1, /*!< Error protection with 0 bit correction */
	SMF_LC3_EPMODE_LOW = 2, /*!< Error protection correcting one symbol per codeword */
	SMF_LC3_EPMODE_MEDIUM = 3, /*!< Error protection correcting two symbols per codeword */
	SMF_LC3_EPMODE_HIGH = 4  /*!< Error protection correcting three symbols per codeword */
};

enum lc3_epmr_e
{
	SMF_LC3_EPMR_ZERO = 0, /*!< Request no error correction. High confidence if returned by decoder. */
	SMF_LC3_EPMR_LOW = 1, /*!< Request low error correction. High confidence if returned by decoder. */
	SMF_LC3_EPMR_MEDIUM = 2, /*!< Request medium error correction. High confidence if returned by decoder. */
	SMF_LC3_EPMR_HIGH = 3, /*!< Request high error correction. High confidence if returned by decoder. */
	SMF_LC3_EPMR_ZERO_MC = 4, /*!< No error correction requested, medium confidence. */
	SMF_LC3_EPMR_LOW_MC = 5, /*!< Low error correction requested, medium confidence. */
	SMF_LC3_EPMR_MEDIUM_MC = 6, /*!< Medium error correction requested, medium confidence. */
	SMF_LC3_EPMR_HIGH_MC = 7, /*!< High error correction requested, medium confidence. */
	SMF_LC3_EPMR_ZERO_NC = 8, /*!< No error correction requested, unvalidated. */
	SMF_LC3_EPMR_LOW_NC = 9, /*!< Low error correction requested, unvalidated. */
	SMF_LC3_EPMR_MEDIUM_NC = 10, /*!< Medium error correction requested, unvalidated. */
	SMF_LC3_EPMR_HIGH_NC = 11  /*!< High error correction requested, unvalidated. */
};

typedef struct {
	//smf_encode_param_t
	smf_media_lc3_t media;
}smf_lc3_enc_open_param_t;

typedef struct {
	smf_media_lc3_t media;
}smf_lc3_dec_open_param_t;

typedef struct {
	uint64_t head_begin;//0x55aaff00
	uint16_t head_size;
	uint16_t is_float;
	uint32_t sample_rate;
	//
	uint8_t channels;
	uint8_t sample_bits;
	uint16_t frame_size;	
	//
	uint8_t frame_dms;
	uint8_t epmode;
	uint8_t hrmode;
	uint8_t lfemode;
	//
	uint32_t head_end;//0x55aaffff
}smf_lc3_param_header_t;

typedef struct {
	uint8_t sync;//ff
	//
	uint8_t crc:4;
	uint8_t sample_rate:4;
	//
	uint8_t channels:2;
	uint8_t frame_dms : 2;
	uint8_t frame_size_0 : 4;
	//
	uint8_t frame_size_1:8;
}smf_lc3_frame_header_t;
/**
 * register lc3 encoder
 */
EXTERNC void smf_lc3_encoder_register(void);
EXTERNC void smf_lc3plus_encoder_register(void);

/**
 * register lc3 decoder
 */
EXTERNC void smf_lc3_decoder_register();
EXTERNC void smf_lc3plus_decoder_register(void);
#endif
