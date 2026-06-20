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
#if defined (RX_IQ_CAL) || defined(DPD_ONCHIP_CAL)
#include "hal_trace.h"
#include "heap_api.h"
#include "bt_drv.h"
#include <math.h>
#include "bt_drv_iq_common.h"
#include "string.h"
#include "bt_drv_internal.h"

#define WRITE_REG(v,b,a) *(volatile uint32_t *)(b+a) = v

#define IQMIS_LPF_EN              1
#define IQMIS_MAX_DIN_LEN         2048
#define IQMIS_SEARCH_LEN          200
#define IQMIS_MIN_INT_SAMPLES     1648

#define IQMIS_GAIN_COM_MAX        0.06
#define IQMIS_PHI_COM_MAX         0.06


static uint16_t reversebit(uint16_t src, int bit)
{
    return src^(1<<bit);
}

void bt_drv_rx_iq_datasort(int *arry,int len)
{
    for (int i=0; i<len; i++)
    {
        arry[i] &= 0x0FFF;         //1> clear [15:12]
        if (getbit(arry[i], 11)) { //2> get bit11
            arry[i] |= (0xF << 12);
        }

        arry[i] = reversebit(arry[i], 15); //3>reverse bit15
        TRACE(2, "arry[%d]=0x%x", i, arry[i]);
    }

    //sort after convert
    for (int i=0; i<len-1; i++)
    {
        for (int j=0; j<len-1-i; j++)
        {
            if (arry[j] > arry[j + 1]){
                int temp = arry[j + 1];
                arry[j + 1] = arry[j];
                arry[j] = temp;
            }
        }
    }

    //revert
    for(int i=0; i<len; i++)
    {
        arry[i] &= 0x0FFF;
        TRACE(2, "after sort : arry[%d]=0x%x", i, arry[i]);
    }
}

void bt_drv_rx_iq_datawrite(int * gain_arr, int * phy_arr, int len)
{
    for (int i=0; i<len; i++)
    {
        TRACE(2, "fina: gain:0x%x phy: 0x%x", gain_arr[i], phy_arr[i]);
        uint32_t addr = 0xd0360000;
        int chs = 27;
        uint32_t iq_rx_val = 0;

        if (i == 0) {
            addr = 0xd0360000;
            chs = 27;
        } else if (i == 1) {
            addr = 0xd036006c;
            chs = 26;
        } else {
            addr = 0xd03600D4;
            chs = 26;
        }

        for (int ch=0; ch<chs; ch++)
        {
            uint32_t val_gain = (gain_arr[i] & 0x800) ? (gain_arr[i] | 0xf000) : gain_arr[i];
            uint32_t val_phy  = (phy_arr[i]  & 0x800) ? (phy_arr[i]  | 0xf000) : phy_arr[i];

            val_gain |= (val_phy << 16);
            iq_rx_val = val_gain;

            WRITE_REG(iq_rx_val, addr, 0);
            //TRACE(2, "addr:0x%x, val=0x%x", addr, iq_rx_val);
            addr += 4;
        }
    }
}

void lib_iqmis_getiq(short *data, int datalength_2X, float *din_i, float *din_q)
{
    int ii;
    int jj=0;

    for (ii = 0; ii < datalength_2X; ii += 2) // split IQ
    {
        din_i[jj] = (float)data[ii];
        din_q[jj] = (float)data[ii + 1];
        if (din_i[jj] >= 32768){
            din_i[jj] = din_i[jj] - 65536.0;
        }
        if (din_q[jj] >= 32768){
            din_q[jj] = din_q[jj] - 65536.0;
        }
        jj++;
    }
}

