#include "cmsis_os.h"
#include "bluetooth_bt_api.h"
#include "spp_api.h"
#include "hal_trace.h"
#include "app_ai_if.h"
#include "app_ai_tws.h"
#include "bt_drv_interface.h"
#include "app_ai_if_thirdparty.h"
#include "hal_location.h"

#ifdef __IAG_BLE_INCLUDE__
#include "bluetooth_ble_api.h"
#endif

#ifdef __AI_VOICE__
#include "ai_manager.h"
#include "ai_control.h"
#include "ai_thread.h"
#include "app_ai_voice.h"
#include "app_ai_ble.h"
#endif

#if defined(IBRT)
#include "app_tws_ibrt.h"
#include "app_tws_ctrl_thread.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_tws_ctrl_thread.h"
#include "app_ibrt_internal.h"
#include "app_ibrt_customif_cmd.h"
#include "earbud_ux_api.h"
#if defined(IBRT_UI)
#include "app_tws_ibrt_conn_api.h"
#endif
#endif

#ifdef BISTO_ENABLED
#include "gsound_custom.h"
#include "gsound_custom_tws.h"
#include "gsound_custom_bt.h"
#endif

#ifdef __BIXBY_VOICE__
#include "app_bixby_thirdparty_if.h"
#endif

#ifdef DUAL_MIC_RECORDING
#include "app_recording_handle.h"
#endif

#define CASE_S(s) \
    case s:       \
        return "[" #s "]";
#define CASE_D() \
    default:     \
        return "[INVALID]";
#define APP_AI_ROLE_SWITCH_TIME_IN_MS (500)

APP_AI_TWS_REBOOT_T REBOOT_CUSTOM_PARAM_LOC app_ai_tws_reboot = {false, 0xFF};

#if defined(IBRT) && defined(__AI_VOICE__)
static void app_ai_role_switch_timeout_cb(void const *n);
osTimerDef(APP_AI_ROLE_SWITCH_TIMER, app_ai_role_switch_timeout_cb);
osTimerId app_ai_role_switch_timer_id = NULL;

static void app_ai_role_switch_timeout_cb(void const *n)
{
#ifdef __IAG_BLE_INCLUDE__
    app_ai_tws_role_switch_dis_ble();
#endif
}

#define APP_AI_BLE_DISC_TIME_IN_MS (100)
static void app_ai_ble_disc_timeout_cb(void const *n);
osTimerDef(APP_AI_BLE_DISC_TIMER, app_ai_ble_disc_timeout_cb);
osTimerId app_ai_ble_disc_timer_id = NULL;

extern "C" bool app_ai_ble_is_connected(uint8_t ai_index);
extern "C" uint16_t app_ai_ble_get_conhdl(uint8_t connect_index);

static void app_ai_ble_disc_timeout_cb(void const *n)
{
    TRACE(1, "%s", __func__);

    app_ai_tws_role_switch_direct();
}
#endif

void app_ai_let_slave_continue_roleswitch(void)
{
#if defined(IBRT) && defined(__AI_VOICE__)
    uint8_t role = app_ai_get_ai_spec(AI_SPEC_AMA);
    tws_ctrl_send_cmd(APP_TWS_CMD_LET_SLAVE_CONTINUE_RS, &role, 1);
#endif
}

bool app_ai_tws_role_switch_direct(void)
{
    bool ret = false;
#if defined(IBRT) && defined(__AI_VOICE__)
    uint8_t ai_index = 0;
    for (uint8_t ai_connect_index = 0; ai_connect_index < AI_CONNECT_NUM_MAX; ai_connect_index++)
    {
        ai_index = app_ai_get_ai_index_from_connect_index(ai_connect_index);

        if (!app_ai_is_role_switching(ai_index))
        {
            TRACE(1,"%s isn't switching now", __func__);
            return ret;
        }

        TRACE(2,"%s initiative %d", __func__, app_ai_is_initiative_switch(ai_index));
        if (app_ai_is_role_switch_direct(ai_index))
        {
            TRACE(1,"%s has done", __func__);
            return ret;
        }
    }

    app_ai_set_role_switch_direct(true, ai_index);

    osTimerStop(app_ai_role_switch_timer_id);
    osTimerStop(app_ai_ble_disc_timer_id);

    if (app_ai_is_initiative_switch(ai_index))
    {
        app_ibrt_if_tws_switch_prepare_done_in_bt_thread(IBRT_ROLE_SWITCH_USER_AI, app_ai_get_ai_spec(ai_index));
        //BES intentional code. ai_index will not be 10.
        return ret;
    }
    else
    {
        app_ai_let_slave_continue_roleswitch();
        //ret = true;
    }
#endif

    return ret;
}

