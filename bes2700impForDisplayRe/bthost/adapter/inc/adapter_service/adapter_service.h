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
#ifndef __BT_ADAPTER_LAYER_H__
#define __BT_ADAPTER_LAYER_H__
#include "bt_common_define.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef int bt_status_t;
#define BT_STS_SUCCESS                      0x00   /* Host error code: Successful and complete */
#define BT_STS_FAILED                       0x01   /* Operation failed */
#define BT_STS_PENDING                      0x02   /* Successfully started but pending */
#define BT_STS_DISCONNECT                   0x03   /* Link disconnected */
#define BT_STS_NO_LINK                      0x04   /* No Link layer Connection exists */
#define BT_STS_IN_USE                       0x05   /* Operation failed - already in use. */
#define BT_STS_MEDIA_BUSY                   0x06   /* IrDA specific return codes: Media is busy */
#define BT_STS_MEDIA_NOT_BUSY               0x07   /* IRDA: Media is not busy */
#define BT_STS_NO_PROGRESS                  0x08   /* IRDA: IrLAP not making progress */
#define BT_STS_LINK_OK                      0x09   /* IRDA: No progress condition cleared */
#define BT_STS_SDU_OVERRUN                  0x0a  /* IRDA: Sent more data than current SDU size */
#define BT_STS_BUSY                         0x0b  /* Bluetooth specific return codes */
#define BT_STS_ACL_ALREADY_EXISTS           0x0b
#define BT_STS_NO_RESOURCES                 0x0c
#define BT_STS_NOT_FOUND                    0x0d
#define BT_STS_DEVICE_NOT_FOUND             0x0e
#define BT_STS_CONNECTION_FAILED            0x0f
#define BT_STS_TIMEOUT                      0x10
#define BT_STS_NO_CONNECTION                0x11
#define BT_STS_INVALID_PARM                 0x12
#define BT_STS_IN_PROGRESS                  0x13
#define BT_STS_RESTRICTED                   0x14
#define BT_STS_INVALID_TYPE                 0x15
#define BT_STS_HCI_INIT_ERR                 0x16
#define BT_STS_NOT_SUPPORTED                0x17
#define BT_STS_CONTINUE                     0x18
#define BT_STS_CANCELLED                    0x19
#define BT_STS_NOT_ALLOW                    0x1a
#define BT_STS_ONGOING                      0x1b
#define BT_STS_NOT_BOND                     0x1c
#define BT_STS_NOT_ENCRYPT                  0x1d
#define BT_STS_ERROR_RESULT                 0x1e
#define BT_STS_RECV_ERROR                   0x1f
#define BT_STS_QUEUE_FULL                   0x20
#define BT_STS_INVALID_ADV_SET_ID           0x21
#define BT_STS_INVALID_ADV_HANDLE           0x22
#define BT_STS_INVALID_ADV_INTERVAL         0x23
#define BT_STS_INVALID_PEER_ADDRESS         0x24
#define BT_STS_INVALID_CONN_HANDLE          0x25
#define BT_STS_INVALID_DURATION_VALUE       0x26
#define BT_STS_INVALID_CIS_COUNT            0x27
#define BT_STS_INVALID_CIG_ID               0x28
#define BT_STS_INVALID_CIS_ID               0x29
#define BT_STS_INVALID_CIG_ITEM             0x2A
#define BT_STS_INVALID_STATUS               0x2B
#define BT_STS_INVALID_CIS_HANDLE           0x2C
#define BT_STS_INVALID_PARAMS               0x2D
#define BT_STS_INVALID_BIG_ID               0x2E
#define BT_STS_INVALID_BIG_ITEM             0x2F
#define BT_STS_INVALID_SMP_PHASE            0x30
#define BT_STS_INVALID_METHOD               0x31
#define BT_STS_INVALID_PHASE                0x32
#define BT_STS_INVALID_EVENT                0x33
#define BT_STS_INVALID_ROLE                 0x34
#define BT_STS_INVALID_CALLBACK             0x35
#define BT_STS_INVALID_SERVICE_INSTANCE     0x36
#define BT_STS_INVALID_CHARACTER_INSTANCE   0x37
#define BT_STS_INVALID_CONN_BITFILEDS       0x38
#define BT_STS_INVALID_VALUE                0x39
#define BT_STS_INVALID_PROC_CODE            0x3a
#define BT_STS_INVALID_COUNT                0x3b
#define BT_STS_INVALID_HANDLE               0x3c
#define BT_STS_INVALID_UUID                 0x3d
#define BT_STS_INVALID_PRF_ID               0x3e
#define BT_STS_INVALID_CHAR_HANDLE          0x3f
#define BT_STS_INVALID_INCL_HANDLE          0x40
#define BT_STS_INVALID_SERV_HANDLE          0x41
#define BT_STS_INVALID_PDU_LEN              0x42
#define BT_STS_INVALID_BEARER               0x43
#define BT_STS_INVALID_TIMER_ID             0x44
#define BT_STS_INVALID_RSP_OPCODE           0x44
#define BT_STS_INVALID_REQ_OPCODE           0x45
#define BT_STS_INVALID_OPCODE               0x46
#define BT_STS_INVALID_PROC                 0x47
#define BT_STS_INVALID_MTU                  0x48
#define BT_STS_INVALID_REGISTER             0x49
#define BT_STS_INVALID_VALUE_LEN            0x4a
#define BT_STS_INVALID_DESC_TYPE            0x4b
#define BT_STS_INVALID_DESC_VALUE           0x4c
#define BT_STS_INVALID_CHAR_TYPE            0x4d
#define BT_STS_INVALID_CHAR_VALUE           0x4e
#define BT_STS_INVALID_CHAR_VALUE_TYPE      0x4f
#define BT_STS_INVALID_CHAR_VALUE_VALUE     0x50
#define BT_STS_INVALID_INCL_TYPE            0x51
#define BT_STS_INVALID_INCL_VALUE           0x52
#define BT_STS_INVALID_SERV_TYPE            0x53
#define BT_STS_INVALID_SERV_VALUE           0x54
#define BT_STS_INVALID_ADDRESS              0x55
#define BT_STS_INVALID_STATE                0x56
#define BT_STS_INVALID_HEADER_LEN           0x57
#define BT_STS_INVALID_DEVICE_ID            0x58
#define BT_STS_INVALID_CONN_INDEX           0x59
#define BT_STS_SET_ENABLED_ADV_PARAMS       0x60
#define BT_STS_COBUF_MALLOC_FAILED          0x61
#define BT_STS_PPBUF_MALLOC_FAILED          0x62
#define BT_STS_ALLOC_HCI_CMD_FAILED         0x63
#define BT_STS_HIGH_DUTY_ADV_NOT_SUPPORT    0x64
#define BT_STS_ADV_DATA_TOO_LONG            0x65
#define BT_STS_SCAN_RSP_DATA_TOO_LONG       0x66
#define BT_STS_SET_ADV_PARAMS_FAILED        0x67
#define BT_STS_ENABLE_ADV_FAILED            0x68
#define BT_STS_NOT_IDENTITY_ADDRESS         0x69
#define BT_STS_DISABLE_ADV_FAILED           0x6A
#define BT_STS_RESOUCE_NOT_AVAILABLE        0x6B
#define BT_STS_REACH_MAX_NUMBER             0x6C
#define BT_STS_CIS_ALREADY_OPENED           0x6D
#define BT_STS_TOO_MUCH_BIS_COUNT           0x6E
#define BT_STS_ISO_CONIDX_NOT_FOUND         0x6F
#define BT_STS_SET_ADV_PARAM_FAILED         0x70
#define BT_STS_SCAN_FORCE_CANCELED          0x71
#define BT_STS_SCAN_TIMER_TIMEOUT           0x72
#define BT_STS_ADV_TIMER_TIMEOUT            0x73
#define BT_STS_ADV_CANCEL_AND_CONTINUE      0x74
#define BT_STS_INIT_TIMER_TIMEOUT           0x75
#define BT_STS_INIT_FORCE_CANCELED          0x76
#define BT_STS_INIT_CANCEL_AND_CONTINUE     0x77
#define BT_STS_START_ADV_TIMER_FAILED       0x78
#define BT_STS_START_SCAN_TIMER_FAILED      0x79
#define BT_STS_START_INIT_TIMER_FAILED      0x7A
#define BT_STS_CIS_IS_NOT_CONFIGUABLE       0x7B
#define BT_STS_SEND_CMD_FAILED              0x7C
#define BT_STS_POWER_OFF                    0x7D
#define BT_STS_NO_DEVICE_IN_FILTER_LIST     0x7E
#define BT_STS_ADDRESS_RESO_ENABLE          0x7F
#define BT_STS_RESOLV_DISABLE_ADVTIVITY     0x80
#define BT_STS_INIT_PENDING_WHEN_OP_RESOLV  0x81
#define BT_STS_ADV_ENABLED_WHEN_OP_FILTER   0x82
#define BT_STS_SCAN_ENABLED_WHEN_OP_FILTER  0x83
#define BT_STS_INIT_PENDING_WHEN_OP_FILTER  0x84
#define BT_STS_ADDRESS_ALREADY_IN_LIST      0x85
#define BT_STS_ADDRESS_NOT_IN_THE_LIST      0x86
#define BT_STS_LIST_IS_FULL                 0x87
#define BT_STS_PAIRING_OR_ENC_ONGODING      0x88
#define BT_STS_BREDR_SMP_PAIRING_DISALLOW   0x89
#define BT_STS_SMP_PAIIRNG_NOT_CENTRAL      0x8A
#define BT_STS_PEER_REQUIRE_MITM_PROTECT    0x8B
#define BT_STS_PEER_REQUIRE_SEC_PAIRING     0x8C
#define BT_STS_MITM_PROTECT_REQUIRED        0x8D
#define BT_STS_SECURE_PAIRING_REQUIRED      0x8E
#define BT_STS_SHORT_ENC_KEY_SIZE           0x8F
#define BT_STS_MORE_STRENTH_METHOD_REQUIRED 0x90
#define BT_STS_REQUEST_CENTRAL_ENCRYPT      0x91
#define BT_STS_SECURITY_REQ_NOT_ALLOWED     0x92
#define BT_STS_MTU_IS_NOT_EXCHANGED         0x93
#define BT_STS_GET_TX_BEARER_FAILED         0x94
#define BT_STS_ATT_READ_ERROR               0x95
#define BT_STS_ATT_WRITE_ERROR              0x96
#define BT_STS_DISCOVER_SERVICE_FAILED      0x97
#define BT_STS_DISCOVER_INCLUDE_FAILED      0x98
#define BT_STS_DISCOVER_CHARACTER_FAILED    0x99
#define BT_STS_DISCOVER_DESCRIPTOR_FAILED   0x9A
#define BT_STS_READ_WRITE_PREPARE_FAILED    0x9B
#define BT_STS_READ_VALUE_FAILED            0x9C
#define BT_STS_SEND_DATA_FAILED             0x9D
#define BT_STS_IRK_DISTED_DEVICE_NOT_EXIST  0x9E
#define BT_STS_NOT_RANDOM_STATIC_ADDRESS    0x9F
#define BT_STS_ALLOC_ADVERTISING_FAILED     0xa0
#define BT_STS_PREV_ADV_FORCE_CANCELED      0xa1
#define BT_STS_ADV_CANCELED_AND_RESTART     0xa2
#define BT_STS_ADV_FORCE_DISABLED           0xa3
#define BT_STS_HFP_SCO_ACTIVE               0xa4
#define BT_STS_ADV_STATUS                   0xa5
#define BT_STS_ADV_DATA_STATUS              0xa6
#define BT_STS_SCAN_STATUS                  0xa7
#define BT_STS_INIT_STATUS                  0xa8
#define BT_STS_CONN_STATUS                  0xa9
#define BT_STS_GLOBAL_STATUS                0xaa
#define BT_STS_START_ADV                    0xab
#define BT_STS_START_CONN_ADV               0xac
#define BT_STS_REFRESH_ADV                  0xad
#define BT_STS_SMP_PAIRING                  0xae
#define BT_STS_SMP_ENCRYPTION               0xaf
#define BT_STS_LTK_TO_LINKKEY               0xb0
#define BT_STS_LINKKEY_TO_LTK               0xb1
#define BT_STS_SMP_GEN_LTK                  0xb2
#define BT_STS_SMP_RECV_LTK                 0xb3
#define BT_STS_SMP_REPLY_LTK                0xb4
#define BT_STS_FREE_CIG_CIS                 0xb5
#define BT_STS_FREE_BIG_BIS                 0xb6
#define BT_STS_NEW_CIG_CIS                  0xb7
#define BT_STS_REAL_LINK_DISCONNECTED       0xb8 // dont change
#define BT_STS_BT_CANCEL_PAGE               0xb9 // dont change
#define BT_STS_BT_DISCONNECT_ITSELF         0xba // dont change
#define BT_STS_BT_IBRT_SLAVE_CLEANUP        0xbb // dont change
#define BT_STS_PAST_RECEIVED                0xbc
#define BT_STS_TERM_SYNC_STATUS             0xbd
#define BT_STS_CREATE_SYNC_STATUS           0xbe
#define BT_STS_PA_SYNC_LOST                 0xbf
#define BT_STS_PA_SYNC_ESTABLISH            0xc0
#define BT_STS_DATA_LENGTH_UPDATE           0xc1
#define BT_STS_RECV_L2CAP_PARAM_UPDATE_RSP  0xc2
#define BT_STS_CONN_ESTABLISHED             0xc3
#define BT_STS_CREATE_LE_CONN               0xc4
#define BT_STS_START_LE_SCAN                0xc5
#define BT_STS_READ_LE_FEATURE              0xc6
#define BT_STS_MAX_ADV_DATA_LENGTH          0xc7
#define BT_STS_NOT_READY                    0xc8 // 200 dont change
#define BT_STS_QUERY_FAILED                 0xc9 // 201 dont change
#define BT_STS_ALREADY_EXIST                0xca // 202 dont change
#define BT_STS_MAX_ADV_SET_NUM              0xcb
#define BT_STS_MAX_FILTER_LIST_SIZE         0xcc
#define BT_STS_MAX_RESOLVING_LIST_SIZE      0xcd
#define BT_STS_BR_START_CTKD                0xce
#define BT_STS_LE_START_CTKD                0xcf
#define BT_STS_SET_KEY_STRENGTH             0xd0
#define BT_STS_SET_SECURITY_LEVEL           0xd1
#define BT_STS_LOCAL_CSRK                   0xd2
#define BT_STS_LOCAL_IRK                    0xd3
#define BT_STS_PEER_IDENTITY                0xd4
#define BT_STS_LEGACY_EDIV_LTK              0xd5
#define BT_STS_GAP_GEN_LRPA                 0xd6
#define BT_STS_GATT_CLIENT_SUPP_FEATURE     0xd7
#define BT_STS_GATT_SERVER_SUPP_FEATURE     0xd8
#define BT_STS_GATT_DEVICE_NAME             0xd9
#define BT_STS_GATT_APPEARANCE              0xda
#define BT_STS_GATT_CENTRAL_ADDR_RESOLUTION 0xdb
#define BT_STS_ONLY_USE_RPA_AFTER_BONDING   0xdc
#define BT_STS_PERIPH_PERFERRED_CONN_PARAMS 0xdd
#define BT_STS_GATT_PEER_SEC_LEVEL          0xde
#define BT_STS_GATT_DATABASE_HASH_VALUE     0xdf
#define BT_STS_CHARACTER_NOT_REGISTERED     0xe0
#define BT_STS_LENGTH_TOO_LONG              0xe1
#define BT_STS_BEARER_NOT_FOUND             0xe2
#define BT_STS_ALLOC_FAILED                 0xe3
#define BT_STS_PREPARE_WRITE_NOT_MATCH      0xe4
#define BT_STS_REACH_UPPER_LIMIT            0xe5
#define BT_STS_REACH_LOWER_LIMIT            0xe6
#define BT_STS_COUNT_NOT_MATCH              0xe7
#define BT_STS_HANDLE_NOT_IN_ORDER          0xe8
#define BT_STS_SERVICE_NOT_MATCH            0xe9
#define BT_STS_GATT_ADD_INCLUDE             0xea
#define BT_STS_GATT_ADD_SERVICE             0xeb
#define BT_STS_ALREADY_REGISTERED           0xec
#define BT_STS_PRF_REGISTER                 0xed
#define BT_STS_SVC_REGISTER                 0xee
#define BT_STS_CANT_UNREGISTER              0xef
#define BT_STS_RECV_REQ                     0xf0
#define BT_STS_RECV_RSP                     0xf1
#define BT_STS_NOT_SUPP_MULTI_CONN          0xf2
#define BT_STS_OPEN_PROFILE                 0xf3
#define BT_STS_OPEN_SERVICE                 0xf4
#define BT_STS_CLOSE_PROFILE                0xf3
#define BT_STS_CLOSE_SERVICE                0xf4
#define BT_STS_GATT_CONN_READY              0xf5
#define BT_STS_MTU_EXCHANGED                0xf6
#define BT_STS_START_TIMER_FAILED           0xf7
#define BT_STS_ATT_BEARER                   0xf8
#define BT_STS_SEND_REQ                     0xf9
#define BT_STS_SEND_RSP                     0xfa
#define BT_STS_PRF_UNREGISTER               0xfb
#define BT_STS_SVC_UNREGISTER               0xfc
#define BT_STS_GATT_PROC                    0xfd
#define BT_STS_RECV_ERROR_RSP               0xfe
#define BT_STS_SEND_ERROR_RSP               0xff
#define BT_STS_REGISTER_NOTIFY              0x100
#define BT_STS_REGISTER_NOT_FOUND           0x101
#define BT_STS_GATT_SERVICE                 0x102
#define BT_STS_GATT_INCLUDE                 0x103
#define BT_STS_GATT_CHARACTER               0x104
#define BT_STS_GATT_DESCRIPTOR              0x105
#define BT_STS_SERVICE_NOT_FOUND            0x106
#define BT_STS_CHARACTER_NOT_FOUND          0x107
#define BT_STS_SERVICE_EVENT                0x108
#define BT_STS_SECURITY_KEY                 0x109
#define BT_STS_EMPTY_PPB                    0x10a
#define BT_STS_REALLOC_PPB_FAILED           0x10b
#define BT_STS_PPB_PUT_INVALID_DATA         0x10c
#define BT_STS_PPB_PUT_DATA_TOO_LONG        0x10d
#define BT_STS_PPB_FREE_NOT_ALLOWED         0x10e
#define BT_STS_COBUF_INVALID_NEXT_BLOCK     0x10f
#define BT_STS_COBUF_INVALID_HANDLE         0x110
#define BT_STS_COBUF_INVALID_ALLOC_SIZE     0x111
#define BT_STS_COBUF_NULL_BUFF_PTR          0x112
#define BT_STS_COBUF_ALLOC_FAILED           0x113
#define BT_STS_COBUF_ALLOC_PEAK_SIZE        0x114
#define BT_STS_ACCEPT_CONN_UPDATE_REQ       0x115
#define BT_STS_SEND_CONN_UPDATE_REQ         0x116
#define BT_STS_RESOLVE_RPA_FAILED           0x117
#define BT_STS_DEVICE_NOT_PAIRED            0x118
#define BT_STS_SET_FILTER_LIST              0x119
#define BT_STS_OWN_ADDRESS_TYPE             0x11a
#define BT_STS_DIFFERENT_BLE_IDENTITY_ADDR  0x11b
#define BT_STS_RECORD_NOT_FOUND             0x11c
#define BT_STS_REFRESH_ADV_STATE            0x11d
#define BT_STS_NEW_BIG_BIS                  0x11e
#define BT_STS_BLE_ONLY_ENABLED             0x11f
#define BT_STS_CENTRAL_ROLE_NOT_SUPPORT     0x120
#define BT_STS_NOT_REGISTERED               0x121
#define BT_STS_CHANNEL_STATUS               0x122
#define BT_STS_INVALID_GFPS                 0x123
#define BT_STS_SEC_IO_CAP                   0x124
#define BT_STS_SEC_AUTH_REQUIRE             0x125
#define BT_STS_MAX_PA_LIST_SIZE             0x126
#define BT_STS_NOT_SUPPORT                  0x127
#define BT_STS_RX_BLOCK_AUTH_FAIL           0x128
#define BT_STS_TX_BLOCK_AUTH_FAIL           0x129
#define BT_STS_EATT_BLOCK_AUTH_FAIL         0x12a
#define BT_STS_NOT_AUTHENTICATED            0x12b
#define BT_STS_NOT_AUTHORIZED               0x12c
#define BT_STS_NOT_ENCRYPTED                0x12d
#define BT_STS_TERMINATE                    0x12e
#define BT_STS_NO_BONDED_DEVICE             0x12f
#define BT_STS_TOO_MANY_ACTIVE_CCCD         0x130
#define BT_STS_PEER_SERVICE_CHANGED         0x131
#define BT_STS_DATABASE_HASH                0x132
#define BT_STS_GEN_DATABASE_HASH_FAILED     0x133
#define BT_STS_SERVICE_CHANGED              0x134
#define BT_STS_SMP_AUTHENCIATION            0x135
#define BT_STS_INVALID_BIG_COUNT            0x136
#define BT_STS_INVALID_BIS_INDEX            0x137
#define BT_STS_INVALID_ISO_CONIDX           0x138
#define BT_STS_INVALID_ISO_HANDLE           0x139
#define BT_STS_INVALID_CIG_OP_ID            0x13a
#define BT_STS_INVALID_CONN_TYPE            0x13b
#define BT_STS_INVALID_SMP_CONN             0x13c
#define BT_STS_INVALID_UUID_LEN             0x13d
#define BT_STS_INVALID_LENGTH               0x13e
#define BT_STS_INVALID_GATT_SERVICE         0x13f
#define BT_STS_INVALID_GATT_INCLUDE         0x140
#define BT_STS_INVALID_GATT_CHARACTER       0x141
#define BT_STS_INVALID_GATT_DESCRIPTOR      0x142
#define BT_STS_INVALID_BDADDR               0x143
#define BT_STS_INVALID_LOCAL_ADDRESS        0x144
#define BT_STS_INVALID_ADV_DATA             0x145
#define BT_STS_ALREADY_DONE_OR_ERROR        0x146
#define BT_STS_DISABLE_ADVERTISING          0x147
#define BT_STS_CONN_ADV_PARAM_NOT_FOUND     0x148
#define BT_STS_BTGATT_PROFILE_DATA_SAVE     0x149
#define BT_STS_BTGATT_PROFILE_DATA_RESTORE  0x14A
#define BT_STS_CANCEL_INITIATING            0x14B
#define BT_STS_RESO_LIST_ADD_ALL            0x14C
#define BT_STS_INVALID_CHANNEL_ID           0x14D
#define BT_STS_INVALID_PEER_ID              0x14E
#define BT_STS_INVALID_CHANNEL              0x14F
#define BT_STS_INVALID_CONNECTION           0x150
#define BT_STS_INVALID_IDENTIFIER           0x151
#define BT_STS_INVALID_FLAG                 0x152
#define BT_STS_INVALID_ATTR_HANDLE          0x153
#define BT_STS_ADD_FAILED                   0x154
#define BT_STS_CURR_PROC_WAIT_MORE_PDU      0x155
#define BT_STS_NOT_IN_THE_SAME_SERVICE      0x156
#define BT_STS_NOT_SAME_TYPE                0x157
#define BT_STS_ACTIVE_STATUS                0x158
#define BT_STS_NO_CLIENT_NOTIFIED           0x159
#define BT_STS_AUTHOR_REQ_FAILED            0x15A
#define BT_STS_READ_REQ_FAILED              0x15B
#define BT_STS_WRITE_REQ_FAILED             0x15C
#define BT_STS_CALL_METHOD_FAILED           0x15D
#define BT_STS_OUTSTANDING_CMD_MISMATCH     0x15E
#define BT_STS_OUTSTANDING_CMD_IS_NULL      0x15F
#define BT_STS_SLE_ADDRESS                  0x160
#define BT_STS_CREATE_FAILED                0x161
#define BT_STS_CANCEL_FAILED                0x162
#define BT_STS_WRITE_WORKING_STATE          0x163
#define BT_STS_WRITE_REPORT_INFO            0x164
#define BT_STS_NEAR_SERVICE                 0x165
#define BT_STS_NEAR_PROPERTY                0x166
#define BT_STS_PROPERTY_NOT_FOUND           0x167
#define BT_STS_START_DISCOVER               0x168
#define BT_STS_REPORT_NOT_FOUND             0x169
#define BT_STS_NEAR_HID_REPORT              0x16A
#define BT_STS_INVALID_DESCRIPTOR           0x16B
#define BT_STS_RECV_REPORT                  0x16C
#define BT_STS_RECV_SERVICE                 0x16D
#define BT_STS_HID_DESCRIPTOR               0x16E
#define BT_STS_WORKING_STATE                0x16F
#define BT_STS_REPORT_INFO                  0x170
#define BT_STS_READ_REQ                     0x171
#define BT_STS_WRITE_REQ                    0x172
#define BT_STS_WRITE_RSP                    0x173
#define BT_STS_GATT_DEFER_RSP               0x174
#define BT_STS_INVALID_DEFER_CODE           0x175
#define BT_STS_SET_DECISION_DATA            0x176
#define BT_STS_DECISION_RESOLVABLE_TAG      0x177
#define BT_STS_SET_DECISION_INSTRUCTIONS    0x178
#define BT_STS_ADD_ADVS_TO_MONITORED_LIST   0x179
#define BT_STS_RM_ADVS_TO_MONITORED_LIST    0x17A
#define BT_STS_CLR_ADVS_TO_MONITORED_LIST   0x17B
#define BT_STS_EN_MONITORING_ADVERTISERS    0x17C
#define BT_STS_CS_MGR_INIT                  0x17D
#define BT_STS_CS_CTRL_EVT_CB_REGISTER      0x17E
#define BT_STS_CS_UNKNOWN_CMD               0x17F
#define BT_STS_CS_UNKNOWN_EVENT             0x180
#define BT_STS_CS_CONFIG_ID_OVERFLOW        0x181
#define BT_STS_CS_NO_REMOTE                 0x182
#define BT_STS_CS_NO_CONN_MGR               0x183
#define BT_STS_CS_UNKNOWN_SPEC_INFO         0x184
#define BT_STS_CS_UNMATACHED_PKT_LEN        0x185
#define BT_STS_CS_CONN_MGR_ALLOC_FAILED     0x186
#define BT_STS_CS_CONNECTED                 0x187
#define BT_STS_CS_DISCONNECTED              0x188
#define BT_STS_CS_NO_GLOBAL_MGR             0x189
#define BT_STS_CS_SEND_CMD                  0x18A
#define BT_STS_CS_CMD_FAILED                0x18B
#define BT_STS_CS_EMPTY_POINTER             0x18C
#define BT_STS_CS_DEFAULT_SETTINGS_INVALID  0x18D
#define BT_STS_CS_ALLOC_CMD_FAILED          0x18E
#define BT_STS_CS_NUM_ANTENNA_PATH_UNMATCH  0x18F
#define BT_STS_SRV_USER_REGISTER            0x200
#define BT_STS_SRV_USER_UNREGISTER          0x201

