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
#include "gsound_target.h"
#include "gsound_dbg.h"
#include "gsound_custom_actions.h"

/*********************external function declearation************************/

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declearation************************/

/************************private variable defination************************/

/****************************function defination****************************/
GSoundStatus GSoundTargetActionRejected(GSoundActionMask rejected_action)
{
    // send back to BES
    gsound_custom_actions_bes_handle_last_key();

    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetActionInit(const GSoundActionInterface *handlers)
{
    gsound_custom_actions_store_lib_interface(handlers);
    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetActionGetAssistantGesture(
    GSoundAssistantGestureAvailability *availability)
{
	*availability = GSOUND_ASSISTANT_GESTURE_AVAILABILITY_UNSET;
    return GSOUND_STATUS_OK;
}