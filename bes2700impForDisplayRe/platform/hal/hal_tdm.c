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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hal_i2s.h"
#include "hal_trace.h"
#include "hal_dma.h"
#include "hal_tdm.h"
#ifndef CHIP_HAS_TDM
#include "hal_i2s_tdm.h"
#endif

#if 0
#define TDM_TRACE   TRACE
#define TDM_DUMP8   DUMP8
#else
#define TDM_TRACE(n, str, ...)
#define TDM_DUMP8(str, ...)
#endif

#ifdef CHIP_HAS_TDM
static bool tdm_stream_status[HAL_I2S_ID_QTY][AUD_STREAM_NUM];
static bool tdm_chan_sep_buf[HAL_I2S_ID_QTY];
static bool tdm_sync_start[HAL_I2S_ID_QTY];
static uint32_t tdm_ext_mclk_freq[HAL_I2S_ID_QTY];

static bool tdm_is_enabled(enum HAL_I2S_ID_T id)
{
    uint32_t i;

    for (i = 0 ; i < AUD_STREAM_NUM; i++) {
        if (tdm_stream_status[id][i]) {
            return true;
        }
    }
    return false;
}

static inline uint32_t _tdm_get_reg_base(enum HAL_I2S_ID_T id)
{
    uint32_t addr = 0;

    ASSERT(id < HAL_I2S_ID_QTY, "%s: Invalid I2S ID: %d", __func__, id);

    switch (id) {
    case HAL_I2S_ID_0:
        addr = I2S0_BASE | I2SIP_TDM_CTRL_REG_OFFSET;
        break;
#if defined(CHIP_HAS_I2S) && (CHIP_HAS_I2S > 1)
    case HAL_I2S_ID_1:
        addr = I2S1_BASE | I2SIP_TDM_CTRL_REG_OFFSET;
        break;
#endif
    default:
        break;
    }
    return addr;
}

static inline bool tdm_slot_cycles_in_arrays(uint8_t slot_cycles)
{
    if (slot_cycles == (uint8_t)HAL_TDM_SLOT_CYCLES_16
        || slot_cycles == (uint8_t)HAL_TDM_SLOT_CYCLES_32) {

        return true;
    }
    return false;
}

static inline bool tdm_cycles_in_arrays(uint16_t cycles)
{
    if (cycles == (uint16_t)HAL_TDM_CYCLES_16
        || cycles == (uint16_t)HAL_TDM_CYCLES_32
        || cycles == (uint16_t)HAL_TDM_CYCLES_64
        || cycles == (uint16_t)HAL_TDM_CYCLES_128
        || cycles == (uint16_t)HAL_TDM_CYCLES_256) {

        return true;
    }
    return false;
}

static inline bool tdm_fs_cycles_in_arrays(uint16_t fs_cycles)
{
    if (fs_cycles == (uint16_t)HAL_TDM_FS_CYCLES_1
        || fs_cycles == (uint16_t)HAL_TDM_FS_CYCLES_8
        || fs_cycles == (uint16_t)HAL_TDM_FS_CYCLES_16
        || fs_cycles == (uint16_t)HAL_TDM_FS_CYCLES_32
        || fs_cycles == (uint16_t)HAL_TDM_FS_CYCLES_64
        || fs_cycles == (uint16_t)HAL_TDM_FS_CYCLES_128
        || fs_cycles == (uint16_t)HAL_TDM_FS_CYCLES_ONE_LESS) {

        return true;
    }
    return false;
}

POSSIBLY_UNUSED static void tdm_dump_config(const char *title,
    struct HAL_TDM_CONFIG_T *cfg)
{
    TRACE(1, "%s: TDM DUMP CONFIG", title);
    TRACE(1, "mode:%d",       cfg->mode);
    TRACE(1, "edge:%d",       cfg->edge);
    TRACE(1, "cycles:%d",     cfg->cycles);
    TRACE(1, "fs_cycles:%d",  cfg->fs_cycles);
    TRACE(1, "slot_cycles:%d",cfg->slot_cycles);
    TRACE(1, "bits:%d",       cfg->bits);
    TRACE(1, "chan:%d",       cfg->channel_num);
    TRACE(1, "DUMP END");
}

