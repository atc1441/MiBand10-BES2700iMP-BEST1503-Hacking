#ifndef __SMF_AUDIO_PROCESS_H__
#define __SMF_AUDIO_PROCESS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "iir_process.h"
#include "drc.h"
#include "limiter.h"

typedef enum {
    AUDIO_PROCESS_TYPE_SW = 1 << 0,
    AUDIO_PROCESS_TYPE_HW = 1 << 1,
    AUDIO_PROCESS_TYPE_QTY
} AUDIO_PROCESS_TYPE_T;

enum AUDIO_PROCESS_ALGO_TYPE {
    AUDIO_PROCESS_SW_IIR_EQ     = 1 << 0,
    AUDIO_PROCESS_HW_DAC_IIR_EQ = 1 << 1,
    AUDIO_PROCESS_DRC           = 1 << 2,
    AUDIO_PROCESS_LIMITER       = 1 << 3,
};

typedef struct {
    IIR_CFG_T     iir_eq;
    DrcConfig     drc;
    LimiterConfig limiter;
} AUDIO_PROCESS_ALGO_T;

typedef struct {
    enum AUDIO_PROCESS_ALGO_TYPE alg_type;
    AUDIO_PROCESS_ALGO_T algo_cfg;
} AUDIO_PROCESS_CFG_T;

typedef struct {
    uint32_t sample_rate;
    uint32_t sample_bits;
    uint32_t ch_num;
    uint32_t frame_size;
}AUDIO_PROCESS_CONFIG_T;

int smf_audio_process_init(void);

int smf_audio_process_set_config(AUDIO_PROCESS_TYPE_T type, void* buff, uint32_t len);

int smf_audio_process_open(AUDIO_PROCESS_CONFIG_T* param, void* buff, uint32_t len);

int smf_audio_process_run(uint8_t *ibuf, uint8_t *obuf, uint32_t len);

int smf_audio_process_close(void);

int smf_audio_process_deinit(void);

#ifdef __cplusplus
}
#endif

#endif