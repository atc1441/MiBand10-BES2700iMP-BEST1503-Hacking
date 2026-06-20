#pragma once
#include "smf_common.h"
#include "smf_frame.h"
#include <functional>
#include "SmfParam.def.h"
namespace smf {
	namespace api {
		struct SourceParam_t{
			uint64_t id;
			union {//uint64x7
				//uint64_t params[7];
				struct {
					const char* filename;
					uint32_t startPosition;
					void* iopara;
				}file;
				struct {
					std::function<bool(smf_frame_t*)>* func;
					bool(*cb)(smf_frame_t*);
					//media info
					uint64_t codec;
					union {
						uint8_t package;
						uint8_t streamType;
					};
				}func;
				DmaParam_t dma;
				struct {
					uint8_t devid;
					uint8_t streamid;
					uint16_t dms; //0.1ms
					uint8_t path;
					uint8_t chmap;
				}af;
				struct {

				}usb;
				struct {

				}linein;
				struct {

				}le;
				struct {
					// uint32_t rate;
					// uint8_t ch;
					// uint8_t bits;
					uint32_t size;
					uint32_t bisflag;
				}ble;
				struct {
					uint32_t kfifo;
					uint64_t codec;
				}a2dp;
				struct {
					uint32_t codecPayload;
					uint32_t codecSamples;
					uint32_t support_nack;
				}rtp;
				struct {
					uint32_t rate;
					uint16_t volume;
					uint16_t frameDuration;
					uint16_t number;
					uint16_t len;
				}dtmf;
			};

			bool to(smf_keys_value_t*& pairs, smf_keys_value_t* end);
			//bool from(smf_keys_value_t* pairs, int max);
		};
	}
}
