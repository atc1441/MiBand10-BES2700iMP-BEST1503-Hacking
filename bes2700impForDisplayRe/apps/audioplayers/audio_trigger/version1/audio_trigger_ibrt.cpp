#include <stdio.h>
#include "cmsis_os.h"
#include "bluetooth_bt_api.h"
#include "app_bt_audio.h"
#include "app_utils.h"
#include "app_bt.h"
#include "app_audio.h"
#include "audio_trigger_checker.h"
#include "a2dp_decoder.h"
#include "btapp.h"
#include "bt_drv.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_interface.h"
#include "audio_trigger_common.h"
#include "audio_trigger_a2dp.h"
#include "audioflinger.h"

#if defined(IBRT)
#include "app_tws_ctrl_thread.h"
#include "app_tws_ibrt_audio_analysis.h"
#include "app_tws_ibrt_audio_sync.h"
#include "audio_trigger_ibrt.h"
#include "app_ibrt_internal.h"
#include "app_ibrt_a2dp.h"
#include "earbud_ux_api.h"

#if BLE_AUDIO_ENABLED
#include "app_audio_active_device_manager.h"
#endif

#ifdef A2DP_CP_ACCEL
#define APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME (8)
#else
#define APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME (4)
#endif
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
#define APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME_LHDC (12)
#endif

extern uint32_t app_bt_stream_get_dma_buffer_delay_us(void);
extern uint32_t app_bt_stream_get_dma_buffer_samples(void);
extern uint32_t tg_acl_trigger_time;

bool is_need_discards_samples = false;

#define APP_BT_STREAM_MISMATCH_PACKET_THRESHOLD     5

APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T app_bt_stream_ibrt_auto_synchronize_initsync_trigger;

int app_bt_stream_ibrt_auto_synchronize_initsync_dataind_cb(uint8_t device_id, btif_media_header_t * header, unsigned char *buf, unsigned int len)
{
    APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger = &app_bt_stream_ibrt_auto_synchronize_initsync_trigger;
    bool synchronize_ok = false;
    A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info;

    TRACE_AUD_STREAM_I("[AUTO_SYNC][INITSYNC][DATAIND] dataind_seq:%d/%d timestamp:%d/%d", header->sequenceNumber,
                                                                             sync_trigger->sequenceNumberStart,
                                                                             header->timestamp,
                                                                             sync_trigger->audio_info.timestamp);

    if (a2dp_audio_lastframe_info_get(&lastframe_info)<0){
        TRACE_AUD_STREAM_W("[AUTO_SYNC][INITSYNC][DATAIND] force retrigger");
        app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_DECODE_STATUS_ERROR);
        return 0;
    }

    if (app_bt_stream_trigger_stauts_get() != BT_STREAM_TRIGGER_STATUS_WAIT){
        TRACE_AUD_STREAM_W("[AUTO_SYNC][INITSYNC][DATAIND] already end");
        a2dp_audio_detect_next_packet_callback_register(NULL);
        a2dp_audio_detect_store_packet_callback_register(NULL);
        return 0;
    }

    if (sync_trigger->sequenceNumberStart < header->sequenceNumber){
#ifdef A2DP_PLAYER_PLAYBACK_WATER_LINE
        A2DP_AUDIO_HEADFRAME_INFO_T headframe_info;
        if (a2dp_audio_decoder_headframe_info_get(&headframe_info) < 0) {
            TRACE_AUD_STREAM_W("[AUTO_SYNC][INITSYNC][DATAIND] force retrigger");
            app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_DECODE_STATUS_ERROR);
            return 0;
        }
        if (sync_trigger->sequenceNumberStart >= headframe_info.sequenceNumber) {
            TRACE_AUD_STREAM_W("[AUTO_SYNC][INITSYNC][DATAIND] seq:sync_trigger<header");
            synchronize_ok = true;
        } else {
            TRACE_AUD_STREAM_W("[AUTO_SYNC][INITSYNC][DATAIND] do not retrigger");
            synchronize_ok = true;
            //app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_SEQ_MISMATCH);
        }
#else
        TRACE_AUD_STREAM_W("[AUTO_SYNC][INITSYNC][DATAIND] force retrigger");
        app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_SEQ_MISMATCH);
#endif
    }else{
        if (header->sequenceNumber >= sync_trigger->sequenceNumberStart){
            synchronize_ok = true;
        }

        if (synchronize_ok && app_bt_stream_trigger_stauts_get() != BT_STREAM_TRIGGER_STATUS_WAIT){
            TRACE_AUD_STREAM_W("[AUTO_SYNC][INITSYNC][DATAIND] synchronize_failed");
            app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_STATUS_ERROR);
            return 0;
        }
#ifdef A2DP_PLAYER_PLAYBACK_WATER_LINE
        //limter to water line upper
        uint32_t list_samples = 0;
        uint32_t limter_water_line_samples = 0;
        limter_water_line_samples = (a2dp_audio_dest_packet_mut_get() * lastframe_info.list_samples);
        a2dp_audio_convert_list_to_samples(&list_samples);
        TRACE_AUD_STREAM_W("[AUTO_SYNC][INITSYNC][DATAIND] synchronize:%d/%d", list_samples, limter_water_line_samples);
        if (list_samples > limter_water_line_samples){
            is_need_discards_samples = true;
            // TRACE_AUD_STREAM_W("[AUTO_SYNC][INITSYNC][DATAIND] skip discards:%d", list_samples - limter_water_line_samples);
            // a2dp_audio_discards_samples(list_samples - limter_water_line_samples);
        }
#else
        //flush all
        a2dp_audio_synchronize_dest_packet_mut(0);
#endif
    }

    if (synchronize_ok){
        A2DP_AUDIO_HEADFRAME_INFO_T headframe_info;
        a2dp_audio_decoder_headframe_info_get(&headframe_info);
        TRACE_AUD_STREAM_W("[AUTO_SYNC][INITSYNC][DATAIND] synchronize_ok :%d", headframe_info.sequenceNumber);
        a2dp_audio_detect_next_packet_callback_register(NULL);
        a2dp_audio_detect_store_packet_callback_register(NULL);
    }else{
        a2dp_audio_detect_first_packet();
    }
    return 0;
}

void app_bt_stream_ibrt_auto_synchronize_initsync_start(uint8_t device_id, APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger)
{
    TRACE_AUD_STREAM_I("[AUTO_SYNC][INITSYNC] trigger_time:%d seq:%d timestamp:%d SubSeq:%d/%d", sync_trigger->trigger_time,
                                                                                       sync_trigger->audio_info.sequenceNumber,
                                                                                       sync_trigger->audio_info.timestamp,
                                                                                       sync_trigger->audio_info.curSubSequenceNumber,
                                                                                       sync_trigger->audio_info.totalSubSequenceNumber
                                                                                       );
    POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    app_bt_stream_ibrt_auto_synchronize_initsync_trigger = *sync_trigger;
    a2dp_audio_detect_next_packet_callback_register(app_bt_stream_ibrt_auto_synchronize_initsync_dataind_cb);
    a2dp_audio_detect_first_packet();
    if (APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote))
    {
        if (sync_trigger->trigger_time > bt_syn_get_curr_ticks(APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote)))
        {
            uint32_t tg_tick = sync_trigger->trigger_time;
            btdrv_syn_clr_trigger(0);
            btdrv_enable_playback_triggler(ACL_TRIGGLE_MODE);
#if BLE_AUDIO_ENABLED
            af_codec_set_device_bt_sync_source(AUD_STREAM_USE_INT_CODEC, AUD_STREAM_PLAYBACK, 0);
#endif
            bt_syn_set_tg_ticks(tg_tick, APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote), BT_TRIG_SLAVE_ROLE,0,false);
            tg_acl_trigger_time = tg_tick;
            app_bt_stream_trigger_stauts_set(BT_STREAM_TRIGGER_STATUS_WAIT);
            TRACE_AUD_STREAM_I("[AUTO_SYNC][INITSYNC] start slave trigger curr(%d)-->tg(%d)", bt_syn_get_curr_ticks(APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote)), tg_tick);
            app_tws_ibrt_audio_analysis_start(sync_trigger->handler_cnt, AUDIO_ANALYSIS_CHECKER_INTERVEL_INVALID);
            app_tws_ibrt_audio_sync_start();
            app_tws_ibrt_audio_sync_new_reference(sync_trigger->factor_reference);
        }else{
            TRACE_AUD_STREAM_I("[AUTO_SYNC][INITSYNC] slave failed trigger(%d)-->tg(%d) need resume",
                                                            bt_syn_get_curr_ticks(APP_IBRT_UI_GET_IBRT_HANDLE(NULL)),
                                                            sync_trigger->trigger_time);
            app_tws_ibrt_audio_sync_mismatch_resume_notify(device_id);
        }
    }
}

