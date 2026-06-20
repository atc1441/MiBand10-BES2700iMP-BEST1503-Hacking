/***************************************************************************
*
*Copyright 2015-2019 BES.
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
#include <stdio.h>
#include <stdlib.h>
#include "gsound_dbg.h"

extern "C"
{
#include "cmsis_os.h"
#include "bluetooth_bt_api.h"
#include "app_bt.h"
#include "hal_aud.h"
#include "gsound_custom_bt.h"
#include "gsound_target.h"
#include "cqueue.h"
#include "me_api.h"
#include "sdp_api.h"
#include "spp_api.h"
#include "a2dp_api.h"
#include "avrcp_api.h"
#include "hfp_api.h"
#include "bt_if.h"
#include "app_ai_manager_api.h"
#include "app_bt_media_manager.h"
}

#include "btapp.h"
#include "gsound_custom_bt.h"
#include "gsound_custom_ota.h"
#include "gsound_custom.h"
#include "cmsis.h"
#include "gsound_custom_audio.h"

#ifdef IBRT
#include "app_ibrt_internal.h"
#include "earbud_ux_api.h"
#include "gsound_custom_tws.h"
#include "app_ai_tws.h"
#endif

#if defined(DUMP_CRASH_ENABLE)
#include "gsound_custom_reset.h"
#endif

#ifdef __AI_VOICE__
#include "ai_manager.h"
#endif

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declearation************************/

/************************private variable defination************************/
static const GSoundBtInterface *bt_interface;

/****************************function defination****************************/

typedef struct
{
    bool isSppConnected;
    GSoundChannelType channelType;
    bool gsoundConnectionAllowed;
    bt_spp_channel_t *spp_chan;
    uint8_t gAddr[6];
} GsoundRFcommServiceEnv_t;

typedef struct {
    CQItemType *e1;
    unsigned int len1;
    CQItemType *e2;
    unsigned int len2;
}GSOUND_BUF_PTR_T;

GsoundRFcommServiceEnv_t gsoundControl[BT_DEVICE_NUM] = {0};
GsoundRFcommServiceEnv_t gsoundAudio[BT_DEVICE_NUM] = {0};
static uint8_t gsound_data_buff[GSOUND_RX_BUF_SIZE];
static uint8_t gsound_data_packet[GSOUND_RX_PACKET_SIZE];
static CQueue gQueue;
static osMutexId gsoundBtMutexId;
osMutexDef(gsoundBtMutex);

#if SPP_SERVER == XA_ENABLED

/****************************************************************************
 * SPP SDP Entries
 ****************************************************************************/

/* 128 bit UUID in Big Endian f8d1fbe4-7966-4334-8024-ff96c9330e15 */
static const uint8_t GSOUND_DATA_UUID_128[16] = {
    0x15, 0x0e, 0x33, 0xc9, 0x96, 0xff, 0x24, 0x80, 0x34, 0x43, 0x66, 0x79, 0xe4, 0xfb, 0xd1, 0xf8};

static const uint8_t GSOUND_DATA_UUID_128_BE[16] = {
    0xf8, 0xd1, 0xfb, 0xe4, 0x79, 0x66, 0x43, 0x34, 0x80, 0x24, 0xff, 0x96, 0xc9, 0x33, 0x0e, 0x15};

/* 128 bit UUID in Big Endian 81c2e72a-0591-443e-a1ff-05f988593351 */
static const uint8_t GSOUND_VOICE_UUID_128[16] = {
    0x51, 0x33, 0x59, 0x88, 0xf9, 0x05, 0xff, 0xa1, 0x3e, 0x44, 0x91, 0x05, 0x2a, 0xe7, 0xc2, 0x81};

static const uint8_t GSOUND_VOICE_UUID_128_BE[16] = {
    0x81, 0xc2, 0xe7, 0x2a, 0x05, 0x91, 0x44, 0x3e, 0xa1, 0xff, 0x05, 0xf9, 0x88, 0x59, 0x33, 0x51};

#endif

