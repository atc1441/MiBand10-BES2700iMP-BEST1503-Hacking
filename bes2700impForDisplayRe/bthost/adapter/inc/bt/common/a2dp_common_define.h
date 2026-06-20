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
#ifndef __A2DP_COMMON_DEFINE_H__
#define __A2DP_COMMON_DEFINE_H__
#include "bt_common_define.h"
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_A2DP_VOL   (127)

#define BTIF_AVRCP_NUM_PLAYER_SETTINGS  4
#define BTIF_AVRCP_NUM_MEDIA_ATTRIBUTES 8

#define BTIF_CONFIG_A2DP_SBC_ON 1

#if defined(A2DP_AAC_ON) || defined(A2DP_SOURCE_AAC_ON)
#define BTIF_CONFIG_A2DP_AAC_ON 1
#else
#define BTIF_CONFIG_A2DP_AAC_ON 0
#endif

#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON) || defined(A2DP_SOURCE_LHDCV5_ON)
#define BTIF_CONFIG_A2DP_LHDC_ON 2
#else
#define BTIF_CONFIG_A2DP_LHDC_ON 0
#endif

#if defined(A2DP_LDAC_ON)
#define BTIF_CONFIG_A2DP__ON 1
#define BTIF_CONFIG_A2DP_LDAC_ON 1
#else
#define BTIF_CONFIG_A2DP_LDAC_ON 0
#endif

#if defined(A2DP_SCALABLE_ON)
#define BTIF_CONFIG_A2DP_SCALABLE_ON 1
#else
#define BTIF_CONFIG_A2DP_SCALABLE_ON 0
#endif

#if defined(MASTER_USE_OPUS) || defined(ALL_USE_OPUS)
#define BTIF_CONFIG_A2DP_OPUS_ON 1
#else
#define BTIF_CONFIG_A2DP_OPUS_ON 0
#endif

#if defined(A2DP_L2HC_ON)
#define BTIF_CONFIG_A2DP_L2HC_ON 1
#else
#define BTIF_CONFIG_A2DP_L2HC_ON 0
#endif

#if defined(A2DP_MIHC_ON)
#define BTIF_CONFIG_A2DP_MIHC_ON 1
#else
#define BTIF_CONFIG_A2DP_MIHC_ON 0
#endif

#define SYS_MAX_A2DP_LOCAL_SEPS \
    (BTIF_CONFIG_A2DP_SBC_ON + \
        BTIF_CONFIG_A2DP_AAC_ON + \
        BTIF_CONFIG_A2DP_LHDC_ON + \
        BTIF_CONFIG_A2DP_LDAC_ON + \
        BTIF_CONFIG_A2DP_SCALABLE_ON + \
        BTIF_CONFIG_A2DP_OPUS_ON + \
        BTIF_CONFIG_A2DP_L2HC_ON + \
        BTIF_CONFIG_A2DP_MIHC_ON + 1)

#define SYS_MAX_A2DP_STREAMS (BT_DEVICE_NUM+BT_SOURCE_DEVICE_NUM)
#define SYS_MAX_AVRCP_CHNS  (BT_DEVICE_NUM+BT_SOURCE_DEVICE_NUM)

typedef uint8_t btif_a2dp_event_t;

/**  Accept or reject by calling btif_a2dp_open_stream_rsp().
 */
#define BTIF_A2DP_EVENT_STREAM_OPEN_IND           1

/** This event is  the response to a call from   btif_a2dp_open_stream() or
 *  btif_a2dp_open_stream_rsp().
 */
#define BTIF_A2DP_EVENT_STREAM_OPEN               2

/** This event is received during the establishment of an
 *  outgoing stream (btif_a2dp_open_stream()) to indicate the capabilities of the
 *  remote device.  This event may also be received after calling
 *  btif_a2dp_get_stream_capabilities().
 */
#define BTIF_A2DP_EVENT_CODEC_INFO                3

/** This event is received  during the establishment of an
 *  outgoing stream (btif_a2dp_open_stream()) to indicated the capabilities of the
 *  remote device.  This event may also be received after calling
 *  btif_a2dp_get_stream_capabilities().
 *
 */
#define BTIF_A2DP_EVENT_CP_INFO                   4

/**  If this event is received, then the remote device
 *  supports the Delay Reporting feature.
 */
#define BTIF_A2DP_EVENT_DELAY_REPORTING_INFO      5

/** This event is received  when a stream is opening and
 *  all matching capabilities have been found.  The application must call
 *  btif_a2dp_set_stream_config() to configure the stream.  If successful, the
 *  stream will be opened.  btif_a2dp_close_stream() can also be called to close
 *  the stream.  This event is only received on outgoing connections.
 */
#define BTIF_A2DP_EVENT_GET_CONFIG_IND            6

/** This event is received after calling btif_a2dp_get_stream_capabilities().  This
 *  event is received after all the capabilities of the remote device have
 *  been indicated (see BTIF_A2DP_EVENT_CODEC_INFO and BTIF_A2DP_EVENT_CP_INFO).
 */
#define BTIF_A2DP_EVENT_GET_CAP_CNF               7

/** This event is received  when an open stream has been
 *  closed.  This can happen as a result of a call to btif_a2dp_close_stream(), if
 *  the stream has been closed by the remote device, if a link loss has been
 *  detected, or if the remote device rejects a request to open the stream.
 *
 */
#define BTIF_A2DP_EVENT_STREAM_CLOSED             8

/** This event is received  when an open stream has been
 *  set to the idle state.   result of a call to btif_a2dp_idle_stream().
 */
#define BTIF_A2DP_EVENT_STREAM_IDLE               9

/** When the remote device requests streaming to begin, this event will be
 *  received .  The application can accept or reject
 *  this request by calling btif_a2dp_start_stream_rsp(). (Note: this event
 *  will be received only after BTIF_A2DP_EVENT_STREAM_OPEN_IND but it may arrive
 *  before BTIF_A2DP_EVENT_STREAM_OPEN.)
 */
#define BTIF_A2DP_EVENT_STREAM_START_IND         10

