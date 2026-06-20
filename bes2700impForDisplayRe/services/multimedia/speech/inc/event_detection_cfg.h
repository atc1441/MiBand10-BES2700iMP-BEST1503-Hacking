#ifndef EVENT_DETECTION_CFG_H
#define EVENT_DETECTION_CFG_H

typedef struct
{
    uint32_t    debug_en;
    uint32_t	scale_size;
    uint32_t    to_none_targettime;
    float       power_thd;
    float       no_thd;
    float       small_thd;
    float       strong_thd;
    float       normal_thd;
    float       gain_none;
    float       gain_small_to_none;
    float       gain_small;
    float       gain_normal_to_small;
    float       gain_normal;
    float       gain_strong_to_normal;
    float       gain_strong;
} WindIdentifyCfg;

typedef struct
{
    uint32_t    debug_en;
    float       strong_low_thd;
    float       strong_limit_thd;
    float       lower_low_thd;
    float       lower_mid_thd;
    float       quiet_out_thd;
    float       quiet_thd;
    float       extremely_quiet_out_thd;
    float       extremely_quiet_in_thd;
    float       snr_thd;
    int         period;
    int         window_size;
    int32_t     trans_table[5][5];
    float       band_freq[4];
    float       band_weight[3];
} NoiseLevelDetectionConfig;

typedef enum {
    TONE_STATUS_INVALID = 0,
    TONE_STATUS_NORMAL,
    TONE_STATUS_HOWLING,
    TONE_STATUS_QTY,
} tone_status_t;

typedef struct {
    uint32_t debug_en;
    uint32_t ind0;
    uint32_t ind1;
    uint32_t time_thd;
    float power_thd;
} ToneDetectionConfig;

typedef struct
{
    uint32_t start_freq;
    uint32_t end_freq;

    float gain_fb;

    uint32_t trans_table[TONE_STATUS_QTY][TONE_STATUS_QTY];
} FbHowlingDetectionConfig;

typedef enum
{
    ENV_NOISE_STATUS_SMALL = 0,
    ENV_NOISE_STATUS_QUIET,
    ENV_NOISE_STATUS_MIDDLE,
    ENV_NOISE_STATUS_LARGE,
    ENV_NOISE_STATUS_QTY,
} env_noise_status_t;

typedef struct
{
    uint32_t debug_en;
    int32_t up_thresh;
    int32_t quiet_thresh;
    int32_t low_thresh;
    int32_t thresh_delta;
    int32_t trans_table[ENV_NOISE_STATUS_QTY][ENV_NOISE_STATUS_QTY];
} EnvNoiseDetectionConfig;

typedef struct {
    uint32_t debug_en;
    int32_t timer;
    float up_thresh;
    float low_thresh;
    int32_t normal_period;
} EnergyDetectionConfig;

typedef struct {
    uint32_t debug_en;
    uint32_t wear_noise_frame_num;
    uint32_t wear_change_num;
    uint32_t noise_change_num;
} WeakDectionCfg;


typedef struct {
    uint32_t debug_en;
    float low_freq_threshold;
    float high_freq_threshold;
    float talk_energy_threshold;
} NearSpeechDetectionConfig;

typedef struct {
    uint32_t debug_en;
    float pnc_energy_ratio_thd;
    float anc_energy_ratio_thd_quiet;
    float msc_thd_quiet;
    float anc_energy_ratio_thd_middle;
    float msc_thd_middle;
    int32_t detection_cnt_thd;
    uint32_t energy_clac_start;
    uint32_t energy_clac_end;
    float    denoise_thd;
} MicrophoneNearSpeechDetectionConfig;

typedef enum {
    WEAR_LEAK_STATUS_INVALID = 0,
    WEAR_LEAK_STATUS_NO_LEAK,
    WEAR_LEAK_STATUS_LOW_LEAK,
    WEAR_LEAK_STATUS_NORMAL_LEAK,
    WEAR_LEAK_STATUS_STRONG_LEAK,
} wear_leak_status_t;
#endif