#ifdef __IAG_BLE_INCLUDE__
bool app_ai_tws_role_switch_dis_ble(void)
{
    bool ret = false;
#if defined(IBRT) && defined(__AI_VOICE__)
    osTimerStop(app_ai_role_switch_timer_id);
#if defined(__AI_VOICE_BLE_ENABLE__)
    uint16_t ble_Conhandle = 0;
    for(uint8_t connect_index = 0; connect_index < AI_CONNECT_NUM_MAX; connect_index++)
    {
        if (app_ai_ble_is_connected(connect_index))
        {
            // disconnect ble connection if existing
            ble_Conhandle = app_ai_ble_get_conhdl(connect_index);
            bt_drv_reg_op_ble_sup_timeout_set(ble_Conhandle, 15); //fix ble disconnection takes long time:connSupervisionTime=150ms
            app_ai_if_ble_disconnect_all();
            osTimerStart(app_ai_ble_disc_timer_id, APP_AI_BLE_DISC_TIME_IN_MS);
            ret = true;
        }
    }

    if(!ret)
    {
        ret = app_ai_tws_role_switch_direct();
    }
#endif
        ret = app_ai_tws_role_switch_direct();
#endif

    return ret;
}
#endif

#if defined(IBRT) && defined(__AI_VOICE__)
static void app_ai_tws_slave_request_master_role_switch(void)
{
    //TRACE(3,"%s complete %d switching %d", __func__,
    //        app_ai_is_setup_complete(),
    //        app_ai_is_role_switching());
    AI_connect_info *ai_info =  app_ai_get_connect_info(ai_manager_get_foreground_ai_conidx());
    if(NULL == ai_info) {
        return;
    }
    uint8_t role = ai_info->ai_spec;

    tws_ctrl_send_cmd(APP_TWS_CMD_LET_MASTER_PREPARE_RS, &role, 1);
    osTimerStart(app_ai_ble_disc_timer_id, APP_AI_ROLE_SWITCH_TIME_IN_MS + APP_AI_BLE_DISC_TIME_IN_MS);
}
#endif

bool app_ai_tws_master_role_switch(void)
{
    bool ret = false;
#if defined(IBRT) && defined(__AI_VOICE__)
    //TRACE(3,"%s complete %d switching %d", __func__,
    //        app_ai_is_setup_complete(),
    //        app_ai_is_role_switching());
    AI_connect_info *ai_info =  app_ai_get_connect_info(ai_manager_get_foreground_ai_conidx());
    if(NULL == ai_info) {
        return true;
    }
    uint8_t ai_index = ai_info->ai_spec;
    uint8_t dest_id = app_ai_get_dest_id(ai_info);

    app_ai_voice_stream_control(false, app_ai_voice_get_user_from_spec(ai_index));

    if (app_ai_is_use_thirdparty(ai_index))
    {
        TRACE(0, "do role switch.");
        #ifdef __THIRDPARTY
        app_ai_if_thirdparty_event_handle(THIRDPARTY_FUNC_KWS, THIRDPARTY_AI_DISCONNECT, ai_index);
        #endif
    }

    app_ai_set_role_switching(true, ai_index);
    osTimerStart(app_ai_role_switch_timer_id, APP_AI_ROLE_SWITCH_TIME_IN_MS);
    if (app_ai_is_stream_running(ai_index) || (app_ai_get_speech_state(ai_index) != AI_SPEECH_STATE__IDLE))
    {
        TRACE(2,"%s stream runing %d", __func__, app_ai_is_stream_running(ai_index));

        ai_function_handle(CALLBACK_STOP_SPEECH, NULL, 0, ai_index, dest_id);
    }

    if (ERROR_RETURN != ai_function_handle(API_AI_ROLE_SWITCH, NULL, 0, ai_index, dest_id))
    {
        return true;
    }

#ifdef __IAG_BLE_INCLUDE__
    ret = app_ai_tws_role_switch_dis_ble();
#endif
#endif

    return ret;
}

