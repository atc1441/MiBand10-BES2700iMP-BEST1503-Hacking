
#include "smf_common.h"
#include "smf_media.h"

namespace smf {
    namespace api {
        class MediaInfo {
        public:
            MediaInfo(const char* url);
            ~MediaInfo();
        public:
            const smf_meta_info_t* GetMetaInfo() const;
			const smf_media_t* GetMedia() const;
            const smf_progress_t* GetProgress() const;
        private:
            void* _inner;
        };
    }
}