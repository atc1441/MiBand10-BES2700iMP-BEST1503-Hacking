#include "audio_prompt_sbc.h"
#include "hal_trace.h"
#include "string.h"
#include "bluetooth_bt_api.h"
#include "app_bt_stream.h"
#include "app_bt_media_manager.h"
#include "app_utils.h"
#include "cmsis.h"
#include "hal_dma.h"
#include "app_bt.h"


#include "bt_drv_interface.h"
#include "audioflinger.h"
#include "app_media_player.h"
#if defined(IBRT)
#include "app_tws_ibrt.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_tws_ctrl_thread.h"
#include "app_ibrt_internal.h"
#endif
#include "app_audio.h"
#include "apps.h"
#include "ae_math.h"
#include "sbc_api.h"

#if BLE_AUDIO_ENABLED
#include "bluetooth_ble_api.h"
#endif

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED

#define AUDIO_PROMPT_RESAMPLE_ITER_NUM               256
#define APP_PROMPT_SBC_SYNC_DELAY_US         (320*1000)
#define SLOT_SIZE 625

#if defined(IBRT)
#ifdef TWS_PROMPT_SYNC
extern void tws_sync_mix_prompt_start_handling(void);
extern void tws_reset_mix_prompt_trigger_ticks(void);
extern void tws_enable_mix_prompt(bool isEnable);
#endif
#endif

extern void app_stop_a2dp_media_stream(uint8_t devId);
extern void app_stop_sco_media_stream(uint8_t devId);
extern void app_bt_stream_copy_track_one_to_two_16bits(int16_t *dst_buf, int16_t *src_buf, uint32_t src_len);
extern void  app_tws_ibrt_audio_mobile_clkcnt_get(uint8_t device_id, uint32_t btclk, uint16_t btcnt,
                                                     uint32_t *mobile_master_clk, uint16_t *mobile_master_cnt);

static uint32_t audio_prompt_sbc_decode(uint8_t* pcm_buffer, uint32_t expectedOutputSize, uint8_t isReset);

static sbc_decoder_t audio_prompt_sbc_decoder;

//static float audio_prompt_sbc_eq_band_gain[8] = {1, 1, 1, 1, 1, 1, 1, 1};

// keep the current volume
#define KEEP_CURRENT_VOLUME_FOR_MIX_PROMPT          -1

// from TGT_VOLUME_LEVEL_T or KEEP_CURRENT_VOLUME_FOR_MIX_PROMPT
#define DEFAULT_VOLUME_FOR_MIX_PROMPT               KEEP_CURRENT_VOLUME_FOR_MIX_PROMPT

#define DEFAULT_COEFF_FOR_MIX_PROMPT_FOR_MUSIC      DB2LIN(codec_dac_vol[MEDIA_VOLUME_LEVEL_WARNINGTONE].sdac_volume)
#define DEFAULT_COEFF_FOR_MIX_MUSIC_FOR_MUSIC       0.4
#define DEFAULT_COEFF_FOR_MIX_PROMPT_FOR_CALL       DB2LIN(codec_dac_vol[MEDIA_VOLUME_LEVEL_WARNINGTONE].sdac_volume)
#define DEFAULT_COEFF_FOR_MIX_CALL_FOR_CALL         0.4

#define DEFAULT_OVERLAP_LENGTH 128

static int audio_prompt_sbc_init_decoder(void)
{
    sbc_decoder_open(&audio_prompt_sbc_decoder);
    return 0;
}

typedef struct
{
    uint32_t    wholeEncodedDataLen;
    uint32_t    leftEncodedDataLen;
    uint8_t*    promptDataBuf;
    uint16_t    tmpSourcePcmDataLen;
    uint16_t    tmpSourcePcmDataOutIndex;
    uint8_t*    tmpSourcePcmDataBuf;
    uint8_t*    tmpTargetPcmDataBuf;
    CQueue      pcmDataQueue;
    uint8_t     isResetDecoder;
    uint8_t     isAudioPromptDecodingDone;
    uint8_t     targetBytesCntPerSample;
    uint32_t    targetSampleRate;
    struct APP_RESAMPLE_T * resampler;
    uint32_t    targetPcmChunkSize;
    float       resampleRatio;
    uint8_t*    bufForResampler;
    uint32_t    resampleBufLen;
    uint8_t     pendingStopOp;
    int8_t      savedStoppedStreamId;
    uint8_t     targetChannelCnt;
    uint8_t     mixType;
    uint16_t    promptId;
    uint8_t     mix_audio_con_id;
    uint32_t    mix_audio_stream_lid;
    void*       mix_audio_stop_func;
    int16_t     volume_level_override;
    uint16_t    promptPram;
    float       coeff_for_mix_prompt_for_music;
    float       coeff_for_mix_music_for_music;
    float       coeff_for_mix_prompt_for_call;
    float       coeff_for_mix_call_for_call;
    uint8_t     isMixPromptOn;
    int16_t     currentVol;
    uint16_t    mergeInOverlapLength;
    uint16_t    mergeOutOverlapLength;
    float       mergeInWeight;
    float       mergeOutWeight;
    float       mergeStep;
} AUDIO_PROMPT_ENV_T;

static AUDIO_PROMPT_ENV_T audio_prompt_env;

static void audio_prompt_set_pending_stop_op(uint8_t op)
{
    audio_prompt_env.pendingStopOp = op;
    TRACE(1,"pendingStopOp is set to %d", op);
}

static uint8_t audio_prompt_get_pending_stop_op(void)
{
    return audio_prompt_env.pendingStopOp;
}

static void audio_prompt_set_saved_stopped_stream_id(int8_t id)
{
    audio_prompt_env.savedStoppedStreamId = id;
    TRACE(1,"savedStoppedStreamId is set to %d", id);
}

static int8_t audio_prompt_get_saved_stopped_stream_id(void)
{
    return audio_prompt_env.savedStoppedStreamId;
}

void audio_prompt_init_handler(void)
{
    memset((uint8_t *)&audio_prompt_env, 0, sizeof(audio_prompt_env));
    audio_prompt_set_saved_stopped_stream_id(-1);
}

bool audio_prompt_is_allow_update_volume(void)
{
    bool isAllow = true;

    uint32_t lock = int_lock();

    isAllow= (!(audio_prompt_env.isMixPromptOn))||
        (KEEP_CURRENT_VOLUME_FOR_MIX_PROMPT ==
        audio_prompt_env.volume_level_override);

    int_unlock(lock);

    return isAllow;
}

bool audio_prompt_is_playing_ongoing(void)
{
    return !!(audio_prompt_env.isMixPromptOn);
}

#ifdef TWS_PROMPT_SYNC

#define MEDIA_SHORT_TRIGGER_DELAY       8
#define MEDIA_LONG_TRIGGER_DELAY        12

#define PROMPT_TICKS_OFFSET_TO_TRIGGER_MIX  10  // 3.25 ms

