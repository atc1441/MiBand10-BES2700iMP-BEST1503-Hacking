/***************************************************************************
 *
 * Copyright 2015-2020 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifdef BT_HFP_SUPPORT
#if defined(BT_HFP_AG_ROLE)
#include "bt_source.h"
#include "app_hfp_ag.h"
#include "btapp.h"
#include "app_bt.h"
#include "app_trace_rx.h"
#include "app_bt_func.h"
#include "audio_policy.h"
#include "bt_drv_interface.h"
#include "app_hfp.h"
#include "app_battery.h"
#include "bt_drv_reg_op.h"
#include "app_bt_media_manager.h"

#ifdef HFP_AG_TEST
void app_hfp_ag_connect_test(void)
{
    bt_bdaddr_t rmt_addr = {{0}};
    app_bt_source_set_source_addr(rmt_addr.address);

    //memcpy(&rmt_addr->address[0],&source_test_bt_addr[0],6);
    TRACE(0,"hfp ag connect addr:");
    DUMP8("%x ",&rmt_addr.address[0],BT_ADDR_OUTPUT_PRINT_NUM);

    bt_source_reconnect_hfp_profile(&rmt_addr);
}

void app_hfp_ag_create_audio_link_test(void)
{
    bt_bdaddr_t rmt_addr = {{0}};
    app_bt_source_set_source_addr(rmt_addr.address);

    //memcpy(&rmt_addr->address[0],&source_test_bt_addr[0],6);
    TRACE(0,"hfp ag connect audio link addr:");
    DUMP8("%x ",&rmt_addr.address[0],BT_ADDR_OUTPUT_PRINT_NUM);

    bt_source_create_audio_link(&rmt_addr);
}

void app_hfp_ag_disc_audio_link_test(void)
{
    bt_bdaddr_t rmt_addr = {{0}};
    app_bt_source_set_source_addr(rmt_addr.address);

    //memcpy(&rmt_addr->address[0],&source_test_bt_addr[0],6);
    TRACE(0,"hfp ag disc audio link addr:");
    DUMP8("%x ",&rmt_addr.address[0],BT_ADDR_OUTPUT_PRINT_NUM);

    bt_source_disc_audio_link(&rmt_addr);
}

const app_hfp_ag_uart_handle_t app_hfp_ag_uart_test_handle[]=
{
    {"connect_test",app_hfp_ag_connect_test},
    {"create_audio_link",app_hfp_ag_create_audio_link_test},
    {"disc_audio_link",app_hfp_ag_disc_audio_link_test},
};


app_hfp_ag_uart_test_function_handle app_hfp_ag_test_find_uart_handle(unsigned char* buf)
{
    app_hfp_ag_uart_test_function_handle p = NULL;

    for(uint8_t i = 0; i<sizeof(app_hfp_ag_uart_test_handle)/sizeof(app_hfp_ag_uart_handle_t); i++)
    {
        if(strncmp((char*)buf, app_hfp_ag_uart_test_handle[i].string, strlen(app_hfp_ag_uart_test_handle[i].string))==0 ||
        strstr(app_hfp_ag_uart_test_handle[i].string, (char*)buf))
        {
            p = app_hfp_ag_uart_test_handle[i].function;
            break;
        }
    }
    return p;
}

int app_hfp_ag_uart_cmd_handler(unsigned char *buf, unsigned int length)
{
    int ret = 0;

    app_hfp_ag_uart_test_function_handle handl_function = app_hfp_ag_test_find_uart_handle(buf);
    if(handl_function)
    {
        handl_function();
    }
    else
    {
        ret = -1;
        TRACE(0,"hfp ag can not find handle function");
    }
    return ret;
}

unsigned int app_hfp_ag_uart_cmd_callback(unsigned char *buf, unsigned int len)
{
    // Check len
    TRACE(2,"[%s] len = %d", __func__, len);
    app_hfp_ag_uart_cmd_handler((unsigned char*)buf,strlen((char*)buf));
    return 0;
}

void app_hfp_ag_uart_cmd_init(void)
{
    TRACE(0,"hfp ag uart cmd init");
    app_trace_rx_register("hfp_ag", app_hfp_ag_uart_cmd_callback);
}
#endif

void app_hfp_ag_event_callback(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
    struct BT_SOURCE_DEVICE_T* curr_device = NULL;
    uint16_t codec_id = 0;
#if defined(SUPPORT_BATTERY_REPORT) || defined(SUPPORT_HF_INDICATORS)
    uint8_t battery_level = 0;
#endif

    if (device_id == BT_DEVICE_INVALID_ID && ctx->event == BTIF_HF_EVENT_SERVICE_DISCONNECTED)
    {
        // hfp profile is closed due to acl created fail
        TRACE(2,"%s ::HF_EVENT_SERVICE_DISCONNECTED acl created error=%x", __func__, ctx->disc_reason);
        return;
    }

    curr_device = app_bt_source_get_device(device_id);

    ASSERT(device_id >= BT_SOURCE_DEVICE_ID_BASE &&
           device_id < (BT_SOURCE_DEVICE_ID_BASE + BT_SOURCE_DEVICE_NUM) &&
           curr_device->base_device->hf_channel == chan, "hfp gateway device channel must match");

    switch(ctx->event)
    {
    case BTIF_HF_EVENT_REMOTE_NOT_SUPPORT:
        TRACE(2,"(d%x) %s ::HF_EVENT_REMOTE_NOT_SUPPORT", device_id, __func__);
        break;
    case BTIF_HF_EVENT_SERVICE_CONNECTED:
        TRACE(3,"(d%x) %s ::HF_EVENT_SERVICE_CONNECTED reason=%x", device_id, __func__, ctx->disc_reason);

        curr_device->base_device->hf_conn_flag = 1;

        app_bt_source_audio_event_handler(device_id, APP_BT_SOURCE_AUDIO_EVENT_AG_SERVICE_CONNECTED, NULL);

#if defined(SUPPORT_BATTERY_REPORT) || defined(SUPPORT_HF_INDICATORS)
        app_hfp_battery_report_reset(device_id);
        app_battery_get_info(NULL, &battery_level, NULL);
        app_hfp_set_battery_level(battery_level);
#endif

#if defined(BT_HFP_AG_ROLE) && defined(HFP_AG_SCO_AUTO_CONN)
        TRACE(1, "(d%x) hfp ag create audio link...", device_id);
        btif_ag_create_audio_link(app_bt_get_device(device_id)->hf_channel);
#endif
        break;
    case BTIF_HF_EVENT_SERVICE_DISCONNECTED:
        TRACE(3,"(d%x) %s ::HF_EVENT_SERVICE_DISCONNECTED reason=%x", device_id, __func__, ctx->disc_reason);

        btif_hf_set_negotiated_codec(chan, BT_HFP_SCO_CODEC_CVSD);

        curr_device->base_device->hf_conn_flag = 0;
        curr_device->base_device->hfchan_call = 0;
        curr_device->base_device->hfchan_callSetup = 0;
        curr_device->base_device->hf_callheld = 0;
        curr_device->base_device->hf_audio_state = BT_HFP_AUDIO_DISCON;

        app_bt_source_audio_event_handler(device_id, APP_BT_SOURCE_AUDIO_EVENT_AG_SERVICE_DISCONNECTED, NULL);
        break;
    case BTIF_HF_EVENT_AUDIO_CONNECTED:
        btif_hf_report_speaker_volume(chan, hfp_volume_get((enum BT_DEVICE_ID_T)device_id));

        codec_id = btif_hf_get_negotiated_codec(curr_device->base_device->hf_channel);

        TRACE(3,"(d%x) %s ::HF_EVENT_AUDIO_CONNECTED codec %d", device_id, __func__, codec_id);

        bt_adapter_set_hfp_sco_codec_type(device_id, codec_id);

        curr_device->base_device->hf_audio_state = BT_HFP_AUDIO_CON;

        app_bt_source_audio_event_handler(device_id, APP_BT_SOURCE_AUDIO_EVENT_AG_SCO_CONNECTED, NULL);
        break;
    case BTIF_HF_EVENT_AUDIO_DISCONNECTED:
        TRACE(2,"(d%x) %s ::HF_EVENT_AUDIO_DISCONNECTED", device_id, __func__);
        curr_device->base_device->hf_audio_state = BT_HFP_AUDIO_DISCON;

        app_bt_source_audio_event_handler(device_id, APP_BT_SOURCE_AUDIO_EVENT_AG_SCO_DISCONNECTED, NULL);
        break;
    case BTIF_HF_EVENT_AUDIO_DATA_SENT:
        break;
    case BTIF_HF_EVENT_AUDIO_DATA:
        break;
    case BTIF_HF_EVENT_SPEAKER_VOLUME:
        hfp_speak_volume_handler(device_id, chan, ctx);
        break;
    default:
        TRACE(3,"(d%x) %s unhandled event %d", device_id, __func__, ctx->event);
        break;
    }

    app_hfp_bt_driver_callback(device_id, ctx->event);
}

void app_hfp_ag_create_audio_link(btif_hf_channel_t* hf_channel)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)hf_channel,
            (uint32_t)NULL, (uint32_t)btif_ag_create_audio_link);
}

void app_hfp_ag_disc_audio_link(btif_hf_channel_t* hf_channel)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)hf_channel,
            (uint32_t)NULL, (uint32_t)btif_ag_disc_audio_link);
}

void app_hfp_ag_toggle_audio_link(void)
{
    uint8_t device_id = BT_SOURCE_DEVICE_INVALID_ID;
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;

    if (app_bt_source_has_streaming_sco())
    {
        device_id = app_bt_source_get_streaming_sco();
    }
    else
    {
        device_id = app_bt_source_get_current_hfp();
    }

    curr_device = app_bt_source_get_device(device_id);

    TRACE(3,"ag_toggle_audio_link d%x hf_conn_flag %d hf_audio_state %d", device_id,
            curr_device->base_device->hf_conn_flag, curr_device->base_device->hf_audio_state);

    if (curr_device->base_device->hf_conn_flag)
    {
        if (curr_device->base_device->hf_audio_state == BT_HFP_AUDIO_CON)
        {
            TRACE(0, "hfp ag disconnect audio link");
            app_hfp_ag_disc_audio_link(curr_device->base_device->hf_channel);
        }
        else
        {
            TRACE(0, "hfp ag create audio link");
            app_hfp_ag_create_audio_link(curr_device->base_device->hf_channel);
        }
    }
}

int app_hfp_ag_battery_report(uint8_t level)
{
    int nRet = 0;
    bt_status_t status = BT_STS_FAILED;
    btif_hf_channel_t* chan;

    for(int i = BT_SOURCE_DEVICE_ID_BASE; i < BT_SOURCE_DEVICE_ID_BASE + BT_SOURCE_DEVICE_NUM; i++)
    {
        chan = app_bt_get_device(i)->hf_channel;
        if (btif_get_hf_chan_state(chan) == BT_HFP_CHAN_STATE_OPEN)
        {
            if (app_bt_get_device(i)->battery_level != level)
            {
                status = btif_hf_batt_report(chan, level);
            }
            if (BT_STS_PENDING == status){
                app_bt_get_device(i)->battery_level = level;
            }
            else
            {
                nRet = -1;
            }
        }
        else
        {
             app_bt_get_device(i)->battery_level = 0xff;
             nRet = -1;
        }
    }
    return nRet;
}


static int app_hfp_ag_answer_call(btif_hf_channel_t *chan)
{
    TRACE(1, "%s", __func__);
    return 0;
}

static int app_hfp_ag_hungup_call(btif_hf_channel_t *chan)
{
    TRACE(1, "%s", __func__);
    return 0;
}

static int app_hfp_ag_dialing_last_number(btif_hf_channel_t *chan)
{
    TRACE(1, "%s", __func__);
    return 0;
}

static int app_hfp_ag_release_held_calls(btif_hf_channel_t *chan)
{
    TRACE(1, "%s", __func__);
    return 0;
}

static int app_hfp_ag_release_active_and_accept_calls(btif_hf_channel_t *chan)
{
    TRACE(1, "%s", __func__);
    return 0;
}

static int app_hfp_ag_hold_active_and_accept_calls(btif_hf_channel_t *chan)
{
    TRACE(1, "%s", __func__);
    return 0;
}

static int app_hfp_ag_add_held_call_to_conversation(btif_hf_channel_t *chan)
{
    TRACE(1, "%s", __func__);
    return 0;
}

static int app_hfp_ag_connect_remote_two_calls(btif_hf_channel_t *chan)
{
    TRACE(1, "%s", __func__);
    return 0;
}

static int app_hfp_ag_disable_mobile_nrec(btif_hf_channel_t *chan)
{
    TRACE(1, "%s", __func__);
    return 0;
}

static int app_hfp_ag_release_specified_active_call(btif_hf_channel_t *chan, int n)
{
    TRACE(1, "%s", __func__);
    return 0;
}

static int app_hfp_ag_hold_all_calls_except_specified_one(btif_hf_channel_t *chan, int n)
{
    TRACE(1, "%s", __func__);
    return 0;
}

static int app_hfp_ag_hf_battery_change(btif_hf_channel_t *chan, int n)
{
    /* battery level 0 ~ 100 */
    TRACE(1, "%s", __func__);
    return 0;
}