static void tdm_get_config(enum HAL_I2S_ID_T i2s_id,struct HAL_TDM_CONFIG_T *tdm_config)
{
    volatile uint32_t *base_addr;
    uint32_t val, fs_width, frm_width;

    base_addr = (uint32_t*)_tdm_get_reg_base(i2s_id);
    val = *base_addr;

    TDM_TRACE(1, "%s: val=%x", __func__, val);

    if((val & (TDM_MODE_FS_ASSERTED_AT_LAST << TDM_MODE_FS_ASSERTED_SHIFT)) == (TDM_MODE_FS_ASSERTED_AT_LAST << TDM_MODE_FS_ASSERTED_SHIFT)) {
        tdm_config->mode = HAL_TDM_MODE_FS_ASSERTED_AT_LAST;
    } else {
        tdm_config->mode = HAL_TDM_MODE_FS_ASSERTED_AT_FIRST;
    }

    if ((val & (TDM_FS_EDGE_NEGEDGE << TDM_FS_EDGE_SHIFT)) == (TDM_FS_EDGE_NEGEDGE << TDM_FS_EDGE_SHIFT)) {
        tdm_config->edge = HAL_TDM_FS_EDGE_NEGEDGE;
    } else {
        tdm_config->edge = HAL_TDM_FS_EDGE_POSEDGE;
    }

    frm_width = (val >> TDM_FRAME_WIDTH_SHIFT) & 7;
    switch (frm_width) {
    case TDM_FRAME_WIDTH_16_CYCLES:
        tdm_config->cycles = HAL_TDM_CYCLES_16;
        break;
    case TDM_FRAME_WIDTH_32_CYCLES:
        tdm_config->cycles = HAL_TDM_CYCLES_32;
        break;
    case TDM_FRAME_WIDTH_64_CYCLES:
        tdm_config->cycles = HAL_TDM_CYCLES_64;
        break;
    case TDM_FRAME_WIDTH_128_CYCLES:
        tdm_config->cycles = HAL_TDM_CYCLES_128;
        break;
    case TDM_FRAME_WIDTH_256_CYCLES:
    default:
        tdm_config->cycles = HAL_TDM_CYCLES_256;
        break;
    }

    fs_width = (val >> TDM_FS_WIDTH_SHIFT) & 0x7;
    switch(fs_width) {
    case TDM_FS_WIDTH_1_CYCLE:
        tdm_config->fs_cycles = HAL_TDM_FS_CYCLES_1;
        break;
    case TDM_FS_WIDTH_8_CYCLES:
        tdm_config->fs_cycles = HAL_TDM_FS_CYCLES_8;
        break;
    case TDM_FS_WIDTH_16_CYCLES:
        tdm_config->fs_cycles = HAL_TDM_FS_CYCLES_16;
        break;
    case TDM_FS_WIDTH_32_CYCLES:
        tdm_config->fs_cycles = HAL_TDM_FS_CYCLES_32;
        break;
    case TDM_FS_WIDTH_64_CYCLES:
        tdm_config->fs_cycles = HAL_TDM_FS_CYCLES_64;
        break;
    case TDM_FS_WIDTH_128_CYCLES:
        tdm_config->fs_cycles = HAL_TDM_FS_CYCLES_128;
        break;
    case TDM_FS_WIDTH_FRAME_LENGTH_1_CYCLES:
    default:
        tdm_config->fs_cycles = HAL_TDM_FS_CYCLES_ONE_LESS;
        break;
    }

    if((val & (TDM_SLOT_WIDTH_16_BIT << TDM_SLOT_WIDTH_SHIFT)) == (TDM_SLOT_WIDTH_16_BIT << TDM_SLOT_WIDTH_SHIFT)) {
        tdm_config->slot_cycles = HAL_TDM_SLOT_CYCLES_16;
    } else {
        tdm_config->slot_cycles = HAL_TDM_SLOT_CYCLES_32;
    }
    tdm_config->bits = tdm_config->slot_cycles;
    tdm_config->channel_num = tdm_config->cycles / tdm_config->slot_cycles;
    tdm_config->chan_sep_buf = tdm_chan_sep_buf[i2s_id];
    tdm_config->sync_start = tdm_sync_start[i2s_id];
    tdm_config->data_offset = ((val >> TDM_DATA_OFFSET_SHIT) & TDM_DATA_OFFSET_MAX);
    tdm_config->ext_mclk_freq = tdm_ext_mclk_freq[i2s_id];

    //tdm_dump_config("TDM_GET_CONFIG", tdm_config);
}