static GsoundRFcommServiceEnv_t *gsound_bt_get_service_by_chnl(uint8_t channel, bool isRfChnl, bool isUsed)
{
    GsoundRFcommServiceEnv_t *gsrvEnv = NULL;
    GsoundRFcommServiceEnv_t *ret = NULL;

    if (isRfChnl)
    {
        gsrvEnv = ((RFCOMM_CHANNEL_GS_CONTROL == channel) ? \
                              gsoundControl : gsoundAudio);
    }
    else
    {
        gsrvEnv = ((GSOUND_CHANNEL_CONTROL == channel) ? \
                              gsoundControl : gsoundAudio);
    }

    for(uint8_t i = 0; i < BT_DEVICE_NUM; i++)
    {
        if ((isUsed && gsrvEnv[i].isSppConnected) || \
            (!isUsed && !gsrvEnv[i].isSppConnected)) {
            ret = (gsrvEnv + i);
            break;
        }
    }

    return ret;
}

void gsound_bt_buf_init()
{
    memset(gsound_data_buff, 0, sizeof(gsound_data_buff));
    InitCQueue(&gQueue, sizeof(gsound_data_buff), (CQItemType *)gsound_data_buff);
}

uint8_t gsound_bt_buf_push(uint8_t *buf, GSOUND_BUF_HEADER_T *header)
{
    int ret = GSOUND_STATUS_OK;
    if (header == NULL)
    {
        return GSOUND_STATUS_ERROR;
    }

    uint32_t lock = int_lock();
    if ((uint32_t)AvailableOfCQueue(&gQueue) >= (header->size + sizeof(GSOUND_BUF_HEADER_T)))
    {
         EnCQueue(&gQueue, (uint8_t *)header, sizeof(GSOUND_BUF_HEADER_T));
         EnCQueue(&gQueue, buf, header->size);
    }
    else
    {
        ret = GSOUND_STATUS_ERROR;
    }
    int_unlock(lock);

    return ret;
}

uint8_t gsound_bt_buf_peek(GSOUND_BUF_PTR_T *buf, GSOUND_BUF_HEADER_T *header)
{
    uint8_t ret = GSOUND_STATUS_OK;
    uint8_t status = 0;
    if (header == NULL)
    {
        return GSOUND_STATUS_ERROR;
    }

    uint32_t lock = int_lock();
    status = PeekCQueueToBuf(&gQueue, (uint8_t *)header, sizeof(GSOUND_BUF_HEADER_T));

    if (status) {
        ret = GSOUND_STATUS_ERROR;
    }
    else if ((uint32_t)LengthOfCQueue(&gQueue) >= (header->size) + sizeof(GSOUND_BUF_HEADER_T))
    {
        PeekCQueue(&gQueue, header->size + sizeof(GSOUND_BUF_HEADER_T),
                   &(buf->e1), &(buf->len1), &(buf->e2), &(buf->len2));
        if (buf->len1 > sizeof(GSOUND_BUF_HEADER_T))
        {
            buf->e1 += sizeof(GSOUND_BUF_HEADER_T);
            buf->len1 -= sizeof(GSOUND_BUF_HEADER_T);
        }
        else
        {
            buf->e1 = buf->e2 + sizeof(GSOUND_BUF_HEADER_T) - buf->len1;
            buf->len1 = buf->len1 + buf->len2 - sizeof(GSOUND_BUF_HEADER_T);
            buf->e2 = NULL;
            buf->len2 = 0;
        }
    }
    else
    {
        ret = GSOUND_STATUS_ERROR;
    }
    int_unlock(lock);

    return ret;
}

void gsound_bt_buf_pop(uint8_t *buf, uint32_t len)
{
    uint32_t lock = int_lock();
    DeCQueue(&gQueue, NULL, len+sizeof(GSOUND_BUF_HEADER_T));
    int_unlock(lock);
}

uint32_t gsound_bt_buf_get_len()
{
    uint32_t len = 0;
    uint32_t lock = int_lock();
    len = LengthOfCQueue(&gQueue);
    int_unlock(lock);

    return len;
}

