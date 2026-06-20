/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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

#include "hal_sdio_host.h"
#include "hal_location.h"
#include "reg_sdmmcip.h"
#include "cmsis_nvic.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_sdmmc.h"
#include "hal_dma.h"
#include "errno.h"
#include <string.h>
#ifdef RTOS
    #include "cmsis_os.h"
#endif

#ifdef SDIO_HOST_DEBUG
    #ifdef FPGA
        #define HAL_SDIO_HOST_TRACE(n, s, ...)      TRACE(n, "[%u]" s, TICKS_TO_MS(hal_sys_timer_get()), ##__VA_ARGS__)
        #define HAL_SDIO_HOST_ASSERT(n, s, ...)     ASSERT(n, "[%u]" s, TICKS_TO_MS(hal_sys_timer_get()), ##__VA_ARGS__)
    #else
        #define HAL_SDIO_HOST_TRACE(n, s, ...)      TRACE(n, s , ##__VA_ARGS__)
        #define HAL_SDIO_HOST_ASSERT(n, s, ...)     ASSERT(n, s , ##__VA_ARGS__)
    #endif
#else
    #define HAL_SDIO_HOST_TRACE(n, s, ...)          TRACE_DUMMY(n, s, ##__VA_ARGS__)
    #ifdef FPGA
        #define HAL_SDIO_HOST_ASSERT(n, s, ...)     ASSERT(n, "[%u]" s, TICKS_TO_MS(hal_sys_timer_get()), ##__VA_ARGS__)
    #else
        #define HAL_SDIO_HOST_ASSERT(n, s, ...)     ASSERT(n, s , ##__VA_ARGS__)
    #endif
#endif

/******************************************************************************/
/******************The first part: sdio host ip related************************/
/******************************************************************************/
#define HAL_SDIO_HOST_USE_DMA           1
#define SDIO_HOST_FIFO_DEPTH            16  //width 32bits

/* sdio bus speed */
#define SDIO_SPEED_DS                   25000000
#define SDIO_SPEED_HS                   50000000
#define SDIO_SPEED_SDR12                25000000
#define SDIO_SPEED_SDR25                50000000
#define SDIO_SPEED_DDR50                50000000
#define SDIO_SPEED_SDR50                100000000
#define SDIO_SPEED_SDR104               208000000

/* sdio cmd list */
#define SDIO_CMD_GO_IDLE_STATE          0   /* RSP:no */
#define SDIO_CMD_SEND_RELATIVE_ADDR     3   /* RSP:R6 */
#define SDIO_CMD_IO_SEND_OP_COND        5   /* RSP:R4 */
#define SDIO_CMD_SELECT_CARD            7   /* RSP:R1b */
#define SDIO_CMD_SWITCH_UHS18V          11  /* RSP:R1 */
#define SDIO_CMD_GO_INACTIVE_STATE      15  /* RSP:no */
#define SDIO_CMD_SEND_TUNING_BLOCK      19  /* RSP:R1 */
#define SDIO_CMD_IO_RW_DIRECT           52  /* RSP:R5 */
#define SDIO_CMD_IO_RW_EXTENDED         53  /* RSP:R5 */
//SD command
#define MMC_CMD_STOP_TRANSMISSION       12

/* rsp list */
#define SDIO_RSP_PRESENT                (1 << 0)
#define SDIO_RSP_136                    (1 << 1)    /* 136 bit response */
#define SDIO_RSP_CRC                    (1 << 2)    /* expect valid crc */
#define SDIO_RSP_BUSY                   (1 << 3)    /* card may send busy */
#define SDIO_RSP_OPCODE                 (1 << 4)    /* response contains opcode */

#define SDIO_RSP_NONE                   (0)
#define SDIO_RSP_R1                     (SDIO_RSP_PRESENT|SDIO_RSP_CRC|SDIO_RSP_OPCODE)
#define SDIO_RSP_R1b                    (SDIO_RSP_PRESENT|SDIO_RSP_CRC|SDIO_RSP_OPCODE|SDIO_RSP_BUSY)
#define SDIO_RSP_R2                     (SDIO_RSP_PRESENT|SDIO_RSP_136|SDIO_RSP_CRC)
#define SDIO_RSP_R3                     (SDIO_RSP_PRESENT)
#define SDIO_RSP_R4                     (SDIO_RSP_PRESENT)
#define SDIO_RSP_R5                     (SDIO_RSP_PRESENT|SDIO_RSP_CRC|SDIO_RSP_OPCODE)
#define SDIO_RSP_R6                     (SDIO_RSP_PRESENT|SDIO_RSP_CRC|SDIO_RSP_OPCODE)
#define SDIO_RSP_R7                     (SDIO_RSP_PRESENT|SDIO_RSP_CRC|SDIO_RSP_OPCODE)

/* OCR: VDD Voltage Window */
#define SDIO_VDD_165_195                0x00000080  /* VDD voltage 1.65 - 1.95 */
#define SDIO_VDD_20_21                  0x00000100  /* VDD voltage 2.0 ~ 2.1 */
#define SDIO_VDD_21_22                  0x00000200  /* VDD voltage 2.1 ~ 2.2 */
#define SDIO_VDD_22_23                  0x00000400  /* VDD voltage 2.2 ~ 2.3 */
#define SDIO_VDD_23_24                  0x00000800  /* VDD voltage 2.3 ~ 2.4 */
#define SDIO_VDD_24_25                  0x00001000  /* VDD voltage 2.4 ~ 2.5 */
#define SDIO_VDD_25_26                  0x00002000  /* VDD voltage 2.5 ~ 2.6 */
#define SDIO_VDD_26_27                  0x00004000  /* VDD voltage 2.6 ~ 2.7 */
#define SDIO_VDD_27_28                  0x00008000  /* VDD voltage 2.7 ~ 2.8 */
#define SDIO_VDD_28_29                  0x00010000  /* VDD voltage 2.8 ~ 2.9 */
#define SDIO_VDD_29_30                  0x00020000  /* VDD voltage 2.9 ~ 3.0 */
#define SDIO_VDD_30_31                  0x00040000  /* VDD voltage 3.0 ~ 3.1 */
#define SDIO_VDD_31_32                  0x00080000  /* VDD voltage 3.1 ~ 3.2 */
#define SDIO_VDD_32_33                  0x00100000  /* VDD voltage 3.2 ~ 3.3 */
#define SDIO_VDD_33_34                  0x00200000  /* VDD voltage 3.3 ~ 3.4 */
#define SDIO_VDD_34_35                  0x00400000  /* VDD voltage 3.4 ~ 3.5 */
#define SDIO_VDD_35_36                  0x00800000  /* VDD voltage 3.5 ~ 3.6 */

#define SDIO_DATA_READ                  1
#define SDIO_DATA_WRITE                 2

#define MAX_RETRY_CNT                   15000
#define TIMEOUT_TIME                    1000

#define __SDMMC_DIV_ROUND_UP(n,d)       (((n) + (d) - 1) / (d))

#ifndef CONFIG_SYS_MMC_MAX_BLK_COUNT
    #define CONFIG_SYS_MMC_MAX_BLK_COUNT 65535
#endif

enum SDIO_HOST_BUS_WIDTH {
    SDIO_HOST_BUS_WIDTH_1 = 1,
    SDIO_HOST_BUS_WIDTH_4 = 4,
    SDIO_HOST_BUS_WIDTH_8 = 8,
};

struct SDIO_CMD_T {
    uint16_t cmdidx;
    uint32_t resp_type;
    uint32_t cmdarg;
    uint32_t response[4];
};

struct SDIO_DATA_T {
    union {
        char *dest;
        const char *src; /* src buffers don't get written to */
    };
    uint32_t flags;
    uint32_t blocks;
    uint32_t blocksize;
};

struct SDIO_CONFIG_T {
    const struct SDIO_OPS_T *ops;
    uint32_t voltages;
    uint32_t f_min;
    uint32_t f_max;
    uint32_t b_max;
};

struct SDIO_T {
    const struct SDIO_CONFIG_T *cfg;
    void *priv;
    uint32_t has_init;
    enum SDIO_HOST_BUS_WIDTH bus_width;
    uint32_t clock;
    uint8_t op_cond_pending;   /* 1 if we are waiting on an op_cond command */
    uint8_t init_in_progress;  /* 1 if we have done mmc_start_init() */
    uint8_t ddr_mode;
};

struct SDIO_IP_HOST_T {
    uint8_t host_id;
    uint32_t clock;
    uint32_t bus_hz;
    uint32_t div;
    uint8_t bus_width;
    void *ioaddr;
    uint32_t fifoth_val;
    uint32_t period_st_ns;
    uint32_t final_bus_speed;
    volatile uint8_t volt_switch_flag;
    void *priv;

    struct SDIO_CONFIG_T cfg;
    struct SDIO_T *sdio;

#ifdef HAL_SDIO_HOST_USE_DMA
    uint8_t dma_ch;
    uint8_t dma_in_use;
    volatile uint8_t sdio_dma_lock;
    HAL_DMA_IRQ_HANDLER_T tx_dma_handler;
    HAL_DMA_IRQ_HANDLER_T rx_dma_handler;
#endif
};

struct SDIO_OPS_T {
    int (*send_cmd)(struct SDIO_T *sdio, struct SDIO_CMD_T *cmd, struct SDIO_DATA_T *data);
    void (*set_ios)(struct SDIO_T *sdio);
    int (*init)(struct SDIO_T *sdio);
};

static struct HAL_SDIO_HOST_CB_T sdio_host_callback_default[HAL_SDIO_HOST_ID_NUM];
static struct HAL_SDIO_HOST_CB_T *sdio_host_callback[HAL_SDIO_HOST_ID_NUM] = {NULL};
static uint32_t sdio_ip_base[HAL_SDIO_HOST_ID_NUM] = {
    SDMMC0_BASE,
#ifdef SDMMC1_BASE
    SDMMC1_BASE,
#endif
    };
static struct SDIO_IP_HOST_T sdio_host[HAL_SDIO_HOST_ID_NUM];
static struct SDIO_T sdio_devices[HAL_SDIO_HOST_ID_NUM];

#ifdef HAL_SDIO_HOST_USE_DMA
#define SDIO_DMA_LINK_SIZE      16380       //one desc can send and receive 16380(=4095*4) bytes
#define SDIO_DMA_TSIZE_MAX      0xFFF       //4095
#ifndef SDIO_DMA_DESC_CNT
    #define SDIO_DMA_DESC_CNT   4           //4*16380=63.98KB
#endif
SYNC_FLAGS_LOC static struct HAL_DMA_DESC_T sdio_dma_desc[HAL_SDIO_HOST_ID_NUM][SDIO_DMA_DESC_CNT];
static void sdio_ip0_ext_dma_tx_handler(uint8_t chan, uint32_t remain_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli);
static void sdio_ip0_ext_dma_rx_handler(uint8_t chan, uint32_t remain_rsize, uint32_t error, struct HAL_DMA_DESC_T *lli);
#ifdef SDMMC1_BASE
static void sdio_ip1_ext_dma_tx_handler(uint8_t chan, uint32_t remain_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli);
static void sdio_ip1_ext_dma_rx_handler(uint8_t chan, uint32_t remain_rsize, uint32_t error, struct HAL_DMA_DESC_T *lli);
#endif

static HAL_DMA_IRQ_HANDLER_T sdio_ip_ext_dma_irq_handlers[HAL_SDIO_HOST_ID_NUM * 2] = {
    sdio_ip0_ext_dma_tx_handler, sdio_ip0_ext_dma_rx_handler,
#ifdef SDMMC1_BASE
    sdio_ip1_ext_dma_tx_handler, sdio_ip1_ext_dma_rx_handler,
#endif
};
#endif
/******************************************************************************/
/************************The first part: end***********************************/
/******************************************************************************/

/******************************************************************************/
/*****************The second part: sdio standard related***********************/
/******************************************************************************/
/* CCCR regiter */
#define SDIO_CCCR_SDIO_VERSION      0x0
#define SDIO_CCCR_SD_VERSION        0x1
#define SDIO_CCCR_IO_ENABLE         0x2
#define SDIO_CCCR_IO_READY          0x3
#define SDIO_CCCR_INT_ENABLE        0x4
#define SDIO_CCCR_INT_PENDING       0x5
#define SDIO_CCCR_IO_ABORT          0x6
#define SDIO_CCCR_BUS_CONTROL       0x7
#define SDIO_CCCR_CARD_CAP          0x8
#define SDIO_CCCR_CIS_PTR           0x9
#define SDIO_CCCR_BUS_SUSPEND       0xc
#define SDIO_CCCR_FUNC_SEL          0xd
#define SDIO_CCCR_EXEC_FLAG         0xe
#define SDIO_CCCR_READY_FLG         0xf
#define SDIO_CCCR_BLK_SIZE          0x10
#define SDIO_CCCR_PWR_CONTROL       0x12
#define SDIO_CCCR_BUS_SPEED_SEL     0x13
#define SDIO_CCCR_UHSI_SUPPORT      0x14
#define SDIO_CCCR_DRV_STRENGTH      0x15
#define SDIO_CCCR_INT_EXTERN        0x16

/* int pending regiter bit*/
#define SDIO_CCCR5_INT1             0x02
#define SDIO_CCCR5_INT2             0x04
#define SDIO_CCCR5_INT3             0x08
#define SDIO_CCCR5_INT4             0x10
#define SDIO_CCCR5_INT5             0x20
#define SDIO_CCCR5_INT6             0x40
#define SDIO_CCCR5_INT7             0x80

/* function1 regiter */
#define SDIO_FUNC1_LOAD_CFG         0x104
#define SDIO_FUNC1_RX_BUF_CFG       0x108
#define SDIO_FUNC1_INT_TO_DEVICE    0x120

#define SDIO_FBR_BASE(x)            ((x) * 0x100)

#ifdef SDIO_BLOCK_SIZE
    #if SDIO_BLOCK_SIZE > 1024
        #error "The value of SDIO_BLOCK_SIZE can not exceed 1024"
    #endif
#else
    #define SDIO_BLOCK_SIZE         512
#endif

enum SDIO_DEVICE_BUS_WIDTH {
    SDIO_DEVICE_BUS_WIDTH_1 = 0x0,
    SDIO_DEVICE_BUS_WIDTH_4 = 0x2,
    SDIO_DEVICE_BUS_WIDTH_8 = 0x3,
};

/* CIS tuple code */
enum CIS_TUPLE_CODE {
    CISTPL_NULL     = 0x00, /* Null tuple */
    CISTPL_CHECKSUM = 0x10, /* Checksum control */
    CISTPL_VERS_1   = 0x15, /* Level 1 version/product-information */
    CISTPL_ALTSTR   = 0x16, /* The Alternate Language String Tuple */
    CISTPL_MANFID   = 0x20, /* Manufacturer Identification String Tuple */
    CISTPL_FUNCID   = 0x21, /* Function Identification Tuple */
    CISTPL_FUNCE    = 0x22, /* Function Extensions */
    CISTPL_VUT      = 0x80, /* Vendor Unique Tuples */
    CISTPL_SDIO_STD = 0x91, /* Additional information for functions built to support application specifications for standard SDIO functions */
    CISTPL_SDIO_EXD = 0x92, /* Reserved for future use with SDIO devices */
    CISTPL_END      = 0xff, /* The End-of-chain Tuple */
};

enum FUNCTION_ENABLE_LIST {
    FUNC_DISABLE = 0,
    FUNC_ENABLE = 1,
};