static void tdm_set_config(enum HAL_I2S_ID_T i2s_id,struct HAL_TDM_CONFIG_T *tdm_config)
{
    volatile uint32_t *base_addr;
    uint32_t val = 0;
    uint32_t tdm_en = 0;

    base_addr = (uint32_t*)_tdm_get_reg_base(i2s_id);
    tdm_en = *base_addr & (TDM_ENABLE<<TDM_ENABLE_SHIFT);

    ASSERT(tdm_config->mode < HAL_TDM_MODE_NUM,
           "%s: mode = %d error!",__func__, tdm_config->mode);
    ASSERT(tdm_config->edge < HAL_TDM_FS_EDGE_NUM,
           "%s: edge = %d error!",__func__, tdm_config->edge);
    ASSERT(tdm_cycles_in_arrays((uint16_t)tdm_config->cycles),
           "%s: cycles = %d error!",__func__, tdm_config->cycles);
    ASSERT(tdm_fs_cycles_in_arrays((uint16_t)tdm_config->fs_cycles),
           "%s: fs_cycles = %d cycles = %d error!",__func__, tdm_config->fs_cycles, tdm_config->cycles);
    ASSERT(tdm_slot_cycles_in_arrays((uint8_t)tdm_config->slot_cycles),
           "%s: slot_cycles = %d error!",__func__, tdm_config->slot_cycles);

    //tdm_dump_config("TDM_SET_CONFIG", tdm_config);

    tdm_chan_sep_buf[i2s_id]    = tdm_config->chan_sep_buf;
    tdm_sync_start[i2s_id]      = tdm_config->sync_start;
    tdm_ext_mclk_freq[i2s_id]   = tdm_config->ext_mclk_freq;

    if (tdm_config->mode == HAL_TDM_MODE_FS_ASSERTED_AT_LAST) {
        val |= (TDM_MODE_FS_ASSERTED_AT_LAST << TDM_MODE_FS_ASSERTED_SHIFT);
    } else {
        val &= ~(TDM_MODE_FS_ASSERTED_AT_LAST << TDM_MODE_FS_ASSERTED_SHIFT);
    }

    if (tdm_config->edge == HAL_TDM_FS_EDGE_NEGEDGE) {
        val |= (TDM_FS_EDGE_NEGEDGE << TDM_FS_EDGE_SHIFT);
    } else {
        val &= ~(TDM_FS_EDGE_NEGEDGE << TDM_FS_EDGE_SHIFT);
    }

    if (tdm_config->cycles == HAL_TDM_CYCLES_16) {
        val |= (TDM_FRAME_WIDTH_16_CYCLES << TDM_FRAME_WIDTH_SHIFT);
    } else if(tdm_config->cycles == HAL_TDM_CYCLES_32) {
        val |= (TDM_FRAME_WIDTH_32_CYCLES << TDM_FRAME_WIDTH_SHIFT);
    } else if(tdm_config->cycles == HAL_TDM_CYCLES_64) {
        val |= (TDM_FRAME_WIDTH_64_CYCLES << TDM_FRAME_WIDTH_SHIFT);
    } else if(tdm_config->cycles == HAL_TDM_CYCLES_128) {
        val |= (TDM_FRAME_WIDTH_128_CYCLES << TDM_FRAME_WIDTH_SHIFT);
    } else {
        val |= (TDM_FRAME_WIDTH_256_CYCLES << TDM_FRAME_WIDTH_SHIFT);
    }

    if (tdm_config->fs_cycles == HAL_TDM_FS_CYCLES_1) {
        val |= (TDM_FS_WIDTH_1_CYCLE << TDM_FS_WIDTH_SHIFT);
    } else if(tdm_config->fs_cycles == HAL_TDM_FS_CYCLES_8) {
        val |= (TDM_FS_WIDTH_8_CYCLES << TDM_FS_WIDTH_SHIFT);
    } else if(tdm_config->fs_cycles == HAL_TDM_FS_CYCLES_16) {
        val |= (TDM_FS_WIDTH_16_CYCLES << TDM_FS_WIDTH_SHIFT);
    } else if(tdm_config->fs_cycles == HAL_TDM_FS_CYCLES_32) {
        val |= (TDM_FS_WIDTH_32_CYCLES << TDM_FS_WIDTH_SHIFT);
    } else if(tdm_config->fs_cycles == HAL_TDM_FS_CYCLES_64) {
        val |= (TDM_FS_WIDTH_64_CYCLES << TDM_FS_WIDTH_SHIFT);
    } else if(tdm_config->fs_cycles == HAL_TDM_FS_CYCLES_128) {
        val |= (TDM_FS_WIDTH_128_CYCLES << TDM_FS_WIDTH_SHIFT);
    } else {
        val |= (TDM_FS_WIDTH_FRAME_LENGTH_1_CYCLES << TDM_FS_WIDTH_SHIFT);
    }

    if (tdm_config->slot_cycles == HAL_TDM_SLOT_CYCLES_16) {
        val |= (TDM_SLOT_WIDTH_16_BIT << TDM_SLOT_WIDTH_SHIFT);
    } else {
        val &= ~(TDM_SLOT_WIDTH_16_BIT << TDM_SLOT_WIDTH_SHIFT);
    }

    if (tdm_config->data_offset >= TDM_DATA_OFFSET_MIN
        && tdm_config->data_offset <= TDM_DATA_OFFSET_MAX) {
        val |= (tdm_config->data_offset << TDM_DATA_OFFSET_SHIT);
    }
    val |= tdm_en;
    TDM_TRACE(1, "%s: val=%x", __func__, val);
    *base_addr = val;
}