static uint32_t mix_prompt_trigger_ticks = 0;
static uint32_t playback_interval_in_ticks = 0;
static uint32_t playback_last_irq_ticks = 0;
static uint8_t isStartMixPrompt = false;

static uint32_t get_prompt_trigger_delay(uint16_t promptId)
{
    switch (promptId)
    {
        case AUDIO_ID_BT_GSOUND_MIC_OPEN:
        case AUDIO_ID_BT_GSOUND_MIC_CLOSE:
        // TODO: add more case for specific prompts here if you wanna it played sooner
            return MEDIA_SHORT_TRIGGER_DELAY;
        default:
            return MEDIA_LONG_TRIGGER_DELAY;
    }
}

bool tws_calculate_mix_prompt_trigger_ticks(uint16_t promptId)
{
    if (0 == mix_prompt_trigger_ticks)
    {
        if ((playback_interval_in_ticks > 0)&&(playback_last_irq_ticks > 0))
        {
            uint32_t tick_offset_us = 0;
            uint32_t ticks_offset = 0;
            uint32_t interval_count = 0;

            if (app_tws_ibrt_tws_link_connected())
            {
                ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
                tick_offset_us = APP_PROMPT_SBC_SYNC_DELAY_US;
                
                if(p_ibrt_ctrl->acl_interval > IBRT_UI_DEFAULT_POLL_INTERVAL)
                {
                    TRACE(0, "[MIX][TRIG] add extra delay for tws interval\n");
                    tick_offset_us += p_ibrt_ctrl->acl_interval * SLOT_SIZE;
                }
                ticks_offset = US_TO_BTCLKS(tick_offset_us);
                if(ticks_offset > playback_interval_in_ticks)
                {
                    interval_count = (ticks_offset / playback_interval_in_ticks);
                    if(ticks_offset % playback_interval_in_ticks)
                    {
                        interval_count += 1;
                    }
                    mix_prompt_trigger_ticks = playback_last_irq_ticks + interval_count * playback_interval_in_ticks;
                    // TRACE(2, "[MIX][TRIG] ticks_offset:%d, interval_count:%d", ticks_offset, interval_count);
                }
                else
                {
                    mix_prompt_trigger_ticks = playback_last_irq_ticks+
                        get_prompt_trigger_delay(promptId)*playback_interval_in_ticks;
                }
            }
            else
            {
                mix_prompt_trigger_ticks = playback_last_irq_ticks+
                    get_prompt_trigger_delay(promptId)*playback_interval_in_ticks;
            }
            // TRACE(1, "[MIX][TRIG] playback_last_irq_ticks %d", playback_last_irq_ticks);
            // TRACE(2, "[MIX][TRIG] playback_interval_in_ticks %d mix_prompt_trigger_ticks %d", playback_interval_in_ticks, mix_prompt_trigger_ticks);
            return true;
        }
    }

    return false;
}

void tws_enable_mix_prompt(bool isEnable)
{
    // TRACE("isStartMixPrompt to %d.", isEnable);
    isStartMixPrompt = isEnable;
}

void tws_set_mix_prompt_trigger_ticks(uint32_t ticks)
{
    mix_prompt_trigger_ticks = ticks;
}

void tws_sync_mix_prompt_start_handling(void)
{
    if (!app_tws_ibrt_tws_link_connected())
    {
        tws_enable_mix_prompt(true);
    }
    else
    {
        tws_enable_mix_prompt(false);
    }
}

bool tws_is_mix_prompt_allowed_to_start(void)
{
    return isStartMixPrompt;
}

void app_ibrt_send_mix_prompt_req(uint8_t device_id)
{
    if (app_tws_ibrt_tws_link_connected())
    {    
        TRACE(0, "send mx prompt req");
        APP_TWS_CMD_MIX_PROMPT_SYNC_T req;
        req.promptId = audio_prompt_get_prompt_id();
        req.promptPram = audio_prompt_env.promptPram;
        req.trigger_time = mix_prompt_trigger_ticks;
        req.sampleRate = audio_prompt_get_sample_rate();
        req.remote = app_bt_get_device(device_id)->remote;
        voice_report_role_t report_role = app_ibrt_voice_report_get_role();
        if (VOICE_REPORT_MASTER == report_role)
        {
            tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_MIX_PROMPT_REQ,(uint8_t*)&req,sizeof(APP_TWS_CMD_MIX_PROMPT_SYNC_T));
        }
    }
}

bool tws_sync_mix_prompt_handling(uint8_t device_id)
{
    if (!tws_is_mix_prompt_allowed_to_start())
    {
        if (tws_calculate_mix_prompt_trigger_ticks(audio_prompt_get_prompt_id()))
        {
            // get the trigger ticks, send request to slave
            app_ibrt_send_mix_prompt_req(device_id);
        }
        return false;
    }
    else
    {
        return true;
    }
}

uint32_t tws_get_mix_prompt_trigger_ticks(void)
{
    return mix_prompt_trigger_ticks;
}

void tws_reset_mix_prompt_trigger_ticks(void)
{
    mix_prompt_trigger_ticks = 0;
    playback_interval_in_ticks = 0;
    playback_last_irq_ticks = 0;
    isStartMixPrompt = false;
}

void tws_playback_ticks_check_for_mix_prompt(uint8_t device_id)
{
    if ((!audio_prompt_is_playing_ongoing()) || tws_is_mix_prompt_allowed_to_start())
    {
        //TRACE(0,"check for mix prompt<1>");
        return;
    }

    uint32_t btclk;
    uint16_t btcnt;
    uint32_t master_btclk = 0;
    uint16_t master_btcnt = 0;
    POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (!(APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote)) && !(APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote)))
    {
        TRACE(0,"check for mix prompt<2>");
        tws_enable_mix_prompt(true);
        return;
    }

#if defined(MULTI_DMA_TC)
    uint8_t adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    uint32_t dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    if(adma_ch != HAL_DMA_CHAN_NONE)
    {
        bt_drv_reg_op_dma_tc_clkcnt_get_by_ch(&btclk, &btcnt,adma_ch&0xFF, dma_base);
    }
#else
    bt_drv_reg_op_dma_tc_clkcnt_get(&btclk, &btcnt);
#endif

    app_tws_ibrt_audio_mobile_clkcnt_get(device_id, btclk, btcnt, &master_btclk, &master_btcnt);

    if (mix_prompt_trigger_ticks > 0)
    {
        if (master_btclk >= mix_prompt_trigger_ticks)
        {
            TRACE(2,"ticks<1> %d - trigger ticks %d", master_btclk,mix_prompt_trigger_ticks);
            tws_enable_mix_prompt(true);
        }
        else if ((master_btclk < mix_prompt_trigger_ticks) &&
                ((mix_prompt_trigger_ticks - master_btclk) < PROMPT_TICKS_OFFSET_TO_TRIGGER_MIX))
        {
            TRACE(2,"ticks<2> %d - trigger ticks %d", master_btclk,mix_prompt_trigger_ticks);
            tws_enable_mix_prompt(true);
        }
    }

    if (0 != playback_last_irq_ticks)
    {
       playback_interval_in_ticks = master_btclk-playback_last_irq_ticks;
    }

    playback_last_irq_ticks = master_btclk;
}

