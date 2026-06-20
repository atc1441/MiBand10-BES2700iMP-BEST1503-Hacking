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
#ifndef __APP_IBRT_OTA_CMD__
#define __APP_IBRT_OTA_CMD__

#define RESEND_TIME 2

#ifndef OTA_TWS_INFO_SIZE
#define OTA_TWS_INFO_SIZE 128
#endif

extern uint32_t ibrt_ota_cmd_type;
extern uint32_t twsBreakPoint;
//extern uint8_t ibrt_connect_slave;
extern uint8_t errOtaCode;

#ifdef __GMA_OTA_TWS__
extern uint8_t gma_start_ota;
extern bool gma_crc;
#endif

#if defined(BES_OTA)
void app_ibrt_ota_cache_slave_info(uint8_t typeCode, uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_ota_get_version_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_get_version_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_get_version_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_get_version_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_ota_select_side_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_select_side_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_select_side_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_select_side_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_ota_bp_check_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_bp_check_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_bp_check_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_bp_check_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_ota_start_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_start_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_start_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_start_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_ota_config_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_config_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_config_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_config_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_ota_segment_crc_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_segment_crc_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_segment_crc_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_segment_crc_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_ota_image_crc_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_image_crc_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_image_crc_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_image_crc_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_ota_image_overwrite_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_image_overwrite_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_image_overwrite_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_image_overwrite_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_ota_set_user_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_set_user_cmd_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_set_user_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_set_user_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_ota_get_ota_version_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_get_ota_version_cmd_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_get_ota_version_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_get_ota_version_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_ota_start_role_switch_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_start_role_switch_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_ota_mobile_disconnected_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_mobile_disconnected_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_ota_update_rd_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_update_rd_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_ota_image_buff_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_ota_image_buff_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
#endif

#ifdef __GMA_OTA_TWS__
void app_ibrt_gmaOta_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_gmaOta_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_gmaOta_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_gmaOta_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
#endif
#if defined(BISTO_ENABLED) || defined(__MMA_VOICE__) && !defined(FREEMAN_ENABLED_STERO) 
void app_ibrt_common_ota_cmd_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_common_ota_cmd_received(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
#endif

#endif
