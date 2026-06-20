#include "bt_sco_chain.h"
#include "speech_memory.h"
#include "speech_utils.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "audio_dump.h"
#include "Soundplus_adapter.h"
#include "app_bt_stream.h"
#if defined(SCO_CP_ACCEL)
#include "hal_location.h"
#include "bt_sco_chain_cp_soundplus.h"
#endif
#include "app_anc.h"

#if (defined(RPC_SUPPORT) && defined(SCO_CP_ACCEL_RPC))
#include "speech_sco_chain_3rd.h"
#endif

short *aec_echo_buf = NULL;

// Use to free buffer
static short *aec_echo_buf_ptr;
float pcm_f32[240*4];

#define FRAME_LEN_16K  240
#define FRAME_LEN_8K  120

int speech_init(int tx_sample_rate, int rx_sample_rate,
                     int tx_frame_ms, int rx_frame_ms,
                     int sco_frame_ms,
                     uint8_t *buf, int len)
{
    // we shoule keep a minmum buffer for speech heap
    // sample rate 8k : 4k bytes
    // sample rate 16k: 10k bytes    
    speech_heap_init(buf, len);

    int frame_len = SPEECH_FRAME_MS_TO_LEN(tx_sample_rate, tx_frame_ms);
	//TRACE(1,"speech_init buf_len=%d,frame_len=%d,is_cvsd=%d,is_ancon=%d",len,frame_len,bt_sco_codec_is_cvsd(),app_anc_work_status());

    aec_echo_buf = (short *)speech_calloc(frame_len, sizeof(short));
    aec_echo_buf_ptr = aec_echo_buf;    
#if defined(SCO_CP_ACCEL)
    //if (frame_len == FRAME_LEN_16K)
    sco_cp_init(240, 3);
#endif

#if (defined(RPC_SUPPORT) && defined(SCO_CP_ACCEL_RPC))
    sco_3rd_init_data_st param = {0};
    sco_3rd_init_data_st out_param = {0};
    param.tx_sample_rate    = tx_sample_rate;
    param.rx_sample_rate    = rx_sample_rate;
    param.tx_frame_ms       = tx_frame_ms;
    param.rx_frame_ms       = rx_frame_ms;
    param.buf               = buf;
    param.len               = len;
    rpc_sco_3rd_init(&param, &out_param);

    TRACE(0, "#### [%s] out_param->rx_sample_rate: %d", __func__, out_param.rx_sample_rate);
    TRACE(0, "#### [%s] out_param->tx_sample_rate: %d", __func__, out_param.tx_sample_rate);
    TRACE(0, "#### [%s] out_param->rx_frame_ms: %d", __func__, out_param.rx_frame_ms);
    TRACE(0, "#### [%s] out_param->tx_frame_ms: %d", __func__, out_param.tx_frame_ms);
    TRACE(0, "#### [%s] out_param->len: %d", __func__, out_param.len);
#endif

#if defined(SNDP_TX_3MIC_ENABLE)
    uint8_t key[16] = {0};
    soundplus_auth(key,16);
    //if (frame_len == FRAME_LEN_16K)
    {
        soundplus_init(bt_sco_codec_is_cvsd());
    }
#endif

#if defined(SNDP_TX_3MIC_DUMP_ENABLE)  
    audio_dump_init(frame_len, sizeof(int16_t), 5);
#endif
    return 0;
}

int speech_deinit(void)
{
#if defined(SCO_CP_ACCEL) 
	sco_cp_deinit();
#endif

    speech_free(aec_echo_buf_ptr);

#if defined(SNDP_TX_3MIC_ENABLE)    
    soundplus_deinit();
#endif

#if (defined(RPC_SUPPORT) && defined(SCO_CP_ACCEL_RPC))
    rpc_sco_3rd_deinit();
#endif

    size_t total = 0, used = 0, max_used = 0;
    speech_memory_info(&total, &used, &max_used);
    TRACE(4,"SPEECH MALLOC MEM: total - %d, used - %d, max_used - %d.", total, used, max_used);
    ASSERT(used == 0, "[%s] used != 0", __func__);

    return 0;
}

