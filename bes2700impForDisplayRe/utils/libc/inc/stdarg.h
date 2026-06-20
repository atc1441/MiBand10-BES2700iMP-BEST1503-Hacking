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
#ifndef __STDARG_H__
#define __STDARG_H__

/*
 * ISO C Standard:  7.15  Variable arguments  <stdarg.h>
 */

/* Define the standard macros for the user,
   if this invocation was from the user program.  */

#define va_start(v,l)    __builtin_va_start(v,l)
#define va_end(v)    __builtin_va_end(v)
#define va_arg(v,l)    __builtin_va_arg(v,l)
#define va_copy(d,s)    __builtin_va_copy(d,s)

/* the version of GNU GCC must be greater than 4.x */
typedef __builtin_va_list       va_list;

#endif /* __STDARG_H__ */
