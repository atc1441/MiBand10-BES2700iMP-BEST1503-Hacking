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
#ifndef __APP_SENSOR_HUB_H__
#define __APP_SENSOR_HUB_H__

#ifndef APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE
#define APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE    (512)
#endif

#ifndef APP_CORE_BRIDGE_RX_BUFF_SIZE
#define APP_CORE_BRIDGE_RX_BUFF_SIZE            (2048)
#endif

#ifndef APP_CORE_BRIDGE_TX_MAILBOX_MAX
#define APP_CORE_BRIDGE_TX_MAILBOX_MAX          (8)
#endif

#ifndef APP_CORE_BRIDGE_MAX_XFER_DATA_SIZE
#define APP_CORE_BRIDGE_MAX_XFER_DATA_SIZE      (512)
#endif

#ifndef APP_CORE_BRIDGE_RX_THREAD_TMP_BUF_SIZE
#define APP_CORE_BRIDGE_RX_THREAD_TMP_BUF_SIZE  (512)
#endif

#ifndef APP_CORE_BRIDGE_TX_THREAD_STACK_SIZE
#define APP_CORE_BRIDGE_TX_THREAD_STACK_SIZE    (2048+1024-512)
#endif

#ifndef APP_CORE_BRIDGE_RX_THREAD_STACK_SIZE
#define APP_CORE_BRIDGE_RX_THREAD_STACK_SIZE    (1024*4+1024-512)
#endif

#define APP_CORE_BRIDGE_DEFAULT_WAIT_RSP_TIMEOUT_MS 500

#define CONCAT_IMPL(a, b)                           a##b
#define CONCAT(a, b)                                CONCAT_IMPL(a, b)
#define TASK_CMD(n)                                 CONCAT(MCU_SENSOR_HUB_TASK_CMD_, n)
#define INSTANT_CMD(n)                              CONCAT(MCU_SENSOR_HUB_INSTANT_CMD_, n)

#define CORE_BRIDGE_CMD_GROUP_TASK                  0x00
#define CORE_BRIDGE_CMD_GROUP_INSTANT               0x01

#define CORE_BRIDGE_CMD_GROUP_OFFSET                (8)
#define CORE_BRIDGE_CMD_SUBCODE_BIT(subCode)        ((subCode) & ((1 << CORE_BRIDGE_CMD_GROUP_OFFSET) -1))
#define CORE_BRIDGE_CMD_CODE(group, subCode)        (((group) << CORE_BRIDGE_CMD_GROUP_OFFSET) | CORE_BRIDGE_CMD_SUBCODE_BIT(subCode))
#define CORE_BRIDGE_CMD_SUBCODE(cmdCode)            ((cmdCode) & ((1 << CORE_BRIDGE_CMD_GROUP_OFFSET) -1))
#define CORE_BRIDGE_CMD_GROUP(cmdCode)              ((cmdCode) >> CORE_BRIDGE_CMD_GROUP_OFFSET)

#define CORE_BRIDGE_CMD_TASK(subCode)               CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, (subCode)),
#define CORE_BRIDGE_CMD_INSTANT(subCode)            CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_INSTANT, (subCode)),

typedef void (*app_core_bridge_cmd_transmit_handler_t)(uint8_t*, uint16_t);
typedef void (*app_core_bridge_cmd_receivd_handler_t)(uint8_t*, uint16_t);
typedef void (*app_core_bridge_wait_rsp_timeout_handle_t)(uint8_t*, uint16_t);
typedef void (*app_core_bridge_rsp_handle_t)(uint8_t*, uint16_t);
typedef void (*app_core_bridge_cmd_transmission_done_handler_t) (uint16_t, uint8_t*, uint16_t);

typedef struct
{
    uint16_t                                cmdcode;
    const char                              *log_cmd_code_str;
    app_core_bridge_cmd_transmit_handler_t  core_bridge_cmd_transmit_handler;
    app_core_bridge_cmd_receivd_handler_t   cmdhandler;
    uint32_t                                wait_rsp_timeout_ms;
    app_core_bridge_wait_rsp_timeout_handle_t       app_core_bridge_wait_rsp_timeout_handle;
    app_core_bridge_rsp_handle_t                    app_core_bridge_rsp_handle;
    app_core_bridge_cmd_transmission_done_handler_t app_core_bridge_transmisson_done_handler;
} __attribute__((aligned(4))) app_core_bridge_task_cmd_instance_t;

typedef struct
{
    uint16_t                                cmdcode;
    app_core_bridge_cmd_transmit_handler_t  core_bridge_cmd_transmit_handler;
    app_core_bridge_cmd_receivd_handler_t   cmdhandler;
} __attribute__((aligned(4))) app_core_bridge_instant_cmd_instance_t;


