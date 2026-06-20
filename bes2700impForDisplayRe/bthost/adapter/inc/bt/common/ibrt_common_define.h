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
#ifndef __IBRT_COMMON_DEFINE_H__
#define __IBRT_COMMON_DEFINE_H__
#include "bt_common_define.h"
#include "me_common_define.h"
#include "hfp_common_define.h"
#include "l2cap_common_define.h"
#ifdef IBRT
#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t btif_besaud_event;

#define BTIF_BESAUD_EVENT_CONTROL_CONNECTED            0x21

#define BTIF_BESAUD_EVENT_CONTROL_DISCONNECTED         0x22

#define BTIF_BESAUD_EVENT_CONTROL_DATA_IND             0x23

#define BTIF_BESAUD_EVENT_CONTROL_DATA_SENT            0x24

#define BTIF_BESAUD_EVENT_CONTROL_SET_IDLE             0x30

#define    BTIF_TWS_LINK_CONNECTED          (1 << 0)
#define    BTIF_TWS_L2CAP_CONNECTED         (1 << 1)
#define BTM_NAME_MAX_LEN                         248  /*include '\0'*/

#define BT_RPOFILE_FINAL_FLAG   (0x5f)

enum PROFILE_CONTEXT_FLAG
{
    BT_HFP_FLAG = 0x01,
    BT_A2DP_FLAG = 0x02,
    BT_AVRCP_FLAG = 0x04,
    BT_MAP_FLAG =0x08,
    BT_GATT_FLAG = 0x10,
    BT_HID_FLAG = 0x20,
    //add new profile flag here

    BT_SPP_FLAG = 0x80, //SPP has multiple app id(total BTIF_APP_SPP_NUM),BT_SPP_FLAG flag should be at high bit
};

#define BT_ALL_CONTEXT_PSM    (PSM_CONTEXT_SDP | PSM_CONTEXT_RFC | PSM_CONTEXT_AVDTP | PSM_CONTEXT_AVCTP | PSM_CONTEXT_BTGATT)  

#define BT_ALL_CONTEXT_FLAG   (BT_HFP_FLAG | BT_A2DP_FLAG  | BT_AVRCP_FLAG | BT_MAP_FLAG | BT_SPP_FLAG | BT_GATT_FLAG | BT_HID_FLAG)

#define BT_ALL_RFC_APP_ID     BTIF_APP_HFP_PROFILE_ID

#ifdef BT_HFP_SUPPORT
#define BT_EARPHONE_BASIC_APP_ID       (BTIF_APP_HFP_PROFILE_ID | BTIF_APP_A2DP_PROFILE_ID | BTIF_APP_AVRCP_PROFILE_ID)
#else
#define BT_EARPHONE_BASIC_APP_ID       (BTIF_APP_A2DP_PROFILE_ID | BTIF_APP_AVRCP_PROFILE_ID)
#endif

typedef void (*btif_besaud_status_change_callback)(btif_besaud_event event);

typedef void (*btif_besaud_data_received_callback)(uint8_t*, uint16_t);

typedef void (*bt_hci_acl_ecc_softbit_handler_func)(uint16_t*,uint16_t*, uint16_t, uint8_t*);

// ibrt fast ack toggle
#define IBRT_FAST_ACK_OFF                    (0)
#define IBRT_FAST_ACK_ON                     (1)
// ibrt fast direction
#define IBRT_FAST_ACK_TX                     0
#define IBRT_FAST_ACK_RX                     1
// ibrt slave flow control
#define IBRT_SLAVE_FLOW_CTRL_ON              IBRT_FAST_ACK_OFF
#define IBRT_SLAVE_FLOW_CTRL_OFF             IBRT_FAST_ACK_ON

