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

#include "plat_addr_map.h"

#if defined(AON_CAP_BASE) && defined(CAPSENSOR_ENABLE)

#include "hal_trace.h"
#include "cmsis.h"
#include "hal_timer.h"
#include CHIP_SPECIFIC_HDR(hal_capsensor)
#include CHIP_SPECIFIC_HDR(reg_capsensor)
#include CHIP_SPECIFIC_HDR(hal_cmu)

#define SDM_2PF 65535

static struct CAPSENSOR_T* capsensor_base = (struct CAPSENSOR_T*)AON_CAP_BASE;
void capsensor_setup_cfg(struct CAPSENSOR_CFG_T * cap_cfg) __attribute__((alias("hal_capsensor_setup")));
int capsensor_get_raw_data(struct capsensor_sample_data *sample, int num) __attribute__((alias("hal_capsensor_get_raw_data")));
void hal_capsensor_sdm_calib(struct capsensor_sample_data *capsensor_sample_buff, uint8_t cap_num);
static int sdm_init_flag = 1;

static uint32_t hal_capsensor_baseline_reload(uint8_t ch_val, uint32_t value, uint32_t baseline_val)
{
    uint8_t ch;
    ch = ch_val % 3;
    if (ch == 0) {
        value &= 0xFFFFFC00;
        value |= baseline_val;
    } else if (ch == 1) {
        value &= 0xFFF003FF;
        value |= baseline_val<<10;
    } else if (ch == 2) {
        value &= 0XC00FFFFF;
        value |= baseline_val<<20;;
    }
    return value;
}

void hal_capsensor_sw_control_baseline_setup(uint8_t ch, uint32_t * baseline_val)
{
    uint32_t value;

    for(uint8_t i=0; i<8; i++) {
        if((ch>>i) & 0x1) {
            if (i < 3) {
                value = capsensor_base->REG_28;
                value = hal_capsensor_baseline_reload(i, value, baseline_val[i]);
                capsensor_base->REG_1C = value;
            } else if (i > 2 && i < 6) {
                value = capsensor_base->REG_2C;
                value = hal_capsensor_baseline_reload(i, value, baseline_val[i]);
                capsensor_base->REG_20 = value;
            } else if (i > 5) {
                value = capsensor_base->REG_30;
                value = hal_capsensor_baseline_reload(i, value, baseline_val[i]);
                capsensor_base->REG_24 = value;
            }
        }
    }
}

void hal_capsensor_sw_control_baseline_en(void)
{
    uint32_t value;
    value = capsensor_base->REG_08; //cdc_baseline_p_bit_in_dr
    value |= CAP_CDC_BASELINE_P_BIT_IN_DR;
    value |= CAP_CDC_BASELINE_N_BIT_IN_DR;
    capsensor_base->REG_08 = value;
}

void hal_capsensor_sw_control_sar_en(void)
{
    uint32_t value;
    value = capsensor_base->REG_08; //cdc_baseline_p_bit_in_dr
    value |= CAP_CDC_SAR_P_BIT_IN_DR;
    capsensor_base->REG_08 = value;
}

