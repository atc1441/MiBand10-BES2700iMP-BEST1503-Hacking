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
#ifndef __CODEC_LC3_H__
#define __CODEC_LC3_H__


#if !defined(A2DP_LC3_HR)
#define A2DP_LC3_OCTET_NUMBER    (10)
// [Byte 0-3] Vendor ID
#define A2DP_LC3_VENDOR_ID    0x0000038F
// [Byte 4-5] Vendor Codec ID
#define A2DP_LC3_CODEC_ID    0x8FAA
#define A2DP_LC3_PLUS_CODEC_ID    0x8FAB

// [Byte 6]  [Bits 0-6] Sample Rate
#define A2DP_LC3_SR_96000    0x01
#define A2DP_LC3_SR_48000    0x02
#define A2DP_LC3_SR_44100    0x04
#define A2DP_LC3_SR_32000    0x08
#define A2DP_LC3_SR_24000    0x10
#define A2DP_LC3_SR_16000    0x20
#define A2DP_LC3_SR_8000    0x40
#define A2DP_LC3_SR_DATA(X)    (X & (A2DP_LC3_SR_96000 | A2DP_LC3_SR_48000 | A2DP_LC3_SR_44100 \
	                                               | A2DP_LC3_SR_32000 |A2DP_LC3_SR_24000 |A2DP_LC3_SR_16000 | A2DP_LC3_SR_8000))

// [Byte 7] [Bits 0-2] Bits per sample
#define A2DP_LC3_FMT_16BIT    0x01
#define A2DP_LC3_FMT_24BIT    0x02
#define A2DP_LC3_FMT_32BIT    0x04
#define A2DP_LC3_FMT_DATA(X)    (X & (A2DP_LC3_FMT_16BIT | A2DP_LC3_FMT_24BIT | A2DP_LC3_FMT_32BIT))

////[Byte 7] [Bits 4-7] Frame len
#define A2DP_LC3_FRAME_LEN_10MS    0x10
#define A2DP_LC3_FRAME_LEN_7POINT5MS    0x20
#define A2DP_LC3_FRAME_LEN_5MS    0x40
#define A2DP_LC3_FRAME_LEN_2POINT5MS    0x80
#define A2DP_LC3_FRAME_LEN_DATA(X)    (X & (A2DP_LC3_FRAME_LEN_10MS | A2DP_LC3_FRAME_LEN_7POINT5MS \
	                                                              | A2DP_LC3_FRAME_LEN_5MS | A2DP_LC3_FRAME_LEN_2POINT5MS))

////[Byte 8] [bits0-6] Bitrate
#define A2DP_LC3_BITRATE_900kBPS    0x01
#define A2DP_LC3_BITRATE_600kBPS    0x02
#define A2DP_LC3_BITRATE_500kBPS    0x04
#define A2DP_LC3_BITRATE_400kBPS    0x08
#define A2DP_LC3_BITRATE_300kBPS    0x10
#define A2DP_LC3_BITRATE_200kBPS    0x20
#define A2DP_LC3_BITRATE_100kBPS    0x40
#define A2DP_LC3_BITRATE_64kBPS      0x80
#define A2DP_LC3_BITRATE_DATA(X)    (X & (A2DP_LC3_BITRATE_900kBPS | A2DP_LC3_BITRATE_600kBPS | A2DP_LC3_BITRATE_500kBPS \
	                                                          | A2DP_LC3_BITRATE_400kBPS |A2DP_LC3_BITRATE_300kBPS |A2DP_LC3_BITRATE_200kBPS \
	                                                          | A2DP_LC3_BITRATE_100kBPS | A2DP_LC3_BITRATE_64kBPS))

////[Byte 9] [bits0-2] Channel Mode
#define A2DP_LC3_CH_MD_MONO    0x01
#define A2DP_LC3_CH_MD_STEREO    0x02
#define A2DP_LC3_CH_MD_MUlTI_MONO    0x04
#define A2DP_LC3_CH_MD_DATA(X)        (X & (A2DP_LC3_CH_MD_MONO | A2DP_LC3_CH_MD_STEREO | A2DP_LC3_CH_MD_MUlTI_MONO))


