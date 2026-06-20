#pragma once
#include <stdint.h>

namespace smf {
	void sleep_for(uint32_t ms);
	//ipc
	bool smf_ipc_open(uint8_t cpuid);
	bool smf_ipc_close(uint8_t cpuid);
}
