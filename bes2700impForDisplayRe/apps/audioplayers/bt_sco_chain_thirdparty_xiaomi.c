#include "bt_sco_chain.h"
#include "speech_memory.h"
#include "speech_utils.h"
#include "speech_cfg.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "audio_dump.h"
#include "app_bt_stream.h"
#include "arm_math.h"
#include "arm_math_ex.h"
#if defined(SCO_CP_ACCEL)
#include "app_utils.h"
#include "hal_location.h"
#include "bt_sco_chain_cp_xiaomi.h"
#endif
#include "app_anc.h"
#if defined(XIAOMI_TX_3MIC_ENABLE) 
#include "xiaomi_enc_adapter.h"
#endif
// #define XIAOMI_TX_3MIC_BYPASS_ENABLE
// #define XIAOMI_TX_3MIC_DUMP_ENABLE

#if defined(SPEECH_TX_24BIT)
int32_t *aec_echo_buf = NULL;
#else
short *aec_echo_buf = NULL;
#endif
// Use to free buffer
#if defined(SPEECH_TX_24BIT)
static int32_t *aec_echo_buf_ptr;
#else
static short *aec_echo_buf_ptr;
#endif
// float pcm_f32[240*4];

#define FRAME_LEN_16K  240
#define FRAME_LEN_8K  120

int speech_init(int tx_sample_rate, int rx_sample_rate,
                     int tx_frame_ms, int rx_frame_ms,
                     int sco_frame_ms,
                     uint8_t *buf, int len)
{
    TRACE(2,"[%s] xiaomi enc start....", __func__);
    TRACE(2,"tx_sample_rate =%d  tx_frame_ms = %d sco_frame_ms = %d ",tx_sample_rate,tx_frame_ms,sco_frame_ms);
	TRACE(2,"SPEECH_CODEC_CAPTURE_CHANNEL_NUM = %d", SPEECH_CODEC_CAPTURE_CHANNEL_NUM);
	TRACE(2,"SPEECH_PROCESS_FRAME_MS = %d", SPEECH_PROCESS_FRAME_MS);
	TRACE(2,"rx_frame_ms = %d", rx_frame_ms);
	TRACE(2,"rx_sample_rate = %d", rx_sample_rate);
    #if defined(SPEECH_TX_24BIT)
    TRACE(2,"[%s] SPEECH_TX_24BIT SET", __func__);
    #endif
    // we shoule keep a minmum buffer for speech heap
    // sample rate 8k : 4k bytes
    // sample rate 16k: 10k bytes    
    speech_heap_init(buf, len);

    int frame_len = SPEECH_FRAME_MS_TO_LEN(tx_sample_rate, tx_frame_ms);
	TRACE(1,"speech_init buf_len=%d,frame_len=%d,is_cvsd=%d,is_ancon=%d",len,frame_len,bt_sco_codec_is_cvsd(),app_anc_work_status());

#if defined(SPEECH_TX_24BIT)
    aec_echo_buf = (int32_t *)speech_calloc(frame_len, sizeof(int32_t));
#else
    aec_echo_buf = (short *)speech_calloc(frame_len, sizeof(short));
#endif
    aec_echo_buf_ptr = aec_echo_buf;    

#if defined(XIAOMI_TX_3MIC_ENABLE) 
    xiaomi_enc_init(tx_sample_rate, tx_sample_rate);
#endif

#if defined(SCO_CP_ACCEL)
    //if (frame_len == FRAME_LEN_16K)
    sco_cp_init(frame_len,  SPEECH_CODEC_CAPTURE_CHANNEL_NUM);
#endif

#if defined(XIAOMI_TX_3MIC_DUMP_ENABLE)  
    audio_dump_init(frame_len, sizeof(short), SPEECH_CODEC_CAPTURE_CHANNEL_NUM+2);
#endif
    return 0;
}

