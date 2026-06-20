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
#ifndef __BLE_L2CAP_COMMON_H__
#define __BLE_L2CAP_COMMON_H__
#ifdef BLE_HOST_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

/// L2CAP attribute Op-Codes
/// sync see@l2cap_att_code
enum bes_l2cap_att_code
{
    /// Reserved
    BES_L2CAP_ATT_RESERVED0_OPCODE          = 0x00,
    /// Error response
    BES_L2CAP_ATT_ERR_RSP_OPCODE            = 0x01,
    /// Exchange MTU Request
    BES_L2CAP_ATT_MTU_REQ_OPCODE            = 0x02,
    /// Exchange MTU Response
    BES_L2CAP_ATT_MTU_RSP_OPCODE            = 0x03,
    /// Find Information Request
    BES_L2CAP_ATT_FIND_INFO_REQ_OPCODE      = 0x04,
    /// Find Information Response
    BES_L2CAP_ATT_FIND_INFO_RSP_OPCODE      = 0x05,
    /// Find By Type Value Request
    BES_L2CAP_ATT_FIND_BY_TYPE_REQ_OPCODE   = 0x06,
    /// Find By Type Value Response
    BES_L2CAP_ATT_FIND_BY_TYPE_RSP_OPCODE   = 0x07,
    /// Read By Type Request
    BES_L2CAP_ATT_RD_BY_TYPE_REQ_OPCODE     = 0x08,
    /// Read By Type Response
    BES_L2CAP_ATT_RD_BY_TYPE_RSP_OPCODE     = 0x09,
    /// Read Request
    BES_L2CAP_ATT_RD_REQ_OPCODE             = 0x0A,
    /// Read Response
    BES_L2CAP_ATT_RD_RSP_OPCODE             = 0x0B,
    /// Read Blob Request
    BES_L2CAP_ATT_RD_BLOB_REQ_OPCODE        = 0x0C,
    /// Read Blob Response
    BES_L2CAP_ATT_RD_BLOB_RSP_OPCODE        = 0x0D,
    /// Read Multiple Request
    BES_L2CAP_ATT_RD_MULT_REQ_OPCODE        = 0x0E,
    /// Read Multiple Response
    BES_L2CAP_ATT_RD_MULT_RSP_OPCODE        = 0x0F,
    /// Read by Group Type Request
    BES_L2CAP_ATT_RD_BY_GRP_TYPE_REQ_OPCODE = 0x10,
    /// Read By Group Type Response
    BES_L2CAP_ATT_RD_BY_GRP_TYPE_RSP_OPCODE = 0x11,
    /// Write Request / Command / Signed
    BES_L2CAP_ATT_WR_REQ_OPCODE             = 0x12,
    /// Write Response
    BES_L2CAP_ATT_WR_RSP_OPCODE             = 0x13,
    /// Reserved
    BES_L2CAP_ATT_RESERVED1_OPCODE          = 0x14,
    /// Reserved
    BES_L2CAP_ATT_RESERVED2_OPCODE          = 0x15,
    /// Prepare Write Request
    BES_L2CAP_ATT_PREP_WR_REQ_OPCODE        = 0x16,
    /// Prepare Write Response
    BES_L2CAP_ATT_PREP_WR_RSP_OPCODE        = 0x17,
    /// Execute Write Request
    BES_L2CAP_ATT_EXE_WR_REQ_OPCODE         = 0x18,
    /// Execute Write Response
    BES_L2CAP_ATT_EXE_WR_RSP_OPCODE         = 0x19,
    /// Reserved
    BES_L2CAP_ATT_RESERVED3_OPCODE          = 0x1A,
    /// Handle Value Notification
    BES_L2CAP_ATT_HDL_VAL_NTF_OPCODE        = 0x1B,
    /// Reserved
    BES_L2CAP_ATT_RESERVED4_OPCODE          = 0x1C,
    /// Handle Value Indication
    BES_L2CAP_ATT_HDL_VAL_IND_OPCODE        = 0x1D,
    /// Handle Value Confirmation
    BES_L2CAP_ATT_HDL_VAL_CFM_OPCODE        = 0x1E,
    /// Reserved
    BES_L2CAP_ATT_RESERVED5_OPCODE          = 0x1F,
    /// Read two or more values of a set of attributes that have a variable or unknown value length request
    BES_L2CAP_ATT_RD_MULT_VAR_REQ_OPCODE    = 0x20,
    /// Read two or more values of a set of attributes that have a variable or unknown value length response
    BES_L2CAP_ATT_RD_MULT_VAR_RSP_OPCODE    = 0x21,
    /// Reserved
    BES_L2CAP_ATT_RESERVED6_OPCODE          = 0x22,
     /// Handle Multiple Value Notification
    BES_L2CAP_ATT_MULT_HDL_VAL_NTF_OPCODE   = 0x23,

    /// max number of security codes
    BES_L2CAP_ATT_OPCODE_MAX,

    /// Write Command
    BES_L2CAP_ATT_WR_CMD_OPCODE             = 0x52,
    /// Write Signed Command
    BES_L2CAP_ATT_WR_SIGNED_OPCODE          = 0xD2,
};

void bes_ble_gap_l2cap_data_rec_over_bt(uint8_t condix, uint16_t conhdl, uint8_t* ptrData, uint16_t dataLen);

#ifdef __cplusplus
}
#endif
#endif
#endif /* __BLE_DP_COMMON_H__ */


