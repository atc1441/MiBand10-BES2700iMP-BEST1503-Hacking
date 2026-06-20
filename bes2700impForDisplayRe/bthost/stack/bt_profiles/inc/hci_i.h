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
#ifndef __HCI_I_H__
#define __HCI_I_H__
#include "bt_common_define.h"
#if defined(__cplusplus)
extern "C" {
#endif

#define HCI_STATUS_OK                               0x00
#define HCI_ERR_UNKNOWN_HCI_CMD                     0x01
#define HCI_ERR_NO_CONNECTION                       0x02
#define HCI_ERR_HARDWARE_FAILURE                    0x03
#define HCI_ERR_PAGE_TIMEOUT                        0x04
#define HCI_ERR_AUTH_FAILURE                        0x05
#define HCI_ERR_KEY_MISSING                         0x06
#define HCI_ERR_MEMORY_FULL                         0x07
#define HCI_ERR_CONN_TIMEOUT                        0x08
#define HCI_ERR_MAX_NUM_CONNS                       0x09
#define HCI_ERR_MAX_SCO_CONNS                       0x0A
#define HCI_ERR_ACL_ALREADY_EXISTS                  0x0B
#define HCI_ERR_CMD_DISALLOWED                      0x0C
#define HCI_ERR_HOST_REJ_NO_RESOURCES               0x0D
#define HCI_ERR_HOST_REJ_SECURITY                   0x0E
#define HCI_ERR_HOST_REJ_PERSONAL_DEV               0x0F
#define HCI_ERR_HOST_TIMEOUT                        0x10
#define HCI_ERR_UNSUPP_FEATUR_PARM_VAL              0x11
#define HCI_ERR_INVAL_HCI_PARM_VAL                  0x12
#define HCI_ERR_CONN_TERM_USER_REQ                  0x13
#define HCI_ERR_CONN_TERM_LOW_RESOURCES             0x14
#define HCI_ERR_CONN_TERM_POWER_OFF                 0x15
#define HCI_ERR_CONN_TERM_LOCAL_HOST                0x16
#define HCI_ERR_REPEATED_ATTEMPTS                   0x17
#define HCI_ERR_PAIRING_DISALLOWED                  0x18
#define HCI_ERR_UNKNOWN_LMP_PDU                     0x19
#define HCI_ERR_UNSUPP_REMOTE_FEATURE               0x1A
#define HCI_ERR_SCO_OFFSET_REJECTED                 0x1B
#define HCI_ERR_SCO_INTERVAL_REJECTED               0x1C
#define HCI_ERR_SCO_AIR_MODE_REJECTED               0x1D
#define HCI_ERR_INVALID_LMP_PARM                    0x1E
#define HCI_ERR_UNSPECIFIED_ERROR                   0x1F
#define HCI_ERR_UNSUPP_LMP_PARM                     0x20
#define HCI_ERR_ROLE_CHANGE_DISALLOWED              0x21
#define HCI_ERR_LMP_RESPONSE_TIMEDOUT               0x22
#define HCI_ERR_LMP_ERR_TRANSACT_COLL               0x23
#define HCI_ERR_LMP_PDU_DISALLOWED                  0x24
#define HCI_ERR_ENCRYPTN_MODE_UNACCEPT              0x25
#define HCI_ERR_UNIT_KEY_USED                       0x26
#define HCI_ERR_QOS_NOT_SUPPORTED                   0x27
#define HCI_ERR_INSTANT_PASSED                      0x28
#define HCI_ERR_PAIRING_W_UNIT_KEY_UNSUPP           0x29
#define HCI_ERR_DIFFERENT_TRANSACTION_COLLISION     0x2A
#define HCI_ERR_INSUFF_RESOURCES_FOR_SCATTER_MODE   0x2B
#define HCI_ERR_QOS_UNACCEPTABLE_PARAMETER          0x2C
#define HCI_ERR_QOS_REJECTED                        0x2D
#define HCI_ERR_CHANNEL_CLASSIF_NOT_SUPPORTED       0x2E
#define HCI_ERR_INSUFFICIENT_SECURITY               0x2F
#define HCI_ERR_PARAMETER_OUT_OF_MANDATORY_RANGE    0x30
#define HCI_ERR_SCATTER_MODE_NO_LONGER_REQUIRED     0x31
#define HCI_ERR_ROLE_SWITCH_PENDING                 0x32
#define HCI_ERR_SCATTER_MODE_PARM_CHNG_PENDING      0x33
#define HCI_ERR_RESERVED_SLOT_VIOLATION             0x34
#define HCI_ERR_SWITCH_FAILED                       0x35
#define HCI_ERR_EXTENDED_INQ_RESP_TOO_LARGE         0x36
#define HCI_ERR_SECURE_SIMPLE_PAIR_NOT_SUPPORTED    0x37
#define HCI_ERR_HOST_BUSY_PAIRING                   0x38

/* voice setting*/
#define INPUT_CODING_LINEAR 0x0000
#define INPUT_CODING_ULAW 0x0100
#define INPUT_CODING_ALAW 0x0200
#define INPUT_DATA_FORMAT_1S 0x0000
#define INPUT_DATA_FORMAT_2S 0x0040
#define INPUT_DATA_FORMAT_SIGN 0x0080
#define INPUT_DATA_FORMAT_UNSIGN 0x00C0
#define INPUT_SAMPLE_SIZE_8BITS 0x0000
#define INPUT_SAMPLE_SIZE_16BITS 0x0020
#define AIR_CODING_FORMAT_CVSD 0x0000
#define AIR_CODING_FORMAT_ULAW 0x0001
#define AIR_CODING_FORMAT_ALAW 0x0002
#define AIR_CODING_FORMAT_TRANSPARENT 0x0003
#define AIR_CODING_FORMAT_MSBC 0x0060

/* ACL */
#define HCI_PKT_TYPE_DM1  0x0008
#define HCI_PKT_TYPE_DH1  0x0010
#define HCI_PKT_TYPE_DM3  0x0400
#define HCI_PKT_TYPE_DH3  0x0800
#define HCI_PKT_TYPE_DM5  0x4000
#define HCI_PKT_TYPE_DH5  0x8000
#define HCI_PKT_TYPE_ACL  0xcc18

/* sco esco */
#define HCI_PKT_TYPE_HV1  0x0001
#define HCI_PKT_TYPE_HV2  0x0002
#define HCI_PKT_TYPE_HV3  0x0004
#define HCI_PKT_TYPE_EV3  0x0008
#define HCI_PKT_TYPE_EV4  0x0010
#define HCI_PKT_TYPE_EV5  0x0020
#define HCI_PKT_TYPE_2EV3   0x0040
#define HCI_PKT_TYPE_3EV3   0x0080
#define HCI_PKT_TYPE_2EV5   0x0100
#define HCI_PKT_TYPE_3EV5   0x0200
#define HCI_PKT_TYPE_SCO  0x003f

#define HCI_HANDLE_MASK                        0x0FFF
#define HCI_FLAG_PB_MASK                      0x3000
#define HCI_FLAG_BROADCAST_MASK       0xC000

#define HCI_BLE_CISHDL_MIN                     0X100
#define HCI_BLE_BISHDL_MIN                     0X200

#define BROADCAST_NONE                         0x0000
#define BROADCAST_ACTIVE                      0x4000
#define BROADCAST_PARKED                     0x8000

/*create connection param*/
#define ALLOW_ROLE_SWITCH_YES   0x01
#define ALLOW_ROLE_SWITCH_NO    0x00

/*accept connection req param*/
#define ROLE_BECAME_MASTER 0x00
#define ROLE_REMAIN_SLAVE 0x01

/* link policy */
#define HCI_LP_ENABLE_ROLE_SWITCH_MASK  0x01
#define HCI_LP_ENABLE_HOLD_MODE_MASK        0x02
#define HCI_LP_ENABLE_SNIFF_MODE_MASK       0x04
#define HCI_LP_ENABLE_PARK_MODE_MASK        0x08

/// Enhanced Synchronous Connection HCI:7.1.41 & 7.1.42
#define    CODING_FORMAT_ULAW          0x00
#define    CODING_FORMAT_ALAW          0x01
#define    CODING_FORMAT_CVSD          0x02
#define    CODING_FORMAT_TRANSP        0x03
#define    CODING_FORMAT_LINPCM        0x04
#define    CODING_FORMAT_MSBC          0x05
#define    CODING_FORMAT_VENDSPEC      0xFF

#define    PCM_FORMAT_NA               0x00
#define    PCM_FORMAT_1SCOMP           0x01
#define    PCM_FORMAT_2SCOMP           0x02
#define    PCM_FORMAT_SIGNMAG          0x03
#define    PCM_FORMAT_UNSIGNED         0x04

#define    PCM_SAMPLE_SIZE_8BITS       8
#define    PCM_SAMPLE_SIZE_16BITS      16

#define    AUDIO_DATA_PATH_HCI               0
#define    AUDIO_DATA_PATH_PCM               1

#ifndef BTM_NAME_MAX_LEN
#define BTM_NAME_MAX_LEN 248
#endif

/******************************************************************************************
 * Event Evtcode Definition  (OLD)
 *****************************************************************************************/
#define HCI_MasterLinkKeyCompleteEvt_Code                0x0A
#define HCI_PageScanModeChangeEvt_Code                   0x1F
#define HCI_PageScanRepetitionModeChangeEvt_Code         0x20
#define HCI_InquiryResultEvt_withRSSI                    0x22

#define HCI_EV_INQUIRY_COMPLETE 0x01
 struct hci_ev_inquiry_complete {
    uint8 status;
}__attribute__ ((packed));

#define HCI_EV_INQUIRY_RESULT        0x02
#define HCI_EV_INQUIRY_RESULT_RSSI   0x22
#define HCI_EV_INQUIRY_RESULT_EXTINQ 0x2F

#define PAGE_SCAN_REPETITION_MODE_R0    0x00
#define PAGE_SCAN_REPETITION_MODE_R1    0x01
#define PAGE_SCAN_REPETITION_MODE_R2    0x02
#define PAGE_SCAN_PERIOD_MODE_P0    0x00
#define PAGE_SCAN_PERIOD_MODE_P1    0x01
#define PAGE_SCAN_PERIOD_MODE_P2    0x02
 struct hci_ev_inquiry_result {
    uint8 num_responses;
    struct bdaddr_t bdaddr;
    uint8 page_scan_repetition_mode;
    uint8 reserved1;
    uint8 reserved2;/*must be 0*/
    uint8 class_dev[3];
    uint16 clk_off;
}__attribute__ ((packed));

#define HCI_EV_CONN_COMPLETE    0x03
struct hci_ev_conn_complete {
    uint8     status;
    uint16   handle;
    struct bdaddr_t bdaddr;
    uint8     link_type;
    uint8     encr_mode;
}__attribute__ ((packed));

#define HCI_EV_CONN_REQUEST 0x04

#define HCI_LINK_TYPE_SCO 0x00
#define HCI_LINK_TYPE_ACL 0x01
#define HCI_LINK_TYPE_ESCO 0x02
struct hci_ev_conn_request {
    struct bdaddr_t bdaddr;
    uint8     class_dev[3];
    uint8     link_type;
}__attribute__ ((packed));

#define HCI_EV_DISCONN_COMPLETE 0x05
struct hci_ev_disconn_complete {
    uint8     status;
    uint16   handle;
    uint8     reason;
}__attribute__ ((packed));

#define HCI_EV_AUTHENTICATION_COMPLETE  0x06
struct hci_ev_authentication_complete {
    uint8     status;
    uint16   handle;
}__attribute__ ((packed));

#define HCI_EV_REMOTENAMEREQ_COMPLETE   0x07
#define HCI_REMOTENAME_MAX    248
struct hci_ev_remote_name_req_complete {
    uint8    status;
    struct   bdaddr_t bdaddr;
    uint8    name[HCI_REMOTENAME_MAX];
}__attribute__ ((packed));

#define HCI_EV_READ_REMOTE_FEATURES 0x0B
 struct hci_ev_read_remote_features{
    uint8    status;
    uint16   conn_handle;
    uint8    features[8];
}__attribute__ ((packed));

#define HCI_EV_QOSSETUP_COMPLETE 0x0D
struct hci_ev_qossetup_complete{
    uint8    status;
    uint16   conn_handle;
    uint8    flags;
    uint8 service_type;
    uint32 token_rate;
    uint32 peak_bandwith;
    uint32 latency;
    uint32 delay_v;
}__attribute__ ((packed));


#define HCI_EV_CMD_COMPLETE 0x0e
struct hci_ev_cmd_complete {
    uint8   num_hci_cmd_packets;
    uint16  cmd_opcode;
    uint8   param[1];
}__attribute__ ((packed));

#define HCI_EV_CMD_STATUS   0x0f
struct hci_ev_cmd_status {
    uint8_t   status;
    uint8_t   num_hci_cmd_packets;
    uint16_t  cmd_opcode;
}__attribute__ ((packed));

#define HCI_EV_HARDWARE_ERROR   0x10
struct hci_ev_hardware_error {
    uint8 hw_code;
}__attribute__ ((packed));

#define HCI_EV_FLUSH_OCCURRED   0x11

#define HCI_EV_ROLE_CHANGE 0x12
 struct hci_ev_role_change{
    uint8 status;
    struct bdaddr_t bdaddr;
    uint8     new_role;
}__attribute__ ((packed));

 struct hci_ev_cmd_complete_read_stored_linkkey{
    uint8     status;
    uint16   max_num_keys;
    uint8    num_keys_read;
}__attribute__ ((packed));

 struct hci_ev_cmd_complete_le_read_buffer_size{
    uint8     status;
    uint16   le_data_packet_length;
    uint8    total_num_le_data_packets;
}__attribute__ ((packed));

 struct hci_ev_cmd_complete_role_discovery{
    uint8     status;
    uint16   connection_handle;
    uint8    current_role;
}__attribute__ ((packed));


struct hci_ev_cmd_complete_read_local_version{
    uint8 status;
    uint8 hci_version;
    uint16 hci_revision;
    uint8 lmp_version;
    uint16 manu_name;
    uint16 lmp_subversion;
}__attribute__ ((packed));

struct hci_ev_cmd_complete_read_local_sup_features{
    uint8 status;
    uint8 features[8];
}__attribute__ ((packed));

struct hci_ev_cmd_complete_read_local_ext_features{
    uint8 status;
    uint8 page_num;
    uint8 max_page_num;
    uint8 features[8];
}__attribute__ ((packed));

#define HCI_EV_NUM_OF_CMPLT 0x13
 struct hci_ev_num_of_complete{
    uint8  num_handle;
    uint16 handle;
    uint16 num_of_comp;
}__attribute__ ((packed));

 struct hci_rd_bt_rssi_cmd_cmp_evt{
     ///Status for command reception
     uint8_t status;
     ///Connection handle
     uint16_t conhdl;
     ///RSSI value
     int8_t rssi;
 }__attribute__ ((packed));

 struct hci_ev_num_of_complete_item{
    uint16 handle;
    uint16 num_of_comp;
}__attribute__ ((packed));

#define HCI_EV_MODE_CHANGE  0x14

#define HCI_MODE_ACTIVE 0x00
#define HCI_MODE_HOLD 0x01
#define HCI_MODE_SNIFF 0x02
#define HCI_MODE_PARK 0x03

 struct hci_ev_mode_change {
    uint8     status;
    uint16   handle;
    uint8    current_mode;
    uint16  interval;
}__attribute__ ((packed));

#define HCI_EV_RETURN_LINKKEYS  0x15

 struct hci_ev_return_linkkeys{
    uint8     num_keys;
    struct bdaddr_t bdaddr;
    uint8   link_key[16];
}__attribute__ ((packed));

#define HCI_EV_PIN_CODE_REQ 0x16
 struct hci_ev_pin_code_req {
    struct bdaddr_t bdaddr;
}__attribute__ ((packed));

#define HCI_EV_LINK_KEY_REQ 0x17
 struct hci_ev_link_key_req {
    struct bdaddr_t bdaddr;
}__attribute__ ((packed));

#define HCI_EV_DATABUF_OVERFLOW 0x1A
struct hci_ev_databuf_overflow{
    uint8 link_type;
}__attribute__ ((packed));


#define HCI_EV_MAX_SLOT_CHANGE  0x1B
 struct hci_ev_max_slot_change {
    uint16 handle;
    uint8   max_slots;
}__attribute__ ((packed));

#define HCI_EV_READ_CLKOFF_Code              0x1C
 struct hci_ev_read_clkoff{
    uint8    status;
    uint16     handle;
    uint16    clkoff;
}__attribute__ ((packed));

#define HCI_EV_CONNPKT_TYPE_CHANGE           0x1D
 struct hci_ev_connpkt_type_change {
    uint8 status;
    uint16 handle;
    uint16 pkt_type;
}__attribute__ ((packed));

#define HCI_EV_QOS_VIOLATION                         0x1E
 struct hci_ev_qos_violation {
    uint16 handle;
}__attribute__ ((packed));

#define HCI_EV_FLOW_SPECIFICATION                    0x21
 struct hci_ev_flow_specification {
    uint8 status;
    uint16 handle;
    uint8 flags;
    uint8 flow_dir;
    uint8 service_type;
    uint32 token_rate;
    uint32 token_bucket;
    uint32 peak_bandwidth;
    uint32 latency;
}__attribute__ ((packed));

#define HCI_EV_READ_REMOTE_EXTFEATURES   0x23
 struct hci_ev_read_remote_extfeatures {
    uint8 status;
    uint16 handle;
    uint8 page_num;
    uint8 max_page_num;
    uint8 ext_features[8];
}__attribute__ ((packed));

#define HCI_EV_SYNC_CONN_COMPLETE   0x2c
struct hci_ev_sync_conn_complete {
    uint8     status;
    uint16   handle;
    struct bdaddr_t bdaddr;
    uint8 link_type;
    uint8 tx_interval;
    uint8 retx_window;
    uint16 rx_pkt_length;
    uint16 tx_pkt_length;
    uint8 air_mode;
}__attribute__ ((packed));

#define HCI_EV_SYNC_CONN_CHANGE 0x2D
 struct hci_ev_sync_conn_change {
    uint8     status;
    uint16   handle;
    uint8 tx_interval;
    uint8 retx_window;
    uint16 rx_pkt_length;
    uint16 tx_pkt_length;
}__attribute__ ((packed));

#define HCI_EV_SNIFF_SUBRATING          0x2E
 struct hci_ev_sniff_subrating {
    uint8 status;
    uint16 handle;
    uint16 maximum_transmit_lantency;
    uint16 maximum_receive_lantency;
    uint16 minimum_remote_timeout;
    uint16 minimum_local_timeout;
}__attribute__ ((packed));

#define HCI_EV_IO_CAPABILITY_REQUEST    0x31
struct hci_ev_io_capability_request {
    struct bdaddr_t bdaddr;
}__attribute__ ((packed));

#define HCI_EV_IO_CAPABILITY_RESPONSE    0x32
struct hci_ev_io_capability_response {
    struct bdaddr_t bdaddr;
    uint8 io_capability;
    uint8 oob_data_present;
    uint8 authentication_requirement;
}__attribute__ ((packed));

#define HCI_EV_USER_CONFIRMATION_REQUEST    0x33
struct hci_ev_user_confirmation_request {
    struct bdaddr_t bdaddr;
    uint32_t numeric_value;
}__attribute__ ((packed));

#define HCI_EV_USER_PASSKEY_REQUEST      0x34
struct hci_ev_user_passkey_request {
    struct bdaddr_t bdaddr;
} __attribute__ ((packed));

#define HCI_EV_SIMPLE_PAIRING_COMPLETE    0x36
struct hci_ev_simple_pairing_complete{
    uint8 status;
    struct bdaddr_t bdaddr;
}__attribute__ ((packed));

#define HCI_EV_LINK_SUPERV_TIMEOUT_CHANGED    0x38

#define HCI_EV_ENH_FLUSH_COMPLETE 0x39

#define BT_HCI_EVT_USER_PASSKEY_NOTIFY      0x3B
struct bt_hci_evt_user_passkey_notify {
    struct bdaddr_t bdaddr;
    uint32_t passkey;
} __attribute__ ((packed));

#define BT_HCI_EVT_KEYPRESS_NOTIFY      0x3C
struct bt_hci_evt_keypress_notify {
    struct bdaddr_t bdaddr;
    uint8_t  type;
} __attribute__ ((packed));

#define HCI_EV_NUM_OF_CMPL_BLOCKS   0x48


#define HCI_LE_META_EVT    0x3E
#define HCI_SLE_META_EVT   0xEF

#define HCI_LE_EV_CONN_COMPLETE 0x01
struct hci_ev_le_conn_complete {
    uint8_t subcode;
    uint8_t status;
    uint16_t conn_handle;
    uint8_t role; // 0x00 central 0x01 peripheral
    uint8_t peer_addr_type; // 0x00 public 0x01 random
    bt_bdaddr_t peer_addr;
    uint16_t conn_interval;
    uint16_t peripheral_latency;
    uint16_t supv_timeout;
    uint8_t central_clock_accuracy;
} __attribute__ ((packed));

#define HCI_LE_EV_ADV_REPORT 0x02

#define HCI_LE_EV_CONN_UPDATE_COMPLETE 0x03
struct hci_ev_le_update_complete {
    uint8_t subcode;
    uint8_t status;
    uint16_t conn_handle;
    uint16_t conn_interval;
    uint16_t peripheral_latency;
    uint16_t supv_timeout;
} __attribute__ ((packed));

#define HCI_LE_EV_ENHANCE_CONN_COMPLETE 0x0A
struct hci_ev_le_enhance_conn_complete {
    uint8_t subcode;
    uint8_t status;
    uint16_t conn_handle;
    uint8_t role; // 0x00 central 0x01 peripheral
    uint8_t peer_addr_type; // 0x00 public 0x01 random 0x02 public identity 0x03 random static identity
    bt_bdaddr_t peer_addr;
    bt_bdaddr_t local_rpa;
    bt_bdaddr_t peer_rpa;
    uint16_t conn_interval;
    uint16_t peripheral_latency;
    uint16_t supv_timeout;
    uint8_t central_clock_accuracy;
} __attribute__ ((packed));

#define HCI_EV_DEBUG                0xFF
 struct hci_ev_debug {
    uint16   debug_evt_code;
    uint8 param[1];
}__attribute__ ((packed));

struct get_buffer
{
    /// length of buffer
    uint8_t length;
    /// data of 128 bytes length
    uint8_t data[128];
};

struct hci_ev_rd_mem_cmp_evt
{
    ///Status
    uint8_t status;
    ///buffer structure to return
    struct get_buffer buf;
};

#define BLE_ADV_DATA_LENGTH 31
#define BLE_SCAN_RSP_DATA_LENGTH 31

/******************************************************************************************
   OGF opcodes group define
   hence the values
******************************************************************************************/
#define HCI_OGF_BIT_OFFSET                ((INT8U) 10) /* Number of bit shifts */
#define HCI_OPCODE_MASK                       0x03FF

/******************************************************************************************
   OCF opcode defines
******************************************************************************************/

/******************************************************************************************
   OCF opcode defines - Link Control Commands  (OGF: 0x01)
******************************************************************************************/
//#define HCI_INQUIRY                         0x0401
#define HCI_INQUIRY_CANCEL                    0x0402
#define HCI_PERIODIC_INQUIRY_MODE             0x0403
#define HCI_EXIT_PERIODIC_INQUIRY_MODE        0x0404
//#define HCI_CREATE_CONNECTION                   0x0405
//#define HCI_DISCONNECT                      0x0406
#define HCI_ADDSCO_CONN                       0x0407
//#define HCI_CREATE_CONNECTION_CANCEL          0x0408
//#define HCI_ACCEPT_CONNECTION_REQ           0x0409
//#define HCI_REJECT_CONNECTION_REQ               0x040A
//#define HCI_LINK_KEY_REQ_REPLY                  0x040B
//#define HCI_LINK_KEY_REQ_NEG_REPLY          0x040C
//#define HCI_PIN_CODE_REQ_REPLY                  0x040D
//#define HCI_PIN_CODE_REQ_NEG_REPLY          0x040E
#define HCI_CHANGE_CONN_PKT_TYPE              0x040F
//#define HCI_AUTH_REQ                        0x0411
//#define HCI_SET_CONN_ENCRYPTION                 0x0413
#define HCI_CHANGE_CONN_LINK_KEY              0x0415
#define HCI_MASTER_LINK_KEY                   0x0417
//#define HCI_REMOTE_NAME_REQ                 0x0419
//#define HCI_REMOTE_NAME_CANCEL              0x041A
#define HCI_READ_REMOTE_SUP_FEATURES          0x041B
#define HCI_READ_REMOTE_VER_INFO              0x041D
#define HCI_READ_CLOCK_OFFSET                 0x041F
#define HCI_READREMOTE_EXT_FEATURES           0x041C
#define HCI_READ_LMP_HANDLE                   0x0420
//#define HCI_SETUP_SYNC_CONN                     0x0428
//#define HCI_ACCEPT_SYNC_CONN                0x0429
//#define HCI_REJECT_SYNC_CONN                0x042A

/******************************************************************************************
   OCF opcode defines - Link Policy Commands  (OGF 0x02)
 ******************************************************************************************/
#define HCI_HOLD_MODE                             0x0801
#define HCI_PARK_MODE                             0x0805
#define HCI_EXIT_PARK_MODE                        0x0806
//#define HCI_QOS_SETUP                             0x0807
#define HCI_ROLE_DISCOVERY                        0x0809
//#define HCI_SWITCH_ROLE                           0x080B
#define HCI_READ_LP_SETTINGS                      0x080C           //LP:LINK POLICY
#define HCI_READ_DEFAULT_LP_SETTINGS              0x080E
#define HCI_WRITE_DEFAULT_LP_SETTINGS             0x080F
#define HCI_FLOW_SPECIFICATION                    0x0810

/******************************************************************************************
 OCF opcode defines -Host Controller and Baseband Commands (OGF 0x03)
******************************************************************************************/
#define HCI_SET_EVENT_MASK                    0x0C01
#define HCI_RESET                             0x0C03
#define HCI_SET_EVENT_FILTER                  0x0C05
#define HCI_FLUSH                             0x0C08
#define HCI_READ_PIN_TYPE                     0x0C09
#define HCI_WRITE_PIN_TYPE                    0x0C0A
#define HCI_CREATE_NEW_UNIT_KEY               0x0C0B
//#define HCI_DELETE_STORED_LINK_KEY          0x0C12
//#define HCI_WRITE_LOCAL_NAME                  0x0C13
#define HCI_READ_LOCAL_NAME                   0x0C14
#define HCI_READ_CONN_ACCEPT_TIMEOUT          0x0C15
#define HCI_WRITE_CONN_ACCEPT_TIMEOUT         0x0C16
#define HCI_READ_PAGE_TIMEOUT                 0x0C17
#define HCI_WRITE_PAGE_TIMEOUT                0x0C18
#define HCI_READ_SCAN_ENABLE                  0x0C19
#define HCI_WRITE_SCAN_ENABLE                 0x0C1A
#define HCI_READ_PAGESCAN_ACTIVITY            0x0C1B
#define HCI_WRITE_PAGESCAN_ACTIVITY           0x0C1C
#define HCI_READ_INQUIRYSCAN_ACTIVITY         0x0C1D
#define HCI_WRITE_INQUIRYSCAN_ACTIVITY        0x0C1E
#define HCI_READ_AUTH_ENABLE                  0x0C1F
//#define HCI_WRITE_AUTH_ENABLE                   0x0C20
#define HCI_READ_ENCRY_MODE                   0x0C21
#define HCI_WRITE_ENCRY_MODE                  0x0C22
#define HCI_READ_CLASS_OF_DEVICE              0x0C23
//#define HCI_WRITE_CLASS_OF_DEVICE               0x0C24
#define HCI_READ_VOICE_SETTING                0x0C25
#define HCI_WRITE_VOICE_SETTING               0x0C26
#define HCI_READ_AUTO_FLUSH_TIMEOUT           0x0C27
#define HCI_WRITE_AUTO_FLUSH_TIMEOUT          0x0C28
#define HCI_READ_NUM_BCAST_RETRANSMIT         0x0C29
#define HCI_WRITE_NUM_BCAST_RETRANSMIT        0x0C2A
#define HCI_READ_HOLD_MODE_ACTIVITY           0x0C2B
#define HCI_WRITE_HOLD_MODE_ACTIVITY          0x0C2C
#define HCI_READ_TX_POWER_LEVEL               0x0C2D
#define HCI_READ_SYNC_FLOW_CON_ENABLE         0x0C2E
#define HCI_WRITE_SYNC_FLOW_CON_ENABLE        0x0C2F
#define HCI_SET_HCTOHOST_FLOW_CONTROL         0x0C31
#define HCI_HOST_BUFFER_SIZE                  0x0C33
#define HCI_HOST_NUM_COMPLETED_PACKETS        0x0C35
#define HCI_READ_LINK_SUPERV_TIMEOUT          0x0C36
//#define HCI_WRITE_LINK_SUPERV_TIMEOUT         0x0C37
#define HCI_READ_NUM_SUPPORTED_IAC            0x0C38
#define HCI_READ_CURRENT_IAC_LAP              0x0C39
#define HCI_WRITE_CURRENT_IAC_LAP             0x0C3A
#define HCI_READ_PAGESCAN_PERIOD_MODE         0x0C3B
#define HCI_WRITE_PAGESCAN_PERIOD_MODE        0x0C3C

#define HCI_READ_PAGESCAN_MODE                0x0C3D
#define HCI_WRITE_PAGESCAN_MODE               0x0C3E

//#define SET_AFH_HOST_CHL_CLASSFICAT           0x0C3F
#define HCI_READ_INQUIRYSCAN_TYPE             0x0C42
#define HCI_WRITE_INQUIRYSCAN_TYPE            0x0C43
#define HCI_READ_INQUIRY_MODE                 0x0C44
#define HCI_WRITE_INQUIRY_MODE                0x0C45
#define HCI_READ_PAGESCAN_TYPE                0x0C46
//#define HCI_WRITE_PAGESCAN_TYPE               0x0C47
#define HCI_READ_AFH_CHL_ASSESS_MODE          0x0C48
#define HCI_WRITE_AFH_CHL_ASSESS_MODE         0x0C49
#define HCI_WRITE_EXTENDED_INQ_RESP           0x0C52
//#define HCI_WRITE_SIMPLE_PAIRING_MODE         0x0C56
#define HCI_READ_DEF_ERR_DATA_REPORTING       0x0C5A
#define HCI_WRITE_DEF_ERR_DATA_REPORTING      0x0C5B
#define HCI_READ_FLOW_CONTROL_MODE            0x0C66
#define HCI_WRITE_FLOW_CONTROL_MODE           0x0C67
#define HCI_WR_LE_HOST_SUPPORT                0x0C6D
#define HCI_WR_SEC_CON_HOST_SUPP              0x0C7A

/******************************************************************************************
 OCF opcode defines -Information Parameters (OGF  0x04)
******************************************************************************************/
#define HCI_READ_LOCAL_VER_INFO               0x1001
#define HCI_READ_LOCAL_SUP_COMMANDS           0x1002
#define HCI_READ_LOCAL_SUP_FEATURES           0x1003
#define HCI_READ_LOCAL_EXT_FEATURES           0x1004
#define HCI_READ_BD_ADDR                      0x1009

/******************************************************************************************
 OCF opcode defines -Status Parameters (0GF 0X05)
******************************************************************************************/
#define HCI_READ_FAILED_CONTACT_COUNT              0x1401
#define HCI_RESET_FAILED_CONTACT_COUNT             0x1402
#define HCI_READ_LINK_QUALITY                      0x1403
#define HCI_READ_RSSI                              0x1405
#define HCI_READ_AFH_CHANNEL_MAP                   0x1406
#define HCI_READ_CLOCK                             0x1407

/******************************************************************************************
 *OCF opcode defines -Testing Commands (OGF 0X06)
******************************************************************************************/
#define HCI_READ_LOOPBACK_MODE                0x1801
#define HCI_WRITE_LOOPBACK_MODE               0x1802
#define HCI_ENABLE_DUT_MODE                   0x1803

/******************************************************************************************
 *OCF opcode defines -BLE Commands (OGF 0X08)
******************************************************************************************/
#define HCI_BLE_SET_EVENT_MASK                0x2001


/******************************************************************************************
 *OCF opcode defines -Vendor Commands (OGF 0xff)
******************************************************************************************/

#define HCI_READ_LMP_PARAM               0xFC01
#define HCI_SET_AFH                      0xFC02
//#define HCI_SET_BD_ADDR                    0xFC04
#define HCI_PRJ_VERSION                  0xFC05
#define HCI_GET_PKT_STATICS              0xFC06
//#define HCI_READ_MEMORY                  0xFC07
//#define HCI_WRITE_MEMORY                 0xFC08
#define HCI_READ_HW_REGISTER             0xFC09
#define HCI_WRITE_HW_REGISTER            0xFC0A
#define HCI_TEST_CONTROL                 0xFC0B
#define HCI_SEND_PDU                     0xFC10
#define HCI_SET_SCO_CHANNEL              0xFC11
#define HCI_SET_ESCO_CHANNEL             0xFC12
#define HCI_DBG_OPCODE                   0xFC3f
#define HCI_SET_UART_BAUD_RATE           0xFC13
#define HCI_SET_UART_PORT                0xFC14
#define HCI_SET_CLOCK                    0xFC15
#define HCI_GET_PKTS_ERR                 0xFC16
#define HCI_DEEP_SLEEP                   0xFC19
//#define HCI_SET_SCOOVER_TYPE             0xFC1A
#define HCI_SET_SCOOVER_TYPE             0xFC04
//#define HCI_GET_SCOOVER_TYPE             0xFC1B
#define HCI_GET_SCOOVER_TYPE             0xFC03

#define HCI_CONFIG_WRITE             0xFC1C
#define HCI_CONFIG_READ             0xFC1D
#define HCI_CONFIG_FIXED_FREQ             0xFC1E
#define HCI_CONFIG_HOP_FREQ               0xFC1F
#define HCI_GET_IVT_SECCODE             0xfc20
#define HCI_SET_IVT_SECCODE             0xfc21
#define HCI_SET_CLK_DBGMODE             0xfc22
#define HCI_SET_SLAVE_TEST_MODE         0xfc23

#define HCI_OGF_VENDOR  0x3f
#define HCI_OCF_CONFIG_WRITE     0x1C
#define HCI_OCF_CONFIG_READ      0x1D

#define HCI_EVTCODE_VENDOR  0xff

#define HCI_GET_OGF(val)  ((unsigned int)val>>10)
#define HCI_GET_OCF(val)  ((unsigned int)val & 0x3ff)
#define HCI_GET_OPCODE(ogf,ocf)  ((ogf<<10) + ocf)

#ifdef HCI_AFH
#define HCI_AFHEvt_Code     0xFD
#define HCI_ChannelEvt_Code  0xFE
#endif

/******************************************************************************************
 *OCF opcode defines -Debug Commands (OGF )
******************************************************************************************/

/* LM/HCI Errors                                                                        */
#define HCI_ERROR_NO_ERROR             0x00
#define HCI_ERROR_UNKNOWN_HCI_CMD      0x01
#define HCI_ERROR_NO_CONNECTION        0x02
#define HCI_ERROR_HARDWARE_FAILURE     0x03
#define HCI_ERROR_PAGE_TIMEOUT         0x04
#define HCI_ERROR_AUTHENTICATE_FAILURE 0x05
#define HCI_ERROR_MISSING_KEY          0x06
#define HCI_ERROR_MEMORY_FULL          0x07
#define HCI_ERROR_CONNECTION_TIMEOUT   0x08
#define HCI_ERROR_MAX_CONNECTIONS      0x09
#define HCI_ERROR_MAX_SCO_CONNECTIONS  0x0a
#define HCI_ERROR_ACL_ALREADY_EXISTS   0x0b
#define HCI_ERROR_COMMAND_DISALLOWED   0x0c
#define HCI_ERROR_LIMITED_RESOURCE     0x0d
#define HCI_ERROR_SECURITY_ERROR       0x0e
#define HCI_ERROR_PERSONAL_DEVICE      0x0f
#define HCI_ERROR_HOST_TIMEOUT         0x10
#define HCI_ERROR_UNSUPPORTED_FEATURE  0x11
#define HCI_ERROR_INVALID_HCI_PARM     0x12
#define HCI_ERROR_USER_TERMINATED      0x13
#define HCI_ERROR_LOW_RESOURCES        0x14
#define HCI_ERROR_POWER_OFF            0x15
#define HCI_ERROR_LOCAL_TERMINATED     0x16
#define HCI_ERROR_REPEATED_ATTEMPTS    0x17
#define HCI_ERROR_PAIRING_NOT_ALLOWED  0x18
#define HCI_ERROR_UNKNOWN_LMP_PDU      0x19
#define HCI_ERROR_UNSUPPORTED_REMOTE   0x1a
#define HCI_ERROR_SCO_OFFSET_REJECT    0x1b
#define HCI_ERROR_SCO_INTERVAL_REJECT  0x1c
#define HCI_ERROR_SCO_AIR_MODE_REJECT  0x1d
#define HCI_ERROR_INVALID_LMP_PARM     0x1e
#define HCI_ERROR_UNSPECIFIED_ERR      0x1f
#define HCI_ERROR_UNSUPPORTED_LMP_PARM 0x20
#define HCI_ERROR_ROLE_CHG_NOT_ALLOWED 0x21
#define HCI_ERROR_LMP_RESPONSE_TIMEOUT 0x22
#define HCI_ERROR_LMP_TRANS_COLLISION  0x23
#define HCI_ERROR_LMP_PDU_NOT_ALLOWED  0x24
#define HCI_ERROR_ENCRYP_MODE_NOT_ACC  0x25
#define HCI_ERROR_UNIT_KEY_USED        0x26
#define HCI_ERROR_QOS_NOT_SUPPORTED    0x27
#define HCI_ERROR_INSTANT_PASSED       0x28
#define HCI_ERROR_PAIR_UNITKEY_NO_SUPP 0x29
#define HCI_ERROR_DIFF_TRANS_COLLISION 0x2A
#define HCI_ERROR_QOS_UNACC_PARAMETER  0x2C
#define HCI_ERROR_QOS_REJECTED         0x2D
#define HCI_ERROR_CHN_ASSESS_NOT_SUPP  0x2E
#define HCI_ERROR_INSUFFI_SECURITY     0x2F
#define HCI_ERROR_PARM_OUT_OF_MAND_RNG 0x30
#define HCI_ERROR_ROLE_SWITCH_PENDING  0x32
#define HCI_ERROR_RSV_SLOT_VIOLATION   0x34
#define HCI_ERROR_ROLE_SWITCH_FAILED   0x35
#define HCI_ERROR_EIR_TOO_LARGE        0x36
#define HCI_ERROR_SSP_NOT_SUPP_BY_HOST 0x37
#define HCI_ERROR_HOST_BUSY_PAIRING    0x38
#define HCI_ERROR_CONN_REJ_NO_CHAN     0x39
#define HCI_ERROR_CONTROLLER_BUSY      0x3A
#define HCI_ERROR_UNACC_CONN_PARAMS    0x3B
#define HCI_ERROR_ADVERTISING_TIMEOUT  0x3C
#define HCI_ERROR_CONN_TERM_MIC_FAIL   0x3D
#define HCI_ERROR_CONN_FAILED_TO_EST   0x3E
#define HCI_ERROR_CCA_REJECTED_TRY_CD  0x40
#define HCI_ERROR_TYPE0_SUBMAP_NO_DEF  0x41
#define HCI_ERROR_UNKNOWN_ADV_ID       0x42
#define HCI_ERROR_LIMIT_REACHED        0x43
#define HCI_ERROR_OP_CANCELLED_BY_HOST 0x44
#define HCI_ERROR_PACKET_TOO_LONG      0x45
#define HCI_ERROR_TOO_LATE             0x46
#define HCI_ERROR_TOO_EARLY            0x47
#define HCI_ERROR_NOT_FOUND            0xf1
#define HCI_ERROR_REQUEST_CANCELLED    0xf2
#define HCI_ERROR_INVALID_SDP_PDU      0xd1
#define HCI_ERROR_SDP_DISCONNECT       0xd2
#define HCI_ERROR_SDP_NO_RESOURCES     0xd3
#define HCI_ERROR_SDP_INTERNAL_ERR     0xd4
#define HCI_ERROR_STORE_LINK_KEY_ERR   0xe0
//BES vendor error code
#define HCI_INVALID_BT_MODE            0xB2


#define HCI_INQUIRY                                   0x0401
struct hci_cp_inquiry
{
    uint8     lap[3];
    uint8     length;
    uint8     num_rsp;
} __attribute__ ((packed));

#define HCI_CREATE_CONNECTION                 0x0405
struct hci_cp_create_conn
{
    struct bdaddr_t bdaddr;
    uint16 pkt_type;
    uint8 page_scan_repetition_mode;
    uint8 reserved;
    uint16 clk_off;
    uint8 allow_role_switch;
} __attribute__ ((packed));

#define HCI_DISCONNECT                    0x0406
struct hci_cp_disconnect
{
    uint16   handle;
    uint8     reason;
} __attribute__ ((packed));

#define HCI_ADDSCO_CONN                       0x0407
struct hci_cp_addsco_conn
{
    uint16 conn_handle;
    uint16 pkt_type;
} __attribute__ ((packed));

#define HCI_CREATE_CONNECTION_CANCEL          0x0408
struct hci_create_connection_cancel
{
    struct bdaddr_t bdaddr;
} __attribute__ ((packed));

#define HCI_ACCEPT_CONNECTION_REQ             0x0409
struct hci_cp_accept_conn_req
{
    struct bdaddr_t bdaddr;

    uint8     role;
} __attribute__ ((packed));

#define HCI_REJECT_CONNECTION_REQ         0x040A
struct hci_cp_reject_conn_req
{
    struct bdaddr_t bdaddr;
    uint8     reason;
} __attribute__ ((packed));
#define HCI_LINK_KEY_REQ_REPLY            0x040B
struct hci_cp_link_key_reply
{
    struct bdaddr_t bdaddr;
    uint8     link_key[16];
} __attribute__ ((packed));

#define HCI_LINK_KEY_REQ_NEG_REPLY            0x040C
struct hci_cp_link_key_neg_reply
{
    struct bdaddr_t bdaddr;
} __attribute__ ((packed));

#define HCI_PIN_CODE_REQ_REPLY                0x040D
struct hci_cp_pin_code_reply
{
    struct bdaddr_t  bdaddr;
    uint8     pin_len;
    uint8     pin_code[16];
} __attribute__ ((packed));

#define HCI_PIN_CODE_REQ_NEG_REPLY            0x040E
struct hci_cp_pin_code_neg_reply
{
    struct bdaddr_t  bdaddr;
} __attribute__ ((packed));
#define HCI_CHANGE_CONN_PKT_TYPE              0x040F
struct hci_cp_change_conn_pkt_type
{
    uint16 conn_handle;
    uint16 pkt_type;
} __attribute__ ((packed));


#define HCI_AUTH_REQ                      0x0411
struct hci_cp_auth_req
{
    uint16 conn_handle;
} __attribute__ ((packed));

#define HCI_SET_CONN_ENCRYPTION                   0x0413
struct hci_cp_set_conn_encryption
{
    uint16 conn_handle;
    uint8 encryption_enable;
} __attribute__ ((packed));


#define HCI_REMOTE_NAME_REQ                   0x0419
struct hci_cp_remote_name_request
{
    struct bdaddr_t bdaddr;
    uint8 page_scan_repetition_mode;
    uint8 reserved;
    uint16 clk_off;
} __attribute__ ((packed));

#define HCI_REMOTE_NAME_CANCEL                0x041A
struct hci_cp_remote_name_cancel
{
    struct bdaddr_t bdaddr;
} __attribute__ ((packed));

#define HCI_REMOTE_SUPPORTED_FEATURES   0x041B
struct hci_cp_remote_supported_feat
{
    uint16 conn_handle;
} __attribute__ ((packed));

#define HCI_REMOTE_EXTENDED_FEATURES    0x041C
struct hci_cp_remote_extended_feat
{
    uint16 conn_handle;
    uint8 page_num;
} __attribute__ ((packed));

#define HCI_SETUP_SYNC_CONN           0x0428
struct hci_cp_setup_sync_conn
{
    uint16 conn_handle;
    uint32 tx_bandwidth;
    uint32 rx_bandwidth;
    uint16 max_latency;
    uint16 voice_setting;
    uint8 retx_effort;
    uint16 pkt_type;
} __attribute__ ((packed));

#define HCI_ACCEPT_SYNC_CONN              0x0429
struct hci_cp_accept_sync_conn
{
    struct bdaddr_t bdaddr;
    uint32 tx_bandwidth;
    uint32 rx_bandwidth;
    uint16 max_latency;
    uint16 voice_setting;
    uint8 retx_effort;
    uint16 pkt_type;
} __attribute__ ((packed));


#define HCI_REJECT_SYNC_CONN              0x042A
struct hci_cp_reject_sync_conn
{
    struct bdaddr_t bdaddr;
    uint8     reason;
} __attribute__ ((packed));

#define HCI_IO_CAPABILIRY_RESPONSE           0x042B
struct hci_cp_io_capability_request_reply
{
    struct bdaddr_t bdaddr;
    uint8  io_caps;
    uint8  oob_present;
    uint8  auth_requirements;
}__attribute__ ((packed));

#define HCI_USER_PASSKEY_REQUEST_REPLY   0x042E
struct hci_cp_user_passkey_request_reply {
    struct bdaddr_t bdaddr;
    uint32_t passkey;
} __attribute__ ((packed));

#define HCI_USER_PASSKEY_REQUEST_NEG_REPLY  0x042F
struct hci_cp_user_passkey_request_neg_reply {
    struct bdaddr_t bdaddr;
} __attribute__ ((packed));

#define HCI_SEND_KEYPRESS_NOTIFY     0x0C60
struct hci_cp_send_keypress_notify {
    struct bdaddr_t bdaddr;
    uint8_t  type;
} __attribute__ ((packed));

#define HCI_ENHANCED_SETUP_SYNC_CONN           0x043d
struct hci_cp_enhanced_setup_sync_conn
{
    uint16 conn_handle;
    uint32 tx_bandwidth;
    uint32 rx_bandwidth;
    uint8 tx_coding_format_type;
    uint16 tx_coding_format_vendor;
    uint16 tx_coding_format_id;
    uint8 rx_coding_format_type;
    uint16 rx_coding_format_vendor;
    uint16 rx_coding_format_id;
    uint16 tx_codec_frame_size;
    uint16 rx_codec_frame_size;
    uint32 input_bandwidth;
    uint32 output_bandwidth;
    uint8 input_coding_format_type;
    uint16 input_coding_format_vendor;
    uint16 input_coding_format_id;
    uint8 output_coding_format_type;
    uint16 output_coding_format_vendor;
    uint16 output_coding_format_id;
    uint16 intput_coded_data_size;
    uint16 output_coded_data_size;
    uint8 input_pcm_data_format;
    uint8 output_pcm_data_format;
    uint8 input_pcm_sample_payload_msb_position;
    uint8 output_pcm_sample_payload_msb_position;
    uint8 intput_data_path;
    uint8 output_data_path;
    uint8 input_transport_unit_size;
    uint8 output_transport_unit_size;
    uint16 max_latency;
    uint16 packet_type;
    uint8 retx_effort;
} __attribute__ ((packed));

#define HCI_IO_CAPABILIRY_NEGATIVE_REPLY          0x0434
struct hci_cp_io_capability_negative_reply
{
    struct bdaddr_t bdaddr;
    uint8  reason;
}__attribute__ ((packed));

#define HCI_USER_CONFIRMATION_REPLY    0x042C
#define HCI_USER_CONFIRMATION_NEG_REPLY    0x042D
struct hci_cp_usr_confirmation_reply
{
    struct bdaddr_t bdaddr;
}__attribute__ ((packed));

#define HCI_SNIFF_MODE                            0x0803
struct hci_cp_sniff_mode
{
    uint16 conn_handle;
    uint16 sniff_max_interval;
    uint16 sniff_min_interval;
    uint16 sniff_attempt;
    uint16 sniff_timeout;
} __attribute__ ((packed));

#define HCI_EXIT_SNIFF_MODE                       0x0804
struct hci_cp_exit_sniff_mode
{
    uint16 conn_handle;
} __attribute__ ((packed));

#define HCI_QOS_SETUP                             0x0807
struct hci_qos_setup
{
    uint16 conn_handle;
    uint8  unused;
    uint8  service_type;
    uint32 token_rate;
    uint32 peak_bandwidth;
    uint32 latency;
    uint32 delay_variation;
} __attribute__ ((packed));

#define HCI_ROLE_DISCOVERY                        0x0809
struct hci_write_role_discovery
{
    uint16 conn_handle;
} __attribute__ ((packed));

#define HCI_SWITCH_ROLE                       0x080B
struct hci_witch_role
{
    struct bdaddr_t bdaddr;
    uint8           role;
} __attribute__ ((packed));

#define HCI_WRITE_LP_SETTINGS                     0x080D
struct hci_cp_write_link_policy
{
    uint16 conn_handle;
    uint16 link_policy_settings;
} __attribute__ ((packed));

#define HCI_SNIFF_SUBRATING             0x0811
struct hci_cp_sniff_subrating
{
    uint16 conn_handle;
    uint16 maximum_lantency;
    uint16 minimum_remote_timeout;
    uint16 minimum_local_timeout;
} __attribute__ ((packed));

#define HCI_SET_FILTER_TYPE_CLEAR   0x00
#define HCI_SET_FILTER_TYPE_INQUIRY 0x01
#define HCI_SET_FILTER_TYPE_CONNECT 0x02

#define HCI_SET_FILTER_COND_ALL     0x00
#define HCI_SET_FILTER_COND_COD     0x01
#define HCI_SET_FILTER_COND_ADDR    0x02

#define HCI_SET_FILTER_DONT_AUTO_CONNECT 0x00
#define HCI_SET_FILTER_AUTO_ACCEPT_DISABLE_MSS 0x01
#define HCI_SET_FILTER_AUTO_ACCEPT_ENABLE_MSS 0x02

struct hci_cp_set_clear_all_filters
{
    uint8 filter_type; // 0x00 clear all, 0x01 inquiry result, 0x02 connection setup
} __attribute__ ((packed));

struct hci_cp_set_inquiry_no_filter
{
    uint8 filter_type; // 0x00 clear all, 0x01 inquiry result, 0x02 connection setup
    uint8 filter_condition_type; // 0x00 return all responses, 0x01 specific cod, 0x02 specific address
} __attribute__ ((packed));

struct hci_cp_set_inquiry_cod_filter
{
    uint8 filter_type; // 0x00 clear all, 0x01 inquiry result, 0x02 connection setup
    uint8 filter_condition_type; // 0x00 return all responses, 0x01 specific cod, 0x02 specific address
    uint8 class_of_device[3]; // zeros, return all; non-zero interested
    uint8 class_of_device_mask[3]; // which bits in 'class_of_device' parameter are 'dont care', zero-bit means 'dont care'
} __attribute__ ((packed));

struct hci_cp_set_inquiry_address_filter
{
    uint8 filter_type; // 0x00 clear all, 0x01 inquiry result, 0x02 connection setup
    uint8 filter_condition_type; // 0x00 return all responses, 0x01 specific cod, 0x02 specific address
    uint8 bd_addr[6]; // interest address
} __attribute__ ((packed));

struct hci_cp_set_connect_no_filter
{
    uint8 filter_type; // 0x00 clear all, 0x01 inquiry result, 0x02 connection setup
    uint8 filter_condition_type; // 0x00 return all responses, 0x01 specific cod, 0x02 specific address
    uint8 auto_accept_flag; // 0x00 dont auto accept, 0x01 auto accept and disable role switch, 0x02 auto accept and enable role switch
} __attribute__ ((packed));

struct hci_cp_set_connect_cod_filter
{
    uint8 filter_type; // 0x00 clear all, 0x01 inquiry result, 0x02 connection setup
    uint8 filter_condition_type; // 0x00 return all responses, 0x01 specific cod, 0x02 specific address
    uint8 class_of_device[3]; // zeros, return all; non-zero interested
    uint8 class_of_device_mask[3]; // use zero to indicate which bits in 'class_of_device' are 'dont care'
    uint8 auto_accept_flag; // 0x00 dont auto accept, 0x01 auto accept and disable role switch, 0x02 auto accept and enable role switch
} __attribute__ ((packed));

struct hci_cp_set_connect_address_filter
{
    uint8 filter_type; // 0x00 clear all, 0x01 inquiry result, 0x02 connection setup
    uint8 filter_condition_type; // 0x00 return all responses, 0x01 specific cod, 0x02 specific address
    uint8 bd_addr[6]; // address interested
    uint8 auto_accept_flag; // 0x00 dont auto accept, 0x01 auto accept and disable role switch, 0x02 auto accept and enable role switch
} __attribute__ ((packed));

struct hci_cp_write_automatic_flush_timeout
{
    uint16   connhandle;
    uint16   flush_timeout;
} __attribute__ ((packed));

#define HCI_READ_STORED_LINK_KEY              0x0C0D
struct hci_cp_read_stored_linkkey
{
    struct bdaddr_t bdaddr;
    uint8 read_all_flag;
} __attribute__ ((packed));

#define HCI_WRITE_STORED_LINK_KEY             0x0C11
#define HCI_HS_STORED_LINK_KEY                0x201C //
struct hci_cp_write_stored_linkkey
{
    uint8 num_keys_to_write;
    struct bdaddr_t bdaddr;
    uint8    link_key[16];
} __attribute__ ((packed));

#define HCI_DELETE_STORED_LINK_KEY            0x0C12
struct hci_cp_delete_stored_linkkey
{
    struct bdaddr_t bdaddr;
    uint8  delete_all_flag;
} __attribute__ ((packed));

#define HCI_WRITE_LOCAL_NAME                  0x0C13
struct hci_cp_write_local_name
{
    uint8    local_name[BTM_NAME_MAX_LEN];
} __attribute__ ((packed));

#define HCI_WRITE_SCAN_ENABLE                 0x0C1A
struct hci_cp_write_scan_enable
{
    uint8     scan_enable;
} __attribute__ ((packed));

#define HCI_WRITE_AUTH_ENABLE                 0x0C20
struct hci_cp_write_auth_enable
{
    uint8 enable_flag;
} __attribute__ ((packed));

#define HCI_WRITE_CLASS_OF_DEVICE             0x0C24
struct hci_cp_write_class_of_device
{
    uint8    class_dev[3];
} __attribute__ ((packed));

#define HCI_WRITE_LINK_SUPERV_TIMEOUT         0x0C37
struct hci_write_superv_timeout
{
    uint16   connhandle;
    uint16   superv_timeout;
} __attribute__ ((packed));

#define HCI_WRITE_CURRENT_IAC_LAP        0x0C3A
struct hci_write_current_iac_lap
{
    uint8  iac_lap[4];
} __attribute__ ((packed));

#define HCI_WRITE_INQ_MODE              0x0C45
struct hci_write_inqmode
{
    uint8  inqmode;
} __attribute__ ((packed));

#define HCI_WRITE_PAGESCAN_TYPE          0x0C47
struct hci_write_pagescan_type
{
    uint8  pagescan_type;
} __attribute__ ((packed));

#define SET_AFH_HOST_CHL_CLASSFICAT           0x0C3F
struct hci_set_afh_channel_classification
{
    uint8  map[10];
} __attribute__ ((packed));

#define HCI_WRITE_INQSCAN_TYPE           0x0C43
struct hci_write_inqscan_type
{
    uint8  inqscan_type;
} __attribute__ ((packed));

#define HCI_WRITE_SIMPLE_PAIRING_MODE         0x0C56
struct hci_cp_write_simple_paring_mode
{
    uint8     simple_paring_mode;
} __attribute__ ((packed));

// HCI_WRITE_EXTENDED_INQ_RESP
struct hci_write_extended_inquiry_response
{
    uint8  fec;
    uint8  eir[240];
} __attribute__ ((packed));

#define HCI_READ_BUFFER_SIZE                  0x1005
struct hci_read_buffer_size_cmpl
{
    uint8_t status;
    uint16_t acl_data_packet_length;
    uint8_t sco_data_packet_length;
    uint16_t total_num_acl_data_packets;
    uint16_t total_num_sco_data_packets;
} __attribute__ ((packed));

#define HCI_READ_DATA_BLOCK_SIZE              0x100A
struct hci_read_data_block_size_cmpl
{
    uint8_t status;
    uint16_t max_acl_data_packet_length; // max length of the data portion of an HCI ACL data packet the btc can accept
    uint16_t data_block_length; // max length of the data portion of each HCI ACL data packet the btc can hold
    uint16_t total_num_data_blocks; // total number of data block buffers available in the btc
} __attribute__ ((packed));

#define HCI_READ_AVG_RSSI                    0x1405
struct hci_read_rssi_avg_rssi
{
    uint16 conn_handle;
}__attribute__ ((packed));

#define HCI_SET_BD_ADDR                  0xFC72
struct hci_cp_set_bdaddr
{
    uint8 addr_type; /* 0 bt, 1 ble */
    struct bdaddr_t bdaddr;
} __attribute__ ((packed));

#define HCI_WRITE_MEMORY                  0xFC02
struct hci_cp_write_memory
{
    uint32 address;
    uint8  type;
    uint8  length;
    uint32 value;
} __attribute__ ((packed));

#define HCI_READ_MEMORY                  0xFC01
struct hci_cp_read_memory
{
    uint32 address;
    uint8  type;
    uint8  length;
} __attribute__ ((packed));

#define HCI_SET_LOCAL_CLOCK                  0xFC0F
struct hci_set_local_clock
{
    uint32 clock;
} __attribute__ ((packed));

#define HCC_DBG_WRITE_SLEEP_EXWAKEUP_EN  0xFC77

//set sco path
#define HCI_DBG_SET_SYNC_CONFIG_CMD_OPCODE            0xFC8F
#define HCC_DBG_SET_SCO_SWITCH           0xFC89

#define HCI_DBG_SET_SYNC_CONFIG_CMD_OPCODE_OLD_VER            0xFC51
#define HCC_DBG_SET_SCO_SWITCH_OLD_VER           0xFC62

struct hci_set_switch_sco
{
    uint16  sco_handle;
} __attribute__ ((packed));

#define HCI_DBG_SET_SNIFFER_ENV_CMD_OPCODE    0xFC8E
struct hci_set_sniffer_env
{
    uint8 sniffer_active;
    uint8 sniffer_role;
    struct bdaddr_t monitor_bdaddr;
    struct bdaddr_t sniffer_bdaddr;
} __attribute__ ((packed));

#define HCI_DBG_SCO_TX_SILENCE_CMD_OPCODE     0xFC94
struct hci_dbg_sco_tx_silence_cmd
{
    uint16   connhandle;
    uint8    silence_on;
} __attribute__ ((packed));

#define HCI_DBG_SNIFFER_CMD_OPCODE            0xFC95
struct hci_dbg_sniffer_interface
{
    uint16   connhandle;
    uint8    subcode;
} __attribute__ ((packed));

#define HCI_LOWLAYER_MONITOR                     0xFC9b
struct hci_cp_lowlayer_monitor
{
    uint16 conn_handle;
    uint8 control_flag;
    uint8 report_format;
    uint32 data_format;
    uint8 report_unit;
} __attribute__ ((packed));

#define HCI_DBG_ENABLE_FASTACK_CMD_OPCODE     0xFCA1
struct hci_enable_fastack
{
    uint16 conn_handle;
    uint8 direction;
    uint8 enable;
} __attribute__ ((packed));

#define HCI_DBG_SEND_PREFER_RATE_CMD_OPCODE 0xFCA9
struct hci_dbg_send_prefer_rate
{
    uint16 conn_handle;
    uint8 rate;
} __attribute__ ((packed));

#define HCI_DBG_SET_TXPWR_LINK_THD_OPCODE 0xFCB4
struct hci_dbg_set_txpwr_link_thd
{
    ///array index.if index is 0xff,reset all index data
    uint8_t     index;
    ///index enable or disable
    uint8_t     enable;
    uint8_t     link_id;
    uint16_t    rssi_avg_nb_pkt;
    int8_t      rssi_high_thr;
    int8_t      rssi_low_thr;
    int8_t      rssi_below_low_thr;
    int8_t      rssi_interf_thr;
} __attribute__ ((packed));

#define HCI_DBG_SET_ISO_QUALITY_REPORT_THR  0xFCDC
struct hci_dbg_iso_quality_report
{
    uint16 conn_handle;
    uint16 qlty_rep_evt_cnt_thr;
    uint16 tx_unack_pkts_thr;
    uint16 tx_flush_pkts_thr;
    uint16 tx_last_subevent_pkts_thr;
    uint16 retrans_pkts_thr;
    uint16 crc_err_pkts_thr;
    uint16 rx_unreceived_pkts_thr;
    uint16 duplicate_pkts_thr;
} __attribute__ ((packed));

#define HCI_DBG_LE_TX_POWER_REQUEST_CMD_OPCODE  0xfcde
struct hci_dbg_le_tx_power_request
{
    uint16_t conhdl;
    // Enable/Disable LE tx power request (0: disable / 1: enable)
    uint8_t  enable;
    /// Delta in power control required (dBm)
    int8_t   delta;
    /// The rate on which delta required (@see enum btif_le_phy_rate)
    uint8_t  rx_rate;
}__attribute__ ((packed));

#define HCI_TRC_LE_LLCP_OPCODE_ENABLED_CMD 0x00000400
#define HCI_ENABLE_BLE_DBG_TRC_REPORT_CMD  0xFC90

struct hci_ble_audio_dbg_trc_enable_cmd
{
    uint32_t trc_enable_opcode;
} __attribute__ ((packed));

#define HCI_DBG_SET_BT_BLE_ACTIVE_LINK_CMD_OPCODE   0xFCDD

struct hci_dbg_set_bt_ble_active_link_struct
{
    uint8_t traffic_mode;
    uint16_t  link_handle;
} __attribute__((packed));

#define HCI_LE_READ_BUFFER_SIZE             0x2002
#define HCI_LE_READ_BUFFER_SIZE_V2          0x2060
struct hci_le_read_buffer_size_cmpl
{
    uint8_t status;
    uint16_t le_acl_data_packet_length;
    uint8_t total_num_le_acl_data_packets;
    uint16_t iso_data_packet_length;
    uint8_t total_num_iso_data_packets;
} __attribute__ ((packed));

#define HCC_WRITE_RANDOM_ADDR               0x2005
struct hci_write_random_addr
{
    struct bdaddr_t bdaddr;
} __attribute__ ((packed));

#define HCC_WRITE_ADV_PARAMETER             0x2006
struct hci_write_adv_param
{
    uint16  interval_min;
    uint16  interval_max;
    uint8   adv_type;
    uint8   own_addr_type;
    uint8   peer_addr_type;
    struct bdaddr_t bdaddr;
    uint8   adv_chanmap;
    uint8   adv_filter_policy;
} __attribute__ ((packed));

#define HCC_WRITE_ADV_DATA                  0x2008
struct hci_write_adv_data
{
    uint8 len;
    uint8 data[BLE_ADV_DATA_LENGTH];
} __attribute__ ((packed));

#define HCC_WRITE_SCAN_RSP_DATA             0x2009
struct hci_write_scan_rsp_data
{
    uint8 len;
    uint8 data[BLE_SCAN_RSP_DATA_LENGTH];
} __attribute__ ((packed));

#define HCC_WRITE_ADV_ENABLE                0x200A
struct hci_write_adv_en
{
    uint8 en;
} __attribute__ ((packed));

#define HCC_WRITE_BLE_SCAN_PARAMETER        0x200B
struct hci_write_ble_scan_param
{
    uint8   scan_type;
    uint16  scan_interval;
    uint16  scan_window;
    uint8   own_addr_type;
    uint8   scan_filter_policy;
} __attribute__ ((packed));

#define HCC_WRITE_BLE_SCAN_ENABLE           0x200C
struct hci_write_ble_can_en
{
    uint8 scan_en;
    uint8 filter_duplicat;
} __attribute__ ((packed));

#define HCC_CLEAR_WHITE_LIST                0x2010
#define HCC_ADD_DEVICE_TO_WHITELIST         0x2011
struct hci_add_device_to_wl
{
    uint8 addr_type;
    struct bdaddr_t bdaddr;
} __attribute__ ((packed));

#define HCI_LE_SET_HOST_CHL_CLASSIFICATION  0x2014
struct hci_set_le_channel_classification
{
    uint8  map[5];
} __attribute__ ((packed));

#define HCI_LE_READ_CHANNEL_MAP             0x2015
struct hci_le_read_chnl_map
{
    uint16 conn_handle;
} __attribute__ ((packed));

#define HCI_LE_READ_LOCAL_SUPP_FEATURES 0x2003
#define HCI_LE_READ_REMOTE_FEATURES 0x2016
#define HCI_LE_READ_LOCAL_SUPP_STATES 0x201C
#define HCI_LE_READ_ALL_LOCAL_SUPP_FEATURES 0x2087 // Core Spec 6.0
#define HCI_LE_READ_ALL_REMOTE_FEATURES 0x2088 // Core Spec 6.0
#define HCI_LE_SET_HOST_FEATURE_V1 0x2074
#define HCI_LE_SET_HOST_FEATURE_V2 0x2097 // Core Spec 6.0
#define HCI_LE_EV_READ_REMOTE_FEATS_CMPL 0x04
#define HCI_LE_EV_READ_ALL_REMOTE_FEATS_CMPL 0x2B
#define HCI_LE_FEATURE_PAGES_MAX_NUM 0x0A
struct hci_le_read_local_supp_features_cmpl {
    uint8_t status;
    uint8_t le_features[8];
} __attribute__ ((packed));
struct hci_le_read_local_supp_states_cmpl {
    uint8_t status;
    uint8_t le_states[8]; // only provide info about the supp states that can be used with legacy advertising
} __attribute__ ((packed));
struct hci_le_read_all_local_supp_features_cmpl {
    uint8_t status;
    // The number of the highest-numbered page of the supported LE features that
    // contains at least one bit set to 1. Ranges 0x00 to 0x0A
    uint8_t max_pages;
    // Bit Mask List of the supported LE features, 248 octets.
    uint8_t le_features[248];
} __attribute__ ((packed));
struct hci_le_read_remote_features {
    uint16_t conn_handle;
} __attribute__ ((packed));
struct hci_ev_le_read_remote_feats_cmpl {
    uint8_t subcode;
    uint8_t status;
    uint16_t conn_handle;
    uint8_t le_features[8];
} __attribute__ ((packed));
struct hci_le_read_all_remote_features {
    uint16_t conn_handle;
    // The number of the highest-numbered page of features that the Host requires
    // and the Controller shall obtain. Ranges 0x00 to 0x0A
    uint8_t pages_requested;
} __attribute__ ((packed));
struct hci_ev_le_read_all_remote_feats_cmpl {
    uint8_t subcode;
    uint8_t status;
    uint16_t conn_handle;
    // The number of the highest-numbered page of the remote device’s supported
    // LE features that contains at least one bit set to 1. Range: 0x00 to 0x0A
    uint8_t max_remote_page;
    // The number of the highest-numbered page of LE_Features that contains
    // valid data. Range: 0x00 to 0x0A
    uint8_t max_valid_page;
    // Bit Mask List of the supported LE features, 248 octets.
    uint8_t le_features[248];
} __attribute__ ((packed));
struct hci_le_set_host_feature_v1 {
    uint8_t bit_number; // 0x00 to 0x3F, bit position in the FeatureSet
    uint8_t bit_value; // 0x00 disabled, 0x01 enabled
} __attribute__ ((packed));
struct hci_le_set_host_feature_v2 {
    uint16_t bit_number; // 0x0000 to 0x07BF, bit position in the FeatureSet
    uint8_t bit_value; // 0x00 disabled, 0x01 enabled
} __attribute__ ((packed));
struct hci_le_set_host_feature_cmpl {
    uint8_t status;
} __attribute__ ((packed));

#define HCI_REMOTE_VERSION_INFO 0x041D
#define HCI_EV_READ_REMOTE_VERSION 0x0C
struct hci_cp_remote_version_info {
    uint16_t conn_handle; // acl-u or le-u conn handle
} __attribute__ ((packed));
struct hci_ev_read_remote_version {
    uint8_t status;
    uint16_t conn_handle;
    uint8_t lmp_version;
    uint16_t company_id;
    uint16_t lmp_subversion;
} __attribute__ ((packed));
struct hci_cmd_complete_param {
    uint8_t status;
} __attribute__ ((packed));

/**
 * HCI_LE_Set_Random_Address is used by the Host to set the LE Random Device
 * Address in the Controller. If this command is used to change the address,
 * the new random address shall take effect for advertising no later than the
 * next succssfully HCI_LE_Set_Advertising_Enable, for scanning no later than
 * the next successful HCI_LE_Set_Scan_Enable or HCI_LE_Set_Extended_Scan_Enable,
 * and for initiating no later than the next successfull HCI_LE_Create_Connection
 * or HCI_LE_Extended_Create_Connection.
 *
 * Note: If the extended advertising commands are in use, this command only
 * affects the address used for scanning and initiating. The addresses used for
 * adveritising are set by the HCI_LE_Set_Advertising_Set_Random_Address.
 *
 * If the Host issues this command when any of advertising (creted using legacy
 * advertising comands), scanning, or initiating are enabled, the Controller shall
 * return the error code Command Disabllowed.
 *
 * The HCI_LE_Set_Advertising_Set_Random_Address is used by the Host to set the
 * random device address. This address is used in the Controller for the advertiser's
 * address contained in the advertising PDUs for the advertising set specificed by
 * the Advertising_Handle parameter.
 *
 * This command may be issued at any time after an advertising set identified by
 * the Advertising_Handle has been created using the HCI_LE_Set_Extended_Advertising_Parameters.
 * However, if the Host issues this command while the advertising set identified by
 * the Advertising_Handle is using connectable advertising and is enabled, the Controller
 * shall return Command Disablled.
 *
 * If this command is used to change the address, the new random address shall take effect
 * for advertising no later than the next successful HCI_LE_Set_Extended_Advertising_Enable
 * and for periodic advertising no later than the next successful HCI_LE_Periodic_Advertising_Enable.
 *
 */
#define HCI_LE_SET_RANDOM_ADDRESS 0x2005
#define HCI_LE_SET_ADV_SET_RANDOM_ADDRESS 0x2035
struct hci_le_set_random_address {
    uint8_t random_address[6];
} __attribute__ ((packed));
struct hci_le_set_adv_set_random_address {
    uint8_t adv_handle; // identify an advertising set
    uint8_t random_address[6];
} __attribute__ ((packed));
struct hci_le_set_random_address_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_set_adv_set_random_address_cmpl {
    uint8_t status;
} __attribute__ ((packed));

/**
 * The HCI_LE_Set_Extended_Advertising_Parameters is used by the Host to set the advertising parameters.
 * The Advertising_Handle identifies the advertising set whose parameters are being configured. If the
 * Advertising_Handle does not identify an existing advertising set and the Controller is unable to support
 * a new advertising set at present, the Controller shall return the error code Memory Capacity Exceeded (0x07).
 *
 * The Advertising_Event_Properties describes the type of advertising event that is being configured and
 * its basic properties. The type shall be one supported by the Controller. In particular, the following
 * restrictions apply to this parameter:
 *      If legcay advertising PDU types are being used, then the parameter value shall be one of those
 *      specified below. If the advertising set already contains data, the type shall be one that supports
 *      advertising data and the amount of data shall not exceed 31 octets.
 *              ADV_IND (conn and scan undirect)                0x13 has advertising data
 *              ADV_DIRECT_IND (conn direct low duty)           0x15 no advertising data
 *              ADV_DIRECT_IND (conn direct high duty)          0x1d no advertising data
 *              ADV_SCAN_ID (scan undirected)                   0x12 has advertising data
 *              ADV_NONCONN_IND (nonconn nonscan undirected)    0x10 has advertising data
 *      If extended advertising PDU types are being used then:
 *              The advertisement shall not be both connectable and scannable;
 *              High duty cycle directed connectable advertising shall not be used;
 *              The parameters beginning with "Secondary" are only valid when extended advertising PDU types are being used
 *
 * Advertising_Handles are assigned by the Host when a new advertising set is created by using the
 * LE Set Extended Advertising Parameters command.
 *
 */
#define HCI_LE_SET_ADV_PARAMETERS 0x2006
#define HCI_LE_SET_EXT_ADV_PARAMS_V1 0x2036
#define HCI_LE_SET_EXT_ADV_PARAMS_V2 0x207F
#define HCI_LE_LEGACY_ADV_IND 0x00
#define HCI_LE_LEGACY_ADV_DIRECT_HIGH_DUTY 0x01
#define HCI_LE_LEGACY_ADV_SCAN_IND 0x02
#define HCI_LE_LEGACY_ADV_NONCONN_IND 0x03
#define HCI_LE_LEGACY_ADV_DIRECT_LOW_DUTY 0x04
#define HCI_LE_CONN_ADVERTISING                 0x01
#define HCI_LE_SCAN_ADVERTISING                 0x02
#define HCI_LE_DIRECTED_ADVERTISING             0x04
#define HCI_LE_HIGH_DUTY_DIRECTED_CONN_ADV      0x08 // <=3.75ms adv interval
#define HCI_LE_USE_LEGACY_ADV_PDU               0x10
#define HCI_LE_OMIT_ADV_ADDRESS_FROM_ALL_PDU    0x20
#define HCI_LE_INCL_TX_POWER_AT_LEAST_ONE_ADV   0x40
// Below are Core spec 6.0 features
#define HCI_LE_USE_DECISIONS_PDU                0x80
#define HCI_LE_INCL_ADVA_IN_DECISONS_PDU        0x0100
#define HCI_LE_INCL_ADI_IN_DECISONS_PDU         0x0200
struct hci_le_set_adv_parameters {
    uint16_t adv_interval_min; // min Tadv for undirected and low duty cycle directed advertising
    uint16_t adv_interval_max; // max Tadv, 0x000020 to 0x4000 * 0.625ms, 20ms to 10.24s, default 0x0800 1.28s
    uint8_t adv_type; // 0x00 ADV_IND, 0x01 ADV_DIRECT_IND high duty, 0x02 ADV_SCAN_IND, 0x03 ADV_NONCONN_IND, 0x04 ADV_DIRECT_IND low duty
    uint8_t own_addr_type; // 0x00 0x01, public or random address, 0x02 0x03, btc gen RPA based on local irk from the resolving list, if no match entry use public address or random address from 0x2005
    uint8_t peer_addr_type; // 0x00 public device address or public ia, 0x01 random device address or random (static) ia
    uint8_t peer_addr[6];
    uint8_t adv_channel_map; // bit 0 channel 37 shall be used, bit 1 38, bit 2 39 (default is 0x07)
    uint8_t adv_filter_policy; // 0x00 accept all req, 0x01 all conn_req only scan_req in list, 0x02 all scan_req only conn_req in list, 0x03 all req in list
} __attribute__ ((packed));
struct hci_le_set_adv_params_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_set_ext_adv_params_v1 {
    uint8_t adv_handle; // 0x00 to 0xEF
    uint16_t adv_event_props;
    uint8_t pri_adv_interval_min[3]; // min Tadv for undirected and low duty cycle directed advertising
    uint8_t pri_adv_interval_max[3]; // max Tadv, 0x000020 to 0xFFFFFF * 0.625ms, 20ms to 10485.759375s
    uint8_t pri_adv_channel_map; // bit 0 channel 37 shall be used, bit 1 38, bit 2 39
    uint8_t own_addr_type; // 0x00 0x01, public or random address, 0x02 0x03, btc gen RPA based on local irk from the resolving list, if no match entry use public address or random address from 0x2035
    uint8_t peer_addr_type; // 0x00 public device address or public ia, 0x01 random device address or random (static) ia
    uint8_t peer_addr[6];
    uint8_t adv_filter_policy; // 0x00 accept all req, 0x01 all conn_req only scan_req in list, 0x02 all scan_req only conn_req in list, 0x03 all req in list
    int8_t adv_tx_power; // 0xXX range -127dBm to 20dBm, 0x7F host has no preference
    uint8_t pri_adv_phy; // 0x01 primary advertisement phy is LE 1M, 0x03 LE Coded
    uint8_t sec_adv_max_skip; // 0x00 AUX_ADV_IND shall be sent prior to the next adv event, 0x01 to 0xFF max adv events the btc can skip before sending AUX_ADV_IND on the sec adv physical channel
    uint8_t sec_adv_phy; // 0x01 secondary advertisement phy is LE 1M, 0x02 LE 2M, 0x03 LE Coded
    uint8_t adv_sid; // 0x00 to 0x0F value of the advertising SID subfield in the ADI field of the PDU
    uint8_t scan_req_notify_enable; // 0x00 disable, 0x01 enable
} __attribute__ ((packed));
struct hci_le_set_ext_adv_params_v2 {
    uint8_t adv_handle; // 0x00 to 0xEF
    uint16_t adv_event_props;
    uint8_t pri_adv_interval_min[3]; // min Tadv for undirected and low duty cycle directed advertising
    uint8_t pri_adv_interval_max[3]; // max Tadv, 0x000020 to 0xFFFFFF * 0.625ms, 20ms to 10485.759375s
    uint8_t pri_adv_channel_map; // bit 0 channel 37 shall be used, bit 1 38, bit 2 39
    uint8_t own_addr_type; // 0x00 0x01, public or random address, 0x02 0x03, btc gen RPA based on local irk from the resolving list, if no match entry use public address or random address from 0x2035
    uint8_t peer_addr_type; // 0x00 public device address or public ia, 0x01 random device address or random (static) ia
    uint8_t peer_addr[6];
    uint8_t adv_filter_policy; // 0x00 accept all req, 0x01 all conn_req only scan_req in list, 0x02 all scan_req only conn_req in list, 0x03 all req in list
    int8_t adv_tx_power; // 0xXX range -127dBm to 20dBm, 0x7F host has no preference
    uint8_t pri_adv_phy; // 0x01 primary advertisement phy is LE 1M, 0x03 LE Coded
    uint8_t sec_adv_max_skip; // 0x00 AUX_ADV_IND shall be sent prior to the next adv event, 0x01 to 0xFF max adv events the btc can skip before sending AUX_ADV_IND on the sec adv physical channel
    uint8_t sec_adv_phy; // 0x01 secondary advertisement phy is LE 1M, 0x02 LE 2M, 0x03 LE Coded
    uint8_t adv_sid; // 0x00 to 0x0F value of the advertising SID subfield in the ADI field of the PDU
    uint8_t scan_req_notify_enable; // 0x00 disable, 0x01 enable
    uint8_t pri_adv_phy_options; // 0x00 host has no preferred or required coding when tx on LE Coded PHY, 0x01 0x03 host prefers/requires S=2 coding be used, 0x02 0x04 prefer/require S=8 coding
    uint8_t sec_adv_phy_options; // 0x00 host has no preferred or required coding when tx on LE Coded PHY, 0x01 0x03 S=2 coding, 0x02 0x04 S=8 coding
} __attribute__ ((packed));
struct hci_le_set_ext_adv_params_cmpl {
    uint8_t status;
    int8_t selected_tx_power;
} __attribute__ ((packed));

/**
 * The HCI_LE_Set_Advertising_Data is used to set the data used in advertising packets that have a data field.
 * If advertising is currently enabled, the Controller shall use the new data in subsequent advertising events.
 * If an advertising evet is in progress when this command is issued, the Controller may use the old or new data
 * for that event. If advertising is currently disable, the data shall be kept by the Controller and used once
 * advertising is enabled. The default Advertising_Data_Length shall be 0 and the default Advertising_Data shall
 * be 31 zero octets.
 *
 * The HCI_LE_Set_Extended_Advertising_Data is used to set the data used in advertising PDUs that have a data
 * field. It may be issued at any time after an advertising set identified by the Advertising_Handle has been
 * created using the HCI_LE_Set_Extended_Advertising_Parameters, regardleass of whether advertising in that set
 * is enabled or disable.
 *
 * The Host may set the advertising data in one or more operations using the Operation parameters. If the combined
 * length of the data exceeds the capacity of the advertising set (sett LE_Read_Max_Adv_Data_Length) or the amount
 * of memory currently available, all the data shall be discarded and the Controller shall return the error code
 * Memory Capacity Exceeded (0x07).
 *
 * If Operation indicates the start of new data (0x01 or 0x02), then any existing partial or complete adv data
 * shall be discared. If the advertising data is discarded by the command or the combined length of the data
 * after the commmand completes is zero, the advertising set will have no adv data.
 *
 * If the advertising set uses legacy advertising PDUs that support adverting data and either Operation is not
 * 0x03 or the adv data length exceeds 31 octets, the controller shall return Invalid HCI Command Parameters.
 *
 * If advertising is currently enabled for the specified advertising set and Operation does not have the value
 * 0x03 or 0x04, the Controller shall return Command Disallowed (0x0C).
 *
 */
#define HCI_LE_SET_ADV_DATA 0x2008
#define HCI_LE_SET_SCAN_RSP_DATA 0x2009
#define HCI_LE_SET_EXT_ADV_DATA 0x2037
#define HCI_LE_SET_EXT_SCAN_RSP_DATA 0x2038
#define HCI_LE_EXT_ADV_DATA_INTERM_FRAG 0x00
#define HCI_LE_EXT_ADV_DATA_FIRST_FRAG 0x01
#define HCI_LE_EXT_ADV_DATA_LAST_FRAG 0x02
#define HCI_LE_COMPLETE_EXT_ADV_DATA 0x03
#define HCI_LE_EXT_ADV_DATA_UNCANGED 0x04 // just update the Adv DID, not used for scan rsp data
#define HCI_LE_BTC_FRAG_ALL_HOST_ADV_DATA 0x00
#define HCI_LE_BTC_SHOULD_NOT_FRAG_ADV_DATA 0x01
struct hci_le_set_adv_data {
    uint8_t adv_data_len;
    uint8_t adv_data[31];
} __attribute__ ((packed));
struct hci_le_set_adv_data_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_set_scan_rsp_data {
    uint8_t scan_rsp_data_len;
    uint8_t scan_rsp_data[31];
} __attribute__ ((packed));
struct hci_le_set_scan_rsp_data_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_set_ext_adv_data {
    uint8_t adv_handle; // 0x00 to 0xEF
    uint8_t operation;
    uint8_t frag_prefer; // 0x00 btc may freg all host adv data, 0x01 btc should not frag or should minimize fragmentation of host adv data
    uint8_t adv_data_len; // 0 to 251
} __attribute__ ((packed));
struct hci_le_set_ext_adv_data_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_set_ext_scan_rsp_data {
    uint8_t adv_handle; // 0x00 to 0xEF
    uint8_t operation;
    uint8_t frag_prefer; // 0x00 btc may freg all host adv data, 0x01 btc should not frag or should minimize fragmentation of host adv data
    uint8_t scan_rsp_data_len; // 0 to 251
} __attribute__ ((packed));
struct hci_le_set_ext_scan_rsp_data_cmpl {
    uint8_t status;
} __attribute__ ((packed));

/**
 * The HCI_LE_Set_Decision_Data command is used to set the decision data used
 * by the specified advertising set in those advertising PDUs that contain decision
 * data. This command may be issued at any time after an advertising set identified
 * by the Advertising_Handle parameter has been created, regardless of whether
 * advertising in that set is enabled or disabled. When an advertising set is created,
 * Decision_Type_Flags shall be set to zero and there shall be zero octets of decision
 * data.
 *
 * The HCI_LE_Set_Decision_Instructions command is used to set the decision
 * instructions used when listening for advertisements containing decision PDUs (when
 * either scanning or initiating a new connection). The Controller shall support at least 8
 * tests in the decision instructions.
 * This command may be used while scanning or initiating is in progress. If so, the new
 * decision instructions shall take effect before the HCI_Command_Complete event is sent
 * to the Host and the change shall not take place while the decision instructions are being
 * used to process a decision PDU (that is, each PDU is processed using either the old
 * decision instructions or the new ones, but not a mixture).
*/
#define HCI_LE_SET_DECISION_DATA 0x2080
#define HCI_LE_SET_DECISION_INSTRUCTIONS 0x2081
struct hci_le_set_decision_data {
    uint8_t adv_handle; // 0x00 to 0xEF
    uint8_t decision_type_flags; // 0b01 A Resolvable Tag subfield is present in the decision data
    uint8_t decision_data_len; // The number of octets in the Decision_Data parameter. Range 0 to 8
    uint8_t decision_data[1]; // resolvable data and or + varaibale 2 octects value if provided
} __attribute__ ((packed));
struct hci_le_set_decision_data_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_set_decision_instructions {
    uint8_t num_tests; // 0x01 to 0xFF, controller supports least 8
} __attribute__ ((packed));
struct hci_le_set_decision_instructions_test {
    uint8_t test_flags; // bit 0 - 3
    uint8_t test_field; // 0 to 255, 240 to 255 vendor
    uint8_t test_params[16];
} __attribute__ ((packed));
struct hci_le_set_decision_instructions_cmpl {
    uint8_t status;
} __attribute__ ((packed));

/**
 * The HCI_LE_Set_Advertising_Enable is used to request the Controller to start or stop advertising. The Controller
 * manages the timing of advertisements as per the advertising parameters given in the HCI_LE_Set_Adv_Parameters.
 * The Controller shall continue advertising until the Host issues an HCI_LE_Set_Adv_Enable with enable set to 0x00,
 * a connection is created using the advertising, or the advertising is timed out due to high duty cycle directed
 * advertising. In these cases, advertising is then disabled.
 *
 * Enabling advertising when it is already enabled can cause the random address to change. Disabling advertising
 * when it is already disabled has no effect.
 *
 * The HCI_LE_Set_Extended_Advertising_Enable is used to request the Controller to enable or disable one or more
 * advertising sets using the advertising sets. If Enable and Num_Sets are both set to 0x00, then all advertising
 * sets are disabled. The Controller shall only start an advertising event when the corresponding advertising
 * set is enabled, and shall continue advertising until all advertising sets have been disabled. An advertising set
 * shall be disabled when the Host issues an HCI_LE_Set_Extended_Advertising_Enable with Enable set to 0x00, a
 * connection is created using that advertising set, the duration specified in the Duration[i] expires, or the
 * number of extended advertising events transmitted for the set exceeds the Max_Extended_Advertising_Events[i].
 *
 * The Duration[i] indicates the duration for which that advertising set is enabled. The duration begins at the
 * start of the first advertising event of this advertising set. The Controller should not start an extended
 * advertising event that is cannot complete within the duration.
 *
 * If the advertising is high duty cycle connectable directed advertising, then Duration[i] shall be less than or
 * equal to 1.28s and shall not be equal to 0.
 *
 * The Max_Extended_Advertising_Events[i], if non-zero, indicates the max number of extended advertising events
 * that shall be sent prior to disabling the extended advertising set even if the Duration[i] has not expired.
 *
 * If the HCI_LE_Set_Extended_Advertising_Enable is sent again for an advertising set while that set is enabled,
 * the timer used for the duraiton and the number of events counter are reset and any change to the random address
 * shall take effect.
 *
 * Disabling the advertising set identified by the Advertising_Handle[i] does not disable any periodic advertising
 * associated with that set. Disabling an advertising set that is already disabled has no effect.
 *
 * The HCI_LE_Read_Max_Adv_Data_Length is used to read the max length of data supported by the Controller for use
 * as advertisement data or scan rsp data in an advertising event or as periodic advertisment data. Note: The
 * max amount may be fragmented across multiple PDUs.
 *
 * The HCI_LE_Read_Num_Supported_Adv_Sets is used to read the max num of adv sets supported by the advertising
 * Controller at the same time. Note: The num of advertising sets that can be supported is not fixed and the
 * Controller can change it at any time becuase the memory used to store advertising sets can also be used
 * for other purposes.
 *
 * ## HCI_LE_REMOVE_ADVERTISING_SET
 *
 * The HCI_LE_Remove_Advertising_Set is used to remove an advertising set from the Controller. If advertising or
 * periodic advertising on the advertising set is enabled, then the Controller shall return Command Disabllowed.
 *
 * ## HCI_LE_CLEAR_ADVERTISING_SETS
 *
 * The HCI_LE_Clear_Advertising_Sets is used to remove all existing advertising sets from the Controller. If
 * advertising or periodic advertising is enabled on any advertising set, then the Controller shall return
 * Command Disallowed. Note: All advertising sets are cleared on HCI reset.
 *
 * ## HCI_LE_EV_ADV_SET_TERMINATED
 *
 * The HCI_LE_Advertising_Set_Terminated event indicates the Controller has terminated advertising in the
 * advertising sets specified by the Advertising_Handle parameter.
 *
 * This event shall be generated every time connectable advertising in an Advertising set results in a connection
 * being created or because the advertising duration (Advertising Timeout 0x3C) or the max number of ext adv events
 * (Limit Reached 0x43) has been reached. It shall not generated if the Host disables the advertising set.
 *
 * This event shall only be generated if adv was enabled using the HCI_LE_Set_Ext_Adv_Enable command. The
 * Connection_Handle is only valid when advertising ends because a connection was created.
 *
 * If the Max_Ext_Adv_Events in HCI_LE_Set_Ext_Adv_Enable was non-zero. the Num_Completed_Ext_Adv_Events shall
 * be set to the num of completed ext adv events the Controller had transmitted when either the duration elapsed
 * or the max num of ext advertising events was reached; otherwise it shall be set to zero.
 *
 */
#define HCI_LE_SET_ADV_ENABLE 0x200A
#define HCI_LE_SET_EXT_ADV_ENABLE 0x2039
#define HCI_LE_READ_MAX_ADV_DATA_LENGTH 0x203A
#define HCI_LE_READ_NUM_SUPP_ADV_SETS 0x203B
#define HCI_LE_REMOVE_ADVERTISING_SET 0x203C
#define HCI_LE_CLEAR_ADVERTISING_SETS 0x203D
#define HCI_LE_EV_ADV_SET_TERMINATED 0x12
struct hci_le_set_adv_enable {
    uint8_t adv_enable;
} __attribute__ ((packed));
struct hci_le_set_adv_enable_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_ext_adv_enable_item {
    uint8_t adv_handle;
    uint16_t duration; // 0x0000 continue adv until host disable, 0xXXXX 0x0001 to 0xFFFF * 10ms, 10ms to 655350ms
    uint8_t max_ext_adv_events; // 0x00 no max num, 0xXX max events shall send prior to terminating the ext advertising
} __attribute__ ((packed));;
struct hci_le_set_ext_adv_enable {
    uint8_t adv_enable;
    uint8_t num_sets;
} __attribute__ ((packed));
struct hci_le_set_ext_adv_enable_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_remove_adv_set {
    uint8_t adv_handle;
} __attribute__ ((packed));
struct hci_le_read_max_adv_data_len_cmpl {
    uint8_t status; // max data len used in an advertising event or as periodic advertisement data
    uint16_t max_adv_data_length; // 0x1F (31) to 0x672 (1650)
} __attribute__ ((packed));
struct hci_le_read_num_supp_adv_sets_cmpl {
    uint8_t status;
    uint8_t num_supp_adv_sets; // 0x01 to 0xF0, num of adv sets supported at the same time
} __attribute__ ((packed));
struct hci_le_remove_adv_set_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_clear_adv_sets_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_ev_le_adv_set_terminated {
    uint8_t subcode;
    uint8_t status;
    uint8_t adv_handle;
    uint16_t conn_handle; // whose creation ended the advertising
    uint8_t num_cmpl_ext_adv_events;
} __attribute__ ((packed));
struct hci_ev_le_scan_req_received {
    uint8_t subcode;
    uint8_t adv_handle;
    uint8_t scanner_addr_type; // 0x00 0x02 public or public ia, 0x01 0x03 random or random ia
    uint8_t scanner_addr[6];
} __attribute__ ((packed));

/**
 * The HCI_LE_Read_Filter_Accept_List_Size is used to read the total num of Filter Accept List entries
 * that can be stored in the Controller. Note: The num of entries that can be stored is not fixed and
 * the Controller can change it at any time (e.g. because the memory used to store the Filter Accept
 * List can also be used for other purpose).
 *
 * The HCI_LE_Clear_Filter_Accept_List shall not be used when:
 * The HCI_LE_Add_Device_To_Filter_Accept_List shall not be used when:
 * The HCI_LE_Remove_Device_From_Filter_Accept_List shall not be used when:
 *          any adv filter policy uses the list and adv is enabled;
 *          scanning filter policy uses the list and scanning is eanbled;
 *          initiator filter policy uses the list and and creat connection is pending;
 *
 * If the device is already in the Filter Accept List, HCI_LE_Add_Device_To_Filter_Accept_List the
 * Controller should not add the device to the list again and should return success.
 *
 */
#define HCI_LE_READ_FILTER_ACCEPT_LIST_SIZE 0x200F
#define HCI_LE_CLEAR_FILTER_ACCEPT_LIST 0x2010
#define HCI_LE_ADD_DEVICE_TO_FILTER_ACCEPT_LIST 0x2011
#define HCI_LE_REMOVE_DEVICE_FROM_FILTER_ACCEPT_LIST 0x2012
struct hci_le_read_filter_accept_list_size_cmpl {
    uint8_t status;
    uint8_t list_size; // 0x01 to 0xFF
} __attribute__ ((packed));
struct hci_le_clear_filter_accept_list_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_add_device_to_filter_accept_list {
    uint8_t addr_type; // 0x00 public device address, 0x01 random device address, 0xFF devices sending anonymous adv
    uint8_t address[6]; // address shall be ignored when addr type is 0xFF
} __attribute__ ((packed));
struct hci_le_add_device_to_filter_accept_list_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_remove_device_from_filter_accept_list {
    uint8_t addr_type; // 0x00 public device address, 0x01 random device address, 0xFF devices sending anonymous adv
    uint8_t address[6]; // address shall be ignored when addr type is 0xFF
} __attribute__ ((packed));
struct hci_le_remove_device_from_filter_accept_list_cmpl {
    uint8_t status;
} __attribute__ ((packed));

/**
 * The HCI_LE_Add_Device_To_Resolving_List is used to add one device to the resolving list used
 * to generate and resolve RPA in the Controller.
 *
 * This command may be used at any time when address resolution is disabled in the Controller.
 * The added device shall be set to Network Privacy mode. If Peer_Identity_Address_Type is 0x01 and
 * Peer_Identity_Address i snot a static address, then the Controller should return the error code
 * Invalid HCI Command Parameters (0x12). If there is an existing entry in the resolving list with
 * the same Peer_Identity_Address and Peer_Idnetity_Address_Type, or with the same non-zero Peer_IRK,
 * the Controller should return Invalid HCI Command Parameters (0x12).
 *
 * HCI_LE_Add_Device_To_Resolving_List shall not be used when address resolution is enabled in Controller and:
 * HCI_LE_Remove_Device_From_Resolving_List shall not be used when addr reso is enabled in Controller and:
 * HCI_LE_Clear_Resolving_List shall not be used when address resolution is enabled in Controller and:
 * HCI_LE_Set_Address_Resolution_Enalbe shall not be used when:
 * HCI_LE_Set_Privacy_Mode shall not be used when address resolution is enabled in the Controller and:
 *          Advertising (other than periodic advertising) is enabled,
 *          Scanning is enabled, or
 *          HCI_LE_Create_Connection, HCI_LE_Extended_Create_Connection, HCI_LE_PA_Create_Sync is pending
 *
 * The HCI_LE_Read_Peer/Local_RPA is used to get the current peer/local RPA being used for the corresponding
 * peer public or random (static) identity address. The peer/local RPA being used may change after the command
 * is called. This command may be used at any time.
 *
 * The HCI_LE_Set_Address_Resolution_Enable is used to enable resolution of RPA in the Controller. This causes
 * the Controller to use the resolving list whenever the Controller receives a local or peer RPA. Enabling address
 * resolution when it is already enabled, or disabling it when it is disabled, has no effect.
 *
 * The HCI_LE_Set_RPA_Timeout set the length of time the Controller uses a RPA before a new RPA is generated
 * and starts being used. This timeout applies to all RPA generated by the Controller.
 *
 * The HCI_LE_Set_Privacy_Mode is used to allow the Host to specify the privacy mode to be used for a given
 * entry on the resolving list. When an entry on the resolving list is removed, the mode associated with that
 * entry shall also be removed. This command may be used at any time when address resolution is disabled in
 * The Controller. If the device is not on the resolving list, the Controller shall return the error code
 * Unknown Connection Idnetifier (0x02).
 *
 * The HCI_LE_Set_Data_Related_Address_Changes specifies circumstances when the Controller shall refresh any
 * RPA used by the advertising set, whether or not the address timeout period has been reached. This command
 * may be used while advertising is enabled. If legacy advertising command are being used, the Controller shall
 * ignore the Advertising_Handle parameter.
 *
 */
#define HCI_LE_ADD_DEVICE_TO_RESOLVING_LIST 0x2027
#define HCI_LE_REMOVE_DEVICE_FROM_RESOLVING_LIST 0x2028
#define HCI_LE_CLEAR_RESOLVING_LIST 0x2029
#define HCI_LE_READ_RESOLVING_LIST_SIZE 0x202A
#define HCI_LE_READ_PEER_RPA 0x202B
#define HCI_LE_READ_LOCAL_RPA 0x202C
#define HCI_LE_SET_ADDRESS_RESOLUTION_ENABLE 0x202D
#define HCI_LE_SET_RPA_TIMEOUT 0x202E
#define HCI_LE_SET_PRIVACY_MODE 0x204E
#define HCI_LE_SET_DATA_RELATED_ADDRESS_CHANGES 0x207C
struct hci_le_add_device_to_resolving_list {
    uint8_t peer_ia_type; // 0x00 public identity address, 0x01 random (static) identity address
    uint8_t peer_ia[6];
    uint8_t peer_irk[16];
    uint8_t local_irk[16];
} __attribute__ ((packed));
struct hci_le_add_device_to_resolving_list_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_remove_device_from_resolving_list {
    uint8_t peer_ia_type;
    uint8_t peer_ia[6];
} __attribute__ ((packed));
struct hci_le_remove_device_from_resolving_list_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_clear_resolving_list_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_read_resolving_list_size_cmpl {
    uint8_t status;
    uint8_t list_size; // 0x01 to 0xFF
} __attribute__ ((packed));
struct hci_le_read_peer_rpa {
    uint8_t peer_ia_type;
    uint8_t peer_ia[6];
} __attribute__ ((packed));
struct hci_le_read_peer_rpa_cmpl {
    uint8_t status;
    uint8_t peer_rpa[6];
} __attribute__ ((packed));
struct hci_le_read_local_rpa {
    uint8_t peer_ia_type;
    uint8_t peer_ia[6];
} __attribute__ ((packed));
struct hci_le_read_local_rpa_cmpl {
    uint8_t status;
    uint8_t local_rpa[6];
} __attribute__ ((packed));
struct hci_le_set_addr_reso_enable {
    uint8_t addr_reso_enable;
} __attribute__ ((packed));
struct hci_le_set_addr_reso_enable_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_set_rpa_timeout {
    uint16_t rpa_timeout; // 1 to 0x0E10, 1s to 1h, default 0x0384 (900s or 15m)
} __attribute__ ((packed));
struct hci_le_set_rpa_timeout_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_set_privacy_mode {
    uint8_t peer_ia_type;
    uint8_t peer_ia[6];
    uint8_t privacy_mode; // 0x00 use network privacy mode for this peer device (default), 0x01 use device privacy mode
} __attribute__ ((packed));
struct hci_le_set_privacy_mode_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_set_data_related_address_changes {
    uint8_t adv_handle;
    uint8_t change_reasons; // bit 0 - change whenever advertising data changes, bit 1 - when scan response data changes
} __attribute__ ((packed));
struct hci_le_set_data_related_address_changes_cmpl {
    uint8_t status;
} __attribute__ ((packed));

/**
 * The HCI_LE_Add_Device_To_Monitored_Advertisers_List command is used to add a
 * single device to a list of devices monitored while scanning.
 *
 * The HCI_LE_Remove_Device_From_Monitored_Advertisers_List command is used to
 * remove a single device from the Monitored Advertisers List.
 *
 * The HCI_LE_Clear_Monitored_Advertisers_List command is used to remove all devices
 * from the Monitored Advertisers List.
 *
 * The HCI_LE_Enable_Monitoring_Advertisers command is used by a Host to request the
 * Controller to enable or disable the monitoring of advertisers in the Monitored Advertisers
 * List and generate the HCI_LE_Monitored_Advertisers_Report event when necessary.
 * When the Enable parameter is set to 0x01, all entries in the Monitored Advertisers List
 * shall be marked as not awaiting an RSSI value above the RSSI threshold high and
 * their timers shall be reset. The Enable parameter may be set to 0x01 at any time as a
 * method of restarting monitoring. When the Enable parameter is set to 0x00 monitoring
 * is disabled.
 *
 * The HCI_LE_Read_Monitored_Advertisers_List_Size command is used to read the total
 * number of entries in the Monitored Advertisers List that the Controller can store.
 *
 */
#define HCI_LE_ADD_DEVICE_TO_MONITORED_ADVERTISERS_LIST 0x2098
#define HCI_LE_REMOVE_DEVICE_FROM_MONITORED_ADVERTISERS_LIST 0x2099
#define HCI_LE_CLEAR_MONITORED_ADVERTISERS_LIST 0x209A
#define HCI_LE_ENABLE_MONITORING_ADVERTISERS 0x209C
#define HCI_LE_READ_MONITORED_ADVERTISERS_LIST_SIZE 0x209B
struct hci_le_add_device_to_monitored_advs_list {
    uint8_t peer_type; // 0x00 public identity address, 0x01 random (static) identity address
    uint8_t peer_addr[6];
    int8_t rssi_thres_low_dbm; // Range -127 to +20
    int8_t rssi_thres_high_dbm; // Range -127 to +20
    uint8_t timeout_s; // 0x01 to 0xFF
} __attribute__ ((packed));
struct hci_le_add_device_to_monitored_advs_list_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_remove_device_from_monitored_advs_list {
    uint8_t peer_type; // 0x00 public identity address, 0x01 random (static) identity address
    uint8_t peer_addr[6];
} __attribute__ ((packed));
struct hci_le_remove_device_from_monitored_advs_list_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_clear_monitored_advs_list_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_read_monitored_advs_list_size_cmpl {
    uint8_t status;
    uint8_t list_size; // 0x01 to 0xFF
} __attribute__ ((packed));
struct hci_le_enable_monitoring_advs {
    uint8_t enable;
} __attribute__ ((packed));
struct hci_le_enable_monitoring_advs_cmpl {
    uint8_t status;
} __attribute__ ((packed));

/**
 * HCI_LE_Set_Scan_Parameters shall not be issued when scanning is enabled in the Controller.
 * HCI_LE_Set_Ext_Scan_Parameters shall not be issued when scanning is eanbled in Controller.
 *
 * HCI_LE_Set_Scan_Enable is used to start and stop scanning for legacy PDUs (but not extended
 * PDUs, even if the device supports extended advertising). If the LE_Scan_Enable is set to 0x01
 * and scanning is already enabled, any change to the Filter_Duplicates setting shall take effect.
 * Disabling scanning when it is disabled has no effect.
 *
 * Zero or more HCI_LE_Advertising_Report events are generated by the Controller based on legacy
 * advertising packets received and the duplicate filtering. No report shall be issued for extended
 * advertising PDUs.
 *
 * When the Scanning_Filter_Policy is set to 0x02 or 0x03 and a directed advertisment was received
 * where the advertiser used a RPA which the controller is unable to resolve, an HCI_LE_Directed_Advertising_Report
 * shall be generated instaed of an HCI_LE_Advertising_Report event.
 *
 * ## HCI_LE_Set_Ext_Scan_Parameters
 *
 * The HCI_LE_Set_Ext_Scan_Parameters is used to set the extended scan parameters to be used on
 * the advertising physical channels. The Scanning_PHYs indicates the PHY(s) on which the advertising
 * packets should be received on the primary advertising physical channel. The Host may enable one
 * or more scanning PHYs. If the Host specifies a PHY that is not supported by the Controller, including
 * a bit that is reserved for future use, it should return Unsupported Feature or Parameter Value.
 *
 * The Scan_Type[i], Scan_Interval[i], and Scan_Window[i] are ordered in the same order as the set bits
 * in the Scanning_PHYs parameter, starting from bit 0.
 *
 * ## HCI_LE_Set_Ext_Scan_Enable
 *
 * The HCI_LE_Set_Ext_Scan_Enable is used to enable or disable scanning for both legacy and extended
 * advertising PDUs. If Enable is set to 0x01 and the Host has not issued HCI_LE_Set_Ext_Scan_Parameters,
 * the Controller shall either use vendoer specific parameters or return Command Disallowed.
 *
 * If the Filter_Duplicates parameter is set to 0x02, duplicate advertisements in a single scan period
 * should not be sent to the Host; this setting shall only be used if both Period and Duration are
 * non-zero. If Filter_Duplicates is set to 0x02 and either Period or Duration to zero, the Controller
 * shall return Invalid HCI Command Parameters.
 *
 * If the Duration is zero or both Duration and Period are non-zero, the Controller shall continue
 * scanning until scanning is disabled by the Host. The Period shall be ignored when the Duration is
 * zero.
 *
 * If the Duration is non-zero and the Period is zero, the Controller shall continue scanning until
 * the duration specified in the Duration has expired. If both Duration and Period are non-zero and
 * the Duration is gerater than or equal to the Period, the Controller shall return Invalid HCI
 * Command Parameters. When both are non-zero, the Controller shall scan for the duration of the
 * Duration within a scan period. After the scan period has expired, a new scan period shall begin
 * and scanning shall begin again for the duration specified. The scan periods continue until the
 * Host disables scanning.
 *
 * If Enable set to 0x01 while scanning is already enabled, the timers used for duration and period
 * are reset to the new parameter values and a new scan period is started. Any change to the Filter_Duplicates
 * setting or the random address shall take effect. Disabling scanning when it is disabled has no effect.
 *
 * Note: The duration of a scan period refers to the time spent scanning on both the primary and secondry
 * advertising physical channels. However, expiry of the duration does not prevent the LL from scanning
 * for and receiving aux packets of received advertisements.
 *
 * Zero or more LE Extended Advertising Reports are generated by the Controller based on any adv packets
 * received and the duplicate filtering in effect. At the end of a sigle scan (Duration non-zero but Period
 * zero), an HCI_LE_Scan_Timeout event shall be generated.
 *
 * ## HCI_LE_EV_SCAN_TIMEOUT
 *
 * This event indicates scanning has ended because the duration has expired. It shall only be generated
 * if scanning was enabled using the HCI_LE_Set_Extended_Scan_Enable command.
 *
 * ## HCI_LE_EV_SCAN_REQ_RECEIVED
 *
 * This event indicates that a SCAN_REQ or an AUX_SCAN_REQ has been received by the advertiser. The request
 * contains a device address from a scanner that is allowed by the advertising filter policy. This event
 * shall only be generated if advertising was enabled using the HCI_LE_Set_Extended_Advertising_Enable
 * command.
 *
 * ## Adv report events
 *
 * HCI_LE_Advertising_Report and HCI_LE_Directed_Advertising_Report only reports advertising events that
 * used legacy advertising PDUs.
 *
 * HCI_LE_Directed_Advertising_Report indicates that directed advertisements have been received where
 * the advertiser is using a RPA for the TargetA which the Controller is unable to resolve and the
 * Scanning_Filter_Policy is equal to 0x02 or 0x03. Direct_Address_Type and Direct_Address specify the
 * address the directed adv are being directed to.
 *
 */
#define HCI_LE_SET_SCAN_PARAMETERS 0x200B
#define HCI_LE_SET_SCAN_ENABLE 0x200C
#define HCI_LE_SET_EXT_SCAN_PARAMS 0x2041
#define HCI_LE_SET_EXT_SCAN_ENABLE 0x2042
#define HCI_LE_EV_SCAN_TIMEOUT 0x11
#define HCI_LE_EV_SCAN_REQ_RECEIVED 0x13
#define HCI_LE_EV_ADV_REPORT 0x02
#define HCI_LE_EV_DIRECTED_ADV_REPORT 0x0B
#define HCI_LE_EV_EXT_ADV_REPORT 0x0D
#define HCI_LE_EV_MONITORED_ADV_REPORT 0x34 // Core Spec 6.0
#define HCI_ADV_TYPE_ADV_IND 0x00
#define HCI_ADV_TYPE_DIRECT_IND 0x01
#define HCI_ADV_TYPE_SCAN_IND 0x02
#define HCI_ADV_TYPE_NONCONN_IND 0x03
#define HCI_ADV_TYPE_SCAN_RSP 0x04
#define HCI_ADV_TYPE_BIT_CONNECTABLE 0x01
#define HCI_ADV_TYPE_BIT_SCANNABLE 0x02
#define HCI_ADV_TYPE_BIT_DIRECTED 0x04
#define HCI_ADV_TYPE_BIT_SCAN_RSP 0x08
#define HCI_ADV_TYPE_BIT_LEGACY_PDU 0x10
#define HCI_ADV_REPOT_DATA_STATUS(t) (((t)>>5)&0x3)
struct hci_le_set_scan_parameters {
    uint8_t le_scan_type; // 0x00 passive scanning, no scanning PDUs shall be sent (default), 0x01 active scanning
    uint16_t le_scan_interval; // 0x04 to 0x4000 * 0.625ms, 2.5ms to 10.24s, default 0x10 (10ms)
    uint16_t le_scan_window; // duration of le scan, 0x04 to 0x4000 * 0.625ms, default 0x10 (10ms)
    uint8_t own_addr_type; // 0x00 pub, 0x01 rnd, 0x02 rpa or pub, 0x03 rpa or rnd from LE_Set_Random_Address
    uint8_t scanning_filter_policy; // 0x00 basic unfiltered, 0x01 basic filtered, 0x02 0x03 extended unfiltered/filtered
} __attribute__ ((packed));
struct hci_le_set_scan_params_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_set_scan_enable {
    uint8_t le_scan_enable;
    uint8_t filter_duplicates;
} __attribute__ ((packed));
struct hci_le_set_scan_enable_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_set_ext_scan_item {
    uint8_t scan_type;
    uint16_t scan_interval; // 0x04 to 0xFFFF * 0.625ms, 2.5ms to 40.959375s
    uint16_t scan_window; // duration of the scan on the primary advertising physical channel
} __attribute__ ((packed));
struct hci_le_set_ext_scan_params {
    uint8_t own_addr_type;
    uint8_t scanning_filter_policy;
    uint8_t scanning_phys; // bit-0 scan adv on LE 1M PHY, bit-2 scan adv on LE Coded PHY
} __attribute__ ((packed));
struct hci_le_set_ext_scan_params_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_set_ext_scan_enable {
    uint8_t scan_enable;
    uint8_t filter_duplicates; // 0x00 dont filter, 0x01 fiter duplicated, 0x02 fiter and reset for each scan period
    uint16_t duration; // 0x0000 need explicity disable, 0xXXXX 0x01 to 0xFFFF * 10ms, 10ms to 655.35s
    uint16_t period; // 0x0000 scan continously, 0xXXXX 0x01 to 0xFFFF * 1.28s, 1.28s to 83884.8s
} __attribute__ ((packed));
struct hci_le_set_ext_scan_enable_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_ev_le_scan_timeout {
    uint8_t subcode;
} __attribute__ ((packed));
struct hci_ev_le_adv_report_item {
    uint8_t event_type; // 0x00 ADV_IND, 0x01 ADV_DIRECT_IND, 0x02 ADV_SCAN_IND, 0x03 ADV_NONCONN_IND, 0x04 SCAN_RSP
    uint8_t peer_addr_type; // 0x00 ~ 0x03
    uint8_t peer_addr[6];
    uint8_t data_length; // 0 to 31
    uint8_t data[1]; // variable length
    // int8_t rssi; // 0x7F rssi is not available, -127 to 20dBm
} __attribute__ ((packed));
struct hci_ev_le_adv_report {
    uint8_t subcode; // 0x02
    uint8_t num_reports; // 0x01 to 0x19
} __attribute__ ((packed));
struct hci_ev_le_directed_adv_report_item {
    uint8_t event_type; // 0x01 ADV_DIRECT_IND
    uint8_t peer_addr_type; // 0x00 ~ 0x03
    uint8_t peer_addr[6];
    uint8_t direct_addr_type; // 0x01
    uint8_t direct_addr[6];
    int8_t rssi; // 0x7F rssi is not available, -127 to 20dBm
} __attribute__ ((packed));
struct hci_ev_le_directed_adv_report {
    uint8_t subcode; // 0x0b
    uint8_t num_reports; // 0x01 to 0x19
} __attribute__ ((packed));
struct hci_ev_le_ext_adv_report_item {
    uint16_t event_type; // bit 0~4 Connectable Scannable Directed ScanRsp LegacyPdu bit 5~6 0 Complete 1 Incmpl more data 2 Incmpl truncated
    uint8_t peer_addr_type; // 0x00 ~ 0x03, 0xFF no address provided (anonymous adv)
    uint8_t peer_addr[6];
    uint8_t primary_phy; // 0x01 LE 1M, 0x03 LE Coded S=8 or LE Coded, 0x04 LE Coded S=2 or RFU
    uint8_t secondary_phy; // 0x00 No packets on the secondary adv, 0x01 0x02 LE 1M 2M, 0x03 LE Coded S=8 or LE Coded, 0x04 LE Coded S=2 or RFU
    uint8_t adv_sid; // 0x00 to 0x0F, 0xFF no ADI field provided
    int8_t tx_power; // 0x7F txpower info not available, -127 to 20dBm
    int8_t rssi; // 0x7F rssi is not available, -127 to 20dBm
    uint16_t pa_interval; // 0x0000 no pa, 0xXXXX 0x06 to 0xFFFF * 1.25ms, 7.5ms to 81918.75s
    uint8_t direct_addr_type; // 0x00~0x03, 0xFE RPA controller unable to resolve
    uint8_t direct_addr[6]; // TargetA or Public IA or Random IA
    uint8_t data_length; // 0 to 229
    uint8_t data[1]; // variable length
} __attribute__ ((packed));
struct hci_ev_le_ext_adv_report {
    uint8_t subcode; // 0x0d
    uint8_t num_reports; // 0x01 to 0x0A
} __attribute__ ((packed));
struct hci_ev_le_monitored_adv_report {
    uint8_t subcode; // 0x34
    uint8_t peer_addr_type; // 0x00 ~ 0x03
    uint8_t peer_addr[6];
    uint8_t condition; // 0x00 or 0x01, Device RSSI below the low RSSI threshold for longer than the timeout period or greater than the high RSSI
} __attribute__ ((packed));

/**
 * The HCI_LE_Create_Connection is used to create an ACL connection, with the local device in the Central
 * role to a connectable advertiser. ??? If a connection is created with the local device in the Peripheral
 * role while this command is pending, then this command remains pending.
 *
 * If the Host issues the command when another HCI_LE_Create_Connecion is pending in the Controller, the
 * Controller shall return Command Disallowed. If the Host issues the command when another HCI_LE_Extented_
 * Create_Connection command is pending in the Controller, the Controller shall reutnr Command Disallowed.
 *
 * The Initiator_Filter_Policy is used to determine whether the Filter Accept List is used. If not used, the
 * Peer_Address_Type and the Peer_Address specify the advertising device to connect to.
 *
 * Peer_Address_Type indicates the type of address used in the connectable advertisement sent by the peer.
 * The Host shall not set it to 0x02 or 0x03 if both the Host and the Controller support HCI_LE_Set_Privacy_Mode.
 * If a Controller that supports HCI_LE_SET_Privacy_Mode receives the HCI_LE_Create_Connection with Peer_Address_Type
 * set to either 0x02 or 0x03, it may use either device privacy mode or network privacy mode for that peer
 * device.
 *
 * If the Own_Address_Type is set to 0x02/0x03, the Initiator_Filter_Policy is set to 0x00, the Controller's
 * resolving list did not contain a matching entry, and the device does not have a public/random address, the
 * Controller should return Invalid Command Parameters (0x12.
 *
 * If the Own_Address_Type is set to 0x02/0x03, the Initiator_Filter_Policy is set to 0x01, the device does
 * not have a public/random address, the Controller should return Invalid Command Parameters (0x12).
 *
 * An HCI_LE_Connection_Complete or HCI_LE_Enhanced_Connection_Complete event shall be generated when a
 * connection is created because of this command or the connection creation procedure is cancelled; until
 * one of these events is generated, the command is considered pending. If a connection is created and the
 * controller supports LE Channel Selection Algotithm #2 feature, this event shall be immediately followed
 * by an HCI_LE_Channel_Selection_Algorithem event.
 *
 * The HCI_LE_Create_Connection_Cancel is used to cancel the HCI_LE_Create_Connection or HCI_LE_Extended_Create_Connection
 * commands. If no create connction command pending, then the Controller shall return Command Disallowed (0x0C).
 * When HCI_LE_Create_Connection_Cancel has completed, an HCI_Command_Complete event shall be generated. If the
 * cancellation was successful then, after the HCI_Command_Complete event, either an HCI_LE_Connection_Complete
 * or an HCI_LE_Enhanced_Connection_Complete shall be generated. In either case, the event shall be sent with
 * the error code Unknown Connection Identifier.
 *
 * ## LE Extended Create Connection
 *
 * The HCI_LE_Extended_Create_Connection is used to create an ACL connection, withe the local device in the
 * Central role, to a connectable advertiser. The command is also used to create an ACL connection between a
 * periodic advertiser and a synchronized device. ??? If a connection is created with the local device in
 * the Peripheral role while this command is pending, then this command remains pending.
 *
 * The Advertising_Handle is used to identify the periodic advertising train. The Subevent is used to identity
 * the subevent where a connection request shall be initiated from a periodic advertising train. These two
 * parameters shall be set to 0xFF if not used. If they set to valid values, then the Initiator_Filter_Policy,
 * Initiating_PHYs, Scan_Interval[i], and Scan_Window[i] parameters shall be ignored.
 *
 * The Initiating_PHYs indicates the PHY(s) on which the advertising packets should be received on the
 * primmary advertising physical channel and the PHYs for which connection parameters have been specified.
 * The Host may enable one or more initiating PHYs. If the Host specifies a PHY that is not supported by
 * the Controller, including a bit that is reserved for future use, shall return Unsupported Feature or
 * Parameter Value. The array elements are ordered in the same order as the set bits in the Initiating_PHYs.
 * When a connectable advertisement is received and a connection request is setn on one PHY, scanning on
 * any other PHYs is terminated. If bit 1 is set in Intiating_PHYs, Scan_Interval[i] and Scan_Window[i]
 * values for the LE 2M PHY shall be ignored.
 *
 * Where the connection is made on a PHY whose bit is not set in the Initiating_PHYs, the Controller
 * shall use the Connection_Interval_Min[i], Connection_Interval_Max[i], Max_Latency[i], Superv_Timeout[i],
 * Min_CE_Length[i], and Max_CE_Length[i] parameters for an implementation-chosen PHY whose bit is
 * set in the Initiating_PHYs parameter.
 *
 * When the Controller receives the HCI_LE_Extended_Create_Connection, the Controller sends the
 * HCI_Command_Status event to the Host. An HCI_LE_Enhanced_Connection_Complete event shall be generated
 * when a connection is created because of this command or the connection creation is cancelled; until
 * the event is generated, the command is considered pending. If a connection creation is discarded,
 * then the error Connection Failed to be Established/Synchronization Timeout (0x3E) shall be used.
 * If a conneciton is created, this event shall be immediately followed by an HCI_LE_Channel_Selection_
 * Algorithm event.
 *
 * ## LE Connection Complete Event
 *
 * The HCI_LE_Enhanced_Connection_Complete event indicates to both of the Hosts forming the connection
 * that a new connection has been created. If the connection createion fails, this event shall be
 * provided to the Host that had issued the HCI_LE_Create_Connection or HCI_LE_Extended_Create_Connection
 * command.
 *
 * The Peer_Address, Peer_RPA, Local_RPA shall always reflect the most recent packet sent and received
 * on air. The Central_Clock_Accuracy is only valid for a Peripheral. On a Central, this parameter shall
 * be set to 0x00.
 *
 * If the connection is established from periodic advertising with responses and Role is 0x00 Central,
 * then the Advertising_Handle shall be set according to the periodic advertising train the connection
 * was established from. If the conneciton is established from pa with responses and Role is 0x01 Pheripheral,
 * then the Sync_Handle shall be set according to the pa train the connection was established from. In
 * all other circumstances, Advertising_Handle and Sync_Handle shall be set to 0xFF and shall be ignored
 * by the Host.
 *
 */
#define HCI_LE_CREATE_CONNECTION 0x200D
#define HCI_LE_CREATE_CONN_CANCEL 0x200E
#define HCI_LE_EXT_CREATE_CONN_V1 0x2043
#define HCI_LE_EXT_CREATE_CONN_V2 0x2085 // core v5.4
#define HCI_LE_EV_CONN_COMPLETE 0x01
#define HCI_LE_EV_ENHANCED_CONN_CMPL_V1 0x0A
#define HCI_LE_EV_ENHANCED_CONN_CMPL_V2 0x29
struct hci_le_create_connection {
    uint16_t le_scan_interval; // 0x04 to 0x4000 * 0.625ms, 2.5ms to 10.24s
    uint16_t le_scan_window; // 0x04 to 0x4000 * 0.625ms, 2.5ms to 10.24s
    uint8_t initiator_filter_policy; // 0x00 use peer addr ignore accept list, 0x01 use accept list to connect
    uint8_t peer_addr_type; // 0x00 public 0x01 random 0x02 0x03 only used if either host or controller dont supp HCI_LE_SET_PRIVACY_MODE
    uint8_t peer_addr[6]; // public, random, public ia, or random ia
    uint8_t own_addr_type; // 0x00 ~ 0x03
    uint16_t conn_interval_min; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4s
    uint16_t conn_interval_max; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4s
    uint16_t max_latency; // 0x00 to 0x01F3, max peripheral latency in num of connection events
    uint16_t superv_timeout; // 0x0A to 0x0C80 * 10ms, 100ms to 32s
    uint16_t min_ce_length; // min len of connection event, 0x00 to 0xFFFF * 0.625ms
    uint16_t max_ce_length; // max len of connection event, 0x00 to 0xFFFF * 0.625ms
} __attribute__ ((packed));
struct hci_le_create_conn_cancel_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_ext_create_param_item {
    uint16_t scan_interval; // 0x04 to 0xFFFF * 0.625ms, 2.5ms to 40.959375s
    uint16_t scan_window; // 0x04 to 0xFFFF * 0.625ms, 2.5ms to 40.959375s
    uint16_t conn_interval_min; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4s
    uint16_t conn_interval_max; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4s
    uint16_t max_latency; // 0x00 to 0x01F3, max peripheral latency in num of connection events
    uint16_t superv_timeout; // 0x0A to 0x0C80 * 10ms, 100ms to 32s
    uint16_t min_ce_length; // min len of connection event, 0x00 to 0xFFFF * 0.625ms
    uint16_t max_ce_length; // max len of connection event, 0x00 to 0xFFFF * 0.625ms
} __attribute__ ((packed));
struct hci_le_ext_create_conn_v2 {
    uint8_t adv_handle; // 0x00 to 0xEF identify the pa train, or 0xFF
    uint8_t subevent; // 0x00 to 0x7F subevent where the conn req is to be sent, or 0xFF
    uint8_t initiator_filter_policy; // 0x00 use peer addr, 0x01 use filter accept list
    uint8_t own_addr_type; // 0x00 ~ 0x03
    uint8_t peer_addr_type; // 0x00 public or public ia, 0x01 random or random ia
    uint8_t peer_addr[6];
    uint8_t initiating_phys; // bit0 scan adv on 1M params provided for 1M, bit1 params provided for 2M, bit2 scan adv on Coded params provided for Coded
} __attribute__ ((packed));
struct hci_le_ext_create_conn_v1 {
    uint8_t initiator_filter_policy; // 0x00 use peer addr, 0x01 use filter accept list
    uint8_t own_addr_type; // 0x00 ~ 0x03
    uint8_t peer_addr_type; // 0x00 public or public ia, 0x01 random or random ia
    uint8_t peer_addr[6];
    uint8_t initiating_phys; // bit0 scan adv on 1M params provided for 1M, bit1 params provided for 2M, bit2 scan adv on Coded params provided for Coded
} __attribute__ ((packed));
struct hci_ev_le_connection_cmpl {
    uint8_t subcode;
    uint8_t status;
    uint16_t conn_handle;
    uint8_t role; // 0x00 Central, 0x01 Peripheral
    uint8_t peer_addr_type; // 0x00 public, 0x01 random
    bt_bdaddr_t peer_addr;
    uint16_t conn_interval; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4000ms
    uint16_t peripheral_latency; // peripheral latency in num of subrated conn events, 0x00 to 0x01F3 (499)
    uint16_t superv_timeout; // 0x0A to 0x0C80 * 10ms, 100ms to 32s
    uint8_t central_clock_accuracy; // 0x00 500, 0x01 250, 0x02 150, 0x03 100, 0x04 75, 0x05 50, 0x06 30, 0x07 20ppm, only valid on Peripheral, on Central shall be set to 0x00
} __attribute__ ((packed));
struct hci_ev_le_ext_conn_cmpl_v2 {
    uint8_t subcode;
    uint8_t status;
    uint16_t conn_handle;
    uint8_t role; // 0x00 Central, 0x01 Peripheral
    uint8_t peer_addr_type; // 0x00 public, 0x01 random, 0x02 public ia, 0x03 random ia
    bt_bdaddr_t peer_addr;
    uint8_t local_rpa[6]; // only valid own_addr_type 0x02 0x03 and non-zero local IRK, otherwise shall return all zeros
    uint8_t peer_rpa[6]; // only valid when peer_addr_type is 0x02 or 0x03, for other peer_addr_type shall set all zeros
    uint16_t conn_interval; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4000ms
    uint16_t peripheral_latency; // peripheral latency in num of subrated conn events, 0x00 to 0x01F3 (499)
    uint16_t superv_timeout; // 0x0A to 0x0C80 * 10ms, 100ms to 32s
    uint8_t central_clock_accuracy; // 0x00 500, 0x01 250, 0x02 150, 0x03 100, 0x04 75, 0x05 50, 0x06 30, 0x07 20ppm, only valid on Peripheral, on Central shall be set to 0x00
    uint8_t adv_handle; // 0x00 to 0xEF, 0xFF no advertising handle
    uint16_t sync_handle; // 0x0000 to 0x0EFF identifying the pa train, 0xFFFF no sync handle
} __attribute__ ((packed));
struct hci_ev_le_ext_conn_cmpl_v1 {
    uint8_t subcode;
    uint8_t status;
    uint16_t conn_handle;
    uint8_t role; // 0x00 Central, 0x01 Peripheral
    uint8_t peer_addr_type; // 0x00 public, 0x01 random, 0x02 public ia, 0x03 random ia
    bt_bdaddr_t peer_addr;
    uint8_t local_rpa[6]; // only valid own_addr_type 0x02 0x03 and non-zero local IRK, otherwise shall return all zeros
    uint8_t peer_rpa[6]; // only valid when peer_addr_type is 0x02 or 0x03, for other peer_addr_type shall set all zeros
    uint16_t conn_interval; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4000ms
    uint16_t peripheral_latency; // peripheral latency in num of subrated conn events, 0x00 to 0x01F3 (499)
    uint16_t superv_timeout; // 0x0A to 0x0C80 * 10ms, 100ms to 32s
    uint8_t central_clock_accuracy; // 0x00 500, 0x01 250, 0x02 150, 0x03 100, 0x04 75, 0x05 50, 0x06 30, 0x07 20ppm, only valid on Peripheral, on Central shall be set to 0x00
} __attribute__ ((packed));

/**
 * The HCI_LE_Connection_Update command is used to change the ACL connection parameters. This command
 * may be issued on both the Central and Peripheral. If this command completes successfully and the
 * connection interval has changed, then the subrating factor shall be set to 1 and the conitnuation
 * number to 0. In this case, Max_Latency must be interpreted in underlying connection events. Otherwise
 * the subrating factor and continuation number shall be unchanged and Max_Latency must be interpreted
 * in subrated events.
 *
 * The HCI_LE_EV_CONN_UPDATE_COMPLETE event shall be issued if the HCI_LE_Connection_Update command was
 * issued by the Host or if the connection parameters are updated following a request from the peer
 * device. If no parameters are updated following a request from the peer device or the parameters were
 * changed using the Connection Subrate Update procedure, then this event shall not be issued.
 *
 * If the Status parameter is zero and the connection interval has changed, then the LL must have set
 * the subrating factor to 1 and the continuation number to 0.
 *
 * Note: This event can be issued auto by the Central's Controller if the decides to change the connection
 * interval based on the range of allowable connection intervals for that connection. The parameter values
 * returned in this event may be different from the parameter values provided by the Host through the
 * HCI_LE_Connection_Update command or the HCI_LE_Remote_Connection_Parameter_Request_Reply command.
 *
 * The HCI_LE_Subrate_Change event shall be issued if the HCI_LE_Subrate_Request command was issued by
 * the Host or the parameters are updated successfully following a request from the peer device. If no
 * parameters are updated following a request from the peer device or the parameters were changed using
 * the Connection Update procedure, then this event shall not be issued.
 *
 * The HCI_LE_Set_Default_PHY allows the host to specify its preferred values for the tx phy and rx phy
 * to be used for all subsequent connections over the LE transport. If the Host sets a bit for a PHY that
 * the Controller does not support, including a bit that is reserved for future use, the Controller shall
 * return the error code Unsupported Feature or Parameter Value (0x11). If the Controller does not support
 * asymmetric connections and the Host set all_phys to 0x00 and tx_phys to a different value than rx_phys,
 * the Controller shall return the error code Unsupported Feature or Parameter Value (0x11).
 *
 * The HCI_LE_Read_Transmit_Power is used to read the min and max transmit powers supported by the
 * Controller across all supported PHYs.
 *
 * The HCI_LE_Frame_Space_Update command allows the Host to request a change to
 * one or more frame space values. This command may be issued on both the Central and
 * the Peripheral.
 *
 */
#define HCI_LE_CONNECTION_UPDATE 0x2013
#define HCI_LE_REMOTE_CONN_PARAM_REQ_REPLY 0x2020
#define HCI_LE_REMOTE_CONN_PARAM_REQ_REJECT 0x2021
#define HCI_LE_SET_DATA_LENGTH 0x2022
#define HCI_LE_READ_DEFAULT_DATA_LENGTH 0x2023
#define HCI_LE_WRITE_DEFAULT_DATA_LENGTH 0x2024
#define HCI_LE_LL_DATA_PDU_TX_MAX_OCTETS_MIN 0x001B
#define HCI_LE_LL_DATA_PDU_TX_MAX_OCTETS_MAX 0x00FB
#define HCI_LE_LL_DATA_PDU_TX_MAX_TIME_US_MIN 0x0148
#define HCI_LE_LL_DATA_PDU_TX_MAX_TIME_US_MAX 0x4290
#define HCI_LE_READ_PHY 0x2030
#define HCI_LE_SET_DEFAULT_PHY 0x2031
#define HCI_LE_SET_PHY 0x2032
#define HCI_LE_READ_TX_POWER 0x204B
#define HCI_LE_READ_ADV_TX_POWER 0x2007
#define HCI_LE_READ_LOCAL_TX_POWER 0x2076
#define HCI_LE_READ_REMOTE_TX_POWER 0x2077
#define HCI_LE_SET_TX_POWER_REPORT_ENABLE 0x207A
#define HCI_LE_SET_DEFAULT_SUBRATE 0x207D
#define HCI_LE_SUBRATE_REQUEST 0x207E
#define HCI_LE_EV_REMOTE_CONN_PARAM_REQ 0x06
#define HCI_LE_EV_CONN_UPDATE_CMPL 0x03
#define HCI_LE_EV_DATA_LENGTH_CHANGED 0x07
#define HCI_LE_EV_PHY_UPDATE_CMPL 0x0C
#define HCI_LE_EV_TX_POWER_REPORTING 0x21
#define HCI_LE_EV_SUBRATE_CHANGE 0x23
#define HCI_LE_FRAME_SPACE_UPDATE 0x209D // Core Spec 6.0
#define HCI_LE_EV_FRAME_SPACE_UPD_CMPL 0x35
#define HCI_LE_FS_UPD_INITIATOR_LOCAL_HOST 0x00
#define HCI_LE_FS_UPD_INITIATOR_LOCAL_CTRL 0x01
#define HCI_LE_FS_UPD_INITIATOR_PEER_DEVICE 0x02
#define HCI_LE_SPACING_TYPE_T_IFS_ACL_CP 0x01
#define HCI_LE_SPACING_TYPE_T_IFS_ACL_PC 0x02
#define HCI_LE_SPACING_TYPE_T_MCES 0x04
#define HCI_LE_SPACING_TYPE_T_IFS_CIS 0x08
#define HCI_LE_SPACING_TYPE_T_MSS_CIS 0x10

struct hci_le_connection_update {
    uint16_t conn_handle;
    uint16_t conn_interval_min; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4000ms
    uint16_t conn_interval_max; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4000ms
    uint16_t max_latency; // peripheral latency in num of subrated conn events, 0x00 to 0x01F3 (499)
    uint16_t superv_timeout; // 0x0A to 0x0C80 * 10ms, 100ms to 32s
    uint16_t min_ce_length; // min len of connection event, 0x00 to 0xFFFF * 0.625ms
    uint16_t max_ce_length; // max len of connection event, 0x00 to 0xFFFF * 0.625ms
} __attribute__ ((packed));
struct hci_ev_le_remote_conn_param_req {
    uint8_t subcode;
    uint16_t conn_handle;
    uint16_t interval_min; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4000ms
    uint16_t interval_max; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4000ms
    uint16_t max_latency; // peripheral latency in num of subrated conn events, 0x00 to 0x01F3 (499)
    uint16_t superv_timeout; // 0x0A to 0x0C80 * 10ms, 100ms to 32s
} __attribute__ ((packed));
struct hci_le_conn_param_req_reply {
    uint16_t conn_handle;
    uint16_t conn_interval_min; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4000ms
    uint16_t conn_interval_max; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4000ms
    uint16_t max_latency; // peripheral latency in num of subrated conn events, 0x00 to 0x01F3 (499)
    uint16_t superv_timeout; // 0x0A to 0x0C80 * 10ms, 100ms to 32s
    uint16_t min_ce_length; // min len of connection event, 0x00 to 0xFFFF * 0.625ms, 0ms to 40.9s
    uint16_t max_ce_length; // max len of connection event, 0x00 to 0xFFFF * 0.625ms, 0ms to 40.9s
} __attribute__ ((packed));
struct hci_le_conn_param_req_reply_cmpl {
    uint8_t status;
    uint16_t conn_handle;
} __attribute__ ((packed));
struct hci_le_conn_param_req_reject {
    uint16_t conn_handle;
    uint8_t reason; // 0x3B, unacceptable conn params, instead of issuing this command, the host should try to provide alternative conn param to LL via Reply command
} __attribute__ ((packed));
struct hci_le_conn_param_req_reject_cmpl {
    uint8_t status;
    uint16_t conn_handle;
} __attribute__ ((packed));
struct hci_ev_le_conn_update_cmpl {
    uint8_t subcode;
    uint8_t status;
    uint16_t conn_handle;
    uint16_t conn_interval; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4000ms
    uint16_t peripheral_latency; // peripheral latency in num of subrated conn events, 0x00 to 0x01F3 (499)
    uint16_t superv_timeout; // 0x0A to 0x0C80 * 10ms, 100ms to 32s
} __attribute__ ((packed));
struct hci_le_set_default_subrate {
    uint16_t subrate_min; // min subrate factor allowed in requests by a Peripheral, 0x01 to 0x01F4, default 0x01
    uint16_t subrate_max; // max subrate factor allowed in requests by a Peripheral, 0x01 to 0x01F4, default 0x01
    uint16_t max_latency; // 0x00 to 0x01F3, default 0x00, peripheral latency allowed in requests by a Peripheral, in units of subrated conn intervals
    uint16_t continuation_number; // 0x00 to 0x01F3, default 0x00, num of underlying conn events to remain active after a packet contain a LL PDU with non-zero length is sent or received
    uint16_t superv_timeout; // 0x0A to 0x0C80 * 10ms, 100ms to 32s, default 0x0C80 32s, max superv timeout allowed in requests by a Peripheral
} __attribute__ ((packed));
struct hci_le_set_default_subrate_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_subrate_request {
    uint16_t conn_handle;
    uint16_t subrate_min; // min subrate factor to be applied to the underlying conn interval, 0x01 to 0x01F4
    uint16_t subrate_max; // max subrate factor to be applied to the underlying conn interval, 0x01 to 0x01F4
    uint16_t max_latency; // 0x00 to 0x01F3, max peripheral latency in units of subrated conn intervals
    uint16_t continuation_number; // 0x00 to 0x01F3, default 0x00, num of underlying conn events to remain active after a packet contain a LL PDU with non-zero length is sent or received
    uint16_t superv_timeout; // 0x0A to 0x0C80 * 10ms, 100ms to 32s, superv timeout for this connection
} __attribute__ ((packed));
struct hci_ev_le_subrate_change {
    uint8_t subcode;
    uint8_t status;
    uint16_t conn_handle;
    uint16_t subrate_factor; // 0x01 to 0x01F4, new subrate factor applied to the specified underlying conn interval
    uint16_t peripheral_latency; // peripheral latency in num of subrated conn events, 0x00 to 0x01F3 (499)
    uint16_t continuation_number; // 0x00 to 0x01F3, num of underlying conn events to remain active after a packet contain a LL PDU with non-zero length is sent or received
    uint16_t superv_timeout; // 0x0A to 0x0C80 * 10ms, 100ms to 32s
} __attribute__ ((packed));
struct hci_le_set_data_length {
    uint16_t conn_handle;
    uint16_t tx_octets; // 0x1B to 0xFB 27 to 251
    uint16_t tx_time; // 0x0148 to 0x4290 preferred max packet transmission time for LL data PDUs on this connection
} __attribute__ ((packed));
struct hci_le_set_data_length_cmpl {
    uint8_t status;
    uint16_t conn_handle;
} __attribute__ ((packed));
struct hci_ev_le_data_length_change {
    uint8_t subcode;
    uint16_t conn_handle;
    uint16_t max_tx_octets;
    uint16_t max_tx_time;
    uint16_t max_rx_octets;
    uint16_t max_rx_time;
} __attribute__ ((packed));
struct hci_le_write_default_data_length {
    uint16_t suggested_max_tx_octets; // 0x1B to 0xFB 27 to 251
    uint16_t suggested_max_tx_time; // 0x0148 to 0x4290 suggested max packet transmission time for LL data PDUs for new connections
} __attribute__ ((packed));
struct hci_le_write_default_data_length_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_read_default_data_length_cmpl {
    uint8_t status;
    uint16_t suggested_max_tx_octets; // 0x1B to 0xFB 27 to 251
    uint16_t suggested_max_tx_time; // 0x0148 to 0x4290 suggested max packet transmission time for LL data PDUs for new connections
} __attribute__ ((packed));
struct hci_le_read_phy {
    uint16_t conn_handle;
} __attribute__ ((packed));
struct hci_le_read_phy_cmpl {
    uint8_t status;
    uint16_t conn_handle;
    uint8_t tx_phy; // 0x01 LE 1M, 0x02 LE 2M, 0x03 LE Coded
    uint8_t rx_phy; // 0x01 LE 1M, 0x02 LE 2M, 0x03 LE Coded
} __attribute__ ((packed));
struct hci_le_set_default_phy {
    uint8_t all_phys; // bit-0 host has no tx phy prefer, bit-1 host has no rx phy prefer
    uint8_t tx_phys; // bit-0 LE 1M, bit-1 LE 2M, bit-2 LE Coded
    uint8_t rx_phys; // bit-0 LE 1M, bit-1 LE 2M, bit-2 LE Coded
} __attribute__ ((packed));
struct hci_le_set_default_phy_cmpl {
    uint8_t status;
} __attribute__ ((packed));
struct hci_le_set_phy {
    uint16_t conn_handle;
    uint8_t all_phys; // bit-0 host has no tx phy prefer, bit-1 host has no rx phy prefer
    uint8_t tx_phys; // bit-0 LE 1M, bit-1 LE 2M, bit-2 LE Coded
    uint8_t rx_phys; // bit-0 LE 1M, bit-1 LE 2M, bit-2 LE Coded
    uint16_t phy_options; // 0x00 host has no preferred coding when tx on LE Coded, 0x01 S=2, 0x02 S=8
} __attribute__ ((packed));
struct hci_ev_le_phy_update {
    uint8_t subcode;
    uint8_t status;
    uint16_t conn_handle;
    uint8_t tx_phy; // 0x01 LE 1M, 0x02 LE 2M, 0x03 LE Coded
    uint8_t rx_phy; // 0x01 LE 1M, 0x02 LE 2M, 0x03 LE Coded
} __attribute__ ((packed));
struct hci_le_read_tx_power_cmpl {
    uint8_t status;
    int8_t min_tx_power; // -127dBm to 20dBm
    int8_t max_tx_power; // -127dBm to 20dBm
} __attribute__ ((packed));
struct hci_le_read_adv_tx_power_cmpl {
    uint8_t status;
    int8_t tx_power_level; // -127dBm to 20dBm
} __attribute__ ((packed));
struct hci_le_read_local_tx_power {
    uint16_t conn_handle;
    uint8_t phy;
} __attribute__ ((packed));
struct hci_le_read_local_tx_power_cmpl {
    uint8_t status;
    uint16_t conn_handle;
    uint8_t phy;
    int8_t curr_tx_power; // -127dBm to 20dBm
    int8_t max_tx_power; // -127dBm to 20dBm
} __attribute__ ((packed));
struct hci_le_read_remote_tx_power {
    uint16_t conn_handle;
    uint8_t phy;
} __attribute__ ((packed));
struct hci_ev_le_tx_power_report {
    uint8_t subcode;
    uint8_t status;
    uint16_t conn_handle;
    uint8_t reason; // 0x00 local tx power changed, 0x01 remote, 0x02 host read remote tx power
    uint8_t phy;
    int8_t tx_power_level; // -127dBm to 20dBm
    uint8_t tx_power_flag; // bit-0 is at min level, bit-1 is at max level
    int8_t delta; // 0x7F change is not available or is out of range, 0xXX zero unchanged, positive increased, negative decreased
} __attribute__ ((packed));
struct hci_le_set_tx_power_report {
    uint16_t conn_handle;
    uint8_t local_enable;
    uint8_t remote_enable;
} __attribute__ ((packed));
struct hci_le_set_tx_power_report_cmpl {
    uint8_t status;
    uint16_t conn_handle;
} __attribute__ ((packed));
struct hci_le_frame_space_update {
    uint16_t conn_handle;
    /// The min frame space value, in microseconds. Range: 0x0000 to 0x2710
    uint16_t frame_space_min_us;
    /// The max frame space value, in microseconds. Range: 0x0000 to 0x2710
    uint16_t frame_space_max_us;
    /// 1M, 2M, Coded bits
    uint8_t phy_bits;
    /// 0x00 ~ 0x04 @see HCI_LE_SPACING_TYPE_T_IFS_ACL_CP as bit more
    uint16_t spacing_types_bits;
} __attribute__ ((packed));
struct hci_le_frame_space_update_cmpl {
    uint8_t subcode;
    uint8_t status;
    uint16_t conn_handle;
    /// 0x00 local host, 0x01 local controler, 0x02 peer @see HCI_LE_FS_UPD_INITIATOR_LOCAL_HOST more
    uint8_t initiator;
    /// The new frame space value being used, in microseconds. Range: 0x0000 to 0x2710
    uint16_t frame_space_us;
    /// 1M, 2M, Coded
    uint8_t phy;
    /// 0x00 ~ 0x04 @see HCI_LE_SPACING_TYPE_T_IFS_ACL_CP more
    uint16_t spacing_types;
} __attribute__ ((packed));

/**
 * If the connection is already encrypted then the Controller shall pause connection
 * encryption before attempting to authenticate the given encryption key, and then
 * re-encrypt the connection. While encryption is paused no user data shall be
 * transmitted. On an authentication failure, the connection shall be automatically
 * disconnected by the LL. If this command succeeds, then the connection shall be
 * encrypted.
 *
 * This command shall only be used when the local device's role is Central. The LE
 * LTK Request Reply and the LE LTK Request Negative Reply commands and the LE LTK
 * Request event shall only be used when the local device's role is Peripheral.
 *
 */
#define HCI_LE_ENABLE_ENCRYPTION 0x2019
struct hci_le_enable_encryption {
    uint16_t conn_handle;
    uint8_t Rand[8];
    uint8_t EDIV[2];
    uint8_t LTK[16];
} __attribute__ ((packed));
#define HCI_LE_LTK_REQUEST_REPLY 0x201A
struct hci_le_ltk_request_reply {
    uint16_t conn_handle;
    uint8_t LTK[16];
} __attribute__ ((packed));
#define HCI_LE_LTK_REQUEST_NEG_REPLY 0x201B
struct hci_le_ltk_request_neg_reply {
    uint16_t conn_handle;
} __attribute__ ((packed));
#define HCI_LE_EV_LTK_REQUEST 0x05
struct hci_ev_le_ltk_request {
    uint8_t subcode;
    uint16_t conn_handle;
    uint8_t Rand[8];
    uint8_t EDIV[2];
} __attribute__ ((packed));

/**
 * The Encryption Change Event will occur on both devices to notify the
 * Hosts when encryption has changed. This event shall not be generated
 * if encryption is paused or resumed.
 *
 * The Encryption Key Refresh Complete event is used to indicate to the
 * Host that the encryption key was refreshed on the given Connection_Handle
 * any time encryption is paused and then resumed.
 *
 */
#define HCI_READ_ENC_KEY_SIZE 0x1408
#define HCI_EV_LINK_KEY_NOTIFY  0x18
#define HCI_EV_ENCRYPTION_CHANGE_V1 0x08
#define HCI_EV_ENCRYPTION_CHANGE_V2 0x59
#define HCI_EV_ENCRYPT_KEY_REFRESH_COMPLETE 0x30
#define HCI_ENCRYPTION_OFF 0x00
#define HCI_BT_E0_LE_AES_CCM_ENC_ON 0x01
#define HCI_BT_AES_CCM_ENC_ON 0x02
#define HCI_KEY_TYPE_COMBINATION 0x00
#define HCI_KEY_TYPE_DEBUG_COMB  0x03
#define HCI_KEY_TYPE_UNAUTH_COMB 0x04
#define HCI_KEY_TYPE_AUTH_COMB   0x05
#define HCI_KEY_TYPE_CHGED_COMB  0x06
#define HCI_KEY_TYPE_SC_UNAUTH   0x07
#define HCI_KEY_TYPE_SC_AUTH     0x08
struct hci_ev_link_key_notify {
    struct bdaddr_t bdaddr;
    uint8_t link_key[16];
    uint8_t key_type;
}__attribute__ ((packed));
struct hci_ev_encryption_change_v1 {
    uint8_t status;
    uint16_t conn_handle;
    uint8_t encryption_enable; // 0x00 LL ENC OFF, 0x01 LL ENC ON with E0 for BR, AES-CCM for LE, 0x02 LL ENC ON with AES-CCM for BR
}__attribute__ ((packed));
struct hci_ev_encryption_change_v2 {
    uint8_t status;
    uint16_t conn_handle;
    uint8_t encryption_enable; // 0x00 LL ENC OFF, 0x01 LL ENC ON with E0 for BR, AES-CCM for LE, 0x02 LL ENC ON with AES-CCM for BR
    uint8_t encryption_key_size; // 0x01 to 0x10, this parameter shall be ignored for LE connections
}__attribute__ ((packed));
struct hci_ev_encryption_refresh {
    uint8_t status;
    uint16_t conn_handle;
}__attribute__ ((packed));
struct hci_read_enc_key_size {
    uint16_t conn_handle;
} __attribute__ ((packed));
struct hci_read_enc_key_size_cmpl {
    uint8_t status;
    uint16_t conn_handle;
    uint8_t key_size;
} __attribute__ ((packed));

/**
 * The HCI_LE_Encrypt is used to request the Controller to encrypt the Plaintext_data using
 * the Key and returns the Encrypted_Data to the host. The AES-128 bit block cypher is deinfed
 * in NIST Publication FIPS-197.
 *
 */
#define HCI_LE_ENCRYPT 0x2017
struct hci_le_encrypt {
    uint8_t key[16]; // 128 bit key, the most significant octet of the key corresponds to k[0] in FIPS 197
    uint8_t plaintext_data[16]; // 128 bit data block, the most significant octet corresponds to in[0] in FIPS 197
} __attribute__ ((packed));
struct hci_le_encrypt_cmpl {
    uint8_t status;
    uint8_t encrypted_data[16]; // 128 bit encrypted data block, the most significant octet corresponds to out[0] in FIPS 197
} __attribute__ ((packed));

/**
 * The HCI_LE_Rand is used to request the Controller to generate 8 ocetes of random
 * data to be sent to the Host. The Random_Number shall be generated according to
 * [Vol 2 BR/EDR Controller] Part H Security Spec, Section 2 if the LE Feature (LE
 * Encryption) is supported.
 *
 */
#define HCI_LE_RAND 0x2018
struct hci_le_rand_cmpl {
    uint8_t status;
    uint8_t random_number[8];
} __attribute__ ((packed));

/**
 * The HCI_LE_READ_LOCAL_P256_PUBLIC_KEY is used to return the local P256 public
 * key from the Controller. The Controller shall generate a new P256 public/private
 * key pair upon receipt of this command.
 *
 * The keys returned via this command shall not be used when Secure Connections
 * is used over the BR/EDR transport.
 *
 */
#define HCI_LE_READ_LOCAL_P256_PUBLIC_KEY 0x2025
#define HCI_LE_EV_READ_LOCAL_P256_PUB_KEY_CMPL 0x08
struct hci_ev_le_read_local_p256_pub_key_cmpl {
    uint8_t subcode;
    uint8_t status;
    uint8_t key_x_coord[32]; // local P256 public key X coordinate
    uint8_t key_y_coord[32]; // local P256 public key Y coordinate
} __attribute__ ((packed));

/**
 * The HCI_LE_Generate_DHKey is used to initiate generation of a Diffie-Hellman key
 * in the Controller for use over the LE transport. This command takes the remote
 * P256 public key specified in the Key_X_Coordinate and Key_Y_Coordinate as input.
 * The DHKey generation uses the private key generated by the HCI_LE_Rand_Local_P256_Public_Key
 * or the private debug key.
 *
 * The DHKey return via this command shall not be generated using any keys used for
 * Secure Connections over the BR/EDR transport.
 *
 * If the remote P256 public key is invalid, the Controller shall return an error
 * with Invalid HCI Command Parameters.
 *
 */
#define HCI_LE_GEN_DHKEY_V1 0x2026
#define HCI_LE_GEN_DHKEY_V2 0x205E
#define HCI_LE_EV_GEN_DHKEY_CMPL 0x09
struct hci_le_gen_dhkey_v1 {
    uint8_t key_x_coord[32];
    uint8_t key_y_coord[32]; // key_type: 0x00 use the generated private key
} __attribute__ ((packed));
struct hci_le_gen_dhkey_v2 {
    uint8_t key_x_coord[32]; // remote P256 public key X coordinate
    uint8_t key_y_coord[32]; // remote P256 public key Y coordinate
    uint8_t key_type; // 0x00 use the generated private key, 0x01 use the debug private key
} __attribute__ ((packed));
struct hci_ev_le_gen_dhkey_cmpl {
    uint8_t subcode;
    uint8_t status;
    uint8_t dhkey[32]; // if the remote p256 public key was invalid, all octets should be set to 0xFF
} __attribute__ ((packed));

/**
 * The HCI_LE_Set_CIG_Parameters command is used by a Central's Host to create a CIG and
 * to set the parameters of one or more CISes that are associated with a CIG in the Controller.
 * And the HCI_LE_Set_CIG_Parameters_Test command should only be used for testing purpose.
 *
 * The CIG_ID parameter identifies a CIG, and it is allocated by the Central's Host and
 * passed to Peripheral's Host through LL during the process of creating a CIS. If the
 * CIG_ID does not exist, then the Controller shall first create a new CIG. Once the CIG
 * is created (whether through this command or previously), the Controller shall modify
 * or add CIS configurations in the CIG that is identified by the CIG_ID and update all
 * the parameters that apply to the CIG.
 *
 * The CIS_ID parameter identifies a CIS, and it is set by the Central's Host and paased
 * to Peripheral's Host through LL during the process of establishing a CIS.
 *
 * If the Host issues this command when the CIG is not in the configurable state, the
 * Controller shall return the error code Command Disallowed (0x0C).
 *
 * If the Host attempts to create a CIG or set parameters that exceed the max supported
 * resources in the Controller, the Controller shall return the error code Memory
 * Capacity Exceeded (0x07).
 *
 * If the Host attempts to set CIS parameters that exceed the maximum supported connections
 * in the Controller, the Controller shall return the error code Connection Limit
 * Exceeded (0x09).
 *
 * If the Host sets the PHY a bit that the Controller does not support, including a bit that
 * is reserved for future use, the Controller shall return error code Unsupported Feature
 * ore Parameter Value (0x11).
 *
 * If the Controller does not support asymmetric PHYs and the Host sets phy_c2p to a different
 * value than phy_p2c, the Controller shall return the error code Unsupported Feature or
 * Parameter Value (0x11).
 *
 * If the Host specifies an invalid combination of CIS parameters, the Controller shall
 * return the error code Unsupported Feature or Parameter Value (0x11).
 *
 */
#define HCI_LE_SET_CIG_PARAMETERS 0x2062
#define HCI_LE_SET_CIG_PARAMETERS_TEST 0x2063
struct hci_le_cis_configure {
    uint8_t cis_id; // 0x00 to 0xEF, used to identify a CIS
    uint16_t max_sdu_c2p; // 0x0000 to 0x0FFFF, max octets of the sdu payload from C host
    uint16_t max_sdu_p2c; // 0x0000 to 0x0FFFF, max octets of the sdu payload from P Host
    uint8_t phy_c2p; // bit 0 - C TX PHY is LE 1M, bit 1 - LE 2M, bit 2 - LE Coded, host shall set at least one bit
    uint8_t phy_p2c; // bit 0 - P TX PHY is LE 1M, bit 1 - LE 2M, bit 2 - LE Coded, host shall set at least one bit
    uint8_t rtn_c2p; // retransmission number of every CIS Data PDU from C to P before ack or flushed, just recommendation
    uint8_t rtn_p2c; // retransmission number of every CIS Data PDU from P to C before ack or flushed, just recommendation
} __attribute__ ((packed));
struct hci_le_set_cig_parameters {
    uint8_t cig_id; // 0x00 to 0xEF, used to identify the CIG
    uint8_t sdu_interval_c2p[3]; // us, 0xFF to 0x0F_FFFF, SDU interval from the C Host for all CISes in CIG
    uint8_t sdu_interval_p2c[3]; // us, 0xFF to 0x0F_FFFF, SDU interval from the P host for all CISes in CIG
    uint8_t worst_case_sca; // worst case sleep clock accuracy of all the Peripherals that will participate in the CIG
    uint8_t packing; // 0x00 sequential, 0x01 interleaved, this is a recommendation the controller may ignore
    uint8_t framing; // 0x00 unframed, 0x01 framed (Segmentable mode), Core Spec 6.0: 0x02 framed (Unsegmented mode)
    uint16_t max_transport_latency_c2p; // ms, 0x05 to 0x0FA0, max transport latency from C btc to P btc
    uint16_t max_transport_latency_p2c; // ms
    uint8_t cis_count; // 0x00 to 0x1F, total number of CIS configurations in the CIG being added or modified
    struct hci_le_cis_configure cis_cfg[1];
} __attribute__ ((packed));
struct hci_le_cis_configure_test {
    uint8_t cis_id; // 0x00 to 0xEF, used to identify a CIS
    uint8_t nse; // 0x01 to 0x1F, max number of subevents in each CIS event
    uint16_t max_sdu_c2p; // 0x0000 to 0x0FFFF, max octets of the sdu payload from C host
    uint16_t max_sdu_p2c; // 0x0000 to 0x0FFFF, max octets of the sdu payload from P Host
    uint16_t max_pdu_c2p; // 0x00 to 0xFB, max octets of the pdu payload from C LL to P LL
    uint16_t max_pdu_p2c; // 0x00 to 0xFB, max octets of the pdu payload from P LL to C LL
    uint8_t phy_c2p; // bit 0 - C TX PHY is LE 1M, bit 1 - LE 2M, bit 2 - LE Coded, host shall set at least one bit
    uint8_t phy_p2c; // bit 0 - P TX PHY is LE 1M, bit 1 - LE 2M, bit 2 - LE Coded, host shall set at least one bit
    uint8_t bn_c2p; // 0x00 no ISO data from C to P, 0x01 to 0x0F BN for C to P transmission
    uint8_t bn_p2c; // 0x00 no ISO data from P to C, 0x01 to 0x0F BN for P to C transmission
} __attribute__ ((packed));
struct hci_le_set_cig_parameters_test {
    uint8_t cig_id; // 0x00 to 0xEF, used to identify the CIG
    uint8_t sdu_interval_c2p[3]; // us, 0xFF to 0x0F_FFFF, SDU interval from the C Host for all CISes in CIG
    uint8_t sdu_interval_p2c[3]; // us, 0xFF to 0x0F_FFFF, SDU interval from the P host for all CISes in CIG
    uint8_t ft_c2p; // 0x01 to 0xFF, flush timeout in multiples of ISO_Interval for each payload sent from C to P
    uint8_t ft_p2c; // 0x01 to 0xFF, flush timeout in multiples of ISO_Interval for each payload sent from P to C
    uint16_t iso_interval; // 0x04 to 0x0C80, per 1.25ms, 5ms to 4s, CIS anchor points interval
    uint8_t worst_case_sca; // worst case sleep clock accuracy of all the Peripherals that will participate in the CIG
    uint8_t packing; // 0x00 sequential, 0x01 interleaved, this is a recommendation the controller may ignore
    uint8_t framing; // 0x00 unframed, 0x01 framed (Segmentable mode), Core Spec 6.0: 0x02 framed (Unsegmented mode)
    uint8_t cis_count; // 0x00 to 0x1F, total number of CIS configurations in the CIG being added or modified
    struct hci_le_cis_configure_test cis_cfg[1];
} __attribute__ ((packed));
struct hci_le_set_cig_params_cmpl {
    uint8_t status;
    uint8_t cig_id; // 0x00 to 0xEF
    uint8_t cis_count;  // 0x00 to 0x1F
    uint16_t cis_handle[1];
} __attribute__ ((packed));

/**
 * The HCI_LE_Create_CIS command is used by the Central's Host to create
 * one or more CISes.
 *
 * The CIS_Connection_Handle parameter specifies the connection handle
 * corresponding to the configuration of the CIS to be created and whose
 * configuration is already stored in a CIG.
 *
 * The ACL_Connection_Handle parameter specifies the connection handle
 * of the ACL associated with each CIS to be created.
 *
 * If the Host issue this command before all the HCI_LE_CIS_Established
 * events from the previous use of the command have been generated, the
 * Controller shall return the error code Command Disallowed (0x0C).
 *
 * If the Host issues this command when the CIS (Host Support) feature
 * bit is not set, the Controller shall return the error code Command
 * Disallowed (0x0C).
 *
 * When the Controller receives this command, the Controller sends the
 * HCI_Command_Status event to the Host. An HCI_LE_CIS_Established event
 * will be generated for each CIS when it is established or if it is
 * disconnected or considered lost before being established; until all
 * the events are generated, the command remains pending.
 *
 * The HCI_LE_CIS_Established event indicates that a CIS has been established,
 * was considered lost before being established, or -- on the Central -- was
 * rejected by the Peripheral. It is generated by the Controller in the
 * Central and Peripheral.
 *
 * If HCI_LE_CIS_REQUEST is rejected by Peripheral, only HCI_Command_Complete
 * event is generated, there is no HCI_LE_CIS_Established event in Peripheral
 * side.
 *
 */
#define HCI_LE_CREATE_CIS           0x2064
struct hci_le_create_cis_item {
    uint16_t cis_handle;
    uint16_t acl_handle;
} __attribute__ ((packed));
struct hci_le_create_cis {
    uint8_t cis_count;
    struct hci_le_create_cis_item cis_item[1];
} __attribute__ ((packed));

#define HCI_LE_EV_CIS_REQUEST 0x1A
struct hci_ev_le_cis_request {
    uint8_t subcode;
    uint16_t acl_handle;
    uint16_t cis_handle;
    uint8_t cig_id;
    uint8_t cis_id;
} __attribute__ ((packed));

#define HCI_LE_ACCEPT_CIS_REQUEST   0x2066
#define HCI_LE_REJECT_CIS_REQUEST   0x2067
struct hci_le_accept_cis_request {
    uint16_t cis_handle;
} __attribute__ ((packed));
struct hci_le_reject_cis_request {
    uint16_t cis_handle;
    uint8_t reason;
} __attribute__ ((packed));
struct hci_le_reject_cis_request_cmpl {
    uint8_t status;
    uint16_t cis_handle;
} __attribute__ ((packed));

#define HCI_LE_EV_CIS_ESTABLISH_V1 0x19
#define HCI_LE_EV_CIS_ESTABLISH_V2 0x2A
struct hci_ev_le_cis_establish {
    uint8_t subcode;
    uint8_t status;
    uint16_t cis_handle;
    uint8_t cig_sync_delay[3]; // 0xEA to 0x7F_FFFF, max us for transmission of PDUs of all CISes in a CIG event
    uint8_t cis_sync_delay[3]; // 0xEA to 0x7F_FFFF, max us for transmission of PDUs of the specified CIS in a CIG event
    uint8_t transport_latency_c2p[3]; // us, 0xEA to 0x7F_FFFF, the actual transport latency us from C btc to P btc
    uint8_t transport_latency_p2c[3]; // us
    uint8_t phy_c2p;
    uint8_t phy_p2c;
    uint8_t nse; // 0x01 to 0x1F, max number of subevents in each ISO event (CIS event)
    uint8_t bn_c2p; // 0x00 no ISO data from C to P, 0x01 to 0x0F BN for C to P transmission
    uint8_t bn_p2c; // 0x00 no ISO data from P to C, 0x01 to 0x0F BN for P to C transmission
    uint8_t ft_c2p; // 0x01 to 0xFF, flush timeout in multiples of ISO_Interval for each payload sent from C to P
    uint8_t ft_p2c; // 0x01 to 0xFF
    uint16_t max_pdu_c2p; // 0x00 to 0xFB, max octets of the pdu payload from C LL to P LL
    uint16_t max_pdu_p2c; // 0x00 to 0xFB
    uint16_t iso_interval; // 0x04 to 0x0C80, per 1.25ms, 5ms to 4s, CIS anchor points interval
} __attribute__ ((packed));

struct hci_ev_le_cis_establish_v2 {
    uint8_t subcode;
    uint8_t status;
    uint16_t cis_handle;
    uint8_t cig_sync_delay[3]; // 0xEA to 0x7F_FFFF, max us for transmission of PDUs of all CISes in a CIG event
    uint8_t cis_sync_delay[3]; // 0xEA to 0x7F_FFFF, max us for transmission of PDUs of the specified CIS in a CIG event
    uint8_t transport_latency_c2p[3]; // us, 0xEA to 0x7F_FFFF, the actual transport latency us from C btc to P btc
    uint8_t transport_latency_p2c[3]; // us
    uint8_t phy_c2p;
    uint8_t phy_p2c;
    uint8_t nse; // 0x01 to 0x1F, max number of subevents in each ISO event (CIS event)
    uint8_t bn_c2p; // 0x00 no ISO data from C to P, 0x01 to 0x0F BN for C to P transmission
    uint8_t bn_p2c; // 0x00 no ISO data from P to C, 0x01 to 0x0F BN for P to C transmission
    uint8_t ft_c2p; // 0x01 to 0xFF, flush timeout in multiples of ISO_Interval for each payload sent from C to P
    uint8_t ft_p2c; // 0x01 to 0xFF
    uint16_t max_pdu_c2p; // 0x00 to 0xFB, max octets of the pdu payload from C LL to P LL
    uint16_t max_pdu_p2c; // 0x00 to 0xFB
    uint16_t iso_interval; // 0x04 to 0x0C80, per 1.25ms, 5ms to 4s, CIS anchor points interval
    uint8_t sub_interval[3]; // 0x000000, NSE = 1, 0x000190 to ISO_Interval×1250 – 1 in us
    uint16_t max_sdu_c2p; // 0 to 0x0FFF
    uint16_t max_sdu_p2c; // 0 to 0x0FFF
    uint8_t sdu_interval_c2p[3]; // 0x0000FF to 0x0FFFFF in us
    uint8_t sdu_interval_p2c[3]; // 0x0000FF to 0x0FFFFF in us
    uint8_t framing; // 0x00 Unframed, 0x01 Framed
} __attribute__ ((packed));

/**
 * The HCI_LE_Remove_CIG command is used by the Central's Host to remove the CIG identified
 * by CIG_ID. This command shall delete the CIG_ID and also delete the Connection_Handles of
 * the CIS configurations stored in the CIG.
 *
 * This command shall also remove the ISO data paths that are associated with the Connection_Handles
 * of the CIS configurations, which is equivalent to issuing the HCI_LE_Remove_ISO_Data_Path command.
 *
 * If the Host tries to remove a CIG which is in the active state, then the Controller shall
 * return the error code Command Disablled (0x0C). So before call this command, all CISes in
 * the CIG shall be disconnected using HCI_Disconnect command issued by Central or Peripheral.
 *
 * If the Host issues this command with a CIG_ID that does not exist, the Controller shall return
 * the error code Unknown Connection Identifier (0x02).
 *
 */
#define HCI_LE_REMOVE_CIG           0x2065
struct hci_le_remove_cig {
    uint8_t cig_id;
} __attribute__ ((packed));
struct hci_le_remove_cig_cmpl {
    uint8_t status;
    uint8_t cig_id;
} __attribute__ ((packed));

/**
 * The HCI_LE_Setup_ISO_Data_Path command is used to identify and create the
 * ISO data path between the Host and the Controller for a CIS, CIS configuration,
 * or BIS identified by the Connection_Handle parameter.
 *
 * The input and output directions are defined from the perspective of the
 * Controller, so "input" refers to data flowing from the Host to Controller.
 *
 * When Data_Path_Direction is set to 0x00 (input, host to controller direction),
 * the Controller_Delay parameter specifies the delay at the data source from the
 * reference time of an SDU to the CIG reference point or BIG anchor point.
 * When Data_Path_Direction is set to 0x01 (output, controller to host direction),
 * Controller_dealy specifies the delay from the SDU_Synchronization_Reference to
 * the point in time at which the Controller begins to transfer the corresponding data
 * to the data path interface. The Host should use the HCI_Read_Local_Supported_Controller_Dealy
 * command to obtain a suitable value for Controller_Delay.
 *
 * Note: Controller vendors may provides additional guidance to the Host on how
 * to select a suitable Controller_Delay value from the range of values provided
 * by the HCI_Read_Local_Supported_Controller_Delay command for various configurations
 * of the data path interface.
 *
 * The Min_Controller_Delay and Max_Controller_Delay parameters returned by the Controller
 * provide a range of allowed values to be used by the Host when issuing the
 * HCI_LE_Setup_ISO_Data_Path command. The Min_Controller_Delay shall be greater than or
 * equal to the codec processing delay for the specified direction and codec configuration.
 * The Max_Controller_delay shall be less than or equal to the sum of the codec processing
 * delay and the max time the Controller can buffer the data for the specified directon
 * and codec configuration.
 *
 * If the Host issues this command more than once for the same Connection_Handle
 * and direction before issuing HCI_LE_Remove_ISO_Data_Path command for that
 * Connection_Handle and direction, the Controller shall return the error code
 * Command Disallowed (0x0C).
 *
 * If the Host issues this command for a CIS on a Peripheral before it has issued
 * the HCI_LE_Accept_CIS_Request for that CIS, then the Controller shall return
 * the error code Command Disallowed (0x0C).
 *
 * The Data_Path_ID parameter specifies the data transport path used. When set
 * to 0x00, the data path shall be over the HCI transport. When set to a value in
 * the range 0x01 to 0xFE, the data path shall use a vendor-specific transport
 * interface (e.g., a PCM interface) with logical transport numbers. The meanings
 * of these logical transport numbers are vendor-specific.
 *
 * If the Host issues this command for a vendor-specific data transport path that
 * has not been configured using the HCI_Configure_Data_Path command, the Controller
 * shall return the error code Command Disallowed (0x0C).
 *
 * If the Host attempts to set a data path with a Connection Handle that does not
 * exist or that is not for a CIS, CIS configuration, or BIS, the Controller shall
 * return the error code Unknown Connection Identifier (0x02).
 *
 * If the Host attempts to set an output data path using a connection handle that
 * is for an ISO Broadcaster, for an input data path on a Synchroized Receiver,
 * or for a data path for the direction on a unidirectional CIS where BN is set
 * to 0, the Controller shall return the error code Command Disabllowed (0x0C).
 *
 * If the Host issues this command with Codec_Configuration_Length non-zero and
 * Codec_ID set to transparent air mode, the Controller shall return the error
 * codec Invalid HCI Command Parameters (0x12).
 *
 * If the Host issues this command with codec-related parameters that exceed the
 * bandwidth and latency allowed on the established CIS or BIS identified by
 * the Connection Handle paramerter, the Controller shall return the error code
 * Invalid HCI Command Parameters (0x12).
 *
 */
typedef enum {
    HCI_ISO_CODEC_U_LAW_LOG = 0x00,
    HCI_ISO_CODEC_A_LAW_LOG = 0x01,
    HCI_ISO_CODEC_CVSD = 0x02,
    HCI_ISO_CODEC_TRANSPARENT = 0x03, // indicates that the controller does not do any transcoding or resampling
    HCI_ISO_CODEC_LINEAR_PCM = 0x04,
    HCI_ISO_CODEC_MSBC = 0x05,
    HCI_ISO_CODEC_LC3 = 0x06,
    HCI_ISO_CODEC_G729A = 0x07,
    HCI_ISO_CODEC_VENDOR = 0xFF,
} hci_iso_codec_t;

typedef enum {
    HCI_CODEC_DT_BT_ACL = 0x01,
    HCI_CODEC_DT_BT_SCO = 0x02,
    HCI_CODEC_DT_LE_CIS = 0x04,
    HCI_CODEC_DT_LE_BIS = 0x08,
} hci_codec_data_transport_t;

typedef enum {
    HCI_LOGICAL_TRANSPORT_BT_ACL = 0x00,
    HCI_LOGICAL_TRANSPORT_BT_SCO = 0x01,
    HCI_LOGICAL_TRANSPORT_LE_CIS = 0x02,
    HCI_LOGICAL_TRANSPORT_LE_BIS = 0x03,
} hci_logical_transport_t;

typedef enum {
    HCI_ISO_DATA_DIR_HOST_TX = 0x00,
    HCI_ISO_DATA_DIR_HOST_RX = 0x01,
} hci_iso_data_dir_t;

typedef enum {
    HCI_ISO_DATA_DIR_BIT_NONE = 0x00,
    HCI_ISO_DATA_DIR_BIT_HOST_TX = 0x01,
    HCI_ISO_DATA_DIR_BIT_HOST_RX = 0x02,
} hci_iso_data_dir_bit_t;

typedef enum {
    HCI_ISO_DATA_ID_OVER_HCI = 0x00,
} hci_iso_data_id_t;

#define HCI_READ_LOCAL_SUPP_CODECS      0x100B // read local controller supported codecs
#define HCI_READ_LOCAL_SUPP_CODESC_V2   0x100D
struct hci_local_standard_codec_item {
    uint8_t standard_codec_id;
    uint8_t codec_transport; // bit 0 BR/EDR ACL, bit 1 BR/EDR SCO/eSCO, bit 2 LE CIS, bit 3 LE BIS
} __attribute__ ((packed));
struct hci_local_vendor_codec_item {
    uint8_t vendor_codec_id[4]; // octets 0 to 1: company id, octets 2 to 3: vendor-defined codec id
    uint8_t codec_transport; // bit 0 BR/EDR ACL, bit 1 BR/EDR SCO/eSCO, bit 2 LE CIS, bit 3 LE BIS
} __attribute__ ((packed));
struct hci_read_local_supp_codecs_v2_cmpl { // v2 shall return codecs supported on all physical transports
    uint8_t status;
    uint8_t num_supp_standard_codecs;
    struct hci_local_standard_codec_item standard_codec[1];
} __attribute__ ((packed));
struct hci_read_local_supp_codec_v2_tail {
    uint8_t num_supp_vendor_codecs;
    struct hci_local_vendor_codec_item vendor_codec[1];
} __attribute__ ((packed));
struct hci_read_local_supp_codecs_cmpl { // v1 only return codec supported on the BR/EDR physical transport
    uint8_t status;
    uint8_t num_supp_standard_codecs;
    uint8_t standard_codec_id[1];
} __attribute__ ((packed));
struct hci_read_local_supp_codec_tail {
    uint8_t num_supp_vendor_codecs;
    uint8_t vendor_codec_id[4];
} __attribute__ ((packed));

#define HCI_READ_LOCAL_SUPP_CODEC_CAPABILITIES  0x100E
struct hci_read_local_supp_codec_capabilities {
    uint8_t codec_id[5]; // octet 0: coding format, octet 1 to 2: company id, octet 3 to 4: vendor-defined codec id
    uint8_t logical_transport_type; // 0x00 BT ACL, 0x01 BT SCO, 0x02 LE CIS, 0x03 LE BIS
    uint8_t direction; // 0x00 input (host to controller), 0x01 output (controller to host)
} __attribute__ ((packed));
struct hci_codec_capability_t {
    uint8_t codec_capability_length;
    uint8_t codec_capability[1];
} __attribute__ ((packed));
struct hci_read_local_supp_codec_capabilities_cmpl {
    uint8_t status;
    uint8_t num_codec_capabilities;
    struct hci_codec_capability_t codec_capability[1];
} __attribute__ ((packed));

#define HCI_READ_LOCAL_SUPP_CONTROLLER_DELAY    0x100F
struct hci_read_local_controller_dealy {
    uint8_t codec_id[5]; // octet 0: coding format, octet 1 to 2: company id, octet 3 to 4: vendor-defined codec id
    uint8_t logical_transport_type; // 0x00 BT ACL, 0x01 BT SCO, 0x02 LE CIS, 0x03 LE BIS
    uint8_t direction; // 0x00 input (host to controller), 0x01 output (controller to host)
    uint8_t codec_configuration_length;
    uint8_t codec_configuration[1]; // codec-specific configuration data
} __attribute__ ((packed));
struct hci_read_local_controller_dealy_cmpl {
    uint8_t status;
    uint8_t min_controller_delay[3]; // 0x00 to 0x3D0900, 0s to 4s, min controller delay in us
    uint8_t max_controller_delay[3]; // 0x00 to 0x3D0900, 0s to 4s, max controller delay in us
} __attribute__ ((packed));

#define HCI_CONFIGURE_DATA_PATH     0x0C83
struct hci_configure_data_path { // request the Controller to config the vendor transport data path in a give dir
    uint8_t data_path_direction; // 0x00 input (host to controller), 0x01 output (controller to host)
    uint8_t vendor_data_path_id; // 0x01 to 0xFE iso over vendor-specific transport, the meaning is vendor-specific
    uint8_t vendor_config_length;
    uint8_t vendor_specific_config[1]; // additional vendor info that the Host may provided to Controller
} __attribute__ ((packed));
struct hci_configure_data_path_cmpl {
    uint8_t status;
} __attribute__ ((packed));

#define HCI_LE_SETUP_ISO_DATA_PATH  0x206E
#define HCI_LE_REMOVE_ISO_DATA_PATH 0x206F
struct hci_le_setup_iso_data_path {
    uint16_t iso_handle; // connection handle of the CIS or BIS
    uint8_t data_path_direction; // 0x00 input (host to controller), 0x01 output (controller to host)
    uint8_t data_path_id; // 0x00 iso over hci, 0x01 to 0xFE iso over vendor-specific transport
    uint8_t codec_id[5]; // octet 0: coding format, octet 1 to 2: company id, octet 3 to 4: vendor-defined codec id
    uint8_t controller_delay[3]; // 0x00 to 0x3D0900, 0s to 4s, controller delay in us
    uint8_t codec_configuration_length;
    uint8_t codec_configuration[1]; // codec-specific configuration data
} __attribute__ ((packed));
struct hci_le_setup_iso_data_path_cmpl {
    uint8_t status;
    uint16_t iso_handle;
} __attribute__ ((packed));
struct hci_le_remove_iso_data_path {
    uint16_t iso_handle;
    uint8_t data_path_direction; // bit 0 input, bit 1 output
} __attribute__ ((packed));
struct hci_le_remove_iso_data_path_cmpl {
    uint8_t status;
    uint16_t iso_handle;
} __attribute__ ((packed));

#define HCI_DBG_ISO_QUALITY_REPORT_EVT_SUBCODE 0x26
struct hci_ev_le_iso_quality_report {
    uint8_t subcode;
    uint16_t iso_handle;
    uint32_t tx_unacked_packets;
    uint32_t tx_flushed_packets;
    uint32_t tx_last_subevent_packets;
    uint32_t retransmitted_packets;
    uint32_t crc_error_packets;
    uint32_t rx_unreceived_packets;
    uint32_t duplicate_packets;
} __attribute__ ((packed));

/**
 * The HCI_LE_Create_BIG command is used to create a BIG with one or more BISes. All BISes
 * in a BIG have the same value for all parameters. The HCI_LE_Create_BIG_Test command
 * should only be used for testing purpose.
 *
 * The BIG_Handle is the identifier of the BIG and it is allocated by the Host and used by
 * the Controller and the Host to identify a BIG. the Advertising_Handle identifies the
 * associated pa train of the BIG.
 *
 * If the Advertising_Handle does not identifiy a PA train or the PA train is associated
 * with another BIG, the Controller shall return the error code Unknown Advertising Identifier (0x42).
 *
 * If the Host issues this command with a BIG_Handle for a BIG that is already created, the
 * Controller shall return the error code Command Disallowed (0x0C).
 *
 * If the Host specifies an invalid combination of BIG parameters, the Controller shall return
 * an error Unsupporte Feature or Parameter Value (0x11).
 *
 * The HCI_LE_Terminate_BIG command is used to terminate a BIG identified by the BIG_Handle.
 * The command also terminates the transmission of all BISes of the BIG, destroys the associated
 * connection handles of the BISes in the BIG and removes the data paths for all BISes in the BIG.
 *
 * If the BIG_Handle does not identify a BIG, the Controller shall return the error code
 * Unknown Advertising Identifier (0x42). If the Controller is not the ISO Broadcaster for the
 * BIG identifified by BIG_Handle, the Controller shall return the error code Command Disallowed (0x0C).
 *
 */
#define HCI_LE_CREATE_BIG               0x2068
#define HCI_LE_CREATE_BIG_TEST          0x2069
struct hci_le_create_big {
    uint8_t big_handle; // 0x00 to 0xEF, used to identify the BIG
    uint8_t adv_handle; // 0x00 to 0xEF, used to identify the pa train
    uint8_t num_bis; // 0x01 to 0x1F, total number of BISes in the BIG
    uint8_t sdu_interval[3]; // 0xFF to 0x0F_FFFF, the interval in us of periodic SDUs
    uint16_t max_sdu; // 0x01 to 0x0FFFF, max octets of an SDU
    uint16_t max_transport_latency; // 0x05 to 0x0FA0, max transport latency in us, this includes pre-transmissions
    uint8_t rtn; // 0x00 to 0x1E, retransmition number of every BIS data PDU, just recommendation Controller may ignore
    uint8_t phy; // bit 0: transmitter phy is LE 1M, bit 1: LE 2M, bit 2: LE Coded, Host shall set at least one bit
    uint8_t packing; // 0x00 sequential, 0x01 interleaved, just recommendation
    uint8_t framing; // 0x00 unframed, 0x01 framed
    uint8_t encryption; // 0x00 unencrypted, 0x01 encrypted
    uint8_t broadcast_code[16]; // used to derive the session key that is used to encrypt and decrpt BIS payloads
} __attribute__ ((packed));
struct hci_le_create_big_test {
    uint8_t big_handle; // 0x00 to 0xEF, used to identify the BIG
    uint8_t adv_handle; // 0x00 to 0xEF, used to identify the pa train
    uint8_t num_bis; // 0x01 to 0x1F, total number of BISes in the BIG
    uint8_t sdu_interval[3]; // 0xFF to 0x0F_FFFF, the interval in us of periodic SDUs
    uint16_t iso_interval; // 0x04 to 0x0C80, per 1.25ms, 5ms to 4s, interval between BIG anchor points
    uint8_t nse; // 0x01 to 0x1F, total number of subevents in each interval of each BIS in the BIG
    uint16_t max_sdu; // 0x01 to 0x0FFFF, max octets of an SDU
    uint16_t max_pdu; // 0x01 to 0xFB, max octets of PDU payload
    uint8_t phy; // bit 0: transmitter phy is LE 1M, bit 1: LE 2M, bit 2: LE Coded
    uint8_t packing; // 0x00 sequential, 0x01 interleaved
    uint8_t framing; // 0x00 unframed, 0x01 framed
    uint8_t bn; // 0x01 to 0x07, number of new payloads in each interval for each BIS
    uint8_t irc; // 0x01 to 0x0F, number of times the scheduled payloads are transmitted in a given event
    uint8_t pto; // 0x00 to 0x0F, offset used for pre-transmissions
    uint8_t encryption; // 0x00 unencrypted, 0x01 encrypted
    uint8_t broadcast_code[16]; // used to derive the session key that is used to encrypt and decrpt BIS payloads
} __attribute__ ((packed));

#define HCI_LE_EV_CREATE_BIG_COMPLETE 0x1B
struct hci_ev_le_create_big_complete {
    uint8_t subcode;
    uint8_t status;
    uint8_t big_handle; // 0x00 to 0xEF, the identifier of the BIG
    uint8_t big_sync_delay[3]; // 0xEA to 0x7F_FFFF, max time in us for transmission of PDUs of all BISes in a BIG event
    uint8_t transport_latency_big[3]; // 0xEA to 0x7F_FFFF, actual transport latency, in us
    uint8_t phy; // 0x01 the phy used to create the BIG is LE 1M, 0x02 LE 2M, 0x03 LE Coded
    uint8_t nse; // 0x01 to 0x1F, num of subevents in each BIS event in the BIG
    uint8_t bn; // 0x01 to 0x07, the number of new payloads in each BIS event
    uint8_t pto; // 0x00 to 0x0F, offset used for pre-transmissions
    uint8_t irc; // 0x01 to 0x0F, num of times a payload is transmitted in a BIS event
    uint16_t max_pdu; // 0x01 to 0xFB, max octets of the PDU payload
    uint16_t iso_interval; // 0x04 to 0x0C80, per 1.25ms, 5ms to 4s, BIG anchor points interval
    uint8_t num_bis; // 0x01 to 0x1F, total number of BISes in the BIG
    uint16_t bis_handle[1];
} __attribute__ ((packed));

#define HCI_LE_TERMINATE_BIG            0x206A
struct hci_le_terminate_big {
    uint8_t big_handle;
    uint8_t reason;
} __attribute__ ((packed));

#define HCI_LE_EV_TERMINATE_BIG_COMPLETE 0x1C
struct hci_ev_le_terminate_big_complete {
    uint8_t subcode;
    uint8_t big_handle;
    uint8_t reason;
} __attribute__ ((packed));

/**
 * The HCI_LE_BIG_Create_Sync command is used to sync to a BIG described in the pa train
 * specified by the Sync_Handle parameter.
 *
 * The BIG_Handle parameter is assigned by the Host to identify the synchronized BIG.
 * If the Host sends this command with a BIG_Handle that is already allocated, the Controller
 * shall return the error code Command Disallowed (0x0C).
 *
 * If the Encryption paramter set by the Host does not match the encryption status of
 * the BIG, the Controller shall return the error Encryption Mode Not Acceptable (0x25).
 *
 * The MSE (Maximum Subevents) parameter is the max number of subevents that a Controller
 * should use to receive data payloads in each interval for a BIS.
 *
 * The BIG_Sync_Timeout parameter specifies the max permitted time between successful
 * receptions of BIS PDUs. If the time is exceeded, sync is lost. When the Controller
 * establishes sync and if the BIG_Sync_Timeout set by the Host is less than 6 * ISO_Interval,
 * the Controller shall set the timeout to 6 * ISO_Interval.
 *
 * The number of BISes requested may be a subset of the BISes available in the BIG. If
 * the Num_BIS parameter is greater than the total number of BISes in the BIG, the
 * Controller shall return the error code Unsupported Feature or Parameter Value (0x11).
 *
 * The BIS_Index is a list of indices corresponding to BISes in the synchronized BIG.
 * The list of BIS indices shall be in ascending order and shall not contain any duplicates.
 * This list of specified BISes may be all or a subset of BISes available in the BIG.
 *
 * If the Sync_Handle does not exist, the Controller shall return the error code Unknown
 * Advertising Identifier (0x42).
 *
 * If the info describing the BIG does not specify a PHY supported by the Controller or
 * does not specify exactly one PHY, the Controller shall return the error code
 * Unsupported Feature or Parameter Value (0x11).
 *
 * If the Host sends this command when the Controller is in the process of synchronizing to
 * any BIG. i.e., a previous HCI_LE_BIG_Create_Sync is sent but the HCI_LE_BIG_Sync_Established
 * event has not been generated yet, the Controller shall return the error code Command
 * Disallowed (0x0C).
 *
 * If the Controller is unalbe to receive PDUs from the specified number of BISes in
 * the syncronized BIG, it shall return the error code Connection Rejected Due to Limited
 * Resources (0x0D).
 *
 * The HCI_LE_BIG_Terminate_Sync command is used to stop synchronizing or cancel the process
 * of synchronizing to the BIG identified by the BIG Handle parameter. The command also
 * terminates the reception of BISes in the BIG specified in the HCI_LE_BIG_Create_Sync,
 * destroys the associated connection handles of the BISes in the BIG and removes the data
 * paths for all BISes in the BIG.
 *
 * The HCI_LE_BIG_Sync_Lost event indicates that the Controller has not received any PDUs
 * on a BIG within the timeout period BIG_Sync_Timeout or the BIS has been terminated by
 * the remote device.
 *
 * The Reason parameter is used to indicate the reason why the synchronization was lost
 * or terminated. If synchronization was terminated due to the Broadcaster terminating the
 * BIG, the Reason shall be set to Remote User Terminated Connection (0x13). If synchronization
 * was terminated due to a timeout, the Reason shall be set to Connection Timeout (0x08).
 * If the synchronization was terminated due to a MIC failure, the Reason shall be set to
 * Connection Terminated due to MIC Failure (0x3D).
 *
 * When the HCI_LE_BIG_Sync_Lost event occurs, the Controller shall remove the connection
 * handles and data paths of all BISes in the BIG with which the Controller was synchronized.
 *
 */
#define HCI_LE_EV_BIG_INFO_ADV_REPORT 0x22
struct hci_ev_le_big_info_adv_report { // shall be generated even if the Controller already sync to the BIG
    uint8_t subcode;
    uint16_t sync_handle; // 0x0000 to 0x0EFE, sync_handle identifying the pa train
    uint8_t num_bis; // 0x01 to 0x1F, value of the Num_BIS subfield of the BIGInfo field
    uint8_t nse; // 0x01 to 0x1F, value of the NSE subfield of the BIGInfo field
    uint16_t iso_interval; // iso_interval subfield of the BIGInfo field
    uint8_t bn; // 0x01 to 0x07, BN subfield of the BIGInfo field
    uint8_t pto; // 0x00 to 0x0F, pre-transmission offset, PTO subfield of the BIGInfo field
    uint8_t irc; // 0x01 to 0x0F, IRC subfield of the BIGInfo field
    uint16_t max_pdu; // 0x01 to 0xFB, max_pdu subfiled of the BIGInfo
    uint8_t sdu_interval[3]; // 0xFF to 0x0F_FFFF, sdu_interval subfield of the BIGInfo field
    uint16_t max_sdu; // 0x01 to 0x0FFF, max_sdu subfield of the BIGInfo
    uint8_t phy; // 0x01 the BIG is transmitted on the LE 1M PHY, 0x02 LE 2M, 0x03 LE Coded
    uint8_t framing; // 0x00 unframed 0x01 framed
    uint8_t encrypt; // 0x00 BIG carries unencrypted data, 0x01 BIG carries encrypted data
} __attribute__ ((packed));

#define HCI_LE_BIG_CREATE_SYNC          0x206B
struct hci_le_big_create_sync {
    uint8_t big_handle; // 0x00 to 0xEF, used to identify the BIG
    uint16_t sync_handle; // 0x0000 to 0x0EFF, identifier of the pa train
    uint8_t encryption; // 0x00 unencrypted Broadcast_Code invalid, 0x01 encrypted Broadcast_Code valid
    uint8_t broadcast_code[16]; // used for deriving the session key for decrypting payloads of BISes in the BIG
    uint8_t mse; // 0x00 controler can schedule reception of any num of se up to NSE, 0x01 to 0x1F max num of se should be used
    uint16_t big_sync_timeout; // 0x0A to 0x4000, per 10ms, 100ms to 163.84s, sync timeout for the BIG
    uint8_t num_bis; // 0x01 to 0x1F, total number of BISes to synchronize
    uint8_t bis_index[1]; // 0x01 to 0x1F, index of a BIS in the BIG
} __attribute__ ((packed));

#define HCI_LE_EV_BIG_SYNC_ESTABLISHED 0x1D
struct hci_ev_le_big_sync_established {
    uint8_t subcode;
    uint8_t status;
    uint8_t big_handle; // 0x00 to 0xEF, the identifier of the BIG
    uint8_t transport_latency_big[3]; // 0xEA to 0x7F_FFFF, the actual transport latency in us
    uint8_t nse; // 0x01 to 0x1F, num of subevents in each BIS event in the BIG
    uint8_t bn; // 0x01 to 0x07, the number of new payloads in each BIS event
    uint8_t pto; // 0x00 to 0x0F, offset used for pre-transmissions
    uint8_t irc; // 0x01 to 0x0F, num of times a payload is transmitted in a BIS event
    uint16_t max_pdu; // 0x01 to 0xFB, max octets of the PDU payload
    uint16_t iso_interval; // 0x0004 to 0x0C80, per 1.25ms, 5ms to 4s, time between two BIG anchor points
    uint8_t num_bis; // 0x01 to 0x1F, total number of established BISes, shall be same in LE_BIG_Create_Sync command
    uint16_t bis_handle[1];
} __attribute__ ((packed));

#define HCI_LE_BIG_TERMINATE_SYNC       0x206C
struct hci_le_big_terminate_sync {
    uint8_t big_handle;
} __attribute__ ((packed));
struct hci_le_big_terminate_sync_cmpl {
    uint8_t status;
    uint8_t big_handle;
} __attribute__ ((packed));

#define HCI_LE_EV_BIG_SYNC_LOST 0x1E
struct hci_ev_le_big_sync_lost {
    uint8_t subcode;
    uint8_t big_handle;
    uint8_t reason;
} __attribute__ ((packed));

/**
 * The HCI_LE_Set_PA_Parameters is used by the Host to set the parameters for periodic advertising.
 * The Advertising_Handle identifies the advertising set whose pa parameters are being configured.
 * If the corresponding advertising set does not already exist, then the Controller shall return
 * the error code Unknown Advertising Identifier (0x42).
 *
 * The Num_Subevents identifiers the number of subevents that shall be transmitted for each pa event.
 * If the Num_Subevents is 0x00, then the Subevent_Interval, Response_Slot_Delay, Response_Slot_Spacing,
 * and Num_Response_Slots shall be ignored.
 *
 * The Subevent_Interval identifiers the time between the subevents of PAwR.
 * Subevent_Interval shall be <= pa_interval_min / num_subevents.
 *
 * The HCI_LE_Set_PA_Subevent_Data is used to set the data for one or more subevents of PAwR in reply
 * to an HCI_LE_PA_Subevent_Data_Request event. The data for a subevent shall be transmitted only once.
 *
 * If the Subevent_Data cannot be transmitted because, for example, the subevent where this data would
 * have been sent has already passed or is too early, then the Controller shall return the error code
 * Too Late (0x46) or Too Early (0x47) and discard the data.
 *
 * The HCI_LE_PA_Subevent_Data_Request event is used to allow the Controller to indicate that it is
 * ready to transmit one or more subevents and is requesting the advertising data for these subevents.
 * This event should be sent from the Controller when it has no data for upcoming subevents. The Controller
 * should request data for as many subevents as it has memory to accept to minimize the number of events
 * generated by the Controller.
 *
 * The HCI_LE_PA_Response_Report event indicates that one or more Bluetooth devices have responded to a
 * pa subevent during a PAwR train. The Controller may queue these advertising reports and send info from
 * multiple devices in one HCI_LE_PA_Response_Report event. The Controller may fail to transmit the sychronizatoin
 * packet required to enable the response packets to be sent. If this happens, the Controller can report
 * this to the Host using the Tx_Status parameter.
 *
 */

#define HCI_LE_SET_PA_PARAMETERS_V1 0x203E
#define HCI_LE_SET_PA_PARAMETERS_V2 0x2086
#define HCI_LE_PA_PROP_INCLUDE_TX_POWER 0x20
struct hci_le_set_pa_param_v1 {
    uint8_t adv_handle; // 0x00 to 0xEF
    uint16_t pa_interval_min_1_25ms; // 0x06 to 0xFFFF * 1.25ms (7.5ms to 81.9s)
    uint16_t pa_interval_max_1_25ms; // 0x06 to 0xFFFF * 1.25ms (7.5ms to 81.9s)
    uint16_t pa_properties; // bit 6 - include tx power in the advertising pdu
} __attribute__ ((packed));
struct hci_le_set_pa_param_v2 {
    uint8_t adv_handle; // 0x00 to 0xEF
    uint16_t pa_interval_min_1_25ms; // 0x06 to 0xFFFF * 1.25ms (7.5ms to 81.9s)
    uint16_t pa_interval_max_1_25ms; // 0x06 to 0xFFFF * 1.25ms (7.5ms to 81.9s)
    uint16_t pa_properties; // bit 6 - include tx power in the advertising pdu
    uint8_t num_subevents; // 0x00 to 0x80
    uint8_t subevent_interval_1_25ms; // 0x06 to 0xFF * 1.25ms (7.5ms to 318.75ms)
    uint8_t response_slot_delay_1_25ms; // 0x00 no response slots, 0xXX 0x01 to 0xFE * 1.25ms (1.25ms to 317.5ms) time between the adv packet in a subevent and the 1st response slot
    uint8_t response_slot_spacing_0_125ms; // 0x00 no response slots, 0xXX 0x02 to 0xFF * 0x125ms (0x25ms to 31.875ms) time between response slots
    uint8_t num_response_slots; // 0x00 no response slots, 0xXX 0x01 to 0xFF num of subevent response slots
} __attribute__ ((packed));
struct hci_ev_le_set_pa_param_cmpl_v1 {
    uint8_t status;
} __attribute__ ((packed));
struct hci_ev_le_set_pa_param_cmpl_v2 {
    uint8_t status;
    uint8_t adv_handle;
} __attribute__ ((packed));

#define HCI_LE_SET_PA_DATA 0x203F
#define HCI_LE_PA_DATA_INTER_FRAGMENT 0x00
#define HCI_LE_PA_DATA_FIRST_FRAGMENT 0x01
#define HCI_LE_PA_DATA_LAST_FRAGMENT 0x02
#define HCI_LE_PA_DATA_COMPLETE 0x03
#define HCI_LE_PA_DATA_UNCHANGED 0x04 // just update Advertising DID of the pa
struct hci_le_set_pa_data {
    uint8_t adv_handle; // 0x00 to 0xEF
    uint8_t operation;
    uint8_t adv_data_len; // 0 to 252
    // pa data formatted as ad type format
} __attribute__ ((packed));
struct hci_ev_le_set_pa_data_cmpl {
    uint8_t status;
} __attribute__ ((packed));

#define HCI_LE_SET_PA_ENABLE 0x2040
#define HCI_LE_PA_ENABLE_FLAG 0x01
#define HCI_LE_PA_ENABLE_INCLUDE_ADI_IN_AUX_SYNC_IND 0x02
struct hci_le_set_pa_enable {
    uint8_t enable; // bit 0 - enable, bit 1 - include ADI field in AUX_SYNC_IND PDUs
    uint8_t adv_handle; // 0x00 to 0xEF
} __attribute__ ((packed));
struct hci_ev_le_set_pa_enable_cmpl {
    uint8_t status;
} __attribute__ ((packed));

#define HCI_LE_EV_PA_SUBEVENT_DATA_REQ 0x27
struct hci_ev_le_pa_subevent_data_req {
    uint8_t subcode; // 0x27
    uint8_t adv_handle; // 0x00 to 0xEF, identify a pa train
    uint8_t subevent_start; // 0x00 to 0x7F, the first subevent that data is requested for
    uint8_t subevent_data_count; // 0x01 to 0x80, num of subevents that data is requested for
} __attribute__ ((packed));

#define HCI_LE_SET_PA_SUBEVENT_DATA 0x2082
struct hci_le_pa_subevent_data_item {
    uint8_t subevent; // 0x00 to 0x7F subevent index
    uint8_t response_slot_start; // the first response slot to be used in the subevent
    uint8_t response_slot_count; // num of response slots to be used
    uint8_t subevent_data_len; // 0 to 251
    // ad type formatted subevent data
} __attribute__ ((packed));
struct hci_le_set_pa_subevent_data {
    uint8_t adv_handle; // 0x00 to 0xEF, identify a pa train
    uint8_t num_subevents_with_data; // 0x01 to 0x0F
    struct hci_le_pa_subevent_data_item item[1];
} __attribute__ ((packed));
struct hci_ev_le_set_pa_subevent_data_cmpl {
    uint8_t status;
    uint8_t adv_handle;
} __attribute__ ((packed));

#define HCI_LE_EV_PA_RESPONSE_REPORT 0x28
#define HCI_LE_CTE_TYPE_AOA 0x00
#define HCI_LE_CTE_TYPE_AOA_1US_SLOTS 0x01
#define HCI_LE_CTE_TYPE_AOA_2US_SLOTS 0x02
#define HCI_LE_CTE_TYPE_NO_CTE 0xFF
#define HCI_LE_PA_RESPONSE_REPORT_AUX_SYNC_SUBEVENT_IND_WAS_TRANSMITTED 0x00
#define HCI_LE_PA_RESPONSE_REPORT_AUX_SYNC_SUBEVENT_IND_NOT_TRANSMITTED 0x01
#define HCI_LE_PA_RESPONSE_REPORT_DATA_COMPLETE 0x00
#define HCI_LE_PA_RESPONSE_REPORT_DATA_INCOMPLETE 0x01
#define HCI_LE_PA_RESPONSE_REPORT_FAIL_RECEIVE_AUX_SYNC_SUBEVENT_RSP 0xFF
struct hci_ev_le_pa_response_data_item {
    int8_t tx_power_dbm; // -127 to +20, 0x7F power info not available
    int8_t rssi_dbm; // -127 to +20, 0x7F rssi is not available
    uint8_t cte_type; // 0x00 to 0x02, 0xFF no cte
    uint8_t response_slot; // 0x00 to 0xFF, the response slot the data was received in
    uint8_t data_status; // 0x00 complete, 0x01 incomplete more data to come, 0xFF failed to receive an AUX_SYNC_SUBEVENT_RSP
    uint8_t data_length;
    uint8_t data[1];
} __attribute__ ((packed));

struct hci_ev_le_pa_response_report {
    uint8_t subcode; // 0x28
    uint8_t adv_handle; // 0x00 to 0xEF, identify a pa train
    uint8_t subevent; // 0x00 to 0x7F
    uint8_t tx_status; // 0x00 AUX_SYNC_SUBEVENT_IND was transmitted, 0x01 not transmitted
    uint8_t num_responses; // 0x00 to 0x19
    struct hci_ev_le_pa_response_data_item item[1];
} __attribute__ ((packed));

/**
 * The HCI_LE_Add_Device_To_PA_List is used to add an entry to the pa list stored in the Controller.
 * Any additions to the pa list take effect immediately. If the entry is already on the list,
 * the controller shall return the error code Invalid HCI command parameters (0x12). When a Controller
 * cannot add an entry to the pa list because the list is full, the Controller shall return error code
 * Memory Capacity Exceeded (0x07).
 *
 * The HCI_LE_Remove_Device_From_PA_List is used to remove one entry from the list of pa stored in the
 * Controller. Removals from the ad list tack effect immediately.
 *
 * The HCI_LE_Clear_PA_List is used to remove all entries from the list of pa in the Controller.
 *
 * If the Host issue HCI_LE_Add_Device_To_PA_List or HCI_LE_Remove_Device_From_PA_List or
 * HCI_LE_Clear_PA_List when an HCI_LE_PA_Create_Sync is pending, the Controller shall return the error
 * code Command Disallowed (0x0C).
 *
 * The HCI_LE_Read_PA_List is used to read the total number of PA list entries that can be stored
 * in the Controller. Note: The number of entries that can be stored is not fixed and the Controller
 * can change it at any time (e.g., because the memory used to store the list can also be used
 * for other purpose).
 *
 */

#define HCI_LE_ADD_DEVICE_TO_PA_LIST 0x2047
struct hci_le_add_device_to_pa_list {
    uint8_t adv_addr_type; // 0x00 public or public ia, 0x01 random or random ia
    bt_bdaddr_t adv_addr; // advertiser address
    uint8_t adv_sid; // 0x00 to 0x0F, advertising SID subfield in the ADI field used to identify the pa
} __attribute__ ((packed));
struct hci_ev_le_add_device_to_pa_list_cmpl {
    uint8_t status;
} __attribute__ ((packed));

#define HCI_LE_REMOVE_DEVICE_FROM_PA_LIST 0x2048
struct hci_le_remove_device_from_pa_list {
    uint8_t adv_addr_type; // 0x00 public or public ia, 0x01 random or random ia
    bt_bdaddr_t adv_addr; // advertiser address
    uint8_t adv_sid; // 0x00 to 0x0F, advertising SID subfield in the ADI field used to identify the pa
} __attribute__ ((packed));
struct hci_ev_le_remove_device_from_pa_list_cmpl {
    uint8_t status;
} __attribute__ ((packed));

#define HCI_LE_CLEAR_PA_LIST 0x2049
struct hci_ev_le_clear_pa_list_cmpl {
    uint8_t status;
} __attribute__ ((packed));

#define HCI_LE_READ_PA_LIST_SIZE 0x204A
struct hci_ev_le_read_pa_list_size_cmpl {
    uint8_t status;
    uint8_t pa_list_size;
} __attribute__ ((packed));

/**
 * The HCI_LE_Periodic_Advertising_Create_Sync command is used to
 * synchronize with a periodic advertising train from an advertiser and begin
 * receiving periodic advertising packets.
 *
 * The HCI_LE_Periodic_Advertising_Create_Sync_Cancel command is used to
 * cancel the HCI_LE_Periodic_Advertising_Create_Sync command while it is
 * pending.
 *
 * The HCI_LE_Periodic_Advertising_Terminate_Sync command is used to stop
 * reception of the periodic advertising train identified by the Sync_Handle
 * parameter.
 *
 * The HCI_LE_Set_Periodic_Advertising_Receive_Enable command will enable
 * or disable reports for the periodic advertising train identified by the
 * Sync_Handle parameter.
 *
 * The HCI_LE_PA_Sync_Established event indicates the Controller has received
 * the first pa packet from the advertiser after the HCI_LE_PA_Create_Sync
 * has been sent to the Controller.
 *
 * The HCI_LE_PA_Sync_Lost event indicates that the Controller has not received
 * a PA packet from the train within the timeout period.
 *
 * The HCI_LE_PA_Report event indicates that the Controller has received a
 * pa or has failed to receive an AUX_SYNC_SUBEVENT_IND PDU.
 *
 */

#define HCI_LE_PA_CREATE_SYNC            0x2044
#define HCI_LE_PA_CREATE_SYNC_CANCEL     0x2045
#define HCI_LE_PA_USING_FLITER_LIST 0x01
#define HCI_LE_PA_REPORTING_INITIAL_DISABLE 0x02
#define HCI_LE_PA_DUPLICATE_FILTER_INITIAL_ENABLE 0x04
#define HCI_LE_PA_NOT_SYNC_TO_AOA_CTE_TYPE 0x01
#define HCI_LE_PA_NOT_SYNC_TO_AOD_CTE_1US_TYPE 0x02
#define HCI_LE_PA_NOT_SYNC_TO_AOD_CTE_2US_TYPE 0x04
#define HCI_LE_PA_NOT_SYNC_TO_CTE_TYPE_3 0x08
#define HCI_LE_PA_NOT_SYNC_TO_PACKET_WO_CTE 0x10
struct hci_le_pa_create_sync {
    uint8_t options;
    uint8_t adv_sid; // 0x00 to 0x0F, Advertising SID subfied in the ADI to identify the pa
    uint8_t adv_addr_type; // 0x00 public or public ia, 0x01 random or random ia
    uint8_t adv_addr[6]; // advertister address if filter list not used
    uint16_t skip; // 0x00 to 0x01F3, max num of pa events can be skipped after a successful receive
    uint16_t sync_to_10ms; // 0x0A to 0x4000 * 10ms (100ms to 163.84s), sync timeout for the pa train
    uint8_t sync_cte_type; // sync to constant tone extension type or not
} __attribute__ ((packed));

#define HCI_LE_SET_PA_RECEIVE_ENABLE     0x2059
#define HCI_LE_PA_REPORTING_ENABLE 0x01
#define HCI_LE_PA_DUPLICATE_FILTER_ENABLE 0x02
struct hci_le_set_pa_receive_enable {
    uint16_t pa_sync_hdl; // 0x0000 to 0x0EFF (12 bits meaningful), identify the pa train
    uint8_t enabled; // bit 0 - reporting enable, bit 1 - duplicate filtering enable
} __attribute__ ((packed));
struct hci_ev_le_set_pa_receive_enable_cmpl {
    uint8_t status;
} __attribute__ ((packed));

#define HCI_LE_PA_TERM_SYNC              0x2046
struct hci_le_pa_term_sync {
    uint16_t pa_sync_hdl; // 0x0000 to 0x0EFF (12 bits meaningful), identify the pa train
} __attribute__ ((packed));
struct hci_ev_le_pa_term_sync_cmpl {
    uint8_t status;
} __attribute__ ((packed));

#define HCI_LE_EV_PA_SYNC_ESTABLISH 0x0E // can be sent as a result of Operation Cancelled by Host (0x44)
#define HCI_LE_EV_PA_SYNC_ESTABLISH_V2 0x24
struct hci_ev_le_pa_sync_establish {
    uint8_t subcode; // 0x0E
    uint8_t status; // 0x00 pa sync succ
    uint16_t sync_handle;
    uint8_t adv_sid;
    uint8_t adv_addr_type; // 0x00 ~ 0x03
    bt_bdaddr_t adv_addr; // public or public ia, random or random ia
    uint8_t adv_phy; // 0x01 LE 1M, 0x02 LE 2M, 0x03 LE Coded
    uint16_t pa_interval_1_25ms; // 0x06 to 0xFFFF * 1.25ms (7.5ms to 81.9s)
    uint8_t adv_clock_accuracy; // 0x00 to 0x07 500ppm to 20ppm
} __attribute__ ((packed));
struct hci_ev_le_pa_sync_establish_v2 {
    uint8_t subcode; // 0x24
    uint8_t status; // 0x00 pa sync succ
    uint16_t sync_handle;
    uint8_t adv_sid;
    uint8_t adv_addr_type; // 0x00 ~ 0x03
    bt_bdaddr_t adv_addr; // public or public ia, random or random ia
    uint8_t adv_phy; // 0x01 LE 1M, 0x02 LE 2M, 0x03 LE Coded
    uint16_t pa_interval_1_25ms; // 0x06 to 0xFFFF * 1.25ms (7.5ms to 81.9s)
    uint8_t adv_clock_accuracy; // 0x00 to 0x07 500ppm to 20ppm
    uint8_t num_subevents; // 0x00 no subevents, 0x01 to 0x80 num of subevents
    uint8_t subevent_interval_1_25ms; // 0x00 no subevents, 0x06 to 0xFF * 1.25ms (7.5ms to 318.75ms)
    uint8_t response_slot_delay_1_25ms; // 0x00 no response slots, 0x01 to 0xFE * 1.25ms (1.25ms to 317.6ms)
    uint8_t response_slot_spacing_0_125ms; // 0x00 no response slots, 0x02 to 0xFF * 0.125ms (0.25ms to 31.875ms)
} __attribute__ ((packed));

#define HCI_LE_EV_PA_SYNC_LOST 0x10
struct hci_ev_le_pa_sync_lost {
    uint8_t subcode; // 0x10
    uint16_t sync_handle;
} __attribute__ ((packed));

#define HCI_LE_EV_PA_REPORT 0x0F
#define HCI_LE_EV_PA_REPORT_V2 0x25
#define HCI_LE_PA_REPORT_DATA_COMPLETE 0x00
#define HCI_LE_PA_REPORT_DATA_INCOMPLETE_MORE_DATA 0x01
#define HCI_LE_PA_REPORT_DATA_INCOMPLETE_TRUNCATED 0x02
#define HCI_LE_PA_REPORT_FAIL_RECEIVE_AUX_SYNC_SUBEVENT_IND 0xFF
#define HCI_LE_PA_REPORT_NO_SUBEVENT 0xFF
struct hci_ev_le_pa_report {
    uint8_t subcode; // 0x0F
    uint16_t sync_handle;
    int8_t tx_power_dbm; // -127 to +20, 0x7F tx power info not available
    int8_t rssi_dbm; // -127 to +20, 0x7F rssi not available
    uint8_t cte_type; // 0x00 to 0x02, 0xFF no cte
    uint8_t data_status; // 0x00 ~ 0x02, 0xFF failed to receive an AUX_SYNC_SUBEVENT_IND PDU
    uint8_t data_length; // 0 to 247
    uint8_t data[1];
} __attribute__ ((packed));
struct hci_ev_le_pa_report_v2 {
    uint8_t subcode; // 0x25
    uint16_t sync_handle;
    int8_t tx_power_dbm; // -127 to +20, 0x7F tx power info not available
    int8_t rssi_dbm; // -127 to +20, 0x7F rssi not available
    uint8_t cte_type; // 0x00 to 0x02, 0xFF no cte
    uint16_t pa_event_counter; // the value of paEventCounter for the reported pa packet
    uint8_t subevent; // 0x00 to 0x7F subevent number, 0xFF no subevents
    uint8_t data_status; // 0x00 ~ 0x02, 0xFF failed to receive an AUX_SYNC_SUBEVENT_IND PDU
    uint8_t data_length; // 0 to 247
    uint8_t data[1];
} __attribute__ ((packed));

/**
 * The HCI_LE_Periodic_Advertising_Sync_Transfer command is used to
 * instruct the Controller to send synchronization information about the periodic
 * advertising train identified by the Sync_Handle parameter to a connected
 * device.
 *
 * The HCI_LE_Periodic_Advertising_Set_Info_Transfer command is used to
 * instruct the Controller to send synchronization information about the periodic
 * advertising in an advertising set to a connected device.
 *
 * The HCI_LE_Set_Periodic_Advertising_Sync_Transfer_Parameters command
 * is used to specify how the Controller will process periodic advertising
 * synchronization information received from the device identified by the
 * Connection_Handle parameter (the "transfer mode").
 *
 * The HCI_LE_Set_Default_Periodic_Advertising_Sync_Transfer_Parameters
 * command is used to specify the initial value for the mode, skip, timeout, and
 * Constant Tone Extension type (set by the
 * HCI_LE_Set_Periodic_Advertising_Sync_Transfer_Parameters command;
 * see Section 7.8.91) to be used for all subsequent connections over the LE
 * transport.
 *
 * The HCI_LE_PAST_Received event is used by the Controller to report that it
 * has received pa sync info from the device referred to by the Conn_Handle and
 * either succ synchronized to the pa train or timeout while attempting to
 * synchronize. The Status will be zero if it succ synchroinzed. The Service_Data
 * value is provided by the Host of the device sending the information.
 *
 */

#define HCI_LE_PER_ADV_SYNC_TRANSFER     0x205A
struct hci_le_pa_sync_transfer {
    uint16_t acl_conn_hdl;
    uint16_t service_data; // a value provided by the host
    uint16_t pa_sync_hdl; // 0x0000 to 0x0EFF (12 bits meaningful), identify the pa train
} __attribute__ ((packed));
struct hci_ev_le_pa_sync_transfer_cmpl {
    uint8_t status;
    uint16_t acl_conn_hdl;
} __attribute__ ((packed));

#define HCI_LE_PER_ADV_SET_INFO_TRANSFER 0x205B
struct hci_le_pa_set_info_transfer {
    uint16_t acl_conn_hdl;
    uint16_t service_data;
    uint8_t adv_hdl;
} __attribute__ ((packed));
struct hci_ev_le_pa_set_info_transfer_cmpl {
    uint8_t status;
    uint16_t acl_conn_hdl;
} __attribute__ ((packed));

#define HCI_LE_SET_PAST_PARAMETERS       0x205C
#define HCI_LE_SET_DFT_PAST_PARAMETERS   0x205D
#define HCI_LE_PAST_MODE_NOT_SYNCHRONIZE 0x00 // default
#define HCI_LE_PAST_MODE_SYNC_NOT_REPORT 0x01
#define HCI_LE_PAST_MODE_SYNC_AND_REPORT 0x02
#define HCI_LE_PAST_MODE_SYNC_AND_DUPLICATE_FILTER_REPORT 0x03
struct hci_le_set_past_params {
    uint16_t acl_conn_hdl;
    uint8_t mode; // 0x01 to 0x03
    uint16_t skip; // 0x00 to 0x01F3, num of pa packets can be skipped after a succ receive
    uint16_t sync_timeout_10ms; // 0x0A to 0x4000 * 10ms (100ms to 163.84s), sync timeout for the pa train
    uint8_t cte_type; // see sync_cte_type
} __attribute__ ((packed));
struct hci_le_set_dft_past_params {
    uint8_t mode; // 0x01 to 0x03
    uint16_t skip; // 0x00 to 0x01F3, num of pa packets can be skipped after a succ receive
    uint16_t sync_timeout_10ms; // 0x0A to 0x4000 * 10ms (100ms to 163.84s), sync timeout for the pa train
    uint8_t cte_type; // see sync_cte_type
} __attribute__ ((packed));
struct hci_ev_le_set_past_param_cmpl {
    uint8_t status;
    uint16_t acl_conn_hdl;
} __attribute__ ((packed));
struct hci_ev_le_set_dft_past_param_cmpl {
    uint8_t status;
} __attribute__ ((packed));

#define HCI_EV_LE_PAST_RECEIVED 0x18
#define HCI_EV_LE_PAST_RECEIVED_V2 0x26
struct hci_ev_le_past_received {
    uint8_t subcode; // 0x18
    uint8_t status; // 0x00 pa sync succ
    uint16_t conn_handle;
    uint16_t service_data; // a value provided by the peer data
    uint16_t sync_handle;
    uint8_t adv_sid;
    uint8_t adv_addr_type; // 0x00 ~ 0x03
    bt_bdaddr_t adv_addr; // public or public ia, random or random ia
    uint8_t adv_phy; // 0x01 LE 1M, 0x02 LE 2M, 0x03 LE Coded
    uint8_t pa_interval_1_25ms; // 0x06 to 0xFFFF * 1.25ms (7.5ms to 81.9s)
    uint8_t adv_clock_accuracy; // 0x00 to 0x07 500ppm to 20ppm
} __attribute__ ((packed));
struct hci_ev_le_past_received_v2 {
    uint8_t subcode; // 0x26
    uint8_t status; // 0x00 pa sync succ
    uint16_t conn_handle;
    uint16_t service_data; // a value provided by the peer data
    uint16_t sync_handle;
    uint8_t adv_sid;
    uint8_t adv_addr_type; // 0x00 ~ 0x03
    bt_bdaddr_t adv_addr; // public or public ia, random or random ia
    uint8_t adv_phy; // 0x01 LE 1M, 0x02 LE 2M, 0x03 LE Coded
    uint16_t pa_interval_1_25ms; // 0x06 to 0xFFFF * 1.25ms (7.5ms to 81.9s)
    uint8_t adv_clock_accuracy; // 0x00 to 0x07 500ppm to 20ppm
    uint8_t num_subevents; // 0x00 no subevents, 0x01 to 0x80 num of subevents
    uint8_t subevent_interval_1_25ms; // 0x00 no subevents, 0x06 to 0xFF * 1.25ms (7.5ms to 318.75ms)
    uint8_t response_slot_delay_1_25ms; // 0x00 no response slots, 0x01 to 0xFE * 1.25ms (1.25ms to 317.6ms)
    uint8_t response_slot_spacing_0_125ms; // 0x00 no response slots, 0x02 to 0xFF * 0.125ms (0.25ms to 31.875ms)
} __attribute__ ((packed));

/**
 * The HCI_LE_Set_PA_Response_Data is used to set data for response slot in a specific subevent of the
 * PAwR identified by the Sync_Handle. The data for a response slot shall be transmitted only once.
 *
 * If the response slot identified by the Response_Slot has passed by the time this command is received
 * by the Controller, the Controller shall return the error code Too Late (0x46) and discard the
 * Response_Data paramter.
 *
 * The HCI_LE_Set_Periodic_Sync_Subevent is used to instruct the Controller to synchroize with a subset
 * of the subevents within a PAwR train identified by the Sync_Handle, listen for packets sent by the
 * peer device and pass any received data up to the Host.
 *
 * If the Controller is synchnrozied with any subevents that are not in the subset subevents in this
 * command, then the Controller shall no longer synchronize with those subevents.
 *
 */

#define HCI_LE_SET_PA_RESPONSE_DATA 0x2083
struct hci_le_set_pa_response_data {
    uint16_t sync_handle; // 0x0000 to 0x0EFF, identifying the PAwR train
    uint16_t request_event; // which pa packet to response (paEventCounter)
    uint8_t request_subevent; // which pa packet to response (subevent)
    uint8_t response_subevent; // 0x00 to 0x7F identify the subevent of PAwR train to response
    uint8_t response_slot; // 0x00 to 0xFF identify the response slot of the PAwR train
    uint8_t response_data_len; // 0 to 251
    // ad type formatted response data
} __attribute__ ((packed));
struct hci_ev_le_set_pa_response_data_cmpl {
    uint8_t status;
    uint16_t sync_handle;
} __attribute__ ((packed));

#define HCI_LE_SET_PA_SYNC_SUBEVENT 0x2084
struct hci_le_set_pa_sync_subevent {
    uint16_t sync_handle;
    uint16_t pa_properties; // bit 6 - include tx power in AUX_SYNC_SUBEVENT_RSP PDUs or not
    uint8_t num_subevents_to_sync; // 0x01 to 0x80 num of subevents to sync
    // subevent[i] 0x00 to 0x7F the subevent to synchronize with
} __attribute__ ((packed));
struct hci_ev_le_set_pa_sync_subevent_cmpl {
    uint8_t status;
    uint16_t sync_handle;
} __attribute__ ((packed));

/**
 * Channel Sounding HCI
 *
 *
 */
#define LE_CS_CHANNEL_NUM           (72)
#define LE_CS_CHANNEL_MAP_BIT_NUM   (80)
#define LE_CS_BYTE_BIT_NUM          (8)
#define LE_CS_CHANNEL_MAP_SIZE      (10)    // LE_CS_CHANNEL_MAP_BIT_NUM / LE_CS_BYTE_BIT_NUM
#define HCI_LE_CMD_CS_READ_LOCAL_SUPP_CAPS              0x2089
#define HCI_LE_CMD_CS_READ_REMOTE_SUPP_CAPS             0x208A
struct hci_le_cmd_cs_read_remote_supp_capas {
    uint16_t conn_handle;
} __attribute__((packed));

#define HCI_LE_CMD_CS_WRITE_CACHED_REMOTE_SUPP_CAPS     0x208B
struct hci_le_cmd_cs_write_cached_remote_supp_capas {
    uint16_t    conn_handle;
    uint8_t     num_config_supp;
    uint16_t    max_cnsv_proc_supp;     // cnsv -> consecutive
    uint8_t     num_antennas_supp;
    uint8_t     max_antenna_paths_supp;
    uint8_t     roles_supp;
    uint8_t     modes_supp;
    uint8_t     rtt_capas;
    uint8_t     rtt_aa_only_n;
    uint8_t     rtt_sounding_n;
    uint8_t     rtt_random_payload_n;
    uint16_t    nadm_sounding_capability;
    uint16_t    nadm_random_capability;
    uint8_t     cs_sync_phys_supp;
    uint16_t    subfeatures_supp;
    uint16_t    t_ip1_times_supp;
    uint16_t    t_ip2_times_supp;
    uint16_t    t_fcs_times_supp;
    uint16_t    t_pm_times_supp;
    uint16_t    t_sw_time_supp;
    uint8_t     tx_snr_capability;
} __attribute__((packed));

#define HCI_LE_CMD_CS_SECURITY_ENABLE               0x208C
struct hci_le_cmd_cs_security_enable {
    uint16_t conn_handle;
} __attribute__((packed));

#define HCI_LE_CMD_CS_SET_DEFAULT_SETTINGS          0x208D
struct hci_le_cmd_cs_set_default_settings {
    uint16_t    conn_handle;
    uint8_t     role_enable;
    uint8_t     cs_sync_antenna_selection;
    uint8_t     max_tx_power;
} __attribute__((packed));

#define HCI_LE_CMD_CS_READ_REMOTE_FAE_TABLE         0x208E
struct hci_le_cmd_cs_read_remote_fae_table {
    uint16_t conn_handle;
} __attribute__((packed));

#define HCI_LE_CMD_CS_WRITE_CACHED_REMOTE_FAE_TABLE 0x208F
struct hci_le_cmd_cs_write_cached_remote_fae_table {
    uint16_t    conn_handle;
    uint8_t     remote_fae_table[LE_CS_CHANNEL_NUM];
} __attribute__((packed));

#define HCI_LE_CMD_CS_CREATE_CONFIG                 0x2090
struct hci_le_cmd_cs_create_config {
    uint16_t    conn_handle;
    uint8_t     config_id;
    uint8_t     create_context;
    uint8_t     main_mode_type;
    uint8_t     sub_mode_type;
    uint8_t     min_main_mode_steps;
    uint8_t     max_main_mode_steps;
    uint8_t     main_mode_repetition;
    uint8_t     mode_0_steps;
    uint8_t     role;
    uint8_t     rtt_type;
    uint8_t     cs_sync_phy;
    uint8_t     channel_map[LE_CS_CHANNEL_MAP_SIZE];
    uint8_t     channel_map_repetition;
    uint8_t     channel_selection_type;
    uint8_t     ch3c_shape;
    uint8_t     ch3c_jump;
    uint8_t     reserved;
} __attribute__((packed));

#define HCI_LE_CMD_CS_REMOVE_CONFIG                 0x2091
struct hci_le_cmd_cs_remove_config {
    uint16_t    conn_handle;
    uint8_t     config_id;
} __attribute__((packed));

// class -> classification
#define HCI_LE_CMD_CS_SET_CHANNEL_CLASSIFICATION    0x2092
struct hci_le_cmd_cs_set_channel_classification {
    uint8_t channel_classification[LE_CS_CHANNEL_MAP_SIZE];
} __attribute__((packed));

#define HCI_LE_CMD_CS_SET_PROC_PARAMS               0x2093
struct hci_le_cmd_cs_set_proc_params {
    uint16_t    conn_handle;
    uint8_t     config_id;
    uint16_t    max_proc_len;
    uint16_t    min_proc_interval;
    uint16_t    max_proc_interval;
    uint16_t    max_proc_count;
    uint8_t     min_subevent_len[3];
    uint8_t     max_subevent_len[3];
    uint8_t     tone_antenna_config_selection;
    uint8_t     phy;
    uint8_t     tx_power_delta;
    uint8_t     preferred_peer_antenna;
    uint8_t     snr_control_initiator;
    uint8_t     snr_control_reflector;
} __attribute__((packed));

#define HCI_LE_CMD_CS_PROC_ENABLE                   0x2094
struct hci_le_cmd_cs_proc_enable {
    uint16_t    conn_handle;
    uint8_t     config_id;
    uint8_t     enable;
} __attribute__((packed));

#define HCI_LE_CMD_CS_TEST                          0x2095
struct hci_le_cmd_cs_test {
    uint8_t     main_mode_type;
    uint8_t     sub_mode_type;
    uint8_t     main_mode_repetition;
    uint8_t     mode_0_steps;
    uint8_t     role;
    uint8_t     rtt_type;
    uint8_t     cs_sync_phy;
    uint8_t     cs_sync_antenna_selection;
    uint8_t     subevent_len[3];
    uint16_t    subevent_interval;
    uint8_t     max_num_subevents;
    uint8_t     transmit_power_level;
    uint8_t     t_ip1_time;
    uint8_t     t_ip2_time;
    uint8_t     t_fcs_time;
    uint8_t     t_pm_time;
    uint8_t     t_sw_time;
    uint8_t     tone_antenna_config_selection;
    uint8_t     reserved;
    uint8_t     snr_control_initiator;
    uint8_t     snr_control_reflector;
    uint16_t    drbg_nonce;
    uint8_t     channel_map_repetition;
    uint16_t    override_config;
    uint8_t     override_params_len;
    uint8_t     override_params_data[];         // length depends on `override_config`
} __attribute__((packed));

// opd -> override parameters data, b0 -> bit 0
struct le_cs_opd_b0_true {
    uint8_t channel_len;
    uint8_t channel[];
} __attribute__((packed));

struct le_cs_opd_b0_false {
    uint8_t channel_map[LE_CS_CHANNEL_MAP_SIZE];
    uint8_t channel_selection_type;
    uint8_t ch3c_type;
    uint8_t ch3c_jump;
} __attribute__((packed));

struct le_cs_opd_b2_true {
    uint8_t main_mode_steps;
} __attribute__((packed));

struct le_cs_opd_b2_false {

} __attribute__((packed));

struct le_cs_opd_b3_true {
    uint8_t t_pm_tone_ext;
} __attribute__((packed));

struct le_cs_opd_b3_false {

} __attribute__((packed));

struct le_cs_opd_b4_true {
    uint8_t tone_antenna_permutation;
} __attribute__((packed));

struct le_cs_opd_b4_false {

} __attribute__((packed));

struct le_cs_opd_b5_true {
    uint32_t cs_sync_aa_initiator;
    uint32_t cs_sync_aa_reflector;
} __attribute__((packed));

struct le_cs_opd_b5_false {

} __attribute__((packed));

struct le_cs_opd_b6_true {
    uint8_t ss_marker1_position;
    uint8_t ss_marker2_position;
} __attribute__((packed));

struct le_cs_opd_b6_false {

} __attribute__((packed));

struct le_cs_opd_b7_true {
    uint8_t ss_marker_value;
} __attribute__((packed));

struct le_cs_opd_b7_false {

} __attribute__((packed));

struct le_cs_opd_b8_true {
    uint8_t cs_sync_payload_pattern;
    uint8_t cs_sync_user_payload;
} __attribute__((packed));

struct le_cs_opd_b8_false {

} __attribute__((packed));

#define LE_CS_TEST_MAX_CHANNEL_LEN          LE_CS_CHANNEL_NUM
#define LE_CS_TEST_MAX_B0_LEN               CO_MAX_VALUE(1 + LE_CS_TEST_MAX_CHANNEL_LEN, sizeof(struct le_cs_opd_b0_false))
#define LE_CS_TEST_MAX_B2_LEN               sizeof(struct le_cs_opd_b2_true)
#define LE_CS_TEST_MAX_B3_LEN               sizeof(struct le_cs_opd_b3_true)
#define LE_CS_TEST_MAX_B4_LEN               sizeof(struct le_cs_opd_b4_true)
#define LE_CS_TEST_MAX_B5_LEN               sizeof(struct le_cs_opd_b5_true)
#define LE_CS_TEST_MAX_B6_LEN               sizeof(struct le_cs_opd_b6_true)
#define LE_CS_TEST_MAX_B7_LEN               sizeof(struct le_cs_opd_b7_true)
#define LE_CS_TEST_MAX_B8_LEN               sizeof(struct le_cs_opd_b8_true)
#define LE_CS_TEST_OVERRIDE_PARAMS_MAX_LEN  (LE_CS_TEST_MAX_B0_LEN + LE_CS_TEST_MAX_B2_LEN + LE_CS_TEST_MAX_B3_LEN + LE_CS_TEST_MAX_B4_LEN \
                                        + LE_CS_TEST_MAX_B5_LEN + LE_CS_TEST_MAX_B6_LEN + LE_CS_TEST_MAX_B7_LEN + LE_CS_TEST_MAX_B8_LEN)

