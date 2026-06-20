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
#ifndef __NN_NS_H__
#define __NN_NS_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct NnNsState_;

typedef struct NnNsState_ NnNsState;

#define CONSTRUCT_FUNC_NAME_A(p, c, m)          p ## _ ## c ## _ ## m
#define CONSTRUCT_FUNC_NAME(p, c, m)            CONSTRUCT_FUNC_NAME_A(p, c, m)

#ifndef NN_NS_IMPL
#define NN_NS_IMPL large
#endif

#define nn_ns_create    CONSTRUCT_FUNC_NAME(nn_ns, NN_NS_IMPL, create)
#define nn_ns_destroy   CONSTRUCT_FUNC_NAME(nn_ns, NN_NS_IMPL, destroy)
#define nn_ns_process   CONSTRUCT_FUNC_NAME(nn_ns, NN_NS_IMPL, process)

NnNsState *nn_ns_create(int32_t sample_rate, int32_t frame_size, int32_t post_coef);
void nn_ns_destroy(NnNsState *st);
void nn_ns_process(NnNsState * st, int16_t *pcm_buf);

#ifdef __cplusplus
}
#endif

#endif
