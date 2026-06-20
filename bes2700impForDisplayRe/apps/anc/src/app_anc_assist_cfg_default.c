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
#include "plat_types.h"
#include "anc_assist.h"

#if defined (VOICE_ASSIST_CUSTOM_LEAK_DETECT)
const static int16_t custom_leak_detect_pcm_data[] = {
   #include "res/ld/tone_peco.h"
};
#else 
const static int16_t custom_leak_detect_pcm_data[] = {
   0,0,0
};
#endif

AncAssistConfig anc_assist_cfg = {
    .bypass = 0,
    .debug_en = 0,
    .howling_debug_en = 0,
    .wind_debug_en = 0,
    .noise_debug_en = 0,

    .ff_howling_en  = 0,
    .fb_howling_en  = 0,
    .noise_en   = 0,
    .noise_classify_en  = 0,
    .wind_en    = 0,
    .wind_single_mic_en = 0,
	.pilot_en   = 0,
	.pnc_en     = 0,
    .wsd_en     = 0,
    .extern_kws_en     = 0,

    .ff_howling_cfg = {
        .ind0 = 16,         // 62.5Hz per 62.5*32=2k
#ifdef ASSIST_LOW_RAM_MOD
        .ind1 = 60,         // 62.5*60=3.75k
#else
        .ind1 = 120,        // 62.5*120=7.5k
#endif
        .time_thd = 1000,   // ff recover time 500*7.5ms=3.75s
        .power_thd = 1e3f,  // 4.6e10 0db
    },

    .fb_howling_cfg = {
        .ind0 = 16,         // 62.5Hz per 62.5*32=2k
#ifdef ASSIST_LOW_RAM_MOD
        .ind1 = 60,         // 62.5*60=3.75k
#else
        .ind1 = 120,        // 62.5*120=7.5k
#endif
        .time_thd = 1000,   // ff recover time 500*7.5ms=3.75s
        .power_thd = 1e9f,  // 4.6e10 0db
    },

/*
trans_table[current_status,target_status]:The number of frames required to switch from current status to target status;
time = trans_table[][]*window_size*period;
*/
    .noise_cfg = {
        .debug_en = 1,
        .init_state = 4,
        .strong_low_thd = 320,
        .strong_limit_thd = 60,
        .lower_low_thd = 3,
        .lower_mid_thd = 17,
        .quiet_out_thd = 1.5,
        .quiet_thd = 0.2,
        .extremely_quiet_out_thd = 0.25,
        .extremely_quiet_in_thd = 0.1,
        .snr_thd = 100,
        .period = 16,
        .window_size = 5,
        .trans_table = {
            {0, 4, 12, 12, 18},
            {4, 0, 12, 12, 18},
            {4, 4, 0 , 12, 18},
            {4, 4, 12, 0 , 18},
            {4, 4, 12, 12, 0 },
        },
        .band_freq = {100, 400, 1000, 2000},
        .band_weight = {0.7, 0.3, 0},
    },

    .noise_classify_cfg = {
        .plane_thd = 700,
        .plane_hold_thd = 350,
        .transport_thd = 500,
        .transport_hold_thd = 250,
        .indoor_thd = 15,
        .indoor_hold_thd = 40,
        .snr_thd = 100,
        .period = 16,
        .window_size = 5,
        .strong_count = 8,
        .normal_count = 12,
        .band_freq = {100, 250, 500, 1000},
        .band_weight = {0.5, 0.5, 0},
    },
/*
lower_stage1_wait_cnt : When the wind become lower,wait lower_stage1_wait_cnt and then change state.If the wind state change during the waiting time,the change is canceled.
lower_stage2_keep_cnt : when the wind state change,such as normal to small,algo will return a intermediate state gain--gain_normal_to_small, keep lower_stage2_keep_cnt and then return gain_small.
time = cnt * 7.5ms
*/
    .wind_cfg = {
        .scale_size = 8,           // freq range,8/scale=1k
        .to_none_targettime = 500, // time=500*7.5ms=3.75s 
        .power_thd = 0.0001, 
		.no_thd = 0.8,		
		.small_thd = 0.7,
		.normal_thd = 0.55,
		.strong_thd = 0.4,
		.gain_none = 1.0,
		.gain_small_to_none = 0.7500,				//xiaomiN77:small wind 3m; normal wind 5m; strong wind 7m
		.gain_small = 0.667,
		.gain_normal_to_small = 0.3750,
		.gain_normal = 0.333,
		.gain_strong_to_normal = 0.1563,
		.gain_strong = 0,
        .lower_stage1_wait_cnt = 1,
        .lower_stage2_keep_cnt = 1,
        .higher_stage1_wait_cnt = 2,
        .higher_stage2_keep_cnt = 120,
    },

    .wind_single_mic_cfg = {
        .close_delay = 7,      //turn into no wind
        .other_delay = 2,     //larger wind
    },

    .pilot_cfg = {
        .dump_en = 0,
	    .delay = 300,
#ifdef VOICE_ASSIST_WD_ENABLED
	    .cal_period = 25,
#else
        .cal_period = 25,
#endif
        .gain_smooth_ms = 300,

        .adaptive_anc_en = 0,
	    .thd_on = {5.6400,  1.5648,  0.6956,  0.4672,  0.4089,  0.2694, 0.1853,0.1373,0.0629,0.0325,0.0043},
	    .thd_off = {5.6400,  1.5648,  0.6956,  0.4672,  0.4089,  0.2694, 0.1853,0.1373,0.0629,0.0325,0.0043},

        .wd_en =0,
        .off_inear_thd = 1, //0.1,
        .off_outear_thd = 0.5, // 0.02,
        .on_inear_thd = 1, //0.1,
        .on_outear_thd = 0.5, // 0.02,
        .tt_inear_thd = 1, //0.1,
        .tt_outear_thd = 0.5, // 0.02,
        .wear_energy_cmp_num = 3, //
        .wear_skip_energy_num = 3, 
        .unwear_energy_cmp_num = 4, //
        .unwear_skip_energy_num = 12,

        .infra_ratio = 0.7,
        .ultrasound_stable_tick = 2,
        .ultrasound_stop_tick = 3,
        .ultrasound_thd = 0.1,

        .custom_leak_detect_en = 0,
        .custom_leak_detect_playback_loop_num = 2,
        .custom_pcm_data = custom_leak_detect_pcm_data,
        .custom_pcm_data_len = sizeof(custom_leak_detect_pcm_data) / sizeof(int16_t),
        .gain_local_pilot_signal = 0.0,
    },

    .pnc_cfg = {
        .pnc_lower_bound = 1400*256/16000,
        .pnc_upper_bound = 1500*256/16000,
        .out_lower_bound = 1400*256/16000,
        .out_upper_bound = 1500*256/16000,
        .cal_period = 12,
        .out_thd = 100,
    },
	
	.prompt_cfg = {
        .dump_en = 1,
	    .cal_period = 10,
        .curve_num = 10,
        .max_env_energy = 1e9,
        .start_index = 1,
        .end_index = 10,
        .freq_point1 = 275,
        .freq_point2 = 460,
        .freq_point3 = 550,
        .thd1 = {1,0,-2,-4,-6,-8,-10,-12,-14,-16},
        .thd2 = {1.5,-0.5,-2.5,-4.5,-6.5,-8.5,-10.5,-12.5,-14.5,-16.5},
        .thd3 = {1,-1,-3,-5,-7,-9,-11,-13,-15,-17}
    },

    .psap_ns_cfg = {
        .PSAP_NS_CHANGE_CNT = 400,
        .freq = {0, 1, 2, 5, 6, 10, 11, 16, 32, 48, 56, 61, 80, 96, 104, 128, 0}, //62.5Hz/sample
        .psap_ns_power_thd = 0.5,     //to judge quite environment
        .initial_status = 3,
    },

};