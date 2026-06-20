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
#ifndef __L2CAP_I_H__
#define __L2CAP_I_H__
#include "bluetooth.h"
#include "btm_i.h"
#if defined(__cplusplus)
extern "C" {
#endif

#define L2CAP_PPB_HEAD_RESERVE 4 /*len+cid+control+sdulen  2+2+2+2*/
#define L2CAP_PPB_TAIL_RESERVE 0 /* fcs 2 */
#define L2CAP_PPB_RESERVE (L2CAP_PPB_HEAD_RESERVE + L2CAP_PPB_TAIL_RESERVE)
#define L2CAP_HCI_HEADER_LEN (HCI_DATA_TYPE_LEN + HCI_ACL_HEADER_LEN)
#define L2CAP_SDU_LEN_FIELD_SIZE 2

#define L2CAP_SIG_RESULT_SUCCESS 0x0
#define L2CAP_SIG_RESULT_PENDING 0x1
#define L2CAP_SIG_RESULT_REFUSE_PSM 0x2
#define L2CAP_SIG_RESULT_REFUSE_SECURITY 0x3
#define L2CAP_SIG_RESULT_REFUSE_RESOURCE 0x4
#define L2CAP_SIG_RESULT_INVALID_SOURCE_ID 0x6
#define L2CAP_SIG_RESULT_SOURCE_ID_ALREADY_ALL0CATED 0x7

// Local specified defines
#define PSM_BESAUD              0x0033
#define PSM_EXTRA_BESAUD        0x0034
#define PSM_LOCAL_BT_ATT        0x00BA
#define PSM_LOCAL_LE_SMP        0x00BB
#define PSM_LOCAL_BT_SMP        0x00BC
#define PSM_AVDTP_SIGNAL        PSM_AVDTP
#define PSM_AVDTP_MEDIA_FAKE    0xffff
#define PSM_OUTGOING_ATT        0xaa00
#define PSM_INCOMING_ATT        0xab00
#define PSM_OUTGOING_EATT       0xbe00
#define PSM_INCOMING_EATT       0xbf00
#define PSM_OUTGOING_SDP_BASE   0xcc00
#define PSM_INCOMING_SDP_BASE   0xdd00
#define PSM_MAP_ACCESS_BASE     0xee00
#define PSM_DYNAMIC_PBAP        0x1011
#define PSM_DYNAMIC_MAP_ACCESS  0x1013
#define PSM_DYNAMIC_MAP_NOTIFY  0x1015
#define PSM_DYNAMIC_OPP         0x1003
#define PSM_DYNAMIC_AVRCP_OBEX  0x1017

#define L2CAP_SCID_PREFIX_MASK              0xff00
#define L2CAP_SCID_INDEX_MASK               0x00ff

#define L2CAP_SCID_PREFIX_OUTGOING_SDP      0x1000
#define L2CAP_SCID_PREFIX_INCOMING_SDP      0x2000
#define L2CAP_SCID_PREFIX_RFCOMM            0x3000
#define L2CAP_SCID_PREFIX_BNEP              0x4000
#define L2CAP_SCID_PREFIX_HID_CTRL          0x5100
#define L2CAP_SCID_PREFIX_HID_INTR          0x5200
#define L2CAP_SCID_PREFIX_UPNP              0x5300
#define L2CAP_SCID_PREFIX_OUTGOING_ATT      0x6a00
#define L2CAP_SCID_PREFIX_INCOMING_ATT      0x6b00
#define L2CAP_SCID_PREFIX_OUTGOING_EATT     0x6e00
#define L2CAP_SCID_PREFIX_INCOMING_EATT     0x6f00
#define L2CAP_SCID_PREFIX_AVCTP             0x7000
#define L2CAP_SCID_PREFIX_AVCTP_BROWSING    0x8000
#define L2CAP_SCID_PREFIX_AVDTP_SINGAL      0x9000
#define L2CAP_SCID_PREFIX_AVDTP_MEDIA       0xa000
#define L2CAP_SCID_PREFIX_PBAP              0xb100
#define L2CAP_SCID_PREFIX_MAP_ACCESS        0xb200
#define L2CAP_SCID_PREFIX_MAP_NOTIFY        0xb300
#define L2CAP_SCID_PREFIX_IDENTIFIER        0xc000
#define L2CAP_SCID_PREFIX_OPP               0xd000

#define L2CAP_MIN_DYNAMIC_CID               (0x40)
#define l2cap_cid_to_index(cid)        (cid - L2CAP_MIN_DYNAMIC_CID)
#define l2cap_index_to_cid(index)      (index + L2CAP_MIN_DYNAMIC_CID)

struct l2cap_hdr {
    uint16 len;
    uint16 cid;
}__attribute__ ((packed));

#define L2CAP_SIG_REJECT_RSP                0x01    // acl-u le-u both allowed
#define L2CAP_SIG_CONN_REQ                  0x02    // only acl-u
#define L2CAP_SIG_CONN_RSP                  0x03    // only acl-u
#define L2CAP_SIG_CFG_REQ                   0x04    // only acl-u
#define L2CAP_SIG_CFG_RSP                   0x05    // only acl-u
#define L2CAP_SIG_DISCONN_REQ               0x06    // acl-u le-u both allowed
#define L2CAP_SIG_DISCONN_RSP               0x07    // acl-u le-u both allowed
#define L2CAP_SIG_ECHO_REQ                  0x08    // only acl-u
#define L2CAP_SIG_ECHO_RSP                  0x09    // only acl-u
#define L2CAP_SIG_INFO_REQ                  0x0A    // only acl-u
#define L2CAP_SIG_INFO_RSP                  0x0B    // only acl-u
#define L2CAP_SIG_CONN_PRAM_UPDATE_REQ      0x12    // only le-u
#define L2CAP_SIG_CONN_PRAM_UPDATE_RSP      0x13    // only le-u
#define L2CAP_SIG_LE_CREDIT_BASED_CONN_REQ  0x14    // only le-u
#define L2CAP_SIG_LE_CREDIT_BASED_CONN_RSP  0x15    // only le-u
#define L2CAP_SIG_FLOW_CONTROL_CREDIT_IND   0x16    // acl-u le-u both allowed
#define L2CAP_SIG_CREDIT_BASED_CONN_REQ     0x17    // acl-u le-u both allowed
#define L2CAP_SIG_CREDIT_BASED_CONN_RSP     0x18    // acl-u le-u both allowed
#define L2CAP_SIG_CREDIT_BASED_RECONF_REQ   0x19    // acl-u le-u both allowed
#define L2CAP_SIG_CREDIT_BASED_RECONF_RSP   0x1A    // acl-u le-u both allowed

struct l2cap_sig_hdr {
    byte code;
    byte id;
    uint16 len;
}__attribute__ ((packed));


#define L2CAP_SIG_REASON_NOT_UNDERSTOOD 0x0
#define L2CAP_SIG_REASON_MTU_EXCEED 0x1
#define L2CAP_SIG_REASON_INVALID_CID 0x2
struct l2cap_sig_rej {

    uint16 reason;
/*data*/
    uint16 scid;            /*the data len is 0 - 4*/
    uint16 dcid;
}__attribute__ ((packed));

struct l2cap_sig_conn_req {
    uint16 psm;
    uint16 scid;
}__attribute__ ((packed));

#define L2CAP_SIG_RESULT_SUCCESS 0x0
#define L2CAP_SIG_RESULT_PENDING 0x1
#define L2CAP_SIG_RESULT_REFUSE_PSM 0x2
#define L2CAP_SIG_RESULT_REFUSE_SECURITY 0x3
#define L2CAP_SIG_RESULT_REFUSE_RESOURCE 0x4
#define L2CAP_SIG_RESULT_INVALID_SOURCE_ID 0x6
#define L2CAP_SIG_RESULT_SOURCE_ID_ALREADY_ALL0CATED 0x7

#define L2CAP_SIG_RESULT_PENDING_NOINFO 0x00
#define L2CAP_SIG_RESULT_PENDING_AUTHEN 0x01
#define L2CAP_SIG_RESULT_PENDING_AUTHOR 0x02
struct l2cap_sig_conn_rsp {
    uint16 dcid;
    uint16 scid;

    uint16 result;
    uint16 status;   /*only defined when result = pending */
}__attribute__ ((packed));

struct l2cap_sig_cfg_req {
    uint16 dcid;
    uint16 flags;           /* bit0=1:continue  bit0=0:complete  */
}__attribute__ ((packed));

#define L2CAP_CFGRSP_SUCCESS                        0x0000
#define L2CAP_CFGRSP_UNACCEPT_PARAMS       0x0001
#define L2CAP_CFGRSP_REJ                                 0x0002
#define L2CAP_CFGRSP_UNKNOWN                       0x0003
struct l2cap_sig_cfg_rsp {
    uint16 scid;
    uint16 flags;
    uint16 result;
}__attribute__ ((packed));

#define L2CAP_CFG_TYPE_MTU          0x01
#define L2CAP_CFG_TYPEF_FLUSH_TO    0x02
#define L2CAP_CFG_TYPE_QOS          0x03
#define L2CAP_CFG_TYPE_RFC          0x04    //retransmission and flow control
#define L2CAP_CFG_TYPE_FCS          0x05
#define L2CAP_CFG_TYPE_EFS          0x05    //extended flow specification
#define L2CAP_CFG_TYPE_EWS          0x06    //extended window size
struct l2cap_sig_cfg_opt_hdr{
    byte type;
    byte len;
}__attribute__ ((packed));

struct l2cap_sig_cfg_opt_mtu {

    uint16 mtu;
}__attribute__ ((packed));
struct l2cap_sig_cfg_opt_flushto {

    uint16 flushto;
}__attribute__ ((packed));

#define L2CAP_QOS_NO_TRAFFIC		0x00
#define L2CAP_QOS_BEST_EFFORT		0x01
#define L2CAP_QOS_GUARANTEED		0x02
struct l2cap_sig_cfg_opt_qos {

    byte	 flags;
    byte     service_type;
    uint32  token_rate;
    uint32  token_size;
    uint32  bandwidth;
    uint32  latency;
    uint32  delay_variation;
}__attribute__ ((packed));

#define L2CAP_MODE_BASE                         0
#define L2CAP_MODE_RETRANSMISSION               1
#define L2CAP_MODE_FLOWCONTROL                  2
#define L2CAP_MODE_ENHANCED_RETRANSMISSION      3
#define L2CAP_MODE_STREAMING                    4
#define L2CAP_MODE_LE_CREDIT_FLOWCONTROL        5
#define L2CAP_MODE_ENHANCED_CREDIT_FLOWCONTROL  6

struct l2cap_sig_cfg_opt_rfc {

    byte     mode;
    byte      txwindow;
    byte      maxtransmit;
    uint16   retransmission_timeout;
    uint16   monitor_timeout;
    uint16   mps;
}__attribute__ ((packed));

#define L2CAP_FCS_TYPE_NONE 0x00
#define L2CAP_FCS_TYPE_16_BIT   0x01
struct l2cap_sig_cfg_opt_fcs {
    byte type;
}__attribute__ ((packed));

struct l2cap_sig_disconn_req {

    uint16 dcid;
    uint16 scid;
}__attribute__ ((packed));

struct l2cap_sig_disconn_rsp {
    uint16 dcid;
    uint16 scid;
}__attribute__ ((packed));

#define L2CAP_INFOTYPE_CONNLESS_MTU     0x01    // 2-octet
#define L2CAP_INFOTYPE_EXTENED_FEATURE  0x02    // 4-octet
#define L2CAP_INFOTYPE_FIXED_CHANNELS   0x03    // 8-octet

#define L2CAP_FIXCH_SUPPORT_SIGNALING_CHANNEL       0x02
#define L2CAP_FIXCH_SUPPORT_CONNLESS_RECEPTION      0x04
#define L2CAP_FIXCH_SUPPORT_BREDR_SECURITY_MANAGER  0x80

struct l2cap_sig_info_req {
    uint16 infotype;
}__attribute__ ((packed));

#define L2CAP_INFOTYPE_SUCCESS          0x00
#define L2CAP_INFOTYPE_NOT_SUPPORT      0x01

#define L2CAP_EXTFEATURE_SUPPORT_FLOW_CONTROL_MODE       0x0001
#define L2CAP_EXTFEATURE_SUPPORT_RETRANSMISSION_MODE     0x0002
#define L2CAP_EXTFEATURE_SUPPORT_BIDIRECTIONAL_QOS       0x0004
#define L2CAP_EXTFEATURE_SUPPORT_ENHANCED_RETRANS_MODE   0x0008
#define L2CAP_EXTFEATURE_SUPPORT_STREAMING_MODE          0x0010
#define L2CAP_EXTFEATURE_SUPPORT_FCS_OPTION              0x0020
#define L2CAP_EXTFEATURE_SUPPORT_EXTENDED_FLOW_SPEC      0x0040
#define L2CAP_EXTFEATURE_SUPPORT_FIXED_CHANNELS          0x0080
#define L2CAP_EXTFEATURE_SUPPORT_EXTENDED_WINDOW_SIZE    0x0100
#define L2CAP_EXTFEATURE_SUPPORT_UNICAST_CONNECTIONLESS_DATA_RECEPTION   0x0200
#define L2CAP_EXTFEATURE_SUPPORT_ENHANCED_CREDIT_BASED_FLOW_CONTROL_MODE 0x0400

struct l2cap_sig_info_rsp {
    uint16 infotype;
    uint16 result;
    /*if result == success, data: mtu(2 bytes), feature mask(4 bytes) */
    uint32 mask;
}__attribute__ ((packed));

struct l2cap_sig_fcs_rsp {
    uint16 infotype;
    uint16 result;
    uint8 bitmask[8];
}__attribute__ ((packed));

#define L2CAP_SIG_CFG_MTU_MASK                  (1<<0)
#define L2CAP_SIG_CFG_FLUSHTO_MASK          (1<<1)
#define L2CAP_SIG_CFG_QOS_MASK                  (1<<2)
#define L2CAP_SIG_CFG_RFC_MASK                   (1<<3)
#define L2CAP_SIG_CFG_FCS_MASK                   (1<<4)


#if mHDT_SUPPORT
#define L2CAP_SIG_CFG_TYPE_MHDT                (0x7F)
// 

struct l2cap_sig_cfg_opt_mhdt 
{
    byte mhdt_support;
}__attribute__ ((packed));
#endif

#define BLUETOOTH_BT_ATT 0xFF

typedef enum {
    BLUETOOTH_BT_SDP = 0,
    BLUETOOTH_BT_RFC,
    BLUETOOTH_PROTO_MAX_NUM,
} bluetooth_proto_id_t;

struct l2cap_ctx_input {
    struct ctx_content ctx;
    struct bdaddr_t *remote;
    uint32 l2cap_handle;
    uint16 conn_handle;
    bool is_ble_l2cap;
};

struct l2cap_ctx_output {
    uint32 l2cap_handle;
};

typedef struct l2cap_conn l2cap_conn_t;
typedef struct l2cap_channel l2cap_channel_t;
struct l2cap_cfg_param_t;
struct l2cap_enre_channel_t;

enum l2cap_channel_state_enum {
    L2CAP_CLOSE,                /*baseband connection closed, wait for hci conn openning, and then can send out conn request signal*/
    L2CAP_WAIT_DISCONNECT,
    L2CAP_WAITING,              /* waitf for the baseband connection to send out conn req signal */
    L2CAP_AUTH_PENDING, /* waiting for baseband authentication or encryption */
    L2CAP_WAIT_CONNECTION_RSP,
    L2CAP_WAIT_CONFIG,
    L2CAP_WAIT_CONFIG_REQ_RSP,
    L2CAP_WAIT_CONFIG_RSP,
    L2CAP_WAIT_CONFIG_REQ,
    L2CAP_OPEN
};

struct l2cap_channel {
    struct single_link_node_t node;
    l2cap_conn_t *conn;
    bt_l2cap_port_t *port;

    uint32 l2cap_handle;
    uint16 scid;
    uint16 dcid;
    uint16 psm_remote;
    int16 scid_arridx;
    uint8 initiator;    /* local or peer initate l2cap channel*/
    uint8 sigid_last_send;   /*to save our last request signal id*/
    uint8 sigid_last_recv;  /*to save the last remote's request signal id*/
    enum l2cap_channel_state_enum state;

    uint8 disconnect_req_timeout_timer;
    uint8 disconnect_req_reason;
    bool disc_req_pending;
    bool disc_channel_after_conn_rsp;
    bool dont_report_close_event;
    bool slave_ignore_wrong_state;
    uint8 l2cap_send_role;

    struct l2cap_cfg_param_t *cfg; // only valid before channel open

#ifdef BT_L2CAP_ENRE_MODE_SUPPORT
    struct l2cap_enre_channel_t *enre_chan;
    bool may_use_enre_mode;
#endif

    uint8 l2cap_mode;

    /**
     * MTU is not a negotiated value, it is an infomational parameter that each device
     * can specify independently. It indicates to the remote device that the local
     * device can receive, in this channel, an MTU larger than the minimum required.
     * All l2cap shall support a min MTU of 48 octets over the ACL-U logical link and
     * 23 octets over the LE-U logical link. A request specify any MTU gerater than
     * or equal to the min MTU for the channel shall be accepted. A request specifying
     * an MTU smaller than the min MTU for the channel may be rejcted. The default value
     * is 672 octets.
     */
    uint16 expected_mtu_size;
    uint16 local_rx_mtu; // max transmission unit, max sdu size
    uint16 remote_rx_mtu;

#if (EATT_CHAN_SUPPORT == 1)
    struct single_link_head_t tx_pend_list;
    struct pp_buff *curr_l2cap_sdu;
    uint16 local_rx_mps; // max pdu payload size
    uint16 remote_rx_mps;
    uint16 tx_credits;
    uint16 rx_credits;
    uint8 rx_initial_credits;
    uint8 rx_credit_give_step;
#endif
};

typedef struct { /* placeholder */ } bt_proto_conn_t; // pointer to (l2cap_proto_conn_t + 1)

struct l2cap_conn {
    struct single_link_head_t chnl_list;
    hci_conn_type_t conn_type;
    bt_addr_type_t addr_type;
    uint16_t conn_handle;
    struct bdaddr_t remote;
    uint8_t info_req_sent: 1;
    uint8_t echo_req_sent: 1;
    uint8_t disconnect_by_acl: 1;
    uint8_t sigid_seed;
    uint8_t device_id;
    uint8_t pending_echo_rsp_count;
    uint8_t disconnect_reason;
    uint8_t delay_free_conn_timer;
    uint8_t latest_scid_arridx;
    uint8_t pend_enfc_out_id;
    uint16_t pend_enfc_out_spsm;
    uint16_t remote_ext_features;
    uint8_t remote_fixed_channels;
    bt_proto_conn_t *proto[BLUETOOTH_PROTO_MAX_NUM];
};

bt_status_t l2cap_create_port(uint16_t local_l2cap_psm, bt_l2cap_callback_t l2cap_callback);
bt_status_t l2cap_remove_port(uint16_t local_l2cap_psm);
bt_status_t l2cap_set_rx_credits(uint16_t local_l2cap_psm, uint16_t expected_rx_mtu, uint16_t initial_credits, uint16_t credit_give_step);
bt_status_t l2cap_listen(uint16_t local_l2cap_psm);
bt_status_t l2cap_remove_listen(uint16_t local_l2cap_psm);
bt_status_t l2cap_enfc_connect(hci_conn_type_t conn_type, uint16_t connhdl, uint16_t local_spsm);
uint32_t l2cap_connect(const bt_bdaddr_t *remote, uint16_t local_l2cap_psm, l2cap_psm_target_profile_t target);
uint32_t l2cap_enre_connect(const bt_bdaddr_t *remote, uint16_t local_l2cap_psm, l2cap_psm_target_profile_t target, uint16_t remote_l2cap_psm);
void l2cap_notify_rx_done_event(uint16_t connhdl, uint32_t l2cap_handle);
void l2cap_defer_free_rx_buffer(struct pp_buff *ppb);
int8 l2cap_send_disconnect_req(uint8 device_id, l2cap_channel_t *channel);

typedef void (*l2cap_sdp_disconnect_callback)(const bt_bdaddr_t *bdaddr);
typedef uint8 (*btm_get_ibrt_role_callback)(const bt_bdaddr_t *para);
typedef uint8 (*btm_get_ui_role_callback)(void);
typedef uint8_t (*btm_get_tss_state_callback)(const bt_bdaddr_t *para);

typedef void (*l2cap_fill_in_echo_req_data_callback_func)(uint8 device_id, l2cap_conn_t *conn, uint8 *data, uint16 data_len);
void register_l2cap_fill_in_echo_req_data_callback(l2cap_fill_in_echo_req_data_callback_func func);
void l2cap_fill_in_echo_req_data(uint8 device_id, l2cap_conn_t *conn, uint8 *data, uint16 data_len);
typedef void (*l2cap_process_echo_req_callback_func)(uint8 device_id, uint16 conhdl, uint8 id, uint16 len, uint8 *data);
void register_l2cap_process_echo_req_callback(l2cap_process_echo_req_callback_func func);
void l2cap_process_echo_req_rewrite_rsp_data(uint8 device_id, uint16 conhdl, uint8 id, uint16 len, uint8 *data);
typedef void (*l2cap_process_echo_res_callback_func)(uint8 device_id, uint16 conhdl, uint8* rxdata, uint16 rxlen);
void register_l2cap_process_echo_res_callback(l2cap_process_echo_res_callback_func func);
void l2cap_process_echo_res_analyze_data(uint8 device_id, uint16 conhdl, uint8* rxdata, uint16 rxlen);

typedef void (*l2cap_process_bredr_smp_req_callback_func)(uint8 device_id, uint16 conn_handle, uint16 len, uint8 *data);
void register_l2cap_process_bredr_smp_req_callback(l2cap_process_bredr_smp_req_callback_func func);
int8 l2cap_send_bredr_security_manager_rsp(uint8 device_id, uint16 conn_handle, uint16 len, uint8 *data);
int8 l2cap_send_echo_req(uint8 device_id, struct l2cap_conn *conn, uint16 len, const uint8* data);
int8 l2cap_send_echo_rsp(uint8 device_id, uint16 conn_handle, uint8 sigid, uint16 len, const uint8* data);

typedef void (*bt_proto_conn_open_close_t)(l2cap_conn_t* l2cap_conn, bt_proto_conn_t *proto_conn, bool open);
bt_proto_conn_t *l2cap_proto_conn_add_new(const bt_bdaddr_t *remote, bluetooth_proto_id_t proto_id);
bt_proto_conn_t *l2cap_proto_conn_find(const bt_bdaddr_t *remote, bluetooth_proto_id_t proto_id);
bt_proto_conn_t *l2cap_proto_conn_search(uint32_t l2cap_handle, bluetooth_proto_id_t proto_id);
l2cap_conn_t *l2cap_conn_from_proto_conn(bt_proto_conn_t *proto_conn, bluetooth_proto_id_t *out_id);
void l2cap_proto_conn_free(bt_proto_conn_t *proto_conn);
bt_bdaddr_t l2cap_proto_conn_get_address(bt_proto_conn_t *proto_conn);
uint16_t l2cap_proto_conn_get_connhdl(bt_proto_conn_t *proto_conn);
uint8_t l2cap_proto_conn_get_device_id(bt_proto_conn_t *proto_conn);
uint8_t l2cap_get_device_id_by_address(const bt_bdaddr_t *remote);
int l2cap_proto_conn_foreach(bluetooth_proto_id_t proto_id, bool (*cb)(bt_proto_conn_t *conn, void *priv), void *priv);
bt_proto_conn_t *l2cap_get_att_proto_conn(uint16_t connhdl);
bt_proto_conn_t *l2cap_get_att_from_l2cap_conn(l2cap_conn_t *conn);
bt_proto_conn_t *l2cap_get_att_conn_by_idx(hci_conn_type_t conn_type, uint8_t con_idx);
bt_proto_conn_t *l2cap_get_att_conn_by_address(hci_conn_type_t conn_type, bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr);
typedef bool (*att_proto_conn_foreach_t)(bt_proto_conn_t *conn, void *priv);
int l2cap_foreach_att_proto_conn(att_proto_conn_foreach_t cb, void *priv);
void l2cap_set_att_expected_mtu_to_port(hci_conn_type_t conn_type, uint16_t expected_mtu);
uint16_t l2cap_get_att_mtu_from_port(hci_conn_type_t conn_type);
bt_status_t l2cap_send_packet(uint32_t connhdl_len, uint32_t l2cap_handle, const uint8_t *data, void *context);
bt_status_t l2cap_send_att_packet(uint16_t connhdl, uint32_t l2cap_handle, struct pp_buff *ppb, void *context);
bt_status_t l2cap_send_smp_packet(uint16_t connhdl, struct pp_buff *ppb);
bt_l2cap_config_t *l2cap_set_config(void);

int8 l2cap_init(uint8_t proto_id, uint16_t struct_size, bt_proto_conn_open_close_t cb);
int8 l2cap_channel_close(uint8 device_id, l2cap_channel_t *channel, uint8 reason);
void l2cap_ble_datarecv_callback(uint16_t connhdl, struct pp_buff *ppb);
void l2cap_send_frame_done(uint16_t connhdl, struct pp_buff *ppb);
void l2cap_create_besaud_extra_channel(void* remote, bt_l2cap_callback_t l2cap_callback);
void l2cap_find_and_free_pending_avdtp_channel(struct bdaddr_t* remote);
uint16 l2cap_get_converted_psm_from_handle(uint32 l2cap_handle);
uint16_t l2cap_get_tx_mtu(uint32 l2cap_handle);
bool l2cap_is_valid_dynamic_psm(uint16 psm);
l2cap_channel_t *l2cap_channel_search_dcid(l2cap_conn_t *conn, uint16 dcid);
struct pp_buff *l2cap_data_ppb_alloc_with_ca_line(uint32 datalen, void *context, uint32_t ca, uint32_t line, bool auto_grow_alloc);
#define l2cap_data_ppb_alloc(datalen, context) l2cap_data_ppb_alloc_with_ca_line(datalen, context, (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__, false)
int8 l2cap_send_data_ppb( uint32 l2cap_handle, struct pp_buff *ppb);
int8 l2cap_send_data_auto_fragment(uint32 l2cap_handle, const uint8* data, uint32 len, void *context);
int8 l2cap_send_data( uint32 l2cap_handle, uint8 *data, uint32 datalen, void *context);
int8 l2cap_send_data_link(uint32 l2cap_handle, struct data_link *head, void *context);
int l2cap_enre_send_data(uint32 l2cap_handle, const uint8 *sdu_data, uint32 datalen, void* context);
void l2cap_btm_notify_callback(uint8_t device_id, enum btm_l2cap_event_enum event, uint16 conn_handle, void *pdata, uint8 reason);
void l2cap_btm_datarecv_callback (uint8_t device_id, uint16 conn_handle, struct pp_buff *ppb);
struct bdaddr_t *l2cap_get_conn_remote_addr(uint32 l2cap_handle);
void l2cap_conn_reset_sigid(struct bdaddr_t *bdaddr);
l2cap_conn_t *l2cap_conn_search(const bt_bdaddr_t *bdaddr);
l2cap_conn_t *l2cap_conn_search_by_handle(uint16 conn_handle);
l2cap_channel_t *l2cap_channel_search_psm(l2cap_conn_t *conn, uint16 psm);
l2cap_channel_t *l2cap_channel_search_exist_psm(const bt_bdaddr_t *remote, uint16_t psm);
l2cap_channel_t *l2cap_channel_search_l2caphandle(uint32_t l2cap_handle, l2cap_conn_t *conn);
l2cap_channel_t *l2cap_accept_conn_req(bt_l2cap_accept_t *accept);
void l2cap_reject_conn_req(bt_l2cap_accept_t *accept, uint16 reason);
void l2cap_clean_ibrt_slave_status(void *remote);
int8 l2cap_close(uint32 l2cap_handle);
int8 l2cap_close_v2(uint32 l2cap_handle,uint8 reason);
int8 l2cap_close_after_conn_rsp(uint32 l2cap_handle);
void l2cap_report_le_fixed_channel_opened(uint16_t connhdl);
struct pp_buff *l2cap_alloc_fragment_ppb(uint16_t dcid, uint32 total_len, const void* curr_data, uint32 curr_len, void *context, bool is_start_packet);

#ifdef BLE_HOST_SUPPORT
typedef struct gap_conn_prefer_params_t gap_conn_prefer_params_t;
void gap_recv_l2cap_conn_param_update_req(uint16_t connhdl, uint8_t trans_id, const gap_conn_prefer_params_t *params);
void gap_recv_l2cap_conn_param_update_rsp(uint16_t connhdl, uint8_t result);
bt_status_t l2cap_accept_le_conn_parameters(uint16_t connhdl, uint8_t trans_id, bool accept, bool send_cmd_rej);
bt_status_t l2cap_update_le_conn_parameters(uint16_t connhdl, const gap_conn_prefer_params_t *params);
#endif

void l2cap_pts_send_disconnect_channel(void);
void l2cap_pts_send_l2cap_data(void);
uint32 l2cap_save_ctx(uint32 l2cap_handle, uint8_t *buf, uint32_t buf_len);
uint32 l2cap_restore_ctx(struct l2cap_ctx_input *input, struct l2cap_ctx_output *output, void (*close_old_channel_cb)(uint8_t));

typedef struct {
    uint16_t proto_size;
    bt_proto_conn_open_close_t cb;
} l2cap_proto_conn_t;

struct l2cap_global_t {
    bool l2cap_init_flag;
    uint8_t l2cap_conn_count;
    uint8_t ble_l2cap_conn_count;
    uint32_t l2cap_handle_seed;
    l2cap_proto_conn_t att_conn;
    l2cap_proto_conn_t proto[BLUETOOTH_PROTO_MAX_NUM];
    struct single_link_head_t psm_reg_list;
    struct single_link_head_t open_req_list;
};

enum l2cap_search_scid_ctx_t
{
    L2CAP_SCID_CTX_DONT_CARE,
    L2CAP_SCID_CTX_DISC_RSP = 1,
    L2CAP_SCID_CTX_CONN_RSP,
    L2CAP_SCID_CTX_CFG_RSP,
    L2CAP_SCID_CTX_CFG_REQ,
    L2CAP_SCID_CTX_DATA_IND,
    L2CAP_SCID_CTX_DISC_REQ,
    L2CAP_SCID_CTX_ENFC_REQ,
};
#if defined(__cplusplus)
}
#endif
#endif /* __L2CAP_I_H__ */
