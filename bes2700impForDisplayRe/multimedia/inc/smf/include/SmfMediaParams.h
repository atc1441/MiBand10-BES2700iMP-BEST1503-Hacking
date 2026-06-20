#pragma once
#include "SmfParam.def.h"
#include "SmfParam.encode.h"
#include "SmfParam.decode.h"
#include "SmfParam.source.h"
#include "SmfParam.sink.h"
namespace smf {
	namespace api {
		class SetParam_t;
		struct AddParams_t {//32+64+64+64 byte
			uint64_t name;//4~5
			uint64_t type;//
			union {
				uint64_t codec;//9~10
				uint64_t keys;
			};
			//head
			struct {//uint64_t
				uint32_t fifoBytes : 24;//6
				uint8_t fifoCount : 8;//7
				union {
					uint32_t flags;//8
					struct {
						bool is_source : 1;
						bool is_record : 1;
						bool is_video : 1;
						bool is_autofree : 1;//auto clean up resources
						bool is_monopoly : 1;
						bool is_cust : 1;
						uint16_t flags_rev0 : 10;// bit mode
						uint8_t props_type : 3;//ePropsTypes
						uint8_t extra_type : 3;//eExtraTypes
						uint16_t flags_rev1 : 10;
					};
				};
			};
			// NOTE: set cust value is invalid for audio source
			CustParam_t cust;//0~3 uint64_t*2
			//is_video
			union {//uint64_tx2
				AudioParam_t audio;
				VideoParam_t video;
			};
			//is_source
			union {//64byte
				SinkParam_t sink;//11
				SourceParam_t src;//11
			};
			//props_type
			union {//64byte
				smf_keys_value_t props[8];//12
				EncodeParam_t enc;//12
				DecodeParam_t dec;//12
				uint64_t ids[8];
			};
			// NOTE: pass addr/size pair
			union {//32byte
				uint32_t extdata[8]; // 45-60
				uint64_t extdata64[4];
				smf_keys_value_t extprops[4];
				smf_params_t params[2];
			};
			//
			bool to(uint32_t*, int max);
			bool save_algo_param();
			bool free_algo_param();
			bool process_algo_param();
		};
		enum class ePropsTypes {
			keyValuePair,
			enc,
			dec,
			id,
		};
		enum class eExtraTypes {
			null,
			extra,
			params,
		};
		class SetParam_t {
		public:
			const char* path;
			uint32_t keys;
			union {
				uint32_t flags;
				struct {
					bool is_local : 1; // default remote
					bool pass_data : 1;
					// bool set_video : 1;
					uint8_t paraType : 4;
					uint8_t target_cpu : 3;
					uint8_t tagid : 2;
				};
			};
			union {
				uint32_t data[32]; // max 128 byte
				void* val;
				struct {
					void* addr;
					uint32_t size;
				};
			};
		};
		enum class eParaType {
			none = 0,
			addr_copy,
			addr_static
		};
		using GetParam_t = SetParam_t;
		struct AddParamNode_t {
			AddParams_t* para;
			AddParamNode_t* next;
		};
	}
}