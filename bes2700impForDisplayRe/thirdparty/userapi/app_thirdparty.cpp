#include "cmsis_os.h"
#include "cmsis.h"
#include "list.h"
#include "hal_trace.h"
#include <string.h>
#include "app_thirdparty.h"
#include "app_ai_if.h"
#include "ai_voice_dbg.h"

/// used for trace log control for AI voice module
#define MODULE_TRACE_LIMIT                      TR_LEVEL_INFO


APP_THIRDPARTY_HANDLE_TAB_T app_thirdparty_handle_table[AI_SPEC_COUNT];

void app_thirdparty_callback_init(THIRDPARTY_CALLBACK_TYPE type, APP_THIRDPARTY_CUSTOM_CB_T cb, uint8_t ai)
{
    LOG_I("%s type:%d, ai:%d", __func__, type, ai);
    ASSERT((ai < AI_SPEC_COUNT) && (ai >= AI_SPEC_INIT), "%s invalid AI(%d)", __func__, ai);

    switch (type)
    {
        case THIRDPARTY_WAKE_UP_CALLBACK:
            app_thirdparty_handle_table[ai]._app_thirdparty_wake_up_callback = cb;
        break;
        default:
        break;
    }
}

uint32_t app_thirdparty_callback_handle(uint8_t ai, THIRDPARTY_CALLBACK_TYPE type, void* param1, uint32_t param2)
{
    uint32_t ret = 0xFFFFFFFF;
    //TRACE(2, "%s type %d", __func__, type);
    ASSERT((ai < AI_SPEC_COUNT) && (ai >= AI_SPEC_INIT), "%s invalid AI(%d)", __func__, ai);

    switch (type)
    {
        case THIRDPARTY_WAKE_UP_CALLBACK:
            if (app_thirdparty_handle_table[ai]._app_thirdparty_wake_up_callback)
            {
                ret = app_thirdparty_handle_table[ai]._app_thirdparty_wake_up_callback(param1, param2);
            }
        break;
        default:
        break;
    }

    return ret;
}

int app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_ID funcId, THIRDPARTY_EVENT_TYPE event_type, uint8_t ai)
{
    LOG_D("%s funcId:%d event_type:0x%x ai:%d", __func__, funcId, event_type, ai);
    ASSERT((ai < AI_SPEC_COUNT) && (ai >= AI_SPEC_INIT), "%s invalid AI(%d)", __func__, ai);
    const APP_THIRDPARTY_HANDLE *dest_handle;

    if (app_thirdparty_handle_table[ai].thirdparty_func_id == funcId)
    {
        for (uint32_t index = 0;
            index < app_thirdparty_handle_table[ai].thirdparty_handler_cnt;
            index++)
        {
            dest_handle = &app_thirdparty_handle_table[ai].thirdparty_handler_tab[index];
            if (dest_handle->thirdparty_sign.event == event_type)
            {
                LOG_V("find index=%d",index);
                if (dest_handle->function)
                {
                    return dest_handle->function(dest_handle->status, dest_handle->param);
                }
            }
        }
    }
    else
    {
        TRACE(1, "func ID not match:%d|%d", app_thirdparty_handle_table[ai].thirdparty_func_id, funcId);
    }

    return 0;
}


