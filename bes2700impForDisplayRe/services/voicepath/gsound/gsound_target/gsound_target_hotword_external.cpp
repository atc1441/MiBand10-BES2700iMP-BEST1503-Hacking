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
#include "gsound_dbg.h"
#include "gsound_target_hotword_external.h"
#include "gsound_custom_hotword_external.h"
#include "gsound_custom_audio.h"

/*********************external function declearation************************/

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declearation************************/

/************************private variable defination************************/

/****************************function defination****************************/
GSoundStatus GSoundTargetHotwordStartDetection(const GSoundTargetAudioInSettings *audio_config,
                                               const GSoundTargetHotwordConfig *hw_config)
{
    GLOG_D("%s", __func__);
    ASSERT(GSOUND_TARGET_MAX_HOTWORD_BUF_LEN_MS >= hw_config->audio_backlog_ms,
           "audio_backlog_ms exceed:%d/%d",
           hw_config->audio_backlog_ms,
           GSOUND_TARGET_MAX_HOTWORD_BUF_LEN_MS);

    gsound_custom_hotword_external_start_detection(audio_config, hw_config);

    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetHotwordRequestForData(uint32_t max_num_bytes,
                                               uint32_t bytes_per_frame,
                                               GSoundTargetHotwordDataMsg *sync_hw_data_out,
                                               bool *ready_now_out)
{
    GLOG_D("%s", __func__);
    gsound_custom_hotword_external_request_data(max_num_bytes,
                                                bytes_per_frame,
                                                sync_hw_data_out,
                                                ready_now_out);

    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetHotwordDataConsumed(GSoundTargetHotwordDataMsg *hw_data)
{
    GLOG_D("%s", __func__);

    gsound_custom_hotword_data_consumed(hw_data);

    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetHotwordStopStream(void)
{
    GLOG_D("%s", __func__);
    GSoundStatus ret = GSOUND_STATUS_OK;

    // start local process
    if (gsound_custom_hotword_external_stop_stream())
    {
        ret = GSOUND_STATUS_ERROR;
    }

    return ret;
}

GSoundStatus GSoundTargetHotwordStopDetection(bool keep_voice_ready)
{
    GLOG_D("%s", __func__);

    gsound_custom_hotword_external_stop_detection(keep_voice_ready);

    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetHotwordReset(const GSoundTargetAudioInSettings *audio_config,
                                      const GSoundTargetHotwordConfig *hw_config)
{
    GLOG_D("%s", __func__);

    gsound_custom_hotword_external_start_detection(audio_config, hw_config);

    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetGetHotwordLibInfo(const GSoundHotwordLibInfo *const info_out)
{
    GLOG_D("%s", __func__);
    ASSERT(info_out, "null pointer received in %s", __func__);

    GSoundStatus ret = GSOUND_STATUS_OK;
    if(gsound_custom_hotword_get_lib_info(info_out))
    {
        ret = GSOUND_STATUS_ERROR;
    }

    GLOG_D("supported_models is %s, strLen is %d", info_out->supported_models, info_out->supported_model_length);
    return ret;
}

GSoundStatus GSoundTargetHotwordModelActivate(const char *model_id,
                                              uint8_t model_length,
                                              bool *ready_now_out)
{
    GLOG_D("%s", __func__);

    gsound_custom_hotword_external_store_active_model(model_id, model_length);
    gsound_custom_hotword_external_init_process(model_id, model_length, false);

    *ready_now_out = true;
    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetHotwordInit(const GSoundHotwordInterface *handlers)
{
    GLOG_D("%s", __func__);
    gsound_custom_hotword_external_store_interface(handlers);
    return GSOUND_STATUS_OK;
}