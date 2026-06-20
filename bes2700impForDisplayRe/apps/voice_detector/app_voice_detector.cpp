#include <stdio.h>
#include <assert.h>
#include "cmsis_os.h"
#include "tgt_hardware.h"
#include "audioflinger.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "app_thread.h"
#include "voice_detector.h"
#include "app_voice_detector.h"

#define APP_VD_DEBUG

#ifdef APP_VD_DEBUG
#define APP_VD_LOG(str, ...) TR_DEBUG(TR_MOD(AUD), str, ##__VA_ARGS__)
#else
#define APP_VD_LOG(...) do{}while(0)
#endif
#define APP_VD_ERR(str, ...) TR_ERROR(TR_MOD(AUD), str, ##__VA_ARGS__)

osMutexId vd_mutex_id = NULL;
osMutexDef(vd_mutex);
bool vad_mic_has_open[VOICE_DETECTOR_QTY] = {false};
bool enable_vad[VOICE_DETECTOR_QTY][VOICE_DET_USER_NUM] = {true};

static int cmd_arr_evt_vad_start[] = {
    VOICE_DET_CMD_AUD_CAP_STOP,
    VOICE_DET_CMD_AUD_CAP_CLOSE,
    VOICE_DET_CMD_VAD_OPEN,
    VOICE_DET_CMD_VAD_START,
    VOICE_DET_CMD_SYS_CLK_32K,
};
#if 0
static int cmd_arr_evt_cap_start[] = {
    VOICE_DET_CMD_VAD_STOP,
    VOICE_DET_CMD_VAD_CLOSE,
    VOICE_DET_CMD_AUD_CAP_OPEN,
    VOICE_DET_CMD_AUD_CAP_START,
};
#else
static int cmd_arr_evt_cap_start[] = {
    VOICE_DET_CMD_AUD_CAP_OPEN,
    VOICE_DET_CMD_AUD_CAP_START,
    VOICE_DET_CMD_VAD_STOP,
    VOICE_DET_CMD_VAD_CLOSE,
};
#endif
static int cmd_arr_evt_close[] = {
    VOICE_DET_CMD_AUD_CAP_STOP,
    VOICE_DET_CMD_AUD_CAP_CLOSE,
    VOICE_DET_CMD_VAD_STOP,
    VOICE_DET_CMD_VAD_CLOSE,
    VOICE_DET_CMD_EXIT,
};
#if 1
static int cmd_arr_evt_cap_close[] = {
    VOICE_DET_CMD_AUD_CAP_STOP,
    VOICE_DET_CMD_AUD_CAP_CLOSE,
    VOICE_DET_CMD_EXIT,
};
#endif
static int cmd_arr_evt_vad_close[] = {
    VOICE_DET_CMD_VAD_STOP,
    VOICE_DET_CMD_VAD_CLOSE,
    VOICE_DET_CMD_EXIT,
};

