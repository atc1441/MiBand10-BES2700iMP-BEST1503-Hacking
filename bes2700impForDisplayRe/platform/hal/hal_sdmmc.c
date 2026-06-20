/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#include "plat_addr_map.h"

#if defined(SDMMC0_BASE) || defined(SDMMC1_BASE)

#include "hal_location.h"
#include "reg_sdmmcip.h"
#include "cmsis_nvic.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sdmmc.h"
#include "hal_iomux.h"
#include "hal_dma.h"
#include "string.h"
#include "stdio.h"
#include "errno.h"
#ifdef RTOS
    #include "cmsis_os.h"
#endif

#ifdef SDMMC_DEBUG
    #define HAL_SDMMC_TRACE(n, s, ...)   TRACE(n, s , ##__VA_ARGS__)
#else
    #define HAL_SDMMC_TRACE(...)
#endif

#ifdef SDMMC_ASSERT
    #define HAL_SDMMC_ASSERT(n, s, ...)  ASSERT(n, s , ##__VA_ARGS__)
#else
    #define HAL_SDMMC_ASSERT(...)
#endif

/*************************Functional configuration area************************/
#define MMC_WRITE
#define MMC_UHS_SUPPORT
#define MMC_IO_VOLTAGE          //Close later
//#define MMC_SUPPORTS_TUNING     //Close later
//#define MMC_HS200_SUPPORT       //Close later
//#define MMC_HS400_SUPPORT       //Close later
//#define MMC_HS400_ES_SUPPORT    //Close later
//#define MMC_HW_PARTITIONING     //Close later
#define CONFIG_CMD_BKOPS_ENABLE     //enable background operations handshake on device
#define CONFIG_MMC_SPEED_MODE_SET
//#define MMC_QUIRKS                    //default close, send cmd retry cnt, open=4, close=0
//#define CONFIG_MMC_BROKEN_CD          //default close, Use polling to detect
//#define MMC_TINY                      //default close
//#define DM_MMC                        //default close
//#define DM_REGULATOR                  //default close
//#define CONFIG_MMC_SPI                //default close
//#define BLK                           //default close, block drivers and devices
//#define CONFIG_SPL_BUILD              //default close
//#define CONFIG_SPL_LIBCOMMON_SUPPORT  //default close

/****************standard related macros and type definitions******************/
#ifdef CONFIG_SYS_64BIT_LBA
    typedef uint64_t lbaint_t;
    #define LBAFlength ""
#else
    typedef uint32_t lbaint_t;
    #define LBAFlength ""
#endif
#define LBAF "%" LBAFlength "x"
#define LBAFU "%" LBAFlength "u"

/* SD/MMC version bits; 8 flags, 8 major, 8 minor, 8 change */
#define SD_VERSION_SD                       (1U << 31)
#define MMC_VERSION_MMC                     (1U << 30)

#define MAKE_SDMMC_VERSION(a, b, c)         ((((uint32_t)(a)) << 16) | ((uint32_t)(b) << 8) | (uint32_t)(c))
#define MAKE_SD_VERSION(a, b, c)            (SD_VERSION_SD | MAKE_SDMMC_VERSION(a, b, c))
#define MAKE_MMC_VERSION(a, b, c)           (MMC_VERSION_MMC | MAKE_SDMMC_VERSION(a, b, c))

#define EXTRACT_SDMMC_MAJOR_VERSION(x)      (((uint32_t)(x) >> 16) & 0xff)
#define EXTRACT_SDMMC_MINOR_VERSION(x)      (((uint32_t)(x) >> 8) & 0xff)
#define EXTRACT_SDMMC_CHANGE_VERSION(x)     ((uint32_t)(x) & 0xff)

#define SD_VERSION_3                        MAKE_SD_VERSION(3, 0, 0)
#define SD_VERSION_2                        MAKE_SD_VERSION(2, 0, 0)
#define SD_VERSION_1_0                      MAKE_SD_VERSION(1, 0, 0)
#define SD_VERSION_1_10                     MAKE_SD_VERSION(1, 10, 0)

#define MMC_VERSION_UNKNOWN                 MAKE_MMC_VERSION(0, 0, 0)
#define MMC_VERSION_1_2                     MAKE_MMC_VERSION(1, 2, 0)
#define MMC_VERSION_1_4                     MAKE_MMC_VERSION(1, 4, 0)
#define MMC_VERSION_2_2                     MAKE_MMC_VERSION(2, 2, 0)
#define MMC_VERSION_3                       MAKE_MMC_VERSION(3, 0, 0)
#define MMC_VERSION_4                       MAKE_MMC_VERSION(4, 0, 0)
#define MMC_VERSION_4_1                     MAKE_MMC_VERSION(4, 1, 0)
#define MMC_VERSION_4_2                     MAKE_MMC_VERSION(4, 2, 0)
#define MMC_VERSION_4_3                     MAKE_MMC_VERSION(4, 3, 0)
#define MMC_VERSION_4_4                     MAKE_MMC_VERSION(4, 4, 0)
#define MMC_VERSION_4_41                    MAKE_MMC_VERSION(4, 4, 1)
#define MMC_VERSION_4_5                     MAKE_MMC_VERSION(4, 5, 0)
#define MMC_VERSION_5_0                     MAKE_MMC_VERSION(5, 0, 0)
#define MMC_VERSION_5_1                     MAKE_MMC_VERSION(5, 1, 0)

#define MMC_CAP(mode)                       (1 << mode)
#define MMC_MODE_HS                         (MMC_CAP(MMC_HS) | MMC_CAP(SD_HS))
#define MMC_MODE_HS_52MHz                    MMC_CAP(MMC_HS_52)
#define MMC_MODE_DDR_52MHz                   MMC_CAP(MMC_DDR_52)
#define MMC_MODE_HS200                       MMC_CAP(MMC_HS_200)
#define MMC_MODE_HS400                       MMC_CAP(MMC_HS_400)
#define MMC_MODE_HS400_ES                    MMC_CAP(MMC_HS_400_ES)

#define UHS_CAPS                            (MMC_CAP(UHS_SDR12) | MMC_CAP(UHS_SDR25)  | \
                                             MMC_CAP(UHS_SDR50) | MMC_CAP(UHS_SDR104) | \
                                             MMC_CAP(UHS_DDR50))

#define BIT(nr)                             (1UL << (nr))
#define MMC_CAP_NONREMOVABLE                BIT(14)
#define MMC_CAP_NEEDS_POLL                  BIT(15)
#define MMC_CAP_CD_ACTIVE_HIGH              BIT(16)

#define MMC_MODE_8BIT                       BIT(30)
#define MMC_MODE_4BIT                       BIT(29)
#define MMC_MODE_1BIT                       BIT(28)
#define MMC_MODE_SPI                        BIT(27)

#define SD_DATA_4BIT                        0x00040000

#define IS_SD(x)                            ((x)->version & SD_VERSION_SD)
#define IS_MMC(x)                           ((x)->version & MMC_VERSION_MMC)

#define MMC_DATA_READ                       1
#define MMC_DATA_WRITE                      2

#define MMC_CMD_GO_IDLE_STATE               0
#define MMC_CMD_GO_PRE_IDLE_STATE           0
#define MMC_CMD_BOOT_INITIATION             0
#define MMC_CMD_SEND_OP_COND                1
#define MMC_CMD_ALL_SEND_CID                2
#define MMC_CMD_SET_RELATIVE_ADDR           3
#define MMC_CMD_SET_DSR                     4
#define MMC_CMD_SLEEP_AWAKE                 5
#define MMC_CMD_SWITCH                      6
#define MMC_CMD_SELECT_CARD                 7
#define MMC_CMD_DESELECT_CARD               7
#define MMC_CMD_SEND_EXT_CSD                8
#define MMC_CMD_SEND_CSD                    9
#define MMC_CMD_SEND_CID                    10
#define MMC_CMD_READ_DAT_UNTIL_STOP         11
#define MMC_CMD_STOP_TRANSMISSION           12
#define MMC_CMD_SEND_STATUS                 13
#define MMC_CMD_BUS_TEST_R                  14
#define MMC_CMD_GO_INACTIVE_STATE           15
#define MMC_CMD_SEND_TUNING_BLOCK           19
#define MMC_CMD_SET_BLOCKLEN                16
#define MMC_CMD_READ_SINGLE_BLOCK           17
#define MMC_CMD_READ_MULTIPLE_BLOCK         18
#define MMC_CMD_SEND_TUNING_BLOCK_HS200     21
#define MMC_CMD_WRITE_DAT_UNTIL_STOP        20
#define MMC_CMDC_CMD22                      22
#define MMC_CMD_SET_BLOCK_COUNT             23
#define MMC_CMD_WRITE_SINGLE_BLOCK          24
#define MMC_CMD_WRITE_MULTIPLE_BLOCK        25
#define MMC_CMD_PROGRAM_CID                 26
#define MMC_CMD_PROGRAM_CSD                 27
#define MMC_CMD_SET_TIME                    49
#define MMC_CMD_SET_WRITE_PROT              28
#define MMC_CMD_CLR_WRITE_PROT              29
#define MMC_CMD_SEND_WRITE_PROT             30
#define MMC_CMD_SEND_WRITE_PROT_TYPE        31
#define MMC_CMD_CMD32                       32
#define MMC_CMD_CMD33                       33
#define MMC_CMD_CMD34                       34
#define MMC_CMD_ERASE_GROUP_START           35
#define MMC_CMD_ERASE_GROUP_END             36
#define MMC_CMD_CMD37                       37
#define MMC_CMD_ERASE                       38
#define MMC_CMD_FAST_IO                     39
#define MMC_CMD_GO_IRQ_STATE                40
#define MMC_CMD_CMD41                       41
#define MMC_CMD_LOCK_UNLOCK                 42
#define MMC_CMD_CMD43                       43
#define MMC_CMD_APP_CMD                     55
#define MMC_CMD_GEN_CMD                     56
#define MMC_CMD_CMD57                       57
#define MMC_CMD_SPI_READ_OCR                58
#define MMC_CMD_SPI_CRC_ON_OFF              59
#define MMC_CMD_CMD60                       60
#define MMC_CMD_CMD61                       61
#define MMC_CMD_RES_MAN                     62
#define MMC_CMD_CMD63                       63
#define MMC_CMD_CMD50                       50
#define MMC_CMD_CMD51                       51
#define MMC_CMD_CMD52                       52
#define MMC_CMD_PROTOCOL_RD                 53
#define MMC_CMD_PROTOCOL_WR                 54
#define MMC_CMD_QUEUED_TASK_PARAMS          44
#define MMC_CMD_QUEUED_TASK_ADDR            45
#define MMC_CMD_EXECUTE_READ_TASK           46
#define MMC_CMD_EXECUTE_WRITE_TASK          47
#define MMC_CMD_CMDQ_TASK_MGMT              48

#define MMC_CMD62_ARG1                      0xefac62ec
#define MMC_CMD62_ARG2                      0xcbaea7

#define SD_CMD_SEND_RELATIVE_ADDR           3
#define SD_CMD_SWITCH_FUNC                  6
#define SD_CMD_SEND_IF_COND                 8
#define SD_CMD_SWITCH_UHS18V                11

#define SD_CMD_APP_SET_BUS_WIDTH            6
#define SD_CMD_APP_SD_STATUS                13
#define SD_CMD_ERASE_WR_BLK_START           32
#define SD_CMD_ERASE_WR_BLK_END             33
#define SD_CMD_APP_SEND_OP_COND             41
#define SD_CMD_APP_SEND_SCR                 51

/* SCR definitions in different words */
#define SD_HIGHSPEED_BUSY                   0x00020000
#define SD_HIGHSPEED_SUPPORTED              0x00020000

#define UHS_SDR12_BUS_SPEED                 0
#define HIGH_SPEED_BUS_SPEED                1
#define UHS_SDR25_BUS_SPEED                 1
#define UHS_SDR50_BUS_SPEED                 2
#define UHS_SDR104_BUS_SPEED                3
#define UHS_DDR50_BUS_SPEED                 4

#define SD_MODE_UHS_SDR12                   BIT(UHS_SDR12_BUS_SPEED)
#define SD_MODE_UHS_SDR25                   BIT(UHS_SDR25_BUS_SPEED)
#define SD_MODE_UHS_SDR50                   BIT(UHS_SDR50_BUS_SPEED)
#define SD_MODE_UHS_SDR104                  BIT(UHS_SDR104_BUS_SPEED)
#define SD_MODE_UHS_DDR50                   BIT(UHS_DDR50_BUS_SPEED)

#define OCR_BUSY                            0x80000000
#define OCR_HCS                             0x40000000
#define OCR_S18R                            0x1000000
#define OCR_VOLTAGE_MASK                    0x007FFF80
#define OCR_ACCESS_MODE                     0x60000000

#define MMC_ERASE_ARG                       0x00000000
#define MMC_SECURE_ERASE_ARG                0x80000000
#define MMC_TRIM_ARG                        0x00000001
#define MMC_DISCARD_ARG                     0x00000003
#define MMC_SECURE_TRIM1_ARG                0x80000001
#define MMC_SECURE_TRIM2_ARG                0x80008000

#define MMC_STATUS_MASK                     (~0x0206BF7F)
#define MMC_STATUS_SWITCH_ERROR             (1 << 7)
#define MMC_STATUS_RDY_FOR_DATA             (1 << 8)
#define MMC_STATUS_CURR_STATE               (0xf << 9)
#define MMC_STATUS_ERROR                    (1 << 19)

#define MMC_STATE_PRG                       (7 << 9)
#define MMC_STATE_TRANS                     (4 << 9)

#define MMC_VDD_165_195                     0x00000080  /* VDD voltage 1.65 - 1.95 */
#define MMC_VDD_20_21                       0x00000100  /* VDD voltage 2.0 ~ 2.1 */
#define MMC_VDD_21_22                       0x00000200  /* VDD voltage 2.1 ~ 2.2 */
#define MMC_VDD_22_23                       0x00000400  /* VDD voltage 2.2 ~ 2.3 */
#define MMC_VDD_23_24                       0x00000800  /* VDD voltage 2.3 ~ 2.4 */
#define MMC_VDD_24_25                       0x00001000  /* VDD voltage 2.4 ~ 2.5 */
#define MMC_VDD_25_26                       0x00002000  /* VDD voltage 2.5 ~ 2.6 */
#define MMC_VDD_26_27                       0x00004000  /* VDD voltage 2.6 ~ 2.7 */
#define MMC_VDD_27_28                       0x00008000  /* VDD voltage 2.7 ~ 2.8 */
#define MMC_VDD_28_29                       0x00010000  /* VDD voltage 2.8 ~ 2.9 */
#define MMC_VDD_29_30                       0x00020000  /* VDD voltage 2.9 ~ 3.0 */
#define MMC_VDD_30_31                       0x00040000  /* VDD voltage 3.0 ~ 3.1 */
#define MMC_VDD_31_32                       0x00080000  /* VDD voltage 3.1 ~ 3.2 */
#define MMC_VDD_32_33                       0x00100000  /* VDD voltage 3.2 ~ 3.3 */
#define MMC_VDD_33_34                       0x00200000  /* VDD voltage 3.3 ~ 3.4 */
#define MMC_VDD_34_35                       0x00400000  /* VDD voltage 3.4 ~ 3.5 */
#define MMC_VDD_35_36                       0x00800000  /* VDD voltage 3.5 ~ 3.6 */

#define MMC_SWITCH_MODE_CMD_SET             0x00 /* Change the command set */
#define MMC_SWITCH_MODE_SET_BITS            0x01 /* Set bits in EXT_CSD byte
                                                    addressed by index which are
                                                    1 in value field */
#define MMC_SWITCH_MODE_CLEAR_BITS          0x02 /* Clear bits in EXT_CSD byte
                                                    addressed by index, which are
                                                    1 in value field */
#define MMC_SWITCH_MODE_WRITE_BYTE          0x03 /* Set target byte to value */

#define SD_SWITCH_CHECK                     0
#define SD_SWITCH_SWITCH                    1

/*
 * EXT_CSD fields
 */
#define EXT_CSD_ENH_START_ADDR              136 /* R/W */
#define EXT_CSD_ENH_SIZE_MULT               140 /* R/W */
#define EXT_CSD_GP_SIZE_MULT                143 /* R/W */
#define EXT_CSD_PARTITION_SETTING           155 /* R/W */
#define EXT_CSD_PARTITIONS_ATTRIBUTE        156 /* R/W */
#define EXT_CSD_MAX_ENH_SIZE_MULT           157 /* R */
#define EXT_CSD_PARTITIONING_SUPPORT        160 /* RO */
#define EXT_CSD_RST_N_FUNCTION              162 /* R/W */
#define EXT_CSD_BKOPS_EN                    163 /* R/W & R/W/E */
#define EXT_CSD_WR_REL_PARAM                166 /* R */
#define EXT_CSD_WR_REL_SET                  167 /* R/W */
#define EXT_CSD_RPMB_MULT                   168 /* RO */
#define EXT_CSD_USER_WP                     171 /* R/W & R/W/C_P & R/W/E_P */
#define EXT_CSD_BOOT_WP                     173 /* R/W & R/W/C_P */
#define EXT_CSD_BOOT_WP_STATUS              174 /* R */
#define EXT_CSD_ERASE_GROUP_DEF             175 /* R/W */
#define EXT_CSD_BOOT_BUS_WIDTH              177
#define EXT_CSD_PART_CONF                   179 /* R/W */
#define EXT_CSD_BUS_WIDTH                   183 /* R/W */
#define EXT_CSD_STROBE_SUPPORT              184 /* R/W */
#define EXT_CSD_HS_TIMING                   185 /* R/W */
#define EXT_CSD_REV                         192 /* RO */
#define EXT_CSD_CARD_TYPE                   196 /* RO */
#define EXT_CSD_PART_SWITCH_TIME            199 /* RO */
#define EXT_CSD_SEC_CNT                     212 /* RO, 4 bytes */
#define EXT_CSD_HC_WP_GRP_SIZE              221 /* RO */
#define EXT_CSD_HC_ERASE_GRP_SIZE           224 /* RO */
#define EXT_CSD_BOOT_MULT                   226 /* RO */
#define EXT_CSD_GENERIC_CMD6_TIME           248 /* RO */
#define EXT_CSD_BKOPS_SUPPORT               502 /* RO */

/*
 * EXT_CSD field definitions
 */
#define EXT_CSD_CMD_SET_NORMAL              (1 << 0)
#define EXT_CSD_CMD_SET_SECURE              (1 << 1)
#define EXT_CSD_CMD_SET_CPSECURE            (1 << 2)

#define EXT_CSD_CARD_TYPE_26                (1 << 0)    /* Card can run at 26MHz */
#define EXT_CSD_CARD_TYPE_52                (1 << 1)    /* Card can run at 52MHz */
#define EXT_CSD_CARD_TYPE_DDR_1_8V          (1 << 2)
#define EXT_CSD_CARD_TYPE_DDR_1_2V          (1 << 3)
#define EXT_CSD_CARD_TYPE_DDR_52            (EXT_CSD_CARD_TYPE_DDR_1_8V | \
                                            EXT_CSD_CARD_TYPE_DDR_1_2V)

#define EXT_CSD_CARD_TYPE_HS200_1_8V        BIT(4)  /* Card can run at 200MHz */
                                                    /* SDR mode @1.8V I/O */
#define EXT_CSD_CARD_TYPE_HS200_1_2V        BIT(5)  /* Card can run at 200MHz */
                                                    /* SDR mode @1.2V I/O */
#define EXT_CSD_CARD_TYPE_HS200             (EXT_CSD_CARD_TYPE_HS200_1_8V | \
                                            EXT_CSD_CARD_TYPE_HS200_1_2V)
#define EXT_CSD_CARD_TYPE_HS400_1_8V        BIT(6)
#define EXT_CSD_CARD_TYPE_HS400_1_2V        BIT(7)
#define EXT_CSD_CARD_TYPE_HS400             (EXT_CSD_CARD_TYPE_HS400_1_8V | \
                                            EXT_CSD_CARD_TYPE_HS400_1_2V)

#define EXT_CSD_BUS_WIDTH_1                 0   /* Card is in 1 bit mode */
#define EXT_CSD_BUS_WIDTH_4                 1   /* Card is in 4 bit mode */
#define EXT_CSD_BUS_WIDTH_8                 2   /* Card is in 8 bit mode */
#define EXT_CSD_DDR_BUS_WIDTH_4             5   /* Card is in 4 bit DDR mode */
#define EXT_CSD_DDR_BUS_WIDTH_8             6   /* Card is in 8 bit DDR mode */
#define EXT_CSD_DDR_FLAG                    BIT(2)  /* Flag for DDR mode */
#define EXT_CSD_BUS_WIDTH_STROBE            BIT(7)  /* Enhanced strobe mode */

#define EXT_CSD_TIMING_LEGACY               0   /* no high speed */
#define EXT_CSD_TIMING_HS                   1   /* HS */
#define EXT_CSD_TIMING_HS200                2   /* HS200 */
#define EXT_CSD_TIMING_HS400                3   /* HS400 */
#define EXT_CSD_DRV_STR_SHIFT               4   /* Driver Strength shift */

#define EXT_CSD_BOOT_ACK_ENABLE             (1 << 6)
#define EXT_CSD_BOOT_PARTITION_ENABLE       (1 << 3)
#define EXT_CSD_PARTITION_ACCESS_ENABLE     (1 << 0)
#define EXT_CSD_PARTITION_ACCESS_DISABLE    (0 << 0)

#define EXT_CSD_BOOT_ACK(x)                 (x << 6)
#define EXT_CSD_BOOT_PART_NUM(x)            (x << 3)
#define EXT_CSD_PARTITION_ACCESS(x)         (x << 0)

#define EXT_CSD_EXTRACT_BOOT_ACK(x)         (((x) >> 6) & 0x1)
#define EXT_CSD_EXTRACT_BOOT_PART(x)        (((x) >> 3) & 0x7)
#define EXT_CSD_EXTRACT_PARTITION_ACCESS(x) ((x) & 0x7)

#define EXT_CSD_BOOT_BUS_WIDTH_MODE(x)      (x << 3)
#define EXT_CSD_BOOT_BUS_WIDTH_RESET(x)     (x << 2)
#define EXT_CSD_BOOT_BUS_WIDTH_WIDTH(x)     (x)

#define EXT_CSD_PARTITION_SETTING_COMPLETED (1 << 0)

#define EXT_CSD_ENH_USR                     (1 << 0)        /* user data area is enhanced */
#define EXT_CSD_ENH_GP(x)                   (1 << ((x)+1))  /* GP part (x+1) is enhanced */

#define EXT_CSD_HS_CTRL_REL                 (1 << 0)        /* host controlled WR_REL_SET */

#define EXT_CSD_WR_DATA_REL_USR             (1 << 0)        /* user data area WR_REL */
#define EXT_CSD_WR_DATA_REL_GP(x)           (1 << ((x)+1))  /* GP part (x+1) WR_REL */

#define R1_ILLEGAL_COMMAND                  (1 << 22)
#define R1_APP_CMD                          (1 << 5)

#define MMC_RSP_PRESENT                     (1 << 0)
#define MMC_RSP_136                         (1 << 1)        /* 136 bit response */
#define MMC_RSP_CRC                         (1 << 2)        /* expect valid crc */
#define MMC_RSP_BUSY                        (1 << 3)        /* card may send busy */
#define MMC_RSP_OPCODE                      (1 << 4)        /* response contains opcode */

#define MMC_RSP_NONE                        (0)
#define MMC_RSP_R1                          (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R1b                         (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE| MMC_RSP_BUSY)
#define MMC_RSP_R2                          (MMC_RSP_PRESENT|MMC_RSP_136|MMC_RSP_CRC)
#define MMC_RSP_R3                          (MMC_RSP_PRESENT)
#define MMC_RSP_R4                          (MMC_RSP_PRESENT)
#define MMC_RSP_R5                          (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R6                          (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R7                          (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)

#define MMCPART_NOAVAILABLE                 (0xff)
#define PART_ACCESS_MASK                    (0x7)
#define PART_SUPPORT                        (0x1)
#define ENHNCD_SUPPORT                      (0x2)
#define PART_ENH_ATTRIB                     (0x1f)

#define MMC_QUIRK_RETRY_SEND_CID            BIT(0)
#define MMC_QUIRK_RETRY_SET_BLOCKLEN        BIT(1)
#define MMC_QUIRK_RETRY_APP_CMD BIT         (2)

enum mmc_voltage {
    MMC_SIGNAL_VOLTAGE_000 = 0,
    MMC_SIGNAL_VOLTAGE_120 = 1,
    MMC_SIGNAL_VOLTAGE_180 = 2,
    MMC_SIGNAL_VOLTAGE_330 = 4,
};

#define MMC_ALL_SIGNAL_VOLTAGE              (MMC_SIGNAL_VOLTAGE_120 | \
                                             MMC_SIGNAL_VOLTAGE_180 | \
                                             MMC_SIGNAL_VOLTAGE_330)

/* Maximum block size for MMC */
#define MMC_MAX_BLOCK_LEN                   512

#define BLK_VEN_SIZE                        40
#define BLK_PRD_SIZE                        20
#define BLK_REV_SIZE                        8
#define PART_FORMAT_PCAT                    0x1
#define PART_FORMAT_GPT                     0x2

/* The number of MMC physical partitions.  These consist of:
 * boot partitions (2), general purpose partitions (4) in MMC v4.4.
 */
#define MMC_NUM_BOOT_PARTITION              2
#define MMC_PART_RPMB                       3       /* RPMB partition number */

/* timing specification used */
#define MMC_TIMING_LEGACY                   0
#define MMC_TIMING_MMC_HS                   1
#define MMC_TIMING_SD_HS                    2
#define MMC_TIMING_UHS_SDR12                3
#define MMC_TIMING_UHS_SDR25                4
#define MMC_TIMING_UHS_SDR50                5
#define MMC_TIMING_UHS_SDR104               6
#define MMC_TIMING_UHS_DDR50                7
#define MMC_TIMING_MMC_DDR52                8
#define MMC_TIMING_MMC_HS200                9
#define MMC_TIMING_MMC_HS400                10

#define ARCH_DMA_MINALIGN                   32

#define ROUND(a, b)                         (((a) + (b) - 1) & ~((b) - 1))
#define PAD_COUNT(s, pad)                   (((s) - 1) / (pad) + 1)
#define PAD_SIZE(s, pad)                    (PAD_COUNT(s, pad) * pad)

#define ALLOC_ALIGN_BUFFER_PAD(type, name, size, align, pad)            \
    char __##name[ROUND(PAD_SIZE((size) * sizeof(type), pad), align)    \
              + (align - 1)];                                           \
    type *name = (type *)ALIGN((uintptr_t)__##name, align)

#define ALLOC_ALIGN_BUFFER(type, name, size, align)         \
    ALLOC_ALIGN_BUFFER_PAD(type, name, size, align, 1)
#define ALLOC_CACHE_ALIGN_BUFFER_PAD(type, name, size, pad) \
    ALLOC_ALIGN_BUFFER_PAD(type, name, size, ARCH_DMA_MINALIGN, pad)
#define ALLOC_CACHE_ALIGN_BUFFER(type, name, size)          \
    ALLOC_ALIGN_BUFFER(type, name, size, ARCH_DMA_MINALIGN)

/*
 * DEFINE_CACHE_ALIGN_BUFFER() is similar to ALLOC_CACHE_ALIGN_BUFFER, but it's
 * purpose is to allow allocating aligned buffers outside of function scope.
 * Usage of this macro shall be avoided or used with extreme care!
 */
#define DEFINE_ALIGN_BUFFER(type, name, size, align)        \
    static char __##name[ALIGN(size * sizeof(type), align)] \
            __attribute__((__aligned__(align)));            \
                                                            \
    static type *name = (type *)__##name
#define DEFINE_CACHE_ALIGN_BUFFER(type, name, size)         \
    DEFINE_ALIGN_BUFFER(type, name, size, ARCH_DMA_MINALIGN)

#define be32_to_cpu(x) \
    ((((x) & 0xff000000) >> 24) | \
     (((x) & 0x00ff0000) >>  8) | \
     (((x) & 0x0000ff00) <<  8) | \
     (((x) & 0x000000ff) << 24))

#define LOG2(x) (((x & 0xaaaaaaaa) ? 1 : 0) + ((x & 0xcccccccc) ? 2 : 0) + \
                 ((x & 0xf0f0f0f0) ? 4 : 0) + ((x & 0xff00ff00) ? 8 : 0) + \
                 ((x & 0xffff0000) ? 16 : 0))

