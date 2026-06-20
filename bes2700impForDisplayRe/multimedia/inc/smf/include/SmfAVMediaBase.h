
#pragma once
#include "SmfMediaApiBase.h"

namespace smf {
	namespace api {
		class AVMediaBase : public MediaApiBase {
        public:
            using Param_t = api::AddParamNode_t;
        public:
            AVMediaBase(const char* s) : MediaApiBase(s) {}
        public:
            using MediaApiBase::Push;
            using MediaApiBase::Pull;
            uint64_t Adds(Param_t& para);
			bool Push(uint64_t id, uint64_t subid, smf_frame_t& frm);
			bool Pull(uint64_t id, uint64_t subid, smf_frame_t& frm);
        };
	}
}