static int tdm_use_same_config(struct HAL_TDM_CONFIG_T *cur, struct HAL_TDM_CONFIG_T *new)
{
    uint32_t err = 0;

    if (cur->mode != new->mode) {
        err |= 0x01;
    }
    if (cur->edge != new->edge) {
        err |= 0x02;
    }
    if (cur->cycles != new->cycles) {
        err |= 0x04;
    }
    if (cur->fs_cycles != new->fs_cycles) {
        err |= 0x08;
    }
    if (cur->slot_cycles != new->slot_cycles) {
        err |= 0x10;
    }
    if (cur->bits != new->bits) {
        err |= 0x20;
    }
    if (cur->channel_num != new->channel_num) {
        err |= 0x40;
    }
    return err;
}

static void tdm_get_i2s_config(struct HAL_TDM_CONFIG_T *tdm_config, uint32_t slot_cycle)
{
    tdm_config->mode        = HAL_TDM_MODE_FS_ASSERTED_AT_LAST;
    tdm_config->edge        = HAL_TDM_FS_EDGE_NEGEDGE;
    if(slot_cycle == 16) {
        tdm_config->cycles      = HAL_TDM_CYCLES_32;
        tdm_config->fs_cycles   = HAL_TDM_FS_CYCLES_16;
        tdm_config->slot_cycles = HAL_TDM_SLOT_CYCLES_16;
        tdm_config->bits = AUD_BITS_16;
    }
    else if(slot_cycle == 32) {
        tdm_config->cycles      = HAL_TDM_CYCLES_64;
        tdm_config->fs_cycles   = HAL_TDM_FS_CYCLES_32;
        tdm_config->slot_cycles = HAL_TDM_SLOT_CYCLES_32;
        tdm_config->bits = AUD_BITS_32;
    }
    tdm_config->channel_num = AUD_CHANNEL_NUM_2;
    tdm_config->data_offset = 0;
    tdm_config->chan_sep_buf = false;
    tdm_config->sync_start = false;
}