uint8_t gsound_bt_send_rx_data()
{
    GSOUND_BUF_HEADER_T header = {0};
    GSOUND_BUF_PTR_T gBuf = {0};
    uint8_t *dataBuf = NULL;
    uint8_t ret = GSOUND_STATUS_OK;

    if (gsound_is_bt_connected() && 
        gsound_get_rfcomm_connect_state(GSOUND_CHANNEL_AUDIO) &&
        gsound_get_rfcomm_connect_state(GSOUND_CHANNEL_CONTROL) &&
        (GSOUND_STATUS_OK == gsound_bt_buf_peek(&gBuf, &header)))
    {
        if ((header.chnl == GSOUND_CHANNEL_CONTROL) || (header.chnl == GSOUND_CHANNEL_AUDIO))
        {
            if (gBuf.len2 == 0)
            {
                dataBuf = (uint8_t *)gBuf.e1;
            }
            else
            {
                memcpy(gsound_data_packet, (uint8_t *)gBuf.e1, gBuf.len1);
                memcpy(gsound_data_packet + gBuf.len1, (uint8_t *)gBuf.e2, gBuf.len2);
                dataBuf = gsound_data_packet;
            }

            bt_interface->gsound_bt_on_rx_ready((GSoundChannelType)header.chnl, dataBuf, header.size);
            gsound_bt_set_rx_available(false);
        }
    }
    else
    {
        ret = GSOUND_STATUS_ERROR;
    }
    return ret;
}

int gsound_spp_handle_data_event_func(const bt_bdaddr_t *remote, bt_spp_event_t event, bt_spp_callback_param_t *param)
{
    bt_spp_channel_t *spp_chan = param->spp_chan;
    bt_spp_port_t *port = spp_chan->port;
    uint8_t *data = (uint8_t *)param->rx_data_ptr;
    uint16_t dataLen = param->rx_data_len;
    GSOUND_BUF_HEADER_T header;

#if defined(IBRT)
    if (IBRT_SLAVE == app_ibrt_if_get_ui_role())
    {
        GLOG_I("slave is not allowed to handle data.");
        return 0;
    }
#endif

    GsoundRFcommServiceEnv_t *service = gsound_bt_get_service_by_chnl(port->local_server_channel, true, true);

    if(!gsound_get_rfcomm_connect_state((uint8_t)service->channelType))
    {
        GLOG_I("channel is not connected.");
        return 0;
    }

    header.chnl = service->channelType;
    header.size = dataLen;
    TRACE(3, "spp data chnl:%d, size:%d, avail:%d.", header.chnl, header.size, AvailableOfCQueue(&gQueue));

    osMutexWait(gsoundBtMutexId, osWaitForever);
    if (GSOUND_STATUS_OK == gsound_bt_buf_push(data, &header))
    {
        if ((gsound_bt_buf_get_len() == sizeof(GSOUND_BUF_HEADER_T) + dataLen) &&
            gsound_bt_is_rx_available())
        {
            gsound_bt_send_rx_data();
        }
    }
    osMutexRelease(gsoundBtMutexId);

    return 0;
}

uint8_t gsound_custom_bt_on_channel_connected_ind(uint8_t *addr, uint8_t chnl)
{
    GSoundBTAddr gAddr = {0};
    gsound_convert_bdaddr_to_gsound(addr,
                                    &gAddr);

    GSoundStatus status = bt_interface->gsound_bt_on_connect_ind((GSoundChannelType)chnl, &gAddr);
    GLOG_I("%s %d", __func__, status);

    return (uint8_t)(status);
}

uint8_t gsound_custom_bt_on_channel_connected(uint8_t *addr, uint8_t chnl)
{
    GSoundBTAddr gAddr = {0};
    gsound_convert_bdaddr_to_gsound(addr,
                                    &gAddr);

    GSoundStatus status = bt_interface->gsound_bt_on_connected((GSoundChannelType)chnl, &gAddr);

    GLOG_I("%s %d", __func__, status);
    if (GSOUND_STATUS_OK == status)
    {
        gsound_set_rfcomm_connect_state(chnl, CHANNEL_CONNECTED);
    }

    return (uint8_t)status;
}

void gsound_custom_bt_on_channel_disconnected(uint8_t chnl)
{
    GLOG_I("%s channel:%s", __func__, gsound_chnl2str(chnl));

    gsound_set_rfcomm_connect_state(chnl, CHANNEL_DISCONNECTED);
    bt_interface->gsound_bt_on_channel_disconnect((GSoundChannelType)chnl);
}