void hal_capsensor_baseline_sar_reg_read(uint32_t* sar_value, uint32_t* baseline_value_p, uint32_t* baseline_value_n)
{
    uint32_t value = 0;
    uint16_t baseline_val[9] = {0};

    for(uint8_t i = 0; i < 3; i++) {
        value = *(&capsensor_base->REG_28 + i);
        baseline_val[i*3]   = value&0x3ff;         //baseline_data[0]: ch(0, 1, 2) baseline
        baseline_val[i*3+1] = (value>>10)&0x3ff;   //baseline_data[1]: ch(3, 4, 5) baseline
        baseline_val[i*3+2] = (value>>20)&0x3ff;   //baseline_data[2]: ch(6, 7, 8) baseline
        TRACE_IMM(0,"baseline[%d]:%x, baseline[%d]:%x, baseline[%d]:%x", i*3, baseline_val[i*3], i*3+1, baseline_val[i*3+1], i*3+2, baseline_val[i*3+2]);
    }

    baseline_value_n[0] = baseline_val[3] << 24 | baseline_val[2] << 16 | baseline_val[1] << 8 | baseline_val[0];
    baseline_value_n[1] = baseline_val[7] << 24 | baseline_val[6] << 16 | baseline_val[5] << 8 | baseline_val[4];
    baseline_value_p[0] = capsensor_base->REG_28;
    baseline_value_p[1] = capsensor_base->REG_2C;
    baseline_value_p[2] = capsensor_base->REG_30;
    sar_value[0] = capsensor_base->REG_44;
    sar_value[1] = capsensor_base->REG_48;
    sar_value[2] = capsensor_base->REG_4C;
    // TRACE(0, "baseline_value_n:%d", *baseline_value_n);
    // TRACE(0, "baseline_value[0]:%d", baseline_value[0]);
    // TRACE(0, "baseline_value[1]:%d", baseline_value[1]);
    // TRACE(0, "baseline_value[2]:%d", baseline_value[2]);
    // TRACE(0, "sar_value[0]:%d", sar_value[0]);
    // TRACE(0, "sar_value[1]:%d", sar_value[1]);
    // TRACE(0, "sar_value[2]:%d", sar_value[2]);
}

void hal_capsensor_baseline_sar_dr(uint32_t* sar_value, uint32_t* baseline_value_p, uint32_t* baseline_value_n)
{
    capsensor_base->REG_34 = baseline_value_n[0];
    capsensor_base->REG_38 = baseline_value_n[1];
    capsensor_base->REG_1C = baseline_value_p[0];
    capsensor_base->REG_20 = baseline_value_p[1];
    capsensor_base->REG_24 = baseline_value_p[2];
    capsensor_base->REG_44 = sar_value[0];
    capsensor_base->REG_48 = sar_value[1];
    capsensor_base->REG_4C = sar_value[2];
    hal_capsensor_sw_control_baseline_en();
    hal_capsensor_sw_control_sar_en();
}

static float formula_convert_sar_vtoc(uint32_t sar_val)
{
    float sar_value = 0;

    sar_value += ((sar_val>>9&0x1)*4.0);
    sar_value += ((sar_val>>8&0x1)*2.0);
    sar_value += ((sar_val>>7&0x1));
    sar_value += ((sar_val>>6&0x1));
    sar_value += ((sar_val>>5&0x1)/2.0);
    sar_value += ((sar_val>>4&0x1)/4.0);
    sar_value += ((sar_val>>3&0x1)/8.0);
    sar_value += ((sar_val>>2&0x1)/8.0);
    sar_value += ((sar_val>>1&0x1)/16.0);
    sar_value += ((sar_val>>0&0x1)/32.0);
    return sar_value;
}

void count_sar_value(struct capsensor_sample_data *sample_value)
{
    float sar_value = 0;

    sar_value = formula_convert_sar_vtoc(sample_value->sar);
    sample_value->sar_int = (int)sar_value;
    sample_value->sar_float = (int)((sar_value-(float)(sample_value->sar_int))*1000000);
}

void count_sdm_value(struct capsensor_sample_data *sample_value)
{
    float sdm_value = 0;

    sdm_value += ((sample_value->sdm)/131071.0*4);  //131071*4
    sample_value->sdm_int = (int)sdm_value;
    sample_value->sdm_float = (int)((sdm_value-(float)(sample_value->sdm_int))*1000000);
}

void count_sar_sdm_value(struct capsensor_sample_data *sample_value)
{
    float sar_value = 0;
    float sdm_value = 0;

    sar_value = formula_convert_sar_vtoc(sample_value->sar);
    sdm_value += ((sample_value->sdm)/131071.0*4);  //131071*4
    sample_value->sar_sdm_int = (int)(sar_value + sdm_value);
    sample_value->sar_sdm_float = (int)((sar_value + sdm_value-(float)(sample_value->sar_sdm_int))*1000000);
}

