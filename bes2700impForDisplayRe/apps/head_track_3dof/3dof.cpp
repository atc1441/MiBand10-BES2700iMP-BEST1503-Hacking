/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
 * Application layer function of head tracking 3dof with lsm6dsl sensor
 ****************************************************************************/
#ifdef HEAD_TRACK_ENABLE
#include "hal_trace.h"
#include "hal_i2c.h"
#include "hal_timer.h"
#include "hal_spi.h"
#include "string.h"
#include "lsm6dsl_reg.h"
#include "stdlib.h"

#ifdef RTOS
#include "cmsis_os.h"
#ifdef KERNEL_RTX
#include "rt_Time.h"
#endif
#endif

#include "3dof.h"
#include "window_node.h"
#include "head_track_3dof.h"

#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND___) || defined(__VIRTUAL_SURROUND_STEREO__)
#include "app_bt_stream.h"
extern "C" int32_t audio_process_stereo_set_yaw(float yaw);
extern "C" int32_t audio_process_stereo_set_pitch(float pitch);
#endif

#define HEAD_TRACK_SIGNAL_ALGO          (0x01)
#define HEAD_TRACK_TIMER_INTERVAL       (10)
#define HEAD_TRACK_TIMER_FREQ           (100)
#define SLIDE_WINDOW_SIZE               (3)

static int head_track_inited = 0;
static int head_track_algo_inited = 0;
static int head_track_send_inited = 0;

static osMutexId head_track_sensor_mutex_id = NULL;
osMutexDef(head_track_sensor_mutex);

static osMutexId head_track_pose_mutex_id = NULL;
osMutexDef(head_track_pose_mutex);

static osSemaphoreId sensor_signal = NULL;
osSemaphoreDef(sensor_signal);

static struct WindowNode* head = NULL;

void head_track_window_link_init(void)
{
    for (int i = 0; i < SLIDE_WINDOW_SIZE; i++) {
        insertNode(&head, 0.0);
    }
}

void head_track_window_link_release(void)
{
    struct WindowNode* current = head;
    while (current != NULL) {
        WindowNode* temp = current;
        current = current->next;
        free(temp);
    }
}

static struct SENSOR_IMU preImu;
void imu_data_set(float ax, float ay, float az, float gx, float gy, float gz, unsigned int timeStamp)
{
    // Accelerometer, unit change, from mg to g, e.g 1000mg -> 1g
    preImu.a_x = ax / 1000.0;
    preImu.a_y = ay / 1000.0;
    preImu.a_z = az / 1000.0;

    // Gyroscope , unit change, from mdeg/s to deg/s, e.g. 1000mdeg/s -> 1deg/s
    preImu.g_x = gx / 1000.0;
    preImu.g_y = gy / 1000.0;
    preImu.g_z = gz / 1000.0;

    // timestamp unit us;  1s = 1000ms = 1000000us
    preImu.frameTimeStamp = timeStamp;
    preImu.rflag = 1;
}

void head_track_resouce_init(void)
{
    if(head_track_sensor_mutex_id == NULL) {
        head_track_sensor_mutex_id = osMutexCreate(osMutex(head_track_sensor_mutex));
        if (head_track_sensor_mutex_id == NULL) {
            TRACE(1, "Failed to Create head_track_sensor_mutex_id");
        }
    }

    if (head_track_pose_mutex_id == NULL) {
        head_track_pose_mutex_id = osMutexCreate(osMutex(head_track_pose_mutex));
        if (head_track_pose_mutex_id == NULL) {
            TRACE(1, "Failed to Create head_track_pose_mutex_id");
        }
    }

    if (sensor_signal == NULL) {
        sensor_signal = osSemaphoreCreate(osSemaphore(sensor_signal), 1);
        if (sensor_signal == NULL) {
            TRACE(1, "Failed to Create sensor_signal");
        }
    }

    if(head == NULL) {
        head_track_window_link_init();
        if(head == NULL) {
            TRACE(1, "Failed to Create head track window LinkList");
        }
    }else{
        /* release, then init again */
        head_track_window_link_release();
        head_track_window_link_init();
        if(head == NULL) {
            TRACE(1, "Failed to Create head track window LinkList");
        }
    }
}

#define HEAD_TRACK_THREAD_STACK_SIZE    1024
static void head_track_task_thread(void const* argument);
static osThreadId head_track_thread_id;
static osThreadDef(head_track_task_thread, osPriorityNormal, 1, HEAD_TRACK_THREAD_STACK_SIZE, "head_track_thread");

#define POSE_SIZE (sizeof(struct POSE_S))
#define SENSOR_SIZE (sizeof(struct SENSOR_IMU))
static struct POSE_S pose_s;
static struct SENSOR_IMU sensor;
static void head_track_task_thread(void const*argument)
{
    osEvent evt;
    uint32_t signals = 0;

    while (1) {
        evt = osSignalWait(0x0, osWaitForever);
        signals = evt.value.signals;

        if (evt.status == osEventSignal) {
            if (signals & HEAD_TRACK_SIGNAL_ALGO) {
                osMutexWait(head_track_sensor_mutex_id, osWaitForever);
                memcpy(&sensor, &preImu, SENSOR_SIZE);
                osSemaphoreRelease(sensor_signal);
                osMutexRelease(head_track_sensor_mutex_id);
            }
        }
    }
}

