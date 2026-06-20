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
#ifndef CHIP_SUBSYS_SENS
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "app_sensor_hub.h"
#include "sensor_hub.h"
#include "mcu_sensor_hub_app.h"
#include "mcu_sensor_hub_app_ai.h"

#define APP_MCU_SENSOR_HUB_WATCH_DOG_TIME_MS    15000

static uint32_t app_mcu_sensor_hub_last_ping_ms = 0;

static bool app_sensor_hub_inited = false;
static bool app_sensor_hub_ping_received = false;

static bool app_mcu_sensor_hub_wdog_enable = false;

typedef struct
{
    void (*sensor_hub_reset_cb)(enum APP_SENSOR_HUB_RESET_REASON reason);
    void (*sensor_hub_boot_done_cb)(enum APP_SENSOR_HUB_RESET_T boot_case);
} SENSOR_HUB_INFO_CB_T;

static SENSOR_HUB_INFO_CB_T *sensor_hub_reset_cb = NULL;

void app_custom_register_sensor_hub_callback(SENSOR_HUB_INFO_CB_T *cb)
{
    sensor_hub_reset_cb = cb;
}

void app_mcu_sensor_hub_enable_watch_dog(void)
{
    app_mcu_sensor_hub_last_ping_ms = GET_CURRENT_MS();
    app_mcu_sensor_hub_wdog_enable = true;
}

void app_mcu_sensor_hub_disable_watch_dog(void)
{
    app_mcu_sensor_hub_wdog_enable = false;
}

void app_mcu_sensor_hub_watch_dog_polling_handler(void)
{
    bool isSensorHubWdTimeout = false;
    uint32_t currentMs = GET_CURRENT_MS();
    uint32_t passedTimerMs;

    if (!app_mcu_sensor_hub_wdog_enable) {
        return;
    }

    if (currentMs >= app_mcu_sensor_hub_last_ping_ms)
    {
        passedTimerMs = currentMs - app_mcu_sensor_hub_last_ping_ms;
    }
    else
    {
        passedTimerMs = TICKS_TO_MS(0xFFFFFFFF) - app_mcu_sensor_hub_last_ping_ms +
            currentMs + 1;
    }

    if (passedTimerMs > APP_MCU_SENSOR_HUB_WATCH_DOG_TIME_MS)
    {
        if (app_mcu_sensor_hub_wdog_enable) {
            isSensorHubWdTimeout = true;
        }
    }

    if (isSensorHubWdTimeout)
    {
        TRACE(0, "currentMs %d latest ping %d passedTimerMs %d",
                        currentMs, app_mcu_sensor_hub_last_ping_ms, passedTimerMs);
        TRACE(0, "sensor hub watch dog time-out!");
        if (sensor_hub_reset_cb &&
            (NULL != sensor_hub_reset_cb->sensor_hub_reset_cb))
        {
            sensor_hub_reset_cb->sensor_hub_reset_cb(APP_SENSOR_HUB_WDOG_TIME_OUT);
        }
        app_mcu_reset_sesor_hub();
        if (sensor_hub_reset_cb &&
            (NULL != sensor_hub_reset_cb->sensor_hub_boot_done_cb))
        {
            sensor_hub_reset_cb->sensor_hub_boot_done_cb(APP_SENSOR_HUB_RESET);
        }
    }
}

/*
 * master cpu AT command send
 */

#ifdef SENSOR_HUB_BSP_TEST

static void app_sensor_hub_at_cmd_send_handler(uint8_t *ptr, uint16_t len)
{
    TRACE(0, "master cpu has sended AT command to sensor hub");
    app_core_bridge_send_data_without_waiting_rsp(MCU_SENSOR_HUB_TASK_CMD_AT_CMD, ptr, len);
}

