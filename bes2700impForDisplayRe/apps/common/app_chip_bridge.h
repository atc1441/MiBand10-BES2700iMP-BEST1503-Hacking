/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef __APP_CHIP_BRIDGE_H__
#define __APP_CHIP_BRIDGE_H__

#ifndef APP_CHIP_BRIDGE_MAX_DATA_PACKET_SIZE
#define APP_CHIP_BRIDGE_MAX_DATA_PACKET_SIZE    (516)
#endif

#ifndef APP_CHIP_BRIDGE_RX_BUFF_SIZE
#define APP_CHIP_BRIDGE_RX_BUFF_SIZE            (5160)
#endif

#ifndef APP_CHIP_BRIDGE_TX_MAILBOX_MAX
#define APP_CHIP_BRIDGE_TX_MAILBOX_MAX          (8)
#endif

#ifndef APP_CHIP_BRIDGE_MAX_XFER_DATA_SIZE
#define APP_CHIP_BRIDGE_MAX_XFER_DATA_SIZE      (516)
#endif

// every transfer has the fixed size
#define APP_UART_HEADER_DATA_SIZE               (6)

#ifndef APP_CHIP_BRIDGE_RX_THREAD_TMP_BUF_SIZE
#define APP_CHIP_BRIDGE_RX_THREAD_TMP_BUF_SIZE  (516)
#endif

#ifndef APP_CHIP_BRIDGE_TX_THREAD_STACK_SIZE
#define APP_CHIP_BRIDGE_TX_THREAD_STACK_SIZE    (2048+1024-512)
#endif

#ifndef APP_CHIP_BRIDGE_RX_THREAD_STACK_SIZE
#define APP_CHIP_BRIDGE_RX_THREAD_STACK_SIZE    (1024*4+1024-512)
#endif

#define RESEND_TIME 2

#define APP_CHIP_BRIDGE_DEFAULT_WAIT_RSP_TIMEOUT_MS 3000


#define CHIP_BRIDGE_CMD_GROUP_TASK      0x00
#define CHIP_BRIDGE_CMD_GROUP_INSTANT   0x01

#define CHIP_BRIDGE_CMD_GROUP_OFFSET           (8)
#define CHIP_BRIDGE_CMD_SUBCODE_BIT(subCode)   ((subCode)&((1<<CHIP_BRIDGE_CMD_GROUP_OFFSET)-1))
#define CHIP_BRIDGE_CMD_CODE(group, subCode)   (((group) << CHIP_BRIDGE_CMD_GROUP_OFFSET)|CHIP_BRIDGE_CMD_SUBCODE_BIT(subCode))
#define CHIP_BRIDGE_CMD_SUBCODE(cmdCode)       ((cmdCode)&((1 << CHIP_BRIDGE_CMD_GROUP_OFFSET)-1))
#define CHIP_BRIDGE_CMD_GROUP(cmdCode)         ((cmdCode) >> CHIP_BRIDGE_CMD_GROUP_OFFSET)

#define CHIP_BRIDGE_CMD_INSTANT(subCode) CHIP_BRIDGE_CMD_CODE(CHIP_BRIDGE_CMD_GROUP_INSTANT, (subCode))
#define CHIP_BRIDGE_CMD_TASK(subCode)    CHIP_BRIDGE_CMD_CODE(CHIP_BRIDGE_CMD_GROUP_TASK, (subCode))

typedef void (*app_chip_bridge_cmd_transmit_handler_t)(uint8_t*, uint16_t);
typedef void (*app_chip_bridge_cmd_receivd_handler_t)(uint8_t*, uint16_t);
typedef void (*app_chip_bridge_wait_rsp_timeout_handle_t)(uint8_t*, uint16_t);
typedef void (*app_chip_bridge_rsp_handle_t)(uint8_t*, uint16_t);
typedef void (*app_chip_bridge_cmd_transmission_done_handler_t) (uint16_t, uint8_t*, uint16_t);

