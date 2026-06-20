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
#if defined (VOICE_ASSIST_CUSTOM_LEAK_DETECT)
#include "hal_trace.h"
#include "anc_assist.h"
#include "app_anc_assist.h"
#include "app_voice_assist_custom_leak_detect.h"
#include "arm_math.h"
#include "cmsis_os.h"
#include "app_media_player.h"

static int32_t _voice_assist_custom_leak_detect_callback(void *buf, uint32_t len, void *other);
#define Average (10)
#define Summary (3)      /*control test time*/
#define BeepLen (500)
#define SINGLE10HZ_THRESHOLD  (100000)
#define BEEP10HZ_THRESHOLD    (20000)
static int cnt=0;
static int energy_cnt=0;
static int summary_cnt=0;
static int stop_flag = 0;

#define SINGLE10HZ (0)
#define BEEP10HZ   (1)
#define NOSWITCH (0)
#define SWITCH   (1)

float standard = 2000;   /*important parameter after calib finish decide*/
float aec_ref = 1;

static enum LEAK_DETECT_STATUS leak_detect_flag;

int32_t app_voice_assist_custom_leak_detect_init(void)
{
    app_anc_assist_register(ANC_ASSIST_USER_CUSTOM_LEAK_DETECT, _voice_assist_custom_leak_detect_callback);
    cnt = 0;

    return 0;
}

extern void app_anc_assist_reset_pilot_state(void);

void app_voice_assist_custom_leak_detect_set_working_status(int32_t status){
    app_anc_assist_set_custom_leak_working_status(status);
}


int32_t app_voice_assist_custom_leak_detect_open(void)
{
    app_anc_assist_open(ANC_ASSIST_USER_CUSTOM_LEAK_DETECT);
    // reset_custom_leak_detect();
    app_anc_assist_reset_pilot_state();
    leak_detect_flag = 0 ;
    stop_flag = 0;
    cnt=0;
    energy_cnt=0;
    summary_cnt=0;
    app_voice_assist_custom_leak_detect_set_working_status(1);
    return 0;
}

int32_t app_voice_assist_custom_leak_detect_close(void)
{
    app_anc_assist_close(ANC_ASSIST_USER_CUSTOM_LEAK_DETECT);

    return 0;
}


#ifdef CUSTOMER_APP_BOAT
extern void app_tota_leak_detect_result();
static uint8_t leak_detect_status = 0;
#endif
static int32_t _voice_assist_custom_leak_detect_callback(void *buf, uint32_t len, void *other)
{
    // TRACE(0, "[%s] len = %d", __func__, len);
    static float fb_energy_l[Average];
    static float fb_energy_s[Summary];
    float energy_monitor = 0.0; 
    float energy_sum = 0.0;
    float energy_summary = 0.0;

    float * res_lst = (float *)buf;
    float res = res_lst[0];
    uint32_t standalone_wd_working_status = (int)res_lst[1];
    // int assist_standalone_wd_total_cnt = (int)res_lst[2];
    // TRACE(2,"!!!!!!!!!!!!!!!!!!!!!!!!!!!! %d ",(int)(100000* res));
    if(standalone_wd_working_status == CUSTOM_LEAK_DETECT_STATUS_UNKNOWN){
        // TRACE(2,"[%s] get unknown status", __func__);
    }
    if(standalone_wd_working_status == CUSTOM_LEAK_DETECT_STATUS_WAIT){
        return 0;
    }
    // TRACE(2,"[%s] stop stream with status: %d %d",__func__,standalone_wd_working_status,cnt);
    if(standalone_wd_working_status != CUSTOM_LEAK_DETECT_STATUS_STOP)
    {
        cnt++;
        // api (fb ref)
        if(cnt == 25)
        {
            if(res < 0){
                fb_energy_l[energy_cnt] = -(100000*res);
            }
            else{
                fb_energy_l[energy_cnt] = (100000*res);
            }
            
            energy_cnt++;
            // TRACE(2, "energy_cnt = %d", energy_cnt);
            cnt = 0;
        }

        if(energy_cnt >= Average)
        {
            for (int i=0;i<Average;i++)
                energy_sum += fb_energy_l[i];
            energy_monitor = energy_sum/Average;
            fb_energy_s[summary_cnt] = energy_monitor;
            TRACE(2, "[%s] leak detect test: [%d]time, fb_energy = %d", __func__, summary_cnt, (int)(fb_energy_s[summary_cnt]));
            summary_cnt++;
            if(summary_cnt >= Summary){
                summary_cnt = Summary-1;
            }
            energy_cnt = 0;
        }

        if(standalone_wd_working_status == CUSTOM_LEAK_DETECT_STATUS_RESULT)
        {
            for(int i=1; i<Summary; i++) energy_summary += fb_energy_s[i];
            energy_summary /=(Summary-1);
            aec_ref = standard/energy_summary;
            summary_cnt = 0;

            // if(leak_detect_flag == 0)
            // {
               
            // }
            
#ifdef LEAK_DETECT_CALIB
            else if(leak_detect_flag == 1)
            {
                TRACE(2, "[%s] ===============CALIB FINISHED==============="__func__);
                TRACE(2, "[%s] fb_energy = %d, aec_ref = %d", __func__, (int)(energy_summary), (int)(10*aec_ref));
            }
#endif
            // else
            // {
            //     TRACE(2, "[%s] unkonw status: %d", __func__, leak_detect_flag);
            // }
            stop_flag++;
            if(stop_flag == 1){
                TRACE(2,"[%s] ======LEAK DETECT TEST FINISHED======",__func__);
                TRACE(2,"[%s] fb_energy = %d, aec_ref = %d  monitor = %d", __func__, (int)(energy_summary), (int)(10*aec_ref),(int)(energy_monitor));
                
                if ((int)(energy_summary) >= BEEP10HZ_THRESHOLD) 
                {
                    TRACE(0,"NORMAL");
#ifdef CUSTOMER_APP_BOAT
                    leak_detect_status = 0x01;
                    app_tota_leak_detect_result();
#endif
                }
                else 
                {
                    TRACE(0,"LOOSE");
#ifdef CUSTOMER_APP_BOAT
                    leak_detect_status = 0x00;
                    app_tota_leak_detect_result();
#endif
                }

                TRACE(2,"[%s] stop stream with status: %d",__func__,standalone_wd_working_status);
                app_voice_assist_custom_leak_detect_close();
            }

        }
    } 
    // if(standalone_wd_working_status)
    // {
    //     app_voice_assist_standalone_wd_close();
    // }


    return 0;
}

#ifdef CUSTOMER_APP_BOAT
uint8_t app_voice_assitant_get_status(void)
{
    return leak_detect_status;
}
#endif

#endif
