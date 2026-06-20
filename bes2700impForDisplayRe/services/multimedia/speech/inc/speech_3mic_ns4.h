#ifndef __SPEECH_3MIC_NS4_H__
#define __SPEECH_3MIC_NS4_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    int32_t     bypass;
    int32_t     wnr_enable;
    float       wind_thd;
    int32_t     wnd_pwr_thd;
    float       wind_gamma;
    int32_t     state_trans_frame_thd;

    int32_t     af_enable;
    float       filter_gamma;
    int32_t     vad_bin_start1;
    int32_t     vad_bin_end1;
    int32_t     vad_bin_start2;
    int32_t     vad_bin_end2;
    int32_t     vad_bin_start3;
    int32_t     vad_bin_end3;
    float       coef1_thd;
    float       coef2_thd;
    float       coef3_thd;

    int32_t     calib_enable;
    int32_t     calib_delay;
    float*      filter;
    int32_t     filter_len;
    float*      filter2;
    int32_t     filter2_len;

    int32_t     low_ram_enable;
    int32_t     low_mips_enable;

    int32_t     echo_supp_enable;
    int32_t     ref_delay;

    int32_t     post_supp_enable;
    float       denoise_db;

    int32_t     mix_band_index;
    float       mix_band_ratio;

} Speech3MicNs4Config;
struct Speech3MicNs4State_;
typedef struct Speech3MicNs4State_ Speech3MicNs4State;
Speech3MicNs4State *speech_3mic_ns4_create(int sample_rate, int frame_size, const Speech3MicNs4Config *cfg);
int32_t speech_3mic_ns4_destroy(Speech3MicNs4State *st);
int32_t speech_3mic_ns4_process(Speech3MicNs4State *st, short *pcm_buf, short *ref_buf, int32_t pcm_len, short *out_buf);
int speech_3mic_ns4_set_config(Speech3MicNs4State *st, const Speech3MicNs4Config *cfg);
float speech_3mic_ns4_get_required_mips(Speech3MicNs4State *st);
#ifdef __cplusplus
}
#endif

#endif