static void tdm_enable(enum HAL_I2S_ID_T i2s_id,bool enable)
{
    volatile uint32_t *base_addr;
    uint32_t val = 0;

    base_addr = (uint32_t *)_tdm_get_reg_base(i2s_id);
    val = *base_addr;

    if(enable) {
        val |= (TDM_ENABLE << TDM_ENABLE_SHIFT);
    } else {
        val &= ~(TDM_ENABLE << TDM_ENABLE_SHIFT);
    }
    *base_addr = val;
}

static int32_t tdm_open(enum HAL_I2S_ID_T i2s_id,enum AUD_STREAM_T stream,enum HAL_I2S_MODE_T mode)
{
    int ret;

    TDM_TRACE(4,"%s:i2s_id=%d,stream=%d,mode=%d", __func__, i2s_id, stream, mode);
    ret = hal_i2s_open(i2s_id, stream, mode);
    ASSERT(ret==0,"%s: i2s open failed,ret=%d", __func__, ret);
    return ret;
}

static int32_t tdm_setup_stream(enum HAL_I2S_ID_T i2s_id,
                                       enum AUD_STREAM_T stream,
                                       uint32_t sample_rate,
                                       struct HAL_TDM_CONFIG_T *tdm_cfg)
{
    struct HAL_I2S_CONFIG_T i2s_cfg;
    int ret;

    TDM_TRACE(4,"%s: i2s_id = %d, stream = %d, sample_rate = %d",
          __func__, i2s_id, stream, sample_rate);

    TDM_TRACE(5,"%s: tdm_cfg: mode = %d,edge = %d,cycles = %d,fs_cycles = %d,"
              "slot_cycles= %d, offs = %d.",
              __func__, tdm_cfg->mode, tdm_cfg->edge, tdm_cfg->cycles,
              tdm_cfg->fs_cycles,tdm_cfg->slot_cycles,tdm_cfg->data_offset);

    ASSERT(tdm_cycles_in_arrays(tdm_cfg->cycles),
            "%s: cycles = %d error!",__func__, tdm_cfg->cycles);

    memset(&i2s_cfg, 0, sizeof(i2s_cfg));
    i2s_cfg.use_dma = true;
    i2s_cfg.chan_sep_buf = tdm_cfg->chan_sep_buf;
    i2s_cfg.sync_start = tdm_cfg->sync_start;
    i2s_cfg.bits = tdm_cfg->bits;
    i2s_cfg.channel_num = 2;
    i2s_cfg.channel_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
    i2s_cfg.sample_rate = sample_rate * (tdm_cfg->channel_num / i2s_cfg.channel_num);
    i2s_cfg.ext_mclk_freq = tdm_cfg->ext_mclk_freq;

    ret = hal_i2s_setup_stream(i2s_id, stream, &i2s_cfg);
    ASSERT(ret==0,"%s: i2s setup stream failed,ret=%d.", __func__, ret);

    if (tdm_is_enabled(i2s_id)) {
        struct HAL_TDM_CONFIG_T cur_cfg;

        memset(&cur_cfg, 0, sizeof(cur_cfg));
        tdm_get_config(i2s_id, &cur_cfg);
        ret = tdm_use_same_config(&cur_cfg, tdm_cfg);
        ASSERT(ret==0, "%s: tdm config conflict, ret=0x%x", __func__, ret);
    }
    tdm_set_config(i2s_id, tdm_cfg);

    TDM_TRACE(1,"%s done", __func__);
    return ret;
}