void lib_iqmis_esti(float *din_i, float *din_q, int din_length, int *gain_comp_reg, int *phi_comp_reg)
{

    int ii;
    int jj;

    float fir40[20] = {-0.000732421875,0.047119140625,-0.012451171875,-0.013916015625,-0.015380859375,-0.01708984375,-0.01806640625,-0.017333984375,-0.0146484375,-0.00927734375,-0.001220703125,0.009521484375,0.022705078125,0.037353515625,0.052734375,0.068115234375,0.08203125,0.093505859375,0.1015625,0.105712890625};
    float reg_i[40];
    float reg_q[40];
    float *dout_i = (float *)bt_drv_malloc(IQMIS_MAX_DIN_LEN * sizeof(float));
    float *dout_q = (float *)bt_drv_malloc(IQMIS_MAX_DIN_LEN * sizeof(float));
    float *int_mag = (float *)bt_drv_malloc(IQMIS_MAX_DIN_LEN * sizeof(float));
    float *int_i = (float *)bt_drv_malloc(IQMIS_MAX_DIN_LEN * sizeof(float));
    float *int_q = (float *)bt_drv_malloc(IQMIS_MAX_DIN_LEN * sizeof(float));

    int max_index1;
    int start_index;
    int max_index2;
    int end_index;
    int int_samples;


    float max_value1;
    float max_value2;

    float dc_i;
    float dc_q;

    float int_mag_sum;
    float int_mag_mean;

    float pwr_dif;
    float pwr_dif_sum;

    float gain_imb;
    float multi_sum;
    float phase_imb;

    float gain_comp;
    float phi_comp;

    int gain_comp_fxpt;
    int phi_comp_fxpt;

    // noise filtering
    if (IQMIS_LPF_EN==1)
    {
        for (jj=0;jj<40;jj++){
            reg_i[jj] = 0.0;
            reg_q[jj] = 0.0;
        }
        for (ii=0;ii<din_length;ii++){
            for (jj=0;jj<39;jj++){
                reg_i[39-jj] = reg_i[39-jj-1];
                reg_q[39-jj] = reg_q[39-jj-1];
            }
            reg_i[0] = din_i[ii];
            reg_q[0] = din_q[ii];

            din_i[ii] = 0.0;
            din_q[ii] = 0.0;
            for (jj=0;jj<20;jj++){
                din_i[ii] += (reg_i[jj] + reg_i[39-jj])*fir40[jj];
                din_q[ii] += (reg_q[jj] + reg_q[39-jj])*fir40[jj];
            }
        }
    }

    // search valid data
    max_index1 = 0;
    max_index2 = 0;
    max_value1 = din_i[0];
    max_value2 = din_i[IQMIS_MIN_INT_SAMPLES + IQMIS_SEARCH_LEN];
    for (ii=1;ii<IQMIS_SEARCH_LEN;ii++){
        if (din_i[ii]>max_value1){
            max_index1 = ii;
            max_value1 = din_i[ii];
        }
        jj = ii + IQMIS_MIN_INT_SAMPLES + IQMIS_SEARCH_LEN;
        if (din_i[jj]>max_value2){
            max_index2 = ii;
            max_value2 = din_i[jj];
        }
    }
    start_index = max_index1+1;
    end_index = IQMIS_MIN_INT_SAMPLES + IQMIS_SEARCH_LEN + max_index2;
    int_samples = end_index - start_index +1;

    dc_i = 0.0;
    dc_q = 0.0;
    for (ii=0;ii<int_samples;ii++)
    {
        dout_i[ii] = din_i[start_index+ii];
        dout_q[ii] = din_q[start_index+ii];
        dc_i += dout_i[ii];
        dc_q += dout_q[ii];
    }
    dc_i = dc_i/(float) int_samples;
    dc_q = dc_q/(float) int_samples;

    for (ii=0;ii<int_samples;ii++)
    {
        int_i[ii] = dout_i[ii] - dc_i;
        int_q[ii] = dout_q[ii] - dc_q;
    }

    // normalize
    int_mag_sum = 0.0;
    for (ii=0;ii<int_samples;ii++)
    {
        int_mag[ii] = (float)sqrtf((float)pow(int_i[ii],2.0)+(float)pow(int_q[ii],2.0));
        int_mag_sum += int_mag[ii];
    }
    int_mag_mean = int_mag_sum/(float) int_samples;

    for (ii=0;ii<int_samples;ii++)
    {
        int_i[ii] = int_i[ii]/int_mag_mean;
        int_q[ii] = int_q[ii]/int_mag_mean;
    }


    // mismatch estimation
    pwr_dif_sum = 0.0;
    for (ii=0;ii<int_samples;ii++)
    {
        pwr_dif = (float)pow(int_i[ii],2.0) - (float)pow(int_q[ii],2.0);
        pwr_dif_sum += pwr_dif;
    }

    gain_imb = pwr_dif_sum/(float) int_samples;

    multi_sum = 0.0;
    for (ii=0;ii<int_samples;ii++)
    {
        multi_sum += int_i[ii]*int_q[ii];
    }
    phase_imb = 2.0*multi_sum/(float) int_samples;


    // IQ mismatch compensation coef
    gain_comp = -gain_imb/2.0;
    phi_comp = -phase_imb/2.0;

#if defined(CHIP_BEST1501) || defined(CHIP_BEST1305)
    //CLIP
    if(gain_comp < -IQMIS_GAIN_COM_MAX) {
        gain_comp = -IQMIS_GAIN_COM_MAX;
    }

    if(gain_comp > IQMIS_GAIN_COM_MAX) {
        gain_comp = IQMIS_GAIN_COM_MAX;
    }

    if(phi_comp < -IQMIS_PHI_COM_MAX) {
        phi_comp = -IQMIS_PHI_COM_MAX;
    }

    if(phi_comp > IQMIS_PHI_COM_MAX) {
        phi_comp = IQMIS_PHI_COM_MAX;
    }
#endif

    gain_comp_fxpt = (int)round(gain_comp*(float)pow(2.0,10.0));
    if (gain_comp_fxpt<0)
    {
        *gain_comp_reg = gain_comp_fxpt + (int)pow(2.0,12.0);
    }
    else
    {
        *gain_comp_reg = gain_comp_fxpt;
    }

    phi_comp_fxpt = (int)round(phi_comp*(float)pow(2.0,10.0));
    if (phi_comp_fxpt<0)
    {
        *phi_comp_reg = phi_comp_fxpt + (int)pow(2.0,12.0);
    }
    else
    {
        *phi_comp_reg = phi_comp_fxpt;
    }

    bt_drv_free(dout_i);
    bt_drv_free(dout_q);
    bt_drv_free(int_mag);
    bt_drv_free(int_i);
    bt_drv_free(int_q);
}

