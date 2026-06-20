#include "cmsis_os.h"
#include "spp_api.h"
#include "hal_trace.h"
#include "app_ai_if.h"
#include "app_ai_tws.h"
#include "app_ai_manager_api.h"
#include "app_ai_if_thirdparty.h"
#include "app_thirdparty.h"
#include "app_bt_media_manager.h"

#ifdef __AI_VOICE__
#include "ai_manager.h"
#include "ai_control.h"
#include "ai_thread.h"
#include "app_ai_voice.h"
#endif

uint8_t app_ai_if_thirdparty_mempool_buf[APP_AI_IF_THIRDPARTY_MEMPOOL_BUFFER_SIZE];
THIRDPARTY_AI_MEMPOOL_BUFFER_T app_ai_if_thirdparty_mempool_buf_t;

POSSIBLY_UNUSED void app_ai_if_thirdparty_mempool_init(void)
{
    if (false == app_ai_if_thirdparty_mempool_buf_t.initialed) {
#if (APP_AI_IF_THIRDPARTY_MEMPOOL_BUFFER_SIZE > 0)
        memset((uint8_t *)app_ai_if_thirdparty_mempool_buf, 0, APP_AI_IF_THIRDPARTY_MEMPOOL_BUFFER_SIZE);
#endif
        app_ai_if_thirdparty_mempool_buf_t.buff = app_ai_if_thirdparty_mempool_buf;
        app_ai_if_thirdparty_mempool_buf_t.buff_size_total = APP_AI_IF_THIRDPARTY_MEMPOOL_BUFFER_SIZE;
        app_ai_if_thirdparty_mempool_buf_t.buff_size_used = 0;
        app_ai_if_thirdparty_mempool_buf_t.buff_size_free = APP_AI_IF_THIRDPARTY_MEMPOOL_BUFFER_SIZE;

        TRACE(3, "%s buf %p size %d", __func__, app_ai_if_thirdparty_mempool_buf, APP_AI_IF_THIRDPARTY_MEMPOOL_BUFFER_SIZE);
        app_ai_if_thirdparty_mempool_buf_t.initialed  = true;
    }
}

void app_ai_if_thirdparty_mempool_deinit(void)
{
    TRACE(2, "%s size %d", __func__, app_ai_if_thirdparty_mempool_buf_t.buff_size_total);

    if (app_ai_if_thirdparty_mempool_buf_t.initialed == true) {
        memset((uint8_t *)app_ai_if_thirdparty_mempool_buf_t.buff, 0, app_ai_if_thirdparty_mempool_buf_t.buff_size_total);

        app_ai_if_thirdparty_mempool_buf_t.buff_size_used = 0;
        app_ai_if_thirdparty_mempool_buf_t.buff_size_free = app_ai_if_thirdparty_mempool_buf_t.buff_size_total;
        app_ai_if_thirdparty_mempool_buf_t.initialed  = false;
    }
}

void app_ai_if_thirdparty_mempool_get_buff(uint8_t **buff, uint32_t size)
{
    if (false == app_ai_if_thirdparty_mempool_buf_t.initialed) {
        TRACE(1,"%s not initialed ! direct return",__func__);
        return ;
    }
    uint32_t buff_size_free;

    buff_size_free = app_ai_if_thirdparty_mempool_buf_t.buff_size_free;

    if (size % 4) {
        size = size + (4 - size % 4);
    }

    TRACE(3,"%s free %d to allocate %d", __func__, buff_size_free, size);

    ASSERT(size <= buff_size_free, "[%s] size = %d > free size = %d", __func__, size, buff_size_free);

    *buff = app_ai_if_thirdparty_mempool_buf_t.buff + app_ai_if_thirdparty_mempool_buf_t.buff_size_used;

    app_ai_if_thirdparty_mempool_buf_t.buff_size_used += size;
    app_ai_if_thirdparty_mempool_buf_t.buff_size_free -= size;

    TRACE(3,"thirdparty allocate %d, now used %d left %d", size,
                    app_ai_if_thirdparty_mempool_buf_t.buff_size_used,
                    app_ai_if_thirdparty_mempool_buf_t.buff_size_free);
}

int app_ai_if_thirdparty_event_handle(THIRDPARTY_FUNC_ID funcId, THIRDPARTY_EVENT_TYPE event_type, uint8_t ai)
{
    int ret = 0;
#ifdef __THIRDPARTY
    ret = app_thirdparty_specific_lib_event_handle(funcId, event_type, ai);
#endif
    return ret;
}

void app_ai_if_thirdparty_init(void)
{
    TRACE(0, "%s", __func__);
#ifdef __THIRDPARTY
    app_ai_if_thirdparty_mempool_init();

#ifdef __SMART_VOICE_KWS_AQE__
    app_thirdparty_callback_init(THIRDPARTY_WAKE_UP_CALLBACK, app_ai_voice_thirdparty_wake_up_callback, AI_SPEC_BES);
    app_ai_if_thirdparty_event_handle(THIRDPARTY_FUNC_KWS, THIRDPARTY_INIT, AI_SPEC_BES);
#endif
#ifdef __ALEXA_WWE
    app_thirdparty_callback_init(THIRDPARTY_WAKE_UP_CALLBACK, app_ai_voice_thirdparty_wake_up_callback, AI_SPEC_AMA);
    app_ai_if_thirdparty_event_handle(THIRDPARTY_FUNC_KWS, THIRDPARTY_INIT, AI_SPEC_AMA);
#endif
#ifdef __BIXBY
    app_thirdparty_callback_init(THIRDPARTY_WAKE_UP_CALLBACK, app_ai_voice_thirdparty_wake_up_callback, AI_SPEC_BIXBY);
    app_ai_if_thirdparty_event_handle(THIRDPARTY_FUNC_KWS, THIRDPARTY_INIT, AI_SPEC_BIXBY);
#endif
#endif
}

#ifdef __AI_VOICE__

static void app_test_start_hotword_supervising(void)
{
    app_ai_set_use_thirdparty(false, AI_SPEC_AMA);
    app_ai_set_setup_complete(true, AI_SPEC_AMA);
    ai_set_can_wake_up(true, AI_SPEC_AMA);
    app_ai_set_use_thirdparty(true, AI_SPEC_AMA);
}

static void app_test_stop_hotword_supervising(void)
{
    app_ai_set_use_thirdparty(false, AI_SPEC_INIT);
    app_ai_set_setup_complete(false, AI_SPEC_AMA);
}

void app_test_toggle_hotword_supervising(void)
{
    static bool isHotwordSupervisingOn = false;
    if (!isHotwordSupervisingOn)    
    {    
        app_test_start_hotword_supervising();
    }
    else
    {
        app_test_stop_hotword_supervising();
    }
    isHotwordSupervisingOn = !isHotwordSupervisingOn;
}
#endif