#define HEAD_TRACK_ALGO_THREAD_STACK_SIZE    4096
static void head_track_algo_task_thread(void const* argument);
static osThreadId head_track_algo_thread_id;
static osThreadDef(head_track_algo_task_thread, osPriorityNormal, 1, HEAD_TRACK_ALGO_THREAD_STACK_SIZE, "head_track_algo_thread");

static void head_track_algo_task_thread(void const*argument)
{
    struct POSE_S algoPose;
    struct SENSOR_IMU localSensor;

    while (1)
    {
        osSemaphoreWait(sensor_signal, osWaitForever);
        osMutexWait(head_track_sensor_mutex_id, osWaitForever);
        memcpy(&localSensor, &sensor, SENSOR_SIZE);
        sensor.rflag = 0;
        osMutexRelease(head_track_sensor_mutex_id);

        if(localSensor.rflag == 1) {
            head_track_3dof_algo_with_fix_param(&algoPose, &localSensor);
        }

        osMutexWait(head_track_pose_mutex_id, osWaitForever);
        memcpy(&pose_s, &algoPose, POSE_SIZE);
        osMutexRelease(head_track_pose_mutex_id);
    }
}

#define HEAD_TRACK_SEND_THREAD_STACK_SIZE    1024
static void head_track_send_task_thread(void const* argument);
static osThreadId head_track_send_thread_id;
static osThreadDef(head_track_send_task_thread, osPriorityNormal, 1, HEAD_TRACK_SEND_THREAD_STACK_SIZE, "head_track_send_thread");

static void head_track_send_task_thread(void const*argument)
{
    struct POSE_S send_pose;
    while (1)
    {
        osMutexWait(head_track_pose_mutex_id, osWaitForever);
        memcpy(&send_pose, &pose_s, POSE_SIZE);
        osMutexRelease(head_track_pose_mutex_id);
        /*
            set spatial audio with 3dof angle if necessary,
            the spatial audio APIs might be different,
            here is just a demo usage.

            also, the pose data can be sent to cell-phone or whatever, according to customer's wishes
        */
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND___) || defined(__VIRTUAL_SURROUND_STEREO__)
        audio_process_stereo_set_yaw((float)pose_s.yaw);
        audio_process_stereo_set_pitch((float)pose_s.pitch);
#endif
        osDelay(10);
    }
}

static int head_track_task_init(void)
{
    TRACE(0, "%s(true),%d \n\r", __FUNCTION__, __LINE__);

    if (!head_track_inited) {
        head_track_thread_id = osThreadCreate(osThread(head_track_task_thread), NULL);
        if (head_track_thread_id == NULL) {
            TRACE(0, "create head_track_thread failed");
            return -1;
        }
        head_track_inited = 1;
    }
    return 0;
}

static int head_track_algo_task_init(void)
{
    TRACE(0, "%s(true),%d \n\r", __FUNCTION__, __LINE__);

    if(!head_track_algo_inited)
    {
        head_track_algo_thread_id = osThreadCreate(osThread(head_track_algo_task_thread), NULL);
        if (head_track_algo_thread_id == NULL) {
            TRACE(0, "create head_track_algo_thread failed");
            return -1;
        }

        head_track_algo_inited = 1;
    }
    return 0;
}

static int head_track_send_task_init(void)
{
    TRACE(0, "%s(true),%d \n\r", __FUNCTION__, __LINE__);

    if(!head_track_send_inited)
    {
        head_track_send_thread_id = osThreadCreate(osThread(head_track_send_task_thread), NULL);
        if (head_track_send_thread_id == NULL) {
            TRACE(0, "create head_track_send_thread failed");
            return -1;
        }

        head_track_send_inited = 1;
    }
    return 0;
}

void imu_timer_handler(void const *param);
osTimerDef (imu_timer, imu_timer_handler);
osTimerId imu_timer_id = NULL;

void imu_timer_handler(void const *param)
{
    osSignalSet(head_track_thread_id, HEAD_TRACK_SIGNAL_ALGO);
}

void head_angle_reset(void)
{
    head_track_3dof_reset();
}

void start_head_track_timer(void)
{
    osStatus_t status = osTimerStart(imu_timer_id, HEAD_TRACK_TIMER_INTERVAL);
    if (status != osOK) {
        TRACE(0, "start head_track algo timer failed!");
    }
}

void pause_head_track_timer(void)
{
    osStatus_t status = osTimerStop(imu_timer_id);
    if (status != osOK) {
        TRACE(0, "pause head_track algo timer failed!");
    }
}

void imusensor_init(void)
{
    /* Init head track task  */
    head_track_resouce_init();

    /* Init head track task  */
    head_track_task_init();
    head_track_algo_task_init();
    head_track_send_task_init();

    /* Create imu timer*/
    imu_timer_id = osTimerCreate(osTimer(imu_timer), osTimerPeriodic, NULL);

    /* start head track timer to release signals */
    if (imu_timer_id != NULL) {
        start_head_track_timer();
    }

    return;
}

void imusensor_reset(void)
{
    head_track_3dof_reset();
}

void head_track_algo_start(void)
{
    pause_head_track_timer();
    imusensor_reset();
    start_head_track_timer();
}

void head_track_algo_pause(void)
{
    imusensor_reset();
    pause_head_track_timer();
}

#endif