void btdrv_update_local_iq_rx_val(uint32_t freq_range, uint16_t gain_val, uint16_t phase_val, BT_IQ_RX_CALIBRATION_CONFIG_T* pConfig)
{
    pConfig->validityMagicNum = BT_IQ_VALID_MAGIC_NUM;
    pConfig->rx_gain_cal_val[freq_range] = gain_val;
    pConfig->rx_phase_cal_val[freq_range] = phase_val;
}

bool btdrv_get_iq_rx_val_from_nv(BT_IQ_RX_CALIBRATION_CONFIG_T* pConfig)
{
    if (nv_record_get_extension_entry_ptr() &&
        (BT_IQ_VALID_MAGIC_NUM ==
        nv_record_get_extension_entry_ptr()->btIqRxCalConfig.validityMagicNum)) {
        *pConfig = nv_record_get_extension_entry_ptr()->btIqRxCalConfig;
        return true;
    } else {
        return false;
    }
}

void btdrv_set_iq_rx_val_to_nv(BT_IQ_RX_CALIBRATION_CONFIG_T* pConfig)
{
    uint32_t lock = nv_record_pre_write_operation();
    nv_record_get_extension_entry_ptr()->btIqRxCalConfig = *pConfig;
    nv_record_post_write_operation(lock);
    nv_record_extension_update();
}

#define DPD_MAX_DATA_LEN    2048
#define DPD_PEAK_PERIOD     512
#define DPD_PEAK_DIST       500
#define DPD_PEAK_OFFSET     5
#define DPD_PEAK_MIN        0.15//0.5
#define DPD_LPF_EN          1
#define DPD_REF_PHA_DOWN    0.0
#define DPD_REF_PHA_UP      3.1415926
#define DPD_AM_PM_LUT_LEN   128
#define DPD_COEF_MODE       2           // 0:down; 1: up; 2: down & up
#define DPD_PM_CLIP_POSI    89
#define DPD_PM_CLIP_MAX     4289        // round(15/180*pi*2^14); 15 degree
#define DPD_PI              3.1415926
#define DPD_DC_RMV_EN       1
#define DPD_PM_SIGN         0           // 1: normal; 0:negative

