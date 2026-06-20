#pragma once
#include "smf_common.h"
#include "smf_frame.h"
#include <functional>
#include "SmfParam.def.h"
namespace smf {
	namespace api {
		struct SinkParam_t{
			uint64_t id;
			union {//uint64x7
				uint64_t params[7];
				struct {
					const char* filename;
				}file;
				struct {
					std::function<bool(smf_frame_t*)>* func;
					bool(*cb)(smf_frame_t*);
				}func;
				DmaParam_t dma;
				struct {
					uint8_t devid;
					uint8_t streamid;
					uint16_t dms; //0.1ms
					uint16_t path;
					uint16_t map;
					uint8_t sync_type;
				}af;
				struct {

				}usb;
				struct {

				}lineout;
				struct {
					uint32_t aseid;
					uint32_t conlid;
					uint32_t size;
					uint32_t bisflag;
				}ble;
				struct {

				}le;
				struct {
					uint64_t codec;
					uint32_t devid;
					uint32_t kfifo;
					uint32_t tcnt;
				}a2dp;
				struct {
					uint32_t streamId;
					uint32_t codecPayload;
					uint32_t dtmfPayload;
					uint32_t level;
					uint32_t codecSamples;
					uint32_t support_nack;
				}rtp;
				struct {
					uint32_t infoAddr;
				}lcdc;
			};

			bool to(smf_keys_value_t*& pairs, smf_keys_value_t* end);
			//bool from(smf_keys_value_t* pairs, int max);
		};
	}
}
