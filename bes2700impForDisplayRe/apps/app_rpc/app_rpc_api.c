/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include "cmsis.h"
#include "cmsis_os.h"
#include "plat_types.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"
#include "hal_mcu2sens.h"
#include "hal_sys2bth.h"
#include "app_rpc_api.h"
#include "rpc_rx_thread.h"

#ifdef APP_RPC_BTH_DSP_EN
#include "rpc_bth_dsp.h"
#endif

#ifdef APP_RPC_BTH_M55_EN
#include "app_dsp_m55.h"
#endif

#ifdef APP_RPC_MCU_SENSOR_EN
#include "app_sensor_hub.h"
#endif

int32_t app_rpc_send_data_no_rsp(enum APP_RPC_CORE_T core, uint16_t cmdcode, uint8_t *p_buff, uint16_t length)
{
    int32_t ret = RPC_RES_FAILD;
    switch (core)
    {
        case APP_RPC_CORE_BTH_M55:
#ifdef APP_RPC_BTH_M55_EN
            app_dsp_m55_bridge_send_data_without_waiting_rsp(cmdcode, p_buff, length);
            ret = RPC_RES_SUCCESS;
#endif
            break;
        case APP_RPC_CORE_BTH_DSP:
#ifdef APP_RPC_BTH_DSP_EN
            ret = rpc_bth_dsp_send_data_no_rsp(cmdcode, p_buff, length);
            break;
#endif

        case APP_RPC_CORE_MCU_SENSOR:
#ifdef APP_RPC_MCU_SENSOR_EN
            app_core_bridge_send_data_without_waiting_rsp(cmdcode, p_buff, length);
            ret = RPC_RES_SUCCESS;
            break;
#endif
        case APP_RPC_CORE_DSP_M55:
            break;
        default:
            break;
    }
    return ret;
}

int32_t app_rpc_send_data_waiting_rsp(enum APP_RPC_CORE_T core, uint16_t cmdcode, uint8_t *p_buff, uint16_t length)
{
    int32_t ret = RPC_RES_FAILD;
    switch (core)
    {
        case APP_RPC_CORE_BTH_M55:
#ifdef APP_RPC_BTH_M55_EN
            app_dsp_m55_bridge_send_data_with_waiting_rsp(cmdcode, p_buff, length);
            ret = RPC_RES_SUCCESS;
#endif
            break;
#ifdef APP_RPC_BTH_DSP_EN
        case APP_RPC_CORE_BTH_DSP:
            ret = rpc_bth_dsp_send_data_wait_rsp(cmdcode, p_buff, length);
            break;
#endif

        case APP_RPC_CORE_MCU_SENSOR:
#ifdef APP_RPC_MCU_SENSOR_EN
            app_core_bridge_send_data_with_waiting_rsp(cmdcode, p_buff, length);
            ret = RPC_RES_SUCCESS;
            break;
#endif
        case APP_RPC_CORE_DSP_M55:
            break;
        default:
            break;
    }
    return ret;
}

int32_t app_rpc_send_cmd(enum APP_RPC_CORE_T core, uint16_t cmd_code, uint8_t *p_buff, uint16_t length)
{
    int32_t ret = RPC_RES_FAILD;
    switch (core)
    {
        case APP_RPC_CORE_BTH_M55:
#ifdef APP_RPC_BTH_M55_EN
            ret = app_dsp_m55_bridge_send_cmd(cmd_code, p_buff, length);
#endif
            break;
        case APP_RPC_CORE_BTH_DSP:
#ifdef APP_RPC_BTH_DSP_EN
            ret = rpc_bth_dsp_send_cmd(cmd_code, p_buff, length);
            break;
#endif

        case APP_RPC_CORE_MCU_SENSOR:
#ifdef APP_RPC_MCU_SENSOR_EN
            ret = app_core_bridge_send_cmd(cmd_code, p_buff, length);
            break;
#endif
        case APP_RPC_CORE_DSP_M55:
            break;
        default:
            break;
    }
    
   return ret;
}

int32_t app_rpc_send_cmd_rsp(enum APP_RPC_CORE_T core, uint16_t rsp_code, uint8_t *p_buff, uint16_t length)
{
    int ret = RPC_RES_FAILD;
    switch (core)
    {
        case APP_RPC_CORE_BTH_M55:
#ifdef APP_RPC_BTH_M55_EN
            ret = app_dsp_m55_bridge_send_rsp(rsp_code, p_buff, length);
#endif
            break;
        case APP_RPC_CORE_BTH_DSP:
#ifdef APP_RPC_BTH_DSP_EN
            ret = rpc_bth_dsp_send_cmd_rsp(rsp_code, p_buff, length);
#endif

        case APP_RPC_CORE_MCU_SENSOR:
#ifdef APP_RPC_MCU_SENSOR_EN
            ret = app_core_bridge_send_rsp(rsp_code, p_buff, length);
            break;
#endif
            break;
        case APP_RPC_CORE_DSP_M55:
            break;
        default:
            break;
    }

   return ret;
}

void app_rpc_ctx_init(enum APP_RPC_CORE_T core)
{
    switch (core)
    {
        case APP_RPC_CORE_BTH_M55:
#ifdef APP_RPC_BTH_M55_EN
            app_dsp_m55_bridge_init();
#endif
            break;
        case APP_RPC_CORE_BTH_DSP:
#ifdef APP_RPC_BTH_DSP_EN
            rpc_bth_dsp_ctx_init();
            rpc_rx_thread_init();
#endif
            break;
        case APP_RPC_CORE_DSP_M55:
            break;
        default:
            break;
    }
}

void app_rpc_ctx_deinit(enum APP_RPC_CORE_T core)
{
    switch (core)
    {
        case APP_RPC_CORE_BTH_M55:
#ifdef APP_RPC_BTH_M55_EN
            // deinit
#endif
            break;
        case APP_RPC_CORE_BTH_DSP:
#ifdef APP_RPC_BTH_DSP_EN
            rpc_bth_dsp_ctx_deinit();
#endif
            break;
        case APP_RPC_CORE_DSP_M55:
            break;
        default:
            break;
    }
}