void app_tws_cmd_sync_mix_prompt_req_handler(uint8_t* ptrParam, uint16_t paramLen)
{
    uint16_t promptPram = 0xFFFF;
    uint32_t curr_ticks = 0xFFFFFFFF;

    APP_TWS_CMD_MIX_PROMPT_SYNC_T* pReq = (APP_TWS_CMD_MIX_PROMPT_SYNC_T *)ptrParam;
    TRACE(2,"promptId, trigger_time:0x%x, %d",
        pReq->promptId, pReq->trigger_time);

    audio_prompt_stop_playing();

    if (APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&pReq->remote))
    {
        curr_ticks = bt_syn_get_curr_ticks(APP_IBRT_UI_GET_IBRT_HANDLE(&pReq->remote));
        TRACE(2, "[%s] curr_ticks=%d", __func__, curr_ticks);
    }
    else if (APP_IBRT_MOBILE_LINK_CONNECTED(&pReq->remote))
    {
        curr_ticks = bt_syn_get_curr_ticks(APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&pReq->remote));
        TRACE(2, "[%s] curr_ticks=%d", __func__, curr_ticks);
    }

    voice_report_role_t report_role = app_ibrt_voice_report_get_role();        

    if ((VOICE_REPORT_SLAVE != report_role) ||
        ((curr_ticks > pReq->trigger_time) &&
         ((curr_ticks - pReq->trigger_time) > PROMPT_TICKS_OFFSET_TO_TRIGGER_MIX)))
    {
        TRACE(0,"return directly.");
        return;
    }

    if((!bt_a2dp_is_run()) && (bt_media_is_music_media_active()))
    {
        TRACE(0,"sbc_player off, return directly.");
        return;
    }

    tws_set_mix_prompt_trigger_ticks(pReq->trigger_time);
    promptPram = pReq->promptId | pReq->promptPram;
    audio_prompt_start_playing(promptPram, pReq->sampleRate);
    if (curr_ticks >= pReq->trigger_time)
    {
        TRACE(1,"Instant passed %d",curr_ticks);
        tws_enable_mix_prompt(true);
    }
    else if ((curr_ticks < pReq->trigger_time) &&
        ((pReq->trigger_time - curr_ticks) < PROMPT_TICKS_OFFSET_TO_TRIGGER_MIX)){
        TRACE(1,"Instant near %d",curr_ticks);
        tws_enable_mix_prompt(true);
    }
}
#endif

static int audio_prompt_resample_iter(uint8_t *buf, uint32_t len)
{
    if (!buf)
    {
        TRACE(0, "NULL pointer received in %s", __func__);
        return -1;
    }

    uint32_t leftLen = audio_prompt_env.tmpSourcePcmDataLen - audio_prompt_env.tmpSourcePcmDataOutIndex;
    uint32_t lenToFetch;

    if (leftLen >= len)
    {
        lenToFetch = len;
    }
    else
    {
        lenToFetch = leftLen;
    }

    memcpy(buf, audio_prompt_env.tmpSourcePcmDataBuf + audio_prompt_env.tmpSourcePcmDataOutIndex,
        lenToFetch);
    audio_prompt_env.tmpSourcePcmDataOutIndex += lenToFetch;

    memset(buf + lenToFetch, 0, len - lenToFetch);

    return 0;
}

void audio_prompt_buffer_config(uint8_t mixType,
                                uint8_t channel_cnt,
                                uint8_t bitNumPerSample,
                                uint8_t *tmpSourcePcmDataBuf,
                                uint8_t *tmpTargetPcmDataBuf,
                                uint8_t *pcmDataBuf,
                                uint32_t pcmBufLen,
                                uint8_t *bufForResampler,
                                uint32_t resampleBufLen)
{
    uint32_t lock = int_lock();
    audio_prompt_env.mixType = mixType;
    audio_prompt_env.targetChannelCnt = channel_cnt;
    if (24 == bitNumPerSample)
    {
        audio_prompt_env.targetBytesCntPerSample = 4;
    }
    else if (16 == bitNumPerSample)
    {
        audio_prompt_env.targetBytesCntPerSample = 2;
    }
    else
    {
        ASSERT(false, "bitNumPerSample %d is not supported by prompt mixer yet!", bitNumPerSample);
    }

    audio_prompt_env.tmpSourcePcmDataBuf = tmpSourcePcmDataBuf;
    audio_prompt_env.tmpTargetPcmDataBuf = tmpTargetPcmDataBuf;
    audio_prompt_env.bufForResampler = bufForResampler;
    audio_prompt_env.resampleBufLen = resampleBufLen;

    InitCQueue(&(audio_prompt_env.pcmDataQueue), pcmBufLen, (CQItemType *)(pcmDataBuf));
    int_unlock(lock);
}


uint16_t audio_prompt_get_prompt_id(void)
{
    return audio_prompt_env.promptId;
}

uint32_t audio_prompt_get_sample_rate(void)
{
    return audio_prompt_env.targetSampleRate;
}

