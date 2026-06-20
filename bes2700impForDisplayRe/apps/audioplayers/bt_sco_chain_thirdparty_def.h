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
#ifndef __BT_SCO_CHAIN_THIRDPARTY_DEF_H__
#define __BT_SCO_CHAIN_THIRDPARTY_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

#define THIRDPARTY_FRAME_LEN        (320)       // Fix sample rate to 16k.

// If you want to transfer fft data from Main Core to CP core, you possible need to use this parameter.
// If don't need this parameter, comment it and modify related code.
#define THIRDPARTY_FFT_LEN          (512+2)

#ifdef __cplusplus
}
#endif

#endif