/*BT Link layer Error*/
#define BT_LL_ERR_NO_ERROR                  0x00
#define BT_LL_ERR_UNKNOWN_HCI_COMMAND       0x01
#define BT_LL_ERR_UNKNOWN_CONNECTION_ID     0x02
#define BT_LL_ERR_HARDWARE_FAILURE          0x03
#define BT_LL_ERR_PAGE_TIMEOUT              0x04
#define BT_LL_ERR_AUTH_FAILURE              0x05
#define BT_LL_ERR_PIN_MISSING               0x06
#define BT_LL_ERR_MEMORY_CAPA_EXCEED        0x07
#define BT_LL_ERR_CON_TIMEOUT               0x08
#define BT_LL_ERR_CON_LIMIT_EXCEED          0x09
#define BT_LL_ERR_SYNC_CON_LIMIT_DEV_EXCEED 0x0A
#define BT_LL_ERR_CON_ALREADY_EXISTS        0x0B
#define BT_LL_ERR_COMMAND_DISALLOWED        0x0C
#define BT_LL_ERR_CON_REJ_LIMITED_RES       0x0D
#define BT_LL_ERR_CON_REJ_SECURITY_REASONS  0x0E
#define BT_LL_ERR_CON_REJ_UNACCEPT_BDADDR   0x0F
#define BT_LL_ERR_CON_ACCEPT_TIMEOUT_EXCEED 0x10
#define BT_LL_ERR_UNSUPPORTED               0x11
#define BT_LL_ERR_INVALID_HCI_PARAM         0x12
#define BT_LL_ERR_REMOTE_USER_TERM_CON      0x13
#define BT_LL_ERR_REMOTE_DEV_TERM_LOW_RES   0x14
#define BT_LL_ERR_REMOTE_DEV_POWER_OFF      0x15
#define BT_LL_ERR_CON_TERM_BY_LOCAL_HOST    0x16
#define BT_LL_ERR_REPEATED_ATTEMPTS         0x17
#define BT_LL_ERR_PAIRING_NOT_ALLOWED       0x18
#define BT_LL_ERR_UNKNOWN_LMP_PDU           0x19
#define BT_LL_ERR_UNSUPPORTED_REMOTE_FEAT   0x1A
#define BT_LL_ERR_SCO_OFFSET_REJECTED       0x1B
#define BT_LL_ERR_SCO_INTERVAL_REJECTED     0x1C
#define BT_LL_ERR_SCO_AIR_MODE_REJECTED     0x1D
#define BT_LL_ERR_INVALID_LMP_PARAM         0x1E
#define BT_LL_ERR_UNSPECIFIED_ERROR         0x1F
#define BT_LL_ERR_UNSUPPORTED_LMP_PARAM     0x20
#define BT_LL_ERR_ROLE_CHANGE_NOT_ALLOWED   0x21
#define BT_LL_ERR_LMP_RSP_TIMEOUT           0x22
#define BT_LL_ERR_LMP_COLLISION             0x23
#define BT_LL_ERR_LMP_PDU_NOT_ALLOWED       0x24
#define BT_LL_ERR_ENC_MODE_NOT_ACCEPT       0x25
#define BT_LL_ERR_LINK_KEY_CANT_CHANGE      0x26
#define BT_LL_ERR_QOS_NOT_SUPPORTED         0x27
#define BT_LL_ERR_INSTANT_PASSED            0x28
#define BT_LL_ERR_PAIR_UNIT_KEY_NOT_SUP     0x29
#define BT_LL_ERR_DIFF_TRANS_COLLISION      0x2A
#define BT_LL_ERR_QOS_UNACCEPTABLE_PARAM    0x2C
#define BT_LL_ERR_QOS_REJECTED              0x2D
#define BT_LL_ERR_CHANNEL_CLASS_NOT_SUP     0x2E
#define BT_LL_ERR_INSUFFICIENT_SECURITY     0x2F
#define BT_LL_ERR_PARAM_OUT_OF_MAND_RANGE   0x30
#define BT_LL_ERR_ROLE_SWITCH_PEND          0x32 /* LM_ROLE_SWITCH_PENDING               */
#define BT_LL_ERR_RESERVED_SLOT_VIOLATION   0x34 /* LM_RESERVED_SLOT_VIOLATION           */
#define BT_LL_ERR_ROLE_SWITCH_FAIL          0x35 /* LM_ROLE_SWITCH_FAILED                */
#define BT_LL_ERR_EIR_TOO_LARGE             0x36 /* LM_EXTENDED_INQUIRY_RESPONSE_TOO_LARGE */
#define BT_LL_ERR_SP_NOT_SUPPORTED_HOST     0x37
#define BT_LL_ERR_HOST_BUSY_PAIRING         0x38
#define BT_LL_ERR_CONTROLLER_BUSY           0x3A
#define BT_LL_ERR_UNACCEPTABLE_CON_PARAM    0x3B
#define BT_LL_ERR_ADV_TIMEOUT               0x3C
#define BT_LL_ERR_TERMINATED_MIC_FAILURE    0x3D
#define BT_LL_ERR_CON_FAILED_TO_BE_EST      0x3E
#define BT_LL_ERR_CCA_REJ_USE_CLOCK_DRAG    0x40
#define BT_LL_ERR_TYPE0_SUBMAP_NOT_DEFINED  0x41
#define BT_LL_ERR_UNKNOWN_ADVERTISING_ID    0x42
#define BT_LL_ERR_LIMIT_REACHED             0x43
#define BT_LL_ERR_OP_CANCELED_BY_HOST       0x44
#define BT_LL_ERR_PKT_TOO_LONG              0x45

