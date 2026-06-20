/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#include "asm_macros.h"

def_FuncExt  v3,  speech_dc_filter_create,                        0x0c12034c
def_FuncExt  v3,  speech_dc_filter_create_with_custom_allocator,  0x0c120190
def_FuncExt  v3,  speech_dc_filter_destroy,                       0x0c12050c
def_FuncExt  v3,  speech_dc_filter_set_config,                    0x0c120524
def_FuncExt  v3,  speech_dc_filter_ctl,                           0x0c120698
def_FuncExt  v3,  speech_dc_filter_process,                       0x0c120854
def_FuncExt  v3,  speech_dc_filter_process_int24,                 0x0c120954
def_FuncExt  v3,  speech_dc_filter_dump,                          0x0c1206f4
def_FuncExt  v3,  speech_dc_filter_get_required_mips,             0x0c120714
def_FuncExt  v3,  ec2float_create,                                0x0c120b94
def_FuncExt  v3,  ec2float_create_with_custom_allocator,          0x0c12095c
def_FuncExt  v3,  ec2float_destroy,                               0x0c120dcc
def_FuncExt  v3,  ec2float_set_config,                            0x0c120e34
def_FuncExt  v3,  ec2float_process,                               0x0c121024
def_FuncExt  v3,  ec2float_set_external_vad,                      0x0c1211d0
def_FuncExt  v3,  ec2float_get_fir_filter_coeffs,                 0x0c1211e4
def_FuncExt  v3,  ec2float_get_fir_filter_freq_coeffs,            0x0c1211ec
def_FuncExt  v3,  ec2float_get_adaptive_filter_length,            0x0c1211f4
def_FuncExt  v3,  ec2float_get_adaptive_filter_data,              0x0c121200
def_FuncExt  v3,  ec2float_get_required_mips,                     0x0c121214
def_FuncExt  v3,  ec2float_get_snr,                               0x0c12128c
def_FuncExt  v3,  ec2float_get_noise_power,                       0x0c121294
def_FuncExt  v3,  eq_arm_init,                                    0x0c121600
def_FuncExt  v3,  eq_arm_init_with_custom_allocator,              0x0c121594
def_FuncExt  v3,  multi_eq_arm_init,                              0x0c121620
def_FuncExt  v3,  multi_eq_arm_init_with_custom_allocator,        0x0c121534
def_FuncExt  v3,  eq_arm_destroy,                                 0x0c12129c
def_FuncExt  v3,  eq_arm_set_config,                              0x0c1212dc
def_FuncExt  v3,  eq_arm_process,                                 0x0c12167c
def_FuncExt  v3,  eq_arm_process_int24,                           0x0c121758
def_FuncExt  v3,  eq_arm_process_float,                           0x0c121828
def_FuncExt  v3,  eq_arm_process2,                                0x0c1218b0
def_FuncExt  v3,  eq_arm_process2_int24,                          0x0c1219b4
def_FuncExt  v3,  eq_arm_process2_float,                          0x0c121ab4
def_FuncExt  v3,  eq_arm_get_required_mips,                       0x0c121bb0
def_FuncExt  v3,  compexp_create,                                 0x0c121ee4
def_FuncExt  v3,  compexp_create_with_custom_allocator,           0x0c121e9c
def_FuncExt  v3,  compexp_destroy,                                0x0c121c04
def_FuncExt  v3,  compexp_set_config,                             0x0c121c20
def_FuncExt  v3,  compexp_process,                                0x0c1221f4
def_FuncExt  v3,  compexp_process_adj_eq,                         0x0c121f30
def_FuncExt  v3,  compexp_process_int24,                          0x0c1224f0
def_FuncExt  v3,  compexp_get_required_mips,                      0x0c1227e4
def_FuncExt  v3,  speech_1mic_ns_create,                          0x0c120004
def_FuncExt  v3,  speech_1mic_ns_destroy,                         0x0c1200a0
def_FuncExt  v3,  speech_1mic_ns_process,                         0x0c1200d0
