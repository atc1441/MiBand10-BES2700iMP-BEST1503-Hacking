/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef __HEAR_COMP_PROCESS_H__
#define __HEAR_COMP_PROCESS_H__

#ifdef __cplusplus
extern "C" {
#endif

float get_hear_compen_val_fir(int *test_buf, float *gain_buf, int num); //fir compensation alg
float get_hear_compen_val_multi_level(int *test_buf, float *gain_buf, int num); //iir compensation alg

#ifdef __cplusplus
}
#endif

#endif