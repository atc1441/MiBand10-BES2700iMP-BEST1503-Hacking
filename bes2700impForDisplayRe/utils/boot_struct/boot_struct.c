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
#include "hal_cmu.h"
#include "norflash_cfg.h"
#include "plat_addr_map.h"
#include "reboot_param.h"
#include "tool_msg.h"
#include "hal_trace.h"
#include "hal_bootmode.h"
#include "string.h"

extern const char sys_build_info[];

#ifdef USER_SECURE_BOOT
extern const unsigned int system_info;
#endif

#define BOOT_STRUCT_VERSION             SECURE_BOOT_VER

// -----------------------------------------------------------
// Boot struct and code sig struct
// -----------------------------------------------------------
#if defined(PROGRAMMER_INFLASH)
#undef PROGRAMMER
#endif

#define BOOT_STRUCT_LOC                 __attribute((section(".boot_struct")))

#ifdef USER_SECURE_BOOT
#define DEFAULT_BUILD_INFO              ((uint32_t)&system_info)
#else
#define DEFAULT_BUILD_INFO              ((uint32_t)sys_build_info)
#endif

#define DEFAULT_CODE_SIG_V1             \
    {                                   \
        .code_size = 0,                 \
        .sig_len = MAX_SIG_LEN_V1,      \
    }

#define DEFAULT_CODE_SIG_V2             DEFAULT_CODE_SIG_V1

#define DEFAULT_CODE_SIG                \
    {                                   \
        .code_size = 0,                 \
    }

#ifndef SECURE_BOOT_CODE_VER
#define SECURE_BOOT_CODE_VER            0
#endif

#define DEFAULT_CODE_VER_INFO           \
    {                                   \
        .code_version = SECURE_BOOT_CODE_VER, \
    }

#ifdef SIMU
#define DEFAULT_NORFLASH_SAMDLY         0x1
#define DEFAULT_NORFLASH_MOD_CLK        NORFLASH_CFG_MOD_CLK_26M
#define DEFAULT_NORFLASH_RDCMD          0x03
#else
#define DEFAULT_NORFLASH_SAMDLY         0x2
// Select 26M for 40M crystal case
#define DEFAULT_NORFLASH_MOD_CLK        NORFLASH_CFG_MOD_CLK_104M
// Below 50M: 0x03, Above 50M: 0x0B
#define DEFAULT_NORFLASH_RDCMD          0x0B
#endif

#define DEFAULT_NORFLASH_CFG            \
    {                                   \
        .neg_phase = 0,                 \
        .pos_neg   = 0,                 \
        .div       = 0x2,               \
        .samdly    = DEFAULT_NORFLASH_SAMDLY, \
        .dualmode  = 1,                 \
        .holdpin   = 0,                 \
        .wprpin    = 0,                 \
        .quadmode  = 0,                 \
        .mod_clk   = DEFAULT_NORFLASH_MOD_CLK, \
        .spiruen   = 0,                 \
        .spirden   = 0,                 \
        .dualiocmd = 0xBB,              \
        .rdcmd     = DEFAULT_NORFLASH_RDCMD, \
        .frdcmd    = 0x0B,              \
        .qrdcmd    = 0xEB,              \
    }

#if (SECURE_BOOT_VER == 1)

#ifdef PROGRAMMER

struct programmer_boot_struct_v1_t {
    struct boot_struct_v1_t boot_struct;
    struct code_sig_struct_v1_t code_sig_struct;
};

const struct programmer_boot_struct_v1_t BOOT_STRUCT_LOC programmer_boot_struct = {
    .boot_struct = {
        .hdr = {
            .magic = BOOT_MAGIC_NUMBER,
            .security = 1,
            .hash_type = BOOT_HASH_TYPE_SHA256,
            .key_type = BOOT_KEY_TYPE_RSA2048,
            .key_len = MAX_KEY_LEN_V1,
            .sig_len = MAX_SIG_LEN_V1,
            .build_info_start = ((uint32_t)sys_build_info),
        },
    },
    .code_sig_struct = DEFAULT_CODE_SIG_V1,
};

#elif defined(SECURE_BOOT)

struct secure_boot_struct_v1_t {
    struct boot_struct_v1_t boot_struct;
    struct code_sig_struct_v1_t code_sig_struct;
    struct norflash_cfg_struct_t norflash_cfg;
};

