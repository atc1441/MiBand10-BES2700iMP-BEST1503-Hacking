/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "app_trace_rx.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "app_ble_usb_latency_test.h"

#ifdef AOB_LATENCY_TEST_MODE
#define LC3_10_MS_AUDIO_PLAYBACK_TIME     (600)   //

typedef struct
{
    uint32_t send_data_counter;
    uint8_t send_data_idx;
}Usb_audio_latency_test_ctx_t;

// Encoder size:     25600
// Scratch size:     0
// Sample rate:      48000
// Channels:         2
// Frame samples:     480
// Frame length:     80
// Output format:    16 bits
// Output format:    16 bits align
// Target bitrate:   64000
static uint8_t lc3_mute_data[80] = {0,};

static uint8_t lc3_10_ms_48k_2ch_16_bit_data[80] = {
    0x84, 0xd8, 0x1c, 0x65, 0x75, 0x6f, 0x9b, 0xb4, 0x4f, 0xd7, 0x85, 0xed, 0x56, 0xd8, 0x83, 0x6e,
    0xc8, 0x01, 0x0d, 0x40, 0x45, 0x39, 0x1f, 0x0d, 0xd7, 0x63, 0xa3, 0xa8, 0x10, 0xc6, 0xc3, 0x86,
    0xac, 0x85, 0xfb, 0x85, 0x1b, 0x5c, 0x20, 0xdc, 0xb0, 0xc4, 0x8e, 0xe3, 0x34, 0x39, 0xde, 0x93,
    0xd2, 0xcd, 0xa2, 0x5d, 0xa2, 0x85, 0xd1, 0x1e, 0xf0, 0x27, 0x50, 0x03, 0xaa, 0x71, 0xb9, 0x9b,
    0x0e, 0xdc, 0x50, 0xb8, 0x72, 0xc6, 0xc3, 0x86, 0xac, 0x85, 0xfb, 0x85, 0x1b, 0x5c, 0x20, 0xdc,
};

// Encoder size:     25600
// Scratch size:     0
// Sample rate:      48000
// Channels:         2
// Frame samples:    240
// Frame length:     80
// Output format:    16 bits
// Output format:    16 bits align
// Target bitrate:   128000
static uint8_t lc3_5ms_48k_2ch_16_bit_data[80] = {
   0x00, 0x88, 0xbe, 0x69, 0x19, 0x38, 0x16, 0xa3, 0x50, 0x84, 0x46, 0x76, 0xbf, 0x68, 0x1b, 0x96, 
   0x6b, 0xaa, 0x62, 0x79, 0x48, 0x94, 0x3f, 0xe6, 0x58, 0xbe, 0x09, 0x6e, 0x67, 0x9d, 0xd1, 0x86, 
   0x76, 0x42, 0xfd, 0xc2, 0x8d, 0xa5, 0xa0, 0xa4, 0x00, 0x88, 0xbe, 0x69, 0x19, 0x38, 0x16, 0xa3, 
   0x50, 0x84, 0x46, 0x76, 0xbf, 0x68, 0x1b, 0x96, 0x6b, 0x2a, 0x62, 0x79, 0x48, 0x94, 0x3f, 0xe6, 
   0x58, 0xbe, 0x09, 0x6e, 0x67, 0x9d, 0x91, 0xc6, 0x76, 0x42, 0xfd, 0xc2, 0x8d, 0xa5, 0xa0, 0xa4, 
};

static Usb_audio_latency_test_ctx_t latency_test_ctx;

void app_ble_usb_latency_test_reset()
{
    TRACE(0,"%s", __func__);
    latency_test_ctx.send_data_counter = 1;
    latency_test_ctx.send_data_idx = 0;
}

uint8_t* app_ble_usb_latency_get_lc3_payload(float dur_frame)
{
    if (latency_test_ctx.send_data_counter % LC3_10_MS_AUDIO_PLAYBACK_TIME == 0)
    {
        latency_test_ctx.send_data_idx = !latency_test_ctx.send_data_idx;
    }
    latency_test_ctx.send_data_counter++;

    if (latency_test_ctx.send_data_idx == 0)
    {
        return lc3_mute_data;
    }
  
    if ((uint32_t)dur_frame == 10)
    {
       return lc3_10_ms_48k_2ch_16_bit_data;
    }
    else if ((uint32_t)dur_frame == 5)
    {
        return lc3_5ms_48k_2ch_16_bit_data;
    }

    ASSERT(0,"Latency Test,not support this frame_dur now = %u",(uint32_t)(dur_frame * 1000));

    return NULL;
}

void app_ble_usb_latency_signal_init()
{
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_gpio[] =
    {
        {HAL_IOMUX_PIN_P0_4, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P3_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };

    hal_iomux_init(pinmux_gpio, ARRAY_SIZE(pinmux_gpio));
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P0_4, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(HAL_GPIO_PIN_P3_2, HAL_GPIO_DIR_OUT, 1);
}

void app_ble_usb_latency_send_data_signal()
{
    hal_gpio_pin_clr(HAL_GPIO_PIN_P0_4);
    hal_gpio_pin_set(HAL_GPIO_PIN_P0_4);
    hal_gpio_pin_clr(HAL_GPIO_PIN_P0_4);
}

void app_ble_usb_latency_playback_signal(void)
{
    hal_gpio_pin_clr(HAL_GPIO_PIN_P3_2);
    hal_gpio_pin_set(HAL_GPIO_PIN_P3_2);
    hal_gpio_pin_clr(HAL_GPIO_PIN_P3_2);
}

#endif /* AOB_LATENCY_TEST_MODE */