void calculate_sample_data(struct capsensor_sample_data *sample, uint8_t cap_num) //calculate V->C
{
    for(int data = 0; data < cap_num; data++)
    {
        count_sar_value(&sample[data]);
        count_sdm_value(&sample[data]);
        count_sar_sdm_value(&sample[data]);
    }
}

void hal_capsensor_fp_mode_clear_irq(void)
{
    capsensor_base->REG_100 |= CAP_FP_MODE_RD_INT_CLR_REG;
}

int get_sample(struct capsensor_sample_data *sample, int i)
{
    uint32_t value = 0;

    capsensor_base->REG_BC = CAP_FIFO_REG_READ;
    value = capsensor_base->REG_7C;  //ch  [2:0] bit,sar [30:20] bit,sdm [19:3] bit
    sample[i].ch = value&(~0xfffffff8);   //ch  [2:0]   bit
    sample[i].sar = (value>>20)&0x3ff;    //sar [30:20] bit
    sample[i].sdm = (value>>3)&0x1ffff;   //sdm [19:3]  bit

    return 0;
}

void hal_capsensor_stop_hw_wr_start_sw_rd(void)
{
    uint32_t value = 0;
    value = CAP_FIFO_CLK_DR | CAP_FIFO_RD_DEBUG_REG | CAP_FIFO_RD_START_REG; //0X45
    capsensor_base->REG_A4 = value;
    hal_sys_timer_delay(US_TO_TICKS(100));
}

void hal_capsensor_stop_sw_rd_start_hw_wr(void)
{
    uint32_t value = 0;
    value = capsensor_base->REG_110;
    value |= CAP_WR_PTR_DR;  //dr read_ptr
    capsensor_base->REG_11C = value;
    value &= ~CAP_WR_PTR_DR; //reset read_ptr
    capsensor_base->REG_11C = value;

    value = capsensor_base->REG_A4;
    value &= ~(CAP_FIFO_CLK_DR | CAP_FIFO_RD_DEBUG_REG | CAP_FIFO_RD_START_REG | CAP_FIFO_RD_DIRECTION_REG);
    capsensor_base->REG_A4 = value;
}

int hal_capsensor_irq_type_judge(void)
{
    uint32_t value = 0;

    value = capsensor_base->REG_100;
    if((value&CAP_FP_MODE_RD_INT) == CAP_FP_MODE_RD_INT) { //FP_MODE_RD
        return 0;
    } else {
        return -1;
    }
}

void select_sort_sample_data(struct capsensor_sample_data *sample, int chnum)
{
    uint32_t swap;
    for (int i = 0; i < chnum; i++)
    {
        for(int j = i+1 ; j < chnum; j++)
        {
            if(sample[j].ch < sample[i].ch)
            {
                swap = sample[i].ch;
                sample[i].ch = sample[j].ch;
                sample[j].ch = swap;

                swap = sample[i].sar;
                sample[i].sar = sample[j].sar;
                sample[j].sar = swap;

                swap = sample[i].sdm;
                sample[i].sdm = sample[j].sdm;
                sample[j].sdm = swap;
            }
        }
    }
}

void capsensor_baseline_read(void)
{
    uint32_t value = 0;
    uint16_t baseline_val[9] = {0};

    for(uint8_t i = 0; i < 3; i++) {
        value = *(&capsensor_base->REG_28 + i);
        baseline_val[i*3]   = value&0x3ff;         //baseline_data[0]: ch(0, 1, 2) baseline
        baseline_val[i*3+1] = (value>>10)&0x3ff;   //baseline_data[1]: ch(3, 4, 5) baseline
        baseline_val[i*3+2] = (value>>20)&0x3ff;   //baseline_data[2]: ch(6, 7, 8) baseline
        TRACE_IMM(0,"baseline[%d]:%x, baseline[%d]:%x, baseline[%d]:%x", i*3, baseline_val[i*3], i*3+1, baseline_val[i*3+1], i*3+2, baseline_val[i*3+2]);
    }
}

