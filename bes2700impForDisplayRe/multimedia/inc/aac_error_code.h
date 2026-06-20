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
#ifndef __AAC_ERROR_CODE_H__
#define __AAC_ERROR_CODE_H__

typedef enum {
    AAC_ENC_DISABLED = -2, /*encoder is diabled*/
    AAC_FAILED = -1, /*process failed*/
    AAC_OK = 0, /*process success*/
    AAC_TRANSPORT_SYNC_ERROR = 0x1001, /*!< The transport decoder had
                                            synchronization problems. Do not
                                            exit decoding. Just feed new
                                              bitstream data. */
    AAC_NOT_ENOUGH_BITS = 0x1002, /*!< The input buffer ran out of bits. */
    AAC_CONCEAL_OUT = 0x4000, /*output buffer is valid and concealed*/
} aac_error_code_e;

#endif
