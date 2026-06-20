#pragma once
#include "SmfAVPlayer.h"
namespace smf {
	namespace api {
		#if 0
		class VideoPlayer : public MediaApiBase {
		public:
			VideoPlayer();
		};
		#endif
		using VideoPlayer = AVPlayer;
	}
}