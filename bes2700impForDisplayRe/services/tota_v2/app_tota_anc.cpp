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

#include "app_tota.h"
#include <stdio.h>
#include "cmsis_os.h"
#include "app_tota_cmd_code.h"
#include "anc_parse_data.h"
#include "app_tota_anc.h"
#include "bluetooth_bt_api.h"
#include "btapp.h"
#include "app_ibrt_middleware.h"
#include "hal_norflash.h"

#if defined(APP_ANC_TEST)
osTimerId app_check_send_synccmd_timer = NULL;
static void app_synccmd_timehandler(void const *param);
osTimerDef (APP_SYNCCMD, app_synccmd_timehandler);

#if 1
#define TOTA_ANC_LOG_DBG TOTA_LOG_DBG
#else
#define TOTA_ANC_LOG_DBG(level,...)
#endif

static void app_synccmd_timehandler(void const *param)
{
    // TODO:
    send_sync_cmd_to_tool();
}



APP_TOTA_CMD_RET_STATUS_E app_anc_tota_cmd_received(uint8_t* ptrData, uint32_t dataLength)
{
    TOTA_LOG_DBG(2,"[%s] length:%d", __func__, dataLength);
    TOTA_LOG_DBG(1,"[%s] data:", __func__);
    //TOTA_LOG_DUMP("0x%02x ", ptrData, dataLength);
    TOTA_LOG_DUMP("0x%02x ", ptrData, dataLength>20?20:dataLength);

    //APP_TOTA_CMD_PAYLOAD_T* pPayload = (APP_TOTA_CMD_PAYLOAD_T *)ptrData;
    // TODO:
    anc_handle_received_data(ptrData, dataLength);
    // TODO:
    if(get_send_sync_flag()==1)
    {
        osTimerStop(app_check_send_synccmd_timer);
    }
    return TOTA_NO_ERROR;
}

static bool is_connected;

/*-----------------------------------------------------------------------------*/

static void _tota_spp_connected(void);
static void _tota_spp_disconnected(void);
static void _tota_spp_tx_done(void);
static void _tota_spp_data_receive_handle(uint8_t * buf, uint16_t len);
/**/
//static void _audio_dump_control(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen);

/**/
static const app_tota_callback_func_t s_func = {
    _tota_spp_connected,
    _tota_spp_disconnected,
    _tota_spp_tx_done,
    _tota_spp_data_receive_handle
};

// static APP_TOTA_MODULE_E s_module = APP_TOTA_ANC;


void app_tota_anc_init()
{
    app_tota_callback_module_register(APP_TOTA_ANC, &s_func);
    // TODO:
    reset_programmer_state(&g_buf, &g_len);
    TOTA_LOG_DBG(1,"[%s] g_len: %d", __func__, g_len);

    //reset_programmer_state(&g_buf, &g_len);
    if (app_check_send_synccmd_timer == NULL)
        app_check_send_synccmd_timer = osTimerCreate (osTimer(APP_SYNCCMD), osTimerPeriodic, NULL);
}

static void _tota_spp_connected(void)
{
    // TODO:
    anc_data_buff_init();
    struct BT_DEVICE_T *curr_device = app_bt_get_device(0);
    app_ibrt_if_prevent_sniff_set((uint8_t *)&curr_device->remote, AVRCP_STATUS_CHANING);

    //add a send sync timer
    osTimerStop(app_check_send_synccmd_timer);
    osTimerStart(app_check_send_synccmd_timer, 2000);
    is_connected = true;
}

static void _tota_spp_disconnected(void)
{
    // TODO:
    anc_data_buff_deinit();

    osTimerStop(app_check_send_synccmd_timer);
    is_connected = false;
}

static void _tota_spp_tx_done(void)
{
    if ( is_connected  )
        // TODO:
        ;//bulk_read_done();
}

static void _tota_spp_data_receive_handle(uint8_t * buf, uint16_t len)
{
    app_anc_tota_cmd_received(buf, len);
}

extern uint32_t __anc_start[];
static void _anc_cmd_control(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen)
{
    TOTA_ANC_LOG_DBG(1,"[%s] funcCode: 0x%x, paramLen: %d", __func__, funcCode, paramLen);
    uint8_t resData[48] = {0};
    uint32_t resLen = 1;
    switch(funcCode) {
        case OP_TOTA_ANC_CONTROL_INIT_CMD:
        {
            reset_programmer_state(&g_buf, &g_len);

            anc_data_buff_init();

            /* ban bt sniff */
            struct BT_DEVICE_T *curr_device = app_bt_get_device(0);
            app_ibrt_if_prevent_sniff_set((uint8_t *)&curr_device->remote, AVRCP_STATUS_CHANING);
            hal_norflash_disable_protection(HAL_FLASH_ID_0);

            app_tota_send_data(funcCode, resData, resLen);
            break;
        }
        case OP_TOTA_ANC_CONTROL_DATA_CMD:
        {
            TOTA_LOG_DUMP("0x%02x ", ptrParam, paramLen>20?20:paramLen);
            anc_handle_received_data(ptrParam, paramLen);

            app_tota_send_data(funcCode, resData, resLen);
            break;
        }
        case OP_TOTA_ANC_CONTROL_DEINIT_CMD:
        {
            anc_data_buff_deinit();
            is_connected = false;

            app_tota_send_data(funcCode, resData, resLen);
            break;
        }
        case OP_TOTA_ANC_CONTROL_ADDR_CMD:
        {
            uint32_t audio_start_int = (uint32_t)__anc_start;
            resLen = sizeof(uint32_t);
            memcpy(resData, &audio_start_int, resLen);
            resData[resLen] = '\0';
            TOTA_LOG_DUMP("0x%02x ", resData, resLen);
            app_tota_send_data(funcCode, resData, resLen);
            break;
        }
        default:
        {
            resData[0] = 0x01;
            app_tota_send_data(funcCode, resData, resLen);
            break;
        }
    }
}

TOTA_COMMAND_TO_ADD(OP_TOTA_ANC_CONTROL_INIT_CMD, _anc_cmd_control, false, 0, NULL);
TOTA_COMMAND_TO_ADD(OP_TOTA_ANC_CONTROL_DATA_CMD, _anc_cmd_control, false, 0, NULL);
TOTA_COMMAND_TO_ADD(OP_TOTA_ANC_CONTROL_DEINIT_CMD, _anc_cmd_control, false, 0, NULL);
TOTA_COMMAND_TO_ADD(OP_TOTA_ANC_CONTROL_ADDR_CMD, _anc_cmd_control, false, 0, NULL);
#endif