const struct secure_boot_struct_v1_t BOOT_STRUCT_LOC secure_boot_struct = {
    .boot_struct = {
        .hdr = {
            .magic = ~0UL,
            .security = 1,
            .hash_type = BOOT_HASH_TYPE_SHA256,
            .key_type = BOOT_KEY_TYPE_RSA2048,
            .key_len = MAX_KEY_LEN_V1,
            .sig_len = MAX_SIG_LEN_V1,
            .build_info_start = DEFAULT_BUILD_INFO,
        },
    },
    .code_sig_struct = DEFAULT_CODE_SIG_V1,
    .norflash_cfg = DEFAULT_NORFLASH_CFG,
};

#else

const struct boot_hdr_v1_t BOOT_STRUCT_LOC boot_struct = {
    .magic = ~0UL,
    .security = 0,
    .build_info_start = DEFAULT_BUILD_INFO,
};

#endif

#elif (SECURE_BOOT_VER == 2)

#ifdef PROGRAMMER

union programmer_boot_struct_v2_t {
    // To keep compatible with the old download tools when downloadig non-secure images
    struct {
        struct boot_struct_v1_t boot_struct;
        struct code_sig_struct_v1_t code_sig_struct;
    } s_v1;
    struct {
        struct boot_struct_v2_t boot_struct;
        struct code_sig_struct_v2_t code_sig_struct;
    } s;
};

const union programmer_boot_struct_v2_t BOOT_STRUCT_LOC programmer_boot_struct = {
    .s = {
        .boot_struct = {
            .hdr = {
                .magic = BOOT_MAGIC_NUMBER,
                .security = 1,
                .version = BOOT_STRUCT_VERSION,
                .build_info_start = ((uint32_t)sys_build_info),
            },
        },
        .code_sig_struct = DEFAULT_CODE_SIG_V2,
    },
};

#elif defined(SECURE_BOOT)

struct secure_boot_struct_v2_t {
    struct boot_struct_v2_t boot_struct;
    struct code_sig_struct_v2_t code_sig_struct;
    struct norflash_cfg_struct_t norflash_cfg;
};

const struct secure_boot_struct_v2_t BOOT_STRUCT_LOC secure_boot_struct = {
    .boot_struct = {
        .hdr = {
            .magic = ~0UL,
            .security = 1,
            .version = BOOT_STRUCT_VERSION,
            .build_info_start = DEFAULT_BUILD_INFO,
        },
    },
    .code_sig_struct = DEFAULT_CODE_SIG_V2,
    .norflash_cfg = DEFAULT_NORFLASH_CFG,
};

#else

const struct boot_hdr_v2_t BOOT_STRUCT_LOC boot_struct = {
    .magic = ~0UL,
    .security = 0,
    .version = BOOT_STRUCT_VERSION,
    .build_info_start = DEFAULT_BUILD_INFO,
};

#endif

#elif (SECURE_BOOT_VER == 3)

#ifdef PROGRAMMER

union programmer_boot_struct_t {
    // To keep compatible with the old download tools when downloadig non-secure images
    struct {
        struct boot_struct_v1_t boot_struct;
        struct code_sig_struct_v1_t code_sig_struct;
    } s_v1;
    struct {
        struct boot_struct_v3_t boot_struct;
        struct code_sig_struct_v3_t code_sig_struct;
    } s;
};

const union programmer_boot_struct_t BOOT_STRUCT_LOC programmer_boot_struct = {
    .s = {
        .boot_struct = {
            .hdr = {
                .magic = BOOT_MAGIC_NUMBER,
                .flag = BOOT_STRUCT_FLAG_SECURE_BOOT,
                .version = BOOT_STRUCT_VERSION,
                .build_info_start = ((uint32_t)sys_build_info),
            },
        },
        .code_sig_struct = DEFAULT_CODE_SIG,
    },
};

#elif defined(SECURE_BOOT)

struct secure_boot_struct_t {
    struct boot_struct_v3_t boot_struct;
    struct code_sig_struct_v3_t code_sig_struct;
    struct norflash_cfg_struct_t norflash_cfg;
    struct code_ver_struct_t code_ver_struct;
};