static int app_voice_detector_process(APP_MESSAGE_BODY *msg_body)
{
    enum voice_detector_id id   = (enum voice_detector_id)msg_body->message_id;
    enum voice_detector_evt evt = (enum voice_detector_evt)msg_body->message_ptr;
    int ret = 0, num = 0, *cmds = NULL;

    voice_detector_enhance_perform(id);// set sys clock to 104M or 208M
    APP_VD_LOG("%s evt %d status %d", __func__, evt, voice_detector_query_status(id));

    osMutexWait(vd_mutex_id, osWaitForever);
    switch(evt) {
    case VOICE_DET_EVT_VAD_START:
        if (voice_detector_query_status(id) == VOICE_DET_STATE_VAD_CLOSE || 
            voice_detector_query_status(id) == VOICE_DET_STATE_AUD_CAP_START) {
            cmds = &cmd_arr_evt_vad_start[0];
            num  = ARRAY_SIZE(cmd_arr_evt_vad_start);
        } else if (voice_detector_query_status(id) != VOICE_DET_STATE_VAD_START){
            cmds = &cmd_arr_evt_vad_start[2];
            num  = ARRAY_SIZE(cmd_arr_evt_vad_start) - 2;
        }
        break;
    case VOICE_DET_EVT_AUD_CAP_START:
        if (voice_detector_query_status(id) == VOICE_DET_STATE_VAD_START)
        {
            cmds = cmd_arr_evt_cap_start;
            num  = ARRAY_SIZE(cmd_arr_evt_cap_start);
        }
        else if (voice_detector_query_status(id) != VOICE_DET_STATE_AUD_CAP_START ||
            voice_detector_query_status(id) != VOICE_DET_STATE_AUD_CAP_CLOSE)
        {
            cmds = cmd_arr_evt_cap_start;
            num  = 2;
        }
        break;
    case VOICE_DET_EVT_CLOSE:
        if (voice_detector_query_status(id) == VOICE_DET_STATE_VAD_CLOSE) {
            cmds = &cmd_arr_evt_cap_close[0];
            num  = ARRAY_SIZE(cmd_arr_evt_cap_close);
        } else {
            cmds = &cmd_arr_evt_vad_close[0];
            num  = ARRAY_SIZE(cmd_arr_evt_vad_close);
        }
        break;
    default:
        cmds = cmd_arr_evt_close;
        num  = ARRAY_SIZE(cmd_arr_evt_close);
        break;
    }
    ret = voice_detector_send_cmd_array(id, cmds, num);
    if (ret) {
        APP_VD_ERR("%s, send cmd error %d", __func__, ret);
    }
    ret = voice_detector_run(id, VOICE_DET_MODE_EXEC_CMD);
    if (ret) {
        APP_VD_ERR("%s, run cmd error %d", __func__, ret);
    }
    if (evt == VOICE_DET_EVT_CLOSE) {
        voice_detector_close(id);
    }
    osMutexRelease(vd_mutex_id);
    return ret;
}

static void voice_detector_send_msg(uint32_t id, uint32_t evt)
{
    APP_MESSAGE_BLOCK msg;

    msg.mod_id = APP_MODULE_VOICE_DETECTOR;
#if defined(USE_BASIC_THREADS)
    msg.mod_level = APP_MOD_LEVEL_1;
#endif 
    msg.msg_body.message_id  = id;
    msg.msg_body.message_ptr = evt;

    app_mailbox_put(&msg);
}

static void voice_detector_wakeup_system(int state, void *param)
{
    enum voice_detector_id id = VOICE_DETECTOR_ID_0;

    if (voice_detector_query_status(id) == VOICE_DET_STATE_VAD_START) {
        app_voice_detector_send_event(id, VOICE_DET_EVT_AUD_CAP_START);
    }

    APP_VD_LOG("%s, state=%d", __func__, state);
    // APP_VD_LOG("cpu freq=%d", hal_sys_timer_calc_cpu_freq(5,0));
}

static void voice_not_detector_wakeup_system(int state, void *param)
{
    enum voice_detector_id id = VOICE_DETECTOR_ID_0;

    if (voice_detector_query_status(id) == VOICE_DET_STATE_VAD_START) {
        app_voice_detector_send_event(id, VOICE_DET_EVT_VAD_START);
    }

    APP_VD_LOG("%s, state=%d", __func__, state);
    // APP_VD_LOG("cpu freq=%d", hal_sys_timer_calc_cpu_freq(5,0));
}

void app_voice_detector_init(void)
{
    APP_VD_LOG("%s", __func__);
    static bool voice_detector_inited = false;

    if (!voice_detector_inited)
    {
        voice_detector_inited = true;
        if(vd_mutex_id == NULL){
            vd_mutex_id = osMutexCreate((osMutex(vd_mutex)));
            app_set_threadhandle(APP_MODULE_VOICE_DETECTOR, app_voice_detector_process);
        }

        for (uint8_t i=0; i<VOICE_DETECTOR_QTY; i++)
        {
            vad_mic_has_open[i] = false;
            for (uint8_t j=0; j<VOICE_DET_USER_NUM; j++)
            {
                enable_vad[i][j] = true;
            }
        }
    }
}