int hal_capsensor_get_raw_data(struct capsensor_sample_data *sample, int num)
{
    int ret = 0;

    ret = hal_capsensor_irq_type_judge();

    if(!ret) {
        hal_capsensor_fp_mode_clear_irq();         //clear irq
        hal_capsensor_stop_hw_wr_start_sw_rd();    //read en
        for(int i=0; i<num; i++) {
            get_sample(sample, i);                 //read data
        }
        hal_capsensor_stop_sw_rd_start_hw_wr();    //read disen

        select_sort_sample_data(sample, num);
    }
#if 0
    capsensor_baseline_read();
    calculate_sample_data(sample, num);

    for (int k = 0; k < num; k++) {
        TRACE_IMM(0 ,"cap_data:%d\t%x\t%d.%06d\t%x\t%d.%06d\t%d.%06d", sample[k].ch, sample[k].sar,
            sample[k].sar_int, sample[k].sar_float,
            sample[k].sdm, sample[k].sdm_int, sample[k].sdm_float,
            sample[k].sar_sdm_int, sample[k].sar_sdm_float);
    }
#endif

    if (sdm_init_flag) {
        hal_capsensor_sdm_calib(sample, num);
        sdm_init_flag = 0;
    }
    return 0;
}

static void hal_capsensor_extend_sdm_overflow_sar(void)
{
    uint32_t value = 0;
    value = capsensor_base->REG_6C;
    value = SET_BITFIELD(value, CAP_CDC_CAP_PLUS_SEL_CH0_REG, 0x1);
    value = SET_BITFIELD(value, CAP_CDC_CAP_PLUS_SEL_CH1_REG, 0x1);
    value = SET_BITFIELD(value, CAP_CDC_CAP_PLUS_SEL_CH2_REG, 0x1);
    value = SET_BITFIELD(value, CAP_CDC_CAP_PLUS_SEL_CH3_REG, 0x1);
    value = SET_BITFIELD(value, CAP_CDC_CAP_PLUS_SEL_CH4_REG, 0x1);
    value = SET_BITFIELD(value, CAP_CDC_CAP_PLUS_SEL_CH5_REG, 0x1);
    value = SET_BITFIELD(value, CAP_CDC_CAP_PLUS_SEL_CH6_REG, 0x1);
    value = SET_BITFIELD(value, CAP_CDC_CAP_PLUS_SEL_CH7_REG, 0x1);
    value |= CAP_CDC_C2X_CH0_REG | CAP_CDC_C2X_CH1_REG | CAP_CDC_C2X_CH2_REG | CAP_CDC_C2X_CH3_REG |
        CAP_CDC_C2X_CH4_REG | CAP_CDC_C2X_CH5_REG | CAP_CDC_C2X_CH6_REG | CAP_CDC_C2X_CH7_REG;
    capsensor_base->REG_6C = value;

    value = capsensor_base->REG_98;
    value = SET_BITFIELD(value, CAP_RSVED_CDC, 0xcccc);
    value |= CAP_SAR_OVERFLOW_DR | CAP_FIFO_SAR_DATA_SEL;   //sar_overflow_dr
    value &= ~CAP_SAR_OVERFLOW_REG;
    capsensor_base->REG_98 = value;

    value = capsensor_base->REG_70;
    value = SET_BITFIELD(value, CAP_CDC_SDM_OP_CIN1_CH0_REG, 0x1); //cin
    value = SET_BITFIELD(value, CAP_CDC_SDM_OP_CIN1_CH1_REG, 0x1);
    value = SET_BITFIELD(value, CAP_CDC_SDM_OP_CIN1_CH2_REG, 0x1);
    value = SET_BITFIELD(value, CAP_CDC_SDM_OP_CIN1_CH3_REG, 0x1);
    value = SET_BITFIELD(value, CAP_CDC_SDM_OP_CIN1_CH4_REG, 0x1);
    value = SET_BITFIELD(value, CAP_CDC_SDM_OP_CIN1_CH5_REG, 0x1);
    value = SET_BITFIELD(value, CAP_CDC_SDM_OP_CIN1_CH6_REG, 0x1);
    value = SET_BITFIELD(value, CAP_CDC_SDM_OP_CIN1_CH7_REG, 0x1);
    capsensor_base->REG_70 = value;
}