#define MMC_CLK_ENABLE              false
#define MMC_CLK_DISABLE             true

#ifdef CONFIG_MMC_SPI
    #define mmc_host_is_spi(mmc)    ((mmc)->cfg->host_caps & MMC_MODE_SPI)
#else
    #define mmc_host_is_spi(mmc)    0
#endif

/* Minimum partition switch timeout in units of 10-milliseconds */
#define MMC_MIN_PART_SWITCH_TIME    30 /* 300 ms */
#define DEFAULT_CMD6_TIMEOUT_MS     500

struct mmc_cid {
    uint32_t psn;
    uint8_t oid;
    uint8_t mid;
    uint8_t prv;
    uint8_t mdt;
    char pnm[7];
};

struct mmc_cmd {
    uint16_t cmdidx;
    uint32_t resp_type;
    uint32_t cmdarg;
    uint32_t response[4];
};

struct mmc_data {
    union {
        char *dest;
        const char *src;
    };
    uint32_t flags;
    uint32_t blocks;
    uint32_t blocksize;
};

struct mmc;
struct mmc_ops {
    int (*send_cmd)(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data);
    int (*set_ios)(struct mmc *mmc);
    int (*init)(struct mmc *mmc);
    int (*wait_dat0)(struct mmc *mmc, int state, int timeout_us);
    int (*getcd)(struct mmc *mmc);
    int (*getwp)(struct mmc *mmc);
    int (*host_power_cycle)(struct mmc *mmc);
    int (*get_b_max)(struct mmc *mmc, void *dst, lbaint_t blkcnt);
};

enum dma_data_direction {
    DMA_BIDIRECTIONAL = 0,
    DMA_TO_DEVICE = 1,
    DMA_FROM_DEVICE = 2,
    DMA_NONE = 3,
};

struct mmc_config {
    const char *name;
#ifndef DM_MMC
    const struct mmc_ops *ops;
#endif
    uint32_t host_caps;
    uint32_t voltages;
    uint32_t f_min;
    uint32_t f_max;
    uint32_t b_max;
    uint8_t part_type;
#ifdef CONFIG_MMC_PWRSEQ
    struct udevice *pwr_dev;
#endif
};

struct sd_ssr {
    uint32_t au;            /* In sectors */
    uint32_t erase_timeout; /* In milliseconds */
    uint32_t erase_offset;  /* In milliseconds */
};

enum bus_mode {
    MMC_LEGACY,
    MMC_HS,
    SD_HS,
    MMC_HS_52,
    MMC_DDR_52,
    UHS_SDR12,
    UHS_SDR25,
    UHS_SDR50,
    UHS_DDR50,
    UHS_SDR104,
    MMC_HS_200,
    MMC_HS_400,
    MMC_HS_400_ES,
    MMC_MODES_END
};

enum if_type {
    IF_TYPE_UNKNOWN = 0,
    IF_TYPE_IDE,
    IF_TYPE_SCSI,
    IF_TYPE_ATAPI,
    IF_TYPE_USB,
    IF_TYPE_DOC,
    IF_TYPE_MMC,
    IF_TYPE_SD,
    IF_TYPE_SATA,
    IF_TYPE_HOST,
    IF_TYPE_NVME,
    IF_TYPE_EFI_LOADER,
    IF_TYPE_PVBLOCK,
    IF_TYPE_VIRTIO,
    IF_TYPE_EFI_MEDIA,

    IF_TYPE_COUNT,
};

//Identifies the partition table type (ie. MBR vs GPT GUID) signature
enum sig_type {
    SIG_TYPE_NONE,
    SIG_TYPE_MBR,
    SIG_TYPE_GUID,

    SIG_TYPE_COUNT
};

typedef struct {
    uint8_t b[16];
} efi_guid_t __attribute__((aligned(8)));

struct blk_desc {
    enum if_type    if_type;    /* type of the interface */
    uint32_t devnum;            /* device number */
    uint8_t part_type;          /* partition type */
    uint8_t target;             /* target SCSI ID */
    uint8_t lun;                /* target LUN */
    uint8_t hwpart;             /* HW partition, e.g. for eMMC */
    uint8_t type;               /* device type */
    uint8_t removable;          /* removable device */
#ifdef CONFIG_LBA48
    /* device can use 48bit addr (ATA/ATAPI v7) */
    uint8_t lba48;
#endif
    lbaint_t    lba;            /* number of blocks */
    uint16_t    blksz;          /* block size */
    uint32_t    log2blksz;      /* for convenience: log2(blksz) */
    char        vendor[BLK_VEN_SIZE + 1];   /* device vendor string */
    char        product[BLK_PRD_SIZE + 1];  /* device product number */
    char        revision[BLK_REV_SIZE + 1]; /* firmware revision */
    enum sig_type   sig_type;   /* Partition table signature type */
    union {
        uint32_t mbr_sig;       /* MBR integer signature */
        efi_guid_t guid_sig;    /* GPT GUID Signature */
    };
    lbaint_t (*block_read)(struct blk_desc *block_dev,
                           lbaint_t start,
                           lbaint_t blkcnt,
                           void *buffer);
    lbaint_t (*block_write)(struct blk_desc *block_dev,
                            lbaint_t start,
                            lbaint_t blkcnt,
                            const void *buffer);
    lbaint_t (*block_erase)(struct blk_desc *block_dev,
                            lbaint_t start,
                            lbaint_t blkcnt);
    void        *priv;          /* driver private struct pointer */
};

struct mmc {
#ifndef BLK
    //struct list_head link;
#endif
    const struct mmc_config *cfg;   /* provided configuration */
    uint32_t version;
    void *priv;
    uint32_t has_init;
    uint8_t high_capacity;
    bool clk_disable; /* true if the clock can be turned off */
    uint32_t bus_width;
    uint32_t clock;
    uint32_t saved_clock;
    enum mmc_voltage signal_voltage;
    uint32_t card_caps;
    uint32_t host_caps;
    uint32_t ocr;
    uint32_t dsr;
    uint32_t dsr_imp;
    uint32_t scr[2];
    uint32_t csd[4];
    uint32_t cid[4];
    uint16_t rca;
    uint8_t part_support;
    uint8_t part_attr;
    uint8_t wr_rel_set;
    uint8_t part_config;
    uint8_t gen_cmd6_time;      /* units: 10 ms */
    uint8_t part_switch_time;   /* units: 10 ms */
    uint32_t tran_speed;
    uint32_t legacy_speed;      /* speed for the legacy mode provided by the card */
    uint32_t read_bl_len;
#ifdef MMC_WRITE
    uint32_t write_bl_len;
    uint32_t erase_grp_size;    /* in 512-byte sectors */
#endif
#ifdef MMC_HW_PARTITIONING
    uint32_t hc_wp_grp_size;    /* in 512-byte sectors */
#endif
#ifdef MMC_WRITE
    struct sd_ssr   ssr;        /* SD status register */
#endif
    uint64_t capacity;
    uint64_t capacity_user;
    uint64_t capacity_boot;
    uint64_t capacity_rpmb;
    uint64_t capacity_gp[4];
#ifndef CONFIG_SPL_BUILD
    uint64_t enh_user_start;
    uint64_t enh_user_size;
#endif
#ifndef BLK
    struct blk_desc block_dev;
#endif
    char op_cond_pending;   /* 1 if we are waiting on an op_cond command */
    char init_in_progress;  /* 1 if we have done mmc_start_init() */
    char preinit;           /* start init as early as possible */
    char ddr_mode;
#ifdef DM_MMC
    struct udevice *dev;    /* Device for this MMC controller */
#ifdef DM_REGULATOR
    struct udevice *vmmc_supply;    /* Main voltage regulator (Vcc)*/
    struct udevice *vqmmc_supply;   /* IO voltage regulator (Vccq)*/
#endif
#endif
    uint8_t *ext_csd;
    uint32_t cardtype;              /* cardtype read from the MMC */
    enum mmc_voltage current_voltage;
    enum bus_mode selected_mode;    /* mode currently used */
    enum bus_mode best_mode;        /* best mode is the supported mode with the
                                      * highest bandwidth. It may not always be the
                                      * operating mode due to limitations when
                                      * accessing the boot partitions
                                      */
    uint32_t quirks;
    uint8_t hs400_tuning;

    enum bus_mode user_speed_mode;  /* input speed mode from user */
};

#ifdef MMC_HW_PARTITIONING
struct mmc_hwpart_conf {
    struct {
        uint32_t enh_start; /* in 512-byte sectors */
        uint32_t enh_size;  /* in 512-byte sectors, if 0 no enh area */
        unsigned wr_rel_change : 1;
        unsigned wr_rel_set : 1;
    } user;
    struct {
        uint32_t size;  /* in 512-byte sectors */
        unsigned enhanced : 1;
        unsigned wr_rel_change : 1;
        unsigned wr_rel_set : 1;
    } gp_part[4];
};

enum mmc_hwpart_conf_mode {
    MMC_HWPART_CONF_CHECK,
    MMC_HWPART_CONF_SET,
    MMC_HWPART_CONF_COMPLETE,
};
#endif

/***************************ip independent function****************************/
static struct mmc sdmmc_devices[HAL_SDMMC_ID_NUM];

//Functional declaration
static int mmc_set_clock(struct mmc *mmc, uint32_t clock, bool disable);
static int mmc_start_init(struct mmc *mmc);
static int mmc_set_signal_voltage(struct mmc *mmc, uint32_t signal_voltage);
static int mmc_switch_part(struct mmc *mmc, uint32_t part_num);
static void mmc_udelay(uint32_t cnt);
static void hal_sdmmc_delay(uint32_t ms);
#ifndef DM_MMC
    static void board_mmc_power_init(void);
#endif
#ifdef MMC_HS400_SUPPORT
    static int mmc_hs400_prepare_ddr(struct mmc *mmc);
#endif

struct mmc *find_mmc_device(int dev_num)
{
    return &sdmmc_devices[dev_num];
}

static void mmc_do_preinit(struct mmc *mmc)
{
    HAL_SDMMC_TRACE(0, "%s:%d", __func__, __LINE__);
    if (mmc->preinit) {
        mmc_start_init(mmc);
    }
}

struct blk_desc *mmc_get_blk_desc(struct mmc *mmc)
{
    return &mmc->block_dev;
}

bool mmc_is_tuning_cmd(uint16_t cmdidx)
{
    if ((cmdidx == MMC_CMD_SEND_TUNING_BLOCK_HS200) ||
        (cmdidx == MMC_CMD_SEND_TUNING_BLOCK))
        return true;
    return false;
}

enum dma_data_direction mmc_get_dma_dir(struct mmc_data *data)
{
    return data->flags & MMC_DATA_WRITE ? DMA_TO_DEVICE : DMA_FROM_DEVICE;
}

static bool mmc_is_mode_ddr(enum bus_mode mode)
{
    if (mode == MMC_DDR_52)
        return true;
#ifdef MMC_UHS_SUPPORT
    else if (mode == UHS_DDR50)
        return true;
#endif
#ifdef MMC_HS400_SUPPORT
    else if (mode == MMC_HS_400)
        return true;
#endif
#ifdef MMC_HS400_ES_SUPPORT
    else if (mode == MMC_HS_400_ES)
        return true;
#endif
    else
        return false;
}

static bool supports_uhs(uint32_t caps)
{
#ifdef MMC_UHS_SUPPORT
    return (caps & UHS_CAPS) ? true : false;
#else
    return false;
#endif
}

static uint32_t __div64_32(uint64_t *n, uint32_t base)
{
    uint64_t rem = *n;
    uint64_t b = base;
    uint64_t res, d = 1;
    uint32_t high = rem >> 32;

    /* Reduce the thing a bit first */
    res = 0;
    if (high >= base) {
        high /= base;
        res = (uint64_t) high << 32;
        rem -= (uint64_t)(high * base) << 32;
    }

    while ((int64_t)b > 0 && b < rem) {
        b = b + b;
        d = d + d;
    }

    do {
        if (rem >= b) {
            rem -= b;
            res += d;
        }
        b >>= 1;
        d >>= 1;
    } while (d);

    *n = res;
    return rem;
}

/* The unnecessary pointer compare is there
 * to check for type safety (n must be 64bit)
 */
#define do_div(n,base) ({                               \
        uint32_t __base = (base);                       \
        uint32_t __rem;                                 \
        (void)(((typeof((n)) *)0) == ((uint64_t *)0));  \
        if (((n) >> 32) == 0) {                         \
            __rem = (uint32_t)(n) % __base;             \
            (n) = (uint32_t)(n) / __base;               \
        } else                                          \
            __rem = __div64_32(&(n), __base);           \
        __rem;                                          \
    })

/* Wrapper for do_div(). Doesn't modify dividend and returns
 * the result, not reminder.
 */
static inline uint64_t sdmmc_lldiv(uint64_t dividend, uint32_t divisor)
{
    uint64_t __res = dividend;
    do_div(__res, divisor);
    return (__res);
}

static inline uint64_t div_u64_rem(uint64_t dividend, uint32_t divisor, uint32_t *remainder)
{
    *remainder = do_div(dividend, divisor);
    return dividend;
}

#ifndef DM_MMC
static int mmc_wait_dat0(struct mmc *mmc, int state, int timeout_us)
{
    if (mmc->cfg->ops->wait_dat0) {
        return mmc->cfg->ops->wait_dat0(mmc, state, timeout_us);
    }

    return HAL_SDMMC_INVALID_SYS_CALL;
}

//write protect
int mmc_getwp(struct mmc *mmc)
{
    int wp;

    if (mmc->cfg->ops->getwp)
        wp = mmc->cfg->ops->getwp(mmc);
    else
        wp = 0;

    return wp;
}
#endif

#ifdef SDMMC_DEBUG
static void mmmc_trace_before_send(struct mmc *mmc, struct mmc_cmd *cmd)
{
    HAL_SDMMC_TRACE(0, "CMD_SEND:%d", cmd->cmdidx);
    HAL_SDMMC_TRACE(0, "\t\tARG\t\t\t 0x%08x", cmd->cmdarg);
}

static void mmmc_trace_after_send(struct mmc *mmc, struct mmc_cmd *cmd, int ret)
{
    uint8_t i;
    uint8_t *ptr;

    if (ret) {
        HAL_SDMMC_TRACE(0, "\t\tRET\t\t\t %d", ret);
    } else {
        switch (cmd->resp_type) {
            case MMC_RSP_NONE:
                HAL_SDMMC_TRACE(0, "\t\tMMC_RSP_NONE");
                break;
            case MMC_RSP_R1:
                HAL_SDMMC_TRACE(0, "\t\tMMC_RSP_R1,5,6,7 \t 0x%08x ",
                                cmd->response[0]);
                break;
            case MMC_RSP_R1b:
                HAL_SDMMC_TRACE(0, "\t\tMMC_RSP_R1b\t\t 0x%08x ",
                                cmd->response[0]);
                break;
            case MMC_RSP_R2:
                HAL_SDMMC_TRACE(0, "\t\tMMC_RSP_R2\t\t 0x%08x ",
                                cmd->response[0]);
                HAL_SDMMC_TRACE(0, "\t\t          \t\t 0x%08x ",
                                cmd->response[1]);
                HAL_SDMMC_TRACE(0, "\t\t          \t\t 0x%08x ",
                                cmd->response[2]);
                HAL_SDMMC_TRACE(0, "\t\t          \t\t 0x%08x ",
                                cmd->response[3]);
                HAL_SDMMC_TRACE(0, " ");
                HAL_SDMMC_TRACE(0, "\t\t\t\t\tDUMPING DATA");
                for (i = 0; i < 4; i++) {
                    uint8_t j;
                    HAL_SDMMC_TRACE(TR_ATTR_NO_LF, "\t\t\t\t\t%03d - ", i * 4);
                    ptr = (uint8_t *)&cmd->response[i];
                    ptr += 3;
                    for (j = 0; j < 4; j++)
                        HAL_SDMMC_TRACE(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, "%02X ", *ptr--);
                    HAL_SDMMC_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, " ");
                }
                break;
            case MMC_RSP_R3:
                HAL_SDMMC_TRACE(0, "\t\tMMC_RSP_R3,4\t\t 0x%08x ",
                                cmd->response[0]);
                break;
            default:
                HAL_SDMMC_TRACE(0, "\t\tERROR MMC rsp not supported");
                break;
        }
    }
}

static void mmc_trace_state(struct mmc *mmc, struct mmc_cmd *cmd)
{
    uint32_t status;

    status = (cmd->response[0] & MMC_STATUS_CURR_STATE) >> 9;
    HAL_SDMMC_TRACE(0, "CURR STATE:%d", status);
}

static const char *mmc_mode_name(enum bus_mode mode)
{
    static const char *const names[] = {
        [MMC_LEGACY]  = "MMC legacy",
        [MMC_HS]      = "MMC High Speed (24MHz)",
        [SD_HS]       = "SD High Speed (48MHz)",
        [UHS_SDR12]   = "UHS SDR12 (24MHz)",
        [UHS_SDR25]   = "UHS SDR25 (48MHz)",
        [UHS_SDR50]   = "UHS SDR50 (96MHz)",
        [UHS_SDR104]  = "UHS SDR104 (200MHz)",
        [UHS_DDR50]   = "UHS DDR50 (48MHz)",
        [MMC_HS_52]   = "MMC High Speed (48MHz)",
        [MMC_DDR_52]  = "MMC DDR52 (48MHz)",
        [MMC_HS_200]  = "HS200 (200MHz)",
        [MMC_HS_400]  = "HS400 (200MHz)",
        [MMC_HS_400_ES]   = "HS400ES (200MHz)",
    };

    if (mode >= MMC_MODES_END)
        return "Unknown mode";
    else
        return names[mode];
}
#endif

static uint32_t mmc_mode2freq(struct mmc *mmc, enum bus_mode mode)
{
    static const uint32_t freqs[] = {
        [MMC_LEGACY]  = 24000000,
        [MMC_HS]      = 24000000,
        [SD_HS]       = 48000000,
        [MMC_HS_52]   = 48000000,
        [MMC_DDR_52]  = 48000000,
        [UHS_SDR12]   = 24000000,
        [UHS_SDR25]   = 48000000,
        [UHS_SDR50]   = 96000000,
        [UHS_DDR50]   = 48000000,
        [UHS_SDR104]  = 200000000,
        [MMC_HS_200]  = 200000000,
        [MMC_HS_400]  = 200000000,
        [MMC_HS_400_ES]   = 200000000,
    };

    if (mode == MMC_LEGACY)
        return mmc->legacy_speed;
    else if (mode >= MMC_MODES_END)
        return 0;
    else
        return freqs[mode];
}

static int mmc_select_mode(struct mmc *mmc, enum bus_mode mode)
{
    mmc->selected_mode = mode;
    mmc->tran_speed = mmc_mode2freq(mmc, mode);
    mmc->ddr_mode = mmc_is_mode_ddr(mode);
    HAL_SDMMC_TRACE(0, "selecting mode %s (freq : %d MHz)", mmc_mode_name(mode),
                    mmc->tran_speed / 1000000);
    return HAL_SDMMC_ERR_NONE;
}

#ifndef DM_MMC
static int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
    int ret;

#ifdef SDMMC_DEBUG
    mmmc_trace_before_send(mmc, cmd);
#endif
    ret = mmc->cfg->ops->send_cmd(mmc, cmd, data);
#ifdef SDMMC_DEBUG
    mmmc_trace_after_send(mmc, cmd, ret);
#endif

    return ret;
}
#endif

static int mmc_send_cmd_retry(struct mmc *mmc, struct mmc_cmd *cmd,
                              struct mmc_data *data, uint32_t retries)
{
    int ret;

    do {
        ret = mmc_send_cmd(mmc, cmd, data);
    } while (ret && retries--);

    return ret;
}

static int mmc_send_cmd_quirks(struct mmc *mmc, struct mmc_cmd *cmd,
                               struct mmc_data *data, uint32_t quirk, uint32_t retries)
{
#ifdef MMC_QUIRKS
    if (mmc->quirks & quirk)
        return mmc_send_cmd_retry(mmc, cmd, data, retries);
    else
#endif
        return mmc_send_cmd(mmc, cmd, data);
}

static int mmc_send_status(struct mmc *mmc, uint32_t *status)
{
    struct mmc_cmd cmd;
    int ret;

    cmd.cmdidx = MMC_CMD_SEND_STATUS;
    cmd.resp_type = MMC_RSP_R1;
    if (!mmc_host_is_spi(mmc))
        cmd.cmdarg = mmc->rca << 16;

    ret = mmc_send_cmd_retry(mmc, &cmd, NULL, 4);
#ifdef SDMMC_DEBUG
    mmc_trace_state(mmc, &cmd);
#endif
    if (!ret)
        *status = cmd.response[0];

    return ret;
}

static int mmc_poll_for_busy(struct mmc *mmc, int timeout_ms)
{
    uint32_t status;
    int err;

    err = mmc_wait_dat0(mmc, 1, timeout_ms * 1000);
    if (err != HAL_SDMMC_INVALID_SYS_CALL)
        return err;

    while (1) {
        err = mmc_send_status(mmc, &status);
        if (err)
            return err;

        if ((status & MMC_STATUS_RDY_FOR_DATA) &&
            (status & MMC_STATUS_CURR_STATE) != MMC_STATE_PRG)
            break;

        if (status & MMC_STATUS_MASK) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
            HAL_SDMMC_TRACE(0, "Status Error: 0x%08x", status);
#endif
            return HAL_SDMMC_COMM_ERR;/* Communication error on send */
        }

        if (timeout_ms-- <= 0)
            break;

        mmc_udelay(1000);
    }

    if (timeout_ms <= 0) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
        HAL_SDMMC_TRACE(0, "Timeout waiting card ready");
#endif
        return HAL_SDMMC_COMM_TIMEOUT;
    }

    return HAL_SDMMC_ERR_NONE;
}

static int mmc_set_blocklen(struct mmc *mmc, int len)
{
    struct mmc_cmd cmd;

    if (mmc->ddr_mode)
        return HAL_SDMMC_ERR_NONE;

    cmd.cmdidx = MMC_CMD_SET_BLOCKLEN;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = len;

    return mmc_send_cmd_quirks(mmc, &cmd, NULL,
                               MMC_QUIRK_RETRY_SET_BLOCKLEN, 4);
}

#ifdef MMC_SUPPORTS_TUNING
static const uint8_t tuning_blk_pattern_4bit[] = {
    0xff, 0x0f, 0xff, 0x00, 0xff, 0xcc, 0xc3, 0xcc,
    0xc3, 0x3c, 0xcc, 0xff, 0xfe, 0xff, 0xfe, 0xef,
    0xff, 0xdf, 0xff, 0xdd, 0xff, 0xfb, 0xff, 0xfb,
    0xbf, 0xff, 0x7f, 0xff, 0x77, 0xf7, 0xbd, 0xef,
    0xff, 0xf0, 0xff, 0xf0, 0x0f, 0xfc, 0xcc, 0x3c,
    0xcc, 0x33, 0xcc, 0xcf, 0xff, 0xef, 0xff, 0xee,
    0xff, 0xfd, 0xff, 0xfd, 0xdf, 0xff, 0xbf, 0xff,
    0xbb, 0xff, 0xf7, 0xff, 0xf7, 0x7f, 0x7b, 0xde,
};

static const uint8_t tuning_blk_pattern_8bit[] = {
    0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00,
    0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc, 0xcc,
    0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff, 0xff,
    0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee, 0xff,
    0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd, 0xdd,
    0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb,
    0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff, 0xff,
    0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee, 0xff,
    0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00,
    0x00, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc,
    0xcc, 0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff,
    0xff, 0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee,
    0xff, 0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd,
    0xdd, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff,
    0xbb, 0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff,
    0xff, 0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee,
};

int mmc_send_tuning(struct mmc *mmc, uint32_t opcode, int *cmd_error)
{
    struct mmc_cmd cmd;
    struct mmc_data data;
    const uint8_t *tuning_block_pattern;
    int size, err;

    if (mmc->bus_width == 8) {
        tuning_block_pattern = tuning_blk_pattern_8bit;
        size = sizeof(tuning_blk_pattern_8bit);
    } else if (mmc->bus_width == 4) {
        tuning_block_pattern = tuning_blk_pattern_4bit;
        size = sizeof(tuning_blk_pattern_4bit);
    } else {
        return HAL_SDMMC_INVALID_PARAMETER;
    }

    ALLOC_CACHE_ALIGN_BUFFER(uint8_t, data_buf, size);

    cmd.cmdidx = opcode;
    cmd.cmdarg = 0;
    cmd.resp_type = MMC_RSP_R1;

    data.dest = (void *)data_buf;
    data.blocks = 1;
    data.blocksize = size;
    data.flags = MMC_DATA_READ;

    err = mmc_send_cmd(mmc, &cmd, &data);
    if (err)
        return err;

    if (memcmp(data_buf, tuning_block_pattern, size))
        return HAL_SDMMC_IO_ERR;

    return HAL_SDMMC_ERR_NONE;
}
#endif

static int mmc_read_blocks(struct mmc *mmc, void *dst, lbaint_t start,
                           lbaint_t blkcnt)
{
    struct mmc_cmd cmd;
    struct mmc_data data;

    if (blkcnt > 1)
        cmd.cmdidx = MMC_CMD_READ_MULTIPLE_BLOCK;
    else
        cmd.cmdidx = MMC_CMD_READ_SINGLE_BLOCK;

    if (mmc->high_capacity)
        cmd.cmdarg = start;
    else
        cmd.cmdarg = start * mmc->read_bl_len;

    cmd.resp_type = MMC_RSP_R1;

    data.dest = dst;
    data.blocks = blkcnt;
    data.blocksize = mmc->read_bl_len;
    data.flags = MMC_DATA_READ;

    if (mmc_send_cmd(mmc, &cmd, &data))
        return 0;

    if (blkcnt > 1) {
        cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
        cmd.cmdarg = 0;
        cmd.resp_type = MMC_RSP_R1b;
        if (mmc_send_cmd(mmc, &cmd, NULL)) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
            HAL_SDMMC_TRACE(0, "mmc fail to send stop cmd");
#endif
            return 0;
        }
    }

    return blkcnt;
}

#ifndef DM_MMC
static int mmc_get_b_max(struct mmc *mmc, void *dst, lbaint_t blkcnt)
{
    if (mmc->cfg->ops->get_b_max)
        return mmc->cfg->ops->get_b_max(mmc, dst, blkcnt);
    else
        return mmc->cfg->b_max;
}
#endif

#ifndef MMC_TINY
static int blk_dselect_hwpart(uint32_t dev_num, uint32_t hwpart)
{
    struct mmc *mmc = find_mmc_device(dev_num);
    int ret;

    if (!mmc)
        return HAL_SDMMC_NO_SUCH_DEVICE;

    if (mmc->block_dev.hwpart == hwpart)
        return HAL_SDMMC_ERR_NONE;

    if (mmc->part_config == MMCPART_NOAVAILABLE) {
        HAL_SDMMC_TRACE(0, "Card doesn't support part_switch");
        return HAL_SDMMC_MEDIUM_TYPE_ERR;
    }

    ret = mmc_switch_part(mmc, hwpart);
    if (ret)
        return ret;

    return HAL_SDMMC_ERR_NONE;
}
#endif

