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
#include "string.h"
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "analog.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hwtimer_list.h"
#include "audioflinger.h"
#include "app_sensor_hub.h"
#include "app_thread.h"
#include "sens_msg.h"
#include "sensor_hub_core.h"
#include "sensor_hub_core_app.h"
#ifdef VOICE_DETECTOR_EN
#include "sensor_hub_vad_core.h"
#endif
#ifdef SENSOR_HUB_MINIMA
#include "sensor_hub_minima_core.h"
#endif
#ifdef AI_VOICE
#include "sensor_hub_core_app_ai.h"
#include "sensor_hub_core_app_ai_ota.h"
#endif
#ifdef CAPSENSOR_ENABLE
#include "touch_wear_core.h"
#endif

#if defined(SPA_AUDIO_ENABLE)
#include "spa_sens_main.h"
#endif
/*
 * sensor hub AT command receive
 */

#ifdef SENSOR_HUB_BSP_TEST
#include "uart_cmd_test.h"

static void app_sensor_hub_at_cmd_receive_handler(uint8_t *buf, uint16_t len)
{
    buf[len] = '\0';
    TRACE(0, "[AT command received]: \"%s\"", buf);
    uart_cmd_callback(buf, len);
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_AT_CMD,
                                "at cmd received",
                                NULL,
                                app_sensor_hub_at_cmd_receive_handler,
                                0,
                                NULL,
                                NULL,
                                NULL);

#endif /* SENSOR_HUB_BSP_TEST */

/*
 * SENSOR HUB CORE PING MSG
 ********************************************************
 */

static void app_sensor_hub_core_transmit_ping_handler(uint8_t* ptr, uint16_t len)
{
    app_core_bridge_send_data_without_waiting_rsp(MCU_SENSOR_HUB_TASK_CMD_PING, ptr, len);
}

static void app_sensor_hub_core_ping_mcu(void)
{
    app_core_bridge_send_cmd(MCU_SENSOR_HUB_TASK_CMD_PING, NULL, 0);
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_PING,
                                "ping mcu",
                                app_sensor_hub_core_transmit_ping_handler,
                                NULL,
                                0,
                                NULL,
                                NULL,
                                NULL);
/*
 * CORE BRIDGE DEMO MSG
 ********************************************************
 */

#ifdef CORE_BRIDGE_DEMO_MSG
#define CORE_BRIDGE_DEMO_REQ_DATA       0x55
#define CORE_BRIDGE_DEMO_REQ_RSP_DATA   0xAA

typedef struct
{
    uint8_t reqData;
} APP_SENSOR_HUB_CORE_DEMO_REQ_T;

typedef struct
{
    uint8_t rspData;
} APP_SENSOR_HUB_CORE_DEMO_RSP_T;

static void app_sensor_hub_core_transmit_demo_no_rsp_cmd_handler(uint8_t* ptr, uint16_t len)
{
    app_core_bridge_send_data_without_waiting_rsp(MCU_SENSOR_HUB_TASK_CMD_DEMO_REQ_NO_RSP, ptr, len);
}

static void app_sensor_hub_core_demo_no_rsp_cmd_received_handler(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "Get demo no rsp command from mcu:");
    DUMP8("%02x ", ptr, len);
}

static void app_sensor_hub_core_demo_no_rsp_cmd_tx_done_handler(uint16_t cmdCode,
    uint8_t* ptr, uint16_t len)
{
    TRACE(0, "cmdCode 0x%x tx done", cmdCode);
    DUMP8("%02x ", ptr, len);
}

void app_sensor_hub_core_send_demo_req_no_rsp(void)
{
    APP_SENSOR_HUB_CORE_DEMO_REQ_T req;
    req.reqData = CORE_BRIDGE_DEMO_REQ_DATA;
    app_core_bridge_send_cmd(MCU_SENSOR_HUB_TASK_CMD_DEMO_REQ_NO_RSP,
        (uint8_t *)&req, sizeof(req));
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_DEMO_REQ_NO_RSP,
                                "demo no rsp req to mcu",
                                app_sensor_hub_core_transmit_demo_no_rsp_cmd_handler,
                                app_sensor_hub_core_demo_no_rsp_cmd_received_handler,
                                0,
                                NULL,
                                NULL,
                                app_sensor_hub_core_demo_no_rsp_cmd_tx_done_handler);