/** When streaming has been started, this event will be received.  This can happen as the result to a call to
 *  btif_a2dp_start_stream() or btif_a2dp_start_stream_rsp().
 */
#define BTIF_A2DP_EVENT_STREAM_STARTED           11

/** When streaming has been suspended, this event will be received .  as the result to a call to
 *  btif_a2dp_suspend_stream(), or when the remote device suspends the stream.
 */
#define BTIF_A2DP_EVENT_STREAM_SUSPENDED         12

/** When the remote device wishes to reconfigure an open stream, this event
 *  is received by the application.  The application can accept or reject
 *  the request by calling btif_a2dp_reconfig_stream_rsp().
 *
 */
#define BTIF_A2DP_EVENT_STREAM_RECONFIG_IND      13

/** When an open stream is reconfigured, this event is received by the
 *  application.
 *
 */
#define BTIF_A2DP_EVENT_STREAM_RECONFIG_CNF      14

/** This event is received when the remote device requests the security
 *  process to begin.  The application responds to this request by calling
 *  btif_a2dp_security_control_rsp().
 */
#define BTIF_A2DP_EVENT_STREAM_SECURITY_IND      15

/** This event is received  when the remote device responds
 *  to the security process request.
 */
#define BTIF_A2DP_EVENT_STREAM_SECURITY_CNF      16

/** When the stream is aborted, this event is received by the application.
 *  This can happen in response to a request from the remote device to abort
 *  the stream, or as the result of a call to btif_a2dp_abort_stream().  When a
 *  stream is aborted, the stream is closed.
 */
#define BTIF_A2DP_EVENT_STREAM_ABORTED           17

/** This event is received when stream data has been received from the remote
 *  device.  The data is raw and is not parsed by A2DP.  It should contain
 *  a single media packet.
 */
#define BTIF_A2DP_EVENT_STREAM_DATA_IND          18

/** This event is received when raw data has been sent to the remote device.
 *  This happens as the result of a call to btif_a2dp_stream_send_raw_packet().
 *
 */
#define BTIF_A2DP_EVENT_STREAM_PACKET_SENT       19

/** This event is received when SBC data has been sent to the remote device.
 *  This happens as the result of a call to btif_a2dp_stream_send_sbc_packet().
 */
#define BTIF_A2DP_EVENT_STREAM_SBC_PACKET_SENT   20

/** This event is received by a Source when the Sink reports the value of its
 *  buffer/processing delay.  This may happen when the stream is configured
 *  (or reconfigured), and when the stream is in the streaming state.
 *
 */
#define BTIF_A2DP_EVENT_DELAY_REPORT_IND         21

/** This event is received by a Sink when the Source acknowldeges the
 *  transmitted buffer/processing delay.
 */
#define BTIF_A2DP_EVENT_DELAY_REPORT_CNF         22

/** When the the lower level AVDTP connection is established between the
 *  local and remote devices, this event will be generated.
 *
 *  During this callback, the 'p.device' parameter contains a pointer to the
 *  device that was connected.
 */
#define BTIF_A2DP_EVENT_AVDTP_CONNECT            23

/** When the the lower level AVDTP connection is disconnected, this event
 *  will be generated.
 *
 */
#define BTIF_A2DP_EVENT_AVDTP_DISCONNECT         24

/* Confirms  has received configuration information after an
 * btif_avdtp_get_config() request.
 */
#define BTIF_A2DP_AVDTP_EVENT_GET_CFG_CNF        27

/* Confirms  has sent configuration information after an
 * btif_avdtp_open_stream() request.
 */
#define BTIF_A2DP_AVDTP_EVENT_SET_CFG_CNF        28

#define BTIF_A2DP_EVENT_STREAM_STARTED_MOCK      29

#define BTIF_A2DP_REMOTE_NOT_SUPPORT             30

#define BTIF_A2DP_EVENT_STREAM_OPEN_MOCK         31

#define BTIF_A2DP_EVENT_CODEC_INFO_MOCK          32

//user define @biao
#define BTIF_A2DP_EVENT_AVDTP_CLOSE_IND          81
#define BTIF_A2DP_EVENT_AVDTP_DISCOVER_IND       82
#define BTIF_A2DP_EVENT_AVDTP_DISCOVER_RSP       83
//#define AVDTP_EVENT_STREAM_CLOSE_IND

/* event that let app level to see a stream is good to select */
#define BTIF_A2DP_EVENT_STREAM_SELECT            84
/* event that let app level to confirm stream(sep) state */
#define BTIF_A2DP_EVENT_STREAM_STATE_CFM         85


typedef uint16_t btif_avdtp_codec_sample_rate_t;

#define BT_A2DP_CODEC_TYPE_INVALID       0xBD

#define BT_A2DP_CODEC_TYPE_SBC           0x00

#define BT_A2DP_CODEC_TYPE_MPEG1_2_AUDIO 0x01

#define BT_A2DP_CODEC_TYPE_MPEG2_4_AAC   0x02

#define BT_A2DP_CODEC_TYPE_ATRAC         0x04

#define BT_A2DP_CODEC_TYPE_OPUS          0x08

#define BT_A2DP_CODEC_TYPE_H263          0x01

#define BT_A2DP_CODEC_TYPE_MPEG4_VSP     0x02

#define BT_A2DP_CODEC_TYPE_H263_PROF3    0x03

#define BT_A2DP_CODEC_TYPE_H263_PROF8    0x04

#define BT_A2DP_CODEC_TYPE_LHDC          0xFF

#define BT_A2DP_CODEC_TYPE_LDAC          0xFF

#define BT_A2DP_CODEC_TYPE_NON_A2DP      0xFF

#define BT_A2DP_CODEC_NONE_TYPE_INVALID  (0x00)

#define BT_A2DP_CODEC_NONE_TYPE_LHDC     (0x01)

#define BT_A2DP_CODEC_NONE_TYPE_LHDCV5   (0x02)

#define BT_A2DP_CODEC_NONE_TYPE_LDAC     (0x03)

#define BT_A2DP_CODEC_NONE_TYPE_SCALABLE (0x04)

#define BT_A2DP_CODEC_NONE_TYPE_LC3      (0x05)