void app_sensor_hub_at_cmd_send(uint8_t *buf, uint16_t len)
{
    TRACE(0, "[AT command sent]: \"%s\"", buf);
    app_core_bridge_send_cmd(MCU_SENSOR_HUB_TASK_CMD_AT_CMD, buf, len);
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_AT_CMD,
                                "at cmd sended",
                                app_sensor_hub_at_cmd_send_handler,
                                NULL,
                                0,
                                NULL,
                                NULL,
                                NULL);

#endif /* SENSOR_HUB_BSP_TEST */

static void app_mcu_sensor_hub_ping_received_handler(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "mcu gets ping from sensorhub core.");
    app_mcu_sensor_hub_last_ping_ms = GET_CURRENT_MS();
    app_sensor_hub_ping_received = true;
#ifdef VAD_IF_TEST
    hal_sysfreq_print_user_freq();
#endif
    if (false == app_sensor_hub_is_inited()) {
        app_sensor_hub_inited = true;
    }
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_PING,
                                "ping mcu",
                                NULL,
                                app_mcu_sensor_hub_ping_received_handler,
                                0,
                                NULL,
                                NULL,
                                NULL);

#ifdef CORE_BRIDGE_DEMO_MSG

#define CORE_BRIDGE_DEMO_MCU_REQ_DATA       0x55
#define CORE_BRIDGE_DEMO_MCU_REQ_RSP_DATA   0xAA

typedef struct
{
    uint8_t reqData;
} APP_MCU_SENSOR_HUB_DEMO_REQ_T;

typedef struct
{
    uint8_t rspData;
} APP_MCU_SENSOR_HUB_DEMO_RSP_T;

static void app_mcu_sensor_hub_transmit_demo_no_rsp_cmd_handler(uint8_t* ptr, uint16_t len)
{
    app_core_bridge_send_data_without_waiting_rsp(MCU_SENSOR_HUB_TASK_CMD_DEMO_REQ_NO_RSP, ptr, len);
}

static void app_mcu_sensor_hub_demo_no_rsp_cmd_received_handler(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "Get demo no rsp command from sensor hub core:");
    DUMP8("%02x ", ptr, len);
}

static void app_mcu_sensor_hub_demo_no_rsp_cmd_tx_done_handler(uint16_t cmdCode,
    uint8_t* ptr, uint16_t len)
{
    TRACE(0, "cmdCode 0x%x tx done", cmdCode);
}

void app_mcu_sensor_hub_send_demo_req_no_rsp(void)
{
    APP_MCU_SENSOR_HUB_DEMO_REQ_T req;
    req.reqData = CORE_BRIDGE_DEMO_MCU_REQ_DATA;
    app_core_bridge_send_cmd(MCU_SENSOR_HUB_TASK_CMD_DEMO_REQ_NO_RSP,
        (uint8_t *)&req, sizeof(req));
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_DEMO_REQ_NO_RSP,
                                "demo no rsp req to sensor hub core",
                                app_mcu_sensor_hub_transmit_demo_no_rsp_cmd_handler,
                                app_mcu_sensor_hub_demo_no_rsp_cmd_received_handler,
                                0,
                                NULL,
                                NULL,
                                app_mcu_sensor_hub_demo_no_rsp_cmd_tx_done_handler);

static void app_mcu_sensor_hub_transmit_demo_wait_rsp_cmd_handler(uint8_t* ptr, uint16_t len)
{
    app_core_bridge_send_data_with_waiting_rsp(MCU_SENSOR_HUB_TASK_CMD_DEMO_REQ_WITH_RSP, ptr, len);
}

static void app_mcu_sensor_hub_demo_wait_rsp_cmd_received_handler(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "Get demo with rsp command from sensor hub:");
    DUMP8("%02x ", ptr, len);
    APP_MCU_SENSOR_HUB_DEMO_RSP_T rsp;
    rsp.rspData = CORE_BRIDGE_DEMO_MCU_REQ_RSP_DATA;
    app_core_bridge_send_rsp(MCU_SENSOR_HUB_TASK_CMD_DEMO_REQ_WITH_RSP, (uint8_t *)&rsp, sizeof(rsp));
}

