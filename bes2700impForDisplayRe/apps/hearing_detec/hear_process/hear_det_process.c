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
#include "hear_det_process.h"
#include <stdlib.h>
#include <math.h>

#define HEAR_DET_LOG_VERSION "2021-03-05"

#define WAV_BUF_SIZE 96
const short loc_wav[WAV_BUF_SIZE] ={
    #include "tone.txt"
};

#define DB2LIN(x) (powf(10.f, (x) / 20.f))
int hear_tone_freq = 500;
int hear_tone_amp_dB = 0;
float hear_tone_amp = 1.0;
int tone_wav_step = 1;

int get_cur_step(void)
{
    TRACE(0,"HEARING DET VERSION : %s",HEAR_DET_LOG_VERSION);
    int cur_step = 1;
    int cur_tone_freq = hear_tone_freq;
    switch (cur_tone_freq)
    {
        case 500:
            cur_step = 1;
            break;
        case 1000:
            cur_step = 2;
            break;
        case 2000:
            cur_step = 4;
            break;
        case 4000:
            cur_step = 8;
            break;
        case 6000:
            cur_step = 12;
            break;
        case 8000:
            cur_step = 16;
            break;
        default:
            cur_step = 1;
            ASSERT(0,"cur_tone_freq(%d) not support",cur_tone_freq);
            break;
    }
    TRACE(2,"[%s] cur_step = %d",__func__,cur_step);
    tone_wav_step = cur_step;
    return cur_step;
}

float hear_spk_calib_gain_buf[6] = {0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F};

void update_spk_calib_gain(float *input_buf)
{
    for(int i=0;i<6;i++)
        hear_spk_calib_gain_buf[i] = input_buf[i];
    TRACE(1,"spk_calib[0]=%d,[1]=%d,[2]=%d,[3]=%d,[4]=%d,[5]=%d",
        (int)hear_spk_calib_gain_buf[0],(int)hear_spk_calib_gain_buf[1],
        (int)hear_spk_calib_gain_buf[2],(int)hear_spk_calib_gain_buf[3],
        (int)hear_spk_calib_gain_buf[4],(int)hear_spk_calib_gain_buf[5]);
}

float get_cur_amp(void)
{
    float tmp_amp = 0.0;
    float speaker_calib_gain = 0.0F;
    int cur_tone_freq = hear_tone_freq;
    switch (cur_tone_freq)
    {
        case 500:
            speaker_calib_gain = hear_spk_calib_gain_buf[0];
            break;
        case 1000:
            speaker_calib_gain = hear_spk_calib_gain_buf[1];
            break;
        case 2000:
            speaker_calib_gain = hear_spk_calib_gain_buf[2];
            break;
        case 4000:
            speaker_calib_gain = hear_spk_calib_gain_buf[3];
            break;
        case 6000:
            speaker_calib_gain = hear_spk_calib_gain_buf[4];
            break;
        case 8000:
            speaker_calib_gain = hear_spk_calib_gain_buf[5];
            break;
        default:
            speaker_calib_gain = hear_spk_calib_gain_buf[0];
            ASSERT(0,"cur_tone_freq(%d) not support",cur_tone_freq);
            break;
    }

    TRACE(2,"cur_amp_dB = %d+%de-2",hear_tone_amp_dB,(int)(speaker_calib_gain*100));
    tmp_amp = DB2LIN((float)hear_tone_amp_dB + speaker_calib_gain);
    TRACE(2,"[%s] cur_amp = %de-4",__func__,(int)(tmp_amp*10000));
    hear_tone_amp = tmp_amp;
    return tmp_amp;
}

int play_time_cnt = 0;
int loc_wav_idx = 0;
short fade_gain = 0;
short fade_cnt = 1152;

void reset_hear_det_para(void)
{
    play_time_cnt = 0;
    loc_wav_idx = 0;
    fade_gain = 0;
    fade_cnt = 1152;
}

int hear_rx_process(int *pcm_buf, int pcm_len)
{
    float play_rx_gain = 1.0;
    short rx_target_gain = 0;
    if(32 > play_time_cnt)
        rx_target_gain = fade_cnt;
    else
        rx_target_gain = 0;

    play_time_cnt++;
    if(64 == play_time_cnt)
        play_time_cnt = 0;

    for(int i=0; i<pcm_len; i++)
    {
        if(fade_gain < rx_target_gain)
            fade_gain++;
        else if(fade_gain > rx_target_gain)
            fade_gain--;
        if(0!=fade_cnt)
            play_rx_gain = (float)fade_gain/(float)fade_cnt;
        else
        {
            play_rx_gain = 1.0;
            TRACE(0,"ERROR!! play_rx_gain=0!!");
        }
        pcm_buf[i] = (int)(play_rx_gain*hear_tone_amp*loc_wav[loc_wav_idx]*256*1);
        loc_wav_idx = (loc_wav_idx + tone_wav_step) % WAV_BUF_SIZE;
    }
    return 0;
}

void set_hearing_para(int uart_freq, int uart_amp_dB)
{
    hear_tone_freq = uart_freq;
    hear_tone_amp_dB = uart_amp_dB;
}

void set_hearing_amp(int uart_amp_dB)
{
    hear_tone_amp_dB = uart_amp_dB;
    hear_tone_amp = get_cur_amp();
}

void set_hearing_freq(int freq)
{
    hear_tone_freq = freq;
    tone_wav_step = get_cur_step();
}


