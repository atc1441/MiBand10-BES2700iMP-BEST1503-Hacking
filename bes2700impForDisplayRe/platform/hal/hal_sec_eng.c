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
#include "plat_addr_map.h"

#ifdef SEC_ENG_BASE

#include "hal_sec_eng.h"
#include "reg_sec_eng.h"
#include "reg_dma.h"
#include "cmsis_nvic.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"

#define SE_OPER_TIMEOUT                     MS_TO_TICKS(100)

#define SE_DMA_CHAN_NUM                     2
#define SE_DMA_DESC_NUM                     4

#define SE_DMA_MAX_DESC_XFER_SIZE           (HAL_DMA_MAX_DESC_XFER_SIZE & ~(4 - 1))

#define SE_DMA_RX_CHAN                      0
#define SE_DMA_TX_CHAN                      1

// Trigger DMA request when RX-FIFO count >= threshold
#define SE_DMA_RX_FIFO_TRIG_LEVEL           8
// Trigger DMA request when TX-FIFO count <= threshold
#define SE_DMA_TX_FIFO_TRIG_LEVEL           8

enum SE_ACC_CLK_T {
    SE_ACC_CLK_DMA      = (1 << 1),
    SE_ACC_CLK_ACC_BUS  = (1 << 2),
    SE_ACC_CLK_CRYPT    = (1 << 3),
    SE_ACC_CLK_ECP      = (1 << 4),
    SE_ACC_CLK_HASH     = (1 << 5),
    SE_ACC_CLK_SCRATCH  = (1 << 6),
    SE_ACC_CLK_ZMODP    = (1 << 7),
    SE_ACC_CLK_MCT      = (1 << 8),
    SE_ACC_CLK_OTP      = (1 << 9),
    SE_ACC_CLK_EBG      = (1 << 10),
};

enum SE_ACC_RST_T {
    SE_ACC_RST_DMA      = (1 << 1),
    SE_ACC_RST_ACC_BUS  = (1 << 2),
    SE_ACC_RST_CRYPT    = (1 << 3),
    SE_ACC_RST_ECP      = (1 << 4),
    SE_ACC_RST_HASH     = (1 << 5),
    SE_ACC_RST_SCRATCH  = (1 << 6),
    SE_ACC_RST_ZMODP    = (1 << 7),
    SE_ACC_RST_MCT      = (1 << 8),
    SE_ACC_RST_OTP      = (1 << 9),
    SE_ACC_RST_EBG      = (1 << 10),
};

enum SE_ACC_INT_T {
    SE_ACC_INT_AES      = (1 << 1),
    SE_ACC_INT_DES      = (1 << 2),
    SE_ACC_INT_RC4      = (1 << 3),
    SE_ACC_INT_OTP      = (1 << 4),
    SE_ACC_INT_ECP      = (1 << 6),
    SE_ACC_INT_MCT      = (1 << 7),
    SE_ACC_INT_HASH     = (1 << 8),
    SE_ACC_INT_ZMODP    = (1 << 9),
    SE_ACC_INT_SCRATCH  = (1 << 10),
};

enum SE_DMA_PERIPH_T {
    SE_DMA_PERIPH_TX    = 0,
    SE_DMA_PERIPH_RX,

    SE_DMA_PERIPH_QTY,
};

enum SE_CRYPT_TYPE_T {
    SE_CRYPT_NONE       = 0,
    SE_CRYPT_AES,
    SE_CRYPT_DES,
    SE_CRYPT_RC4,

    SE_CRYPT_QTY,
};

enum SE_AES_KEY_LEN_T {
    SE_AES_KEY_128      = 0,
    SE_AES_KEY_256,
    SE_AES_KEY_192,

    SE_AES_KEY_LEN_QTY,
};

enum SE_HASH_OP_T {
    SE_HASH_OP_INIT     = 1,
    SE_HASH_OP_UPDATE,
    SE_HASH_OP_FINAL,
};

struct SE_HASH_INIT_CB_PARAM_T {
    const void *seg_ctx_buf;
    uint32_t seg_ctx_len;
    const void *key;
    uint32_t key_len;
};

typedef void (*SE_HASH_INIT_PRE_START_CB)(const struct SE_HASH_INIT_CB_PARAM_T *param);

static struct DMA_T * const se_dma = (struct DMA_T *)SEDMA_BASE;

static struct SE_ADEC_T * const se_adec = (struct SE_ADEC_T *)SE_ADEC_BASE;
POSSIBLY_UNUSED static struct SE_ACB_T * const se_acb = (struct SE_ACB_T *)SE_ACB_BASE;
static struct SE_DMACFG_T * const se_dmacfg = (struct SE_DMACFG_T *)SE_DMACFG_BASE;
POSSIBLY_UNUSED static struct SE_CRYPT_T * const se_crypt = (struct SE_CRYPT_T *)SE_CRYPT_BASE;
POSSIBLY_UNUSED static struct SE_HASH_T * const se_hash = (struct SE_HASH_T *)SE_HASH_BASE;
POSSIBLY_UNUSED static struct SE_OTP_T * const se_otp = (struct SE_OTP_T *)SE_OTP_BASE;

#ifndef SEC_ENG_ROM_ONLY
static bool se_enabled;

static HAL_SE_DONE_HANDLER_T se_done_hdlr;

static uint32_t se_in_addr;
static uint32_t se_total_in_len;
static uint32_t se_cur_in_len;

static uint32_t se_out_addr;
static uint32_t se_total_out_len;
static uint32_t se_cur_out_len;

static enum HAL_SE_SEG_OPER_T se_seg_oper;
static void *se_seg_ctx_buf;

static struct HAL_DMA_DESC_T dma_desc[SE_DMA_CHAN_NUM][SE_DMA_DESC_NUM];

static uint32_t se_dma_init_rx_desc(uint32_t out, uint32_t out_len, uint32_t src, uint32_t ctrl);
static uint32_t se_dma_init_tx_desc(uint32_t in, uint32_t in_len, uint32_t dst, uint32_t ctrl);
static uint32_t se_aes_get_key_len(void);
#endif

static void read_mreg(const volatile uint32_t *reg, void *mem, uint32_t len);

#ifdef SEC_ENG_HAS_HASH
static uint32_t se_hash_get_digest_len(void);
#ifndef SEC_ENG_ROM_ONLY
static void se_hash_save_ctx(int hmac, void *seg_ctx_buf, uint32_t seg_ctx_len);
#endif
#endif

