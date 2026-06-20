#pragma once
#ifndef __SMF_DEMUXER_H__
#define __SMF_DEMUXER_H__

#include "smf_api.h"
#include "smf_media.h"
typedef struct {
	bool estimated;
	bool unsupported;
	int bitrate_max_kbps;
	int bitrate_avg_kbps;
	int demux_cpu_max_mips;
	int demux_cpu_avg_mips;
	int demux_memory_max_kbytes;
	int demux_memory_avg_kbytes;
	int decode_cpu_max_mips;
	int decode_cpu_avg_mips;
	int decode_memory_max_kbytes;
	int decode_memory_avg_kbytes;
}smf_demuxer_estimate_t;
//
typedef struct {
	smf_media_t* media;
	smf_progress_t* progress;
	///
	smf_demuxer_estimate_t* estimate;
	smf_meta_info_t* meta;
	///
	const char* url;
	int startPostion;
	void* other;
	///
	void* io;
	int ioReadCacheLen;
	int ioReadAlign;
	bool packed;
}smf_iosource_param_t, smf_demuxer_param_t;

#endif