typedef enum {
    NEAR_STS_SUCCESS                      = 0x00,
    NEAR_STS_FAILED                       ,
    NEAR_STS_PENDING                      ,
    NEAR_STS_ONGOING                      ,
    NEAR_STS_IN_PROGRESS                  ,
    NEAR_STS_IN_USE                       ,
    NEAR_STS_IS_EMPTY                     ,
    NEAR_STS_IS_FULL                      ,
    NEAR_STS_IS_BUSY                      ,
    NEAR_STS_POWER_OFF                    ,
    NEAR_STS_LIST_FULL                    ,
    NEAR_STS_QUEUE_FULL                   ,
    NEAR_STS_ERROR_RESULT                 ,
    NEAR_STS_ERROR_STATUS                 ,
    NEAR_STS_RECV_ERROR                   ,
    NEAR_STS_START_FAILED                 ,
    NEAR_STS_STOP_FAILED                  ,
    NEAR_STS_ENABLE_FAILED                ,
    NEAR_STS_DISABLE_FAILED               ,
    NEAR_STS_READ_FAILED                  ,
    NEAR_STS_WRITE_FAILED                 ,
    NEAR_STS_CONFIG_FAILED                ,
    NEAR_STS_SET_FAILED                   ,
    NEAR_STS_ADD_FAILED                   ,
    NEAR_STS_CONN_FAILED                  ,
    NEAR_STS_SEND_FAILED                  ,
    NEAR_STS_SEND_DATA_FAILED             ,
    NEAR_STS_SEND_CMD_FAILED              ,
    NEAR_STS_CREATE_FAILED                ,
    NEAR_STS_CANCEL_FAILED                ,
    NEAR_STS_REGISTER_FAILED              ,
    NEAR_STS_AUTHOR_REQ_FAILED            ,
    NEAR_STS_READ_REQ_FAILED              ,
    NEAR_STS_WRITE_REQ_FAILED             ,
    NEAR_STS_CALL_METHOD_FAILED           ,
    NEAR_STS_APP_INIT                     ,
    NEAR_STS_INITIALIZE                   ,
    NEAR_STS_APP_READY                    ,
    NEAR_STS_STACK_READY                  ,
    NEAR_STS_CANCELLED                    ,
    NEAR_STS_RESTRICTED                   ,
    NEAR_STS_CONNECT                      ,
    NEAR_STS_DISCONNECT                   ,
    NEAR_STS_TERMINATE                    ,
    NEAR_STS_REGISTER                     ,
    NEAR_STS_UNREGISTER                   ,
    NEAR_STS_CANCEL                       ,
    NEAR_STS_DISCOVER                     ,
    NEAR_STS_START_DISCOVER               ,
    NEAR_STS_CONN_OPENED                  ,
    NEAR_STS_CONN_CLOSED                  ,
    NEAR_STS_CHANNEL_OPENED               ,
    NEAR_STS_CHANNEL_CLOSED               ,
    NEAR_STS_INITIATING                   ,
    NEAR_STS_SCANNING                     ,
    NEAR_STS_START_SCAN                   ,
    NEAR_STS_STOP_SCAN                    ,
    NEAR_STS_BROADCAST                    ,
    NEAR_STS_START_BROADCAST              ,
    NEAR_STS_STOP_BROADCAST               ,
    NEAR_STS_TIMEOUT                      ,
    NEAR_STS_PROC_TIMEOUT                 ,
    NEAR_STS_MULTIPLE_INSTANCE            ,
    NEAR_STS_PEER_MULTIPLE_INSTANCE       ,
    NEAR_STS_NO_LINK                      ,
    NEAR_STS_NO_CONNECTION                ,
    NEAR_STS_NO_RESOURCES                 ,
    NEAR_STS_NO_BOND_DEVICE               ,
    NEAR_STS_NO_CLIENT_NOTIFIED           ,
    NEAR_STS_NOT_AUTHENTICATED            ,
    NEAR_STS_NOT_AUTHORIZED               ,
    NEAR_STS_NOT_ENCRYPTED                ,
    NEAR_STS_NOT_READY                    ,
    NEAR_STS_NOT_OPEN                     ,
    NEAR_STS_NOT_ALLOW                    ,
    NEAR_STS_NOT_AVAILABLE                ,
    NEAR_STS_NOT_IN_LIST                  ,
    NEAR_STS_NOT_IN_ORDER                 ,
    NEAR_STS_NOT_MATCH                    ,
    NEAR_STS_NOT_SAME_TYPE                ,
    NEAR_STS_NOT_IN_SAME_SERVICE          ,
    NEAR_STS_NOT_SUPPORT                  ,
    NEAR_STS_NOT_FOUND                    ,
    NEAR_STS_CONN_NOT_FOUND               ,
    NEAR_STS_LINK_NOT_FOUND               ,
    NEAR_STS_CHANNEL_NOT_FOUND            ,
    NEAR_STS_RECROD_NOT_FOUND             ,
    NEAR_STS_DEVICE_NOT_FOUND             ,
    NEAR_STS_SERVICE_NOT_FOUND            ,
    NEAR_STS_PROPERTY_NOT_FOUND           ,
    NEAR_STS_REPORT_NOT_FOUND             ,
    NEAR_STS_USE_EXIST_CONN               ,
    NEAR_STS_CONN_ALREADY_EXIST           ,
    NEAR_STS_ALREADY_EXIST                ,
    NEAR_STS_ALREADY_OPEN                 ,
    NEAR_STS_ALREADY_IN_LIST              ,
    NEAR_STS_ALREADY_REGISTERED           ,
    NEAR_STS_INVALID_PARM                 ,
    NEAR_STS_INVALID_TYPE                 ,
    NEAR_STS_INVALID_ID                   ,
    NEAR_STS_INVALID_INSTANCE             ,
    NEAR_STS_INVALID_INSTANCE_ID          ,
    NEAR_STS_INVALID_IDENTIFIER           ,
    NEAR_STS_INVALID_PEER_ID              ,
    NEAR_STS_INVALID_CHANNEL_ID           ,
    NEAR_STS_INVALID_TCID                 ,
    NEAR_STS_INVALID_OPID                 ,
    NEAR_STS_INVALID_UUID                 ,
    NEAR_STS_INVALID_UUID_LEN             ,
    NEAR_STS_INVALID_VALUE                ,
    NEAR_STS_INVALID_LENGTH               ,
    NEAR_STS_INVALID_SIZE                 ,
    NEAR_STS_INVALID_DATA                 ,
    NEAR_STS_INVALID_PROC                 ,
    NEAR_STS_INVALID_OPCODE               ,
    NEAR_STS_INVALID_MTU                  ,
    NEAR_STS_INVALID_PDU                  ,
    NEAR_STS_INVALID_FRAME                ,
    NEAR_STS_INVALID_HEADER               ,
    NEAR_STS_INVALID_CONN                 ,
    NEAR_STS_INVALID_CHANNEL              ,
    NEAR_STS_INVALID_CONIDX               ,
    NEAR_STS_INVALID_INDEX                ,
    NEAR_STS_INVALID_HANDLE               ,
    NEAR_STS_INVALID_CONN_HANDLE          ,
    NEAR_STS_INVALID_ATTR_HANDLE          ,
    NEAR_STS_INVALID_SERVICE_HANDLE       ,
    NEAR_STS_INVALID_PROPERTY_HANDLE      ,
    NEAR_STS_INVALID_INTERVAL             ,
    NEAR_STS_INVALID_WINDOW               ,
    NEAR_STS_INVALID_DURATION             ,
    NEAR_STS_INVALID_PERIOD               ,
    NEAR_STS_INVALID_TIMEOUT              ,
    NEAR_STS_INVALID_PEER_TYPE            ,
    NEAR_STS_INVALID_ADDR_TYPE            ,
    NEAR_STS_INVALID_ADDRESS              ,
    NEAR_STS_INVALID_PEER_ADDRESS         ,
    NEAR_STS_INVALID_COUNT                ,
    NEAR_STS_INVALID_ITEM                 ,
    NEAR_STS_INVALID_FLAG                 ,
    NEAR_STS_INVALID_STATE                ,
    NEAR_STS_INVALID_STATUS               ,
    NEAR_STS_INVALID_METHOD               ,
    NEAR_STS_INVALID_PHASE                ,
    NEAR_STS_INVALID_EVENT                ,
    NEAR_STS_INVALID_ROLE                 ,
    NEAR_STS_INVALID_CALLBACK             ,
    NEAR_STS_INVALID_SERVICE              ,
    NEAR_STS_INVALID_PROPERTY             ,
    NEAR_STS_INVALID_SERVICE_ATTR         ,
    NEAR_STS_INVALID_PROPERTY_ATTR        ,
    NEAR_STS_INVALID_REPORT               ,
    NEAR_STS_INVALID_DESCRIPOTR           ,
    NEAR_STS_INVALID_BITFILED             ,
    NEAR_STS_ALLOC_FAILED                 ,
    NEAR_STS_ALLOC_PPB_FAILED             ,
    NEAR_STS_ALLOC_HCI_FAILED             ,
    NEAR_STS_REACH_MAX_SIZE               ,
    NEAR_STS_REACH_MAX_COUNT              ,
    NEAR_STS_REACH_MAX_NUMBER             ,
    NEAR_STS_LENGTH_TOO_LONG              ,
    NEAR_STS_SIZE_TOO_LARGE               ,
    NEAR_STS_DATA_TOO_LONG                ,
    NEAR_STS_VALUE_TOO_LARGE              ,
    NEAR_STS_REACH_UPPER_LIMIT            ,
    NEAR_STS_REACH_LOWER_LIMIT            ,
    NEAR_STS_OVERFLOW                     ,
    NEAR_STS_UNDERFLOW                    ,
    NEAR_STS_WAIT_MORE_PDU                ,
    NEAR_STS_RECV_REQ                     ,
    NEAR_STS_RECV_RSP                     ,
    NEAR_STS_RECV_SERVICE                 ,
    NEAR_STS_RECV_PROPERTY                ,
    NEAR_STS_RECV_REPORT                  ,
    NEAR_STS_SEND_REQ                     ,
    NEAR_STS_SEND_RSP                     ,
    NEAR_STS_OUTSTANDING_CMD_MISMATCH     ,
    NEAR_STS_OUTSTANDING_CMD_IS_NULL      ,
    NEAR_STS_SLE_ADDRESS                  ,
    NEAR_STS_WRITE_WORKING_STATE          ,
    NEAR_STS_WRITE_REPORT_INFO            ,
    NEAR_STS_SERVICE                      ,
    NEAR_STS_PROPERTY                     ,
    NEAR_STS_PEER_SERVICE                 ,
    NEAR_STS_PEER_PROPERTY                ,
    NEAR_STS_HID_REPORT                   ,
    NEAR_STS_HID_DESCRIPTOR               ,
    NEAR_STS_WORKING_STATE                ,
    NEAR_STS_REPORT_INFO                  ,
    NEAR_STS_DEVICE_ADDRESS               ,
    NEAR_STS_LOCAL_ADDRESS                ,
    NEAR_STS_PEER_ADDRESS                 ,
} near_status_t;