#define HCI_LE_CMD_CS_TEST_END                          0x2096
#define HCI_LE_EVT_CS_READ_REMOTE_SUPP_CAPS_CMPL        0x2C
struct hci_le_evt_cs_read_remote_supp_capas_cmpl {
    uint8_t     status;
    uint16_t    conn_handle;
    uint8_t     num_config_supp;
    uint16_t    max_cnsv_proc_supp;
    uint8_t     num_antennas_supp;
    uint8_t     max_antenna_paths_supp;
    uint8_t     roles_supp;
    uint8_t     modes_supp;
    uint8_t     rtt_capability;
    uint8_t     rtt_aa_only_n;
    uint8_t     rtt_sounding_n;
    uint8_t     rtt_random_payload_n;
    uint16_t    nadm_sounding_capability;
    uint16_t    nadm_random_capability;
    uint8_t     cs_sync_phys_supp;
    uint16_t    subfeatures_supp;
    uint8_t     t_ip1_times_supp;
    uint8_t     t_ip2_times_supp;
    uint16_t    t_fcs_times_supp;
    uint16_t    t_pm_times_supp;
    uint8_t     t_sw_time_supp;
    uint8_t     tx_snr_capability;
} __attribute__((packed));

#define HCI_LE_EVT_CS_READ_REMOTE_FAE_TABLE_CMPL        0x2D
struct hci_le_evt_cs_read_remote_fae_table_cmpl {
    uint8_t     status;
    uint16_t    conn_handle;
    uint8_t     remote_fae_table[LE_CS_CHANNEL_NUM];
} __attribute__((packed));

