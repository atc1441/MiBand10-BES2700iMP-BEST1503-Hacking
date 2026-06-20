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

#if defined(SPEECH_DC_FILTER_IN_ROM)
def_RomFunc  speech_dc_filter_create_with_custom_allocator
def_RomFunc  speech_dc_filter_destroy
def_RomFunc  speech_dc_filter_set_config
def_RomFunc  speech_dc_filter_ctl
def_RomFunc  speech_dc_filter_process
def_RomFunc  speech_dc_filter_process_int24
def_RomFunc  speech_dc_filter_dump
def_RomFunc  speech_dc_filter_get_required_mips
#endif

#if defined(SPEECH_AEC2FLOAT_IN_ROM)
def_RomFunc  ec2float_create_with_custom_allocator
def_RomFunc  ec2float_destroy
def_RomFunc  ec2float_set_config
def_RomFunc  ec2float_stereo
def_RomFunc  ec2float_process
def_RomFunc  ec2float_set_external_vad
def_RomFunc  ec2float_get_fir_filter_coeffs
def_RomFunc  ec2float_get_fir_filter_freq_coeffs
def_RomFunc  ec2float_get_adaptive_filter_length
def_RomFunc  ec2float_get_adaptive_filter_data
def_RomFunc  ec2float_get_required_mips
def_RomFunc  ec2float_get_snr
def_RomFunc  ec2float_get_noise_power
#endif

#if defined(SPEECH_EQ_IN_ROM)
def_RomFunc  eq_arm_init
def_RomFunc  eq_arm_init_with_custom_allocator
def_RomFunc  multi_eq_arm_init
def_RomFunc  multi_eq_arm_init_with_custom_allocator
def_RomFunc  eq_arm_destroy
def_RomFunc  eq_arm_set_config
def_RomFunc  eq_arm_process
def_RomFunc  eq_arm_process_int24
def_RomFunc  eq_arm_process_float
def_RomFunc  eq_arm_process2
def_RomFunc  eq_arm_process2_int24
def_RomFunc  eq_arm_process2_float
def_RomFunc  eq_arm_get_required_mips
#endif

#if defined(SPEECH_TX_COMPEXP_IN_ROM)
def_RomFunc  compexp_create_with_custom_allocator
def_RomFunc  compexp_destroy
def_RomFunc  compexp_set_config
def_RomFunc  compexp_process
def_RomFunc  compexp_process_adj_eq
def_RomFunc  compexp_process_int24
def_RomFunc  compexp_get_required_mips
#endif

#if defined(SPEECH_TX_1MIC_NS_IN_ROM)
def_RomFunc  speech_1mic_ns_create
def_RomFunc  speech_1mic_ns_destroy
def_RomFunc  speech_1mic_ns_process
#endif

#if defined(SPEECH_2MIC_NS8_IN_ROM)
def_RomFunc  speech_2mic_ns8_create
def_RomFunc  speech_2mic_ns8_destroy
def_RomFunc  speech_2mic_ns8_process
def_RomFunc  speech_2mic_ns8_get_required_mips
def_RomFunc  speech_2mic_ns8_get_delay
def_RomFunc  speech_2mic_ns8_set_config
#endif

#if defined(SPEECH_NS2FLOAT_IN_ROM)
def_RomFunc  speech_ns2float_create
def_RomFunc  speech_ns2float_create_with_custom_allocator
def_RomFunc  speech_ns2float_destroy
def_RomFunc  speech_ns2float_set_config
def_RomFunc  speech_ns2float_process
def_RomFunc  speech_ns2float_process_int24
def_RomFunc  speech_ns2float_set_echo_state
def_RomFunc  speech_ns2float_set_echo_suppress
def_RomFunc  speech_ns2float_set_ringtone_mode
def_RomFunc  speech_ns2float_set_external_vad
def_RomFunc  speech_ns2float_get_required_mips
#endif