int app_bt_stream_ibrt_audio_master_detect_next_packet_cb(uint8_t device_id, btif_media_header_t * header, unsigned char *buf, unsigned int len)
{
#ifdef A2DP_PLAYER_PLAYBACK_WATER_LINE
    A2DP_AUDIO_SYNCFRAME_INFO_T sync_info;
    A2DP_AUDIO_HEADFRAME_INFO_T headframe_info;
#endif

    if(app_bt_stream_trigger_stauts_get() == BT_STREAM_TRIGGER_STATUS_INIT){
        ibrt_ctrl_t  *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
        int32_t dma_buffer_samples = app_bt_stream_get_dma_buffer_samples()/2;
        POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

        if (APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote)){
            TRACE_AUD_STREAM_W("[AUTO_SYNC][MASTER]cache ok but currRole:%d mismatch\n", APP_IBAT_UI_GET_CURRENT_ROLE(&curr_device->remote));
            app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_ROLE_MISMATCH);
        }else if (!APP_IBRT_IS_PROFILE_EXCHANGED(&curr_device->remote) &&
                  !app_ibrt_if_start_ibrt_onprocess(&curr_device->remote) &&
                  !app_ibrt_sync_a2dp_status_onprocess(&curr_device->remote)){
            if (APP_IBRT_UI_GET_MOBILE_MODE(&curr_device->remote) == IBRT_SNIFF_MODE){
                //flush all
                a2dp_audio_synchronize_dest_packet_mut(0);
                a2dp_audio_detect_first_packet();
                TRACE_AUD_STREAM_I("[AUTO_SYNC][MASTER] cache skip delay dma trigger1\n");
                return 0;
            }
            TRACE_AUD_STREAM_I("[AUTO_SYNC][MASTER] cache ok use dma trigger1\n");
            a2dp_audio_detect_next_packet_callback_register(NULL);
            a2dp_audio_detect_store_packet_callback_register(NULL);
#ifdef A2DP_PLAYER_PLAYBACK_WATER_LINE
            app_bt_stream_trigger_start(device_id, A2DP_PLAYER_PLAYBACK_WATER_LINE);
#else
            app_bt_stream_trigger_start(device_id, 0);
#endif
        }else if (app_ibrt_if_start_ibrt_onprocess(&curr_device->remote) ||
                  app_ibrt_sync_a2dp_status_onprocess(&curr_device->remote) ||
                  !APP_IBRT_IS_A2DP_PROFILE_EXCHNAGED(&curr_device->remote)
#if BLE_AUDIO_ENABLED
                  || (app_audio_adm_get_le_audio_music_stream_device() != BT_DEVICE_INVALID_ID)
#endif
                  ){
            //flush all
            a2dp_audio_synchronize_dest_packet_mut(0);
            a2dp_audio_detect_first_packet();
            TRACE_AUD_STREAM_I("[AUTO_SYNC][MASTER] ibrt_onporcess:%d,sync_a2dp_status_onporcess:%d,a2dp_profile_exchanged:%d",
                                app_ibrt_if_start_ibrt_onprocess(&curr_device->remote),
                                app_ibrt_sync_a2dp_status_onprocess(&curr_device->remote),
                                APP_IBRT_IS_A2DP_PROFILE_EXCHNAGED(&curr_device->remote));
            TRACE_AUD_STREAM_I("[AUTO_SYNC][MASTER] cache skip profile_exchanged sync_a2dp_status_onporcess\n");
            return 0;
        }else{
            if (p_ibrt_ctrl->tws_mode == IBRT_SNIFF_MODE    ||
                APP_IBRT_UI_GET_MOBILE_MODE(&curr_device->remote) == IBRT_SNIFF_MODE){
                //flush all
                a2dp_audio_synchronize_dest_packet_mut(0);
                a2dp_audio_detect_first_packet();
                TRACE_AUD_STREAM_I("[AUTO_SYNC][MASTER] cache skip delay dma trigger2\n");
                return 0;
            }

#ifdef A2DP_PLAYER_PLAYBACK_WATER_LINE
            uint32_t dest_waterline_samples = 0;
            uint32_t list_samples = 0;
            dest_waterline_samples = app_bt_stream_get_dma_buffer_samples()/2*A2DP_PLAYER_PLAYBACK_WATER_LINE;
            a2dp_audio_convert_list_to_samples(&list_samples);
            if (list_samples < dest_waterline_samples){
                a2dp_audio_detect_first_packet();
                TRACE_AUD_STREAM_I("[AUTO_SYNC][MASTER] cache skip fill data sample:%d\n", list_samples);
                return 0;
            }
            a2dp_audio_decoder_headframe_info_get(&headframe_info);
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
            uint8_t codec_type = bta_get_curr_a2dp_codec_type();
            if(codec_type == BT_A2DP_CODEC_TYPE_LHDC)
            {
                sync_info.sequenceNumber = headframe_info.sequenceNumber;
            }
            else
#endif
            {
                sync_info.sequenceNumber = headframe_info.sequenceNumber+1;
            }
            a2dp_audio_synchronize_packet(&sync_info, A2DP_AUDIO_SYNCFRAME_MASK_SEQ);
#else
            a2dp_audio_synchronize_dest_packet_mut(0);
#endif
            TRACE_AUD_STREAM_I("[AUTO_SYNC][MASTER] cache ok use dma trigger2\n");

#ifdef A2DP_CP_ACCEL
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
            if(codec_type == BT_A2DP_CODEC_TYPE_LHDC)
            {
                app_bt_stream_trigger_start(device_id, APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME_LHDC - a2dp_audio_frame_delay_get());
            }
            else
#endif
            {
                app_bt_stream_trigger_start(device_id, APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME - a2dp_audio_frame_delay_get());
            }
#else
            app_bt_stream_trigger_start(device_id, APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME);
#endif
            APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T sync_trigger;
            A2DP_AUDIO_HEADFRAME_INFO_T headframe_info;
            A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info;
            sync_trigger.trigger_time = tg_acl_trigger_time;
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
            if(codec_type == BT_A2DP_CODEC_TYPE_LHDC)
            {
                sync_trigger.trigger_skip_frame = APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME_LHDC - a2dp_audio_frame_delay_get();
            }
            else
#endif
            {
                sync_trigger.trigger_skip_frame = APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME - a2dp_audio_frame_delay_get();
            }
            sync_trigger.trigger_type = APP_TWS_IBRT_AUDIO_TRIGGER_TYPE_INIT_SYNC;
            if (a2dp_audio_lastframe_info_get(&lastframe_info)<0){
                goto exit;
            }

            a2dp_audio_decoder_headframe_info_get(&headframe_info);
            sync_trigger.sequenceNumberStart = headframe_info.sequenceNumber;
            TRACE(0, "sequenceNumberStart %d", sync_trigger.sequenceNumberStart);
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
            if(codec_type == BT_A2DP_CODEC_TYPE_LHDC)
            {
                sync_trigger.audio_info.sequenceNumber = lastframe_info.sequenceNumber + APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME_LHDC;
                if (lastframe_info.totalSubSequenceNumber){
                    sync_trigger.audio_info.timestamp = lastframe_info.timestamp +
                                                        (lastframe_info.totalSubSequenceNumber * lastframe_info.frame_samples) *
                                                        APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME_LHDC;
                }else{
                    sync_trigger.audio_info.timestamp = lastframe_info.timestamp + dma_buffer_samples * APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME_LHDC;
                }
            }
            else
#endif
            {
                sync_trigger.audio_info.sequenceNumber = lastframe_info.sequenceNumber + APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME;
                if (lastframe_info.totalSubSequenceNumber){
                    sync_trigger.audio_info.timestamp = lastframe_info.timestamp +
                                                        (lastframe_info.totalSubSequenceNumber * lastframe_info.frame_samples) *
                                                        APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME;
                }else{
                    sync_trigger.audio_info.timestamp = lastframe_info.timestamp + dma_buffer_samples * APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME;
                }
            }
            sync_trigger.audio_info.curSubSequenceNumber = lastframe_info.curSubSequenceNumber;
            sync_trigger.audio_info.totalSubSequenceNumber = lastframe_info.totalSubSequenceNumber;
            sync_trigger.audio_info.frame_samples = lastframe_info.frame_samples;
            sync_trigger.factor_reference = a2dp_audio_get_output_config()->factor_reference;
            sync_trigger.a2dp_session = a2dp_ibrt_session_get(device_id);
            sync_trigger.handler_cnt = 0;
#if defined(IBRT_UI)
            sync_trigger.mobile_addr = curr_device->remote;
#endif
            app_bt_stream_ibrt_auto_synchronize_initsync_start(device_id, &sync_trigger);

            if (APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote) &&
                APP_IBRT_IS_PROFILE_EXCHANGED(&curr_device->remote)){
                tws_ctrl_send_cmd(APP_TWS_CMD_SET_TRIGGER_TIME, (uint8_t*)&sync_trigger, sizeof(APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T));
            }
        }
    }else{
        if(app_bt_stream_trigger_stauts_get() == BT_STREAM_TRIGGER_STATUS_NULL){
            TRACE_AUD_STREAM_W("[AUTO_SYNC][MASTER] audio not ready skip it");
        }else{
            TRACE_AUD_STREAM_I("[AUTO_SYNC][MASTER] unhandle status:%d", app_bt_stream_trigger_stauts_get());
            app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_UNKNOW);
        }
    }
