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
#ifndef __APP_IBRT_UI_TEST_H__
#define __APP_IBRT_UI_TEST_H__
#include <stdint.h>

#if defined(IBRT) && defined(IBRT_UI)
#ifdef __cplusplus
extern "C" {
#endif

int app_ibrt_ui_v2_test_config_load(void *config);
void app_tws_ibrt_raw_ui_test_key_init();

#ifdef SPA_AUDIO_SEC
void app_tz_add_test_cmd_table(void);
#endif

int app_ibrt_raw_ui_test_cmd_handler_with_param(unsigned char *buf, unsigned char *param, unsigned int length);
void app_ibrt_ui_automate_test_cmd_handler(uint8_t group_code, uint8_t operation_code, uint8_t *param, uint8_t param_len);
void app_ibrt_auto_test_inform_cmd_received(uint8_t group_code, uint8_t operation_code);

//void app_ibrt_search_ui_gpio_key_handle(APP_KEY_STATUS *status, void *param);
WEAK void app_ibrt_initialize_nv_role_callback(void *config, void * record_env);


void app_tws_ibrt_ui_cmd_init(void);

void app_tws_ibrt_ui_cmd_deinit(void);
#ifdef __cplusplus
}
#endif

#endif

#endif /*__APP_IBRT_RAW_UI_TEST_H__ */
