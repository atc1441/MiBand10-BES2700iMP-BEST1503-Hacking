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
#ifndef SPEECH_EQ_CFG_H
#define SPEECH_EQ_CFG_H

#include "iirfilt.h"

#define MAX_VQE_EQ_BAND 20

typedef struct
{
    enum IIR_BIQUARD_TYPE type;
    union
    {
        /* Raw config, used when type is IIR_BIQUARD_RAW */
        struct
        {
            float a1; float a2; float b0; float b1; float b2;
        } raw;
        /* Generate coeffs using user defined params, used in other type */
        struct
        {
            float f0; float gain; float q;
        } design;
    };
} BiquardParam;

typedef struct
{
    int32_t     bypass;
    float       gain;
    int32_t     num;
	BiquardParam params[MAX_VQE_EQ_BAND];
} EqConfig;

void speech_eq_dump_config(const EqConfig *config);

#endif