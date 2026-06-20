#pragma once
#include "SmfMediaParams.h"
namespace smf {
	namespace api {
		class AudioSco{
		public:
			AudioSco();
            bool Start(uint64_t codec = 0);
            bool Stop();
            bool SetDownVolume(uint16_t vol);
            bool SetDownMute(bool mute);
            bool SetUpMute(bool mute);
        public:
            AddParams_t upSink;
            AddParams_t upSrc;
            AddParams_t downSink;
            AddParams_t downSrc;
		};
	}
}