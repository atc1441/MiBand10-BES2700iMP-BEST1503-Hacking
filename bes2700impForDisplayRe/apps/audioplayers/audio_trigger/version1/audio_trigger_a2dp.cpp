#include <stdio.h>
#include "cmsis_os.h"
#include "bluetooth_bt_api.h"
#include "app_bt_audio.h"
#include "app_utils.h"
#include "app_bt.h"
#include "audio_policy.h"
#include "app_audio.h"
#include "audio_trigger_checker.h"
#include "a2dp_decoder.h"
#include "btapp.h"
#include "bt_drv.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_interface.h"
#include "audioflinger.h"
#include "audio_trigger_common.h"
#include "audio_trigger_a2dp.h"

#if defined(IBRT)
#include "app_tws_ctrl_thread.h"
#include "app_tws_ibrt_audio_analysis.h"
#include "app_tws_ibrt_audio_sync.h"
#include "audio_trigger_ibrt.h"
#include "app_ibrt_internal.h"
#include "app_ibrt_a2dp.h"
#endif

#include "app_a2dp.h"

extern uint32_t app_bt_stream_get_dma_buffer_delay_us(void);
extern uint32_t app_bt_stream_get_dma_buffer_samples(void);
extern uint8_t a2dp_lhdc_config_llc_get(void);

uint32_t tg_acl_trigger_time = 0;
uint32_t tg_acl_trigger_start_time = 0;
uint32_t tg_acl_trigger_init_time = 0;


void app_bt_stream_set_trigger_time(uint32_t trigger_time_us)
{
    struct AF_STREAM_CONFIG_T *stream_cfg = NULL;
    uint32_t curr_ticks = 0;
    uint32_t dma_buffer_delay_us = 0;
    uint32_t tg_acl_trigger_offset_time = 0;
    POSSIBLY_UNUSED uint8_t device_id = app_bt_audio_get_curr_a2dp_device();

    if (trigger_time_us){
#if defined(IBRT)
        uint16_t conhandle = BT_INVALID_CONN_HANDLE;
        struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
        if (APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote)){
            conhandle = APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote);

            curr_ticks = bt_syn_get_curr_ticks(conhandle);
        }else if (APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote)){
            conhandle = APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote);
            curr_ticks = bt_syn_get_curr_ticks(conhandle);
        }else{
            return;
        }
#else
        uint16_t conhandle = 0xffff;
        conhandle = app_bt_get_conhandle_by_device_id(device_id);
        curr_ticks = bt_syn_get_curr_ticks(conhandle);
#endif
        app_sysfreq_req(APP_SYSFREQ_USER_TRIGGER, APP_SYSFREQ_52M);
        af_stream_get_cfg(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg, false);
        if (NULL == stream_cfg)
        {
            TRACE(1,"%s Fail!!!", __func__);
            return;
        }

        btdrv_syn_clr_trigger(0);

        btdrv_enable_playback_triggler(ACL_TRIGGLE_MODE);

        dma_buffer_delay_us = app_bt_stream_get_dma_buffer_delay_us();
        dma_buffer_delay_us /= 2;
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][SETTIME] %d-%d-%d dma_sz:%d dly:%d", stream_cfg->sample_rate, stream_cfg->channel_num, stream_cfg->bits,
                                                                                   stream_cfg->data_size, dma_buffer_delay_us);

        tg_acl_trigger_offset_time = US_TO_BTCLKS(trigger_time_us-dma_buffer_delay_us);
        tg_acl_trigger_time = curr_ticks + tg_acl_trigger_offset_time;
        tg_acl_trigger_start_time = curr_ticks;
#if defined(IBRT)
#if BLE_AUDIO_ENABLED
        af_codec_set_device_bt_sync_source(AUD_STREAM_USE_INT_CODEC, AUD_STREAM_PLAYBACK, 0);
