#pragma once
#ifndef __SMF_FILTER_RESAMPLES_H__
#define __SMF_FILTER_RESAMPLES_H__

#include "smf_api.h"

typedef struct smf_resample_open_param_s {
	//static const char* keywords = "resamp";
	int rate;
	int channels;
	int sampleBits;
	int target_rate;
	int target_channels;
	int target_sampleBits;
	//
	void* other;
}smf_resample_open_param_t;

/**
 * register resample processer
 */
EXTERNC void smf_resample_register();
//
#endif