#ifndef SEC_ENG_ROM_ONLY
static void sec_eng_irq_handler(void)
{
    enum HAL_SE_DONE_ERR_T err;
    uint32_t all_int;
    uint32_t aes_int = 0;
    uint32_t hash_int = 0;

    all_int = se_adec->ADEC_INT;

    if (all_int & SE_ACC_INT_AES) {
        aes_int = se_crypt->AES_INTRPT;

        // Clear IRQs
        se_crypt->AES_INTRPT = aes_int;
        se_adec->ADEC_INT = SE_ACC_INT_AES;

        if (aes_int & AES_INTRPT_DONE) {
            if (se_seg_oper == HAL_SE_SEG_INIT || se_seg_oper == HAL_SE_SEG_CONT) {
                uint32_t key_len;

                ASSERT(se_seg_ctx_buf, "%s: Bad AES se_seg_ctx_buf. se_seg_oper=%d", __func__, se_seg_oper);
                key_len = se_aes_get_key_len();
                read_mreg(&se_crypt->IV[0], se_seg_ctx_buf, 16);
                read_mreg(&se_crypt->KEY1[0], se_seg_ctx_buf + 16, key_len);
            }
        }
        if (aes_int & (AES_INTRPT_ERROR1 | AES_INTRPT_ERROR2)) {
            if (se_done_hdlr) {
                if ((aes_int & (AES_INTRPT_ERROR1 | AES_INTRPT_ERROR2)) == (AES_INTRPT_ERROR1 | AES_INTRPT_ERROR2)) {
                    err = HAL_SE_DONE_ERR_ENG_ERR1_ERR2;
                } else if (aes_int & AES_INTRPT_ERROR1) {
                    err = HAL_SE_DONE_ERR_ENG_ERR1;
                } else {
                    err = HAL_SE_DONE_ERR_ENG_ERR2;
                }
                se_done_hdlr((void *)se_out_addr, 0, err);
            }
        }
    }

#ifdef SEC_ENG_HAS_HASH
    if (all_int & SE_ACC_INT_HASH) {
        hash_int = se_hash->HASH_STATUS;

        // Clear IRQs
        se_hash->HASH_STATUS = hash_int;
        se_adec->ADEC_INT = SE_ACC_INT_HASH;

        if (hash_int & HASH_STATUS_HASH_DONE) {
            enum SE_HASH_OP_T op;

            op = GET_BITFIELD(se_hash->HASH_CTRL, HASH_CTRL_HASH_OP_MODE);
            if (op == SE_HASH_OP_UPDATE &&
                    (se_seg_oper == HAL_SE_SEG_INIT || se_seg_oper == HAL_SE_SEG_CONT)) {
                uint32_t hash_cfg;
                enum HAL_SE_HASH_MODE_T mode;
                uint32_t ctx_len;
                int hmac;

                ASSERT(se_seg_ctx_buf, "%s: Bad HASH se_seg_ctx_buf. se_seg_oper=%d", __func__, se_seg_oper);

                hash_cfg = se_hash->HASH_CFG;
                mode = GET_BITFIELD(hash_cfg, HASH_CFG_ALG_SELECT);
                ctx_len = hal_se_hash_mode_ctx_len(mode);
                hmac = !!(hash_cfg & HASH_CFG_HASH_MODE);
                if (hmac) {
                    ctx_len *= 2;
                }
                se_hash_save_ctx(hmac, se_seg_ctx_buf, ctx_len);
            }
            if (se_done_hdlr) {
                if (op == SE_HASH_OP_UPDATE) {
                    se_done_hdlr(NULL, 0, HAL_SE_DONE_OK);
                } else if (op == SE_HASH_OP_FINAL) {
                    enum HAL_SE_RET_T ret;
                    uint32_t digest[16];
                    uint32_t len;

                    ret = hal_se_hash_get_digest(&digest[0], sizeof(digest), &len);
                    if (ret == HAL_SE_OK) {
                        se_done_hdlr(&digest[0], len, HAL_SE_DONE_OK);
                    } else {
                        se_done_hdlr(NULL, 0, HAL_SE_DONE_ERR_GET_DIGEST);
                    }
                }
            }
        }
    }
#endif

    TR_INFO(0, "%s: all_int=0x%08X aes_int=0x%08X hash_int=%d", __func__, all_int, aes_int, hash_int);
}

static void sedma_irq_handler(void)
{
    uint8_t hwch;
    uint32_t remains;
    uint32_t len;
    uint32_t ctrl;
    bool tcint, errint;

    for (hwch = 0; hwch < SE_DMA_CHAN_NUM; hwch++) {
        if ((se_dma->INTSTAT & DMA_STAT_CHAN(hwch)) == 0) {
            continue;
        }

        /* Check counter terminal status */
        tcint = !!(se_dma->INTTCSTAT & DMA_STAT_CHAN(hwch));
        /* Check error terminal status */
        errint = !!(se_dma->INTERRSTAT & DMA_STAT_CHAN(hwch));

        if (tcint || errint) {
            if (tcint) {
                /* Clear terminate counter Interrupt pending */
                se_dma->INTTCCLR = DMA_STAT_CHAN(hwch);
            }
            if (errint) {
                /* Clear error counter Interrupt pending */
                se_dma->INTERRCLR = DMA_STAT_CHAN(hwch);
            }

            remains = GET_BITFIELD(se_dma->CH[hwch].CONTROL, DMA_CONTROL_TRANSFERSIZE);
            if (errint) {
                se_dma->CH[hwch].CONFIG &= ~DMA_CONFIG_EN;
                if (se_done_hdlr) {
                    se_done_hdlr((uint8_t *)se_out_addr, 0,
                        (hwch == SE_DMA_RX_CHAN) ? HAL_SE_DONE_ERR_DMA_OUT : HAL_SE_DONE_ERR_DMA_IN);
                }
            } else {
                if (hwch == SE_DMA_RX_CHAN) {
                    bool cont;
#ifdef SEC_ENG_UNALIGNED_OUTPUT
                    cont = (se_cur_out_len < (se_total_out_len & ~(4 - 1)));
#else
                    cont = (se_cur_out_len < se_total_out_len);
#endif
                    if (cont) {
                        len = se_total_out_len - se_cur_out_len;
                        ctrl = se_dma->CH[hwch].CONTROL & ~DMA_CONTROL_TC_IRQ;
                        se_cur_out_len += se_dma_init_rx_desc(se_out_addr + se_cur_out_len, len, se_dma->CH[hwch].SRCADDR, ctrl);
                        se_dma->CH[SE_DMA_RX_CHAN].CONFIG |= DMA_CONFIG_EN;
                    } else {
#ifdef SEC_ENG_UNALIGNED_OUTPUT
                        if (remains == 0 && (se_total_out_len & (4 - 1))) {
                            while (se_crypt->AES_STATUS & AES_STATUS_BUSY) {};
                            uint32_t last_word = se_dmacfg->DMA_RXFIFO;
                            while (se_cur_out_len < se_total_out_len) {
                                *(volatile uint8_t *)(se_out_addr + se_cur_out_len) = last_word & 0xFF;
                                last_word >>= 8;
                                se_cur_out_len++;
                            }
                        }
#endif
                        if (se_done_hdlr) {
                            if (remains) {
                                se_done_hdlr((void *)se_out_addr, se_cur_out_len - remains, HAL_SE_DONE_ERR_DMA_IN_REMAIN);
                            } else {
                                se_done_hdlr((void *)se_out_addr, se_cur_out_len, HAL_SE_DONE_OK);
                            }
                        }
                    }
                } else {
                    if (se_cur_in_len < se_total_in_len) {
                        len = se_total_in_len - se_cur_in_len;
                        ctrl = se_dma->CH[hwch].CONTROL & ~DMA_CONTROL_TC_IRQ;
                        se_cur_in_len += se_dma_init_tx_desc(se_in_addr + se_cur_in_len, len, se_dma->CH[hwch].DSTADDR, ctrl);
                        se_dma->CH[SE_DMA_TX_CHAN].CONFIG |= DMA_CONFIG_EN;
                    } else {
                        if (remains) {
                            if (se_done_hdlr) {
                                se_done_hdlr((void *)se_out_addr, 0, HAL_SE_DONE_ERR_DMA_IN_REMAIN);
                            }
                        }
                    }
                }
            }
            TR_INFO(0, "%s: ch=%d tcint=%d errint=%d remains=%u", __func__, hwch, tcint, errint, remains);
        }
    }
}
#endif

static void se_dma_cancel_rx(void)
{
    se_dma->CH[SE_DMA_RX_CHAN].CONFIG &= ~DMA_CONFIG_EN;
}

static void se_dma_cancel_tx(void)
{
    se_dma->CH[SE_DMA_TX_CHAN].CONFIG &= ~DMA_CONFIG_EN;
}

static void se_dma_open_no_irq(void)
{
    uint8_t i;

    se_adec->ADEC_CTRL |= ADEC_CTRL_CLK_EN_15_0(SE_ACC_CLK_DMA) | ADEC_CTRL_RST_15_0(SE_ACC_RST_DMA);
    se_adec->ADEC_CTRL &= ~ADEC_CTRL_RST_15_0(SE_ACC_RST_DMA);
    se_adec->ADEC_INT_MSK = ~0UL;

    se_dmacfg->DMA_CTRL = DMA_CTRL_RX_DMA_EN | DMA_CTRL_TX_DMA_EN;
    se_dmacfg->DMA_RDL = SE_DMA_RX_FIFO_TRIG_LEVEL;
    se_dmacfg->DMA_TDL = SE_DMA_TX_FIFO_TRIG_LEVEL;

    /* Reset all channel configuration register */
    for (i = 0; i < SE_DMA_CHAN_NUM; i++) {
        se_dma->CH[i].CONFIG = 0;
    }

    /* Clear all DMA interrupt and error flag */
    se_dma->INTTCCLR = ~0UL;
    se_dma->INTERRCLR = ~0UL;

    se_dma->DMACONFIG = (se_dma->DMACONFIG & ~(DMA_DMACONFIG_AHB1_BIGENDIAN |
        DMA_DMACONFIG_AHB2_BIGENDIAN)) | DMA_DMACONFIG_EN | DMA_DMACONFIG_CROSS_1KB_EN;
}

static void se_dma_close_no_irq(void)
{
    se_dma_cancel_rx();
    se_dma_cancel_tx();
    se_dma->DMACONFIG = 0;
}