enum FUNCTION_INT_ENABLE_LIST {
    FUNC_INT_DISABLE = 0,
    FUNC_INT_ENABLE = 1,
};

struct SDIO_FUNC_T {
    uint8_t func_num;       /* func id */
    uint8_t func_status;    /* func enable status 0:disable 1:enable  */
    uint8_t func_int_status;/* func int status 0:disable 1:enable */
    uint16_t cur_blk_size;  /* func block size */
    uint16_t max_blk_size;  /* func max block size */
};

struct SDIO_CORE_T {
    uint8_t func_total_num; /* number of func */
    uint8_t cccr_version;   /* CCCR version */
    uint8_t sdio_version;   /* SDIO version*/
    uint8_t sdio_int_ctrl;  /* int control */
    uint16_t manf_code;     /* Manufacturer code */
    uint16_t manf_info;     /* Manufacturer info */
    struct SDIO_FUNC_T *func[HAL_SDIO_FUNC_NUM];
};

/* Mainly store the return value information of the command */
struct SDIO_CARD_RSP_T {
    uint32_t    cmd5r4_io_ocr: 24;
    uint8_t     cmd5r4_s18a: 1;
    uint8_t     cmd5r4_mp: 1;
    uint32_t    cmd5r4_funcs: 3;
    uint8_t     cmd5r4_busy: 1;
    uint16_t    cmd3r6_rca;
    uint8_t     cmd3r6_com_crc_error: 1;
    uint8_t     cmd3r6_illegal_command: 1;
    uint8_t     cmd3r6_error: 1;
    uint32_t    cmd7r1b_card_status;
};

static struct SDIO_FUNC_T host_sdio_func[HAL_SDIO_HOST_ID_NUM][HAL_SDIO_FUNC_NUM];
static struct SDIO_CORE_T host_sdio_core[HAL_SDIO_HOST_ID_NUM];
static struct SDIO_CORE_T *phost_sdio_core = host_sdio_core;
/******************************************************************************/
/************************The second part: end**********************************/
/******************************************************************************/

/******************************************************************************/
/********************sdio host register-level functions************************/
/******************************************************************************/
static inline void sdio_ip_writel(struct SDIO_IP_HOST_T *host, uint32_t reg, uint32_t val)
{
    *((volatile uint32_t *)(host->ioaddr + reg)) = val;
}

static inline uint32_t sdio_ip_readl(struct SDIO_IP_HOST_T *host, uint32_t reg)
{
    return *((volatile uint32_t *)(host->ioaddr + reg));
}

static int sdio_ip_wait_reset(struct SDIO_IP_HOST_T *host, uint32_t value)
{
    uint32_t ctrl;
    uint32_t timeout = TIMEOUT_TIME;

    sdio_ip_writel(host, SDMMCIP_REG_CTRL, value);
    while (timeout) {
        timeout--;
        ctrl = sdio_ip_readl(host, SDMMCIP_REG_CTRL);
        if (!(ctrl & SDMMCIP_REG_RESET_ALL)) {
            return 0;//reset success
        }
    }

    return 1;//reset failure
}

static void sdio_ip_reset_fifo(struct SDIO_IP_HOST_T *host)
{
    uint32_t ctrl;

    ctrl = sdio_ip_readl(host, SDMMCIP_REG_CTRL);
    ctrl |= SDMMCIP_REG_CTRL_FIFO_RESET;
    sdio_ip_wait_reset(host, ctrl);
}

#ifdef HAL_SDIO_HOST_USE_DMA
static void sdio_ip_reset_dma(struct SDIO_IP_HOST_T *host)
{
    uint32_t ctrl;

    if (host->dma_in_use) {
        host->dma_in_use = 0;
    } else {
        return;//Prevent multiple calls
    }

    //reset sdmmc ip dma
    ctrl = sdio_ip_readl(host, SDMMCIP_REG_CTRL);
    ctrl |= SDMMCIP_REG_CTRL_DMA_RESET;
    sdio_ip_wait_reset(host, ctrl);

    //free gpdma channel
    hal_gpdma_free_chan(host->dma_ch);

    //close sdmmc ip dma
    ctrl = sdio_ip_readl(host, SDMMCIP_REG_CTRL);
    ctrl &= ~SDMMCIP_REG_DMA_EN;
    sdio_ip_writel(host, SDMMCIP_REG_CTRL, ctrl);
}

static void sdio_base_dma_tx_handler(enum HAL_SDIO_HOST_ID_T id, uint8_t chan, uint32_t remain_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    uint32_t ip_raw_int_status = 0;
    struct SDIO_IP_HOST_T *host = &sdio_host[id];

    ip_raw_int_status = sdio_ip_readl(host, SDMMCIP_REG_RINTSTS);
    HAL_SDIO_HOST_TRACE(3, "%s:%d, tx ip_raw_int_status 0x%x", __func__, __LINE__, (uint32_t)ip_raw_int_status);
    HAL_SDIO_HOST_TRACE(4, "---tx dma handler,chan:%d,remain:%d,error:%d,lli:0x%X", chan, remain_tsize, error, (uint32_t)lli);
    if (ip_raw_int_status & (SDMMCIP_REG_DATA_ERR | SDMMCIP_REG_DATA_TOUT)) {
        HAL_SDIO_HOST_TRACE(3, "%s:%d, sdmmcip0 tx dma error 0x%x", __func__, __LINE__, (uint32_t)ip_raw_int_status);
    }

    sdio_ip_reset_dma(host);
    if (sdio_host_callback[id]->hal_sdio_host_dma_tx_done) {
        sdio_host_callback[id]->hal_sdio_host_dma_tx_done(remain_tsize, error);
    } else {
        host->sdio_dma_lock = 0;
    }
}

static void sdio_base_dma_rx_handler(enum HAL_SDIO_HOST_ID_T id, uint8_t chan, uint32_t remain_rsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    uint32_t ip_raw_int_status = 0;
    struct SDIO_IP_HOST_T *host = &sdio_host[id];

    ip_raw_int_status = sdio_ip_readl(host, SDMMCIP_REG_RINTSTS);
    HAL_SDIO_HOST_TRACE(3, "%s:%d, ip_raw_int_status 0x%x", __func__, __LINE__, (uint32_t)ip_raw_int_status);
    HAL_SDIO_HOST_TRACE(4, "---rx dma handler,chan:%d,remain:%d,error:%d,lli:0x%X", chan, remain_rsize, error, (uint32_t)lli);
    if (ip_raw_int_status & (SDMMCIP_REG_DATA_ERR | SDMMCIP_REG_DATA_TOUT)) {
        HAL_SDIO_HOST_TRACE(3, "%s:%d, sdmmcip0 rx dma error 0x%x", __func__, __LINE__, (uint32_t)ip_raw_int_status);
    }

    sdio_ip_reset_dma(host);
    if (sdio_host_callback[id]->hal_sdio_host_dma_rx_done) {
        sdio_host_callback[id]->hal_sdio_host_dma_rx_done(remain_rsize, error);
    } else {
        host->sdio_dma_lock = 0;
    }
}

static void sdio_ip0_ext_dma_tx_handler(uint8_t chan, uint32_t remain_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    sdio_base_dma_tx_handler(HAL_SDIO_HOST_ID_0, chan, remain_tsize, error, lli);
}

static void sdio_ip0_ext_dma_rx_handler(uint8_t chan, uint32_t remain_rsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    sdio_base_dma_rx_handler(HAL_SDIO_HOST_ID_0, chan, remain_rsize, error, lli);
}

#ifdef SDMMC1_BASE
static void sdio_ip1_ext_dma_tx_handler(uint8_t chan, uint32_t remain_tsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    sdio_base_dma_tx_handler(HAL_SDIO_HOST_ID_1, chan, remain_tsize, error, lli);
}

static void sdio_ip1_ext_dma_rx_handler(uint8_t chan, uint32_t remain_rsize, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    sdio_base_dma_rx_handler(HAL_SDIO_HOST_ID_1, chan, remain_rsize, error, lli);
}
#endif
#endif

static int sdio_ip_prepare_data(struct SDIO_IP_HOST_T *host, struct SDIO_DATA_T *data)
{
#ifdef HAL_SDIO_HOST_USE_DMA
    uint8_t  real_src_width;
    uint16_t i;
    uint32_t ctrl;
    uint16_t link_cnt;
    uint32_t remain_transfer_size;
    enum HAL_DMA_RET_T dret;
    struct HAL_DMA_CH_CFG_T dma_cfg;
#endif
    uint8_t id;
    uint32_t real_size;

    id = host->host_id;
    real_size = data->blocksize * data->blocks;
    sdio_ip_writel(host, SDMMCIP_REG_BLKSIZ, data->blocksize);
    sdio_ip_writel(host, SDMMCIP_REG_BYTCNT, real_size);

#ifdef HAL_SDIO_HOST_USE_DMA
    if (host->dma_in_use) {
        return HAL_SDIO_DMA_IN_USE;
    }
    host->sdio_dma_lock = 1;

    /* enable sdio ip dma function */
    ctrl = sdio_ip_readl(host, SDMMCIP_REG_CTRL);
    ctrl |= SDMMCIP_REG_DMA_EN;
    sdio_ip_writel(host, SDMMCIP_REG_CTRL, ctrl);

    memset(&dma_cfg, 0, sizeof(dma_cfg));
    if (data->flags & SDIO_DATA_READ) {
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
            return HAL_SDIO_INVALID_PARAMETER;
        }
        dma_cfg.src_width = HAL_DMA_WIDTH_WORD;
        dma_cfg.try_burst = 1;
        dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
        dma_cfg.src = (uint32_t)0; // useless
        dma_cfg.ch = hal_gpdma_get_chan(dma_cfg.src_periph, HAL_DMA_HIGH_PRIO);
        real_src_width = dma_cfg.src_width ? dma_cfg.src_width * 2 : 1;

        HAL_SDIO_HOST_TRACE(0, "  ");
        HAL_SDIO_HOST_TRACE(0, "---sdio host use dma read");
        HAL_SDIO_HOST_TRACE(1, "---dma read len      :%d", real_size);
        HAL_SDIO_HOST_TRACE(1, "---dma_cfg.dst       :0x%x", dma_cfg.dst);
        HAL_SDIO_HOST_TRACE(1, "---dma_cfg.dst_width :%d", dma_cfg.dst_width ? dma_cfg.dst_width * 2 : 1);
        HAL_SDIO_HOST_TRACE(1, "---dma_cfg.src_width :%d", real_src_width);
        HAL_SDIO_HOST_TRACE(1, "---dma_cfg.dst_bsize :%d", dma_cfg.dst_bsize ? (2 * (2 << dma_cfg.dst_bsize)) : 1);
        HAL_SDIO_HOST_TRACE(1, "---dma_cfg.src_bsize :%d", dma_cfg.src_bsize ? (2 * (2 << dma_cfg.src_bsize)) : 1);
        HAL_SDIO_HOST_TRACE(1, "---dma_cfg.src_periph:%d", dma_cfg.src_periph);
        HAL_SDIO_HOST_TRACE(1, "---dma_cfg.ch        :%d", dma_cfg.ch);
        HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");

        remain_transfer_size = real_size / 4;
        if (remain_transfer_size > SDIO_DMA_TSIZE_MAX) {
            if (remain_transfer_size % SDIO_DMA_TSIZE_MAX) {
                link_cnt = remain_transfer_size / SDIO_DMA_TSIZE_MAX + 1;
            } else {
                link_cnt = remain_transfer_size / SDIO_DMA_TSIZE_MAX;
            }
        } else {
            link_cnt = 1;
        }
        HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "---link_cnt          :%d", link_cnt);

        //Generate dma link configuration
        for (i = 0; i < link_cnt; i++) {
            dma_cfg.dst = (uint32_t)(data->dest + SDIO_DMA_TSIZE_MAX * real_src_width * i);
            if (remain_transfer_size > SDIO_DMA_TSIZE_MAX) {
                dma_cfg.src_tsize = SDIO_DMA_TSIZE_MAX;
                remain_transfer_size -= SDIO_DMA_TSIZE_MAX;
            } else {
                dma_cfg.src_tsize = remain_transfer_size;
                remain_transfer_size = 0;
            }
            if (i + 1 == link_cnt) {
                dret = hal_dma_init_desc(&sdio_dma_desc[id][i], &dma_cfg, NULL, 1);
            } else {
                dret = hal_dma_init_desc(&sdio_dma_desc[id][i], &dma_cfg, &sdio_dma_desc[id][i + 1], 0);
            }
            HAL_SDIO_HOST_ASSERT(dret == HAL_DMA_OK, "%s:%d,sdio dma: Failed to init rx desc %d", __func__, __LINE__, i);

            HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "---dma_cfg.dst       :0x%x", dma_cfg.dst);
            HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "---dma_cfg.src_tsize :%d", dma_cfg.src_tsize);
            HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "sdio_dma_desc[%d][%d].src/dst/lli/ctrl=0x%08X 0x%08X 0x%08X 0x%08X@0x%08X", id, i,
                                sdio_dma_desc[id][i].src, sdio_dma_desc[id][i].dst, sdio_dma_desc[id][i].lli, sdio_dma_desc[id][i].ctrl, (uint32_t)&sdio_dma_desc[id][i]);
        }
        HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");
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
            return HAL_SDIO_INVALID_PARAMETER;
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

        HAL_SDIO_HOST_TRACE(0, "  ");
        HAL_SDIO_HOST_TRACE(0, "---sdio host use dma write");
        HAL_SDIO_HOST_TRACE(1, "---dma write len     :%d", real_size);
        HAL_SDIO_HOST_TRACE(1, "---dma_cfg.src       :0x%x", dma_cfg.src);
        HAL_SDIO_HOST_TRACE(1, "---dma_cfg.dst_width :%d", dma_cfg.dst_width ? dma_cfg.dst_width * 2 : 1);
        HAL_SDIO_HOST_TRACE(1, "---dma_cfg.src_width :%d", real_src_width);
        HAL_SDIO_HOST_TRACE(1, "---dma_cfg.dst_bsize :%d", dma_cfg.dst_bsize ? (2 * (2 << dma_cfg.dst_bsize)) : 1);
        HAL_SDIO_HOST_TRACE(1, "---dma_cfg.src_bsize :%d", dma_cfg.src_bsize ? (2 * (2 << dma_cfg.src_bsize)) : 1);
        HAL_SDIO_HOST_TRACE(1, "---dma_cfg.dst_periph:%d", dma_cfg.dst_periph);
        HAL_SDIO_HOST_TRACE(1, "---dma_cfg.ch        :%d", dma_cfg.ch);
        HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");

        if (remain_transfer_size > SDIO_DMA_TSIZE_MAX) {
            if (remain_transfer_size % SDIO_DMA_TSIZE_MAX) {
                link_cnt = remain_transfer_size / SDIO_DMA_TSIZE_MAX + 1;
            } else {
                link_cnt = remain_transfer_size / SDIO_DMA_TSIZE_MAX;
            }
        } else {
            link_cnt = 1;
        }
        HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "---link_cnt          :%d", link_cnt);

        //Generate dma link configuration
        for (i = 0; i < link_cnt; i++) {
            dma_cfg.src = (uint32_t)(data->src + SDIO_DMA_TSIZE_MAX * real_src_width * i);
            if (remain_transfer_size > SDIO_DMA_TSIZE_MAX) {
                dma_cfg.src_tsize = SDIO_DMA_TSIZE_MAX;
                remain_transfer_size -= SDIO_DMA_TSIZE_MAX;
            } else {
                dma_cfg.src_tsize = remain_transfer_size;
                remain_transfer_size = 0;
            }
            if (i + 1 == link_cnt) {
                dret = hal_dma_init_desc(&sdio_dma_desc[id][i], &dma_cfg, NULL, 1);
            } else {
                dret = hal_dma_init_desc(&sdio_dma_desc[id][i], &dma_cfg, &sdio_dma_desc[id][i + 1], 0);
            }
            HAL_SDIO_HOST_ASSERT(dret == HAL_DMA_OK, "%s:%d,sdio dma: Failed to init rx desc %d", __func__, __LINE__, i);

            HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "---dma_cfg.src       :0x%x", dma_cfg.src);
            HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "---dma_cfg.src_tsize :%d", dma_cfg.src_tsize);
            HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "sdio_dma_desc[%d][%d].src/dst/lli/ctrl=0x%08X 0x%08X 0x%08X 0x%08X@0x%08X", id, i,
                                sdio_dma_desc[id][i].src, sdio_dma_desc[id][i].dst, sdio_dma_desc[id][i].lli, sdio_dma_desc[id][i].ctrl, (uint32_t)&sdio_dma_desc[id][i]);
        }
        HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "  ");
    }

    host->dma_in_use = 1;
    host->dma_ch = dma_cfg.ch;
    hal_gpdma_sg_start(&sdio_dma_desc[id][0], &dma_cfg);