static void gsound_inform_available_rfcomm_tx_buf(void)
{
    bt_interface->gsound_bt_on_tx_available(GSOUND_CHANNEL_CONTROL);
    bt_interface->gsound_bt_on_tx_available(GSOUND_CHANNEL_AUDIO);
}

static int gsound_spp_callback(const bt_bdaddr_t *remote, bt_spp_event_t event, bt_spp_callback_param_t *param)
{
    bt_spp_channel_t *spp_chan = param->spp_chan;
    bt_spp_port_t *port = spp_chan->port;
    uint8_t device_id = spp_chan->device_id;
    uint8_t instanceIndex = port->server_instance_id;
    uint16_t connHandle = spp_chan->connhdl;
    uint8_t *pBtAddr = spp_chan->remote.address;
    GsoundRFcommServiceEnv_t *pGsoundRFCommService = NULL;

    GLOG_I("gsound spp event 0x%0x, hcihandle 0x%x service index %d ch %d",
           event,
           connHandle,
           instanceIndex,
           port->local_server_channel);

    switch (event)
    {
    case BT_SPP_EVENT_OPENED:
    {
        pGsoundRFCommService =
        gsound_bt_get_service_by_chnl(port->local_server_channel, true, false);

        if(pGsoundRFCommService == NULL)
        {
            return BT_STS_FAILED;
        }

        if(gsound_bt_is_channel_connected(pGsoundRFCommService->channelType))
        {
            GLOG_I("already connected one phone!");
            return BT_STS_FAILED;
        }

        app_tws_ibrt_spp_bisto_save_ctx((bt_bdaddr_t *)pBtAddr, spp_chan);

        GLOG_I("Connected SPP Bd addr:");
        DUMP8("%02x ", pBtAddr, BT_ADDR_OUTPUT_PRINT_NUM);

        gsound_bt_buf_init();
        gsound_bt_set_rx_available(true);
        gsound_set_connected_bd_addr(pBtAddr);
        gsound_set_bt_connection_state(true);
        pGsoundRFCommService->isSppConnected = true;
        pGsoundRFCommService->spp_chan = spp_chan;
        memcpy(pGsoundRFCommService->gAddr, pBtAddr, 6);

        if (pGsoundRFCommService->channelType == GSOUND_CHANNEL_AUDIO)
        {
            ai_manager_update_gsound_spp_info(pBtAddr, device_id, BT_SOCKET_STATE_OPENED);
        }

        if(bt_interface) {
            GSoundBTAddr gsound_addr;
            gsound_convert_bdaddr_to_gsound((void const *)pBtAddr, &gsound_addr);
            bt_interface->gsound_bt_on_link_change(GSOUND_TARGET_BT_LINK_CONNECTED,
                                                   &gsound_addr);
        }

        if (GSOUND_STATUS_OK != gsound_custom_bt_on_channel_connected_ind(pBtAddr,
                                                                          (uint8_t)pGsoundRFCommService->channelType))
        {
            pGsoundRFCommService->gsoundConnectionAllowed = false;
        }
        else
        {
            pGsoundRFCommService->gsoundConnectionAllowed = true;
        }

        if (pGsoundRFCommService->gsoundConnectionAllowed &&
            GSOUND_STATUS_OK == gsound_custom_bt_on_channel_connected(pBtAddr,
                                                                      (uint8_t)pGsoundRFCommService->channelType))
        {
            if (app_ai_manager_is_in_multi_ai_mode())
            {
                if (pGsoundRFCommService->channelType == GSOUND_CHANNEL_AUDIO)
                {
#ifdef __AI_VOICE__
                    ai_manager_set_spec_connected_status(AI_SPEC_GSOUND, 1);
#endif
                }
            }

            //gsound_rfcomm_reset_tx_buf(pGsoundRFCommService->channelType);
        }
        else
        {
            GLOG_I("connect Disallowed");
            GLOG_I("voicepath bt connect state: %d", gsound_is_bt_connected());
            // if (gsound_is_bt_connected())
            // {
            //     GLOG_I("connected bdAddr: 0x");
            //     DUMP8("%02x ", gsound_get_connected_bd_addr(), BT_ADDR_OUTPUT_PRINT_NUM);
            // }

            // if (gsound_get_rfcomm_connect_state(pGsoundRFCommService->channelType))
            // {
            //     GLOG_I("already connected");
            //     return false;
            // }

            // app_rfcomm_close(pGsoundRFCommService->serviceIndex);
            // gsound_rfcomm_reset_tx_buf(pGsoundRFCommService->channelType);
        }
        break;
    }
    case BT_SPP_EVENT_CLOSED:
    {
        pGsoundRFCommService = gsound_bt_get_service_by_chnl(port->local_server_channel, true, true);
        if(pGsoundRFCommService == NULL)
        {
            GLOG_I("channel is not connected!");
            return BT_STS_FAILED;
        }

        if (GSOUND_CHANNEL_AUDIO == pGsoundRFCommService->channelType)
        {
            ai_manager_update_gsound_spp_info(pBtAddr, device_id, BT_SOCKET_STATE_CLOSED);
        }
        if (pGsoundRFCommService->gsoundConnectionAllowed)
        {
            gsound_custom_audio_close_mic();
            // Do not notify gsound that the channel disconnected if gsound was the
            // did not allow the connection in the first place (it aborts).
            gsound_custom_bt_on_channel_disconnected((uint8_t)(pGsoundRFCommService->channelType));

            if (app_ai_manager_is_in_multi_ai_mode())
            {
                if (pGsoundRFCommService->channelType == GSOUND_CHANNEL_AUDIO)
                {
#ifdef __AI_VOICE__
                    ai_manager_set_spec_connected_status(AI_SPEC_GSOUND, 0);
#endif
                }
            }

        }

        //gsound_bt_rx_complete_handler(pGsoundRFCommService->channelType, NULL, 0);
        pGsoundRFCommService->isSppConnected = false;
        memset(pGsoundRFCommService->gAddr, 0, 6);

        //gsound_custom_ota_exit(OTA_PATH_BT);
        break;
    }
    case BT_SPP_EVENT_TX_DONE:
    {
        gsound_inform_available_rfcomm_tx_buf();
        break;
    }
    case BT_SPP_EVENT_RX_DATA:
    {
        gsound_spp_handle_data_event_func(remote, event, param);
        break;
    }
    default:
        break;
    }
    return BT_STS_SUCCESS;
}