#ifdef BLK
lbaint_t mmc_bread(struct udevice *dev, lbaint_t start, lbaint_t blkcnt, void *dst)
#else
lbaint_t mmc_bread(struct blk_desc *block_dev, lbaint_t start, lbaint_t blkcnt,
                   void *dst)
#endif
{
#ifdef BLK
    struct blk_desc *block_dev = dev_get_uclass_plat(dev);
#endif
    uint32_t dev_num = block_dev->devnum;
    int err;
    lbaint_t cur, blocks_todo = blkcnt;
    uint32_t b_max;

    if (blkcnt == 0)
        return 0;

    struct mmc *mmc = find_mmc_device(dev_num);
    if (!mmc)
        return 0;

#ifdef MMC_TINY
    err = mmc_switch_part(mmc, block_dev->hwpart);
#else
    err = blk_dselect_hwpart(dev_num, block_dev->hwpart);
#endif
    if (err)
        return 0;

    if ((start + blkcnt) > block_dev->lba) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
        HAL_SDMMC_TRACE(0, "MMC: block number 0x" LBAF " exceeds max(0x" LBAF ")",
                        start + blkcnt, block_dev->lba);
#endif
        return 0;
    }

    if (mmc_set_blocklen(mmc, mmc->read_bl_len)) {
        HAL_SDMMC_TRACE(0, "%s: Failed to set blocklen", __FUNCTION__);
        return 0;
    }

    b_max = mmc_get_b_max(mmc, dst, blkcnt);

    do {
        cur = (blocks_todo > b_max) ? b_max : blocks_todo;
        if (mmc_read_blocks(mmc, dst, start, cur) != cur) {
            HAL_SDMMC_TRACE(0, "%s: Failed to read blocks", __FUNCTION__);
            return 0;
        }
        blocks_todo -= cur;
        start += cur;
        dst += cur * mmc->read_bl_len;
    } while (blocks_todo > 0);

    return blkcnt;
}

static int mmc_erase_t(struct mmc *mmc, lbaint_t start, lbaint_t blkcnt)
{
    struct mmc_cmd cmd;
    lbaint_t end;
    int err, start_cmd, end_cmd;

    if (mmc->high_capacity) {
        end = start + blkcnt - 1;
    } else {
        end = (start + blkcnt - 1) * mmc->write_bl_len;
        start *= mmc->write_bl_len;
    }

    if (IS_SD(mmc)) {
        start_cmd = SD_CMD_ERASE_WR_BLK_START;
        end_cmd = SD_CMD_ERASE_WR_BLK_END;
    } else {
        start_cmd = MMC_CMD_ERASE_GROUP_START;
        end_cmd = MMC_CMD_ERASE_GROUP_END;
    }

    cmd.cmdidx = start_cmd;
    cmd.cmdarg = start;
    cmd.resp_type = MMC_RSP_R1;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err)
        goto err_out;

    cmd.cmdidx = end_cmd;
    cmd.cmdarg = end;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err)
        goto err_out;

    cmd.cmdidx = MMC_CMD_ERASE;
    cmd.cmdarg = MMC_ERASE_ARG;
    cmd.resp_type = MMC_RSP_R1b;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err)
        goto err_out;

    return HAL_SDMMC_ERR_NONE;

err_out:
    HAL_SDMMC_TRACE(0, "mmc erase failed");
    return err;
}

#ifdef BLK
    lbaint_t mmc_berase(struct udevice *dev, lbaint_t start, lbaint_t blkcnt)
#else
    lbaint_t mmc_berase(struct blk_desc *block_dev, lbaint_t start, lbaint_t blkcnt)
#endif
{
#ifdef BLK
    struct blk_desc *block_dev = dev_get_uclass_plat(dev);
#endif
    uint32_t dev_num = block_dev->devnum;
    int err = 0;
    uint32_t start_rem, blkcnt_rem;
    struct mmc *mmc = find_mmc_device(dev_num);
    lbaint_t blk = 0, blk_r = 0;
    uint32_t timeout_ms = 1000;

    if (!mmc)
        return HAL_SDMMC_NO_SUCH_DEVICE;

    err = blk_dselect_hwpart(dev_num, block_dev->hwpart);
    if (err)
        return err;

    /*
     * We want to see if the requested start or total block count are
     * unaligned.  We discard the whole numbers and only care about the
     * remainder.
     */
    err = div_u64_rem(start, mmc->erase_grp_size, &start_rem);
    err = div_u64_rem(blkcnt, mmc->erase_grp_size, &blkcnt_rem);
    if (start_rem || blkcnt_rem)
        HAL_SDMMC_TRACE(0, "\n\nCaution! Your devices Erase group is 0x%x\n"
                        "The erase range would be change to "
                        "0x" LBAF "~0x" LBAF "\n\n",
                        mmc->erase_grp_size, start & ~(mmc->erase_grp_size - 1),
                        ((start + blkcnt + mmc->erase_grp_size)
                         & ~(mmc->erase_grp_size - 1)) - 1);

    while (blk < blkcnt) {
        if (IS_SD(mmc) && mmc->ssr.au) {
            blk_r = ((blkcnt - blk) > mmc->ssr.au) ?
                    mmc->ssr.au : (blkcnt - blk);
        } else {
            blk_r = ((blkcnt - blk) > mmc->erase_grp_size) ?
                    mmc->erase_grp_size : (blkcnt - blk);
        }
        err = mmc_erase_t(mmc, start + blk, blk_r);
        if (err)
            break;

        blk += blk_r;

        /* Waiting for the ready status */
        if (mmc_poll_for_busy(mmc, timeout_ms))
            return 0;
    }

    return blk;
}

static lbaint_t mmc_write_blocks(struct mmc *mmc, lbaint_t start,
                                 lbaint_t blkcnt, const void *src)
{
    struct mmc_cmd cmd;
    struct mmc_data data;
    uint32_t timeout_ms = 1000;

    if ((start + blkcnt) > mmc_get_blk_desc(mmc)->lba) {
        HAL_SDMMC_TRACE(0, "MMC: block number 0x" LBAF " exceeds max(0x" LBAF ")",
                        start + blkcnt, mmc_get_blk_desc(mmc)->lba);
        return 0;
    }

    if (blkcnt == 0)
        return 0;
    else if (blkcnt == 1)
        cmd.cmdidx = MMC_CMD_WRITE_SINGLE_BLOCK;
    else
        cmd.cmdidx = MMC_CMD_WRITE_MULTIPLE_BLOCK;

    if (mmc->high_capacity)
        cmd.cmdarg = start;
    else
        cmd.cmdarg = start * mmc->write_bl_len;

    cmd.resp_type = MMC_RSP_R1;

    data.src = src;
    data.blocks = blkcnt;
    data.blocksize = mmc->write_bl_len;
    data.flags = MMC_DATA_WRITE;

    if (mmc_send_cmd(mmc, &cmd, &data)) {
        HAL_SDMMC_TRACE(0, "mmc write failed");
        return 0;
    }

    /* SPI multiblock writes terminate using a special
     * token, not a STOP_TRANSMISSION request.
     */
    if (!mmc_host_is_spi(mmc) && blkcnt > 1) {
        cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
        cmd.cmdarg = 0;
        cmd.resp_type = MMC_RSP_R1b;
        if (mmc_send_cmd(mmc, &cmd, NULL)) {
            HAL_SDMMC_TRACE(0, "mmc fail to send stop cmd");
            return 0;
        }
    }

    /* Waiting for the ready status */
    if (mmc_poll_for_busy(mmc, timeout_ms))
        return 0;

    return blkcnt;
}

#ifdef BLK
lbaint_t mmc_bwrite(struct udevice *dev, lbaint_t start, lbaint_t blkcnt,
                    const void *src)
#else
lbaint_t mmc_bwrite(struct blk_desc *block_dev, lbaint_t start, lbaint_t blkcnt,
                    const void *src)
#endif
{
#ifdef BLK
    struct blk_desc *block_dev = dev_get_uclass_plat(dev);
#endif
    uint32_t dev_num = block_dev->devnum;
    lbaint_t cur, blocks_todo = blkcnt;
    int err;

    struct mmc *mmc = find_mmc_device(dev_num);
    if (!mmc)
        return 0;

    err = blk_dselect_hwpart(dev_num, block_dev->hwpart);
    if (err)
        return 0;

    if (mmc_set_blocklen(mmc, mmc->write_bl_len))
        return 0;

    do {
        cur = (blocks_todo > mmc->cfg->b_max) ?
              mmc->cfg->b_max : blocks_todo;
        if (mmc_write_blocks(mmc, start, cur, src) != cur)
            return 0;
        blocks_todo -= cur;
        start += cur;
        src += cur * mmc->write_bl_len;
    } while (blocks_todo > 0);

    return blkcnt;
}

static int mmc_go_idle(struct mmc *mmc)
{
    struct mmc_cmd cmd;
    int err;

    mmc_udelay(1000);
    cmd.cmdidx = MMC_CMD_GO_IDLE_STATE;
    cmd.cmdarg = 0;
    cmd.resp_type = MMC_RSP_NONE;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err)
        return err;

    mmc_udelay(2000);

    return HAL_SDMMC_ERR_NONE;
}

#ifdef MMC_UHS_SUPPORT
static int mmc_switch_voltage(struct mmc *mmc, int signal_voltage)
{
    struct mmc_cmd cmd;
    int err = 0;

    /*
     * Send CMD11 only if the request is to switch the card to
     * 1.8V signalling.
     */
    if (signal_voltage == MMC_SIGNAL_VOLTAGE_330)
        return mmc_set_signal_voltage(mmc, signal_voltage);

    cmd.cmdidx = SD_CMD_SWITCH_UHS18V;
    cmd.cmdarg = 0;
    cmd.resp_type = MMC_RSP_R1;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err)
        return err;

    if (!mmc_host_is_spi(mmc) && (cmd.response[0] & MMC_STATUS_ERROR))
        return HAL_SDMMC_IO_ERR;

    /*
     * The card should drive cmd and dat[0:3] low immediately
     * after the response of cmd11, but wait 100 us to be sure
     */
    err = mmc_wait_dat0(mmc, 0, 100);
    if (err == HAL_SDMMC_INVALID_SYS_CALL)
        mmc_udelay(100);
    else if (err)
        return HAL_SDMMC_COMM_TIMEOUT;

    /*
     * During a signal voltage level switch, the clock must be gated
     * for 5 ms according to the SD spec
     */
    mmc_set_clock(mmc, mmc->clock, MMC_CLK_DISABLE);

    err = mmc_set_signal_voltage(mmc, signal_voltage);
    if (err)
        return err;

    /* Keep clock gated for at least 10 ms, though spec only says 5 ms */
    hal_sdmmc_delay(10);
    mmc_set_clock(mmc, mmc->clock, MMC_CLK_ENABLE);

    /*
     * Failure to switch is indicated by the card holding
     * dat[0:3] low. Wait for at least 1 ms according to spec
     */
    err = mmc_wait_dat0(mmc, 1, 1000);
    if (err == HAL_SDMMC_INVALID_SYS_CALL)
        mmc_udelay(1000);
    else if (err)
        return HAL_SDMMC_COMM_TIMEOUT;

    return HAL_SDMMC_ERR_NONE;
}
#endif

static int sd_send_op_cond(struct mmc *mmc, bool uhs_en)
{
    int32_t timeout = 1000;
    int err;
    struct mmc_cmd cmd;

    while (1) {
        cmd.cmdidx = MMC_CMD_APP_CMD;
        cmd.resp_type = MMC_RSP_R1;
        cmd.cmdarg = 0;

        err = mmc_send_cmd(mmc, &cmd, NULL);

        if (err)
            return err;

        cmd.cmdidx = SD_CMD_APP_SEND_OP_COND;
        cmd.resp_type = MMC_RSP_R3;

        /*
         * Most cards do not answer if some reserved bits
         * in the ocr are set. However, Some controller
         * can set bit 7 (reserved for low voltages), but
         * how to manage low voltages SD card is not yet
         * specified.
         */
        cmd.cmdarg = mmc_host_is_spi(mmc) ? 0 :
                     (mmc->cfg->voltages & 0xff8000);

        if (mmc->version == SD_VERSION_2)
            cmd.cmdarg |= OCR_HCS;

        if (uhs_en)
            cmd.cmdarg |= OCR_S18R;

        err = mmc_send_cmd(mmc, &cmd, NULL);

        if (err)
            return err;

        if (cmd.response[0] & OCR_BUSY)
            break;

        if (timeout-- <= 0)
            return HAL_SDMMC_OP_NOT_SUPPORTED_EP;

        //mmc_udelay(1000);
        hal_sdmmc_delay(1);
    }

    if (mmc->version != SD_VERSION_2)
        mmc->version = SD_VERSION_1_0;

    if (mmc_host_is_spi(mmc)) { /* read OCR for spi */
        cmd.cmdidx = MMC_CMD_SPI_READ_OCR;
        cmd.resp_type = MMC_RSP_R3;
        cmd.cmdarg = 0;

        err = mmc_send_cmd(mmc, &cmd, NULL);

        if (err)
            return err;
    }

    mmc->ocr = cmd.response[0];

#ifdef MMC_UHS_SUPPORT
    if (uhs_en && !(mmc_host_is_spi(mmc)) && (cmd.response[0] & 0x41000000) == 0x41000000) {
        err = mmc_switch_voltage(mmc, MMC_SIGNAL_VOLTAGE_180);
        if (err)
            return err;
    }
#endif

    mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
    mmc->rca = 0;

    return HAL_SDMMC_ERR_NONE;
}

static int mmc_send_op_cond_iter(struct mmc *mmc, int use_arg)
{
    struct mmc_cmd cmd;
    int err;

    cmd.cmdidx = MMC_CMD_SEND_OP_COND;
    cmd.resp_type = MMC_RSP_R3;
    cmd.cmdarg = 0;
    if (use_arg && !mmc_host_is_spi(mmc))
        cmd.cmdarg = OCR_HCS |
                     (mmc->cfg->voltages &
                      (mmc->ocr & OCR_VOLTAGE_MASK)) |
                     (mmc->ocr & OCR_ACCESS_MODE);

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err)
        return err;
    mmc->ocr = cmd.response[0];
    return HAL_SDMMC_ERR_NONE;
}

static int mmc_send_op_cond(struct mmc *mmc)
{
    int err, i;
    uint32_t timeout = MS_TO_TICKS(1000);
    uint32_t start;

    /* Some cards seem to need this */
    mmc_go_idle(mmc);

    start = hal_sys_timer_get();
    /* Asking to the card its capabilities */
    for (i = 0; ; i++) {
        err = mmc_send_op_cond_iter(mmc, i != 0);
        if (err)
            return err;

        /* exit if not busy (flag seems to be inverted) */
        if (mmc->ocr & OCR_BUSY)
            break;

        if (hal_sys_timer_get() > (start + timeout))
            return HAL_SDMMC_COMM_TIMEOUT;
        mmc_udelay(100);
    }
    mmc->op_cond_pending = 1;
    return HAL_SDMMC_ERR_NONE;
}

static int mmc_complete_op_cond(struct mmc *mmc)
{
    struct mmc_cmd cmd;
    uint32_t timeout = MS_TO_TICKS(1000);
    uint32_t start;
    int err;

    mmc->op_cond_pending = 0;
    if (!(mmc->ocr & OCR_BUSY)) {
        /* Some cards seem to need this */
        mmc_go_idle(mmc);

        start = hal_sys_timer_get();
        while (1) {
            err = mmc_send_op_cond_iter(mmc, 1);
            if (err)
                return err;
            if (mmc->ocr & OCR_BUSY)
                break;
            if (hal_sys_timer_get() > (start + timeout))
                return HAL_SDMMC_OP_NOT_SUPPORTED_EP;
            mmc_udelay(100);
        }
    }

    if (mmc_host_is_spi(mmc)) { /* read OCR for spi */
        cmd.cmdidx = MMC_CMD_SPI_READ_OCR;
        cmd.resp_type = MMC_RSP_R3;
        cmd.cmdarg = 0;

        err = mmc_send_cmd(mmc, &cmd, NULL);

        if (err)
            return err;

        mmc->ocr = cmd.response[0];
    }

    mmc->version = MMC_VERSION_UNKNOWN;

    mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
    mmc->rca = 1;

    return HAL_SDMMC_ERR_NONE;
}

static int mmc_send_ext_csd(struct mmc *mmc, uint8_t *ext_csd)
{
    struct mmc_cmd cmd;
    struct mmc_data data;
    int err;

    /* Get the Card Status Register */
    cmd.cmdidx = MMC_CMD_SEND_EXT_CSD;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = 0;

    data.dest = (char *)ext_csd;
    data.blocks = 1;
    data.blocksize = MMC_MAX_BLOCK_LEN;
    data.flags = MMC_DATA_READ;

    err = mmc_send_cmd(mmc, &cmd, &data);

    return err;
}

static int __mmc_switch(struct mmc *mmc, uint8_t set, uint8_t index, uint8_t value,
                        bool send_status)
{
    uint32_t status, start;
    struct mmc_cmd cmd;
    int timeout_ms = DEFAULT_CMD6_TIMEOUT_MS;
    bool is_part_switch = (set == EXT_CSD_CMD_SET_NORMAL) &&
                          (index == EXT_CSD_PART_CONF);
    int ret;

    if (mmc->gen_cmd6_time)
        timeout_ms = mmc->gen_cmd6_time * 10;

    if (is_part_switch  && mmc->part_switch_time)
        timeout_ms = mmc->part_switch_time * 10;

    cmd.cmdidx = MMC_CMD_SWITCH;
    cmd.resp_type = MMC_RSP_R1b;
    cmd.cmdarg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
                 (index << 16) |
                 (value << 8);

    ret = mmc_send_cmd_retry(mmc, &cmd, NULL, 3);
    if (ret)
        return ret;

    start = hal_sys_timer_get();

    /* poll dat0 for rdy/buys status */
    ret = mmc_wait_dat0(mmc, 1, timeout_ms * 1000);
    if (ret && ret != HAL_SDMMC_INVALID_SYS_CALL)
        return ret;

    /*
     * In cases when neiter allowed to poll by using CMD13 nor we are
     * capable of polling by using mmc_wait_dat0, then rely on waiting the
     * stated timeout to be sufficient.
     */
    if (ret == HAL_SDMMC_INVALID_SYS_CALL && !send_status) {
        hal_sdmmc_delay(timeout_ms);
        return HAL_SDMMC_ERR_NONE;
    }

    /* Finally wait until the card is ready or indicates a failure
     * to switch. It doesn't hurt to use CMD13 here even if send_status
     * is false, because by now (after 'timeout_ms' ms) the bus should be
     * reliable.
     */
    do {
        ret = mmc_send_status(mmc, &status);

        if (!ret && (status & MMC_STATUS_SWITCH_ERROR)) {
            HAL_SDMMC_TRACE(0, "switch failed %d/%d/0x%x !", set, index, value);
            return HAL_SDMMC_IO_ERR;
        }
        if (!ret && (status & MMC_STATUS_RDY_FOR_DATA) &&
            (status & MMC_STATUS_CURR_STATE) == MMC_STATE_TRANS)
            return HAL_SDMMC_ERR_NONE;
        mmc_udelay(100);
    } while (hal_sys_timer_get() < start + timeout_ms);

    return HAL_SDMMC_COMM_TIMEOUT;
}

static int mmc_switch(struct mmc *mmc, uint8_t set, uint8_t index, uint8_t value)
{
    return __mmc_switch(mmc, set, index, value, true);
}

int mmc_boot_wp(struct mmc *mmc)
{
    return mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_BOOT_WP, 1);
}

#ifndef MMC_TINY
static int mmc_set_card_speed(struct mmc *mmc, enum bus_mode mode,
                              bool hsdowngrade)
{
    int err;
    uint32_t speed_bits;

    ALLOC_CACHE_ALIGN_BUFFER(uint8_t, test_csd, MMC_MAX_BLOCK_LEN);

    switch (mode) {
        case MMC_HS:
        case MMC_HS_52:
        case MMC_DDR_52:
            speed_bits = EXT_CSD_TIMING_HS;
            break;
#ifdef MMC_HS200_SUPPORT
        case MMC_HS_200:
            speed_bits = EXT_CSD_TIMING_HS200;
            break;
#endif
#ifdef MMC_HS400_SUPPORT
        case MMC_HS_400:
            speed_bits = EXT_CSD_TIMING_HS400;
            break;
#endif
#ifdef MMC_HS400_ES_SUPPORT
        case MMC_HS_400_ES:
            speed_bits = EXT_CSD_TIMING_HS400;
            break;
#endif
        case MMC_LEGACY:
            speed_bits = EXT_CSD_TIMING_LEGACY;
            break;
        default:
            return HAL_SDMMC_INVALID_PARAMETER;
    }

    err = __mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING,
                       speed_bits, !hsdowngrade);
    if (err)
        return err;

#if defined(MMC_HS200_SUPPORT) || \
    defined(MMC_HS400_SUPPORT)
    /*
     * In case the eMMC is in HS200/HS400 mode and we are downgrading
     * to HS mode, the card clock are still running much faster than
     * the supported HS mode clock, so we can not reliably read out
     * Extended CSD. Reconfigure the controller to run at HS mode.
     */
    if (hsdowngrade) {
        mmc_select_mode(mmc, MMC_HS);
        mmc_set_clock(mmc, mmc_mode2freq(mmc, MMC_HS), false);
    }
#endif

    if ((mode == MMC_HS) || (mode == MMC_HS_52)) {
        /* Now check to see that it worked */
        err = mmc_send_ext_csd(mmc, test_csd);
        if (err)
            return err;

        /* No high-speed support */
        if (!test_csd[EXT_CSD_HS_TIMING])
            return HAL_SDMMC_OP_NOT_SUPPORTED;
    }

    return HAL_SDMMC_ERR_NONE;
}

static int mmc_get_capabilities(struct mmc *mmc)
{
    uint8_t *ext_csd = mmc->ext_csd;
    char cardtype;

    mmc->card_caps = MMC_MODE_1BIT | MMC_CAP(MMC_LEGACY);

    if (mmc_host_is_spi(mmc))
        return HAL_SDMMC_ERR_NONE;

    /* Only version 4 supports high-speed */
    if (mmc->version < MMC_VERSION_4)
        return HAL_SDMMC_ERR_NONE;

    if (!ext_csd) {
        HAL_SDMMC_TRACE(0, "No ext_csd found!"); /* this should enver happen */
        return HAL_SDMMC_OP_NOT_SUPPORTED;
    }

    mmc->card_caps |= MMC_MODE_4BIT | MMC_MODE_8BIT;

    cardtype = ext_csd[EXT_CSD_CARD_TYPE];
    mmc->cardtype = cardtype;

#ifdef MMC_HS200_SUPPORT
    if (cardtype & (EXT_CSD_CARD_TYPE_HS200_1_2V |
                    EXT_CSD_CARD_TYPE_HS200_1_8V)) {
        mmc->card_caps |= MMC_MODE_HS200;
    }
#endif
#if defined(MMC_HS400_SUPPORT) || \
    defined(MMC_HS400_ES_SUPPORT)
    if (cardtype & (EXT_CSD_CARD_TYPE_HS400_1_2V |
                    EXT_CSD_CARD_TYPE_HS400_1_8V)) {
        mmc->card_caps |= MMC_MODE_HS400;
    }
#endif
    if (cardtype & EXT_CSD_CARD_TYPE_52) {
        if (cardtype & EXT_CSD_CARD_TYPE_DDR_52)
            mmc->card_caps |= MMC_MODE_DDR_52MHz;
        mmc->card_caps |= MMC_MODE_HS_52MHz;
    }
    if (cardtype & EXT_CSD_CARD_TYPE_26)
        mmc->card_caps |= MMC_MODE_HS;

#ifdef MMC_HS400_ES_SUPPORT
    if (ext_csd[EXT_CSD_STROBE_SUPPORT] &&
        (mmc->card_caps & MMC_MODE_HS400)) {
        mmc->card_caps |= MMC_MODE_HS400_ES;
    }
#endif

    return HAL_SDMMC_ERR_NONE;
}
#endif

static int mmc_set_capacity(struct mmc *mmc, int part_num)
{
    switch (part_num) {
        case 0:
            mmc->capacity = mmc->capacity_user;
            break;
        case 1:
        case 2:
            mmc->capacity = mmc->capacity_boot;
            break;
        case 3:
            mmc->capacity = mmc->capacity_rpmb;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            mmc->capacity = mmc->capacity_gp[part_num - 4];
            break;
        default:
            return HAL_SDMMC_INVALID_PARAMETER;
    }

    mmc_get_blk_desc(mmc)->lba = sdmmc_lldiv(mmc->capacity, mmc->read_bl_len);

    return HAL_SDMMC_ERR_NONE;
}

static int mmc_switch_part(struct mmc *mmc, uint32_t part_num)
{
    int ret;
    int retry = 3;

    do {
        ret = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
                         EXT_CSD_PART_CONF,
                         (mmc->part_config & ~PART_ACCESS_MASK)
                         | (part_num & PART_ACCESS_MASK));
    } while (ret && retry--);

    /*
     * Set the capacity if the switch succeeded or was intended
     * to return to representing the raw device.
     */
    if ((ret == 0) || ((ret == HAL_SDMMC_NO_SUCH_DEVICE) && (part_num == 0))) {
        ret = mmc_set_capacity(mmc, part_num);
        mmc_get_blk_desc(mmc)->hwpart = part_num;
    }

    return ret;
}

#ifdef MMC_HW_PARTITIONING
int mmc_hwpart_config(struct mmc *mmc,
                      const struct mmc_hwpart_conf *conf,
                      enum mmc_hwpart_conf_mode mode)
{
    uint8_t part_attrs = 0;
    uint32_t enh_size_mult;
    uint32_t enh_start_addr;
    uint32_t gp_size_mult[4];
    uint32_t max_enh_size_mult;
    uint32_t tot_enh_size_mult = 0;
    uint8_t wr_rel_set;
    int i, pidx, err;
    ALLOC_CACHE_ALIGN_BUFFER(uint8_t, ext_csd, MMC_MAX_BLOCK_LEN);

    if (mode < MMC_HWPART_CONF_CHECK || mode > MMC_HWPART_CONF_COMPLETE)
        return HAL_SDMMC_INVALID_PARAMETER;

    if (IS_SD(mmc) || (mmc->version < MMC_VERSION_4_41)) {
        HAL_SDMMC_TRACE(0, "eMMC >= 4.4 required for enhanced user data area");
        return HAL_SDMMC_MEDIUM_TYPE_ERR;
    }

    if (!(mmc->part_support & PART_SUPPORT)) {
        HAL_SDMMC_TRACE(0, "Card does not support partitioning");
        return HAL_SDMMC_MEDIUM_TYPE_ERR;
    }

    if (!mmc->hc_wp_grp_size) {
        HAL_SDMMC_TRACE(0, "Card does not define HC WP group size");
        return HAL_SDMMC_MEDIUM_TYPE_ERR;
    }

    /* check partition alignment and total enhanced size */
    if (conf->user.enh_size) {
        if (conf->user.enh_size % mmc->hc_wp_grp_size ||
            conf->user.enh_start % mmc->hc_wp_grp_size) {
            HAL_SDMMC_TRACE(0, "User data enhanced area not HC WP group size aligned");
            return HAL_SDMMC_INVALID_PARAMETER;
        }
        part_attrs |= EXT_CSD_ENH_USR;
        enh_size_mult = conf->user.enh_size / mmc->hc_wp_grp_size;
        if (mmc->high_capacity) {
            enh_start_addr = conf->user.enh_start;
        } else {
            enh_start_addr = (conf->user.enh_start << 9);
        }
    } else {
        enh_size_mult = 0;
        enh_start_addr = 0;
    }
    tot_enh_size_mult += enh_size_mult;