exit:
    return 0;
}

int app_bt_stream_ibrt_audio_master_detect_next_packet_start(void)
{
    TRACE_AUD_STREAM_I("[AUTO_SYNC][MASTER] start");

    a2dp_audio_detect_next_packet_callback_register(app_bt_stream_ibrt_audio_master_detect_next_packet_cb);
    return 0;
}

#define SLAVE_DETECT_NEXT_PACKET_TO_RETRIGGER_THRESHOLD (120)
static uint32_t slave_detect_next_packet_cnt = 0;
int app_bt_stream_ibrt_audio_slave_detect_next_packet_waitforever_cb(uint8_t device_id, btif_media_header_t * header, unsigned char *buf, unsigned int len)
{
    POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote) || ++slave_detect_next_packet_cnt > SLAVE_DETECT_NEXT_PACKET_TO_RETRIGGER_THRESHOLD){
        TRACE_AUD_STREAM_W("[AUTO_SYNC][SLAVE] detect_next_packet ok but currRole:%d mismatch packet_cnt:%d\n", APP_IBAT_UI_GET_CURRENT_ROLE(&curr_device->remote),
                                                                                slave_detect_next_packet_cnt);
        slave_detect_next_packet_cnt = 0;
        app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_ROLE_MISMATCH);
    }else{
        TRACE_AUD_STREAM_I("[AUTO_SYNC][SLAVE] detect_next_packet cnt:%d\n", slave_detect_next_packet_cnt);
        a2dp_audio_detect_first_packet();
    }
    return 0;
}

int app_bt_stream_ibrt_audio_slave_detect_next_packet_cb(uint8_t device_id, btif_media_header_t * header, unsigned char *buf, unsigned int len)
{
    if(app_bt_stream_trigger_onprocess()){
        ibrt_ctrl_t  *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
        int32_t dma_buffer_samples = app_bt_stream_get_dma_buffer_samples()/2;
        POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

        if (APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote))
        {
            TRACE_AUD_STREAM_W("[AUTO_SYNC][SLAVE] cache ok but currRole:%d mismatch\n", APP_IBAT_UI_GET_CURRENT_ROLE(&curr_device->remote));
            app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_ROLE_MISMATCH);
        }
        else
        {
            if (APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote) &&
                (p_ibrt_ctrl->tws_mode == IBRT_SNIFF_MODE    ||
                 APP_IBRT_UI_GET_MOBILE_MODE(&curr_device->remote) == IBRT_SNIFF_MODE ||
                !APP_IBRT_IS_PROFILE_EXCHANGED(&curr_device->remote)))
            {
                //flush all
                a2dp_audio_synchronize_dest_packet_mut(0);
                a2dp_audio_detect_first_packet();
                TRACE_AUD_STREAM_W("[AUTO_SYNC][SLAVE]cache skip delay dma trigger2\n");
                return 0;
            }
            TRACE_AUD_STREAM_I("[AUTO_SYNC][SLAVE]cache ok use dma trigger2\n");
            //flush all
            a2dp_audio_synchronize_dest_packet_mut(0);
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
            uint8_t codec_type = bta_get_curr_a2dp_codec_type();
#endif
#ifdef A2DP_CP_ACCEL
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
            if(codec_type == BT_A2DP_CODEC_TYPE_LHDC)
            {
                app_bt_stream_trigger_start(device_id, APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME_LHDC - a2dp_audio_frame_delay_get());
            }
            else
#endif
            {
                app_bt_stream_trigger_start(device_id, APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME - a2dp_audio_frame_delay_get());
            }
#else
            app_bt_stream_trigger_start(device_id, APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME);
#endif
            APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T sync_trigger = {0,};
            A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info = {0,};
            sync_trigger.trigger_time = tg_acl_trigger_time;
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
            if(codec_type == BT_A2DP_CODEC_TYPE_LHDC)
            {
                sync_trigger.trigger_skip_frame = APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME_LHDC - a2dp_audio_frame_delay_get();
            }
            else
#endif
            {
                sync_trigger.trigger_skip_frame = APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME - a2dp_audio_frame_delay_get();
            }
            sync_trigger.trigger_type = APP_TWS_IBRT_AUDIO_TRIGGER_TYPE_INIT_SYNC;
            if (a2dp_audio_lastframe_info_get(&lastframe_info)<0){
                goto exit;
            }
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
            if(codec_type == BT_A2DP_CODEC_TYPE_LHDC)
            {
                sync_trigger.audio_info.sequenceNumber = lastframe_info.sequenceNumber + APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME_LHDC;
                sync_trigger.audio_info.timestamp = lastframe_info.timestamp + dma_buffer_samples * APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME_LHDC;
            }
            else
