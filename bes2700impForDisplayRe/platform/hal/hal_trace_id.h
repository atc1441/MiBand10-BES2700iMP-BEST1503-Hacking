/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __HAL_TRACE_ID_H__
#define __HAL_TRACE_ID_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "stdarg.h"

// #define LITE_VERSION

#ifdef LITE_VERSION
#define USE_ID_1_AS_CRC
//#define USE_ID_2_AS_CRC
#define ID1_CRC             0x15

typedef struct {
    unsigned int crc:5;     //5 bit crc
    unsigned int count:4;   //16 param list
    unsigned int tskid:5;   //32 taskid
    unsigned int addr:18;   //256 KB trace space support
}trace_info_t;

typedef struct {
    unsigned int id:8;
    unsigned int timestamp:24; // 4 hours support
}trace_head_t;

#else
//define USE_ID_1_AS_CRC
#define USE_ID_2_AS_CRC
//define USE_CRC_CHECK
#define ID2_CRC             0xA5BE

typedef struct {
    unsigned int count:4;   //16 param list
    unsigned int tskid:7;   //128 taskid
    unsigned int addr:21;   //2048 KB trace space support
}trace_info_t;

typedef struct {
    unsigned int id_reserved:8;
    unsigned int timestamp:24; // 4 hours support
    unsigned int id:16;
#ifdef USE_ID_2_AS_CRC
    unsigned int crc:16;
#endif
}trace_head_t;

#endif

typedef struct {
    trace_head_t trace_head;
    trace_info_t trace_info;
}__attribute__((packed)) LOG_DATA_T;

void hal_trace_set_id(char *buf, unsigned int value);
unsigned int hal_trace_get_id(char *buf);

#define USER_ID_TWS_LR_SHIFT                (0)
#define USER_ID_TWS_LR_MASK                 (0x03<<USER_ID_TWS_LR_SHIFT)   //2 bit ,L,R,N
#define USER_ID_TWS_LR(n)                   (((n) & 0x03) << 0)
#define USER_ID_TWS_TWS_L                   0
#define USER_ID_TWS_TWS_R                   1
#define USER_ID_TWS_TWS_N                   2

#define USER_ID_TWS_ROLE_SHIFT              (2)
#define USER_ID_TWS_ROLE_MASK               (0x03<<USER_ID_TWS_ROLE_SHIFT) //2 bit ,M,S,U,F
#define USER_ID_TWS_ROLE(n)                 (((n) & 0x03) << 2)
#define USER_ID_ROLE_MASTER                 0
#define USER_ID_ROLE_SLAVE                  1
#define USER_ID_ROLE_UNKNOW                 2
#define USER_ID_ROLE_FREEMAN                3

#define USER_ID_CPUID_SHIFT                 (4)
#define USER_ID_CPUID_MASK                  (0x07<<USER_ID_CPUID_SHIFT) //3 bit
#define USER_ID_CPUID(n)                    (((n) & 0x07) << 4)
#define USER_ID_CPU_DSPM                    0
#define USER_ID_CPU_DSPH                    1
#define USER_ID_CPU_DSPA0                   0
#define USER_ID_CPU_DSPA1                   1
#define USER_ID_CPU_SYS0                    2
#define USER_ID_CPU_SYS1                    3
#define USER_ID_CPU_MCU0                    2
#define USER_ID_CPU_MCU1                    3
#define USER_ID_CPU_BTH0                    4
#define USER_ID_CPU_BTH1                    5
#define USER_ID_CPU_SENS0                   6
#define USER_ID_CPU_SENS1                   7

#define USER_ID_SECURE_SHIFT                (7)
#define USER_ID_SECURE_MASK                 (1<<USER_ID_SECURE_SHIFT) //1 bit, NS/,SE/
#define USER_ID_SECURE(n)                   (((n) & 0x01) << 7)
#define USER_ID_SECURE_NS                   0
#define USER_ID_SECURE_SE                   1

#define USER_ID_SN_SHIFT                    (8)
#define USER_ID_SN_MASK                     (0xff<<USER_ID_SN_SHIFT) //8 bit
#define USER_ID_SN(n)                       (((n) & 0xff) << 8)

#define USER_ID_RESERVED_SHIFT              (16)
#define USER_ID_RESERVED                    (1<<USER_ID_RESERVED_SHIFT) //0 bit

#ifdef __cplusplus
}
#endif
#endif