    for (pidx = 0; pidx < 4; pidx++) {
        if (conf->gp_part[pidx].size % mmc->hc_wp_grp_size) {
            HAL_SDMMC_TRACE(0, "GP%i partition not HC WP group size "
                            "aligned", pidx + 1);
            return HAL_SDMMC_INVALID_PARAMETER;
        }
        gp_size_mult[pidx] = conf->gp_part[pidx].size / mmc->hc_wp_grp_size;
        if (conf->gp_part[pidx].size && conf->gp_part[pidx].enhanced) {
            part_attrs |= EXT_CSD_ENH_GP(pidx);
            tot_enh_size_mult += gp_size_mult[pidx];
        }
    }

    if (part_attrs && !(mmc->part_support & ENHNCD_SUPPORT)) {
        HAL_SDMMC_TRACE(0, "Card does not support enhanced attribute");
        return HAL_SDMMC_MEDIUM_TYPE_ERR;
    }

    err = mmc_send_ext_csd(mmc, ext_csd);
    if (err)
        return err;

    max_enh_size_mult =
        (ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT + 2] << 16) +
        (ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT + 1] << 8) +
        ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT];
    if (tot_enh_size_mult > max_enh_size_mult) {
        HAL_SDMMC_TRACE(0, "Total enhanced size exceeds maximum (%u > %u)",
                        tot_enh_size_mult, max_enh_size_mult);
        return HAL_SDMMC_MEDIUM_TYPE_ERR;
    }

    /* The default value of EXT_CSD_WR_REL_SET is device
     * dependent, the values can only be changed if the
     * EXT_CSD_HS_CTRL_REL bit is set. The values can be
     * changed only once and before partitioning is completed. */
    wr_rel_set = ext_csd[EXT_CSD_WR_REL_SET];
    if (conf->user.wr_rel_change) {
        if (conf->user.wr_rel_set)
            wr_rel_set |= EXT_CSD_WR_DATA_REL_USR;
        else
            wr_rel_set &= ~EXT_CSD_WR_DATA_REL_USR;
    }
    for (pidx = 0; pidx < 4; pidx++) {
        if (conf->gp_part[pidx].wr_rel_change) {
            if (conf->gp_part[pidx].wr_rel_set)
                wr_rel_set |= EXT_CSD_WR_DATA_REL_GP(pidx);
            else
                wr_rel_set &= ~EXT_CSD_WR_DATA_REL_GP(pidx);
        }
    }

    if (wr_rel_set != ext_csd[EXT_CSD_WR_REL_SET] &&
        !(ext_csd[EXT_CSD_WR_REL_PARAM] & EXT_CSD_HS_CTRL_REL)) {
        HAL_SDMMC_TRACE(0, "Card does not support host controlled partition write "
                        "reliability settings");
        return HAL_SDMMC_MEDIUM_TYPE_ERR;
    }

    if (ext_csd[EXT_CSD_PARTITION_SETTING] &
        EXT_CSD_PARTITION_SETTING_COMPLETED) {
        HAL_SDMMC_TRACE(0, "Card already partitioned");
        return HAL_SDMMC_OP_NOT_PERMITTED;
    }

    if (mode == MMC_HWPART_CONF_CHECK)
        return HAL_SDMMC_ERR_NONE;

    /* Partitioning requires high-capacity size definitions */
    if (!(ext_csd[EXT_CSD_ERASE_GROUP_DEF] & 0x01)) {
        err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
                         EXT_CSD_ERASE_GROUP_DEF, 1);

        if (err)
            return err;

        ext_csd[EXT_CSD_ERASE_GROUP_DEF] = 1;

#ifdef MMC_WRITE
        /* update erase group size to be high-capacity */
        mmc->erase_grp_size =
            ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 1024;
#endif
    }

    /* all OK, write the configuration */
    for (i = 0; i < 4; i++) {
        err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
                         EXT_CSD_ENH_START_ADDR + i,
                         (enh_start_addr >> (i * 8)) & 0xFF);
        if (err)
            return err;
    }
    for (i = 0; i < 3; i++) {
        err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
                         EXT_CSD_ENH_SIZE_MULT + i,
                         (enh_size_mult >> (i * 8)) & 0xFF);
        if (err)
            return err;
    }
    for (pidx = 0; pidx < 4; pidx++) {
        for (i = 0; i < 3; i++) {
            err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
                             EXT_CSD_GP_SIZE_MULT + pidx * 3 + i,
                             (gp_size_mult[pidx] >> (i * 8)) & 0xFF);
            if (err)
                return err;
        }
    }
    err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
                     EXT_CSD_PARTITIONS_ATTRIBUTE, part_attrs);
    if (err)
        return err;

    if (mode == MMC_HWPART_CONF_SET)
        return HAL_SDMMC_ERR_NONE;

    /* The WR_REL_SET is a write-once register but shall be
     * written before setting PART_SETTING_COMPLETED. As it is
     * write-once we can only write it when completing the
     * partitioning. */
    if (wr_rel_set != ext_csd[EXT_CSD_WR_REL_SET]) {
        err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
                         EXT_CSD_WR_REL_SET, wr_rel_set);
        if (err)
            return err;
    }

    /* Setting PART_SETTING_COMPLETED confirms the partition
     * configuration but it only becomes effective after power
     * cycle, so we do not adjust the partition related settings
     * in the mmc struct. */
    err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
                     EXT_CSD_PARTITION_SETTING,
                     EXT_CSD_PARTITION_SETTING_COMPLETED);
    if (err)
        return err;

    return HAL_SDMMC_ERR_NONE;
}
#endif

#ifndef DM_MMC
static int mmc_getcd(struct mmc *mmc)
{
    int cd;

    if (mmc->cfg->ops->getcd)
        cd = mmc->cfg->ops->getcd(mmc);
    else
        cd = 1;

    return cd;
}
#endif

#ifndef MMC_TINY
static int sd_switch(struct mmc *mmc, int mode, int group, uint8_t value, uint8_t *resp)
{
    struct mmc_cmd cmd;
    struct mmc_data data;

    /* Switch the frequency */
    cmd.cmdidx = SD_CMD_SWITCH_FUNC;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = (mode << 31) | 0xffffff;
    cmd.cmdarg &= ~(0xf << (group * 4));
    cmd.cmdarg |= value << (group * 4);

    data.dest = (char *)resp;
    data.blocksize = 64;
    data.blocks = 1;
    data.flags = MMC_DATA_READ;

    return mmc_send_cmd(mmc, &cmd, &data);
}

static int sd_get_capabilities(struct mmc *mmc)
{
    int err;
    struct mmc_cmd cmd;
    ALLOC_CACHE_ALIGN_BUFFER(uint32_t, scr, 2);
    ALLOC_CACHE_ALIGN_BUFFER(uint32_t, switch_status, 16);
    struct mmc_data data;
    int timeout;
#ifdef MMC_UHS_SUPPORT
    uint32_t sd3_bus_mode;
#endif

    mmc->card_caps = MMC_MODE_1BIT | MMC_CAP(MMC_LEGACY);

    if (mmc_host_is_spi(mmc))
        return HAL_SDMMC_ERR_NONE;

    /* Read the SCR to find out if this card supports higher speeds */
    cmd.cmdidx = MMC_CMD_APP_CMD;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = mmc->rca << 16;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err)
        return err;

    cmd.cmdidx = SD_CMD_APP_SEND_SCR;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = 0;

    data.dest = (char *)scr;
    data.blocksize = 8;
    data.blocks = 1;
    data.flags = MMC_DATA_READ;

    err = mmc_send_cmd_retry(mmc, &cmd, &data, 3);

    if (err)
        return err;

    mmc->scr[0] = be32_to_cpu(scr[0]);
    mmc->scr[1] = be32_to_cpu(scr[1]);

    switch ((mmc->scr[0] >> 24) & 0xf) {
        case 0:
            mmc->version = SD_VERSION_1_0;
            break;
        case 1:
            mmc->version = SD_VERSION_1_10;
            break;
        case 2:
            mmc->version = SD_VERSION_2;
            if ((mmc->scr[0] >> 15) & 0x1)
                mmc->version = SD_VERSION_3;
            break;
        default:
            mmc->version = SD_VERSION_1_0;
            break;
    }

    if (mmc->scr[0] & SD_DATA_4BIT)
        mmc->card_caps |= MMC_MODE_4BIT;

    /* Version 1.0 doesn't support switching */
    if (mmc->version == SD_VERSION_1_0)
        return HAL_SDMMC_ERR_NONE;

    timeout = 4;
    while (timeout--) {
        err = sd_switch(mmc, SD_SWITCH_CHECK, 0, 1,
                        (uint8_t *)switch_status);

        if (err)
            return err;

        /* The high-speed function is busy.  Try again */
        if (!(be32_to_cpu(switch_status[7]) & SD_HIGHSPEED_BUSY))
            break;
    }

    /* If high-speed isn't supported, we return */
    if (be32_to_cpu(switch_status[3]) & SD_HIGHSPEED_SUPPORTED)
        mmc->card_caps |= MMC_CAP(SD_HS);

#ifdef MMC_UHS_SUPPORT
    /* Version before 3.0 don't support UHS modes */
    if (mmc->version < SD_VERSION_3)
        return HAL_SDMMC_ERR_NONE;

    sd3_bus_mode = be32_to_cpu(switch_status[3]) >> 16 & 0x1f;
    if (sd3_bus_mode & SD_MODE_UHS_SDR104)
        mmc->card_caps |= MMC_CAP(UHS_SDR104);
    if (sd3_bus_mode & SD_MODE_UHS_SDR50)
        mmc->card_caps |= MMC_CAP(UHS_SDR50);
    if (sd3_bus_mode & SD_MODE_UHS_SDR25)
        mmc->card_caps |= MMC_CAP(UHS_SDR25);
    if (sd3_bus_mode & SD_MODE_UHS_SDR12)
        mmc->card_caps |= MMC_CAP(UHS_SDR12);
    if (sd3_bus_mode & SD_MODE_UHS_DDR50)
        mmc->card_caps |= MMC_CAP(UHS_DDR50);
#endif

    return HAL_SDMMC_ERR_NONE;
}

static int sd_set_card_speed(struct mmc *mmc, enum bus_mode mode)
{
    int err;

    ALLOC_CACHE_ALIGN_BUFFER(uint32_t, switch_status, 16);
    int speed;

    /* SD version 1.00 and 1.01 does not support CMD 6 */
    if (mmc->version == SD_VERSION_1_0)
        return HAL_SDMMC_ERR_NONE;

    switch (mode) {
        case MMC_LEGACY:
            speed = UHS_SDR12_BUS_SPEED;
            break;
        case SD_HS:
            speed = HIGH_SPEED_BUS_SPEED;
            break;
#ifdef MMC_UHS_SUPPORT
        case UHS_SDR12:
            speed = UHS_SDR12_BUS_SPEED;
            break;
        case UHS_SDR25:
            speed = UHS_SDR25_BUS_SPEED;
            break;
        case UHS_SDR50:
            speed = UHS_SDR50_BUS_SPEED;
            break;
        case UHS_DDR50:
            speed = UHS_DDR50_BUS_SPEED;
            break;
        case UHS_SDR104:
            speed = UHS_SDR104_BUS_SPEED;
            break;
#endif
        default:
            return HAL_SDMMC_INVALID_PARAMETER;
    }

    err = sd_switch(mmc, SD_SWITCH_SWITCH, 0, speed, (uint8_t *)switch_status);
    if (err)
        return err;

    if (((be32_to_cpu(switch_status[4]) >> 24) & 0xF) != speed)
        return HAL_SDMMC_OP_NOT_SUPPORTED;

    return HAL_SDMMC_ERR_NONE;
}

static int sd_select_bus_width(struct mmc *mmc, int w)
{
    int err;
    struct mmc_cmd cmd;

    if ((w != 4) && (w != 1))
        return HAL_SDMMC_INVALID_PARAMETER;

    cmd.cmdidx = MMC_CMD_APP_CMD;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = mmc->rca << 16;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err)
        return err;

    cmd.cmdidx = SD_CMD_APP_SET_BUS_WIDTH;
    cmd.resp_type = MMC_RSP_R1;
    if (w == 4)
        cmd.cmdarg = 2;
    else if (w == 1)
        cmd.cmdarg = 0;
    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err)
        return err;

    return HAL_SDMMC_ERR_NONE;
}
#endif

#ifdef MMC_WRITE
#define SZ_1                0x00000001
#define SZ_2                0x00000002
#define SZ_4                0x00000004
#define SZ_8                0x00000008
#define SZ_16               0x00000010
#define SZ_32               0x00000020
#define SZ_64               0x00000040
#define SZ_128              0x00000080
#define SZ_256              0x00000100
#define SZ_512              0x00000200

#define SZ_1K               0x00000400
#define SZ_2K               0x00000800
#define SZ_4K               0x00001000
#define SZ_8K               0x00002000
#define SZ_16K              0x00004000
#define SZ_32K              0x00008000
#define SZ_64K              0x00010000
#define SZ_128K             0x00020000
#define SZ_256K             0x00040000
#define SZ_512K             0x00080000

#define SZ_1M               0x00100000
#define SZ_2M               0x00200000
#define SZ_4M               0x00400000
#define SZ_8M               0x00800000
#define SZ_16M              0x01000000
#define SZ_32M              0x02000000
#define SZ_64M              0x04000000
#define SZ_128M             0x08000000
#define SZ_256M             0x10000000
#define SZ_512M             0x20000000

#define SZ_1G               0x40000000
#define SZ_2G               0x80000000

static int sd_read_ssr(struct mmc *mmc)
{
    static const uint32_t sd_au_size[] = {
        0,              SZ_16K / 512,           SZ_32K / 512,
        SZ_64K / 512,   SZ_128K / 512,          SZ_256K / 512,
        SZ_512K / 512,  SZ_1M / 512,            SZ_2M / 512,
        SZ_4M / 512,    SZ_8M / 512,            (SZ_8M + SZ_4M) / 512,
        SZ_16M / 512,   (SZ_16M + SZ_8M) / 512, SZ_32M / 512,
        SZ_64M / 512,
    };
    int err, i;
    struct mmc_cmd cmd;
    ALLOC_CACHE_ALIGN_BUFFER(uint32_t, ssr, 16);
    struct mmc_data data;
    uint32_t au, eo, et, es;

    cmd.cmdidx = MMC_CMD_APP_CMD;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = mmc->rca << 16;

    err = mmc_send_cmd_quirks(mmc, &cmd, NULL, MMC_QUIRK_RETRY_APP_CMD, 4);
    if (err)
        return err;

    cmd.cmdidx = SD_CMD_APP_SD_STATUS;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = 0;

    data.dest = (char *)ssr;
    data.blocksize = 64;
    data.blocks = 1;
    data.flags = MMC_DATA_READ;

    err = mmc_send_cmd_retry(mmc, &cmd, &data, 3);
    if (err)
        return err;

    for (i = 0; i < 16; i++)
        ssr[i] = be32_to_cpu(ssr[i]);

    au = (ssr[2] >> 12) & 0xF;
    if ((au <= 9) || (mmc->version == SD_VERSION_3)) {
        mmc->ssr.au = sd_au_size[au];
        es = (ssr[3] >> 24) & 0xFF;
        es |= (ssr[2] & 0xFF) << 8;
        et = (ssr[3] >> 18) & 0x3F;
        if (es && et) {
            eo = (ssr[3] >> 16) & 0x3;
            mmc->ssr.erase_timeout = (et * 1000) / es;
            mmc->ssr.erase_offset = eo * 1000;
        }
    } else {
        HAL_SDMMC_TRACE(0, "Invalid Allocation Unit Size.");
    }

    return HAL_SDMMC_ERR_NONE;
}
#endif

/* frequency bases */
/* divided by 10 to be nice to platforms without floating point */
static const int fbase[] = {
    10000,
    100000,
    1000000,
    10000000,
};

/* Multiplier values for TRAN_SPEED.  Multiplied by 10 to be nice
 * to platforms without floating point.
 */
static const uint8_t multipliers[] = {
    0,  /* reserved */
    10,
    12,
    13,
    15,
    20,
    25,
    30,
    35,
    40,
    45,
    50,
    55,
    60,
    70,
    80,
};

static int bus_width(uint32_t cap)
{
    if (cap == MMC_MODE_8BIT)
        return 8;
    if (cap == MMC_MODE_4BIT)
        return 4;
    if (cap == MMC_MODE_1BIT)
        return 1;
    HAL_SDMMC_TRACE(0, "invalid bus witdh capability 0x%x", cap);
    return 0;
}

#ifndef DM_MMC
#ifdef MMC_SUPPORTS_TUNING
static int mmc_execute_tuning(struct mmc *mmc, uint32_t opcode)
{
    return HAL_SDMMC_OP_NOT_SUPPORTED;
}
#endif

static int mmc_set_ios(struct mmc *mmc)
{
    int ret = 0;

    if (mmc->cfg->ops->set_ios)
        ret = mmc->cfg->ops->set_ios(mmc);

    return ret;
}

static int mmc_host_power_cycle(struct mmc *mmc)
{
    int ret = 0;

    if (mmc->cfg->ops->host_power_cycle)
        ret = mmc->cfg->ops->host_power_cycle(mmc);

    return ret;
}
#endif

static int mmc_set_clock(struct mmc *mmc, uint32_t clock, bool disable)
{
    if (!disable) {
        if (clock > mmc->cfg->f_max)
            clock = mmc->cfg->f_max;

        if (clock < mmc->cfg->f_min)
            clock = mmc->cfg->f_min;
    }

    mmc->clock = clock;
    mmc->clk_disable = disable;
    HAL_SDMMC_TRACE(0, "%s, clock is %s (%dHz)", __FUNCTION__, disable ? "disabled" : "enabled", clock);

    return mmc_set_ios(mmc);
}

static int mmc_set_bus_width(struct mmc *mmc, uint32_t width)
{
    mmc->bus_width = width;
    HAL_SDMMC_TRACE(0, "%s, width is %d", __FUNCTION__, width);

    return mmc_set_ios(mmc);
}

#ifdef SDMMC_DEBUG
static void mmc_dump_capabilities(const char *text, uint32_t caps)
{
    enum bus_mode mode;

    HAL_SDMMC_TRACE(0, "  ");
    HAL_SDMMC_TRACE(TR_ATTR_NO_LF, "%s: widths [", text);
    if (caps & MMC_MODE_8BIT)
        HAL_SDMMC_TRACE(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, "8, ");
    if (caps & MMC_MODE_4BIT)
        HAL_SDMMC_TRACE(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, "4, ");
    if (caps & MMC_MODE_1BIT)
        HAL_SDMMC_TRACE(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, "1, ");
    HAL_SDMMC_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "]");
    HAL_SDMMC_TRACE(0, "modes [");
    for (mode = MMC_LEGACY; mode < MMC_MODES_END; mode++)
        if (MMC_CAP(mode) & caps)
            HAL_SDMMC_TRACE(0, "        %s, ", mmc_mode_name(mode));
    HAL_SDMMC_TRACE(0, "      ]");
}
#endif

#ifdef MMC_IO_VOLTAGE
int mmc_voltage_to_mv(enum mmc_voltage voltage)
{
    switch (voltage) {
        case MMC_SIGNAL_VOLTAGE_000:
            return 0;
        case MMC_SIGNAL_VOLTAGE_330:
            return 3300;
        case MMC_SIGNAL_VOLTAGE_180:
            return 1800;
        case MMC_SIGNAL_VOLTAGE_120:
            return 1200;
        default:
            return 0;
    }
    return HAL_SDMMC_INVALID_PARAMETER;
}

static int mmc_set_signal_voltage(struct mmc *mmc, uint32_t signal_voltage)
{
    int err;

    if (mmc->signal_voltage == signal_voltage)
        return HAL_SDMMC_ERR_NONE;

    mmc->signal_voltage = signal_voltage;
    HAL_SDMMC_TRACE(0, "%s, signal_voltage is %d", __FUNCTION__, signal_voltage);

    err = mmc_set_ios(mmc);
    if (err)
        HAL_SDMMC_TRACE(0, "unable to set voltage (err %d)", err);

    return err;
}
#else
static int mmc_set_signal_voltage(struct mmc *mmc, uint32_t signal_voltage)
{
    return HAL_SDMMC_ERR_NONE;
}
#endif

#ifndef MMC_TINY
struct mode_width_tuning {
    enum bus_mode mode;
    uint32_t widths;
#ifdef MMC_SUPPORTS_TUNING
    uint32_t tuning;
#endif
};

static const struct mode_width_tuning sd_modes_by_pref[] = {
#ifdef MMC_UHS_SUPPORT
#ifdef MMC_SUPPORTS_TUNING
    {
        .mode = UHS_SDR104,
        .widths = MMC_MODE_4BIT | MMC_MODE_1BIT,
        .tuning = MMC_CMD_SEND_TUNING_BLOCK
    },
#endif
    {
        .mode = UHS_SDR50,
        .widths = MMC_MODE_4BIT | MMC_MODE_1BIT,
    },
    {
        .mode = UHS_DDR50,
        .widths = MMC_MODE_4BIT | MMC_MODE_1BIT,
    },
    {
        .mode = UHS_SDR25,
        .widths = MMC_MODE_4BIT | MMC_MODE_1BIT,
    },
#endif
    {
        .mode = SD_HS,
        .widths = MMC_MODE_4BIT | MMC_MODE_1BIT,
    },
#ifdef MMC_UHS_SUPPORT
    {
        .mode = UHS_SDR12,
        .widths = MMC_MODE_4BIT | MMC_MODE_1BIT,
    },
#endif
    {
        .mode = MMC_LEGACY,
        .widths = MMC_MODE_4BIT | MMC_MODE_1BIT,
    }
};

#define for_each_sd_mode_by_pref(caps, mwt) \
    for (mwt = sd_modes_by_pref;\
         mwt < sd_modes_by_pref + ARRAY_SIZE(sd_modes_by_pref);\
         mwt++) \
        if (caps & MMC_CAP(mwt->mode))

static int sd_select_mode_and_width(struct mmc *mmc, uint32_t card_caps)
{
    int err;
    uint32_t widths[] = {MMC_MODE_4BIT, MMC_MODE_1BIT};
    const struct mode_width_tuning *mwt;
#ifdef MMC_UHS_SUPPORT
    bool uhs_en = (mmc->ocr & OCR_S18R) ? true : false;
#else
    bool uhs_en = false;
#endif
    uint32_t caps;

#ifdef SDMMC_DEBUG
    mmc_dump_capabilities("sd card", card_caps);
    mmc_dump_capabilities("host", mmc->host_caps);
#endif

    if (mmc_host_is_spi(mmc)) {
        mmc_set_bus_width(mmc, 1);
        mmc_select_mode(mmc, MMC_LEGACY);
        mmc_set_clock(mmc, mmc->tran_speed, MMC_CLK_ENABLE);
#ifdef MMC_WRITE
        err = sd_read_ssr(mmc);
        if (err)
            HAL_SDMMC_TRACE(0, "unable to read ssr");
#endif
        return HAL_SDMMC_ERR_NONE;
    }

    /* Restrict card's capabilities by what the host can do */
    caps = card_caps & mmc->host_caps;

    if (!uhs_en)
        caps &= ~UHS_CAPS;

    for_each_sd_mode_by_pref(caps, mwt) {
        uint32_t *w;

        for (w = widths; w < widths + ARRAY_SIZE(widths); w++) {
            if (*w & caps & mwt->widths) {
                HAL_SDMMC_TRACE(0, "trying mode %s width %d (at %d MHz)",
                                mmc_mode_name(mwt->mode),
                                bus_width(*w),
                                mmc_mode2freq(mmc, mwt->mode) / 1000000);

                /* configure the bus width (card + host) */
                err = sd_select_bus_width(mmc, bus_width(*w));
                if (err)
                    goto error;
                mmc_set_bus_width(mmc, bus_width(*w));

                /* configure the bus mode (card) */
                err = sd_set_card_speed(mmc, mwt->mode);
                if (err)
                    goto error;

                /* configure the bus mode (host) */
                mmc_select_mode(mmc, mwt->mode);
                mmc_set_clock(mmc, mmc->tran_speed,
                              MMC_CLK_ENABLE);

#ifdef MMC_SUPPORTS_TUNING
                /* execute tuning if needed */
                if (mwt->tuning && !mmc_host_is_spi(mmc)) {
                    err = mmc_execute_tuning(mmc,
                                             mwt->tuning);
                    if (err) {
                        HAL_SDMMC_TRACE(0, "tuning failed");
                        goto error;
                    }
                }
#endif

#ifdef MMC_WRITE
                err = sd_read_ssr(mmc);
                if (err)
                    HAL_SDMMC_TRACE(0, "unable to read ssr");
#endif
                if (!err)
                    return HAL_SDMMC_ERR_NONE;
error:
                /* revert to a safer bus speed */
                mmc_select_mode(mmc, MMC_LEGACY);
                mmc_set_clock(mmc, mmc->tran_speed,
                              MMC_CLK_ENABLE);
            }
        }
    }

    HAL_SDMMC_TRACE(0, "unable to select a mode");
    return HAL_SDMMC_OP_NOT_SUPPORTED;
}

static int mmc_read_and_compare_ext_csd(struct mmc *mmc)
{
    int err;
    const uint8_t *ext_csd = mmc->ext_csd;
    ALLOC_CACHE_ALIGN_BUFFER(uint8_t, test_csd, MMC_MAX_BLOCK_LEN);

    if (mmc->version < MMC_VERSION_4)
        return HAL_SDMMC_ERR_NONE;

    err = mmc_send_ext_csd(mmc, test_csd);
    if (err)
        return err;

    /* Only compare read only fields */
    if (ext_csd[EXT_CSD_PARTITIONING_SUPPORT]
        == test_csd[EXT_CSD_PARTITIONING_SUPPORT] &&
        ext_csd[EXT_CSD_HC_WP_GRP_SIZE]
        == test_csd[EXT_CSD_HC_WP_GRP_SIZE] &&
        ext_csd[EXT_CSD_REV]
        == test_csd[EXT_CSD_REV] &&
        ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]
        == test_csd[EXT_CSD_HC_ERASE_GRP_SIZE] &&
        memcmp(&ext_csd[EXT_CSD_SEC_CNT],
               &test_csd[EXT_CSD_SEC_CNT], 4) == 0)
        return HAL_SDMMC_ERR_NONE;

    return HAL_SDMMC_NOT_DATA_MSG;
}

#ifdef MMC_IO_VOLTAGE
/*
 * ffs: find first bit set. This is defined the same way as
 * the libc and compiler builtin ffs routines, therefore
 * differs in spirit from the above ffz (man ffs).
 */
static inline int generic_ffs(int x)
{
    int r = 1;

    if (!x)
        return 0;
    if (!(x & 0xffff)) {
        x >>= 16;
        r += 16;
    }
    if (!(x & 0xff)) {
        x >>= 8;
        r += 8;
    }
    if (!(x & 0xf)) {
        x >>= 4;
        r += 4;
    }
    if (!(x & 3)) {
        x >>= 2;
        r += 2;
    }
    if (!(x & 1)) {
        x >>= 1;
        r += 1;
    }
    return r;
}