#endif
            {
                sync_trigger.audio_info.sequenceNumber = lastframe_info.sequenceNumber + APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME;
                sync_trigger.audio_info.timestamp = lastframe_info.timestamp + dma_buffer_samples * APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME;
            }
            sync_trigger.audio_info.curSubSequenceNumber = lastframe_info.curSubSequenceNumber;
            sync_trigger.audio_info.totalSubSequenceNumber = lastframe_info.totalSubSequenceNumber;
            sync_trigger.audio_info.frame_samples = lastframe_info.frame_samples;
            sync_trigger.factor_reference = a2dp_audio_get_output_config() ? a2dp_audio_get_output_config()->factor_reference : 1.0f;
            sync_trigger.a2dp_session = a2dp_ibrt_session_get(device_id);
            sync_trigger.handler_cnt = 0;

            app_bt_stream_ibrt_auto_synchronize_initsync_start(device_id, &sync_trigger);
        }
    }
exit:
    return 0;
}

int app_bt_stream_ibrt_audio_slave_detect_next_packet_start(int need_autotrigger)
{
    TRACE_AUD_STREAM_I("[AUTO_SYNC][SLAVE] start");
    slave_detect_next_packet_cnt = 0;
    if (need_autotrigger){
        a2dp_audio_detect_next_packet_callback_register(app_bt_stream_ibrt_audio_slave_detect_next_packet_waitforever_cb);
    }else{
        app_tws_ibrt_audio_analysis_start(0, AUDIO_ANALYSIS_CHECKER_INTERVEL_INVALID);
        app_tws_ibrt_audio_sync_start();
        a2dp_audio_detect_next_packet_callback_register(app_bt_stream_ibrt_audio_slave_detect_next_packet_cb);
    }
    return 0;
}


typedef enum {
    APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_IDLE,
    APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_ONPROCESS,
    APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_SYNCOK,
}APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_TYPE;

#define APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_CNT_LIMIT (100)
void app_bt_stream_ibrt_set_trigger_time(uint8_t device_id, APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger);
void app_bt_stream_ibrt_auto_synchronize_initsync_start(uint8_t device_id, APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger);
static APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T app_bt_stream_ibrt_auto_synchronize_trigger;
static uint32_t app_bt_stream_ibrt_auto_synchronize_cnt = 0;
int app_bt_stream_ibrt_audio_mismatch_stopaudio(uint8_t device_id);
void app_bt_stream_ibrt_auto_synchronize_hungup(void);
void app_bt_stream_ibrt_auto_synchronize_stop(void);
static APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_TYPE ibrt_auto_synchronize_status = APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_IDLE;

int app_bt_stream_ibrt_auto_synchronize_status_set(APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_TYPE status)
{
    TRACE_AUD_STREAM_I("[AUTO_SYNC] status set:%d", status);
    ibrt_auto_synchronize_status = status;
    return 0;
}

APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_TYPE app_bt_stream_ibrt_auto_synchronize_status_get(void)
{
    TRACE_AUD_STREAM_I("[AUTO_SYNC] get status:%d", ibrt_auto_synchronize_status);
    return ibrt_auto_synchronize_status;
}

int app_bt_stream_ibrt_auto_synchronize_trigger_start(uint8_t device_id, btif_media_header_t * header, unsigned char *buf, unsigned int len)
{
    APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger = &app_bt_stream_ibrt_auto_synchronize_trigger;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    app_bt_stream_ibrt_auto_synchronize_stop();
    TRACE_AUD_STREAM_I("[AUTO_SYNC] trigger:%d Seq:%d timestamp:%d SubSeq:%d/%d currSeq:%d",
                                                                                   sync_trigger->trigger_time,
                                                                                   sync_trigger->audio_info.sequenceNumber,
                                                                                   sync_trigger->audio_info.timestamp,
                                                                                   sync_trigger->audio_info.curSubSequenceNumber,
                                                                                   sync_trigger->audio_info.totalSubSequenceNumber,
                                                                                   header->sequenceNumber
                                                                                   );

    if (APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote)){
        if (sync_trigger->trigger_time >= bt_syn_get_curr_ticks(APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote))){
            app_bt_stream_ibrt_set_trigger_time(device_id, sync_trigger);
        }else{
            TRACE_AUD_STREAM_W("[AUTO_SYNC]failed trigger(%d)-->tg(%d) need resume",
                    bt_syn_get_curr_ticks(APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote)),
                    sync_trigger->trigger_time);

            app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_TRIGGER_FAIL);
            //app_tws_ibrt_audio_sync_mismatch_resume_notify(device_id);
        }
    }else{
        TRACE_AUD_STREAM_I("[AUTO_SYNC] ok but currRole:%d mismatch\n", app_tws_get_ibrt_role(&curr_device->remote));
        app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_ROLE_MISMATCH);
    }

    return 0;
}

int app_bt_stream_ibrt_auto_synchronize_dataind_cb(uint8_t device_id, btif_media_header_t * header, unsigned char *buf, unsigned int len)
{
    APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger = &app_bt_stream_ibrt_auto_synchronize_trigger;
    bool synchronize_ok = false;
    int32_t timestamp_diff = 0;
    int32_t dma_buffer_samples = 0;
    int32_t frame_totle_samples = 0;
    POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    frame_totle_samples = sync_trigger->audio_info.totalSubSequenceNumber * sync_trigger->audio_info.frame_samples;
    timestamp_diff = sync_trigger->audio_info.timestamp - header->timestamp;

    TRACE_AUD_STREAM_I("[AUTO_SYNC][DATAIND] seq:%d/%d timestamp:%d/%d cnt:%d", header->sequenceNumber,
                                                                                    sync_trigger->audio_info.sequenceNumber,
                                                                                    header->timestamp,
                                                                                    sync_trigger->audio_info.timestamp,
                                                                                    app_bt_stream_ibrt_auto_synchronize_cnt);

    if (++app_bt_stream_ibrt_auto_synchronize_cnt > APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_CNT_LIMIT){
        app_bt_stream_ibrt_auto_synchronize_stop();
        TRACE_AUD_STREAM_W("[AUTO_SYNC][DATAIND] SYNCHRONIZE_CNT_LIMIT, we need force retrigger");
        app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_SYNCHRONIZE_CNT_LIMIT);
    }else if (APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote)){
        app_bt_stream_ibrt_auto_synchronize_stop();
        TRACE_AUD_STREAM_W("[AUTO_SYNC][DATAIND] find role to master, we need force retrigger");
        app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_ROLE_MISMATCH);
    }else if (sync_trigger->audio_info.sequenceNumber < header->sequenceNumber){
        app_bt_stream_ibrt_auto_synchronize_stop();
        TRACE_AUD_STREAM_W("[AUTO_SYNC][DATAIND] seq timestamp:%d/%d mismatch need resume", header->timestamp,
                                                                                               sync_trigger->audio_info.timestamp);
        app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_SEQ_MISMATCH);
    }else{
        if (header->sequenceNumber >= sync_trigger->audio_info.sequenceNumber &&
            !sync_trigger->audio_info.totalSubSequenceNumber){
            synchronize_ok = true;
        }else if (header->timestamp == sync_trigger->audio_info.timestamp){
            synchronize_ok = true;
        }

        dma_buffer_samples = app_bt_stream_get_dma_buffer_samples()/2;

        if (sync_trigger->audio_info.timestamp >= header->timestamp && sync_trigger->audio_info.totalSubSequenceNumber){
            if (timestamp_diff < dma_buffer_samples){
                TRACE_AUD_STREAM_I("[AUTO_SYNC][DATAIND] timestamp_diff < dma_buffer_samples synchronize ok");
                synchronize_ok = true;
            }else if (timestamp_diff < frame_totle_samples){
                TRACE_AUD_STREAM_I("[AUTO_SYNC][DATAIND] timestamp_diff < frame_totle_samples synchronize ok");
                synchronize_ok = true;
            }
        }

        if (!synchronize_ok && header->sequenceNumber >= sync_trigger->audio_info.sequenceNumber){

            TRACE_AUD_STREAM_I("[AUTO_SYNC][DATAIND] timestamp %d vs %d", sync_trigger->audio_info.timestamp - header->timestamp,
                                                                      frame_totle_samples);
            if ((sync_trigger->audio_info.timestamp - header->timestamp) <= (uint32_t)(frame_totle_samples*3)){
                sync_trigger->audio_info.sequenceNumber++;
                TRACE_AUD_STREAM_I("[AUTO_SYNC][DATAIND] timestamp try sequenceNumber:%d", header->sequenceNumber);
            }
        }

        //flush all
        a2dp_audio_synchronize_dest_packet_mut(0);

        if (synchronize_ok){
            A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info;
            if (a2dp_audio_lastframe_info_get(&lastframe_info) < 0){
                TRACE_AUD_STREAM_I("[AUTO_SYNC][DATAIND]synchronize ok but lastframe error");
                goto exit;
            }

            TRACE_AUD_STREAM_I("[AUTO_SYNC][DATAIND]synchronize ok timestamp_diff:%d frame_samples:%d", timestamp_diff,
                                                                                                    lastframe_info.frame_samples);
            sync_trigger->trigger_type = APP_TWS_IBRT_AUDIO_TRIGGER_TYPE_LOCAL;
            sync_trigger->audio_info.sequenceNumber = header->sequenceNumber;
            sync_trigger->audio_info.timestamp = header->timestamp;
            if (sync_trigger->audio_info.totalSubSequenceNumber){
                sync_trigger->audio_info.curSubSequenceNumber = timestamp_diff/lastframe_info.frame_samples;
                TRACE_AUD_STREAM_I("[AUTO_SYNC][DATAIND]synchronize ok tstmp_diff:%d/%d SubSeq:%d", timestamp_diff, sync_trigger->audio_info.frame_samples,
                                                                                           sync_trigger->audio_info.curSubSequenceNumber);
            }else{
                sync_trigger->audio_info.curSubSequenceNumber = 0;
            }
            sync_trigger->audio_info.totalSubSequenceNumber = lastframe_info.totalSubSequenceNumber;
            sync_trigger->audio_info.frame_samples = lastframe_info.frame_samples;
            if (sync_trigger->audio_info.totalSubSequenceNumber &&
                sync_trigger->audio_info.curSubSequenceNumber >= sync_trigger->audio_info.totalSubSequenceNumber){
                TRACE_AUD_STREAM_W("[AUTO_SYNC][DATAIND]synchronize ok but sbc & timestamp is ms so force trigger");
                app_bt_stream_ibrt_auto_synchronize_stop();
                app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_SEQ_MISMATCH);
            }else{
                a2dp_audio_detect_store_packet_callback_register(app_bt_stream_ibrt_auto_synchronize_trigger_start);
            }
        }else{
            a2dp_audio_detect_first_packet();
        }
    }
