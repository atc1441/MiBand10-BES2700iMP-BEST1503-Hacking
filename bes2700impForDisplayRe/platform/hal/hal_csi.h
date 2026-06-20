/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef __HAL_CSI_H__
#define __HAL_CSI_H__

#ifdef __cplusplus
extern "C" {
#endif

enum CSI_PIX_FMT_T {
    CSI_PIX_FMT_RBB565      = 0,
    CSI_PIX_FMT_RGB888      = 1,
    CSI_PIX_FMT_YUV422      = 2,
    CSI_PIX_FMT_RGBA888     = 3,
};

void hal_csi_init(void);
void hal_csi_start(enum CSI_PIX_FMT_T fmt, uint32_t width, uint32_t height, uint32_t addr);
void hal_csi_stop(void);
void hal_csi_restart(void);

#ifdef __cplusplus
}
#endif

#endif

