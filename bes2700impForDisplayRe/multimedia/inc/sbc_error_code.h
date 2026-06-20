/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#ifndef _SBC_ERROR_CODE_H_
#define _SBC_ERROR_CODE_H_

typedef enum {
    SBC_OK = 0, /*process success*/
    SBC_FAILED = 1, /*process failed*/
    SBC_INVALID_HANDLE = 2,
    SBC_INVALID_PARAM = 18,
    SBC_CONTINUE = 24,
} sbc_error_code_e;

#endif