static int32_t tdm_as_i2s_setup_stream(enum HAL_I2S_ID_T i2s_id,
                                       enum AUD_STREAM_T stream,
                                       uint32_t sample_rate,
                                       uint32_t bits)
{
    struct HAL_I2S_CONFIG_T i2s_cfg;
    struct HAL_TDM_CONFIG_T tdm_cfg;
    int ret;

    TDM_TRACE(4,"%s: i2s_id = %d, stream = %d, sample_rate = 0x%x.",
                  __func__, i2s_id, stream, sample_rate);

    memset(&i2s_cfg, 0, sizeof(i2s_cfg));
    i2s_cfg.use_dma = true;
    i2s_cfg.sync_start = stream == AUD_STREAM_PLAYBACK ? true : false;
    i2s_cfg.chan_sep_buf = false;
    i2s_cfg.bits = bits;
    i2s_cfg.channel_num = 2;
    i2s_cfg.channel_map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
    i2s_cfg.sample_rate = sample_rate;

    ret = hal_i2s_setup_stream(i2s_id, stream, &i2s_cfg);
    ASSERT(ret==0,"%s: i2s setup stream failed,ret = %d.", __func__, ret);

    tdm_get_i2s_config(&tdm_cfg, bits);
    tdm_set_config(i2s_id, &tdm_cfg);

    TDM_TRACE(1,"%s: done.", __func__);
    return ret;
}

static int32_t tdm_start_stream(enum HAL_I2S_ID_T i2s_id, enum AUD_STREAM_T stream)
{
    int ret;

    TDM_TRACE(3,"%s: i2s_id = %d, stream = %d.", __func__, i2s_id, stream);
    if (tdm_stream_status[i2s_id][AUD_STREAM_PLAYBACK] == false &&
        tdm_stream_status[i2s_id][AUD_STREAM_CAPTURE] == false) {

        tdm_enable(i2s_id,true);
    }
    tdm_stream_status[i2s_id][stream] = true;
    ret = hal_i2s_start_stream(i2s_id, stream);
    ASSERT(ret==0, "%s: i2s start stream faild, ret=%d", __func__, ret);

    TDM_TRACE(2,"%s done. ret = %d.", __func__, ret);
    return ret;
}

static int32_t tdm_stop_stream(enum HAL_I2S_ID_T i2s_id, enum AUD_STREAM_T stream)
{
    int ret;

    TDM_TRACE(3,"%s: i2s_id = %d, stream = %d.", __func__, i2s_id, stream);
    tdm_stream_status[i2s_id][stream] = false;
    if (tdm_stream_status[i2s_id][AUD_STREAM_PLAYBACK] == false &&
        tdm_stream_status[i2s_id][AUD_STREAM_CAPTURE] == false) {

        tdm_enable(i2s_id,false);
    }
    ret = hal_i2s_stop_stream(i2s_id,stream);
    ASSERT(ret==0, "%s: i2s stop stream faild, ret=%d", __func__, ret);

    TDM_TRACE(2,"%s done. ret = %d.", __func__, ret);
    return ret;
}

static int32_t tdm_close(enum HAL_I2S_ID_T i2s_id, enum AUD_STREAM_T stream)
{
    int ret;

    TDM_TRACE(3,"%s: i2s_id = %d, stream = %d.", __func__, i2s_id, stream);

    ret = hal_i2s_close(i2s_id,stream);

    TDM_TRACE(2,"%s done. ret = %d.", __func__, ret);
    return ret;
}
#endif // CHIP_HAS_TDM

int32_t hal_tdm_open(enum HAL_I2S_ID_T i2s_id,enum AUD_STREAM_T stream,enum HAL_I2S_MODE_T mode)
{
    int ret;

    TDM_TRACE(3,"hal_tdm_open:i2s_id = %d, stream = %d, mode = %d.",
              i2s_id, stream, mode);
    ASSERT(i2s_id < HAL_I2S_ID_QTY,"%s: i2s_id = %d!", __func__, i2s_id);

#ifdef CHIP_HAS_TDM
    ret = tdm_open(i2s_id, stream, mode);
#else
    ASSERT(stream == AUD_STREAM_CAPTURE, "stream = AUD_STREAM_PLAYBACK!");
    ret = hal_i2s_tdm_open(i2s_id, mode);
#endif

    TDM_TRACE(2,"%s done. ret = %d.", __func__, ret);
    return ret;
}