bool app_ai_role_switch(void)
{
    bool ret = false;
#ifdef __AI_VOICE__
    uint8_t ai_index = 0;
    uint8_t ai_index_tmep = 0;

    for (uint8_t ai_connect_index = 0; ai_connect_index < AI_CONNECT_NUM_MAX; ai_connect_index++)
    {
        ai_index = app_ai_get_ai_index_from_connect_index(ai_connect_index);

        TRACE(3,"%s complete %d switching %d", __func__, \
                app_ai_is_setup_complete(ai_index), \
                app_ai_is_role_switching(ai_index));

        if (app_ai_is_role_switching(ai_index))
        {
            TRACE(1,"%s is switching now", __func__);
            app_ai_set_role_switching(false, ai_index_tmep);
            app_ai_set_initiative_switch(false, ai_index_tmep);
            return ret;
        }
        app_ai_set_role_switching(true, ai_index);
        app_ai_set_initiative_switch(true, ai_index);

        ai_index_tmep = ai_index;

        if(!app_ai_is_setup_complete(ai_index) && !app_ai_is_peer_setup_complete(ai_index))
            //BES intentional code. ai_index will not be 10
        {
            TRACE(1,"%s ai not setup complete", __func__);
            return ret;
        }
    }

#if defined(IBRT)
    if (app_ai_tws_get_local_role() == APP_AI_TWS_MASTER)
    {
        ret = app_ai_tws_master_role_switch();
    }
    else
    {
        app_ai_tws_slave_request_master_role_switch();
        ret = true;
    }
#endif
#endif

    return ret;
}

uint32_t app_ai_tws_role_switch_prepare(uint32_t *wait_ms)
{
    uint32_t ret = 0;
#ifdef BISTO_ENABLED
    *wait_ms = 800;
    ret |= (1 << AI_SPEC_GSOUND);
#endif

#ifdef __AI_VOICE__
#ifndef __BIXBY_VOICE__
    uint8_t ai_index = app_ai_get_ai_index_from_connect_index(ai_manager_get_foreground_ai_conidx());
    if (app_ai_role_switch())
    {
        *wait_ms = 800;
        ret |= (1 << app_ai_get_ai_spec(ai_index));
    }
#else
    uint8_t ai_index = AI_SPEC_BIXBY;
    if (app_bixby_start_role_switch_handle(wait_ms))
    {
        ret |= (1 << app_ai_get_ai_spec(ai_index));
    }
#endif
#endif

    TRACE(3, "[%s] ret=%d, wait_ms=%d", __func__, ret, *wait_ms);
    return ret;
}

void app_ai_tws_master_role_switch_prepare(void)
{
#ifdef __AI_VOICE__
    app_ai_tws_master_role_switch();
#endif
}

void app_ai_tws_role_switch_prepare_done(void)
{
#if defined(IBRT) && defined(__AI_VOICE__)
    TRACE(1, "%s", __func__);
    if (APP_AI_TWS_MASTER == app_ai_tws_get_local_role())
    {
        app_ibrt_if_sync_info(TWS_SYNC_USER_AI_INFO);
        app_ibrt_if_sync_info(TWS_SYNC_USER_AI_MANAGER);
    }

    app_ai_tws_role_switch_direct();
#endif
}

void app_ai_tws_role_switch(void)
{
    TRACE(1, "[%s]", __func__);

#ifdef BISTO_ENABLED
    gsound_tws_request_roleswitch();
#endif
}

