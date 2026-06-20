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
#ifndef __DATA_DUMP_TOTA_H__
#define __DATA_DUMP_TOTA_H__

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

void data_dump_tota_open(void);
void data_dump_tota_close(void);
void data_dump_tota_start(void);
void data_dump_tota_stop(void);
void data_dump_tota_run(uint8_t *buf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif