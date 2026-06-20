/***************************************************************************
 *
 * Copyright 2022-2023 BES.
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
#include "app_capsensor.h"
#include "stdint.h"
#include "touch_wear_core.h"
#include "plat_addr_map.h"
#include CHIP_SPECIFIC_HDR(capsensor_driver)
#include "capsensor_driver.h"
#if defined(CAPSENSOR_AT_SENS)
#include "app_sensor_hub.h"
#endif
#include "hal_trace.h"
#include "app_key.h"
#include "stdint.h"
#include "string.h"

void app_capsensor_click_event(uint8_t key_event);

#if defined(CAPSENSOR_AT_SENS)
static void app_mcu_sensor_hub_transmit_touch_no_rsp_cmd_handler(uint8_t* ptr, uint16_t len)
{
    app_core_bridge_send_data_without_waiting_rsp(MCU_SENSOR_HUB_TASK_CMD_TOUCH_REQ_NO_RSP, ptr, len);
}

static void app_mcu_sensor_hub_touch_no_rsp_cmd_received_handler(uint8_t* ptr, uint16_t len)
{
    APP_KEY_STATUS status = *((APP_KEY_STATUS *)ptr);
    TRACE(0, "Get touch no rsp command from sensor hub core, status.event: %d", status.event);
    app_capsensor_click_event(status.event);
}

static void app_mcu_sensor_hub_touch_no_rsp_cmd_tx_done_handler(uint16_t cmdCode,
    uint8_t* ptr, uint16_t len)
{
    TRACE(0, "cmdCode 0x%x tx done", cmdCode);
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_TOUCH_REQ_NO_RSP,
                                "touch no rsp req to sensor hub core",
                                app_mcu_sensor_hub_transmit_touch_no_rsp_cmd_handler,
                                app_mcu_sensor_hub_touch_no_rsp_cmd_received_handler,
                                0,
                                NULL,
                                NULL,
                                app_mcu_sensor_hub_touch_no_rsp_cmd_tx_done_handler);
#endif

/***************************************************************************
 * @brief app_capsensor_click_event function
 *
 * @param key_event touch or wear event
 ***************************************************************************/
void app_capsensor_click_event(uint8_t key_event)
{
    APP_KEY_STATUS status;

    status.event = key_event;
    TRACE(1, "[%s]:%d",  __func__, status.event);

    switch (status.event)
    {
        case CAP_KEY_EVENT_OFF_EAR:
            TRACE(1, "capsensor state= off ear\n");
            status.event = APP_KEY_EVENT_OFF_EAR;
            break;

        case CAP_KEY_EVENT_ON_EAR:
            TRACE(1, "capsensor state= on ear\n");
            status.event = APP_KEY_EVENT_ON_EAR;
            break;

        case CAP_KEY_EVENT_UP:
            TRACE(1, "capsensor state= key up\n");
            status.event = APP_KEY_EVENT_UP;
            break;

        case CAP_KEY_EVENT_DOWN:
            TRACE(1, "capsensor state= key down\n");
            status.event = APP_KEY_EVENT_DOWN;
            break;

        case CAP_KEY_EVENT_UPSLIDE:
            TRACE(1, "capsensor state= up slide\n");
            status.event = APP_KEY_EVENT_UPSLIDE;
            break;

        case CAP_KEY_EVENT_DOWNSLIDE:
            TRACE(1, "capsensor state= down slide\n");
            status.event = APP_KEY_EVENT_DOWNSLIDE;
            break;

        case CAP_KEY_EVENT_CLICK:
            TRACE(1, "capsensor state= single click\n");
            status.event = APP_KEY_EVENT_CLICK;
            break;

        case CAP_KEY_EVENT_DOUBLECLICK:
            TRACE(1, "capsensor state= double click\n");
            status.event = APP_KEY_EVENT_DOUBLECLICK;
            break;

        case CAP_KEY_EVENT_TRIPLECLICK:
            TRACE(1, "capsensor state= triple click\n");
            status.event = APP_KEY_EVENT_TRIPLECLICK;
            break;

        case CAP_KEY_EVENT_ULTRACLICK:
            TRACE(1, "capsensor state= ultra click\n");
            status.event = APP_KEY_EVENT_ULTRACLICK;
            break;

        case CAP_KEY_EVENT_RAMPAGECLICK:
            TRACE(1, "capsensor state= rampage click\n");
            status.event = APP_KEY_EVENT_RAMPAGECLICK;
            break;

        case CAP_KEY_EVENT_SIXCLICK:
            TRACE(1, "capsensor state= six click\n");
            break;

        case CAP_KEY_EVENT_SEVENCLICK:
            TRACE(1, "capsensor state= seven click\n");
            break;

        case CAP_KEY_EVENT_LONGPRESS:
            TRACE(1, "capsensor state= long press\n");
            status.event = APP_KEY_EVENT_LONGPRESS;
            break;

        case CAP_KEY_EVENT_LONGLONGPRESS:
            TRACE(1, "capsensor state= long  long press\n");
            status.event = APP_KEY_EVENT_LONGLONGPRESS;
            break;

        case CAP_KEY_EVENT_CLICK_AND_LONGPRESS:
            TRACE(1, "capsensor state= click and long press\n");
            break;

        case CAP_KEY_EVENT_CLICK_AND_LONGLONGPRESS:
            TRACE(1, "capsensor state= click and longlongpress\n");
            break;

        case CAP_KEY_EVENT_DOUBLECLICK_AND_LONGLONGPRESS:
            TRACE(1, "capsensor state= double click and longlongpress\n");
            break;

        case CAP_KEY_EVENT_TRIPLECLICK_AND_LONGLONGPRESS:
            TRACE(1, "capsensor state= triple click and longlongpress\n");
            break;

        default:
            break;
    }
}

/***************************************************************************
 * @brief  app_mcu_core_capsensor_init function
 *
 ***************************************************************************/
void app_mcu_core_capsensor_init(void)
{
    int ret = -1;
    ret = register_capsensor_click_event_callback(app_capsensor_click_event);
    if (ret) {
        TRACE(1,"register_capsensor_click_event_callback failed:%d\n", ret);
    }

    capsensor_driver_init();

#ifdef CAPSENSOR_AT_MCU
    capsensor_sens2mcu_irq_set();
    cap_sensor_core_thread_init();
#endif
}

#endif

