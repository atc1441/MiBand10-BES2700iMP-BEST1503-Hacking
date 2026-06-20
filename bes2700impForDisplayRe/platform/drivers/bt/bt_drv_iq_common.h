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
#ifndef __BT_DRV_IQ_COMMON_H__
#define  __BT_DRV_IQ_COMMON_H__
#if defined (RX_IQ_CAL) || defined(TX_PULLING_CAL) || defined(DPD_ONCHIP_CAL)
#include "nvrecord_extension.h"

#define BTRF_REG_DUMP(addr) do{ \
                                uint16_t rf_reg_val = 0; \
                                btdrv_read_rf_reg(addr, &rf_reg_val); \
                                TRACE(2,"btrf reg addr:%04x val:%04x", addr, rf_reg_val); \
                            }while(0);

#define BTRF_DIG_DUMP(addr) do{ \
                                uint32_t dig_reg_val = 0; \
                                dig_reg_val = BTDIGITAL_REG(addr); \
                                TRACE(2,"dig reg addr:%08x val:%08x", addr, dig_reg_val); \
                            }while(0);

void bt_drv_rx_iq_datasort(int *arry,int len);

void bt_drv_rx_iq_datawrite(int * gain_arr, int * phy_arr, int len);

void lib_iqmis_getiq(short *data, int datalength_2X, float *din_i, float *din_q);

void lib_iqmis_esti(float *din_i, float *din_q, int din_length, int *gain_comp_reg, int *phi_comp_reg);

void btdrv_update_local_iq_rx_val(uint32_t freq_range, uint16_t gain_val, uint16_t phase_val, BT_IQ_RX_CALIBRATION_CONFIG_T* pConfig);

bool btdrv_get_iq_rx_val_from_nv(BT_IQ_RX_CALIBRATION_CONFIG_T* pConfig);

void btdrv_set_iq_rx_val_to_nv(BT_IQ_RX_CALIBRATION_CONFIG_T* pConfig);

void lib_bt_cal_dpd_coef_core(float *din_i, float *din_q, int din_length, int *am_lut_base, int *am_lut, int *pm_lut, int *data_valid_flag);
#endif
#endif