exit:
    return 0;
}

void app_bt_stream_ibrt_auto_synchronize_start(APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger)
{
    TRACE_AUD_STREAM_I("[AUTO_SYNC][DATAIND] trigger_time:%d seq:%d timestamp:%d SubSeq:%d/%d", sync_trigger->trigger_time,
                                                                                       sync_trigger->audio_info.sequenceNumber,
                                                                                       sync_trigger->audio_info.timestamp,
                                                                                       sync_trigger->audio_info.curSubSequenceNumber,
                                                                                       sync_trigger->audio_info.totalSubSequenceNumber
                                                                                       );
    app_bt_stream_ibrt_auto_synchronize_status_set(APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_ONPROCESS);
    app_bt_stream_ibrt_auto_synchronize_cnt = 0;
    app_bt_stream_ibrt_auto_synchronize_trigger = *sync_trigger;
    a2dp_audio_detect_next_packet_callback_register(app_bt_stream_ibrt_auto_synchronize_dataind_cb);
    a2dp_audio_detect_first_packet();
}


void app_bt_stream_ibrt_auto_synchronize_hungup(void)
{
    a2dp_audio_detect_next_packet_callback_register(NULL);
    a2dp_audio_detect_store_packet_callback_register(NULL);
}

void app_bt_stream_ibrt_auto_synchronize_stop(void)
{
    app_bt_stream_ibrt_auto_synchronize_hungup();
    app_bt_stream_ibrt_auto_synchronize_cnt = 0;
    app_bt_stream_ibrt_auto_synchronize_status_set(APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_IDLE);
}

void app_bt_stream_ibrt_reset_synchronize_status(void)
{
    app_bt_stream_ibrt_auto_synchronize_status_set(APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_IDLE);
}

bool app_bt_stream_ibrt_auto_synchronize_on_porcess(void)
{
    bool nRet = true;
    APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_TYPE synchronize_status = app_bt_stream_ibrt_auto_synchronize_status_get();
    if (synchronize_status == APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_IDLE){
        nRet = false;
    }
    return nRet;
}

void app_bt_stream_ibrt_start_sbc_player_callback(uint8_t device_id, uint32_t status, uint32_t param)
{
    if (app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC)){
        TRACE_AUD_STREAM_I("start_sbc_player_cb trigger(%d)-->tg(%d)", param, ((APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *)param)->trigger_time);
        app_bt_stream_ibrt_set_trigger_time(device_id, (APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *)param);
    }else{
        TRACE_AUD_STREAM_I("start_sbc_player_cb try again");
        app_audio_manager_sendrequest_need_callback(
                                                    APP_BT_STREAM_MANAGER_START,BT_STREAM_MUSIC,
                                                    device_id,
                                                    MAX_RECORD_NUM,
                                                    (uint32_t)app_bt_stream_ibrt_start_sbc_player_callback,
                                                    param);
    }
}

int app_bt_stream_ibrt_start_sbc_player(uint8_t device_id, APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger)
{
    TRACE_AUD_STREAM_I("start_sbc_player tg(%d)", sync_trigger->trigger_time);
    app_audio_manager_sendrequest_need_callback(
                                                APP_BT_STREAM_MANAGER_START,BT_STREAM_MUSIC,
                                                device_id,
                                                MAX_RECORD_NUM,
                                                (uint32_t)app_bt_stream_ibrt_start_sbc_player_callback,
                                                (uint32_t)sync_trigger);
    return 0;
}

uint16_t app_bt_stream_ibrt_trigger_seq_diff_calc(int32_t dma_samples, int32_t frame_samples, int32_t total_subseq, int32_t interval)
{
    float seq_factor = 1.0f;
    if (total_subseq){
        seq_factor = (float)(dma_samples/frame_samples)/(float)total_subseq;
    }else{
        seq_factor = (float)(dma_samples/frame_samples);
    }
    return (uint16_t)(seq_factor * (float)interval);
}

#define MOBILE_LINK_PLAYBACK_INFO_TRIG_DUMMY_DMA_CNT (8)
#define SYNCHRONIZE_DATAIND_CNT_LIMIT (25)

static int synchronize_need_discards_dma_cnt = 0;
static int synchronize_dataind_cnt = 0;

