/***************************************************************************
 *
 * Copyright (c) 2015-2023 BES Technic
 *
 * Authored by BES CD team (Blueelf Prj).
 *
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
#ifndef __GAF_GMAS_COMMON__
#define __GAF_GMAS_COMMON__

#include "gaf_cfg.h"

/// Characteristic type values for Gaming Audio Service
enum gmap_char_type
{
    /// GMAP Role characteristic
    GMAP_CHAR_TYPE_ROLE = 0,
    /// GMAP UGG feature characteristic
    GMAP_CHAR_TYPE_UGG_FEAT,
    /// GMAP UGT feature characteristic
    GMAP_CHAR_TYPE_UGT_FEAT,
    /// GMAP BGS feature characteristic
    GMAP_CHAR_TYPE_BGS_FEAT,
    /// GMAP BGR feature characteristic
    GMAP_CHAR_TYPE_BGR_FEAT,

    GMAP_CHAR_TYPE_MAX,
};

/// GMAP Role characteristic bit field meaning
enum gmap_role_bf
{
    /// Indicate if Server supports Unicast Game Gateway (UGG) role (= 1) or not - Bit
    GMAP_ROLE_UGG_BIT = CO_BIT(0),
    /// Indicate if Server supports Unicast Game Terminal (UGT) role (= 1) or not - Bit
    GMAP_ROLE_UGT_BIT = CO_BIT(1),
    /// Indicate if Server supports Broadcast Game Sender (BGS) role (= 1) or not - Bit
    GMAP_ROLE_BGS_BIT = CO_BIT(2),
    /// Indicate if Server supports Broadcast Game Receiver (BGR) role (= 1) or not - Bit
    GMAP_ROLE_BGR_BIT = CO_BIT(3),
    /// Mask indicating that all roles are supported
    GMAP_ROLE_ALLSUPP_MASK = 0x0F,
    /// Mask indicating RFU bits
    GMAP_ROLE_RFU_MASK = 0xF0,
};

/// GMAP UGG features bit field meaning
enum gmap_ugg_feat_bf
{
    /// Indicate if UGG Multiplex feature support (= 1) or not - Bit
    GMAP_UGG_FEAT_MULTIPLEX_BIT = CO_BIT(0),
    /// Indicate if UGG 96 kbps Source feature support (= 1) or not - Bit
    GMAP_UGG_FEAT_96KBPS_SRC_BIT = CO_BIT(1),
    /// Indicate if UGG Multisink feature support role (= 1) or not - Bit
    GMAP_UGG_FEAT_MULTISINK_BIT = CO_BIT(2),
    /// Mask indicating that all features are supported
    GMAP_UGG_FEAT_ALLSUPP_MASK = 0x07,
    /// Mask indicating RFU bits
    GMAP_UGG_FEAT_RFU_MASK = 0xF8,
};

/// GMAP UGT features bit field meaning
enum gmap_ugt_feat_bf
{
    /// Indicate if UGT Source feature support (= 1) or not - Bit
    GMAP_UGT_FEAT_SOURCE_BIT = CO_BIT(0),
    /// Indicate if UGT 80 kbps Source feature support (= 1) or not - Bit
    GMAP_UGT_FEAT_80KBPS_SRC_BIT = CO_BIT(1),
    /// Indicate if UGT Sink feature support (= 1) or not - Bit
    GMAP_UGT_FEAT_SINK_BIT = CO_BIT(2),
    /// Indicate if UGT 64 kbps Sink feature support (= 1) or not - Bit
    GMAP_UGT_FEAT_64KBPS_SINK_BIT = CO_BIT(3),
    /// Indicate if UGT Multiplex feature support (= 1) or not - Bit
    GMAP_UGT_FEAT_MULTIPLEX_BIT = CO_BIT(4),
    /// Indicate if UGT Multisink feature support (= 1) or not - Bit
    GMAP_UGT_FEAT_MULTISINK_BIT = CO_BIT(5),
    /// Indicate if UGT Multisource feature support (= 1) or not - Bit
    GMAP_UGT_FEAT_MULTISOURCE_BIT = CO_BIT(6),
    /// Mask indicating that all features are supported
    GMAP_UGT_FEAT_ALLSUPP_MASK = 0x7F,
    /// Mask indicating RFU bits
    GMAP_UGT_FEAT_RFU_MASK = 0x80,
};

/// GMAP BGS features bit field meaning
enum gmap_bgs_feat_bf
{
    /// Indicate if BGS 96 kbps feature support (= 1) or not - Bit
    GMAP_BGS_FEAT_96KBPS_BIT = CO_BIT(0),
    /// Mask indicating that all features are supported
    GMAP_BGS_FEAT_ALLSUPP_MASK = 0x01,
    /// Mask indicating RFU bits
    GMAP_BGS_FEAT_RFU_MASK = 0xFE,
};

/// GMAP BGR features bit field meaning
enum gmap_bgr_feat_bf
{
    /// Indicate if BGR Multisink feature support (= 1) or not - Bit
    GMAP_BGR_FEAT_MULTISINK_BIT = CO_BIT(0),
    /// Indicate if BGR Multiplex feature support (= 1) or not - Bit
    GMAP_BGR_FEAT_MULTIPLEX_BIT = CO_BIT(1),
    /// Mask indicating that all features are supported
    GMAP_BGR_FEAT_ALLSUPP_MASK = 0x03,
    /// Mask indicating RFU bits
    GMAP_BGR_FEAT_RFU_MASK = 0xFC,
};

#endif /// __GAF_GMAS_COMMON__
