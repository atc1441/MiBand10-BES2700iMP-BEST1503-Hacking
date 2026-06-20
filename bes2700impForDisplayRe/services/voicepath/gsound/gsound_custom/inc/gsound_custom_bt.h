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


#ifndef __GSOUND_CUSTOM_BT_H__
#define __GSOUND_CUSTOM_BT_H__

#ifdef __cplusplus
extern "C"{
#endif

/*****************************header include********************************/
#include "gsound.h"
#include "bluetooth_bt_api.h"
/******************************macro defination*****************************/

#define MAXIMUM_GSOUND_RFCOMM_TX_SIZE (( BT_SPP_MAX_TX_MTU )-10)

#define SPP_TX_BUF_DATA_SIZE        4096
#define SPP_DATA_MAX_PACKET_NUM \
    (( int )(SPP_TX_BUF_DATA_SIZE / MAXIMUM_GSOUND_RFCOMM_TX_SIZE))

#define SPP_TX_BUF_VOICE_SIZE       4096
#define SPP_VOICE_MAX_PACKET_NUM \
    (( int )(SPP_TX_BUF_VOICE_SIZE / MAXIMUM_GSOUND_RFCOMM_TX_SIZE))

#define GSOUND_RX_BUF_SIZE             (RFCOMM_DEFAULT_MTU * 5)
#define GSOUND_RX_PACKET_SIZE          (RFCOMM_DEFAULT_MTU)

/******************************type defination******************************/
typedef enum {
    APP_A2DP_MEDIA_PLAYING  = 0,
    APP_A2DP_STREAMING_ONLY = 1,
    APP_A2DP_IDLE           = 2,
} APP_A2DP_STREAMING_STATE_E;


typedef struct {
    uint8_t chnl;
    uint8_t reserved;
    uint16_t size;
}GSOUND_BUF_HEADER_T;

/****************************function declearation**************************/
/*---------------------------------------------------------------------------
 *            gsound_bt_init
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    Create and register the SPP endpoints
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
uint8_t gsound_bt_init(void);

/*---------------------------------------------------------------------------
 *            gsound_custom_bt_on_channel_connected_ind
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    update channel connected indication state to libgsound
 *
 * Parameters:
 *    addr - pointer of connected bluetooth device address
 *    chnl - channel type of gsound channel
 *
 * Return:
 *    status
 */
uint8_t gsound_custom_bt_on_channel_connected_ind(uint8_t *addr, uint8_t chnl);

/*---------------------------------------------------------------------------
 *            gsound_custom_bt_on_channel_connected
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    update channel connected state to libgsound
 *
 * Parameters:
 *    addr - pointer of connected bluetooth device address
 *    chnl - channel type of gsound channel
 *
 * Return:
 *    status
 */
uint8_t gsound_custom_bt_on_channel_connected(uint8_t *addr, uint8_t chnl);

/*---------------------------------------------------------------------------
 *            gsound_custom_bt_on_channel_disconnected
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    update channel disconnected state to libgsound
 *
 * Parameters:
 *    chnl - channel type of gsound channel
 *
 * Return:
 *    void
 */
void gsound_custom_bt_on_channel_disconnected(uint8_t chnl);

/*---------------------------------------------------------------------------
 *            gsound_bt_update_channel_connection_state
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    update the channel connection state
 *
 * Parameters:
 *    channel - gsound channel type
 *    uint8_t *connBdAddr
 *    state - true for connection, false for disconnection
 *
 * Return:
 *    void
 */
void gsound_bt_update_channel_connection_state(uint8_t channel,uint8_t *connBdAddr,bool state);

/*---------------------------------------------------------------------------
 *            gsound_bt_is_channel_connected
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get the gsound connection status according to the incoming gsound channel
 *
 * Parameters:
 *    channel - gsound channel type, see @GSOUND_CHANNEL_TYPE to get more info
 *
 * Return:
 *    true - incoming channel is connected
 *    false - incoming channel is not connected
 */
bool gsound_bt_is_channel_connected(uint8_t channel);

/*---------------------------------------------------------------------------
 *            gsound_bt_is_all_connected
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    is all the gsound related channel is connected
 *
 * Parameters:
 *    void
 *
 * Return:
 *    true - all of the gsound related channel is connected
 *    false - not all of the gsound related channel is connected
 */
bool gsound_bt_is_all_connected(void);

/*---------------------------------------------------------------------------
 *            gsound_bt_is_any_connected
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    is any gsound related channel is connected
 *
 * Parameters:
 *    void
 *
 * Return:
 *    true - at least one of the gsound related channel is connected
 *    false - no gsound related channel is connected
 */
bool gsound_bt_is_any_connected(void);

/*---------------------------------------------------------------------------
 *            gsound_bt_disconnect_bt
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    disconnect gsound related spp connections
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void gsound_custom_bt_disconnect_all_channel(void);

/*---------------------------------------------------------------------------
 *            gsound_custom_bt_link_connected_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    gsound callback function of BT link is connected
 *
 * Parameters:
 *    pBtAddr - connected address
 *
 * Return:
 *    void
 */
void gsound_custom_bt_link_connected_handler(uint8_t *pBtAddr);

/*---------------------------------------------------------------------------
 *            gsound_custom_bt_link_disconnected_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    gsound callback function of BT link is disconnected
 *
 * Parameters:
 *    pBtAddr - disconnected address
 *
 * Return:
 *    void
 */
void gsound_custom_bt_link_disconnected_handler(uint8_t *pBtAddr);

/*---------------------------------------------------------------------------
 *            gsound_custom_bt_stop_sniff
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    don't allow gsound device enter sniff mode
 *
 * Parameters:
 *    void
 *
 * Return:
 *    0 - cmd send to controller
 *    -1 - gsound device not connected
 */
int gsound_custom_bt_stop_sniff(void);

/*---------------------------------------------------------------------------
 *            gsound_custom_bt_allow_sniff
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    allow gsound device enter sniff mode
 *
 * Parameters:
 *    void
 *
 * Return:
 *    0 - cmd send to controller
 *    -1 - gsound device not connected
 */
int gsound_custom_bt_allow_sniff(void);

/*---------------------------------------------------------------------------
 *            gsound_custom_bt_link_disconnected_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    gsound callback function of BT link is disconnected
 *
 * Parameters:
 *    pBtAddr - disconnected address
 *
 * Return:
 *    void
 */
void app_gsound_bt_rx_complete(GSoundChannelType type, const uint8_t *buffer, uint32_t len);

/**
 * Resume any previously paused non-Bisto stream.
 * This will result in reseting of the a2dp streaming handler state
 * After this call, nothing gets resumed unless "pre_voice_query" is called
 * again
 */
void app_gsound_a2dp_streaming_handler_post_voice_query(void);

bool gsound_target_bt_is_all_rfcomm_connected(void);

bool gsound_target_bt_is_any_rfcomm_connected(void);

void gsound_bt_rx_complete_handler(GSoundChannelType type, const uint8_t *buffer, uint32_t len);

bool app_gsound_a2dp_streaming_handler_pre_voice_query(void const *bisto_addr);

void gsound_bt_register_target_handle(const void *handler);

uint16_t gsound_bt_get_mtu(void);

bt_status_t gsound_bt_transmit(uint8_t  channel, const uint8_t *ptrData, uint32_t length);

void gsound_rfcomm_reset_tx_buf(GSoundChannelType channel);
bool gsound_rfcomm_free_tx_chunk(GSoundChannelType channel, uint8_t* ptr);

/**
 * @brief Check if HFP active
 * 
 * @return true         HFP is active
 * @return false        HFP is not active
 */
bool gsound_custom_bt_is_hfp_active(void);
bool gsound_bt_is_rf_channel(const uint8_t *uuid);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __GSOUND_CUSTOM_BT_H__ */
