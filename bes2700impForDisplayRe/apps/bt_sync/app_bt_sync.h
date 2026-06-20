/***************************************************************************
 *
 * Copyright 2015-2025 BES.
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

#ifndef __APP_BT_STAMP_H__
#define __APP_BT_STAMP_H__
#include "plat_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************macro defination*****************************/
// Default value. decide whether to add it or not by CMD handler
#define APP_BT_SYNC_THREAD_STACK_SIZE               (2048)
#define APP_BT_SYNC_INVALID_CHANNEL                 (0xFF)
// Total 4, channel0 reserved for HFP/A2DP, channel1 for prompt
#define APP_BT_SYNC_CHANNEL_OFFSET                  (2)
#define APP_BT_SYNC_CHANNEL_MAX                     (2)
// Depending on chipset
#define APP_BT_SYNC_CHANNEL_TOTAL                   (4)
#define APP_BT_SYNC_SUPERVISON_TIMEOUT              (3000)
#define APP_BT_SYNC_REUSED_CHANNEL                  (0xFE)
#define APP_BT_SYNC_RESULT_FAILED                   (false)
#define APP_BT_SYNC_RESULT_PASS                     (true)
#define APP_BT_SYNC_EXTRA_INFO_LEN                  (128)

typedef enum
{
    APP_BT_SYNC_OP_RETRIGGER                = 0x00000000,
    APP_BT_SYNC_OP_VOLUME                   = 0x00000001,
    APP_BT_SYNC_OP_VOLUME_OFFSET            = 0x00000002,
    APP_BT_SYNC_OP_SWITCH                   = 0x00000003,
    APP_BT_SYNC_OP_BIS_SELFSCAN             = 0x00000004,

    APP_BT_SYNC_OP_MAX                       ,
// TODO: add more operation codes
} APP_BT_SYNC_OP_CODE_E;

typedef enum
{
    // Find new trigger channel
    APP_BT_SYNC_POLICY_DEFAULT,
    // Allow same opcode runing, audio retrigger
    APP_BT_SYNC_POLICY_MULTIPLEX,
    // Refresh trigger if found same opCode, Volume
    APP_BT_SYNC_POLICY_UPDATE,
} APP_BT_SYNC_POLICY_E;

extern uint32_t __app_bt_sync_command_handler_table_start[];
extern uint32_t __app_bt_sync_command_handler_table_end[];

#define APP_BT_SYNC_COMMAND_TO_ADD(opCode, cmdhandler, statusNotify)        \
    static const BT_SYNC_JOB_LIST_T syncOp##opCode##_entry              \
    __attribute__((used, section(".app_bt_sync_command_handler_table"))) =  \
    {(opCode), (cmdhandler), (statusNotify)};

#define APP_BT_SYNC_COMMAND_PTR_FROM_ENTRY_INDEX(index)    \
    ((BT_SYNC_JOB_LIST_T *)((uint32_t)__app_bt_sync_command_handler_table_start \
    + (index)*sizeof(BT_SYNC_JOB_LIST_T)))

/******************************type defination******************************/
typedef void (*APP_BT_SYNC_HANDLER_T)(void);
typedef void (*APP_BT_SYNC_STATUS_NOTIFY_T)(uint32_t opCode, bool triStatus, bool triInfoSentStatus);
typedef void (*APP_BT_SYNC_INFO_REPORT_T)(uint32_t opCode, uint8_t *ptrInfo, uint8_t length);

typedef struct {
    uint8_t                                       triResult;
    uint8_t                                         workIdx;
} __attribute__((packed)) BT_SYNC_MSG_BLOCK_T;

typedef struct {
    uint32_t                                         opCode;
    APP_BT_SYNC_HANDLER_T                      syncCallback;
    APP_BT_SYNC_STATUS_NOTIFY_T                statusNotify;
} __attribute__((packed)) BT_SYNC_JOB_LIST_T;

typedef struct {
    uint8_t                                          triChl;
    uint8_t                                          policy;
    uint32_t                                         opCode;
    uint32_t                                        triTick;
} __attribute__((packed)) BT_SYNC_TRIGGER_INFO_T;

typedef struct {
    BT_SYNC_TRIGGER_INFO_T                     trigger_info;
    uint8_t          extra_info[APP_BT_SYNC_EXTRA_INFO_LEN];
}  __attribute__((packed)) BT_SYNC_SHARE_INFO_T;