#define HCI_LE_EVT_CS_SECURITY_ENABLE_CMPL              0x2E
struct hci_le_evt_cs_security_enable_cmpl {
    uint8_t     status;
    uint16_t    conn_handle;
} __attribute__((packed));

#define HCI_LE_EVT_CS_CONFIG_CMPL                       0x2F
struct hci_le_evt_cs_config_cmpl {
    uint8_t     status;
    uint16_t    conn_handle;
    uint8_t     config_id;
    uint8_t     action;
    uint8_t     main_mode_type;
    uint8_t     sub_mode_type;
    uint8_t     min_main_mode_steps;
    uint8_t     max_main_mode_steps;
    uint8_t     main_mode_repetition;
    uint8_t     mode_0_steps;
    uint8_t     role;
    uint8_t     rtt_type;
    uint8_t     cs_sync_phy;
    uint8_t     channel_map[LE_CS_CHANNEL_MAP_SIZE];
    uint8_t     channel_map_repetition;
    uint8_t     channel_selection_type;
    uint8_t     ch3c_shape;
    uint8_t     ch3c_jump;
    uint8_t     reserved;
    uint8_t     t_ip1_time;
    uint8_t     t_ip2_time;
    uint8_t     t_fcs_time;
    uint8_t     t_pm_time;
} __attribute__((packed));