bool audio_prompt_start_playing(uint16_t promptPram, uint32_t targetSampleRate)
{
    uint16_t promptId = PROMPT_ID_FROM_ID_VALUE(promptPram);

    if (audio_prompt_is_playing_ongoing())
    {
#if defined(PROMPT_CTRL_SWITCH_ANC_MODE)
        extern uint16_t app_prompt_ctrl_anc_init(uint16_t aud_id);
        extern void app_prompt_ctrl_anc_process(void);
        app_prompt_ctrl_anc_init(promptPram);
        app_prompt_ctrl_anc_process();
#endif
        return false;
    }

    TRACE(0, "[%s]", __func__);

#ifdef TWS_PROMPT_SYNC
    bool isPlayingLocally = false;
    if (app_tws_ibrt_tws_link_connected())
    {
        isPlayingLocally = false;
    }
    else
    {
        isPlayingLocally = true;
    }
#endif

    promptId = PROMPT_ID_FROM_ID_VALUE(promptId);

    uint32_t lock = int_lock();
    audio_prompt_env.isMixPromptOn = true;
    ResetCQueue(&(audio_prompt_env.pcmDataQueue));

    uint8_t* promptDataPtr = NULL;
    uint32_t promptDataLen = 0;
    PROMPT_MIX_PROPERTY_T* pPromptProperty = NULL;

#ifdef MEDIA_PLAYER_SUPPORT
#if defined(PROMPT_CTRL_SWITCH_ANC_MODE)
    media_runtime_audio_prompt_update(promptPram, &promptDataPtr, &promptDataLen);
#else
    media_runtime_audio_prompt_update(promptId, &promptDataPtr, &promptDataLen);
#endif
    pPromptProperty = get_prompt_mix_property(promptId);
#endif

    if (NULL == pPromptProperty)
    {
        TRACE(0,"use default mix property");

        // use default property
        audio_prompt_env.volume_level_override = DEFAULT_VOLUME_FOR_MIX_PROMPT;

        audio_prompt_env.coeff_for_mix_prompt_for_call = DEFAULT_COEFF_FOR_MIX_PROMPT_FOR_CALL;
        audio_prompt_env.coeff_for_mix_call_for_call = DEFAULT_COEFF_FOR_MIX_CALL_FOR_CALL;
        audio_prompt_env.coeff_for_mix_prompt_for_music = DEFAULT_COEFF_FOR_MIX_PROMPT_FOR_MUSIC;
        audio_prompt_env.coeff_for_mix_music_for_music = DEFAULT_COEFF_FOR_MIX_MUSIC_FOR_MUSIC;
    }
    else
    {
        audio_prompt_env.volume_level_override =
            pPromptProperty->volume_level_override;

        audio_prompt_env.coeff_for_mix_prompt_for_call =
            pPromptProperty->coeff_for_mix_prompt_for_call;
        audio_prompt_env.coeff_for_mix_call_for_call =
            pPromptProperty->coeff_for_mix_call_for_call;
        audio_prompt_env.coeff_for_mix_prompt_for_music =
            pPromptProperty->coeff_for_mix_prompt_for_music;
        audio_prompt_env.coeff_for_mix_music_for_music =
            pPromptProperty->coeff_for_mix_music_for_music;
    }

    if (KEEP_CURRENT_VOLUME_FOR_MIX_PROMPT != audio_prompt_env.volume_level_override)
    {
        // if the prompt's volume is smaller than the current volume, don't change it
        if (audio_prompt_env.volume_level_override <= app_bt_stream_local_volume_get())
        {
            audio_prompt_env.volume_level_override = KEEP_CURRENT_VOLUME_FOR_MIX_PROMPT;
        }
    }

    audio_prompt_env.promptId = promptId;
    audio_prompt_env.promptPram = PROMPT_PRAM_FROM_ID_VALUE(promptPram);
    audio_prompt_env.promptDataBuf = promptDataPtr;
    audio_prompt_env.isResetDecoder = true;
    audio_prompt_env.isAudioPromptDecodingDone = false;
    audio_prompt_env.wholeEncodedDataLen = promptDataLen;
    audio_prompt_env.leftEncodedDataLen = audio_prompt_env.wholeEncodedDataLen;
    audio_prompt_env.targetSampleRate = targetSampleRate;

    audio_prompt_env.resampleRatio = ((float)AUDIO_PROMPT_SBC_SAMPLE_RATE_VALUE) / audio_prompt_env.targetSampleRate;
    audio_prompt_env.targetPcmChunkSize = (uint32_t)(AUDIO_PROMPT_SBC_PCM_DATA_SIZE_PER_FRAME/audio_prompt_env.resampleRatio);

    audio_prompt_env.resampler = app_playback_resample_any_open_with_pre_allocated_buffer(( enum AUD_CHANNEL_NUM_T )AUDIO_PROMPT_SBC_CHANNEL_COUNT,
                                                                                          audio_prompt_resample_iter,
                                                                                          AUDIO_PROMPT_RESAMPLE_ITER_NUM,
                                                                                          audio_prompt_env.resampleRatio,
                                                                                          audio_prompt_env.bufForResampler,
                                                                                          audio_prompt_env.resampleBufLen);

    // audio resample out size should be even
    uint16_t targetOverlapLength = ((uint16_t)(DEFAULT_OVERLAP_LENGTH / audio_prompt_env.resampleRatio) / 2 * 2);
    audio_prompt_env.mergeInOverlapLength = targetOverlapLength * audio_prompt_env.targetChannelCnt;
    audio_prompt_env.mergeOutOverlapLength = targetOverlapLength * audio_prompt_env.targetChannelCnt;
    audio_prompt_env.mergeInWeight = 0.f;
    audio_prompt_env.mergeOutWeight = 1.f;
    audio_prompt_env.mergeStep = 1.f / (targetOverlapLength - 1);

    audio_prompt_set_saved_stopped_stream_id(-1);

    int_unlock(lock);

    TRACE(1,"start audio prompt. target sample rate %d", targetSampleRate);

    app_sysfreq_req(APP_SYSFREQ_USER_PROMPT_MIXER, APP_SYSFREQ_104M);

#ifdef TWS_PROMPT_SYNC
    if (!isPlayingLocally)
    {
        tws_sync_mix_prompt_start_handling();
    }
    else
    {
        tws_enable_mix_prompt(true);
    }
#endif

    app_prompt_start_callback(audio_prompt_get_prompt_id());

    return true;
}

void audio_prompt_forcefully_stop(void)
{
    app_playback_resample_close(audio_prompt_env.resampler);
    audio_prompt_set_saved_stopped_stream_id(-1);
    audio_prompt_env.isAudioPromptDecodingDone  = true;
    audio_prompt_env.leftEncodedDataLen = 0;
    app_sysfreq_req(APP_SYSFREQ_USER_PROMPT_MIXER, APP_SYSFREQ_32K);
#if defined(IBRT) && defined(MEDIA_PLAYER_SUPPORT)
    app_tws_sync_prompt_check();
#endif
}

bool audio_prompt_check_on_stopping_ble_audio_stream(uint8_t pendingStopOp, uint8_t con_id,
                                                     uint32_t stream_lid, void *mix_audio_stop_func)
{
    uint32_t lock = int_lock();
    if (audio_prompt_env.isMixPromptOn)
    {
        TRACE(1,"Prompt mixing ongoing, pending op:%d", pendingStopOp);
        audio_prompt_env.pendingStopOp              = pendingStopOp;
        audio_prompt_env.mix_audio_con_id           = con_id;
        audio_prompt_env.mix_audio_stream_lid     = stream_lid;
        audio_prompt_env.mix_audio_stop_func        = mix_audio_stop_func;
        audio_prompt_set_saved_stopped_stream_id(0);
        int_unlock(lock);
        return false;
    }

    int_unlock(lock);
    return true;
}

bool audio_prompt_check_on_stopping_stream(uint8_t pendingStopOp, uint8_t deviceId)
{
    uint32_t lock = int_lock();
    if (audio_prompt_env.isMixPromptOn)
    {
        if (bt_is_playback_triggered())
        {
            TRACE(1,"Prompt mixing ongoing, pending op:%d", pendingStopOp);
            audio_prompt_env.pendingStopOp        = pendingStopOp;
            audio_prompt_set_saved_stopped_stream_id(deviceId);
            int_unlock(lock);
            return false;
        }
        else
        {
            int_unlock(lock);
            audio_prompt_stop_playing();
            return true;
        }
    }

    int_unlock(lock);
    return true;
}