static int app_spp_gsound_accept_channel_request(const bt_bdaddr_t *remote, bt_socket_event_t event, bt_socket_accept_t *accept)
{
    bool ret = true;
    uint8_t server_channel = accept->local_server_channel;
    uint8_t connection_path = gsound_custom_get_connection_path();

    if((server_channel == RFCOMM_CHANNEL_GS_CONTROL) || (server_channel == RFCOMM_CHANNEL_GS_AUDIO)){
        if(connection_path == CONNECTION_BLE){
            ret = false;
        }else if(gsound_bt_is_all_connected()){
            ret = false;
        }
    }
    TRACE(5,"%s ret = %d chnnl = %d path = %d full ?= %d",__func__,ret,server_channel,connection_path,gsound_bt_is_all_connected());
    return ret;
}

// return -1 means no bt connection exists
static int8_t _get_bd_id(bt_bdaddr_t *addr)
{
    return app_bt_get_device_id_byaddr(addr);
}

int gsound_custom_bt_stop_sniff()
{
    bt_bdaddr_t besAddr;
    memcpy(besAddr.address, gsound_get_connected_bd_addr(), 6);
    int8_t id = _get_bd_id(&besAddr);

#ifdef IBRT
    app_ibrt_if_prevent_sniff_set(gsound_get_connected_bd_addr(), OTA_ONGOING);
#else
    if (-1 != id)
    {
        app_bt_active_mode_set(BT_ACTIVE_MODE_KEEP_USER_AI_VOICE_STREAM, id);
    }
#endif

    return id;
}

int gsound_custom_bt_allow_sniff(void)
{
    bt_bdaddr_t besAddr;
    memcpy(besAddr.address, gsound_get_connected_bd_addr(), 6);
    int8_t id = _get_bd_id(&besAddr);

#ifdef IBRT
    app_ibrt_if_prevent_sniff_clear(gsound_get_connected_bd_addr(), OTA_ONGOING);
#else
    if (-1 != id)
    {
        app_bt_active_mode_clear(BT_ACTIVE_MODE_KEEP_USER_AI_VOICE_STREAM, id);
    }
#endif

    return id;
}

