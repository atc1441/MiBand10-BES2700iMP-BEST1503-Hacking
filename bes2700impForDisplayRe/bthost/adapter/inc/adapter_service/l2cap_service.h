/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __BT_L2CAP_SERVICE_H__
#define __BT_L2CAP_SERVICE_H__
#include "hci_service.h"
#ifdef __cplusplus
extern "C" {
#endif

#if BLE_AUDIO_ENABLED
#define EATT_CHAN_SUPPORT 1
#else
#define EATT_CHAN_SUPPORT 0
#endif

#define PSM_SDP                 0x0001
#define PSM_RFCOMM              0x0003
#define PSM_TCS_BIN             0x0005 // telephony control spec / tcs binary
#define PSM_TCS_BIN_CORDLESS    0x0007
#define PSM_BNEP                0x000F
#define PSM_HID_CTRL            0x0011
#define PSM_HID_INTR            0x0013
#define PSM_UPNP                0x0015
#define PSM_AVCTP               0x0017
#define PSM_AVDTP               0x0019
#define PSM_AVCTP_BROWSING      0x001B
#define PSM_UDI_C_PLANE         0x001D // unrestricted digital information profile UDI
#define PSM_ATT                 0x001F
#define PSM_3DSP                0x0021 // 3D synchronization profile
#define PSM_LE_IPSP             0x0023 // internet protocol support profile IPSP
#define PSM_OTS                 0x0025 // object transfer service OTS
#define PSM_EATT                0x0027
#define PSM_SPSM_GFPS           0x0081

#define L2CAP_NULL_CID              0x0000
#define L2CAP_SIG_CID               0x0001
#define L2CAP_CONNECTIONLESS_CID    0x0002
#define L2CAP_AMP_MANAGER_CID       0x0003
#define L2CAP_LE_ATT_CID            0x0004
#define L2CAP_LE_SIGNALING_CID      0x0005
#define L2CAP_LE_SMP_CID            0x0006
#define L2CAP_BREDR_SMP_CID         0x0007
#define L2CAP_AMP_TEST_MANAGER_CID  0x003F

typedef uint16_t l2cap_psm_target_profile_t;
typedef enum {
    L2CAP_PSM_TARGET_SELF,
    L2CAP_PSM_TARGET_AVDTP_SIGNAL = 1,
    L2CAP_PSM_TARGET_AVDTP_MEDIA,
    L2CAP_PSM_TARGET_OUTGOING_SDP,
    L2CAP_PSM_TARGET_INCOMING_SDP,
    L2CAP_PSM_TARGET_OUTGOING_ATT,
    L2CAP_PSM_TARGET_INCOMING_ATT,
    L2CAP_PSM_TARGET_OUTGOING_EATT,
    L2CAP_PSM_TARGET_INCOMING_EATT,
    L2CAP_DYNAMIC_PSM_TARGET_PBAP,
    L2CAP_DYNAMIC_PSM_TARGET_MAP_ACCESS,
    L2CAP_DYNAMIC_PSM_TARGET_MAP_NOTIFY,
    L2CAP_DYNAMIC_PSM_TARGET_OPP,
    L2CAP_DYNAMIC_PSM_TARGET_AVRCP_ART,
} _impl_l2cap_psm_target_profile_t;

typedef struct {
    uint8_t ctkd_over_br_edr: 1;
    uint8_t gatt_over_br_edr: 1;
    uint8_t eatt_over_br_edr: 1;
    uint8_t stack_new_design: 1;
} bt_l2cap_config_t;

bt_l2cap_config_t bt_l2cap_get_config(void);
void bt_l2cap_set_config(bt_l2cap_config_t *cfg);

typedef enum {
    BT_L2CAP_EVENT_OPENED = BT_EVENT_L2CAP_OPENED,
    BT_L2CAP_EVENT_CLOSED,
    BT_L2CAP_EVENT_TX_DONE,
    BT_L2CAP_EVENT_RX_DATA,
    BT_L2CAP_EVENT_RX_DONE,
    BT_L2CAP_EVENT_ACCEPT,
    BT_L2CAP_EVENT_CREDIT_IND,
} bt_l2cap_event_t;

typedef enum {
    BT_L2CAP_ACCEPT_REQ = 0,
    BT_L2CAP_REJECT_REQ,
    BT_L2CAP_UPPER_ACCEPT,
} bt_l2cap_accept_result_t;

typedef struct {
    uint8_t error_code;
    uint8_t device_id;
    bool is_initiator;
    bool dont_report_tx_done;
    hci_conn_type_t conn_type;
    bt_addr_type_t addr_type;
    bt_bdaddr_t peer_addr;
    uint16_t connhdl;
    uint16_t remote_mtu;
    int32_t free_credits;
    uint8_t l2cap_mode;
    uint32_t l2cap_handle;
    void *tx_priv_rx_ppb;
} bt_l2cap_param_t;

typedef struct {
    hci_conn_type_t conn_type;
    bt_addr_type_t addr_type;
    bt_bdaddr_t peer_addr;
    uint16_t conn_handle;
    uint16_t remote_scid;
    uint16_t remote_req_psm;
    uint8_t device_id;
    uint8_t identifier;
    l2cap_psm_target_profile_t target_profile;
} bt_l2cap_accept_t;

typedef union {
    void *param_ptr;
    bt_l2cap_param_t *opened;
    bt_l2cap_param_t *closed;
    bt_l2cap_param_t *tx_done;
    bt_l2cap_param_t *rx_data;
    bt_l2cap_accept_t *accept;
    bt_l2cap_param_t *credit_ind;
} bt_l2cap_callback_param_t;

typedef int (*bt_l2cap_callback_t)(uintptr_t connhdl, bt_l2cap_event_t event, bt_l2cap_callback_param_t param);

typedef struct {
    struct single_link_node_t node;
    uint16_t local_l2cap_psm;
    uint16_t expected_rx_mtu;
    uint8_t conn_count;
    bool is_in_listen;
    uint8_t initial_credits;
    uint8_t credit_give_step;
    bt_l2cap_callback_t l2cap_callback;
} bt_l2cap_port_t;

typedef struct {
    hci_conn_type_t conn_type;
    bt_addr_type_t addr_type;
    uint16_t conn_handle;
    uint32_t l2cap_handle;
    bt_bdaddr_t peer_addr;
    bool is_initiator;
    uint8_t device_id;
    uint16_t dlci;
    uint16_t tx_mtu;
    uint16_t rx_credits;
    uint16_t tx_credits;
    uint16_t free_tx_packets; // number of packets currently can be delivered to socket with each packet size <= tx_mtu
    bt_l2cap_port_t *port;
    void *l2cap_priv;
} bt_l2cap_socket_t;

typedef enum {
    L2CAP_CREDIT_RESULT_OK                  = 0x00,
    L2CAP_CREDIT_SPSM_NOT_SUPP              = 0x02,
    L2CAP_CREDIT_SOME_REFUSED_NO_RESOUCES   = 0x04,
    L2CAP_CREDIT_INSUFFI_AUTHEN             = 0x05,
    L2CAP_CREDIT_INSUFFI_AUTHOR             = 0x06,
    L2CAP_CREDIT_ENC_KEY_TOO_SHORT          = 0x07,
    L2CAP_CREDIT_INSUFFI_ENCRYPT            = 0x08,
    L2CAP_CREDIT_SOME_REFUSED_INVALID_SCID  = 0x09,
    L2CAP_CREDIT_SOME_REFUSED_SCID_ALLOCED  = 0x0A,
    L2CAP_CREDIT_UNACCEPT_PARAM             = 0x0B,
    L2CAP_CREDIT_INVALID_PARAM              = 0x0C,
    L2CAP_CREDIT_PENDING_NO_FURTHER_INFO    = 0x0D,
    L2CAP_CREDIT_PENDING_FOR_AUTHEN         = 0x0E,
    L2CAP_CREDIT_PENDING_FOR_AUTHOR         = 0x0F,
} l2cap_credit_result_t;

bt_status_t bt_l2cap_create_port(uint16_t local_l2cap_psm, bt_l2cap_callback_t l2cap_callback);
bt_status_t bt_l2cap_remove_port(uint16_t local_l2cap_psm);
bt_status_t bt_l2cap_set_rx_credits(uint16_t local_l2cap_psm, uint16_t expected_rx_mtu, uint16_t initial_credits, uint16_t credit_give_step);
bt_status_t bt_l2cap_listen(uint16_t local_l2cap_psm);
bt_status_t bt_l2cap_remove_listen(uint16_t local_l2cap_psm);
bt_status_t bt_l2cap_send_packet(uint16_t connhdl, uint32_t l2cap_handle, const uint8_t *data, uint16_t len, void *context);
bt_status_t bt_l2cap_disconnect(uint32_t l2cap_handle, uint8_t reason);

uint32_t bt_l2cap_connect(const bt_bdaddr_t *remote, uint16_t local_l2cap_psm, l2cap_psm_target_profile_t target);

#ifdef BT_L2CAP_ENRE_MODE_SUPPORT
uint32_t bt_l2cap_enre_connect(const bt_bdaddr_t *remote, uint16_t local_l2cap_psm, l2cap_psm_target_profile_t target, uint16_t remote_l2cap_psm);
#endif

#if (EATT_CHAN_SUPPORT == 1)
bt_status_t bt_l2cap_enfc_connect(hci_conn_type_t conn_type, uint16_t connhdl, uint16_t local_spsm);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __BT_L2CAP_SERVICE_H__ */
