/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include <stdlib.h>
#include <string.h>
#include <cmsis.h>
#include <stdio.h>
#ifdef RTOS
#include <cmsis_os.h>
#endif
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_aud.h"
#include "hal_codec.h"
#include "audioflinger.h"
#include "pmu.h"
#if defined(CODEC_DAC_DC_NV_DATA) || defined(CODEC_ADC_DC_NV_DATA)
#include "nvrecord_bt.h"
#include "nvrecord_env.h"
#include "nvrecord_dev.h"
#endif
#if defined(CODEC_DAC_DC_DYN_BUF) || defined(CODEC_ADC_DC_DYN_BUF)
#include "heap_api.h"
#endif
#ifdef CODEC_DAC_ON_OFF_TEST
#include "hwtimer_list.h"
#endif
#include "tgt_hardware.h"
#include "plat_types.h"
#include "bluetooth_bt_api.h"

#ifdef AUDIO_OUTPUT_DC_AUTO_CALIB
//#define CODEC_DAC_DC_MUTE_TEST

#define DAC_DC_SET_VALID_CH(c)    (DAC_DC_VALID_MARK|((c)&0xF))
#define DAC_DC_GET_VALID_CH(v)    ((v)&0xF)
#define DAC_DC_GET_VALID_MARK(v)  ((v)&(~(0xF)))

#define DC_OUT_VALUE      0x2000
#define DC_CALIB_BUF_SIZE 0x7000

bool static af_open_for_dac_dc_calib = false;

static void codec_dac_dc_dump_param(char *s, void *cfg);

static uint8_t *codec_dac_dc_calib_malloc(uint32_t len)
{
#if !defined(CODEC_DAC_DC_DYN_BUF)
    static uint8_t play_dac_buff[DC_CALIB_BUF_SIZE];
    ASSERT(len <= DC_CALIB_BUF_SIZE, "%s: len=%d out of range %d", __func__, len, DC_CALIB_BUF_SIZE);
    return (uint8_t *)play_dac_buff;
#else
    uint8_t *play_dac_buff = NULL;

    ASSERT(len <= DC_CALIB_BUF_SIZE, "%s: len=%d out of range %d", __func__, len, DC_CALIB_BUF_SIZE);
    syspool_init_specific_size(DC_CALIB_BUF_SIZE);
    syspool_get_buff(&play_dac_buff, len);
    ASSERT(play_dac_buff != NULL, "%s: alloc buff failed", __func__);
    return play_dac_buff;
#endif
}

static void codec_dac_dc_calib_free(uint8_t *buf)
{
    //TODO: free memory
}

#ifdef ANC_PROD_TEST

extern enum AUD_CHANNEL_MAP_T anc_ff_mic_ch_l;
extern enum AUD_CHANNEL_MAP_T anc_ff_mic_ch_r;
extern enum AUD_CHANNEL_MAP_T anc_fb_mic_ch_l;
extern enum AUD_CHANNEL_MAP_T anc_fb_mic_ch_r;
extern enum AUD_CHANNEL_MAP_T anc_tt_mic_ch_l;
extern enum AUD_CHANNEL_MAP_T anc_tt_mic_ch_r;

static void codec_calib_disable_anc_prod_param(bool disable)
{
#ifndef USB_AUDIO_APP
    static enum AUD_CHANNEL_MAP_T tmp_anc_ff_mic_ch_l=AUD_CHANNEL_MAP_CH0;
    static enum AUD_CHANNEL_MAP_T tmp_anc_ff_mic_ch_r=AUD_CHANNEL_MAP_CH1;
    static enum AUD_CHANNEL_MAP_T tmp_anc_fb_mic_ch_l=AUD_CHANNEL_MAP_CH2;
    static enum AUD_CHANNEL_MAP_T tmp_anc_fb_mic_ch_r=AUD_CHANNEL_MAP_CH3;
    static enum AUD_CHANNEL_MAP_T tmp_anc_tt_mic_ch_l=AUD_CHANNEL_MAP_CH0;
    static enum AUD_CHANNEL_MAP_T tmp_anc_tt_mic_ch_r=AUD_CHANNEL_MAP_CH1;

    if (disable) {
        tmp_anc_ff_mic_ch_l = anc_ff_mic_ch_l;
        tmp_anc_ff_mic_ch_r = anc_ff_mic_ch_r;
        tmp_anc_fb_mic_ch_l = anc_fb_mic_ch_l;
        tmp_anc_fb_mic_ch_r = anc_fb_mic_ch_r;
        tmp_anc_tt_mic_ch_l = anc_tt_mic_ch_l;
        tmp_anc_tt_mic_ch_r = anc_tt_mic_ch_r;

        anc_ff_mic_ch_l = AUD_CHANNEL_MAP_CH0;
        anc_ff_mic_ch_r = AUD_CHANNEL_MAP_CH1;
        anc_fb_mic_ch_l = AUD_CHANNEL_MAP_CH2;
        anc_fb_mic_ch_r = AUD_CHANNEL_MAP_CH3;
        anc_tt_mic_ch_l = AUD_CHANNEL_MAP_CH0;
        anc_tt_mic_ch_r = AUD_CHANNEL_MAP_CH1;
    } else {
        anc_ff_mic_ch_l = tmp_anc_ff_mic_ch_l;
        anc_ff_mic_ch_r = tmp_anc_ff_mic_ch_r;
        anc_fb_mic_ch_l = tmp_anc_fb_mic_ch_l;
        anc_fb_mic_ch_r = tmp_anc_fb_mic_ch_r;
        anc_tt_mic_ch_l = tmp_anc_tt_mic_ch_l;
        anc_tt_mic_ch_r = tmp_anc_tt_mic_ch_r;
    }
#endif
}

#define codec_calib_save_anc_prod_param()    codec_calib_disable_anc_prod_param(true)
#define codec_calib_restore_anc_prod_param() codec_calib_disable_anc_prod_param(false)
#endif

#if defined(CODEC_DAC_DC_CHECK)
int hal_codec_check_dac_dc_offset(bool major, int range_idx, int32_t dc_l, int32_t dc_r);
static int codec_dac_dc_check_calib_cfg(struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *cfg, uint32_t num)
{
    int r = 0;
    int32_t dc_val_l=0, dc_val_r=0;
    uint32_t i;

    if (num < 2) {
        return -1;
    }

    dc_val_l = ((int32_t)(cfg[0].dc_l) + (int32_t)(cfg[1].dc_l)) / 2;

#ifdef AUDIO_OUTPUT_DC_CALIB_DUAL_CHAN
    dc_val_r = ((int32_t)(cfg[0].dc_r) + (int32_t)(cfg[1].dc_r)) / 2;
#endif

    r = hal_codec_check_dac_dc_offset(true, 1, dc_val_l, dc_val_r);
    if (r) {
        TRACE(0, "[%s]: DC err1, r=0x%x", __func__, r);
        return r;
    }
    for (i = 0; i < num; i++) {
        int32_t dc_l = (int32_t)(cfg[i].dc_l) - dc_val_l;
        r = hal_codec_check_dac_dc_offset(false, 1, dc_l, 0);
        if (r) {
            TRACE(0, "[%s]: DC err2, r=0x%x", __func__, r);
            return r;
        }
    }
    return 0;
}

static int codec_dac_dc_check(void)
{
    struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *cfg;
    uint32_t num = 0, i;
    int r;

    TRACE(0, "[%s]: ", __func__);

    cfg = hal_codec_dac_dre_get_calib_cfg(&num);
    for (i = 0; i < num; i++) {
        codec_dac_dc_dump_param("DRE_CALIB_PARAM", (void *)&cfg[i]);
    }
    r = codec_dac_dc_check_calib_cfg(cfg, num);

#ifdef PROGRAMMER
    if (r) {
        hal_cmu_simu_fail();
    } else {
        hal_cmu_simu_pass();
    }
    for(;;);
#endif

    if (r) {
        TRACE(0, "#### CHECK CALIB CFG ERROR ###, r=0x%x", r);
    } else {
        TRACE(0, ">>>> CHECK CALIB CFG SUCCESS <<<<");
    }
    return 0;
}

int codec_dac_dc_check_nv(void)
{
#ifdef CODEC_DAC_DC_NV_DATA
#define DRE_ERR_RET(r) (r|0x1000)
#define DC_ERR_RET(r)  (r|0x2000)
    struct HAL_CODEC_DAC_DRE_CALIB_CFG_NV_T *nv_cfg;
    struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *cfg;
    struct HAL_CODEC_DAC_DRE_CALIB_CFG_T tmp_cfg[2];
    int r = 0;
    uint32_t num = 0, i;

    TRACE(0, "[%s]: ", __func__);

    cfg = hal_codec_dac_dre_get_calib_cfg(&num);
    nv_cfg = nv_record_get_extension_entry_ptr()->dac_dre_calib_cfg_nv;
    if (nv_cfg==NULL) {
        TRACE(0, "[%s]: null ptr", __func__);
        return -1;
    }
    for (i = 0; i < num; i++) {
        cfg = (struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *)&nv_cfg[i];
        r = hal_codec_dac_dre_check_calib_cfg(cfg);
        if (r) {
            TRACE(0, "[%s]: DRE param err, r=%d", __func__, r);
            return DRE_ERR_RET(r);
        }
        memcpy((void *)&tmp_cfg[i], cfg, sizeof(struct HAL_CODEC_DAC_DRE_CALIB_CFG_T));
        codec_dac_dc_dump_param("DRE_CALIB_PARAM", (void *)&tmp_cfg[i]);
    }
    r = codec_dac_dc_check_calib_cfg(tmp_cfg, num);
    if (r) {
        TRACE(0, "[%s]: DC param err, r=%d", __func__, r);
        return DC_ERR_RET(r);
    }
    return 0;
#else
    ASSERT(false, "[%s]: Bad NV data", __func__);
#endif
}

