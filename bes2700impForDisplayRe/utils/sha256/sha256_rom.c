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
#include "sha256.h"
#include "stddef.h"
#include "export_fn_rom.h"

void SHA256_init(SHA256_CTX* ctx)
{
#ifdef CHIP_BEST1306
    export_fn_rom->SHA256_init(ctx);
#else
    __export_fn_rom.SHA256_init(ctx);
#endif
}

void SHA256_update(SHA256_CTX* ctx, const void* data, uint32_t len)
{
#ifdef CHIP_BEST1306
    export_fn_rom->SHA256_update(ctx, data, len);
#else
    __export_fn_rom.SHA256_update(ctx, data, len);
#endif
}

const uint8_t* SHA256_final(SHA256_CTX* ctx)
{
#ifdef CHIP_BEST1306
    return export_fn_rom->SHA256_final(ctx);
#else
    return __export_fn_rom.SHA256_final(ctx);
#endif
}

const uint8_t* SHA256_hash(const void* data, uint32_t len, uint8_t* digest)
{
#ifdef CHIP_BEST1306
    return export_fn_rom->SHA256_hash(data, len, digest);
#else
    return __export_fn_rom.SHA256_hash(data, len, digest);
#endif
}


