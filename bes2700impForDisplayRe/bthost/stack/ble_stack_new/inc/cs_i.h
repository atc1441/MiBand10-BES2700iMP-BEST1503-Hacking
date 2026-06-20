/***************************************************************************
 *
 * Copyright (c) 2015-2024 BES Technic
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
#ifndef __CS_I_H__
#define __CS_I_H__

#include "bluetooth.h"
#include "cs_service.h"

enum cs_gap_evt
{
    /// Indicate that CS is connected
    CS_GAP_EVT_LE_ENCRYPTED,
    CS_GAP_EVT_LE_UNENCRYPTED,
    CS_GAP_EVT_LE_DISCONNECTED,
};

typedef void (*cs_subevt_result_report_cb_t)(const cs_recv_subevent_result_t *recv_subevt_result);
bt_status_t cs_register_subevt_result_report_callback(cs_subevt_result_report_cb_t cb);

bt_status_t cs_init(void);
void cs_inherent_event_handler(uint8_t subcode, const uint8_t *evt_data, uint8_t len);
void cs_recv_gap_conn_event(enum cs_gap_evt event, uint16_t connhdl);

#endif /* __CS_I_H__ */