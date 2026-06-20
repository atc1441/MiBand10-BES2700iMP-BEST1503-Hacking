/***************************************************************************
 * Copyright 2022-2023 BES.
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
 ***************************************************************************/
#ifdef CAPSENSOR_FAC_CALCULATE
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "string.h"
#include "hal_trace.h"
#include "capsensor_factory_cal.h"
#include "tgt_hardware_capsensor.h"
#ifdef CHIP_SUBSYS_SENS
#include "app_sensor_hub.h"
#endif
#include "capsensor_algorithm.h"

static int cal_count = 0;
static bool cal_offset_flag = false;
static bool cal_diff_flag = false;
static int cal_data_sum[CAP_CHNUM] = {0};
static int cal_data_offset_avg[CAP_CHNUM] = {0};
static int cal_data_diff_avg[CAP_CHNUM] = {0};
static int cal_data_max[CAP_CHNUM] = {0};
static int cal_data_min[CAP_CHNUM] = {0};
#if defined(CAPSENSOR_WEAR)
capsensor_wear_cal_data wear_cal_data;
capsensor_wear_cal_range wear_cal_range;
#endif
#if defined(CAPSENSOR_TOUCH)
capsensor_touch_cal_range touch_cal_range;
capsensor_touch_cal_data touch_cal_data;
#endif

#if defined(CHIP_SUBSYS_SENS)
void app_send_capsensor_factory_test_data(uint8_t * ptr, uint16_t len)
{
    //TRACE(2, "%s  send capsensor test %d", __func__, len);
    app_core_bridge_send_cmd(MCU_SENSOR_HUB_TASK_CMD_CAPSENSOR_FACTORY_TEST, ptr, len);
}

static void app_sensor_hub_capsensor_factory_test_cmd_received_handler(uint8_t* ptr, uint16_t len)
{
    CAPSENSOR_FACTORY_CAL_FLAG cap_cal_flag;
    cap_cal_flag = *((CAPSENSOR_FACTORY_CAL_FLAG*)ptr);
    //TRACE(2, "%s receive sensor hub data len %d", __func__, len);
    capsensor_factory_calculate_flag_set(cap_cal_flag.offset_cal, cap_cal_flag.diff_cal);
}

static void app_sensor_hub_spp_transmit_capsensor_test_cmd_handler(uint8_t* ptr, uint16_t len)
{
    //TRACE(2, "%s len %d", __func__, len);
    app_core_bridge_send_data_without_waiting_rsp(MCU_SENSOR_HUB_TASK_CMD_CAPSENSOR_FACTORY_TEST, ptr, len);
}

static void app_sensor_hub_capsensor_factory_test_tx_done_handler(uint16_t cmdCode, uint8_t* ptr, uint16_t len)
{
    //TRACE(2, "%s cmdCode 0x%x", __func__, cmdCode);
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_CAPSENSOR_FACTORY_TEST,
                                "spp receive no rsp req from sensor",
                                app_sensor_hub_spp_transmit_capsensor_test_cmd_handler,
                                app_sensor_hub_capsensor_factory_test_cmd_received_handler,
                                0,
                                NULL,
                                NULL,
                                app_sensor_hub_capsensor_factory_test_tx_done_handler);
#endif