static int mmc_set_lowest_voltage(struct mmc *mmc, enum bus_mode mode,
                                  uint32_t allowed_mask)
{
    uint32_t card_mask = 0;

    HAL_SDMMC_TRACE(0, "%s:%d, bus_mode:%d, mask:0x%X", __func__, __LINE__, (uint32_t)mode, allowed_mask);
    switch (mode) {
        case MMC_HS_400_ES:
        case MMC_HS_400:
        case MMC_HS_200:
            if (mmc->cardtype & (EXT_CSD_CARD_TYPE_HS200_1_8V |
                                 EXT_CSD_CARD_TYPE_HS400_1_8V))
                card_mask |= MMC_SIGNAL_VOLTAGE_180;
            if (mmc->cardtype & (EXT_CSD_CARD_TYPE_HS200_1_2V |
                                 EXT_CSD_CARD_TYPE_HS400_1_2V))
                card_mask |= MMC_SIGNAL_VOLTAGE_120;
            break;
        case MMC_DDR_52:
            if (mmc->cardtype & EXT_CSD_CARD_TYPE_DDR_1_8V)
                card_mask |= MMC_SIGNAL_VOLTAGE_330 |
                             MMC_SIGNAL_VOLTAGE_180;
            if (mmc->cardtype & EXT_CSD_CARD_TYPE_DDR_1_2V)
                card_mask |= MMC_SIGNAL_VOLTAGE_120;
            break;
        default:
            card_mask |= MMC_SIGNAL_VOLTAGE_330;
            break;
    }

    while (card_mask & allowed_mask) {
        enum mmc_voltage best_match;

        best_match = 1 << (generic_ffs(card_mask & allowed_mask) - 1);
        HAL_SDMMC_TRACE(0, "best match:0x%X", best_match);
        if (!mmc_set_signal_voltage(mmc,  best_match))
            return HAL_SDMMC_ERR_NONE;

        allowed_mask &= ~best_match;
    }

    return HAL_SDMMC_OP_NOT_SUPPORTED;
}
#else
static int mmc_set_lowest_voltage(struct mmc *mmc, enum bus_mode mode,
                                  uint32_t allowed_mask)
{
    return HAL_SDMMC_ERR_NONE;
}
#endif

static const struct mode_width_tuning mmc_modes_by_pref[] = {
#ifdef MMC_HS400_ES_SUPPORT
    {
        .mode = MMC_HS_400_ES,
        .widths = MMC_MODE_8BIT,
    },
#endif
#ifdef MMC_HS400_SUPPORT
    {
        .mode = MMC_HS_400,
        .widths = MMC_MODE_8BIT,
        .tuning = MMC_CMD_SEND_TUNING_BLOCK_HS200
    },
#endif
#ifdef MMC_HS200_SUPPORT
    {
        .mode = MMC_HS_200,
        .widths = MMC_MODE_8BIT | MMC_MODE_4BIT,
        .tuning = MMC_CMD_SEND_TUNING_BLOCK_HS200
    },
#endif
    {
        .mode = MMC_DDR_52,
        .widths = MMC_MODE_8BIT | MMC_MODE_4BIT,
    },
    {
        .mode = MMC_HS_52,
        .widths = MMC_MODE_8BIT | MMC_MODE_4BIT | MMC_MODE_1BIT,
    },
    {
        .mode = MMC_HS,
        .widths = MMC_MODE_8BIT | MMC_MODE_4BIT | MMC_MODE_1BIT,
    },
    {
        .mode = MMC_LEGACY,
        .widths = MMC_MODE_8BIT | MMC_MODE_4BIT | MMC_MODE_1BIT,
    }
};

#define for_each_mmc_mode_by_pref(caps, mwt) \
    for (mwt = mmc_modes_by_pref;\
         mwt < mmc_modes_by_pref + ARRAY_SIZE(mmc_modes_by_pref);\
         mwt++) \
        if (caps & MMC_CAP(mwt->mode))

static const struct ext_csd_bus_width {
    uint32_t cap;
    bool is_ddr;
    uint32_t ext_csd_bits;
} ext_csd_bus_width[] = {
    {MMC_MODE_8BIT, true, EXT_CSD_DDR_BUS_WIDTH_8},
    {MMC_MODE_4BIT, true, EXT_CSD_DDR_BUS_WIDTH_4},
    {MMC_MODE_8BIT, false, EXT_CSD_BUS_WIDTH_8},
    {MMC_MODE_4BIT, false, EXT_CSD_BUS_WIDTH_4},
    {MMC_MODE_1BIT, false, EXT_CSD_BUS_WIDTH_1},
};

#ifdef MMC_HS400_SUPPORT
static int mmc_select_hs400(struct mmc *mmc)
{
    int err;

    /* Set timing to HS200 for tuning */
    err = mmc_set_card_speed(mmc, MMC_HS_200, false);
    if (err)
        return err;

    /* configure the bus mode (host) */
    mmc_select_mode(mmc, MMC_HS_200);
    mmc_set_clock(mmc, mmc->tran_speed, false);

    /* execute tuning if needed */
    mmc->hs400_tuning = 1;
    err = mmc_execute_tuning(mmc, MMC_CMD_SEND_TUNING_BLOCK_HS200);
    mmc->hs400_tuning = 0;
    if (err) {
        HAL_SDMMC_TRACE(0, "tuning failed");
        return err;
    }

    /* Set back to HS */
    mmc_set_card_speed(mmc, MMC_HS, true);

    err = mmc_hs400_prepare_ddr(mmc);
    if (err)
        return err;

    err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_BUS_WIDTH,
                     EXT_CSD_BUS_WIDTH_8 | EXT_CSD_DDR_FLAG);
    if (err)
        return err;

    err = mmc_set_card_speed(mmc, MMC_HS_400, false);
    if (err)
        return err;

    mmc_select_mode(mmc, MMC_HS_400);
    err = mmc_set_clock(mmc, mmc->tran_speed, false);
    if (err)
        return err;

    return HAL_SDMMC_ERR_NONE;
}
#else
static int mmc_select_hs400(struct mmc *mmc)
{
    return HAL_SDMMC_OP_NOT_SUPPORTED;
}
#endif

#ifdef MMC_HS400_ES_SUPPORT
#ifndef DM_MMC
static int mmc_set_enhanced_strobe(struct mmc *mmc)
{
    return HAL_SDMMC_OP_NOT_SUPPORTED;
}
#endif
static int mmc_select_hs400es(struct mmc *mmc)
{
    int err;

    err = mmc_set_card_speed(mmc, MMC_HS, true);
    if (err)
        return err;

    err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_BUS_WIDTH,
                     EXT_CSD_BUS_WIDTH_8 | EXT_CSD_DDR_FLAG |
                     EXT_CSD_BUS_WIDTH_STROBE);
    if (err) {
        HAL_SDMMC_TRACE(0, "switch to bus width for hs400 failed");
        return err;
    }
    /* TODO: driver strength */
    err = mmc_set_card_speed(mmc, MMC_HS_400_ES, false);
    if (err)
        return err;

    mmc_select_mode(mmc, MMC_HS_400_ES);
    err = mmc_set_clock(mmc, mmc->tran_speed, false);
    if (err)
        return err;

    return mmc_set_enhanced_strobe(mmc);
}
#else
static int mmc_select_hs400es(struct mmc *mmc)
{
    return HAL_SDMMC_OP_NOT_SUPPORTED;
}
#endif

#define for_each_supported_width(caps, ddr, ecbv) \
    for (ecbv = ext_csd_bus_width;\
         ecbv < ext_csd_bus_width + ARRAY_SIZE(ext_csd_bus_width);\
         ecbv++) \
        if ((ddr == ecbv->is_ddr) && (caps & ecbv->cap))

static int mmc_select_mode_and_width(struct mmc *mmc, uint32_t card_caps)
{
    int err = 0;
    const struct mode_width_tuning *mwt;
    const struct ext_csd_bus_width *ecbw;

#ifdef SDMMC_DEBUG
    mmc_dump_capabilities("mmc", card_caps);
    mmc_dump_capabilities("host", mmc->host_caps);
#endif

    if (mmc_host_is_spi(mmc)) {
        mmc_set_bus_width(mmc, 1);
        mmc_select_mode(mmc, MMC_LEGACY);
        mmc_set_clock(mmc, mmc->tran_speed, MMC_CLK_ENABLE);
        return HAL_SDMMC_ERR_NONE;
    }

    /* Restrict card's capabilities by what the host can do */
    card_caps &= mmc->host_caps;

    /* Only version 4 of MMC supports wider bus widths */
    if (mmc->version < MMC_VERSION_4)
        return HAL_SDMMC_ERR_NONE;

    if (!mmc->ext_csd) {
        HAL_SDMMC_TRACE(0, "No ext_csd found!"); /* this should enver happen */
        return HAL_SDMMC_OP_NOT_SUPPORTED;
    }

#if defined(MMC_HS200_SUPPORT) || \
    defined(MMC_HS400_SUPPORT) || \
    defined(MMC_HS400_ES_SUPPORT)
    /*
     * In case the eMMC is in HS200/HS400 mode, downgrade to HS mode
     * before doing anything else, since a transition from either of
     * the HS200/HS400 mode directly to legacy mode is not supported.
     */
    if (mmc->selected_mode == MMC_HS_200 ||
        mmc->selected_mode == MMC_HS_400 ||
        mmc->selected_mode == MMC_HS_400_ES)
        mmc_set_card_speed(mmc, MMC_HS, true);
    else
#endif
        mmc_set_clock(mmc, mmc->legacy_speed, MMC_CLK_ENABLE);

    for_each_mmc_mode_by_pref(card_caps, mwt) {
        for_each_supported_width(card_caps & mwt->widths,
                                 mmc_is_mode_ddr(mwt->mode), ecbw) {
            enum mmc_voltage old_voltage;
            HAL_SDMMC_TRACE(0, "trying mode %s width %d (at %d MHz)",
                            mmc_mode_name(mwt->mode),
                            bus_width(ecbw->cap),
                            mmc_mode2freq(mmc, mwt->mode) / 1000000);
            old_voltage = mmc->signal_voltage;
            err = mmc_set_lowest_voltage(mmc, mwt->mode,
                                         MMC_ALL_SIGNAL_VOLTAGE);
            if (err)
                continue;

            /* configure the bus width (card + host) */
            err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
                             EXT_CSD_BUS_WIDTH,
                             ecbw->ext_csd_bits & ~EXT_CSD_DDR_FLAG);
            if (err)
                goto error;
            mmc_set_bus_width(mmc, bus_width(ecbw->cap));

            if (mwt->mode == MMC_HS_400) {
                err = mmc_select_hs400(mmc);
                if (err) {
                    HAL_SDMMC_TRACE(0, "Select HS400 failed %d", err);
                    goto error;
                }
            } else if (mwt->mode == MMC_HS_400_ES) {
                err = mmc_select_hs400es(mmc);
                if (err) {
                    HAL_SDMMC_TRACE(0, "Select HS400ES failed %d",
                                    err);
                    goto error;
                }
            } else {
                /* configure the bus speed (card) */
                err = mmc_set_card_speed(mmc, mwt->mode, false);
                if (err)
                    goto error;

                /*
                 * configure the bus width AND the ddr mode
                 * (card). The host side will be taken care
                 * of in the next step
                 */
                if (ecbw->ext_csd_bits & EXT_CSD_DDR_FLAG) {
                    err = mmc_switch(mmc,
                                     EXT_CSD_CMD_SET_NORMAL,
                                     EXT_CSD_BUS_WIDTH,
                                     ecbw->ext_csd_bits);
                    if (err)
                        goto error;
                }

                /* configure the bus mode (host) */
                mmc_select_mode(mmc, mwt->mode);
                mmc_set_clock(mmc, mmc->tran_speed,
                              MMC_CLK_ENABLE);
#ifdef MMC_SUPPORTS_TUNING
                /* execute tuning if needed */
                if (mwt->tuning) {
                    err = mmc_execute_tuning(mmc,
                                             mwt->tuning);
                    if (err) {
                        HAL_SDMMC_TRACE(0, "tuning failed : %d", err);
                        goto error;
                    }
                }
#endif
            }

            /* do a transfer to check the configuration */
            err = mmc_read_and_compare_ext_csd(mmc);
            if (!err)
                return HAL_SDMMC_ERR_NONE;
error:
            mmc_set_signal_voltage(mmc, old_voltage);
            /* if an error occurred, revert to a safer bus mode */
            mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
                       EXT_CSD_BUS_WIDTH, EXT_CSD_BUS_WIDTH_1);
            mmc_select_mode(mmc, MMC_LEGACY);
            mmc_set_bus_width(mmc, 1);
        }
    }

    HAL_SDMMC_TRACE(0, "unable to select a mode : %d", err);

    return HAL_SDMMC_OP_NOT_SUPPORTED;
}
#endif

DEFINE_CACHE_ALIGN_BUFFER(uint8_t, ext_csd_bkup, MMC_MAX_BLOCK_LEN);
static int mmc_startup_v4(struct mmc *mmc)
{
    int err, i;
    uint64_t capacity;
    bool has_parts = false;
    bool part_completed;
    static const uint32_t mmc_versions[] = {
        MMC_VERSION_4,
        MMC_VERSION_4_1,
        MMC_VERSION_4_2,
        MMC_VERSION_4_3,
        MMC_VERSION_4_4,
        MMC_VERSION_4_41,
        MMC_VERSION_4_5,
        MMC_VERSION_5_0,
        MMC_VERSION_5_1
    };

    uint8_t *ext_csd = ext_csd_bkup;

    if (IS_SD(mmc) || mmc->version < MMC_VERSION_4)
        return HAL_SDMMC_ERR_NONE;

    if (!mmc->ext_csd)
        memset(ext_csd_bkup, 0, sizeof(*ext_csd_bkup));

    err = mmc_send_ext_csd(mmc, ext_csd);
    if (err)
        goto error;

    /* store the ext csd for future reference */
    if (!mmc->ext_csd)
        mmc->ext_csd = ext_csd;

    if (ext_csd[EXT_CSD_REV] >= ARRAY_SIZE(mmc_versions))
        return HAL_SDMMC_INVALID_PARAMETER;

    mmc->version = mmc_versions[ext_csd[EXT_CSD_REV]];

    if (mmc->version >= MMC_VERSION_4_2) {
        /*
         * According to the JEDEC Standard, the value of
         * ext_csd's capacity is valid if the value is more
         * than 2GB
         */
        capacity = ext_csd[EXT_CSD_SEC_CNT] << 0
                   | ext_csd[EXT_CSD_SEC_CNT + 1] << 8
                   | ext_csd[EXT_CSD_SEC_CNT + 2] << 16
                   | ext_csd[EXT_CSD_SEC_CNT + 3] << 24;
        capacity *= MMC_MAX_BLOCK_LEN;
        if ((capacity >> 20) > 2 * 1024)
            mmc->capacity_user = capacity;
    }

    if (mmc->version >= MMC_VERSION_4_5)
        mmc->gen_cmd6_time = ext_csd[EXT_CSD_GENERIC_CMD6_TIME];

    /* The partition data may be non-zero but it is only
     * effective if PARTITION_SETTING_COMPLETED is set in
     * EXT_CSD, so ignore any data if this bit is not set,
     * except for enabling the high-capacity group size
     * definition (see below).
     */
    part_completed = !!(ext_csd[EXT_CSD_PARTITION_SETTING] &
                        EXT_CSD_PARTITION_SETTING_COMPLETED);

    mmc->part_switch_time = ext_csd[EXT_CSD_PART_SWITCH_TIME];
    /* Some eMMC set the value too low so set a minimum */
    if (mmc->part_switch_time < MMC_MIN_PART_SWITCH_TIME && mmc->part_switch_time)
        mmc->part_switch_time = MMC_MIN_PART_SWITCH_TIME;

    /* store the partition info of emmc */
    mmc->part_support = ext_csd[EXT_CSD_PARTITIONING_SUPPORT];
    if ((ext_csd[EXT_CSD_PARTITIONING_SUPPORT] & PART_SUPPORT) ||
        ext_csd[EXT_CSD_BOOT_MULT])
        mmc->part_config = ext_csd[EXT_CSD_PART_CONF];
    if (part_completed &&
        (ext_csd[EXT_CSD_PARTITIONING_SUPPORT] & ENHNCD_SUPPORT))
        mmc->part_attr = ext_csd[EXT_CSD_PARTITIONS_ATTRIBUTE];

    mmc->capacity_boot = ext_csd[EXT_CSD_BOOT_MULT] << 17;

    mmc->capacity_rpmb = ext_csd[EXT_CSD_RPMB_MULT] << 17;

    for (i = 0; i < 4; i++) {
        int idx = EXT_CSD_GP_SIZE_MULT + i * 3;
        uint32_t mult = (ext_csd[idx + 2] << 16) +
                        (ext_csd[idx + 1] << 8) + ext_csd[idx];
        if (mult)
            has_parts = true;
        if (!part_completed)
            continue;
        mmc->capacity_gp[i] = mult;
        mmc->capacity_gp[i] *=
            ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE];
        mmc->capacity_gp[i] *= ext_csd[EXT_CSD_HC_WP_GRP_SIZE];
        mmc->capacity_gp[i] <<= 19;
    }

#ifndef CONFIG_SPL_BUILD
    if (part_completed) {
        mmc->enh_user_size =
            (ext_csd[EXT_CSD_ENH_SIZE_MULT + 2] << 16) +
            (ext_csd[EXT_CSD_ENH_SIZE_MULT + 1] << 8) +
            ext_csd[EXT_CSD_ENH_SIZE_MULT];
        mmc->enh_user_size *= ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE];
        mmc->enh_user_size *= ext_csd[EXT_CSD_HC_WP_GRP_SIZE];
        mmc->enh_user_size <<= 19;
        mmc->enh_user_start =
            (ext_csd[EXT_CSD_ENH_START_ADDR + 3] << 24) +
            (ext_csd[EXT_CSD_ENH_START_ADDR + 2] << 16) +
            (ext_csd[EXT_CSD_ENH_START_ADDR + 1] << 8) +
            ext_csd[EXT_CSD_ENH_START_ADDR];
        if (mmc->high_capacity)
            mmc->enh_user_start <<= 9;
    }
#endif

    /*
     * Host needs to enable ERASE_GRP_DEF bit if device is
     * partitioned. This bit will be lost every time after a reset
     * or power off. This will affect erase size.
     */
    if (part_completed)
        has_parts = true;
    if ((ext_csd[EXT_CSD_PARTITIONING_SUPPORT] & PART_SUPPORT) &&
        (ext_csd[EXT_CSD_PARTITIONS_ATTRIBUTE] & PART_ENH_ATTRIB))
        has_parts = true;
    if (has_parts) {
        err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
                         EXT_CSD_ERASE_GROUP_DEF, 1);

        if (err)
            goto error;

        ext_csd[EXT_CSD_ERASE_GROUP_DEF] = 1;
    }

    if (ext_csd[EXT_CSD_ERASE_GROUP_DEF] & 0x01) {
#ifdef MMC_WRITE
        /* Read out group size from ext_csd */
        mmc->erase_grp_size =
            ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 1024;
#endif
        /*
         * if high capacity and partition setting completed
         * SEC_COUNT is valid even if it is smaller than 2 GiB
         * JEDEC Standard JESD84-B45, 6.2.4
         */
        if (mmc->high_capacity && part_completed) {
            capacity = (ext_csd[EXT_CSD_SEC_CNT]) |
                       (ext_csd[EXT_CSD_SEC_CNT + 1] << 8) |
                       (ext_csd[EXT_CSD_SEC_CNT + 2] << 16) |
                       (ext_csd[EXT_CSD_SEC_CNT + 3] << 24);
            capacity *= MMC_MAX_BLOCK_LEN;
            mmc->capacity_user = capacity;
        }
    }
#ifdef MMC_WRITE
    else {
        /* Calculate the group size from the csd value. */
        int erase_gsz, erase_gmul;

        erase_gsz = (mmc->csd[2] & 0x00007c00) >> 10;
        erase_gmul = (mmc->csd[2] & 0x000003e0) >> 5;
        mmc->erase_grp_size = (erase_gsz + 1)
                              * (erase_gmul + 1);
    }
#endif
#ifdef MMC_HW_PARTITIONING
    mmc->hc_wp_grp_size = 1024
                          * ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]
                          * ext_csd[EXT_CSD_HC_WP_GRP_SIZE];
#endif

    mmc->wr_rel_set = ext_csd[EXT_CSD_WR_REL_SET];

    return HAL_SDMMC_ERR_NONE;
error:
    if (mmc->ext_csd) {
#ifndef MMC_TINY
        //free(mmc->ext_csd);
#endif
        mmc->ext_csd = NULL;
    }
    return err;
}

static void part_init(struct blk_desc *dev_desc)
{
    return;
}

static int mmc_startup(struct mmc *mmc)
{
    int err, i;
    uint32_t mult, freq;
    uint64_t cmult, csize;
    struct mmc_cmd cmd;
    struct blk_desc *bdesc;

#ifdef CONFIG_MMC_SPI_CRC_ON
    if (mmc_host_is_spi(mmc)) { /* enable CRC check for spi */
        cmd.cmdidx = MMC_CMD_SPI_CRC_ON_OFF;
        cmd.resp_type = MMC_RSP_R1;
        cmd.cmdarg = 1;
        err = mmc_send_cmd(mmc, &cmd, NULL);
        if (err)
            return err;
    }
#endif

    /* Put the Card in Identify Mode */
    cmd.cmdidx = mmc_host_is_spi(mmc) ? MMC_CMD_SEND_CID :
                 MMC_CMD_ALL_SEND_CID; /* cmd not supported in spi */
    cmd.resp_type = MMC_RSP_R2;
    cmd.cmdarg = 0;

    err = mmc_send_cmd_quirks(mmc, &cmd, NULL, MMC_QUIRK_RETRY_SEND_CID, 4);
    if (err)
        return err;

    memcpy(mmc->cid, cmd.response, 16);

    /*
     * For MMC cards, set the Relative Address.
     * For SD cards, get the Relatvie Address.
     * This also puts the cards into Standby State
     */
    if (!mmc_host_is_spi(mmc)) { /* cmd not supported in spi */
        cmd.cmdidx = SD_CMD_SEND_RELATIVE_ADDR;
        cmd.cmdarg = mmc->rca << 16;
        cmd.resp_type = MMC_RSP_R6;

        err = mmc_send_cmd(mmc, &cmd, NULL);

        if (err)
            return err;

        if (IS_SD(mmc))
            mmc->rca = (cmd.response[0] >> 16) & 0xffff;
    }

    /* Get the Card-Specific Data */
    cmd.cmdidx = MMC_CMD_SEND_CSD;
    cmd.resp_type = MMC_RSP_R2;
    cmd.cmdarg = mmc->rca << 16;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err)
        return err;

    mmc->csd[0] = cmd.response[0];
    mmc->csd[1] = cmd.response[1];
    mmc->csd[2] = cmd.response[2];
    mmc->csd[3] = cmd.response[3];

    if (mmc->version == MMC_VERSION_UNKNOWN) {
        int version = (cmd.response[0] >> 26) & 0xf;

        switch (version) {
            case 0:
                mmc->version = MMC_VERSION_1_2;
                break;
            case 1:
                mmc->version = MMC_VERSION_1_4;
                break;
            case 2:
                mmc->version = MMC_VERSION_2_2;
                break;
            case 3:
                mmc->version = MMC_VERSION_3;
                break;
            case 4:
                mmc->version = MMC_VERSION_4;
                break;
            default:
                mmc->version = MMC_VERSION_1_2;
                break;
        }
    }

    /* divide frequency by 10, since the mults are 10x bigger */
    freq = fbase[(cmd.response[0] & 0x7)];
    mult = multipliers[((cmd.response[0] >> 3) & 0xf)];

    mmc->legacy_speed = freq * mult;
    mmc_select_mode(mmc, MMC_LEGACY);

    mmc->dsr_imp = ((cmd.response[1] >> 12) & 0x1);
    mmc->read_bl_len = 1 << ((cmd.response[1] >> 16) & 0xf);

#ifdef MMC_WRITE
    if (IS_SD(mmc))
        mmc->write_bl_len = mmc->read_bl_len;
    else
        mmc->write_bl_len = 1 << ((cmd.response[3] >> 22) & 0xf);
#endif

    if (mmc->high_capacity) {
        csize = (mmc->csd[1] & 0x3f) << 16
                | (mmc->csd[2] & 0xffff0000) >> 16;
        cmult = 8;
    } else {
        csize = (mmc->csd[1] & 0x3ff) << 2
                | (mmc->csd[2] & 0xc0000000) >> 30;
        cmult = (mmc->csd[2] & 0x00038000) >> 15;
    }

    mmc->capacity_user = (csize + 1) << (cmult + 2);
    mmc->capacity_user *= mmc->read_bl_len;
    mmc->capacity_boot = 0;
    mmc->capacity_rpmb = 0;
    for (i = 0; i < 4; i++)
        mmc->capacity_gp[i] = 0;

    if (mmc->read_bl_len > MMC_MAX_BLOCK_LEN)
        mmc->read_bl_len = MMC_MAX_BLOCK_LEN;

#ifdef MMC_WRITE
    if (mmc->write_bl_len > MMC_MAX_BLOCK_LEN)
        mmc->write_bl_len = MMC_MAX_BLOCK_LEN;
#endif

    if ((mmc->dsr_imp) && (0xffffffff != mmc->dsr)) {
        cmd.cmdidx = MMC_CMD_SET_DSR;
        cmd.cmdarg = (mmc->dsr & 0xffff) << 16;
        cmd.resp_type = MMC_RSP_NONE;
        if (mmc_send_cmd(mmc, &cmd, NULL))
            HAL_SDMMC_TRACE(0, "MMC: SET_DSR failed");
    }

    /* Select the card, and put it into Transfer Mode */
    if (!mmc_host_is_spi(mmc)) { /* cmd not supported in spi */
        cmd.cmdidx = MMC_CMD_SELECT_CARD;
        cmd.resp_type = MMC_RSP_R1;
        cmd.cmdarg = mmc->rca << 16;
        err = mmc_send_cmd(mmc, &cmd, NULL);

        if (err)
            return err;
    }

    /*
     * For SD, its erase group is always one sector
     */
#ifdef MMC_WRITE
    mmc->erase_grp_size = 1;
#endif
    mmc->part_config = MMCPART_NOAVAILABLE;

    err = mmc_startup_v4(mmc);
    if (err)
        return err;

    err = mmc_set_capacity(mmc, mmc_get_blk_desc(mmc)->hwpart);
    if (err)
        return err;

#ifdef MMC_TINY
    mmc_set_clock(mmc, mmc->legacy_speed, false);
    mmc_select_mode(mmc, MMC_LEGACY);
    mmc_set_bus_width(mmc, 1);
#else
    if (IS_SD(mmc)) {
        err = sd_get_capabilities(mmc);
        if (err)
            return err;
        err = sd_select_mode_and_width(mmc, mmc->card_caps);
    } else {
        err = mmc_get_capabilities(mmc);
        if (err)
            return err;
        err = mmc_select_mode_and_width(mmc, mmc->card_caps);
    }
#endif
    if (err)
        return err;

    mmc->best_mode = mmc->selected_mode;

    /* Fix the block length for DDR mode */
    if (mmc->ddr_mode) {
        mmc->read_bl_len = MMC_MAX_BLOCK_LEN;
#ifdef MMC_WRITE
        mmc->write_bl_len = MMC_MAX_BLOCK_LEN;
#endif
    }

    /* fill in device description */
    bdesc = mmc_get_blk_desc(mmc);
    bdesc->lun = 0;
    bdesc->hwpart = 0;
    bdesc->type = 0;
    bdesc->blksz = mmc->read_bl_len;
    bdesc->log2blksz = LOG2(bdesc->blksz);
    bdesc->lba = sdmmc_lldiv(mmc->capacity, mmc->read_bl_len);
