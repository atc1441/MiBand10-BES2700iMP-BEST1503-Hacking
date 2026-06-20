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
#ifndef __CODEC_LHDCV5_H__
#define __CODEC_LHDCV5_H__


#define A2DP_LHDCV5_OCTET_NUMBER                   (11)
#define A2DP_LHDC_OCTET_NUMBER                     (9)

#define A2DP_LHDCV5_VENDOR_ID                       0x0000053a

#define A2DP_LHDCV5_CODEC_ID                        0x4C35

//To indicate Sampling Rate.
#define A2DP_LHDCV5_SR_192000                       0x01
#define A2DP_LHDCV5_SR_96000                        0x04
#define A2DP_LHDCV5_SR_48000                        0x10
#define A2DP_LHDCV5_SR_44100                        0x20
#define A2DP_LHDCV5_SR_DATA(X)                      ((X) & (A2DP_LHDCV5_SR_192000 | A2DP_LHDCV5_SR_96000 | A2DP_LHDCV5_SR_48000 | A2DP_LHDCV5_SR_44100))

//To indicate bits per sample.
#define A2DP_LHDCV5_FMT_32                          0x01
#define A2DP_LHDCV5_FMT_24                          0x02
#define A2DP_LHDCV5_FMT_16                          0x04
#define A2DP_LHDCV5_FMT_DATA(X)                     ((X) & (A2DP_LHDCV5_FMT_32 | A2DP_LHDCV5_FMT_24 | A2DP_LHDCV5_FMT_16))

#define A2DP_LHDCV5_LOSSLESS_OFF                    0X00
#define A2DP_LHDCV5_LOSSLESS_ON                     0X01
#define A2DP_LHDCV5_LOSSLESS_DATA(X)                ((X) >> 7)

#define A2DP_LHDCV5_HAS_AR                           0x01
#define A2DP_LHDCV5_HAS_JAS                          0x02
#define A2DP_LHDCV5_HAS_META                         0x04

#define A2DP_LHDCV5_VERSION_NUM                      0x01

#define A2DP_LHDCV5_MAX_BR_1000                      0x00
#define A2DP_LHDCV5_MAX_BR_900                       0x30
#define A2DP_LHDCV5_MAX_BR_600                       0x20
#define A2DP_LHDCV5_MAX_BR_400                       0x10
#define A2DP_LHDCV5_MIN_BR_400                       0xC0
#define A2DP_LHDCV5_MIN_BR_256                       0x80
#define A2DP_LHDCV5_MIN_BR_128                       0x40
#define A2DP_LHDCV5_MIN_BR_64                        0x00

#define A2DP_LHDCV5_FRAME_5MS                        0x10

#define A2DP_LHDCV5_LL_MODE                          0x40

#define A2DP_LHDCV5_LOSSLESS_MODE                    0x80

#define A2DP_LHDCV5_AR_ON                            0x01

typedef struct {
    uint32_t vendor_id;
    uint16_t codec_id;
    uint8_t bits;
    uint8_t sample_rater;       //uint:K
    uint8_t version_num;
    uint16_t max_sample_rate;   //uint:K  
    uint16_t min_sample_rate;   //uint:K  
    bool llc_enable;            //low latency control  
} lhdcv5_info_t;

#ifdef __cplusplus
extern "C" {
#endif                          /*  */

    void lhdcv5_info_parse(uint8_t * elements, lhdcv5_info_t * info);
    uint8_t a2dp_lhdcv5_get_sample_rate(uint8_t * elements);

#ifdef __cplusplus
}
#endif

#endif /* __CODEC_LHDCV5_H__ */