static void capsensor_cal_offset(struct capsensor_sample_data * data, int len)
{
    uint32_t i = 0;
    uint32_t chan_data_sum[CAP_CHNUM*CAP_REPNUM];
    calcuate_ch_adc_sum(data, CAP_CHNUM, chan_data_sum, len);
    for(i = 0; i < CAP_CHNUM; i++)
    {
        cal_data_sum[i] += chan_data_sum[i];
        TRACE(0, "capsensor_cal_offset chan_data_sum[%d] = %d", i, chan_data_sum[i]);
        TRACE(0, "capsensor_cal_offset cal_data_sum[%d] = %d", i, cal_data_sum[i]);

        if(cal_count == 0)
        {
            cal_data_min[i] = chan_data_sum[i];
            TRACE(0, "capsensor_cal_offset cal_data_min[%d] = %d", i, cal_data_min[i]);
        }

        if(chan_data_sum[i] > cal_data_max[i])
        {
            cal_data_max[i] = chan_data_sum[i];
            TRACE(0, "capsensor_cal_offset cal_data_max[%d] = %d", i, cal_data_max[i]);
        }

        if(chan_data_sum[i] < cal_data_min[i])
        {
            cal_data_min[i] = chan_data_sum[i];
            TRACE(0, "capsensor_cal_offset cal_data_min[%d] = %d", i, cal_data_min[i]);
        }
    }

    if(cal_count >= 9)
    {
        cal_offset_flag = false;

        for(i = 0; i < CAP_CHNUM; i++)
        {
            cal_data_offset_avg[i] = cal_data_sum[i] / 10;
            TRACE(0, "capsensor_cal_offset cal_data_offset_avg[%d] = %d", i, cal_data_offset_avg[i]);
            cal_data_sum[i] = 0;
        }

#if defined(CAPSENSOR_WEAR)
        wear_cal_data.wear_offset0 = cal_data_offset_avg[cap_wear_config.wear_detect_channel_0] - cal_data_offset_avg[cap_wear_config.wear_reference_channel_0];
        wear_cal_data.wear_offset1 = cal_data_offset_avg[cap_wear_config.wear_detect_channel_1] - cal_data_offset_avg[cap_wear_config.wear_reference_channel_1];
        TRACE(0, "capsensor_cal_offset wear_offset0 = %d", wear_cal_data.wear_offset0);
        TRACE(0, "capsensor_cal_offset wear_offset1 = %d", wear_cal_data.wear_offset1);
        wear_cal_data.wear_noise0  = cal_data_max[cap_wear_config.wear_detect_channel_0] - cal_data_min[cap_wear_config.wear_detect_channel_0];
        wear_cal_data.wear_noise1  = cal_data_max[cap_wear_config.wear_detect_channel_1] - cal_data_min[cap_wear_config.wear_detect_channel_1];
        TRACE(0, "capsensor_cal_offset wear_max0 = %d", cal_data_max[cap_wear_config.wear_detect_channel_0]);
        TRACE(0, "capsensor_cal_offset wear_max1 = %d", cal_data_max[cap_wear_config.wear_detect_channel_1]);
        TRACE(0, "capsensor_cal_offset wear_min0 = %d", cal_data_min[cap_wear_config.wear_detect_channel_0]);
        TRACE(0, "capsensor_cal_offset wear_min1 = %d", cal_data_min[cap_wear_config.wear_detect_channel_1]);
        TRACE(0, "capsensor_cal_offset wear_noise0 = %d", wear_cal_data.wear_noise0);
        TRACE(0, "capsensor_cal_offset wear_noise1 = %d", wear_cal_data.wear_noise1);
#endif

#if defined(CAPSENSOR_TOUCH)
#ifdef CAPSENSOR_SLIDE
        touch_cal_data.touch_noise0 = cal_data_max[cap_touch_config.slide_channel_0] - cal_data_min[cap_touch_config.slide_channel_0];
        touch_cal_data.touch_noise1 = cal_data_max[cap_touch_config.slide_channel_1] - cal_data_min[cap_touch_config.slide_channel_1];
        touch_cal_data.touch_noise2 = cal_data_max[cap_touch_config.slide_channel_2] - cal_data_min[cap_touch_config.slide_channel_2];
        TRACE(0, "capsensor_cal_offset touch_max0 = %d", cal_data_max[cap_touch_config.slide_channel_0]);
        TRACE(0, "capsensor_cal_offset touch_max1 = %d", cal_data_max[cap_touch_config.slide_channel_1]);
        TRACE(0, "capsensor_cal_offset touch_max2 = %d", cal_data_max[cap_touch_config.slide_channel_2]);
        TRACE(0, "capsensor_cal_offset touch_min0 = %d", cal_data_min[cap_touch_config.slide_channel_0]);
        TRACE(0, "capsensor_cal_offset touch_min1 = %d", cal_data_min[cap_touch_config.slide_channel_1]);
        TRACE(0, "capsensor_cal_offset touch_min2 = %d", cal_data_min[cap_touch_config.slide_channel_2]);
        TRACE(0, "capsensor_cal_offset touch_noise0 = %d", touch_cal_data.touch_noise0);
        TRACE(0, "capsensor_cal_offset touch_noise1 = %d", touch_cal_data.touch_noise1);
        TRACE(0, "capsensor_cal_offset touch_noise2 = %d", touch_cal_data.touch_noise2);
#else
        touch_cal_data.touch_noise0 = cal_data_max[cap_touch_config.slide_channel_0] - cal_data_min[cap_touch_config.slide_channel_0];
        TRACE(0, "capsensor_cal_offset touch_max0 = %d", cal_data_max[cap_touch_config.slide_channel_0]);
        TRACE(0, "capsensor_cal_offset touch_min0 = %d", cal_data_min[cap_touch_config.slide_channel_0]);
        TRACE(0, "capsensor_cal_offset touch_noise0 = %d", touch_cal_data.touch_noise0);
#endif
#endif
    }
}

