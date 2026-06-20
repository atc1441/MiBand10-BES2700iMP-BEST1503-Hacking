/*
 * Copyright (c) 2013-2016 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ----------------------------------------------------------------------
 *
 * interface.h    API definition for the non-secure state
 *
 * Version 1.0
 *    Initial Release
 *---------------------------------------------------------------------------*/
#ifndef __TZ_INTERFACE_H__
#define __TZ_INTERFACE_H__

#ifdef __cplusplus
extern "C"{
#endif

/* Function pointer declaration */
typedef int (*funcptr)(int);

/* Non-secure callable functions */
extern int func1(int x);
extern int func2(funcptr callback, int x);
extern int get_demo_string (char *dst, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif
