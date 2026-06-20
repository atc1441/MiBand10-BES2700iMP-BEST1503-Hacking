#ifndef PROMPT_ADAPTIVE_ANC_H
#define PROMPT_ADAPTIVE_ANC_H

#include <stdint.h>
#include <stdbool.h>


#define PROMPT_APDPTIVE_ANC_CURVE_NUM (8)

typedef struct {
	int dump_en;
	int cal_period;
	float thd1[PROMPT_APDPTIVE_ANC_CURVE_NUM];
	float thd2[PROMPT_APDPTIVE_ANC_CURVE_NUM];
	float thd3[PROMPT_APDPTIVE_ANC_CURVE_NUM];


} PromptAdaptiveANCConfig;

struct PromptAdaptiveANCState_;
typedef struct PromptAdaptiveANCState_ PromptAdaptiveANCState;

void prompt_adaptive_anc_reset(uint32_t sample_rate, uint32_t frame_len, PromptAdaptiveANCConfig *cfg);
int32_t prompt_adaptive_anc_process(float *fb_mic, float *ref,float *ff, uint32_t pcm_len);

int32_t prompt_adaptive_anc_set_working_status(uint32_t status);
int32_t prompt_adaptive_anc_set_anc_cfg(void * anc_cfg);

#endif