static void capsensor_cal_diff(struct capsensor_sample_data * data, int len)
{
    uint32_t i = 0;
    uint32_t chan_data_sum[CAP_CHNUM*CAP_REPNUM];
    calcuate_ch_adc_sum(data, CAP_CHNUM, chan_data_sum, len);
    for(i = 0; i < CAP_CHNUM; i++)
    {
        cal_data_sum[i] += chan_data_sum[i];
        TRACE(0, "capsensor_cal_diff chan_data_sum[%d] = %d", i, chan_data_sum[i]);
    }

    if(cal_count >= 9)
    {
        cal_diff_flag = false;

        for(i = 0; i < CAP_CHNUM; i++)
        {
            cal_data_diff_avg[i] = cal_data_sum[i] / 10;
            TRACE(0, "capsensor_cal_diff cal_data_diff_avg[%d] = %d", i, cal_data_diff_avg[i]);
        }

#if defined(CAPSENSOR_WEAR)
        wear_cal_data.wear_snr_result = 1;
        wear_cal_data.wear_diff_result = 1;

        wear_cal_data.wear_diff0 = cal_data_diff_avg[cap_wear_config.wear_detect_channel_0] - cal_data_diff_avg[cap_wear_config.wear_reference_channel_0] - wear_cal_data.wear_offset0;
        wear_cal_data.wear_diff1 = cal_data_diff_avg[cap_wear_config.wear_detect_channel_1] - cal_data_diff_avg[cap_wear_config.wear_reference_channel_1] - wear_cal_data.wear_offset1;
        TRACE(0, "capsensor_cal_diff wear_diff0 = %d", wear_cal_data.wear_diff0);
        TRACE(0, "capsensor_cal_diff wear_diff1 = %d", wear_cal_data.wear_diff1);

        if(!wear_cal_data.wear_noise0 || !wear_cal_data.wear_noise1)
        {
            TRACE(0, "capsensor_cal_diff wear noise = 0, return");
            wear_cal_data.wear_snr_result = 0;
            return;
        }

        wear_cal_data.wear_snr0 = wear_cal_data.wear_diff0 / wear_cal_data.wear_noise0;
        wear_cal_data.wear_snr1 = wear_cal_data.wear_diff1 / wear_cal_data.wear_noise1;
        TRACE(0, "capsensor_cal_diff wear_snr0 = %d", wear_cal_data.wear_snr0);
        TRACE(0, "capsensor_cal_diff wear_snr1 = %d", wear_cal_data.wear_snr1);

#if (CHIP_CAPSENSOR_VER < 1)
        if(wear_cal_data.wear_snr0 > wear_cal_range.wear_snr0_down || wear_cal_data.wear_snr0 > wear_cal_range.wear_snr1_down)
#else
        if(wear_cal_data.wear_snr0 < wear_cal_range.wear_snr0_down || wear_cal_data.wear_snr0 < wear_cal_range.wear_snr1_down)
#endif
        {
            wear_cal_data.wear_snr_result = 0;
            TRACE(0, "capsensor_cal_diff wear_snr_result = %d", wear_cal_data.wear_snr_result);
        }

        if(wear_cal_data.wear_diff0 > wear_cal_range.wear_diff0_up || wear_cal_data.wear_diff0 < wear_cal_range.wear_diff0_down)
        {
            wear_cal_data.wear_diff_result = 0;
            TRACE(0, "capsensor_cal_diff wear_diff0_result = %d", wear_cal_data.wear_diff_result);
        }

        if(wear_cal_data.wear_diff1 > wear_cal_range.wear_diff1_up || wear_cal_data.wear_diff1 < wear_cal_range.wear_diff1_down)
        {
            wear_cal_data.wear_diff_result = 0;
            TRACE(0, "capsensor_cal_diff wear_diff1_result = %d", wear_cal_data.wear_diff_result);
        }
#endif

#if defined(CAPSENSOR_TOUCH)
        touch_cal_data.touch_snr_result = 1;
        touch_cal_data.touch_diff_result = 1;
#ifdef CAPSENSOR_SLIDE
        touch_cal_data.touch_diff0 = cal_data_diff_avg[cap_touch_config.slide_channel_0] - cal_data_offset_avg[cap_touch_config.slide_channel_0];
        touch_cal_data.touch_diff1 = cal_data_diff_avg[cap_touch_config.slide_channel_1] - cal_data_offset_avg[cap_touch_config.slide_channel_1];
        touch_cal_data.touch_diff2 = cal_data_diff_avg[cap_touch_config.slide_channel_2] - cal_data_offset_avg[cap_touch_config.slide_channel_2];
        TRACE(0, "capsensor_cal_diff touch_diff0 = %d", touch_cal_data.touch_diff0);
        TRACE(0, "capsensor_cal_diff touch_diff1 = %d", touch_cal_data.touch_diff1);
        TRACE(0, "capsensor_cal_diff touch_diff2 = %d", touch_cal_data.touch_diff2);

        if(!touch_cal_data.touch_noise0 || !touch_cal_data.touch_noise1 || !touch_cal_data.touch_noise2)
        {
            TRACE(0, "capsensor_cal_diff touch noise = 0, return");
            touch_cal_data.touch_snr_result = 0;
            return;
        }

        touch_cal_data.touch_snr0 = touch_cal_data.touch_diff0 / touch_cal_data.touch_noise0;
        touch_cal_data.touch_snr1 = touch_cal_data.touch_diff1 / touch_cal_data.touch_noise1;
        touch_cal_data.touch_snr2 = touch_cal_data.touch_diff2 / touch_cal_data.touch_noise2;
        TRACE(0, "capsensor_cal_diff touch_snr0 = %d", touch_cal_data.touch_snr0);
        TRACE(0, "capsensor_cal_diff touch_snr1 = %d", touch_cal_data.touch_snr1);
        TRACE(0, "capsensor_cal_diff touch_snr2 = %d", touch_cal_data.touch_snr2);

        if(touch_cal_data.touch_snr0 > touch_cal_range.touch_snr0_down || touch_cal_data.touch_snr1 > touch_cal_range.touch_snr1_down || touch_cal_data.touch_snr2 > touch_cal_range.touch_snr2_down)
        {
            touch_cal_data.touch_snr_result = 0;
            TRACE(0, "capsensor_cal_diff touch_snr_result = %d", touch_cal_data.touch_snr_result);
        }

        if(touch_cal_data.touch_diff0 > touch_cal_range.touch_diff0_up || touch_cal_data.touch_diff0 < touch_cal_range.touch_diff0_down)
        {
            touch_cal_data.touch_diff_result = 0;
            TRACE(0, "capsensor_cal_diff touch_diff0_result = %d", touch_cal_data.touch_diff_result);
        }

        if(touch_cal_data.touch_diff1 > touch_cal_range.touch_diff1_up || touch_cal_data.touch_diff1 < touch_cal_range.touch_diff1_down)
        {
            touch_cal_data.touch_diff_result = 0;
            TRACE(0, "capsensor_cal_diff touch_diff1_result = %d", touch_cal_data.touch_diff_result);
        }

        if(touch_cal_data.touch_diff2 > touch_cal_range.touch_diff2_up || touch_cal_data.touch_diff2 < touch_cal_range.touch_diff2_down)
        {
            touch_cal_data.touch_diff_result = 0;
            TRACE(0, "capsensor_cal_diff touch_diff2_result = %d", touch_cal_data.touch_diff_result);
        }
#else
        touch_cal_data.touch_diff0 = cal_data_diff_avg[cap_touch_config.slide_channel_0] - cal_data_offset_avg[cap_touch_config.slide_channel_0];
        TRACE(0, "capsensor_cal_diff touch_diff0 = %d", touch_cal_data.touch_diff0);

        if(!touch_cal_data.touch_noise0)
        {
            TRACE(0, "capsensor_cal_diff touch noise = 0, return");
            touch_cal_data.touch_snr_result = 0;
            return;
        }

        touch_cal_data.touch_snr0 = touch_cal_data.touch_diff0 / touch_cal_data.touch_noise0;
        TRACE(0, "capsensor_cal_diff touch_snr0 = %d", touch_cal_data.touch_snr0);

        if(touch_cal_data.touch_snr0 > touch_cal_range.touch_snr0_down)
        {
            touch_cal_data.touch_snr_result = 0;
            TRACE(0, "capsensor_cal_diff touch_snr0_result = %d", touch_cal_data.touch_snr_result);
        }

        if(touch_cal_data.touch_diff0 > touch_cal_range.touch_diff0_up || touch_cal_data.touch_diff0 < touch_cal_range.touch_diff0_down)
        {
            touch_cal_data.touch_diff_result = 0;
            TRACE(0, "capsensor_cal_diff touch_diff0_result = %d", touch_cal_data.touch_diff_result);
        }
#endif
#endif

        for(i = 0; i < CAP_CHNUM; i++)
        {
            cal_data_sum[i] = 0;
            cal_data_offset_avg[i] = 0;
            cal_data_diff_avg[i] = 0;
            cal_data_max[i] = 0;
            cal_data_min[i] = 0;
        }
    }
}

