#include "plat_types.h"
#include "cmsis.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include <string.h>
#include "rpc.h"
#include "speech_sco_chain_3rd.h"

#define SPEECH_3RD_PROCESS          "3rd_process"
#define SPEECH_3RD_PROCESS_RX       "3rd_process_rx"
#define SPEECH_3RD_PROCESS_TX       "3rd_process_tx"
#define SPEECH_3RD_INIT             "3rd_init"
#define SPEECH_3RD_DEINIT           "3rd_deinit"

struct sco_3rd_process {
    int16_t *pcm_buf;
    int16_t *ref_buf;
    int32_t pcm_len;
};

int32_t rpc_sco_3rd_process(int16_t *pcm_buf,
                            int32_t pcm_len)
{
    struct rpc_args in_args = {0};
    struct rpc_args out_args = {0};
    struct sco_3rd_process in_param;
    struct sco_3rd_process out_param;

    in_param.pcm_buf = pcm_buf;
    in_param.pcm_len = pcm_len;

    in_args.args = &in_param;
    in_args.args_len = sizeof(in_param);

    out_args.args = &out_param;
    out_args.args_len = sizeof(out_param);

    rpc_status_t ret = rpc_exec_cmd_sync(RPC_CORE_BTH_DSP, SPEECH_3RD_PROCESS,
                                        &in_args, &out_args, RPC_WAIT_FOREVER);
    if (ret != RPC_OK)
        return -1;

    //TRACE(0, "#### [%s] pcm_len: %d", __func__, out_param.pcm_len);

    return out_param.pcm_len;
}

int32_t rpc_sco_3rd_process_rx(int16_t *pcm_buf,
                               int32_t pcm_len)
{
    struct rpc_args in_args = {0};
    struct rpc_args out_args = {0};
    struct sco_3rd_process in_param;
    struct sco_3rd_process out_param;

    in_param.pcm_buf = pcm_buf;
    in_param.pcm_len = pcm_len;

    in_args.args = &in_param;
    in_args.args_len = sizeof(in_param);

    out_args.args = &out_param;
    out_args.args_len = sizeof(out_param);

    rpc_status_t ret = rpc_exec_cmd_sync(RPC_CORE_BTH_DSP, SPEECH_3RD_PROCESS_RX,
                                        &in_args, &out_args, RPC_WAIT_FOREVER);
    if (ret != RPC_OK)
        return -1;

    //TRACE(0, "#### [%s] pcm_len: %d", __func__, out_param.pcm_len);

    return out_param.pcm_len;
}

int32_t rpc_sco_3rd_process_tx(int16_t *pcm_buf,
                               int16_t *ref_buf,
                               int32_t pcm_len)
{
    struct rpc_args in_args = {0};
    struct rpc_args out_args = {0};
    struct sco_3rd_process in_param;
    struct sco_3rd_process out_param;

    in_param.pcm_buf = pcm_buf;
    in_param.ref_buf = ref_buf;
    in_param.pcm_len = pcm_len;

    in_args.args = &in_param;
    in_args.args_len = sizeof(in_param);

    out_args.args = &out_param;
    out_args.args_len = sizeof(out_param);

    rpc_status_t ret = rpc_exec_cmd_sync(RPC_CORE_BTH_DSP, SPEECH_3RD_PROCESS_TX,
                                        &in_args, &out_args, RPC_WAIT_FOREVER);
    if (ret != RPC_OK)
        return -1;

    //TRACE(0, "#### [%s] pcm_len: %d", __func__, out_param.pcm_len);

    return out_param.pcm_len;
}

int32_t rpc_sco_3rd_init(sco_3rd_init_data_st *param, sco_3rd_init_data_st *out_param)
{
    struct rpc_args in_args = {0};
    struct rpc_args out_args = {0};

    if(param == NULL || out_param == NULL) {
        TRACE(0, "#### [%s] param is NULL", __func__);
        return -1;
    }

#if 0
    TRACE(0, "#### [%s] param->rx_sample_rate: %d", __func__, param->rx_sample_rate);
    TRACE(0, "#### [%s] param->tx_sample_rate: %d", __func__, param->tx_sample_rate);
    TRACE(0, "#### [%s] param->rx_frame_ms: %d", __func__, param->rx_frame_ms);
    TRACE(0, "#### [%s] param->tx_frame_ms: %d", __func__, param->tx_frame_ms);
    TRACE(0, "#### [%s] param->len: %d", __func__, param->len);
#endif

    in_args.args = param;
    in_args.args_len = sizeof(sco_3rd_init_data_st);

    out_args.args = out_param;
    out_args.args_len = sizeof(sco_3rd_init_data_st);

    rpc_status_t ret = rpc_exec_cmd_sync(RPC_CORE_BTH_DSP, SPEECH_3RD_INIT,
                                        &in_args, &out_args, RPC_WAIT_FOREVER);
    if (ret != RPC_OK)
        return -1;

    return 0;
}

int32_t rpc_sco_3rd_deinit()
{
    rpc_status_t ret = 1;

    TRACE(0, "#### [%s] handle", __func__);
    ret = rpc_exec_cmd_sync(RPC_CORE_BTH_DSP, SPEECH_3RD_DEINIT,
                                        NULL, NULL, RPC_WAIT_FOREVER);
    if (ret != RPC_OK)
        return -1;

    return 0;
}