bool audio_prompt_clear_pending_stream(uint8_t op)
{
    bool isToClearActiveMedia = false;

    TRACE(4,
          "%s stop_id %d pendingStopOp %d op %d",
          __func__,
          audio_prompt_get_saved_stopped_stream_id(),
          audio_prompt_env.pendingStopOp,
          op);

    if (-1 != audio_prompt_get_saved_stopped_stream_id())
    {
        uint32_t lock = int_lock();
        if ((PENDING_TO_STOP_A2DP_STREAMING == op) &&
            (PENDING_TO_STOP_A2DP_STREAMING == audio_prompt_get_pending_stop_op()))
        {
            audio_prompt_set_saved_stopped_stream_id(-1);
            audio_prompt_set_pending_stop_op(PENDING_TO_STOP_STREAM_INVALID);
            isToClearActiveMedia = true;
        }
        else if ((PENDING_TO_STOP_SCO_STREAMING == op) &&
                 (PENDING_TO_STOP_SCO_STREAMING == audio_prompt_get_pending_stop_op()))
        {
            audio_prompt_set_saved_stopped_stream_id(-1);
            audio_prompt_set_pending_stop_op(PENDING_TO_STOP_STREAM_INVALID);
            isToClearActiveMedia = true;
        }
        int_unlock(lock);
    }

    return isToClearActiveMedia;
}

void audio_prompt_stop_playing(void)
{
    if (!audio_prompt_env.isMixPromptOn)
    {
        return;
    }

    TRACE(0,"Stop audio prompt.");

    app_playback_resample_close(audio_prompt_env.resampler);

    uint32_t lock = int_lock();

    audio_prompt_env.leftEncodedDataLen = 0;
    audio_prompt_env.isMixPromptOn = false;

#ifdef TWS_PROMPT_SYNC
    tws_reset_mix_prompt_trigger_ticks();
#endif

    int_unlock(lock);

    if (KEEP_CURRENT_VOLUME_FOR_MIX_PROMPT != audio_prompt_env.volume_level_override)
    {
        // restore the volume
        app_bt_stream_volumeset_handler(app_bt_stream_local_volume_get());
    }

    uint8_t pendingStopOp;
    int8_t savedStoppedStreamId;
    uint8_t con_id;
    uint32_t context_type;

    lock = int_lock();

    pendingStopOp = audio_prompt_env.pendingStopOp;
    savedStoppedStreamId = audio_prompt_get_saved_stopped_stream_id();

    con_id = audio_prompt_env.mix_audio_con_id;
    context_type = audio_prompt_env.mix_audio_stream_lid;
    void (*p_mix_audio_stop_func)(bool, uint8_t, uint32_t) = (void (*)(bool, uint8_t, uint32_t))audio_prompt_env.mix_audio_stop_func;

    audio_prompt_set_saved_stopped_stream_id(-1);

    int_unlock(lock);

    if (savedStoppedStreamId >= 0)
    {
        if (PENDING_TO_STOP_A2DP_STREAMING == pendingStopOp)
        {
            TRACE(0,"Stop the pending stopped a2dp media stream.");
            app_stop_a2dp_media_stream(savedStoppedStreamId);
        }
        else if (PENDING_TO_STOP_SCO_STREAMING == pendingStopOp)
        {
            TRACE(0,"Stop the pending stopped sco media stream.");
            app_stop_sco_media_stream(savedStoppedStreamId);
        }
        else if (PENDING_TO_STOP_BLE_AUDIO_STREAMING == pendingStopOp)
        {
            p_mix_audio_stop_func(false, con_id, context_type);
        }
        else if (PENDING_TO_STOP_BLE_AUDIO_SINGLE_STREAMING == pendingStopOp)
        {
            p_mix_audio_stop_func(true, con_id, context_type);
        }
    }

    if(audio_prompt_env.mix_audio_stop_func)
    {
        mix_audio_stop_func_t func_handler = (mix_audio_stop_func_t  )audio_prompt_env.mix_audio_stop_func;
        func_handler(audio_prompt_env.pendingStopOp,audio_prompt_env.mix_audio_con_id,audio_prompt_env.mix_audio_stream_lid);

        audio_prompt_env.pendingStopOp = 0;
        audio_prompt_env.mix_audio_con_id = 0;
        audio_prompt_env.mix_audio_stream_lid = 0;
        audio_prompt_env.mix_audio_stop_func = 0;
    }

    app_prompt_finish_callback(audio_prompt_get_prompt_id());

    app_sysfreq_req(APP_SYSFREQ_USER_PROMPT_MIXER, APP_SYSFREQ_32K);

#if defined(IBRT) && defined(MEDIA_PLAYER_SUPPORT)
    app_tws_sync_prompt_check();
#endif
    //check if there is any pending prompt need to play
    APP_AUDIO_STATUS status_next = {0};
    APP_AUDIO_STATUS aud_status = {0};
    aud_status.id = APP_PLAY_BACK_AUDIO;
    if (app_audio_list_rmv_callback(&aud_status, &status_next,APP_BT_SETTING_Q_POS_HEAD))
    {
        TRACE(4,"%s next id: 0x%x%s, aud_id %x", __func__, status_next.id, player2str(status_next.id), status_next.aud_id);
#if defined(IBRT)
        if (status_next.id == APP_PLAY_BACK_AUDIO)
        {
            app_ibrt_if_voice_report_handler(status_next.aud_id, 0, aud_status.device_id);
        }
#endif
    }

    app_prompt_inform_completed_event();
}

template<typename DstType>
static inline DstType prompt_data_ssat(int32_t in)
{
    DstType out;

    if (sizeof(DstType) == 2) {
        out = __SSAT(in, 16);
    } else {
        out = __SSAT(in, 24);
    }

    return out;
}

template<typename DstType>
static inline DstType prompt_data_extend(int16_t in)
{
    DstType out;

    if (sizeof(DstType) == 2) {
        out = in;
    } else {
        out = (in << 8);
    }

    return out;
}

static bool isUpdateDstBuf = true;