#endif

static int codec_dac_dc_do_calib(uint32_t *status)
{
    int ret = 0;
    uint32_t i;
    uint32_t dc_l = 0, dc_r = 0;
    uint16_t ana_dc_l = 0, ana_dc_r = 0, max_l, max_r;
    enum HAL_SYSFREQ_USER_T user = HAL_SYSFREQ_USER_APP_7;
    uint32_t num;
    struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *cfg, *calib_cfg;
    bool calib_ch_l, calib_ch_r;
    struct AF_CODEC_CALIB_CFG_T acfg, *afcfg;

    afcfg = &acfg;
    memset((void*)afcfg, 0, sizeof(acfg));

    hal_sysfreq_req(user, HAL_CMU_FREQ_104M);
    hal_sysfreq_print_user_freq();
    TRACE(1, "cpu clock: %u", hal_sys_timer_calc_cpu_freq(5, 0));

#ifdef ANC_PROD_TEST
    codec_calib_save_anc_prod_param();
#endif

    if (af_open_for_dac_dc_calib) {
        af_open();
    }

    hal_codec_dac_dre_init_calib_cfg();
    hal_codec_set_dac_calib_status(true);
    calib_cfg = hal_codec_dac_dre_get_calib_cfg(&num);
    if (calib_cfg == NULL || num == 0) {
        TRACE(1, "%s: invalid calib_cfg=%x or num=%d", __func__, (int)calib_cfg, num);
        ret = -1;
        goto _exit0;
    }
    afcfg->len = DC_CALIB_BUF_SIZE;
    afcfg->buf = codec_dac_dc_calib_malloc(afcfg->len);

    calib_ch_l = true;
#ifdef AUDIO_OUTPUT_DC_CALIB_DUAL_CHAN
    calib_ch_r = true;
#else
    calib_ch_r = false;
#endif

    af_codec_calib_dac_chan_enable(calib_ch_l, calib_ch_r);
    af_codec_calib_dac_dc(CODEC_CALIB_CMD_OPEN, afcfg);

    max_l = max_r = 0;
#ifdef AUDIO_OUTPUT_SET_LARGE_ANA_DC
    for (i = 0, cfg = calib_cfg; i < num; i++, cfg++) {
        uint8_t ana_gain = cfg->ana_gain;
        uint8_t ini_gain = cfg->ini_ana_gain;
        uint8_t gain_offs = cfg->gain_offset;

        af_codec_calib_param_setup(DAC_PARAM_ANA_GAIN, ana_gain, ini_gain, gain_offs);
        ret = af_codec_calib_dac_dc(CODEC_CALIB_CMD_SET_LARGE_ANA_DC, afcfg);
        if (ret) {
            TRACE(1, "%s: error1 %d", __func__, ret);
            ret = -2;
            goto _exit1;
        }
        ana_dc_l = afcfg->ana_dc_l;
        ana_dc_r = afcfg->ana_dc_r;
        if(ana_dc_l > max_l) {
            max_l = ana_dc_l;
        }
        if(ana_dc_r > max_r) {
            max_r = ana_dc_r;
        }
    }
#else
    max_l = 0;
    max_r = 0;
#endif
    for (i = 0, cfg = calib_cfg; i < num; i++, cfg++) {
        cfg->ana_dc_l = max_l;
        cfg->ana_dc_r = max_r;
        TRACE(1, "AUTO CALIB ANA DAC DC: ana_gain=%d, L=0x%04X R=0x%04X, ret=%d",
            cfg->ana_gain, cfg->ana_dc_l, cfg->ana_dc_r, ret);
    }
    ana_dc_l = max_l;
    ana_dc_r = max_r;
    for (i = 0, cfg = calib_cfg; i < num; i++, cfg++) {
        uint8_t ana_gain = cfg->ana_gain;
        uint8_t ini_gain = cfg->ini_ana_gain;
        uint8_t gain_offs = cfg->gain_offset;

        af_codec_calib_param_setup(DAC_PARAM_ANA_GAIN, ana_gain, ini_gain, gain_offs);
        af_codec_calib_param_setup(DAC_PARAM_ANA_DC, ana_dc_l, ana_dc_r, 0);
        ret = af_codec_calib_dac_dc(CODEC_CALIB_CMD_DIG_DC, afcfg);
        if (ret) {
            TRACE(1, "%s: error2 %d", __func__, ret);
            ret = -3;
            goto _exit1;
        }
#ifdef AUDIO_OUTPUT_ERROR_LARGE_DC
        if (afcfg->state) {
            TRACE(1, "DAC CALIB DC ERR: state=%d, i=%d, gain=%d", afcfg->state, i, ana_gain);
            ret = afcfg->state;
            goto _exit1;
        }
#endif
        dc_l = afcfg->dig_dc_l;
        dc_r = afcfg->dig_dc_r;
        cfg->dc_l = dc_l;
        cfg->dc_r = dc_r;
        if (calib_ch_l) {
            cfg->valid |= DAC_DC_SET_VALID_CH(1<<0);
        }
        if (calib_ch_r) {
            cfg->valid |= DAC_DC_SET_VALID_CH(1<<1);
        }
        TRACE(1, "AUTO CALIB DAC DC: ana_gain=%d, L=0x%08X R=0x%08X, ret=%d",
            cfg->ana_gain, dc_l, dc_r, ret);
    }
    int32_t first_dc_l, first_dc_r;
    for (i = 0, cfg = calib_cfg; i < num; i++, cfg++) {
        uint8_t ana_gain = cfg->ana_gain;
        uint8_t ini_gain = cfg->ini_ana_gain;
        uint8_t gain_offs = cfg->gain_offset;
        uint32_t dig_dc_l = cfg->dc_l;
        uint32_t dig_dc_r = cfg->dc_r;
        float gain_comp_l, gain_comp_r;
        int out_l, out_r;

        af_codec_calib_param_setup(DAC_PARAM_ANA_GAIN, ana_gain, ini_gain, gain_offs);
        af_codec_calib_param_setup(DAC_PARAM_ANA_DC, ana_dc_l, ana_dc_r, 0);
        af_codec_calib_param_setup(DAC_PARAM_DIG_DC, dig_dc_l, dig_dc_r, 0);
        af_codec_calib_param_setup(DAC_PARAM_OUT_DC, DC_OUT_VALUE, 0, 0);
        ret = af_codec_calib_dac_dc(CODEC_CALIB_CMD_DIG_GAIN, afcfg);
        if (ret) {
            TRACE(1, "%s: error3 %d", __func__, ret);
            ret = -4;
            goto _exit1;
        }
        dc_l = afcfg->out_dc_l;
        dc_r = afcfg->out_dc_r;
        out_l = (int)dc_l;
        out_r = (int)dc_r;
        if (i == 0) {
            first_dc_l = out_l;
            first_dc_r = out_r;
            gain_comp_l = 1;
            gain_comp_r = 1;
        } else {
            gain_comp_l = (float)first_dc_l/(float)out_l;
            gain_comp_r = (float)first_dc_r/(float)out_r;
        }
        cfg->gain_l = gain_comp_l;
        cfg->gain_r = gain_comp_r;
        if (calib_ch_l) {
            cfg->valid |= DAC_DC_SET_VALID_CH(1<<2);
        }
        if (calib_ch_r) {
            cfg->valid |= DAC_DC_SET_VALID_CH(1<<3);
        }
        TRACE(1, "AUTO CALIB DAC GAIN: ana_gain=%d, L*1000=%d R*1000=%d",
            cfg->ana_gain, (int)(gain_comp_l*1000), (int)(gain_comp_r*1000));
    }
#if !defined(CODEC_DAC_DC_NV_DATA)
    for (i = 0; i < num; i++) {
        codec_dac_dc_dump_param("DRE_CALIB_PARAM", (void *)&calib_cfg[i]);
    }
#endif

_exit1:
    af_codec_calib_dac_dc(CODEC_CALIB_CMD_CLOSE, afcfg);
    codec_dac_dc_calib_free(afcfg->buf);
    hal_codec_set_dac_calib_status(false);
_exit0:
    if (af_open_for_dac_dc_calib) {
        af_close();
    }

#ifdef ANC_PROD_TEST
    codec_calib_restore_anc_prod_param();
#endif

    hal_sysfreq_req(user, HAL_CMU_FREQ_32K);
    return ret;
}

