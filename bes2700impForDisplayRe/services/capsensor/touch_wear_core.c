/***************************************************************************
 * Copyright 2022-2023 BES.
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
 ***************************************************************************/
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif

#include "hal_trace.h"
#include "hal_timer.h"
#include "touch_wear_core.h"
#include "capsensor_driver.h"
#include "capsensor_algorithm.h"
#include "app_utils.h"
#if (CHIP_CAPSENSOR_VER < 1 || CHIP_CAPSENSOR_VER == 3)
#include CHIP_SPECIFIC_HDR(hal_capsensor)
#endif
#ifdef CAPSENSOR_SPP_SERVER
#include "capsensor_debug_server.h"
#endif
#ifdef CHIP_SUBSYS_SENS
#include "app_sensor_hub.h"
#endif
#include "capsensor_factory_cal.h"
#include "capsensor_driver_best1502p.h"
#include "capsensor_spi_best1502p.h"

// Global variables
static void capsensor_thread(const void *arg);
#if (CHIP_CAPSENSOR_VER == 3)//1306
#ifdef CAPSENSOR_SPP_SERVER
osThreadDef(capsensor_thread, osPriorityHigh, 1, (896), "capsensor_thread");
#else
osThreadDef(capsensor_thread, osPriorityHigh, 1, (768), "capsensor_thread");
#endif
#else
osThreadDef(capsensor_thread, osPriorityHigh, 1, (1280), "capsensor_thread");
#endif
static osThreadId capsensor_thread_id = NULL;

static struct capsensor_sample_data capsense_sample_buff[CAP_CHNUM*CAP_REPNUM]={0};
static capsensor_click_event_func_type capsensor_click_event_cb = NULL;

#ifdef CAPSENSOR_READ_DATA_POLLING
static int capsensor_polling_rawdata(int * rawdata_ready)
{
    int ret;

    *rawdata_ready = 0;
    ret = read_capsensor_fifo(capsense_sample_buff);
    if(ret == CAPSENSOR_READ_DATA_WAIT_READY) {
        osDelay((int)(CAP_CHNUM*2.304f + 0.5f));  //one ch:1/333.3k*96*8 = 2.304ms, 2.304*5 = 11.52ms
        ret = read_capsensor_fifo(capsense_sample_buff);
    }

    capsensor_state_machine_reset();

    if(ret == CAPSENSOR_READ_DATA_READY) {
        *rawdata_ready = 1;
        ret = 0;
    } else {
        TRACE(0, "%s error ret:%d", __func__, ret);
        ret = -1;
    }
    return ret;
}
#endif

/***************************************************************************
 * @brief touch or wear event callback function
 *
 * @param p function pointer
 * @return int
 ***************************************************************************/
int register_capsensor_click_event_callback(capsensor_click_event_func_type p)
{
    int8_t ret = 0;

    if (p == NULL) {
        ret = -1;
        return ret;
    }

    capsensor_click_event_cb = p;
    return ret;
}

#ifdef CHIP_SUBSYS_SENS
/* no response command start */
/***************************************************************************
 * @brief
 *
 * @param ptr : message pointer.
 * @param len : message lengths.
 ***************************************************************************/
static void app_sensor_hub_core_transmit_touch_no_rsp_cmd_handler(uint8_t* ptr, uint16_t len)
{
    app_core_bridge_send_data_without_waiting_rsp(MCU_SENSOR_HUB_TASK_CMD_TOUCH_REQ_NO_RSP, ptr, len);
}


/***************************************************************************
 * @brief
 *
 * @param ptr : message pointer.
 * @param len : message lengths.
 ***************************************************************************/
static void app_sensor_hub_core_touch_no_rsp_cmd_received_handler(uint8_t* ptr, uint16_t len)
{
    CAPSENSOR_TRACE(0, "Get touch no rsp command from mcu:");
    DUMP8("%02x ", ptr, len);
}

/***************************************************************************
 * @brief
 *
 * @param cmdCode
 * @param ptr : message pointer.
 * @param len : message lengths.
 ***************************************************************************/
static void app_sensor_hub_core_touch_no_rsp_cmd_tx_done_handler(uint16_t cmdCode,
    uint8_t* ptr, uint16_t len)
{
    CAPSENSOR_TRACE(0, "cmdCode 0x%x tx done", cmdCode);
}

