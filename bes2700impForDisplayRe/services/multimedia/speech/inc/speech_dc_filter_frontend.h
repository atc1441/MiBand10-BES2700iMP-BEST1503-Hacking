#ifndef __SPEECH_DC_FILTER_FRONTEND_H__
#define __SPEECH_DC_FILTER_FRONTEND_H__

#ifdef __cplusplus
extern "C" {
#endif

SpeechDcFilterState * rpc_speech_dc_filter_create(int32_t sample_rate,
                    int32_t frame_size, const SpeechDcFilterConfig *cfg);

int32_t rpc_speech_dc_filter_set_config(SpeechDcFilterState *st,
                                        const SpeechDcFilterConfig *cfg);

int32_t rpc_speech_dc_filter_ctl(SpeechDcFilterState *st, int32_t ctl, void *ptr);

int32_t rpc_speech_dc_filter_process(SpeechDcFilterState *st, int16_t *pcm_buf,
                                                            int32_t pcm_len);

int32_t rpc_speech_dc_filter_destroy(SpeechDcFilterState *st);

#define  speech_dc_filter_create rpc_speech_dc_filter_create

#define  speech_dc_filter_ctl rpc_speech_dc_filter_ctl

#define  speech_dc_filter_set_config rpc_speech_dc_filter_set_config

#define  speech_dc_filter_process  rpc_speech_dc_filter_process

#define  speech_dc_filter_destroy  rpc_speech_dc_filter_destroy

#ifdef __cplusplus
}
#endif

#endif /* __SPEECH_DC_FILTER_FRONTEND_H__ */
