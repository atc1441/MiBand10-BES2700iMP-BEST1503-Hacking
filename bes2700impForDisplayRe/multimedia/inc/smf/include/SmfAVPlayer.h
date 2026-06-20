#pragma once
#include "SmfAVMediaBase.h"
namespace smf {
	namespace api {
		class AVPlayer : public AVMediaBase {
		public:
			using AddPrameNode_t = api::AddParamNode_t;
        public:
			AVPlayer();
       };
	}
}