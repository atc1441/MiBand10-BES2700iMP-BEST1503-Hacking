#pragma once
#ifndef ____DEBUG_statistics_H____
#define ____DEBUG_statistics_H____
#include "smf_statistics.h"

namespace smf {
	class DbgRec {
	public:
		int _idx;
		DbgRec(int idx) {
			_idx = idx;
			smf_statistics_start(_idx);
		}
		~DbgRec() {
			smf_statistics_stop(_idx);
		}
	};
	static inline int smf_statistics_get_index0(const char* func, const char* file, unsigned line, const char* other = 0) {
		return smf_statistics_get_index(func, file, line, other);
	}
}
#define dbgCodecCpx(f,l,...) 
// static char _##l = smf::smf_statistics_get_index0(__func__,__FILE__,__LINE__,##__VA_ARGS__); 
// smf::DbgRec x##l(_##l); 
// smf_statistics_tmp(&x##l)
//
#define dbgCodecCp(...) dbgCodecCpx(__func__,__LINE__,##__VA_ARGS__)

#endif
