#ifndef __SPEECH_SCO_3RD_H__
#define __SPEECH_SCO_3RD_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sco_3rd_init_data_s
{
    int         tx_sample_rate;
    int         rx_sample_rate;
    int         tx_frame_ms;
    int         rx_frame_ms;
    int         sco_frame_ms;
    int8_t      *buf;
    int         len;
}sco_3rd_init_data_st;

int32_t rpc_sco_3rd_process(int16_t *pcm_buf, int32_t pcm_len);

int32_t rpc_sco_3rd_process_rx(int16_t *pcm_buf, int32_t pcm_len);

int32_t rpc_sco_3rd_process_tx(int16_t *pcm_buf, int16_t *ref_buf, int32_t pcm_len);

int32_t rpc_sco_3rd_init(sco_3rd_init_data_st *param, sco_3rd_init_data_st *out_param);

int32_t rpc_sco_3rd_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* __SPEECH_DC_FILTER_RPC_H__ */