void app_ai_tws_role_switch_complete(void)
{
#ifdef __AI_VOICE__
    TRACE(1, "%s", __func__);

#ifdef __IAG_BLE_INCLUDE__
    app_ai_if_ble_set_adv(AI_BLE_ADVERTISING_INTERVAL);
#endif

#ifndef __BIXBY_VOICE__
    for (uint8_t ai_connect_index = 0; ai_connect_index < AI_CONNECT_NUM_MAX; ai_connect_index++)
    {
        uint8_t ai_index = app_ai_get_ai_index_from_connect_index(ai_connect_index);
        app_ai_set_role_switching(false, ai_index);
        app_ai_set_initiative_switch(false, ai_index);
        app_ai_set_role_switch_direct(false, ai_index);
        app_ai_set_can_role_switch(false, ai_index);

        if (app_ai_is_setup_complete(ai_index))
        {
            app_ai_set_speech_state(AI_SPEECH_STATE__IDLE, ai_index);
            app_ai_voice_deinit(ai_index, ai_connect_index);
#if defined(IBRT) && defined(__AI_VOICE__)
            if (app_tws_ibrt_tws_link_connected() && (app_ai_tws_get_local_role() == APP_AI_TWS_MASTER))
#endif
            {
                if (app_ai_is_in_multi_ai_mode())
                {
                    if (AI_SPEC_GSOUND == ai_manager_get_current_spec() ||
                        AI_SPEC_INIT == ai_manager_get_current_spec())
                    {
                        return;
                    }
                }

                if (app_ai_is_use_thirdparty(ai_index))
                {
                    #ifdef __THIRDPARTY
                    app_ai_if_thirdparty_event_handle(THIRDPARTY_FUNC_KWS, THIRDPARTY_AI_CONNECT, ai_index);
                    #endif
                }
                app_ai_voice_stream_control(true, app_ai_voice_get_user_from_spec(ai_index));
            }
#if defined(IBRT) && defined(__AI_VOICE__)
            else if (app_tws_ibrt_tws_link_connected() && (APP_AI_TWS_SLAVE == app_ai_tws_get_local_role()))
            {
                /// close mic if slave is not allowed to open mic
                if (!app_ai_voice_get_slave_open_mic_flag(app_ai_voice_get_user_from_spec(ai_index)))
                {
                    app_ai_voice_stream_control(false, app_ai_voice_get_user_from_spec(ai_index));
                }
            }
#endif
        }
    }
#ifdef DUAL_MIC_RECORDING
    app_recording_role_switch_done_handle();
#endif

#else
    app_bixby_role_switch_done_handle();
#endif

    app_ai_ui_global_handler_ind(AI_CUSTOM_CODE_AI_ROLE_SWITCH_COMPLETE, NULL, 0, 0, 0);
#endif
}

void app_ai_tws_sync_info_prepare_handler(uint8_t *buf,
    uint16_t *totalLen, uint16_t *len, uint16_t expectLen)
{
#ifdef __AI_VOICE__
    uint16_t buf_len = 0;

    *totalLen = *len = ai_save_ctx(buf, buf_len);
#endif
}

void app_ai_tws_sync_info_received_handler(uint8_t *buf, uint16_t length, bool isContinueInfo)
{
#ifdef __AI_VOICE__
    ai_restore_ctx(buf, length);
#endif
}

void app_ai_tws_sync_init(void)
{
#if defined(IBRT) && defined(__AI_VOICE__)
    TWS_SYNC_USER_T user_app_ai_t = {
        app_ai_tws_sync_info_prepare_handler,
        app_ai_tws_sync_info_received_handler,
        NULL,
        NULL,
        NULL,
    };

    app_ibrt_if_register_sync_user(TWS_SYNC_USER_AI_INFO, &user_app_ai_t);

    if (NULL == app_ai_role_switch_timer_id)
    {
        app_ai_role_switch_timer_id =
            osTimerCreate(osTimer(APP_AI_ROLE_SWITCH_TIMER), osTimerOnce, NULL);
    }
    if (NULL == app_ai_ble_disc_timer_id)
    {
        app_ai_ble_disc_timer_id =
            osTimerCreate(osTimer(APP_AI_BLE_DISC_TIMER), osTimerOnce, NULL);
    }
#endif
}

