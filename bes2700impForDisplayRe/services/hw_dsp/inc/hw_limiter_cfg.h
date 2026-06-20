#ifndef __HW_LIMITER_CFG_H__
#define __HW_LIMITER_CFG_H__

#include "hal_aud.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__AUDIO_HW_LIMITER__)
int hwlimiter_open(int ch_map);
void hwlimiter_enable(int ch_map);
int hwlimiter_set_cfg(enum AUD_SAMPRATE_T rate,int index);
void hwlimiter_disable(int ch_map);
void hwlimiter_close(void);

#endif

#ifdef __cplusplus
}
#endif

#endif
