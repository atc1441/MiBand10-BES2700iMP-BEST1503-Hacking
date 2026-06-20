#ifndef EVENT_DETECTION_H
#define EVENT_DETECTION_H

#include <stdint.h>
#include "anc_fir_lms_common.h"
#include "custom_allocator.h"
#include "event_detection_cfg.h"
#include "anc_assist.h"

// #define FB_HOWLING_DETECTION

#define MAX_REF_BEFORE_ANC_CHANNEL_NUM (MAX_REF_CHANNEL_NUM - FIR_CHANNEL_NUM)

#define ENABLE_EVENT_DETECTION
#ifdef ENABLE_EVENT_DETECTION
#define EVENT_DETECTION_SAMPLE_RATE (16000)
#define EVENT_DETECTION_FRAME_SIZE (120)
#endif

#define ANC_FF_FIR_LMS_FIR_FLAG_UPDATE_W                    (1 << 0)
#define ANC_FF_FIR_LMS_FIR_FLAG_UPDATE_W_SHIFT              (0)
#define ANC_FF_FIR_LMS_FIR_FLAG_CONVERGENCE_STATUS          (1 << 1)
#define ANC_FF_FIR_LMS_FIR_FLAG_CONVERGENCE_STATUS_SHIFT    (1)
#define ANC_FF_FIR_LMS_FIR_FLAG_ENV_CHANGED                 (1 << 2)
#define ANC_FF_FIR_LMS_FIR_FLAG_ENV_CHANGED_SHIFT           (2)

typedef struct
{
    WindIdentifyCfg wind_cfg;
    NoiseLevelDetectionConfig noise_cfg;
    FbHowlingDetectionConfig fb_howling_cfg;
    ToneDetectionConfig ff_tone_cfg;
    ToneDetectionConfig fb_tone_cfg;
    EnvNoiseDetectionConfig env_noise_cfg;
    EnergyDetectionConfig energy_detection_cfg;
    WeakDectionCfg weak_fir_cfg;
#ifdef ANC_ASSIST_VPU
    NearSpeechDetectionConfig speech_detection_cfg;
#else
    MicrophoneNearSpeechDetectionConfig speech_detection_cfg;
#endif
} EventDetectionConfig;

typedef struct
{
    // Status changed flag
    uint32_t    ff_gain_changed[MAX_FF_CHANNEL_NUM];
    uint32_t    fb_gain_changed[MAX_FB_CHANNEL_NUM];
    uint32_t    curve_changed[MAX_FB_CHANNEL_NUM];
    uint32_t    fir_flag_changed;

    // Who change gain or curve
    anc_assist_algo_id_t    ff_gain_id[MAX_FF_CHANNEL_NUM];
    anc_assist_algo_id_t    fb_gain_id[MAX_FB_CHANNEL_NUM];
    anc_assist_algo_id_t    curve_id[MAX_FB_CHANNEL_NUM];
    anc_assist_algo_id_t    fir_flag_id;

    // Gain or curve
    float   ff_gain[MAX_FF_CHANNEL_NUM];
    float   fb_gain[MAX_FB_CHANNEL_NUM];
    uint32_t    curve_index[MAX_FB_CHANNEL_NUM];
    uint32_t    fir_flag;

    howling_status_t fb_howling_status[MAX_FB_CHANNEL_NUM];
    howling_status_t hybrid_howling_status[MAX_FF_CHANNEL_NUM];
    noise_status_t noise_status[MAX_FF_CHANNEL_NUM];
    wind_status_t wind_status;

} EventDetectionRes;

typedef struct
{
    bool update_Pz;
    bool update_Sz;
    bool update_MC;
    bool update_W;
    uint32_t fir_flag;
    bool fir_open_again;
    float ff_fb_msc[FIR_CHANNEL_NUM];
    env_noise_status_t env_noise[FIR_CHANNEL_NUM];
} EventController;

typedef struct {
    float *inbuf;
    float *ft;
    float *ps;
} mic_freq_t;

typedef struct EventDetectionState_ EventDetectionState;

EventDetectionState *event_detection_create(int32_t sample_rate, int32_t frame_size, const EventDetectionConfig *cfg, custom_allocator *allocator, EventDetectionRes *res);

void event_detection_process(EventDetectionState *st,process_frame_data_t *buf, int frame_len, EventDetectionRes *res, EventController *curr_controller);

void event_detection_destroy(EventDetectionState *st);

void event_detection_reset_controller(EventController *controller);

int32_t event_detection_set_fir_flag(EventDetectionState* st, uint32_t flag);

int32_t event_detection_reset_fir_flag(EventDetectionState* st,EventDetectionRes *res);

void event_detection_set_anc_on_flag(EventDetectionState* st, bool flag);

void event_detection_set_ff_fir_enable(EventDetectionState *st,bool flag);

#endif
