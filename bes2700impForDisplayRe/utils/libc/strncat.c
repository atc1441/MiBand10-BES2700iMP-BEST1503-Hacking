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

char *strncat(char *s1, const char *s2, size_t n)
{
    char c;
    char *s = s1;

    if (n == 0) {
      return s1;
    }

    while (*s1++ != '\0') {};
    s1--;

    while (n-- > 0) {
        c = *s2++;
        *s1++ = c;
        if (c == '\0') {
            return s;
        }
    }
    *s1 = '\0';

    return s;
}