#define HCI_LE_EVT_CS_PROC_ENABLE_CMPL                  0x30
struct hci_le_evt_cs_proc_enable_cmpl {
    uint8_t     status;
    uint16_t    conn_handle;
    uint8_t     config_id;
    uint8_t     state;
    uint8_t     tone_antenna_config_selection;
    uint8_t     selected_tx_power;
    uint8_t     subevent_len[3];
    uint8_t     subevents_per_evt;
    uint16_t    subevent_interval;
    uint16_t    event_interval;
    uint16_t    proc_interval;
    uint16_t    proc_count;
    uint16_t    max_proc_len;
} __attribute__((packed));

#define HCI_LE_EVT_CS_SUBEVENT_RESULT                   0x31
struct hci_le_evt_cs_subevent_result {
    uint16_t    conn_handle;
    uint8_t     config_id;
    uint16_t    start_acl_conn_evt_counter;
    uint16_t    proc_counter;
    uint16_t    freq_compensation;
    uint8_t     reference_power_level;
    uint8_t     proc_done_status;
    uint8_t     subevent_done_status;
    uint8_t     abort_reason;
    uint8_t     num_antenna_paths;
    uint8_t     num_steps_reported;
    uint8_t     step_params[];              // elem length is subject to `num_steps_reported`
} __attribute__((packed));