static void hal_capsensor_irq_config(uint8_t chan_num, uint8_t rep_num, int16_t samp_fs, uint8_t cap_ch_map)
{
    uint32_t value = 0;
    uint16_t sleep_time = 0;
    uint8_t ch_map[8]={0};

    value = CAP_PU_OSC_INT_RAW | CAP_PU_OSC_INT_MASK | CAP_PD_OSC_INT_RAW | CAP_PD_OSC_INT_MASK |
        CAP_CAP_SENSOR_PRESS_INT_MASK | CAP_FP_MODE_RD_INT_RAW_EN_REG | CAP_SAR_OVERFLOW_INT_MASK;   //90466:fp_mode_rd, 94066:cap_sensor_press
    capsensor_base->REG_100 = value;

    sleep_time = samp_fs*32;
    value = capsensor_base->REG_68; //trigger irq
    value = SET_BITFIELD(value, CAP_FIFO_DATA_NUM_TH, (chan_num+1)*rep_num);   //fifo_data_num
    value = SET_BITFIELD(value, CAP_LP_SLEEP_TIME, sleep_time);    //sleep time 10ms 0x14D  , 20ms 0x29A, 50ms 0x681
    capsensor_base->REG_68 = value;

    for(uint8_t i=0,j=0; i<8; i++) {
        if(cap_ch_map & (1<<i)) {
            ch_map[j++] = i;
            if(j >= (chan_num+1))
                break;
        }
    }

    capsensor_base->REG_A8 = SET_BITFIELD(capsensor_base->REG_A8, CAP_CH_CONFIG_IDX_0, ch_map[0]);
    capsensor_base->REG_A8 = SET_BITFIELD(capsensor_base->REG_A8, CAP_CH_CONFIG_IDX_1, ch_map[1]);
    capsensor_base->REG_A8 = SET_BITFIELD(capsensor_base->REG_A8, CAP_CH_CONFIG_IDX_2, ch_map[2]);
    capsensor_base->REG_A8 = SET_BITFIELD(capsensor_base->REG_A8, CAP_CH_CONFIG_IDX_3, ch_map[3]);
    capsensor_base->REG_AC = SET_BITFIELD(capsensor_base->REG_AC, CAP_CH_CONFIG_IDX_4, ch_map[4]);
    capsensor_base->REG_AC = SET_BITFIELD(capsensor_base->REG_AC, CAP_CH_CONFIG_IDX_5, ch_map[5]);
    capsensor_base->REG_AC = SET_BITFIELD(capsensor_base->REG_AC, CAP_CH_CONFIG_IDX_6, ch_map[6]);
    capsensor_base->REG_AC = SET_BITFIELD(capsensor_base->REG_AC, CAP_CH_CONFIG_IDX_7, ch_map[7]);
    // value = capsensor_base->REG_FC; //trigger irq
    // value = SET_BITFIELD(value, CAP_CLK_SETTLE_TIME, 0x1);
    // capsensor_base->REG_FC = value;
    value = capsensor_base->REG_FC;
    value &= ~CAP_PU_OSC_CAP_SENSOR_DR;
    capsensor_base->REG_FC = value;
}

static const uint16_t capsensor_cin_value_table[][3] =
{
    {0,  51,  0},   //0PF   < x <= 10PF,  10.125PF => 51
    {51, 320, 1},   //10PF  < x <= 60PF,  60PF => 320
    {320,691, 3},   //60PF  < x <= 100PF, 100.125PF => 691
    {691,800, 5},   //100PF < x <= 120PF, 120PF => 800
    {800,1023,7},   //120PF < x <= 160PF, 164.625PF =>1023
};