#endif

    return HAL_SDIO_NONE;
}

static int sdio_ip_set_transfer_mode(struct SDIO_IP_HOST_T *host, struct SDIO_DATA_T *data)
{
    uint32_t mode;

    mode = SDMMCIP_REG_CMD_DATA_EXP;
    if (data->flags & SDIO_DATA_WRITE)
        mode |= SDMMCIP_REG_CMD_RW;

    return mode;
}

static void sdio_ip_func_int_mask(enum HAL_SDIO_HOST_ID_T id, uint8_t mask0_unmask1)
{
    uint32_t mask;
    struct SDIO_IP_HOST_T *host = &sdio_host[id];

    mask = sdio_ip_readl(host, SDMMCIP_REG_INTMASK);
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
    sdio_ip_writel(host, SDMMCIP_REG_INTMASK, mask);
}

static int sdio_ip_send_cmd(struct SDIO_T *sdio, struct SDIO_CMD_T *cmd, struct SDIO_DATA_T *data)
{
    int ret = 0;
    int flags = 0, i;
    uint32_t retry = MAX_RETRY_CNT;
    uint32_t mask = 0;
    uint32_t busy_timeout = MS_TO_TICKS(10), busy_t = 0;
#ifndef HAL_SDIO_HOST_USE_DMA
    uint32_t status = 0, fifo_data = 0;
#endif
    struct SDIO_IP_HOST_T *host = sdio->priv;

    busy_t = hal_sys_timer_get();
    while ((sdio_ip_readl(host, SDMMCIP_REG_STATUS) & SDMMCIP_REG_BUSY) && hal_sys_timer_get() < (busy_t + busy_timeout)) {
        HAL_SDIO_HOST_TRACE(0, "[sdio host]busy");
    }

#ifdef HAL_SDIO_HOST_USE_DMA
    sdio_ip_writel(host, SDMMCIP_REG_RINTSTS, SDMMCIP_REG_RINTSTS_CDONE |
                   SDMMCIP_REG_RINTSTS_RTO | SDMMCIP_REG_RINTSTS_RE); //clear command flag
#else
    sdio_ip_writel(host, SDMMCIP_REG_RINTSTS, SDMMCIP_REG_RINTSTS_ALL);//clear interrupt status
#endif
    sdio_ip_writel(host, SDMMCIP_REG_CMDARG, cmd->cmdarg);

    if (data) {
        flags = sdio_ip_set_transfer_mode(host, data);
    }

    if ((cmd->resp_type & SDIO_RSP_136) && (cmd->resp_type & SDIO_RSP_BUSY)) {
        return HAL_SDIO_RESPONSE_BUSY;
    }

    if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION) {
        flags |= SDMMCIP_REG_CMD_ABORT_STOP;
    } else {
        flags |= SDMMCIP_REG_CMD_PRV_DAT_WAIT;
    }

    if (cmd->cmdidx == SDIO_CMD_SWITCH_UHS18V) {
        flags |= SDMMCIP_REG_CMD_VOLT_SWITCH;
    }

    if (cmd->resp_type & SDIO_RSP_PRESENT) {
        flags |= SDMMCIP_REG_CMD_RESP_EXP;
        if (cmd->resp_type & SDIO_RSP_136) {
            flags |= SDMMCIP_REG_CMD_RESP_LENGTH;
        }
    }

    if (cmd->resp_type & SDIO_RSP_CRC) {
        flags |= SDMMCIP_REG_CMD_CHECK_CRC;
    }

    if (data) {
        ret = sdio_ip_prepare_data(host, data);
        if (ret) {
            return ret;
        }
    }

    flags |= (cmd->cmdidx | SDMMCIP_REG_CMD_START | SDMMCIP_REG_CMD_USE_HOLD_REG);
    sdio_ip_writel(host, SDMMCIP_REG_CMD, flags);
    for (i = 0; (i < retry) && (cmd->cmdidx != SDIO_CMD_SWITCH_UHS18V); i++) {
        mask = sdio_ip_readl(host, SDMMCIP_REG_RINTSTS);
        if (mask & SDMMCIP_REG_RINTSTS_CDONE) {
            sdio_ip_writel(host, SDMMCIP_REG_RINTSTS, SDMMCIP_REG_RINTSTS_CDONE);
            break;
        }
    }

    if (i == retry) {
        HAL_SDIO_HOST_TRACE(1, "%s: Timeout.", __func__);

        sdio_ip_reset_fifo(host);
#ifdef HAL_SDIO_HOST_USE_DMA
        if (data) {
            sdio_ip_reset_dma(host);
        }
#endif
        return HAL_SDIO_CMD_SEND_TIMEOUT;
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
        HAL_SDIO_HOST_TRACE(1, "%s: Response Timeout.", __func__);

        sdio_ip_reset_fifo(host);
#ifdef HAL_SDIO_HOST_USE_DMA
        if (data) {
            sdio_ip_reset_dma(host);
        }
#endif
        return HAL_SDIO_RESPONSE_TIMEOUT;
    } else if (mask & SDMMCIP_REG_RINTSTS_RE) {
        HAL_SDIO_HOST_TRACE(1, "%s: Response Error.", __func__);

        sdio_ip_reset_fifo(host);
#ifdef HAL_SDIO_HOST_USE_DMA
        if (data) {
            sdio_ip_reset_dma(host);
        }
#endif
        return HAL_SDIO_RESPONSE_ERR;
    }

    if (cmd->resp_type & SDIO_RSP_PRESENT) {
        if (cmd->resp_type & SDIO_RSP_136) {
            cmd->response[0] = sdio_ip_readl(host, SDMMCIP_REG_RESP3);
            cmd->response[1] = sdio_ip_readl(host, SDMMCIP_REG_RESP2);
            cmd->response[2] = sdio_ip_readl(host, SDMMCIP_REG_RESP1);
            cmd->response[3] = sdio_ip_readl(host, SDMMCIP_REG_RESP0);
        } else {
            cmd->response[0] = sdio_ip_readl(host, SDMMCIP_REG_RESP0);
        }
    }

    //Processing after the completion of the transfer process: use dma
#ifdef HAL_SDIO_HOST_USE_DMA
    if (data) {
        if ((sdio_host_callback[host->host_id]->hal_sdio_host_dma_tx_done) && (data->flags == SDIO_DATA_WRITE)) {
            //Non-blocking mode: there will be a callback function after the transmission is completed
        } else if ((sdio_host_callback[host->host_id]->hal_sdio_host_dma_rx_done) && (data->flags == SDIO_DATA_READ)) {
            //Non-blocking mode: there will be a callback function after the transmission is completed
        } else {
            while (host->sdio_dma_lock) {
                osDelay(1);
            }
        }
    }
#else
    //Processing after the completion of the transfer process: do not use dma
    if (data) {
        i = 0;
        while (1) {
            mask = sdio_ip_readl(host, SDMMCIP_REG_RINTSTS);
            if (mask & (SDMMCIP_REG_DATA_ERR | SDMMCIP_REG_DATA_TOUT)) {
                HAL_SDIO_HOST_TRACE(1, "%s: READ DATA ERROR!", __func__);
                ret = HAL_SDIO_READ_DATA_ERR;
                goto out;
            }
            status = sdio_ip_readl(host, SDMMCIP_REG_STATUS);
            if (data->flags == SDIO_DATA_READ) {
                if (status & SDMMCIP_REG_FIFO_COUNT_MASK) {
                    fifo_data = sdio_ip_readl(host, SDMMCIP_REG_FIFO_OFFSET);
                    //HAL_SDIO_HOST_TRACE(3,"%s: count %d, read -> 0x%x", __func__, i, fifo_data);
                    /* FIXME: now we just deal with 32bit width fifo one time */
                    if (i < data->blocks * data->blocksize) {
                        memcpy(data->dest + i, &fifo_data, sizeof(fifo_data));
                        i += sizeof(fifo_data);
                    } else {
                        HAL_SDIO_HOST_TRACE(1, "%s: fifo data too much", __func__);
                        ret = HAL_SDIO_FIFO_OVERFLOW;
                        goto out;
                    }
                }
                /* nothing to read from fifo and DTO is set */
                else if (mask & SDMMCIP_REG_RINTSTS_DTO) {
                    if (i != data->blocks * data->blocksize) {
                        HAL_SDIO_HOST_TRACE(3, "%s: need to read %d, actually read %d", __func__, data->blocks * data->blocksize, i);
                    }
                    ret = HAL_SDIO_NONE;
                    goto out;
                }
            } else {
                /* nothing to write to fifo and DTO is set */
                if (mask & SDMMCIP_REG_RINTSTS_DTO) {
                    /* check if number is right */
                    if (i != data->blocks * data->blocksize) {
                        HAL_SDIO_HOST_TRACE(3, "%s: need to write %d, actually written %d", __func__, data->blocks * data->blocksize, i);
                    }
                    ret = HAL_SDIO_NONE;
                    goto out;
                } else if (!(status & SDMMCIP_REG_FIFO_COUNT_MASK)) {
                    /* FIXME: now we just deal with 32bit width fifo one time */
                    if (i < data->blocks * data->blocksize) {
                        memcpy(&fifo_data, data->src + i, sizeof(fifo_data));
                        //HAL_SDIO_HOST_TRACE(4,"%s: fifo %d, count %d, write -> 0x%x", __func__, ((status & SDMMCIP_REG_FIFO_COUNT_MASK)>>SDMMCIP_REG_FIFO_COUNT_SHIFT), i, fifo_data);
                        i += sizeof(fifo_data);
                        sdio_ip_writel(host, SDMMCIP_REG_FIFO_OFFSET, fifo_data);
                    } else {
                        HAL_SDIO_HOST_TRACE(1, "%s: no data to write to fifo, do nothing", __func__);
                    }
                }
            }
        }
    }
#endif

#ifndef HAL_SDIO_HOST_USE_DMA
out:
#endif

    return ret;
}

static int sdio_ip_setup_bus(struct SDIO_IP_HOST_T *host, uint32_t freq)
{
    uint32_t div, status;
    int32_t  timeout;
    uint32_t sclk;

    if ((freq == host->clock) || (freq == 0))
        return HAL_SDIO_NONE;
    if (host->bus_hz)
        sclk = host->bus_hz;
    else {
        HAL_SDIO_HOST_TRACE(1, "%s: Didn't get source clock value.", __func__);
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (sclk <= freq)
        div = 0;    /* bypass mode */
    else
        div = __SDMMC_DIV_ROUND_UP(sclk, 2 * freq);

    HAL_SDIO_HOST_TRACE(5, "%s: freq %d, sclk %d, reg div %d, final div %d", __func__, freq, sclk, div, div * 2);
    host->div = div * 2;
    host->final_bus_speed = host->div ? sclk / host->div : sclk;

    sdio_ip_writel(host, SDMMCIP_REG_CLKENA, 0);
    sdio_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_PRV_DAT_WAIT |
                   SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START);

    timeout = TIMEOUT_TIME;
    do {
        status = sdio_ip_readl(host, SDMMCIP_REG_CMD);
        if (timeout-- < 0) {
            HAL_SDIO_HOST_TRACE(2, "%s:%d: Timeout!", __func__, __LINE__);
            return HAL_SDIO_CMD_START_TIMEOUT1;
        }
    } while (status & SDMMCIP_REG_CMD_START);

    sdio_ip_writel(host, SDMMCIP_REG_CLKDIV, div);
    sdio_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_PRV_DAT_WAIT |
                   SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START);

    timeout = TIMEOUT_TIME;
    do {
        status = sdio_ip_readl(host, SDMMCIP_REG_CMD);
        if (timeout-- < 0) {
            HAL_SDIO_HOST_TRACE(2, "%s:%d: Timeout!", __func__, __LINE__);
            return HAL_SDIO_CMD_START_TIMEOUT2;
        }
    } while (status & SDMMCIP_REG_CMD_START);

    sdio_ip_writel(host, SDMMCIP_REG_CLKENA, SDMMCIP_REG_CLKEN_ENABLE |
                   SDMMCIP_REG_CLKEN_LOW_PWR);
    sdio_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_PRV_DAT_WAIT |
                   SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START);

    timeout = TIMEOUT_TIME;
    do {
        status = sdio_ip_readl(host, SDMMCIP_REG_CMD);
        if (timeout-- < 0) {
            HAL_SDIO_HOST_TRACE(2, "%s:%d: Timeout!", __func__, __LINE__);
            return HAL_SDIO_CMD_START_TIMEOUT3;
        }
    } while (status & SDMMCIP_REG_CMD_START);

    host->clock = freq;

    return HAL_SDIO_NONE;
}

static int sdio_ip_clk_disable_low_power(struct SDIO_IP_HOST_T *host)
{
    uint32_t status;
    int32_t  timeout;

    sdio_ip_writel(host, SDMMCIP_REG_CLKENA, 0);
    sdio_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_PRV_DAT_WAIT |
                   SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START);

    timeout = TIMEOUT_TIME;
    do {
        status = sdio_ip_readl(host, SDMMCIP_REG_CMD);
        if (timeout-- < 0) {
            HAL_SDIO_HOST_TRACE(2, "%s:%d: Timeout!", __func__, __LINE__);
            return HAL_SDIO_CMD_START_TIMEOUT4;
        }
    } while (status & SDMMCIP_REG_CMD_START);

    sdio_ip_writel(host, SDMMCIP_REG_CLKENA, SDMMCIP_REG_CLKEN_ENABLE);
    sdio_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_PRV_DAT_WAIT |
                   SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START);

    timeout = TIMEOUT_TIME;
    do {
        status = sdio_ip_readl(host, SDMMCIP_REG_CMD);
        if (timeout-- < 0) {
            HAL_SDIO_HOST_TRACE(2, "%s:%d: Timeout!", __func__, __LINE__);
            return HAL_SDIO_CMD_START_TIMEOUT5;
        }
    } while (status & SDMMCIP_REG_CMD_START);

    return HAL_SDIO_NONE;
}

