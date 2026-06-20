/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#if BLE_AUDIO_ENABLED
#include "bluetooth_bt_api.h"
#include "app_bt_func.h"
#include "app_bt_sync.h"
#include "app_utils.h"
#include "audio_dump.h"
#include "audioflinger.h"
#include "cqueue.h"
#include "hal_dma.h"
#include "hal_aud.h"
#include "hal_trace.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_interface.h"

#include "app_tws_ibrt.h"

#include "gaf_stream_dbg.h"

void gaf_media_prepare_playback_trigger(uint8_t trigger_channel)
{
    LOG_D("%s start", __func__);
    uint8_t adma_ch = HAL_DMA_CHAN_NONE;
    uint32_t dma_base;
    af_stream_dma_tc_irq_enable(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    if(adma_ch != HAL_DMA_CHAN_NONE)
    {
        bt_drv_reg_op_enable_dma_tc(adma_ch&0xFF, dma_base);
    }
    else
    {
        LOG_I("error adma_ch HAL_DMA_CHAN_NONE %d", adma_ch);
    }

    btdrv_syn_clr_trigger(trigger_channel);
    af_codec_sync_config(AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, false);
    af_codec_sync_config(AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, true);
    LOG_D("%s end", __func__);
}

void gaf_media_prepare_capture_trigger(uint8_t trigger_channel)
{
    LOG_D("%s start", __func__);
    uint8_t adma_ch = 0;
    uint32_t dma_base;
    btdrv_syn_clr_trigger(trigger_channel);

    af_stream_dma_tc_irq_enable(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    if(adma_ch != HAL_DMA_CHAN_NONE)
    {
        bt_drv_reg_op_enable_dma_tc(adma_ch&0xFF, dma_base);
    }

    af_codec_sync_config(AUD_STREAM_CAPTURE, AF_CODEC_SYNC_TYPE_BT, false);
    af_codec_sync_config(AUD_STREAM_CAPTURE, AF_CODEC_SYNC_TYPE_BT, true);
    LOG_D("%s end", __func__);
}

uint32_t gaf_media_sync_get_curr_time(void)   //uinit -- us
{
    return btdrv_syn_get_cis_curr_time();
}

uint32_t gaf_media_sync_get_curr_bt_clk_in_us(void)   //uinit -- us
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    uint32_t curr_ticks = 0;
    curr_ticks = bt_syn_get_curr_ticks(p_ibrt_ctrl->tws_conhandle);
    return BTCLKS_TO_US(curr_ticks);
}

#endif

/// @} APP