static void codec_dac_dc_dump_param(char *s, void *cfg)
{
    struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *c = (struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *)cfg;

    TRACE(1, "--------DUMP DAC DC param: %s, cfg=%x--------", s, (uint32_t)cfg);
    TRACE(1, "valid:        0x%x", c->valid);
    TRACE(1, "dc_l:         0x%x(%d)", c->dc_l, c->dc_l);
    TRACE(1, "dc_r:         0x%x(%d)", c->dc_r, c->dc_r);
    TRACE(1, "ana_dc_l:     0x%x", c->ana_dc_l);
    TRACE(1, "ana_dc_r:     0x%x", c->ana_dc_r);
    TRACE(1, "gain_l*1000:  %d", (int)(c->gain_l*1000));
    TRACE(1, "gain_r*1000:  %d", (int)(c->gain_r*1000));
    TRACE(1, "ana_gain:     0x%x", c->ana_gain);
    TRACE(1, "ini_ana_gain: 0x%x", c->ini_ana_gain);
    TRACE(1, "gain_offset:  0x%x", c->gain_offset);
    TRACE(1, "step_mode:    0x%x", c->step_mode);
    TRACE(1, "top_gain:     0x%x", c->top_gain);
    TRACE(1, "--------DUMP END--------");
}

static bool codec_dac_dc_load_calib_value(void)
{
#ifdef CODEC_DAC_DC_NV_DATA
    struct HAL_CODEC_DAC_DRE_CALIB_CFG_T tmp[4] = {0};
    struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *cfg;
    struct HAL_CODEC_DAC_DRE_CALIB_CFG_NV_T *nv_cfg;
    uint32_t num, i, valid = 0;
    bool success = false;

    TRACE(1, "%s: start", __func__);
    cfg = hal_codec_dac_dre_get_calib_cfg(&num);
    nv_cfg = nv_record_get_extension_entry_ptr()->dac_dre_calib_cfg_nv;
    TRACE(1, "cfg=%x, nv_cfg=%x, num=%d", (int)cfg, (int)nv_cfg, num);

    for (i = 0; i < num; i++) {
        int error = 0;
        codec_dac_dc_dump_param("NV", (void *)&nv_cfg[i]);
        error = hal_codec_dac_dre_check_calib_cfg((struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *)&nv_cfg[i]);
        if (!error) {
            TRACE(1, "nv_cfg[%d](%x) -> tmp[%d](%x)", i, (int)(&nv_cfg[i]), i, (int)(&tmp[i]));
            memcpy((void *)&tmp[i], (void *)&nv_cfg[i], sizeof(struct HAL_CODEC_DAC_DRE_CALIB_CFG_T));
            valid++;
        }
    }
    if (valid == num) {
        for (i = 0; i < num; i++) {
            TRACE(1, "tmp[%d](%x) -> cfg[%d](%x)", i, (int)(&tmp[i]), i, (int)(&cfg[i]));
            memcpy((void *)&cfg[i], (void *)&tmp[i], sizeof(struct HAL_CODEC_DAC_DRE_CALIB_CFG_T));
            codec_dac_dc_dump_param("NV->DC", (void *)&cfg[i]);
        }
        success = true;
    }
    TRACE(1, "%s: success=%d, valid=%d, num=%d", __func__, success, valid, num);
    return success;
#else
    return false;
#endif
}

static bool codec_dac_dc_save_calib_value(void)
{
#ifdef CODEC_DAC_DC_NV_DATA
    uint32_t lock;
    struct HAL_CODEC_DAC_DRE_CALIB_CFG_NV_T *nv_cfg;
    struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *cfg;
    uint32_t num, i;

    cfg = hal_codec_dac_dre_get_calib_cfg(&num);
    nv_cfg = nv_record_get_extension_entry_ptr()->dac_dre_calib_cfg_nv;
    TRACE(1, "%s: cfg=%x, nv_cfg=%x, num=%d", __func__, (int)cfg, (int)nv_cfg, num);

    lock = nv_record_pre_write_operation();
    for (i = 0; i < num; i++) {
        int error = hal_codec_dac_dre_check_calib_cfg(&cfg[i]);
        ASSERT(!error, "%s: invalid param %x", __func__, error);

        TRACE(1, "nv_cfg[%d]=%x, cfg[%d]=%x", i, (int)(&nv_cfg[i]), i, (int)(&cfg[i]));
        memcpy((void *)&nv_cfg[i], (void *)&cfg[i], sizeof(struct HAL_CODEC_DAC_DRE_CALIB_CFG_T));
        codec_dac_dc_dump_param("DC->NV", (void *)&cfg[i]);
    }
    nv_record_post_write_operation(lock);
    nv_record_update_runtime_userdata();
    nv_record_flash_flush();
    osDelay(50);
#endif
    TRACE(1, "%s: done", __func__);
    return true;
}

#if defined(CODEC_DAC_DC_MUTE_TEST) || defined(CODEC_DAC_ON_OFF_TEST)
static uint32_t play_data_handler(uint8_t *buf, uint32_t len)
{
    uint32_t curtime = TICKS_TO_MS(hal_sys_timer_get());

    memset((void*)buf, 0, len);

    (void)curtime;
    //TRACE(1, "[%8d] PLAY: buf=%x, len=%d", curtime, (int)buf, len);
    return 0;
}
#endif

#ifdef CODEC_DAC_DC_MUTE_TEST
int codec_dac_play_mute(bool on)
{
#define PLAY_RATE  (AUD_SAMPRATE_48000)
#define PLAY_BITS  (AUD_BITS_24)
#define PLAY_SIZE  (4)
#define PLAY_FRM   (8)
#define PLAY_CHNUM (AUD_CHANNEL_NUM_2)
#define PLAY_CHMAP (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1)
    int ret = 0;

    TRACE(1, "%s: on=%d", __func__, on);
    if (on) {
        struct AF_STREAM_CONFIG_T stream_cfg;
        uint32_t play_buf_size = (PLAY_RATE/1000)*PLAY_SIZE*PLAY_CHNUM*PLAY_FRM;
        uint8_t *play_buf = codec_dac_dc_calib_malloc(play_buf_size);

        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_2, HAL_CMU_FREQ_52M);
        if (af_open_for_dac_dc_calib) {
            af_open();
        }
        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.sample_rate = PLAY_RATE;
        stream_cfg.bits        = PLAY_BITS;
        stream_cfg.channel_num = PLAY_CHNUM;
        stream_cfg.channel_map = PLAY_CHMAP;
        stream_cfg.device      = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.vol         = 15;
        stream_cfg.handler     = play_data_handler;
        stream_cfg.io_path     = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.data_ptr    = play_buf;
        stream_cfg.data_size   = play_buf_size;
        TRACE(1,"playback rate: %d, bits = %d, buf_size=%d",
            stream_cfg.sample_rate, stream_cfg.bits, play_buf_size);
        ret = af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);
        ASSERT(ret == 0, "af_stream_open playback failed: %d", ret);
        ret = af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        ASSERT(ret == 0, "af_stream_start playback failed: %d", ret);

        while (1) {
#ifndef RTOS
            extern void af_thread(void const *argument);
            af_thread(NULL);
#else
            osDelay(20);
#endif
        }

    } else {
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        if (af_open_for_dac_dc_calib) {
            af_close();
        }
        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_2, HAL_CMU_FREQ_32K);
    }
    return ret;
}
#endif

#ifdef CODEC_DAC_ON_OFF_TEST
#define PLAY_RATE  (AUD_SAMPRATE_48000)
#define PLAY_BITS  (AUD_BITS_24)
#define PLAY_SIZE  (4)
#define PLAY_FRM   (8)
#define PLAY_CHNUM (AUD_CHANNEL_NUM_2)
#define PLAY_CHMAP (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1)
static struct AF_STREAM_CONFIG_T stream_cfg;

#define MAX_TIME_MS          (TICKS_TO_MS(0xFFFFFFFF))
#define SW_TIMER_PERIOD_TIME (3000)
#define SW_TIMER_TICK_TIME   (20)
static volatile uint32_t sw_timer_fired = 0;
static volatile uint32_t sw_timer_started = 0;
static volatile uint32_t sw_timer_begine_tick = 0;
static volatile uint32_t sw_timer_period_ms = 0;
static volatile uint32_t sw_timer_cnt = 0;

static int sw_timer_run_tick(void);

#ifndef RTOS
static HWTIMER_ID hwt = NULL;
static uint32_t hwt_period = 0;
static void hwt_timeout(void *param)
{
    sw_timer_run_tick();
}

static void hw_timer_start(uint32_t period_ms)
{
    hwt_period = MS_TO_TICKS(period_ms);
    if (hwt == NULL) {
        hwt = hwtimer_alloc(hwt_timeout, 0);
    }
    hwtimer_start(hwt, hwt_period);
}
#endif

static void sw_timer_init(uint32_t period_ms)
{
    sw_timer_started = 0;
    sw_timer_fired = 0;
    sw_timer_begine_tick = hal_sys_timer_get();
    sw_timer_period_ms = period_ms;
#ifndef RTOS
    hw_timer_start(period_ms);
#endif
}

