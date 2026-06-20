#ifndef  __THIRDPARTY_ASSIST_MIC_H__
#define  __THIRDPARTY_ASSIST_MIC_H__

#include <stdint.h>
#include <stdbool.h>
#include "plat_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t en;
} ThirdpartyAssistRes;

typedef struct {
    int32_t en;
} ThirdpartyAssistConfig;

int32_t thirdparty_anc_assist_create(int sample_rate, int ch_num, int frame_size, const ThirdpartyAssistConfig *cfg);
int32_t thirdparty_anc_assist_destroy(void);
int32_t thirdparty_anc_assist_set_cfg(ThirdpartyAssistConfig *cfg);
ThirdpartyAssistRes thirdparty_anc_assist_process(float **ff_mic, uint8_t ff_ch_num, float **fb_mic, uint8_t fb_ch_num, float *talk_mic, float *ref, uint32_t frame_len);

#ifdef __cplusplus
}
#endif

#endif