// step params: `step_mode[]`, `step_channel[]`, `step_data_len[]`, `step_data[]`
struct cs_step_params {
    uint8_t step_mode;
    uint8_t step_channel;
    uint8_t step_data_len;
    uint8_t step_data[];        // i.e. `mode_role_specific_info` , whose contents are subject to `mode_type` & `role`
} __attribute__((packed));

// cs step data -- i.e. `cs_mode_role_specific_info`
struct cs_mode_role_specific_info {
    uint8_t     pkt_qty;        // pkt --> packet, qty --> quality
    uint8_t     pkt_nadm;
    uint8_t     pkt_rssi;
    uint8_t     pkt_antenna;
    uint32_t    pkt_pct1;
    uint32_t    pkt_pct2;
    uint16_t    measured_freq_offset;
    uint16_t    toa_tod_initiator;
    uint16_t    tod_toa_reflector;
    uint8_t     antenna_permutation_idx;
    uint8_t     tone_params[];  // i.e. `tone_pct` & `tone_qty_indicator`, whose quantity is subject to `num_antenna_paths`
} __attribute__((packed));

// cs tone params -- i.e. `tone_pct` & `tone_qty_indicator`, whose quantity is subject to `num_antenna_paths`
struct cs_tone_params {
    uint8_t tone_pct[3];
    uint8_t tone_qty_indicator;
} __attribute__((packed));

