/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef __APP_BT_PBAP_H__
#define __APP_BT_PBAP_H__
#include "bt_common_define.h"
#ifdef BT_PBAP_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

struct btif_pbap_channel_t;

void app_bt_pbap_init(void);

bool app_bt_pbap_is_connected(struct btif_pbap_channel_t *pbap_chan);

void app_bt_connect_pbap_profile(bt_bdaddr_t *remote);

void app_bt_disconnect_pbap_profile(struct btif_pbap_channel_t *pbap_chan);

void app_bt_pbap_send_obex_disconnect_req(struct btif_pbap_channel_t *pbap_chan);

void app_bt_pbap_send_abort_req(struct btif_pbap_channel_t *pbap_chan);

void app_bt_pbap_get_phonebook_size(struct btif_pbap_channel_t *pbap_chan, const char* phonebook_object_path_name);

void app_bt_pbap_pull_single_phonebook(struct btif_pbap_channel_t *pbap_chan, const char* phonebook_object_path_name, uint16_t pb_index);

void app_bt_pbap_client_test(void);
#ifdef __cplusplus
}
#endif
#endif /* BT_PBAP_SUPPORT */
#endif /* __APP_BT_PBAP_H__ */
