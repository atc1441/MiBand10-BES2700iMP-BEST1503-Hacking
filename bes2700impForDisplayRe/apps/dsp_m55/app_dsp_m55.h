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
#ifndef __APP_DSP_M55_H__
#define __APP_DSP_M55_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef APP_DSP_M55_BRIDGE_MAX_DATA_PACKET_SIZE
#define APP_DSP_M55_BRIDGE_MAX_DATA_PACKET_SIZE    (512)
#endif

#ifndef APP_DSP_M55_BRIDGE_RX_BUFF_SIZE
#define APP_DSP_M55_BRIDGE_RX_BUFF_SIZE            (2048)
#endif

#ifndef APP_DSP_M55_BRIDGE_TX_MAILBOX_MAX
#define APP_DSP_M55_BRIDGE_TX_MAILBOX_MAX          (20)
#endif

#ifndef APP_DSP_M55_BRIDGE_MAX_XFER_DATA_SIZE
#define APP_DSP_M55_BRIDGE_MAX_XFER_DATA_SIZE      (512)
#endif

#ifndef APP_DSP_M55_BRIDGE_RX_THREAD_TMP_BUF_SIZE
#define APP_DSP_M55_BRIDGE_RX_THREAD_TMP_BUF_SIZE  (512)
#endif

#ifndef APP_DSP_M55_BRIDGE_TX_THREAD_STACK_SIZE
#define APP_DSP_M55_BRIDGE_TX_THREAD_STACK_SIZE    (2048+1024-512)
#endif

#ifndef APP_DSP_M55_BRIDGE_RX_THREAD_STACK_SIZE
#define APP_DSP_M55_BRIDGE_RX_THREAD_STACK_SIZE    (1024*4+1024-512)
#endif

#define APP_DSP_M55_BRIDGE_DEFAULT_WAIT_RSP_TIMEOUT_MS 500

#define CORE_BRIDGE_CMD_GROUP_TASK      0x00
#define CORE_BRIDGE_CMD_GROUP_INSTANT   0x01

#define CORE_BRIDGE_CMD_GROUP_OFFSET           (8)
#define CORE_BRIDGE_CMD_SUBCODE_BIT(subCode)   ((subCode)&((1<<CORE_BRIDGE_CMD_GROUP_OFFSET)-1))
#define CORE_BRIDGE_CMD_CODE(group, subCode)   (((group) << CORE_BRIDGE_CMD_GROUP_OFFSET)|CORE_BRIDGE_CMD_SUBCODE_BIT(subCode))
#define CORE_BRIDGE_CMD_SUBCODE(cmdCode)       ((cmdCode)&((1 << CORE_BRIDGE_CMD_GROUP_OFFSET)-1))
#define CORE_BRIDGE_CMD_GROUP(cmdCode)         ((cmdCode) >> CORE_BRIDGE_CMD_GROUP_OFFSET)

#define CORE_BRIDGE_CMD_INSTANT(subCode) CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_INSTANT, (subCode))
#define CORE_BRIDGE_CMD_TASK(subCode)    CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, (subCode))

typedef void (*app_dsp_m55_bridge_cmd_transmit_handler_t)(uint8_t*, uint16_t);
typedef void (*app_dsp_m55_bridge_cmd_receivd_handler_t)(uint8_t*, uint16_t);
typedef void (*app_dsp_m55_bridge_wait_rsp_timeout_handle_t)(uint8_t*, uint16_t);
typedef void (*app_dsp_m55_bridge_rsp_handle_t)(uint8_t*, uint16_t);
typedef void (*app_dsp_m55_bridge_cmd_transmission_done_handler_t) (uint16_t, uint8_t*, uint16_t);

typedef struct
{
    uint16_t                                cmdcode;
    const char                              *log_cmd_code_str;
    app_dsp_m55_bridge_cmd_transmit_handler_t  core_bridge_cmd_transmit_handler;
    app_dsp_m55_bridge_cmd_receivd_handler_t   cmdhandler;
    uint32_t                                wait_rsp_timeout_ms;
    app_dsp_m55_bridge_wait_rsp_timeout_handle_t       app_dsp_m55_bridge_wait_rsp_timeout_handle;
    app_dsp_m55_bridge_rsp_handle_t                    app_dsp_m55_bridge_rsp_handle;
    app_dsp_m55_bridge_cmd_transmission_done_handler_t app_dsp_m55_bridge_transmisson_done_handler;
} __attribute__((aligned(4))) app_dsp_m55_bridge_task_cmd_instance_t;

typedef struct
{
    uint16_t                                cmdcode;
    app_dsp_m55_bridge_cmd_transmit_handler_t  core_bridge_cmd_transmit_handler;
    app_dsp_m55_bridge_cmd_receivd_handler_t   cmdhandler;
} __attribute__((aligned(4))) app_dsp_m55_bridge_instant_cmd_instance_t;