#if defined(CAPSENSOR_WEAR)
void capsensor_set_wear_cal_range(capsensor_wear_cal_range* range)
{
    wear_cal_range.wear_offset_up = range->wear_offset_up;
    wear_cal_range.wear_offset_down = range->wear_offset_down;
    wear_cal_range.wear_diff0_up = range->wear_diff0_up;
    wear_cal_range.wear_diff0_down = range->wear_diff0_down;
    wear_cal_range.wear_diff1_up = range->wear_diff1_up;
    wear_cal_range.wear_diff1_down = range->wear_diff1_down;
    wear_cal_range.wear_snr0_down = range->wear_snr0_down;
    wear_cal_range.wear_snr1_down = range->wear_snr1_down;
}

void capsensor_get_wear_cal_data(capsensor_wear_cal_data* data)
{
    data->wear_offset0  = wear_cal_data.wear_offset0;
    data->wear_offset1  = wear_cal_data.wear_offset1;
    data->wear_noise0 = wear_cal_data.wear_noise0;
    data->wear_noise1 = wear_cal_data.wear_noise1;
    data->wear_diff0 = wear_cal_data.wear_diff0;
    data->wear_diff1 = wear_cal_data.wear_diff1;
    data->wear_snr0 = wear_cal_data.wear_snr0;
    data->wear_snr1 = wear_cal_data.wear_snr1;
    data->wear_snr_result = wear_cal_data.wear_snr_result;
    data->wear_diff_result  = wear_cal_data.wear_diff_result;
}
#endif

