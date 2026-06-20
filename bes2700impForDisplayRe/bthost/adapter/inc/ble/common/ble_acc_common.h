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
#ifndef __BLE_ACC_COMMON_H__
#define __BLE_ACC_COMMON_H__
#include "ble_common_define.h"
#ifdef BLE_HOST_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

#define AOB_COMMON_CALL_MAX_NB_IDS  (2)
#define AOB_COMMON_INVALID_CALL_ID  (0)
#define AOB_COMMON_CALL_INVALID_IDS (0xFF)

#define APP_ACC_DFT_TECHNO_VAL ACC_TB_TECHNO_4G
#define APP_ACC_DFT_SIGN_STRENGTH_VAL (100)
#define APP_ACC_DFT_PROV_NAME "CHINA MOBILE"
#define APP_ACC_DFT_URI_SCHEMES_LIST "tel,sip,skype"
#define APP_ACC_DFT_CALL_INCOMING_URI "tel:10086"
#define APP_ACC_DFT_CALL_INCOMING_TGT_URI "tel:10086"
#define APP_ACC_DFT_CALL_INCOMING_FRIENDLY_NAME "INCOMING CALL"
#define APP_ACC_DFT_CALL1_INCOMING_URI "tel:100861111111111"
#define APP_ACC_DFT_CALL1_INCOMING_TGT_URI "tel:100861111111111"
#define APP_ACC_DFT_CALL1_INCOMING_FRIENDLY_NAME "INCOMING1 CALL11111"
#define APP_ACC_DFT_CALL_OUTGOING_URI "tel:10086"
#define APP_ACC_DFT_CALL_OUTGOING_URI_SKYPE "tel:+19991111234"
#define APP_ACC_DFT_CALL_OUTGOING_FRIENDLY_NAME "OUTGOING CALL"

/// MEDIA control operation code values
typedef enum
{
    /// Start playing the current track
    AOB_MCP_OP_PLAY = 1,
    /// Pause playing the current track
    AOB_MCP_OP_PAUSE,
    /// Fast rewind the current track
    AOB_MCP_OP_FAST_RW,
    /// Fast forward the current track
    AOB_MCP_OP_FAST_FW,
    /// Stop current activity and return to inactive state
    AOB_MCP_OP_STOP,
    /// Set the current position relative to the current position
    AOB_MCP_OP_MOVE_RELATIVE = 16,
    /// Set the current position to the previous segment of the current track
    AOB_MCP_OP_PREV_SEG = 32,
    /// Set the current position to the next segment of the current track
    AOB_MCP_OP_NEXT_SEG,
    /// Set the current position to the first segment of the current track
    AOB_MCP_OP_FIRST_SEG,
    /// Set the current position to the last segment of the current track
    AOB_MCP_OP_LAST_SEG,
    /// Set the current position to the nth segment of the current track
    AOB_MCP_OP_GOTO_SEG,
    /// Set the current track to the previous track in the current group playing order
    AOB_MCP_OP_PREV_TRACK = 48,
    /// Set the current track to the next track in the current group playing order
    AOB_MCP_OP_NEXT_TRACK,
    /// Set the current track to the first track in the current group playing order
    AOB_MCP_OP_FIRST_TRACK,
    /// Set the current track to the last track in the current group playing order
    AOB_MCP_OP_LAST_TRACK,
    /// Set the current track to the nth track in the current group playing order
    AOB_MCP_OP_GOTO_TRACK,
    /// Set the current group to the previous group in the sequence of groups
    AOB_MCP_OP_PREV_GROUP = 64,
    /// Set the current group to the next group in the sequence of groups
    AOB_MCP_OP_NEXT_GROUP,
    /// Set the current group to the first group in the sequence of groups
    AOB_MCP_OP_FIRST_GROUP,
    /// Set the current group to the last group in the sequence of groups
    AOB_MCP_OP_LAST_GROUP,
    /// Set the current group to the nth group in the sequence of groups
    AOB_MCP_OP_GOTO_GROUP,

    AOB_MCP_MEDIA_CP_OPCODES_SUPP_MASK = 0x001FFFFF,
} AOB_MCP_OPCODE_E;

/// Action type values
typedef enum
{
    /// No action
    AOB_MCP_ACTION_NO_ACTION = 0,
    /// Play
    AOB_MCP_ACTION_PLAY,
    /// Pause
    AOB_MCP_ACTION_PAUSE,
    /// Stop
    AOB_MCP_ACTION_STOP,
    /// Fast Forward or Fast Rewind
    AOB_MCP_ACTION_SEEK,
    /// Current Track becomes invalid
    AOB_MCP_ACTION_INACTIVE,
    /// Change track
    AOB_MCP_ACTION_CHANGE_TRACK,

    AOB_MCP_ACTION_MAX,
} AOB_MCP_ACTION_E;

#ifdef __cplusplus
}
#endif
#endif
#endif /* __BLE_ACC_COMMON_H__ */