const struct secure_boot_struct_t BOOT_STRUCT_LOC secure_boot_struct = {
    .boot_struct = {
        .hdr = {
            .magic = ~0UL,
            .flag = BOOT_STRUCT_FLAG_SECURE_BOOT,
            .version = BOOT_STRUCT_VERSION,
            .build_info_start = DEFAULT_BUILD_INFO,
        },
    },
    .code_sig_struct = DEFAULT_CODE_SIG,
    .norflash_cfg = DEFAULT_NORFLASH_CFG,
    .code_ver_struct = DEFAULT_CODE_VER_INFO,
};

#else

const struct boot_hdr_t BOOT_STRUCT_LOC boot_struct = {
#ifdef MAGIC_NUM_AUTO
    .magic = BOOT_MAGIC_NUMBER,
#else
    .magic = ~0UL,
#endif
    .flag = 0,
    .version = BOOT_STRUCT_VERSION,
    .build_info_start = DEFAULT_BUILD_INFO,
};

#endif

#else // (SECURE_BOOT_VER >= 4)

#ifdef PROGRAMMER

union programmer_boot_struct_t {
    struct {
        struct boot_struct_t boot_struct;
        struct code_sig_struct_t code_sig_struct;
    } s;
};

const union programmer_boot_struct_t BOOT_STRUCT_LOC programmer_boot_struct = {
    .s = {
        .boot_struct = {
            .hdr = {
                .magic = BOOT_MAGIC_NUMBER,
                .flag = BOOT_STRUCT_FLAG_SECURE_BOOT,
                .version = BOOT_STRUCT_VERSION,
                .build_info_start = ((uint32_t)sys_build_info),
            },
        },
        .code_sig_struct = DEFAULT_CODE_SIG,
    },
};

#elif defined(SECURE_BOOT)

struct secure_boot_struct_t {
    struct boot_struct_t boot_struct;
    struct code_sig_struct_t code_sig_struct;
    struct norflash_cfg_struct_t norflash_cfg;
    struct code_ver_struct_t code_ver_struct;
};

const struct secure_boot_struct_t BOOT_STRUCT_LOC secure_boot_struct = {
    .boot_struct = {
        .hdr = {
            .magic = ~0UL,
            .flag = BOOT_STRUCT_FLAG_SECURE_BOOT,
            .version = BOOT_STRUCT_VERSION,
            .build_info_start = DEFAULT_BUILD_INFO,
        },
    },
    .code_sig_struct = DEFAULT_CODE_SIG,
    .norflash_cfg = DEFAULT_NORFLASH_CFG,
    .code_ver_struct = DEFAULT_CODE_VER_INFO,
};

#else

const struct boot_hdr_t BOOT_STRUCT_LOC boot_struct = {
#ifdef MAGIC_NUM_AUTO
    .magic = BOOT_MAGIC_NUMBER,
#else
    .magic = ~0UL,
#endif
    .flag = 0,
    .version = BOOT_STRUCT_VERSION,
    .build_info_start = DEFAULT_BUILD_INFO,
};

#endif

#endif // SECURE_BOOT_VER

// -----------------------------------------------------------
// Reboot param
// -----------------------------------------------------------

#ifdef __ARMCC_VERSION
#define REBOOT_PARAM_LOC __attribute((section(".bss.reboot_param")))
#else
#define REBOOT_PARAM_LOC __attribute((section(".reboot_param")))
#endif

struct REBOOT_PARAM_T {
    uint32_t reserved[REBOOT_PARAM_ID_QTY];
};

struct REBOOT_PARAM_T REBOOT_PARAM_LOC reboot_param;

void reboot_param_init(void)
{
    if ((hal_sw_bootmode_get() & HAL_SW_BOOTMODE_REBOOT) == 0) {
        memset(&reboot_param, 0, sizeof(reboot_param));
    }
}

void reboot_param_set(enum REBOOT_PARAM_ID_T id, uint32_t flags)
{
    ASSERT(id < REBOOT_PARAM_ID_QTY, "wrong id:%d\n",id);
    reboot_param.reserved[id] |= flags;
}

void reboot_param_clear(enum REBOOT_PARAM_ID_T id, uint32_t flags)
{
    ASSERT(id < REBOOT_PARAM_ID_QTY, "wrong id:%d\n",id);
    reboot_param.reserved[id] &= ~flags;
}

uint32_t reboot_param_get(enum REBOOT_PARAM_ID_T id)
{
    ASSERT(id < REBOOT_PARAM_ID_QTY, "wrong id:%d\n",id);
    return reboot_param.reserved[id];
}