int app_bt_stream_ibrt_auto_synchronize_initsync_dataind_cb_v2(uint8_t device_id, btif_media_header_t * header, unsigned char *buf, unsigned int len)
{
    bool synchronize_ok = false;
    bool discards_samples_finished = false;
    int dest_discards_samples = 0;
    uint32_t list_samples = 0;
    uint32_t curr_ticks = 0;
    A2DP_AUDIO_HEADFRAME_INFO_T headframe_info;
    APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger = &app_bt_stream_ibrt_auto_synchronize_trigger;
    POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    synchronize_dataind_cnt++;
    if (APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote)){
        TRACE_AUD_STREAM_W("[AUTO_SYNCV2] mobile_link is connect retrigger because role switch");
        app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_ROLE_SWITCH);
        return 0;
    }

    if (synchronize_dataind_cnt >= SYNCHRONIZE_DATAIND_CNT_LIMIT){
        TRACE_AUD_STREAM_W("[AUTO_SYNCV2] mobile_link is connect retrigger because CNT_LIMIT");
        app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_SYNCHRONIZE_CNT_LIMIT);
        return 0;
    }

    is_need_discards_samples = false;
    dest_discards_samples = app_bt_stream_get_dma_buffer_samples()/2*synchronize_need_discards_dma_cnt;
    a2dp_audio_convert_list_to_samples(&list_samples);
    if ((int)list_samples >= dest_discards_samples){
        discards_samples_finished = true;
        a2dp_audio_discards_samples(dest_discards_samples);
    }
    a2dp_audio_decoder_headframe_info_get(&headframe_info);
    TRACE_AUD_STREAM_I("[AUTO_SYNCV2] sample:%d->%d seq:%d sub_seq:%d/%d",
                                                      list_samples,dest_discards_samples,
                                                      headframe_info.sequenceNumber,
                                                      headframe_info.curSubSequenceNumber,
                                                      headframe_info.totalSubSequenceNumber);

    if (discards_samples_finished){
        uint32_t lock = int_lock();
        bool trig_valid = false;
        curr_ticks = bt_syn_get_curr_ticks(APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote));
        if (sync_trigger->trigger_time > curr_ticks){
            trig_valid =  true;
        }else{
            TRACE_AUD_STREAM_I("[AUTO_SYNCV2] trig:%x/%x", curr_ticks, sync_trigger->trigger_time);
        }
        if (trig_valid){
            synchronize_ok = true;
            tg_acl_trigger_time = sync_trigger->trigger_time;
            btdrv_syn_clr_trigger(0);
            btdrv_enable_playback_triggler(ACL_TRIGGLE_MODE);
#if BLE_AUDIO_ENABLED
            af_codec_set_device_bt_sync_source(AUD_STREAM_USE_INT_CODEC, AUD_STREAM_PLAYBACK, 0);
#endif
            bt_syn_set_tg_ticks(sync_trigger->trigger_time, APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote), BT_TRIG_SLAVE_ROLE,0,false);
        }
        int_unlock(lock);
        if (trig_valid){
            app_tws_ibrt_audio_analysis_start(sync_trigger->handler_cnt, AUDIO_ANALYSIS_CHECKER_INTERVEL_INVALID);
            app_tws_ibrt_audio_sync_start();
            app_tws_ibrt_audio_sync_new_reference(sync_trigger->factor_reference);
            TRACE_AUD_STREAM_I("[AUTO_SYNCV2] trigger curr(%d)-->tg(%d)",
                                                                bt_syn_get_curr_ticks(APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote)),
                                                                sync_trigger->trigger_time);
            synchronize_need_discards_dma_cnt = 0;
            synchronize_dataind_cnt = 0;
            a2dp_audio_detect_first_packet_clear();
            a2dp_audio_detect_next_packet_callback_register(NULL);
            a2dp_audio_detect_store_packet_callback_register(NULL);
            TRACE_AUD_STREAM_I("[AUTO_SYNCV2] synchronize_ok");
            return 0;
        }else{
            TRACE_AUD_STREAM_I("[AUTO_SYNCV2] synchronize_failed");
            app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_SYNC_FAIL);
            return 0;
        }
    }

    if (!synchronize_ok){
        a2dp_audio_detect_first_packet();
    }

    return 0;
}


void app_bt_stream_ibrt_mobile_link_playback_info_receive(uint8_t device_id, APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger)
{
    uint32_t tg_tick = 0;
    uint32_t next_dma_cnt = 0;
    A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info;
    A2DP_AUDIO_SYNCFRAME_INFO_T sync_info;
    A2DP_AUDIO_HEADFRAME_INFO_T headframe_info;
    APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger_loc = &app_bt_stream_ibrt_auto_synchronize_trigger;
    struct BT_DEVICE_T* POSSIBLY_UNUSED p_bt_device = app_bt_get_device(device_id);

    TRACE_AUD_STREAM_I("[AUTO_SYNCV2][INFO_RECV] session:%d hdl:%d clk:%d cnt:%d seq:%d/%d/%d", sync_trigger->a2dp_session,
                                                                                    sync_trigger->handler_cnt,
                                                                                    sync_trigger->trigger_bt_clk,
                                                                                    sync_trigger->trigger_bt_cnt,
                                                                                    sync_trigger->audio_info.sequenceNumber,
                                                                                    sync_trigger->audio_info.curSubSequenceNumber,
                                                                                    sync_trigger->audio_info.totalSubSequenceNumber);

    if (app_bt_stream_ibrt_auto_synchronize_on_porcess()){
        TRACE_AUD_STREAM_W("[AUTO_SYNCV2][INFO_RECV] auto_synchronize_on_porcess skip it");
        return;
    }

    if (!app_bt_is_a2dp_streaming(device_id)){
        TRACE_AUD_STREAM_W("[AUTO_SYNCV2][INFO_RECV] streaming not ready skip it");
        app_bt_stream_ibrt_audio_mismatch_stopaudio(device_id);
        return;
    }

    if (a2dp_ibrt_session_get(device_id) != sync_trigger->a2dp_session){
        TRACE_AUD_STREAM_W("[AUTO_SYNCV2][INFO_RECV] session mismatch skip it loc:%d rmt:%d", a2dp_ibrt_session_get(device_id), sync_trigger->a2dp_session);
        TRACE_AUD_STREAM_W("[AUTO_SYNCV2][INFO_RECV] session froce resume and try retrigger");
        a2dp_ibrt_session_set(sync_trigger->a2dp_session,device_id);
        app_bt_stream_ibrt_audio_mismatch_stopaudio(device_id);
        return;
    }

    if (a2dp_audio_lastframe_info_get(&lastframe_info) < 0){
        TRACE_AUD_STREAM_W("[AUTO_SYNCV2][INFO_RECV] lastframe not ready mismatch_stopaudio");
        app_bt_stream_ibrt_audio_mismatch_stopaudio(device_id);
        return;
    }
    a2dp_audio_decoder_headframe_info_get(&headframe_info);

    *sync_trigger_loc = *sync_trigger;

    sync_info.sequenceNumber = sync_trigger->audio_info.sequenceNumber;
    sync_info.timestamp = sync_trigger->audio_info.timestamp;
    sync_info.curSubSequenceNumber = sync_trigger->audio_info.curSubSequenceNumber;
    sync_info.totalSubSequenceNumber = sync_trigger->audio_info.totalSubSequenceNumber;
    sync_info.frame_samples = sync_trigger->audio_info.frame_samples;

    int32_t discard_cnt = 0;
    int16_t pk_diff = headframe_info.sequenceNumber - sync_trigger->audio_info.sequenceNumber;
    int32_t dma_mtu = app_bt_stream_get_dma_buffer_samples() / 2 / lastframe_info.frame_samples;

    if (headframe_info.totalSubSequenceNumber) {
        pk_diff *= headframe_info.totalSubSequenceNumber;
    }
    if (pk_diff > 0 && pk_diff < (MOBILE_LINK_PLAYBACK_INFO_TRIG_DUMMY_DMA_CNT + 2) * dma_mtu)
    {
        discard_cnt = (pk_diff / dma_mtu) * (-1);
    }
    else
    {
        if (a2dp_audio_synchronize_packet(&sync_info, A2DP_AUDIO_SYNCFRAME_MASK_SEQ)){
            TRACE(0,"[playback_info_receive] synchronize_packe mismatch");
            app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_SYNC_MISMATCH);
            return;
        }
    }

    TRACE_AUD_STREAM_I("[AUTO_SYNCV2][INFO_RECV] sync with master packet step1 seq:%d sub_seq:%d/%d",
                                                                  headframe_info.sequenceNumber,
                                                                  headframe_info.curSubSequenceNumber,
                                                                  headframe_info.totalSubSequenceNumber);
    is_need_discards_samples = false;
    a2dp_audio_discards_samples(lastframe_info.list_samples);
    a2dp_audio_decoder_headframe_info_get(&headframe_info);
    TRACE_AUD_STREAM_I("[AUTO_SYNCV2][INFO_RECV] sync with master packet step2 seq:%d sub_seq:%d/%d",
                                                                  headframe_info.sequenceNumber,
                                                                  headframe_info.curSubSequenceNumber,
                                                                  headframe_info.totalSubSequenceNumber);

    uint32_t btclk;
    uint16_t btcnt;
    uint32_t mobile_master_clk = 0;
    uint16_t mobile_master_cnt = 0;
    int64_t mobile_master_us = 0;

