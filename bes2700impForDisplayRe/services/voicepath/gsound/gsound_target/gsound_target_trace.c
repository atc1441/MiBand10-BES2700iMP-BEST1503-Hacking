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
#include <stdarg.h>
#include <stdio.h>
#include "gsound_target.h"
#include "hal_trace.h"

void GSoundTargetTraceInit()
{
}

void GSoundTargetTrace(const char *filename,
                       int         lineno,
                       const char *fmt,
                       ...)
{
    // 100 byte buffer matches hal_trace_printf.
    char    buf[100];
    int     len;
    va_list ap;

    va_start(ap, fmt);
    len = vsnprintf(&buf[0], sizeof(buf) - 2, fmt, ap);
    va_end(ap);

    buf[len++] = '\r';
    buf[len++] = '\n';

    hal_trace_output(( unsigned char * )buf, len);
}

void GSoundTargetTraceAssert(const char *filename,
                             int         lineno,
                             const char *condition)
{
#if defined(ASSERT_SHOW_FILE_FUNC)
    hal_trace_assert_dump(filename, __FUNCTION__, lineno, condition);
#elif defined(ASSERT_SHOW_FILE)
    hal_trace_assert_dump(filename, lineno, condition);
#elif defined(ASSERT_SHOW_FUNC)
    hal_trace_assert_dump(, lineno, condition);
#else
    hal_trace_assert_dump(condition);
#endif
}