typedef enum {
    BT_GROUP_INDEX_LINK = 0,    // 0x1000
    BT_GROUP_INDEX_HFP_HF,      // 0x1100
    BT_GROUP_INDEX_HFP_AG,      // 0x1200
    BT_GROUP_INDEX_A2DP_SNK,    // 0x1300
    BT_GROUP_INDEX_A2DP_SRC,    // 0x1400
    BT_GROUP_INDEX_AVRCP,       // 0x1500
    BT_GROUP_INDEX_HID,         // 0x1600
    BT_GROUP_INDEX_MAP,         // 0x1700
    BT_GROUP_INDEX_PBAP,        // 0x1800
    BT_GROUP_INDEX_DIP,         // 0x1900
    BT_GROUP_INDEX_PAN,         // 0x1A00
    BT_GROUP_INDEX_OPP,         // 0x1B00
    BT_GROUP_INDEX_SPP,         // 0x1C00
    BT_GROUP_INDEX_HCI,         // 0x1D00
    BT_GROUP_INDEX_L2CAP,       // 0x1E00
    BT_GROUP_INDEX_GATT,        // 0x1F00
    BT_GROUP_INDEX_GAP,         // 0x2000
    BT_GROUP_INDEX_BAP,         // 0x2100
    BT_GROUP_INDEX_CS,          // 0x2200
} bt_event_group_index_t;