int speech_deinit(void)
{
#if defined(SCO_CP_ACCEL) 
    sco_cp_deinit();
#endif

    speech_free(aec_echo_buf_ptr);
#if defined(XIAOMI_TX_3MIC_ENABLE) 
    xiaomi_enc_deinit();
#endif

    size_t total = 0, used = 0, max_used = 0;
    speech_memory_info(&total, &used, &max_used);
    TRACE(4,"SPEECH MALLOC MEM: total - %d, used - %d, max_used - %d.", total, used, max_used);
    ASSERT(used == 0, "[%s] used != 0", __func__);

    return 0;
}
#ifdef XIAOMI_TX_3MIC_ENABLE
// static int16_t pcm_dump[720]={0};
#endif
int speech_tx_process(void *pcm_buf, void *ref_buf, int *pcm_len)
{
	int16_t *buf = (int16_t *)pcm_buf;
    int16_t *ref_dump = (int16_t *)ref_buf;
	int pcm16_len = *pcm_len;
    int tmp_len;
    // TRACE(1,"speech_tx_process pcm_len=%d",pcm16_len);

    int frame_len = *pcm_len/SPEECH_CODEC_CAPTURE_CHANNEL_NUM;
#if defined(SPEECH_TX_24BIT)
    // TRACE(2, "[%s] pcm len = %d channel_num = %d\n", __func__, pcm_len, SPEECH_CODEC_CAPTURE_CHANNEL_NUM);
    arm_q23_to_q15((int32_t *)ref_buf, (int16_t *)ref_buf, frame_len);
    arm_q23_to_q15((int32_t *)pcm_buf, (int16_t *)pcm_buf, frame_len * SPEECH_CODEC_CAPTURE_CHANNEL_NUM);
#endif

#if defined(XIAOMI_TX_3MIC_DUMP_ENABLE)
    int16_t pcm_dump[720];
    audio_dump_clear_up();
    // audio_dump_add_channel_data(0, pcm_buf, frame_len * (SPEECH_CODEC_CAPTURE_CHANNEL_NUM + 1));
    for(int i = 0; i < frame_len; i++)
    {
        pcm_dump[i] = buf[2*i+0]; //ff
        pcm_dump[i+frame_len] = buf[2*i+1]; //fb
        // pcm_dump[i+frame_len*2] = buf[3*i+2]; //ref
        pcm_dump[i+frame_len*2] = ref_dump[i];
    }
    audio_dump_add_channel_data(0, pcm_dump, frame_len);
    audio_dump_add_channel_data(1, pcm_dump+frame_len, frame_len);
    audio_dump_add_channel_data(2, pcm_dump+frame_len*2, frame_len);
#endif

#if defined(XIAOMI_TX_3MIC_TIME_ENABLE)
    //  uint32_t start_ticks, end_ticks;    
    //  start_ticks = hal_fast_sys_timer_get();
#endif

#if defined(XIAOMI_TX_3MIC_BYPASS_ENABLE)  //bypass
    for(int i=0; i<pcm16_len/3; i++)
	{
		// buf[i] = buf[i*3+0];   //fb
		// buf[i] = buf[i*3+1];   //ff
		buf[i] = buf[i*3+2];   //talk
	}
#else  
/**************alg process section************************/
#if (defined(XIAOMI_TX_3MIC_ENABLE) && defined(SCO_CP_ACCEL))
    sco_status_t status = {
        .vad = true,
    };
    tmp_len = pcm16_len;
    // tmp_len = frame_len * SPEECH_CODEC_CAPTURE_CHANNEL_NUM;
    // xiaomi_enc_deal_Tx(buf, buf, ref_dump, tmp_len / 3);
    sco_cp_process(buf, ref_dump, &tmp_len, &status); 
    // memcpy(buf, pcm_dump, sizeof(short) * (tmp_len / 3)); 
#endif
#endif 

    *pcm_len = pcm16_len/SPEECH_CODEC_CAPTURE_CHANNEL_NUM;

#if defined(XIAOMI_TX_3MIC_TIME_ENABLE)
    // end_ticks = hal_fast_sys_timer_get();
    // TRACE(4,"[xiaomi_mcu_process] takes %d us\n", FAST_TICKS_TO_US(end_ticks - start_ticks));
#endif

    // memcpy(buf, pcm_dump, sizeof(short) * (tmp_len / 3)); 

#if defined(XIAOMI_TX_3MIC_DUMP_ENABLE)    
    audio_dump_add_channel_data(3, pcm_buf, frame_len);
    audio_dump_run();
#endif
    return 0;
}

#if defined(SCO_CP_ACCEL)
CP_TEXT_SRAM_LOC
int sco_cp_algo(short *pcm_buf, short *ref_buf, int *_pcm_len, sco_status_t *status)
{
#if defined(SCO_TRACE_CP_ACCEL)
    TRACE(1,"[%s] ...", __func__);
#endif
    // TRACE(1, "[%s] cp vad = %d", __FUNCTION__, status->vad);
    int pcm_len = *_pcm_len / SPEECH_CODEC_CAPTURE_CHANNEL_NUM;
    // TRACE(1, "[%s] pcm_len = %d", __FUNCTION__, pcm_len);
    // xiaomi_enc_cp_process(pcm_buf, pcm_buf, ref_buf, pcm_len);
    xiaomi_enc_deal_Tx(pcm_buf, pcm_buf, ref_buf, pcm_len);
    return 0;
}
#endif

int speech_rx_process(void *pcm_buf, int *pcm_len)
{
    //int16_t *pcm16_buf = (int16_t *)pcm_buf;
    //int pcm16_len = *pcm_len;

    return 0;
}

int speech_tx_process_audio_dump(void *pcm_buf, int *_pcm_len, int channel_num)
{
#if defined(BT_SCO_CHAIN_AUDIO_DUMP)
	//TRACE(1,"[%s] ...channel_num: %d", __func__, channel_num);
	int pcm_len = *_pcm_len;

	for(int i = 0; i < channel_num; i++) {
		audio_dump_add_channel_data_from_multi_channels(i, pcm_buf, pcm_len/channel_num, channel_num, (i%2));
	}
    audio_dump_run();
#endif
    return 0;
}
#if defined(AUDIO_DEBUG)
#include "bt_sco_chain_cfg.h"
#include "bt_sco_chain_tuning.h"

extern const SpeechConfig speech_cfg_default;
static SpeechConfig *speech_cfg = NULL;

int speech_store_config(const SpeechConfig *cfg)
{
    if (speech_cfg) {
        memcpy(speech_cfg, cfg, sizeof(SpeechConfig));
    } else {
        TRACE(1,"[%s] WARNING: Please phone call...", __func__);
    }
    return 0;
}

int speech_set_config(const SpeechConfig *cfg)
{
#if defined(SPEECH_TX_DC_FILTER)
    // speech_dc_filter_set_config(speech_tx_dc_filter_st, &cfg->tx_dc_filter);
#endif
#if defined(SPEECH_RX_COMPEXP)
    // multi_compexp_set_config(speech_rx_compexp_st, &cfg->rx_compexp);
#endif
#if defined(SPEECH_RX_EQ)
    // eq_set_config(speech_rx_eq_st, &cfg->rx_eq);
#endif

    return 0;
}
#endif