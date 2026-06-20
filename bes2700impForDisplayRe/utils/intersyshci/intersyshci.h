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
#ifndef __INTERSYSHCI_H__
#define __INTERSYSHCI_H__
#if defined(__cplusplus)
extern "C" {
#endif

void BESHCI_Open(void);
void BESHCI_Close(void);

int bes_hci_send_data(const uint8_t *buf, uint16_t size);
void bes_hci_bt_rx_isr(void (*cb)(const uint8_t *data, uint16_t len));

void beshci_enable_sleep_checker(bool enable);

#if defined(__cplusplus)
}
#endif
#endif /* __INTERSYSHCI_H__ */