#define BT_A2DP_CODEC_NONE_TYPE_L2HC     (0x06)

#define BT_A2DP_CODEC_NONE_TYPE_MIHC     (0x07)

#define BT_A2DP_MAX_CODEC_ELEM_SIZE      10

#define BT_A2DP_MAX_CP_VALUE_SIZE        10

typedef uint8_t btif_avctp_event_t;

#define BTIF_AVCTP_CONNECT_EVENT             1

#define BTIF_AVCTP_CONNECT_IND_EVENT         2

#define BTIF_AVCTP_DISCONNECT_EVENT          3

#define BTIF_AVCTP_COMMAND_EVENT             4

#define BTIF_AVCTP_RESPONSE_EVENT            5

#define BTIF_AVCTP_REJECT_EVENT              6

#define BTIF_AVCTP_TX_DONE_EVENT            8

#define BTIF_AVCTP_OPERANDS_EVENT            9

#define BTIF_AVCTP_CT_SDP_INFO_EVENT        10

#define BTIF_AVCTP_TG_SDP_INFO_EVENT        11

#define BTIF_AVCTP_CONNECT_EVENT_MOCK       12

#define BTIF_AVCTP_EVENT_PLAYBACK_STATUS_CHANGE_EVENT_SUPPORT 13

#define BTIF_AVCTP_EVENT_PLAYBACK_STATUS_CHANGED 14

#define BTIF_AVRCP_BROWSING_EVENT_CONNECT                       15

#define BTIF_AVRCP_BROWSING_EVENT_DISCONNECT                    16

#define BTIF_AVRCP_OBEX_EVENT_OPEN                              17

#define BTIF_AVRCP_OBEX_EVENT_CHANNEL_CONNECTED                 18

#define BTIF_AVRCP_OBEX_EVENT_DISCONNECT                        19

#define BTIF_AVRCP_OBEX_GET_RSP                                 20


#define BTIF_AVCTP_LAST_EVENT               30

#define BTIF_AVCTP_CTYPE_CONTROL               0x00
#define BTIF_AVCTP_CTYPE_STATUS                0x01
#define BTIF_AVCTP_CTYPE_SPECIFIC_INQUIRY      0x02
#define BTIF_AVCTP_CTYPE_NOTIFY                0x03
#define BTIF_AVCTP_CTYPE_GENERAL_INQUIRY       0x04
#define BTIF_AVCTP_CTYPE_BROWSING              0x80
#define BTIF_AVCTP_CTYPE_RESERVED              0x0A

enum {
    BTIF_AVRCP_STATE_DISCONNECTED,
    BTIF_AVRCP_STATE_CONNECTING,
    BTIF_AVRCP_STATE_CONNECTED
};

/** The transport layer is connected and commands/responses can now
 *  be exchanged.
 */
#define BTIF_AVRCP_EVENT_CONNECT             BTIF_AVCTP_CONNECT_EVENT

/** The application will receive this event when a lower layer connection
 *  (L2CAP) has been disconnected.  Both the target and controller of the
 *  connection are notified.
 *
 *  During this callback, the 'p.remDev' parameter is valid.
 */
#define BTIF_AVRCP_EVENT_DISCONNECT           BTIF_AVCTP_DISCONNECT_EVENT

/** A remote device is attempting to connect the transport layer.
 *  Only the acceptor of the connection is notified.
 */
#define BTIF_AVRCP_EVENT_CONNECT_IND          BTIF_AVCTP_CONNECT_IND_EVENT

/* Group: Events for the exchange of basic AV/C commands that are not routed
 * to the panel subunit.
 */
#define BTIF_AVRCP_EVENT_COMMAND             BTIF_AVCTP_COMMAND_EVENT

/** A AV/C response was received from the remote device (target). This event
 *  is received for responses not routed to the panel subunit.
 */
#define BTIF_AVRCP_EVENT_RESPONSE            BTIF_AVCTP_RESPONSE_EVENT

/** The remote device (target) rejected the AV/C command.  This event is
 *  received for responses not routed to the panel subunit.
 */
#define BTIF_AVRCP_EVENT_REJECT               BTIF_AVCTP_REJECT_EVENT

/** A command (see AVRCP_SendCommand) or response (see AVRCP_SendResponse)
 * has been sent.
 */
#define BTIF_AVRCP_EVENT_TX_DONE             BTIF_AVCTP_TX_DONE_EVENT

/** Additional operand data has been received for the previous
 *  AVRCP_EVENT_COMMAND or AVRCP_EVENT_RESPONSE.
 */
#define BTIF_AVRCP_EVENT_OPERANDS             BTIF_AVCTP_OPERANDS_EVENT

/** Avrcp sdp query result information
 */
#define BTIF_AVRCP_EVENT_CT_SDP_INFO             BTIF_AVCTP_CT_SDP_INFO_EVENT

/** Avrcp sdp query result information
 */
#define BTIF_AVRCP_EVENT_TG_SDP_INFO             BTIF_AVCTP_TG_SDP_INFO_EVENT

/** Avrcp connect mock event
 */
#define BTIF_AVRCP_EVENT_CONNECT_MOCK            BTIF_AVCTP_CONNECT_EVENT_MOCK

/** Avrcp playback status change support
 */
#define BTIF_AVRCP_EVENT_PLAYBACK_STATUS_CHANGE_EVENT_SUPPORT BTIF_AVCTP_EVENT_PLAYBACK_STATUS_CHANGE_EVENT_SUPPORT

/** Avrcp playback status changed
 */
#define BTIF_AVRCP_EVENT_PLAYBACK_STATUS_CHANGED BTIF_AVCTP_EVENT_PLAYBACK_STATUS_CHANGED

/** An AV/C command has timed out.
 */
#define BTIF_AVRCP_EVENT_CMD_TIMEOUT         ( BTIF_AVCTP_LAST_EVENT + 1)

/* Group: Events for the exchange of standard AV/C panel subunit commands.
 */

#define BTIF_AVRCP_EVENT_PANEL_PRESS        ( BTIF_AVCTP_LAST_EVENT + 2)