#define BT_EVENT_ID_START_BASE 0x1000
#define BT_EVENT_ID_GROUP_DIFF 0x0100
#define BT_EVENT_ID_GROUP_START(idx) (BT_EVENT_ID_START_BASE + (idx) * BT_EVENT_ID_GROUP_DIFF)
#define BT_EVENT_TO_GROUP_MASK(evid) (1 << (((evid&0xff00) - BT_EVENT_ID_START_BASE) / BT_EVENT_ID_GROUP_DIFF))

#define BT_EVENT_MASK_LINK_GROUP        (1<<BT_GROUP_INDEX_LINK)
#define BT_EVENT_MASK_HFP_HF_GROUP      (1<<BT_GROUP_INDEX_HFP_HF)
#define BT_EVENT_MASK_HFP_AG_GROUP      (1<<BT_GROUP_INDEX_HFP_AG)
#define BT_EVENT_MASK_A2DP_SNK_GROUP    (1<<BT_GROUP_INDEX_A2DP_SNK)
#define BT_EVENT_MASK_A2DP_SRC_GROUP    (1<<BT_GROUP_INDEX_A2DP_SRC)
#define BT_EVENT_MASK_AVRCP_GROUP       (1<<BT_GROUP_INDEX_AVRCP)
#define BT_EVENT_MASK_HID_GROUP         (1<<BT_GROUP_INDEX_HID)
#define BT_EVENT_MASK_MAP_GROUP         (1<<BT_GROUP_INDEX_MAP)
#define BT_EVENT_MASK_PBAP_GROUP        (1<<BT_GROUP_INDEX_PBAP)
#define BT_EVENT_MASK_DIP_GROUP         (1<<BT_GROUP_INDEX_DIP)
#define BT_EVENT_MASK_PAN_GROUP         (1<<BT_GROUP_INDEX_PAN)
#define BT_EVENT_MASK_OPP_GROUP         (1<<BT_GROUP_INDEX_OPP)
#define BT_EVENT_MASK_SPP_GROUP         (1<<BT_GROUP_INDEX_SPP)
#define BT_EVENT_MASK_HCI_GROUP         (1<<BT_GROUP_INDEX_HCI)
#define BT_EVENT_MASK_L2CAP_GROUP       (1<<BT_GROUP_INDEX_L2CAP)
#define BT_EVENT_MASK_GATT_GROUP        (1<<BT_GROUP_INDEX_GATT)
#define BT_EVENT_MASK_GAP_GROUP         (1<<BT_GROUP_INDEX_GAP)
#define BT_EVENT_MASK_BAP_GROUP         (1<<BT_GROUP_INDEX_BAP)
#define BT_EVENT_MASK_CS_GROUP          (1<<BT_GROUP_INDEX_CS)

