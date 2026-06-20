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
#ifndef __DEBUG_CFG_H__
#define __DEBUG_CFG_H__

#define NULL_MODULE         0
#define HCI_MODULE          1
#define L2CAP_MODULE        2
#define GAP_MODULE          3
#define GATT_MODULE         4
#define SDP_MODULE          5
#define SMP_MODULE          6
#define BTM_MODULE          7
#define A2DP_MODULE         8
#define AVRCP_MODULE        9
#define BNEP_MODULE         10
#define HFP_MODULE          11
#define HID_MODULE          12
#define MAP_MODULE          13
#define OBEX_MODULE         14
#define OPP_MODULE          15
#define PAN_MODULE          16
#define PBAP_MODULE         17
#define RFCOMM_MODULE       18
#define SPP_MODULE          19
#define BTIF_MODULE         20
#define DIP_MODULE          21
#define BESAUD_MODULE       22
#define BTGATT_MODULE       23
#define A2DP_API_MODULE     24
#define AVRCP_API_MODULE    25
#define HFP_API_MODULE      26
#define HID_API_MODULE      27
#define MAP_API_MODULE      28
#define ME_API_MODULE       29
#define OPP_API_MODULE      30
#define PAN_API_MODULE      31
#define PBAP_API_MODULE     32
#define BAP_MODULE          33
#define ISO_MODULE          34
#define APP_BLE_MODULE      35
#define TEST_MODULE         36
#define COBT_MODULE         37
#define APP_BT_MODULE       38
#define A2DP_APP_MODULE     39
#define HFP_APP_MODULE      40
#define AVRCP_APP_MODULE    41
#define SSAP_MODULE         42
#define DTCP_MODULE         43
#define NHCI_MODULE         44
#define NAPP_MODULE         45
#define NDIP_MODULE         46
#define NBTT_MODULE         47
#define NSPP_MODULE         48
#define HIOD_MODULE         49
#define HIOH_MODULE         50
#define CSET_MODULE         51
#define NMCP_MODULE         52
#define NASC_MODULE         53
#define NTST_MODULE         54
#define CS_MODULE           55
#define LOG_MODULE_MAX      56

#define BT_DBG_EX_CONCAT(a, b) a##b
#define BT_DBG_CONCAT(a, b) BT_DBG_EX_CONCAT(a, b)
#define BT_DBG_EX_NAME(a) #a
#define BT_DBG_GET_NAME(a) BT_DBG_EX_NAME(a)

#define DEBUG_PRINT TRACE

#define DBG_ASSERT_LEVEL    0
#define DBG_ERROR_LEVEL     1
#define DBG_WARNING_LEVEL   2
#define DBG_INFO_LEVEL      3
#define DBG_VERBOSE_LEVEL   4
#define DBG_MAIN_FLOW_LEVEL 5