/***************************************************************************
 * @brief
 *
 * @param ptr : message pointer.
 * @param len : message lengths.
 ***************************************************************************/
void app_sensor_hub_core_send_touch_req_no_rsp(uint8_t *ptr, uint16_t len)
{
    app_core_bridge_send_cmd(MCU_SENSOR_HUB_TASK_CMD_TOUCH_REQ_NO_RSP,
        ptr, len);
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_TOUCH_REQ_NO_RSP,
                                "touch no rsp req to mcu",
                                app_sensor_hub_core_transmit_touch_no_rsp_cmd_handler,
                                app_sensor_hub_core_touch_no_rsp_cmd_received_handler,
                                0,
                                NULL,
                                NULL,
                                app_sensor_hub_core_touch_no_rsp_cmd_tx_done_handler);
/* no response command end */

/***************************************************************************
 * @brief An instant message request from the sensor hub to the MCU.
 *
 * @param ptr : message pointer.
 * @param len : message lengths.
 ***************************************************************************/
static void app_sensor_hub_core_transmit_touch_instant_req_handler(uint8_t* ptr, uint16_t len)
{
    app_core_bridge_send_instant_cmd_data(MCU_SENSOR_HUB_INSTANT_CMD_TOUCH_REQ,
        ptr, len);
}


/***************************************************************************
 * @brief An instant message request from the MCU to the sensor hub.
 *
 * @param ptr : message pointer.
 * @param len : message lengths.
 ***************************************************************************/
static void app_sensor_hub_core_touch_instant_req_handler(uint8_t* ptr, uint16_t len)
{
    // for test purpose, we add log print here.
    // but as instant cmd handler will be directly called in intersys irq context,
    // for realistic use, should never do log print
    CAPSENSOR_TRACE(0, "Get demo instant req command from mcu:");
    DUMP8("%02x ", ptr, len);
}

CORE_BRIDGE_INSTANT_COMMAND_TO_ADD(MCU_SENSOR_HUB_INSTANT_CMD_TOUCH_REQ,
                                app_sensor_hub_core_transmit_touch_instant_req_handler,
                                app_sensor_hub_core_touch_instant_req_handler);
#endif

#ifdef CAPSENSOR_MEDIAN_FILTERING
#define CUR_SDM_BUF_LENGTH  3
static uint32_t cur_last_samp[CUR_SDM_BUF_LENGTH][CAP_CHNUM] = {0};

/***************************************************************************
 * @brief capsensor_median_filtering
 *
 * @param ptr :capsensor_sample_data.
 ***************************************************************************/
void capsensor_median_filter(struct capsensor_sample_data *sample, int num)
{
    int len = num;
    uint32_t sdm_temp_samp[CUR_SDM_BUF_LENGTH][CAP_CHNUM] = {0};

    for(int j = 0; j < len; j++) {
        for (int i=0;i<CUR_SDM_BUF_LENGTH-1;i++) {
            cur_last_samp[CUR_SDM_BUF_LENGTH-1-i][j]=cur_last_samp[CUR_SDM_BUF_LENGTH-2-i][j];
        }
        cur_last_samp[0][j] = sample[j].sdm;
    }

    for(int j = 0; j < len; j++) {
        for (int i=0;i<CUR_SDM_BUF_LENGTH;i++) {
            sdm_temp_samp[i][j] = cur_last_samp[i][j];
            // TRACE(0, "cur_last_samp[%d][%d]:%d", i, j, cur_last_samp[i][j]);
        }
    }

    for(int p = 0; p < len; p++) {//ch:0-3
        if(cur_last_samp[CUR_SDM_BUF_LENGTH-1][p] != 0) {
            for (int i = 0; i < CUR_SDM_BUF_LENGTH - 1; i++)
            {
               for (int j = 0; j < CUR_SDM_BUF_LENGTH - i - 1; j++)
               {
                   if (sdm_temp_samp[j][p] > sdm_temp_samp[j + 1][p])
                   {
                        uint32_t temp = sdm_temp_samp[j][p];
                        sdm_temp_samp[j][p] = sdm_temp_samp[j + 1][p];
                        sdm_temp_samp[j + 1][p] = temp;
                   }
               }
            }
            sample[p].sdm = sdm_temp_samp[CUR_SDM_BUF_LENGTH/2][p];
        }
    }
}
#endif

#ifdef CAPSENSOR_ALPHA_FILTERING

#define FRONT_SDM_BUF_LENGTH  3

