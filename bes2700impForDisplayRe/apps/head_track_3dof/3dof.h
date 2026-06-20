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
 * Application layer function of head tracking 3dof algo with lsm6dsl sensor
 ****************************************************************************/
#ifndef __3DOF_H__
#define __3DOF_H__

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif

#ifdef __cplusplus
extern "C" {
#endif

void imusensor_init(void);

void head_track_algo_start(void);

void head_track_algo_pause(void);

void head_angle_reset(void);

void imu_data_set(float ax, float ay, float az, float gx, float gy, float gz, unsigned int timeStamp);

#ifdef __cplusplus
}
#endif

#endif