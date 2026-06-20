/***************************************************************************
*
*Copyright 2015-2020 BES.
*All rights reserved. All unpublished rights reserved.
*
*No part of this work may be used or reproduced in any form or by any
*means, or stored in a database or retrieval system, without prior written
*permission of BES.
*
*Use of this work is governed by a license granted by BES.
*This work contains confidential and proprietary information of
*BES. which is protected by copyright, trade secret,
*trademark and other intellectual property rights.
*
****************************************************************************/


/*****************************header include********************************/
#include "bluetooth_bt_api.h"
#include "gsound_dbg.h"
#include "gsound_target_audio.h"
#include "gsound_custom.h"
#include "gsound_custom_bt.h"
#include "gsound_custom_audio.h"
#include "apps.h"

#ifdef IBRT
#include "app_ibrt_internal.h"
#include "earbud_ux_api.h"
#include "app_tws_ibrt.h"
#endif
#include "app_media_player.h"

/*********************external function declearation************************/

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declearation************************/

/************************private variable defination************************/

/****************************function defination****************************/
GSoundStatus GSoundTargetAudioOutStart(GSoundTargetAudioOutClip clip)
{
    GLOG_I("%s clip %d", __FUNCTION__, clip);

#ifdef IBRT
    if (TWS_UI_SLAVE == app_ibrt_if_get_ui_role())
    {
        GLOG_I("slave not play the clip.");
        return GSOUND_STATUS_OK;
    }
#endif

    switch (clip)
    {
    case GSOUND_AUDIO_OUT_CLIP_REMOTE_MIC_OPEN:
    case GSOUND_AUDIO_OUT_CLIP_PREPARE_MIC_OPEN:
        media_PlayAudio(AUDIO_ID_BT_GSOUND_MIC_OPEN, 0);
        break;

    case GSOUND_AUDIO_OUT_CLIP_REMOTE_MIC_CLOSE:
    case GSOUND_AUDIO_OUT_CLIP_PTT_MIC_CLOSE:
        media_PlayAudio(AUDIO_ID_BT_GSOUND_MIC_CLOSE, 0);
        break;

    case GSOUND_AUDIO_OUT_CLIP_GSOUND_NC:
        media_PlayAudio(AUDIO_ID_BT_GSOUND_NC, 0);
        break;

    default:
        GLOG_I("Unsupported clip %d", clip);
        break;
    };
    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetAudioOutInit(void)
{
    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetAudioPrepareForStreaming(const GSoundTargetAudioStreamingSettings *settings)
{
    bt_bdaddr_t bes_addr;
    gsound_convert_bdaddr_to_plateform(&(settings->connected_address), &bes_addr);

    if (memcmp(gsound_get_connected_bd_addr(), (uint8_t *)(&bes_addr), 6))
    {
        GLOG_I("connected addr:");
        DUMP8("%02x ", gsound_get_connected_bd_addr(), BT_ADDR_OUTPUT_PRINT_NUM);
        GLOG_I("received addr:");
        DUMP8("%02x ", (uint8_t *)(&bes_addr), BT_ADDR_OUTPUT_PRINT_NUM);

        /// check the gsound device is connected or not
        // ASSERT(0, "BD addr received from glib is not connected!");
    }

    GLOG_I("Voice detected! reason is %d", settings->reason);
    gsound_custom_audio_on_upstream_started();

    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetAudioUndoPrepareForStreaming(void)
{
    GLOG_I("%s ", __FUNCTION__);

    gsound_custom_audio_on_upstream_stopped();

    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetAudioInOpen(const GSoundTargetAudioInSettings *audio_in_settings)
{
    GLOG_I("[%s] raw_samples_required:%d, sbc_bitpool:%d, header_include:%d",
           __func__,
           audio_in_settings->raw_samples_required,
           audio_in_settings->sbc_bitpool,
           audio_in_settings->include_sbc_headers);

    gsound_custom_audio_store_audio_in_settings(audio_in_settings);

    gsound_custom_audio_set_libgsound_request_data_flag(true);
    gsound_custom_audio_open_mic(audio_in_settings->sbc_bitpool);
    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetAudioInClose(void)
{
    GLOG_I("%s ", __FUNCTION__);
#ifdef GSOUND_HOTWORD_ENABLED
#else
    gsound_custom_audio_close_mic();
#endif
    gsound_custom_audio_set_libgsound_request_data_flag(false);
    gsound_custom_bt_allow_sniff();

    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetAudioInInit(const GSoundAudioInterface *handler)
{
    gsound_custom_audio_store_lib_interface(handler);

#ifdef AI_ALGORITHM_ENABLE
    app_ai_algorithm_init();
#endif

    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetSetSidetone(bool sidetone_enabled)
{
    return GSOUND_STATUS_OK;
}
