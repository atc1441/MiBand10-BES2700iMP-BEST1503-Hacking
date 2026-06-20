
extern "C" {

#include "gsound_target_ohd.h"

static const GSoundOhdInterface *ohd_interface = NULL;

#define SYNC_IMPL 0

GSoundStatus GSoundTargetGetOhdState(GSoundOhdState *ohd_status_out) {
#if SYNC_IMPL
  *ohd_status_out = OHD_UNSUPPORTED;
#else
  (void)ohd_status_out;
  GSoundOhdState ohd_status = OHD_UNSUPPORTED;
  ohd_interface->gsound_ohd_status(ohd_status);
#endif
  return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetOhdInit(const GSoundOhdInterface *handlers) {
  ohd_interface = handlers;
  return GSOUND_STATUS_OK;
}

}
