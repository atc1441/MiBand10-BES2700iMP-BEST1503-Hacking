#pragma once
#include "smf_common.h"
namespace smf {
	namespace api {
		struct DecodeParam_t {
			uint64_t id;//string
			uint32_t bitrate;
			union {
				uint32_t params[13];
				struct {//smf_codec_lc3.h
					uint8_t frameDMs;//10ms=100,7.5ms=75,5ms=50,2.5ms=25
					uint8_t hrmode;//true=1,false=0
					uint8_t plcMeth;//lc3_plcmode_e:STANDARD=0,ADVANCED=1
					uint8_t no_interlace;
					uint8_t sampleWidth;
					uint16_t frame_size;
				}lc3;
				struct {
					uint8_t package;
				}aac;
			};
			bool to(smf_keys_value_t*& pairs, smf_keys_value_t* end);
			//bool from(smf_keys_value_t* pairs, int max);
		};
	}
}

