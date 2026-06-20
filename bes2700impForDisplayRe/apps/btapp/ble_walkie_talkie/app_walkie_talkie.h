/**
 *
 * @copyright Copyright (c) 2015-2022 BES Technic.
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
 */

#ifndef __APP_WALKIE_TALKIE__H__
#define __APP_WALKIE_TALKIE__H__

#include "app_walkie_talkie_key_handler.h"

void app_walkie_talkie_init();

void app_walkie_talkie_handle_event(APP_W_T_KEY_EVENT_E event);

void app_walkie_talkie_exit();

// void app_walkie_talkie_reg_state_changed_callback(walkie_talkie_state_changed cb);

#endif /* __APP_WALKIE_TALKIE__H__ */