typedef enum {
    BT_EVENT_INVALID_EVENT = 0,

    BT_EVENT_ACL_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_LINK), // 0x1000
    BT_EVENT_ACL_CLOSED,
    BT_EVENT_ACL_CONNECT_REQ,
    BT_EVENT_SCO_OPENED,
    BT_EVENT_SCO_CLOSED,
    BT_EVENT_SCO_CONNECT_REQ,
    BT_EVENT_SCO_CONNECT_FAIL,
    BT_EVENT_ACCESS_CHANGE,
    BT_EVENT_ROLE_DISCOVER,
    BT_EVENT_ROLE_CHANGE,
    BT_EVENT_MODE_CHANGE,
    BT_EVENT_AUTHENTICATED,
    BT_EVENT_ENC_CHANGE,
    BT_EVENT_INQUIRY_RESULT,
    BT_EVENT_INQUIRY_COMPLETE,
    BT_EVENT_BOND_STATE_CHANGE,
    BT_EVENT_PROPERTY_STATE,
    BT_EVENT_LINK_END,

    BT_EVENT_HF_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_HFP_HF), // 0x1100
    BT_EVENT_HF_CLOSED,
    BT_EVENT_HF_NETWORK_STATE,
    BT_EVENT_HF_NETWORK_ROAMING,
    BT_EVENT_HF_NETWORK_SIGNAL,
    BT_EVENT_HF_BATTERY_LEVEL,
    BT_EVENT_HF_CALL_IND,
    BT_EVENT_HF_CALLSETUP_IND,
    BT_EVENT_HF_CALLHELD_IND,
    BT_EVENT_HF_RING_IND,
    BT_EVENT_HF_CLIP_IND,
    BT_EVENT_HF_CALL_WAITING_IND,
    BT_EVENT_HF_RESP_AND_HOLD,
    BT_EVENT_HF_CURRENT_CALLS,
    BT_EVENT_HF_VOLUME_CHANGE,
    BT_EVENT_HF_AT_CMD_COMPLETE,
    BT_EVENT_HF_CURRENT_OPERATOR,
    BT_EVENT_HF_SUBSCRIBER_INFO,
    BT_EVENT_HF_VOICE_RECOGNITION_STATE,
    BT_EVENT_HF_IN_BAND_RING_STATE,
    BT_EVENT_HF_LAST_VOICE_TAG_NUMBER,
    BT_EVENT_HF_RECEIVE_UNKNOWN,
    BT_EVENT_HF_END,

    BT_EVENT_AG_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_HFP_AG), // 0x1200
    BT_EVENT_AG_CLOSED,
    BT_EVENT_AG_AUDIO_OPENED,
    BT_EVENT_AG_AUDIO_CLOSED,
    BT_EVENT_AG_VOLUME_CONTROL,
    BT_EVENT_AG_WBS_STATE,
    BT_EVENT_AG_BIA_IND,
    BT_EVENT_AG_BIEV_IND,
    BT_EVENT_AG_DIAL_CALL_REQ,
    BT_EVENT_AG_DIAL_MEMORY_REQ,
    BT_EVENT_AG_CALL_ACTION_REQ,
    BT_EVENT_AG_AT_DTMF_REQ,
    BT_EVENT_AG_AT_BVRA_REQ,
    BT_EVENT_AG_AT_NREC_REQ,
    BT_EVENT_AG_AT_CNUM_REQ,
    BT_EVENT_AG_AT_CIND_REQ,
    BT_EVENT_AG_AT_COPS_REQ,
    BT_EVENT_AG_AT_CLCC_REQ,
    BT_EVENT_AG_AT_CKPD_REQ,
    BT_EVENT_AG_AT_BIND_REQ,
    BT_EVENT_AG_UNKNOWN_AT_REQ,
    BT_EVENT_AG_END,

    BT_EVENT_A2DP_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_A2DP_SNK), // 0x1300
    BT_EVENT_A2DP_CLOSED,
    BT_EVENT_A2DP_STREAM_START,
    BT_EVENT_A2DP_STREAM_RECONFIG,
    BT_EVENT_A2DP_STREAM_SUSPEND,
    BT_EVENT_A2DP_STREAM_CLOSE,
    BT_EVENT_A2DP_STREAM_DATA_IND,
    BT_EVENT_A2DP_CUSTOM_CMD_REQ,
    BT_EVENT_A2DP_CUSTOM_CMD_RSP,
    BT_EVENT_A2DP_END,

    BT_EVENT_A2DP_SOURCE_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_A2DP_SRC), // 0x1400
    BT_EVENT_A2DP_SOURCE_CLOSED,
    BT_EVENT_A2DP_SOURCE_STREAM_START,
    BT_EVENT_A2DP_SOURCE_STREAM_RECONFIG,
    BT_EVENT_A2DP_SOURCE_STREAM_SUSPEND,
    BT_EVENT_A2DP_SOURCE_STREAM_CLOSE,
    BT_EVENT_A2DP_SOURCE_END,

    BT_EVENT_AVRCP_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_AVRCP), // 0x1500
    BT_EVENT_AVRCP_CLOSED,
    BT_EVENT_AVRCP_VOLUME_CHANGE,
    BT_EVENT_AVRCP_PLAY_STATUS_CHANGE,
    BT_EVENT_AVRCP_PLAY_POS_CHANGE,
    BT_EVENT_AVRCP_TRACK_CHANGE,
    BT_EVENT_AVRCP_BATTERY_CHANGE,
    BT_EVENT_AVRCP_RECV_PASSTHROUGH_CMD,
    BT_EVENT_AVRCP_RECV_PLAY_STATUS,
    BT_EVENT_AVRCP_RECV_MEDIA_STATUS,
    BT_EVENT_AVRCP_SEND_KEY_INFO,
    BT_EVENT_AVRCP_GET_COMPANY_ID,
    BT_EVENT_AVRCP_GET_CAPABILITY,
    BT_EVENT_AVRCP_NOTIFY_CHANGE,
    BT_EVENT_AVRCP_BROSING_OPENED,
    BT_EVENT_AVRCP_BROSING_CMD_INFO,
    BT_EVENT_AVRCP_BROSING_RSP_INFO,
    BT_EVENT_AVRCP_BROSING_CLOSED,
    BT_EVENT_AVRCP_OBEX_OPENED,
    BT_EVENT_AVRCP_OBEX_CLOSED,
    BT_EVENT_AVRCP_OBEX_GET_IMAGE_DONE,
    BT_EVENT_AVRCP_OBEX_GET_THM_DONE,
    BT_EVENT_AVRCP_OBEX_GET_IMAGE_PROPERTIES_DONE,
    BT_EVENT_AVRCP_OBEX_EVENT_INFO,
    BT_EVENT_AVRCP_END,

    BT_EVENT_HID_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_HID), // 0x1600
    BT_EVENT_HID_CLOSED,
    BT_EVENT_HID_SENSOR_STATE_CHANGED,
    BT_EVENT_HID_RCV_CTL_DATA,
    BT_EVENT_HID_RCV_INT_DATA,
    BT_EVENT_HID_SDP_REC_DATA,
    BT_EVENT_HID_TXDONE,
    BT_EVENT_HID_END,

    BT_EVENT_MAP_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_MAP), // 0x1700
    BT_EVENT_MAP_CLOSED,
    BT_EVENT_MAP_FOLDER_LISTING_RSP,
    BT_EVENT_MAP_MSG_LISTING_ITEM_RSP,
    BT_EVENT_MAP_GET_MSG_RSP,
    BT_EVENT_MAP_PUT_MSG_RSP,
    BT_EVENT_MAP_READ_STATUS_CHANGED,
    BT_EVENT_MAP_END,

    BT_EVENT_SPP_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_SPP), // 0x1C00
    BT_EVENT_SPP_CLOSED,
    BT_EVENT_SPP_END = BT_EVENT_SPP_OPENED + 0xf,

    BT_EVENT_L2CAP_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_L2CAP), // 0x1E00
    BT_EVENT_L2CAP_CLOSED,
    BT_EVENT_L2CAP_END = BT_EVENT_L2CAP_OPENED + 0xf,

    BT_EVENT_GATT_SERV_OPENED = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_GATT), // 0x1F00
    BT_EVENT_GATT_SERV_END = BT_EVENT_GATT_SERV_OPENED + 0xf,

    BT_EVENT_GATT_PROF_OPENED, // 0x1F10
    BT_EVENT_GATT_PROF_CLOSED,
    BT_EVENT_GATT_PROF_END = BT_EVENT_GATT_PROF_OPENED + 0xf,

    BT_EVENT_GAP_EVENT_START = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_GAP), // 0x2000
    BT_EVENT_GAP_EVENT_END = BT_EVENT_GAP_EVENT_START + 0xf,

    BT_EVENT_GAP_ADV_EVENT_START,       // 0x2010
    BT_EVENT_GAP_ADV_EVENT_END = BT_EVENT_GAP_ADV_EVENT_START + 0xf,

    BT_EVENT_GAP_SCAN_EVENT_START,      // 0x2020
    BT_EVENT_GAP_SCAN_EVENT_END = BT_EVENT_GAP_SCAN_EVENT_START + 0xf,

    BT_EVENT_GAP_CONN_EVENT_START,      // 0x2030
    BT_EVENT_GAP_CONN_EVENT_END = BT_EVENT_GAP_CONN_EVENT_START + 0xf,

    BT_EVENT_GAP_GLOBAL_EVENT_START,    // 0x2040
    BT_EVENT_GAP_GLOBAL_EVENT_END = BT_EVENT_GAP_GLOBAL_EVENT_START + 0xf,

    BT_EVENT_BAP_EVENT_START = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_BAP),
    BT_EVENT_BAP_EVENT_END = BT_EVENT_BAP_EVENT_START + 0xf,

    BT_EVENT_CS_EVENT_START = BT_EVENT_ID_GROUP_START(BT_GROUP_INDEX_CS),
    BT_EVENT_CS_EVENT_END   = BT_EVENT_CS_EVENT_START + 0x1f,
} BT_EVENT_T;

#define BT_SOCKET_MAX_TX_PEND_PACKETS (12) // bt controller max support 12 acl data buffer for speed

typedef struct bt_socket_t bt_socket_t;
typedef struct bt_service_port_t bt_service_port_t;
typedef struct bt_sdp_record_attr_t bt_sdp_record_attr_t;

typedef struct {
    uint8_t error_code;
    uint8_t device_id;
    bt_socket_t *socket;
} bt_socket_opened_t;

typedef struct {
    uint8_t error_code;
    uint8_t device_id;
    uint16_t remote_server_channel;
    bt_socket_t *socket;
    bt_service_port_t *port;
    void *sock_priv;
} bt_socket_closed_t;

typedef struct {
    bt_socket_t *socket;
    void *tx_priv;
} bt_socket_tx_done_t;

typedef struct {
    bt_socket_t *socket;
    const uint8_t *data;
    uint16_t len;
} bt_socket_rx_data_t;

typedef struct {
    uint8_t error_code;
    uint8_t device_id;
    uint16_t dlci;
    uint16_t local_server_channel;
    uint32_t channel_handle;
    void *set_sock_priv;
} bt_socket_accept_t;

typedef union {
    void *param_ptr;
    bt_socket_opened_t *opened;
    bt_socket_closed_t *closed;
    bt_socket_tx_done_t *tx_done;
    bt_socket_rx_data_t *rx_data;
    bt_socket_accept_t *accept;
} bt_socket_callback_param_t;

typedef enum {
    BT_SOCKET_EVENT_OPENED = BT_EVENT_SPP_OPENED,
    BT_SOCKET_EVENT_CLOSED,
    BT_SOCKET_EVENT_TX_DONE,
    BT_SOCKET_EVENT_RX_DATA,
    BT_SOCKET_EVENT_ACCEPT,
} bt_socket_event_t;

typedef enum {
    BT_SOCKET_STATE_IDLE = 0,
    BT_SOCKET_STATE_CLOSED,
    BT_SOCKET_STATE_CREATING_SESSION,
    BT_SOCKET_STATE_CONNECTING,
    BT_SOCKET_STATE_OPENED,
    BT_SOCKET_STATE_DISCONNECTING,
} bt_socket_state_t;

typedef int (*bt_socket_callback_t)(const bt_bdaddr_t *remote, bt_socket_event_t event, bt_socket_callback_param_t param);
typedef int (*bt_socket_accept_callback_t)(const bt_bdaddr_t *remote, bt_socket_event_t event, bt_socket_accept_t *accept);

typedef struct bt_service_port_t {
    uint16_t local_server_channel;
    bool server_accept_multi_device_request;
    bool upper_layer_give_credit;
    uint16_t initial_credits;
    uint16_t credit_give_step;
    uint16_t attr_count;
    const bt_sdp_record_attr_t *attr_list;
    bt_socket_callback_t socket_callback;
    bt_socket_accept_callback_t socket_accept_callback;
} bt_service_port_t;

typedef struct bt_socket_t {
    uint8_t device_id;
    bt_socket_state_t state;
    bt_bdaddr_t remote;
    uint32_t channel_handle;
    bool is_initiator;
    uint16_t dlci;
    uint16_t tx_mtu;
    uint16_t rx_credits;
    uint16_t tx_credits;
    uint16_t free_tx_packets; // number of packets currently can be delivered to socket with each packet size <= tx_mtu
    uint16_t remote_server_channel;
    bt_service_port_t *port;
    void *sock_priv;
} bt_socket_t;

bt_status_t bt_socket_create_port(uint16_t local_server_channel, bt_socket_callback_t socket_callback, const bt_sdp_record_attr_t *attr_list, uint16_t attr_count);
bt_status_t bt_socket_set_rx_credits(uint16_t local_server_channel, uint16_t rx_buff_size, bool upper_layer_give_credit, uint16_t initial_credits, uint16_t credit_give_step);
bt_status_t bt_socket_listen(uint16_t local_server_channel, bool support_multi_device, bt_socket_accept_callback_t accept_callback);
bt_status_t bt_socket_remove_listen(uint16_t local_server_channel);
bt_status_t bt_socket_connect(const bt_bdaddr_t *remote, uint16_t local_server_channel, const uint8_t *uuid, uint16_t uuid_len, void *sock_priv);
bt_status_t bt_socket_connect_server_channel(const bt_bdaddr_t *remote, uint16_t local_server_channel, uint16_t remote_server_channel, void *sock_priv);
bt_status_t bt_socket_give_credits(uint32_t channel_handle, uint16_t handled_credits);
bt_status_t bt_socket_disconnect(uint32_t channel_handle, uint8_t reason);
bt_status_t bt_socket_write(uint32_t channel_handle, const uint8_t *data, uint16_t size);
uint16_t bt_socket_count_free_tx_packets(uint32_t channel_handle);
bool bt_socket_is_connected(uint32_t channel_handle);