static int sw_timer_run_tick(void)
{
    if (sw_timer_started) {
        uint32_t cur_time = hal_sys_timer_get();
        uint32_t elapsed_ms = 0;

        elapsed_ms = TICKS_TO_MS(cur_time - sw_timer_begine_tick);
        if (elapsed_ms >= sw_timer_period_ms) {
            sw_timer_fired = 1;
            sw_timer_cnt++;
            TRACE(1, "========> [%d] CODEC SW TIMER FIRED [%d]", sw_timer_cnt, elapsed_ms);
        }
        return sw_timer_fired;
    }
    return 0;
}

static void sw_timer_process(void)
{
    static int codec_state = 0;
    int ret = 0;

#ifdef RTOS
    sw_timer_run_tick();
#endif

    if (!sw_timer_started || !sw_timer_fired) {
        return;
    }
    if (sw_timer_fired) {
        codec_state = codec_state ^ 1;
    }
    if (codec_state) {
        ret = af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);
        ASSERT(ret == 0, "af_stream_open playback failed: %d", ret);

        ret = af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        ASSERT(ret == 0, "af_stream_start playback failed: %d", ret);
    } else {
        ret = af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        ret = af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    }
    if (sw_timer_fired) {
        sw_timer_init(SW_TIMER_PERIOD_TIME);
        sw_timer_started = true;
    }
}

int codec_dac_on_off_test(void)
{
    int ret = 0;
    uint32_t play_buf_size = (PLAY_RATE/1000)*PLAY_SIZE*PLAY_CHNUM*PLAY_FRM;
    uint8_t *play_buf = codec_dac_dc_calib_malloc(play_buf_size);

    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_2, HAL_CMU_FREQ_52M);
    if (af_open_for_dac_dc_calib) {
        af_open();
    }
    memset(&stream_cfg, 0, sizeof(stream_cfg));
    stream_cfg.sample_rate = PLAY_RATE;
    stream_cfg.bits        = PLAY_BITS;
    stream_cfg.channel_num = PLAY_CHNUM;
    stream_cfg.channel_map = PLAY_CHMAP;
    stream_cfg.device      = AUD_STREAM_USE_INT_CODEC;
    stream_cfg.vol         = 15;
    stream_cfg.handler     = play_data_handler;
    stream_cfg.io_path     = AUD_OUTPUT_PATH_SPEAKER;
    stream_cfg.data_ptr    = play_buf;
    stream_cfg.data_size   = play_buf_size;
    TRACE(1,"playback rate: %d, bits = %d, buf_size=%d",
        stream_cfg.sample_rate, stream_cfg.bits, play_buf_size);

    sw_timer_init(SW_TIMER_PERIOD_TIME);
    sw_timer_started = true;

    while (1) {
        sw_timer_process();
#ifndef RTOS
        extern void af_thread(void const *argument);
        af_thread(NULL);
#else
        osDelay(SW_TIMER_TICK_TIME);
#endif
    }
    return ret;
}
#endif /* CODEC_DAC_ON_OFF_TEST */

int codec_dac_dc_auto_load(bool open_af, bool reboot, bool init_nv)
{
    static int done = 0;
    int r = 0;
    uint32_t time = hal_sys_timer_get();

#if defined(CODEC_DAC_DC_CHECK) && defined(PROGRAMMER)
    hal_cmu_simu_set_val(0xCAFE);
#endif
    if (done) {
        TRACE(1, "%s: already done", __func__);
        return 0;
    }
    af_open_for_dac_dc_calib = open_af;
    TRACE(1, "%s: start: open_af=%d, reboot=%d", __func__, open_af, reboot);
#ifdef CODEC_DAC_DC_NV_DATA
    if (init_nv) {
        nv_record_init();
    }
#endif
    if (!codec_dac_dc_load_calib_value()) {
        r = codec_dac_dc_do_calib(NULL);
#ifdef AUDIO_OUTPUT_ERROR_LARGE_DC
        if (r) {
            TRACE(1, "%s: calib DAC DC again !!", __func__);
            r = codec_dac_dc_do_calib(NULL);
        }
#endif
        if (!r) {
            codec_dac_dc_save_calib_value();
            codec_dac_dc_load_calib_value();
            done = 1;
            if (reboot) {
                hal_sys_timer_delay(MS_TO_TICKS(5));
                pmu_reboot();
            }
        } else {
            ASSERT(false, "%s: calib DAC DC failed !!", __func__);
        }
    }
    time = hal_sys_timer_get() - time;
    TRACE(1, "%s: CALIB_COST_TIME=%d, r=%d", __func__, TICKS_TO_MS(time), r);

#ifdef CODEC_DAC_DC_MUTE_TEST
    codec_dac_play_mute(true);
#elif defined(CODEC_DAC_ON_OFF_TEST)
    codec_dac_on_off_test();
#elif defined(CODEC_DAC_DC_CHECK)
    codec_dac_dc_check();
#endif
    return r;
}

#ifdef AUDIO_OUTPUT_DC_PROD_TEST
#define CHAN_L 0
#define CHAN_R 1
#define CHAN_QTY 2
#define GAIN_QTY 2
#define DC_RANGE_THRES  (80)
#define DC_COMP_MAX     (262143) //((2^18)-1)
#define DC_COMP_ABS     (DC_COMP_MAX * DC_RANGE_THRES / 100)
#define DC_DET_MAX      (65535) //((2^16)-1)
#define DC_DET_ABS      (DC_DET_MAX * DC_RANGE_THRES / 100)

int codec_dac_dc_prod_test(int cmd)
{
    int err = 0;
    uint32_t i;
    enum HAL_SYSFREQ_USER_T user = HAL_SYSFREQ_USER_APP_7;
    uint32_t num;
    struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *cfg, *calib_cfg;
    bool calib_chan_l, calib_chan_r;
    struct AF_CODEC_CALIB_CFG_T acfg, *pcfg;

    TRACE(1, "%s: start", __func__);

    pcfg = &acfg;
    memset((void*)&acfg, 0, sizeof(acfg));

    hal_sysfreq_req(user, HAL_CMU_FREQ_104M);
    hal_sys_timer_delay(MS_TO_TICKS(2));
//    hal_sysfreq_print_user_freq();

    hal_codec_dac_dre_init_calib_cfg();
    hal_codec_set_dac_calib_status(true);

    calib_cfg = hal_codec_dac_dre_get_calib_cfg(&num);

    ASSERT((calib_cfg!=NULL) && (num>0),
        "%s: invalid calib_cfg=%x or num=%d", __func__, (int)calib_cfg, num);

    pcfg->len = DC_CALIB_BUF_SIZE;
    pcfg->buf = codec_dac_dc_calib_malloc(pcfg->len);

    calib_chan_l = true;
#ifdef AUDIO_OUTPUT_DC_CALIB_DUAL_CHAN
    calib_chan_r = true;
#else
    calib_chan_r = false;
#endif

    // select DAC channel;
    af_codec_calib_dac_chan_enable(calib_chan_l, calib_chan_r);

    // open stream
    af_codec_calib_dac_dc(CODEC_CALIB_CMD_OPEN, pcfg);

    // read adc/dac dc value for each analog gain
    int32_t adc_dc[GAIN_QTY][CHAN_QTY];
    int32_t dac_dc[GAIN_QTY][CHAN_QTY];
    int32_t det_dc[GAIN_QTY][CHAN_QTY];
    ASSERT(num <= GAIN_QTY, "%s: num(%d) is too large, qty=%d", __func__, num, GAIN_QTY);

    cfg = calib_cfg;
    for (i = 0; i < num; i++) {
        TRACE(1, "set ana_gain=%d, ini_gain=%d, gain_offset=%d",
            cfg->ana_gain, cfg->ini_ana_gain, cfg->gain_offset);
        af_codec_calib_param_setup(DAC_PARAM_ANA_GAIN,
            cfg->ana_gain, cfg->ini_ana_gain, cfg->gain_offset);

        err = af_codec_calib_dac_dc(CODEC_CALIB_CMD_GET_CUR_DC, pcfg);
        if (err) {
            TRACE(1, "%s: get cur dc failed %d", __func__, err);
            goto _exit;
        }
        adc_dc[i][CHAN_L] = pcfg->dig_dc_l;
        adc_dc[i][CHAN_R] = pcfg->dig_dc_r;
        dac_dc[i][CHAN_L] = pcfg->out_dc_l;
        dac_dc[i][CHAN_R] = pcfg->out_dc_r;
        det_dc[i][CHAN_L] = dac_dc[i][CHAN_L] - adc_dc[i][CHAN_L];
        det_dc[i][CHAN_R] = dac_dc[i][CHAN_R] - adc_dc[i][CHAN_R];

        int32_t tmp, max;

        // check adc dc l & r
        max = (int32_t)DC_COMP_ABS;
        TRACE(1, "DC_COMP_ABS=%d, DC_RANGE_THRES=%d",DC_COMP_ABS, DC_RANGE_THRES);

        if (calib_chan_l) {
            tmp = ABS(adc_dc[i][CHAN_L]);
            if (tmp > max) {
                err |= (1<<(i*4+CHAN_L));
                TRACE(1, "adc_dc_l is too large: %d", tmp);
            }
        }
        if (calib_chan_r) {
            tmp = ABS(adc_dc[i][CHAN_R]);
            if (tmp > max) {
                err |= (1<<(i*4+CHAN_R));
                TRACE(1, "adc_dc_r is too large: %d", tmp);
            }
        }
        // check adc dc l & r
        if (calib_chan_l) {
            tmp = ABS(dac_dc[i][CHAN_L]);
            if (tmp > max) {
                err |= (1<<(i*4+CHAN_L+2));
                TRACE(1, "dac_dc_l is too large: %d", tmp);
            }
        }
        if (calib_chan_r) {
            tmp = ABS(dac_dc[i][CHAN_R]);
            if (tmp > max) {
                err |= (1<<(i*4+CHAN_R+2));
                TRACE(1, "dac_dc_r is too large: %d", tmp);
            }
        }
        if (err) {
            TRACE(1, "======== FIND LARGE DC: err=%x, ========", err);
            TRACE(1, "adc_dc_l=%d, adc_dc_r=%d", adc_dc[i][CHAN_L], adc_dc[i][CHAN_R]);
            TRACE(1, "dac_dc_l=%d, dac_dc_r=%d", dac_dc[i][CHAN_L], dac_dc[i][CHAN_R]);
            TRACE(1, "========================================");
            goto _exit;
        }
        cfg++;
    }
    int32_t diff_dc, det_max = DC_DET_ABS;

    if (calib_chan_l) {
        diff_dc = ABS(det_dc[1][CHAN_L] - det_dc[0][CHAN_L]);
        TRACE(1, "L: diff_dc=%d, det_max=%d", diff_dc, det_max);
        if (diff_dc > det_max) {
            TRACE(1, "L: diff dc is too large: [%d, %d]",det_dc[1][CHAN_L],det_dc[0][CHAN_L]);
            err |= 0x100;
        }
    }
    if (calib_chan_r) {
        diff_dc = ABS(det_dc[1][CHAN_R] - det_dc[0][CHAN_R]);
        TRACE(1, "R: diff_dc=%d, det_max=%d", diff_dc, det_max);
        if (diff_dc > det_max) {
            TRACE(1, "R: diff dc is too large: [%d, %d]",det_dc[1][CHAN_R],det_dc[0][CHAN_R]);
            err |= 0x200;
        }
    }
    if (err) {
        TRACE(1, "======== FIND LARGE DET DC: err=%x, ========", err);
        goto _exit;
    }

    TRACE(1, "DAC DC TEST SUCCESS !!");
_exit:
    af_codec_calib_dac_dc(CODEC_CALIB_CMD_CLOSE, pcfg);
    hal_sysfreq_req(user, HAL_CMU_FREQ_32K);
    return err;
}
#endif

