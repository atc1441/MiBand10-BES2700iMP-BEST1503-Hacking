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
#ifndef __APP_BTMAP_SMS_H__
#define __APP_BTMAP_SMS_H__

#ifdef BT_MAP_SUPPORT

#include "map_service.h"

#ifdef __cplusplus
extern "C" {
#endif

void bt_map_set_obex_over_rfcomm(void);
void bt_map_clear_obex_over_rfcomm(void);
void bt_map_client_test(const bt_bdaddr_t *remote);
bt_map_state_t bt_map_get_state(const bt_bdaddr_t *remote);

#define bes_bt_map_connect bt_map_connect
#define bes_bt_map_disconnect bt_map_disconnect
#define bes_bt_map_send_sms bt_map_send_sms
#define bes_bt_map_is_connected bt_map_is_connected
#define bes_bt_map_get_state bt_map_get_state
#ifdef __cplusplus
}
#endif

#endif /* BT_MAP_SUPPORT */

#endif /*__APP_BTMAP_SMS_H__*/