static void hal_capsensor_cap_ch_cin_write(void)
{
    uint32_t value = 0;
    uint16_t baseline_val[9] = {0};

    for(uint8_t i = 0; i < 3; i++) {
        value = *(&capsensor_base->REG_28 + i);
        baseline_val[i*3]   = value&0x3ff;         //baseline_data[0]: ch(0, 1, 2) baseline
        baseline_val[i*3+1] = (value>>10)&0x3ff;   //baseline_data[1]: ch(3, 4, 5) baseline
        baseline_val[i*3+2] = (value>>20)&0x3ff;   //baseline_data[2]: ch(6, 7, 8) baseline
        // TRACE(0,"baseline[%d]:%x, baseline[%d]:%x, baseline[%d]:%x", i*3, baseline_val[i*3], i*3+1, baseline_val[i*3+1], i*3+2, baseline_val[i*3+2]);
    }

    const uint16_t (*capsensor_cin_value_table_p)[3] = NULL;
    uint32_t tbl_size = 0;
    capsensor_cin_value_table_p = &capsensor_cin_value_table[0];
    tbl_size = sizeof(capsensor_cin_value_table)/sizeof(capsensor_cin_value_table[0]);

    for (uint8_t j = 0; j < tbl_size; j++) {
        if(baseline_val[0] <= capsensor_cin_value_table_p[j][1]) {
            capsensor_base->REG_70 = SET_BITFIELD(capsensor_base->REG_70, CAP_CDC_SDM_OP_CIN1_CH0_REG, capsensor_cin_value_table_p[j][2]);
            break;
        }
    }

    for (uint8_t j = 0; j < tbl_size; j++) {
        if(baseline_val[1] <= capsensor_cin_value_table_p[j][1]) {
            capsensor_base->REG_70 = SET_BITFIELD(capsensor_base->REG_70, CAP_CDC_SDM_OP_CIN1_CH1_REG, capsensor_cin_value_table_p[j][2]);
            break;
        }
    }

    for (uint8_t j = 0; j < tbl_size; j++) {
        if(baseline_val[2] <= capsensor_cin_value_table_p[j][1]) {
            capsensor_base->REG_70 = SET_BITFIELD(capsensor_base->REG_70, CAP_CDC_SDM_OP_CIN1_CH2_REG, capsensor_cin_value_table_p[j][2]);
            break;
        }
    }

    for (uint8_t j = 0; j < tbl_size; j++) {
        if(baseline_val[3] <= capsensor_cin_value_table_p[j][1]) {
            capsensor_base->REG_70 = SET_BITFIELD(capsensor_base->REG_70, CAP_CDC_SDM_OP_CIN1_CH3_REG, capsensor_cin_value_table_p[j][2]);
            break;
        }
    }

    for (uint8_t j = 0; j < tbl_size; j++) {
        if(baseline_val[4] <= capsensor_cin_value_table_p[j][1]) {
            capsensor_base->REG_70 = SET_BITFIELD(capsensor_base->REG_70, CAP_CDC_SDM_OP_CIN1_CH4_REG, capsensor_cin_value_table_p[j][2]);
            break;
        }
    }

    for (uint8_t j = 0; j < tbl_size; j++) {
        if(baseline_val[5] <= capsensor_cin_value_table_p[j][1]) {
            capsensor_base->REG_70 = SET_BITFIELD(capsensor_base->REG_70, CAP_CDC_SDM_OP_CIN1_CH5_REG, capsensor_cin_value_table_p[j][2]);
            break;
        }
    }

    for (uint8_t j = 0; j < tbl_size; j++) {
        if(baseline_val[6] <= capsensor_cin_value_table_p[j][1]) {
            capsensor_base->REG_70 = SET_BITFIELD(capsensor_base->REG_70, CAP_CDC_SDM_OP_CIN1_CH6_REG, capsensor_cin_value_table_p[j][2]);
            break;
        }
    }

    for (uint8_t j = 0; j < tbl_size; j++) {
        if(baseline_val[7] <= capsensor_cin_value_table_p[j][1]) {
            capsensor_base->REG_70 = SET_BITFIELD(capsensor_base->REG_70, CAP_CDC_SDM_OP_CIN1_CH7_REG, capsensor_cin_value_table_p[j][2]);
            break;
        }
    }
}