#ifdef SDIO_HOST_CMD11_ENABLE
static int sdio_ip_wait_cmd_start(struct SDIO_IP_HOST_T *host, uint8_t en_volt_switch)
{
    uint32_t status;
    int32_t  timeout;

    sdio_ip_writel(host, SDMMCIP_REG_CMD, SDMMCIP_REG_CMD_PRV_DAT_WAIT |
                   SDMMCIP_REG_CMD_UPD_CLK | SDMMCIP_REG_CMD_START |
                   (en_volt_switch ? SDMMCIP_REG_CMD_VOLT_SWITCH : 0));

    timeout = TIMEOUT_TIME;
    do {
        status = sdio_ip_readl(host, SDMMCIP_REG_CMD);
        if (timeout-- < 0) {
            HAL_SDIO_HOST_TRACE(2, "%s:%d: Timeout!", __func__, __LINE__);
            return HAL_SDIO_CMD_START_TIMEOUT6;
        }
    } while (status & SDMMCIP_REG_CMD_START);

    return HAL_SDIO_NONE;
}
#endif

static void sdio_ip_set_ios(struct SDIO_T *sdio)
{
    struct SDIO_IP_HOST_T *host = (struct SDIO_IP_HOST_T *)sdio->priv;
    uint32_t ctype, regs;

    HAL_SDIO_HOST_TRACE(3, "%s, Buswidth = %d, clock: %d", __func__, sdio->bus_width, sdio->clock);

    if (sdio->clock) {
        sdio_ip_setup_bus(host, sdio->clock);
    }

    if (sdio->bus_width) {
        switch (sdio->bus_width) {
            case SDIO_HOST_BUS_WIDTH_8:
                ctype = SDMMCIP_REG_CTYPE_8BIT;
                break;
            case SDIO_HOST_BUS_WIDTH_4:
                ctype = SDMMCIP_REG_CTYPE_4BIT;
                break;
            default:
                ctype = SDMMCIP_REG_CTYPE_1BIT;
                break;
        }

        sdio_ip_writel(host, SDMMCIP_REG_CTYPE, ctype);
        regs = sdio_ip_readl(host, SDMMCIP_REG_UHS_REG);
        if (sdio->ddr_mode)
            regs |= SDMMCIP_REG_DDR_MODE;
        else
            regs &= ~SDMMCIP_REG_DDR_MODE;
        sdio_ip_writel(host, SDMMCIP_REG_UHS_REG, regs);
    }
}

static int sdio_ip_init(struct SDIO_T *sdio)
{
    struct SDIO_IP_HOST_T *host = sdio->priv;

    sdio_ip_writel(host, SDMMCIP_REG_PWREN, 1);
    HAL_SDIO_HOST_TRACE(2, "%s, host->ioaddr:0x%X, ip_pwr:%d", __func__, (uint32_t)host->ioaddr, sdio_ip_readl(host, SDMMCIP_REG_PWREN));

    if (sdio_ip_wait_reset(host, SDMMCIP_REG_RESET_ALL)) {
        HAL_SDIO_HOST_ASSERT(0, "%s:%d,Fail-reset!!", __func__, __LINE__);
        return HAL_SDIO_RESET_FAIL;
    }

    sdio_ip_writel(host, SDMMCIP_REG_INTMASK, ~SDMMCIP_REG_INTMSK_ALL); //disable all int
    sdio_ip_writel(host, SDMMCIP_REG_TMOUT, SDMMCIP_REG_RINTSTS_ALL);   //modify response timeout value for maximum
    sdio_ip_writel(host, SDMMCIP_REG_IDINTEN, 0);   //disable internal DMAC interrupt
    sdio_ip_writel(host, SDMMCIP_REG_BMOD, 1);      //software reset internal DMA controller
    sdio_ip_writel(host, SDMMCIP_REG_FIFOTH, host->fifoth_val);

    return HAL_SDIO_NONE;
}

static const struct SDIO_OPS_T sdio_ip_ops = {
    .send_cmd   = sdio_ip_send_cmd,
    .set_ios    = sdio_ip_set_ios,
    .init       = sdio_ip_init,
};

/******************************************************************************/
/**************************sdio host hal layer function************************/
/******************************************************************************/
static int sdio_send_cmd(struct SDIO_T *sdio, struct SDIO_CMD_T *cmd, struct SDIO_DATA_T *data)
{
    int ret;

    HAL_SDIO_HOST_TRACE(1, "CMD_SEND:%d", cmd->cmdidx);
    HAL_SDIO_HOST_TRACE(1, "\t\tARG\t\t\t 0x%08X", cmd->cmdarg);
    ret = sdio->cfg->ops->send_cmd(sdio, cmd, data);
    HAL_SDIO_HOST_TRACE(0, "send cmd end...");
    switch (cmd->resp_type) {
        case SDIO_RSP_NONE:
            HAL_SDIO_HOST_TRACE(0, "\t\tMMC_RSP_NONE");
            break;
        case SDIO_RSP_R1:
            HAL_SDIO_HOST_TRACE(1, "\t\tMMC_RSP_R1,5,6,7 \t 0x%08X ",
                                cmd->response[0]);
            break;
        case SDIO_RSP_R1b:
            HAL_SDIO_HOST_TRACE(1, "\t\tMMC_RSP_R1b\t\t 0x%08X ",
                                cmd->response[0]);
            break;
        case SDIO_RSP_R2:
            HAL_SDIO_HOST_TRACE(1, "\t\tMMC_RSP_R2\t\t 0x%08X ",
                                cmd->response[0]);
            HAL_SDIO_HOST_TRACE(1, "\t\t          \t\t 0x%08X ",
                                cmd->response[1]);
            HAL_SDIO_HOST_TRACE(1, "\t\t          \t\t 0x%08X ",
                                cmd->response[2]);
            HAL_SDIO_HOST_TRACE(1, "\t\t          \t\t 0x%08X ",
                                cmd->response[3]);
            HAL_SDIO_HOST_TRACE(0, "  ");
            HAL_SDIO_HOST_TRACE(0, "\t\t\t\t\tDUMPING DATA");
#ifdef SDIO_HOST_DEBUG
            uint8_t i;
            uint8_t *ptr;
            for (i = 0; i < 4; i++) {
                uint8_t j;
                HAL_SDIO_HOST_TRACE(TR_ATTR_NO_LF, "\t\t\t\t\t%03d - ", i * 4);
                ptr = (uint8_t *)&cmd->response[i];
                ptr += 3;
                for (j = 0; j < 4; j++)
                    HAL_SDIO_HOST_TRACE(TR_ATTR_NO_LF | TR_ATTR_NO_TS | TR_ATTR_NO_ID, "%02X ", *ptr--);
                HAL_SDIO_HOST_TRACE(TR_ATTR_NO_TS | TR_ATTR_NO_ID, "   ");
            }
#endif
            break;
        case SDIO_RSP_R3:
            HAL_SDIO_HOST_TRACE(1, "\t\tMMC_RSP_R3,4\t\t 0x%08X ",
                                cmd->response[0]);
            break;
        default:
            HAL_SDIO_HOST_TRACE(0, "\t\tERROR MMC rsp not supported");
            break;
    }
    return ret;
}

static void sdio_set_ios(struct SDIO_T *sdio)
{
    if (sdio->cfg->ops->set_ios)
        sdio->cfg->ops->set_ios(sdio);
}

static void sdio_set_clock(struct SDIO_T *sdio, uint32_t clock)
{
    if (clock > sdio->cfg->f_max)
        clock = sdio->cfg->f_max;

    if (clock < sdio->cfg->f_min)
        clock = sdio->cfg->f_min;

    sdio->clock = clock;
    sdio_set_ios(sdio);
}

static void sdio_set_host_bus_width(struct SDIO_T *sdio, enum SDIO_HOST_BUS_WIDTH width)
{
    sdio->bus_width = width;
    sdio_set_ios(sdio);
}

static struct SDIO_T *sdio_create(int id, const struct SDIO_CONFIG_T *cfg, void *priv)
{
    struct SDIO_T *sdio;

    /* quick validation */
    if (cfg == NULL || cfg->ops == NULL || cfg->ops->send_cmd == NULL ||
        cfg->f_min == 0 || cfg->f_max == 0 || cfg->b_max == 0)
        return NULL;

    sdio = &sdio_devices[id];
    if (sdio == NULL)
        return NULL;

    sdio->cfg = cfg;
    sdio->priv = priv;

    return sdio;
}

/******************************************************************************/
/***********************sdio standard related functions************************/
/******************************************************************************/
int hal_sdio_host_send_cmd(enum HAL_SDIO_HOST_ID_T id, uint8_t cmdindex, uint32_t cmdarg, uint8_t rsp_type, uint32_t *rsp)
{
    int err;
    struct SDIO_CMD_T cmd;

    memset(&cmd, 0, sizeof(struct SDIO_CMD_T));
    cmd.cmdidx = cmdindex;
    cmd.cmdarg = cmdarg;
    cmd.resp_type = rsp_type;

    err = sdio_send_cmd(sdio_host[id].sdio, &cmd, NULL);
    if (err) {
        return err;
    }
    if (rsp) {
        *rsp = cmd.response[0];
    }

    return HAL_SDIO_NONE;
}

int hal_sdio_io_rw_direct(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_DEVICE_RW read0_write1,
                          enum HAL_SDIO_FUNC_NUM_T func_num, uint32_t addr, uint8_t data, uint8_t *rsp)
{
    struct SDIO_CMD_T cmd;
    int err = HAL_SDIO_NONE;

    memset(&cmd, 0, sizeof(struct SDIO_CMD_T));
    cmd.cmdidx  = SDIO_CMD_IO_RW_DIRECT;
    cmd.cmdarg  = read0_write1 ? 0x80000000 : 0x00000000;   //R/W flag
    cmd.cmdarg |= ((uint32_t)(func_num & 0x7)) << 28;       //funcion number
    cmd.cmdarg |= (read0_write1 && (uint32_t)rsp) ? 0x08000000 : 0x00000000; //RAW flag
    cmd.cmdarg |= addr << 9;    //register address
    cmd.cmdarg |= data;         //write data
    cmd.resp_type = SDIO_RSP_R5;

    sdio_ip_func_int_mask(id, 0);
    err = sdio_send_cmd(sdio_host[id].sdio, &cmd, NULL);
    if (err) {
        goto err_cmd52;
    }
    if (rsp) {
        *rsp = cmd.response[0];
    }

err_cmd52:
    sdio_ip_func_int_mask(id, 1);

    return err;
}

int hal_sdio_io_rw_extended(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_DEVICE_RW read0_write1,
                            enum HAL_SDIO_FUNC_NUM_T func_num, uint32_t addr, uint8_t *rsp, uint8_t incr_addr, uint8_t *data_buff, uint32_t data_size)
{
    struct SDIO_CMD_T cmd;
    struct SDIO_DATA_T data;
    int err = HAL_SDIO_NONE;
    uint16_t func_cur_blk_size;
    uint16_t blk_count;

    if (phost_sdio_core[id].func[func_num]) {
        func_cur_blk_size = phost_sdio_core[id].func[func_num]->cur_blk_size;
        if (!func_cur_blk_size) {
            return HAL_SDIO_BLOCK_SIZE_ZERO;
        }
    } else {
        return HAL_SDIO_INVALID_FUNCTION_NUM;
    }

    HAL_SDIO_HOST_ASSERT(data_size, "%s:%d,data size cannot be 0", __func__, __LINE__);
#ifdef HAL_SDIO_HOST_USE_DMA
    HAL_SDIO_HOST_ASSERT((data_size / SDIO_DMA_LINK_SIZE) < (SDIO_DMA_DESC_CNT - 1), "%s:%d,SDIO_DMA_DESC_CNT is too small", __func__, __LINE__);
#endif

    //Automatically select byte mode or block mode
    if (data_size < func_cur_blk_size) {
        blk_count = 0;
        data_size = (data_size + 3) & (~3);//Round up to a multiple of 4 bytes
    } else {
        if (data_size % func_cur_blk_size) {
            blk_count = data_size / func_cur_blk_size + 1;  //byte block count
        } else {
            blk_count = data_size / func_cur_blk_size;      //byte block count
        }

        if (blk_count > 511) {
            return HAL_SDIO_INVALID_PARAMETER;
        }
    }

    memset(&cmd, 0, sizeof(struct SDIO_CMD_T));
    cmd.cmdidx  = SDIO_CMD_IO_RW_EXTENDED;
    cmd.cmdarg  = read0_write1 ? 0x80000000 : 0x00000000;   //R/W flag
    cmd.cmdarg |= ((uint32_t)(func_num & 0x7)) << 28;       //funcion number
    cmd.cmdarg |= incr_addr ? (1 << 26) : 0; //op code
    cmd.cmdarg |= addr << 9;    //register address
    if (blk_count) {
        //block mode
        cmd.cmdarg |= (1 << 27);
        cmd.cmdarg |= blk_count;
    } else {
        //bytes mode
        cmd.cmdarg |= (data_size == 512) ? 0 : data_size;
    }

    cmd.resp_type = SDIO_RSP_R5;

    memset(&data, 0, sizeof(struct SDIO_DATA_T));
    if (read0_write1) {
        data.src = (const char *)data_buff;
    } else {
        data.dest = (char *)data_buff;
    }

    data.blocks = blk_count ? blk_count : 1;
    data.blocksize = blk_count ? func_cur_blk_size : data_size;
    data.flags = read0_write1 ? SDIO_DATA_WRITE : SDIO_DATA_READ;

    HAL_SDIO_HOST_TRACE(0, "   ");
    HAL_SDIO_HOST_TRACE(1, "cmd53,arg = 0x%08x", cmd.cmdarg);
    HAL_SDIO_HOST_TRACE(1, "cmd53,data.blocks=%d", data.blocks);
    HAL_SDIO_HOST_TRACE(1, "cmd53,data.blocksize=%d", data.blocksize);
    HAL_SDIO_HOST_TRACE(1, "cmd53,data.flags=%d", data.flags);
    HAL_SDIO_HOST_TRACE(0, "   ");

    sdio_ip_func_int_mask(id, 0);
    err = sdio_send_cmd(sdio_host[id].sdio, &cmd, &data);
    if (err) {
        goto err_cmd53;
    }
    if (rsp) {
        *rsp = cmd.response[0];
    }

err_cmd53:
    sdio_ip_func_int_mask(id, 1);

    return err;
}

enum HAL_SDIO_ERR hal_sdio_get_cccr_version(enum HAL_SDIO_HOST_ID_T id, uint8_t *cccr_version)
{
    uint8_t version;