int32_t hal_tdm_setup_stream(enum HAL_I2S_ID_T i2s_id,
                                       enum AUD_STREAM_T stream,
                                       uint32_t sample_rate,
                                       struct HAL_TDM_CONFIG_T *tdm_cfg)
{
    int ret;

    TDM_TRACE(4,"%s:i2s_id = %d, stream = %d, sample_rate = %d.",
        __func__, i2s_id, stream, sample_rate);
    ASSERT(i2s_id < HAL_I2S_ID_QTY,"%s: i2s_id = %d!", __func__, i2s_id);
    TDM_TRACE(4,"%s: cycles = %d, fs_cycles = %d,slot_cycles = %d.",
                __func__,
                tdm_cfg->cycles,
                tdm_cfg->fs_cycles,
                tdm_cfg->slot_cycles);

#ifdef CHIP_HAS_TDM
    ret = tdm_setup_stream(i2s_id, stream, sample_rate,tdm_cfg);
#else
    struct HAL_I2S_TDM_CONFIG_T i2s_tdm_cfg;

    i2s_tdm_cfg.cycles = (enum HAL_I2S_TDM_CYCLES_T)tdm_cfg->cycles;
    i2s_tdm_cfg.fs_cycles = (enum HAL_I2S_TDM_FS_CYCLES_T)tdm_cfg->fs_cycles;
    i2s_tdm_cfg.slot_cycles = (enum HAL_I2S_TDM_SLOT_CYCLES_T)tdm_cfg->slot_cycles;
    i2s_tdm_cfg.data_offset = tdm_cfg->data_offset;
    ret = hal_i2s_tdm_setup(i2s_id, sample_rate, &i2s_tdm_cfg);
#endif

    TDM_TRACE(2,"%s done. ret = %d.", __func__, ret);
    return ret;
}

int32_t hal_tdm_as_i2s_setup_stream(enum HAL_I2S_ID_T i2s_id,
                                       enum AUD_STREAM_T stream,
                                       uint32_t sample_rate,
                                       uint32_t bits)
{
    int ret;

    TDM_TRACE(4,"%s: i2s_id = %d, stream = %d, sample_rate = %d, bits = %d.",
        __func__, i2s_id, stream, sample_rate, bits);
    ASSERT(i2s_id < HAL_I2S_ID_QTY,"%s: i2s_id = %d!", __func__, i2s_id);

    // i2s setup stream playback and capture.
#ifdef CHIP_HAS_TDM
    ret = tdm_as_i2s_setup_stream(i2s_id, stream, sample_rate,bits);
#else
    struct HAL_I2S_TDM_CONFIG_T i2s_tdm_cfg={
        bits == 16 ? HAL_I2S_TDM_CYCLES_32 : HAL_I2S_TDM_CYCLES_64,
        bits == 16 ? HAL_I2S_TDM_FS_CYCLES_16 : HAL_I2S_TDM_FS_CYCLES_32,
        bits == 16 ? HAL_I2S_TDM_SLOT_CYCLES_16 : HAL_I2S_TDM_SLOT_CYCLES_32,};
    ret = hal_i2s_tdm_setup(i2s_id, sample_rate, &i2s_tdm_cfg);
#endif

    TDM_TRACE(2,"%s done. ret = %d.", __func__, ret);
    return ret;
}

int32_t hal_tdm_start_stream(enum HAL_I2S_ID_T i2s_id, enum AUD_STREAM_T stream)
{
    int ret;

    TDM_TRACE(3,"%s: i2s_id = %d stream = %d.",
              __func__, i2s_id, stream);
    ASSERT(i2s_id < HAL_I2S_ID_QTY,"%s: i2s_id = %d!", __func__, i2s_id);

#ifdef CHIP_HAS_TDM
    ret = tdm_start_stream(i2s_id, stream);
#else
    ret = hal_i2s_tdm_start_stream(i2s_id);
#endif
    TDM_TRACE(2,"%s done. ret = %d.", __func__, ret);
    return ret;
}

