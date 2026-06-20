/***************************************************************************
 *
 * Copyright (c) 2015-2023 BES Technic
 *
 * Authored by BES CD team (Blueelf Prj).
 *
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
#ifndef __GAF_TMAS_INC__
#define __GAF_TMAS_INC__

#include "gaf_cfg.h"
#include "gaf_log.h"

#include "tma_common.h"

/**
 * @brief Telephony and Media Audio server initilization
 *
 * @param  role_bf     Role bitfield @see enum tmap_role_bf
 * @param  pref_mtu_size
 *                     Preferred MTU size
 *
 * @return int         status
 */
int tmas_init(uint16_t role_bf, uint16_t pref_mtu_size);

/**
 * @brief Telephony and Media Audio server deinitilization
 *
 * @return int         status
 */
int tmas_deinit(void);

#endif /// __GAF_TMAS_INC__
