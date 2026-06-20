/*******************************************************************************
    Copyright 2016-2017 Sony Corporation
*******************************************************************************/
#ifndef LDAC_H__
#define LDAC_H__

#include <stdint.h>
#include "ldacBT.h"

#if defined __cplusplus
extern "C" {
#endif /* __cplusplus */

#define LDAC_FREQ_44100 LDACBT_SAMPLING_FREQ_044100
#define LDAC_FREQ_48000 LDACBT_SAMPLING_FREQ_048000
#define LDAC_FREQ_88200 LDACBT_SAMPLING_FREQ_088200
#define LDAC_FREQ_96000 LDACBT_SAMPLING_FREQ_096000
#define LDAC_FREQ_176400 LDACBT_SAMPLING_FREQ_176400
#define LDAC_FREQ_192000 LDACBT_SAMPLING_FREQ_192000

#define LDAC_CHANNEL_MONO LDACBT_CHANNEL_MODE_MONO
#define LDAC_CHANNEL_DUAL LDACBT_CHANNEL_MODE_DUAL_CHANNEL
#define LDAC_CHANNEL_STEREO LDACBT_CHANNEL_MODE_STEREO

#define A2DP_SPEC_LDAC_VENDOR_ID \
    { LDACBT_VENDOR_ID0, LDACBT_VENDOR_ID1, LDACBT_VENDOR_ID2, LDACBT_VENDOR_ID3 }
#define A2DP_SPEC_LDAC_CODEC_ID \
    { LDACBT_CODEC_ID0, LDACBT_CODEC_ID1 }

#define A2DP_NON_A2DP_CODEC 0xff

typedef struct {
    uint8_t vendor[4];
    uint8_t codec[2];
    uint8_t frequency;
    uint8_t channel;
} __attribute__((packed)) a2dp_ldac_t;

#if defined __cplusplus
}
#endif /* __cplusplus */

#endif /* LDAC_H__ */