void app_ai_tws_sync_ai_info(void)
{
#if defined(IBRT) && defined(__AI_VOICE__)
    app_ibrt_if_prepare_sync_info();
    app_ibrt_if_sync_info(TWS_SYNC_USER_AI_INFO);
    app_ibrt_if_flush_sync_info();
#endif
}

void ai_manager_sync_info_prepare_handler(uint8_t *buf,
    uint16_t *totalLen, uint16_t *len, uint16_t expectLen)
{
#ifdef IS_MULTI_AI_ENABLED
    uint16_t buf_len = 0;

    *totalLen = *len = ai_manager_save_ctx(buf, buf_len);
#endif
}

void ai_manager_sync_info_received_handler(uint8_t *buf, uint16_t length, bool isContinueInfo)
{
#ifdef IS_MULTI_AI_ENABLED
    ai_manager_restore_ctx(buf, length);
#endif
}

void ai_manager_sync_info_received_rsp_handler(uint8_t *buf, uint16_t length, bool isContinueInfo)
{
#ifdef IS_MULTI_AI_ENABLED
    ai_manager_save_ctx_rsp_handle(buf, length);
#endif
}

void ai_manager_sync_init(void)
{
#if defined(IBRT) && defined(IS_MULTI_AI_ENABLED)
    TWS_SYNC_USER_T user_ai_manager_t = {
        ai_manager_sync_info_prepare_handler,
        ai_manager_sync_info_received_handler,
        ai_manager_sync_info_prepare_handler,
        ai_manager_sync_info_received_rsp_handler,
        ai_manager_sync_info_received_rsp_handler,
    };

    app_ibrt_if_register_sync_user(TWS_SYNC_USER_AI_MANAGER, &user_ai_manager_t);
#endif
}

void app_ai_tws_sync_ai_manager_info(void)
{
#if defined(IBRT) && defined(IS_MULTI_AI_ENABLED) && !defined(FREEMAN_ENABLED_STERO)
    app_ibrt_if_prepare_sync_info();
    app_ibrt_if_sync_info(TWS_SYNC_USER_AI_MANAGER);
    app_ibrt_if_flush_sync_info();
#endif
}

#ifdef IBRT
static void gsound_connect_sync_info_prepare_handler(uint8_t *buf,
    uint16_t *totalLen, uint16_t *len, uint16_t expectLen)
{
#ifdef BISTO_ENABLED
    AI_CONNECTION_STATE_T *pAiInfo = (AI_CONNECTION_STATE_T *)buf;

    pAiInfo->connBtState = gsound_is_bt_connected();
    pAiInfo->connPathType = gsound_custom_get_connection_path();
    memcpy(pAiInfo->connBdAddr, gsound_get_connected_bd_addr(), 6);

    if (CONNECTION_BLE == pAiInfo->connPathType)
    {
        pAiInfo->connPathState = gsound_get_ble_connect_state();
    }
    else if (CONNECTION_BT == pAiInfo->connPathType)
    {
        pAiInfo->connPathState = gsound_get_bt_connect_state();
    }
    else
    {
        pAiInfo->connPathState = 0;
    }

    *totalLen = *len = sizeof(AI_CONNECTION_STATE_T);
#endif
}

static void gsound_connect_sync_info_received_handler(uint8_t *buf, uint16_t length, bool isContinueInfo)
{
#ifdef BISTO_ENABLED
    gsound_custom_connection_state_received_handler(buf);
#endif
}
#endif

void app_ai_tws_gsound_sync_init(void)
{
#ifdef IBRT
    TWS_SYNC_USER_T userAiConnect = {
        gsound_connect_sync_info_prepare_handler,
        gsound_connect_sync_info_received_handler,
        NULL,
        NULL,
        NULL,
    };

    app_ibrt_if_register_sync_user(TWS_SYNC_USER_AI_CONNECTION, &userAiConnect);
#endif
}