static int app_hfp_ag_hf_spk_gain_change(btif_hf_channel_t *chan, int n)
{
    /* speaker gain 0 ~ 15 */
    TRACE(1, "%s", __func__);
    return 0;
}

static int app_hfp_ag_hf_mic_gain_change(btif_hf_channel_t *chan, int n)
{
    /* mic gain 0 ~ 15 */
    TRACE(1, "%s", __func__);
    return 0;
}

static int app_hfp_ag_transmit_dtmf_code(btif_hf_channel_t *chan, int n)
{
    TRACE(1, "%s", __func__);
    return 0;
}

static int app_hfp_ag_memory_dialing_call(btif_hf_channel_t *chan, int n)
{
    TRACE(1, "%s", __func__);
    return 0;
}

static int app_hfp_ag_dialing_call(btif_hf_channel_t *chan, const char* s)
{
    TRACE(1, "%s", __func__);
    return 0;
}

static int app_hfp_ag_handle_at_command(btif_hf_channel_t *chan, const char* s)
{
    TRACE(1, "%s", __func__);
    return 0;
}

static const char *app_hfp_ag_query_current_operator(btif_hf_channel_t *chan)
{
    TRACE(1, "%s", __func__);
    return 0;
}

static int app_hfp_ag_iterate_current_call(btif_hf_channel_t *chan, struct btif_ag_call_info* out)
{
    TRACE(1, "%s", __func__);
    return 0;
}