    if (!cccr_version) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_SDIO_VERSION, 0, &version)) {
        return HAL_SDIO_GET_VERSION_FAIL;
    }

    *cccr_version = version & 0xf;

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_get_sdio_version(enum HAL_SDIO_HOST_ID_T id, uint8_t *sdio_version)
{
    uint8_t version;

    if (!sdio_version) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_SDIO_VERSION, 0, &version)) {
        return HAL_SDIO_GET_VERSION_FAIL;
    }

    *sdio_version = (version >> 4) & 0xf;

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_set_device_bus_width(enum HAL_SDIO_HOST_ID_T id, uint8_t bus_width)
{
    uint8_t width;

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_BUS_CONTROL, 0, &width)) {
        width &= ~0x3;
        width |= bus_width;
    } else {
        return HAL_SDIO_SET_BUS_WIDTH_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_BUS_CONTROL, width, NULL)) {
        return HAL_SDIO_SET_BUS_WIDTH_FAIL;
    }

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_get_bus_width(enum HAL_SDIO_HOST_ID_T id, uint8_t *bus_width)
{
    uint8_t width;

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_BUS_CONTROL, 0, &width)) {
        return HAL_SDIO_GET_BUS_WIDTH_FAIL;
    }

    if (bus_width) {
        *bus_width = width & 0x3;
    } else {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_get_cis_ptr(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num, uint32_t *ptr_addr)
{
    uint8_t data = 0;
    uint8_t index;
    uint32_t temp = 0;

    if (func_num >= HAL_SDIO_FUNC_NUM || (!ptr_addr)) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    /* common cis pointer:0x09,0x0A,0x0B */
    for (index = 0; index < 3; index++) {
        hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_FBR_BASE(func_num) + SDIO_CCCR_CIS_PTR + index, 0, &data);

        temp |= data << (index * 8);
    }

    *ptr_addr = temp;

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_cis_parse(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num, uint32_t cis_ptr)
{
    uint8_t data[255];
    uint8_t index = 0;
    uint8_t len = 0;
    uint8_t tpl_code = CISTPL_NULL;
    uint8_t tpl_link = 0;
    uint32_t temp_cis_ptr = cis_ptr;
    memset(data, 0, sizeof(data));
    while (1) {
        hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, temp_cis_ptr++, 0, &tpl_code);
        HAL_SDIO_HOST_TRACE(2, "CIS[0x%x] tpl_code=0x%X", temp_cis_ptr - 1, (uint32_t)tpl_code);
        if ((tpl_code == CISTPL_NULL) || (tpl_code == CISTPL_END)) {
            break;
        }

        hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, temp_cis_ptr++, 0, &tpl_link);
        HAL_SDIO_HOST_TRACE(2, "CIS[0x%x] tpl_link=%d", temp_cis_ptr - 1, tpl_link);

        for (index = 0; index < tpl_link; index++) {
            hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, temp_cis_ptr + index, 0, &data[index]);
            HAL_SDIO_HOST_TRACE(2, "tpl data[%d]=0x%x", index, data[index]);
        }
        switch (tpl_code) {
            case CISTPL_VERS_1:
                HAL_SDIO_HOST_TRACE(0, "Product Information:");
                for (index = 2; data[index] != 0xff; index += len + 1) {
                    len = strlen((char *)data + index);
                    if (len)
                        HAL_SDIO_HOST_TRACE(1, " %s", data + index);
                }
                break;
            case CISTPL_MANFID:
                // 16.6 CISTPL_MANFID: Manufacturer Identification String Tuple
                HAL_SDIO_HOST_TRACE(1, "Manufacturer Code: 0x%04x", *(uint16_t *)data);
                HAL_SDIO_HOST_TRACE(1, "Manufacturer Information: 0x%04x", *(uint16_t *)(data + 2));
                phost_sdio_core[id].manf_code = *(uint16_t *)data;
                phost_sdio_core[id].manf_info = *(uint16_t *)(data + 2);
                break;
            case CISTPL_FUNCID:
                // 16.7.1 CISTPL_FUNCID: Function Identification Tuple
                HAL_SDIO_HOST_TRACE(1, "Card Function Code: 0x%02X", data[0]);
                HAL_SDIO_HOST_TRACE(1, "System Initialization Bit Mask: 0x%02X", data[1]);
                break;
            case CISTPL_FUNCE:
                // 16.7.2 CISTPL_FUNCE: Function Extension Tuple
                if (data[0] == 0) {
                    // 16.7.3 CISTPL_FUNCE Tuple for Function 0 (Extended Data 00h)
                    HAL_SDIO_HOST_TRACE(1, "Maximum Block Size case1: func: %d,size %d", func_num, *(uint16_t *)(data + 1));
                    HAL_SDIO_HOST_TRACE(1, "Maximum Transfer Rate Code: 0x%02X", data[3]);
                    if (phost_sdio_core[id].func[func_num]) {
                        phost_sdio_core[id].func[func_num]->max_blk_size = *(uint16_t *)(data + 1);
                    }
                } else if (data[0] == 1) {
                    // 16.7.4 CISTPL_FUNCE Tuple for Function 1-7 (Extended Data 01h)
                    HAL_SDIO_HOST_TRACE(2, "Maximum Block Size case2 func: %d,size %d", func_num, *(uint16_t *)(data + 0x0c));
                    if (phost_sdio_core[id].func[func_num]) {
                        phost_sdio_core[id].func[func_num]->max_blk_size = *(uint16_t *)(data + 0x0c);
                    }
                } else {
                    HAL_SDIO_HOST_TRACE(2, "TPLFE_PS func: %d,size %d", func_num, *(uint16_t *)(data + 0x02));
                }
                break;
            default:
                HAL_SDIO_HOST_TRACE(3, "[CIS Tuple 0x%02X] addr=0x%08x size=%d", tpl_code, temp_cis_ptr - 2, tpl_link);
                break;
        }

        if (tpl_link == CISTPL_END)
            break;
        else
            temp_cis_ptr += tpl_link;
    }

    return HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_enable_func(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num)
{
    uint8_t enable;
    uint8_t ready = 0;

    if (func_num >= HAL_SDIO_FUNC_NUM) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_ENABLE, 0, &enable)) {
        enable |= (1 << func_num);
    } else {
        return HAL_SDIO_ENABLE_FUNCTION_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_ENABLE, enable, NULL)) {
        return HAL_SDIO_ENABLE_FUNCTION_FAIL;
    }

    while (!(ready & (1 << func_num))) {
        hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_READY, 0, &ready);
    }

    if (phost_sdio_core[id].func[func_num]) {
        phost_sdio_core[id].func[func_num]->func_status = FUNC_ENABLE;
    }

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_disable_func(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num)
{
    uint8_t enable;

    if (func_num >= HAL_SDIO_FUNC_NUM) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_ENABLE, 0, &enable)) {
        enable &= ~(1 << func_num);
    } else {
        return HAL_SDIO_DISABLE_FUNCTION_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_ENABLE, enable, NULL)) {
        return HAL_SDIO_DISABLE_FUNCTION_FAIL;
    }

    if (phost_sdio_core[id].func[func_num]) {
        phost_sdio_core[id].func[func_num]->func_status = FUNC_DISABLE;
    }

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_enable_master_int(enum HAL_SDIO_HOST_ID_T id)
{
    uint8_t enable;

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, 0, &enable)) {
        enable |= 0x1;
    } else {
        return HAL_SDIO_ENABLE_MASTER_INT_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, enable, NULL)) {
        return HAL_SDIO_ENABLE_MASTER_INT_FAIL;
    }

    phost_sdio_core[id].sdio_int_ctrl = FUNC_INT_ENABLE;

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_disable_master_int(enum HAL_SDIO_HOST_ID_T id)
{
    uint8_t enable;

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, 0, &enable)) {
        enable &= ~0x1;
    } else {
        return HAL_SDIO_DISABLE_MASTER_INT_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, enable, NULL)) {
        return HAL_SDIO_DISABLE_MASTER_INT_FAIL;
    }

    phost_sdio_core[id].sdio_int_ctrl = FUNC_INT_DISABLE;

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_enable_func_int(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num)
{
    uint8_t enable;

    if (func_num >= HAL_SDIO_FUNC_NUM) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, 0, &enable)) {
        enable |= (1 << func_num);
    } else {
        return HAL_SDIO_ENABLE_FUNCTION_INT_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, enable, NULL)) {
        return HAL_SDIO_ENABLE_FUNCTION_INT_FAIL;
    }

    if (phost_sdio_core[id].func[func_num]) {
        phost_sdio_core[id].func[func_num]->func_int_status = FUNC_INT_ENABLE;
    }

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_disable_func_int(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num)
{
    uint8_t enable;

    if (func_num >= HAL_SDIO_FUNC_NUM) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, 0, &enable)) {
        enable &= ~(1 << func_num);
    } else {
        return HAL_SDIO_DISABLE_FUNCTION_INT_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_ENABLE, enable, NULL)) {
        return HAL_SDIO_DISABLE_FUNCTION_INT_FAIL;
    }

    if (phost_sdio_core[id].func[func_num]) {
        phost_sdio_core[id].func[func_num]->func_int_status = FUNC_INT_DISABLE;
    }

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_set_block_size(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num, uint16_t block_size)
{
    /* set block size */
    hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_FBR_BASE(func_num) + SDIO_CCCR_BLK_SIZE, block_size & 0xff, NULL);
    hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_FBR_BASE(func_num) + SDIO_CCCR_BLK_SIZE + 1, (block_size >> 8) & 0xff, NULL);

    if (phost_sdio_core[id].func[func_num]) {
        phost_sdio_core[id].func[func_num]->cur_blk_size = block_size;
    } else {
        return HAL_SDIO_INVALID_FUNCTION_NUM;
    }

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_get_block_size(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num, uint16_t *block_size)
{
    if (!block_size) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (phost_sdio_core[id].func[func_num]) {
        *block_size = phost_sdio_core[id].func[func_num]->cur_blk_size;
    } else {
        return HAL_SDIO_INVALID_FUNCTION_NUM;
    }

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_get_int_pending(enum HAL_SDIO_HOST_ID_T id, uint8_t *int_pending)
{
    if (!int_pending) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_PENDING, 0, int_pending)) {
        return HAL_SDIO_GET_INT_PENDING_FAIL;
    }

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_set_func_abort(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_FUNC_NUM_T func_num)
{
    uint8_t abort;

    if (func_num >= HAL_SDIO_FUNC_NUM) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_ABORT, 0, &abort)) {
        abort |= func_num;
    } else {
        return HAL_SDIO_SET_ABORT_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_ABORT, abort, NULL)) {
        return HAL_SDIO_SET_ABORT_FAIL;
    }

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_enable_asyn_int(enum HAL_SDIO_HOST_ID_T id)
{
    uint8_t int_ext;

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_EXTERN, 0, &int_ext)) {
        int_ext |= 0x2;//set EAI bit
        HAL_SDIO_HOST_TRACE(1, "%s:%d,CCCR->int ext(asyn int en)=0x%X", __func__, __LINE__, int_ext);
    } else {
        return HAL_SDIO_ENABLE_INT_EXT_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_EXTERN, int_ext, NULL)) {
        return HAL_SDIO_ENABLE_INT_EXT_FAIL;
    }

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_disable_asyn_int(enum HAL_SDIO_HOST_ID_T id)
{
    uint8_t int_ext;

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_EXTERN, 0, &int_ext)) {
        HAL_SDIO_HOST_TRACE(1, "CCCR->int ext=0x%X", int_ext);
        int_ext &= 0xFD;//clear EAI bit
    } else {
        return HAL_SDIO_DISABLE_INT_EXT_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_INT_EXTERN, int_ext, NULL)) {
        return HAL_SDIO_DISABLE_INT_EXT_FAIL;
    }

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_set_bus_speed(enum HAL_SDIO_HOST_ID_T id, enum HAL_SDIO_BUS_SPEED_MODE bus_speed)
{
    uint8_t val;

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_BUS_SPEED_SEL, 0, &val)) {
        val &= 0xF1;
        val |= (uint8_t)(bus_speed << 1);
    } else {
        return HAL_SDIO_SET_BUS_SPEED_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_BUS_SPEED_SEL, val, NULL)) {
        return HAL_SDIO_SET_BUS_SPEED_FAIL;
    }

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_get_bus_speed(enum HAL_SDIO_HOST_ID_T id, uint8_t *bus_speed)
{
    uint8_t val;

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_BUS_SPEED_SEL, 0, &val)) {
        return HAL_SDIO_GET_BUS_SPEED_FAIL;
    }

    if (bus_speed) {
        *bus_speed = (val & 0x0E) >> 1;
    }

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_get_capability(enum HAL_SDIO_HOST_ID_T id, uint8_t *cap)
{
    uint8_t val;

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_CARD_CAP, 0, &val)) {
        return HAL_SDIO_GET_CAPABILITY_FAIL;
    }

    if (cap) {
        *cap = val;
    }

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_host_gen_int_to_device(enum HAL_SDIO_HOST_ID_T id)
{
    uint8_t flag;

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, SDIO_FUNC1_INT_TO_DEVICE, 0, &flag)) {
        flag |= 0x1;//set bit0
    } else {
        return HAL_SDIO_ENABLE_FUNC1_INT_TO_DEVICE_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_1, SDIO_FUNC1_INT_TO_DEVICE, flag, NULL)) {
        return HAL_SDIO_ENABLE_FUNC1_INT_TO_DEVICE_FAIL;
    }

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_host_check_device_load_ready(enum HAL_SDIO_HOST_ID_T id, uint8_t *status)
{
    if (!status) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, SDIO_FUNC1_LOAD_CFG, 0, status)) {
        return HAL_SDIO_GET_LOAD_CFG_FAIL;
    }

    *status = *status & 0x01;
    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_host_check_device_rx_success(enum HAL_SDIO_HOST_ID_T id, uint8_t *status)
{
    if (!status) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, SDIO_FUNC1_LOAD_CFG, 0, status)) {
        return HAL_SDIO_GET_LOAD_CFG_FAIL;
    }

    *status = (*status & 0x02) >> 1;
    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_host_check_device_fw_status(enum HAL_SDIO_HOST_ID_T id, uint32_t *status)
{
    uint8_t val;

    if (!status) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, SDIO_FUNC1_LOAD_CFG + 1, 0, &val)) {
        return HAL_SDIO_GET_LOAD_CFG_FAIL;
    }
    *status = val;

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, SDIO_FUNC1_LOAD_CFG + 2, 0, &val)) {
        return HAL_SDIO_GET_LOAD_CFG_FAIL;
    }
    *status |= (val << 8);

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, SDIO_FUNC1_LOAD_CFG + 3, 0, &val)) {
        return HAL_SDIO_GET_LOAD_CFG_FAIL;
    }
    *status |= (val << 16);

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_host_get_func1_rx_buf_cfg(enum HAL_SDIO_HOST_ID_T id, uint8_t *buf_cnt, uint16_t *buf_len)
{
    if ((!buf_cnt) && (!buf_len)) {
        return HAL_SDIO_INVALID_PARAMETER;
    }

    uint8_t u8temp;
    uint16_t u16temp;

    //read buf cnt
    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, SDIO_FUNC1_RX_BUF_CFG, 0, &u8temp)) {
        return HAL_SDIO_GET_FUNC1_RX_BUF_CFG_FAIL;
    }

    if (buf_cnt) {
        *buf_cnt = u8temp;
    }

    //read buf len
    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, SDIO_FUNC1_RX_BUF_CFG + 1, 0, &u8temp)) {
        return HAL_SDIO_GET_FUNC1_RX_BUF_CFG_FAIL;
    }

    u16temp = u8temp;
    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_1, SDIO_FUNC1_RX_BUF_CFG + 2, 0, &u8temp)) {
        return HAL_SDIO_GET_FUNC1_RX_BUF_CFG_FAIL;
    }

    if (buf_len) {
        u16temp |= (uint16_t)(u8temp << 8);
        *buf_len = u16temp;
    }

    return  HAL_SDIO_NONE;
}

enum HAL_SDIO_ERR hal_sdio_reset(enum HAL_SDIO_HOST_ID_T id)
{
    uint8_t abort;

    if (!hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_READ, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_ABORT, 0, &abort)) {
        abort |= 0x8;//RES bit
    } else {
        return HAL_SDIO_RESET_FAIL;
    }

    if (hal_sdio_io_rw_direct(id, HAL_SDIO_DEVICE_WRITE, HAL_SDIO_FUNC_0, SDIO_CCCR_IO_ABORT, abort, NULL)) {
        return HAL_SDIO_RESET_FAIL;
    }

    return  HAL_SDIO_NONE;
}

/*
 * Mask off any voltages we don't support and select
 * the lowest voltage
 */
