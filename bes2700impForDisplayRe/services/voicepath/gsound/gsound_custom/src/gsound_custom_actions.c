/***************************************************************************
*
*Copyright 2015-2020 BES.
*All rights reserved. All unpublished rights reserved.
*
*No part of this work may be used or reproduced in any form or by any
*means, or stored in a database or retrieval system, without prior written
*permission of BES.
*
*Use of this work is governed by a license granted by BES.
*This work contains confidential and proprietary information of
*BES. which is protected by copyright, trade secret,
*trademark and other intellectual property rights.
*
****************************************************************************/


/*****************************header include********************************/
#include "cmsis.h"
#include "bluetooth_bt_api.h"
#include "btapp.h"
#include "app_key.h"
#include "gsound_dbg.h"
#include "gsound_target.h"
#include "gsound_custom_actions.h"
#include "ai_thread.h"
#include "ai_manager.h"

/*********************external function declearation************************/

/************************private macro defination***************************/

/************************private type defination****************************/

/************************extern function declearation***********************/

/**********************private function declearation************************/

/************************private variable defination************************/
static APP_KEY_STATUS last_key_status;
static const GSoundActionInterface *actionInterface;

#ifdef IS_GSOUND_BUTTION_HANDLER_WORKAROUND_ENABLED
static uint8_t last_gsound_key_event = APP_KEY_EVENT_NONE;
#endif

/****************************function defination****************************/
/**
 * @brief store the actions interface comes from libgsound
 * 
 * @param handlers pointer of actions handler
 */
void gsound_custom_actions_store_lib_interface(const GSoundActionInterface *handlers)
{
    actionInterface = handlers;
}

/**
 * @brief send unhandled key event back to BES plateform
 * 
 */
void gsound_custom_actions_bes_handle_last_key(void)
{
    GLOG_I("send key event back to BES plateform");
    bt_key_send(&last_key_status);
}

#ifdef IS_GSOUND_BUTTION_HANDLER_WORKAROUND_ENABLED
static void _push_action_event(GSoundActionMask action)
{
    actionInterface->gsound_action_on_event(action, NULL);
}
#endif

/**
 * @brief Convert a BES GA button event into a Google button event.
 * 
 * @param event incoming event.
 * @param result pointer of convert result
 * @return GSoundStatus @see GSoundStatus
 */
GSoundStatus gsound_util_google_key_to_action(uint8_t event,
                                              GSoundActionMask *result)
{
    if (result == NULL)
    {
        return GSOUND_STATUS_ERROR;
    }

#ifdef IS_GSOUND_BUTTION_HANDLER_WORKAROUND_ENABLED
    uint8_t last_key_event = last_gsound_key_event;
    last_gsound_key_event  = event;
#endif

    switch (event)
    {
    case APP_KEY_EVENT_FIRST_DOWN:
        *result = GSOUND_TARGET_ACTION_GA_VOICE_PREPARE |
                  GSOUND_TARGET_ACTION_GA_FETCH_PREPARE |
                  GSOUND_TARGET_ACTION_GA_WILL_PAUSE;
        return GSOUND_STATUS_OK;
        break;
    case APP_KEY_EVENT_LONGPRESS:
#ifdef IS_GSOUND_BUTTION_HANDLER_WORKAROUND_ENABLED
        if (APP_KEY_EVENT_UP == last_key_event)
        {
            _push_action_event(GSOUND_TARGET_ACTION_GA_VOICE_PREPARE |
                               GSOUND_TARGET_ACTION_GA_FETCH_PREPARE |
                               GSOUND_TARGET_ACTION_GA_WILL_PAUSE);
        }
#endif
#ifdef REDUCED_GUESTURE_ENABLED
        *result = GSOUND_TARGET_ACTION_GA_VOICE_PTT_OR_FETCH;
#else
        *result = GSOUND_TARGET_ACTION_GA_VOICE_PTT;
#endif
        return GSOUND_STATUS_OK;
        break;
    case APP_KEY_EVENT_DOUBLECLICK:
        *result = GSOUND_TARGET_ACTION_GA_STOP_ASSISTANT;
        return GSOUND_STATUS_OK;
        break;
    case APP_KEY_EVENT_CLICK:
#ifndef IS_GSOUND_BUTTION_HANDLER_WORKAROUND_ENABLED
        *result =
            GSOUND_TARGET_ACTION_GA_FETCH |
            GSOUND_TARGET_ACTION_GA_VOICE_CONFIRM |  //GSOUND_TARGET_ACTION_GA_STOP_ASSISTANT |
            GSOUND_TARGET_ACTION_GA_VOICE_DONE;
#else
        *result =
            GSOUND_TARGET_ACTION_GA_FETCH |
            GSOUND_TARGET_ACTION_GA_VOICE_CONFIRM;
#endif
        return GSOUND_STATUS_OK;
        break;

    case APP_KEY_EVENT_UP_AFTER_LONGPRESS:
#ifdef IS_GSOUND_BUTTION_HANDLER_WORKAROUND_ENABLED
        return GSOUND_STATUS_ERROR;
#else
        *result = GSOUND_TARGET_ACTION_GA_VOICE_DONE;
        return GSOUND_STATUS_OK;
#endif
        break;
#ifdef IS_GSOUND_BUTTION_HANDLER_WORKAROUND_ENABLED
    case APP_KEY_EVENT_UP:
        *result = GSOUND_TARGET_ACTION_GA_VOICE_DONE;
        return GSOUND_STATUS_OK;
        break;
#endif
    default:
        // GLOG_I(0,"unhandled google key event");
        break;
    }
    return GSOUND_STATUS_ERROR;
}

