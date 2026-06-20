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

#ifndef __APP_WALKIE_TALKIE_HALF_DUPLEX_H__
#define __APP_WALKIE_TALKIE_HALF_DUPLEX_H__


void app_walkie_talkie_half_dup_init(const walkie_bdaddr* filter_addr_list,uint8_t list_size,uint8_t work_mode);

void app_walkie_talkie_half_dup_handle_key_event(APP_W_T_KEY_EVENT_E event);

#endif /* __APP_WALKIE_TALKIE_HALF_DUPLEX_H__ */