#include "hci_service.h"
#include "l2cap_service.h"
#include "sdp_service.h"
#include "a2dp_service.h"
#include "avrcp_service.h"
#include "hfp_service.h"
#include "spp_service.h"
#include "hid_service.h"
#include "map_service.h"

typedef enum {
    BT_PROPERTY_TYPE_LOCAL_BDNAME = 0x01,   // only set
    BT_PROPERTY_TYPE_LOCAL_BDADDR,          // only set
    BT_PROPERTY_TYPE_ACCESS_MODE,           // only set
    BT_PROPERTY_TYPE_REMOTE_NAME,           // only get
} bt_property_data_type_t;

typedef enum {
    BT_MODE_ACTIVE_MODE = 0x00,
    BT_MODE_HOLD_MODE   = 0x01,
    BT_MODE_SNIFF_MODE  = 0x02,
    BT_MODE_PARK_MODE   = 0x03,
} bt_link_mode_t;

typedef enum {
    BT_ROLE_MASTER  = 0x00,
    BT_ROLE_SLAVE   = 0x01,
} bt_link_role_t;

typedef enum {
    BT_ACCESS_NOT_ACCESSIBLE        = 0x00,
    BT_ACCESS_DISCOVERABLE_ONLY     = 0x01,
    BT_ACCESS_CONNECTABLE_ONLY      = 0x02,
    BT_ACCESS_GENERAL_ACCESSIBLE    = 0x03,
    BT_ACCESS_LIMITED_ACCESSIBLE    = 0x13,
} bt_access_mode_t;

typedef struct {
    uint8_t error_code;
    uint8_t acl_bt_role;
    uint16_t conn_handle;
    uint8_t device_id;
    uint8_t local_is_source;
} bt_adapter_acl_opened_param_t;

typedef struct {
    uint8_t error_code;
    uint8_t disc_reason;
    uint16_t conn_handle;
    uint8_t device_id;
} bt_adapter_acl_closed_param_t;

typedef struct {
    uint8_t error_code;
} bt_adapter_acl_connect_req_param_t;

typedef struct {
    uint8_t error_code;
    uint8_t codec;
    uint16_t sco_handle;
    uint8_t interval;
    uint8_t window;
} bt_adapter_sco_opened_param_t;

typedef struct {
    uint8_t error_code;
    uint8_t disc_reason;
    uint16_t sco_handle;
} bt_adapter_sco_closed_param_t;

typedef struct {
    uint8_t device_id;
    const uint8_t *remote;
} bt_adapter_sco_conn_req_param_t;

typedef struct {
    bt_access_mode_t access_mode;
} bt_adapter_access_change_param_t;

typedef struct {
    uint8_t error_code;
    bt_link_role_t acl_bt_role;
} bt_adapter_role_discover_param_t;

typedef struct {
    uint8_t error_code;
    bt_link_role_t acl_bt_role;
} bt_adapter_role_change_param_t;

typedef struct {
    uint8_t error_code;
    bt_link_mode_t acl_link_mode;
    uint16_t sniff_interval;
} bt_adapter_mode_change_param_t;

typedef struct {
    uint8_t error_code;
} bt_adapter_authenticated_param_t;

typedef struct {
    uint8_t error_code;
    uint8_t encrypted;
} bt_adapter_enc_change_param_t;

#define BT_RSSI_INVALID_VALUE 127

typedef struct {
    bt_bdaddr_t remote;
    uint8_t page_scan_repeat_mode;
    int8_t rssi;
    uint16_t clock_offset;
    uint32_t class_of_device;
    uint8_t *eir;
} bt_adapter_inquiry_result_param_t;

typedef struct {
    uint8_t error_code;
} bt_adapter_inquiry_complete_param_t;

typedef struct {
    bool is_bonded;
} bt_adapter_bond_state_change_param_t;

typedef struct {
    bt_property_data_type_t type;
    int len;
    uintptr_t data;
} bt_adapter_property_param_t;

typedef union {
    bt_adapter_acl_opened_param_t *acl_opened;
    bt_adapter_acl_closed_param_t *acl_closed;
    bt_adapter_acl_connect_req_param_t *acl_connect_req;
    bt_adapter_sco_opened_param_t *sco_opened;
    bt_adapter_sco_closed_param_t *sco_closed;
    bt_adapter_access_change_param_t *access_change;
    bt_adapter_role_discover_param_t *role_discover;
    bt_adapter_role_change_param_t *role_change;
    bt_adapter_mode_change_param_t *mode_change;
    bt_adapter_authenticated_param_t *authenticated;
    bt_adapter_enc_change_param_t *enc_change;
    bt_adapter_inquiry_result_param_t *inq_result;
    bt_adapter_inquiry_complete_param_t *inq_complete;
    bt_adapter_bond_state_change_param_t *bond_change;
    bt_adapter_property_param_t *perperty_state;
} bt_adapter_callback_param_t;

typedef enum {
    BT_ADAPTER_EVENT_ACL_OPENED = BT_EVENT_ACL_OPENED,
    BT_ADAPTER_EVENT_ACL_CLOSED,
    BT_ADAPTER_EVENT_ACL_CONNECT_REQ,
    BT_ADAPTER_EVENT_SCO_OPENED,
    BT_ADAPTER_EVENT_SCO_CLOSED,
    BT_ADAPTER_EVENT_ACCESS_CHANGE,
    BT_ADAPTER_EVENT_ROLE_DISCOVER,
    BT_ADAPTER_EVENT_ROLE_CHANGE,
    BT_ADAPTER_EVENT_MODE_CHANGE,
    BT_ADAPTER_EVENT_AUTHENTICATED,
    BT_ADAPTER_EVENT_ENC_CHANGE,
    BT_ADAPTER_EVENT_INQUIRY_RESULT,
    BT_ADAPTER_EVENT_INQUIRY_COMPLETE,
    BT_ADAPTER_EVENT_BOND_STATE_CHANGE,
    BT_ADAPTER_EVENT_END,
} bt_adapter_event_t;

#if BT_ADAPTER_EVENT_END != BT_EVENT_LINK_END
#error "bt_adapter_event_t error define"
#endif

typedef int (*bt_adapter_callback_t)(const bt_bdaddr_t *bd_addr, bt_adapter_event_t event, bt_adapter_callback_param_t param);

bt_status_t bt_adapter_init(bt_adapter_callback_t callback);
bt_status_t bt_adapter_cleanup(void);
bt_status_t bt_adapter_start_inquiry(void);
bt_status_t bt_adapter_cancel_inquiry(void);
bt_status_t bt_adapter_connect_acl(const bt_bdaddr_t *bd_addr);
bt_status_t bt_adapter_disconnect_acl(const bt_bdaddr_t *bd_addr);
bt_status_t bt_adapter_create_bond(const bt_bdaddr_t *bd_addr);
bt_status_t bt_adapter_remove_bond(const bt_bdaddr_t *bd_addr);
bt_status_t bt_adapter_get_property(const bt_bdaddr_t *bd_addr, bt_property_data_type_t type);
bt_status_t bt_adapter_set_property(const bt_adapter_property_param_t *property);
bt_status_t bt_adapter_write_sleep_enable(bool enable);

typedef union {
    void *param_ptr;
    bt_adapter_callback_param_t bt;
#ifdef BT_HFP_SUPPORT
    bt_hf_callback_param_t hf;
#ifdef BT_HFP_AG_ROLE
    bt_ag_callback_param_t ag;
#endif
#endif
#ifdef BT_A2DP_SUPPORT
    bt_a2dp_callback_param_t av;
#endif
#ifdef BT_AVRCP_SUPPORT
    bt_avrcp_callback_param_t ar;
#endif
#ifdef BT_HID_DEVICE
    bt_hid_callback_param_t hid;
#endif
#ifdef BT_MAP_SUPPORT
    bt_map_callback_param_t map;
#endif
} BT_CALLBACK_PARAM_T;

typedef int (*bt_event_callback_t)(const bt_bdaddr_t *bd_addr, BT_EVENT_T event, BT_CALLBACK_PARAM_T param);

typedef struct _hfp_adapter_context
{
    bool hfp_is_connected;
    uint8_t hfp_call_state;
    uint8_t hfp_callsetup_state;
    uint8_t hfp_callhold_state;
    uint8_t hfp_speak_vol;
} HFP_ADAPTER_CONTEXT;

typedef struct _a2dp_adapter_context
{
    bool a2dp_is_connected;
    bool a2dp_is_streaming;
    uint8_t a2dp_codec_type;
    uint8_t a2dp_sample_rate;
    bool rsv_avdtp_start_signal;
} A2DP_ADAPTER_CONTEXT;

typedef struct _avrcp_adapter_context
{
    bool avrcp_is_connected;
    uint8_t avrcp_playback_status;
} AVRCP_ADAPTER_CONTEXT;

struct BT_ADAPTER_DEVICE_T {
    bt_bdaddr_t remote;
    uint8_t device_id;
    bool acl_is_connected;
    uint8_t acl_bt_role;
    uint8_t acl_link_mode;
    uint16_t acl_conn_hdl;
    uint16_t sniff_interval;

    bool sco_is_connected;
    uint8_t sco_codec_type;
    uint16_t sco_handle;

    uint8_t is_use_local_sbm;
    uint16_t media_active;

    HFP_ADAPTER_CONTEXT hfp_ctx;
    A2DP_ADAPTER_CONTEXT a2dp_ctx;
    AVRCP_ADAPTER_CONTEXT avrcp_ctx;
};

#define BT_ADAPTER_MAX_DEVICE_NUM (BT_DEVICE_NUM+BT_SOURCE_DEVICE_NUM)

struct BT_ADAPTER_MANAGER_T {
    struct BT_ADAPTER_DEVICE_T bt_sink_device[BT_ADAPTER_MAX_DEVICE_NUM];
    struct BT_ADAPTER_DEVICE_T bt_source_device[BT_ADAPTER_MAX_DEVICE_NUM];
    struct BT_ADAPTER_DEVICE_T bt_tws_device;
#ifdef RTOS
    osMutexId adapter_lock;
#endif
    uint8_t access_mode;
    bt_iocap_requirement_t iocap_requirement;
};

