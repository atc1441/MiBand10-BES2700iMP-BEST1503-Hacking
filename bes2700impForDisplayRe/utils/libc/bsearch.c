/***************************************************************************
 *
 * Copyright 2015-2015 BES.
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
#include "stddef.h"

void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
     int (*compar)(const void *, const void *))
{
    size_t from, to, idx;
    const unsigned char *p;
    int result;

    if (nmemb == 0) {
        return NULL;
    }
    from = 0;
    to = nmemb;

    while (from < to) {
        idx = (from + to) / 2;
        p = (unsigned char *)base + idx * size;
        result = (*compar)(key, p);
        if (result < 0) {
            to = idx;
        } else if (result > 0) {
            from = idx + 1;
        } else {
            return (void *)p;
        }
    }
    return NULL;
}