#if defined(BT_STACK_LOG_DISABLE) || defined(BT_LOG_SIMPLIFY)
#define HCI_LEVEL           DBG_ERROR_LEVEL
#define L2CAP_LEVEL         DBG_ERROR_LEVEL
#define SDP_LEVEL           DBG_ERROR_LEVEL
#define GAP_LEVEL           DBG_ERROR_LEVEL
#define SMP_LEVEL           DBG_ERROR_LEVEL
#define GATT_LEVEL          DBG_ERROR_LEVEL
#define BTM_LEVEL           DBG_ERROR_LEVEL
#define A2DP_LEVEL          DBG_ERROR_LEVEL
#define AVRCP_LEVEL         DBG_ERROR_LEVEL
#define BNEP_LEVEL          DBG_ERROR_LEVEL
#define HFP_LEVEL           DBG_ERROR_LEVEL
#define HID_LEVEL           DBG_ERROR_LEVEL
#define MAP_LEVEL           DBG_ERROR_LEVEL
#define OBEX_LEVEL          DBG_ERROR_LEVEL
#define OPP_LEVEL           DBG_ERROR_LEVEL
#define PAN_LEVEL           DBG_ERROR_LEVEL
#define PBAP_LEVEL          DBG_ERROR_LEVEL
#define RFCOMM_LEVEL        DBG_ERROR_LEVEL
#define SPP_LEVEL           DBG_ERROR_LEVEL
#define BTIF_LEVEL          DBG_ERROR_LEVEL
#define DIP_LEVEL           DBG_ERROR_LEVEL
#define BESAUD_LEVEL        DBG_ERROR_LEVEL
#define BTGATT_LEVEL        DBG_ERROR_LEVEL
#define A2DP_API_LEVEL      DBG_ERROR_LEVEL
#define AVRCP_API_LEVEL     DBG_ERROR_LEVEL
#define HFP_API_LEVEL       DBG_ERROR_LEVEL
#define HID_API_LEVEL       DBG_ERROR_LEVEL
#define MAP_API_LEVEL       DBG_ERROR_LEVEL
#define ME_API_LEVEL        DBG_ERROR_LEVEL
#define OPP_API_LEVEL       DBG_ERROR_LEVEL
#define PAN_API_LEVEL       DBG_ERROR_LEVEL
#define PBAP_API_LEVEL      DBG_ERROR_LEVEL
#define BAP_LEVEL           DBG_ERROR_LEVEL
#define ISO_LEVEL           DBG_ERROR_LEVEL
#define APP_BLE_LEVEL       DBG_ERROR_LEVEL
#define APP_BT_LEVEL        DBG_ERROR_LEVEL
#define A2DP_APP_LEVEL      DBG_ERROR_LEVEL
#define HFP_APP_LEVEL       DBG_ERROR_LEVEL
#define CS_LEVEL            DBG_ERROR_LEVEL
#else
#define HCI_LEVEL           DBG_INFO_LEVEL
#define L2CAP_LEVEL         DBG_INFO_LEVEL
#define SDP_LEVEL           DBG_INFO_LEVEL
#define GAP_LEVEL           DBG_VERBOSE_LEVEL
#define SMP_LEVEL           DBG_VERBOSE_LEVEL
#define GATT_LEVEL          DBG_INFO_LEVEL
#define BTM_LEVEL           DBG_INFO_LEVEL
#define A2DP_LEVEL          DBG_INFO_LEVEL
#define AVRCP_LEVEL         DBG_INFO_LEVEL
#define BNEP_LEVEL          DBG_ERROR_LEVEL
#define HFP_LEVEL           DBG_INFO_LEVEL
#define HID_LEVEL           DBG_ERROR_LEVEL
#define MAP_LEVEL           DBG_ERROR_LEVEL
#define OBEX_LEVEL          DBG_ERROR_LEVEL
#define OPP_LEVEL           DBG_ERROR_LEVEL
#define PAN_LEVEL           DBG_ERROR_LEVEL
#define PBAP_LEVEL          DBG_ERROR_LEVEL
#define RFCOMM_LEVEL        DBG_INFO_LEVEL
#define SPP_LEVEL           DBG_INFO_LEVEL
#define BTIF_LEVEL          DBG_ERROR_LEVEL
#define DIP_LEVEL           DBG_ERROR_LEVEL
#define BESAUD_LEVEL        DBG_ERROR_LEVEL
#define BTGATT_LEVEL        DBG_ERROR_LEVEL
#define A2DP_API_LEVEL      DBG_INFO_LEVEL
#define AVRCP_API_LEVEL     DBG_INFO_LEVEL
#define HFP_API_LEVEL       DBG_INFO_LEVEL
#define HID_API_LEVEL       DBG_ERROR_LEVEL
#define MAP_API_LEVEL       DBG_ERROR_LEVEL
#define ME_API_LEVEL        DBG_ERROR_LEVEL
#define OPP_API_LEVEL       DBG_ERROR_LEVEL
#define PAN_API_LEVEL       DBG_ERROR_LEVEL
#define PBAP_API_LEVEL      DBG_ERROR_LEVEL
#define BAP_LEVEL           DBG_VERBOSE_LEVEL
#define ISO_LEVEL           DBG_VERBOSE_LEVEL
#define APP_BLE_LEVEL       DBG_VERBOSE_LEVEL
#define APP_BT_LEVEL        DBG_INFO_LEVEL
#define A2DP_APP_LEVEL      DBG_INFO_LEVEL
#define HFP_APP_LEVEL       DBG_INFO_LEVEL
#define AVRCP_APP_LEVEL     DBG_INFO_LEVEL
#define CS_LEVEL            DBG_VERBOSE_LEVEL
#endif

#define SSAP_LEVEL          DBG_VERBOSE_LEVEL
#define DTCP_LEVEL          DBG_VERBOSE_LEVEL
#define NHCI_LEVEL          DBG_VERBOSE_LEVEL
#define NAPP_LEVEL          DBG_VERBOSE_LEVEL
#define NDIP_LEVEL          DBG_VERBOSE_LEVEL
#define NBTT_LEVEL          DBG_VERBOSE_LEVEL
#define NSPP_LEVEL          DBG_VERBOSE_LEVEL
#define HIOD_LEVEL          DBG_VERBOSE_LEVEL
#define HIOH_LEVEL          DBG_VERBOSE_LEVEL
#define CSET_LEVEL          DBG_VERBOSE_LEVEL
#define NMCP_LEVEL          DBG_VERBOSE_LEVEL
#define NASC_LEVEL          DBG_VERBOSE_LEVEL
#define NTST_LEVEL          DBG_VERBOSE_LEVEL

#define TEST_LEVEL          DBG_VERBOSE_LEVEL
#define COBT_LEVEL          DBG_ERROR_LEVEL

#endif /* __DEBUG_CFG_H__ */