#endif /* AUDIO_OUTPUT_DC_AUTO_CALIB */

#ifdef AUDIO_ADC_DC_AUTO_CALIB
#include "app_utils.h"

#ifndef CFG_ADC_DC_CALIB_MIC_DEV
#define CFG_ADC_DC_CALIB_MIC_DEV                (AUD_CHANNEL_MAP_CH1 | AUD_CHANNEL_MAP_CH4)
#endif
#ifndef CFG_ADC_DC_CALIB_CH_NUM
#define CFG_ADC_DC_CALIB_CH_NUM                 (AUD_CHANNEL_NUM_2)
#endif

#define ADC_DC_SET_VALID_CH(c)                  (ADC_DC_VALID_MARK | ((c) & 0x1F))
#define ADC_DC_GET_VALID_CH(v)                  ((v) & 0x1F)
#define ADC_DC_GET_VALID_MARK(v)                ((v) & (~(0x1F)))

#define NORMAL_ADC_CH_NUM                       (5)

#define ADC_DMA_DC_VALUE_INVALID                (0x7FFFFFFF)
#define ADC_CALIB_DMA_DATA_READY_SIGNAL         (0x01 << 13)

#define CAPTURE_SAMPLE_RATE                     (AUD_SAMPRATE_8000)
#define CAPTURE_BITS_SIZE                       (AUD_BITS_24)
#define CAPTURE_CHANNEL_NUM                     (CFG_ADC_DC_CALIB_CH_NUM)
#define CAPTURE_DMA_FRAME_MS                    (5) //ms

#define ADC_DC_CALIB_GET_BIT_OFFSET_DELAY_MS    (CAPTURE_DMA_FRAME_MS * 6)
#define APP_AUTO_CALIB_ADC_DC_CH_MAP            (CFG_ADC_DC_CALIB_MIC_DEV)

static int32_t  adc_avg[NORMAL_ADC_CH_NUM], adc_max[NORMAL_ADC_CH_NUM], adc_min[NORMAL_ADC_CH_NUM];
static uint32_t adc_max_index[NORMAL_ADC_CH_NUM], adc_min_index[NORMAL_ADC_CH_NUM];

#define ADC_DC_CALIB_MAX_THRESOLD_VALUE         (21)  // 30uv/1.4=21
#define ADC_DC_CALIB_MAX_CNT_NUM                (2)

typedef enum {
    ADC_DC_VALUE_READ_STATE_IDLE,
    ADC_DC_VALUE_READ_STATE_WAIT_READY,
    ADC_DC_VALUE_READ_STATE_READY_DONE,
} ADC_DC_VALUE_READ_STATE_E;

static ADC_DC_VALUE_READ_STATE_E adc_dc_data_state = ADC_DC_VALUE_READ_STATE_IDLE;

#ifdef RTOS
static osThreadId_t adc_dc_calib_thread_tid = NULL;
#else
extern void af_thread(void const *argument);
#endif

//#define AD_DA_AF_STREAM_LOOP_TEST
#ifdef AD_DA_AF_STREAM_LOOP_TEST
static enum APP_AUDIO_CACHE_T play_data_cache_status = APP_AUDIO_CACHE_QTY;
#endif

static void codec_adc_dc_dump_param(char *s, void *cfg)
{
    osDelay(5);
    struct HAL_CODEC_ADC_DC_CALIB_CFG_T *c = (struct HAL_CODEC_ADC_DC_CALIB_CFG_T *)cfg;
    TRACE(1, "--------DUMP ADC DC param: %s, cfg=%x--------", s, (uint32_t)cfg);
    TRACE(1, "calib_valid:                  0x%x",      c->valid);
    TRACE(1, "channel_map:                  0x%04x",    c->adc_channel_map);
    TRACE(1, "dc_origin_value:              %d",        c->adc_dc_original);
    TRACE(1, "dc_calib_result:              %d",        c->adc_dc_calib_result);
    TRACE(1, "dc_calib_reg_offset:          0x%04x",    c->adc_dc_calib_offset);
    TRACE(1, "dc_calib_reg_step:            0x%04x",    c->adc_dc_calib_step);
    TRACE(1, "dc_calib_fixed_dc_offset:     0x%04x",    c->rsvd0);
    TRACE(1, "--------DUMP END--------");
}

static bool codec_adc_dc_load_calib_value(void)
{
#ifdef CODEC_ADC_DC_NV_DATA
    struct HAL_CODEC_ADC_DC_CALIB_CFG_T tmp[8] = {0};
    struct HAL_CODEC_ADC_DC_CALIB_CFG_T *cfg;
    HAL_CODEC_ADC_DC_CALIB_CFG_NV_T *nv_cfg;
    uint32_t num, i, valid = 0;
    bool success = false;

    osDelay(5);
    TRACE(1, "%s: start", __func__);
    cfg = hal_codec_adc_get_calib_cfg(&num);
    ASSERT(num <= ARRAY_SIZE(tmp), "calib_cfg too large, tmp_size = %d, cfg_size = %d", ARRAY_SIZE(tmp), num);

    nv_cfg = nv_record_get_extension_entry_ptr()->adc_dc_calib_cfg_info;
    TRACE(1, "cfg=%x, nv_cfg=%x, num=%d", (int)cfg, (int)nv_cfg, num);

    for (i = 0; i < num; i++) {
        codec_adc_dc_dump_param("NV", (void *)&nv_cfg[i]);
        if(ADC_DC_GET_VALID_MARK(nv_cfg[i].valid) != ADC_DC_VALID_MARK) {
            TRACE(0, "valid mark update, old_mark = 0x%x, new_mark = 0x%x",ADC_DC_GET_VALID_MARK(nv_cfg[i].valid), ADC_DC_VALID_MARK);
            break;
        }
        if (ADC_DC_GET_VALID_CH(nv_cfg[i].valid)) {
            valid++;
        }
        TRACE(1, "nv_cfg[%d](%x) -> tmp[%d](%x)", i, (int)(&nv_cfg[i]), i, (int)(&tmp[i]));
        memcpy((void *)&tmp[i], (void *)&nv_cfg[i], sizeof(struct HAL_CODEC_ADC_DC_CALIB_CFG_T));
    }
    TRACE(1, " load calib val valid=%d, num=%d", valid, num);
    if (valid == CAPTURE_CHANNEL_NUM) {
        for (i = 0; i < num; i++) {
            TRACE(1, "tmp[%d](%x) -> cfg[%d](%x)", i, (int)(&tmp[i]), i, (int)(&cfg[i]));
            memcpy((void *)&cfg[i], (void *)&tmp[i], sizeof(struct HAL_CODEC_ADC_DC_CALIB_CFG_T));
            codec_adc_dc_dump_param("NV->DC", (void *)&cfg[i]);
        }
        success = true;
    }
    TRACE(1, "%s: success=%d, valid=%d, num=%d", __func__, success, valid, num);
    return success;
#else
    return false;
#endif
}

