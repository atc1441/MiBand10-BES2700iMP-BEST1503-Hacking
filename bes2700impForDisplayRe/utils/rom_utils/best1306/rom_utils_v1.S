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
#include "asm_macros.h"

//METAL_ID_0
    def_FuncExt v1, rom1_entry,                         0x00030000
    def_FuncExt v1, set__impure_ptr,                    0x00030004
    def_FuncExt v1, sbc_bits_for_mono_alloc,            0x00030004
    def_FuncExt v1, sbc_bits_for_stereo_alloc,          0x00030220
    def_FuncExt v1, sbc_header_process,                 0x00030520
    def_FuncExt v1, sbc_factors_process,                0x00030640
    def_FuncExt v1, sbc_sbsamples_make,                 0x0003073c
    def_FuncExt v1, sbc_factors_calc,                   0x00031970
    def_FuncExt v1, sbc_header_make,                    0x00031b94
    def_FuncExt v1, sbc_frame_mute,                     0x00031dc4
    def_FuncExt v1, sbc_samples_quant,                  0x00031df4
    def_FuncExt v1, sbc_frame_length_get,               0x00032cf4
    def_FuncExt v1, sbc_decoder_init,                   0x00032d84
    def_FuncExt v1, sbc_4band_synth_filter,             0x00032d90
    def_FuncExt v1, sbc_8band_synth_filter,             0x0003363c
    def_FuncExt v1, sbc_frames_parser,                  0x000348f4
    def_FuncExt v1, sbc_frames_decode_do,               0x00034af4
    def_FuncExt v1, sbc_frames_decode,                  0x00034ec0
    def_FuncExt v1, sbc_frames_decode_select_channel,   0x00034ee0
    def_FuncExt v1, sbc_frames_decode_out_sbsamples,    0x00034ef4
    def_FuncExt v1, sbc_encoder_init,                   0x00035360
    def_FuncExt v1, sbc_frames_encode,                  0x0003536c
    def_FuncExt v1, sbc_frames_encode_with_sbsamples,   0x00037338
    def_FuncExt v1, lc3_api_decoder_init_sneak_rom,     0x00038000
    def_FuncExt v1, lc3_api_encoder_init_sneak_rom,     0x00039000

    def_Symbol dec_coeff_4band_tbl_v1,                  0x24017564
    def_Symbol dec_coeff_8band_tbl_v1,                  0x24017604
    def_Symbol loudness_offs_4band_v1,                  0x24017744
    def_Symbol loudness_offs_8band_v1,                  0x24017754
    def_Symbol sbc_coef_4band_v1,                       0x24017774
    def_Symbol sbc_coef_8band_v1,                       0x24017814
    def_Symbol sbc_crc_tbl_v1,                          0x24017954
    def_Symbol sbc_enc_analyze_4band_tbl_v1,            0x24017a54
    def_Symbol sbc_enc_analyze_8band_tbl_v1,            0x24017ad4
    def_Symbol sbc_samples_quant.str1.4_v1,             0x24017cd4
    def_Symbol synth_4band_tbl_v1,                      0x24017ce0
    def_Symbol synth_8band_tbl_v1,                      0x24017d60
//dumy
    def_FuncExt v1, btif_sbc_init_decoder_real,         0x00000000
    def_FuncExt v1, btif_sbc_decode_frames_real,        0x00000000
    def_FuncExt v1, btif_sbc_decode_frames_select_channel_real, 0x00000000
    def_FuncExt v1, btif_sbc_decode_frames_parser_real, 0x00000000
    def_FuncExt v1, btif_sbc_init_encoder_real,         0x00000000
    def_FuncExt v1, btif_sbc_encode_frames_real,        0x00000000
    def_FuncExt v1, btif_plc_update_sbc_decoder_state_real, 0x00000000