static void app_sensor_hub_core_transmit_demo_wait_rsp_cmd_handler(uint8_t* ptr, uint16_t len)
{
    app_core_bridge_send_data_without_waiting_rsp(MCU_SENSOR_HUB_TASK_CMD_DEMO_REQ_WITH_RSP, ptr, len);
}

static void app_sensor_hub_core_demo_wait_rsp_cmd_received_handler(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "Get demo with rsp command from mcu:");
    DUMP8("%02x ", ptr, len);
    APP_SENSOR_HUB_CORE_DEMO_RSP_T rsp;
    rsp.rspData = CORE_BRIDGE_DEMO_REQ_RSP_DATA;
    app_core_bridge_send_rsp(MCU_SENSOR_HUB_TASK_CMD_DEMO_REQ_WITH_RSP, (uint8_t *)&rsp, sizeof(rsp));
}

static void app_sensor_hub_core_demo_wait_rsp_cmd_wait_rsp_timeout(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "sensor core waiting for rsp to demo cmd timeout.");
}

static void app_sensor_hub_core_demo_wait_rsp_cmd_rsp_received_handler(uint8_t* ptr, uint16_t len)
{
    APP_SENSOR_HUB_CORE_DEMO_RSP_T* ptrRsp = (APP_SENSOR_HUB_CORE_DEMO_RSP_T *)ptr;
    TRACE(0, "Sensor hub gets rsp 0x%x from mcu:", ptrRsp->rspData);
}

static void app_sensor_hub_core_demo_with_rsp_cmd_tx_done_handler(uint16_t cmdCode,
    uint8_t* ptr, uint16_t len)
{
    TRACE(0, "cmdCode 0x%x tx done", cmdCode);
}

void app_sensor_hub_core_send_demo_req_with_rsp(void)
{
    APP_SENSOR_HUB_CORE_DEMO_REQ_T req;
    req.reqData = CORE_BRIDGE_DEMO_REQ_DATA;
    app_core_bridge_send_cmd(MCU_SENSOR_HUB_TASK_CMD_DEMO_REQ_WITH_RSP,
        (uint8_t *)&req, sizeof(req));
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_DEMO_REQ_WITH_RSP,
                                "demo with rsp req to mcu",
                                app_sensor_hub_core_transmit_demo_wait_rsp_cmd_handler,
                                app_sensor_hub_core_demo_wait_rsp_cmd_received_handler,
                                APP_CORE_BRIDGE_DEFAULT_WAIT_RSP_TIMEOUT_MS,
                                app_sensor_hub_core_demo_wait_rsp_cmd_wait_rsp_timeout,
                                app_sensor_hub_core_demo_wait_rsp_cmd_rsp_received_handler,
                                app_sensor_hub_core_demo_with_rsp_cmd_tx_done_handler);

static void app_sensor_hub_core_transmit_demo_instant_req_handler(uint8_t* ptr, uint16_t len)
{
    app_core_bridge_send_instant_cmd_data(MCU_SENSOR_HUB_INSTANT_CMD_DEMO_REQ,
        ptr, len);
}

static void app_sensor_hub_core_demo_instant_req_handler(uint8_t* ptr, uint16_t len)
{
    // for test purpose, we add log print here.
    // but as instant cmd handler will be directly called in intersys irq context,
    // for realistic use, should never do log print
    TRACE(0, "Get demo instant req command from mcu:");
    DUMP8("%02x ", ptr, len);
}