#define CORE_BRIDGE_TASK_COMMAND_TO_ADD(cmdCode,                            \
                                        log_cmd_code_str,                   \
                                        core_bridge_cmd_transmit_handler,   \
                                        cmdhandler,                         \
                                        wait_rsp_timeout_ms,                \
                                        app_dsp_m55_bridge_wait_rsp_timeout_handle,    \
                                        app_dsp_m55_bridge_rsp_handle,                 \
                                        app_dsp_m55_bridge_transmisson_done_handler)   \
                                        static const app_dsp_m55_bridge_task_cmd_instance_t cmdCode##task##_entry  \
                                        __attribute__((used, section(".core_bridge_task_cmd_table"))) =     \
                                        {(cmdCode),       \
                                        (log_cmd_code_str),                         \
                                        (core_bridge_cmd_transmit_handler),         \
                                        (cmdhandler),                               \
                                        (wait_rsp_timeout_ms),                      \
                                        (app_dsp_m55_bridge_wait_rsp_timeout_handle),  \
                                        (app_dsp_m55_bridge_rsp_handle),               \
                                        (app_dsp_m55_bridge_transmisson_done_handler)};


#define CORE_BRIDGE_INSTANT_COMMAND_TO_ADD(cmdCode,                         \
                                        core_bridge_cmd_transmit_handler,   \
                                        cmdhandler)                         \
                                        static const app_dsp_m55_bridge_instant_cmd_instance_t cmdCode##task##_entry   \
                                        __attribute__((used, section(".core_bridge_instant_cmd_table"))) =      \
                                        {(cmdCode),        \
                                        (core_bridge_cmd_transmit_handler), \
                                        (cmdhandler)};

typedef enum
{
    MCU_DSP_M55_TASK_CMD_PING = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x80),
    MCU_DSP_M55_TASK_CMD_RSP  = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x81),

    // sample command
    MCU_DSP_M55_TASK_CMD_DEMO_REQ_NO_RSP    = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x82),
    MCU_DSP_M55_TASK_CMD_DEMO_REQ_WITH_RSP  = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x83),

    // voice detector
    MCU_DSP_M55_TASK_CMD_VAD  = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x84),

    // ai kws enginne signaling cmd
    MCU_DSP_M55_TASK_CMD_AI  = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x85),

    // capsensor touch cmd
    MCU_DSP_M55_TASK_CMD_TOUCH_REQ_NO_RSP  = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x86),

    // soundplus vad cmd
    MCU_DSP_M55_TASK_CMD_SNDP  = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x87),

    // speech
    MCU_DSP_M55_TASK_CMD_SCO_INIT_NO_RSP = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x88),
    MCU_DSP_M55_TASK_CMD_SCO_DEINIT_NO_RSP = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x89),
    MCU_DSP_M55_TASK_CMD_SCO_CAPTURE_RSP = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x8a),
    MCU_DSP_M55_TASK_CMD_SCO_PLAYBACK_RSP = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x8b),

#ifdef A2DP_DECODER_CROSS_CORE_USE_M55
    CROSS_CORE_TASK_CMD_A2DP_INIT_WAITING_RSP = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x90),
    CROSS_CORE_TASK_CMD_A2DP_DEINIT_WAITING_RSP = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x91),

    // bth to off-bth core, no waiting but keep the a2dp encoded data buffer
    CROSS_CORE_TASK_CMD_A2DP_FEED_ENCODED_DATA_PACKET = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x92),

    // off-bth core to bth after copying data to local buffer
    CROSS_CORE_TASK_CMD_A2DP_ACK_FED_ENCODED_DATA_PACKET = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x93),

    // off-bth core to bth after decoding the data, no waiting but keep the
    // pcm data buffer
    CROSS_CORE_TASK_CMD_A2DP_FEED_PCM_DATA_PACKET = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x94),

    // bth to off-bth core after consuming the pcm data to codec dma
    CROSS_CORE_TASK_CMD_A2DP_ACK_FED_PCM_DATA_PACKET = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x95),

    CROSS_CORE_TASK_CMD_A2DP_RETRIGGER_REQ_NO_RSP = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x96),

    CROSS_CORE_TASK_CMD_A2DP_REMOVE_SPECIFIC_FRAME = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x97),

    CROSS_CORE_TASK_CMD_A2DP_BOOST_FREQ_REQ = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x98),

    CROSS_CORE_TASK_CMD_A2DP_BOOST_FREQ_DONE = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x99),

    CROSS_CORE_TASK_CMD_A2DP_SCALABLE_SBM_TO_OFF_BTH = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x9a),

    CROSS_CORE_TASK_CMD_A2DP_SCALABLE_SBM_TO_BTH = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x9b),

    CROSS_CORE_TASK_CMD_A2DP_SCALABLE_SBM_STATUS = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0x9c),
#endif