static void hal_capsensor_detected_cfg(uint8_t cap_chnum)
{
    uint32_t value = 0;

    value = capsensor_base->REG_80; //down_sel 96
    value = SET_BITFIELD(value, CAP_DOWN_SEL, 0x1);
    value |= CAP_ADC_HBF1_BYPASS;
    value &= ~CAP_ADC_HBF2_BYPASS;
    capsensor_base->REG_80 = value;

    value = CAP_CDC_PU_VREF0P7_REG | CAP_CDC_PU_LDO_REG;
    value = SET_BITFIELD(value, CAP_ACTIVE_CH_NUM, cap_chnum);
    value = SET_BITFIELD(value, CAP_OPERATION_MODE, 0x2);
    value |= CAP_POWER_MODE_DR | CAP_CDC_RSTB;
    value = SET_BITFIELD(value, CAP_CDC_DLATCH_IBSEL, 0xf);
    capsensor_base->REG_04 = value;
    hal_sys_timer_delay(MS_TO_TICKS(5));
    value |= CAP_FSM_EN;
    value |= CAP_CDC_SAR_P_MODE_DR;
    value &= ~CAP_CDC_SAR_P_MODE_REG;
    capsensor_base->REG_04 = value;

    hal_sys_timer_delay(US_TO_TICKS(18*(cap_chnum+1))); // one ch:1/571K * 10 = 17.5us
    hal_capsensor_cap_ch_cin_write();
    capsensor_base->REG_04 &= ~CAP_CDC_SAR_P_MODE_DR;
}

static uint32_t formula_convert_sar(uint32_t sar_val)
{
    uint32_t sar_value = 0;
    sar_value += ((sar_val>>9&0x1)*128);
    sar_value += ((sar_val>>8&0x1)*64);
    if (((sar_val>>7&0x1) || (sar_val>>6&0x1)) == 1) //bit 6、7
    {
        sar_value += 32;
    }
    sar_value += ((sar_val>>5&0x1)*16);
    sar_value += ((sar_val>>4&0x1)*8);
    if (((sar_val>>3&0x1) || (sar_val>>2&0x1)) == 1) //bit 2、3
    {
        sar_value += 4;
    }
    sar_value += ((sar_val>>1&0x1)*2);
    sar_value += ((sar_val>>0&0x1)*1);
    return sar_value;
}

static uint32_t convert_recovery_sar(uint32_t sar_reg, uint32_t sar_val)
{
    uint32_t sar_value = 0;
    sar_value += ((sar_val>>7&0x1)*512);
    sar_value += ((sar_val>>6&0x1)*256);
    sar_value += ((sar_val>>5&0x1)*128);
    if (((sar_reg>>7&0x1) && (sar_reg>>6&0x1)) == 1) { //bit 6、7
        sar_value += 64;
    }
    sar_value += ((sar_val>>4&0x1)*32);
    sar_value += ((sar_val>>3&0x1)*16);
    sar_value += ((sar_val>>2&0x1)*8);
    if (((sar_reg>>3&0x1) && (sar_reg>>2&0x1)) == 1) { //bit 2、3
        sar_value += 4;
    }
    sar_value += ((sar_val>>1&0x1)*2);
    sar_value += ((sar_val>>0&0x1)*1);
    return sar_value;
}