static bool codec_adc_dc_save_calib_value(void)
{
#ifdef CODEC_ADC_DC_NV_DATA
    uint32_t lock;
    HAL_CODEC_ADC_DC_CALIB_CFG_NV_T *nv_cfg;
    struct HAL_CODEC_ADC_DC_CALIB_CFG_T *cfg;
    uint32_t num, i;

    cfg = hal_codec_adc_get_calib_cfg(&num);
    nv_cfg = nv_record_get_extension_entry_ptr()->adc_dc_calib_cfg_info;
    TRACE(1, "%s: cfg=%x, nv_cfg=%x, num=%d", __func__, (int)cfg, (int)nv_cfg, num);

    lock = nv_record_pre_write_operation();
    for (i = 0; i < num; i++) {
        TRACE(1, "nv_cfg[%d]=%x, cfg[%d]=%x", i, (int)(&nv_cfg[i]), i, (int)(&cfg[i]));
        memcpy((void *)&nv_cfg[i], (void *)&cfg[i], sizeof(struct HAL_CODEC_ADC_DC_CALIB_CFG_T));
        codec_adc_dc_dump_param("DC->NV", (void *)&cfg[i]);
    }
    nv_record_post_write_operation(lock);
    nv_record_update_runtime_userdata();
    nv_record_flash_flush();
    osDelay(50);
#endif
    TRACE(1, "%s: done", __func__);
    return true;
}

#ifdef AD_DA_AF_STREAM_LOOP_TEST
static uint32_t adc_dc_calib_stream_loop_play_more_data(uint8_t *buf, uint32_t len)
{
    if (play_data_cache_status != APP_AUDIO_CACHE_QTY)
    {
        app_audio_pcmbuff_get(buf, len);
    }
    return len;
}
#endif

static uint32_t adc_dc_calib_stream_data_come(uint8_t *buf, uint32_t len)
{
    int32_t *samp;
    uint32_t samp_cnt;
    uint32_t i;
    uint32_t j;
    int32_t  adc_avg_tmp[CAPTURE_CHANNEL_NUM], adc_max_tmp[CAPTURE_CHANNEL_NUM];
    int32_t  adc_min_tmp[CAPTURE_CHANNEL_NUM];
    uint32_t adc_max_index_tmp[CAPTURE_CHANNEL_NUM], adc_min_index_tmp[CAPTURE_CHANNEL_NUM];
    uint32_t dma_ch_index = 0;
    uint32_t mic_ch = 0;
    enum AUD_CHANNEL_MAP_T calib_ch_map = APP_AUTO_CALIB_ADC_DC_CH_MAP;

    samp = (int32_t *)buf;
    samp_cnt = len / CAPTURE_CHANNEL_NUM / 4;

    if (adc_dc_data_state == ADC_DC_VALUE_READ_STATE_WAIT_READY)
    {
        //mid-value calibrate method
        bool sort_done;
        int32_t tmp;
        for (i = 0; i < samp_cnt; i++)
        {
            sort_done = true;
            for (uint32_t k = 0; k < samp_cnt - i - 1; k++)
            {
                for (j = 0; j < CAPTURE_CHANNEL_NUM; j++)
                {
                    if (samp[k*CAPTURE_CHANNEL_NUM + j] > samp[(k+1)*CAPTURE_CHANNEL_NUM + j])
                    {
                        tmp = samp[k*CAPTURE_CHANNEL_NUM + j];
                        samp[k*CAPTURE_CHANNEL_NUM + j] = samp[(k+1)*CAPTURE_CHANNEL_NUM + j];
                        samp[(k+1)*CAPTURE_CHANNEL_NUM + j] = tmp;
                        sort_done = false;
                    }
                }
            }
            if (sort_done == true)
                break;
        }

        for (j = 0; j < CAPTURE_CHANNEL_NUM; j++)
        {
            adc_avg_tmp[j] = samp[samp_cnt/2*CAPTURE_CHANNEL_NUM + j]/9;
            adc_max_tmp[j] = samp[(samp_cnt-1)*CAPTURE_CHANNEL_NUM + j]/9;
            adc_min_tmp[j] = samp[j]/9;
            adc_min_index_tmp[j] = j;
            adc_max_index_tmp[j] = (samp_cnt-1)*CAPTURE_CHANNEL_NUM + j;
        }

        //remap dma channel to mic channel
        for (mic_ch = 0; mic_ch < NORMAL_ADC_CH_NUM; mic_ch++)
        {
            if (calib_ch_map & (AUD_CHANNEL_MAP_CH0 << mic_ch))
            {
                adc_avg[mic_ch] = adc_avg_tmp[dma_ch_index];
                adc_max[mic_ch] = adc_max_tmp[dma_ch_index];
                adc_min[mic_ch] = adc_min_tmp[dma_ch_index];
                adc_max_index[mic_ch] = adc_max_index_tmp[dma_ch_index];
                adc_min_index[mic_ch] = adc_min_index_tmp[dma_ch_index];
                dma_ch_index++;

                TRACE(1, "#### adc_ch=%d ####", mic_ch);
                TRACE(6, "ADC avg=%d max[%d]=%d min[%d]=%d delta=%d",
                        adc_avg[mic_ch], adc_max_index[mic_ch], adc_max[mic_ch],
                        adc_min_index[mic_ch], adc_min[mic_ch], adc_max[mic_ch]-adc_min[mic_ch]);
            }
        }
        adc_dc_data_state = ADC_DC_VALUE_READ_STATE_READY_DONE;
#ifdef RTOS
        if (adc_dc_calib_thread_tid != NULL)
        {
            osSignalSet(adc_dc_calib_thread_tid, ADC_CALIB_DMA_DATA_READY_SIGNAL);
        }
#endif
    }

#ifdef AD_DA_AF_STREAM_LOOP_TEST
    app_audio_pcmbuff_put(buf, len);
    if (play_data_cache_status == APP_AUDIO_CACHE_QTY)
    {
        play_data_cache_status = APP_AUDIO_CACHE_OK;
    }
#endif

#if 1
    static uint8_t value_print_int = 0;
    value_print_int++;
    if (value_print_int%100 == 0)
    {
        value_print_int = 0;
        TRACE(0, "\n\n");
        TRACE(4, "capture sample: %d %d %d %d len=%d",
                    samp[0]/9, samp[1]/9, samp[2]/9, samp[3]/9, len);
        TRACE(0, "\n\n");
    }
#endif
    return len;
}

#define REAL_CAPTURE_BUFF_SIZE         (48 * CAPTURE_CHANNEL_NUM * CAPTURE_DMA_FRAME_MS * 4)
#define ALIGNED4                       ALIGNED(4)

static uint8_t *codec_adc_dc_calib_malloc(uint32_t len)
{
#if !defined(CODEC_ADC_DC_DYN_BUF)
    static uint8_t ALIGNED4 cap_adc_buff[REAL_CAPTURE_BUFF_SIZE];
    ASSERT(len <= REAL_CAPTURE_BUFF_SIZE, "%s: len=%d out of range %d", __func__, len, REAL_CAPTURE_BUFF_SIZE);
    return (uint8_t *)cap_adc_buff;
#else
    uint8_t *cap_adc_buff = NULL;

    ASSERT(len <= REAL_CAPTURE_BUFF_SIZE, "%s: len=%d out of range %d", __func__, len, REAL_CAPTURE_BUFF_SIZE);
    syspool_init_specific_size(REAL_CAPTURE_BUFF_SIZE);
    syspool_get_buff(&cap_adc_buff, len);
    ASSERT(cap_adc_buff != NULL, "%s: alloc buff failed", __func__);
    return cap_adc_buff;
#endif
}