#define HCI_LE_EVT_CS_SUBEVENT_RESULT_CONTINUE          0x32
struct hci_le_evt_cs_subevent_result_continue {
    uint16_t    conn_handle;
    uint8_t     config_id;
    uint8_t     proc_done_status;
    uint8_t     subevent_done_status;
    uint8_t     abort_reason;
    uint8_t     num_antenna_paths;
    uint8_t     num_steps_reported;
    uint8_t     step_params[];  // i.e. `cs_step_params_t`, elem length is subject to `num_steps_reported`
} __attribute__((packed));

#define HCI_LE_EVT_CS_TEST_END_CMPL                     0x33
struct hci_le_evt_cs_test_end_cmpl {
    uint8_t status;
} __attribute__((packed));

/**
 * BES TWS fast pair vender HCI command
*/
#define HCI_LE_TWS_FASTPAIR_CONFIG  0xFCF8
struct hci_le_tws_fastpair_config {
    bool enable;
    uint8_t tws_role;
    uint8_t activity_id;
    uint8_t tws_addr[6];
    uint8_t peer_le_addr[6];
} __attribute__ ((packed));

#if (mHDT_LE_SUPPORT)
#define HCI_DBG_MHDT_LE_RD_LOC_FEAT_CMD_OPCODE       0xFCE1
#define HCI_DBG_MHDT_LE_RD_RM_FEAT_CMD_OPCODE        0x20FF

