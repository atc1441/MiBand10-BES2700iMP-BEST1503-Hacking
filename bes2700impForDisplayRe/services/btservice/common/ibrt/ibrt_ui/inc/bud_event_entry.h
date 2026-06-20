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
#ifndef __BUD_EVENT_ENTRY_H__
#define __BUD_EVENT_ENTRY_H__

#include "app_ui_evt.h"

void bud_box_event_handle_init(void);
void bud_box_event_entry(app_ui_evt_t box_evt);
bud_box_state bud_get_box_state_by_box_evt(app_ui_evt_t box_evt);


#endif /* __BUD_EVENT_ENTRY_H__ */