/** The key corresponding to a panel operation has been held down on the
 * remote controller device for at least AVRCP_PANEL_PRESSHOLD_TIME.
 */
#define BTIF_AVRCP_EVENT_PANEL_HOLD       ( BTIF_AVCTP_LAST_EVENT + 3)

/** The key corresponding to a panel operation has been released on the
 * remote controller device.
 */
#define BTIF_AVRCP_EVENT_PANEL_RELEASE      ( BTIF_AVCTP_LAST_EVENT + 4)

/** A panel response has been received from the remote target device.
 * All fields of "p.panelCnf" are valid.
 */
#define BTIF_AVRCP_EVENT_PANEL_CNF          ( BTIF_AVCTP_LAST_EVENT + 5)

/* Group: Events for the exchange of Bluetooth specific advanced AV/C
 * commands routed through the panel subunit.
 */

/** Advanced status information has been received from the controller.
 */
#define BTIF_AVRCP_EVENT_ADV_INFO          ( BTIF_AVCTP_LAST_EVENT + 6)

/** An advanced request was received and requires a response by the
 *  application.
 */
#define BTIF_AVRCP_EVENT_ADV_REQ            ( BTIF_AVCTP_LAST_EVENT + 7)

/** The notification of a registered event has been received.
 */
#define BTIF_AVRCP_EVENT_ADV_NOTIFY        ( BTIF_AVCTP_LAST_EVENT + 8)

/** An Advanced command was sent successfully to the target and a response
 *  was received.
 */
#define BTIF_AVRCP_EVENT_ADV_RESPONSE      ( BTIF_AVCTP_LAST_EVENT + 9)

/** No response was received for an Advanced command and it has timed out.
 *  This event is received for advanced commands routed to the panel subunit.
 */
#define BTIF_AVRCP_EVENT_ADV_CMD_TIMEOUT    ( BTIF_AVCTP_LAST_EVENT + 10)

/** An Advanced PDU was sent successfully.
 */
#define BTIF_AVRCP_EVENT_ADV_TX_DONE        ( BTIF_AVCTP_LAST_EVENT + 11)

/* Group: Connection events for establishing and releasing the AVRCP browsing
 * channel.  The browsing channel is used for the exchange of AVCTP-based
 * commands/responses which contain Bluetooth specific media
 * operations.
 */
#define BTIF_AVRCP_EVENT_BROWSE_CONNECT     ( BTIF_AVCTP_LAST_EVENT + 12)

/** The application will receive this event when a lower layer connection
 *  (L2CAP) has been disconnected.  Both the target and controller of the
 *  connection are notified.
 */
#define BTIF_AVRCP_EVENT_BROWSE_DISCONNECT  ( BTIF_AVCTP_LAST_EVENT + 13)

/**  remote device is attempting to connect the browsing channel.
 */
#define BTIF_AVRCP_EVENT_BROWSE_CONNECT_IND ( BTIF_AVCTP_LAST_EVENT + 14)

/* Group: Events for the exchange of browsing channel commands.
 */
#define BTIF_AVRCP_EVENT_BROWSE_REQ        ( BTIF_AVCTP_LAST_EVENT + 15)

#define BTIF_AVRCP_EVENT_BROWSE_INFO       ( BTIF_AVCTP_LAST_EVENT + 16)

#define BTIF_AVRCP_EVENT_BROWSE_RESPONSE    ( BTIF_AVCTP_LAST_EVENT + 17)

#define BTIF_AVRCP_EVENT_BROWSE_CMD_TIMEOUT ( BTIF_AVCTP_LAST_EVENT + 18)

#define BTIF_AVRCP_EVENT_BROWSE_TX_DONE     ( BTIF_AVCTP_LAST_EVENT + 19)

#define BTIF_AVRCP_EVENT_BROWSE_CMD         ( BTIF_AVCTP_LAST_EVENT + 20)

#define BTIF_AVRCP_EVENT_LAST               ( BTIF_AVCTP_LAST_EVENT + 20)

typedef uint8_t avrcp_event_Id_t;

#define BTIF_AVRCP_EID_MEDIA_STATUS_CHANGED        0x01 /* Change in media status */

#define BTIF_AVRCP_EID_TRACK_CHANGED               0x02 /* Current track changed */

#define BTIF_AVRCP_EID_TRACK_END                   0x03 /* Reached end of track */

#define BTIF_AVRCP_EID_TRACK_START                 0x04 /* Reached track start */

#define BTIF_AVRCP_EID_PLAY_POS_CHANGED            0x05 /* Change in playback position.
                                                         * Returned after the specified
                                                         * playback notification change
                                                         * notification interval.
                                                         */

#define BTIF_AVRCP_EID_BATT_STATUS_CHANGED         0x06 /* Change in battery status   */
#define BTIF_AVRCP_EID_SYS_STATUS_CHANGED          0x07 /* Change in system status    */
#define BTIF_AVRCP_EID_APP_SETTING_CHANGED         0x08 /* Change in player
                                                         * application setting
                                                         */

#define BTIF_AVRCP_EID_NOW_PLAYING_CONTENT_CHANGED 0x09 /* Contents of the now playing
                                                         * list have changed
                                                         */

#define BTIF_AVRCP_EID_AVAILABLE_PLAYERS_CHANGED   0x0A /* The available players have
                                                         * changed
                                                         */

#define BTIF_AVRCP_EID_ADDRESSED_PLAYER_CHANGED    0x0B /* The addressed player has
                                                         * changed
                                                         */

#define BTIF_AVRCP_EID_UIDS_CHANGED                0x0C /* The UIDS have changed */

#define BTIF_AVRCP_EID_VOLUME_CHANGED              0x0D /* The volume was changed */

#define BTIF_AVRCP_EID_FLAG_INTERIM                0x80 /* Used Internally */

typedef struct {
    U8 version;
    U8 padding;
    U8 marker;
    U8 payloadType;
    U16 sequenceNumber;
    U32 timestamp;
    U32 ssrc;
    U8 csrcCount;
    U32 csrcList[15];
} btif_avdtp_media_header_t;