template<typename DstType>
static void audio_prompt_crossfade(DstType *dst_buf, DstType *src_buf1, int16_t *src_buf0,
                                   float coeff_for_source, float coeff_for_prompt,
                                   uint32_t merge_in_end, uint32_t merge_out_start, uint32_t src_len)
{
    uint32_t i = 0;
    if (audio_prompt_env.targetChannelCnt == 2) {
        float weight = audio_prompt_env.mergeInWeight;
        for (; i < merge_in_end; i += 2) {
            /* TODO: increase coeff by step instead of calcualted every step */
            float coeff0 = 1 - weight + weight * coeff_for_source;
            float coeff1 = weight * coeff_for_prompt;

            float tmp0 = coeff0 * src_buf1[i] + coeff1 * prompt_data_extend<DstType>(src_buf0[i]);
            float tmp1 = coeff0 * src_buf1[i + 1] + coeff1 * prompt_data_extend<DstType>(src_buf0[i + 1]);
            if (isUpdateDstBuf)
            {
                dst_buf[i] = prompt_data_ssat<DstType>((int32_t)tmp0);
                dst_buf[i + 1] = prompt_data_ssat<DstType>((int32_t)tmp1);
            }
            weight += audio_prompt_env.mergeStep;
        }
        audio_prompt_env.mergeInOverlapLength -= merge_in_end;
        audio_prompt_env.mergeInWeight = weight;
    } else if (audio_prompt_env.targetChannelCnt == 1) {
        float weight = audio_prompt_env.mergeInWeight;
        for (; i < merge_in_end; i++) {
            float coeff0 = 1 - weight + weight * coeff_for_source;
            float coeff1 = weight * coeff_for_prompt;

            float tmp0 = coeff0 * src_buf1[i] + coeff1 * prompt_data_extend<DstType>(src_buf0[i]);
            if (isUpdateDstBuf)
            {
                dst_buf[i] = prompt_data_ssat<DstType>((int32_t)tmp0);
            }
            weight += audio_prompt_env.mergeStep;
        }
        audio_prompt_env.mergeInOverlapLength -= merge_in_end;
        audio_prompt_env.mergeInWeight = weight;
    } else {
        ASSERT(0, "[%s] channel number %d not supported", __FUNCTION__, audio_prompt_env.targetChannelCnt);
    }
    for (; i < merge_out_start; i++) {
        float mix_value = coeff_for_prompt * prompt_data_extend<DstType>(src_buf0[i]) +
                          coeff_for_source * src_buf1[i];
        if (isUpdateDstBuf)
        {
            dst_buf[i] = prompt_data_ssat<DstType>((int32_t)mix_value);
        }
    }
    if (audio_prompt_env.targetChannelCnt == 2) {
        float weight = audio_prompt_env.mergeOutWeight;
        for (; i < src_len; i += 2) {
            float coeff0 = (1 - weight + weight * coeff_for_source);
            float coeff1 = weight * coeff_for_prompt;

            float tmp0 = coeff0 * src_buf1[i] + coeff1 * prompt_data_extend<DstType>(src_buf0[i]);
            float tmp1 = coeff0 * src_buf1[i + 1] + coeff1 * prompt_data_extend<DstType>(src_buf0[i + 1]);
            if (isUpdateDstBuf)
            {
                dst_buf[i] = prompt_data_ssat<DstType>((int32_t)tmp0);
                dst_buf[i + 1] = prompt_data_ssat<DstType>((int32_t)tmp1);
            }
            weight -= audio_prompt_env.mergeStep;
        }
        audio_prompt_env.mergeOutOverlapLength -= src_len - merge_out_start;
        audio_prompt_env.mergeOutWeight = weight;
    } else if (audio_prompt_env.targetChannelCnt == 1) {
        float weight = audio_prompt_env.mergeOutWeight;
        for (; i < src_len; i++) {
            float coeff0 = (1 - weight + weight * coeff_for_source);
            float coeff1 = weight * coeff_for_prompt;

            float tmp0 = coeff0 * src_buf1[i] + coeff1 * prompt_data_extend<DstType>(src_buf0[i]);
            if (isUpdateDstBuf)
            {
                dst_buf[i] = prompt_data_ssat<DstType>((int32_t)tmp0);
            }
            weight -= audio_prompt_env.mergeStep;
        }
        audio_prompt_env.mergeOutOverlapLength -= src_len - merge_out_start;
        audio_prompt_env.mergeOutWeight = weight;
    } else {
        ASSERT(0, "[%s] channel number %d not supported", __FUNCTION__, audio_prompt_env.targetChannelCnt);
    }
}

