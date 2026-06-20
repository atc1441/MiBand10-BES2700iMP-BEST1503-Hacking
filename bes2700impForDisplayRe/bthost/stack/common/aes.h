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
#ifndef __AES_H__
#define __AES_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Main Functions
unsigned char *aes_encrypt_128(const unsigned char *in, unsigned char *out, const unsigned char *key);
unsigned char *aes_decrypt_128(const unsigned char *in, unsigned char *out, const unsigned char *key);

#ifdef __cplusplus
}
#endif

#endif // !AES_H
