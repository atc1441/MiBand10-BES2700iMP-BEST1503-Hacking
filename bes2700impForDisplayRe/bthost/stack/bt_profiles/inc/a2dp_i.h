/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifndef __A2DP_I_H__
#define __A2DP_I_H__
#include "bluetooth.h"
#include "btm_i.h"
#include "sdp_i.h"
#include "l2cap_i.h"
#ifdef BT_A2DP_SUPPORT
#if defined(__cplusplus)
extern "C" {
#endif

#define AVDTP_DISCOVER 0x01
#define AVDTP_GET_CAPABILITIES 0x02
#define AVDTP_SET_CONFIGURATION 0x03
#define AVDTP_GET_CONFIGURATION 0x04
#define AVDTP_RECONFIGURE 0x05
#define AVDTP_OPEN 0x06
#define AVDTP_START 0x07
#define AVDTP_CLOSE 0x08
#define AVDTP_SUSPEND 0x09
#define AVDTP_ABORT 0x0A
#define AVDTP_SECURITY_CONTROL 0x0B
#define AVDTP_GET_ALL_CAPABILITIES 0x0C
#define AVDTP_DELAYREPORT 0x0D
#define AVDTP_CUSTOM_CMD_VIVO 0x30

#define AVDTP_PKT_TYPE_SINGLE 0x00
#define AVDTP_PKT_TYPE_START 0x01
#define AVDTP_PKT_TYPE_CONTINUE 0x02
#define AVDTP_PKT_TYPE_END 0x03

#define AVDTP_MSG_TYPE_COMMAND 0x00
#define AVDTP_MSG_TYPE_GENERAL_REJECT 0x01
#define AVDTP_MSG_TYPE_ACCEPT 0x02
#define AVDTP_MSG_TYPE_REJECT 0x03

/* AVDTP error definitions */
#define AVDTP_BAD_HEADER_FORMAT 0x01
#define AVDTP_BAD_LENGTH 0x11
#define AVDTP_BAD_ACP_SEID 0x12
#define AVDTP_SEP_IN_USE 0x13
#define AVDTP_SEP_NOT_IN_USE 0x14
#define AVDTP_BAD_SERV_CATEGORY 0x17
#define AVDTP_BAD_PAYLOAD_FORMAT 0x18
#define AVDTP_NOT_SUPPORTED_COMMAND 0x19
#define AVDTP_INVALID_CAPABILITIES 0x1A
#define AVDTP_BAD_RECOVERY_TYPE 0x22
#define AVDTP_BAD_MEDIA_TRANSPORT_FORMAT 0x23
#define AVDTP_BAD_RECOVERY_FORMAT 0x25
#define AVDTP_BAD_REPORT_FORMAT 0x65
#define AVDTP_BAD_ROHC_FORMAT 0x26
#define AVDTP_BAD_CP_FORMAT 0x27
#define AVDTP_BAD_MULTIPLEXING_FORMAT 0x28
#define AVDTP_UNSUPPORTED_CONFIGURATION 0x29
#define AVDTP_BAD_STATE 0x31

#define AVDTP_INVALID_CODEC_TYPE                    0xC1
#define AVDTP_NOT_SUPPORTED_CODEC_TYPE              0xC2
#define AVDTP_INVALID_SAMPLING_FREQUENCY            0xC3
#define AVDTP_NOT_SUPPORTED_SAMPLING_FREQUENCY      0xC4
#define AVDTP_INVALID_CHANNEL_MODE                  0xC5
#define AVDTP_NOT_SUPPORTED_CHANNEL_MODE            0xC6
#define AVDTP_INVALID_SUBBANDS                      0xC7
#define AVDTP_NOT_SUPPORTED_SUBBANDS                0xC8
#define AVDTP_INVALID_ALLOCATION_METHOD             0xC9
#define AVDTP_NOT_SUPPORTED_ALLOCATION_METHOD       0xCA
#define AVDTP_INVALID_MINIMUM_BITPOOL_VALUE         0xCB
#define AVDTP_NOT_SUPPORTED_MINIMUM_BITPOOL_VALUE   0xCC
#define AVDTP_INVALID_MAXIMUM_BITPOOL_VALUE         0xCD
#define AVDTP_NOT_SUPPORTED_MAXIMUM_BITPOOL_VALUE   0xCE
#define AVDTP_INVALID_OBJECT_TYPE                   0xD6
#define AVDTP_NOT_SUPPORTED_OBJECT_TYPE             0xD7
#define AVDTP_INVALID_CHANNELS                      0xD8
#define AVDTP_NOT_SUPPORTED_CHANNELS                0xD9
#define AVDTP_INVALID_VERSION                       0xDA
#define AVDTP_NOT_SUPPORTED_VERSION                 0xDB
#define AVDTP_INVALID_BLOCK_LENGTH                  0xDD
#define AVDTP_INVALID_DRC                           0xE4
#define AVDTP_NOT_SUPPORTED_DRC                     0xE5
#define AVDTP_INVALID_CP_TYPE                       0xE0
#define AVDTP_INVALID_CP_FORMAT 					0xE1
#define AVDTP_NOT_SUPPORTED_CODEC_TYPE 				0xC2

#define AVDTP_CONN_TIMEOUT //(HZ * 30)
#define AVDTP_DISC_TIMEOUT //(HZ * 20)
#define AVDTP_AUTH_TIMEOUT //(HZ * 25)

#define AVDTP_DEFAULT_MTU 127
#define AVDTP_DEFAULT_CREDITS 7

#define AVDTP_MAX_L2CAP_MTU 1013
#define AVDTP_MAX_CREDITS 40

#define AVDTP_PPB_HEAD_RESERVE 8
#define AVDTP_PPB_TAIL_RESERVE 2
#define AVDTP_PPB_RESERVE (AVDTP_PPB_HEAD_RESERVE + AVDTP_PPB_TAIL_RESERVE)

#define AVDTP_CFC_DISABLED 0
#define AVDTP_CFC_ENABLED AVDTP_DEFAULT_CREDITS
#define AVDTP_CFG_SESSIONS_MAX 5 /* means how many l2cap channel */

/* SEP types definitions */
#define AVDTP_SEP_TYPE_SOURCE 0x00
#define AVDTP_SEP_TYPE_SINK 0x01

/* Media types definitions */
#define AVDTP_MEDIA_TYPE_AUDIO 0x00
#define AVDTP_MEDIA_TYPE_VIDEO 0x01
#define AVDTP_MEDIA_TYPE_MULTIMEDIA 0x02

/* SEP capability categories */
#define AVDTP_MEDIA_TRANSPORT_MASK    0x01
#define AVDTP_REPORTING_MASK          0x02
#define AVDTP_RECOVERY_MASK           0x04
#define AVDTP_CONTENT_PROTECTION_MASK 0x08
#define AVDTP_HEADER_COMPRESSION_MASK 0x10
#define AVDTP_MULTIPLEXING_MASK       0x20
#define AVDTP_MEDIA_CODEC_MASK        0x40
#define AVDTP_DELAY_REPORTING_MASK    0x80


/* SEP capability categories */
#define AVDTP_MEDIA_TRANSPORT 0x01
#define AVDTP_REPORTING 0x02
#define AVDTP_RECOVERY 0x03
#define AVDTP_CONTENT_PROTECTION 0x04
#define AVDTP_HEADER_COMPRESSION 0x05
#define AVDTP_MULTIPLEXING 0x06
#define AVDTP_MEDIA_CODEC 0x07
#define AVDTP_DELAY_REPORTING 0x08

#define AVDTP_CFG_SERVER_CHANNEL 0x07

/* AVDTP Content Protection Type */
#define AVDTP_CP_TYPE_DTCP      0x0001
#define AVDTP_CP_TYPE_SCMS_T    0x0002

#define A2DP_RECVDATA_BUF_SIZE 48

/* notify upper layer */
enum a2dp_event_t
{
    /* user command event*/
    AV_CONNECT_REQ = 1,
    AV_DISCONNECT_REQ,
    AV_TRANS_REQ,
    AV_SUSPEND_REQ,
    AV_START_REQ,
    AV_ABORT_REQ,
    AV_SET_CONFIG_REQ,
    AV_RECONF_REQ,

    /* internal event */
    //  AV_EVNT_BEGIN,
    AV_CONN_OPENED,
    AV_CONN_CLOSED,
    AV_CONN_CLOSED_DUE_TO_SDP_FAIL,

    AV_MEDIA_GET_CAP_IND,
    AV_MEDIA_SET_CFG_IND,
    AV_MEDIA_GET_CFG_CFM,
    AV_MEDIA_STREAM_CLOSE,
    AV_MEDIA_STREAM_ABORT,
    AV_MEDIA_RECONF_IND,
    AV_MEDIA_DISCOVERY_COMPLETE,

    AV_MEDIA_OPENED,
    AV_MEDIA_STREAM_START,
    AV_MEDIA_STREAM_DATA_IND,
    AV_MEDIA_STREAM_SUSPEND,
    AV_MEDIA_FREQ_CHANGED,
    AV_CONN_REQ_FAIL,
    AV_MEDIA_SECURITY_CONTROL_CMD,
    AV_MEDIA_SECURITY_CONTROL_CFM,
    AV_MEDIA_TX_HANDLED,

    AV_REMOTE_NOT_SUPPORT,
};

enum av_conn_state_enum
{
    AV_STOP,
    AV_STANDBY = 1, //ready
    AV_QUERING, //sdp quering
    AV_CONNECTING,  //initializing
    AV_OPEN,        //AV_SIG_ACTIVE,
    AV_CONNECTED,
    AV_STREAMING
};

#define AVDTP_CODEC_CAP_MAX_LENGTH (64)

#define AVDTP_VER_1_2 0x0102
#define AVDTP_VER_1_3 0x0103
#define AVDTP_LOCAL_VERSION AVDTP_VER_1_3

#define A2DP_VER_1_2 0x0102
#define A2DP_VER_1_3 0x0103
#define A2DP_LOCAL_VERSION A2DP_VER_1_3

#define A2DP_MEDIA_TYPE_AUDIO 	    0x00

#define A2DP_CODEC_SBC              0x00
#define A2DP_CODEC_MPEG12_AUDIO     0x01
#define A2DP_CODEC_MPEG24_AAC       0x02
#define A2DP_CODEC_ATRAC            0x03
#define A2DP_CODEC_VENDOR_SPECIFIC  0xff

#define A2DP_SAMPLING_FREQ_16000	(1 << 3)
#define A2DP_SAMPLING_FREQ_32000	(1 << 2)
#define A2DP_SAMPLING_FREQ_44100	(1 << 1)
#define A2DP_SAMPLING_FREQ_48000	1

#define A2DP_CHANNEL_MODE_MONO		(1 << 3)
#define A2DP_CHANNEL_MODE_DUAL_CHANNEL	(1 << 2)
#define A2DP_CHANNEL_MODE_STEREO	(1 << 1)
#define A2DP_CHANNEL_MODE_JOINT_STEREO	1

#define A2DP_BLOCK_LENGTH_4		(1 << 3)
#define A2DP_BLOCK_LENGTH_8		(1 << 2)
#define A2DP_BLOCK_LENGTH_12		(1 << 1)
#define A2DP_BLOCK_LENGTH_16		1

#define A2DP_SUBBANDS_4			(1 << 1)
#define A2DP_SUBBANDS_8			1

#define A2DP_ALLOCATION_SNR		(1 << 1)
#define A2DP_ALLOCATION_LOUDNESS	1

#define AVDTP_MEDIA_TRANSPORT_CAP_LEN               (2)
#define AVDTP_MEDIA_CODEC_CAP_HEADER_LEN            (4)
#define AVDTP_L2HC_MEDIA_CODEC_SPEC_INFO_ELE_LEN    (12)
#define AVDTP_MIHC_MEDIA_CODEC_SPEC_INFO_ELE_LEN    (13)

typedef struct
{
	U8 version; /* RTP Version */

	U8 padding; /* If the padding bit is set, the packet contains
                         * one or more additional padding octets at the end,
                         * which are not parts of the payload.  The last
                         * octet of the padding contains a count of how many
                         * padding octets should be ignored.
                         */

	U8 marker; /* Profile dependent.  Used to mark significant
                         * events such as frame boundaries in the packet
                         * stream.
                         */

	U8 payloadType; /* Profile dependent.  Identifies the RTP payload
                         * type.
                         */

	U16 sequenceNumber; /* Incremented by one for each packet sent */

	U32 timestamp; /* Time stamp of the sample */

	U32 ssrc; /* Synchronization source */

	U8 csrcCount; /* The number of CSRC (Contributing Source)
                         * identifiers that follow the fixed header.
                         */

	U32 csrcList[15]; /* List of CSRC identifiers */

} avdtp_media_header_t;

struct avdtp_header
{
	uint32 message_type : 2;
	uint32 packet_type : 2;
	uint32 transaction : 4;
	uint32 signal_id : 6;
	uint32 rfa0 : 2;
	//	uint32 unused:16;
} __attribute__((packed));

struct seid_req
{
	uint32 message_type : 2;
	uint32 packet_type : 2;
	uint32 transaction : 4;
	uint32 signal_id : 6;
	uint32 rfa0 : 2;
	uint32 rfa1 : 2;
	uint32 acp_seid : 6;
	uint8 param[0];
	//	uint32 unused:8;
} __attribute__((packed));

struct discover_resp
{
	//	struct avdtp_header header;
	//	struct seid_info *seps;
	uint32 message_type : 2;
	uint32 packet_type : 2;
	uint32 transaction : 4;
	uint32 signal_id : 6;
	uint32 rfa0 : 2;

	uint32 rfa1 : 1;
	uint32 inuse : 1;
	uint32 seid : 6;
	uint32 rfa2 : 3;
	uint32 type : 1;
	uint32 media_type : 4;
} __attribute__((packed));

struct discover_rej
{
	//	struct avdtp_header header;
	//	struct seid_info *seps;
	uint32 message_type : 2;
	uint32 packet_type : 2;
	uint32 transaction : 4;
	uint32 signal_id : 6;
	uint32 rfa0 : 2;
    uint8  error_code;
} __attribute__((packed));

struct security_control_req
{
	//avdtp hrader
	uint32 message_type : 2;
	uint32 packet_type : 2;
	uint32 transaction : 4;
	uint32 signal_id : 6;
	uint32 rfa0 : 2;
	uint32 rfa2 : 2;
	uint32 acp_seid : 6;
	//Content Protection Method Dependent Data
	uint8 *data;
} __attribute__((packed));

struct security_control_resp
{
	struct avdtp_header header;
	uint8 *data;
} __attribute__((packed));

struct getcap_resp
{
	struct avdtp_header header;
	uint8 *caps;
} __attribute__((packed));

struct getcap_req
{
    uint32 message_type : 2;
    uint32 packet_type : 2;
    uint32 transaction : 4;
    uint32 signal_id : 6;
    uint32 rfa0 : 2;
    uint32 rfa2 : 2;
    uint32 ACP_seid : 6;
} __attribute__((packed));


struct start_req
{
	struct avdtp_header header;

	uint8 first_seid;
	uint8 *other_seids;
} __attribute__((packed));

struct suspend_req
{
	struct avdtp_header header;

	uint8 first_seid;
	uint8 *other_seids;
} __attribute__((packed));

struct setconf_req
{
	//	struct avdtp_header header;
	uint32 message_type : 2;
	uint32 packet_type : 2;
	uint32 transaction : 4;
	uint32 signal_id : 6;
	uint32 rfa0 : 2;

	uint32 rfa2 : 2;
	uint32 acp_seid : 6;
	uint32 rfa1 : 2;
	uint32 int_seid : 6;

	uint8 *caps;
} __attribute__((packed));

struct reconf_req
{
	//	struct avdtp_header header;
	uint32 message_type : 2;
	uint32 packet_type : 2;
	uint32 transaction : 4;
	uint32 signal_id : 6;
	uint32 rfa0 : 2;

	uint32 rfa2 : 2;
	uint32 acp_seid : 6;

	uint8 *caps;
} __attribute__((packed));
struct general_rej
{
	uint32 message_type : 2;
	uint32 packet_type : 2;
	uint32 transaction : 4;
	//uint32 rfa0:8;
	uint32 signal_id : 6; //modified by owen.liu, for support version 1.3
	uint32 rfa0 : 2;
} __attribute__((packed));

struct seid_rej
{
	//	struct avdtp_header header;
	uint32 message_type : 2;
	uint32 packet_type : 2;
	uint32 transaction : 4;
	uint32 signal_id : 6;
	uint32 rfa0 : 2;

	uint32 error : 8;
	uint32 unused : 8;
} __attribute__((packed));

struct conf_rej
{
	struct avdtp_header header;
	uint8 category;
	uint8 error;
} __attribute__((packed));

struct stream_rej
{
	struct avdtp_header header;
	uint8 acp_seid;
	uint8 error;
} __attribute__((packed));

enum avdtp_session_state_enum
{
    AVDTP_SESSION_CLOSE,        /* l2cap connection closed, wait for openning, and then can send out sabm request */
    AVDTP_SESSION_CONNECTING,   /* l2cap channel is connecting */
    AVDTP_SESSION_CONNECTED,    /* l2cap channel created */
    AVDTP_SESSION_OPEN          /* avdtp session open, ready for start stream */
};

struct avdtp_session
{
    uint32 l2cap_handle;
    enum avdtp_session_state_enum state;
} __attribute__((packed));

enum avdtp_event_enum
{
    AVDTP_OPENED,
    AVDTP_ACCEPT_OPENED,
    AVDTP_MEDIA_OPENED,
    AVDTP_CLOSED,
    AVDTP_TX_HANDLED,
};

typedef enum
{
    AVDTP_STATE_IDLE,
    AVDTP_STATE_DISCOVER,
    AVDTP_STATE_GETCAP,
    AVDTP_STATE_SETCONFIG,
    AVDTP_STATE_CONFIGURED,
    AVDTP_STATE_OPEN,
    AVDTP_STATE_STREAMING,
    AVDTP_STATE_CLOSING,
    AVDTP_STATE_ABORTING
} avdtp_state_t;

struct seid_info
{
    uint32 rfa0 : 1;
    uint32 inuse : 1;
    uint32 seid : 6;
    uint32 rfa2 : 3;
    uint32 type : 1;
    uint32 media_type : 4;
    //  uint32 unused:16;
} __attribute__((packed));

struct avdtp_service_capability
{
    uint8 category;
    uint8 length;
    uint8 data[1];
} __attribute__((packed));

struct avdtp_media_codec_capability
{
    uint8 media_codec_type;
    uint8 len;
    uint8 *data;
} __attribute__((packed));


struct avdtp_media_content_protect_capability
{
    uint16 protect_type;
    uint8 len;
    uint8 *data;
} __attribute__((packed));


struct avdtp_media_delay_report_capability
{
    uint8 len;
} __attribute__((packed));

struct avdtp_service_cap_req
{
    struct avdtp_service_capability *cap;
    uint8  parse_error;
}__attribute__((packed));

struct avdtp_config_request
{
    struct avdtp_service_cap_req *trans_cap;
    struct avdtp_service_cap_req *codec_cap;
    struct avdtp_service_cap_req *cp_cap;
    struct avdtp_service_cap_req *dr_cap;
    struct avdtp_service_cap_req *rp_cap;
    struct avdtp_service_cap_req *mp_cap;
    struct avdtp_service_cap_req *hc_cap;
    struct avdtp_service_cap_req *rc_cap;

    struct avdtp_service_cap_req *unknown_cap;
};

struct avdtp_local_sep
{
    struct seid_info info;
    uint8 allocated;
    uint8 codec;
    uint8 sep_prio;
    bool sep_is_discoverable;
    struct avdtp_media_codec_capability *avdtp_codec_cap;
    struct avdtp_media_content_protect_capability *avdtp_content_protect_cap;
    uint8 delay_reporting_support;
    uint16 dr_delay_ms;
    struct avdtp_local_sep *next;
};

struct avdtp_remote_sep
{
    uint8 seid;
    uint8 type;
    uint8 transact_id;
    uint8 delay_reporting_support;
    uint16 avdtp_cp_type;
    struct avdtp_service_capability *codec;
    struct avdtp_remote_sep *next;
};

struct avdtp_control_t
{
    void *a2dp_stream;
    struct bdaddr_t remote;
    uint8 remote_seid;
    uint8 local_seid;
    uint8 same_sepid_workaround_for_redmi5;
    uint8 delay_reporting_enabled_on_the_stream;
    uint8 delay_resp_discover_req;
    uint8 discover_req_transaction;
    uint8 discover_cnt;
    uint8 avdtp_trans_id;

    bool is_source;
    bool initiator;
    avdtp_state_t state;

    struct avdtp_sep_ind *ind;
    struct avdtp_sep_cfm *cfm;

    void (*notify_callback)(uint8 device_id, struct avdtp_control_t* avdtp_ctl, uint8 event, uint32 l2cap_channel, void *pdata, uint8 reason);
    void (*datarecv_callback)(uint8 device_id, struct avdtp_control_t* avdtp_ctl, uint32 l2cap_channel, struct pp_buff *ppb);

    struct avdtp_session signal_session;
    struct avdtp_session media_session;
    struct avdtp_remote_sep* rsep_list;
    void * cur_op;

    uint32 pending_avdtp_signal_data_count;
};

struct avdtp_ctx_input {
    struct ctx_content ctx;
    struct bdaddr_t *remote;
    struct avdtp_control_t *avdtp_ctl;
    avdtp_state_t state;
    uint32 sig_handle;
    uint32 med_handle;
    void (*avdtp_notify_callback)(uint8 device_id, struct avdtp_control_t* avdtp_ctl, uint8 event, uint32 l2cap_channel, void *pdata, uint8 reason);
    void (*avdtp_datarecv_callback)(uint8 device_id, struct avdtp_control_t* avdtp_ctl, uint32 l2cap_handle, struct pp_buff *ppb);
};

struct avdtp_ctx_output {
    struct avdtp_control_t *avdtp_ctl;
    struct avdtp_local_sep *local_sep;
};

//callback functions for dealing with remote response to local command
struct avdtp_sep_cfm
{
    void (*setconf_rsp_cb)(struct avdtp_control_t* avdtp_ctl);
    void (*setconf_done_cb)(struct avdtp_control_t* avdtp_ctl);
    void (*getconf_rsp_cb)(struct avdtp_control_t* avdtp_ctl);
    void (*open_rsp_cb)(struct avdtp_control_t* avdtp_ctl);
    void (*start_rsp_cb)(struct avdtp_control_t* avdtp_ctl);
    void (*suspend_rsp_cb)(struct avdtp_control_t* avdtp_ctl);
    void (*close_rsp_cb)(struct avdtp_control_t* avdtp_ctl);
    void (*abort_rsp_cb)(struct avdtp_control_t* avdtp_ctl);
    void (*reconf_rsp_cb)(struct avdtp_control_t* avdtp_ctl);
    void (*security_control_rsp_cb)(struct avdtp_control_t* avdtp_ctl, struct avdtp_media_content_protect_capability* cp);
};
//callback functions to deal with remote command
struct avdtp_sep_ind
{
    uint8 *(*getcap_req_cb)(struct avdtp_control_t* avdtp_ctl, struct avdtp_local_sep *local_sep, uint16 *len, uint8 sig_type);
    int8 (*setconf_apply_cb)(struct avdtp_control_t* avdtp_ctl, struct avdtp_local_sep *local_sep, struct avdtp_config_request* cfg_req);
    int8 (*open_req_cb)(struct avdtp_control_t* avdtp_ctl, struct avdtp_local_sep *local_sep);
    int8 (*start_req_cb)(struct avdtp_control_t* avdtp_ctl, struct avdtp_local_sep *local_sep);
    int8 (*suspend_req_cb)(struct avdtp_control_t* avdtp_ctl, struct avdtp_local_sep *local_sep);
    int8 (*close_req_cb)(struct avdtp_control_t* avdtp_ctl, struct avdtp_local_sep *local_sep);
    int8 (*abort_req_cb)(struct avdtp_control_t* avdtp_ctl, struct avdtp_local_sep *local_sep);
    int8 (*reconf_req_cb)(struct avdtp_control_t* avdtp_ctl, struct avdtp_local_sep *local_sep, struct avdtp_config_request* cfg_req);
    int8 (*security_control_req_cb)(struct avdtp_control_t* avdtp_ctl, struct avdtp_local_sep *local_sep, struct avdtp_media_content_protect_capability* cp);
};

struct a2dp_sep {
    uint8 type;   //src or snk
    struct avdtp_local_sep *sep;
};

struct sbc_codec_cap {
    uint32 rfa0:4;
    uint32 media_type:4;
    uint32 media_codec_type:8;
    uint32 channel_mode:4;
    uint32 frequency:4;
    uint32 allocation_method:2;
    uint32 subbands:2;
    uint32 block_length:4;
    uint8 min_bitpool;
    uint8 max_bitpool;
} __attribute__ ((packed));

#define AAC_OCTECT0_MPEG2_AAC_LC     0x80
#define AAC_OCTECT1_SAMP_FREQ_44100  0x01
#define AAC_OCTECT2_SAMP_FREQ_48000  0x8
#define AAC_OCTECT2_CHAN_MODE_MONO   0x08
#define AAC_OCTECT2_CHAN_MODE_STEREO 0x04

struct aac_codec_cap {
    uint32 media_type:8;                //Audio            0x00
    uint32 media_codec_type:8;          //MEPG-2,4 AAC     0x02
    uint32 object_types_support:8;      //MPEG-2 AAC LC    0x80
    uint32 samp_freq_441:8;             //44100            0x01
    uint32 channel_mode:4;              //1(0x8) 2(0x4) 1 2(0xc)
    uint32 samp_freq_48k:4;             //48000            0x8-
    uint32 max_peak_bitrate_high:7;
    uint32 vbr_supported:1;
    uint8 max_peak_bitrate_low2;
    uint8 max_peak_bitrate_low1;
} __attribute__ ((packed));

#define LHDC_CODEC_SAMP_RATE_96000  0x01
#define LHDC_CODEC_SAMP_RATE_88200  0x02
#define LHDC_CODEC_SAMP_RATE_48000  0x04
#define LHDC_CODEC_SAMP_RATE_44100  0x08
#define LHDC_CODEC_BITS_PER_SAMP_24 0x10
#define LHDC_CODEC_BITS_PER_SAMP_16 0x20

#define LHDC_CODEC_VERSION_V3       0x01
#define LHDC_CODEC_VERSION_LOWER    0x00
#define LHDC_CODEC_VERSION_MASK     0x0f

#define LHDC_CODEC_MAX_SR_900       0x00
#define LHDC_CODEC_MAX_SR_500       0x10
#define LHDC_CODEC_MAX_SR_400       0x20
#define LHDC_CODEC_MAX_SR_MASK      0x30
#define LHDC_CODEC_LLC_EN           0x40
#define LHDC_CODEC_LLAC             0x80

#define LHDC_CODEC_COF_CSC_DISABLE  0x01
#define LHDC_CODEC_COF_CSC          0x02
#define LHDC_CODEC_COF_CSC_PRE      0x04
#define LHDC_CODEC_COF_CSC_RFU      0x08
#define LHDC_CODEC_COF_CSC_MASK     0x0f
#define LHDC_CODEC_MQA              0x10
#define LHDC_CODEC_MinBR            0x20
#define LHDC_CODEC_LARC             0x40
#define LHDC_CODEC_V4               0x80

#define LHDCV5_CODEC_SAMP_RATE_192000  0x01
#define LHDCV5_CODEC_SAMP_RATE_96000   0x04
#define LHDCV5_CODEC_SAMP_RATE_48000   0x10
#define LHDCV5_CODEC_SAMP_RATE_44100   0x20

#define LHDCV5_CODEC_BITS_PER_SAMP_32 0x01
#define LHDCV5_CODEC_BITS_PER_SAMP_24 0x02
#define LHDCV5_CODEC_BITS_PER_SAMP_16 0x04

#define LHDCV5_CODEC_MAX_SR_900       0x00
#define LHDCV5_CODEC_MAX_SR_500       0x10
#define LHDCV5_CODEC_MAX_SR_400       0x20
#define LHDCV5_CODEC_MAX_SR_MASK      0x30

#define LHDCV5_CODEC_MIN_SR_MASK      0xC0

#define LHDCV5_CODEC_AR_EN            0x01
#define LHDCV5_CODEC_JAS_EN           0x02
#define LHDCV5_CODEC_MATE_EN          0x04
#define LHDCV5_CODEC_LLC_EN           0x40
#define LHDCV5_CODEC_LOST_EN          0x80
struct lhdc_codec_cap {
    uint8 media_type; //Audio 0x00
    uint8 media_codec_type; //Vendor-Specific Codec 0xff
    uint8 vendor_id[4]; //3a 05 00 00
    uint8 codec_id[2];  //32 4c (Lower Ver.) or 33 4c (V3)
    uint8 sample_rate;
    uint8 max_sr_ver;
    uint8 cof_csc;
} __attribute__ ((packed));

struct lhdcv5_codec_cap {
    uint8 media_type; //Audio 0x00
    uint8 media_codec_type; //Vendor-Specific Codec 0xff
    uint8 vendor_id[4]; //3a 05 00 00
    uint8 codec_id[2];  //32 4c (Lower Ver.) or 33 4c (V3)
    uint8 sample_rate;
    uint8 max_bit_rate;
    uint8 version;
    uint8 ll_ar;
    uint8 resaved;
} __attribute__ ((packed));

#define LDAC_CODEC_SAMP_FREQ_96000 0x04
#define LDAC_CODEC_SAMP_FREQ_88200 0x08
#define LDAC_CODEC_SAMP_FREQ_48000 0x10
#define LDAC_CODEC_SAMP_FREQ_44100 0x20

#define LDAC_CODEC_CHAN_MODE_STEREO 0x01
#define LDAC_CODEC_CHAN_MODE_DUAL   0x02
#define LDAC_CODEC_CHAN_MODE_MONO   0x04

struct ldac_codec_cap {
    uint8 media_type; //Audio 0x00
    uint8 media_codec_type; //Vendor-Specific Codec 0xff
    uint8 vendor_id[4]; //2d 01 00 00
    uint8 codec_id[2];  //aa 00
    uint8 sample_freq;
    uint8 chan_mode;
} __attribute__ ((packed));

#define SCALABLE_CODEC_BITS_16 0x00
#define SCALABLE_CODEC_BITS_24 0x08
#define SCALABLE_CODEC_OLD_SAMP_48000 0x10 //old case
#define SCALABLE_CODEC_NEW_SAMP_48000 0x04 //new case
#define SCALABLE_CODEC_SAMP_44100 0x20
#define SCALABLE_CODEC_SAMP_32000 0x40
#define SCALABLE_CODEC_SAMP_96000 0x80

struct scalable_codec_cap {
    uint8 media_type; //Audio 0x00
    uint8 media_codec_type; //Vendor-Specific Codec 0xff
    uint8 vendor_id[4]; //75 00 00 00
    uint8 codec_id[2];  //03 01
    uint8 sample_rate;
} __attribute__ ((packed));

#define L2HC_CODEC_RES_32   (0x04)
#define L2HC_CODEC_RES_24   (0x02)
#define L2HC_CODEC_RES_16   (0x01)
#define L2HC_CODEC_SR_192000      (0x40)
#define L2HC_CODEC_SR_176400      (0x20)
#define L2HC_CODEC_SR_96000       (0x10)
#define L2HC_CODEC_SR_88200       (0x08)
#define L2HC_CODEC_SR_48000       (0x04)
#define L2HC_CODEC_SR_44100       (0x02)
#define L2HC_CODEC_SR_32000       (0x01)
#define L2HC_CODEC_BR_1920              (0x40)
#define L2HC_CODEC_BR_1600              (0x20)
#define L2HC_CODEC_BR_1280              (0x10)
#define L2HC_CODEC_BR_960               (0x08)
#define L2HC_CODEC_BR_640               (0x04)
#define L2HC_CODEC_BR_480               (0x02)
#define L2HC_CODEC_BR_320               (0x01)
#define L2HC_CODEC_BR_256               (0x80)
#define L2HC_CODEC_BR_192               (0x40)
#define L2HC_CODEC_BR_128               (0x20)
#define L2HC_CODEC_BR_96                (0x10)
#define L2HC_CODEC_BR_64                (0x08)
#define L2HC_CODEC_FD_10    (0x02)
#define L2HC_CODEC_FD_7D5   (0x01)
#define L2HC_CODEC_FD_5     (0x80)
#define L2HC_CODEC_CM_MONO          (0x04)
#define L2HC_CODEC_CM_DUAL          (0x08)

#define L2HC_CODEC_ALL_BIT_ZERO        (0x00)

struct l2hc_codec_cap {
    uint8 media_type; //Audio 0x00
    uint8 media_codec_type; //Vendor-Specific Codec 0xff
    uint8 vendor_id[4]; //CF; 0C 00 00
    uint8 codec_id[2];  //01 CA
    uint8 vers_res;
    uint8 sample_rate;
    uint8 br_msb;
    uint8 br_lsb_fd;
    uint8 fd_cm;
    uint8 reserved;
} __attribute__ ((packed));

// mihc codec version
#define MIHC_CODEC_VERSION_00       (0x00) // default version
#define MIHC_CODEC_VERSION_01       (0x01)
#define MIHC_CODEC_VERSION_02       (0x02)
#define MIHC_CODEC_VERSION_04       (0x04)
#define MIHC_CODEC_VERSION_08       (0x08)
#define MIHC_CODEC_VERSION_10       (0x10)
#define MIHC_CODEC_VERSION_20       (0x20)
#define MIHC_CODEC_VERSION_40       (0x40)
#define MIHC_CODEC_VERSION_80       (0x80)
// mihc codec min birate (note: version-dependent)
#define MIHC_CODEC_MIN_BR_1280      (0x80)
#define MIHC_CODEC_MIN_BR_960       (0x40)
#define MIHC_CODEC_MIN_BR_640       (0x20)
#define MIHC_CODEC_MIN_BR_320       (0x10)
#define MIHC_CODEC_MIN_BR_DEFAULT   (0x10) // default use minimum min-birate
// mihc codec sample rate
#define MIHC_CODEC_SR_48000         (0b00000100)
#define MIHC_CODEC_SR_44100         (0b00001000)
#define MIHC_CODEC_SR_32000         (0b00010000)
#define MIHC_CODEC_SR_24000         (0b00100000)
#define MIHC_CODEC_SR_16000         (0b01000000)
// mihc codec constant bit rate (note: Fixed to 1)
#define MIHC_CODEC_CBR_ENABLE       (0b1)
#define MIHC_CODEC_CBR_DISABLE      (0b0)
// mihc codec frame duration (note: alias LD in spec?)
#define MIHC_CODEC_FD_10            (0b01)
#define MIHC_CODEC_FD_5             (0b10)
// mihc codec bit deepth
#define MIHC_CODEC_BD_32            (0b001)
#define MIHC_CODEC_BD_24            (0b010)
#define MIHC_CODEC_BD_16            (0b100)
// mihc codec channel mode
#define MIHC_CODEC_CM_STEREO        (0b01)
#define MIHC_CODEC_CM_MONO          (0b10)
// mihc codec traffic mode
#define MIHC_CODEC_MODE_ADATPIVE    (0b0001)
#define MIHC_CODEC_MODE_LOW_LATENCY (0b0010)
#define MIHC_CODEC_MODE_LOSSLESS    (0b0100)
#define MIHC_CODEC_MODE_LOSSY       (0b1000)
// mihc codec MD
#define MIHC_CODEC_MD_ENABLE        (0b1)
#define MIHC_CODEC_MD_DISABLE       (0b0)

struct mihc_codec_cap {
    uint8 media_type;       // Audio 0x00
    uint8 media_codec_type; // Vendor-Specific Codec 0xff
    uint8 vendor_id[4];     // byte 0 ~ 3, value = 8F 03 00 00
    uint8 codec_id[2];      // byte 4 ~ 5, value = 31 4D
    uint8 version;
    uint8 sample_rate;
    uint8 cbr   : 1;        // byte 8
    uint8       : 1;
    uint8 fd    : 2;
    uint8       : 1;
    uint8 bd    : 3;
    uint8 cm    : 2;        // byte 9
    uint8       : 2;
    uint8 mode  : 4;
    uint8 min_bitrate;      // byte 10
    uint8       : 7;        // byte 11
    uint8 md    : 1;
    uint8 reserved;         // byte 12
} __attribute__ ((packed));

#define A2DP_OPEN_CTX_MAX_SIZE (SYS_MAX_A2DP_STREAMS+1)

struct a2dp_open_context {
    bt_bdaddr_t remote;
    bool inuse;
    btif_avdtp_codec_t *prev_conn_codec;
};

struct a2dp_control_t
{
    struct bdaddr_t remote;
    bool is_source;
    bool cp_enable;
    uint8 disc_reason;
    uint32 l2capSignalHandle;
    uint32 l2capMediaHandle;
    uint32 freq;
    uint16 avdtp_local_version;
    uint16 avdtp_remote_version;
    uint16 a2dp_remote_features;
    uint8 channel_mode;
    bool reconfig_codec_pending;
    bool reconfig_restore_streaming;
    uint8_t reconfig_codec_id;
    uint8_t reconfig_a2dp_non_type;

    enum av_conn_state_enum state;
    struct avdtp_control_t avdtp;

    struct avdtp_media_codec_capability *force_use_codec;
    struct avdtp_local_sep *lsep_list;
    struct avdtp_local_sep *selected_lsep;
    uint8 curr_lsep_prio;

    void *avdtp_codec_req; //finally setconf/reconf-ed codec capabilities
    void *avdtp_cp_req; //finally setconf/reconf-ed cp capabilities
    struct avdtp_media_content_protect_capability *security_cp_cap;

    uint8_t (*indicate)(uint8 device_id, struct a2dp_control_t *stream, uint8 event, void *pData);
    void (*data_cb)(uint8 device_id, struct a2dp_control_t *stream,struct pp_buff *ppb);

    avdtp_media_header_t media_header;
};

struct a2dp_ctx_input {
    struct ctx_content ctx;
    struct bdaddr_t *remote;
    struct a2dp_control_t *a2dp_ctl;
    struct avdtp_control_t *avdtp_ctl;
    struct avdtp_local_sep *local_sep;
    uint32 l2capSignalHandle;
    uint32 l2capMediaHandle;
    uint8_t (*indicate_callback)(uint8 device_id, struct a2dp_control_t *stream, uint8 event, void *pData);
    void (*data_callback)(uint8 device_id, struct a2dp_control_t *stream, struct pp_buff *ppb);
};

struct a2dp_ctx_output {
};

extern struct a2dp_control_t *a2dp_ctl_list;

int8 avdtp_send(uint32 avdtp_handle, uint8 *data, uint32 datalen);
int8 avdtp_init(struct avdtp_control_t* (*search)(uint8_t device_id, bool is_channel_req_context));
void avdtp_stream_init(struct avdtp_control_t* avdtp_ctl,
                void (*avdtp_notify_callback)(uint8 device_id, struct avdtp_control_t* avdtp_ctl, uint8 event, uint32 l2cap_channel, void *pdata, uint8 reason),
                void (*avdtp_datarecv_callback)(uint8 device_id, struct avdtp_control_t* avdtp_ctl, uint32 l2cap_handle, struct pp_buff *ppb),
                struct avdtp_sep_ind *ind, struct avdtp_sep_cfm *cfm);
uint32 avdtp_open_i(struct avdtp_control_t *avdtp_ctl, struct bdaddr_t *remote);
int8 avdtp_close_i(struct avdtp_control_t* avdtp_ctl);
int8 avdtp_send_delay_report(struct avdtp_control_t * avdtp_ctl, uint16 delay_ms);
int8 avdtp_send_security_control_req(struct avdtp_control_t *stream, uint8_t *data, uint16_t len);
int8 avdtp_send_security_control_resp(struct avdtp_control_t *stream,
                                      uint8_t *data, uint16_t len, uint8 error);
int8 avdtp_discover(struct avdtp_control_t *avdtp_ctl);
void avdtp_discovery_complete(struct avdtp_control_t* avdtp_ctl, struct avdtp_remote_sep *rsep_list);
int8 avdtp_set_configuration(struct avdtp_control_t *avdtp_ctl, struct avdtp_remote_sep *rsep, struct avdtp_local_sep *lsep, uint8 *conf_data, uint16 caps_len);
int8 avdtp_send_command_with_param(struct avdtp_control_t *avdtp_ctl, uint32 signal_id, uint8 *params, uint32 param_len);

#define avdtp_send_command_len3(c,s) \
    avdtp_send_command_with_param(c,s,NULL,0)

#define avdtp_open(avdtp_ctl) \
    avdtp_send_command_len3(avdtp_ctl, AVDTP_OPEN)

#define avdtp_start(avdtp_ctl) \
    avdtp_send_command_len3(avdtp_ctl, AVDTP_START)

#define avdtp_close(avdtp_ctl) \
    avdtp_send_command_len3(avdtp_ctl, AVDTP_CLOSE)

#define avdtp_suspend(avdtp_ctl) \
    avdtp_send_command_len3((avdtp_ctl), AVDTP_SUSPEND)

#define avdtp_abort(avdtp_ctl) \
    avdtp_send_command_len3((avdtp_ctl), AVDTP_ABORT)

void avdtp_accept_stream_request_command(struct bdaddr_t* remote, uint8_t transaction, uint8_t signal_id);
uint32 avdtp_get_signal_l2cap_handle(struct avdtp_control_t *avdtp_ctl);
uint32 avdtp_get_media_l2cap_handle(struct avdtp_control_t *avdtp_ctl);
uint32 avdtp_save_ctx(struct avdtp_control_t *avdtp_ctl, uint8_t *buf, uint32_t buf_len);
uint32 avdtp_restore_ctx(struct avdtp_ctx_input *input);
void avdtp_set_synced_streaming_state(struct bdaddr_t *addr);
uint8 avdtp_generate_trans_id_per_channel(struct avdtp_control_t *avdtp_ctl);

void pts_send_discover_cmd(void);
void pts_send_get_capability_cmd(void);
void pts_send_set_configuration_cmd(void);
void pts_send_get_configuration_cmd(void);
void pts_send_reconfigure_cmd(void);
void pts_send_open_cmd(void);
void pts_send_close_cmd(void);
void pts_send_abort_cmd(void);
void pts_send_get_all_capability_cmd(void);
void pts_send_suspend_cmd(void);
void pts_send_start_cmd(void);
void pts_send_security_control_cmd(void);

uint8_t a2dp_is_critical_avdtp_cmd_handling(void);
void a2dp_critical_avdtp_cmd_timeout(void);

const char *avdtp_state2str(avdtp_state_t state);
const char *avdtp_event2str(enum avdtp_event_enum event);
U16 a2dp_MediaPacketSize(struct a2dp_control_t *Stream);
#define a2dp_MediaPacketSize(s) (l2cap_get_tx_mtu((s)->l2capMediaHandle))
void a2dp_register_is_pts_address_callback(bool (*cb)(void *remote));
U16 a2dp_create_media_header(struct a2dp_control_t *a2dp_ctl, U8 *buffer);
void a2dp_notifyCallback(uint8 device_id, struct avdtp_control_t* avdtp_ctl, uint8 event, uint32 l2cap_channel, void *pData, uint8 reason);
void a2dp_dataRecvCallback(uint8 device_id, struct avdtp_control_t* avdtp_ctl, uint32 l2cap_handle, struct pp_buff *ppb);
int8 a2dp_close(struct a2dp_control_t * stream);
struct a2dp_open_context* a2dp_set_open_context(bt_bdaddr_t* remote, btif_avdtp_codec_t *prev_codec);
struct a2dp_open_context* a2dp_get_open_context(bt_bdaddr_t* remote);
void a2dp_stack_init(struct avdtp_control_t* (*search)(uint8_t device_id, bool is_channel_req_context));
void a2dp_stream_init(struct a2dp_control_t *a2dp_ctl,
            uint8_t (*indicate_callback)(uint8 device_id, struct a2dp_control_t *stream, uint8 event, void *pData),
            void (*data_callback)(uint8 device_id, struct a2dp_control_t *stream, struct pp_buff *ppb));

int a2dp_register_sep(
            struct a2dp_control_t *stream,
            uint8 sep_type,
            struct avdtp_media_codec_capability *sep_code,
            struct avdtp_media_content_protect_capability *sep_cp,
            uint8 sep_prioprity);

int a2dp_unregister_sep(struct a2dp_control_t *ctl, uint8 codec_type);
enum av_conn_state_enum av_getState(struct  a2dp_control_t*stream) ;
void av_setState( struct  a2dp_control_t *stream, enum av_conn_state_enum state);
int8 av_close(struct a2dp_control_t * stream);
int8 av_connectReq(struct a2dp_control_t *stream, struct bdaddr_t *peer);
int8 av_disconnectReq(struct  a2dp_control_t *stream);
const char *a2dp_state2str(enum av_conn_state_enum state);
int8 av_suspendReq(struct a2dp_control_t *stream);
int8 av_startReq(  struct  a2dp_control_t  *stream);
int8 av_abortReq( struct  a2dp_control_t  *stream);
int8 av_setConfReq( struct  a2dp_control_t  *stream);
int8 av_closeReq(struct  a2dp_control_t *stream );
int8 av_reconfig_codec(struct a2dp_control_t *stream, uint8_t codec_id);
int8 av_reconfig_to_vendor_codec(struct a2dp_control_t *stream, uint8_t codec_id, uint8_t a2dp_non_type);
int8 av_setSinkDelay(struct a2dp_control_t *stream, uint16 delay_ms);
int8 av_security_control_req(struct a2dp_control_t *stream, uint8_t *data, uint16_t len);
int8 av_security_control_resp(struct a2dp_control_t *stream, uint8_t *data, uint16_t len, uint8 error);
struct avdtp_control_t *a2dp_get_avdtp_control(struct a2dp_control_t *stream);
uint32 a2dp_save_ctx(struct a2dp_control_t *a2dp_ctl, uint8_t *buf, uint32_t buf_len);
uint32 a2dp_restore_ctx(struct a2dp_ctx_input *input);

#if defined(__cplusplus)
}
#endif
#endif /* BT_A2DP_SUPPORT */
#endif /* __A2DP_I_H__ */
