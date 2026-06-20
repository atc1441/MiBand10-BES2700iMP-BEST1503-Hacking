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
#ifndef __APP_TWS_PROFILE_SYNC__
#define __APP_TWS_PROFILE_SYNC__

#include "bluetooth_bt_api.h"
#include "spp_api.h"
#include "app_tws_ibrt.h"

void app_tws_profile_data_sync(uint8_t *p_buff, uint16_t length);
void app_tws_parse_profile_data(ibrt_mobile_info_t *p_mobile_info, uint8_t *p_buff, uint32_t length);
uint32_t app_tws_profile_data_tx(ibrt_mobile_info_t *p_mobile_info,uint8_t flag,uint8_t *buf,uint16_t buf_len);
uint32_t app_tws_profile_data_rx(ibrt_mobile_info_t *p_mobile_info, uint8_t flag,uint8_t *buf,uint32_t length);
void app_tws_profile_data_save_temporarily(ibrt_mobile_info_t *p_mobile_info,uint8_t *p_buff, uint32_t length);
bool app_tws_profile_data_rx_needed(ibrt_mobile_info_t *p_mobile_info,uint8_t profile_flag);
bool app_tws_profile_data_rx_completed(uint8_t final_flag);
void app_tws_profile_rx_parse(ibrt_mobile_info_t *p_mobile_info);
uint8_t app_tws_profile_data_tx_filter(ibrt_mobile_info_t *p_mobile_info,uint64_t profile_mask,uint8_t data_array[][8]);
bool app_tws_profile_data_tx_allowed(ibrt_mobile_info_t *p_mobile_info);
bool app_tws_profile_connecting(ibrt_mobile_info_t *p_mobile_info);
uint64_t app_tws_profile_mapping_data_fragment(uint8_t flag);
void app_tws_profile_resume_a2dp_hfp(ibrt_mobile_info_t *p_mobile_info);
void app_tws_profile_disconnect_old_profiles(ibrt_mobile_info_t *p_mobile_info,uint8_t profile_flag,uint8_t device_id);
void app_tws_profile_remove_from_basic_profiles(bt_bdaddr_t *p_mobile_addr,uint64_t profile_id);
void app_tws_profile_data_rx_buf_init(ibrt_mobile_info_t *p_mobile_info);
void app_tws_profile_data_rx_buf_free(ibrt_mobile_info_t *p_mobile_info);


#endif