typedef enum {
    // No synchronization
    BT_SYNC_TYPE_NONE = 0,
     // Common synchronization
    BT_SYNC_TYPE_COMMON,
    // Audio synchronization
    BT_SYNC_TYPE_AUDIO,

    BT_SYNC_TYPE_LIMIT,
} BT_SYNC_TYPE_E;
_Static_assert(BT_SYNC_TYPE_LIMIT <= 4, "Error: BT_SYNC_TYPE_E values must fit within 2 bits");

typedef struct {
    // Synchronization type (2 bits, @BT_SYNC_TYPE_E)
    uint8_t                                         syncType:2;

    // Indicates if the trigger information has been shared with peer device (1 bit)
    uint8_t                                     twsShareDone:1;

    // Indicates the work instance has been cancelled
    uint8_t                                      isAbandoned:1;

    // Reserved bits for future use (5 bits)
    uint8_t                                         reserved:4;

    // Trigger information associated with the synchronization
    BT_SYNC_TRIGGER_INFO_T                             triInfo;

    // Run-time timeout left in milliseconds
    uint32_t                                     msTillTimeout;
} __attribute__((packed)) BT_SYNC_INSTANCE_T;

typedef struct {
    BT_SYNC_INSTANCE_T workInstance[APP_BT_SYNC_CHANNEL_MAX];
    uint32_t                                    lastSysTicks;
    uint8_t                                   supervisorMask;
    uint8_t                                      nearest_idx;
} __attribute__((packed)) BT_SYNC_ENV_T;

/**
 * @brief Get available trigger channel from BT trigger module
 * NOTE: There are 4 channel for BT trigger, channel0 is reserved for A2DP/HFP playback
 * @param[in]  policy           For an ongoing and identical OPCODE processing strategy
 * @return uint8_t              The avaliable trigger channel
 */
uint8_t app_bt_sync_get_available_trigger_channel(uint32_t opcode, uint8_t policy);

/**
 * @brief Release the given BT trigger channel
 *
 * @param chnl          The trigger channel to release
 */
void app_bt_sync_release_trigger_channel(uint8_t chnl);

/**
 ****************************************************************************************
 * @brief Enable the both buds trigger function
 *
 * @param[out] none
 * @param[in]  opCode             Point to the expected callback handler
 * @param[in]  extra_len          The length of information shared between TWS
 * @param[in]  p_extra_info       Point to the share info
 * @param[in]  policy             For an ongoing and identical OPCODE processing strategy
 *
 * @return Execute result status
 ****************************************************************************************/
bool app_bt_sync_enable(uint32_t opCode, uint8_t extra_len, uint8_t *p_extra_info, uint8_t policy);

/**
 ****************************************************************************************
 * @brief SE device to process the sync command
 *
 * @param[out] none
 * @param[in]  p_buff             Trigger info from PE device
 * @param[in]  length             Length of information shared between TWS
 *
 * @return none
 ****************************************************************************************/
void app_bt_sync_tws_cmd_handler(uint8_t *p_buff, uint16_t length);

/**
 ****************************************************************************************
 * @brief trigger info send done
 * @param[out] none
 * @param[in]  ptrParam             Trigger info from PE device
 * @param[in]  paramLen             Length of information shared between TWS
 ****************************************************************************************/
void app_bt_sync_send_tws_cmd_done(uint8_t *ptrParam, uint16_t paramLen);

/**
 ****************************************************************************************
 * @brief Cancel the previously initiated BT SYNC operation.
 * @param[out] none
 * @param[in]  opCode                operation code
 ****************************************************************************************/
void app_bt_sync_abandon_work_instance(uint32_t opCode);

/**
 ****************************************************************************************
 * @brief register the callback for reporting the extra shared information
 ****************************************************************************************/
void app_bt_sync_register_report_info_callback(APP_BT_SYNC_INFO_REPORT_T cb);

void app_bt_sync_get_master_time_from_slave_time(uint32_t SlaveBtTicksUs,
    uint32_t* p_master_clk_cnt, uint16_t* p_master_bit_cnt);
uint32_t app_bt_sync_get_slave_time_from_master_time(uint32_t master_clk_cnt,
    uint16_t master_bit_cnt);

#ifdef __cplusplus
}
#endif
#endif