//METAL_ID_1
    def_FuncExt v2, rom1_entry,                         0x00029000
    def_FuncExt v2, set__impure_ptr,                    0x00029004
    def_FuncExt v2, sbc_bits_for_mono_alloc,            0x00029004
    def_FuncExt v2, sbc_bits_for_stereo_alloc,          0x0002924c
    def_FuncExt v2, sbc_header_process,                 0x00029558
    def_FuncExt v2, sbc_sbsamples_make,                 0x0002967c
    def_FuncExt v2, sbc_joint_process,                  0x00029824
    def_FuncExt v2, sbc_filter_8band_analysis_part1,    0x0002986c
    def_FuncExt v2, sbc_filter_8band_analysis_part2,    0x00029904
    def_FuncExt v2, sbc_factors_calc,                   0x00029ac4
    def_FuncExt v2, sbc_header_make,                    0x00029d20
    def_FuncExt v2, sbc_samples_quant,                  0x00029f84
    def_FuncExt v2, sbc_factors_process,                0x0002a1a4
    def_FuncExt v2, sbc_frame_length_get,               0x0002a2a4
    def_FuncExt v2, sbc_decoder_init,                   0x0002a354
    def_FuncExt v2, sbc_4band_synth_filter_part1,       0x0002a360
    def_FuncExt v2, sbc_4band_synth_filter_part2,       0x0002a3dc
    def_FuncExt v2, sbc_4band_synth_filter,             0x0002a520
    def_FuncExt v2, sbc_8band_synth_filter_part1,       0x0002a680
    def_FuncExt v2, sbc_8band_synth_filter_part2,       0x0002a72c
    def_FuncExt v2, sbc_8band_synth_filter,             0x0002a878
    def_FuncExt v2, sbc_frames_parser,                  0x0002aa20
    def_FuncExt v2, sbc_frames_decode_do,               0x0002abf0
    def_FuncExt v2, sbc_frames_decode,                  0x0002af48
    def_FuncExt v2, sbc_frames_decode_select_channel,   0x0002af68
    def_FuncExt v2, sbc_frames_decode_out_sbsamples,    0x0002af80
    def_FuncExt v2, sbc_encoder_init,                   0x0002b3a0
    def_FuncExt v2, sbc_filter_8band_analysis,          0x0002b3ac
    def_FuncExt v2, sbc_frames_encode,                  0x0002b4a0
    def_FuncExt v2, sbc_frames_encode_with_sbsamples,   0x0002b5bc
    def_FuncExt v2, lc3_api_decoder_init_sneak_rom,     0x0002c98c
    def_FuncExt v2, lc3_api_encoder_init_sneak_rom,     0x0002cb38

    def_Symbol dec_coeff_4band_tbl_v2,                  0x240161a0
    def_Symbol dec_coeff_8band_tbl_v2,                  0x24016240
    def_Symbol loudness_offs_4band_v2,                  0x24016380
    def_Symbol loudness_offs_8band_v2,                  0x24016390
    def_Symbol sbc_coef_4band_v2,                       0x240163b0
    def_Symbol sbc_coef_8band_v2,                       0x24016450
    def_Symbol sbc_crc_tbl_v2,                          0x24016590
    def_Symbol sbc_enc_analyze_4band_tbl_v2,            0x24016690
    def_Symbol sbc_enc_analyze_8band_tbl_v2,            0x24016710
    def_Symbol synth_4band_tbl_v2,                      0x24016910
    def_Symbol synth_8band_tbl_v2,                      0x24016990
//dumy
    def_FuncExt v2, btif_sbc_init_decoder_real,         0x00000000
    def_FuncExt v2, btif_sbc_decode_frames_real,        0x00000000
    def_FuncExt v2, btif_sbc_decode_frames_select_channel_real, 0x00000000
    def_FuncExt v2, btif_sbc_decode_frames_parser_real, 0x00000000
    def_FuncExt v2, btif_sbc_init_encoder_real,         0x00000000
    def_FuncExt v2, btif_sbc_encode_frames_real,        0x00000000
    def_FuncExt v2, btif_plc_update_sbc_decoder_state_real, 0x00000000

//METAL_ID_2
    def_FuncExt v3, rom1_entry,                         0x00029000
    def_FuncExt v3, set__impure_ptr,                    0x00029004
    def_FuncExt v3, sbc_bits_for_mono_alloc,            0x00029ad8
    def_FuncExt v3, sbc_bits_for_stereo_alloc,          0x00029da0
    def_FuncExt v3, sbc_header_process,                 0x0002aaa0
    def_FuncExt v3, sbc_factors_process,                0x0002a9f4
    def_FuncExt v3, sbc_factors_calc,                   0x0002a760
    def_FuncExt v3, sbc_header_make,                    0x0002b6d4
    def_FuncExt v3, sbc_samples_quant,                  0x0002bb80
    def_FuncExt v3, sbc_decoder_init,                   0x0002b2e0
    def_FuncExt v3, sbc_4band_synth_filter,             0x0002c088
    def_FuncExt v3, sbc_8band_synth_filter,             0x0002a55c
    def_FuncExt v3, sbc_frames_parser,                  0x0002adb8
    def_FuncExt v3, sbc_decode,                         0x0002b2f0
    def_FuncExt v3, sbc_encoder_init,                   0x00029a64
    def_FuncExt v3, sbc_encode,                         0x00029aa8

    def_FuncExt v3, btif_sbc_init_decoder_real,         0x0002b35c
    def_FuncExt v3, btif_sbc_decode_frames_real,        0x0002b360
    def_FuncExt v3, btif_sbc_decode_frames_select_channel_real, 0x0002b4a8
    def_FuncExt v3, btif_sbc_decode_frames_parser_real, 0x0002b5d0
    def_FuncExt v3, btif_sbc_init_encoder_real,         0x0002b5d8
    def_FuncExt v3, btif_sbc_encode_frames_real,        0x0002b5e0
    def_FuncExt v3, btif_plc_update_sbc_decoder_state_real, 0x0002b63c
    def_Symbol sbc_crc_tbl_v3,                          0x24017060
    def_FuncExt v3, lc3_api_decoder_init_sneak_rom,     0x0002d4b0
    def_FuncExt v3, lc3_api_encoder_init_sneak_rom,     0x0002d65c
//dumy
    def_FuncExt v3, sbc_frames_decode_select_channel,   0x00000000
    def_FuncExt v3, sbc_frames_encode,   0x00000000
    def_FuncExt v3, sbc_frames_decode,   0x00000000
    def_FuncExt v3, sbc_frames_decode_out_sbsamples,    0x00000000
    def_FuncExt v3, sbc_frames_encode_with_sbsamples,   0x00000000