#if !defined(CONFIG_SPL_BUILD) || \
        (defined(CONFIG_SPL_LIBCOMMON_SUPPORT) && \
        !defined(USE_TINY_PRINTF))
    sprintf(bdesc->vendor, "Man %06x Snr %04x%04x",
            mmc->cid[0] >> 24, (mmc->cid[2] & 0xffff),
            (mmc->cid[3] >> 16) & 0xffff);
    sprintf(bdesc->product, "%c%c%c%c%c%c", mmc->cid[0] & 0xff,
            (mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
            (mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff,
            (mmc->cid[2] >> 24) & 0xff);
    sprintf(bdesc->revision, "%d.%d", (mmc->cid[2] >> 20) & 0xf,
            (mmc->cid[2] >> 16) & 0xf);
#else
    bdesc->vendor[0] = 0;
    bdesc->product[0] = 0;
    bdesc->revision[0] = 0;
#endif

#if !defined(CONFIG_DM_MMC) && (!defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBDISK_SUPPORT))
    part_init(bdesc);
#endif

    return HAL_SDMMC_ERR_NONE;
}

static int mmc_send_if_cond(struct mmc *mmc)
{
    struct mmc_cmd cmd;
    int err;

    cmd.cmdidx = SD_CMD_SEND_IF_COND;
    /* We set the bit if the host supports voltages between 2.7 and 3.6 V */
    cmd.cmdarg = ((mmc->cfg->voltages & 0xff8000) != 0) << 8 | 0xaa;
    cmd.resp_type = MMC_RSP_R7;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err)
        return err;

    if ((cmd.response[0] & 0xff) != 0xaa)
        return HAL_SDMMC_OP_NOT_SUPPORTED_EP;
    else
        mmc->version = SD_VERSION_2;

    return HAL_SDMMC_ERR_NONE;
}

static int mmc_power_init(struct mmc *mmc)
{
#ifdef DM_MMC
#ifdef DM_REGULATOR
    int ret;

    ret = device_get_supply_regulator(mmc->dev, "vmmc-supply",
                                      &mmc->vmmc_supply);
    if (ret)
        HAL_SDMMC_TRACE(0, "%s: No vmmc supply", mmc->dev->name);

    ret = device_get_supply_regulator(mmc->dev, "vqmmc-supply",
                                      &mmc->vqmmc_supply);
    if (ret)
        HAL_SDMMC_TRACE(0, "%s: No vqmmc supply", mmc->dev->name);
#endif
#else /* !CONFIG_DM_MMC */
    /*
     * Driver model should use a regulator, as above, rather than calling
     * out to board code.
     */
    board_mmc_power_init();
#endif
    return HAL_SDMMC_ERR_NONE;
}

static void mmc_set_initial_state(struct mmc *mmc)
{
    int err;

    /* First try to set 3.3V. If it fails set to 1.8V */
    err = mmc_set_signal_voltage(mmc, MMC_SIGNAL_VOLTAGE_330);
    if (err != 0)
        err = mmc_set_signal_voltage(mmc, MMC_SIGNAL_VOLTAGE_180);
    if (err != 0)
        HAL_SDMMC_TRACE(0, "mmc: failed to set signal voltage");

    mmc_select_mode(mmc, MMC_LEGACY);
    mmc_set_bus_width(mmc, 1);
    mmc_set_clock(mmc, 0, MMC_CLK_ENABLE);
}

static int mmc_power_on(struct mmc *mmc)
{
#if defined(DM_MMC) && defined(DM_REGULATOR)
    if (mmc->vmmc_supply) {
        int ret = regulator_set_enable(mmc->vmmc_supply, true);

        if (ret && ret != -EACCES) {
            HAL_SDMMC_TRACE(0, "Error enabling VMMC supply : %d", ret);
            return ret;
        }
    }
#endif
    return HAL_SDMMC_ERR_NONE;
}

static int mmc_power_off(struct mmc *mmc)
{
    mmc_set_clock(mmc, 0, MMC_CLK_DISABLE);
#if defined(DM_MMC) && defined(DM_REGULATOR)
    if (mmc->vmmc_supply) {
        int ret = regulator_set_enable(mmc->vmmc_supply, false);

        if (ret && ret != -EACCES) {
            HAL_SDMMC_TRACE(0, "Error disabling VMMC supply : %d", ret);
            return ret;
        }
    }
#endif
    return HAL_SDMMC_ERR_NONE;
}

static int mmc_power_cycle(struct mmc *mmc)
{
    int ret;

    ret = mmc_power_off(mmc);
    if (ret)
        return ret;

    ret = mmc_host_power_cycle(mmc);
    if (ret)
        return ret;

    /*
     * SD spec recommends at least 1ms of delay. Let's wait for 2ms
     * to be on the safer side.
     */
    //mmc_udelay(2000);
    hal_sdmmc_delay(2);
    return mmc_power_on(mmc);
}

static int mmc_get_op_cond(struct mmc *mmc, bool quiet)
{
    bool uhs_en = supports_uhs(mmc->cfg->host_caps);
    int err;

    HAL_SDMMC_TRACE(0, "%s:%d, quiet:%d", __func__, __LINE__, quiet);
    if (mmc->has_init)
        return HAL_SDMMC_ERR_NONE;

    err = mmc_power_init(mmc);
    if (err)
        return err;

#ifdef CONFIG_MMC_QUIRKS
    mmc->quirks = MMC_QUIRK_RETRY_SET_BLOCKLEN |
                  MMC_QUIRK_RETRY_SEND_CID |
                  MMC_QUIRK_RETRY_APP_CMD;
#endif

    err = mmc_power_cycle(mmc);
    if (err) {
        /*
         * if power cycling is not supported, we should not try
         * to use the UHS modes, because we wouldn't be able to
         * recover from an error during the UHS initialization.
         */
        HAL_SDMMC_TRACE(0, "Unable to do a full power cycle. Disabling the UHS modes for safety");
        uhs_en = false;
        mmc->host_caps &= ~UHS_CAPS;
        err = mmc_power_on(mmc);
    }
    if (err)
        return err;

#ifdef DM_MMC
    /*
     * Re-initialization is needed to clear old configuration for
     * mmc rescan.
     */
    err = mmc_reinit(mmc);
#else
    /* made sure it's not NULL earlier */
    err = mmc->cfg->ops->init(mmc);
#endif
    if (err)
        return err;
    mmc->ddr_mode = 0;

retry:
    mmc_set_initial_state(mmc);

    /* Reset the Card */
    err = mmc_go_idle(mmc);

    if (err)
        return err;

    /* The internal partition reset to user partition(0) at every CMD0 */
    mmc_get_blk_desc(mmc)->hwpart = 0;

    /* Test for SD version 2 */
    err = mmc_send_if_cond(mmc);

    /* Now try to get the SD card's operating condition */
    err = sd_send_op_cond(mmc, uhs_en);

    if (err && uhs_en) {
        uhs_en = false;
        mmc_power_cycle(mmc);
        goto retry;
    }

    /* If the command timed out, we check for an MMC card */
    if (err == HAL_SDMMC_COMM_TIMEOUT) {
        err = mmc_send_op_cond(mmc);
        if (err) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
            if (!quiet)
                HAL_SDMMC_TRACE(0, "Card did not respond to voltage select! : %d", err);
#endif
            return HAL_SDMMC_OP_NOT_SUPPORTED_EP;
        }
    }

    return err;
}

static int mmc_start_init(struct mmc *mmc)
{
    bool no_card;
    int err = 0;

    HAL_SDMMC_TRACE(0, "%s:%d", __func__, __LINE__);
    /*
     * all hosts are capable of 1 bit bus-width and able to use the legacy
     * timings.
     */
    mmc->host_caps = mmc->cfg->host_caps | MMC_CAP(MMC_LEGACY) | MMC_MODE_1BIT;

#ifdef CONFIG_MMC_SPEED_MODE_SET
    if (mmc->user_speed_mode != MMC_MODES_END) {
        int i;
        /* set host caps */
        if (mmc->host_caps & MMC_CAP(mmc->user_speed_mode)) {
            /* Remove all existing speed capabilities */
            for (i = MMC_LEGACY; i < MMC_MODES_END; i++) {
                mmc->host_caps &= ~MMC_CAP(i);
            }
            mmc->host_caps |= (MMC_CAP(mmc->user_speed_mode)
                               | MMC_CAP(MMC_LEGACY) | MMC_MODE_1BIT);
        } else {
            HAL_SDMMC_TRACE(0, "bus_mode requested is not supported");
            return HAL_SDMMC_INVALID_PARAMETER;
        }
    }
#endif

#ifdef DM_MMC
    mmc_deferred_probe(mmc);
#endif
#if !defined(CONFIG_MMC_BROKEN_CD)
    no_card = mmc_getcd(mmc) == 0;
#else
    no_card = 0;
#endif
#ifndef DM_MMC
    /* we pretend there's no card when init is NULL */
    no_card = no_card || (mmc->cfg->ops->init == NULL);
#endif
    if (no_card) {
        mmc->has_init = 0;
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
        HAL_SDMMC_TRACE(0, "MMC: no card present");
#endif
        return HAL_SDMMC_NO_MEDIUM_FOUND;
    }

    err = mmc_get_op_cond(mmc, false);
    if (!err)
        mmc->init_in_progress = 1;

    return err;
}

static int mmc_complete_init(struct mmc *mmc)
{
    int err = 0;

    mmc->init_in_progress = 0;
    if (mmc->op_cond_pending)
        err = mmc_complete_op_cond(mmc);

    if (!err)
        err = mmc_startup(mmc);
    if (err)
        mmc->has_init = 0;
    else
        mmc->has_init = 1;
    return err;
}

static int mmc_init(struct mmc *mmc)
{
    int err = 0;
    POSSIBLY_UNUSED uint32_t start;
#ifdef DM_MMC
    struct mmc_uclass_priv *upriv = dev_get_uclass_priv(mmc->dev);
    upriv->mmc = mmc;
#endif
    if (mmc->has_init)
        return HAL_SDMMC_ERR_NONE;

    start = hal_sys_timer_get();
    if (!mmc->init_in_progress)
        err = mmc_start_init(mmc);

    if (!err)
        err = mmc_complete_init(mmc);
    if (err)
        HAL_SDMMC_TRACE(0, "%s: %d, time %ums", __FUNCTION__, err, TICKS_TO_MS(hal_sys_timer_get() - start));

    return err;
}

#if defined(MMC_UHS_SUPPORT) || \
    defined(MMC_HS200_SUPPORT) || \
    defined(MMC_HS400_SUPPORT)
static int mmc_deinit(struct mmc *mmc)
{
    uint32_t caps_filtered;

    if (!mmc->has_init)
        return HAL_SDMMC_ERR_NONE;

    if (IS_SD(mmc)) {
        caps_filtered = mmc->card_caps &
                        ~(MMC_CAP(UHS_SDR12) | MMC_CAP(UHS_SDR25) |
                          MMC_CAP(UHS_SDR50) | MMC_CAP(UHS_DDR50) |
                          MMC_CAP(UHS_SDR104));

        return sd_select_mode_and_width(mmc, caps_filtered);
    } else {
        caps_filtered = mmc->card_caps &
                        ~(MMC_CAP(MMC_HS_200) | MMC_CAP(MMC_HS_400) | MMC_CAP(MMC_HS_400_ES));

        return mmc_select_mode_and_width(mmc, caps_filtered);
    }
}
#endif

int mmc_set_dsr(struct mmc *mmc, uint16_t val)
{
    mmc->dsr = val;
    return HAL_SDMMC_ERR_NONE;
}

static void mmc_set_preinit(struct mmc *mmc, int preinit)
{
    mmc->preinit = preinit;
}

#ifdef DM_MMC
static int mmc_probe(struct bd_info *bis)
{
    int ret, i;
    struct uclass *uc;
    struct udevice *dev;

    ret = uclass_get(UCLASS_MMC, &uc);
    if (ret)
        return ret;

    /*
     * Try to add them in sequence order. Really with driver model we
     * should allow holes, but the current MMC list does not allow that.
     * So if we request 0, 1, 3 we will get 0, 1, 2.
     */
    for (i = 0; ; i++) {
        ret = uclass_get_device_by_seq(UCLASS_MMC, i, &dev);
        if (ret == HAL_SDMMC_NO_SUCH_DEVICE)
            break;
    }
    uclass_foreach_dev(dev, uc) {
        ret = device_probe(dev);
        if (ret)
            HAL_SDMMC_TRACE(0, "%s - probe failed: %d", dev->name, ret);
    }

    return HAL_SDMMC_ERR_NONE;
}
#else
static int mmc_probe(struct mmc *mmc)
{
    return HAL_SDMMC_ERR_NONE;
}
#endif

static int mmc_initialize(struct mmc *mmc)
{
    int ret = 0;
    HAL_SDMMC_TRACE(0, "%s:%d", __func__, __LINE__);

#ifndef BLK
#ifndef MMC_TINY
    //mmc_list_init();
#endif
#endif
    ret = mmc_probe(mmc);
    if (ret)
        return ret;

#ifndef CONFIG_SPL_BUILD
    //print_mmc_devices(',');
#endif

    mmc_do_preinit(mmc);
    return HAL_SDMMC_ERR_NONE;
}

#ifdef DM_MMC
int mmc_init_device(int num)
{
    struct udevice *dev;
    struct mmc *m;
    int ret;

    if (uclass_get_device_by_seq(UCLASS_MMC, num, &dev)) {
        ret = uclass_get_device(UCLASS_MMC, num, &dev);
        if (ret)
            return ret;
    }

    m = mmc_get_mmc_dev(dev);
    m->user_speed_mode = MMC_MODES_END; /* Initialising user set speed mode */

    if (!m)
        return HAL_SDMMC_ERR_NONE;
    if (m->preinit)
        mmc_start_init(m);

    return HAL_SDMMC_ERR_NONE;
}
#endif

#ifdef CONFIG_CMD_BKOPS_ENABLE
int mmc_set_bkops_enable(struct mmc *mmc)
{
    int err;
    ALLOC_CACHE_ALIGN_BUFFER(uint8_t, ext_csd, MMC_MAX_BLOCK_LEN);

    err = mmc_send_ext_csd(mmc, ext_csd);
    if (err) {
        HAL_SDMMC_TRACE(0, "Could not get ext_csd register values");
        return err;
    }

    if (!(ext_csd[EXT_CSD_BKOPS_SUPPORT] & 0x1)) {
        HAL_SDMMC_TRACE(0, "Background operations not supported on device");
        return HAL_SDMMC_MEDIUM_TYPE_ERR;
    }

    if (ext_csd[EXT_CSD_BKOPS_EN] & 0x1) {
        HAL_SDMMC_TRACE(0, "Background operations already enabled");
        return HAL_SDMMC_ERR_NONE;
    }

    err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_BKOPS_EN, 1);
    if (err) {
        HAL_SDMMC_TRACE(0, "Failed to enable manual background operations");
        return err;
    }

    HAL_SDMMC_TRACE(0, "Enabled manual background operations");

    return HAL_SDMMC_ERR_NONE;
}
#endif

int mmc_get_env_dev(void)
{
#ifdef CONFIG_SYS_MMC_ENV_DEV
    return CONFIG_SYS_MMC_ENV_DEV;
#else
    return HAL_SDMMC_ERR_NONE;
#endif
}
/****************************platform related functions************************/
static HAL_SDMMC_DELAY_FUNC sdmmc_delay = NULL;

static void mmc_udelay(uint32_t cnt)
{
    volatile uint32_t i = 0, c = 0;
    for (i = 0; i < cnt; ++i) {
        c++;
        __asm("nop");
    }
}

static void hal_sdmmc_delay(uint32_t ms)
{
    if (sdmmc_delay) {
        sdmmc_delay(ms);
    } else {
        osDelay(ms);
    }
}

#ifndef DM_MMC
static void board_mmc_power_init(void)
{

}
#endif

/*****************************phy related functions****************************/
#ifdef MMC_HS400_SUPPORT
static int mmc_hs400_prepare_ddr(struct mmc *mmc)
{
    return HAL_SDMMC_ERR_NONE;
}
#endif

/***************************ip related functions*******************************/
#define HAL_SDMMC_USE_DMA               1
#define MAX_RETRY_CNT                   15000
#define TIMEOUT_TIME                    1000

#define __SDMMC_DIV_ROUND_UP(n,d)       (((n) + (d) - 1) / (d))

#ifndef CONFIG_SYS_MMC_MAX_BLK_COUNT
    #define CONFIG_SYS_MMC_MAX_BLK_COUNT 65535
#endif

struct sdmmc_ip_host {
    uint8_t host_id;
    uint32_t clock;
    uint32_t bus_hz;
    uint32_t div;
    uint8_t buswidth;
    void *ioaddr;
    uint32_t fifoth_val;
    uint32_t period_st_ns;
    uint32_t final_bus_speed;
    void *priv;

    struct mmc_config cfg;
    struct mmc *mmc;

#ifdef HAL_SDMMC_USE_DMA
    uint8_t dma_ch;
    uint8_t dma_in_use;
    volatile uint8_t sdmmc_dma_lock;
    HAL_DMA_IRQ_HANDLER_T tx_dma_handler;
    HAL_DMA_IRQ_HANDLER_T rx_dma_handler;
#endif
};

enum sdmmc_bus_width {
    SDMMC_BUS_WIDTH_1 = 1,
    SDMMC_BUS_WIDTH_4 = 4,
    SDMMC_BUS_WIDTH_8 = 8,
};

static struct sdmmc_ip_host sdmmc_host[HAL_SDMMC_ID_NUM];
static struct HAL_SDMMC_CB_T sdmmc_callback_default[HAL_SDMMC_ID_NUM];
static struct HAL_SDMMC_CB_T *sdmmc_callback[HAL_SDMMC_ID_NUM] = {NULL};
static uint32_t sdmmc_ip_base[HAL_SDMMC_ID_NUM] = {
    SDMMC0_BASE,
#ifdef SDMMC1_BASE
    SDMMC1_BASE,
#endif
};

#ifdef HAL_SDMMC_USE_DMA
#define SDMMC_DMA_LINK_SIZE     16380       //one desc can send and receive 16380(=4095*4) bytes
#define SDMMC_DMA_TSIZE_MAX     0xFFF       //4095
#ifndef SDMMC_DMA_DESC_CNT
    #define SDMMC_DMA_DESC_CNT   4          //4*16380=63.98KB
#endif
SYNC_FLAGS_LOC static struct HAL_DMA_DESC_T sdmmc_dma_desc[HAL_SDMMC_ID_NUM][SDMMC_DMA_DESC_CNT];
static void sdmmc_ip0_ext_dma_tx_handler(uint8_t chan, uint32_t remain_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli);
static void sdmmc_ip0_ext_dma_rx_handler(uint8_t chan, uint32_t remain_rsize, uint32_t error, struct HAL_DMA_DESC_T *lli);
#ifdef SDMMC1_BASE
static void sdmmc_ip1_ext_dma_tx_handler(uint8_t chan, uint32_t remain_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli);
static void sdmmc_ip1_ext_dma_rx_handler(uint8_t chan, uint32_t remain_rsize, uint32_t error, struct HAL_DMA_DESC_T *lli);
#endif
static HAL_DMA_IRQ_HANDLER_T sdmmc_ip_ext_dma_irq_handlers[HAL_SDMMC_ID_NUM * 2] = {
    sdmmc_ip0_ext_dma_tx_handler, sdmmc_ip0_ext_dma_rx_handler,
#ifdef SDMMC1_BASE
    sdmmc_ip1_ext_dma_tx_handler, sdmmc_ip1_ext_dma_rx_handler,
#endif
};
#endif

static inline void sdmmc_ip_writel(struct sdmmc_ip_host *host, uint32_t reg, uint32_t val)
{
    *((volatile uint32_t *)(host->ioaddr + reg)) = val;
}

static inline uint32_t sdmmc_ip_readl(struct sdmmc_ip_host *host, uint32_t reg)
{
    return *((volatile uint32_t *)(host->ioaddr + reg));
}

static int sdmmc_ip_wait_reset(struct sdmmc_ip_host *host, uint32_t value)
{
    uint32_t ctrl;
    uint32_t timeout = TIMEOUT_TIME;

    sdmmc_ip_writel(host, SDMMCIP_REG_CTRL, value);
    while (timeout) {
        timeout--;
        ctrl = sdmmc_ip_readl(host, SDMMCIP_REG_CTRL);
        if (!(ctrl & SDMMCIP_REG_RESET_ALL)) {
            return 0;//reset success
        }
    }

    return 1;//reset failure
}

static void sdmmc_ip_reset_fifo(struct sdmmc_ip_host *host)
{
    uint32_t ctrl;

    ctrl = sdmmc_ip_readl(host, SDMMCIP_REG_CTRL);
    ctrl |= SDMMCIP_REG_CTRL_FIFO_RESET;
    sdmmc_ip_wait_reset(host, ctrl);
}

#ifdef HAL_SDMMC_USE_DMA
static void sdmmc_ip_reset_dma(struct sdmmc_ip_host *host)
{
    uint32_t ctrl;

    if (host->dma_in_use) {
        host->dma_in_use = 0;
    } else {
        return;//Prevent multiple calls
    }

    //reset sdmmc ip dma
    ctrl = sdmmc_ip_readl(host, SDMMCIP_REG_CTRL);
    ctrl |= SDMMCIP_REG_CTRL_DMA_RESET;
    sdmmc_ip_wait_reset(host, ctrl);

    //free gpdma channel
    hal_gpdma_free_chan(host->dma_ch);

    //close sdmmc ip dma
    ctrl = sdmmc_ip_readl(host, SDMMCIP_REG_CTRL);
    ctrl &= ~SDMMCIP_REG_DMA_EN;
    sdmmc_ip_writel(host, SDMMCIP_REG_CTRL, ctrl);
}

static void sdmmc_base_dma_tx_handler(enum HAL_SDMMC_ID_T id, uint8_t chan, uint32_t remain_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    uint32_t ip_raw_int_status = 0;
    struct sdmmc_ip_host *host = &sdmmc_host[id];

    ip_raw_int_status = sdmmc_ip_readl(host, SDMMCIP_REG_RINTSTS);
    HAL_SDMMC_TRACE(3, "%s:%d, tx ip_raw_int_status 0x%x", __FUNCTION__, __LINE__, (uint32_t)ip_raw_int_status);
    HAL_SDMMC_TRACE(4, "---tx dma handler,chan:%d,remain:%d,error:%d,lli:0x%X", chan, remain_tsize, error, (uint32_t)lli);
    if (ip_raw_int_status & (SDMMCIP_REG_DATA_ERR | SDMMCIP_REG_DATA_TOUT)) {
        HAL_SDMMC_TRACE(3, "%s:%d, sdmmcip0 tx dma error 0x%x", __FUNCTION__, __LINE__, (uint32_t)ip_raw_int_status);
    }

    sdmmc_ip_reset_dma(host);
    host->sdmmc_dma_lock = 0;
}

static void sdmmc_base_dma_rx_handler(enum HAL_SDMMC_ID_T id, uint8_t chan, uint32_t remain_rsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    uint32_t ip_raw_int_status = 0;
    struct sdmmc_ip_host *host = &sdmmc_host[id];

    ip_raw_int_status = sdmmc_ip_readl(host, SDMMCIP_REG_RINTSTS);
    HAL_SDMMC_TRACE(3, "%s:%d, ip_raw_int_status 0x%x", __FUNCTION__, __LINE__, (uint32_t)ip_raw_int_status);
    HAL_SDMMC_TRACE(4, "---rx dma handler,chan:%d,remain:%d,error:%d,lli:0x%X", chan, remain_rsize, error, (uint32_t)lli);
    if (ip_raw_int_status & (SDMMCIP_REG_DATA_ERR | SDMMCIP_REG_DATA_TOUT)) {
        HAL_SDMMC_TRACE(3, "%s:%d, sdmmcip0 rx dma error 0x%x", __FUNCTION__, __LINE__, (uint32_t)ip_raw_int_status);
    }

    sdmmc_ip_reset_dma(host);
    host->sdmmc_dma_lock = 0;
}

static void sdmmc_ip0_ext_dma_tx_handler(uint8_t chan, uint32_t remain_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    sdmmc_base_dma_tx_handler(HAL_SDMMC_ID_0, chan, remain_tsize, error, lli);
}

static void sdmmc_ip0_ext_dma_rx_handler(uint8_t chan, uint32_t remain_rsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    sdmmc_base_dma_rx_handler(HAL_SDMMC_ID_0, chan, remain_rsize, error, lli);
}

#ifdef SDMMC1_BASE
static void sdmmc_ip1_ext_dma_tx_handler(uint8_t chan, uint32_t remain_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    sdmmc_base_dma_tx_handler(HAL_SDMMC_ID_1, chan, remain_tsize, error, lli);
}

static void sdmmc_ip1_ext_dma_rx_handler(uint8_t chan, uint32_t remain_rsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    sdmmc_base_dma_rx_handler(HAL_SDMMC_ID_1, chan, remain_rsize, error, lli);
}
#endif
#endif