#ifndef SEC_ENG_ROM_ONLY
static void se_dma_open(void)
{
    se_dma_open_no_irq();

    NVIC_SetVector(SEDMA_IRQn, (uint32_t)sedma_irq_handler);
    NVIC_SetPriority(SEDMA_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(SEDMA_IRQn);
    NVIC_EnableIRQ(SEDMA_IRQn);
}

static void se_dma_close(void)
{
    NVIC_DisableIRQ(SEDMA_IRQn);

    se_dma_close_no_irq();
}

static uint32_t se_dma_init_rx_desc(uint32_t out, uint32_t out_len, uint32_t src, uint32_t ctrl)
{
    uint32_t len;
    uint32_t cfg_len;
    uint32_t desc_idx;
    uint32_t i;

    out_len &= ~(4 - 1);
    len = out_len;
    if (len > SE_DMA_MAX_DESC_XFER_SIZE * 4) {
        len = SE_DMA_MAX_DESC_XFER_SIZE * 4;
    }
    len &= ~(4 - 1);
    ctrl = SET_BITFIELD(ctrl, DMA_CONTROL_TRANSFERSIZE, len / 4);
    if (len >= out_len) {
        // Always enable IRQ at the end of each xfer
        ctrl |= DMA_CONTROL_TC_IRQ;
    }
    se_dma->CH[SE_DMA_RX_CHAN].CONTROL = ctrl;
    se_dma->CH[SE_DMA_RX_CHAN].SRCADDR = src;
    se_dma->CH[SE_DMA_RX_CHAN].DSTADDR = out;

    if (len >= out_len) {
        se_dma->CH[SE_DMA_RX_CHAN].LLI = 0;
        return len;
    }

    cfg_len = len;
    desc_idx = 0;
    while (desc_idx < SE_DMA_DESC_NUM && cfg_len < out_len) {
        len = out_len - cfg_len;
        if (len > SE_DMA_MAX_DESC_XFER_SIZE * 4) {
            len = SE_DMA_MAX_DESC_XFER_SIZE * 4;
        }
        len &= ~(4 - 1);
        ctrl = SET_BITFIELD(ctrl, DMA_CONTROL_TRANSFERSIZE, len / 4);
        dma_desc[SE_DMA_RX_CHAN][desc_idx].ctrl = ctrl;
        dma_desc[SE_DMA_RX_CHAN][desc_idx].src = src;
        dma_desc[SE_DMA_RX_CHAN][desc_idx].dst = out + cfg_len;
        cfg_len += len;
        desc_idx++;
    }

    // Always enable IRQ at the end of each xfer
    dma_desc[SE_DMA_RX_CHAN][desc_idx - 1].ctrl |= DMA_CONTROL_TC_IRQ;

    for (i = 0; i + 1 < desc_idx; i++) {
        dma_desc[SE_DMA_RX_CHAN][i].lli = (uint32_t)&dma_desc[SE_DMA_RX_CHAN][i + 1];
    }
    dma_desc[SE_DMA_RX_CHAN][desc_idx - 1].lli = 0;

    se_dma->CH[SE_DMA_RX_CHAN].LLI = (uint32_t)&dma_desc[SE_DMA_RX_CHAN][0];

    return cfg_len;
}

static void se_dma_enable_rx(void *out, uint32_t out_len)
{
    uint32_t ctrl;

    ASSERT(out_len >= 4 &&
#ifdef SEC_ENG_UNALIGNED_OUTPUT
        true
#else
        (out_len & (4 - 1)) == 0
#endif
        , "%s: Bad out_len=%u", __func__, out_len);

    se_out_addr = (uint32_t)out;
    se_total_out_len = out_len;

    /* Reset the Interrupt status */
    se_dma->INTTCCLR = DMA_STAT_CHAN(SE_DMA_RX_CHAN);
    se_dma->INTERRCLR = DMA_STAT_CHAN(SE_DMA_RX_CHAN);

    ctrl = DMA_CONTROL_SBSIZE(HAL_DMA_BSIZE_8) |
           DMA_CONTROL_DBSIZE(HAL_DMA_BSIZE_16) |
           DMA_CONTROL_SWIDTH(HAL_DMA_WIDTH_WORD) |
           DMA_CONTROL_DWIDTH(HAL_DMA_WIDTH_BYTE) |
           DMA_CONTROL_DI;

    se_cur_out_len = se_dma_init_rx_desc((uint32_t)out, out_len, (uint32_t)&se_dmacfg->DMA_RXFIFO, ctrl);

    se_dma->CH[SE_DMA_RX_CHAN].CONFIG  = DMA_CONFIG_SRCPERIPH(SE_DMA_PERIPH_RX) |
                                         DMA_CONFIG_TRANSFERTYPE(HAL_DMA_FLOW_P2M_DMA) |
                                         DMA_CONFIG_ERR_IRQMASK | DMA_CONFIG_TC_IRQMASK;
    se_dma->CH[SE_DMA_RX_CHAN].CONFIG |= DMA_CONFIG_EN;
}

static uint32_t se_dma_init_tx_desc(uint32_t in, uint32_t in_len, uint32_t dst, uint32_t ctrl)
{
    uint32_t len;
    uint32_t cfg_len;
    uint32_t desc_idx;
    uint32_t i;

    len = in_len;
    if (len > SE_DMA_MAX_DESC_XFER_SIZE) {
        len = SE_DMA_MAX_DESC_XFER_SIZE;
    }
    ctrl = SET_BITFIELD(ctrl, DMA_CONTROL_TRANSFERSIZE, len);
    se_dma->CH[SE_DMA_TX_CHAN].CONTROL = ctrl;
    se_dma->CH[SE_DMA_TX_CHAN].SRCADDR = in;
    se_dma->CH[SE_DMA_TX_CHAN].DSTADDR = dst;

    if (len >= in_len) {
        se_dma->CH[SE_DMA_TX_CHAN].LLI = 0;
        return len;
    }

    cfg_len = len;
    desc_idx = 0;
    while (desc_idx < SE_DMA_DESC_NUM && cfg_len < in_len) {
        len = in_len - cfg_len;
        if (len > SE_DMA_MAX_DESC_XFER_SIZE) {
            len = SE_DMA_MAX_DESC_XFER_SIZE;
        }
        ctrl = SET_BITFIELD(ctrl, DMA_CONTROL_TRANSFERSIZE, len);
        dma_desc[SE_DMA_TX_CHAN][desc_idx].ctrl = ctrl;
        dma_desc[SE_DMA_TX_CHAN][desc_idx].src = in + cfg_len;
        dma_desc[SE_DMA_TX_CHAN][desc_idx].dst = dst;
        cfg_len += len;
        desc_idx++;
    }

    if (cfg_len < in_len) {
        // Enable IRQ at the end of xfer
        dma_desc[SE_DMA_TX_CHAN][desc_idx - 1].ctrl |= DMA_CONTROL_TC_IRQ;
    }

    for (i = 0; i + 1 < desc_idx; i++) {
        dma_desc[SE_DMA_TX_CHAN][i].lli = (uint32_t)&dma_desc[SE_DMA_TX_CHAN][i + 1];
    }
    dma_desc[SE_DMA_TX_CHAN][desc_idx - 1].lli = 0;

    se_dma->CH[SE_DMA_TX_CHAN].LLI = (uint32_t)&dma_desc[SE_DMA_TX_CHAN][0];

    return cfg_len;
}

static void se_dma_enable_tx(const void *in, uint32_t in_len)
{
    uint32_t ctrl;

    if (in_len & (4 - 1)) {
        in_len = (in_len + (4 - 1)) & ~(4 - 1);
    }

    se_in_addr = (uint32_t)in;
    se_total_in_len = in_len;

    /* Reset the Interrupt status */
    se_dma->INTTCCLR = DMA_STAT_CHAN(SE_DMA_TX_CHAN);
    se_dma->INTERRCLR = DMA_STAT_CHAN(SE_DMA_TX_CHAN);

    ctrl = DMA_CONTROL_SBSIZE(HAL_DMA_BSIZE_16) |
           DMA_CONTROL_DBSIZE(HAL_DMA_BSIZE_8) |
           DMA_CONTROL_SWIDTH(HAL_DMA_WIDTH_BYTE) |
           DMA_CONTROL_DWIDTH(HAL_DMA_WIDTH_WORD) |
           DMA_CONTROL_SI;

    se_cur_in_len = se_dma_init_tx_desc((uint32_t)in, in_len, (uint32_t)&se_dmacfg->DMA_TXFIFO, ctrl);

    se_dma->CH[SE_DMA_TX_CHAN].CONFIG  = DMA_CONFIG_DSTPERIPH(SE_DMA_PERIPH_TX) |
                                         DMA_CONFIG_TRANSFERTYPE(HAL_DMA_FLOW_M2P_DMA) |
                                         DMA_CONFIG_ERR_IRQMASK | DMA_CONFIG_TC_IRQMASK;
    se_dma->CH[SE_DMA_TX_CHAN].CONFIG |= DMA_CONFIG_EN;
}
#endif

POSSIBLY_UNUSED static uint32_t se_dma_enable_tx_no_irq(const void *in, uint32_t in_len)
{
    uint32_t ctrl;
    uint32_t len;

    len = in_len;
    if (len & (4 - 1)) {
        len = (len + (4 - 1)) & ~(4 - 1);
    }
    if (len > SE_DMA_MAX_DESC_XFER_SIZE) {
        len = SE_DMA_MAX_DESC_XFER_SIZE;
    }

    ctrl = DMA_CONTROL_TRANSFERSIZE(len) |
           DMA_CONTROL_SBSIZE(HAL_DMA_BSIZE_16) |
           DMA_CONTROL_DBSIZE(HAL_DMA_BSIZE_8) |
           DMA_CONTROL_SWIDTH(HAL_DMA_WIDTH_BYTE) |
           DMA_CONTROL_DWIDTH(HAL_DMA_WIDTH_WORD) |
           DMA_CONTROL_SI;

    se_dma->CH[SE_DMA_TX_CHAN].CONTROL = ctrl;
    se_dma->CH[SE_DMA_TX_CHAN].SRCADDR = (uint32_t)in;
    se_dma->CH[SE_DMA_TX_CHAN].DSTADDR = (uint32_t)&se_dmacfg->DMA_TXFIFO;

    se_dma->CH[SE_DMA_TX_CHAN].CONFIG  = DMA_CONFIG_DSTPERIPH(SE_DMA_PERIPH_TX) |
                                         DMA_CONFIG_TRANSFERTYPE(HAL_DMA_FLOW_M2P_DMA);
    se_dma->CH[SE_DMA_TX_CHAN].CONFIG |= DMA_CONFIG_EN;

    return len;
}

static void se_mod_open(void)
{
    hal_cmu_sec_eng_clock_enable();
}

static void se_mod_close(void)
{
    se_adec->ADEC_INT_MSK = ~0UL;
    se_adec->ADEC_CTRL = ~0UL;
    se_adec->ADEC_CTRL2 = ~0UL;
    se_adec->ADEC_CTRL = 0x0000FFFF;
    se_adec->ADEC_CTRL2 = 0x0000FFFF;

    hal_cmu_sec_eng_clock_disable();
}

#ifdef SEC_ENG_ROM_ONLY
enum HAL_SE_RET_T hal_se_rom_open(void)
{
    se_mod_open();
    se_dma_open_no_irq();

    return HAL_SE_OK;
}

enum HAL_SE_RET_T hal_se_rom_close(void)
{
    if (hal_cmu_clock_get_status(HAL_CMU_MOD_H_SEC_ENG) != HAL_CMU_CLK_ENABLED) {
        return HAL_SE_NOT_OPENED;
    }

    se_dma_close_no_irq();
    se_mod_close();

    return HAL_SE_OK;
}
#else
enum HAL_SE_RET_T hal_se_open(void)
{
    if (se_enabled) {
        return HAL_SE_ALREADY_OPENED;
    }

    se_enabled = true;

    se_mod_open();
    se_dma_open();

    NVIC_SetVector(SEC_ENG_IRQn, (uint32_t)sec_eng_irq_handler);
    NVIC_SetPriority(SEC_ENG_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(SEC_ENG_IRQn);
    NVIC_EnableIRQ(SEC_ENG_IRQn);

    return HAL_SE_OK;
}

enum HAL_SE_RET_T hal_se_close(void)
{
    if (!se_enabled) {
        return HAL_SE_NOT_OPENED;
    }

    NVIC_DisableIRQ(SEC_ENG_IRQn);

    se_dma_close();
    se_mod_close();

    se_enabled = false;

    return HAL_SE_OK;
}
#endif

POSSIBLY_UNUSED static void write_mreg(volatile uint32_t *reg, const void *mem, uint32_t len)
{
    const uint32_t *p32 = (const uint32_t *)mem;
    uint32_t i;
    bool aligned_state;

    aligned_state = config_unaligned_access(true);

    i = 0;
    while (i < len) {
        *reg++ = *p32++;
        i += 4;
    }

    config_unaligned_access(aligned_state);
}

POSSIBLY_UNUSED static void read_mreg(const volatile uint32_t *reg, void *mem, uint32_t len)
{
    uint32_t *p32 = (uint32_t *)mem;
    uint32_t i;
    bool aligned_state;

    aligned_state = config_unaligned_access(true);

    i = 0;
    while (i < len) {
        *p32++ = *reg++;
        i += 4;
    }

    config_unaligned_access(aligned_state);
}

#ifndef SEC_ENG_ROM_ONLY
enum HAL_SE_RET_T se_aes_crypt(const struct HAL_SE_AES_CFG_T *cfg, bool decrypt)
{
    enum SE_AES_KEY_LEN_T key_len_type;
    uint8_t modular;

    if (!se_enabled) {
        return HAL_SE_NOT_OPENED;
    }

    if (cfg == NULL) {
        return HAL_SE_CFG_NULL;
    }
    if (cfg->in == NULL) {
        return HAL_SE_INPUT_NULL;
    }
    if (cfg->out == NULL) {
        return HAL_SE_OUTPUT_NULL;
    }
    if (cfg->mode == HAL_SE_AES_ECB || cfg->seg_oper == HAL_SE_SEG_NONE || cfg->seg_oper == HAL_SE_SEG_INIT) {
        if (cfg->key == NULL) {
            return HAL_SE_KEY_NULL;
        }
    }
    if (cfg->mode >= HAL_SE_AES_MODE_QTY) {
        return HAL_SE_BAD_AES_MODE;
    }
    if (cfg->cbc_cs >= HAL_SE_CBC_CS_QTY) {
        return HAL_SE_BAD_CBC_CS;
    }
    if (cfg->seg_oper >= HAL_SE_SEG_QTY) {
        return HAL_SE_BAD_SEG_OPER;
    }
    if (cfg->seg_oper != HAL_SE_SEG_NONE && cfg->mode != HAL_SE_AES_ECB) {
        if (cfg->seg_ctx_buf == NULL) {
            return HAL_SE_BAD_SEG_CTX_BUF;
        }
        if (cfg->seg_ctx_len < 16 + cfg->key_len) {
            return HAL_SE_BAD_SEG_CTX_LEN;
        }
    }
    if (cfg->seg_oper == HAL_SE_SEG_NONE || cfg->seg_oper == HAL_SE_SEG_INIT) {
        if (cfg->mode == HAL_SE_AES_XTS && cfg->key2 == NULL) {
            return HAL_SE_KEY_NULL;
        }
        if (cfg->mode != HAL_SE_AES_ECB && cfg->iv == NULL) {
            return HAL_SE_IV_NULL;
        }
    }
    if (cfg->in_len < 16) {
        return HAL_SE_BAD_INPUT_LEN;
    }
    if (cfg->mode == HAL_SE_AES_ECB ||
            (cfg->mode == HAL_SE_AES_CBC && cfg->cbc_cs == HAL_SE_CBC_CS_NONE)) {
        if (cfg->in_len & (16 - 1)) {
            return HAL_SE_BAD_INPUT_LEN;
        }
    }
    if (cfg->mode == HAL_SE_AES_KEY_WRAP) {
        if (cfg->in_len & (8 - 1)) {
            return HAL_SE_BAD_INPUT_LEN;
        }
        if (decrypt) {
            if (cfg->out_len + 8 != cfg->in_len) {
                return HAL_SE_BAD_OUTPUT_LEN;
            }
        } else {
            if (cfg->out_len != cfg->in_len + 8) {
                return HAL_SE_BAD_OUTPUT_LEN;
            }
        }
    } else {
        if (cfg->out_len != cfg->in_len &&
#ifdef SEC_ENG_UNALIGNED_OUTPUT
                true
#else
                cfg->out_len != ((cfg->in_len + (4 - 1)) & ~(4 - 1))
#endif
                ) {
            return HAL_SE_BAD_OUTPUT_LEN;
        }
    }
    modular = 0;
    if (cfg->mode == HAL_SE_AES_CTR) {
        if (cfg->ctr_modular > 128) {
            return HAL_SE_BAD_AES_MODULAR;
        } else if (cfg->ctr_modular < 128) {
            modular = cfg->ctr_modular;
        }
    }

    if (cfg->key_len == 16) {
        key_len_type = SE_AES_KEY_128;
    } else if (cfg->key_len == 24) {
        key_len_type = SE_AES_KEY_192;
    } else if (cfg->key_len == 32) {
        key_len_type = SE_AES_KEY_256;
    } else {
        return HAL_SE_BAD_KEY_LEN;
    }

#ifdef SEC_ENG_UNALIGNED_OUTPUT
    if ((cfg->out_len & (4 - 1)) && cfg->done_hdlr == NULL) {
        return HAL_SE_BAD_DONE_HDLR;
    }
#endif

    if (se_crypt->AES_STATUS & AES_STATUS_BUSY) {
        return HAL_SE_ENG_BUSY;
    }
    if (se_dma->ENBLDCHNS & (DMA_STAT_CHAN(SE_DMA_RX_CHAN) | DMA_STAT_CHAN(SE_DMA_TX_CHAN))) {
        return HAL_SE_DMA_BUSY;
    }

    se_done_hdlr = cfg->done_hdlr;
    se_seg_oper = cfg->seg_oper;
    se_seg_ctx_buf = cfg->seg_ctx_buf;

    se_adec->ADEC_CTRL |= ADEC_CTRL_CLK_EN_15_0(SE_ACC_CLK_CRYPT) | ADEC_CTRL_RST_15_0(SE_ACC_RST_CRYPT);
    se_adec->ADEC_CTRL &= ~ADEC_CTRL_RST_15_0(SE_ACC_RST_CRYPT);
    se_adec->ADEC_INT = SE_ACC_INT_AES;
    se_adec->ADEC_INT_MSK &= ~SE_ACC_INT_AES;

    se_dmacfg->DMA_FIFOCLR = DMA_FIFOCLR_RXFIFO_CLR | DMA_FIFOCLR_TXFIFO_CLR;

    se_acb->ACB_CTRL &= ~(ACB_CTRL_WR_CB_CTRL | ACB_CTRL_RD_CB_CTRL);

    se_crypt->ENGINE_SELECT = ENGINE_SELECT_SELECT(SE_CRYPT_AES);
    // TODO: Set cts_mode, rkey
    se_crypt->AES_CFG = ((cfg->cbc_cs == HAL_SE_CBC_CS2) ? AES_CFG_CTS_MODE : 0) |
        AES_CFG_MODULAR(modular) | AES_CFG_MODE(cfg->mode) |
        AES_CFG_KEYLEN(key_len_type) | (decrypt ? AES_CFG_DECRYPT : 0);
    se_crypt->AES_INTRPT = ~0UL;
    se_crypt->AES_INTRPT_SRC_EN = AES_INTRPT_SRC_EN_ERROR1 | AES_INTRPT_SRC_EN_ERROR2;
    if (se_seg_oper == HAL_SE_SEG_INIT || se_seg_oper == HAL_SE_SEG_CONT) {
        se_crypt->AES_INTRPT_SRC_EN |= AES_INTRPT_SRC_EN_DONE;
    }
    // TODO: Set outputblock
    if (se_seg_oper == HAL_SE_SEG_CONT || se_seg_oper == HAL_SE_SEG_TERM) {
        se_crypt->AES_CTRL |= AES_CTRL_RESUME;
    } else {
        se_crypt->AES_CTRL &= ~AES_CTRL_RESUME;
    }
    se_crypt->AES_CTRL |= AES_CTRL_RESET;
    se_crypt->AES_CTRL &= ~AES_CTRL_RESET;

    if (cfg->mode == HAL_SE_AES_ECB) {
        write_mreg(&se_crypt->KEY1[0], cfg->key, cfg->key_len);
    } else {
        if (cfg->seg_oper == HAL_SE_SEG_NONE || cfg->seg_oper == HAL_SE_SEG_INIT) {
            write_mreg(&se_crypt->KEY1[0], cfg->key, cfg->key_len);
            write_mreg(&se_crypt->IV[0], cfg->iv, 16);
            if (cfg->mode == HAL_SE_AES_XTS) {
                write_mreg(&se_crypt->KEY2[0], cfg->key2, cfg->key_len);
            }
        } else {
            write_mreg(&se_crypt->IV[0], cfg->seg_ctx_buf, 16);
            write_mreg(&se_crypt->KEY1[0], cfg->seg_ctx_buf + 16, cfg->key_len);
        }
    }

    se_dma_enable_tx(cfg->in, cfg->in_len);
    se_dma_enable_rx(cfg->out, cfg->out_len);

    se_crypt->AES_STREAM_SIZE = cfg->in_len;
    se_crypt->AES_CMD = AES_CMD_START;

    return HAL_SE_OK;
}

enum HAL_SE_RET_T hal_se_aes_encrypt(const struct HAL_SE_AES_CFG_T *cfg)
{
    return se_aes_crypt(cfg, false);
}

enum HAL_SE_RET_T hal_se_aes_decrypt(const struct HAL_SE_AES_CFG_T *cfg)
{
    return se_aes_crypt(cfg, true);
}

static uint32_t se_aes_get_key_len(void)
{
    enum SE_AES_KEY_LEN_T key_len_type;
    uint32_t key_len;

    key_len_type = GET_BITFIELD(se_crypt->AES_CFG, AES_CFG_KEYLEN);
    if (key_len_type == SE_AES_KEY_128) {
        key_len = 16;
    } else if (key_len_type == SE_AES_KEY_192) {
        key_len = 24;
    } else {
        key_len = 32;
    }

    return key_len;
}

enum HAL_SE_RET_T hal_se_aes_save_seg_ctx(void *seg_ctx_buf, uint32_t seg_ctx_len)
{
    uint32_t key_len;

    if (!se_enabled) {
        return HAL_SE_NOT_OPENED;
    }
    if (hal_se_aes_busy()) {
        return HAL_SE_ENG_BUSY;
    }
    if (se_seg_oper != HAL_SE_SEG_INIT && se_seg_oper != HAL_SE_SEG_CONT) {
        return HAL_SE_BAD_SEG_OPER;
    }
    if (seg_ctx_buf == NULL) {
        return HAL_SE_BAD_SEG_CTX_BUF;
    }
    key_len = se_aes_get_key_len();
    if (seg_ctx_len < 16 + key_len) {
        return HAL_SE_BAD_SEG_CTX_LEN;
    }

    read_mreg(&se_crypt->IV[0], seg_ctx_buf, 16);
    read_mreg(&se_crypt->KEY1[0], seg_ctx_buf + 16, key_len);

    return HAL_SE_OK;
}

int hal_se_aes_busy(void)
{
    if (se_enabled) {
        if (se_crypt->AES_STATUS & AES_STATUS_BUSY) {
            return true;
        }
        if (se_dma->ENBLDCHNS & DMA_STAT_CHAN(SE_DMA_RX_CHAN)) {
            return true;
        }
    }
    return false;
}

enum HAL_SE_RET_T hal_se_aes_reset(void)
{
    uint32_t lock;

    if (!se_enabled) {
        return HAL_SE_NOT_OPENED;
    }

    lock = int_lock();
    se_dma_cancel_rx();
    se_dma_cancel_tx();
    /* Clear all DMA interrupt and error flag */
    se_dma->INTTCCLR = ~0UL;
    se_dma->INTERRCLR = ~0UL;

    se_adec->ADEC_CTRL |= ADEC_CTRL_CLK_EN_15_0(SE_ACC_CLK_CRYPT) | ADEC_CTRL_RST_15_0(SE_ACC_RST_CRYPT);
    se_adec->ADEC_CTRL &= ~ADEC_CTRL_RST_15_0(SE_ACC_RST_CRYPT);
    se_crypt->AES_INTRPT = ~0UL;
    se_adec->ADEC_INT = SE_ACC_INT_AES;

    se_done_hdlr = NULL;
    int_unlock(lock);

    return HAL_SE_OK;
}
#endif

#ifdef SEC_ENG_HAS_HASH

static enum HAL_SE_RET_T se_hash_init(enum HAL_SE_HASH_MODE_T mode, SE_HASH_INIT_PRE_START_CB cb, const struct SE_HASH_INIT_CB_PARAM_T *param)
{
    uint32_t val;

    if (mode >= HAL_SE_HASH_QTY) {
        return HAL_SE_BAD_MODE;
    }

    if (se_hash->HASH_STATUS & HASH_STATUS_HASH_BUSY) {
        return HAL_SE_ENG_BUSY;
    }
    if (se_dma->ENBLDCHNS & DMA_STAT_CHAN(SE_DMA_TX_CHAN)) {
        return HAL_SE_DMA_BUSY;
    }

    se_adec->ADEC_CTRL |= ADEC_CTRL_CLK_EN_15_0(SE_ACC_CLK_HASH) | ADEC_CTRL_RST_15_0(SE_ACC_RST_HASH);
    se_adec->ADEC_CTRL &= ~ADEC_CTRL_RST_15_0(SE_ACC_RST_HASH);
    se_adec->ADEC_INT = SE_ACC_INT_HASH;
    se_adec->ADEC_INT_MSK |= SE_ACC_INT_HASH;

    se_dmacfg->DMA_FIFOCLR = DMA_FIFOCLR_TXFIFO_CLR;

    se_acb->ACB_CTRL |= ACB_CTRL_RD_CB_CTRL;

    se_hash->HASH_CFG = HASH_CFG_ALG_SELECT(mode);
    val = se_hash->HASH_CTRL;
    //val |= HASH_CTRL_RESET;
    se_hash->HASH_CTRL = val;
    val &= ~(HASH_CTRL_RESET | HASH_CTRL_HW_PADDING);
    se_hash->HASH_CTRL = val;
    val = SET_BITFIELD(val, HASH_CTRL_HASH_OP_MODE, SE_HASH_OP_INIT);
    se_hash->HASH_CTRL = val;
    if (cb) {
        cb(param);
    }
    se_hash->HASH_CMD = HASH_CMD_START;

    // HASH: 500ns; HMAC: 3000ns
    while ((se_hash->HASH_STATUS & HASH_STATUS_HASH_DONE) == 0);

    return HAL_SE_OK;
}

static enum HAL_SE_RET_T se_hash_prepare_start_cmd(const struct HAL_SE_HASH_CFG_T *cfg,
                                                    enum SE_HASH_OP_T op, uint64_t in_len)
{
    enum HAL_SE_HASH_MODE_T mode;
    enum SE_HASH_OP_T prev_op;
    uint32_t val;

    if (cfg == NULL) {
        return HAL_SE_CFG_NULL;
    }
    if (op == SE_HASH_OP_FINAL) {
        if (cfg->in == NULL && cfg->in_len) {
            return HAL_SE_INPUT_NULL;
        }
    } else {
        if (cfg->in == NULL) {
            return HAL_SE_INPUT_NULL;
        }
        if (cfg->in_len == 0) {
            return HAL_SE_BAD_INPUT_LEN;
        }
        mode = GET_BITFIELD(se_hash->HASH_CFG, HASH_CFG_ALG_SELECT);
        if (mode == HAL_SE_HASH_SHA384 || mode == HAL_SE_HASH_SHA512) {
            val = 128;
        } else {
            val = 64;
        }
        if (cfg->in_len & (val - 1)) {
            return HAL_SE_BAD_INPUT_LEN;
        }
    }

    prev_op = GET_BITFIELD(se_hash->HASH_CTRL, HASH_CTRL_HASH_OP_MODE);
    if (prev_op != SE_HASH_OP_INIT && prev_op != SE_HASH_OP_UPDATE) {
        return HAL_SE_BAD_OP;
    }
    if (se_hash->HASH_STATUS & HASH_STATUS_HASH_BUSY) {
        return HAL_SE_ENG_BUSY;
    }
    if (se_dma->ENBLDCHNS & DMA_STAT_CHAN(SE_DMA_TX_CHAN)) {
        return HAL_SE_DMA_BUSY;
    }

    se_hash->HASH_STATUS = ~0UL;
    se_adec->ADEC_INT = SE_ACC_INT_HASH;
    se_adec->ADEC_INT_MSK &= ~SE_ACC_INT_HASH;

    se_dmacfg->DMA_FIFOCLR = DMA_FIFOCLR_TXFIFO_CLR;

    return HAL_SE_OK;
}

static void se_hash_set_start_cmd(const struct HAL_SE_HASH_CFG_T *cfg, enum SE_HASH_OP_T op, uint64_t in_len)
{
    uint32_t val;

    val = se_hash->HASH_CTRL;
    val = SET_BITFIELD(val, HASH_CTRL_HASH_OP_MODE, op);
    if (op == SE_HASH_OP_UPDATE) {
        val &= ~HASH_CTRL_HW_PADDING;
    } else {
        val |= HASH_CTRL_HW_PADDING;
    }
    se_hash->HASH_CTRL = val;
    if (op == SE_HASH_OP_FINAL) {
        se_hash->HASH_MSG_SIZE_L = (uint32_t)in_len;
        se_hash->HASH_MSG_SIZE_H = (uint32_t)(in_len >> 32);
    }
    se_hash->HASH_SEG_SIZE = cfg->in_len;
    se_hash->HASH_CMD = HASH_CMD_START;
}

uint32_t hal_se_hash_mode_ctx_len(enum HAL_SE_HASH_MODE_T mode)
{
    uint32_t len;

    if (mode == HAL_SE_HASH_MD5 || mode == HAL_SE_HASH_SHA1 ||
            mode == HAL_SE_HASH_SHA224 || mode == HAL_SE_HASH_SHA256) {
        len = 32;
    } else {
        len = 64;
    }

    return len;
}

uint32_t hal_se_hash_mode_digest_len(enum HAL_SE_HASH_MODE_T mode)
{
    uint32_t len;

    if (mode == HAL_SE_HASH_MD5) {
        len = 16;
    } else if (mode == HAL_SE_HASH_SHA1) {
        len = 20;
    } else if (mode == HAL_SE_HASH_SHA224) {
        len = 28;
    } else if (mode == HAL_SE_HASH_SHA256) {
        len = 32;
    } else if (mode == HAL_SE_HASH_SHA384) {
        len = 48;
    } else {
        len = 64;
    }

    return len;
}

static uint32_t se_hash_get_digest_len(void)
{
    enum HAL_SE_HASH_MODE_T mode;

    mode = GET_BITFIELD(se_hash->HASH_CFG, HASH_CFG_ALG_SELECT);

    return hal_se_hash_mode_digest_len(mode);
}

static enum HAL_SE_RET_T se_hash_get_digest(void *out, uint32_t out_len, uint32_t *real_len)
{
    enum SE_HASH_OP_T prev_op;
    uint32_t max_len;
    int i;

    if (out == NULL) {
        return HAL_SE_OUTPUT_NULL;
    }
    if (out_len & (4 - 1)) {
        return HAL_SE_BAD_OUTPUT_LEN;
    }

    prev_op = GET_BITFIELD(se_hash->HASH_CTRL, HASH_CTRL_HASH_OP_MODE);
    if (prev_op != SE_HASH_OP_FINAL) {
        return HAL_SE_BAD_OP;
    }
    if (se_hash->HASH_STATUS & HASH_STATUS_HASH_BUSY) {
        return HAL_SE_ENG_BUSY;
    }
    if (se_dma->ENBLDCHNS & DMA_STAT_CHAN(SE_DMA_TX_CHAN)) {
        return HAL_SE_DMA_BUSY;
    }

    max_len = se_hash_get_digest_len();
    if (out_len > max_len) {
        out_len = max_len;
    }
    if (real_len) {
        *real_len = out_len;
    }
    if (out_len > 32) {
        for (i = 0; i < out_len / 8; i++) {
            read_mreg(&se_hash->HASH_CTX_H[i], out, 4);
            read_mreg(&se_hash->HASH_CTX[i], out + 4, 4);
            out += 8;
        }
    } else {
        read_mreg(&se_hash->HASH_CTX[0], out, out_len);
    }

    return HAL_SE_OK;
}

#ifdef SEC_ENG_ROM_ONLY
int hal_se_hash_rom_done(void)
{
    if (se_hash->HASH_STATUS & HASH_STATUS_HASH_DONE) {
        return true;
    }
    return false;
}

int hal_se_hash_rom_busy(void)
{
    if (se_hash->HASH_STATUS & HASH_STATUS_HASH_BUSY) {
        return true;
    }
    return false;
}

int hal_se_hash_dma_rom_busy(void)
{
    if (se_dma->ENBLDCHNS & DMA_STAT_CHAN(SE_DMA_TX_CHAN)) {
        return true;
    }
    return false;
}

enum HAL_SE_RET_T hal_se_hash_rom_init(enum HAL_SE_HASH_MODE_T mode)
{
    if (hal_cmu_clock_get_status(HAL_CMU_MOD_H_SEC_ENG) != HAL_CMU_CLK_ENABLED) {
        return HAL_SE_NOT_OPENED;
    }

    return se_hash_init(mode, NULL, NULL);
}

static enum HAL_SE_RET_T se_hash_rom_start(const struct HAL_SE_HASH_CFG_T *cfg, enum SE_HASH_OP_T op, uint64_t in_len)
{
    enum HAL_SE_RET_T ret;
    uint32_t tx_len;
    uint32_t start;

    if (hal_cmu_clock_get_status(HAL_CMU_MOD_H_SEC_ENG) != HAL_CMU_CLK_ENABLED) {
        return HAL_SE_NOT_OPENED;
    }

    ret = se_hash_prepare_start_cmd(cfg, op, in_len);
    if (ret != HAL_SE_OK) {
        return ret;
    }

    tx_len = 0;
    if (cfg->in_len) {
        tx_len += se_dma_enable_tx_no_irq(cfg->in, cfg->in_len);
    }

    se_hash_set_start_cmd(cfg, op, in_len);

    while (tx_len < cfg->in_len) {
        start = hal_sys_timer_get();
        while (hal_se_hash_dma_rom_busy() && (hal_sys_timer_get() - start < SE_OPER_TIMEOUT));
        if (hal_se_hash_dma_rom_busy()) {
            return HAL_SE_ERR;
        }
        tx_len += se_dma_enable_tx_no_irq(cfg->in + tx_len, cfg->in_len - tx_len);
    }

    start = hal_sys_timer_get();
    while ((hal_se_hash_rom_done() == 0) && (hal_sys_timer_get() - start < SE_OPER_TIMEOUT));
    if (hal_se_hash_rom_done() == 0) {
        return HAL_SE_ERR;
    }

    return HAL_SE_OK;
}

enum HAL_SE_RET_T hal_se_hash_rom_update(const struct HAL_SE_HASH_CFG_T *cfg)
{
    return se_hash_rom_start(cfg, SE_HASH_OP_UPDATE, 0);
}

enum HAL_SE_RET_T hal_se_hash_rom_final(const struct HAL_SE_HASH_CFG_T *cfg, uint64_t total_in_len)
{
    return se_hash_rom_start(cfg, SE_HASH_OP_FINAL, total_in_len);
}

enum HAL_SE_RET_T hal_se_hash_rom_get_digest(void *out, uint32_t out_len, uint32_t *real_len)
{
    if (hal_cmu_clock_get_status(HAL_CMU_MOD_H_SEC_ENG) != HAL_CMU_CLK_ENABLED) {
        return HAL_SE_NOT_OPENED;
    }

    return se_hash_get_digest(out, out_len, real_len);
}

enum HAL_SE_RET_T hal_se_hash_rom(enum HAL_SE_HASH_MODE_T mode, const struct HAL_SE_HASH_CFG_T *cfg)
{
    enum HAL_SE_RET_T ret;

    ret = hal_se_hash_rom_init(mode);
    if (ret != HAL_SE_OK) {
        return ret;
    }
    ret = hal_se_hash_rom_final(cfg, cfg->in_len);
    return ret;
}

#else // !SEC_ENG_ROM_ONLY
static void se_hash_save_ctx(int hmac, void *seg_ctx_buf, uint32_t seg_ctx_len)
{
    void *buf = seg_ctx_buf;
    uint32_t len = seg_ctx_len;
    int i;

    if (hmac) {
        len /= 2;
    }

    if (len > 32) {
        for (i = 0; i < len / 8; i++) {
            read_mreg(&se_hash->HASH_CTX_H[i], buf, 4);
            read_mreg(&se_hash->HASH_CTX[i], buf + 4, 4);
            buf += 8;
        }
        if (hmac) {
            for (i = 0; i < len / 8; i++) {
                read_mreg(&se_hash->HMAC_CTX_H[i], buf, 4);
                read_mreg(&se_hash->HMAC_CTX[i], buf + 4, 4);
                buf += 8;
            }
        }
    } else {
        read_mreg(&se_hash->HASH_CTX[0], buf, len);
        if (hmac) {
            buf += len;
            read_mreg(&se_hash->HMAC_CTX[0], buf, len);
        }
    }
}

static void se_hash_restore_ctx(int hmac, const void *seg_ctx_buf, uint32_t seg_ctx_len)
{
    const void *buf = seg_ctx_buf;
    uint32_t len = seg_ctx_len;
    int i;

    if (hmac) {
        len /= 2;
    }

    if (len > 32) {
        for (i = 0; i < len / 8; i++) {
            write_mreg(&se_hash->HASH_CTX_H[i], buf, 4);
            write_mreg(&se_hash->HASH_CTX[i], buf + 4, 4);
            buf += 8;
        }
        if (hmac) {
            for (i = 0; i < len / 8; i++) {
                write_mreg(&se_hash->HMAC_CTX_H[i], buf, 4);
                write_mreg(&se_hash->HMAC_CTX[i], buf + 4, 4);
                buf += 8;
            }
        }
    } else {
        write_mreg(&se_hash->HASH_CTX[0], buf, len);
        if (hmac) {
            buf += len;
            write_mreg(&se_hash->HMAC_CTX[0], buf, len);
        }
    }
}

enum HAL_SE_RET_T hal_se_hash_init(enum HAL_SE_HASH_MODE_T mode)
{
    if (!se_enabled) {
        return HAL_SE_NOT_OPENED;
    }

    se_seg_ctx_buf = NULL;
    se_seg_oper = HAL_SE_SEG_NONE;

    return se_hash_init(mode, NULL, NULL);
}

static enum HAL_SE_RET_T se_hash_start(const struct HAL_SE_HASH_CFG_T *cfg, enum SE_HASH_OP_T op, uint64_t in_len, int hmac)
{
    enum HAL_SE_RET_T ret;
    bool save_ctx = false;
    bool restore_ctx = false;

    if (!se_enabled) {
        return HAL_SE_NOT_OPENED;
    }

    ret = se_hash_prepare_start_cmd(cfg, op, in_len);
    if (ret != HAL_SE_OK) {
        return ret;
    }

    if (op == SE_HASH_OP_UPDATE) {
        if (cfg->seg_oper == HAL_SE_SEG_INIT) {
            save_ctx = true;
        } else if (cfg->seg_oper == HAL_SE_SEG_CONT) {
            restore_ctx = true;
            save_ctx = true;
        } else if (cfg->seg_oper != HAL_SE_SEG_NONE && cfg->seg_oper != HAL_SE_SEG_INIT) {
            return HAL_SE_BAD_SEG_OPER;
        }
    } else {
        if (cfg->seg_oper == HAL_SE_SEG_TERM) {
            restore_ctx = true;
        } else if (cfg->seg_oper != HAL_SE_SEG_NONE) {
            return HAL_SE_BAD_SEG_OPER;
        }
    }
    if (restore_ctx) {
        enum HAL_SE_HASH_MODE_T mode;
        uint32_t ctx_len;

        if (cfg->seg_ctx_buf == NULL) {
            return HAL_SE_BAD_SEG_CTX_BUF;
        }
        mode = GET_BITFIELD(se_hash->HASH_CFG, HASH_CFG_ALG_SELECT);
        ctx_len = hal_se_hash_mode_ctx_len(mode);
        if (hmac) {
            ctx_len *= 2;
        }
        if (cfg->seg_ctx_len < ctx_len) {
            return HAL_SE_BAD_SEG_CTX_LEN;
        }
        se_hash_restore_ctx(hmac, cfg->seg_ctx_buf, ctx_len);
    }
    if (save_ctx) {
        if (cfg->seg_ctx_buf == NULL) {
            return HAL_SE_BAD_SEG_CTX_BUF;
        }
    }

    se_done_hdlr = cfg->done_hdlr;
    se_seg_oper = cfg->seg_oper;
    se_seg_ctx_buf = cfg->seg_ctx_buf;

    if (cfg->in_len) {
        se_dma_enable_tx(cfg->in, cfg->in_len);
    }

    se_hash_set_start_cmd(cfg, op, in_len);

    return HAL_SE_OK;
}

enum HAL_SE_RET_T hal_se_hash_update(const struct HAL_SE_HASH_CFG_T *cfg)
{
    return se_hash_start(cfg, SE_HASH_OP_UPDATE, 0, false);
}

enum HAL_SE_RET_T hal_se_hash_final(const struct HAL_SE_HASH_CFG_T *cfg, uint64_t total_in_len)
{
    return se_hash_start(cfg, SE_HASH_OP_FINAL, total_in_len, false);
}

static enum HAL_SE_RET_T se_hash_hmac_save_seg_ctx(int hmac, void *seg_ctx_buf, uint32_t seg_ctx_len)
{
    enum HAL_SE_HASH_MODE_T mode;
    uint32_t ctx_len;

    if (!se_enabled) {
        return HAL_SE_NOT_OPENED;
    }
    if (hal_se_hash_busy()) {
        return HAL_SE_ENG_BUSY;
    }
    if (se_seg_oper != HAL_SE_SEG_INIT && se_seg_oper != HAL_SE_SEG_CONT) {
        return HAL_SE_BAD_SEG_OPER;
    }
    if (seg_ctx_buf == NULL) {
        return HAL_SE_BAD_SEG_CTX_BUF;
    }

    mode = GET_BITFIELD(se_hash->HASH_CFG, HASH_CFG_ALG_SELECT);
    ctx_len = hal_se_hash_mode_ctx_len(mode);
    if (hmac) {
        ctx_len *= 2;
    }
    if (seg_ctx_len < ctx_len) {
        return HAL_SE_BAD_SEG_CTX_LEN;
    }
    seg_ctx_len = ctx_len;

    se_hash_save_ctx(hmac, seg_ctx_buf, seg_ctx_len);

    return HAL_SE_OK;
}

enum HAL_SE_RET_T hal_se_hash_save_seg_ctx(void *seg_ctx_buf, uint32_t seg_ctx_len)
{
    return se_hash_hmac_save_seg_ctx(false, seg_ctx_buf, seg_ctx_len);
}

enum HAL_SE_RET_T hal_se_hash(enum HAL_SE_HASH_MODE_T mode, const struct HAL_SE_HASH_CFG_T *cfg)
{
    enum HAL_SE_RET_T ret;

    if (cfg == NULL) {
        return HAL_SE_CFG_NULL;
    }
    if (cfg->seg_oper != HAL_SE_SEG_NONE) {
        return HAL_SE_BAD_SEG_OPER;
    }

    ret = hal_se_hash_init(mode);
    if (ret != HAL_SE_OK) {
        return ret;
    }
    ret = hal_se_hash_final(cfg, cfg->in_len);
    return ret;
}

enum HAL_SE_RET_T hal_se_hash_get_digest(void *out, uint32_t out_len, uint32_t *real_len)
{
    if (!se_enabled) {
        return HAL_SE_NOT_OPENED;
    }

    return se_hash_get_digest(out, out_len, real_len);
}

int hal_se_hash_busy(void)
{
    if (se_enabled) {
        if (se_hash->HASH_STATUS & HASH_STATUS_HASH_BUSY) {
            return true;
        }
        if (se_dma->ENBLDCHNS & DMA_STAT_CHAN(SE_DMA_TX_CHAN)) {
            return true;
        }
    }
    return false;
}

enum HAL_SE_RET_T hal_se_hash_reset(void)
{
    uint32_t lock;

    if (!se_enabled) {
        return HAL_SE_NOT_OPENED;
    }

    lock = int_lock();
    se_dma_cancel_tx();
    /* Clear all DMA interrupt and error flag */
    se_dma->INTTCCLR = ~0UL;
    se_dma->INTERRCLR = ~0UL;

    se_adec->ADEC_CTRL |= ADEC_CTRL_CLK_EN_15_0(SE_ACC_CLK_HASH) | ADEC_CTRL_RST_15_0(SE_ACC_RST_HASH);
    se_adec->ADEC_CTRL &= ~ADEC_CTRL_RST_15_0(SE_ACC_RST_HASH);
    se_hash->HASH_STATUS = ~0UL;
    se_adec->ADEC_INT = SE_ACC_INT_HASH;

    se_done_hdlr = NULL;
    int_unlock(lock);

    return HAL_SE_OK;
}

static void hal_se_hmac_init_pre_start(const struct SE_HASH_INIT_CB_PARAM_T *param)
{
    if (param->seg_ctx_buf) {
        se_hash_restore_ctx(true, param->seg_ctx_buf, param->seg_ctx_len);
    }
    se_hash->HASH_CFG |= HASH_CFG_HASH_MODE;
    write_mreg(&se_hash->HMAC_KEY[0], param->key, param->key_len);
    se_hash->HMAC_KEY_LEN = param->key_len;
}

enum HAL_SE_RET_T hal_se_hmac_init(enum HAL_SE_HASH_MODE_T mode, const void *key, uint32_t key_len, const struct HAL_SE_HASH_CFG_T *cfg)
{
    struct SE_HASH_INIT_CB_PARAM_T param;
    void *ctx_buf;
    uint32_t ctx_len;

    if (key == NULL) {
        return HAL_SE_KEY_NULL;
    }
    if (key_len == 0) {
        return HAL_SE_BAD_KEY_LEN;
    }
    if (mode == HAL_SE_HASH_SHA384 || mode == HAL_SE_HASH_SHA512) {
        if (key_len > 128) {
            return HAL_SE_BAD_KEY_LEN;
        }
    } else {
        if (key_len > 64) {
            return HAL_SE_BAD_KEY_LEN;
        }
    }
    if (cfg && cfg->seg_oper == HAL_SE_SEG_CONT) {
        if (cfg->seg_ctx_buf == NULL) {
            return HAL_SE_BAD_SEG_CTX_BUF;
        }
        ctx_buf = cfg->seg_ctx_buf;
        ctx_len = hal_se_hash_mode_ctx_len(mode);
        ctx_len *= 2;
        if (cfg->seg_ctx_len < ctx_len) {
            return HAL_SE_BAD_SEG_CTX_LEN;
        }
    } else {
        ctx_buf = NULL;
        ctx_len = 0;
    }
    if (!se_enabled) {
        return HAL_SE_NOT_OPENED;
    }

    se_seg_ctx_buf = NULL;
    se_seg_oper = HAL_SE_SEG_NONE;

    memset(&param, 0, sizeof(param));
    param.seg_ctx_buf = ctx_buf;
    param.seg_ctx_len = ctx_len;
    param.key = key;
    param.key_len = key_len;

    return se_hash_init(mode, hal_se_hmac_init_pre_start, &param);
}

enum HAL_SE_RET_T hal_se_hmac_update(const struct HAL_SE_HASH_CFG_T *cfg)
{
    return se_hash_start(cfg, SE_HASH_OP_UPDATE, 0, true);
}

enum HAL_SE_RET_T hal_se_hmac_final(const struct HAL_SE_HASH_CFG_T *cfg, uint64_t total_in_len)
{
    return se_hash_start(cfg, SE_HASH_OP_FINAL, total_in_len, true);
}

enum HAL_SE_RET_T hal_se_hmac_save_seg_ctx(void *seg_ctx_buf, uint32_t seg_ctx_len)
{
    return se_hash_hmac_save_seg_ctx(true, seg_ctx_buf, seg_ctx_len);
}

enum HAL_SE_RET_T hal_se_hmac(enum HAL_SE_HASH_MODE_T mode, const void *key,
                        uint32_t key_len, const struct HAL_SE_HASH_CFG_T *cfg)
{
    enum HAL_SE_RET_T ret;

    if (cfg == NULL) {
        return HAL_SE_CFG_NULL;
    }
    if (cfg->seg_oper != HAL_SE_SEG_NONE) {
        return HAL_SE_BAD_SEG_OPER;
    }

    ret = hal_se_hmac_init(mode, key, key_len, NULL);
    if (ret != HAL_SE_OK) {
        return ret;
    }
    ret = hal_se_hmac_final(cfg, cfg->in_len);
    return ret;
}

enum HAL_SE_RET_T hal_se_hmac_get_digest(void *out, uint32_t out_len, uint32_t *real_len) __attribute__((alias("hal_se_hash_get_digest")));

int hal_se_hmac_busy(void) __attribute__((alias("hal_se_hash_busy")));

enum HAL_SE_RET_T hal_se_hmac_reset(void) __attribute__((alias("hal_se_hash_reset")));
#endif // !SEC_ENG_ROM_ONLY

#endif // SEC_ENG_HAS_HASH

#endif // SEC_ENG_BASE