void gsound_custom_bt_link_disconnected_handler(uint8_t *pBtAddr)
{
    GLOG_I("%s", __func__);
    DUMP8("%02x ", pBtAddr, BT_ADDR_OUTPUT_PRINT_NUM);

    if (memcmp(pBtAddr, gsound_get_connected_bd_addr(), 6))
    {
        GLOG_I("device is not connected:");
        DUMP8("0x%02x ", pBtAddr, BT_ADDR_OUTPUT_PRINT_NUM);
        return;
    }

    if (!gsound_is_bt_connected())
    {
        GLOG_I("BT already disconnected");
        return;
    }

    if(!memcmp(gsound_get_connected_bd_addr(),pBtAddr, 6)) {
        GSoundBTAddr gsound_addr;
        gsound_convert_bdaddr_to_gsound((void const *)pBtAddr, &gsound_addr);
    
        // inform glib that gsound channels are disconnected
        gsound_custom_bt_on_channel_disconnected(GSOUND_CHANNEL_AUDIO);
        gsound_custom_bt_on_channel_disconnected(GSOUND_CHANNEL_CONTROL);
        gsound_set_bt_connection_state(false);
        bt_interface->gsound_bt_on_link_change(GSOUND_TARGET_BT_LINK_DISCONNECTED,
                                               &gsound_addr);
    }
    gsound_tws_role_update(app_ai_tws_get_local_role());
}

void gsound_custom_bt_link_connected_handler(uint8_t *pBtAddr)
{
    GLOG_I("%s", __func__);
    DUMP8("%02x ", pBtAddr, BT_ADDR_OUTPUT_PRINT_NUM);

    if (gsound_is_bt_connected() && (gsound_get_bt_connect_state() || gsound_get_ble_connect_state()))
    {
        GLOG_I("ready connected.");
        return;
    }

    gsound_set_connected_bd_addr(pBtAddr);
    gsound_set_bt_connection_state(true);

    gsound_tws_role_update(app_ai_tws_get_local_role());
}

#ifdef __BT_ONE_BRING_TWO__
static uint8_t last_paused_device_id = 0;

static bool app_gsound_is_bt_in_bisto(void const *bt_addr, void const *bisto_addr)
{
    return (memcmp(bt_addr, bisto_addr, BTIF_BD_ADDR_SIZE) == 0);
}

static APP_A2DP_STREAMING_STATE_E a2dp_get_streaming_state(uint8_t deviceId)
{
    if (app_bt_is_music_player_working(deviceId))
    {
        return APP_A2DP_MEDIA_PLAYING;
    }
    else if (app_bt_is_a2dp_streaming(deviceId))
    {
        return APP_A2DP_STREAMING_ONLY;
    }
    else
    {
        return APP_A2DP_IDLE;
    }
}

static APP_A2DP_STREAMING_STATE_E app_gsound_get_stream_state_of_device_not_in_bisto(
    uint8_t *deviceIdNotInBisto, void const *bisto_addr)
{
    uint8_t deviceId;
    uint8_t bdAddr[BTIF_BD_ADDR_SIZE];
    APP_A2DP_STREAMING_STATE_E streamState = APP_A2DP_IDLE;
    for (deviceId = 0; deviceId < BT_DEVICE_NUM; deviceId++)
    {
        if (app_bt_get_device_bdaddr(deviceId, bdAddr))
        {
            GLOG_I("MP: Get bdaddr of dev %d:", deviceId);
            DUMP8("%02x ", bdAddr, BT_ADDR_OUTPUT_PRINT_NUM);
            if (app_gsound_is_bt_in_bisto(bdAddr, bisto_addr))
            {
                GLOG_I("MP: Device bisto, Ignore.");
                continue;
            }
            else
            {
                GLOG_I("MP: Device is not in bisto.");
                *deviceIdNotInBisto = deviceId;
                streamState = a2dp_get_streaming_state(deviceId);
                GLOG_I("MP: Device stream state is %d", streamState);
            }
        }
    }

    GLOG_I("MP: Inactive=Final State %d", streamState);
    return streamState;
}
#endif

