#pragma once
#include "smf_common.h"
namespace smf {
	namespace api {
		struct AudioParam_t{//128bits
			bool setvol : 1;
			bool monopoly : 1;
			uint8_t flags : 6;
			uint32_t rate : 24;
			uint8_t ch : 4;
			uint8_t width : 4;
			uint8_t bits;
			uint16_t volume;
			uint8_t fadein;
			uint8_t fadeout;
			uint8_t weakenOthers;// weaken 0~100% volume of others
			bool to(smf_keys_value_t*& pairs, smf_keys_value_t* end);
		};
		struct VideoParam_t {//128bits
			uint8_t flags : 8;
			uint16_t x : 14;
			uint16_t y : 14;
			uint16_t width : 14;
			uint16_t height : 14;
			bool to(smf_keys_value_t*& pairs, smf_keys_value_t* end);
		};
		struct CustParam_t {
			union {
				uint32_t revs[4];
				struct {
					uint32_t blksize;
					uint32_t blknum;
					uint32_t blkstart;
					uint32_t fsize;
				}fifo;
				uint64_t i64[2];
				char name[16];
				struct {
					uint64_t keys;
					uint32_t fifoCount;
					uint32_t fifoBytes;
				}shmFifo;
				struct {
					uint32_t rate;
					uint32_t ch;
					uint32_t bits;
				}audio;
			};
		};
		struct DmaParam_t : public smf_dma_param_t {
			bool to(smf_keys_value_t*& pairs, smf_keys_value_t* end);
		};
	}
}