static void audio_prompt_processing_handler_func(uint32_t acquiredPcmDataLen, uint8_t* pcmDataToMerge)
{
    if(audio_prompt_env.isMixPromptOn ==  false){
        TRACE(0, "[%s] WARNING: Mix is off", __func__);
        return;
    }

#ifdef TWS_PROMPT_SYNC
    uint16_t prompt_chnlsel = PROMPT_CHNLSEl_FROM_ID_VALUE(audio_prompt_env.promptPram);
#endif

    uint32_t lock = int_lock();
	
    uint32_t pcmDataToGetFromPrompt = acquiredPcmDataLen/(audio_prompt_env.targetChannelCnt*audio_prompt_env.targetBytesCntPerSample/2);

    while ((uint32_t)LengthOfCQueue(&(audio_prompt_env.pcmDataQueue)) < pcmDataToGetFromPrompt)
    {
        if (audio_prompt_env.isAudioPromptDecodingDone)
        {
            break;
        }

        // decode the audio prompt
        uint32_t returnedPcmDataLen = audio_prompt_sbc_decode(audio_prompt_env.tmpSourcePcmDataBuf,
                                                              AUDIO_PROMPT_SBC_PCM_DATA_SIZE_PER_FRAME,
                                                              audio_prompt_env.isResetDecoder);

        if (returnedPcmDataLen < AUDIO_PROMPT_SBC_PCM_DATA_SIZE_PER_FRAME)
        {
            audio_prompt_env.isAudioPromptDecodingDone = true;
        }

        audio_prompt_env.isResetDecoder = false;
        audio_prompt_env.tmpSourcePcmDataLen = returnedPcmDataLen;
        audio_prompt_env.tmpSourcePcmDataOutIndex = 0;

        // do resmpling
        if (audio_prompt_env.targetSampleRate != AUDIO_PROMPT_SBC_SAMPLE_RATE_VALUE)
        {
            uint32_t targetPcmSize = returnedPcmDataLen;
            if (AUDIO_PROMPT_SBC_PCM_DATA_SIZE_PER_FRAME == returnedPcmDataLen)
            {
                targetPcmSize = audio_prompt_env.targetPcmChunkSize;
            }
            else
            {
                targetPcmSize = (uint32_t)(returnedPcmDataLen/audio_prompt_env.resampleRatio);
            }

            targetPcmSize = (targetPcmSize/4)*4;

            app_playback_resample_run(audio_prompt_env.resampler,
                                      audio_prompt_env.tmpTargetPcmDataBuf,
                                      targetPcmSize);

            // fill into pcm data queue
            EnCQueue(&(audio_prompt_env.pcmDataQueue), audio_prompt_env.tmpTargetPcmDataBuf, targetPcmSize);
        }
        else
        {
            EnCQueue(&(audio_prompt_env.pcmDataQueue), audio_prompt_env.tmpSourcePcmDataBuf, returnedPcmDataLen);
        }
    }

    uint32_t pcmDataLenToMerge = pcmDataToGetFromPrompt;
    if ((uint32_t)LengthOfCQueue(&(audio_prompt_env.pcmDataQueue)) < pcmDataToGetFromPrompt)
    {
        pcmDataLenToMerge = LengthOfCQueue(&(audio_prompt_env.pcmDataQueue));
    }

    if (pcmDataLenToMerge == 0) {
        goto exit;
    }

    // copy to multiple channel if needed
    if (audio_prompt_env.targetChannelCnt > 1)
    {
        // get the data
        DeCQueue(&(audio_prompt_env.pcmDataQueue), audio_prompt_env.tmpSourcePcmDataBuf, pcmDataLenToMerge);
        app_bt_stream_copy_track_one_to_two_16bits(( int16_t * )audio_prompt_env.tmpTargetPcmDataBuf,
                                                   ( int16_t * )audio_prompt_env.tmpSourcePcmDataBuf,
                                                   pcmDataLenToMerge / sizeof(uint16_t));
    }
    else
    {
        DeCQueue(&(audio_prompt_env.pcmDataQueue), audio_prompt_env.tmpTargetPcmDataBuf, pcmDataLenToMerge);
    }

#ifdef TWS_PROMPT_SYNC
    if (IS_PROMPT_CHNLSEl_ALL(prompt_chnlsel) || app_ibrt_voice_report_is_me(prompt_chnlsel))
    {
        isUpdateDstBuf = true;
    }
    else
    {
        isUpdateDstBuf = false;
    }    
#endif
    {
        // merge the data
        int16_t *src_buf0 = (int16_t *)audio_prompt_env.tmpTargetPcmDataBuf;
        uint32_t src_len = pcmDataLenToMerge*audio_prompt_env.targetChannelCnt/sizeof(uint16_t);

        float coeff_for_prompt;
        float coeff_for_source;

        if (MIX_WITH_A2DP_STREAMING == audio_prompt_env.mixType ||
            MIX_WITH_BLE_AUDIO_MEDIA_STREAMING == audio_prompt_env.mixType) {
            coeff_for_prompt = audio_prompt_env.coeff_for_mix_prompt_for_music;
            coeff_for_source = audio_prompt_env.coeff_for_mix_music_for_music;
        } else {
            coeff_for_prompt = audio_prompt_env.coeff_for_mix_prompt_for_call;
            coeff_for_source = audio_prompt_env.coeff_for_mix_call_for_call;
        }

        /*
         * 0 --------------- merge_in_end --- merge_out_start --- src_len
         * |---- merge in ---------|---- merge ------|----- merge out ---|
         * Split data into three segments, merge in stands for merge begin crossfade, merge out for merge end crossfade.
         * In merge begin crossfade stage, old data is music/sco pcm data, new data is merged data.
         * In merge end crossfade stage, old data is merged data, new data is music/sco data.
         * Assume music/soc is a, ring is b, then merged data is x = m * a + p * b,
         * merge begin final data y = (1 - w) * a + w * x = (1 - w + w * m) * a + w * p * b for w = 0:step:1,
         * merge end final data y = (1 - w) * a + w * x = (1 - w + w * m) * a + w * p * b for w = 1:-step:0.
         */
        uint32_t merge_in_end = 0;
        if (audio_prompt_env.mergeInOverlapLength > 0) {
            TRACE(3, "[%s] bytes_per_sample %d, channel_num %d", __FUNCTION__,
                  audio_prompt_env.targetBytesCntPerSample, audio_prompt_env.targetChannelCnt);
            TRACE(2, "[%s] merge start, remain %d", __FUNCTION__, audio_prompt_env.mergeInOverlapLength);
            merge_in_end = MIN(audio_prompt_env.mergeInOverlapLength, src_len);
        }

        uint32_t merge_out_start = src_len;
        /* TODO: calc remain decoded pcm samples for DEFAULT_OVERLAP_LENGTH > 128 */
        if (audio_prompt_env.leftEncodedDataLen == 0) {
            if (LengthOfCQueue(&(audio_prompt_env.pcmDataQueue)) < audio_prompt_env.mergeOutOverlapLength / audio_prompt_env.targetChannelCnt * (int32_t)sizeof(uint16_t)) {
                TRACE(2, "[%s] merge end, remain %d", __FUNCTION__, audio_prompt_env.mergeOutOverlapLength);
                merge_out_start = src_len - (audio_prompt_env.mergeOutOverlapLength - LengthOfCQueue(&(audio_prompt_env.pcmDataQueue)) / sizeof(uint16_t) * audio_prompt_env.targetChannelCnt);
            }
        }

        //TRACE(3, "[%s] merge_in_end = %d, merge_out_start = %d", __FUNCTION__, merge_in_end, merge_out_start);
        //TRACE(3, "[%s] pcm queue size = %d, src_len = %d", __FUNCTION__, LengthOfCQueue(&(audio_prompt_env.pcmDataQueue)), src_len);

        if (2 == audio_prompt_env.targetBytesCntPerSample)
        {
            int16_t *src_buf1 = (int16_t *)pcmDataToMerge;
            int16_t *dst_buf = (int16_t *)pcmDataToMerge;

            audio_prompt_crossfade<int16_t>(dst_buf, src_buf1, src_buf0,
                                            coeff_for_source, coeff_for_prompt,
                                            merge_in_end, merge_out_start, src_len);
        }
        else if (4 == audio_prompt_env.targetBytesCntPerSample)
        {
            int32_t *src_buf1 = (int32_t *)pcmDataToMerge;
            int32_t *dst_buf = (int32_t *)pcmDataToMerge;

            audio_prompt_crossfade<int32_t>(dst_buf, src_buf1, src_buf0,
                                            coeff_for_source, coeff_for_prompt,
                                            merge_in_end, merge_out_start, src_len);
        }
    }

exit:

    int_unlock(lock);

    if ((0 == audio_prompt_env.mergeOutOverlapLength) ||
        (0 == audio_prompt_env.leftEncodedDataLen))
    {
        // prompt playing is completed
        audio_prompt_stop_playing();
        app_sysfreq_req(APP_SYSFREQ_USER_PROMPT_MIXER, APP_SYSFREQ_32K);
    }

}

void audio_prompt_processing_handler(uint8_t device_id, uint32_t acquiredPcmDataLen, uint8_t* pcmDataToMerge)
{
#ifdef TWS_PROMPT_SYNC
    if (!tws_sync_mix_prompt_handling(device_id))
    {
        return;
    }
#endif

#if defined(PROMPT_CTRL_SWITCH_ANC_MODE)
    extern void app_prompt_ctrl_anc_process(void);
    app_prompt_ctrl_anc_process();
#endif
    uint32_t lock = int_lock();

    if (audio_prompt_env.leftEncodedDataLen ==
        audio_prompt_env.wholeEncodedDataLen)
    {
        int_unlock(lock);
        if (KEEP_CURRENT_VOLUME_FOR_MIX_PROMPT != audio_prompt_env.volume_level_override)
        {
            // first entering, coordinate the volume here
            app_bt_stream_volumeset_handler(audio_prompt_env.volume_level_override);
        }
    }
    else
    {
        int_unlock(lock);
    }

    uint32_t gotDataLen = 0;
    while (gotDataLen < acquiredPcmDataLen)
    {
        uint32_t lenToGet;
        if ((acquiredPcmDataLen - gotDataLen) > AUDIO_PROMPT_PCM_FILL_UNIT_SIZE)
        {
            lenToGet = AUDIO_PROMPT_PCM_FILL_UNIT_SIZE;
        }
        else
        {
            lenToGet = acquiredPcmDataLen - gotDataLen;
        }

        audio_prompt_processing_handler_func(lenToGet, pcmDataToMerge+gotDataLen);

        gotDataLen += lenToGet;
    }
}