enum mhdt_hci_read_proprietary_feat_sub_opcode
{
    HCI_LE_RD_LOCAL_PROPRIETARY_FEAT_SUBCODE   = 0x07,
    HCI_LE_RD_REMOTE_PROPRIETARY_FEAT_SUBCODE  = 0x08,
};

struct hci_le_rd_local_proprietary_feat_cmd
{
    uint8_t subcode;
};

struct hci_le_rd_remote_proprietary_feat_cmd
{
    uint16_t conhdl;
    uint8_t subcode;
}__attribute__ ((packed));
#endif

#ifdef IBRT
typedef void (*hci_tx_buf_tss_process_cb_type)(void);
typedef void (*bt_hci_acl_ecc_softbit_handler_func)(uint16_t*,uint16_t*, uint16_t, uint8_t*);
void register_hci_acl_ecc_softbit_handler_callback(bt_hci_acl_ecc_softbit_handler_func func);
#endif

bool hci_tx_buff_process(void);
void hci_rx_handle(struct hci_rx_desc_t *rx_desc);
struct pp_buff *hci_fc_each_link_has_tx_chance(hci_conn_type_t conn_type);

bool hci_tx_buff_process(void);
void hci_rx_handle(struct hci_rx_desc_t *rx_desc);
struct pp_buff *hci_fc_each_link_has_tx_chance(hci_conn_type_t conn_type);

bt_status_t hci_simulate_event(const uint8_t *buff, uint16_t buff_len);
bt_status_t hci_send_cmd_direct(const uint8_t *cmd_packet, uint8_t packet_len);
struct hci_conn_item_t *hci_find_conn_by_address(hci_conn_type_t conn_type, bt_addr_type_t addr_type, const bt_bdaddr_t *peer_addr);
void hci_foreach_acl_conn_item(hci_conn_type_t conn_type, int (*cb)(void *priv_param, void *param), void *param);
struct hci_conn_item_t *hci_find_conn_by_handle(hci_conn_type_t type, uint16_t connhdl);
struct hci_conn_item_t *hci_find_acl_conn_item(hci_conn_type_t type, uint16_t connhdl);
struct hci_conn_item_t *hci_alloc_le_conn_and_report(uint16_t connhdl, uint8_t is_central,
                                                        uint8_t peer_addr_type, const bt_bdaddr_t *peer_addr);

#if defined(__cplusplus)
}
#endif
#endif /* __HCI_I_H__ */