CORE_BRIDGE_INSTANT_COMMAND_TO_ADD(MCU_SENSOR_HUB_INSTANT_CMD_DEMO_REQ,
                                app_sensor_hub_core_transmit_demo_instant_req_handler,
                                app_sensor_hub_core_demo_instant_req_handler);

#endif /* CORE_BRIDGE_DEMO_MSG */

#ifdef VPU_CFG_ON_SENSOR_HUB
#include "vpu_cfg.h"

static void app_sensor_hub_core_transmit_vpu_wait_rsp_cmd_handler(uint8_t* ptr, uint16_t len)
{
    app_core_bridge_send_data_without_waiting_rsp(MCU_SENSOR_HUB_TASK_CMD_VPU, ptr, len);
}

static void app_sensor_hub_core_vpu_wait_rsp_cmd_received_handler(uint8_t* ptr, uint16_t len)
{
    int32_t ret = 0;
    bool vpu_enable = ptr[0];

    if (vpu_enable) {
        TRACE(0, "Turn on VPU.");
        ret = vpu_cfg_init();
    } else {
        TRACE(0, "Turn off VPU.");
        ret = vpu_cfg_deinit();
    }

    app_core_bridge_send_rsp(MCU_SENSOR_HUB_TASK_CMD_VPU, (uint8_t *)&ret, sizeof(ret));
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_VPU,
                                "cfg vpu on sensor hub core",
                                app_sensor_hub_core_transmit_vpu_wait_rsp_cmd_handler,
                                app_sensor_hub_core_vpu_wait_rsp_cmd_received_handler,
                                APP_CORE_BRIDGE_DEFAULT_WAIT_RSP_TIMEOUT_MS,
                                NULL,
                                NULL,
                                NULL);
#endif

/*
 * SENSOR HUB TIMER APPS FOR PING TO MCU
 ********************************************************
 */

#define APP_SENSOR_HUB_PING_MCU_TIMER_PERIOD_MS    (10000)

static void sensor_hub_ping_mcu_timer_handler(void const *param);
osTimerDef(sensor_hub_ping_mcu_timer, sensor_hub_ping_mcu_timer_handler);
static osTimerId sensor_hub_ping_mcu_timer_id = NULL;

static void sensor_hub_ping_mcu_timer_handler(void const *param)
{
    app_sensor_hub_core_ping_mcu();
}

static void app_sensor_hub_core_timer_init(void)
{
    TRACE(0, "%s:", __func__);

    if (sensor_hub_ping_mcu_timer_id == NULL) {
        sensor_hub_ping_mcu_timer_id = osTimerCreate(osTimer(sensor_hub_ping_mcu_timer), osTimerPeriodic, NULL);
    }
    if (sensor_hub_ping_mcu_timer_id != NULL) {
        osTimerStart(sensor_hub_ping_mcu_timer_id, APP_SENSOR_HUB_PING_MCU_TIMER_PERIOD_MS);
    }
}

void sensor_hub_core_app_init(void)
{


#if defined(VOICE_DETECTOR_EN) || defined(AI_VOICE) \
//    || defined(CAPSENSOR_ENABLE)

    af_open();
    app_os_init();
#endif

    app_core_bridge_init();

    sensor_hub_core_register_rx_irq_handler(app_core_bridge_data_received);

    sensor_hub_core_register_tx_done_irq_handler(app_core_bridge_data_tx_done);

    app_sensor_hub_core_ping_mcu();

    app_sensor_hub_core_timer_init();

#ifdef SENSOR_HUB_MINIMA
    app_sensor_hub_minima_init();
#endif
#ifdef VOICE_DETECTOR_EN
    app_sensor_hub_core_vad_init();
#endif
#ifdef AI_VOICE
    app_sensor_hub_core_data_update_init();
#endif
#ifdef CAPSENSOR_ENABLE
    cap_sensor_core_thread_init();
#endif
#if defined(SPA_AUDIO_ENABLE)
    spatial_audio_sens_process_init_if(0,0,0,0,NULL);
#endif
}

