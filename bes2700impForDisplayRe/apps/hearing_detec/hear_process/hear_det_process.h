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
#ifndef __HEAR_DET_PROCESS_H__
#define __HEAR_DET_PROCESS_H__

#ifdef __cplusplus
extern "C" {
#endif

int hear_rx_process(int *pcm_buf, int pcm_len); // playback RX process
void update_spk_calib_gain(float *input_buf); // update speaker calib gain based on SPK
int get_cur_step(void); // update test tone freq
float get_cur_amp(void); // update test tone amp
void reset_hear_det_para(void); // reset hearing detection tone para

#ifdef __cplusplus
}
#endif

#endif