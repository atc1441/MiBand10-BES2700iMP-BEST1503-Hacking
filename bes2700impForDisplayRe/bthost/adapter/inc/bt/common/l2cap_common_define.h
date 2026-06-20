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
#ifndef __L2CAP_COMMON_DEFINE_H__
#define __L2CAP_COMMON_DEFINE_H__
#include "bt_common_define.h"
#ifdef __cplusplus
extern "C" {
#endif

#define L2CAP_BESAUD_EXTRA_CHAN_ID 0xbbee

enum l2cap_event_enum {
    L2CAP_CHANNEL_CONN_REQ,
    L2CAP_CHANNEL_OPENED,
    L2CAP_CHANNEL_NEW_OPENED,
    L2CAP_CHANNEL_TX_HANDLED,
    L2CAP_CHANNEL_CLOSED,
};

enum PSM_CONTEXT_TYPE
{
     PSM_CONTEXT_SDP     = 0x01,  //SDP
     PSM_CONTEXT_RFC     = 0x02,  //RFCOMM MUX,HFP and SPP share
     PSM_CONTEXT_AVDTP   = 0x04,  //A2DP
     PSM_CONTEXT_AVCTP   = 0x08,  //AVRCP
     PSM_CONTEXT_BTGATT  = 0x10,  //GATT OVER BREDR

     PSM_CONTEXT_INVALID = 0x80,
};

#define L2CAP_PPB_HEAD_RESERVE      4       /*len+cid+control+sdulen  2+2+2+2*/
#define L2CAP_PPB_TAIL_RESERVE      0        /* fcs 2 */
#define L2CAP_PPB_RESERVE  (L2CAP_PPB_HEAD_RESERVE + L2CAP_PPB_TAIL_RESERVE)
#define L2CAP_HCI_HEADER_LEN (HCI_DATA_TYPE_LEN + HCI_ACL_HEADER_LEN)

#ifdef __cplusplus
}
#endif
#endif /* __L2CAP_COMMON_DEFINE_H__ */
