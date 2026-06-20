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
#ifndef __APP_HFP_AG_ROLE_H__
#define __APP_HFP_AG_ROLE_H__
#include "hfp_api.h"
#ifdef BT_HFP_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

typedef void (*app_hfp_ag_uart_test_function_handle)(void);
typedef struct
{
    const char* string;
    app_hfp_ag_uart_test_function_handle function;
} app_hfp_ag_uart_handle_t;
app_hfp_ag_uart_test_function_handle app_hfp_ag_test_find_uart_handle(unsigned char* buf);

void app_hfp_ag_uart_cmd_init(void);
uint32_t app_hfp_ag_uart_cmd_callback(unsigned char *buf, uint32_t len);
int app_hfp_ag_uart_cmd_handler(unsigned char *buf, unsigned int length);

int app_hfp_ag_battery_report(uint8_t level);

void app_hfp_ag_create_audio_link(btif_hf_channel_t* hf_channel);

void app_hfp_ag_disc_audio_link(btif_hf_channel_t* hf_channel);

void app_hfp_ag_toggle_audio_link(void);

#ifdef __cplusplus
}
#endif
#endif /* BT_HFP_SUPPORT */
#endif /* __APP_HFP_AG_ROLE_H__ */