#if defined(IBRT)
void app_tws_ai_send_cmd_to_peer(uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"[%s]", __func__);

    tws_ctrl_send_cmd(APP_TWS_CMD_AI_SEND_CMD_TO_PEER, p_buff, length);
}

void app_tws_ai_rev_peer_cmd_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"[%s]", __func__);

#ifdef __AI_VOICE__
    app_ai_tws_rev_peer_cmd_hanlder(rsp_seq, p_buff, length);
#endif
}

void app_tws_ai_send_cmd_with_rsp_to_peer(uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"[%s]", __func__);
    tws_ctrl_send_cmd(APP_TWS_CMD_AI_SEND_CMD_TO_PEER_WITH_RSP, p_buff, length);
}

void app_tws_ai_send_cmd_rsp_to_peer(uint8_t *p_buff, uint16_t rsp_seq, uint16_t length)
{
    TRACE(1,"[%s]", __func__);
    tws_ctrl_send_rsp(APP_TWS_CMD_AI_SEND_CMD_TO_PEER_WITH_RSP, rsp_seq, p_buff, length);
}

void app_tws_ai_rev_peer_cmd_with_rsp_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"[%s]", __func__);

#ifdef __AI_VOICE__
    app_ai_tws_rev_peer_cmd_with_rsp_hanlder(rsp_seq, p_buff, length);
#endif
}

void app_tws_ai_rev_cmd_rsp_from_peer_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"[%s]", __func__);

#ifdef __AI_VOICE__
    app_ai_tws_rev_cmd_rsp_from_peer_hanlder(rsp_seq, p_buff, length);
#endif
}

void app_tws_ai_rev_cmd_rsp_timeout_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    TRACE(1,"[%s]", __func__);

#ifdef __AI_VOICE__
    app_ai_tws_rev_cmd_rsp_timeout_hanlder(rsp_seq, p_buff, length);
#endif
}

void app_ai_tws_send_cmd_to_peer(uint8_t *p_buff, uint16_t length)
{
#ifdef IBRT
    if (app_tws_ibrt_tws_link_connected())
    {
        app_tws_ai_send_cmd_to_peer(p_buff, length);
    }
#endif
}

void app_ai_tws_rev_peer_cmd_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
#ifdef __AI_VOICE__
    app_ai_rev_peer_cmd_hanlder(rsp_seq, p_buff, length);
#endif
}

void app_ai_tws_send_cmd_with_rsp_to_peer(uint8_t *p_buff, uint16_t length)
{
#ifdef IBRT
    if (app_tws_ibrt_tws_link_connected())
    {
        app_tws_ai_send_cmd_with_rsp_to_peer(p_buff, length);
    }
#endif
}

void app_ai_tws_send_cmd_rsp_to_peer(uint8_t *p_buff, uint16_t rsp_seq, uint16_t length)
{
#ifdef IBRT
    if (app_tws_ibrt_tws_link_connected())
    {
        app_tws_ai_send_cmd_rsp_to_peer(p_buff, rsp_seq, length);
    }
#endif
}

void app_ai_tws_rev_peer_cmd_with_rsp_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
#ifdef __AI_VOICE__
    app_ai_rev_peer_cmd_hanlder(rsp_seq, p_buff, length);
#endif
}

void app_ai_tws_rev_cmd_rsp_from_peer_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
#ifdef __AI_VOICE__
    app_ai_rev_peer_cmd_hanlder(rsp_seq, p_buff, length);
#endif
}

void app_ai_tws_rev_cmd_rsp_timeout_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
#ifdef __AI_VOICE__
    app_ai_rev_peer_cmd_rsp_timeout_hanlder(rsp_seq, p_buff, length);
#endif
}
#endif

bool app_ai_tws_init_done(void)
{
#if defined(IBRT)
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    return (p_ibrt_ctrl->init_done != 0);
#endif
    return false;
}

bool app_ai_tws_link_connected(void)
{
#if defined(IBRT)
    return app_tws_ibrt_tws_link_connected();
#endif
    return false;
}

