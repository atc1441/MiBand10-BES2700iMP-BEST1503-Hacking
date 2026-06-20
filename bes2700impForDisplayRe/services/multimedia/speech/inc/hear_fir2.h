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
#ifndef __HEAR_FIR2_H__
#define __HEAR_FIR2_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct HearFir2State_;
typedef struct HearFir2State_ HearFir2State;

float hear_val2db(float val);
HearFir2State *HearFir2_create(int sample_rate);
void HearFir2_destroy(HearFir2State *st);
float HearFir2_process(HearFir2State *st, float* filer_coef, float* hear_result);

#ifdef __cplusplus
}
#endif

#endif