#if defined(BT_CLK_UNIT_IN_HSLOT)
    btclk = btdrv_syn_get_curr_ticks();
#else
    btclk = btdrv_syn_get_curr_ticks()/2;
#endif
    btcnt = 0;
    app_tws_ibrt_audio_mobile_clkcnt_get(device_id, btclk, btcnt,
                                         &mobile_master_clk, &mobile_master_cnt);

    mobile_master_us = btdrv_clkcnt_to_us(mobile_master_clk,mobile_master_cnt);
    uint32_t rmt_mobile_master_clk = sync_trigger->trigger_bt_clk;
    uint16_t rmt_mobile_master_cnt = sync_trigger->trigger_bt_cnt;
    int64_t rmt_mobile_master_us = 0;
    int64_t tmp_mobile_master_us = 0;
    rmt_mobile_master_us = btdrv_clkcnt_to_us(rmt_mobile_master_clk,rmt_mobile_master_cnt);

    uint32_t dma_buffer_us = 0;
    dma_buffer_us = app_bt_stream_get_dma_buffer_delay_us()/2;

    tmp_mobile_master_us = rmt_mobile_master_us;
    do{
        if (tmp_mobile_master_us - mobile_master_us >= 0){
            break;
        }
        tmp_mobile_master_us += dma_buffer_us;
        next_dma_cnt++;
    }while(1);
    next_dma_cnt += MOBILE_LINK_PLAYBACK_INFO_TRIG_DUMMY_DMA_CNT ;
    tmp_mobile_master_us += dma_buffer_us * (MOBILE_LINK_PLAYBACK_INFO_TRIG_DUMMY_DMA_CNT-1);
    synchronize_need_discards_dma_cnt = next_dma_cnt + a2dp_audio_frame_delay_get() - 1 + discard_cnt;
    synchronize_dataind_cnt = 0;
    TRACE_AUD_STREAM_I("[AUTO_SYNCV2][INFO_RECV] loc:%08x%08x rmt:%08x%08x tg:%08x%08x", (uint32_t)((uint64_t)mobile_master_us>>32U), (uint32_t)((uint64_t)mobile_master_us&0xffffffff),
                                                                             (uint32_t)((uint64_t)rmt_mobile_master_us>>32U), (uint32_t)((uint64_t)rmt_mobile_master_us&0xffffffff),
                                                                             (uint32_t)((uint64_t)tmp_mobile_master_us>>32U), (uint32_t)((uint64_t)tmp_mobile_master_us&0xffffffff));

    tmp_mobile_master_us = tmp_mobile_master_us/SLOT_SIZE;

    tg_tick = tmp_mobile_master_us * 2;
    tg_tick &= 0x0fffffff;

    sync_trigger_loc->trigger_type = APP_TWS_IBRT_AUDIO_TRIGGER_TYPE_LOCAL;
    sync_trigger_loc->trigger_time = tg_tick;
    sync_trigger_loc->handler_cnt += next_dma_cnt;
    a2dp_audio_detect_next_packet_callback_register(app_bt_stream_ibrt_auto_synchronize_initsync_dataind_cb_v2);
    a2dp_audio_detect_first_packet();


    TRACE_AUD_STREAM_I("[AUTO_SYNCV2][INFO_RECV] mobile clk:%x/%x tg:%x",
    bt_syn_get_curr_ticks(APP_IBRT_UI_GET_IBRT_HANDLE(&p_bt_device->remote))/2,mobile_master_clk,tg_tick/2);

    TRACE_AUD_STREAM_I("[AUTO_SYNCV2][INFO_RECV] master_us:%x/%x/%x dma_cnt:%d/%d", (int32_t)mobile_master_us, (int32_t)rmt_mobile_master_us, (int32_t)tmp_mobile_master_us,
                                                                         next_dma_cnt, sync_trigger_loc->handler_cnt);
}