typedef enum {
    RETRIGGER_BY_ROLE_SWITCH                  = 0,
    RETRIGGER_BY_DECODE_ERROR                 = 1,
    RETRIGGER_BY_DECODE_STATUS_ERROR          = 2,
    RETRIGGER_BY_ROLE_MISMATCH                = 3,
    RETRIGGER_BY_TRIGGER_FAIL                 = 4,
    RETRIGGER_BY_L_R_SYNC_DETECT              = 5,
    RETRIGGER_BY_SYNCHRONIZE_CNT_LIMIT        = 6,
    RETRIGGER_BY_LOW_BUFFER                   = 7,
    RETRIGGER_BY_SEQ_MISMATCH                 = 8,
    RETRIGGER_BY_AUTO_SYNC_NO_SUPPORT         = 9,
    RETRIGGER_BY_PLAYER_NOT_ACTIVE            = 10,
    RETRIGGER_BY_STATUS_ERROR                 = 11,
    RETRIGGER_BY_STREAM_RESTART               = 12,
    RETRIGGER_BY_SYNC_MISMATCH                = 13,
    RETRIGGER_BY_SYNC_FAIL                    = 14,
    RETRIGGER_BY_SYS_BUSY                     = 15,
    RETRIGGER_BY_SYNC_TARGET_CNT_ERROR        = 16,
    RETRIGGER_BY_SYNC_TARGET_MS_ERROR         = 16,
    RETRIGGER_BY_AI_STREAM                    = 17,
    RETRIGGER_BY_IN_SNIFF                     = 18,
    RETRIGGER_BY_UNKNOW                       = 19,
    RETRIGGER_BY_NON_CONSECTIVE_SEQ           = 20,
    RETRIGGER_BY_LOCAL_PCM_BUF_OVERFLOW       = 21,
    RETRIGGER_BY_OFF_CORE_BOOT_FAILED         = 22,
    RETRIGGER_BY_HEADFRAME_CHECK_FAIL         = 23,
    RETRIGGER_MAX                             = 24,
} AppIbrtAudioRetriggerType;

struct a2dp_command_t {
    bool is_valid;
    uint8_t transaction;
    uint8_t signal_id;
} __attribute__ ((packed));

struct btif_sync_data_to_new_master_t {
    bt_bdaddr_t remote_bdaddr;
    struct a2dp_command_t stream_cmd;
} __attribute__ ((packed));

typedef struct bt_ibrt_state_t {
    bool ibrt_slave_force_disc_hfp;
    bool ibrt_slave_force_disc_a2dp;
    bool ibrt_slave_force_disc_avrcp;
    bool mock_hfp_after_force_disc;
    bool mock_a2dp_after_force_disc;
    bool mock_avrcp_after_force_disc;
} bt_ibrt_state_t;

typedef enum {
    BT_IBRT_SLAVE_FORCE_DISC_HFP = 1,
    BT_IBRT_SLAVE_FORCE_DISC_A2DP,
    BT_IBRT_SLAVE_FORCE_DISC_AVRCP,
    BT_IBRT_MOCK_HFP_AFTER_FORCE_DISC,
    BT_IBRT_MOCK_A2DP_AFTER_FORCE_DISC,
    BT_IBRT_MOCK_AVRCP_AFTER_FORCE_DISC,
} BT_IBRT_FIELD_ENUM_T;

typedef enum {
    BT_PROFILE_SYNC_BDADDR,
    BT_PROFILE_SYNC_APP_A2DP,
    BT_PROFILE_SYNC_APP_AVRCP,
    BT_PROFILE_SYNC_APP_HFP,
    BT_PROFILE_SYNC_APP_SPP,
    BT_PROFILE_SYNC_APP_MAP,
    BT_PROFILE_SYNC_STACK_A2DP,
    BT_PROFILE_SYNC_STACK_AVRCP,
    BT_PROFILE_SYNC_STACK_HFP,
    BT_PROFILE_SYNC_STACK_SPP,
    BT_PROFILE_SYNC_STACK_MAP,
    BT_PROFILE_SYNC_STACK_BTGATT,
} BT_PROFILE_SYNC_ENUM_T;

void btif_ibrt_old_master_receive_ready_req(struct btif_sync_data_to_new_master_t *sync_data, const bt_bdaddr_t *remote);
void btif_ibrt_new_master_receive_ready_rsp(struct btif_sync_data_to_new_master_t *sync_data);

#ifdef __cplusplus
}
#endif
#endif /* IBRT */
#endif /* __IBRT_COMMON_DEFINE_H__ */