#define CORE_BRIDGE_TASK_COMMAND_TO_ADD(cmdCode,                            \
                                        log_cmd_code_str,                   \
                                        core_bridge_cmd_transmit_handler,   \
                                        cmdhandler,                         \
                                        wait_rsp_timeout_ms,                \
                                        app_core_bridge_wait_rsp_timeout_handle,    \
                                        app_core_bridge_rsp_handle,                 \
                                        app_core_bridge_transmisson_done_handler)   \
                                        static const app_core_bridge_task_cmd_instance_t cmdCode##task##_entry  \
                                        __attribute__((used, section(".core_bridge_task_cmd_table"))) =     \
                                        {(cmdCode),       \
                                        (log_cmd_code_str),                         \
                                        (core_bridge_cmd_transmit_handler),         \
                                        (cmdhandler),                               \
                                        (wait_rsp_timeout_ms),                      \
                                        (app_core_bridge_wait_rsp_timeout_handle),  \
                                        (app_core_bridge_rsp_handle),               \
                                        (app_core_bridge_transmisson_done_handler)};


#define CORE_BRIDGE_INSTANT_COMMAND_TO_ADD(cmdCode,                         \
                                        core_bridge_cmd_transmit_handler,   \
                                        cmdhandler)                         \
                                        static const app_core_bridge_instant_cmd_instance_t cmdCode##task##_entry   \
                                        __attribute__((used, section(".core_bridge_instant_cmd_table"))) =      \
                                        {(cmdCode),        \
                                        (core_bridge_cmd_transmit_handler), \
                                        (cmdhandler)};

typedef enum
{
    TASK_CMD(PING)      = CORE_BRIDGE_CMD_TASK(0)
    TASK_CMD(RSP)       = CORE_BRIDGE_CMD_TASK(1)
    /* sample command */
    TASK_CMD(DEMO_REQ_NO_RSP)   = CORE_BRIDGE_CMD_TASK(2)
    TASK_CMD(DEMO_REQ_WITH_RSP) = CORE_BRIDGE_CMD_TASK(3)
    /* voice detector */
    TASK_CMD(VAD)       = CORE_BRIDGE_CMD_TASK(4)
    /* ai kws enginne signaling cmd */
    TASK_CMD(AI)        = CORE_BRIDGE_CMD_TASK(5)
    /* capsensor touch cmd */
    TASK_CMD(TOUCH_REQ_NO_RSP)  = CORE_BRIDGE_CMD_TASK(6)
    /* soundplus vad cmd */
    TASK_CMD(SNDP)      = CORE_BRIDGE_CMD_TASK(7)
    /* at cmd */
    TASK_CMD(AT_CMD)    = CORE_BRIDGE_CMD_TASK(8)
    /* vpu */
    TASK_CMD(VPU)    = CORE_BRIDGE_CMD_TASK(9)
    /* spp test capsensor */
    TASK_CMD(SPP_CAPSENSOR_TEST)  = CORE_BRIDGE_CMD_TASK(25)
    /* capsensor factory calculate */
    TASK_CMD(CAPSENSOR_FACTORY_TEST)  = CORE_BRIDGE_CMD_TASK(26)
#if defined (SPA_AUDIO_ENABLE)
    TASK_CMD(DATA_SYNC) = CORE_BRIDGE_CMD_TASK(0xa0)
#endif

    BTH_SENS_TASK_CMD_SCO_INIT_RSP        = CORE_BRIDGE_CMD_TASK(16)
    BTH_SENS_TASK_CMD_SCO_DEINIT_RSP      = CORE_BRIDGE_CMD_TASK(17)
    BTH_SENS_TASK_CMD_SCO_CAPTURE_NO_RSP  = CORE_BRIDGE_CMD_TASK(18)
    BTH_SENS_TASK_CMD_SCO_PLAYBACK_NO_RSP = CORE_BRIDGE_CMD_TASK(19)

    BTH_SENS_TASK_CMD_MCPP_RSP            = CORE_BRIDGE_CMD_TASK(23)
    BTH_SENS_TASK_CMD_MCPP_NO_RSP         = CORE_BRIDGE_CMD_TASK(24)

    INSTANT_CMD(DEMO_REQ)       = CORE_BRIDGE_CMD_INSTANT(0)
    INSTANT_CMD(TOUCH_REQ)      = CORE_BRIDGE_CMD_INSTANT(1)
} CORE_BRIDGE_CMD_CODE_E;

#ifdef __cplusplus
extern "C" {
#endif

void app_core_bridge_init(void);
void app_sensor_hub_send_data(uint8_t* data, uint32_t len);
void app_core_bridge_send_data_without_waiting_rsp(uint16_t cmdcode, uint8_t *p_buff, uint16_t length);
void app_core_bridge_send_data_with_waiting_rsp(uint16_t cmdcode, uint8_t *p_buff, uint16_t length);
int32_t app_core_bridge_send_cmd(uint16_t cmd_code, uint8_t *p_buff, uint16_t length);
int32_t app_core_bridge_send_rsp(uint16_t rsp_code, uint8_t *p_buff, uint16_t length);
unsigned int app_core_bridge_data_received(const void* data, unsigned int len);
void app_core_bridge_data_tx_done(const void* data, unsigned int len);
void app_core_bridge_send_instant_cmd_data(uint16_t cmdcode, uint8_t *p_buff, uint16_t length);
void app_core_bridge_free_tx_mailbox(void);

#ifdef __cplusplus
}
#endif

#endif