typedef btif_avdtp_media_header_t btif_media_header_t;

typedef struct a2dp_stream_t { /* empty */ } a2dp_stream_t; /* used to hold the pointer to struct a2dp_control_t */
typedef struct a2dp_callback_parms_t { /* empty */ } a2dp_callback_parms_t; /* used to hold the pointer to btif_a2dp_callback_parms_t */

typedef U8 btif_avrcp_operation_t;

#define BTIF_AVRCP_OP_GET_CAPABILITIES               0x10

#define BTIF_AVRCP_OP_LIST_PLAYER_SETTING_ATTRIBS    0x11

#define BTIF_AVRCP_OP_LIST_PLAYER_SETTING_VALUES     0x12

#define BTIF_AVRCP_OP_GET_PLAYER_SETTING_VALUE       0x13

#define BTIF_AVRCP_OP_SET_PLAYER_SETTING_VALUE       0x14

#define BTIF_AVRCP_OP_GET_PLAYER_SETTING_ATTR_TEXT   0x15

#define BTIF_AVRCP_OP_GET_PLAYER_SETTING_VALUE_TEXT  0x16

#define BTIF_AVRCP_OP_INFORM_DISP_CHAR_SET           0x17

#define BTIF_AVRCP_OP_INFORM_BATT_STATUS             0x18

#define BTIF_AVRCP_OP_GET_MEDIA_INFO                 0x20

#define BTIF_AVRCP_OP_GET_PLAY_STATUS                0x30

#define BTIF_AVRCP_OP_REGISTER_NOTIFY                0x31

#define BTIF_AVRCP_OP_REQUEST_CONT_RESP              0x40

#define BTIF_AVRCP_OP_ABORT_CONT_RESP                0x41

#define BTIF_AVRCP_OP_SET_ABSOLUTE_VOLUME            0x50

#define BTIF_AVRCP_OP_SET_ADDRESSED_PLAYER           0x60

#define BTIF_AVRCP_OP_SET_BROWSED_PLAYER             0x70

#define BTIF_AVRCP_OP_GET_FOLDER_ITEMS               0x71

#define BTIF_AVRCP_OP_CHANGE_PATH                    0x72

#define BTIF_AVRCP_OP_GET_ITEM_ATTRIBUTES            0x73

#define BTIF_AVRCP_OP_PLAY_ITEM                      0x74

#define BTIF_AVRCP_OP_SEARCH                         0x80

#define BTIF_AVRCP_OP_ADD_TO_NOW_PLAYING             0x90

#define BTIF_AVRCP_OP_GENERAL_REJECT                 0xA0

#define BTIF_AVRCP_OP_CUSTOM_CMD                     0xF0

#define BTIF_AVRCP_OBEX_GET_IMG_CMD                  0xF1

#define BTIF_AVRCP_OBEX_GET_THM_CMD                  0xF2

#define BTIF_AVRCP_OBEX_GET_IMG_PROPTS               0xF3

enum avrcp_event_t {
//    AVRCP_TURN_ON = 1,
//    AVRCP_TURN_OFF,
    AVRCP_PLAY_REQ = 1, //241
    AVRCP_PAUSE_REQ,
    AVRCP_STOP_REQ,
    AVRCP_RECORD_REQ,
    AVRCP_FORWARD_REQ,
    AVRCP_BACKWARD_REQ,
    AVRCP_FAST_FORWARD_START_REQ,
    AVRCP_FAST_BACKWARD_START_REQ,
    AVRCP_FF_FB_STOP_REQ,
    AVRCP_CONN_REQ, //250
    AVRCP_DISCONN_REQ,

//   AV_C_EVNT_BEGIN,
    AVRCP_CHANNEL_OPENED,
    AVRCP_CHANNEL_NEW_OPEN,
    AVRCP_CHANNEL_CT_SDP_INFO,
    AVRCP_CHANNEL_TG_SDP_INFO,
    AVRCP_CHANNEL_CLOSED,
    AVRCP_CHANNEL_TX_HANDLED,
    AVRCP_CHANNEL_RESPONSE,
    AVRCP_CHANNEL_COMMAND,
    AVRCP_CHANNEL_DATA_IND,
    AVRCP_RESPONSE,            //no more than 15

    AVRCP_BROWSING_CHANNEL_OPENED,
    AVRCP_BROWSING_CHANNEL_CLOSED,
};

typedef uint8_t avrcp_media_status_t;

#define BTIF_AVRCP_MEDIA_STOPPED       0x00
#define BTIF_AVRCP_MEDIA_PLAYING       0x01
#define BTIF_AVRCP_MEDIA_PAUSED        0x02
#define BTIF_AVRCP_MEDIA_FWD_SEEK      0x03
#define BTIF_AVRCP_MEDIA_REV_SEEK      0x04
#define BTIF_AVRCP_MEDIA_ERROR         0xFF

/* for Codec Specific Information Element */
#if defined(A2DP_SCALABLE_ON) || defined(A2DP_LHDCV5_ON)
#define A2D_SCALABLE_IE_SAMP_FREQ_MSK    0xFF    /* b7-b0 sampling frequency */
#endif

#if defined(A2DP_LHDC_ON)
#define A2D_LHDC__IE_SAMP_FREQ_MSK    0xFF    /* b7-b0 sampling frequency */
#endif

#define A2D_STREAM_SAMP_FREQ_MSK    0xFFFFFFFF    /* b7-b4 sampling frequency */

#define A2D_SBC_IE_SAMP_FREQ_MSK    0xF0    /* b7-b4 sampling frequency */

// bit31-8: Reserved for Future Use
#define A2D_SBC_IE_SAMP_FREQ_24         (0x00000100)    /* b8:24  kHz */
#define A2D_SBC_IE_SAMP_FREQ_16         (0x00000080)    /* b7:16  kHz */
#define A2D_SBC_IE_SAMP_FREQ_32         (0x00000040)    /* b6:32  kHz */
#define A2D_SBC_IE_SAMP_FREQ_44         (0x00000020)    /* b5:44.1kHz */
#define A2D_SBC_IE_SAMP_FREQ_48         (0x00000010)    /* b4:48  kHz */
#define A2D_SBC_IE_SAMP_FREQ_96         (0x00000008)    /* b3:96  kHz */
#define A2D_SBC_IE_SAMP_FREQ_192        (0x00000004)    /* b2:192  kHz */
#define A2D_SBC_IE_SAMP_FREQ_88D2       (0x00000002)    /* b1:88.2  kHz */
#define A2D_SBC_IE_SAMP_FREQ_176D4      (0x00000001)    /* b0:176.4 kHz */

