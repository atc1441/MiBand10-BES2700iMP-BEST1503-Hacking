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
#ifndef __BT_RADIO_H__
#define  __BT_RADIO_H__

#ifdef __cplusplus
extern "C" {
#endif

void btdrv_digital_common_config(void);
void btdrv_bt_modem_config(void);
uint8_t btdrv_rf_init(void);
void btdrv_ble_modem_config(void);
void btdrv_ecc_config(void);

#ifdef __cplusplus
}
#endif

#endif