uint8_t *app_ai_tws_local_address(void)
{
    uint8_t *local_addr = NULL;
#if defined(IBRT)
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    local_addr = p_ibrt_ctrl->local_addr.address;
#endif
    return local_addr;
}

void app_ai_tws_reboot_record_box_state(void)
{

}

uint8_t app_ai_tws_reboot_get_box_action(void)
{
    return 0xFF;
}

void app_ai_tws_clear_reboot_box_state(void)
{
#if defined(IBRT)
    TRACE(2, "%s box state %d", __func__, app_ai_tws_reboot.box_state);
    app_ai_tws_reboot.is_ai_reboot = false;
    app_ai_tws_reboot.box_state = 0xFF;
#endif
}

void app_ai_tws_disconnect_all_bt_connection(void)
{
#if defined(IBRT)
    app_tws_ibrt_disconnect_all_connection();
#endif
}

bool app_ai_tws_is_mobile_link_connected(void)
{
#ifdef IBRT
    ibrt_mobile_info_t *p_mobile_info = app_ibrt_conn_get_mobile_info_ext();
    return app_tws_ibrt_mobile_link_connected(&p_mobile_info->mobile_addr);
#else   //IBRT
    return false;
#endif
}

bool app_ai_tws_is_profile_exchanged(void)
{
#ifdef IBRT
    ibrt_mobile_info_t *p_mobile_info = app_ibrt_conn_get_mobile_info_ext();
    return APP_IBRT_IS_PROFILE_EXCHANGED(&p_mobile_info->mobile_addr);
#else   //IBRT
    return false;
#endif
}

bool app_ai_tws_is_slave_ibrt_link_connected(void)
{
#ifdef IBRT
    ibrt_mobile_info_t *p_mobile_info = app_ibrt_conn_get_mobile_info_ext();
    return app_tws_ibrt_slave_ibrt_link_connected(&p_mobile_info->mobile_addr);
#else   //IBRT
    return false;
#endif
}

uint8_t app_ai_tws_get_local_role(void)
{
    uint8_t local_role = APP_AI_TWS_UNKNOW;
#if defined(IBRT)
    bool tws_link_connected = app_tws_ibrt_tws_link_connected();
    bool tws_profile_exchanged = app_ai_tws_is_profile_exchanged();
    bool mobile_link_connected = app_ai_tws_is_mobile_link_connected();
    bool slave_ibrt_link_connected = app_ai_tws_is_slave_ibrt_link_connected();
    TWS_UI_ROLE_E current_role = app_ibrt_if_get_ui_role();

    if (tws_profile_exchanged)
    {
        if (mobile_link_connected)
        {
            local_role = APP_AI_TWS_MASTER;
        }
        else if (slave_ibrt_link_connected)
        {
            local_role = APP_AI_TWS_SLAVE;
        }
        else
        {
            local_role = APP_AI_TWS_UNKNOW;
        }
    }
    else if (tws_link_connected)
    {
        if (mobile_link_connected)
        {
            local_role = APP_AI_TWS_MASTER;
        }
        else if(current_role == IBRT_MASTER)
        {
            local_role = APP_AI_TWS_MASTER;
        }
        else
        {
            local_role = APP_AI_TWS_SLAVE;
        }

    }
    else
    {
        if (mobile_link_connected)
        {
            local_role = APP_AI_TWS_MASTER;
        }
        else
        {
            local_role = APP_AI_TWS_UNKNOW;
        }
    }

#if 0
    TRACE(5, "[%s] %d ex %d tws %d mob %d ibrt %d cur %d",
                            __func__, local_role,
                            tws_profile_exchanged,
                            tws_link_connected,
                            mobile_link_connected,
                            slave_ibrt_link_connected,
                            current_role);
#endif
#endif
    return local_role;
}

void app_ai_tws_init(void)
{
#if defined(IBRT) && defined(__AI_VOICE__)
#ifdef FREEMAN_ENABLED_STERO
    app_ai_set_in_tws_mode(false, 0);
#else
    app_ai_set_in_tws_mode(true, 0);
#endif
#endif
}