typedef struct {

    /* The most significant 32 bits of the track index information.  */
    uint32_t msU32;

    /* The least significant 32 bits of the track index information.  */
    uint32_t lsU32;
} avrcp_track_struct_t;

typedef uint8_t avrcp_battery_status_t;

#define BTIF_AVRCP_BATT_STATUS_NORMAL      0
#define BTIF_AVRCP_BATT_STATUS_WARNING     1
#define BTIF_AVRCP_BATT_STATUS_CRITICAL    2
#define BTIF_AVRCP_BATT_STATUS_EXTERNAL    3
#define BTIF_AVRCP_BATT_STATUS_FULL_CHARGE 4

typedef uint8_t avrcp_system_status_t;

#define BTIF_AVRCP_SYS_POWER_ON   0
#define BTIF_AVRCP_SYS_POWER_OFF  1
#define BTIF_AVRCP_SYS_UNPLUGGED  2

typedef uint16_t avrcp_player_attrId_mask_t;

typedef uint8_t avrcp_eq_value_t;

#define BTIF_AVRCP_EQ_OFF  1
#define BTIF_AVRCP_EQ_ON   2

typedef uint8_t avrcp_repeat_value_t;

#define BTIF_AVRCP_REPEAT_OFF     1
#define BTIF_AVRCP_REPEAT_SINGLE  2
#define BTIF_AVRCP_REPEAT_ALL     3
#define BTIF_AVRCP_REPEAT_GROUP   4

typedef uint8_t avrcp_shuffle_value_t;

#define BTIF_AVRCP_SHUFFLE_OFF    1
#define BTIF_AVRCP_SHUFFLE_ALL    2
#define BTIF_AVRCP_SHUFFLE_GROUP  3

typedef uint8_t avrcp_scan_value_t;

#define BTIF_AVRCP_SCAN_OFF    1
#define BTIF_AVRCP_SCAN_ALL    2
#define BTIF_AVRCP_SCAN_GROUP  3

typedef uint8_t avrcp_ext_value_t;
typedef uint8_t avrcp_player_attrId_t;

typedef struct {
    uint8_t valueId;
    uint16_t charSet;
    uint8_t length;
    const char *string;
} avrcp_player_settings_t;

typedef struct {
    U32 attrId;
    uint16_t charSet;
    uint16_t length;
    const char *string;
} avrcp_media_attr_t;


typedef struct {
    uint32_t length;
    uint32_t position;
    uint8_t mediaStatus;
} adv_play_status_t;

typedef struct {
    avrcp_player_attrId_t attrId;
    uint16_t charSet;
    uint8_t length;
    const char *string;
} avrcp_player_attrib_t;

typedef struct avrcp_adv_notify_parms_t {

    /* Defines the event ID that was received */
    avrcp_event_Id_t event;
    union {

        /* Play status of the media.  Valid when the event ID is
         * AVRCP_EID_MEDIA_STATUS_CHANGED.
         */
        avrcp_media_status_t mediaStatus;

        /* The current track.  Valid when the event ID is
         * AVRCP_EID_TRACK_CHANGED.
         */
        avrcp_track_struct_t track;

        /* The position (ms) of the current track.  Valid when the event
         * ID is AVRCP_EID_PLAY_POS_CHANGED.
         */
        uint32_t position;

        /* The battery status of the target.  Valid when the event ID is
         * AVRCP_EID_BATT_STATUS_CHANGED.
         */
        avrcp_battery_status_t battStatus;

        /* The system status of the target.  Valid when the event ID is
         * AVRCP_EID_SYS_STATUS_CHANGED.
         */
        avrcp_system_status_t sysStatus;

        /* The player settings.  Valid when the event ID is
         * AVRCP_EID_APP_SETTING_CHANGED
         */
        struct {

            /* Bitmask that describes which
             * attributes are being reported
             */
            avrcp_player_attrId_mask_t attrMask;

            /* The equalizer setting. */
            avrcp_eq_value_t eq;

            /* The repeat setting. */
            avrcp_repeat_value_t repeat;

            /* The shuffle setting. */
            avrcp_shuffle_value_t shuffle;

            /* The scan setting. */
            avrcp_scan_value_t scan;

#if BTIF_AVRCP_NUM_PLAYER_SETTINGS > 4
            /* Extended player setting. */
            avrcp_ext_value_t extValue[BTIF_AVRCP_NUM_PLAYER_SETTINGS - 4];

#endif                          /*  */
        } setting;

        /* Absolute volume.  Valid when the event ID is
         * AVRCP_EID_VOLUME_CHANGED.
         */
        uint8_t volume;

        /* The addressed player.  Valid when the event ID is
         * AVRCP_EID_ADDRESSED_PLAYER_CHANGED.
         */
        struct {
            uint16_t playerId;
            uint16_t uidCounter;
        } addrPlayer;

        /* The UID counter.  Valid when the event ID is
         * AVRCP_EID_UIDS_CHANGED.
         */
        uint16_t uidCounter;
    } p;
} avrcp_adv_notify_parms_t;

typedef uint16_t btif_avdtp_content_prot_type_t;
typedef uint8_t btif_avdtp_capability_type_t;
typedef uint8_t btif_avdtp_codec_type_t;

typedef struct btif_avdtp_codec_t {
    btif_avdtp_codec_type_t codecType;
    uint8_t elemLen;
    uint8_t *elements;
    uint8_t *pstreamflags; 
    bool discoverable;
}__attribute__((packed)) btif_avdtp_codec_t ;

typedef struct {
    bool free;
    uint8_t state;
    a2dp_stream_t *a2dp_stream; //stack A2dpStream  object
    btif_avdtp_codec_t *prev_conn_codec;
} btif_a2dp_stream_t;

