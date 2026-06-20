/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef __BES_RATE_TEST_API_H__
#define __BES_RATE_TEST_API_H__

#ifdef BLE_RATE_TEST_SERVER
#include "app_rate_tests.h"
#endif

#ifdef BLE_RATE_TEST_CLIENT
#include "app_rate_testc.h"
#endif

#if (GATT_RATE_TESTS)
typedef void(*bes_ble_rate_test_server_disconnected_done_t)(uint8_t conidx);
typedef void(*bes_ble_rate_test_server_connected_done_t)(uint8_t conidx);
typedef void(*bes_ble_rate_test_server_conn_param_update_done_t)(uint8_t conidx, uint16_t interval);
typedef void(*bes_ble_rate_test_server_tx_done_t)(void);

void bes_ble_rate_test_server_send_data_via_notification(uint8_t* ptrData, uint32_t length);

void bes_ble_rate_test_server_send_data_via_intification(uint8_t* ptrData, uint32_t length);

void bes_ble_rate_test_server_register_connected_done(bes_ble_rate_test_server_connected_done_t callback);

void bes_ble_rate_test_server_register_conn_param_update_done(bes_ble_rate_test_server_conn_param_update_done_t callback);

void bes_ble_rate_test_server_register_tx_done(bes_ble_rate_test_server_tx_done_t callback);

#endif /* GATT_RATE_TESTS */

#if (GATT_RATE_TESTC)
typedef void(*bes_ble_rate_test_client_disconnected_done_t)(uint8_t conidx);
typedef void(*bes_ble_rate_test_client_connected_done_t)(uint8_t conidx);
typedef void(*bes_ble_rate_test_client_conn_param_update_done_t)(uint8_t conidx, uint16_t interval);
typedef void(*bes_ble_rate_test_client_tx_done_t)(void);

void bes_ble_rate_test_client_register_connected_done(bes_ble_rate_test_client_connected_done_t callback);
void bes_ble_rate_test_client_discover(uint8_t conidx);
void bes_ble_rate_test_client_register_conn_param_update_done(bes_ble_rate_test_client_conn_param_update_done_t callback);
void bes_ble_rate_test_client_register_tx_done(bes_ble_rate_test_client_tx_done_t callback);
void bes_ble_rate_test_client_send_data_via_write_command(uint8_t* ptrData, uint32_t length);
#endif /* GATT_RATE_TESTC */

#endif