static int sdmmc_ip_prepare_data(struct sdmmc_ip_host *host, struct mmc_data *data)
{
#ifdef HAL_SDMMC_USE_DMA
    uint8_t  real_src_width;
    uint16_t i;
    uint32_t ctrl;
    uint16_t link_cnt;
    uint32_t remain_transfer_size;
    POSSIBLY_UNUSED enum HAL_DMA_RET_T dret;
    struct HAL_DMA_CH_CFG_T dma_cfg;
#endif
    uint8_t id;
    uint32_t real_size;

    id = host->host_id;
    real_size = data->blocksize * data->blocks;
    sdmmc_ip_writel(host, SDMMCIP_REG_BLKSIZ, data->blocksize);
    sdmmc_ip_writel(host, SDMMCIP_REG_BYTCNT, real_size);

#ifdef HAL_SDMMC_USE_DMA
    if (host->dma_in_use) {
        return HAL_SDMMC_DMA_IN_USE;
    }
    host->sdmmc_dma_lock = 1;

    /* enable sdmmc ip dma function */
    ctrl = sdmmc_ip_readl(host, SDMMCIP_REG_CTRL);
    ctrl |= SDMMCIP_REG_DMA_EN;
    sdmmc_ip_writel(host, SDMMCIP_REG_CTRL, ctrl);

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    if (data->flags & MMC_DATA_READ) {
        dma_cfg.dst = (uint32_t)data->dest;
        if (dma_cfg.dst % 4) {
            dma_cfg.dst_width = HAL_DMA_WIDTH_BYTE;
        } else {
            dma_cfg.dst_width = HAL_DMA_WIDTH_WORD;
        }
        dma_cfg.dst_bsize = HAL_DMA_BSIZE_1;
        dma_cfg.dst_periph = 0;  // useless
        dma_cfg.handler = host->rx_dma_handler;
        dma_cfg.src_bsize = HAL_DMA_BSIZE_1;
        if (id == HAL_SDMMC_ID_0) {
            dma_cfg.src_periph = HAL_GPDMA_SDMMC0;
        }
#ifdef SDMMC1_BASE
        else if (id == HAL_SDMMC_ID_1) {
            dma_cfg.src_periph = HAL_GPDMA_SDMMC1;
        }
#endif
        else {
            return HAL_SDMMC_INVALID_PARAMETER;
        }
        dma_cfg.src_width = HAL_DMA_WIDTH_WORD;
        dma_cfg.try_burst = 1;
        dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
        dma_cfg.src = (uint32_t)0; // useless
        dma_cfg.ch = hal_gpdma_get_chan(dma_cfg.src_periph, HAL_DMA_HIGH_PRIO);
        real_src_width = dma_cfg.src_width ? dma_cfg.src_width * 2 : 1;

        HAL_SDMMC_TRACE(0, "  ");
        HAL_SDMMC_TRACE(0, "---sdmmc host use dma read");
        HAL_SDMMC_TRACE(1, "---dma read len      :%d", real_size);
        HAL_SDMMC_TRACE(1, "---dma_cfg.dst       :0x%x", dma_cfg.dst);
        HAL_SDMMC_TRACE(1, "---dma_cfg.dst_width :%d", dma_cfg.dst_width ? dma_cfg.dst_width * 2 : 1);
        HAL_SDMMC_TRACE(1, "---dma_cfg.src_width :%d", real_src_width);
        HAL_SDMMC_TRACE(1, "---dma_cfg.dst_bsize :%d", dma_cfg.dst_bsize ? (2 * (2 << dma_cfg.dst_bsize)) : 1);
        HAL_SDMMC_TRACE(1, "---dma_cfg.src_bsize :%d", dma_cfg.src_bsize ? (2 * (2 << dma_cfg.src_bsize)) : 1);
        HAL_SDMMC_TRACE(1, "---dma_cfg.src_periph:%d", dma_cfg.src_periph);
        HAL_SDMMC_TRACE(1, "---dma_cfg.ch        :%d", dma_cfg.ch);
        //HAL_SDMMC_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");

        remain_transfer_size = real_size / 4;
        if (remain_transfer_size > SDMMC_DMA_TSIZE_MAX) {
            if (remain_transfer_size % SDMMC_DMA_TSIZE_MAX) {
                link_cnt = remain_transfer_size / SDMMC_DMA_TSIZE_MAX + 1;
            } else {
                link_cnt = remain_transfer_size / SDMMC_DMA_TSIZE_MAX;
            }
        } else {
            link_cnt = 1;
        }
        //HAL_SDMMC_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "---link_cnt          :%d", link_cnt);

        //Generate dma link configuration
        for (i = 0; i < link_cnt; i++) {
            dma_cfg.dst = (uint32_t)(data->dest + SDMMC_DMA_TSIZE_MAX * real_src_width * i);
            if (remain_transfer_size > SDMMC_DMA_TSIZE_MAX) {
                dma_cfg.src_tsize = SDMMC_DMA_TSIZE_MAX;
                remain_transfer_size -= SDMMC_DMA_TSIZE_MAX;
            } else {
                dma_cfg.src_tsize = remain_transfer_size;
                remain_transfer_size = 0;
            }
            if (i + 1 == link_cnt) {
                dret = hal_dma_init_desc(&sdmmc_dma_desc[id][i], &dma_cfg, NULL, 1);
            } else {
                dret = hal_dma_init_desc(&sdmmc_dma_desc[id][i], &dma_cfg, &sdmmc_dma_desc[id][i + 1], 0);
            }
            HAL_SDMMC_ASSERT(dret == HAL_DMA_OK, "%s:%d,sdmmc dma: Failed to init rx desc %d", __FUNCTION__, __LINE__, i);

            //HAL_SDMMC_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "---dma_cfg.dst       :0x%x", dma_cfg.dst);
            //HAL_SDMMC_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "---dma_cfg.src_tsize :%d", dma_cfg.src_tsize);
            //HAL_SDMMC_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "sdmmc_dma_desc[%d][%d].src/dst/lli/ctrl=0x%08X 0x%08X 0x%08X 0x%08X@0x%08X", id, i,
            //                    sdmmc_dma_desc[id][i].src, sdmmc_dma_desc[id][i].dst, sdmmc_dma_desc[id][i].lli, sdmmc_dma_desc[id][i].ctrl, (uint32_t)&sdmmc_dma_desc[id][i]);
        }
        //HAL_SDMMC_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");
    } else {
        dma_cfg.dst = 0; // useless
        dma_cfg.dst_bsize = HAL_DMA_BSIZE_1;
        if (id == HAL_SDMMC_ID_0) {
            dma_cfg.dst_periph = HAL_GPDMA_SDMMC0;
        }
#ifdef SDMMC1_BASE
        else if (id == HAL_SDMMC_ID_1) {
            dma_cfg.dst_periph = HAL_GPDMA_SDMMC1;
        }
#endif
        else {
            return HAL_SDMMC_INVALID_PARAMETER;
        }
        dma_cfg.dst_width = HAL_DMA_WIDTH_WORD;
        dma_cfg.handler = host->tx_dma_handler;
        dma_cfg.src_bsize = HAL_DMA_BSIZE_1;
        dma_cfg.src_periph = 0; // useless
        dma_cfg.try_burst = 1;
        dma_cfg.type = HAL_DMA_FLOW_M2P_DMA;
        dma_cfg.src = (uint32_t)data->src;
        if (dma_cfg.src % 4) {
            dma_cfg.src_width = HAL_DMA_WIDTH_BYTE;
            remain_transfer_size = real_size;
        } else {
            dma_cfg.src_width = HAL_DMA_WIDTH_WORD;
            remain_transfer_size = real_size / 4;
        }
        dma_cfg.ch = hal_gpdma_get_chan(dma_cfg.dst_periph, HAL_DMA_HIGH_PRIO);
        real_src_width = dma_cfg.src_width ? dma_cfg.src_width * 2 : 1;

        HAL_SDMMC_TRACE(0, "  ");
        HAL_SDMMC_TRACE(0, "---sdmmc host use dma write");
        HAL_SDMMC_TRACE(1, "---dma write len     :%d", real_size);
        HAL_SDMMC_TRACE(1, "---dma_cfg.src       :0x%x", dma_cfg.src);
        HAL_SDMMC_TRACE(1, "---dma_cfg.dst_width :%d", dma_cfg.dst_width ? dma_cfg.dst_width * 2 : 1);
        HAL_SDMMC_TRACE(1, "---dma_cfg.src_width :%d", real_src_width);
        HAL_SDMMC_TRACE(1, "---dma_cfg.dst_bsize :%d", dma_cfg.dst_bsize ? (2 * (2 << dma_cfg.dst_bsize)) : 1);
        HAL_SDMMC_TRACE(1, "---dma_cfg.src_bsize :%d", dma_cfg.src_bsize ? (2 * (2 << dma_cfg.src_bsize)) : 1);
        HAL_SDMMC_TRACE(1, "---dma_cfg.dst_periph:%d", dma_cfg.dst_periph);
        HAL_SDMMC_TRACE(1, "---dma_cfg.ch        :%d", dma_cfg.ch);
        //HAL_SDMMC_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");

        if (remain_transfer_size > SDMMC_DMA_TSIZE_MAX) {
            if (remain_transfer_size % SDMMC_DMA_TSIZE_MAX) {
                link_cnt = remain_transfer_size / SDMMC_DMA_TSIZE_MAX + 1;
            } else {
                link_cnt = remain_transfer_size / SDMMC_DMA_TSIZE_MAX;
            }
        } else {
            link_cnt = 1;
        }
        //HAL_SDMMC_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "---link_cnt          :%d", link_cnt);

        //Generate dma link configuration
        for (i = 0; i < link_cnt; i++) {
            dma_cfg.src = (uint32_t)(data->src + SDMMC_DMA_TSIZE_MAX * real_src_width * i);
            if (remain_transfer_size > SDMMC_DMA_TSIZE_MAX) {
                dma_cfg.src_tsize = SDMMC_DMA_TSIZE_MAX;
                remain_transfer_size -= SDMMC_DMA_TSIZE_MAX;
            } else {
                dma_cfg.src_tsize = remain_transfer_size;
                remain_transfer_size = 0;
            }
            if (i + 1 == link_cnt) {
                dret = hal_dma_init_desc(&sdmmc_dma_desc[id][i], &dma_cfg, NULL, 1);
            } else {
                dret = hal_dma_init_desc(&sdmmc_dma_desc[id][i], &dma_cfg, &sdmmc_dma_desc[id][i + 1], 0);
            }
            HAL_SDMMC_ASSERT(dret == HAL_DMA_OK, "%s:%d,sdmmc dma: Failed to init rx desc %d", __FUNCTION__, __LINE__, i);

            //HAL_SDMMC_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "---dma_cfg.src       :0x%x", dma_cfg.src);
            //HAL_SDMMC_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "---dma_cfg.src_tsize :%d", dma_cfg.src_tsize);
            //HAL_SDMMC_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "sdmmc_dma_desc[%d][%d].src/dst/lli/ctrl=0x%08X 0x%08X 0x%08X 0x%08X@0x%08X", id, i,
            //                    sdmmc_dma_desc[id][i].src, sdmmc_dma_desc[id][i].dst, sdmmc_dma_desc[id][i].lli, sdmmc_dma_desc[id][i].ctrl, (uint32_t)&sdmmc_dma_desc[id][i]);
        }
        //HAL_SDMMC_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");
    }

    host->dma_in_use = 1;
    host->dma_ch = dma_cfg.ch;
    hal_gpdma_sg_start(&sdmmc_dma_desc[id][0], &dma_cfg);
#endif

    return HAL_SDMMC_ERR_NONE;
}

static int sdmmc_ip_set_transfer_mode(struct sdmmc_ip_host *host, struct mmc_data *data)
{
    uint32_t mode;

    mode = SDMMCIP_REG_CMD_DATA_EXP;
    if (data->flags & MMC_DATA_WRITE)
        mode |= SDMMCIP_REG_CMD_RW;

    return mode;
}

POSSIBLY_UNUSED static void sdmmc_ip_func_int_mask(enum HAL_SDMMC_ID_T id, uint8_t mask0_unmask1)
{
    uint32_t mask;
    struct sdmmc_ip_host *host = &sdmmc_host[id];

    mask = sdmmc_ip_readl(host, SDMMCIP_REG_INTMASK);
    if (mask0_unmask1) {
        //unmask(enable) func interrupt
        mask |= (SDMMCIP_REG_INTMSK_SDIO_FUNC1 | SDMMCIP_REG_INTMSK_SDIO_FUNC2
                 | SDMMCIP_REG_INTMSK_SDIO_FUNC3 | SDMMCIP_REG_INTMSK_SDIO_FUNC4
                 | SDMMCIP_REG_INTMSK_SDIO_FUNC5 | SDMMCIP_REG_INTMSK_SDIO_FUNC6
                 | SDMMCIP_REG_INTMSK_SDIO_FUNC7);
    } else {
        //mask(disable) func interrupt
        mask &= ~(SDMMCIP_REG_INTMSK_SDIO_FUNC1 | SDMMCIP_REG_INTMSK_SDIO_FUNC2
                  | SDMMCIP_REG_INTMSK_SDIO_FUNC3 | SDMMCIP_REG_INTMSK_SDIO_FUNC4
                  | SDMMCIP_REG_INTMSK_SDIO_FUNC5 | SDMMCIP_REG_INTMSK_SDIO_FUNC6
                  | SDMMCIP_REG_INTMSK_SDIO_FUNC7);
    }
    sdmmc_ip_writel(host, SDMMCIP_REG_INTMASK, mask);
}

static int sdmmc_ip_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
    int ret = 0;
    int flags = 0, i;
    uint32_t retry = MAX_RETRY_CNT;
    uint32_t mask = 0;
    uint32_t busy_timeout = MS_TO_TICKS(10), busy_t = 0;
#ifndef HAL_SDMMC_USE_DMA
    uint32_t status = 0, fifo_data = 0;
#endif
    struct sdmmc_ip_host *host = mmc->priv;

    busy_t = hal_sys_timer_get();
    while ((sdmmc_ip_readl(host, SDMMCIP_REG_STATUS) & SDMMCIP_REG_BUSY) && hal_sys_timer_get() < (busy_t + busy_timeout)) {
        HAL_SDMMC_TRACE(0, "[sdmmc host]busy");
    }

#ifdef HAL_SDMMC_USE_DMA
    sdmmc_ip_writel(host, SDMMCIP_REG_RINTSTS, SDMMCIP_REG_RINTSTS_CDONE |
                    SDMMCIP_REG_RINTSTS_RTO | SDMMCIP_REG_RINTSTS_RE); //clear command flag
#else
    sdmmc_ip_writel(host, SDMMCIP_REG_RINTSTS, SDMMCIP_REG_RINTSTS_ALL);//clear interrupt status
#endif
    sdmmc_ip_writel(host, SDMMCIP_REG_CMDARG, cmd->cmdarg);

    if (data) {
        flags = sdmmc_ip_set_transfer_mode(host, data);
    }

    if ((cmd->resp_type & MMC_RSP_136) && (cmd->resp_type & MMC_RSP_BUSY)) {
        return HAL_SDMMC_RESPONSE_BUSY;
    }

    if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION) {
        flags |= SDMMCIP_REG_CMD_ABORT_STOP;
    } else {
        flags |= SDMMCIP_REG_CMD_PRV_DAT_WAIT;
    }

    if (cmd->cmdidx == SD_CMD_SWITCH_UHS18V) {
        flags |= SDMMCIP_REG_CMD_VOLT_SWITCH;
    }

    if (cmd->resp_type & MMC_RSP_PRESENT) {
        flags |= SDMMCIP_REG_CMD_RESP_EXP;
        if (cmd->resp_type & MMC_RSP_136) {
            flags |= SDMMCIP_REG_CMD_RESP_LENGTH;
        }
    }

    if (cmd->resp_type & MMC_RSP_CRC) {
        flags |= SDMMCIP_REG_CMD_CHECK_CRC;
    }

    if (data) {
        ret = sdmmc_ip_prepare_data(host, data);
        if (ret) {
            return ret;
        }
    }

    flags |= (cmd->cmdidx | SDMMCIP_REG_CMD_START | SDMMCIP_REG_CMD_USE_HOLD_REG);
    sdmmc_ip_writel(host, SDMMCIP_REG_CMD, flags);
    for (i = 0; (i < retry) && (cmd->cmdidx != SD_CMD_SWITCH_UHS18V); i++) {
        mask = sdmmc_ip_readl(host, SDMMCIP_REG_RINTSTS);
        if (mask & SDMMCIP_REG_RINTSTS_CDONE) {
            sdmmc_ip_writel(host, SDMMCIP_REG_RINTSTS, SDMMCIP_REG_RINTSTS_CDONE);
            break;
        }
    }

    if (i == retry) {
        HAL_SDMMC_TRACE(1, "%s: Timeout.", __FUNCTION__);

        sdmmc_ip_reset_fifo(host);
#ifdef HAL_SDMMC_USE_DMA
        if (data) {
            sdmmc_ip_reset_dma(host);
        }
#endif
        return HAL_SDMMC_CMD_SEND_TIMEOUT;
    }

    if (mask & SDMMCIP_REG_RINTSTS_RTO) {
        /*
         * Timeout here is not necessarily fatal. (e)MMC cards
         * will splat here when they receive CMD55 as they do
         * not support this command and that is exactly the way
         * to tell them apart from SD cards. Thus, this output
         * below shall be TRACE(). eMMC cards also do not favor
         * CMD8, please keep that in mind.
         */
        HAL_SDMMC_TRACE(1, "%s: Response Timeout.", __FUNCTION__);

        sdmmc_ip_reset_fifo(host);
#ifdef HAL_SDMMC_USE_DMA
        if (data) {
            sdmmc_ip_reset_dma(host);
        }
#endif
        return HAL_SDMMC_COMM_TIMEOUT;
        //return HAL_SDMMC_RESPONSE_TIMEOUT;
    } else if (mask & SDMMCIP_REG_RINTSTS_RE) {
        HAL_SDMMC_TRACE(1, "%s: Response Error.", __FUNCTION__);

        sdmmc_ip_reset_fifo(host);
#ifdef HAL_SDMMC_USE_DMA
        if (data) {
            sdmmc_ip_reset_dma(host);
        }
#endif
        return HAL_SDMMC_RESPONSE_ERR;
    }

    if (cmd->resp_type & MMC_RSP_PRESENT) {
        if (cmd->resp_type & MMC_RSP_136) {
            cmd->response[0] = sdmmc_ip_readl(host, SDMMCIP_REG_RESP3);
            cmd->response[1] = sdmmc_ip_readl(host, SDMMCIP_REG_RESP2);
            cmd->response[2] = sdmmc_ip_readl(host, SDMMCIP_REG_RESP1);
            cmd->response[3] = sdmmc_ip_readl(host, SDMMCIP_REG_RESP0);
        } else {
            cmd->response[0] = sdmmc_ip_readl(host, SDMMCIP_REG_RESP0);
        }
    }

    //Processing after the completion of the transfer process: use dma
#ifdef HAL_SDMMC_USE_DMA
    if (data) {
        while (host->sdmmc_dma_lock) {
            hal_sdmmc_delay(1);
        }
    }
#else
    //Processing after the completion of the transfer process: do not use dma
    if (data) {
        i = 0;
        while (1) {
            mask = sdmmc_ip_readl(host, SDMMCIP_REG_RINTSTS);
            if (mask & (SDMMCIP_REG_DATA_ERR | SDMMCIP_REG_DATA_TOUT)) {
                HAL_SDMMC_TRACE(1, "%s: READ DATA ERROR!", __FUNCTION__);
                ret = HAL_SDIO_READ_DATA_ERR;
                goto out;
            }
            status = sdmmc_ip_readl(host, SDMMCIP_REG_STATUS);
            if (data->flags == MMC_DATA_READ) {
                if (status & SDMMCIP_REG_FIFO_COUNT_MASK) {
                    fifo_data = sdmmc_ip_readl(host, SDMMCIP_REG_FIFO_OFFSET);
                    //HAL_SDMMC_TRACE(3,"%s: count %d, read -> 0x%x", __FUNCTION__, i, fifo_data);
                    /* FIXME: now we just deal with 32bit width fifo one time */
                    if (i < data->blocks * data->blocksize) {
                        memcpy(data->dest + i, &fifo_data, sizeof(fifo_data));
                        i += sizeof(fifo_data);
                    } else {
                        HAL_SDMMC_TRACE(1, "%s: fifo data too much", __FUNCTION__);
                        ret = HAL_SDIO_FIFO_OVERFLOW;
                        goto out;
                    }
                }
                /* nothing to read from fifo and DTO is set */
                else if (mask & SDMMCIP_REG_RINTSTS_DTO) {
                    if (i != data->blocks * data->blocksize) {
                        HAL_SDMMC_TRACE(3, "%s: need to read %d, actually read %d", __FUNCTION__, data->blocks * data->blocksize, i);
                    }
                    ret = HAL_SDMMC_ERR_NONE;
                    goto out;
                }
            } else {
                /* nothing to write to fifo and DTO is set */
                if (mask & SDMMCIP_REG_RINTSTS_DTO) {
                    /* check if number is right */
                    if (i != data->blocks * data->blocksize) {
                        HAL_SDMMC_TRACE(3, "%s: need to write %d, actually written %d", __FUNCTION__, data->blocks * data->blocksize, i);
                    }
                    ret = HAL_SDMMC_ERR_NONE;
                    goto out;
                } else if (!(status & SDMMCIP_REG_FIFO_COUNT_MASK)) {
                    /* FIXME: now we just deal with 32bit width fifo one time */
                    if (i < data->blocks * data->blocksize) {
                        memcpy(&fifo_data, data->src + i, sizeof(fifo_data));
                        //HAL_SDMMC_TRACE(4,"%s: fifo %d, count %d, write -> 0x%x", __FUNCTION__, ((status & SDMMCIP_REG_FIFO_COUNT_MASK)>>SDMMCIP_REG_FIFO_COUNT_SHIFT), i, fifo_data);
                        i += sizeof(fifo_data);
                        sdmmc_ip_writel(host, SDMMCIP_REG_FIFO_OFFSET, fifo_data);
                    } else {
                        HAL_SDMMC_TRACE(1, "%s: no data to write to fifo, do nothing", __FUNCTION__);
                    }
                }
            }
        }
    }
#endif

#ifndef HAL_SDMMC_USE_DMA
out:
#endif

    return ret;
}

static int sdmmc_ip_setup_bus(struct sdmmc_ip_host *host, uint32_t freq)
{
    uint32_t div, status;
    int32_t  timeout;
    uint32_t sclk;

    if ((freq == host->clock) || (freq == 0))
        return HAL_SDMMC_ERR_NONE;
    if (host->bus_hz)
        sclk = host->bus_hz;
    else {
        HAL_SDMMC_TRACE(1, "%s: Didn't get source clock value.", __FUNCTION__);
        return HAL_SDMMC_INVALID_PARAMETER;
    }

    if (sclk <= freq)
        div = 0;    /* bypass mode */
    else
        div = __SDMMC_DIV_ROUND_UP(sclk, 2 * freq);

    HAL_SDMMC_TRACE(5, "%s: freq %d, sclk %d, reg div %d, final div %d", __FUNCTION__, freq, sclk, div, div * 2);
    host->div = div * 2;
    host->final_bus_speed = host->div ? sclk / host->div : sclk;

    sdmmc_ip_writel(host, SDMMCIP_REG_CLKENA, 0);
    sdmmc_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_PRV_DAT_WAIT |
                    SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START);

    timeout = TIMEOUT_TIME;
    do {
        status = sdmmc_ip_readl(host, SDMMCIP_REG_CMD);
        if (timeout-- < 0) {
            HAL_SDMMC_TRACE(2, "%s:%d: Timeout!", __FUNCTION__, __LINE__);
            return HAL_SDMMC_CMD_START_TIMEOUT1;
        }
    } while (status & SDMMCIP_REG_CMD_START);

    sdmmc_ip_writel(host, SDMMCIP_REG_CLKDIV, div);
    sdmmc_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_PRV_DAT_WAIT |
                    SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START);

    timeout = TIMEOUT_TIME;
    do {
        status = sdmmc_ip_readl(host, SDMMCIP_REG_CMD);
        if (timeout-- < 0) {
            HAL_SDMMC_TRACE(2, "%s:%d: Timeout!", __FUNCTION__, __LINE__);
            return HAL_SDMMC_CMD_START_TIMEOUT2;
        }
    } while (status & SDMMCIP_REG_CMD_START);

    sdmmc_ip_writel(host, SDMMCIP_REG_CLKENA, SDMMCIP_REG_CLKEN_ENABLE |
                    SDMMCIP_REG_CLKEN_LOW_PWR);
    sdmmc_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_PRV_DAT_WAIT |
                    SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START);

    timeout = TIMEOUT_TIME;
    do {
        status = sdmmc_ip_readl(host, SDMMCIP_REG_CMD);
        if (timeout-- < 0) {
            HAL_SDMMC_TRACE(2, "%s:%d: Timeout!", __FUNCTION__, __LINE__);
            return HAL_SDMMC_CMD_START_TIMEOUT3;
        }
    } while (status & SDMMCIP_REG_CMD_START);

    host->clock = freq;

    return HAL_SDMMC_ERR_NONE;
}

static int sdmmc_ip_clk_disable_low_power(struct sdmmc_ip_host *host)
{
    uint32_t status;
    int32_t  timeout;

    sdmmc_ip_writel(host, SDMMCIP_REG_CLKENA, 0);
    sdmmc_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_PRV_DAT_WAIT |
                    SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START);

    timeout = TIMEOUT_TIME;
    do {
        status = sdmmc_ip_readl(host, SDMMCIP_REG_CMD);
        if (timeout-- < 0) {
            HAL_SDMMC_TRACE(2, "%s:%d: Timeout!", __FUNCTION__, __LINE__);
            return HAL_SDMMC_CMD_START_TIMEOUT4;
        }
    } while (status & SDMMCIP_REG_CMD_START);

    sdmmc_ip_writel(host, SDMMCIP_REG_CLKENA, SDMMCIP_REG_CLKEN_ENABLE);
    sdmmc_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_PRV_DAT_WAIT |
                    SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START);

    timeout = TIMEOUT_TIME;
    do {
        status = sdmmc_ip_readl(host, SDMMCIP_REG_CMD);
        if (timeout-- < 0) {
            HAL_SDMMC_TRACE(2, "%s:%d: Timeout!", __FUNCTION__, __LINE__);
            return HAL_SDMMC_CMD_START_TIMEOUT5;
        }
    } while (status & SDMMCIP_REG_CMD_START);

    return HAL_SDMMC_ERR_NONE;
}

static int sdmmc_ip_set_ios(struct mmc *mmc)
{
    int ret = HAL_SDMMC_ERR_NONE;
    struct sdmmc_ip_host *host = (struct sdmmc_ip_host *)mmc->priv;
    uint32_t ctype, regs;

    HAL_SDMMC_TRACE(3, "%s, Buswidth = %d, clock: %d", __FUNCTION__, mmc->bus_width, mmc->clock);

    if (mmc->clock) {
        ret = sdmmc_ip_setup_bus(host, mmc->clock);
    }

    if (mmc->bus_width) {
        switch (mmc->bus_width) {
            case SDMMC_BUS_WIDTH_8:
                ctype = SDMMCIP_REG_CTYPE_8BIT;
                break;
            case SDMMC_BUS_WIDTH_4:
                ctype = SDMMCIP_REG_CTYPE_4BIT;
                break;
            default:
                ctype = SDMMCIP_REG_CTYPE_1BIT;
                break;
        }

        sdmmc_ip_writel(host, SDMMCIP_REG_CTYPE, ctype);
        regs = sdmmc_ip_readl(host, SDMMCIP_REG_UHS_REG);
        if (mmc->ddr_mode)
            regs |= SDMMCIP_REG_DDR_MODE;
        else
            regs &= ~SDMMCIP_REG_DDR_MODE;
        sdmmc_ip_writel(host, SDMMCIP_REG_UHS_REG, regs);
    }

    return ret;
}