typedef struct btif_avdtp_content_prot_t {
    btif_avdtp_content_prot_type_t cpType;
    uint8_t dataLen;
    uint8_t *data;
}__attribute__((packed))  btif_avdtp_content_prot_t;

typedef struct btif_avdtp_config_request_t {
    btif_avdtp_codec_t codec;
    btif_avdtp_content_prot_t cp;
    BOOL delayReporting;
} btif_avdtp_config_request_t;

#define BTIF_AVTP_HEADER_LEN 10

typedef struct {
    list_entry_t node;
    U8 transId;
    U8 msgType;
    U8 msgHdrLen;
    U8 msgHdr[BTIF_AVTP_HEADER_LEN];
    U8 txIdSize;
    U16 txId;
    U16 txDataLen;
    U8 *txData;
    U32 context;
} avtp_packet_t;

typedef struct avctp_cmd_frame_t {
    list_entry_t node;          /* Used internally by AVCTP. */
    uint8_t transId;            /* Transaction ID */
    uint8_t ctype;              /* 4 bits */
    uint8_t subunitType;        /* 5 bits */
    uint8_t subunitId;          /* 3 bits */
    uint8_t opcode;             /* 8 bits */
    uint8_t headerLen;          /* Header length */
    uint8_t header[6];          /* Header information */
    uint16_t operandLen;         /* Length of buffer in "operands" */
    uint8_t *operands;          /* Buffer containing the command data */
    BOOL more;                  /* Indicates whether to expect additional
                                 * frames containing more operand data. */
    avtp_packet_t avtpPacket;   /* For sending over AVTP */

    // EvmTimer    timer;          /* Timer for the command */
} avctp_cmd_frame_t;

struct avrcp_advanced_cmd_pdu;

typedef union {

    /* The capabilities of the target.
     * This is valid when "advOp" is set to AVRCP_OP_GET_CAPABILITIES.
     */
    struct {

        /* The type of capability. */
        uint8_t type;

        /* The capability info. */
        union {

            /* The list of company IDs.
             * (type == AVRCP_CAPABILITY_COMPANY_ID)
             */
            struct {

                /* The number of supported company IDs. */
                uint8_t numIds;

                /* An array of company IDs (3 bytes each). */
                uint8_t *ids;
            } companyId;

            /* A bitmask of the supported events.
             * (type == AVRCP_CAPABILITY_EVENTS_SUPPORTED)
             */
            uint16_t eventMask;
        } info;
    } capability;

    /* A bitmask that indicates the player settings supported by
     * the target device.
     * This is valid when "advOp" is set to
     * AVRCP_OP_LIST_PLAYER_SETTING_ATTRIBS.
     */
    uint16_t attrMask;

    /* The list of values for the requested attribute on the target
     * device.
     * This is valid when "advOp" is set to AVRCP_OP_LIST_PLAYER_SETTING_VALUES.
     */
    struct {

        /* The number of supported value IDs. */
        uint8_t numIds;

        /* An array of value ID (1 byte each). */
        uint8_t *ids;
    } attrValues;

    /* The text of each player setting attribute.
     * This is valid when "advOp" is set to
     * AVRCP_OP_GET_PLAYER_SETTING_ATTR_TEXT
     */
    struct {

        /* The number of attribute IDs returned. */
        uint8_t numIds;

        /* An array of attribute text information */
        avrcp_player_attrib_t txt[BTIF_AVRCP_NUM_PLAYER_SETTINGS];
    } attrStrings;

    /* The value of each player setting.
     * This is valid when "advOp" is set to AVRCP_OP_GET_PLAYER_SETTING_VALUE.
     */
    struct {

        /* Bitmask that describes which
         * attributes are being reported
         */
        avrcp_player_attrId_mask_t attrMask;

        /* The equalizer setting. */
        avrcp_eq_value_t eq;

        /* The repeat setting. */
        avrcp_repeat_value_t repeat;

        /* The shuffle setting. */
        avrcp_shuffle_value_t shuffle;

        /* The scan setting. */
        avrcp_scan_value_t scan;

#if BTIF_AVRCP_NUM_PLAYER_SETTINGS > 4
        /* Extended player setting. */
        avrcp_ext_value_t extValue[BTIF_AVRCP_NUM_PLAYER_SETTINGS - 4];

#endif                          /*  */
    } setting;

    /* The list of setting values for the requested attribute on the target.
     * This is valid when "advOp" is set to
     * AVRCP_OP_GET_PLAYER_SETTING_VALUE_TEXT.
     */
    struct {

        /* The number of settings returned */
        uint8_t numSettings;

        /* An array of value text information */
        avrcp_player_settings_t txt[BTIF_AVRCP_NUM_PLAYER_SETTINGS];
    } settingStrings;

    /* The list of element values for the current track on the
     * target.  This is valid when "advOp" is set to AVRCP_OP_GET_MEDIA_INFO.
     */
    struct {

        /* The number of elements returned */
        uint8_t numIds;

        /* An array of element value text information */
        avrcp_media_attr_t txt[BTIF_AVRCP_NUM_MEDIA_ATTRIBUTES];
    } element;

    adv_play_status_t playStatus;

    /* The Absolute Volume
     * This is valid when "advOp" is set to AVRCP_OP_SET_ABSOLUTE_VOLUME.
     */
    uint8_t volume;

    struct {
        uint8_t cmd_trans_id;
        /* The number of supported value IDs. */
        uint8_t buffer_len;
        /* An array of value ID (1 byte each). */
        uint8_t *buffer;
    } borwsing_info;
} avrcp_adv_rsp_parms_t;

typedef struct
{
    struct avrcp_advanced_cmd_pdu *tx_cmd;
    avrcp_adv_rsp_parms_t rsp;
    avrcp_adv_notify_parms_t notify;
} btif_avrcp_rsp_adv;

typedef btif_avctp_event_t btif_avrcp_event_t;
typedef struct btif_avrcp_channel_t btif_avrcp_channel_t;