void capsensor_alpha_filter(struct capsensor_sample_data *sample, int num)
{
    int len = num;
    static uint8_t count_int[CAP_CHNUM] = {0};
	static uint32_t front_sdm_data[FRONT_SDM_BUF_LENGTH][CAP_CHNUM] = {0};

    for(int j = 0; j < len; j++) {
        for (int i=0;i<FRONT_SDM_BUF_LENGTH-1;i++) {
            front_sdm_data[FRONT_SDM_BUF_LENGTH-1-i][j]=front_sdm_data[FRONT_SDM_BUF_LENGTH-i-2][j];
        }
        front_sdm_data[0][j] = sample[j].sdm;

        if(count_int[j] <= FRONT_SDM_BUF_LENGTH-1) {
            count_int[j]++;
        }

        if(count_int[j] == 2) {
            front_sdm_data[1][j] = (front_sdm_data[1][j]+front_sdm_data[0][j])/2;
            sample[j].sdm = front_sdm_data[1][j];
        }
    }

    for(int j = 0; j < len; j++) {
        if(count_int[j] > FRONT_SDM_BUF_LENGTH-1) {
            front_sdm_data[0][j] = (front_sdm_data[2][j]+front_sdm_data[1][j]+front_sdm_data[0][j])/3;
            sample[j].sdm = front_sdm_data[0][j];
        }
    }
}
#endif

/***************************************************************************
 * @brief cap sensor interrupt handler
 *
 ***************************************************************************/
void cap_sensor_irq_handler(void)
{
#if (CHIP_CAPSENSOR_VER < 1)
    hal_capsensor_irq_disable();
#endif
    read_capsensor_fifo(capsense_sample_buff);

    osSignalSet(capsensor_thread_id, 0x0001);
}

/***************************************************************************
 * @brief cap sensor interrupt init function
 *
 ***************************************************************************/
