#pragma once
#ifndef __SMF_PROCESS_PLC_H__
#define __SMF_PROCESS_PLC_H__

#include "smf_api.h"

enum smf_plc_channel_e {
};


typedef struct smf_audio_speed_ctr_open_param_s {
	int rate;
	int channelNum;
	int sampleBits;
	float speed;
	void* other;
}smf_audio_speed_ctr_open_param_t;

/**
 * register plc processer
 */
EXTERNC void smf_audio_speed_ctr_register();

#endif