static void app_mcu_sensor_hub_demo_wait_rsp_cmd_wait_rsp_timeout(uint8_t* ptr, uint16_t len)
{
    TRACE(0, "mcu waiting for rsp to demo cmd timeout.");
}

static void app_mcu_sensor_hub_demo_wait_rsp_cmd_rsp_received_handler(uint8_t* ptr, uint16_t len)
{
    APP_MCU_SENSOR_HUB_DEMO_RSP_T* ptrRsp = (APP_MCU_SENSOR_HUB_DEMO_RSP_T *)ptr;
    TRACE(0, "mcu gets rsp 0x%x from sensor hub core:", ptrRsp->rspData);
}

static void app_mcu_sensor_hub_demo_with_rsp_cmd_tx_done_handler(uint16_t cmdCode,
    uint8_t* ptr, uint16_t len)
{
    TRACE(0, "cmdCode 0x%x tx done", cmdCode);
}

void app_mcu_sensor_hub_send_demo_req_with_rsp(void)
{
    APP_MCU_SENSOR_HUB_DEMO_REQ_T req;
    req.reqData = CORE_BRIDGE_DEMO_MCU_REQ_DATA;
    app_core_bridge_send_cmd(MCU_SENSOR_HUB_TASK_CMD_DEMO_REQ_WITH_RSP,
        (uint8_t *)&req, sizeof(req));
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_DEMO_REQ_WITH_RSP,
                                "demo with rsp req to sensor hub core",
                                app_mcu_sensor_hub_transmit_demo_wait_rsp_cmd_handler,
                                app_mcu_sensor_hub_demo_wait_rsp_cmd_received_handler,
                                APP_CORE_BRIDGE_DEFAULT_WAIT_RSP_TIMEOUT_MS,
                                app_mcu_sensor_hub_demo_wait_rsp_cmd_wait_rsp_timeout,
                                app_mcu_sensor_hub_demo_wait_rsp_cmd_rsp_received_handler,
                                app_mcu_sensor_hub_demo_with_rsp_cmd_tx_done_handler);

static void app_mcu_sensor_hub_transmit_demo_instant_req_handler(uint8_t* ptr, uint16_t len)
{
    app_core_bridge_send_instant_cmd_data(MCU_SENSOR_HUB_INSTANT_CMD_DEMO_REQ,
        ptr, len);
}

static void app_mcu_sensor_hub_demo_instant_req_handler(uint8_t* ptr, uint16_t len)
{
    // for test purpose, we add log print here.
    // but as instant cmd handler will be directly called in intersys irq context,
    // for realistic use, should never do log print
    TRACE(0, "Get demo instant req command from sensor core:");
    DUMP8("%02x ", ptr, len);
}

void app_mcu_sensor_hub_send_demo_instant_req(void)
{
    APP_MCU_SENSOR_HUB_DEMO_REQ_T req;
    req.reqData = CORE_BRIDGE_DEMO_MCU_REQ_DATA;
    app_core_bridge_send_cmd(MCU_SENSOR_HUB_INSTANT_CMD_DEMO_REQ,
        (uint8_t *)&req, sizeof(req));
}

CORE_BRIDGE_INSTANT_COMMAND_TO_ADD(MCU_SENSOR_HUB_INSTANT_CMD_DEMO_REQ,
                                app_mcu_sensor_hub_transmit_demo_instant_req_handler,
                                app_mcu_sensor_hub_demo_instant_req_handler);

#endif /* CORE_BRIDGE_DEMO_MSG */

#if defined(VPU_CFG_ON_SENSOR_HUB)
static void app_mcu_sensor_hub_transmit_vpu_wait_rsp_cmd_handler(uint8_t* ptr, uint16_t len)
{
    app_core_bridge_send_data_with_waiting_rsp(MCU_SENSOR_HUB_TASK_CMD_VPU, ptr, len);
}

static void app_mcu_sensor_hub_vpu_wait_rsp_cmd_wait_rsp_timeout(uint8_t* ptr, uint16_t len)
{
    TRACE(1, "[VPU] MCU waiting for rsp to demo cmd timeout.");
}

