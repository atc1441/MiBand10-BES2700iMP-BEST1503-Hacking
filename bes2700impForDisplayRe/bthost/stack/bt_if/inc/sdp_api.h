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
#ifndef	__SDP_API_H__
#define	__SDP_API_H__
#include "me_api.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t sdp_query_type;
#define BTIF_BSQT_ATTRIB_REQ                 0x04
#define BTIF_BSQT_SERVICE_SEARCH_ATTRIB_REQ  0x06
typedef uint8_t sdp_query_resp;
#define BTIF_BSQR_ERROR_RESP                 0x01
#define BTIF_BSQR_SERVICE_SEARCH_RESP        0x03
#define BTIF_BSQR_ATTRIB_RESP                0x05
#define BTIF_BSQR_SERVICE_SEARCH_ATTRIB_RESP 0x07

typedef uint16_t sdp_service_class_uuid;
#define BTIF_SC_SERVICE_DISCOVERY_SERVER             0x1000
#define BTIF_SC_BROWSE_GROUP_DESC                    0x1001
#define BTIF_SC_PUBLIC_BROWSE_GROUP                  0x1002
#define BTIF_SC_SERIAL_PORT                          0x1101
#define BTIF_SC_LAN_ACCESS_PPP                       0x1102
#define BTIF_SC_DIALUP_NETWORKING                    0x1103
#define BTIF_SC_IRMC_SYNC                            0x1104
#define BTIF_SC_OBEX_OBJECT_PUSH                     0x1105
#define BTIF_SC_OBEX_FILE_TRANSFER                   0x1106
#define BTIF_SC_IRMC_SYNC_COMMAND                    0x1107
#define BTIF_SC_HEADSET                              0x1108
#define BTIF_SC_CORDLESS_TELEPHONY                   0x1109
#define BTIF_SC_AUDIO_SOURCE                         0x110A
#define BTIF_SC_AUDIO_SINK                           0x110B
#define BTIF_SC_AV_REMOTE_CONTROL_TARGET             0x110C
#define BTIF_SC_AUDIO_DISTRIBUTION                   0x110D
#define BTIF_SC_AV_REMOTE_CONTROL                    0x110E
#define BTIF_SC_VIDEO_CONFERENCING                   0x110F
#define BTIF_SC_INTERCOM                             0x1110
#define BTIF_SC_FAX                                  0x1111
#define BTIF_SC_HEADSET_AUDIO_GATEWAY                0x1112
#define BTIF_SC_WAP                                  0x1113
#define BTIF_SC_WAP_CLIENT                           0x1114
#define BTIF_SC_PANU                                 0x1115
#define BTIF_SC_NAP                                  0x1116
#define BTIF_SC_GN                                   0x1117
#define BTIF_SC_DIRECT_PRINTING                      0x1118
#define BTIF_SC_REFERENCE_PRINTING                   0x1119
#define BTIF_SC_IMAGING                              0x111A
#define BTIF_SC_IMAGING_RESPONDER                    0x111B
#define BTIF_SC_IMAGING_AUTOMATIC_ARCHIVE            0x111C
#define BTIF_SC_IMAGING_REFERENCED_OBJECTS           0x111D
#define BTIF_SC_HANDSFREE                            0x111E
#define BTIF_SC_HANDSFREE_AUDIO_GATEWAY              0x111F
#define BTIF_SC_DIRECT_PRINTING_REF_OBJECTS          0x1120
#define BTIF_SC_REFLECTED_UI                         0x1121
#define BTIF_SC_BASIC_PRINTING                       0x1122
#define BTIF_SC_PRINTING_STATUS                      0x1123
#define BTIF_SC_HUMAN_INTERFACE_DEVICE               0x1124
#define BTIF_SC_HCR                                  0x1125
#define BTIF_SC_HCR_PRINT                            0x1126
#define BTIF_SC_HCR_SCAN                             0x1127
#define BTIF_SC_ISDN                                 0x1128
#define BTIF_SC_VIDEO_CONFERENCING_GW                0x1129
#define BTIF_SC_UDI_MT                               0x112A
#define BTIF_SC_UDI_TA                               0x112B
#define BTIF_SC_AUDIO_VIDEO                          0x112C
#define BTIF_SC_SIM_ACCESS                           0x112D
#define BTIF_SC_PBAP_CLIENT                          0x112E
#define BTIF_SC_PBAP_SERVER                          0x112F
#define BTIF_SC_PBAP_PROFILE                         0x1130
#define BTIF_SC_MAP_SERVER                           0x1132
#define BTIF_SC_MAP_NOTIFY_SERVER                    0x1133
#define BTIF_SC_MAP_PROFILE                          0x1134
#define BTIF_SC_PNP_INFO                             0x1200
#define BTIF_SC_GENERIC_NETWORKING                   0x1201
#define BTIF_SC_GENERIC_FILE_TRANSFER                0x1202
#define BTIF_SC_GENERIC_AUDIO                        0x1203
#define BTIF_SC_GENERIC_TELEPHONY                    0x1204
#define BTIF_SC_UPNP_SERVICE                         0x1205
#define BTIF_SC_UPNP_IP_SERVICE                      0x1206
#define BTIF_SC_ESDP_UPNP_IP_PAN                     0x1300
#define BTIF_SC_ESDP_UPNP_IP_LAP                     0x1301
#define BTIF_SC_ESDP_UPNP_L2CAP                      0x1302
#define BTIF_SC_VIDEO_SOURCE                         0x1303
#define BTIF_SC_VIDEO_SINK                           0x1304
#define BTIF_SC_VIDEO_DISTRIBUTION                   0x1305

typedef uint8_t sdp_parsing_mode;
#define BTIF_BSPM_BEGINNING   0x00
#define BTIF_BSPM_RESUME      0x01
#define BTIF_BSPM_CONT_STATE  0x02
#define BTIF_BSPM_NO_SKIP   0x04

typedef uint8_t sdp_query_mode;
#define BTIF_BSQM_FIRST     0x00
#define BTIF_BSQM_CONTINUE  0x01
#define BTIF_BSQM_DONT_CARE 0xFF

U16 sdp_get_u16(U8 * buff);
#define sdp_get_u16(buff) be_to_host16((buff))
U32 sdp_get_u32(U8 * buff);
#define sdp_get_u32(buff) BEtoHost32((buff))
void sdp_put_u16(U8 * buff, U16 val);
#define sdp_put_u16(buff,val) StoreBE16((buff),(val))
void sdp_put_u32(U8 * buff, U32 val);
#define sdp_put_u32(buff,val) StoreBE32((buff),(val))

bt_status_t btif_sdp_init(void);
bt_status_t btif_sdp_uuid_search(const bt_bdaddr_t *bd_addr, const bt_sdp_uuid_search_t *param);
bt_status_t btif_sdp_queue_uuid_search(const bt_bdaddr_t *bd_addr, bt_sdp_service_uuid_t uuid);
bt_status_t btif_sdp_service_search(const bt_bdaddr_t *bd_addr, const bt_sdp_service_search_t *param);
bt_status_t btif_sdp_create_record(const bt_sdp_record_attr_t *attr_list, int attr_count);
bt_status_t btif_sdp_remove_record(const bt_sdp_record_attr_t *attr_list);

bool btif_sdp_is_record_registered(const bt_sdp_record_attr_t* attr_list);
void btif_sdp_register_ibrt_tws_switch_protect_handle(void (*cb)(uint8_t, bt_bdaddr_t *, bool));

#ifdef __cplusplus
}
#endif
#endif /*__SDP_API_H__*/
