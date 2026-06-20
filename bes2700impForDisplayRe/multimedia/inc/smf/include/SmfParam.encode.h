#pragma once
#include "smf_common.h"
namespace smf {
	namespace api {
		struct EncodeParam_t {
			uint64_t id;//string
			uint32_t bitrate;
			union {
				uint32_t params[13];
				struct {//smf_codec_aac.h
					uint8_t aot;//smf_aac_aot_e//LC = 2,HE = 5,HE_V2 = 29,LC_MPEG2 = 129
					uint8_t layer;//0
					uint8_t package;//smf_aac_package_e//RAW=0,ADIF=1,ADTS=2,MCP1=6,MCP0=7,LOAS=10,
					uint8_t vbr;//smf_aac_brmode_e//CBR=0,VBR=1~5(LOW_BITRATE~HIGH_BITRATE)
				}aac;
				struct {//smf_codec_sbc.h
					uint8_t bitPool;
					uint8_t channelMode;//smf_sbc_channel_mode_e:MONO=0,DUAL=1,STEREO=2,JOINT_STEREO=3,
					uint8_t allocMethod;//smf_sbc_alloc_method_e:LOUDNESS=0,SNR=1,
					uint8_t numBlocks;
					uint8_t numSubBands;//4/8
					uint8_t sbcMode;//4/8
				}sbc;
				struct {//smf_codec_lc3.h
					uint8_t frameDMs;//10ms=100,7.5ms=75,5ms=50,2.5ms=25
					uint8_t hrmode;//true=1,false=0
					uint8_t no_interlace;
					uint8_t sampleWidth;
				}lc3;
				struct {//smf_codec_mp3.h
					uint8_t version;//smf_mp3_version_e:
					uint8_t layer;//smf_mp3_layer_e:LAYER1=0,LAYER2=1,LAYER3=2,
				}mp3;
				struct {
					uint16_t frameDMs;//auto=0,2.5ms=25,5ms=50,10ms=100,20ms=200,400,600,800,1000,1200
					uint16_t application;
					uint16_t bitrate;
					uint8_t lsb_depth;
					uint8_t complexity;
					uint8_t have_head;
				}opus;
				struct {
				}silk;
			};

			bool to(smf_keys_value_t*& pairs, smf_keys_value_t* end);
			//bool from(smf_keys_value_t* pairs, int max);
		};
	}
}
