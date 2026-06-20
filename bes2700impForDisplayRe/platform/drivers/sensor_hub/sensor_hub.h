/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef __SENSOR_HUB_H__
#define __SENSOR_HUB_H__

#include "plat_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int (*SENSOR_HUB_RX_IRQ_HANDLER)(const void *data, unsigned int len);

typedef void (*SENSOR_HUB_TX_IRQ_HANDLER)(const void *data, unsigned int len);

int sensor_hub_open(SENSOR_HUB_RX_IRQ_HANDLER rx_hdlr, SENSOR_HUB_TX_IRQ_HANDLER tx_hdlr);

int sensor_hub_close(void);

int sensor_hub_send_seq(const void *data, unsigned int len, unsigned int *seq);

int sensor_hub_send(const void *data, unsigned int len);

int sensor_hub_tx_active(unsigned int seq);

#ifdef __cplusplus
}
#endif

#endif

