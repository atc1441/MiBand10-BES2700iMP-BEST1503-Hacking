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
#ifndef __APP_TOTA_H__
#define __APP_TOTA_H__

#include "app_tota_cmd_code.h"
#include "app_tota_cmd_handler.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Type of the tota module
 *
 */
typedef enum
{
    APP_TOTA_AUDIO_DUMP = 0,
    APP_TOTA_ANC,

    APP_TOTA_MODULE_NONE = 0xFF
} APP_TOTA_MODULE_E;


typedef struct{
    void (*connected_cb)(void);
    void (*disconnected_cb)(void);
    void (*tx_done_cb)(void);
    void (*rx_cb)(uint8_t * buf, uint16_t len);
}app_tota_callback_func_t;

void app_tota_callback_module_register(APP_TOTA_MODULE_E module,
                        const app_tota_callback_func_t *tota_callback_func);
void app_tota_callback_module_set(APP_TOTA_MODULE_E module);
APP_TOTA_MODULE_E tota_callback_module_get();


void app_tota_init(void);
void app_tota_payload_set(APP_TOTA_CMD_PAYLOAD_T* buf);

void app_tota_handle_received_data(uint8_t* buffer, uint16_t maxBytes);
void app_tota_ble_disconnected(void);

/* interface */
bool is_tota_connected();
uint8_t tota_ble_get_conidx(void);

/**/
bool app_tota_send(uint8_t * pdata, uint16_t dataLen, APP_TOTA_CMD_CODE_E opCode );
bool app_tota_send_data(APP_TOTA_CMD_CODE_E opCode, uint8_t * data, uint32_t dataLen);
bool app_tota_send_rsp(APP_TOTA_CMD_CODE_E rsp_opCode, APP_TOTA_CMD_RET_STATUS_E rsp_status, uint8_t * pdata, uint16_t dataLen);

void tota_printf(const char * format, ...);
void tota_print(const char * format, ...);
bool app_tota_if_customers_access_valid(uint8_t access_code);
#if defined(OTA_OVER_TOTA_ENABLED)
void app_ota_over_tota_receive_data(uint8_t* ota_packet, uint32_t ota_packet_len);
void ota_spp_tota_send_data(uint8_t* ptrData, uint16_t length);
void ota_tota_send_notification(uint8_t* ptrData, uint16_t length);
#endif

#ifdef TOTA_FACTORY_USED
bool is_in_tota_mode(void);
void set_in_tota_mode(bool en);
void reboot_to_enter_tota_mode(void);
#endif

#ifdef __cplusplus
}
#endif

#endif

