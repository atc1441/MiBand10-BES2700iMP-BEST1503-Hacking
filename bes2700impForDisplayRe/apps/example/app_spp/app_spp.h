/***************************************************************************
 *
 * Copyright 2023-2033 BES.
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

#ifndef __APP_SPP_EXAMPLE_H__
#define __APP_SPP_EXAMPLE_H__

typedef void (*APP_SPP_CLIENT_CALLBACK_IND)(uint8_t client_type, const bt_bdaddr_t *remote, bt_spp_event_t event, bt_spp_callback_param_t *param);
typedef void (*APP_SPP_SERVER_CALLBACK_IND)(uint8_t server_type, const bt_bdaddr_t *remote, bt_spp_event_t event, bt_spp_callback_param_t *param);

void app_spp_serial_port_client_open_by_uuid(const bt_bdaddr_t *remote_addr, uint16_t uuid, bool snoop_spp);

void app_spp_serial_port_client_open_by_port(const bt_bdaddr_t *remote_addr, uint8_t rfcomm_port, bool snoop_spp);

void app_spp_serial_port_server_init(void);

void app_spp_register_client_callback_handle(APP_SPP_CLIENT_CALLBACK_IND handle);

bool app_spp_serial_port_is_connected(const bt_bdaddr_t *remote);

void app_spp_close_serial_port(const bt_bdaddr_t *remote);

bool app_spp_serial_port_send_data(bt_bdaddr_t *remote, const uint8_t* ptrData, uint16_t length);

void app_spp_restore_serial_port_ctx(bt_bdaddr_t *bdaddr_p, uint64_t app_id);

void app_spp_main(const bt_bdaddr_t *addr);
#endif /* __APP_SPP_EXAMPLE_H__ */