void  cap_sensor_irqinit(void)
{
#if defined(CHIP_SUBSYS_SENS) && (CHIP_CAPSENSOR_VER < 1) || (CHIP_CAPSENSOR_VER == 3) || (CHIP_CAPSENSOR_VER == 4)
    NVIC_SetVector(CAP_SENSOR_IRQn, (uint32_t)cap_sensor_irq_handler);
    NVIC_SetPriority(CAP_SENSOR_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(CAP_SENSOR_IRQn);
    NVIC_EnableIRQ(CAP_SENSOR_IRQn);
#if (CHIP_CAPSENSOR_VER < 1)
    hal_capsensor_irq_enable();
#endif
#elif (CHIP_CAPSENSOR_VER < 1)
    NVIC_SetVector(SENS_IRQn, (uint32_t)cap_sensor_irq_handler);
    NVIC_SetPriority(SENS_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(SENS_IRQn);
    NVIC_EnableIRQ(SENS_IRQn);
    hal_capsensor_irq_enable();
#endif
}

#ifdef CAPSENSOR_READ_DATA_POLLING
void capsensor_send_signal(void)
{
    osSignalSet(capsensor_thread_id, 0x0002);
}

void capsensor_reset_all_reg(void)
{
    capsensor_close();
    capsensor_open();
    capsensor_baseline_read(CAP_CHNUM);
    osDelay(100);
}
#endif

/***************************************************************************
 * @brief cap sensor process thread
 *
 * @param arg
 ***************************************************************************/
static void capsensor_thread(const void *arg)
{
#if defined(CAPSENSOR_TOUCH) || defined(CAPSENSOR_WEAR)
    uint32_t chan_data_sum[CAP_CHNUM] = {0};
#endif

#ifdef CAPSENSOR_SPP_SERVER
    static uint8_t touch_event = 0;
    int16_t touch_pos[7] = {0};
#endif

#if defined(CAPSENSOR_TOUCH)
    uint64_t timer_start[CAP_USED_TOUCH_NUM] = {0};
    uint64_t timer_stop[CAP_USED_TOUCH_NUM]  = {0};
#endif

    capsensor_drv_start();       /* init cap sensor regesiter. */

#ifndef CAPSENSOR_READ_DATA_POLLING
    cap_sensor_irqinit();       /* init cap sensor irq */
#endif

#if defined(CAPSENSOR_TOUCH)
    capsensor_touch_config_init();
#if defined(CAPSENSOR_SLIDE)
    capsensor_set_ms_per_cnt(CAP_KEY_SLIDE_MS_PER_CNT);
#else
    capsensor_set_ms_per_cnt(CAP_KEY_TOUCH_MS_PER_CNT);
#endif
#endif

#if defined(CAPSENSOR_WEAR)
    capsensor_wear_config_init();
#endif
    CAPSENSOR_TRACE(0, "sensor MCU freq=%d", hal_sys_timer_calc_cpu_freq(5,0));     /* trace sensor mcu freq */

    while(1) {
        osEvent evt;
        int rawdata_ready = 0;
#if defined(CAPSENSOR_TOUCH) || defined(CAPSENSOR_WEAR)
        CAP_KEY_STATUS status;
#endif
        app_sysfreq_req(APP_SYSFREQ_USER_CAPSENSOR, APP_SYSFREQ_32K);
#ifdef CAPSENSOR_READ_DATA_POLLING
        evt = osSignalWait(0, 0);
        if(evt.status == osEventSignal) {
            if (evt.value.signals == 0x02) {
                capsensor_reset_all_reg();
            }
        }
        osDelay(CAP_SAMP_FS); //cap sleep time
        capsensor_polling_rawdata(&rawdata_ready);
#else
        evt = osSignalWait(0, osWaitForever);
        if (evt.status == osEventSignal) {
            if (evt.value.signals == 0x01) {
                rawdata_ready = 1;
            }
        }
#endif
        app_sysfreq_req(APP_SYSFREQ_USER_CAPSENSOR, APP_SYSFREQ_26M);

        if(rawdata_ready) {
#ifdef CAPSENSOR_MEDIAN_FILTERING
                capsensor_median_filter(capsense_sample_buff, CAP_CHNUM);
#endif

#ifdef CAPSENSOR_ALPHA_FILTERING
                capsensor_alpha_filter(capsense_sample_buff, CAP_CHNUM);
#endif

#if defined(CAPSENSOR_TOUCH)
                status = capsensor_touch_process(capsense_sample_buff, CAP_CHNUM, chan_data_sum, (CAP_CHNUM * CAP_REPNUM), timer_start, timer_stop);
#ifdef CAPSENSOR_SPP_SERVER
                touch_event = status.event;
                capsensor_get_touch_pos(touch_pos);
#endif
                if (status.event != CAP_KEY_EVENT_NONE) {
#ifdef CHIP_SUBSYS_SENS
                    app_core_bridge_send_cmd(MCU_SENSOR_HUB_TASK_CMD_TOUCH_REQ_NO_RSP, (uint8_t *)&status, sizeof(CAP_KEY_STATUS));
#else
                    if (capsensor_click_event_cb != NULL) {
                        capsensor_click_event_cb(status.event);
                    }
#endif
                }
#endif
#if defined(CAPSENSOR_WEAR)
                static uint8_t wear_event = 0;
                status = capsensor_wear_process(capsense_sample_buff, CAP_CHNUM, chan_data_sum, (CAP_CHNUM * CAP_REPNUM));
                if (status.event != CAP_KEY_EVENT_NONE && wear_event != status.event) {
                    wear_event = status.event;
#ifdef CHIP_SUBSYS_SENS
                    app_core_bridge_send_cmd(MCU_SENSOR_HUB_TASK_CMD_TOUCH_REQ_NO_RSP, (uint8_t *)&status, sizeof(CAP_KEY_STATUS));
#else
                    if (capsensor_click_event_cb != NULL) {
                        capsensor_click_event_cb(status.event);
                    }
#endif
                }
#endif
#ifdef CAPSENSOR_SPP_SERVER
                capsensor_check_mem_data(capsense_sample_buff, (CAP_CHNUM * CAP_REPNUM), touch_event, touch_pos);
#endif
#ifdef CAPSENSOR_FAC_CALCULATE
                capsensor_factory_calculate(capsense_sample_buff, (CAP_CHNUM * CAP_REPNUM));
#endif
        } //rawdata_ready
    }
}

/***************************************************************************
 * @brief init snesorhub touch key functions.
 *
 ***************************************************************************/
void cap_sensor_core_thread_init(void)
{
    if (capsensor_thread_id == NULL) {
        capsensor_thread_id = osThreadCreate(osThread(capsensor_thread), NULL);
        CAPSENSOR_TRACE(1, "%s capsensor_thread successed !!!", __func__);
    }
}