/**
 * @brief Convert a BES keypress event into a Google action.
 * 
 * @param status BES key status
 * @param result Google style action event
 * @return GSoundStatus @see GSoundStatus
 */
GSoundStatus gsound_util_key_to_action(const APP_KEY_STATUS *status,
                                       GSoundActionEvents *actionEvent)
{
    GSoundStatus ret = GSOUND_STATUS_ERROR;
    // If either input or output is invalid, return error
    if (status != NULL && actionEvent != NULL)
    {
        switch (status->code)
        {
        case APP_KEY_CODE_FN1:
            GLOG_D("this is left button.");
            if (APP_KEY_EVENT_CLICK == status->event)
            {
                *actionEvent = GSOUND_TARGET_ACTION_TOGGLE_PLAY_PAUSE |
                               GSOUND_TARGET_ACTION_GA_STOP_ASSISTANT;
                ret = GSOUND_STATUS_OK;
            }
            else
            {
                GLOG_I("event is not recognized!");
            }
            break;

        case APP_KEY_CODE_FN2:
            GLOG_D("this is central button.");
            if (APP_KEY_EVENT_CLICK == status->event)
            {
                *actionEvent = (GSOUND_TARGET_ACTION_PREV_TRACK);
                ret          = GSOUND_STATUS_OK;
            }
            else if (APP_KEY_EVENT_DOUBLECLICK == status->event)
            {
                *actionEvent = (GSOUND_TARGET_ACTION_NEXT_TRACK);
                ret          = GSOUND_STATUS_OK;
            }
            else
            {
                GLOG_I("event is not recognized!");
            }
            break;

        case APP_KEY_CODE_FN8:
            GLOG_D("this up button.");
            if (APP_KEY_EVENT_CLICK == status->event)
            {
                *actionEvent = GSOUND_TARGET_ACTION_LEGACY_VOICE_ACTIVATION;
                ret          = GSOUND_STATUS_OK;
            }
            else
            {
                GLOG_I("event is not recognized!");
            }
            break;

        default:
            GLOG_I("unhandled key code");
            break;
        }
    }

    return ret;
}

/**
 * @brief Decides whether or not a keypress is shared between Target/Platform and
 * GSound or is fully owned by GSound.
 * 
 * @param status incoming key status
 * @return true Shared, GSound will decide whether to consume or release
 * @return false Not shared, fully owned by GSound and GSound will always
 *               consume
 */
inline bool gsound_util_key_is_shared(APP_KEY_STATUS *status)
{
    if (status == NULL)
    {
        return false;
    }

    switch (status->code)
    {
    case APP_KEY_CODE_PWR:
        return true;
        break;
    case APP_KEY_CODE_FN8:
        return true;
        break;
    case APP_KEY_CODE_GOOGLE:
        return false;
        break;
    default:
        break;
    }
    return true;
}

/**
 * @brief handle the gsound action key
 * 
 * @param status key status, @see APP_KEY_STATUS
 * @param param pointer of parameter
 */
void gsound_custom_actions_handle_key(APP_KEY_STATUS *status, void *param)
{
    AI_connect_info *ai_info = app_ai_get_connect_info(ai_manager_get_foreground_ai_conidx());
    if(NULL == ai_info) {
        return;
    }

    if (AI_SPEC_GSOUND != ai_info->ai_spec)
    {
        return;
    }

    GLOG_I("%s: code=0x%X, event=%d", __func__, status->code, status->event);
    if (status == NULL)
    {
        return;
    }

    if (gsound_util_key_is_shared(status) == false)
    {

        // Shared key, possible Action. Check and send to GSound if needed
        GSoundActionMask gsound_event;
        if (gsound_util_google_key_to_action(status->event, &gsound_event) ==
            GSOUND_STATUS_OK)
        {
            last_key_status = *status;
            actionInterface->gsound_action_on_event(gsound_event, NULL);
        }
    }
    else
    {
        GLOG_D("it is shared key");
        GSoundActionEvents gsound_event;
        if (gsound_util_key_to_action(status, &gsound_event) == GSOUND_STATUS_OK)
        {
            last_key_status = *status;
            actionInterface->gsound_action_on_event(gsound_event, NULL);
        }
    }
}
