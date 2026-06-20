/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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

#ifndef __ANC_FF_FIR_LMS_AEC_H__
#define __ANC_FF_FIR_LMS_AEC_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ANCFFFIRlmsAecSt_ ANCFFFIRlmsAecSt;

ANCFFFIRlmsAecSt *anc_fir_lms_aec_init(int sample_rate, int frame_size);
void anc_fir_lms_aec_destroy(ANCFFFIRlmsAecSt *st);
void anc_fir_lms_aec_process_f32(ANCFFFIRlmsAecSt *st, float *in, float *ref, float *out);

#ifdef __cplusplus
}
#endif

#endif