static struct btif_ag_module_handler app_hfp_ag_module_handler = {
    app_hfp_ag_answer_call,
    app_hfp_ag_hungup_call,
    app_hfp_ag_dialing_last_number,
    app_hfp_ag_release_held_calls,
    app_hfp_ag_release_active_and_accept_calls,
    app_hfp_ag_hold_active_and_accept_calls,
    app_hfp_ag_add_held_call_to_conversation,
    app_hfp_ag_connect_remote_two_calls,
    app_hfp_ag_disable_mobile_nrec,
    app_hfp_ag_release_specified_active_call,
    app_hfp_ag_hold_all_calls_except_specified_one,
    app_hfp_ag_hf_battery_change,
    app_hfp_ag_hf_spk_gain_change,
    app_hfp_ag_hf_mic_gain_change,
    app_hfp_ag_transmit_dtmf_code,
    app_hfp_ag_memory_dialing_call,
    app_hfp_ag_dialing_call,
    app_hfp_ag_handle_at_command,
    app_hfp_ag_query_current_operator,
    app_hfp_ag_iterate_current_call,
};

void app_hfp_ag_init(void)
{
    struct BT_SOURCE_DEVICE_T* curr_device = NULL;
    int i = 0;

    if (bt_source_manager.config.ag_enable)
    {
        btif_hfp_initialize();

        for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1)
        {
            curr_device = app_bt_source_get_device(i);
            curr_device->base_device->hf_channel = btif_ag_alloc_channel();
            ASSERT(curr_device->base_device->hf_channel, "app_hfp_ag_init cannot alloc hf channel");
            btif_hf_init_channel(curr_device->base_device->hf_channel);
            btif_ag_register_module_handler(curr_device->base_device->hf_channel, &app_hfp_ag_module_handler);
        }

        btif_ag_register_callback(app_hfp_ag_event_callback);
    }
}

#endif /* BT_HFP_AG_ROLE */
#endif /* BT_HFP_SUPPORT */

