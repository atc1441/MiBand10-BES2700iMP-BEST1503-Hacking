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
#include "beco_average_pooling_2x2.hpp"
#include "beco_nnfunctions.h"
#include <assert.h>


void beco_average_pooling_2x2_HWC_q7(const q7_t* src,
                                     const int dim_in_h,
                                     const int dim_in_w,
                                     const int dim_in_c,
                                     q7_t *dest)
{
    assert(dim_in_c % 8 == 0);
    beco_average_pooling_2x2<q7_t>(src, dim_in_h, dim_in_w, dim_in_c, dest);
}

void beco_average_pooling_2x2_HWC_q15(const q15_t* src,
                                      const int dim_in_h,
                                      const int dim_in_w,
                                      const int dim_in_c,
                                      q15_t *dest)
{
    assert(dim_in_c % 4 == 0);
    beco_average_pooling_2x2<q15_t>(src, dim_in_h, dim_in_w, dim_in_c, dest);
}

void beco_imgCHW2HWC_q7(const q7_t *src,
                        const int dim_in_h,
                        const int dim_in_w,
                        const int dim_in_c,
                        q7_t *src_hwc)
{
    beco_imgCHW2HWC<q7_t>(src, dim_in_h, dim_in_w, dim_in_c, src_hwc);
}

void beco_imgCHW2HWC_q15(const q15_t *src,
                         const int dim_in_h,
                         const int dim_in_w,
                         const int dim_in_c,
                         q15_t *src_hwc)
{
    beco_imgCHW2HWC<q15_t>(src, dim_in_h, dim_in_w, dim_in_c, src_hwc);
}