void capsensor_sar_calib(uint8_t index, uint32_t sdm_val, uint16_t sar_val)
{
    uint8_t ch;
    uint32_t value = 0;
    uint32_t sar_value_before = 0;
    uint32_t sar_value_after = 0;
    uint32_t sdm_detla = 0;
    uint32_t step = 0;

    TRACE(0, "sdm_val[%d]:%d", index, sdm_val);
    if (sdm_val > SDM_2PF) {
        sdm_detla = sdm_val - SDM_2PF;
    } else {
        sdm_detla = SDM_2PF - sdm_val;
    }

    step = sdm_detla / 32; //sar step 1/32pf,  transform to ff
    TRACE(0, "sdm_detla:%d, step:%d uf", sdm_detla, step);
    sar_value_before = formula_convert_sar(sar_val);
    TRACE(0, "sar_value_before1:%d", sar_value_before);
    if (sdm_val > SDM_2PF) {
        sar_value_before += (step *31 /1000);
        TRACE(0, "sar_value_before2:%d", sar_value_before);
    } else {
        sar_value_before -= (step *31 /1000);
        TRACE(0, "sar_value_before3:%d", sar_value_before);
    }
    sar_value_after = convert_recovery_sar(sar_val, sar_value_before);
    TRACE(0, "sar_value_after:%d", sar_value_after);

    ch = index % 3;
    value = *(&capsensor_base->REG_44 + index/3);
    if (ch == 0) {
        value &= 0xFFFFFC00;
        value |= sar_value_after;
    } else if (ch == 1) {
        value &= 0xFFF003FF;
        value |= sar_value_after<<10;
    } else if (ch == 2) {
        value &= 0XC00FFFFF;
        value |= sar_value_after<<20;
    }
    *(&capsensor_base->REG_44 + index/3) = value;

    sar_value_before = 0;
    sar_value_after = 0;
}

void hal_capsensor_sdm_calib(struct capsensor_sample_data *capsensor_sample_buff, uint8_t cap_num)
{
    uint32_t value = 0;
    uint16_t capsense_sar[9] = {0};

    for(uint8_t i = 0; i < 3; i++) {
        value = *(&capsensor_base->REG_50 + i);
		capsense_sar[i*3] = value&0x3FF;
        capsense_sar[i*3+1] = (value>>10)&0x3ff;
        capsense_sar[i*3+2] = (value>>20)&0x3ff;
        TRACE_IMM(0,"sar[%d]:%x, sar[%d]:%x,sar[%d]:%x",i*3, capsense_sar[i*3], i*3+1, capsense_sar[i*3+1], i*3+2, capsense_sar[i*3+2]);
        *(&capsensor_base->REG_44 + i) = value;
	}
    capsensor_base->REG_08 |= CAP_CDC_SAR_P_BIT_IN_DR; //cdc_sar_p_bit_in_dr

    for(uint8_t j = 0; j < cap_num; j++) {
        capsensor_sar_calib(j, capsensor_sample_buff[j].sdm, capsense_sar[j]);
    }
}

void hal_capsensor_setup(struct CAPSENSOR_CFG_T * cap_cfg)
{
    hal_capsensor_extend_sdm_overflow_sar();
    hal_cmu_capsensor_clk_cfg();

    hal_capsensor_irq_config(cap_cfg->ch_num, cap_cfg->conversion_num, cap_cfg->samp_fs, cap_cfg->ch_map);
    hal_capsensor_detected_cfg(cap_cfg->ch_num);
}

void hal_capsensor_suspend_pu_osc_dr(void)
{
    capsensor_base->REG_FC |= CAP_PU_OSC_CAP_SENSOR_DR;
    capsensor_base->REG_FC &= ~CAP_PU_OSC_CAP_SENSOR_REG;
}

void hal_capsensor_start_pu_osc_reg(void)
{
    capsensor_base->REG_FC |= CAP_PU_OSC_CAP_SENSOR_REG;
    capsensor_base->REG_FC &= ~CAP_PU_OSC_CAP_SENSOR_DR;
}

void hal_capsensor_fp_mode_set_mask(void)
{
    capsensor_base->REG_100 |= CAP_FP_MODE_RD_INT_MASK;
}

void hal_capsensor_fp_mode_clr_mask(void)
{
    capsensor_base->REG_100 &= ~(CAP_FP_MODE_RD_INT_MASK);
}

void hal_capsensor_irq_enable(void)
{
}

void hal_capsensor_irq_disable(void)
{
}

void hal_capsensor_set_sdm_init_flag(bool m_sdm_init_flag)
{
    sdm_init_flag = m_sdm_init_flag;
}

int hal_capsensor_judge_machine_state(void) /*return 1:fail , return 0:success*/
{
    uint8_t value = 0;

    value = capsensor_base->REG_F8 & 0xff;
    if (!(value & 0x55)) {
        return 1;
    }
    return 0;
}

#endif

