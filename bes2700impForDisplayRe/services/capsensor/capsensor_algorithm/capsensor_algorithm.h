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
#ifndef _CAPSENSOR_ALGORITHM_H_
#define _CAPSENSOR_ALGORITHM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "tgt_hardware_capsensor.h"
#include "cmsis.h"
#include "capsensor_driver.h"

/* CAPSENSOR_TRACE_DEBUG */
#ifdef CAPSENSOR_TRACE_DEBUG
#define CAPSENSOR_TRACE(n, s, ...)            TRACE(n, "CAPSENSOR" s, ##__VA_ARGS__)
#else
#define CAPSENSOR_TRACE(n, s, ...)            TRACE_DUMMY(n, s, ##__VA_ARGS__)
#endif

typedef struct CAP_KEY_STATUS {
    uint32_t code;
    uint8_t event;
} CAP_KEY_STATUS;

int calcuate_ch_adc_sum(struct capsensor_sample_data * data, int ch_num, uint32_t *ch_data_sum, int len);

// touch key states.
enum key_state
{
    KEY_STATE_NONE          = 0,
    KEY_STATE_PRESSED       = 1,
    KEY_STATE_WAIT_RELEASE  = 2
};

struct cap_state_t
{
    uint8_t pre_state;
    uint8_t cur_state;
};

/* ===================== Extern Function declaration ===================== */
/* dc calculate */
float dc_value(int xin, uint8_t index);

/* read key value */
uint8_t key_read(int xin, float dc, uint8_t *key, uint8_t channels, uint64_t * timer_start, uint64_t * timer_stop);

/* capsensor process function */
CAP_KEY_STATUS capsensor_touch_process(struct capsensor_sample_data * data, int ch_num,  uint32_t * chan_data_sum, int len,  uint64_t * timer_start, uint64_t * timer_stop);

void capsensor_set_pos_gap_thre(float m_pos_gap_thre);
void capsensor_set_ms_per_cnt(uint16_t m_ms_per_cnt);
void capsensor_touch_config_init(void);
void capsensor_get_touch_pos(int16_t* m_touch_pos);
void capsensor_set_touch_diffkr(float m_touch_diffKr0, float m_touch_diffKr1, float m_touch_diffKr2);

/* capsensor process function */
CAP_KEY_STATUS capsensor_wear_process(struct capsensor_sample_data * data, int ch_num,  uint32_t * chan_data_sum, int len);

void capsensor_wear_config_init(void);

void capsensor_set_offset(int m_offset0, int m_offset1);

void capsensor_get_app_para(int* m_offset0, int* m_offset1, int* m_offset0_cur, int* m_offset1_cur, int* m_ear_state);

void capsensor_get_max_offset(int* m_max_offset0, int* m_max_offset1);

void capsensor_get_box_offset(int* m_box_offset0, int* m_box_offset1);

void capsensor_get_diff(int* m_wear_diff0, int* m_wear_diff1, int* m_touch_diff0, int* m_touch_diff1, int* m_touch_diff2);

void capsensor_set_box_state(int m_in_box_state);

void capsensor_get_box_state(int* m_in_box_state);

void capsensor_set_calculate_flag(bool m_calculate_flag);

void capsensor_set_wear_diffkr(float m_wear_diffKr0, float m_wear_diffKr1);

void capsensor_set_on_ear_delay_flag(bool m_on_ear_delay_flag);

void capsensor_get_offset_learn_flag(int* m_offset0_learn_flag, int* m_offset1_learn_flag);

void capsensor_set_factory_offset(int m_factory_offset0, int m_factory_offset1);

void capsensor_set_learn_offset(int m_learn_offset0, int m_learn_offset1);

void capsensor_get_learn_offset(int* m_learn_offset0, int* m_learn_offset1);

void capsensor_get_ear_state(int* m_ear_state);

#ifdef __cplusplus
}
#endif

#endif /* _CAPSENSOR_ALGORITHM_H_ */