static void app_mcu_sensor_hub_vpu_wait_rsp_cmd_rsp_received_handler(uint8_t* ptr, uint16_t len)
{
    int32_t *ret = (int32_t *)ptr;
    TRACE(1, "[VPU] MCU gets rsp from sensor hub core: %d", *ret);
}

static void app_mcu_sensor_hub_vpu_with_rsp_cmd_tx_done_handler(uint16_t cmdCode,
    uint8_t* ptr, uint16_t len)
{
    TRACE(1, "[VPU] cmdCode 0x%x tx done", cmdCode);
}

extern "C" void app_mcu_sensor_hub_ctrl_vpu(bool enable)
{
    app_core_bridge_send_cmd(MCU_SENSOR_HUB_TASK_CMD_VPU, (uint8_t *)&enable, sizeof(enable));
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_VPU,
                                "cfg vpu on sensor hub core",
                                app_mcu_sensor_hub_transmit_vpu_wait_rsp_cmd_handler,
                                NULL,
                                APP_CORE_BRIDGE_DEFAULT_WAIT_RSP_TIMEOUT_MS,
                                app_mcu_sensor_hub_vpu_wait_rsp_cmd_wait_rsp_timeout,
                                app_mcu_sensor_hub_vpu_wait_rsp_cmd_rsp_received_handler,
                                app_mcu_sensor_hub_vpu_with_rsp_cmd_tx_done_handler);
#endif

#ifdef VAD_IF_TEST
void app_mcu_vad_if_test(void);
#endif
#ifdef SNDP_VAD_ENABLE
void sensor_hub_soundplus_mcu_app_init(void);
#endif

// mcu to initialize sensor hub module
void app_sensor_hub_init(void)
{
    if (true == app_sensor_hub_is_inited()) {
        return;
    }
    app_sensor_hub_ping_received = false;
    app_core_bridge_init();

    int ret;
    ret = sensor_hub_open(app_core_bridge_data_received, app_core_bridge_data_tx_done);
    ASSERT(ret == 0, "sensor_hub_open failed: %d", ret);

    osDelay(5);

#ifdef VAD_IF_TEST
    app_mcu_vad_if_test();
#endif

#ifdef  __AI_VOICE__
    sensor_hub_ai_mcu_app_init();
#endif

#ifdef SNDP_VAD_ENABLE
    sensor_hub_soundplus_mcu_app_init();
#endif

    if (sensor_hub_reset_cb &&
        (NULL != sensor_hub_reset_cb->sensor_hub_boot_done_cb))
    {
        sensor_hub_reset_cb->sensor_hub_boot_done_cb(APP_SENSOR_HUB_INIT);
    }

    app_mcu_sensor_hub_enable_watch_dog();
}

void app_sensor_hub_deinit(void)
{
    if (false == app_sensor_hub_is_inited()) {
        return;
    }

    sensor_hub_close();
    osDelay(10);

    // free all of the tx mailbox
    app_core_bridge_free_tx_mailbox();

    // release the waiting tx done semphore
    app_core_bridge_data_tx_done(NULL, 0);
    app_mcu_sensor_hub_disable_watch_dog();
    app_sensor_hub_inited = false;
    app_sensor_hub_ping_received = false;
}


void app_mcu_reset_sesor_hub(void)
{
    app_sensor_hub_deinit();
    int ret;
    ret = sensor_hub_open(app_core_bridge_data_received, app_core_bridge_data_tx_done);
    ASSERT(ret == 0, "sensor_hub_open failed: %d", ret);

    // free all of the tx mailbox
    app_core_bridge_free_tx_mailbox();

    // release the waiting tx done semphore
    app_core_bridge_data_tx_done(NULL, 0);

    app_mcu_sensor_hub_enable_watch_dog();
}

bool app_sensor_hub_is_inited(void)
{
    return app_sensor_hub_ping_received;
}
#endif