#if defined(CAPSENSOR_TOUCH)
void capsensor_set_touch_cal_range(capsensor_touch_cal_range* range)
{
#ifdef CAPSENSOR_SLIDE
    touch_cal_range.touch_diff0_up = range->touch_diff0_up;
    touch_cal_range.touch_diff0_down = range->touch_diff0_down;
    touch_cal_range.touch_diff1_up = range->touch_diff1_up;
    touch_cal_range.touch_diff1_down = range->touch_diff1_down;
    touch_cal_range.touch_diff2_up = range->touch_diff2_up;
    touch_cal_range.touch_diff2_down = range->touch_diff2_down;
    touch_cal_range.touch_snr0_down = range->touch_snr0_down;
    touch_cal_range.touch_snr1_down = range->touch_snr1_down;
    touch_cal_range.touch_snr2_down = range->touch_snr2_down;
#else
    touch_cal_range.touch_diff0_up = range->touch_diff0_up;
    touch_cal_range.touch_diff0_down = range->touch_diff0_down;
    touch_cal_range.touch_snr0_down = range->touch_snr0_down;
#endif
}

void capsensor_get_touch_cal_data(capsensor_touch_cal_data* data)
{
#ifdef CAPSENSOR_SLIDE
    data->touch_diff0 = touch_cal_data.touch_diff0;
    data->touch_diff1 = touch_cal_data.touch_diff1;
    data->touch_diff2 = touch_cal_data.touch_diff2;
    data->touch_noise0 = touch_cal_data.touch_noise0;
    data->touch_noise1 = touch_cal_data.touch_noise1;
    data->touch_noise2 = touch_cal_data.touch_noise2;
    data->touch_snr0 = touch_cal_data.touch_snr0;
    data->touch_snr1 = touch_cal_data.touch_snr1;
    data->touch_snr2 = touch_cal_data.touch_snr2;
#else
    data->touch_diff0 = touch_cal_data.touch_diff0;
    data->touch_noise0 = touch_cal_data.touch_noise0;
    data->touch_snr0 = touch_cal_data.touch_snr0;
#endif
    data->touch_snr_result = touch_cal_data.touch_snr_result;
    data->touch_diff_result = touch_cal_data.touch_diff_result;
}
#endif

void capsensor_factory_calculate_flag_set(bool m_cal_offset_flag, bool m_cal_diff_flag)
{
    cal_offset_flag = m_cal_offset_flag;
    cal_diff_flag = m_cal_diff_flag;
}

void capsensor_factory_calculate(struct capsensor_sample_data * data, int len)
{
    if(cal_offset_flag)
    {
        capsensor_cal_offset(data, len);
        TRACE(0, "capsensor_cal_offset cal_count = %d", cal_count);
        if(cal_count >= 9)
        {
            cal_count = 0;
        }
        else
        {
            cal_count++;
        }
    }

    if(cal_diff_flag)
    {
        TRACE(0, "capsensor_cal_diff cal_count = %d", cal_count);
        capsensor_cal_diff(data, len);
        if(cal_count >= 9)
        {
            cal_count = 0;
        }
        else
        {
            cal_count++;
        }
    }
}
#endif