void lib_bt_cal_dpd_coef_core(float *din_i, float *din_q, int din_length, int *am_lut_base, int *am_lut, int *pm_lut, int *data_valid_flag)
{
    int ii;
    int jj;

    float fir40[20] = {-0.0383300781250000,-0.00488281250000000,-0.00415039062500000,-0.00268554687500000,-0.000488281250000000,0.00244140625000000,0.00610351562500000,0.0102539062500000,0.0153808593750000,0.0207519531250000,0.0266113281250000,0.0327148437500000,0.0385742187500000,0.0444335937500000,0.0498046875000000,0.0546875000000000,0.0588378906250000,0.0620117187500000,0.0642089843750000,0.0654296875000000};
    float reg_i[40];
    float reg_q[40];

    float *mag = (float *)bt_drv_malloc(DPD_MAX_DATA_LEN * sizeof(float));
    float *tri_mag_down = (float *)bt_drv_malloc(DPD_PEAK_PERIOD/2 * sizeof(float));
    float *tri_mag_up = (float *)bt_drv_malloc(DPD_PEAK_PERIOD/2 * sizeof(float));
    float *tri_pha_down = (float *)bt_drv_malloc(DPD_PEAK_PERIOD/2 * sizeof(float));
    float *tri_pha_up = (float *)bt_drv_malloc(DPD_PEAK_PERIOD/2 * sizeof(float));

    float *pa_am_lut = (float *)bt_drv_malloc(DPD_AM_PM_LUT_LEN * sizeof(float));
    float *pa_pm_lut = (float *)bt_drv_malloc(DPD_AM_PM_LUT_LEN * sizeof(float));
    float *dpd_am_lut_base = (float *)bt_drv_malloc(DPD_AM_PM_LUT_LEN * sizeof(float));
    float *dpd_am_lut = (float *)bt_drv_malloc(DPD_AM_PM_LUT_LEN * sizeof(float));
    float *dpd_pm_lut = (float *)bt_drv_malloc(DPD_AM_PM_LUT_LEN * sizeof(float));

    float peak_value_tab[8];
    int peak_index_tab[8];
    int peak_index;

    float max_peak_value;
    int left_peak_index;
    int right_peak_index;
    float dc_i;
    float dc_q;
    float ref_pha;
    int step_len;
    int index_left;

    TRACE(2, "%s DPD_PEAK_MIN*100=%d",__func__,(int)(DPD_PEAK_MIN*100));

    // noise filtering
    if (DPD_LPF_EN==1) {
        for (jj=0;jj<40;jj++) {
            reg_i[jj] = 0.0;
            reg_q[jj] = 0.0;
        }
        for (ii=0;ii<din_length;ii++) {

            for (jj=0;jj<39;jj++) {
                reg_i[39-jj] = reg_i[39-jj-1];
                reg_q[39-jj] = reg_q[39-jj-1];
            }
            reg_i[0] = din_i[ii];
            reg_q[0] = din_q[ii];

            din_i[ii] = 0.0;
            din_q[ii] = 0.0;
            for (jj=0;jj<20;jj++) {
                din_i[ii] += (reg_i[jj] + reg_i[39-jj])*fir40[jj];
                din_q[ii] += (reg_q[jj] + reg_q[39-jj])*fir40[jj];
            }
        }
    }

    for (ii=0;ii<din_length;ii++) {
        mag[ii] = (float)sqrtf((float)pow(din_i[ii],2.0)+(float)pow(din_q[ii],2.0));
    }

    for (ii=0;ii<8;ii++) {
        peak_value_tab[ii] = 0.0;
        peak_index_tab[ii] = 0;
    }

    peak_index = 0;
    for (ii=1;ii<din_length-1;ii++) {
        if (mag[ii]>=mag[ii-1] && mag[ii]>mag[ii+1]) {
            if (ii>=peak_index_tab[peak_index]+DPD_PEAK_DIST) {
                if (peak_index<7) {
                    peak_index = peak_index + 1;
                }
                peak_value_tab[peak_index] = mag[ii];
                peak_index_tab[peak_index] = ii;
            } else {
                if (mag[ii]>=peak_value_tab[peak_index]) {
                    peak_value_tab[peak_index] = mag[ii];
                    peak_index_tab[peak_index] = ii;
                }
            }
        }

    }

    // find max peak
    max_peak_value = 0.0;
    for (ii=0;ii<8;ii++) {
        if (peak_value_tab[ii]>max_peak_value) {
            max_peak_value = peak_value_tab[ii];
        }
    }

    left_peak_index = 0;
    right_peak_index = 0;
    *data_valid_flag = 0;

    for (ii=0;ii<7;ii++) {
        if ((peak_value_tab[ii]>max_peak_value*DPD_PEAK_MIN) && (peak_value_tab[ii+1]>max_peak_value*DPD_PEAK_MIN)) {
            if ((int) fabs((float)(peak_index_tab[ii+1]-peak_index_tab[ii]-DPD_PEAK_PERIOD))<=DPD_PEAK_OFFSET) {
                left_peak_index = peak_index_tab[ii];
                right_peak_index = peak_index_tab[ii+1];
                *data_valid_flag = 1;
            }
        }
    }

    // DC remover
    if (DPD_DC_RMV_EN==1) {
        dc_i = (din_i[left_peak_index] + din_i[right_peak_index])/2.0;
        dc_q = (din_q[left_peak_index] + din_q[right_peak_index])/2.0;
    } else {
        dc_i = 0.0;
        dc_q = 0.0;
    }

    if(*data_valid_flag==1) {
        for (ii=0;ii<DPD_PEAK_PERIOD/2;ii++) {
            tri_mag_down[ii] = (float)sqrtf((float)pow((din_i[left_peak_index+(DPD_PEAK_PERIOD/2-1)-ii]-dc_i),2.0)+(float)pow((din_q[left_peak_index+(DPD_PEAK_PERIOD/2-1)-ii]-dc_q),2.0));
            tri_mag_up[ii] = (float)sqrtf((float)pow((din_i[right_peak_index-(DPD_PEAK_PERIOD/2-1)+ii]-dc_i),2.0)+(float)pow((din_q[right_peak_index-(DPD_PEAK_PERIOD/2-1)+ii]-dc_q),2.0));

            tri_pha_down[ii] = (float) atan2((din_q[left_peak_index+(DPD_PEAK_PERIOD/2-1)-ii]-dc_q),(din_i[left_peak_index+(DPD_PEAK_PERIOD/2-1)-ii]-dc_i));
            tri_pha_up[ii] = (float) atan2((din_q[right_peak_index-(DPD_PEAK_PERIOD/2-1)+ii]-dc_q),(din_i[right_peak_index-(DPD_PEAK_PERIOD/2-1)+ii]-dc_i));
            tri_pha_down[ii] -= DPD_REF_PHA_DOWN;
            tri_pha_up[ii] -= DPD_REF_PHA_UP;
        }

        ref_pha = 0.0;
        for (ii=DPD_PEAK_PERIOD/4;ii<DPD_PEAK_PERIOD/2;ii++) {
            ref_pha += tri_pha_down[ii];
        }
        ref_pha = ref_pha/(float)(DPD_PEAK_PERIOD/4);

        for (ii=0;ii<DPD_PEAK_PERIOD/2;ii++) {
            tri_pha_down[ii] -= ref_pha;
            if (tri_pha_down[ii]>=DPD_PI) {
                tri_pha_down[ii] -= 2.0*DPD_PI;
            } else if (tri_pha_down[ii]<-DPD_PI) {
                tri_pha_down[ii] += 2.0*DPD_PI;
            }

            tri_pha_up[ii] -= ref_pha;
            if (tri_pha_up[ii]>=DPD_PI) {
                tri_pha_up[ii] -= 2.0*DPD_PI;
            } else if (tri_pha_up[ii]<-DPD_PI) {
                tri_pha_up[ii] += 2.0*DPD_PI;
            }
        }

        // cal PA am & pm coef
        for (ii=0;ii<DPD_AM_PM_LUT_LEN;ii++) {
            pa_am_lut[ii] = 0.0;
            pa_pm_lut[ii] = 0.0;
        }

        step_len = DPD_PEAK_PERIOD/2/DPD_AM_PM_LUT_LEN;
        for (ii=0;ii<DPD_AM_PM_LUT_LEN;ii++) {
            for(jj=0;jj<step_len;jj++) {
                if (DPD_COEF_MODE==0) {
                    pa_am_lut[ii] += tri_mag_down[(ii)*step_len+jj];
                    pa_pm_lut[ii] += tri_pha_down[(ii)*step_len+jj];
                } else if (DPD_COEF_MODE==1) {
                    pa_am_lut[ii] += tri_mag_up[(ii)*step_len+jj];
                    pa_pm_lut[ii] += tri_pha_up[(ii)*step_len+jj];
                } else {
                    pa_am_lut[ii] += tri_mag_down[(ii)*step_len+jj] + tri_mag_up[(ii)*step_len+jj];
                    pa_pm_lut[ii] += tri_pha_down[(ii)*step_len+jj] + tri_pha_up[(ii)*step_len+jj];
                }
            }

            if (DPD_COEF_MODE==0 || DPD_COEF_MODE==1) {
                pa_am_lut[ii] /= (float)step_len;
                pa_pm_lut[ii] /= (float)step_len;
            } else {
                pa_am_lut[ii] /= (float)(step_len*2);
                pa_pm_lut[ii] /= (float)(step_len*2);
            }
        }

        // am normalize
        for(ii=0;ii<DPD_AM_PM_LUT_LEN;ii++) {
            pa_am_lut[ii] /= pa_am_lut[DPD_AM_PM_LUT_LEN-1];
        }

        // pm clip
        for (ii=0;ii<DPD_PM_CLIP_POSI;ii++) {
            pa_pm_lut[ii] = pa_pm_lut[DPD_PM_CLIP_POSI-1];
        }

        // cal DPD am & pm coef
        for (ii=0;ii<DPD_AM_PM_LUT_LEN;ii++) {
            dpd_am_lut_base[ii] = (float)(ii+1)/(float)DPD_AM_PM_LUT_LEN;
        }

        // am
        index_left = -1;
        for (ii=0;ii<DPD_AM_PM_LUT_LEN;ii++) {
            for (jj=0;jj<DPD_AM_PM_LUT_LEN;jj++) {
                if (dpd_am_lut_base[ii]>=pa_am_lut[jj]) {
                    index_left = jj;
                }
            }

            if (index_left==-1) {
                dpd_am_lut[ii] = dpd_am_lut_base[0];
            } else if (index_left==(DPD_AM_PM_LUT_LEN-1)) {
                dpd_am_lut[ii] = dpd_am_lut_base[DPD_AM_PM_LUT_LEN-1];
            } else {
                dpd_am_lut[ii] = (dpd_am_lut_base[ii]-pa_am_lut[index_left])/(pa_am_lut[index_left+1]-pa_am_lut[index_left])*(dpd_am_lut_base[index_left+1]-dpd_am_lut_base[index_left]) + dpd_am_lut_base[index_left];
            }
        }

        // pm
        index_left = -1;
        for (ii=0;ii<DPD_AM_PM_LUT_LEN;ii++) {
            for (jj=0;jj<DPD_AM_PM_LUT_LEN;jj++) {
                if (dpd_am_lut[ii]>=dpd_am_lut_base[jj]) {
                    index_left = jj;
                }
            }

            if (index_left==-1) {
                dpd_pm_lut[ii] = pa_pm_lut[0];
            } else if (index_left==(DPD_AM_PM_LUT_LEN-1)) {
                dpd_pm_lut[ii] = pa_pm_lut[DPD_AM_PM_LUT_LEN-1];
            } else {
                dpd_pm_lut[ii] = (dpd_am_lut[ii]-dpd_am_lut_base[index_left])/(dpd_am_lut_base[index_left+1]-dpd_am_lut_base[index_left])*(pa_pm_lut[index_left+1]-pa_pm_lut[index_left]) + pa_pm_lut[index_left];
            }
            if (DPD_PM_SIGN==1) {
                dpd_pm_lut[ii] = -dpd_pm_lut[ii];
            }
        }
    } else {
        for (ii=0;ii<DPD_AM_PM_LUT_LEN;ii++) {
            dpd_am_lut_base[ii] = (float)(ii+1)/(float)DPD_AM_PM_LUT_LEN;
            dpd_am_lut[ii] = (float)(ii+1)/(float)DPD_AM_PM_LUT_LEN;
            dpd_pm_lut[ii] = 0.0;
        }
    }


    // fxpt
    for (ii=0;ii<DPD_AM_PM_LUT_LEN;ii++) {
        am_lut[ii] = (int) round(dpd_am_lut[ii]*(float)pow(2.0,14.0));  // <u,0,14>
        if (am_lut[ii]>16383) {
            am_lut[ii] = 16383;
        }

        am_lut_base[ii] = (int) round(dpd_am_lut_base[ii]*(float)pow(2.0,14.0));  // <u,0,14>
        if (am_lut_base[ii]>16383) {
            am_lut_base[ii] = 16383;
        }

        pm_lut[ii] = (int) round(dpd_pm_lut[ii]*(float)pow(2.0,14.0));             // <u,0,14>

        if (pm_lut[ii]>DPD_PM_CLIP_MAX) {
            pm_lut[ii] = DPD_PM_CLIP_MAX;
        } else if (pm_lut[ii]<-DPD_PM_CLIP_MAX) {
            pm_lut[ii] = -DPD_PM_CLIP_MAX;
        }
    }

    bt_drv_free(mag);
    bt_drv_free(tri_mag_down);
    bt_drv_free(tri_mag_up);
    bt_drv_free(tri_pha_down);
    bt_drv_free(tri_pha_up);
    bt_drv_free(pa_am_lut);
    bt_drv_free(pa_pm_lut);
    bt_drv_free(dpd_am_lut_base);
    bt_drv_free(dpd_am_lut);
    bt_drv_free(dpd_pm_lut);
}

#endif
