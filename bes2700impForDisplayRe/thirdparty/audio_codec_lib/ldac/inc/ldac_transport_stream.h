/*******************************************************************************
    Copyright 2016-2017 Sony Corporation
*******************************************************************************/
#ifndef LDAC_TRANSPORT_STREAM_H__
#define LDAC_TRANSPORT_STREAM_H__

#include <stdint.h>
#include <unistd.h>

#include "ldac_pthread_undefine.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum LDAC_SAMPLING_RATE {
    LDAC_RATE_44100,
    LDAC_RATE_48000,
    LDAC_RATE_88200,
    LDAC_RATE_96000,
    LDAC_RATE_176400,
    LDAC_RATE_192000,
    LDAC_RATE_NA,
};

enum LDAC_CHANNEL_MODE {
    LDAC_CH_MONO,
    LDAC_CH_DUAL,
    LDAC_CH_STEREO,
    LDAC_CH_NA,
};

enum LDAC_FRAME_STATUS {
    LDAC_STAT_0 = 0,
    LDAC_STAT_1 = 1,
    LDAC_STAT_2 = 2,
    LDAC_STAT_3 = 3,
};

struct ldac_ts_frame {
    enum LDAC_SAMPLING_RATE rate;
    enum LDAC_CHANNEL_MODE channel;
    enum LDAC_FRAME_STATUS status;

    void *data;
    ssize_t length;
};

struct ldac_ts_integrity_check {
    enum LDAC_SAMPLING_RATE rate;
    enum LDAC_CHANNEL_MODE channel;
};

int32_t ts_parse_frame(const void *data, ssize_t length, struct ldac_ts_integrity_check *check,
                       struct ldac_ts_frame *info);

void ts_print(struct ldac_ts_frame *info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LDAC_TRANSPORT_STREAM_H__ */
