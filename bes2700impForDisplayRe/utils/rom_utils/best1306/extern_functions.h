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
#ifndef _EXTERN_FUNCTIONS_
#define _EXTERN_FUNCTIONS_

#include "plat_types.h"
#include "hal_trace.h"
#include "hal_dma.h"
#include "hal_timer.h"
#include "hal_cmu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
//#include "cmsis_os.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_chipid.h"
#include "hal_trace.h"
#include "hal_sleep.h"
#if defined(__cplusplus)
extern "C" {
#endif

void rom1_entry(struct __extern_functions__ *efp);
void set__impure_ptr(void *p);

#if defined(__cplusplus)
}
#endif

#endif /* _EXTERN_FUNCTIONS_ */