#endif
        bt_syn_set_tg_ticks(tg_acl_trigger_time, conhandle, BT_TRIG_SLAVE_ROLE,0,false);
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][SETTIME] %d->%d trig_dly:%d aud_dly:%dus",
                                            curr_ticks, tg_acl_trigger_time, trigger_time_us-dma_buffer_delay_us, trigger_time_us+dma_buffer_delay_us);
#else
        bt_syn_set_tg_ticks(tg_acl_trigger_time, conhandle, BT_TRIG_SLAVE_ROLE,0,false);
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][SETTIME] %d->%d trig_dly:%d aud_dly:%dus",
                                    curr_ticks, tg_acl_trigger_time, trigger_time_us-dma_buffer_delay_us, trigger_time_us+dma_buffer_delay_us);
#endif
        app_bt_stream_trigger_stauts_set(BT_STREAM_TRIGGER_STATUS_WAIT);
    }else{
        tg_acl_trigger_time = 0;
        tg_acl_trigger_start_time = 0;
        btdrv_syn_clr_trigger(0);
        bt_syn_cancel_tg_ticks(0);
        app_bt_stream_trigger_stauts_set(BT_STREAM_TRIGGER_STATUS_NULL);
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][SETTIME] trigger clear");
        app_sysfreq_req(APP_SYSFREQ_USER_TRIGGER, APP_SYSFREQ_32K);
    }
}

void app_bt_stream_trigger_result(uint8_t device_id)
{
    POSSIBLY_UNUSED uint32_t curr_ticks = 0;

    if(tg_acl_trigger_time){
#if defined(IBRT)
        POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
        if (APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote)){
            curr_ticks = bt_syn_get_curr_ticks(APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote));
            bt_syn_trig_checker(APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote));
        }else if (APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote)){
            curr_ticks = bt_syn_get_curr_ticks(APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote));
            bt_syn_trig_checker(APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote));
        }else{
            TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][RESULT] mobile_link:%d %04x ibrt_link:%d %04x", \
                                       APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote), \
                                       APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote), \
                                       APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote), \
                                       APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote));
        }
#else
        curr_ticks = btdrv_syn_get_curr_ticks();
#endif
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][RESULT] trig:%d curr:%d tg:%d start:%d", (curr_ticks - (uint32_t)US_TO_BTCLKS(app_bt_stream_get_dma_buffer_delay_us()/2)),
                                                                         curr_ticks, tg_acl_trigger_time, tg_acl_trigger_start_time);
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][RESULT] tg_trig_diff:%d trig_diff:%d", (uint32_t)BTCLKS_TO_US(curr_ticks-tg_acl_trigger_time),
                                              (uint32_t)BTCLKS_TO_US(curr_ticks-tg_acl_trigger_start_time));
        app_bt_stream_set_trigger_time(0);
        app_bt_stream_trigger_stauts_set(BT_STREAM_TRIGGER_STATUS_OK);
    #if defined(IBRT)
        A2DP_AUDIO_HEADFRAME_INFO_T headframe_info;
        a2dp_audio_decoder_headframe_info_get(&headframe_info);
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][RESULT] synchronize_ok :%d", headframe_info.sequenceNumber);
    #endif
    }
}


void app_bt_stream_trigger_init(void)
{
#if defined(IBRT)
    POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = app_bt_get_device(app_bt_audio_get_curr_a2dp_device());
    if (APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote)){
        tg_acl_trigger_init_time = bt_syn_get_curr_ticks(APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote));
    }else if (APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote)){
        tg_acl_trigger_init_time = bt_syn_get_curr_ticks(APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote));
    }else{
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][INIT] mobile_link:%d %04x ibrt_link:%d %04x", \
                                    APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote), \
                                    APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote), \
                                    APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote), \
                                    APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote));
   }
#else
    tg_acl_trigger_init_time = btdrv_syn_get_curr_ticks();
#endif
    app_bt_stream_set_trigger_time(0);