uint32_t sdio_select_voltage(struct SDIO_CONFIG_T *cfg, uint32_t ocr)
{
    uint32_t bit;

    /*
     * Sanity check the voltages that the card claims to
     * support.
     */
    if (ocr & 0x7F) {
        HAL_SDIO_HOST_TRACE(0, "***card claims to support voltages below defined range");
        ocr &= ~0x7F;
    }

    HAL_SDIO_HOST_TRACE(1, "---host default ocr:0x%X", cfg->voltages);
    ocr &= cfg->voltages;
    if (!ocr) {
        HAL_SDIO_HOST_TRACE(0, "***no support for card's volts");
        return HAL_SDIO_NONE;
    } else {
        HAL_SDIO_HOST_TRACE(1, "---new ocr:0x%X", ocr);
    }

    bit = get_lsb_pos(ocr) - 1;
    ocr &= 3 << bit;
    HAL_SDIO_HOST_TRACE(1, "---final ocr:0x%X,bit%d", ocr, bit + 1);

    return ocr;
}

static int32_t hal_sdio_device_init(enum HAL_SDIO_HOST_ID_T id, struct SDIO_T *sdio, struct SDIO_CONFIG_T *cfg, bool blocking_en)
{
    uint32_t val;
    int32_t err = 0;
    uint32_t rsp = 0;
    uint32_t rocr = 0;
    uint8_t func_index;
    struct SDIO_CARD_RSP_T sdio_card_rsp;
    struct SDIO_IP_HOST_T *host = &sdio_host[id];

    memset(host_sdio_func[id], 0, sizeof(host_sdio_func[id]));
    memset(&phost_sdio_core[id], 0, sizeof(struct SDIO_CORE_T));
    host_sdio_func[id][HAL_SDIO_FUNC_0].func_status = FUNC_INT_ENABLE;

    //get device ocr:send cmd5
    do {
        err = hal_sdio_host_send_cmd(id, SDIO_CMD_IO_SEND_OP_COND, 0, SDIO_RSP_R4, &rsp);
        sdio_card_rsp.cmd5r4_io_ocr = rsp & 0x00FFFFFF;
        sdio_card_rsp.cmd5r4_s18a = (rsp >> 24) & 1;
        sdio_card_rsp.cmd5r4_mp = (rsp >> 27) & 1;
        sdio_card_rsp.cmd5r4_funcs = (rsp >> 28) & 7;
        sdio_card_rsp.cmd5r4_busy = (rsp >> 31) & 1;
        HAL_SDIO_HOST_TRACE(1, "R4->IO OCR=0x%X", (uint32_t)sdio_card_rsp.cmd5r4_io_ocr);
        HAL_SDIO_HOST_TRACE(1, "R4->S18A=%d", sdio_card_rsp.cmd5r4_s18a);
        HAL_SDIO_HOST_TRACE(1, "R4->Memory Present=%d", sdio_card_rsp.cmd5r4_mp);
        HAL_SDIO_HOST_TRACE(1, "R4->IO functions=%d", sdio_card_rsp.cmd5r4_funcs);
        HAL_SDIO_HOST_TRACE(1, "R4->sdio ready=%d", sdio_card_rsp.cmd5r4_busy);

        if (!sdio_card_rsp.cmd5r4_io_ocr) {
            if (blocking_en) {
                HAL_SDIO_HOST_TRACE(0, "  ");
                HAL_SDIO_HOST_TRACE(0, "******wait for device init complete");
                osDelay(100);
            } else {
                return HAL_SDIO_CMD5_FAIL;
            }
        }
    } while (!sdio_card_rsp.cmd5r4_io_ocr);

    rocr = sdio_select_voltage(cfg, sdio_card_rsp.cmd5r4_io_ocr);
    HAL_SDIO_HOST_ASSERT(rocr, "%s:%d,common ocr cannot be 0,err:%d", __func__, __LINE__, err);

    if (host->cfg.voltages & SDIO_VDD_165_195) {
        rocr |=  1 << 24;//UHS-I host
    }
    HAL_SDIO_HOST_TRACE(1, "---sdio common ocr:0x%X", rocr);

    //send io operate condition:send cmd5[S18R=1]
    do {
        err = hal_sdio_host_send_cmd(id, SDIO_CMD_IO_SEND_OP_COND, rocr, SDIO_RSP_R4, &rsp);
        sdio_card_rsp.cmd5r4_busy = (rsp >> 31) & 1;
        if (!sdio_card_rsp.cmd5r4_busy) {
            if (blocking_en) {
                HAL_SDIO_HOST_TRACE(0, "  ");
                HAL_SDIO_HOST_TRACE(0, "******wait for device set ocr complete");
                osDelay(100);
            } else {
                return HAL_SDIO_SET_OCR_FAIL;
            }
        }
    } while (!sdio_card_rsp.cmd5r4_busy); //wait for card init ready

    sdio_card_rsp.cmd5r4_io_ocr = rsp & 0x00FFFFFF;
    sdio_card_rsp.cmd5r4_s18a = (rsp >> 24) & 1;
    sdio_card_rsp.cmd5r4_mp = (rsp >> 27) & 1;
    sdio_card_rsp.cmd5r4_funcs = (rsp >> 28) & 7;
    sdio_card_rsp.cmd5r4_busy = (rsp >> 31) & 1;
    HAL_SDIO_HOST_TRACE(1, "R4->IO OCR=0x%X", (uint32_t)sdio_card_rsp.cmd5r4_io_ocr);
    HAL_SDIO_HOST_TRACE(1, "R4->S18A=%d", sdio_card_rsp.cmd5r4_s18a);
    HAL_SDIO_HOST_TRACE(1, "R4->Memory Present=%d", sdio_card_rsp.cmd5r4_mp);
    HAL_SDIO_HOST_TRACE(1, "R4->IO functions=%d", sdio_card_rsp.cmd5r4_funcs);
    HAL_SDIO_HOST_TRACE(1, "R4->sdio ready=%d", sdio_card_rsp.cmd5r4_busy);

    phost_sdio_core[id].func_total_num = sdio_card_rsp.cmd5r4_funcs;
    for (uint8_t index = 0; index <= phost_sdio_core[id].func_total_num; index++) {
        phost_sdio_core[id].func[index] = &host_sdio_func[id][index];
        host_sdio_func[id][index].func_num = index;
    }

    if (sdio_host_callback[id]->hal_sdio_host_vdd_voltage_switch) {
        sdio_host_callback[id]->hal_sdio_host_vdd_voltage_switch();
    }

    //check number of io functions and ocr
    if (sdio_card_rsp.cmd5r4_funcs && sdio_card_rsp.cmd5r4_io_ocr && (!sdio_card_rsp.cmd5r4_mp)) {
        if (sdio_card_rsp.cmd5r4_s18a) {
#ifdef SDIO_HOST_CMD11_ENABLE
            //stop low power clk function,2021-07-29
            HAL_SDIO_HOST_TRACE(0, "---clkena bit16 set to 0");
            val = sdio_ip_readl(&sdio_host[id], SDMMCIP_REG_CLKENA);
            val &= ~SDMMCIP_REG_CLKEN_LOW_PWR;
            sdio_ip_writel(&sdio_host[id], SDMMCIP_REG_CLKENA, val);
            sdio_ip_wait_cmd_start(&sdio_host[id], 0);

            //send cmd11
            HAL_SDIO_HOST_TRACE(0, "---send cmd11");
            err = hal_sdio_host_send_cmd(id, SDIO_CMD_SWITCH_UHS18V, 0, SDIO_RSP_R1, &rsp);
            HAL_SDIO_HOST_TRACE(0, "---cmd11 rsp:0x%X,err:%d", rsp, err);

            int32_t timeout;
            timeout = 10;//ms, wait device driver cmd and data line to low, and host stop clk
            do {
                if (!host->volt_switch_flag) {
                    osDelay(1);
                }
                if (timeout-- < 0) {
                    HAL_SDIO_HOST_TRACE(2, "%s:%d: Timeout!", __func__, __LINE__);
                    return HAL_SDIO_VOLT_SWITCH_FAIL;
                }
            } while (!host->volt_switch_flag);
            host->volt_switch_flag = 0;

            //start a 5ms(min) timer
            HAL_SDIO_HOST_TRACE(0, "---delay 5ms");
            osDelay(5);

            //supply clk
            HAL_SDIO_HOST_TRACE(0, "---clkena bit0 set to 1,clk open");
            sdio_ip_writel(&sdio_host[id], SDMMCIP_REG_CLKENA, SDMMCIP_REG_CLKEN_ENABLE);
            err = sdio_ip_wait_cmd_start(&sdio_host[id], 1);

            /* Wait for at least 1 ms according to spec */
            HAL_SDIO_HOST_TRACE(0, "---delay 1ms,last err:%d", err);
            osDelay(1);

            //wait device drive cmd and data line to high
            HAL_SDIO_HOST_TRACE(0, "---wait device drive cmd and data line to high");
            timeout = 20;//timeout 2ms
            do {
                val = sdio_ip_readl(&sdio_host[id], SDMMCIP_REG_RINTSTS);
                HAL_SDIO_HOST_TRACE(1, "---val:0x%X", val);
                if ((val & (SDMMCIP_REG_RINTSTS_CDONE | SDMMCIP_REG_RINTSTS_HTO)) !=
                    (SDMMCIP_REG_RINTSTS_CDONE | SDMMCIP_REG_RINTSTS_HTO)) {
                    hal_sys_timer_delay_us(100);
                }
                if (timeout-- < 0) {
                    HAL_SDIO_HOST_TRACE(2, "%s:%d: Timeout!", __func__, __LINE__);
                    return HAL_SDIO_VOLT_SWITCH_TIMEOUT;
                }
            } while (!((val & SDMMCIP_REG_RINTSTS_CDONE) && (val & SDMMCIP_REG_RINTSTS_HTO)));
            sdio_ip_writel(&sdio_host[id], SDMMCIP_REG_RINTSTS, SDMMCIP_REG_RINTSTS_ALL); //clear interrupt status
            HAL_SDIO_HOST_TRACE(0, "---voltage switch success");

            //enable low power clk
            sdio_ip_writel(&sdio_host[id], SDMMCIP_REG_CLKENA, SDMMCIP_REG_CLKEN_ENABLE |
                           SDMMCIP_REG_CLKEN_LOW_PWR);
            sdio_ip_wait_cmd_start(&sdio_host[id], 0);
#endif
        } else {
            //This situation will be judged after cmd7
        }

        //Ask the card to publish a new relative address (RCA):cmd3
        err = hal_sdio_host_send_cmd(id, SDIO_CMD_SEND_RELATIVE_ADDR, 0, SDIO_RSP_R6, &rsp);

        //parse r6:get rca and status
        sdio_card_rsp.cmd3r6_rca = (rsp >> 16) & 0xFFFF;
        sdio_card_rsp.cmd3r6_com_crc_error = (rsp & 0x8000) >> 15;
        sdio_card_rsp.cmd3r6_illegal_command = (rsp & 0x4000) >> 14;
        sdio_card_rsp.cmd3r6_error = (rsp & 0x2000) >> 13;
        if ((sdio_card_rsp.cmd3r6_com_crc_error == 0) && (sdio_card_rsp.cmd3r6_illegal_command == 0) && (sdio_card_rsp.cmd3r6_error == 0)) {
            HAL_SDIO_HOST_TRACE(1, "R6->RCA=0x%X", (uint32_t)sdio_card_rsp.cmd3r6_rca);
            HAL_SDIO_HOST_TRACE(1, "R6->COM CRC ERROR=%d", sdio_card_rsp.cmd3r6_com_crc_error);
            HAL_SDIO_HOST_TRACE(1, "R6->ILLEGAL COMMAND=%d", sdio_card_rsp.cmd3r6_illegal_command);
            HAL_SDIO_HOST_TRACE(1, "R6->ERROR=%d", sdio_card_rsp.cmd3r6_error);
            HAL_SDIO_HOST_TRACE(0, "-------SDIO INIT SUCCESS------");

            //select card:cmd7
            err = hal_sdio_host_send_cmd(id, SDIO_CMD_SELECT_CARD, sdio_card_rsp.cmd3r6_rca << 16, SDIO_RSP_R1b, &rsp);
            sdio_card_rsp.cmd7r1b_card_status = rsp;
            HAL_SDIO_HOST_TRACE(1, "R1b->card status=0x%X", sdio_card_rsp.cmd7r1b_card_status);
            if ((sdio_card_rsp.cmd7r1b_card_status & 0x1E00) == 0x1E00) {
                HAL_SDIO_HOST_TRACE(0, "CURRENT_STATE is sdio mode,err:%d", err);
            }

            //The device signal voltage is already 1.8V or the card does not support UHS-I
            if (!sdio_card_rsp.cmd5r4_s18a) {
                HAL_SDIO_HOST_TRACE(0, "voltage of device is already 1.8V or the device does not support UHS-I");

                //Read cccr, check if the card supports UHS-I:cmd52
                err = hal_sdio_host_send_cmd(id, SDIO_CMD_IO_RW_DIRECT, SDIO_CCCR_UHSI_SUPPORT << 9, SDIO_RSP_R5, &rsp);
                if (rsp & 0x7) {    //check SDR50/SDR104/DDR50
                    HAL_SDIO_HOST_TRACE(0, "------device support UHS-I");

                    //The card supports UHS-I, and the host voltage is switched to 1.8v
                    if ((sdio_host_callback[id]->hal_sdio_host_signal_voltage_switch) \
                        && (host->cfg.voltages & SDIO_VDD_165_195)) {
                        sdio_host_callback[id]->hal_sdio_host_signal_voltage_switch();
                    } else {
                        HAL_SDIO_HOST_TRACE(0, "******warning: host does not support UHS-I");
                    }
                } else {
                    //The card does not support UHS-I, so the host does not do voltage switching
                    HAL_SDIO_HOST_TRACE(0, "device does not support UHS-I, so the host does not do voltage switching");
                }
            }

            hal_sdio_get_cccr_version(id, &phost_sdio_core[id].cccr_version);
            hal_sdio_get_sdio_version(id, &phost_sdio_core[id].sdio_version);

            HAL_SDIO_HOST_TRACE(2, "cccr_version:%d,%s", phost_sdio_core[id].cccr_version, (phost_sdio_core[id].cccr_version == 3) ? "3.00" : "*.00");
            HAL_SDIO_HOST_TRACE(2, "sdio_version:%d,%s", phost_sdio_core[id].sdio_version, (phost_sdio_core[id].sdio_version == 4) ? "3.00" : "*.00");

            //config device:UHS-I mode
            hal_sdio_set_device_bus_width(id, SDIO_DEVICE_BUS_WIDTH_4);
            HAL_SDIO_HOST_TRACE(0, "set sdio device bus width 4");

            //config device:bus speed mode
#ifdef SDIO_DDR_MODE
            hal_sdio_set_bus_speed(id, HAL_SDIO_BUS_DDR50_MODE);
            HAL_SDIO_HOST_TRACE(0, "set sdio bus speed mode: DDR50");
#else
            hal_sdio_set_bus_speed(id, HAL_SDIO_BUS_SDR104_MODE);
            HAL_SDIO_HOST_TRACE(0, "set sdio bus speed mode: SDRxx");
#endif

            //config host:UHS-I mode
            sdio_set_host_bus_width(sdio, SDIO_HOST_BUS_WIDTH_4);
            sdio_set_clock(sdio, sdio->cfg->f_max);
            //sdio_ip_writel(&sdio_host[id], SDMMCIP_REG_CLKSRC, SDMMCIP_REG_CLKSRC_CCLK_PRESAMPLE_SEL | SDMMCIP_REG_CLKSRC_CCLK_DRV_SEL);
#if 0
            for (func_index = 0; func_index <= phost_sdio_core[id].func_total_num; func_index++) {
#else
            for (func_index = 0; func_index < 1; func_index++) {
#endif
                uint32_t cis_ptr;

                hal_sdio_get_cis_ptr(id, func_index, &cis_ptr);
                HAL_SDIO_HOST_TRACE(0, "------cis_ptr[%d]:0x%X", func_index, cis_ptr);
                hal_sdio_cis_parse(id, func_index, cis_ptr);
            }

            enum HAL_SDIO_ERR sdio_err = HAL_SDIO_NONE;
#ifdef SDIO_HOST_SPECIAL_FUNCTION
            for (func_index = HAL_SDIO_FUNC_1; func_index <= phost_sdio_core[id].func_total_num; func_index++) {
                /* enable Func */
                sdio_err = hal_sdio_enable_func(id, func_index);
                HAL_SDIO_HOST_TRACE(2, "sdio device func%d has been turned on,err:%d", func_index, sdio_err);

                /* Enable interrupt of each FUNC */
                sdio_err = hal_sdio_enable_func_int(id, func_index);
                HAL_SDIO_HOST_TRACE(2, "sdio device func%d int has been turned on,err:%d", func_index, sdio_err);
            }
            HAL_SDIO_HOST_TRACE(0, "sdio:enable function ok");
            HAL_SDIO_HOST_TRACE(1, "sdio:enable function int ok");
#endif
            sdio_err = hal_sdio_enable_master_int(id);
            HAL_SDIO_HOST_TRACE(1, "sdio:enable master int ok,err:%d", sdio_err);
#ifdef SDIO_HOST_SPECIAL_FUNCTION
            hal_sdio_enable_asyn_int(id);
#endif

            val = sdio_ip_readl(&sdio_host[id], SDMMCIP_REG_INTMASK);
            val |= (SDMMCIP_REG_INTMSK_SDIO_FUNC1 | SDMMCIP_REG_INTMSK_SDIO_FUNC2
                    | SDMMCIP_REG_INTMSK_SDIO_FUNC3 | SDMMCIP_REG_INTMSK_SDIO_FUNC4
                    | SDMMCIP_REG_INTMSK_SDIO_FUNC5 | SDMMCIP_REG_INTMSK_SDIO_FUNC6
                    | SDMMCIP_REG_INTMSK_SDIO_FUNC7);
            sdio_ip_writel(&sdio_host[id], SDMMCIP_REG_INTMASK, val);

            /* set block size */
#ifdef SDIO_HOST_SPECIAL_FUNCTION
            for (func_index = HAL_SDIO_FUNC_0; func_index <= phost_sdio_core[id].func_total_num; func_index++) {
#else
            for (func_index = HAL_SDIO_FUNC_0; func_index < HAL_SDIO_FUNC_1; func_index++) {
#endif
                hal_sdio_set_block_size(id, func_index, SDIO_BLOCK_SIZE);
            }
            HAL_SDIO_HOST_TRACE(1, "sdio:enable function block size ok,size=%d", SDIO_BLOCK_SIZE);
        } else {
            HAL_SDIO_HOST_TRACE(0, "  ");
            HAL_SDIO_HOST_TRACE(0, "***assert***");
            HAL_SDIO_HOST_TRACE(1, "R6->RCA=0x%X", (uint32_t)sdio_card_rsp.cmd3r6_rca);
            HAL_SDIO_HOST_TRACE(1, "R6->COM CRC ERROR=%d", sdio_card_rsp.cmd3r6_com_crc_error);
            HAL_SDIO_HOST_TRACE(1, "R6->ILLEGAL COMMAND=%d", sdio_card_rsp.cmd3r6_illegal_command);
            HAL_SDIO_HOST_TRACE(1, "R6->ERROR=%d", sdio_card_rsp.cmd3r6_error);
            HAL_SDIO_HOST_ASSERT(0, "%s:%d CMD3 response error", __func__, __LINE__);
        }
    } else {
        HAL_SDIO_HOST_ASSERT(0, "%s:%d CMD5 TX error or response:R4 error or device is not an sdio device.", __func__, __LINE__);
    }

    return err;
}

/******************************************************************************/
/****************************General function**********************************/
/******************************************************************************/
SRAM_TEXT_LOC static void sdio_base_irq_handler(enum HAL_SDIO_HOST_ID_T id)
{
    uint32_t i;
    uint32_t branch;
    uint32_t raw_int_status;
    struct SDIO_IP_HOST_T *host = &sdio_host[HAL_SDIO_HOST_ID_0];
#ifdef SDIO_HOST_CMD11_ENABLE
    uint32_t time_start = 0, time_ns;
#endif

    raw_int_status = sdio_ip_readl(host, SDMMCIP_REG_RINTSTS);//read raw interrupt status
#ifdef SDIO_HOST_CMD11_ENABLE
    if (raw_int_status & SDMMCIP_REG_RINTSTS_HTO) {
        time_start = hal_fast_sys_timer_get();
    }
#endif
    sdio_ip_writel(host, SDMMCIP_REG_RINTSTS, raw_int_status & (~SDMMCIP_REG_RINTSTS_CDONE)); //clear interrupt status
    __DSB();
    HAL_SDIO_HOST_TRACE(3, "%s:%d,raw_int_status=0x%X", __func__, __LINE__, raw_int_status);

#if 0
    uint32_t int_status;
    int_status = sdio_ip_readl(host, SDMMCIP_REG_RINTSTS);
    HAL_SDIO_HOST_TRACE(3, "++++++int_status=0x%X", int_status);
#endif

    //Clear the redundant signs, because they are not used, the purpose is to reduce the number of subsequent cycles
    raw_int_status &= (~(SDMMCIP_REG_RINTSTS_CDONE | SDMMCIP_REG_RINTSTS_DTO | SDMMCIP_REG_RINTSTS_TXDR | SDMMCIP_REG_RINTSTS_RXDR));

    while (raw_int_status) {
        i = get_lsb_pos(raw_int_status);
        branch = raw_int_status & (1 << i);
        raw_int_status &= ~(1 << i);
        switch (branch) {
            case SDMMCIP_REG_RINTSTS_EBE: {
                HAL_SDIO_HOST_TRACE(0, "bit 15:End-bit error (read)/write no CRC (EBE)");
                if (sdio_host_callback[id]->hal_sdio_host_error) {
                    sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_READ_END_BIT_ERR_WRITE_NOCRC);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_SBE: {
                HAL_SDIO_HOST_TRACE(0, "bit 13:Start-bit error (SBE)");
                if (sdio_host_callback[id]->hal_sdio_host_error) {
                    sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_START_BIT_ERR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_HLE: {
                HAL_SDIO_HOST_TRACE(0, "bit 12:Hardware locked write error (HLE)");
                if (sdio_host_callback[id]->hal_sdio_host_error) {
                    sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_HARDWARE_LOCKED_WRITE_ERR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_FRUN: {
                sdio_ip_reset_fifo(host);
                HAL_SDIO_HOST_TRACE(0, "bit 11:FIFO underrun/overrun error (FRUN)");
                if (sdio_host_callback[id]->hal_sdio_host_error) {
                    sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_FIFO_ERR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_HTO: {
#ifdef SDIO_HOST_CMD11_ENABLE
                uint32_t val;
                uint32_t lock;

                lock = int_lock();
                //clear useless signs
                raw_int_status &= ~(SDMMCIP_REG_RINTSTS_SDIO_FUNC1 | SDMMCIP_REG_RINTSTS_SDIO_FUNC2 |
                                    SDMMCIP_REG_RINTSTS_SDIO_FUNC3 | SDMMCIP_REG_RINTSTS_SDIO_FUNC4 |
                                    SDMMCIP_REG_RINTSTS_SDIO_FUNC5 | SDMMCIP_REG_RINTSTS_SDIO_FUNC6 |
                                    SDMMCIP_REG_RINTSTS_SDIO_FUNC7);

                //turn off Volt switch interrupts
                val = sdio_ip_readl(host, SDMMCIP_REG_INTMASK);
                val &= ~SDMMCIP_REG_RINTSTS_HTO;
                sdio_ip_writel(host, SDMMCIP_REG_INTMASK, val);
                time_ns = FAST_TICKS_TO_NS(hal_fast_sys_timer_get() - time_start);

                /*
                    clk=50k, time=20us
                    clk=100k,time=10us
                    clk=350k,time=2.9us
                    clk=400k,time=2.5us
                    3 <= clk cnt <=80 //the digital design is 90, and the software controls it at 80 here
                    time(min) = 10us * 6clk = 60us, time/2.9us=20clk,time/2.5us=24clk
                    time(max) = 10us * 80clk = 800us
                */
                time_start = 8 * host->period_st_ns;

                //The number of outputs before clk stops should be enough
                if (time_start > time_ns) {
                    time_ns = (time_start - time_ns) / 1000;
                    hal_sys_timer_delay_us(time_ns);
                }

                //stop clk supply,2021-07-29
                sdio_ip_writel(host, SDMMCIP_REG_CLKENA, 0);
                sdio_ip_wait_cmd_start(host, 1);
                int_unlock(lock);
                HAL_SDIO_HOST_TRACE(0, "---clkena bit0 set to 0,clk closed");

                val = sdio_ip_readl(host, SDMMCIP_REG_UHS_REG);
                HAL_SDIO_HOST_TRACE(0, "---UHS_REG set to 1.8V");
                val |= SDMMCIP_REG_1V8_MODE;
                sdio_ip_writel(host, SDMMCIP_REG_UHS_REG, val);

                HAL_SDIO_HOST_TRACE(0, "bit 10:Volt_switch_int");
                if (sdio_host_callback[id]->hal_sdio_host_signal_voltage_switch) {
                    sdio_host_callback[id]->hal_sdio_host_signal_voltage_switch();
                }

                host->volt_switch_flag = 1;
#else
                HAL_SDIO_HOST_TRACE(0, "bit 10:Data starvation-by-host timeout (HTO)");
                if (sdio_host_callback[id]->hal_sdio_host_error) {
                    sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_STARVATION_TIMEOUT);
                }
#endif
                break;
            }
            case SDMMCIP_REG_RINTSTS_DRTO: {
                HAL_SDIO_HOST_TRACE(0, "bit 9:Data read timeout (DRTO)/Boot Data Start (BDS)");
                if (sdio_host_callback[id]->hal_sdio_host_error) {
                    sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_DATA_READ_TIMEOUT_BDS);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_RTO: {
                HAL_SDIO_HOST_TRACE(0, "bit 8:Response timeout (RTO)/Boot Ack Received (BAR)");
                if (sdio_host_callback[id]->hal_sdio_host_error) {
                    sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_RESPONSE_TIMEOUT_BAR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_DCRC: {
                HAL_SDIO_HOST_TRACE(0, "bit 7:Data CRC error (DCRC)");
                if (sdio_host_callback[id]->hal_sdio_host_error) {
                    sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_DATA_CRC_ERR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_RCRC: {
                HAL_SDIO_HOST_TRACE(0, "bit 6:Response CRC error (RCRC)");
                if (sdio_host_callback[id]->hal_sdio_host_error) {
                    sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_RESPONSE_CRC_ERR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_RE: {
                HAL_SDIO_HOST_TRACE(0, "bit 1:Response error (RE)");
                if (sdio_host_callback[id]->hal_sdio_host_error) {
                    sdio_host_callback[id]->hal_sdio_host_error(HAL_SDIO_HOST_RESPONSE_ERR);
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_CD: {
                HAL_SDIO_HOST_TRACE(0, "bit 0:Card detect (RE)");
                if (sdio_host_callback[id]->hal_sdio_host_card_detect) {
                    sdio_host_callback[id]->hal_sdio_host_card_detect();
                }
                break;
            }
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC1:
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC2:
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC3:
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC4:
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC5:
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC6:
            case SDMMCIP_REG_RINTSTS_SDIO_FUNC7: {
                uint8_t int_pending = 0;

                raw_int_status &= ~(SDMMCIP_REG_RINTSTS_SDIO_FUNC1 | SDMMCIP_REG_RINTSTS_SDIO_FUNC2 |
                                    SDMMCIP_REG_RINTSTS_SDIO_FUNC3 | SDMMCIP_REG_RINTSTS_SDIO_FUNC4 |
                                    SDMMCIP_REG_RINTSTS_SDIO_FUNC5 | SDMMCIP_REG_RINTSTS_SDIO_FUNC6 |
                                    SDMMCIP_REG_RINTSTS_SDIO_FUNC7);
#ifdef SDIO_HOST_FUNC_OPT
                int_pending = SDIO_CCCR5_INT1;
#else
                hal_sdio_get_int_pending(HAL_SDIO_HOST_ID_0, &int_pending);
                HAL_SDIO_HOST_TRACE(0, "int pending:0x%X", int_pending);
#endif
                if (int_pending & SDIO_CCCR5_INT1) {
                    HAL_SDIO_HOST_TRACE(0, "bit 16:func1 int");
                    if (sdio_host_callback[id]->hal_sdio_host_func_int) {
                        sdio_host_callback[id]->hal_sdio_host_func_int(HAL_SDIO_FUNC_1);
                    }
                }
#ifndef SDIO_HOST_FUNC_OPT
                if (int_pending & SDIO_CCCR5_INT2) {
                    HAL_SDIO_HOST_TRACE(0, "bit 17:func2 int");
                    if (sdio_host_callback[id]->hal_sdio_host_func_int) {
                        sdio_host_callback[id]->hal_sdio_host_func_int(HAL_SDIO_FUNC_2);
                    }
                }
                if (int_pending & SDIO_CCCR5_INT3) {
                    HAL_SDIO_HOST_TRACE(0, "bit 18:func3 int");
                    if (sdio_host_callback[id]->hal_sdio_host_func_int) {
                        sdio_host_callback[id]->hal_sdio_host_func_int(HAL_SDIO_FUNC_3);
                    }
                }
                if (int_pending & SDIO_CCCR5_INT4) {
                    HAL_SDIO_HOST_TRACE(0, "bit 19:func4 int");
                    if (sdio_host_callback[id]->hal_sdio_host_func_int) {
                        sdio_host_callback[id]->hal_sdio_host_func_int(HAL_SDIO_FUNC_4);
                    }
                }
                if (int_pending & SDIO_CCCR5_INT5) {
                    HAL_SDIO_HOST_TRACE(0, "bit 20:func5 int");
                    if (sdio_host_callback[id]->hal_sdio_host_func_int) {
                        sdio_host_callback[id]->hal_sdio_host_func_int(HAL_SDIO_FUNC_5);
                    }
                }
                if (int_pending & SDIO_CCCR5_INT6) {
                    HAL_SDIO_HOST_TRACE(0, "bit 21:func6 int");
                    if (sdio_host_callback[id]->hal_sdio_host_func_int) {
                        sdio_host_callback[id]->hal_sdio_host_func_int(HAL_SDIO_FUNC_6);
                    }
                }
                if (int_pending & SDIO_CCCR5_INT7) {
                    HAL_SDIO_HOST_TRACE(0, "bit 22:func7 int");
                    if (sdio_host_callback[id]->hal_sdio_host_func_int) {
                        sdio_host_callback[id]->hal_sdio_host_func_int(HAL_SDIO_FUNC_7);
                    }
                }
#endif
                break;
            }
            default : {
                //Don't care about other states
                break;
            }
        }
    }
}

static void hal_sdio0_irq_handler(void)
{
    sdio_base_irq_handler(HAL_SDIO_HOST_ID_0);
}

#ifdef SDMMC1_BASE
static void hal_sdio1_irq_handler(void)
{
    sdio_base_irq_handler(HAL_SDIO_HOST_ID_1);
}
#endif

POSSIBLY_UNUSED static int hal_sdio_host_int_enable(enum HAL_SDIO_HOST_ID_T id)
{
    struct SDIO_IP_HOST_T *host = NULL;
    uint32_t irq_num = SDMMC0_IRQn;

    HAL_SDIO_HOST_TRACE(2, "%s:%d", __func__, __LINE__);

    host = &sdio_host[id];
    sdio_ip_writel(host, SDMMCIP_REG_RINTSTS, SDMMCIP_REG_RINTSTS_ALL); //clear interrupt status
    sdio_ip_writel(host, SDMMCIP_REG_INTMASK, SDMMCIP_REG_INTMSK_ALL & (~(SDMMCIP_REG_INTMSK_CDONE | \
                   SDMMCIP_REG_INTMSK_DTO | SDMMCIP_REG_INTMSK_TXDR | SDMMCIP_REG_INTMSK_RXDR |      \
                   SDMMCIP_REG_INTMSK_ACD | SDMMCIP_REG_INTMSK_SDIO_FUNC1 |                          \
                   SDMMCIP_REG_INTMSK_SDIO_FUNC2 | SDMMCIP_REG_INTMSK_SDIO_FUNC3 |                   \
                   SDMMCIP_REG_INTMSK_SDIO_FUNC4 | SDMMCIP_REG_INTMSK_SDIO_FUNC5 |                   \
                   SDMMCIP_REG_INTMSK_SDIO_FUNC6 | SDMMCIP_REG_INTMSK_SDIO_FUNC7)));    //open interrupt
    sdio_ip_writel(host, SDMMCIP_REG_CTRL, SDMMCIP_REG_INT_EN);                         //enable interrupt

    if (id == HAL_SDIO_HOST_ID_0) {
        irq_num = SDMMC0_IRQn;
        NVIC_SetVector(irq_num, (uint32_t)hal_sdio0_irq_handler);
    }
#ifdef SDMMC1_BASE
    else if (id == HAL_SDIO_HOST_ID_1) {
        irq_num = SDMMC1_IRQn;
        NVIC_SetVector(irq_num, (uint32_t)hal_sdio1_irq_handler);
    }
#endif

    NVIC_SetPriority(irq_num, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(irq_num);
    NVIC_EnableIRQ(irq_num);

    return HAL_SDIO_NONE;
}

static int32_t hal_sdio_host_device_cfg(enum HAL_SDIO_HOST_ID_T id, struct HAL_SDIO_HOST_CB_T *callback, bool blocking_en, bool device_init)
{
    int32_t ret = 0;
    uint32_t bus_clk;
    struct SDIO_IP_HOST_T *host = NULL;
    HAL_SDIO_HOST_ASSERT(id < HAL_SDIO_HOST_ID_NUM, "Invalid sdio host id: %d", id);

#ifndef SDIO_SPEED
#define SDIO_SPEED      (12 * 1000 * 1000)
#endif

#ifdef FPGA
    switch (id) {
        case HAL_SDIO_HOST_ID_0:
            hal_cmu_sdmmc0_set_freq(HAL_CMU_PERIPH_FREQ_26M);
            break;
#ifdef SDMMC1_BASE
        case HAL_SDIO_HOST_ID_1:
            hal_cmu_sdmmc1_set_freq(HAL_CMU_PERIPH_FREQ_26M);
            break;
#endif
        default:
            break;
    }
    bus_clk = hal_cmu_get_crystal_freq();
    HAL_SDIO_HOST_ASSERT(bus_clk >= SDIO_SPEED, "%s:%d, SDIO_SPEED %d > src clk %d", __func__, __LINE__, SDIO_SPEED, bus_clk);
#else
    if (0) {
#ifdef PERIPH_PLL_FREQ
    } else if (SDIO_SPEED > 2 * hal_cmu_get_crystal_freq()) {
        int ret;
        uint32_t div;//division is one step in place, the sdmmc ip is no longer divided(Support even division)

        div = PERIPH_PLL_FREQ / SDIO_SPEED;
        if (PERIPH_PLL_FREQ % SDIO_SPEED) {
            div += 1;
        }

        bus_clk = PERIPH_PLL_FREQ / div;
        switch (id) {
            case HAL_SDIO_HOST_ID_0:
                ret = hal_cmu_sdmmc0_set_div(div);
                break;
#ifdef SDMMC1_BASE
            case HAL_SDIO_HOST_ID_1:
                ret = hal_cmu_sdmmc1_set_div(div);
                break;
#endif
            default:
                break;
        }
        HAL_SDIO_HOST_ASSERT(!ret, "The SDIO_SPEED value is invalid, causing the div to be out of range, ret %d, div is %d", ret, div);
        HAL_SDIO_HOST_ASSERT(bus_clk <= SDIO_SPEED, "%s:%d, div clk %d > SDIO_SPEED %d", __func__, __LINE__, bus_clk, SDIO_SPEED);
        HAL_SDIO_HOST_TRACE(1, "PERIPH_PLL_FREQ is %d, the final PLL div is %d", PERIPH_PLL_FREQ, div);
#endif
    } else if (SDIO_SPEED >= 3 * hal_cmu_get_crystal_freq() / 2) {  //sdmmc ip may also be divided(Only even division)
        switch (id) {
            case HAL_SDIO_HOST_ID_0:
                hal_cmu_sdmmc0_set_freq(HAL_CMU_PERIPH_FREQ_52M);
                break;
#ifdef SDMMC1_BASE
            case HAL_SDIO_HOST_ID_1:
                hal_cmu_sdmmc1_set_freq(HAL_CMU_PERIPH_FREQ_52M);
                break;
#endif
            default:
                break;
        }
        bus_clk = 2 * hal_cmu_get_crystal_freq();
        HAL_SDIO_HOST_ASSERT(bus_clk >= SDIO_SPEED, "%s:%d, SDIO_SPEED %d > src clk %d", __func__, __LINE__, SDIO_SPEED, bus_clk);
    } else {                                                        //sdmmc ip may also be divided(Only even division)
        switch (id) {
            case HAL_SDIO_HOST_ID_0:
                hal_cmu_sdmmc0_set_freq(HAL_CMU_PERIPH_FREQ_26M);
                break;
#ifdef SDMMC1_BASE
            case HAL_SDIO_HOST_ID_1:
                hal_cmu_sdmmc1_set_freq(HAL_CMU_PERIPH_FREQ_26M);
                break;
#endif
            default:
                break;
        }
        bus_clk = hal_cmu_get_crystal_freq();
        HAL_SDIO_HOST_ASSERT(bus_clk >= SDIO_SPEED, "%s:%d, SDIO_SPEED %d > src clk %d", __func__, __LINE__, SDIO_SPEED, bus_clk);
    }
#endif

    HAL_SDIO_HOST_TRACE(2, "SDIO_SPEED %d, bus clk(Not necessarily the final speed) %d", SDIO_SPEED, bus_clk);

    /* sdio host iomux */
    switch (id) {
        case HAL_SDIO_HOST_ID_0:
#if defined(CHIP_BEST2002) || defined(CHIP_BEST2005)
            hal_iomux_set_sdio();
#else
            hal_iomux_set_sdmmc0();
#endif
            hal_cmu_sdmmc0_clock_enable();
            break;
#ifdef SDMMC1_BASE
        case HAL_SDIO_HOST_ID_1:
            hal_iomux_set_sdmmc1();
            hal_cmu_sdmmc1_clock_enable();
            break;
#endif
        default:
            break;
    }
    
#ifdef FPGA
#ifdef CHIP_BEST1000
#elif CHIP_BEST2002
    hal_sys_timer_delay_us(500);//palladium use:wait complete,2021-05-15
#endif
#endif

#ifdef SDIO_DDR_MODE
    HAL_SDIO_HOST_ASSERT(SDIO_SPEED <= SDIO_SPEED_DDR50, "%s:%d, sdio ddr50 speed %d > max speed %d", __func__, __LINE__, SDIO_SPEED, SDIO_SPEED_DDR50);
#endif

    host = &sdio_host[id];
    host->ioaddr     = (void *)sdio_ip_base[id];
    host->clock      = 0;
    host->bus_hz     = bus_clk;
    host->host_id    = id;
    host->bus_width  = SDIO_HOST_BUS_WIDTH_4;
    host->volt_switch_flag = 0;
    host->fifoth_val = MSIZE(0) | RX_WMARK(0) | TX_WMARK(1);

    host->cfg.ops = &sdio_ip_ops;
    host->cfg.voltages = SDIO_VDD_27_28 | SDIO_VDD_28_29 | SDIO_VDD_29_30 | SDIO_VDD_30_31 | SDIO_VDD_31_32 | SDIO_VDD_32_33 | SDIO_VDD_165_195;
    host->cfg.f_min = 400 * 1000;     //Don't modify
    host->cfg.f_max = SDIO_SPEED;
    host->cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;
    host->sdio = sdio_create(host->host_id, &host->cfg, host);
    HAL_SDIO_HOST_TRACE(4, "%s:%d, sdio clk min %d, max %d", __func__, __LINE__, host->cfg.f_min, host->cfg.f_max);

#ifdef HAL_SDIO_HOST_USE_DMA
    host->dma_ch = 0;
    host->dma_in_use = 0;
    host->sdio_dma_lock = 0;
    host->tx_dma_handler = sdio_ip_ext_dma_irq_handlers[id * 2];
    host->rx_dma_handler = sdio_ip_ext_dma_irq_handlers[id * 2 + 1];
#endif

    if (host->sdio->has_init) {
        HAL_SDIO_HOST_TRACE(2, "%s:%d,sdio host has been initialized", __func__, __LINE__);
        return HAL_SDIO_NONE;
    }
    if (!host->sdio->init_in_progress) {
        host->sdio->init_in_progress = 1;

        /* make sure it's not NULL earlier */
        ret = host->sdio->cfg->ops->init(host->sdio);

        if (ret) {
            HAL_SDIO_HOST_TRACE(3, "%s:%d, sdio ip init error,ret=%d", __func__, __LINE__, ret);
            return ret;
        } else {
            HAL_SDIO_HOST_TRACE(3, "%s:%d, sdio ip init ok,ret=%d", __func__, __LINE__, ret);
        }
#ifdef SDIO_DDR_MODE
        host->sdio->ddr_mode = 1;
#else
        host->sdio->ddr_mode = 0;
#endif
        if (device_init) {
            sdio_set_host_bus_width(host->sdio, SDIO_HOST_BUS_WIDTH_1);
            sdio_set_clock(host->sdio, 1);//set 400KHz
        } else {
            //config host:UHS-I mode
            sdio_set_host_bus_width(host->sdio, SDIO_HOST_BUS_WIDTH_4);
            sdio_set_clock(host->sdio, host->sdio->cfg->f_max);
        }

        //Calculate period at start frequency
        uint32_t sdio_start_freq;
        sdio_start_freq = host->bus_hz / hal_cmu_get_crystal_freq();
        sdio_start_freq *= hal_cmu_get_crystal_freq();
        sdio_start_freq = sdio_start_freq / 1000 / 1000;
        host->period_st_ns = (1000 * host->div) / sdio_start_freq;
        HAL_SDIO_HOST_TRACE(1, "sdio period of start frequency: %dns", host->period_st_ns);

        if (device_init) {
            sdio_host_callback[id] = &sdio_host_callback_default[id];
            memset(sdio_host_callback[id], 0, sizeof(struct HAL_SDIO_HOST_CB_T));
            if (callback) {
                sdio_host_callback[id] = callback;
            }
            hal_sdio_host_int_enable(id);
            ret = hal_sdio_device_init(id, host->sdio, &host->cfg, blocking_en);
            if (ret) {
                return ret;
            }

#ifdef SDIO_HOST_SPECIAL_FUNCTION
            //Read the cis register, in order to get the interrupt of the device by the host,2021-06-25
            uint8_t cis_data;
            uint32_t cis_ptr;
            hal_sdio_get_cis_ptr(id, 0, &cis_ptr);
            hal_sdio_io_rw_extended(id, HAL_SDIO_DEVICE_READ, 0, cis_ptr, NULL, 1, &cis_data, 1);
            hal_sdio_set_func_abort(id, 0);
#endif
        } else {
            memset(&phost_sdio_core[id], 0, sizeof(struct SDIO_CORE_T));
            for (uint8_t index = 0; index < 8; index++) {
                phost_sdio_core[id].func[index] = &host_sdio_func[id][index];
                host_sdio_func[id][index].func_num = index;
                phost_sdio_core[id].func[index]->cur_blk_size = SDIO_BLOCK_SIZE;
            }

            //Disable the function of stopping output when clk is idle
            sdio_ip_clk_disable_low_power(host);
        }

        HAL_SDIO_HOST_TRACE(2, "%s:%d,sdio device init complete", __func__, __LINE__);
    } else {
        HAL_SDIO_HOST_TRACE(2, "%s:%d,sdio host is being initialized", __func__, __LINE__);
    }
    host->sdio->has_init = 1;
    host->sdio->init_in_progress = 0;

    return ret;
}

int32_t hal_sdio_host_open(enum HAL_SDIO_HOST_ID_T id, struct HAL_SDIO_HOST_CB_T *callback, bool blocking_en)
{
    return hal_sdio_host_device_cfg(id, callback, blocking_en, 1);
}

int32_t hal_sdio_host_cfg(enum HAL_SDIO_HOST_ID_T id)
{
    return hal_sdio_host_device_cfg(id, NULL, 0, 0);
}

void hal_sdio_host_close(enum HAL_SDIO_HOST_ID_T id)
{
    struct SDIO_IP_HOST_T *host = NULL;
    HAL_SDIO_HOST_ASSERT(id < HAL_SDIO_HOST_ID_NUM, "Invalid sdio host id: %d", id);
    HAL_SDIO_HOST_TRACE(2, "%s:%d", __func__, __LINE__);

    host = &sdio_host[id];
    if (host->sdio == NULL) {
        return;
    }

    if (host->sdio->has_init == 0) {
        HAL_SDIO_HOST_TRACE(2, "%s:%d,sdio host has been closed", __func__, __LINE__);
        return;
    }

    while (host->sdio->init_in_progress)
        hal_sys_timer_delay_us(500);

    host->sdio->has_init = 0;

#ifdef HAL_SDIO_HOST_USE_DMA
    if (host->dma_in_use) {
        sdio_ip_reset_dma(host);
    }
#endif

    switch (id) {
        case HAL_SDIO_HOST_ID_0:
            hal_cmu_sdmmc0_clock_disable();
            break;
#ifdef SDMMC1_BASE
        case HAL_SDIO_HOST_ID_1:
            hal_cmu_sdmmc1_clock_disable();
            break;
#endif
        default:
            break;
    }
}

int hal_sdio_get_host_status(enum HAL_SDIO_HOST_ID_T id)
{
    struct SDIO_IP_HOST_T *host = NULL;

    host = &sdio_host[id];
    if (host->sdio == NULL) {
        return HAL_SDIO_NONE;
    }

    return host->sdio->has_init;
}

uint32_t hal_sdio_get_host_speed(enum HAL_SDIO_HOST_ID_T id)
{
    if (hal_sdio_get_host_status(id)) {
        return sdio_host[id].final_bus_speed;
    } else {
        return HAL_SDIO_NONE;
    }
}

#endif