typedef struct
{
    uint16_t                                        cmdcode;
    const char                                      *log_cmd_code_str;
    app_chip_bridge_cmd_transmit_handler_t           chip_bridge_cmd_transmit_handler;
    app_chip_bridge_cmd_receivd_handler_t            cmdhandler;
    uint32_t                                        wait_rsp_timeout_ms;
    app_chip_bridge_wait_rsp_timeout_handle_t        app_chip_bridge_wait_rsp_timeout_handle;
    app_chip_bridge_rsp_handle_t                     app_chip_bridge_rsp_handle;
    app_chip_bridge_cmd_transmission_done_handler_t  app_chip_bridge_transmisson_done_handler;
} __attribute__((aligned(4))) app_chip_bridge_task_cmd_instance_t;

typedef struct
{
    uint16_t                                cmdcode;
    app_chip_bridge_cmd_transmit_handler_t   chip_bridge_cmd_transmit_handler;
    app_chip_bridge_cmd_receivd_handler_t    cmdhandler;
} __attribute__((aligned(4))) app_chip_bridge_instant_cmd_instance_t;


#define CHIP_BRIDGE_TASK_COMMAND_TO_ADD(cmdCode,                            \
                                        log_cmd_code_str,                   \
                                        chip_bridge_cmd_transmit_handler,   \
                                        cmdhandler,                         \
                                        wait_rsp_timeout_ms,                \
                                        app_chip_bridge_wait_rsp_timeout_handle,    \
                                        app_chip_bridge_rsp_handle,                 \
                                        app_chip_bridge_transmisson_done_handler)   \
                                        static const app_chip_bridge_task_cmd_instance_t cmdCode##task##_entry  \
                                        __attribute__((used, section(".chip_bridge_task_cmd_table"))) =     \
                                        {(cmdCode),       \
                                        (log_cmd_code_str),                         \
                                        (chip_bridge_cmd_transmit_handler),         \
                                        (cmdhandler),                               \
                                        (wait_rsp_timeout_ms),                      \
                                        (app_chip_bridge_wait_rsp_timeout_handle),  \
                                        (app_chip_bridge_rsp_handle),               \
                                        (app_chip_bridge_transmisson_done_handler)};


#define CHIP_BRIDGE_INSTANT_COMMAND_TO_ADD(cmdCode,                         \
                                        chip_bridge_cmd_transmit_handler,   \
                                        cmdhandler)                         \
                                        static const app_chip_bridge_instant_cmd_instance_t cmdCode##task##_entry   \
                                        __attribute__((used, section(".chip_bridge_instant_cmd_table"))) =      \
                                        {(cmdCode),        \
                                        (chip_bridge_cmd_transmit_handler), \
                                        (cmdhandler)};

typedef enum
{
    CHIP_TASK_CMD_RSP                           = CHIP_BRIDGE_CMD_CODE(CHIP_BRIDGE_CMD_GROUP_TASK, 0x00),
    SET_TEST_MODE_CMD                           = CHIP_BRIDGE_CMD_CODE(CHIP_BRIDGE_CMD_GROUP_TASK, 0x01),
} CHIP_BRIDGE_CMD_CODE_E;

#ifdef __cplusplus
extern "C" {
#endif

void app_chip_bridge_init(void);
void app_chip_bridge_send_data_without_waiting_rsp(uint16_t cmdcode, uint8_t *p_buff, uint16_t length);
void app_chip_bridge_send_data_with_waiting_rsp(uint16_t cmdcode, uint8_t *p_buff, uint16_t length);
int32_t app_chip_bridge_send_cmd(uint16_t cmd_code, uint8_t *p_buff, uint16_t length);
int32_t app_chip_bridge_send_rsp(uint16_t rsp_code, uint8_t *p_buff, uint16_t length);
unsigned int app_chip_bridge_data_received(const void* data, unsigned int len);
void app_chip_bridge_data_tx_done(const void* data, unsigned int len);
void app_chip_bridge_send_instant_cmd_data(uint16_t cmdcode, uint8_t *p_buff, uint16_t length);
void app_chip_bridge_free_tx_mailbox(void);


#ifdef __cplusplus
}
#endif

#endif