/**
 * Pause and/or suspend any streaming non-Bisto device.
 *
 * If called multiple times, it will retain the paused
 * state of the first call until post_voice_query is called.
 */
bool app_gsound_a2dp_streaming_handler_pre_voice_query(void const *bisto_addr)
{
#ifdef __BT_ONE_BRING_TWO__
    uint8_t deviceIdNotInBisto = 0;
    APP_A2DP_STREAMING_STATE_E streamingState =
        app_gsound_get_stream_state_of_device_not_in_bisto(&deviceIdNotInBisto, bisto_addr);
    if (APP_A2DP_MEDIA_PLAYING == streamingState)
    {
        // pause the media player now, and resume it after the voice query
        GLOG_I("MP: Inactive=Media streaming");
        bool isPaused = app_bt_pause_music_player(deviceIdNotInBisto);
        if (isPaused)
        {
            last_paused_device_id = deviceIdNotInBisto;
            GLOG_I("MP: Inactive=Force-Paused %d", last_paused_device_id);
            return true;
        }
    }
    else if (APP_A2DP_STREAMING_ONLY == streamingState)
    {
        GLOG_I("MP: Inactive=Non-media streaming");
        // stop the a2dp streaming only if the media player is not on
        // Return false so we don't incorrectly resume any media play-back
        app_bt_suspend_a2dp_streaming(deviceIdNotInBisto);
    }
    else
    {
        GLOG_I("MP: Inactive=Idle");
    }

    return false;
#else
    return false;
#endif
}

void app_gsound_a2dp_streaming_handler_post_voice_query(void)
{
#ifdef __BT_ONE_BRING_TWO__
    if (app_bt_get_device(last_paused_device_id)->a2dp_need_resume_flag)
    {
        GLOG_I("MP: Inactive Resume! %d", last_paused_device_id);
        app_bt_resume_music_player(last_paused_device_id);
    }
    else
    {
        GLOG_I("MP: Inactive no-resume");
    }
#endif
}

uint8_t gsound_bt_init(void)
{
    GLOG_I("[%s]+++", __func__);

    bt_status_t status = BT_STS_SUCCESS;

    if (gsound_is_bt_init_done() != GSOUND_INIT_DONE)
    {

#if defined(DUMP_CRASH_ENABLE)
        gsound_crash_dump_init();
#endif

        bt_spp_server_config_t config = {0};
        config.app_spp_callback = gsound_spp_callback;
        config.accept_callback = app_spp_gsound_accept_channel_request;

        config.local_server_channel = RFCOMM_CHANNEL_GS_CONTROL;
        config.local_spp_128bit_uuid = GSOUND_DATA_UUID_128;
        bt_spp_server_listen(&config);
        bt_spp_set_app_layer_give_credit(RFCOMM_CHANNEL_GS_CONTROL, true);

        config.local_server_channel = RFCOMM_CHANNEL_GS_AUDIO;
        config.local_spp_128bit_uuid = GSOUND_VOICE_UUID_128;
        bt_spp_server_listen(&config);
        bt_spp_set_app_layer_give_credit(RFCOMM_CHANNEL_GS_AUDIO, true);

        for(uint8_t i = 0; i < BT_DEVICE_NUM; i++)
        {
            gsoundControl[i].channelType = GSOUND_CHANNEL_CONTROL;
            gsoundControl[i].spp_chan = NULL;
            gsoundControl[i].isSppConnected = false;
            gsoundControl[i].gsoundConnectionAllowed = false;

            gsoundAudio[i].channelType = GSOUND_CHANNEL_AUDIO;
            gsoundAudio[i].spp_chan = NULL;
            gsoundAudio[i].isSppConnected = false;
            gsoundAudio[i].gsoundConnectionAllowed = false;
        }
        gsound_bt_buf_init();
        gsound_set_bt_init_state(true);

        if (gsoundBtMutexId == NULL)
        {
            gsoundBtMutexId = osMutexCreate((osMutex(gsoundBtMutex)));
        }
        gsound_bt_set_rx_available(true);
    }
    else
    {
        GLOG_I("Already init.");
    }

    GLOG_I("[%s]---", __func__);
    return status;
}

