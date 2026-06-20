#ifndef __ANC_MC_FIR_LMS_H__
#define __ANC_MC_FIR_LMS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "anc_fir_lms_common.h"
#include "event_detection.h"

typedef enum
{
    ANC_MC_FIR_LMS_STATE_BYPASS = 0,
    ANC_MC_FIR_LMS_STATE_CONVERGENT,
    ANC_MC_FIR_LMS_STATE_NO_CONVERGENT,
} ANC_MC_FIR_LMS_STATE;

typedef enum {
    ANC_MC_FIR_LMS_STATE_NO_CHANGED = 0,
    ANC_MC_FIR_LMS_STATE_CHANGED_TO_CONVERGENT,
    ANC_MC_FIR_LMS_STATE_CHANGED_TO_NO_CONVERGENT
} ANC_MC_FIR_LMS_STATUS;

typedef struct {
    uint32_t debug_en;

    int32_t mc_max_cnt;
    int32_t mc_period_cnt;

    uint32_t mc_filt_cnt;
    uint32_t fb_filt_cnt;

    float mc_filt_ref_gain;
    float fb_filt_ref_gain;

    const FIR_LMS_FILTER_CFG_T  *mc_filt_cfg;
    const FIR_LMS_FILTER_CFG_T  *fb_filt_cfg;
} ANC_MC_FIR_LMS_CFG_T;

typedef struct ANCMCFirLmsSt_ ANCMCFirLmsSt;

int32_t anc_mc_fir_lms_reset(ANCMCFirLmsSt* st,int32_t status, enum AUD_CHANNEL_MAP_T map);

ANCMCFirLmsSt * anc_mc_fir_lms_create(int32_t sample_rate, int32_t frame_len, const ANC_MC_FIR_LMS_CFG_T * cfg, custom_allocator *allocator,FIR_LMS_CALIB_GAIN *calib_gain);
void anc_mc_fir_lms_destroy(ANCMCFirLmsSt* st);
ANC_MC_FIR_LMS_STATUS anc_mc_fir_lms_process(ANCMCFirLmsSt* st, float *ref, float *fb, int frame_len, EventController *curr_controller, float *update_mc_fir, enum AUD_CHANNEL_MAP_T map);

int32_t *mc_fir_lms_coeff_cache(ANCMCFirLmsSt* st);
// int32_t *get_mc_local_fir_cache(ANCMCFirLmsSt* st);
float* get_mc_local_fir(ANCMCFirLmsSt* st);
void anc_ff_fir_lms_reset_mc_FIR(ANCMCFirLmsSt* st,  enum AUD_CHANNEL_MAP_T map);
void anc_ff_fir_lms_set_mc_fir_calib_gain(ANCMCFirLmsSt *st, FIR_LMS_CALIB_GAIN *calib_gain);

int32_t anc_ff_fir_lms_set_mc_fir_status(ANCMCFirLmsSt* st, bool enable);
#ifdef __cplusplus
}
#endif

#endif