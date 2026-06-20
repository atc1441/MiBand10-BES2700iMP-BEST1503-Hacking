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
#ifndef __APP_TRACE_RX_H__
#define __APP_TRACE_RX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "hal_trace.h"

#if defined(_AUTO_TEST_)
extern int auto_test_send(char *resp);
#define AUTO_TEST_SEND(str)         auto_test_send((char*)str)
#endif

typedef unsigned int (*APP_TRACE_RX_CALLBACK_T)(unsigned char *buf, unsigned int len);

void app_trace_rx_open(void);

int app_trace_rx_register(const char *name, APP_TRACE_RX_CALLBACK_T callback);

int app_trace_rx_deregister(const char *name);

#ifdef __cplusplus
}
#endif

#endif//__FMDEC_H__