#ifdef PLAYBACK_USE_I2S
    af_i2s_sync_config(AUD_STREAM_PLAYBACK, AF_I2S_SYNC_TYPE_BT, false);
    af_i2s_sync_config(AUD_STREAM_PLAYBACK, AF_I2S_SYNC_TYPE_BT, true);
#else
    af_codec_sync_config(AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, false);
    af_codec_sync_config(AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, true);
#endif
    app_bt_stream_trigger_stauts_set(BT_STREAM_TRIGGER_STATUS_INIT);
}

void app_bt_stream_trigger_deinit(void)
{
    app_bt_stream_set_trigger_time(0);
}

void app_bt_stream_trigger_start(uint8_t device_id, uint8_t offset)
{
    float tg_trigger_time = 0;
    POSSIBLY_UNUSED uint32_t curr_ticks;
    struct AF_STREAM_CONFIG_T *stream_cfg = NULL;
    uint8_t codec_type = bta_get_curr_a2dp_codec_type();
    uint8_t nonType = bta_get_curr_a2dp_vender_codec_type();

    af_stream_get_cfg(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg, false);
    if (NULL == stream_cfg)
    {
        TRACE(1,"%s Fail!!!", __func__);
        return;
    }

#if defined(IBRT)
    POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote))
    {
        curr_ticks = bt_syn_get_curr_ticks(APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote));
    }
    else if (APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote))
    {
        curr_ticks = bt_syn_get_curr_ticks(APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote));
    }
    else
    {
        return;
    }
#else
    curr_ticks = btdrv_syn_get_curr_ticks();
#endif

    TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][START] init(%d)-->set_trig(%d) %dus", tg_acl_trigger_init_time, curr_ticks, (uint32_t)BTCLKS_TO_US(curr_ticks-tg_acl_trigger_init_time));
    if (codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
    {
        if (nonType == BT_A2DP_CODEC_NONE_TYPE_INVALID)
        {
            TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][START] non_type invalid %d", nonType);
        }
#if defined(A2DP_SCALABLE_ON)
        else if (nonType == BT_A2DP_CODEC_NONE_TYPE_SCALABLE)
        {
            if (stream_cfg->sample_rate > AUD_SAMPRATE_48000)
            {
                tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_HIRES_US * a2dp_audio_latency_factor_get();

                tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_HIRES_BASE;
#if (A2DP_DECODER_VER < 2)
                a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_HIRES_MTU);
#endif
            }
            else
            {
                tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_US * a2dp_audio_latency_factor_get();
                tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_BASE;
#if (A2DP_DECODER_VER < 2)
                a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_MTU);
#endif
            }
        }
#endif
#if defined(A2DP_LHDC_ON)
        else if (nonType == BT_A2DP_CODEC_NONE_TYPE_LHDC)
        {
            if (a2dp_lhdc_config_llc_get())
            {
                tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_US * a2dp_audio_latency_factor_get();
                tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_BASE;
#if (A2DP_DECODER_VER < 2)
                a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_MTU);
#endif
            }
            else if (stream_cfg->sample_rate > AUD_SAMPRATE_48000)
            {
                tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_US * a2dp_audio_latency_factor_get();
                tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_BASE;
#if (A2DP_DECODER_VER < 2)
                a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_MTU);
#endif
            }
            else
            {
                tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_US * a2dp_audio_latency_factor_get();
                tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_BASE;
#if (A2DP_DECODER_VER < 2)
                a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_MTU);
#endif
            }
        }
#endif
#if defined(A2DP_LHDCV5_ON)
        else if(nonType == BT_A2DP_CODEC_NONE_TYPE_LHDCV5)
        {
            if (a2dp_lhdc_config_llc_get()){
                tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_LHDCV5_LLC_US * a2dp_audio_latency_factor_get();
                tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_LHDCV5_BASE;
#if (A2DP_DECODER_VER < 2)
                a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_LHDCV5_LLC_MTU);
#endif
            }
            else
            {
                tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_LHDCV5_US * a2dp_audio_latency_factor_get();
                tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_LHDCV5_BASE;
#if (A2DP_DECODER_VER < 2)
                a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_LHDCV5_MTU);
