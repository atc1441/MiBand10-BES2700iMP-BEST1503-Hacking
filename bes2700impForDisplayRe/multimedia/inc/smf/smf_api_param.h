#pragma once
#ifndef __SMF_API_PARAM_H__
#define __SMF_API_PARAM_H__
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "smf_media.h"
//
#if 0
typedef struct {
	uint32_t params[6];
}smf_open_param_t;
//
typedef struct {	
	smf_media_t* media;
}smf_source_param_t;
//
typedef struct {
	smf_media_t media;
}smf_sink_param_t;
//
typedef struct {
	smf_media_t media;
}smf_filter_param_t;
//
typedef struct {
	smf_media_t media;
}smf_encode_param_t;
//
typedef struct {
	smf_media_t media;
}smf_decode_param_t;
#endif
//
typedef struct {
	const char* url;
	const char* para;
	void* other;
}smf_io_param_t;
//
typedef struct {
	smf_io_param_t file;
	char* buff;
	int size;
}smf_io_buff_param_t;
//
typedef struct {
	smf_io_param_t file;
	void* handle;
	void*(*open)(void*);
	bool(*close)(void* hd);
	bool(*seek)(void* hd, int offset, int pos);
	unsigned(*getsize)(void* hd);
	unsigned(*offset)(void* hd);
	unsigned(*read)(void* hd, void* buff, unsigned size);
	unsigned(*write)(void* hd, void* buff, unsigned size);
}smf_io_callback_param_t;
#endif
