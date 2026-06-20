#pragma once
#include "stdint.h"
#include "stdbool.h"
#ifndef EXTERNC
#ifndef __cplusplus
#define EXTERNC
#else
#define EXTERNC extern "C"
#endif
#endif

#define SMF_MSG_VERSION 2023082800
///smf msg message header
typedef struct SmfMsg_t {
	union {
		uint64_t head;
		struct {
			uint16_t cmd;
			uint16_t size;
			union {
				uint8_t cpuid;
				struct {
					uint8_t cpusrc : 4;
					uint8_t cpudst : 4;
				};
			};
			uint8_t svcid;
			uint8_t flags;
			union {
				uint8_t flagsExt;
				struct {
					uint8_t ext : 4;
					uint8_t localRemote : 1;
					uint8_t result : 1;
					uint8_t rev0 : 1;
					uint8_t rev1 : 1;
				};
			};
		};
	};
} smf_msg_t, SmfMsg_t, smf_msg_response_t, SmfMsgResponse_t;
//

enum smf_cpu_id_e{
	SMF_CPU_ID_BEGIN = 0X00,
	SMF_CPU_DEFAULT = 0X00,
	SMF_CPU_M55 = 0X01,
	SMF_CPU_BTH = 0X02,
	SMF_CPU_SEN = 0X03,
	SMF_CPU_DSP = 0X04,
	SMF_CPU_M55_1 = 0X05,
	SMF_CPU_M33 = 0x06,
	SMF_CPU_M33_1 = 0x07,
	SMF_CPU_ID_END,
};

typedef struct {
	uint16_t start;
	uint16_t end;
	uint8_t cpuid;
}smf_msg_cpuid_cfg_t, SmfCpuidCfg_t;

typedef struct  {
	const char *ept_name;
	const char *cpu_name;
	uint8_t cpuid;
	uint8_t master;
}smf_msg_eptname_cpu_t;

typedef bool(*CbMsg)(smf_msg_t*);
typedef bool(*CbMsgPriv)(smf_msg_t*,void* priv);
typedef bool(*CbMsgSizePriv)(smf_msg_t*,uint32_t size,void* priv);

