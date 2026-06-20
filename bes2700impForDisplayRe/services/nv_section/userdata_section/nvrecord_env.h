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
#if defined(NEW_NV_RECORD_ENABLED)

#ifndef NVRECORD_ENV_H
#define NVRECORD_ENV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bluetooth_bt_api.h"
#include "me_api.h"
#include "nvrecord_extension.h"

#define NVRAM_ENV_MEDIA_LANGUAGE_DEFAULT (0)
#define NVRAM_ENV_TWS_MODE_DEFAULT (0xff)
#define NVRAM_ENV_FACTORY_TESTER_STATUS_DEFAULT (0xaabbccdd)
#define NVRAM_ENV_FACTORY_TESTER_STATUS_TEST_PASS (0xffffaa55)

int nv_record_env_init(void);
int nv_record_env_get(struct nvrecord_env_t **nvrecord_env);
int nv_record_env_set(struct nvrecord_env_t *nvrecord_env);
void nv_record_update_ibrt_info(uint32_t newMode,bt_bdaddr_t *ibrtPeerAddr);
void nv_record_update_tws_side(uint8_t side);
int nv_record_get_tws_side();
void nv_record_update_conn_mode(uint8_t support);
int nv_record_get_conn_mode();
void nv_record_set_tws_enable(void);
void nv_record_clean_tws_enable(void);
bool nv_record_tws_is_enabled(void);
int nv_record_set_tws_name(uint8_t* name, uint8_t length);
int nv_record_get_tws_name(uint8_t* name, uint8_t *length);
void nv_record_clean_tws_name(void);
bool nv_record_tws_name_is_valid(void);
void nvrecord_rebuild_system_env(struct nvrecord_env_t* pSystemEnv, bool isRebuildAll);
void nv_record_update_factory_tester_status(uint32_t status);
void nv_record_update_factory_le_rx_test_env(uint32_t to, uint16_t remote_tx_nb,uint8_t test_type,uint8_t rx_channel, uint8_t phy, uint8_t mod_idx);
void nv_record_update_factory_le_tx_test_env(uint32_t to, uint8_t test_type, uint8_t tx_channel, uint8_t data_len, uint8_t pkt_payload, uint8_t phy);
void nv_record_reset_factory_test_env(void);
void nv_record_update_factory_le_rx_test_result(bool test_done, uint16_t result);
void nv_record_factory_get_le_rx_test_result(bool* test_done, uint16_t* result);
void nv_record_update_factory_bt_nonsignaling_test_result(bool test_done, uint16_t result, uint16_t hec_nb,uint16_t crc_nb);
void nv_record_factory_get_bt_nonsignalint_rx_test_result(bool* test_done, uint16_t* result, uint16_t* hec_nb, uint16_t* crc_nb);
void nv_record_update_factory_bt_nonsignaling_env(uint32_t to, uint8_t test_type,
                                uint8_t hopping_mode, uint8_t whitening_mode, uint8_t tx_freq, uint8_t rx_freq, uint8_t power_level,
                                uint8_t lt_addr, uint8_t edr_enabled, uint8_t packet_type, uint8_t payload_pattern, uint16_t payload_length,
                                uint32_t tx_packet_num);
void nv_record_update_factory_tx_single_tone_env(uint8_t channel);
#ifdef __cplusplus
}
#endif
#endif
#endif // #if defined(NEW_NV_RECORD_ENABLED)