void app_bt_stream_ibrt_set_trigger_time(uint8_t device_id, APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger)
{
    uint32_t curr_ticks = 0;
    uint32_t tg_tick = sync_trigger->trigger_time;
    A2DP_AUDIO_SYNCFRAME_INFO_T sync_info;
    int synchronize_ret;
    A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info;
    A2DP_AUDIO_HEADFRAME_INFO_T headframe_info;

    if (app_bt_stream_ibrt_auto_synchronize_on_porcess()){
        TRACE_AUD_STREAM_W("[STRM_TRIG][A2DP][IBRT] auto_synchronize_on_porcess skip it");
        return;
    }

    if (!app_bt_is_a2dp_streaming(device_id)){
        TRACE_AUD_STREAM_W("[STRM_TRIG][A2DP][IBRT] streaming not ready skip it");
        app_bt_stream_ibrt_audio_mismatch_stopaudio(device_id);
        return;
    }

    if (a2dp_ibrt_session_get(device_id) != sync_trigger->a2dp_session){
        TRACE_AUD_STREAM_W("[STRM_TRIG][A2DP][IBRT] session mismatch skip it loc:%d rmt:%d", a2dp_ibrt_session_get(device_id), sync_trigger->a2dp_session);
        TRACE_AUD_STREAM_W("[STRM_TRIG][A2DP][IBRT] session froce resume and try retrigger");
        a2dp_ibrt_session_set(sync_trigger->a2dp_session,device_id);
        app_bt_stream_ibrt_audio_mismatch_stopaudio(device_id);
        return;
    }

    if (a2dp_audio_lastframe_info_get(&lastframe_info) < 0){
        TRACE_AUD_STREAM_W("[STRM_TRIG][A2DP][IBRT] lastframe not ready mismatch_stopaudio");
        app_bt_stream_ibrt_audio_mismatch_stopaudio(device_id);
        return;
    }

    struct BT_DEVICE_T* POSSIBLY_UNUSED p_bt_device = app_bt_get_device(device_id);

    if (a2dp_audio_decoder_headframe_info_get(&headframe_info) < 0){
        TRACE_AUD_STREAM_W("[STRM_TRIG][A2DP][IBRT] lastframe not ready mismatch_stopaudio");
        app_bt_stream_ibrt_audio_mismatch_stopaudio(device_id);
        goto exit;
    }
    TRACE_AUD_STREAM_W("[STRM_TRIG][A2DP][IBRT] info base_seq:%d/%d", headframe_info.sequenceNumber, sync_trigger->sequenceNumberStart);

    a2dp_audio_detect_next_packet_callback_register(NULL);
    a2dp_audio_detect_store_packet_callback_register(NULL);

    sync_info.sequenceNumber = sync_trigger->sequenceNumberStart;

    synchronize_ret = 0;
    if ((headframe_info.sequenceNumber>sync_trigger->sequenceNumberStart)
        &&((headframe_info.sequenceNumber-sync_trigger->sequenceNumberStart)<APP_BT_STREAM_MISMATCH_PACKET_THRESHOLD))
    {
        TRACE_AUD_STREAM_W("[STRM_TRIG][A2DP][IBRT] synchronize_packet failed");
        TRACE_AUD_STREAM_W("[STRM_TRIG][A2DP][IBRT] do_not_retrigger");
        uint8_t diff_subframe = 0;
        if (0 == headframe_info.totalSubSequenceNumber)
        {
            diff_subframe = headframe_info.sequenceNumber-sync_trigger->sequenceNumberStart;
        }
        else
        {
            diff_subframe = (headframe_info.sequenceNumber-sync_trigger->sequenceNumberStart)*headframe_info.totalSubSequenceNumber;
        }
        app_tws_ibrt_audio_analysis_set_refill_frames(diff_subframe);
    }
    else
    {
        synchronize_ret = a2dp_audio_synchronize_packet(&sync_info, A2DP_AUDIO_SYNCFRAME_MASK_SEQ);
    }
    if (synchronize_ret){
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][IBRT] synchronize_packet failed");
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][IBRT] force_audio_retrigger");
        app_bt_stream_ibrt_auto_synchronize_stop();
        app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_SYNC_FAIL);
        goto exit;
    }

    curr_ticks = bt_syn_get_curr_ticks(APP_IBRT_UI_GET_IBRT_HANDLE(&p_bt_device->remote));
    if (tg_tick < curr_ticks){
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][IBRT] synchronize tick failed:%x->%x", curr_ticks, tg_tick);
        TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][IBRT] force_audio_retrigger");
        app_bt_stream_ibrt_auto_synchronize_stop();
        app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_SYNC_FAIL);
        goto exit;
    }
    sync_info.sequenceNumber = sync_trigger->audio_info.sequenceNumber;
    sync_info.timestamp = sync_trigger->audio_info.timestamp;
    sync_info.curSubSequenceNumber = sync_trigger->audio_info.curSubSequenceNumber;
    sync_info.totalSubSequenceNumber = sync_trigger->audio_info.totalSubSequenceNumber;
    sync_info.frame_samples = sync_trigger->audio_info.frame_samples;

    if (APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&p_bt_device->remote)){
        if (app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC)){
            if (sync_trigger->trigger_type == APP_TWS_IBRT_AUDIO_TRIGGER_TYPE_INIT_SYNC){
                TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][IBRT] TRIGGER_TYPE_INIT_SYNC needskip:%d", sync_trigger->trigger_skip_frame);
                //limter to water line upper
                uint32_t list_samples = 0;
                uint32_t limter_water_line_samples = 0;
                limter_water_line_samples = (a2dp_audio_dest_packet_mut_get() * lastframe_info.list_samples);
                a2dp_audio_convert_list_to_samples(&list_samples);
                TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][IBRT] synchronize:%d/%d", list_samples, limter_water_line_samples);
                if (list_samples > limter_water_line_samples){
                    is_need_discards_samples = true;
                    // TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][IBRT] skip discards:%d", list_samples - limter_water_line_samples);
                    // a2dp_audio_discards_samples(list_samples - limter_water_line_samples);
                }
                app_bt_stream_ibrt_auto_synchronize_initsync_start(device_id, sync_trigger);
                app_bt_stream_ibrt_auto_synchronize_status_set(APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_SYNCOK);
            }
        }else if (!app_bt_stream_isrun(APP_BT_STREAM_HFP_PCM)){
            TRACE_AUD_STREAM_I("[STRM_TRIG][A2DP][IBRT] sbc player not active, so try to start it");
            app_bt_stream_ibrt_auto_synchronize_trigger = *sync_trigger;
            app_bt_stream_ibrt_audio_mismatch_stopaudio(device_id);
        }
    }else{
        TRACE_AUD_STREAM_W("[STRM_TRIG][A2DP][IBRT] Not Connected");
    }
exit:
    return;
}

void app_bt_stream_ibrt_audio_mismatch_resume(uint8_t device_id)
{
    ibrt_a2dp_status_t a2dp_status;

    POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    a2dp_ibrt_sync_get_status(device_id, &a2dp_status);

    TRACE_AUD_STREAM_I("[MISMATCH] resume state:%d", a2dp_status.state);

    if (a2dp_status.state == BT_A2DP_STREAM_STATE_STREAMING){
        if (APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote)){
            TRACE_AUD_STREAM_I("[MISMATCH] resume find role switch so force retrigger");
            app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_ROLE_SWITCH);
        }else{
            app_tws_ibrt_audio_sync_mismatch_resume_notify(device_id);
        }
    }
}

void app_bt_stream_ibrt_audio_mismatch_stopaudio_cb(uint8_t device_id, uint32_t status, uint32_t param)
{
    TRACE_AUD_STREAM_I("[MISMATCH] stopaudio_cb");

    if (app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC)){
        TRACE_AUD_STREAM_I("[MISMATCH] stopaudio_cb try again");
        app_audio_manager_sendrequest_need_callback(
                                                    APP_BT_STREAM_MANAGER_STOP,BT_STREAM_MUSIC,
                                                    device_id,
                                                    MAX_RECORD_NUM,
                                                    (uint32_t)app_bt_stream_ibrt_audio_mismatch_stopaudio_cb,
                                                    (uint32_t)NULL);
    }else{
        app_bt_stream_ibrt_audio_mismatch_resume(device_id);
    }
}

int app_bt_stream_ibrt_audio_mismatch_stopaudio(uint8_t device_id)
{
    ibrt_a2dp_status_t a2dp_status;
    POSSIBLY_UNUSED struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);

    a2dp_ibrt_sync_get_status(device_id, &a2dp_status);

    TRACE_AUD_STREAM_I("[MISMATCH] stopaudio state:%d sco:%d sbc:%d media:%d", a2dp_status.state,
                                                 app_audio_manager_hfp_is_active(device_id),
                                                 app_audio_manager_a2dp_is_active(device_id),
                                                 app_bt_stream_isrun(APP_PLAY_BACK_AUDIO));

    if (a2dp_status.state == BT_A2DP_STREAM_STATE_STREAMING){
        if (app_audio_manager_a2dp_is_active(device_id)){
            TRACE_AUD_STREAM_I("[MISMATCH] stopaudio");
            app_audio_sendrequest_param(APP_BT_STREAM_A2DP_SBC, (uint8_t)APP_BT_SETTING_RESTART, 0, APP_SYSFREQ_52M);
            app_bt_stream_ibrt_audio_mismatch_resume(device_id);
        }else{
            if (APP_IBRT_IS_PROFILE_EXCHANGED(&curr_device->remote)){
                if (!bt_media_is_music_media_active()){
                    TRACE_AUD_STREAM_I("[MISMATCH] stopaudio not active resume it & force retrigger");
                    app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_PLAYER_NOT_ACTIVE);
                }else{
                    if (app_bt_stream_isrun(APP_PLAY_BACK_AUDIO)
#ifdef MEDIA_PLAYER_SUPPORT
                        &&app_play_audio_get_aud_id() == AUDIO_ID_BT_MUTE
#endif
                        ){
                        TRACE_AUD_STREAM_I("[MISMATCH] stopaudio resum on process skip it");
                    }else{
                        TRACE_AUD_STREAM_I("[MISMATCH] stopaudio cancel_media and force retrigger");
                        app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_UNKNOW);
                    }
                }
            }else{
                TRACE_AUD_STREAM_I("[MISMATCH] stopaudio profile not exchanged skip it");
            }
        }
    }

    return 0;
}
#endif