int32_t hal_tdm_stop_stream(enum HAL_I2S_ID_T i2s_id, enum AUD_STREAM_T stream)
{
    int32_t ret;

    TDM_TRACE(3,"%s: i2s_id = %d stream = %d.",
              __func__, i2s_id, stream);
    ASSERT(i2s_id < HAL_I2S_ID_QTY,"%s: i2s_id = %d!", __func__, i2s_id);
#ifdef CHIP_HAS_TDM
    ret = tdm_stop_stream(i2s_id, stream);
#else
    ret = hal_i2s_tdm_stop_stream(i2s_id);
#endif
    TDM_TRACE(2,"%s done. ret = %d.", __func__, ret);
    return ret;
}

int32_t hal_tdm_close(enum HAL_I2S_ID_T i2s_id, enum AUD_STREAM_T stream)
{
    int32_t ret;

    TDM_TRACE(3,"%s: i2s_id = %d stream = %d.",
              __func__, i2s_id, stream);
    ASSERT(i2s_id < HAL_I2S_ID_QTY,"%s: i2s_id = %d!", __func__, i2s_id);
#ifdef CHIP_HAS_TDM
    ret = tdm_close(i2s_id, stream);
#else
    ret = hal_i2s_tdm_close(i2s_id);
#endif
    TDM_TRACE(2,"%s done. ret = %d.", __func__, ret);
    return ret;
}

void hal_tdm_get_config(enum HAL_I2S_ID_T i2s_id,struct HAL_TDM_CONFIG_T *tdm_cfg)
{
    ASSERT(i2s_id < HAL_I2S_ID_QTY,"%s: i2s_id = %d!", __func__, i2s_id);
#ifdef CHIP_HAS_TDM
    tdm_get_config(i2s_id,tdm_cfg);
#else
    struct HAL_I2S_TDM_CONFIG_T i2s_tdm_cfg;
    hal_i2s_tdm_get_config(i2s_id, &i2s_tdm_cfg);
    tdm_cfg->mode = HAL_TDM_MODE_FS_ASSERTED_AT_FIRST;
    tdm_cfg->edge = HAL_TDM_FS_EDGE_POSEDGE;
    tdm_cfg->cycles = (enum HAL_TDM_CYCLES_T)i2s_tdm_cfg.cycles;
    tdm_cfg->fs_cycles = (enum HAL_TDM_FS_CYCLES)i2s_tdm_cfg.fs_cycles;
    tdm_cfg->slot_cycles = (enum HAL_TDM_SLOT_CYCLES_T)i2s_tdm_cfg.slot_cycles;
    tdm_cfg->data_offset = i2s_tdm_cfg.data_offset;
#endif
}

void hal_tdm_set_config(enum HAL_I2S_ID_T i2s_id,struct HAL_TDM_CONFIG_T *tdm_cfg)
{
    ASSERT(i2s_id < HAL_I2S_ID_QTY,"%s: i2s_id = %d!", __func__, i2s_id);
#ifdef CHIP_HAS_TDM
    tdm_set_config(i2s_id,tdm_cfg);
#else
    struct HAL_I2S_TDM_CONFIG_T i2s_tdm_cfg;
    i2s_tdm_cfg.cycles = HAL_I2S_TDM_CYCLES_32;
    i2s_tdm_cfg.fs_cycles = HAL_I2S_TDM_FS_CYCLES_16;
    i2s_tdm_cfg.slot_cycles = HAL_I2S_TDM_SLOT_CYCLES_16;
    i2s_tdm_cfg.data_offset = 0;
    hal_i2s_tdm_set_config(i2s_id, &i2s_tdm_cfg);
#endif
}