void gsound_bt_update_channel_connection_state(uint8_t channel,uint8_t *connBdAddr,bool state)
{
    GLOG_I("BT_%s_%s", gsound_chnl2str(channel), state ? "CONNECTED" : "DISCONNECTED");
    GsoundRFcommServiceEnv_t *channelService = gsound_bt_get_service_by_chnl(channel, false, true);

    if(channelService && \
        (!memcmp(channelService->gAddr, connBdAddr, 6)))
    {
        channelService->isSppConnected = state;
    }
}

bool gsound_bt_is_channel_connected(uint8_t channel)
{
    bool ret = false;
    if (gsound_bt_get_service_by_chnl(channel, false, true)) {
        ret = true;
    }

    return ret;
}

bool gsound_bt_is_all_connected(void)
{
    return (gsound_bt_is_channel_connected(GSOUND_CHANNEL_CONTROL) &&
            gsound_bt_is_channel_connected(GSOUND_CHANNEL_AUDIO));
}

bool gsound_bt_is_any_connected(void)
{
    return (gsound_bt_is_channel_connected(GSOUND_CHANNEL_CONTROL) ||
            gsound_bt_is_channel_connected(GSOUND_CHANNEL_AUDIO));
}

static void _disconnect_channel(uint8_t channel)
{
    GsoundRFcommServiceEnv_t *channelService = gsound_bt_get_service_by_chnl(channel, false, true);

    if (channelService) {
        bt_spp_disconnect(channelService->spp_chan->rfcomm_handle, 0);
    } else {
        GLOG_I("%s already disconnected", gsound_chnl2str(channel));
    }
}

void gsound_custom_bt_disconnect_all_channel(void)
{
    _disconnect_channel(GSOUND_CHANNEL_CONTROL);
    _disconnect_channel(GSOUND_CHANNEL_AUDIO);
}

void gsound_bt_rx_complete_handler(GSoundChannelType type, const uint8_t *buffer, uint32_t len)
{
    GsoundRFcommServiceEnv_t *gsrv = gsound_bt_get_service_by_chnl(type, false, true);
    TRACE(3, "%s, buf:%p len:%d", __func__, buffer, len);

    osMutexWait(gsoundBtMutexId, osWaitForever);
    if (buffer && (len != 0))
    {
        gsound_bt_buf_pop((uint8_t *)buffer, len);
    }
    gsound_bt_set_rx_available(true);

    gsound_bt_send_rx_data();
    osMutexRelease(gsoundBtMutexId);

    bt_spp_give_handled_credits(gsrv->spp_chan->rfcomm_handle, 1);

}

void gsound_bt_register_target_handle(const void *handler)
{
    bt_interface = (const GSoundBtInterface *)handler;
}

uint16_t gsound_bt_get_mtu(void)
{
    return ((BT_SPP_MAX_TX_MTU)-6 - 4);
}

bt_status_t gsound_bt_transmit(uint8_t channel,
                           const uint8_t *ptrData,
                           uint32_t length)
{
    uint16_t len = (uint16_t)length;
    GsoundRFcommServiceEnv_t *service = gsound_bt_get_service_by_chnl(channel, false, true);
    if (service == NULL)
    {
        return BT_STS_FAILED;
    }

    for(uint8_t i = 0; i < BT_DEVICE_NUM; i++)
    {
        if(service[i].isSppConnected)
        {
            if (BT_STS_FAILED == bt_spp_write(service[i].spp_chan->rfcomm_handle, ptrData, len))
            {
                return BT_STS_FAILED;
            }
            break;
        }
    }

    return BT_STS_SUCCESS;
}

bool gsound_custom_bt_is_hfp_active(void)
{
    bool ret = (bool)bt_media_is_media_active_by_type(BT_STREAM_VOICE);
    return ret;
}

bool gsound_bt_is_rf_channel(const uint8_t *uuid)
{
    bool ret = false;
    if(!memcmp(uuid, GSOUND_DATA_UUID_128_BE, 16) || (!memcmp(uuid, GSOUND_VOICE_UUID_128_BE, 16)))
    {
        ret = true;
    }
    return ret;
}