#endif
            }
        }
#endif
#if defined(A2DP_LDAC_ON)
        else if(nonType == BT_A2DP_CODEC_NONE_TYPE_LDAC)
        {
            uint32_t frame_mtu = A2DP_PLAYER_PLAYBACK_DELAY_LDAC_FRAME_MTU;
#if (A2DP_DECODER_VER == 2)
            A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info;
            if (!a2dp_audio_lastframe_info_get(&lastframe_info)){
                frame_mtu = lastframe_info.totalSubSequenceNumber;
            }
#endif
            tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_LDAC_US * a2dp_audio_latency_factor_get();
            tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_LDAC_BASE*frame_mtu;
#if (A2DP_DECODER_VER < 2)
            a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_LDAC_MTU);
#endif
            TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][START] [%d,%d,%d]", A2DP_PLAYER_PLAYBACK_DELAY_LDAC_MTU,A2DP_PLAYER_PLAYBACK_DELAY_LDAC_BASE,A2DP_PLAYER_PLAYBACK_DELAY_LDAC_US);
        }
#endif
#if defined(A2DP_LC3_ON)
        else if(nonType == BT_A2DP_CODEC_NONE_TYPE_LC3)
        {
            if(stream_cfg->sample_rate > AUD_SAMPRATE_48000){
                tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_LC3_HIRES_US * a2dp_audio_latency_factor_get();
                tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_LC3_HIRES_BASE;
#if (A2DP_DECODER_VER < 2)
             a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_LC3_HIRES_MTU);
#endif
            }
            else
            {
                tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_LC3_BASERES_US * a2dp_audio_latency_factor_get();
                tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_BASE;
#if (A2DP_DECODER_VER < 2)
                a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_MTU);
#endif
            }
        }
#endif
    }
#if defined(A2DP_AAC_ON)
    else if (codec_type == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC)
    {
        tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_AAC_US * a2dp_audio_latency_factor_get();
        tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_AAC_BASE;
#ifdef CUSTOM_BITRATE
        tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_AAC_BASE * app_audio_a2dp_player_playback_delay_mtu_get(A2DP_AUDIO_CODEC_TYPE_MPEG2_4_AAC) * a2dp_audio_latency_factor_get();
#endif
#if (A2DP_DECODER_VER < 2)
        a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU);
#endif
    }
#endif
    else
    {
        uint32_t frame_mtu = A2DP_PLAYER_PLAYBACK_DELAY_SBC_FRAME_MTU;
#if (A2DP_DECODER_VER == 2)
        A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info;
        if (!a2dp_audio_lastframe_info_get(&lastframe_info)){
            frame_mtu = lastframe_info.totalSubSequenceNumber;
        }
#endif
        tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_SBC_US * a2dp_audio_latency_factor_get();
#ifdef CUSTOM_BITRATE
        tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_SBC_BASE * app_audio_a2dp_player_playback_delay_mtu_get(A2DP_AUDIO_CODEC_TYPE_SBC) * a2dp_audio_latency_factor_get();
        frame_mtu = 0;
#endif
        tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_SBC_BASE*frame_mtu;
#if (A2DP_DECODER_VER < 2)
        a2dp_audio_set_mtu_limit(A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU);
#endif
    }

    if (app_is_in_multipoint_timer())
    {
        tg_trigger_time += 100000;
        TRACE(0, "%s add 100ms", __func__);
    }
    app_bt_stream_set_trigger_time((uint32_t)tg_trigger_time);
}

bool app_bt_stream_trigger_onprocess(void)
{
    if (app_bt_stream_trigger_stauts_get() == BT_STREAM_TRIGGER_STATUS_INIT){
        return true;
    }else{
        return false;
    }
}