void app_thirdparty_init(void)
{
    memset(&app_thirdparty_handle_table, 0, sizeof(app_thirdparty_handle_table));
#ifdef __ALEXA_WWE
    app_thirdparty_handle_table[AI_SPEC_AMA].thirdparty_handler_tab = THIRDPARTY_GET_HANDLER_TAB(ALEXA_WWE_LIB_NAME);
    app_thirdparty_handle_table[AI_SPEC_AMA].thirdparty_handler_cnt = THIRDPARTY_GET_HANDLER_TAB_SIZE(ALEXA_WWE_LIB_NAME);
    app_thirdparty_handle_table[AI_SPEC_AMA].thirdparty_func_id = app_thirdparty_handle_table[AI_SPEC_AMA].thirdparty_handler_tab[0].thirdparty_sign.func_id;
    app_thirdparty_handle_table[AI_SPEC_AMA].thirdparty_lib_id  = app_thirdparty_handle_table[AI_SPEC_AMA].thirdparty_handler_tab[0].thirdparty_sign.lib_id;
    LOG_I("%s ai:%d, cnt:%d id:%d", __func__, AI_SPEC_AMA,
          app_thirdparty_handle_table[AI_SPEC_AMA].thirdparty_handler_cnt,
          app_thirdparty_handle_table[AI_SPEC_AMA].thirdparty_lib_id);
#endif
#if defined(__BIXBY)
    app_thirdparty_handle_table[AI_SPEC_BIXBY].thirdparty_handler_tab = THIRDPARTY_GET_HANDLER_TAB(BIXBY_NAME);
    app_thirdparty_handle_table[AI_SPEC_BIXBY].thirdparty_handler_cnt = THIRDPARTY_GET_HANDLER_TAB_SIZE(BIXBY_NAME);
    app_thirdparty_handle_table[AI_SPEC_BIXBY].thirdparty_func_id = app_thirdparty_handle_table[AI_SPEC_BIXBY].thirdparty_handler_tab[0].thirdparty_sign.func_id;
    app_thirdparty_handle_table[AI_SPEC_BIXBY].thirdparty_lib_id  = app_thirdparty_handle_table[AI_SPEC_BIXBY].thirdparty_handler_tab[0].thirdparty_sign.lib_id;
    LOG_I("%s ai:%d, cnt:%d id:%d", __func__, AI_SPEC_BIXBY,
          app_thirdparty_handle_table[AI_SPEC_BIXBY].thirdparty_handler_cnt,
          app_thirdparty_handle_table[AI_SPEC_BIXBY].thirdparty_lib_id);
#endif

#ifdef __SMART_VOICE_KWS_AQE__
    app_thirdparty_handle_table[AI_SPEC_BES].thirdparty_handler_tab = THIRDPARTY_GET_HANDLER_TAB(BES_AQE_NAME);
    app_thirdparty_handle_table[AI_SPEC_BES].thirdparty_handler_cnt = THIRDPARTY_GET_HANDLER_TAB_SIZE(BES_AQE_NAME);
    app_thirdparty_handle_table[AI_SPEC_BES].thirdparty_func_id = app_thirdparty_handle_table[AI_SPEC_BES].thirdparty_handler_tab[0].thirdparty_sign.func_id;
    app_thirdparty_handle_table[AI_SPEC_BES].thirdparty_lib_id  = app_thirdparty_handle_table[AI_SPEC_BES].thirdparty_handler_tab[0].thirdparty_sign.lib_id;
#endif

#if 0
#if defined(AQE_KWS_ALEXA)
    app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_handler_tab = THIRDPARTY_GET_HANDLER_TAB(KWS_ALEXA_LIB_NAME);
    app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_handler_cnt = THIRDPARTY_GET_HANDLER_TAB_SIZE(KWS_ALEXA_LIB_NAME);
#elif defined(__CYBERON)
    app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_handler_tab = THIRDPARTY_GET_HANDLER_TAB(CYBERON_LIB_NAME);
    app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_handler_cnt = THIRDPARTY_GET_HANDLER_TAB_SIZE(CYBERON_LIB_NAME);
#elif defined(__KNOWLES)
    app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_handler_tab = THIRDPARTY_GET_HANDLER_TAB(KNOWLES_LIB_NAME);
    app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_handler_cnt = THIRDPARTY_GET_HANDLER_TAB_SIZE(KNOWLES_LIB_NAME);
#elif defined(ANC_NOISE_TRACKER)
    app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_handler_tab = THIRDPARTY_GET_HANDLER_TAB(NOISE_TRACKER_LIB_NAME);
    app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_handler_cnt = THIRDPARTY_GET_HANDLER_TAB_SIZE(NOISE_TRACKER_LIB_NAME);
#elif defined(_VOICESPOT_)
    app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_handler_tab = THIRDPARTY_GET_HANDLER_TAB(VOICESPOT_LIB_NAME);
    app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_handler_cnt = THIRDPARTY_GET_HANDLER_TAB_SIZE(VOICESPOT_LIB_NAME);
#elif defined(__SNDP)
    app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_handler_tab = THIRDPARTY_GET_HANDLER_TAB(SNDP_NAME);
    app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_handler_cnt = THIRDPARTY_GET_HANDLER_TAB_SIZE(SNDP_NAME);
#else
    app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_handler_tab = THIRDPARTY_GET_HANDLER_TAB(DEMO_LIB_NAME);
    app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_handler_cnt = THIRDPARTY_GET_HANDLER_TAB_SIZE(DEMO_LIB_NAME);
    app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_func_id = app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_handler_tab[0].thirdparty_sign.func_id;
    app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_lib_id  = app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_handler_tab[0].thirdparty_sign.lib_id;

    LOG_I("%s ai:%d, cnt:%d id:%d", __func__, AI_SPEC_INIT,
          app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_handler_cnt,
          app_thirdparty_handle_table[AI_SPEC_INIT].thirdparty_lib_id);
#endif
#endif
}