int speech_tx_process(void *pcm_buf, void *ref_buf, int *pcm_len)
{
	int16_t *buf = (int16_t *)pcm_buf;
    int16_t *ref_dump = (int16_t *)ref_buf;
	int pcm16_len = *pcm_len;
    int tmp_len;
    TRACE(1,"speech_tx_process pcm_len=%d",pcm16_len);
#if defined(SNDP_TX_3MIC_DUMP_ENABLE)
    int frame_len = *pcm_len/SPEECH_CODEC_CAPTURE_CHANNEL_NUM;
    int16_t pcm_dump[720]={0};
    audio_dump_clear_up();
    for(int i = 0; i < frame_len; i++)
    {
        pcm_dump[i] = buf[3*i+0];
        pcm_dump[i+frame_len] = buf[3*i+1];
        pcm_dump[i+frame_len*2] = buf[3*i+2];
    }
    audio_dump_add_channel_data(0, pcm_dump, frame_len);
    audio_dump_add_channel_data(1, pcm_dump+frame_len, frame_len);
    audio_dump_add_channel_data(2, pcm_dump+frame_len*2, frame_len);
    audio_dump_add_channel_data(3, ref_dump, frame_len);
#endif

#if defined(SNDP_TX_3MIC_TIME_ENABLE)
     uint32_t start_ticks, end_ticks;    
     start_ticks = hal_fast_sys_timer_get();
#endif

#if (defined(RPC_SUPPORT) && defined(SCO_CP_ACCEL_RPC))
    int out_pcm_len = rpc_sco_3rd_process_tx(buf, ref_buf, pcm16_len);
    TRACE(1, "[%s] rpc_sco_3rd_process_tx out pcm_len=%d", __func__, out_pcm_len);
#endif

#if defined(SNDP_TX_3MIC_BYPASS_ENABLE)  //bypass
    for(int i=0; i<pcm16_len/3; i++)
	{
		buf[i] = buf[i*3+0];   //talk
		//buf[i] = buf[i*3+1];   //FF
		//buf[i] = buf[i*3+2];   //FB
	}
#else  
/**************alg process section************************/
#if (defined(SNDP_TX_3MIC_ENABLE) && defined(SCO_CP_ACCEL))
    sco_status_t status = {
        .vad = true,
    };
	soundplus_deal_Aec(pcm_f32, buf, ref_dump, pcm16_len, pcm16_len/3);
    tmp_len = 720;
    sco_cp_process(buf, pcm_f32, &pcm_f32[240*3], &tmp_len,&status);   
#endif
#endif 

    *pcm_len = pcm16_len/SPEECH_CODEC_CAPTURE_CHANNEL_NUM;

#if defined(SNDP_TX_3MIC_TIME_ENABLE)
    end_ticks = hal_fast_sys_timer_get();
    TRACE(4,"[sco_ap_process] takes %d us\n", FAST_TICKS_TO_US(end_ticks - start_ticks));
#endif

#if defined(SNDP_TX_3MIC_DUMP_ENABLE)    
    audio_dump_add_channel_data(4, pcm_buf, frame_len);
    audio_dump_run();
#endif
    return 0;
}

int speech_rx_process(void *pcm_buf, int *pcm_len)
{
    //int16_t *pcm16_buf = (int16_t *)pcm_buf;

#if (defined(RPC_SUPPORT) && defined(SCO_CP_ACCEL_RPC))
    int pcm16_len = *pcm_len;

    int out_pcm_len = rpc_sco_3rd_process_rx(pcm_buf, pcm16_len);
    TRACE(1, "[%s] rpc_sco_3rd_process_rx out pcm_len=%d", __func__, out_pcm_len);
#endif

    return 0;
}