void bt_add_event_callback(bt_event_callback_t cb, uint32_t masks);

typedef struct bes_aud_bt_t {
    void (*aud_set_stay_active_mode)(bool keep_active);
    uint8_t (*aud_get_curr_a2dp_device)(void); // app_bt_audio_get_curr_a2dp_device
    uint8_t (*aud_get_curr_sco_device)(void); // app_bt_audio_get_curr_sco_device
    bool (*aud_is_sco_prompt_play_mode)(void); // app_bt_manager.config.sco_prompt_play_mode
    uint8_t (*aud_get_max_sco_number)(void); // btif_sco_get_max_number
    uint8_t (*aud_get_a2dp_codec_type)(int device_id); // bt_sbc_player_get_codec_type
    int (*aud_switch_sco)(uint16_t sco_handle); // app_bt_Me_switch_sco
    void (*aud_report_hfp_speak_gain)(void); // btapp_hfp_report_speak_gain
    void (*aud_report_a2dp_speak_gain)(void); // btapp_a2dp_report_speak_gain
    bool (*aud_hfp_mic_need_skip_frame)(void); // btapp_hfp_mic_need_skip_frame
    uint8_t (*aud_hfp_need_mute)(void); // btapp_hfp_need_mute
    void (*aud_hfp_set_local_vol)(int id, uint8_t vol); // hfp_volume_local_set
    uint8_t (*aud_adjust_hfp_volume)(uint8_t device_id, bool up, bool adjust_local_vol_level); // app_bt_hfp_adjust_volume
    uint8_t (*aud_adjust_a2dp_volume)(uint8_t device_id, bool up, bool adjust_local_vol_level); // app_bt_a2dp_adjust_volume
    bool (*ignore_ring_and_play_tone_self)(int device_id);
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
    bool (*a2dp_lhdc_get_ext_flags)(uint32_t flags); // a2dp_lhdc_get_ext_flags
    uint8_t (*a2dp_lhdc_config_llc_get)(void); // a2dp_lhdc_config_llc_get
#endif
    uint8_t (*a2dp_get_non_type_by_device_id)(uint8_t device_id);
} bes_aud_bt_t;

typedef struct {
    void (*key_click_func)(void);
    void (*key_doubleclick_func)(void);
    void (*key_tripleclick_func)(void);
    void (*key_longpress_func)(void);
}bta_key_click_event_cb_t;

extern const bes_aud_bt_t * const bes_aud_bt;

struct BT_ADAPTER_DEVICE_T *bt_adapter_get_device(int device_id);
struct BT_ADAPTER_DEVICE_T *bt_adapter_get_connected_device_by_id(int device_id);
struct BT_ADAPTER_DEVICE_T *bt_adapter_get_connected_device_by_connhdl(uint16_t connhdl);
struct BT_ADAPTER_DEVICE_T *bt_adapter_get_connected_device_byaddr(const bt_bdaddr_t *remote);
uint8_t bt_adapter_reset_device_id(struct BT_ADAPTER_DEVICE_T *curr_device, bool reset_to_source);
uint8_t bt_adapter_get_hfp_sco_codec_type(int device_id);
uint8_t bt_adapter_count_mobile_link(void);
uint8_t bt_adapter_count_connected_hfp(void);
uint8_t bt_adapter_count_streaming_a2dp(void);
uint8_t bt_adapter_count_streaming_sco(void);
uint8_t bt_adapter_has_incoming_call(void);
int bt_adapter_get_device_id_by_connhdl(uint16_t connhdl);
int bt_adapter_get_device_id_byaddr(const bt_bdaddr_t *remote);
void bt_adapter_mutex_lock(void);
void bt_adapter_mutex_unlock(void);
void bt_adapter_manager_init(void);
void bt_adapter_callback_func_init(void);
void bt_adapter_local_volume_down(void);
void bt_adapter_local_volume_up(void);
void bt_adapter_local_volume_down_with_callback(void (*cb)(uint8_t device_id));
void bt_adapter_local_volume_up_with_callback(void (*cb)(uint8_t device_id));
void bt_adapter_set_a2dp_codec_info(int device_id, uint8_t codec_type, uint8_t sample_rate, uint8_t sample_bit);
void bt_adapter_set_hfp_sco_codec_type(int device_id, uint8_t sco_codec);

void bt_adapter_report_acl_connected(const bt_bdaddr_t *bd_addr, const bt_adapter_acl_opened_param_t *acl_con);
void bt_adapter_report_acl_disconnected(const bt_bdaddr_t *bd_addr, const bt_adapter_acl_closed_param_t *acl_dis);
void bt_adapter_report_sco_connected(const bt_bdaddr_t *bd_addr, const bt_adapter_sco_opened_param_t *sco_con);
void bt_adapter_report_sco_disconnected(const bt_bdaddr_t *bd_addr, const bt_adapter_sco_closed_param_t *sco_dis);
void bt_adapter_report_access_change(const bt_bdaddr_t *bd_addr, const bt_adapter_access_change_param_t *access_change);
void bt_adapter_report_role_discover(const bt_bdaddr_t *bd_addr, const bt_adapter_role_discover_param_t *role_discover);
void bt_adapter_report_role_change(const bt_bdaddr_t *bd_addr, const bt_adapter_role_change_param_t *role_change);
void bt_adapter_report_mode_change(const bt_bdaddr_t *bd_addr, const bt_adapter_mode_change_param_t *mode_change);
void bt_adapter_report_authenticated(const bt_bdaddr_t *bd_addr, const bt_adapter_authenticated_param_t *auth);
void bt_adapter_report_enc_change(const bt_bdaddr_t *bd_addr, const bt_adapter_enc_change_param_t *enc_change);
void bt_adapter_report_inquiry_result(const bt_bdaddr_t *bd_addr, const bt_adapter_inquiry_result_param_t *inq_result);
void bt_adapter_report_inquiry_complete(const bt_bdaddr_t *bd_addr, const bt_adapter_inquiry_complete_param_t *inq_complete);
void bt_adapter_reconfig_hci_trace(uint8_t cfg_type, bool enable);
void bt_adapter_echo_register(void(*echo_req)(uint8_t device_id, uint16_t conhdl, uint8_t id, uint16_t len, uint8_t *data),
                                     void(*echo_rsp)(uint8_t device_id, uint16_t conhdl, uint8_t *rxdata, uint16_t rxle));
void bt_adapter_echo_req_send(uint8_t device_id, void *conn, uint8_t *data, uint16_t data_len);
void bt_adapter_echo_rsp_send(uint8_t device_id, uint16 conn_handle, uint8 sigid, uint16 len, const uint8* data);
void bt_adapter_export_esco_param_cb(void(*cb)(void *bdaddr, uint8_t tesco, uint8_t wesco));
#ifdef BT_HFP_SUPPORT
void bt_adapter_report_hfp_connected(const bt_bdaddr_t *bd_addr, const bt_hf_opened_param_t *hfp_conn);
void bt_adapter_report_hfp_disconnected(const bt_bdaddr_t *bd_addr, const bt_hf_closed_param_t *hfp_disc);
void bt_adapter_report_hfp_ring(const bt_bdaddr_t *bd_addr, const void *param);
void bt_adapter_report_hfp_clip_ind(const bt_bdaddr_t *bd_addr, const bt_hf_clip_ind_param_t *hfp_caller_ind);
void bt_adapter_report_hfp_call_state(const bt_bdaddr_t *bd_addr, const bt_hf_call_ind_param_t *hfp_call);
void bt_adapter_report_hfp_callsetup_state(const bt_bdaddr_t *bd_addr, const bt_hf_callsetup_ind_param_t *hfp_callsetup);
void bt_adapter_report_hfp_callhold_state(const bt_bdaddr_t *bd_addr, const bt_hf_callheld_ind_param_t *hfp_callhold);
void bt_adapter_report_hfp_volume_change(const bt_bdaddr_t *bd_addr, const bt_hf_volume_change_param_t *volume_change);
#endif /* BT_HFP_SUPPORT */
#ifdef BT_A2DP_SUPPORT
void bt_adapter_report_a2dp_connected(const bt_bdaddr_t *bd_addr, const bt_a2dp_opened_param_t *a2dp_conn);
void bt_adapter_report_a2dp_disconnected(const bt_bdaddr_t *bd_addr, const bt_a2dp_closed_param_t *a2dp_disc);
void bt_adapter_report_a2dp_stream_start(const bt_bdaddr_t *bd_addr, const bt_a2dp_stream_start_param_t *a2dp_stream_start);
void bt_adapter_report_a2dp_stream_reconfig(const bt_bdaddr_t *bd_addr, const bt_a2dp_stream_reconfig_param_t *a2dp_stream_reconfig);
void bt_adapter_report_a2dp_stream_suspend(const bt_bdaddr_t *bd_addr, const bt_a2dp_stream_suspend_param_t *a2dp_stream_suspend);
void bt_adapter_report_a2dp_stream_close(const bt_bdaddr_t *bd_addr, const bt_a2dp_stream_close_param_t *a2dp_stream_close);
#endif /* BT_A2DP_SUPPORT */
#ifdef BT_AVRCP_SUPPORT
void bt_adapter_report_avrcp_connected(const bt_bdaddr_t *bd_addr, const bt_avrcp_opened_t *avrcp_conn);
void bt_adapter_report_avrcp_disconnected(const bt_bdaddr_t *bd_addr, const bt_avrcp_closed_t *avrcp_disc);
void bt_adapter_report_avrcp_status_changed(const bt_bdaddr_t *bd_addr, const bt_avrcp_play_status_change_t *avrcp_changed);
#endif /* BT_AVRCP_SUPPORT */
void bes_bt_a2dp_get_device_codec_info(uint8_t dev_num, uint8_t *codec);

void app_bt_get_local_device_address(void *bd_addr);

void bt_adapter_register_hci_log_report_callback(uint16_t max_len,
                int (*tx_cb)(const uint8_t *buf, uint16_t len),
                int (*rx_cb)(const uint8_t *buf, uint16_t len));

void bt_adapter_register_key_evt_handle_cb(const bta_key_click_event_cb_t *cbs);
#ifdef __cplusplus
}
#endif
#endif /* __BT_ADAPTER_LAYER_H__ */
