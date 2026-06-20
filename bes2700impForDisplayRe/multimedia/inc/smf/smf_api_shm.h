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
#pragma once
#ifndef __SMF_API_SHM_H__
#define __SMF_API_SHM_H__
#include <stdint.h>
#include <stdbool.h>
#ifndef EXTERNC
#ifndef __cplusplus
#define EXTERNC
#else
#define EXTERNC extern "C"
#endif
#endif

EXTERNC void smf_register_shm(void* shm, int size, bool master);
EXTERNC void smf_shm_update();
EXTERNC void smf_shm_poweroff(uint8_t cpuid);
//EXTERNC bool smf_shm_sync_power(uint8_t cpuid, int timeout, int skip);
EXTERNC bool smf_shm_sync_ready(uint8_t cpuid, int timeout, int skip);
EXTERNC void smf_shm_poweron(uint8_t cpuid);
EXTERNC void smf_shm_ready();

#endif /* __SMF_PORTING_SHM_H__ */