//[Byte 9] [bit4] Bitrate mode
#define A2DP_LC3_BR_MODE_NONE             0x00
#define A2DP_LC3_BR_MODE_AUTO        0x10
#define A2DP_LC3_BR_MODE_DATA(X)        (X & (A2DP_LC3_BR_MODE_AUTO))
#else
#define A2DP_LC3_OCTET_NUMBER    (11)
// [Byte 0-3] Vendor ID
#define A2DP_LC3_VENDOR_ID    0x000008A9
// [Byte 4-5] Vendor Codec ID
#define A2DP_LC3_CODEC_ID    0x0001
#define A2DP_LC3_PLUS_CODEC_ID    0x8FAB


////[Byte 6] [Bits 0-7] Frame len ms
#define A2DP_LC3_FRAME_LEN_10MS    0x40
#define A2DP_LC3_FRAME_LEN_5MS    0x20
#define A2DP_LC3_FRAME_LEN_2POINT5MS    0x10
#define A2DP_LC3_FRAME_LEN_DATA(X)    (X & (A2DP_LC3_FRAME_LEN_10MS  \
	                                                              | A2DP_LC3_FRAME_LEN_5MS | A2DP_LC3_FRAME_LEN_2POINT5MS))



////[Byte 7 [bits 4-7] Channel Mode
#define A2DP_LC3_CH_MD_MONO    0x10
#define A2DP_LC3_CH_MD_STEREO    0x40
#define A2DP_LC3_CH_MD_DATA(X)        (X & (A2DP_LC3_CH_MD_MONO | A2DP_LC3_CH_MD_STEREO ))



// [Byte 8]  [Bits 3-4] Sample Rate
#define A2DP_LC3_SR_96000    0x80
#define A2DP_LC3_SR_48000    0x01

#define A2DP_LC3_SR_DATA(X)    (X & (A2DP_LC3_SR_96000 | A2DP_LC3_SR_48000))

// [Byte 9] [Bits 0-2] Bits per sample
#define A2DP_LC3_FMT_16BIT    0x01
#define A2DP_LC3_FMT_24BIT    0x02
#define A2DP_LC3_FMT_32BIT    0x04
#define A2DP_LC3_FMT_DATA(X)    (X & (A2DP_LC3_FMT_16BIT | A2DP_LC3_FMT_24BIT | A2DP_LC3_FMT_32BIT))


#if 0
////[Byte 8] [bits0-6] Bitrate
#define A2DP_LC3_BITRATE_900kBPS    0x01
#define A2DP_LC3_BITRATE_600kBPS    0x02
#define A2DP_LC3_BITRATE_500kBPS    0x04
#define A2DP_LC3_BITRATE_400kBPS    0x08
#define A2DP_LC3_BITRATE_300kBPS    0x10
#define A2DP_LC3_BITRATE_200kBPS    0x20
#define A2DP_LC3_BITRATE_100kBPS    0x40
#define A2DP_LC3_BITRATE_64kBPS      0x80
#define A2DP_LC3_BITRATE_DATA(X)    (X & (A2DP_LC3_BITRATE_900kBPS | A2DP_LC3_BITRATE_600kBPS | A2DP_LC3_BITRATE_500kBPS \
	                                                          | A2DP_LC3_BITRATE_400kBPS |A2DP_LC3_BITRATE_300kBPS |A2DP_LC3_BITRATE_200kBPS \
#endif	                                                          | A2DP_LC3_BITRATE_100kBPS | A2DP_LC3_BITRATE_64kBPS))



#endif

#endif
#ifdef __cplusplus
extern "C" {
#endif                          /*  */


#ifdef __cplusplus
}
#endif

#endif /* __CODEC_LHDC_H__ */
