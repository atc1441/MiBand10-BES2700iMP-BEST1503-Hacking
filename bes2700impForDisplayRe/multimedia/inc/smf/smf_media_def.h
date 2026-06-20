#pragma once
#ifndef __SMF_MEDIA_DEF_H__
#define __SMF_MEDIA_DEF_H__
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
typedef struct {
	uint16_t _mediaExtSize;
}smf_media_ext_def_t;

//
typedef struct {
	uint32_t* vtable;
	uint32_t _size;
	uint64_t _codec;
	uint32_t _mflags;
	smf_media_ext_def_t* _ext;
	uint32_t _extraSize;
	uint8_t* _extraData;
}smf_media_def_t;
//
typedef struct {
	uint32_t _size;
	uint32_t _codec;//not used uint64_t
	uint32_t _codec_high;//not used uint64_t
	uint32_t _mflags;
	smf_media_ext_def_t* _ext;
	uint32_t _extraSize;
	uint8_t* _extraData;
}smf_media32_def_t;
//
typedef struct {
	uint32_t _rate; 
	uint8_t _channels; 
	uint8_t _rev; 
	uint16_t _frameSamples; 
}smf_media_audio_def_t;
typedef struct {
	uint16_t _volumeTarget;//q15
	uint16_t _volumeChangeStep;
	uint8_t _volumeWeakenOthers;//0%~100%
	union {
		uint8_t _audioExtFlags;
		struct {
			bool _mute : 1;
			bool _prompt : 1;
			bool _fadeout : 1;
		};
	};
}smf_media_audio_ext_def_t;
//
typedef struct {
	uint16_t _width;
	uint16_t _height;
	uint16_t _frameRate;
	uint16_t _frameRateDenum;
	uint16_t _roiX, _roiY, _roiW, _roiH;//Region of Interest
}smf_media_video_def_t;
typedef struct {
	uint8_t _format;//format_e
	uint8_t _pixelBits;//8,16,24,32
}smf_media_video_raw_def_t;
//
typedef enum {
	SMF_MEDIA_IS_CHANGED = 1U << 0
}smf_media_mflags_e;
#endif