#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
    CROSS_CORE_TASK_CMD_GAF_ENCODE_INIT_WAITING_RSP = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0xa4),
    CROSS_CORE_TASK_CMD_GAF_ENCODE_DEINIT_WAITING_RSP = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0xa5),

    // core to off core (M55), no waiting but keep the gaf audio pcm data buffer
    CROSS_CORE_TASK_CMD_GAF_ENCODE_FEED_PCM_DATA_PACKET = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0xa6),

    // M55 ack feed pcm data to core cpu after copying data to local buffer
    CROSS_CORE_TASK_CMD_GAF_ACK_ENCODE_FEED_PCM_DATA_PACKET = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0xa7),

    // ask to mcu core to retriggle audio media
    CROSS_CORE_TASK_CMD_GAF_RETRIGGER_REQ_NO_RSP = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0xa8),

    // m55 feed encoded data to mcu after encoding the pcm data, no waiting
    CROSS_CORE_TASK_CMD_GAF_FEED_ENCODED_DATA_TO_CORE = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0xa9),

    // mcu core ack M55 about encoding the pcm data
    CROSS_CORE_TASK_CMD_GAF_ACK_FEED_ENCODED_DATA_TO_CORE = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0Xaa),

#endif

#ifdef GAF_DECODER_CROSS_CORE_USE_M55
    // mcu core ask m55 to decoder init, has waiting rsp
    CROSS_CORE_TASK_CMD_GAF_DECODE_INIT_WAITING_RSP = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0xab),

    // mcu core ask m55 to decoder deinit, has waiting rsp
    CROSS_CORE_TASK_CMD_GAF_DECODE_DEINIT_WAITING_RSP = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0xac),

    // core to off core (M55), no waiting but keep the gaf audio emcoded data buffer
    CROSS_CORE_TASK_CMD_GAF_DECODE_FEED_ENCODED_DATA_PACKET = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0xad),
    // bth core to m55 after copying data to local buffer
    CROSS_CORE_TASK_CMD_GAF_DECODE_ACK_FEED_ENCODED_DATA_PACKET = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0xaf),

    // ask to mcu core to retrigger audio media
    CROSS_CORE_TASK_CMD_GAF_DECODE_RETRIGGER_REQ_NO_RSP = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0xb0),

    // m55 core to bth after decoding the data, no waiting but keep the pcm data buffer
    CROSS_CORE_TASK_CMD_GAF_DECODE_FEED_PCM_DATA_PACKET = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0xb1),

    // bth to m55 core after consuming the pcm data to codec dma
    CROSS_CORE_TASK_CMD_GAF_DECODE_ACK_FED_PCM_DATA_PACKET = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0xb2),

    // mcu ask to remove specfic frame
    CROSS_CORE_TASK_CMD_GAF_DECODE_REMOVE_SPECIFIC_FRAME = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0xb3),
#endif

    MCU_DSP_M55_TASK_CMD_SMF_NO_RSP = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0xb5),
    MCU_DSP_M55_TASK_CMD_SMF_RSP    = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_TASK, 0xb6),

    MCU_DSP_M55_INSTANT_CMD_DEMO_REQ  = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_INSTANT, 0x80),

    MCU_DSP_M55_INSTANT_CMD_TOUCH_REQ  = CORE_BRIDGE_CMD_CODE(CORE_BRIDGE_CMD_GROUP_INSTANT, 0x81),

} CORE_BRIDGE_CMD_CODE_E;

/* Core-Bridge Thread Init */
void app_dsp_m55_bridge_init(void);

/* Client Thread APIs, runs at Client Thread */
int32_t app_dsp_m55_bridge_send_cmd(uint16_t cmd_code, uint8_t *p_buff, uint16_t length);

/* Command Callback APIs, runs at Core-Bridge Thread */
int32_t app_dsp_m55_bridge_send_rsp(uint16_t rsp_code, uint8_t *p_buff, uint16_t length);
void app_dsp_m55_bridge_send_data_without_waiting_rsp(uint16_t cmdcode, uint8_t *p_buff, uint16_t length);
void app_dsp_m55_bridge_send_data_with_waiting_rsp(uint16_t cmdcode, uint8_t *p_buff, uint16_t length);
/* Low-level Driver Handler APIs */
unsigned int app_dsp_m55_bridge_data_received(const void* data, unsigned int len);
void app_dsp_m55_bridge_data_tx_done(const void* data, unsigned int len);

/* Not usually used APIs */
void app_dsp_m55_bridge_send_instant_cmd_data(uint16_t cmdcode, uint8_t *p_buff, uint16_t length);
void app_dsp_m55_bridge_free_tx_mailbox(void);
app_dsp_m55_bridge_instant_cmd_instance_t* app_dsp_m55_bridge_find_instant_cmd_entry(uint16_t cmdcode);
app_dsp_m55_bridge_task_cmd_instance_t* app_dsp_m55_bridge_find_task_cmd_entry(uint16_t cmdcode);
void app_dsp_m55_bridge_fetch_tx_mailbox(uint16_t cmd_code, uint8_t *buffer, uint16_t *length, uint16_t threshold);
uint32_t app_dsp_m55_bridge_get_tx_mailbox_cnt(void);

#ifdef __cplusplus
}
#endif

#endif