SRAM_TEXT_LOC static void sdmmc_base_irq_handler(enum HAL_SDMMC_ID_T id)
{
    uint32_t i;
    uint32_t branch;
    uint32_t raw_int_status;
    struct sdmmc_ip_host *host = &sdmmc_host[id];

    raw_int_status = sdmmc_ip_readl(host, SDMMCIP_REG_RINTSTS);//read raw interrupt status
    sdmmc_ip_writel(host, SDMMCIP_REG_RINTSTS, raw_int_status & (~SDMMCIP_REG_RINTSTS_CDONE)); //clear interrupt status
    __DSB();
    HAL_SDMMC_TRACE(3, "%s:%d,raw_int_status=0x%X", __FUNCTION__, __LINE__, raw_int_status);

#if 0
    uint32_t int_status;
    int_status = sdmmc_ip_readl(host, SDMMCIP_REG_RINTSTS);
    HAL_SDMMC_TRACE(3, "++++++int_status=0x%X", int_status);
#endif

    //Clear the redundant signs, because they are not used, the purpose is to reduce the number of subsequent cycles
    raw_int_status &= (~(SDMMCIP_REG_RINTSTS_CDONE | SDMMCIP_REG_RINTSTS_TXDR | SDMMCIP_REG_RINTSTS_RXDR |
                         SDMMCIP_REG_INTMSK_SDIO_FUNC1 | SDMMCIP_REG_INTMSK_SDIO_FUNC2 | SDMMCIP_REG_INTMSK_SDIO_FUNC3 |
                         SDMMCIP_REG_INTMSK_SDIO_FUNC4 | SDMMCIP_REG_INTMSK_SDIO_FUNC5 | SDMMCIP_REG_INTMSK_SDIO_FUNC6 |
                         SDMMCIP_REG_INTMSK_SDIO_FUNC7));

    while (raw_int_status) {
        i = get_lsb_pos(raw_int_status);
        branch = raw_int_status & (1 << i);
        raw_int_status &= ~(1 << i);
        switch (branch) {
            case SDMMCIP_REG_RINTSTS_DTO: {
                HAL_SDMMC_TRACE(0, "bit 3:Data transfer over (DTO)");
                if (sdmmc_callback[id]->hal_sdmmc_txrx_done) {
                    sdmmc_callback[id]->hal_sdmmc_txrx_done();
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_EBE: {
                HAL_SDMMC_TRACE(0, "bit 15:End-bit error (read)/write no CRC (EBE)");
                if (sdmmc_callback[id]->hal_sdmmc_host_error) {
                    sdmmc_callback[id]->hal_sdmmc_host_error(HAL_SDMMC_HOST_READ_END_BIT_ERR_WRITE_NOCRC);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_SBE: {
                HAL_SDMMC_TRACE(0, "bit 13:Start-bit error (SBE)");
                if (sdmmc_callback[id]->hal_sdmmc_host_error) {
                    sdmmc_callback[id]->hal_sdmmc_host_error(HAL_SDMMC_HOST_START_BIT_ERR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_HLE: {
                HAL_SDMMC_TRACE(0, "bit 12:Hardware locked write error (HLE)");
                if (sdmmc_callback[id]->hal_sdmmc_host_error) {
                    sdmmc_callback[id]->hal_sdmmc_host_error(HAL_SDMMC_HOST_HARDWARE_LOCKED_WRITE_ERR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_FRUN: {
                sdmmc_ip_reset_fifo(host);
                HAL_SDMMC_TRACE(0, "bit 11:FIFO underrun/overrun error (FRUN)");
                if (sdmmc_callback[id]->hal_sdmmc_host_error) {
                    sdmmc_callback[id]->hal_sdmmc_host_error(HAL_SDMMC_HOST_FIFO_ERR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_HTO: {
                HAL_SDMMC_TRACE(0, "bit 10:Data starvation-by-host timeout (HTO)");
                if (sdmmc_callback[id]->hal_sdmmc_host_error) {
                    sdmmc_callback[id]->hal_sdmmc_host_error(HAL_SDMMC_HOST_STARVATION_TIMEOUT);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_DRTO: {
                HAL_SDMMC_TRACE(0, "bit 9:Data read timeout (DRTO)/Boot Data Start (BDS)");
                if (sdmmc_callback[id]->hal_sdmmc_host_error) {
                    sdmmc_callback[id]->hal_sdmmc_host_error(HAL_SDMMC_HOST_DATA_READ_TIMEOUT_BDS);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_RTO: {
                HAL_SDMMC_TRACE(0, "bit 8:Response timeout (RTO)/Boot Ack Received (BAR)");
                if (sdmmc_callback[id]->hal_sdmmc_host_error) {
                    sdmmc_callback[id]->hal_sdmmc_host_error(HAL_SDMMC_HOST_RESPONSE_TIMEOUT_BAR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_DCRC: {
                HAL_SDMMC_TRACE(0, "bit 7:Data CRC error (DCRC)");
                if (sdmmc_callback[id]->hal_sdmmc_host_error) {
                    sdmmc_callback[id]->hal_sdmmc_host_error(HAL_SDMMC_HOST_DATA_CRC_ERR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_RCRC: {
                HAL_SDMMC_TRACE(0, "bit 6:Response CRC error (RCRC)");
                if (sdmmc_callback[id]->hal_sdmmc_host_error) {
                    sdmmc_callback[id]->hal_sdmmc_host_error(HAL_SDMMC_HOST_RESPONSE_CRC_ERR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_RE: {
                HAL_SDMMC_TRACE(0, "bit 1:Response error (RE)");
                if (sdmmc_callback[id]->hal_sdmmc_host_error) {
                    sdmmc_callback[id]->hal_sdmmc_host_error(HAL_SDMMC_HOST_RESPONSE_ERR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_CD: {
                //ip does not support, detect signal is tie to 1, 2022-06-02
                HAL_SDMMC_TRACE(0, "bit 0:Card detect (RE)");
                break;
            }
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC1:
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC2:
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC3:
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC4:
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC5:
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC6:
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC7: {
                HAL_SDMMC_TRACE(0, "bit 16:funcX int");
                break;
            }
            default : {
                break;
            }
        }
    }
}

static void hal_sdmmc0_irq_handler(void)
{
    sdmmc_base_irq_handler(HAL_SDMMC_ID_0);
}

#ifdef SDMMC1_BASE
static void hal_sdmmc1_irq_handler(void)
{
    sdmmc_base_irq_handler(HAL_SDMMC_ID_1);
}
#endif

static int hal_sdmmc_host_int_enable(struct sdmmc_ip_host *host)
{
    uint32_t val;
    uint32_t irq_num = SDMMC0_IRQn;

    HAL_SDMMC_TRACE(2, "%s:%d", __FUNCTION__, __LINE__);
    sdmmc_ip_writel(host, SDMMCIP_REG_RINTSTS, SDMMCIP_REG_RINTSTS_ALL); //clear interrupt status

    val = ~(SDMMCIP_REG_INTMSK_CDONE | SDMMCIP_REG_INTMSK_ACD | SDMMCIP_REG_INTMSK_TXDR |
            SDMMCIP_REG_INTMSK_RXDR | SDMMCIP_REG_INTMSK_RTO | SDMMCIP_REG_INTMSK_SDIO_FUNC1 |
            SDMMCIP_REG_INTMSK_SDIO_FUNC2 | SDMMCIP_REG_INTMSK_SDIO_FUNC3 | SDMMCIP_REG_INTMSK_SDIO_FUNC4 |
            SDMMCIP_REG_INTMSK_SDIO_FUNC5 | SDMMCIP_REG_INTMSK_SDIO_FUNC6 | SDMMCIP_REG_INTMSK_SDIO_FUNC7);
    if (!sdmmc_callback[host->host_id]->hal_sdmmc_txrx_done) {
        val &= ~SDMMCIP_REG_RINTSTS_DTO;
    }
    sdmmc_ip_writel(host, SDMMCIP_REG_INTMASK, SDMMCIP_REG_INTMSK_ALL & val);   //open interrupt except for values in parentheses
    sdmmc_ip_writel(host, SDMMCIP_REG_CTRL, SDMMCIP_REG_INT_EN);                //enable interrupt

    if (host->host_id == HAL_SDMMC_ID_0) {
        irq_num = SDMMC0_IRQn;
        NVIC_SetVector(irq_num, (uint32_t)hal_sdmmc0_irq_handler);
    }
#ifdef SDMMC1_BASE
    else if (host->host_id == HAL_SDMMC_ID_1) {
        irq_num = SDMMC1_IRQn;
        NVIC_SetVector(irq_num, (uint32_t)hal_sdmmc1_irq_handler);
    }
#endif

    NVIC_SetPriority(irq_num, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(irq_num);
    NVIC_EnableIRQ(irq_num);

    return HAL_SDMMC_ERR_NONE;
}

static int sdmmc_ip_init(struct mmc *mmc)
{
    struct sdmmc_ip_host *host = mmc->priv;

    sdmmc_ip_writel(host, SDMMCIP_REG_PWREN, 1);
    HAL_SDMMC_TRACE(2, "%s, host->ioaddr:0x%X, ip_pwr:%d", __func__, (uint32_t)host->ioaddr, sdmmc_ip_readl(host, SDMMCIP_REG_PWREN));

    if (sdmmc_ip_wait_reset(host, SDMMCIP_REG_RESET_ALL)) {
        HAL_SDMMC_ASSERT(0, "%s:%d,Fail-reset!!", __FUNCTION__, __LINE__);
        return HAL_SDMMC_RESET_FAIL;
    }

    sdmmc_ip_writel(host, SDMMCIP_REG_INTMASK, ~SDMMCIP_REG_INTMSK_ALL); //disable all int
    sdmmc_ip_writel(host, SDMMCIP_REG_TMOUT, SDMMCIP_REG_RINTSTS_ALL);   //modify response timeout value for maximum
    sdmmc_ip_writel(host, SDMMCIP_REG_IDINTEN, 0);   //disable internal DMAC interrupt
    sdmmc_ip_writel(host, SDMMCIP_REG_BMOD, 1);      //software reset internal DMA controller
    sdmmc_ip_writel(host, SDMMCIP_REG_FIFOTH, host->fifoth_val);

    return hal_sdmmc_host_int_enable(host);
}

static int sdmmc_ip_wait_dat0(struct mmc *mmc, int state, int timeout_us)
{
    uint32_t val;
    struct sdmmc_ip_host *host = mmc->priv;
    uint32_t busy_timeout = US_TO_TICKS(timeout_us), busy_t = 0;

    HAL_SDMMC_TRACE(2, "timeout_us:%d,busy_timeout:%d", timeout_us, busy_timeout);
    busy_t = hal_sys_timer_get();
    do {
        val = sdmmc_ip_readl(host, SDMMCIP_REG_STATUS);
        if (!(val & SDMMCIP_REG_BUSY) ==  !!state) {
            return HAL_SDMMC_ERR_NONE;
        }
    } while (hal_sys_timer_get() < (busy_t + busy_timeout));

    return HAL_SDMMC_WAIT_DAT0_TIMEOUT;
}

static const struct mmc_ops sdmmc_ip_ops = {
    .send_cmd           = sdmmc_ip_send_cmd,
    .set_ios            = sdmmc_ip_set_ios,
    .init               = sdmmc_ip_init,
    .wait_dat0          = sdmmc_ip_wait_dat0,
    .getcd              = NULL,
    .getwp              = NULL,
    .host_power_cycle   = NULL,
    .get_b_max          = NULL,
};

static struct mmc *mmc_create(enum HAL_SDMMC_ID_T id, const struct mmc_config *cfg, void *priv)
{
    struct blk_desc *bdesc;
    struct mmc *mmc;

    /* quick validation */
    if (cfg == NULL || cfg->f_min == 0 ||
        cfg->f_max == 0 || cfg->b_max == 0)
        return NULL;

#ifndef DM_MMC
    if (cfg->ops == NULL || cfg->ops->send_cmd == NULL)
        return NULL;
#endif

    mmc = find_mmc_device(id);
    if (mmc == NULL)
        return NULL;

    mmc->cfg = cfg;
    mmc->priv = priv;

    /* the following chunk was mmc_register() */

    /* Setup dsr related values */
    mmc->dsr_imp = 0;
    mmc->dsr = 0xffffffff;
    /* Setup the universal parts of the block interface just once */
    bdesc = mmc_get_blk_desc(mmc);
    bdesc->if_type = IF_TYPE_MMC;
    bdesc->removable = 1;
    bdesc->devnum = id;
    bdesc->block_read = mmc_bread;
    bdesc->block_write = mmc_bwrite;
    bdesc->block_erase = mmc_berase;

    /* setup initial part type */
    bdesc->part_type = mmc->cfg->part_type;

    return mmc;
}

static int hal_sdmmc_host_device_cfg(enum HAL_SDMMC_ID_T id, struct HAL_SDMMC_CB_T *callback, bool device_init)
{
    int ret = 0;
    uint32_t bus_clk;
    struct sdmmc_ip_host *host = NULL;
    HAL_SDMMC_ASSERT(id < HAL_SDMMC_ID_NUM, "Invalid sdmmc host id: %d", id);

#ifndef SDMMC_SPEED
#define SDMMC_SPEED      (12 * 1000 * 1000)
#endif

#ifdef FPGA
    switch (id) {
        case HAL_SDMMC_ID_0:
            hal_cmu_sdmmc0_set_freq(HAL_CMU_PERIPH_FREQ_26M);
            break;
#ifdef SDMMC1_BASE
        case HAL_SDMMC_ID_1:
            hal_cmu_sdmmc1_set_freq(HAL_CMU_PERIPH_FREQ_26M);
            break;
#endif
        default:
            break;
    }

    bus_clk = hal_cmu_get_crystal_freq();
    HAL_SDMMC_ASSERT(bus_clk >= SDMMC_SPEED, "%s:%d, SDMMC_SPEED %d > src clk %d", __FUNCTION__, __LINE__, SDMMC_SPEED, bus_clk);
#else
    if (0) {
#ifdef PERIPH_PLL_FREQ
    } else if (SDMMC_SPEED > 2 * hal_cmu_get_crystal_freq()) {
        uint32_t div;//division is one step in place, the sdmmc ip is no longer divided(Support even division)

        div = PERIPH_PLL_FREQ / SDMMC_SPEED;
        if (PERIPH_PLL_FREQ % SDMMC_SPEED) {
            div += 1;
        }

        bus_clk = PERIPH_PLL_FREQ / div;
        switch (id) {
            case HAL_SDMMC_ID_0:
                ret = hal_cmu_sdmmc0_set_div(div);
                break;
#ifdef SDMMC1_BASE
            case HAL_SDMMC_ID_1:
                ret = hal_cmu_sdmmc1_set_div(div);
                break;
#endif
            default:
                break;
        }
        HAL_SDMMC_ASSERT(!ret, "The SDMMC_SPEED value is invalid, causing the div to be out of range, ret %d, div is %d", ret, div);
        HAL_SDMMC_ASSERT(bus_clk <= SDMMC_SPEED, "%s:%d, div clk %d > SDMMC_SPEED %d", __FUNCTION__, __LINE__, bus_clk, SDMMC_SPEED);
        HAL_SDMMC_TRACE(1, "PERIPH_PLL_FREQ is %d, the final PLL div is %d", PERIPH_PLL_FREQ, div);
#endif
    } else if (SDMMC_SPEED >= 3 * hal_cmu_get_crystal_freq() / 2) {  //sdmmc ip may also be divided(Only even division)
        switch (id) {
            case HAL_SDMMC_ID_0:
                hal_cmu_sdmmc0_set_freq(HAL_CMU_PERIPH_FREQ_52M);
                break;
#ifdef SDMMC1_BASE
            case HAL_SDMMC_ID_1:
                hal_cmu_sdmmc1_set_freq(HAL_CMU_PERIPH_FREQ_52M);
                break;
#endif
            default:
                break;
        }
        bus_clk = 2 * hal_cmu_get_crystal_freq();
        HAL_SDMMC_ASSERT(bus_clk >= SDMMC_SPEED, "%s:%d, SDMMC_SPEED %d > src clk %d", __FUNCTION__, __LINE__, SDMMC_SPEED, bus_clk);
    } else {                                                        //sdmmc ip may also be divided(Only even division)
        switch (id) {
            case HAL_SDMMC_ID_0:
                hal_cmu_sdmmc0_set_freq(HAL_CMU_PERIPH_FREQ_26M);
                break;
#ifdef SDMMC1_BASE
            case HAL_SDMMC_ID_1:
                hal_cmu_sdmmc1_set_freq(HAL_CMU_PERIPH_FREQ_26M);
                break;
#endif
            default:
                break;
        }
        bus_clk = hal_cmu_get_crystal_freq();
        HAL_SDMMC_ASSERT(bus_clk >= SDMMC_SPEED, "%s:%d, SDMMC_SPEED %d > src clk %d", __FUNCTION__, __LINE__, SDMMC_SPEED, bus_clk);
    }
#endif
    HAL_SDMMC_TRACE(2, "SDMMC_SPEED %d, bus clk(Not necessarily the final speed) %d", SDMMC_SPEED, bus_clk);

    /* sdmmc host clock and iomux */
    switch (id) {
        case HAL_SDMMC_ID_0:
            hal_iomux_set_sdmmc0();
            hal_cmu_sdmmc0_clock_enable();
            break;
#ifdef SDMMC1_BASE
        case HAL_SDMMC_ID_1:
            hal_iomux_set_sdmmc1();
            hal_cmu_sdmmc1_clock_enable();
            break;
#endif
        default:
            break;
    }

#ifdef SDMMC_DDR_MODE
    uint32_t freq_max;
    freq_max = mmc_mode2freq(NULL, MMC_DDR_52);
    HAL_SDMMC_ASSERT(SDMMC_SPEED <= freq_max, "%s:%d, mmc ddr50 speed %d > max speed %d", __FUNCTION__, __LINE__, SDMMC_SPEED, freq_max);
#endif

    host = &sdmmc_host[id];
    host->ioaddr     = (void *)sdmmc_ip_base[id];
    host->clock      = 0;
    host->bus_hz     = bus_clk;
    host->host_id    = id;
    host->buswidth   = 4;
    host->fifoth_val = MSIZE(0) | RX_WMARK(0) | TX_WMARK(1);

    host->cfg.ops = &sdmmc_ip_ops;
    host->cfg.voltages = MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30 | MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33 | MMC_VDD_165_195;
    host->cfg.f_min = 400 * 1000;     //Don't modify
    host->cfg.f_max = SDMMC_SPEED;
    host->cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;
    host->mmc = mmc_create(host->host_id, &host->cfg, host);
    HAL_SDMMC_TRACE(4, "%s:%d, sdmmc clk min %d, max %d", __FUNCTION__, __LINE__, host->cfg.f_min, host->cfg.f_max);

#ifdef HAL_SDMMC_USE_DMA
    host->dma_ch = 0;
    host->dma_in_use = 0;
    host->sdmmc_dma_lock = 0;
    host->tx_dma_handler = sdmmc_ip_ext_dma_irq_handlers[id * 2];
    host->rx_dma_handler = sdmmc_ip_ext_dma_irq_handlers[id * 2 + 1];
#endif

    host->cfg.host_caps = 0;
    if (host->buswidth == 8) {
        host->cfg.host_caps |= MMC_MODE_8BIT;
    } else if (host->buswidth == 4) {
        host->cfg.host_caps |= MMC_MODE_4BIT;
    } else {
        host->cfg.host_caps |= MMC_MODE_4BIT;
    }

#ifdef SDMMC_DDR_MODE
    host->cfg.host_caps |= MMC_MODE_DDR_52MHz;
#endif

    switch (id) {
        case HAL_SDMMC_ID_0:
            host->cfg.host_caps |= MMC_MODE_HS | MMC_MODE_HS_52MHz;
            break;
#ifdef SDMMC1_BASE
        case HAL_SDMMC_ID_1:
            host->cfg.host_caps |= MMC_MODE_HS | MMC_MODE_HS_52MHz | UHS_CAPS;
            break;
#endif
        default:
            break;
    }

    //Calculate period at start frequency
    uint32_t sdmmc_start_freq;
    sdmmc_start_freq = host->bus_hz / hal_cmu_get_crystal_freq();
    sdmmc_start_freq *= hal_cmu_get_crystal_freq();
    sdmmc_start_freq = sdmmc_start_freq / 1000 / 1000;
    host->period_st_ns = (1000 * host->div) / sdmmc_start_freq;
    HAL_SDMMC_TRACE(1, "sdmmc period of start frequency:%dns", host->period_st_ns);

    if (host->mmc->has_init) {
        HAL_SDMMC_TRACE(2, "%s:%d,sdmmc host has been initialized", __FUNCTION__, __LINE__);
        return HAL_SDMMC_ERR_NONE;
    }
    if (!host->mmc->init_in_progress) {
        host->mmc->init_in_progress = 1;

        if (device_init) {
            sdmmc_callback[id] = &sdmmc_callback_default[id];
            memset(sdmmc_callback[id], 0, sizeof(struct HAL_SDMMC_CB_T));
            HAL_SDMMC_TRACE(0, "default callback[%d] addr:0x%X", id, (uint32_t)sdmmc_callback[id]);
            if (callback) {
                HAL_SDMMC_TRACE(0, "new callback addr:0x%X", (uint32_t)callback);
                sdmmc_callback[id] = callback;
            }

            mmc_set_preinit(host->mmc, 1);
            if (mmc_initialize(host->mmc)) {
                HAL_SDMMC_TRACE(1, "%s failed", __func__);
            }
            ret = mmc_init(host->mmc);
            if (ret) {
                return ret;
            }
        } else {
            //config host:UHS-I mode
            mmc_set_bus_width(host->mmc, SDMMC_BUS_WIDTH_4);
            mmc_set_clock(host->mmc, host->mmc->cfg->f_max, false);

            //Disable the function of stopping output when clk is idle
            sdmmc_ip_clk_disable_low_power(host);
        }
        HAL_SDMMC_TRACE(2, "%s:%d,sdmmc device init complete", __FUNCTION__, __LINE__);
        HAL_SDMMC_TRACE(0, "  ");
        HAL_SDMMC_TRACE(0, "  ");
    } else {
        HAL_SDMMC_TRACE(2, "%s:%d,sdmmc host is being initialized", __FUNCTION__, __LINE__);
    }

    return ret;
}
/*************************external interface function**************************/
POSSIBLY_UNUSED static const char *const invalid_id = "Invalid sdmmc id: %d";

HAL_SDMMC_DELAY_FUNC hal_sdmmc_set_delay_func(HAL_SDMMC_DELAY_FUNC new_func)
{
    HAL_SDMMC_DELAY_FUNC old_func = sdmmc_delay;
    sdmmc_delay = new_func;
    return old_func;
}

uint32_t hal_sdmmc_read_blocks(enum HAL_SDMMC_ID_T id, uint32_t start_block, uint32_t block_count, uint8_t *dest)
{
    struct mmc *mmc = sdmmc_host[id].mmc;

    HAL_SDMMC_ASSERT(id < HAL_SDMMC_ID_NUM, invalid_id, id);
#ifdef HAL_SDMMC_USE_DMA
    HAL_SDMMC_ASSERT((((uint64_t)(block_count * MMC_MAX_BLOCK_LEN)) / SDMMC_DMA_LINK_SIZE) < (SDMMC_DMA_DESC_CNT - 1), "%s:%d,SDMMC_DMA_DESC_CNT is too small", __func__, __LINE__);
#endif

    return (uint32_t)mmc->block_dev.block_read(&mmc->block_dev, start_block, block_count, dest);
}

uint32_t hal_sdmmc_write_blocks(enum HAL_SDMMC_ID_T id, uint32_t start_block, uint32_t block_count, uint8_t *src)
{
    struct mmc *mmc = sdmmc_host[id].mmc;

    HAL_SDMMC_ASSERT(id < HAL_SDMMC_ID_NUM, invalid_id, id);
#ifdef HAL_SDMMC_USE_DMA
    HAL_SDMMC_ASSERT((((uint64_t)(block_count * MMC_MAX_BLOCK_LEN)) / SDMMC_DMA_LINK_SIZE) < (SDMMC_DMA_DESC_CNT - 1), "%s:%d,SDMMC_DMA_DESC_CNT is too small", __func__, __LINE__);
#endif

    return (uint32_t)mmc->block_dev.block_write(&mmc->block_dev, start_block, block_count, src);
}

int hal_sdmmc_open_ext(enum HAL_SDMMC_ID_T id, struct HAL_SDMMC_CB_T *callback)
{
    return hal_sdmmc_host_device_cfg(id, callback, 1);
}

int hal_sdmmc_open(enum HAL_SDMMC_ID_T id)
{
    return hal_sdmmc_host_device_cfg(id, NULL, 1);
}

void hal_sdmmc_close(enum HAL_SDMMC_ID_T id)
{
    struct sdmmc_ip_host *host = NULL;
    HAL_SDMMC_ASSERT(id < HAL_SDMMC_ID_NUM, "Invalid sdmmc host id: %d", id);
    HAL_SDMMC_TRACE(2, "%s:%d", __FUNCTION__, __LINE__);

    host = &sdmmc_host[id];
    if (host->mmc == NULL) {
        return;
    }

    if (host->mmc->has_init == 0) {
        HAL_SDMMC_TRACE(2, "%s:%d,sdmmc host has been closed", __FUNCTION__, __LINE__);
        return;
    }

    while (host->mmc->init_in_progress)
        hal_sys_timer_delay_us(500);

    host->mmc->has_init = 0;

#if defined(MMC_UHS_SUPPORT) || \
    defined(MMC_HS200_SUPPORT) || \
    defined(MMC_HS400_SUPPORT)
    mmc_deinit(host->mmc);
#endif

#ifdef HAL_SDMMC_USE_DMA
    if (host->dma_in_use) {
        sdmmc_ip_reset_dma(host);
    }
#endif

    switch (id) {
        case HAL_SDMMC_ID_0:
            hal_cmu_sdmmc0_clock_disable();
            break;
#ifdef SDMMC1_BASE
        case HAL_SDMMC_ID_1:
            hal_cmu_sdmmc1_clock_disable();
            break;
#endif
        default:
            break;
    }
}

int hal_sdmmc_get_status(enum HAL_SDMMC_ID_T id)
{
    struct sdmmc_ip_host *host = NULL;

    host = &sdmmc_host[id];
    if (host->mmc == NULL) {
        return HAL_SDMMC_ERR_NONE;
    }

    return host->mmc->has_init;
}

uint32_t hal_sdmmc_get_bus_speed(enum HAL_SDMMC_ID_T id)
{
    if (hal_sdmmc_get_status(id)) {
        return sdmmc_host[id].final_bus_speed;
    } else {
        return HAL_SDMMC_ERR_NONE;
    }
}

void hal_sdmmc_dump(enum HAL_SDMMC_ID_T id)
{
    struct mmc *mmc = sdmmc_host[id].mmc;
    HAL_SDMMC_ASSERT(id < HAL_SDMMC_ID_NUM, invalid_id, id);

    TRACE(0, "-----------hal_sdmmc_dump-------------");
    TRACE(1, "[sdmmc id]                       : %d", id);
    TRACE(1, "[io register address]            : 0x%x", sdmmc_ip_base[id]);
    TRACE(1, "[ddr mode]                       : %d", mmc->ddr_mode);
    switch (mmc->version) {
        case SD_VERSION_1_0:
            TRACE(0, "[version]                        : SD_VERSION_1_0");
            break;
        case SD_VERSION_1_10:
            TRACE(0, "[version]                        : SD_VERSION_1_10");
            break;
        case SD_VERSION_2:
            TRACE(0, "[version]                        : SD_VERSION_2");
            break;
        case SD_VERSION_3:
            TRACE(0, "[version]                        : SD_VERSION_3");
            break;
        case MMC_VERSION_5_0:
            TRACE(0, "[version]                        : MMC_VERSION_5_0");
            break;
        case MMC_VERSION_4_5:
            TRACE(0, "[version]                        : MMC_VERSION_4_5");
            break;
        case MMC_VERSION_4_41:
            TRACE(0, "[version]                        : MMC_VERSION_4_41");
            break;
        case MMC_VERSION_4_3:
            TRACE(0, "[version]                        : MMC_VERSION_4_3");
            break;
        case MMC_VERSION_4_2:
            TRACE(0, "[version]                        : MMC_VERSION_4_2");
            break;
        case MMC_VERSION_4_1:
            TRACE(0, "[version]                        : MMC_VERSION_4_1");
            break;
        case MMC_VERSION_4:
            TRACE(0, "[version]                        : MMC_VERSION_4");
            break;
        case MMC_VERSION_3:
            TRACE(0, "[version]                        : MMC_VERSION_3");
            break;
        case MMC_VERSION_2_2:
            TRACE(0, "[version]                        : MMC_VERSION_2_2");
            break;
        case MMC_VERSION_1_4:
            TRACE(0, "[version]                        : MMC_VERSION_1_4");
            break;
        case MMC_VERSION_1_2:
            TRACE(0, "[version]                        : MMC_VERSION_1_2");
            break;
        default:
            TRACE(0, "[version]                        : unkown version");
            break;
    }
    TRACE(1, "[is SD card]                     : 0x%x", IS_SD(mmc));
    TRACE(1, "[high_capacity]                  : 0x%x", mmc->high_capacity);
    TRACE(1, "[bus_width]                      : 0x%x", mmc->bus_width);
    TRACE(1, "[clock]                          : %d", mmc->clock);
    TRACE(1, "[card_caps]                      : 0x%x", mmc->card_caps);
    TRACE(1, "[ocr]                            : 0x%x", mmc->ocr);
    TRACE(1, "[dsr]                            : 0x%x", mmc->dsr);
    TRACE(1, "[capacity_user/1024]             : %d(K)", (uint32_t)(mmc->capacity_user / 1024));
    TRACE(1, "[capacity_user/1024/1024]        : %d(M)", (uint32_t)(mmc->capacity_user / 1024 / 1024));
    TRACE(1, "[capacity_user/1024/1024/1024]   : %d(G)", (uint32_t)(mmc->capacity_user / 1024 / 1024 / 1024));
    TRACE(1, "[read_bl_len]                    : %d", mmc->read_bl_len);
    TRACE(1, "[write_bl_len]                   : %d", mmc->write_bl_len);
    TRACE(0, "--------------------------------------");
}

void hal_sdmmc_info(enum HAL_SDMMC_ID_T id, uint32_t *sector_count, uint32_t *sector_size)
{
    struct mmc *mmc = sdmmc_host[id].mmc;
    HAL_SDMMC_ASSERT(id < HAL_SDMMC_ID_NUM, invalid_id, id);

    if (sector_size) {
        *sector_size = mmc->read_bl_len;
        HAL_SDMMC_TRACE(1, "[sdmmc] sector_size %d", *sector_size);
    }
    if (sector_count) {
        if (mmc->read_bl_len != 0)
            *sector_count = mmc->capacity_user / mmc->read_bl_len;
        else
            *sector_count = 0;
        HAL_SDMMC_TRACE(1, "[sdmmc] sector_count %d", *sector_count);
    }
}

#endif