int app_voice_detector_open(enum voice_detector_id id, enum AUD_VAD_TYPE_T vad_type)
{
    int r;

    APP_VD_LOG("%s", __func__);
    if (vad_mic_has_open[id])
    {
        APP_VD_LOG("%s id %d has open", __func__, id);
        return -1;
    }
    vad_mic_has_open[id] = true;

    if (!vd_mutex_id) {
        APP_VD_LOG("%s, mutex is null", __func__);
        return -1;
    }
    osMutexWait(vd_mutex_id, osWaitForever);
    r = voice_detector_open(id, vad_type);
    if (!r) {
        voice_detector_setup_callback(id, VOICE_DET_FIND_APP,
                        voice_detector_wakeup_system, NULL);
        voice_detector_setup_callback(id, VOICE_DET_NOT_FIND_APP,
                        voice_not_detector_wakeup_system, NULL);
    }
    osMutexRelease(vd_mutex_id);

    return r;
}

int app_voice_detector_setup_vad(enum voice_detector_id id,
                                struct AUD_VAD_CONFIG_T *conf)
{
    int r;

    APP_VD_LOG("%s", __func__);

    if (!vd_mutex_id) {
        APP_VD_LOG("%s, mutex is null", __func__);
        return -1;
    }
    osMutexWait(vd_mutex_id, osWaitForever);
    r = voice_detector_setup_vad(id, conf);
    osMutexRelease(vd_mutex_id);
    return r;
}

int app_voice_detector_setup_stream(enum voice_detector_id id,
                                    enum AUD_STREAM_T stream_id,
                                    struct AF_STREAM_CONFIG_T *stream)
{
    int r;

    APP_VD_LOG("%s", __func__);

    if (!vd_mutex_id) {
        APP_VD_LOG("%s, mutex is null", __func__);
        return -1;
    }
    osMutexWait(vd_mutex_id, osWaitForever);
    r = voice_detector_setup_stream(id, stream_id, stream);
    osMutexRelease(vd_mutex_id);
    return r;
}

int app_voice_detector_setup_callback(enum voice_detector_id id,
                                      enum voice_detector_cb_id func_id,
                                      voice_detector_cb_t func,
                                      void *param)
{
    int r;

    APP_VD_LOG("%s", __func__);

    if (!vd_mutex_id) {
        APP_VD_LOG("%s, mutex is null", __func__);
        return -1;
    }
    osMutexWait(vd_mutex_id, osWaitForever);
    r = voice_detector_setup_callback(id, func_id, func, param);
    osMutexRelease(vd_mutex_id);
    return r;
}

int app_voice_detector_send_event(enum voice_detector_id id,
                                enum voice_detector_evt evt)
{
    APP_VD_LOG("%s, id=%d, evt=%d", __func__, id, evt);

    voice_detector_send_msg(id, evt);
    return 0;
}

void app_voice_detector_close(enum voice_detector_id id)
{
    APP_VD_LOG("%s", __func__);
    if (!vad_mic_has_open[id])
    {
        APP_VD_LOG("%s id %d don't open", __func__, id);
        return;
    }
    vad_mic_has_open[id] = false;

    voice_detector_send_msg(id, VOICE_DET_EVT_CLOSE);
}

void app_voice_detector_get_vad_data_info(enum voice_detector_id id,
                              struct CODEC_VAD_BUF_INFO_T* vad_buf_info)
{
    voice_detector_get_vad_data_info(id, vad_buf_info);
}

void app_voice_detector_capture_enable_vad(enum voice_detector_id id, enum voice_detector_user user)
{
    APP_VD_LOG("%s user %d", __func__, user);

    enable_vad[id][user] = true;

    if (vad_mic_has_open[id])
    {
        if (app_voice_detector_capture_vad_is_enable(id))
        {
            app_voice_detector_send_event(id, VOICE_DET_EVT_VAD_START);
        }
        else
        {
            app_voice_detector_send_event(id, VOICE_DET_EVT_AUD_CAP_START);
        }
    }
}

void app_voice_detector_capture_disable_vad(enum voice_detector_id id, enum voice_detector_user user)
{
    APP_VD_LOG("%s user %d", __func__, user);

    enable_vad[id][user] = false;

    if (vad_mic_has_open[id])
    {
        voice_detector_send_msg(id, VOICE_DET_EVT_AUD_CAP_START);
    }
}

bool app_voice_detector_capture_vad_is_enable(enum voice_detector_id id)
{
    //APP_VD_LOG("%s", __func__);
    for (uint8_t user=0; user<VOICE_DET_USER_NUM; user++)
    {
        if (!enable_vad[id][user])
        {
            return false;
        }
    }

    return true;
}