static int start_adc_dc_calib_capture_stream(bool on, enum APP_SYSFREQ_FREQ_T freq)
{
#ifdef AD_DA_AF_STREAM_LOOP_TEST
    uint8_t *buff_play = NULL;
    uint8_t *buff_loop = NULL;
#endif
    uint8_t *buff_cap = codec_adc_dc_calib_malloc(REAL_CAPTURE_BUFF_SIZE);

    struct AF_STREAM_CONFIG_T stream_cfg;
    static bool isRun =  false;
    enum HAL_SYSFREQ_USER_T user = HAL_SYSFREQ_USER_APP_7;
    TRACE(3, "start_adc_capture_stream work:%d op:%d freq:%d", isRun, on, freq);

    if (isRun==on)
        return 0;

    if (on)
    {
        hal_sysfreq_req(user, HAL_CMU_FREQ_104M);
        hal_sysfreq_print_user_freq();
        TRACE(1, "cpu clock: %u", hal_sys_timer_calc_cpu_freq(5, 0));

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.bits = CAPTURE_BITS_SIZE;
        stream_cfg.channel_num = CAPTURE_CHANNEL_NUM;
        stream_cfg.sample_rate = CAPTURE_SAMPLE_RATE;
#if FPGA==0
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
#else
        stream_cfg.device = AUD_STREAM_USE_EXT_CODEC;
#endif
        stream_cfg.vol = TGT_VOLUME_LEVEL_15;
        stream_cfg.io_path = AUD_INPUT_PATH_DC_CALIB;
        stream_cfg.handler = adc_dc_calib_stream_data_come;

        stream_cfg.data_size = REAL_CAPTURE_BUFF_SIZE;
        stream_cfg.data_ptr = buff_cap;
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);

#ifdef AD_DA_AF_STREAM_LOOP_TEST
        stream_cfg.data_size = dma_size*2;
        stream_cfg.vol = TGT_VOLUME_LEVEL_15;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.handler = adc_dc_calib_stream_loop_play_more_data;

        app_audio_mempool_get_buff(&buff_play, stream_cfg.data_size);
        app_audio_mempool_get_buff(&buff_loop, stream_cfg.data_size*2);
        app_audio_pcmbuff_init(buff_loop, stream_cfg.data_size*2);

        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(buff_play);

        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
#endif
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_set_chan_vol(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, APP_AUTO_CALIB_ADC_DC_CH_MAP, 1);
        TRACE(0, "start_adc_capture_stream on");
    }
    else
    {
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
#ifdef AD_DA_AF_STREAM_LOOP_TEST
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
#endif
        TRACE(0, "start_adc_capture_stream off");
        hal_sysfreq_req(user, HAL_CMU_FREQ_32K);
    }

    isRun=on;
    return 0;
}

static uint32_t adc_dc_calib_porc(uint32_t mic_ch, int32_t dc_val, int32_t offset_gain[])
{
    int32_t i = 0;
    uint16_t reg_val = 0;

    if (dc_val > 0)
    {
        reg_val |= (1<<13);
    }

    for (i = 12; i>= 0; i--)
    {
        if (ABS(dc_val) >= ABS(offset_gain[i]))
        {
            dc_val = dc_val + offset_gain[i];
            reg_val |= (1<<i);
            TRACE(3, "bit:%d dc_val:%d offset:%d", i, dc_val, offset_gain[i]);
        }
    }
    TRACE(3, "dc calib channel:%d calib_offset:0x%04x calib_result:%d", mic_ch, reg_val, dc_val);
    return reg_val;
}

static int32_t adc_dc_value_wait_signal_ready_get(int32_t wait_signal)
{
    int32_t ret = -1;
#ifdef RTOS
    osEvent evt = osSignalWait(0x00, osWaitForever);
    if(osEventSignal == evt.status)
    {
        switch (evt.value.signals)
        {
            case ADC_CALIB_DMA_DATA_READY_SIGNAL:
                ret = 0;
                break;
            default:
                break;
        }
    }
#else
    af_thread(NULL);
    ret = 0;
#endif
    return ret;
}

#ifdef AUDIO_ADC_DIG_DC_CALIB
static int codec_adc_dig_dc_do_calib(bool on)
{
    int16_t reg_bit = 0;
    enum AUD_CHANNEL_MAP_T calib_ch_map = APP_AUTO_CALIB_ADC_DC_CH_MAP;
    struct HAL_CODEC_ADC_DC_CALIB_CFG_T *cfg, *calib_cfg;
    int32_t adc_dc_value[NORMAL_ADC_CH_NUM] = {0};
    int16_t adc_dc_offset[NORMAL_ADC_CH_NUM] = {0};
    uint32_t num = 0;
    uint16_t comp_val[NORMAL_ADC_CH_NUM] = {0};

#ifdef RTOS
    adc_dc_calib_thread_tid = osThreadGetId();
#endif
    if (on)
    {
#ifdef RTOS
        af_set_priority(AF_USER_TEST, osPriorityHigh);
#endif
        hal_codec_set_adc_calib_status(true);
        start_adc_dc_calib_capture_stream(true, APP_SYSFREQ_104M);

        hal_codec_adc_dc_auto_calib_enable(calib_ch_map);
        osDelay(ADC_DC_CALIB_GET_BIT_OFFSET_DELAY_MS);

        calib_cfg = hal_codec_adc_get_calib_cfg(&num);

        hal_codec_adc_dig_dc_offset_enable(calib_ch_map, 0);
        for (reg_bit = 15; reg_bit >= 0; ) {
            osDelay(ADC_DC_CALIB_GET_BIT_OFFSET_DELAY_MS);
            adc_dc_data_state = ADC_DC_VALUE_READ_STATE_WAIT_READY;
            adc_dc_value_wait_signal_ready_get(ADC_CALIB_DMA_DATA_READY_SIGNAL);

            for (uint32_t ch_index = 0; ch_index < num; ch_index++) {
                if (calib_ch_map & (AUD_CHANNEL_MAP_CH0 << ch_index)) {
                    adc_dc_value[ch_index] = adc_avg[ch_index];

                    if (adc_dc_value[ch_index] < 0) {
                        if (reg_bit < 15) {
                            comp_val[ch_index] &= ~(1 << (reg_bit + 1));
                            comp_val[ch_index] |= (1 << reg_bit);
                        } else {
                            comp_val[ch_index] |= (1 << reg_bit);
                        }
                    } else {
                        if (reg_bit < 15) {
                            comp_val[ch_index] |= (1 << reg_bit);
                        } else {
                            comp_val[ch_index] &= ~(1 << reg_bit);
                        }
                    }

                    if (ABS(adc_dc_value[ch_index]) > 50) {
                        adc_dc_offset[ch_index] = (int16_t)(comp_val[ch_index]);
                    }

                    hal_codec_adc_dig_dc_offset_enable((AUD_CHANNEL_MAP_CH0 << ch_index), adc_dc_offset[ch_index]);

                    TRACE(2, "[%d] dc calib mic_ch:%d adc_dc_value:%d adc_dc_offset:0x%04x",
                        reg_bit, ch_index, adc_dc_value[ch_index], adc_dc_offset[ch_index]);
                }
            }
            reg_bit--;
        }

        cfg = calib_cfg;
        for (uint32_t ch_index = 0; ch_index < num; ch_index++, cfg++) {
            if (calib_ch_map & (AUD_CHANNEL_MAP_CH0 << ch_index)) {
                cfg->rsvd0 = adc_dc_offset[ch_index];
            }
        }

        // close audio stream before operate NV record
        hal_codec_adc_dc_auto_calib_disable(calib_ch_map);
        start_adc_dc_calib_capture_stream(false, APP_SYSFREQ_32K);
        hal_codec_set_adc_calib_status(false);
#ifdef RTOS
        af_reset_priority();
#endif

        //flush result to nv and dump nv info
        codec_adc_dc_save_calib_value();

        //dump adc calib param
        cfg = calib_cfg;
        for (uint16_t i = 0; i < num; i++) {
            codec_adc_dc_dump_param("ADC_CALIB_PARAM", (void *)&cfg[i]);
        }
    }
    return 0;
}
#endif

