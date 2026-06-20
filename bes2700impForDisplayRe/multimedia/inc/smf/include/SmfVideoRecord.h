#pragma once
#include "SmfAVRecord.h"
namespace smf {
	namespace api {
		#if 0
		class VideoRecord : public MediaApiBase {
		public:
			VideoRecord();
		};
		#endif
		using VideoRecord = AVRecord;
	}
}