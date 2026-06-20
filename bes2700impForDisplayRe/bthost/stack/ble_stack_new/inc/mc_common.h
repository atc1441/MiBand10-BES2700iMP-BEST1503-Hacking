/***************************************************************************
 *
 * Copyright (c) 2015-2023 BES Technic
 *
 * Authored by BES CD team (Blueelf Prj).
 *
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
#ifndef __MCS_COMMON__
#define __MCS_COMMON__

#include "bluetooth.h"
#include "gatt_service.h"
#include "gaf_cfg.h"

#if (ACC_OTS_ENABLE) || (ACC_OTC_ENABLE)
#include "ot_common.h"
#endif /// (ACC_OTS_ENABLE) || (ACC_OTC_ENABLE)

/// Minimum length
#define MCS_CP_POS_WR_LEN_MIN            (1)
/// Maximum length
#define MCS_CP_POS_WR_LEN_MAX            (5)
/// Media CP characteristic value ntf len
#define MCS_CP_POS_NTF_LEN               (2)
/// Length of Object ID
#define MCS_OBJ_ID_LEN                   (6)
/// Length of Track Changed characteristic value
#define MCS_TRACK_CHANGED_LEN            (0)
/// Length of Media State characteristic value
#define MCS_MEDIA_STATE_LEN              (1)
/// Length of Track Duration characteristic value
#define MCS_TRACK_DURATION_LEN           (4)
/// Length of Track Position characteristic value
#define MCS_TRACK_POSITION_LEN           (4)
/// Length of Playback Speed characteristic value
#define MCS_PLAYBACK_SPEED_LEN           (1)
/// Length of Seeking Speed characteristic value
#define MCS_SEEKING_SPEED_LEN            (1)
/// Length of Playing Order characteristic value
#define MCS_PLAYING_ORDER_LEN            (1)
/// Length of Playing Order Supported characteristic value
#define MCS_PLAYING_ORDER_SUPP_LEN       (2)
/// Length of Media Control Point Opcodes Supported characteristic value
#define MCS_MEDIA_CP_OPCODES_SUPP_LEN    (4)
/// Length of Search Control Point Result Code value
#define MCS_SEARCH_CP_RESULT_LEN         (1)
/// Indicate that Track Duration is unknown
#define MCS_TRACK_DUR_UNKNOWN            (0xFFFFFFFF)

/// Characteristic type values
/// Values are ordered so that the notification-capable characteristic type values are equal
/// to the characteristic type values
enum mcs_char_type
{
    /// Media Player Name characteristic
    MCS_CHAR_TYPE_PLAYER_NAME = 0,
    /// Track Changed characteristic
    MCS_CHAR_TYPE_TRACK_CHANGED,
    /// Track Title characteristic
    MCS_CHAR_TYPE_TRACK_TITLE,
    /// Track Duration characteristic
    MCS_CHAR_TYPE_TRACK_DURATION,
    /// Track Position characteristic
    MCS_CHAR_TYPE_TRACK_POSITION,
    /// Playback Speed characteristic
    MCS_CHAR_TYPE_PLAYBACK_SPEED,
    /// Seeking Speed characteristic
    MCS_CHAR_TYPE_SEEKING_SPEED,
    /// Current Track Object ID characteristic
    MCS_CHAR_TYPE_CUR_TRACK_OBJ_ID,
    /// Next Track Object ID characteristic
    MCS_CHAR_TYPE_NEXT_TRACK_OBJ_ID,
    /// Current Group Object ID characteristic
    MCS_CHAR_TYPE_CUR_GROUP_OBJ_ID,
    /// Parent Group Object ID characteristic
    MCS_CHAR_TYPE_PARENT_GROUP_OBJ_ID,
    /// Playing Order characteristic
    MCS_CHAR_TYPE_PLAYING_ORDER,
    /// Media State characteristic
    MCS_CHAR_TYPE_MEDIA_STATE,
    /// Media Control Point
    MCS_CHAR_TYPE_MEDIA_CP,
    /// Media Control Point Opcodes Supported
    MCS_CHAR_TYPE_MEDIA_CP_OPCODES_SUPP,
    /// Search Results Object ID
    MCS_CHAR_TYPE_SEARCH_RESULTS_OBJ_ID,
    /// Search Control Point
    MCS_CHAR_TYPE_SEARCH_CP,

    /// All characteristics above are notification-capable
    MCS_NTF_CHAR_TYPE_MAX,

    /// Current Track Segments Object ID characteristic
    MCS_CHAR_TYPE_CUR_TRACK_SEG_OBJ_ID = MCS_NTF_CHAR_TYPE_MAX,
    /// Media Player Icon Object ID characteristic
    MCS_CHAR_TYPE_PLAYER_ICON_OBJ_ID,
    /// Media Player Icon URL characteristic
    MCS_CHAR_TYPE_PLAYER_ICON_URL,
    /// Playing Order Supported characteristic
    MCS_CHAR_TYPE_PLAYING_ORDER_SUPP,
    /// Content Control ID
    MCS_CHAR_TYPE_CCID,

    MCS_CHAR_TYPE_MAX,
};

/// Descriptor type values
enum mcs_desc_type
{
    /// Client Characteristic Configuration descriptor for Media Player Name characteristic
    MCS_DESC_TYPE_CCC_PLAYER_NAME = 0,
    /// Client Characteristic Configuration descriptor for Track Changed characteristic
    MCS_DESC_TYPE_CCC_TRACK_CHANGED,
    /// Client Characteristic Configuration descriptor for Track Title characteristic
    MCS_DESC_TYPE_CCC_TRACK_TITLE,
    /// Client Characteristic Configuration descriptor for Track Duration characteristic
    MCS_DESC_TYPE_CCC_TRACK_DURATION,
    /// Client Characteristic Configuration descriptor for Track Position characteristic
    MCS_DESC_TYPE_CCC_TRACK_POSITION,
    /// Client Characteristic Configuration descriptor for Playback Speed characteristic
    MCS_DESC_TYPE_CCC_PLAYBACK_SPEED,
    /// Client Characteristic Configuration descriptor for Seeking Speed characteristic
    MCS_DESC_TYPE_CCC_SEEKING_SPEED,
    /// Client Characteristic Configuration descriptor for Current Track Object ID characteristic
    MCS_DESC_TYPE_CCC_CUR_TRACK_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Next Track Object ID characteristic
    MCS_DESC_TYPE_CCC_NEXT_TRACK_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Current Group Object ID characteristic
    MCS_DESC_TYPE_CCC_CUR_GROUP_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Parent Group Object ID characteristic
    MCS_DESC_TYPE_CCC_PARENT_GROUP_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Playing Order characteristic
    MCS_DESC_TYPE_CCC_PLAYING_ORDER,
    /// Client Characteristic Configuration descriptor for Media State characteristic
    MCS_DESC_TYPE_CCC_MEDIA_STATE,
    /// Client Characteristic Configuration descriptor for Media Control Point
    MCS_DESC_TYPE_CCC_MEDIA_CP,
    /// Client Characteristic Configuration descriptor for Media Control Point Opcodes Supported
    MCS_DESC_TYPE_CCC_MEDIA_CP_OPCODES_SUPP,
    /// Client Characteristic Configuration descriptor for Search Results Object ID
    MCS_DESC_TYPE_CCC_SEARCH_RESULTS_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Search Control Point
    MCS_DESC_TYPE_CCC_SEARCH_CP,

    MCS_DESC_TYPE_MAX,
};

/// Operation code values
enum mcs_opcode
{
    MCS_OPCODE_MIN = 0,
    /// Start playing the current track
    MCS_OPCODE_PLAY = 1,
    /// Pause playing the current track
    MCS_OPCODE_PAUSE,
    /// Fast rewind the current track
    MCS_OPCODE_FAST_RW,
    /// Fast forward the current track
    MCS_OPCODE_FAST_FW,
    /// Stop current activity and return to inactive state
    MCS_OPCODE_STOP,
    /// Set the current position relative to the current position
    MCS_OPCODE_MOVE_RELATIVE = 16,
    /// Set the current position to the previous segment of the current track
    MCS_OPCODE_PREV_SEG = 32,
    /// Set the current position to the next segment of the current track
    MCS_OPCODE_NEXT_SEG,
    /// Set the current position to the first segment of the current track
    MCS_OPCODE_FIRST_SEG,
    /// Set the current position to the last segment of the current track
    MCS_OPCODE_LAST_SEG,
    /// Set the current position to the nth segment of the current track
    MCS_OPCODE_GOTO_SEG,
    /// Set the current track to the previous track in the current group playing order
    MCS_OPCODE_PREV_TRACK = 48,
    /// Set the current track to the next track in the current group playing order
    MCS_OPCODE_NEXT_TRACK,
    /// Set the current track to the first track in the current group playing order
    MCS_OPCODE_FIRST_TRACK,
    /// Set the current track to the last track in the current group playing order
    MCS_OPCODE_LAST_TRACK,
    /// Set the current track to the nth track in the current group playing order
    MCS_OPCODE_GOTO_TRACK,
    /// Set the current group to the previous group in the sequence of groups
    MCS_OPCODE_PREV_GROUP = 64,
    /// Set the current group to the next group in the sequence of groups
    MCS_OPCODE_NEXT_GROUP,
    /// Set the current group to the first group in the sequence of groups
    MCS_OPCODE_FIRST_GROUP,
    /// Set the current group to the next group in the sequence of groups
    MCS_OPCODE_LAST_GROUP,
    /// Set the current group to the nth group in the sequence of groups
    MCS_OPCODE_GOTO_GROUP,

    MCS_OPCODE_MAX,

    MCS_MEDIA_CP_OPCODES_SUPP_MASK = 0x001FFFFF,
};

/// Action type values
enum mcs_action
{
    /// No action
    MCS_ACTION_NO_ACTION = 0,
    /// Play
    MCS_ACTION_PLAY,
    /// Pause
    MCS_ACTION_PAUSE,
    /// Stop
    MCS_ACTION_STOP,
    /// Fast Forward or Fast Rewind
    MCS_ACTION_SEEK,
    /// Current Track becomes invalid
    MCS_ACTION_INACTIVE,
    /// Change track
    MCS_ACTION_CHANGE_TRACK,

    MCS_ACTION_MAX
};

/// Search type values
enum mcs_search_type
{
    MCS_SEARCH_TYPE_MIN = 1,
    /// Track Name
    MCS_SEARCH_TRACK_NAME = MCS_SEARCH_TYPE_MIN,
    /// Artist Name
    MCS_SEARCH_ARTIST_NAME,
    /// Album Name
    MCS_SEARCH_ALBUM_NAME,
    /// Group Name
    MCS_SEARCH_GROUP_NAME,
    /// Earliest Year
    MCS_SEARCH_EARLIEST_YEAR,
    /// Latest Year
    MCS_SEARCH_LATEST_YEAR,
    /// Genre
    MCS_SEARCH_GENRE,
    /// Only Tracks
    MCS_SEARCH_ONLY_TRACKS,
    /// Only Groups
    MCS_SEARCH_ONLY_GROUPS,

    MCS_SEARCH_TYPE_MAX,
};

/// Media State values
enum mcs_media_state
{
    /// Inactive
    MCS_MEDIA_STATE_INACTIVE = 0,
    /// Playing
    MCS_MEDIA_STATE_PLAYING,
    /// Paused
    MCS_MEDIA_STATE_PAUSED,
    /// Seeking
    MCS_MEDIA_STATE_SEEKING,

    MCS_MEDIA_STATE_MAX,
};

/// Playing Order values
enum mcs_play_order
{
    MCS_PLAY_ORDER_MIN = 1,
    /// A single track is played once there is no next track
    MCS_PLAY_ORDER_SINGLE_ONCE = MCS_PLAY_ORDER_MIN,
    /// A single track is played repeatedly; the next track is the current track
    MCS_PLAY_ORDER_SINGLE_REPEAT,
    /// The tracks within a group are played once in track order
    MCS_PLAY_ORDER_ORDER_ONCE,
    /// The tracks within a group are played in track order repeatedly
    MCS_PLAY_ORDER_ORDER_REPEAT,
    /// The tracks within a group are played once only from the oldest first
    MCS_PLAY_ORDER_OLDEST_ONCE,
    /// The tracks within a group are played from the oldest first repeatedly
    MCS_PLAY_ORDER_OLDEST_REPEAT,
    /// The tracks within a group are played once only from the newest first
    MCS_PLAY_ORDER_NEWEST_ONCE,
    /// The tracks within a group are played from the newest first repeatedly
    MCS_PLAY_ORDER_NEWEST_REPEAT,
    /// The tracks within a group are played in random order once
    MCS_PLAY_ORDER_SHUFFLE_ONCE,
    /// The tracks within a group are played in random order repeatedly
    MCS_PLAY_ORDER_SHUFFLE_REPEAT,

    MCS_PLAY_ORDER_MAX,
    /// Mask for Playing Order Supported characteristic value
    MCS_PLAY_ORDER_SUPP_MASK = 0x03FF,
};

/// Result code values of Media Control Point Notificaiton
enum mcs_media_cp_result
{
    MCS_MEDIA_CP_RESULT_MIN = 0,
    /// Success
    MCS_MEDIA_CP_RESULT_SUCCESS,
    /// Operation code not supported
    MCS_MEDIA_CP_RESULT_NOT_SUPPORTED,
    /// Media player inactive
    MCS_MEDIA_CP_RESULT_PLAYER_INACTIVE,
    /// Command cannot be completed
    MCS_MEDIA_CP_RESULT_CANNOT_COMPLETE,

    MCS_MEDIA_CP_RESULT_MAX,
};

/// Result code values of Search Control Point Notification
enum mcs_search_cp_result
{
    MCS_SEARCH_CP_RESULT_MIN = 1,
    /// Success
    MCS_SEARCH_CP_RESULT_SUCCESS = MCS_SEARCH_CP_RESULT_MIN,
    /// Failure
    MCS_SEARCH_CP_RESULT_FAILURE,

    MCS_SEARCH_CP_RESULT_MAX,
};


/// Search Control Point characteristic value position
enum mcs_search_cp_pos
{
    /// Item length
    MCS_SEARCH_CP_ITEM_LEN_POS = 0,
    /// Type
    MCS_SEARCH_CP_TYPE_POS,
    /// Minimum length
    MCS_SEARCH_CP_LEN_MIN,
    /// Minimum item length
    MCS_SEARCH_CP_ITEM_LEN_MIN = 1,
    /// Parameter
    MCS_SEARCH_CP_PARAM_POS = MCS_SEARCH_CP_LEN_MIN,
    /// Maximum length
    MCS_SEARCH_CP_LEN_MAX = 64,
};

/// Application error codes for Media Control Service
enum mcs_app_err
{
    /// Value changed during Read Long
    MCS_ERR_VALUE_CHANGED = 0x80,

    MCS_ERR_UNEXPECTED,
};

#if (ACC_OTS_ENABLE) || (ACC_OTC_ENABLE)
/// Object ID
typedef ot_object_id_t mcs_object_id_t;
#else
/// Object ID
typedef uint8_t mcs_object_id_t;
#endif /// (ACC_OTS_ENABLE) || (ACC_OTC_ENABLE)

// Media CP pdu
typedef struct mcs_media_cp_req_pdu
{
    uint8_t op_code;
    uint8_t param[GAF_ARRAY_EMPTY];
} mcs_media_cp_req_t;

typedef struct mcs_media_cp_notify
{
    uint8_t op_code;
    uint8_t result;
} mcs_media_cp_ntf_t;

/// MCS CHARACTER UUID LIST
extern __CONST uint16_t mcs_char_uuid_list[MCS_CHAR_TYPE_MAX];

/**
 * @brief Media control service character uuid to character type
 *
 * @param  uuid        Character uuid
 *
 * @return uint8_t     Character type
 */
uint8_t mcs_char_uuid_to_char_type_enum(uint16_t uuid);

/**
 * @brief Media control service check is opcode supported
 *
 * @param  opcode_supp_bf
 *                     Opcode supported bitfiled
 * @param  opcode      Operation code
 *
 * @return true        Opcode is suooported
 * @return false       Opcode is not suooported
 */
bool mcs_check_is_opcode_supported(uint32_t opcode_supp_bf, uint8_t opcode);

#if (ACC_OTS_ENABLE) || (ACC_OTC_ENABLE)
/**
 * @brief Media control service is object ID value is valid
 *
 * @param  p_object_id Object ID value pointer
 *
 * @return true        Object ID value is valid
 * @return false       Object ID value is invalid
 */
bool mcs_is_object_id_valid(const ot_object_id_t *p_object_id);

/**
 * @brief Media control service check search item parameters
 *
 * @param  p_param     Parameters
 * @param  param_len   Parameters length
 *
 * @return uint16_t    status
 */
uint16_t mcs_check_search_items(const uint8_t *p_param, uint8_t param_len);
#endif /// (ACC_OTS_ENABLE) || (ACC_OTC_ENABLE)

#endif /// __MCS_COMMON__