static int codec_adc_dc_do_calib(bool on)
{
    uint16_t dc_reg_step = 0x8;
    uint16_t dc_reg_offset = 0;
    uint16_t reg_bit = 0;
    enum AUD_CHANNEL_MAP_T calib_ch_map = APP_AUTO_CALIB_ADC_DC_CH_MAP;
    enum AUD_CHANNEL_MAP_T active_ch_map = 0;
    struct HAL_CODEC_ADC_DC_CALIB_CFG_T *cfg, *calib_cfg;
    int32_t adc_dc_value[NORMAL_ADC_CH_NUM][14] = {0};
    int32_t adc_dc_offset[NORMAL_ADC_CH_NUM][14] = {0};
    int32_t adc_dc_offset_max[NORMAL_ADC_CH_NUM] = {0};
    uint16_t calib_recheck_ok_cnt = 0;
    uint16_t calib_mic_cnt = 0;
    uint16_t dc_calib_cnt = 0;
    uint32_t num = 0;

#ifdef RTOS
    adc_dc_calib_thread_tid = osThreadGetId();
#endif
    if (on)
    {
#ifdef RTOS
        af_set_priority(AF_USER_TEST, osPriorityHigh);
#endif
#ifdef AD_DA_AF_STREAM_LOOP_TEST
        play_data_cache_status = APP_AUDIO_CACHE_QTY;
#endif
        hal_codec_set_adc_calib_status(true);
        start_adc_dc_calib_capture_stream(true, APP_SYSFREQ_104M);

        hal_codec_adc_dc_auto_calib_enable(calib_ch_map);
        osDelay(ADC_DC_CALIB_GET_BIT_OFFSET_DELAY_MS);

dc_calib_recheck:
        dc_calib_cnt++;
        //TRACE(2, "dc calib test_times=%d dc_calib_cnt=%d", dc_calib_test_times, dc_calib_cnt);

        calib_cfg = hal_codec_adc_get_calib_cfg(&num);
        //get the original dc value
        TRACE(0, "===========Get the original adc dc value===========");
        for (uint32_t ch_index = 0; ch_index < num; ch_index++)
        {
            if (calib_ch_map & (AUD_CHANNEL_MAP_CH0 << ch_index))
            {
                active_ch_map = AUD_CHANNEL_MAP_CH0 << ch_index;
                dc_reg_offset = 0;
                hal_codec_adc_dc_offset_update(active_ch_map, dc_reg_step, dc_reg_offset);
            }
        }

        osDelay(ADC_DC_CALIB_GET_BIT_OFFSET_DELAY_MS);
        adc_dc_data_state = ADC_DC_VALUE_READ_STATE_WAIT_READY;
        adc_dc_value_wait_signal_ready_get(ADC_CALIB_DMA_DATA_READY_SIGNAL);
        for (uint32_t ch_index = 0; ch_index < num; ch_index++)
        {
            if (calib_ch_map & (AUD_CHANNEL_MAP_CH0 << ch_index))
            {
                adc_dc_value[ch_index][13] = adc_avg[ch_index];
                TRACE(2, "dc calib mic_ch:%d original_dc:%d", ch_index, adc_dc_value[ch_index][13]);
            }
        }

        //get the corresponding bit dc absolute value
        for (reg_bit = 0; reg_bit < 13; reg_bit++)
        {
            for (uint32_t ch_index = 0; ch_index < num; ch_index++)
            {
                if (calib_ch_map & (AUD_CHANNEL_MAP_CH0 << ch_index))
                {
                    active_ch_map = AUD_CHANNEL_MAP_CH0 << ch_index;
                    dc_reg_offset = 0;
                    if (adc_dc_value[ch_index][13] > 0)
                    {
                        dc_reg_offset |= (1<<13);
                    }
                    dc_reg_offset |= (1<<reg_bit);
                    TRACE(2, "===========Set the ret bit adc dc mic_ch:%d value:0x%04x===========",
                            ch_index, dc_reg_offset);
                    hal_codec_adc_dc_offset_update(active_ch_map, dc_reg_step, dc_reg_offset);
                }
            }

            osDelay(ADC_DC_CALIB_GET_BIT_OFFSET_DELAY_MS);
            adc_dc_data_state = ADC_DC_VALUE_READ_STATE_WAIT_READY;
            adc_dc_value_wait_signal_ready_get(ADC_CALIB_DMA_DATA_READY_SIGNAL);

            for (uint32_t ch_index = 0; ch_index < num; ch_index++)
            {
                if (calib_ch_map & (AUD_CHANNEL_MAP_CH0 << ch_index))
                {
                    adc_dc_value[ch_index][reg_bit] = adc_avg[ch_index];
                }
            }
        }

        //get the reg bit dc offset
        cfg = calib_cfg;
        for (uint32_t ch_index = 0; ch_index < num; ch_index++, cfg++)
        {
            if (calib_ch_map & (AUD_CHANNEL_MAP_CH0 << ch_index))
            {
                active_ch_map = AUD_CHANNEL_MAP_CH0 << ch_index;

                adc_dc_offset_max[ch_index] = 0;
                if (adc_dc_value[ch_index][13] > 0)
                    adc_dc_offset[ch_index][13] = 1;
                else
                    adc_dc_offset[ch_index][13] = 0;
                for (reg_bit = 0; reg_bit < 13; reg_bit++)
                {
                    adc_dc_offset[ch_index][reg_bit] = adc_dc_value[ch_index][reg_bit] - adc_dc_value[ch_index][13];
                    adc_dc_offset_max[ch_index] += adc_dc_offset[ch_index][reg_bit];
                }
                TRACE(1, "===========Dump adc dc register info: %d===========", ch_index);
                for (reg_bit = 0; reg_bit < 14; reg_bit++)
                {
                    TRACE(3, "%d:   %d:      %d",
                            reg_bit, adc_dc_value[ch_index][reg_bit], adc_dc_offset[ch_index][reg_bit]);
                }

                //get the dc calibrate offset
                if (ABS(adc_dc_value[ch_index][13]) >
                                ABS(adc_dc_offset_max[ch_index]) + ADC_DC_CALIB_MAX_THRESOLD_VALUE)
                {
                    TRACE(3, "[ERROR] channel(%d) dc value exceeds the max calib value, dc_orig=%d offset_max=%d!!\n",
                                ch_index, adc_dc_value[ch_index][13], adc_dc_offset_max[ch_index]);
                    cfg->valid = ADC_DC_SET_VALID_CH(0);
                    dc_reg_offset = 0x1FFF;
                    if (adc_dc_value[ch_index][13] > 0)
                    {
                        dc_reg_offset |= (1<<13);
                    }
                }
                else
                {
                    cfg->valid = ADC_DC_SET_VALID_CH(1<<ch_index);
                    dc_reg_offset = adc_dc_calib_porc(ch_index, adc_dc_value[ch_index][13], adc_dc_offset[ch_index]);
                }
                cfg->adc_channel_map = active_ch_map;
                cfg->adc_dc_calib_step = dc_reg_step;
                cfg->adc_dc_calib_offset = dc_reg_offset;
                cfg->adc_dc_original = adc_dc_value[ch_index][13];
            } else {
                cfg->valid = ADC_DC_SET_VALID_CH(0);
                cfg->adc_channel_map = AUD_CHANNEL_MAP_CH0 << ch_index;
            }
        }

        //recheck the calibrate result
        cfg = calib_cfg;
        for (uint32_t ch_index = 0; ch_index < num; ch_index++, cfg++)
        {
            if (calib_ch_map & (AUD_CHANNEL_MAP_CH0 << ch_index))
            {
                active_ch_map = AUD_CHANNEL_MAP_CH0 << ch_index;
                hal_codec_adc_dc_offset_update(active_ch_map, dc_reg_step, cfg->adc_dc_calib_offset);
            }
        }

        osDelay(ADC_DC_CALIB_GET_BIT_OFFSET_DELAY_MS);
        adc_dc_data_state = ADC_DC_VALUE_READ_STATE_WAIT_READY;
        adc_dc_value_wait_signal_ready_get(ADC_CALIB_DMA_DATA_READY_SIGNAL);
        calib_recheck_ok_cnt = 0;
        calib_mic_cnt = 0;

        cfg = calib_cfg;
        for (uint32_t ch_index = 0; ch_index < num; ch_index++, cfg++)
        {
            if (calib_ch_map & (AUD_CHANNEL_MAP_CH0 << ch_index))
            {
                cfg->adc_dc_calib_result = adc_avg[ch_index];
                if (ABS(cfg->adc_dc_calib_result) <= ADC_DC_CALIB_MAX_THRESOLD_VALUE)
                {
                    calib_recheck_ok_cnt++;
                }
                calib_mic_cnt++;
            }
        }
        if ((calib_recheck_ok_cnt != calib_mic_cnt) && (dc_calib_cnt < ADC_DC_CALIB_MAX_CNT_NUM))
        {
            goto dc_calib_recheck;
        }

        // close audio stream before operate NV record
        dc_calib_cnt = 0;
        hal_codec_adc_dc_auto_calib_disable(calib_ch_map);
        start_adc_dc_calib_capture_stream(false, APP_SYSFREQ_32K);
        hal_codec_set_adc_calib_status(false);
#ifdef RTOS
        af_reset_priority();
#endif

        //flush result to nv and dump nv info
        codec_adc_dc_save_calib_value();

        //dump adc calib param
        cfg = calib_cfg;
        for (uint16_t i = 0; i < num; i++) {
            codec_adc_dc_dump_param("ADC_CALIB_PARAM", (void *)&cfg[i]);
        }
    }
    else
    {
#if 0
        dc_calib_cnt = 0;
        hal_codec_adc_dc_auto_calib_disable(calib_ch_map);
#ifdef RTOS
        af_reset_priority();
#endif
        start_adc_dc_calib_capture_stream(false, APP_SYSFREQ_32K);
#endif
    }
    return 0;
}

int codec_adc_dc_auto_load(bool open_af, bool reboot, bool init_nv)
{
    static int done = 0;
    int ret = 0;
    uint32_t time = hal_sys_timer_get();

    if (done) {
        TRACE(1, "%s: already done", __func__);
        return 0;
    }
    if (open_af) {
        af_open();
    }
    TRACE(1, "%s: start: open_af=%d, reboot=%d", __func__, open_af, reboot);
#ifdef CODEC_ADC_DC_NV_DATA
    if (init_nv) {
        nv_record_init();
    }
#endif

    if (!codec_adc_dc_load_calib_value()) {
        ret = codec_adc_dc_do_calib(true);
#ifdef AUDIO_ADC_DIG_DC_CALIB
        ret |= codec_adc_dig_dc_do_calib(true);
#endif
        if (!ret) {
            codec_adc_dc_do_calib(false);
            codec_adc_dc_load_calib_value();
            done = 1;
            if (reboot) {
                hal_sys_timer_delay(MS_TO_TICKS(5));
                pmu_reboot();
            }
        } else {
            ASSERT(false, "%s: calib DAC DC failed !!", __func__);
        }
    }
    time = hal_sys_timer_get() - time;
    TRACE(1, "%s: CALIB_COST_TIME=%d, r=%d", __func__, TICKS_TO_MS(time), ret);

    if (open_af) {
        af_close();
    }
    return ret;
}
#endif /* AUDIO_ADC_DC_AUTO_CALIB */
