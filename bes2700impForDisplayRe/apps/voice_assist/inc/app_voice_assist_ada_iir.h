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
#ifndef __APP_VOICE_ASSIST_ADA_IIR_H__
#define __APP_VOICE_ASSIST_ADA_IIR_H__

#include "anc_ada_iir.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t app_voice_assist_ada_iir_init(void);

int32_t app_voice_assist_ada_iir_open(void);

int32_t app_voice_assist_ada_iir_close(void);

int32_t app_voice_assist_ada_iir_reset(void);


#define ADA_IIR_SR (16000)
#define ADA_IIR_PARTICLES (52)
#define ADA_IIR_NFFT (1024)

#ifdef LOW_FS_8k
#undef ADA_IIR_SR
#define ADA_IIR_SR (8000)
#endif

#ifdef LITTLE_FF
#undef ADA_IIR_PARTICLES
#define ADA_IIR_PARTICLES (30)
#endif

#ifdef RESOLUTION_LOW
#undef ADA_IIR_NFFT
#define ADA_IIR_NFFT (512)
#elif (defined RESOLUTION_2048)
#undef ADA_IIR_NFFT
#define ADA_IIR_NFFT (2048)
#endif

#ifdef __cplusplus
}
#endif

#endif