static uint32_t audio_prompt_sbc_decode(uint8_t* pcm_buffer, uint32_t expectedOutputSize, uint8_t isReset)
{
    if (isReset)
    {
        audio_prompt_sbc_init_decoder();
    }

    uint32_t sbcDataBytesToDecode;
    unsigned int pcm_offset = 0;
    uint16_t byte_decode;
    int8_t ret;
    sbc_frame_t sbc_data;
    pcm_frame_t pcm_data;
    pcm_data.pcm_data = (int16_t *)pcm_buffer;
    pcm_data.buffer_size = expectedOutputSize;
    pcm_data.valid_size = 0;

get_again:

    if (audio_prompt_env.leftEncodedDataLen > AUDIO_PROMPT_SBC_ENCODED_DATA_SIZE_PER_FRAME)
    {
        sbcDataBytesToDecode = AUDIO_PROMPT_SBC_ENCODED_DATA_SIZE_PER_FRAME;
    }
    else
    {
        sbcDataBytesToDecode = audio_prompt_env.leftEncodedDataLen;
    }
    
    sbc_data.sbc_data = audio_prompt_env.promptDataBuf+audio_prompt_env.wholeEncodedDataLen-audio_prompt_env.leftEncodedDataLen;
    sbc_data.buffer_size = sbcDataBytesToDecode;
    sbc_data.valid_size = sbcDataBytesToDecode;
    ret = sbc_decoder_process_frame(&audio_prompt_sbc_decoder,&sbc_data, &pcm_data);
    byte_decode = sbc_data.buffer_size - sbc_data.valid_size;
    audio_prompt_env.leftEncodedDataLen -= byte_decode;

    pcm_offset = pcm_data.valid_size;

    if (0 == audio_prompt_env.leftEncodedDataLen)
    {
        goto exit;
    }

    if (expectedOutputSize == pcm_offset)
    {
        goto exit;
    }

    if ((ret == BT_STS_CONTINUE) || (ret == BT_STS_SUCCESS))  {
        goto get_again;
    }

exit:
    return pcm_offset;
}
#endif // MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED

#if defined(IBRT)
void app_tws_stop_peer_prompt(void)
{
    uint8_t stub_param = 0;

    tws_ctrl_send_cmd(APP_TWS_CMD_STOP_PEER_PROMPT_REQ, &stub_param, sizeof(stub_param));
}

bool app_tws_let_peer_device_play_audio_prompt(uint16_t promptId,
    uint16_t promptType, uint8_t dev_id)
{
    int ret = 0;
    app_tws_voice_prompt_to_play_t req;
    req.voicePrompt = promptId;
    req.dev_id = dev_id;
    req.prompt_type = promptType;
    ret = tws_ctrl_send_cmd(APP_TWS_CMD_LET_PEER_PLAY_PROMPT, (uint8_t *)&req, sizeof(req));
    if (0 == ret)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void app_tws_cmd_prompt_play_req_handler(uint8_t *ptrParam, uint32_t paramLen)
{
#ifdef MEDIA_PLAYER_SUPPORT
    app_tws_voice_prompt_to_play_t *ptPlayReq = (app_tws_voice_prompt_to_play_t *)ptrParam;
    TRACE(0, "Receive request to play prompt 0x%x%s prompt_type 0x%x",
        ptPlayReq->voicePrompt, aud_id2str(ptPlayReq->voicePrompt & 0xFF),
        ptPlayReq->prompt_type);
    voice_report_role_t report_role =  app_ibrt_voice_report_get_role();

#if BLE_AUDIO_ENABLED
    if (AUD_ID_LE_AUD_INCOMING_CALL == (AUD_ID_ENUM)ptPlayReq->voicePrompt)
    {
        if (bes_ble_is_connection_on_by_index(ptPlayReq->dev_id))
        {
            return;
        }
    }
#endif

    struct BT_DEVICE_T *curr_device = app_bt_get_device(ptPlayReq->dev_id);

    if (APP_PROMPT_LOCAL_PLAY == ptPlayReq->prompt_type)
    {
        ptPlayReq->prompt_type = APP_PROMPT_REMOTE_PLAY;
    }
    else if (APP_PROMPT_LOCAL_CONTINUOUS_NORMAL_PLAY == ptPlayReq->prompt_type)
    {
        ptPlayReq->prompt_type = APP_PROMPT_CONTINUOUS_REMOTE_PLAY;;
    }
    else if (APP_PROMPT_CONTINUOUS_NORMAL_PLAY == ptPlayReq->prompt_type)
    {
        media_playAudio_set_continous_prompt_env((AUD_ID_ENUM)ptPlayReq->voicePrompt);
    }
    else if (APP_PROMPT_REMOTE_PLAY == ptPlayReq->prompt_type)
    {
        ptPlayReq->prompt_type = APP_PROMPT_LOCAL_PLAY;
    }
    else if (APP_PROMPT_STANDALONE_LOCAL_PLAY == ptPlayReq->prompt_type)
    {
        ptPlayReq->prompt_type = APP_PROMPT_STANDALONE_REMOTE_PLAY;
    }
    else if (APP_PROMPT_STANDALONE_REMOTE_PLAY == ptPlayReq->prompt_type)
    {
        ptPlayReq->prompt_type = APP_PROMPT_STANDALONE_LOCAL_PLAY;
    }

    if (report_role == VOICE_REPORT_MASTER)
    {
        if (((BT_HFP_AUDIO_CON == curr_device->hf_audio_state)
            || (BT_HFP_CALL_ACTIVE == curr_device->hfchan_call))
            && (AUD_ID_BT_CALL_INCOMING_CALL == ptPlayReq->voicePrompt))
        {
            TRACE(1, "%s audio already connected, don't push prompt", __func__);
            return;
        }

        app_prompt_push_request((APP_PROMPT_PLAY_REQ_TYPE_E)ptPlayReq->prompt_type,
            (AUD_ID_ENUM)ptPlayReq->voicePrompt, ptPlayReq->dev_id, false);
    }
    else
    {
        TRACE(1, "[%s] current role is slave", __func__);
    }
#endif
}
#endif
