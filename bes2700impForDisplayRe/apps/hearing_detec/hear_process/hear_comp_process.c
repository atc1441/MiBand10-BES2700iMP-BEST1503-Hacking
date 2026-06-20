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
#include "hal_trace.h"
#include "hear_comp_process.h"
#include <stdlib.h>

#define HEAR_LOG_VERSION "2021-02-24-public"

static int hear_result_map(int pwr_val)
{
    int pwr_lvl = 0;
    if(11 == pwr_val)
        pwr_lvl = 11;
    else if( 10 == pwr_val)
        pwr_lvl = 10;
    else if(  9 == pwr_val)
        pwr_lvl = 9;
    else if(  8 == pwr_val)
        pwr_lvl = 8;
    else if( 7 == pwr_val)
        pwr_lvl = 7;
    else if( 6 == pwr_val)
        pwr_lvl = 6;
    else if( 5 == pwr_val)
        pwr_lvl = 5;
    else if( 4 == pwr_val)
        pwr_lvl = 4;
    else if( 4 == pwr_val)
        pwr_lvl = 3;
    else if( 2 == pwr_val)
        pwr_lvl = 2;
    else if( 1 == pwr_val)
        pwr_lvl = 1;
    else if( 0 == pwr_val)
        pwr_lvl = 0;
    else
        TRACE(1,"[%s] invalid pwr result = %d",__func__,pwr_val);

    return pwr_lvl;
}

float level_lut0[12] = {0.0F, 1.0F, 2.0F, 3.0F, 4.5F, 5.0F,
                        6.0F, 7.0F, 8.0F, 9.0F, 10.0F, 11.0F};

float get_hear_compen_val_fir(int *test_buf, float *gain_buf, int num)
{
    ASSERT(num==6,"[%s]num = %d should be 6",__func__,num);
    TRACE(0,"HEARING COMP VERSION : %s",HEAR_LOG_VERSION);
    float* level_lut = NULL;
    level_lut = level_lut0;

    for(int i=0;i<num;i++)
        test_buf[i] = hear_result_map(test_buf[i]);
    TRACE(1,"hearing level is [%d,%d,%d,%d,%d,%d]",
               test_buf[0],test_buf[1],test_buf[2],
               test_buf[3],test_buf[4],test_buf[5]);

    for(int i=0;i<num;i++)
    {
       int tmp_band = test_buf[i];
       if(0<=tmp_band && 11>=tmp_band)
           gain_buf[i] = level_lut[tmp_band];
       else
           ASSERT(0,"test level(band[%d]=%d) should be in range [0,9]",i,tmp_band);
    }

    int max_gain = 20;
    for(int i=0;i<num;i++)
    {
        if(1 >= i)
            max_gain = 8;
        else if(4 >= i)
            max_gain = 10;
        else
            max_gain = 14;

        if(max_gain < gain_buf[i])
            gain_buf[i] = max_gain;
    }
    return 0;
}

short hear_comp_level = 0;
short get_hear_level(void)
{
    return hear_comp_level;
}
float get_hear_compen_val_multi_level(int *test_buf, float *gain_buf, int num)
{
    ASSERT(num==6,"[%s]num = %d should be 6",__func__,num);
    TRACE(0,"HEARING COMP VERSION : %s",HEAR_LOG_VERSION);
    float* level_lut = NULL;
    for(int i=0;i<num;i++)
    {
       test_buf[i] = hear_result_map(test_buf[i]);
    }

    TRACE(1,"hearing level is [%d,%d,%d,%d,%d,%d]",
               test_buf[0],test_buf[1],test_buf[2],
               test_buf[3],test_buf[4],test_buf[5]);
    //combine 4K&6K
    test_buf[3] = (test_buf[3] + test_buf[4])/2;
    test_buf[4] = test_buf[5];

    float min_in_buf = 100;
    float max_in_buf = 0;
    for(int i=0;i<5;i++)
    {
        if(min_in_buf > test_buf[i])
            min_in_buf = test_buf[i];
        if(max_in_buf < test_buf[i])
            max_in_buf = test_buf[i];
    }

    int max_gain = 12;
    float mean_gain = 0;
    float max_levl_mid_freq = 0;
    for(int i=1;i<4;i++)
    {
        level_lut = level_lut0;
        ASSERT(NULL!=level_lut,"[%s] level_lut = %d",__func__,(int)level_lut);

        int tmp_band = test_buf[i];
        if(0<=tmp_band && 11>=tmp_band)
            gain_buf[i] = level_lut[tmp_band];
        else
            ASSERT(0,"test level(band[%d]=%d) should be in range [0,9]",i,tmp_band);

        mean_gain += gain_buf[i];
        if(max_levl_mid_freq<tmp_band)
            max_levl_mid_freq = tmp_band;
    }

    float levl_mid_thd = 5.0;
    int tmp_band  = 0;
    tmp_band = test_buf[0];
    if(levl_mid_thd < max_levl_mid_freq)
    {
        tmp_band = (int)(tmp_band/2);
    }
    level_lut = level_lut0;
    if(0<=tmp_band && 11>=tmp_band)
        gain_buf[0] = level_lut[tmp_band];
    else
        ASSERT(0,"test level(band[%d]=%d) should be in range [0,11]",0,tmp_band);

    tmp_band = test_buf[4];
    if(levl_mid_thd < max_levl_mid_freq)
    {
        tmp_band = (int)(tmp_band/2);
    }
    level_lut = level_lut0;
    if(0<=tmp_band && 11>=tmp_band)
        gain_buf[4] = level_lut[tmp_band];
    else
        ASSERT(0,"test level(band[%d]=%d) should be in range [0,11]",4,tmp_band);

    TRACE(1,"gain_buf is [%d,%d,%d,%d,%d]e-2",
               (int)(gain_buf[0]*100),(int)(gain_buf[1]*100),
               (int)(gain_buf[2]*100),(int)(gain_buf[3]*100),
               (int)(gain_buf[4]*100));

    mean_gain += gain_buf[0];
    mean_gain += gain_buf[4];
    mean_gain = mean_gain/5;

    for(int i=0;i<5;i++)
    {
        if(1 >= i)
            max_gain = 8;
        else if(3 >= i)
            max_gain = 10;
        else
            max_gain = 14;
        if(max_gain < gain_buf[i])
            gain_buf[i] = max_gain;
    }
    TRACE(1,"min=%d,max=%d",(int)min_in_buf,(int)max_in_buf);
    if( (max_in_buf - min_in_buf) < 2)
    {
        gain_buf[0] -= mean_gain;
        gain_buf[1] -= mean_gain;
        gain_buf[2] -= mean_gain;
        gain_buf[3] -= mean_gain;
        gain_buf[4] -= mean_gain;
    }

    //get hear_comp_level
    float gain0 = gain_buf[0];
    float gain1 = (gain_buf[1]+gain_buf[2]+gain_buf[3])/3;
    float gain2 = gain_buf[4];
    int max_idx= 0;
    float max_tmp = gain0;
    if(max_tmp<gain1)
    {
        max_idx = 1;
        max_tmp = gain1;
    }
    if(max_tmp<gain2)
    {
        max_idx = 2;
        max_tmp = gain2;
    }
    hear_comp_level = max_idx;
    TRACE(1,"| --- hear_comp_level = %d --- |",hear_comp_level);

    return 0;
}






