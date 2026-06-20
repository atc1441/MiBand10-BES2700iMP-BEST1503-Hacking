/***************************************************************************
 *
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
 *
 ****************************************************************************/

#ifndef _CAPSENSOR_DRIVER_H_
#define _CAPSENSOR_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define WORKING_CH0             0
#define WORKING_CH1             1
#define WORKING_CH2             2
#define WORKING_CH3             3
#define WORKING_CH4             4
#define WORKING_CH5             5
#define WORKING_CH6             6
#define WORKING_CH7             7
#define WORKING_NULL            8

typedef struct
{
    int32_t  cap_keydown_th;
    int32_t  cap_keyup_th;
    uint8_t  slide_channel_0;
    uint8_t  slide_channel_1;
    uint8_t  slide_channel_2;
    uint16_t cap_key_click_max_duration;
    uint16_t cap_key_click_min_duration;
    uint16_t cap_key_double_interval;
    uint16_t cap_key_triple_interval;
    uint16_t cap_key_ultra_interval;
    uint16_t cap_key_rampage_interval;
    uint16_t cap_key_long_interval;
    uint16_t cap_key_longlong_interval;
    uint16_t cap_dc_reset_interval;
}TouchConfig;
extern const TouchConfig cap_touch_config;

typedef struct
{
    uint8_t wear_detect_channel_0;
    uint8_t wear_detect_channel_1;
    uint8_t wear_reference_channel_0;
    uint8_t wear_reference_channel_1;
    int32_t cap_wear_threshold_high1;
    int32_t cap_wear_threshold_high2;
    int32_t cap_wear_threshold_low1;
    int32_t cap_wear_threshold_low2;
    uint32_t cap_wear_twopad_threshold1;
    uint8_t cap_wear_twopad_state_win1;
    uint8_t cap_wear_ear_on_state_win;
    uint8_t cap_wear_ear_off_state_win;
    int16_t cap_wear_offset0_update_threshold;
    int16_t cap_wear_offset1_update_threshold;
    int32_t cap_wear_jump_threshold1;
    int32_t cap_wear_jump_threshold2;
    uint8_t cap_wear_on_delay_count;
    int16_t cap_wear_on_delay_mse_th;
    int16_t cap_wear_out_box_count;
    int16_t cap_wear_in_box_count;
    int16_t cap_wear_in_box_diff1;
    int16_t cap_wear_in_box_diff2;
    int32_t cap_wear_offset_max;
    int16_t cap_wear_offset0_update_std;
    int16_t cap_wear_offset1_update_std;
}WearConfig;
extern const WearConfig cap_wear_config;

enum CAP_KEY_EVENT_T {
    CAP_KEY_EVENT_NONE              = 0,
    CAP_KEY_EVENT_DOWN              = 1,
    CAP_KEY_EVENT_UP                = 4,
    CAP_KEY_EVENT_LONGPRESS         = 6,
    CAP_KEY_EVENT_LONGLONGPRESS     = 7,
    CAP_KEY_EVENT_CLICK             = 8,
    CAP_KEY_EVENT_DOUBLECLICK       = 9,
    CAP_KEY_EVENT_TRIPLECLICK       = 10,
    CAP_KEY_EVENT_ULTRACLICK        = 11,
    CAP_KEY_EVENT_RAMPAGECLICK      = 12,
    CAP_KEY_EVENT_UPSLIDE           = 21,
    CAP_KEY_EVENT_DOWNSLIDE         = 22,
    CAP_KEY_EVENT_ON_EAR            = 23,
    CAP_KEY_EVENT_OFF_EAR           = 24,

    CAP_KEY_EVENT_NUM,
};

enum CAP_EXTEND_KEY_EVENT_T{
    CAP_KEY_EVENT_EXTEND            = 40,
    CAP_KEY_EVENT_SIXCLICK ,
    CAP_KEY_EVENT_SEVENCLICK ,
    CAP_KEY_EVENT_CLICK_AND_LONGPRESS ,
    CAP_KEY_EVENT_CLICK_AND_LONGLONGPRESS ,
    CAP_KEY_EVENT_DOUBLECLICK_AND_LONGLONGPRESS ,
    CAP_KEY_EVENT_TRIPLECLICK_AND_LONGLONGPRESS ,
    CAP_KEY_EVENT_CLICK_AND_UPSLIDE ,
    CAP_KEY_EVENT_CLICK_AND_DOWNSLIDE ,
    CAP_KEY_EVENT_DOUBLECLICK_AND_UPSLIDE ,
    CAP_KEY_EVENT_DOUBLECLICK_AND_DOWNSLIDE ,
    CAP_KEY_EVENT_TRIPLECLICK_AND_UPSLIDE ,
    CAP_KEY_EVENT_TRIPLECLICK_AND_DOWNSLIDE ,
};

struct capsensor_sample_data {
    uint8_t ch;
    uint8_t value;
    uint32_t sar;
    uint32_t sar_int;
    uint32_t sar_float;
    uint32_t sdm;
    uint32_t sdm_int;
    uint32_t sdm_float;
    uint32_t sar_sdm_int;
    uint32_t sar_sdm_float;
};

struct CAPSENSOR_CFG_T{
    uint8_t ch_num;
    uint8_t conversion_num;   //[4:3] 00=1; 01=2; 10=4; 11=8;
    uint16_t samp_fs;         //0x1900 5Hz; 0xC80 10Hz; 0x140 100Hz; 0xA0 200Hz;
    uint8_t ch_map; //[2:0] for channel select
};

enum CAPSENSOR_READ_DATA_STATE_T {
    CAPSENSOR_READ_DATA_IDLE = 0,
    CAPSENSOR_READ_DATA_WAIT_READY,
    CAPSENSOR_READ_DATA_NOT_ENOUGH,
    CAPSENSOR_READ_DATA_READY,
    CAPSENSOR_READ_DATA_START,
    CAPSENSOR_READ_DATA_DONE,
};

void capsensor_drv_start(void);

int read_capsensor_fifo(struct capsensor_sample_data *sample);

uint8_t capsensor_ch_num_get(void);

uint8_t capsensor_conversion_num_get(void);

uint16_t capsensor_samp_fs_get(void);

uint8_t capsensor_ch_map_get(void);

void capsensor_driver_init(void);

void capsensor_gpio_init(void);

void capsensor_clk_calib(void);

void capsensor_sens2mcu_irq_set(void);

void capsensor_setup_cfg(struct CAPSENSOR_CFG_T * cap_cfg);

int capsensor_get_raw_data(struct capsensor_sample_data *sample, int num);

void capsensor_write_pointer_position(uint16_t * write_addr);

void capsensor_state_machine_reset(void);

void capsensor_reg_val_print(void);

#ifdef __cplusplus
}
#endif

#endif /* _CAPSENSOR_DRIVER_H_ */
