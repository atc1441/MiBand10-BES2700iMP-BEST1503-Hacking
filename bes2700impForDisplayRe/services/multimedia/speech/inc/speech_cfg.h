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
#ifndef __SPEECH_CFG_H__
#define __SPEECH_CFG_H__

// Speech process module
#include "speech_dc_filter.h"
#include "speech_iir_calibration.h"
#include "speech_fir_calibration.h"
#include "speech_aec.h"
#include "speech_aec2.h"
#include "echo_canceller.h"
#include "dual_mic_denoise.h"
#include "speech_2mic_ns2_denoise.h"
#include "speech_2mic_ns7.h"
#include "sensormic_denoise.h"
#include "leftright_denoise.h"
#include "speech_3mic_ns.h"
#include "speech_3mic_ns2.h"
#include "triple_mic_denoise3.h"
#include "speech_3mic_ns4.h"
#include "speech_fb_aec.h"
#include "far_field_speech_enhancement.h"
#include "speech_ns.h"
#include "lc_mmse_ns.h"
#include "lc_mmse_ns_float.h"
#include "ns3.h"
#include "speech_ns3_new_float.h"
#include "speech_ns4.h"
#include "speech_ns5.h"
#include "speech_1mic_ns.h"
#include "nn_ns.h"
#include "nn_ns2.h"
#include "dtln_denoise.h"
#include "wnr.h"
#include "VoiceActivityDetection.h"
#include "speech_noise_gate.h"
#include "compexp.h"
#include "multi_compexp.h"
#include "agc.h"
#include "speech_eq.h"
#include "speech_hw_eq.h"
#include "speech_dac_eq.h"
#include "speech_gain.h"
#include "speech_2mic_ns6.h"
#include "speech_2mic_preaf.h"
#include "speech_2mic_ns8.h"
#include "speech_2mic_ns9.h"
#include "SubBandBasedAEC.h"
#include "speech_dereverberation2.h"
// Tool module
#include "speech_utils.h"
#include "speech_ssat.h"
#include "speech_memory.h"
#include "iir_resample.h"
#include "frame_resize.h"
#include "ae_math.h"
// Other module
#include "noise_detection.h"
#include "noise_estimator.h"
#include "mmse_snr.h"
#include "aqe_kws.h"
#include "hear_fir2.h"
typedef struct {
    bool vad;
} sco_status_t;

#endif