typedef struct {
    /* Operation to which the remote target responded */
    uint16_t operation;

    /* The press state of the key in the command to which
     * the target responded.
     */
    BOOL press;

    /* Response from the target. May indicate
     * an "extended" response code such as
     * AVRCP_RESPONSE_SKIPPED or AVRCP_RESPONSE_TIMEOUT.
     */
    uint8_t response;
} avrcp_panel_cnf_t;
typedef struct {

    /* Operation corresponding to the key pressed, held, or
     * released. AVRCP will only indicate a new operation
     * when the previous one has been _RELEASE'd.
     */
    uint16_t operation;
} avrcp_panel_ind_t;

typedef struct
{
    btif_avrcp_event_t event;
    btif_avrcp_channel_t *channel;
    bt_status_t status;
    uint8_t error_code;
    btif_avrcp_operation_t adv_op;
    union
    {
        avctp_cmd_frame_t *cmdFrame;
        btif_avrcp_rsp_adv adv;
        avrcp_panel_cnf_t panel_cnf;
        avrcp_panel_ind_t panel_ind;
    } p;
} btif_avrcp_callback_parms_t;

struct avrcp_remote_sdp_info {
    uint16_t remote_avrcp_version;
    uint16_t remote_avctp_version;
    uint16_t remote_support_features;
};

typedef struct
{
    bt_bdaddr_t remote;
    struct avrcp_remote_sdp_info avrcp_sdp_info;
} __attribute__ ((packed)) btif_avrcp_sdp_info_t ;

typedef struct btif_a2dp_sbc_packet_t {
    list_entry_t node;
    uint16_t reserved_data_size;
    uint16_t reserved_header_size;
    uint8_t *data;
    uint16_t dataLen;
    uint16_t frameSize;
    uint8_t frameNum;  // save the number of frame when the encoder is LHDCV
    btif_bt_packet_t packet;
    uint16_t dataSent;
    uint16_t frmDataSent;
} btif_a2dp_sbc_packet_t;

typedef struct avrcp_adv_notify_parms_t avrcp_adv_notify_parms_t;
typedef struct avrcp_callback_parms_t avrcp_callback_parms_t;
typedef struct avctp_cmd_frame_t avctp_cmd_frame_t;
typedef struct btif_avrcp_channel_t btif_avrcp_channel_t;
typedef struct ibrt_a2dp_status_t ibrt_a2dp_status_t;
typedef struct btif_avdtp_content_prot_t btif_avdtp_content_prot_t;
typedef struct btif_avdtp_codec_t btif_avdtp_codec_t;
typedef struct btif_avdtp_capability_t btif_avdtp_capability_t;
typedef struct btif_avdtp_config_request_t btif_avdtp_config_request_t;
typedef struct btif_a2dp_sbc_packet_t btif_a2dp_sbc_packet_t;
typedef struct btif_avdtp_stream_info_t btif_avdtp_stream_info_t;
typedef void btif_av_device_t;
struct btif_get_codec_cap_t;

typedef struct btif_a2dp_callback_parms_t {
    uint8_t event;
    uint16_t len;
    int8_t status;
    uint8_t error;
    uint8_t discReason;
    bool a2dp_closed_due_to_sdp_fail;
    bool start_stream_already_sent;
    union {
        btif_avdtp_content_prot_t *cp;
        btif_avdtp_codec_t *codec;
        btif_avdtp_capability_t *capability;
        btif_avdtp_config_request_t *configReq; /* AVDTP Config request          */
        btif_bt_packet_t *btPacket; /* Raw Transmit packet           */
        btif_a2dp_sbc_packet_t *sbcPacket;  /* SBC Transmit packet           */
        uint8_t *data;          /* Receive data                  */
        uint16_t delayMs;       /* Buffering/processing delay of
                                 * the Sink.
                                 */
        btif_avdtp_stream_info_t *streamInfo;   /* Stream information */
        btif_av_device_t *device;   /* The connecting or disconnectin device. */
        struct btif_get_codec_cap_t *get_cap;

        a2dp_stream_t    *dstStream;  /* Stream to switch */
    } p;
    void   *remDev;
    bt_bdaddr_t remote;
} btif_a2dp_callback_parms_t;

typedef enum {
    BT_A2DP_STREAM_STATE_IDLE = 0,
    BT_A2DP_STREAM_STATE_CONFIGURED,
    BT_A2DP_STREAM_STATE_OPEN,
    BT_A2DP_STREAM_STATE_STREAMING,
    BT_A2DP_STREAM_STATE_CLOSING,
    BT_A2DP_STREAM_STATE_ABORTING,
} bt_a2dp_stream_state_t;

typedef struct bt_a2dp_state_t {
    bt_bdaddr_t remote;
    bt_a2dp_stream_state_t a2dp_stream_state;
    uint8_t a2dp_is_connected;
    uint8_t a2dp_is_streaming;
    uint8_t a2dp_channel_num;
    uint8_t a2dp_channel_mode;
    uint8_t a2dp_codec_type;
    uint8_t a2dp_sample_rate;
    uint8_t a2dp_sample_bit;
    uint8_t delay_report_enabled;
} bt_a2dp_state_t;

typedef struct bt_avrcp_state_t {
    uint8_t avrcp_is_connected;
    uint8_t avrcp_playback_status;
    uint8_t remote_support_playback_status_change_event;
    uint8_t avrcp_play_pause_flag;
    uint8_t curr_abs_volume;
} bt_avrcp_state_t;

typedef enum {
    BT_A2DP_DISC_ON_PROCESS = 1,
    BT_THIS_IS_CLOSED_BG_A2DP,
    BT_DISC_A2DP_PROFILE_ONLY,
    BT_A2DP_SET_CONN_FLAG,
    BT_A2DP_SET_STREAM_STATE,
    BT_A2DP_GET_CONN_FLAG,
    BT_A2DP_GET_STREAM_STATE,
    BT_A2DP_LAST_PAUSED_DEVICE,
} BT_A2DP_FIELD_ENUM_T;

#ifdef __cplusplus
}
#endif
#endif /* __A2DP_COMMON_DEFINE_H__ */
