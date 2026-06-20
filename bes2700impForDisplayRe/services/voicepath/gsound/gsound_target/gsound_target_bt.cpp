#include "cmsis.h"
#include "cmsis_os.h"
#include "bluetooth_bt_api.h"
#include "app_bt.h"
#include "btapp.h"
#include "spp_api.h"
#include "hfp_api.h"
#include "gsound_dbg.h"
#include "gsound_custom.h"
#include "gsound_custom_bt.h"
#include "app_bt_func.h"

#ifdef IBRT
#include "app_tws_ibrt.h"
#include "app_ibrt_internal.h"
#include "earbud_ux_api.h"
#endif

extern "C" bt_bdaddr_t *app_bt_get_remote_device_address(uint8_t device_id);

uint8_t GSoundTargetBtNumDevicesConnected(void)
{
    return app_bt_count_connected_device();
}

#if 0

typedef struct
{
    uint16_t rfcomm_tx_buf_chunk_cnt;
    uint16_t rfcomm_tx_buf_allocated_status_mask;
    uint16_t rfcomm_tx_buf_chunk_size;
    uint16_t rfcomm_tx_buf_allocated_status;
    uint8_t *rfcomm_tx_buf_ptr;
} GSOUND_RFCOMM_TX_BUF_CONFIG_T;

#define GSOUND_RFCOMM_CONTROL_TX_BUF_CHUNK_CNT 3
#define GSOUND_RFCOMM_CONTROL_TX_BUF_ALLOCTED_STATUS_MASK ((1 << GSOUND_RFCOMM_CONTROL_TX_BUF_CHUNK_CNT) - 1)
#define GSOUND_RFCOMM_CONTROL_TX_BUF_CHUNK_SIZE MAXIMUM_GSOUND_RFCOMM_TX_SIZE
#define GSOUND_RFCOMM_CONTROL_TX_BUF_SIZE (GSOUND_RFCOMM_CONTROL_TX_BUF_CHUNK_CNT * GSOUND_RFCOMM_CONTROL_TX_BUF_CHUNK_SIZE)

static uint8_t gsound_rfcomm_control_tx_buf[GSOUND_RFCOMM_CONTROL_TX_BUF_CHUNK_CNT][GSOUND_RFCOMM_CONTROL_TX_BUF_CHUNK_SIZE];

#define GSOUND_RFCOMM_TX_BUF_CHUNK_CNT 4
#define GSOUND_RFCOMM_TX_BUF_ALLOCTED_STATUS_MASK ((1 << GSOUND_RFCOMM_TX_BUF_CHUNK_CNT) - 1)
#define GSOUND_RFCOMM_TX_BUF_CHUNK_SIZE MAXIMUM_GSOUND_RFCOMM_TX_SIZE
#define GSOUND_RFCOMM_TX_BUF_SIZE (GSOUND_RFCOMM_TX_BUF_CHUNK_CNT * GSOUND_RFCOMM_TX_BUF_CHUNK_SIZE)

static uint8_t gsound_rfcomm_tx_buf[GSOUND_RFCOMM_TX_BUF_CHUNK_CNT][GSOUND_RFCOMM_TX_BUF_CHUNK_SIZE];

static GSOUND_RFCOMM_TX_BUF_CONFIG_T audio_tx_buf_config[GSOUND_NUM_CHANNEL_TYPES] = {
    {
        GSOUND_RFCOMM_CONTROL_TX_BUF_CHUNK_CNT,
        GSOUND_RFCOMM_CONTROL_TX_BUF_ALLOCTED_STATUS_MASK,
        GSOUND_RFCOMM_CONTROL_TX_BUF_CHUNK_SIZE,
        0,
        (uint8_t *)&gsound_rfcomm_control_tx_buf,
    },

    {
        GSOUND_RFCOMM_TX_BUF_CHUNK_CNT,
        GSOUND_RFCOMM_TX_BUF_ALLOCTED_STATUS_MASK,
        GSOUND_RFCOMM_TX_BUF_CHUNK_SIZE,
        0,
        (uint8_t *)&gsound_rfcomm_tx_buf,
    },
};

static uint8_t *gsound_rfcomm_tx_buf_addr(GSoundChannelType channel, uint32_t chunk)
{
    GSOUND_RFCOMM_TX_BUF_CONFIG_T *pTxBufConfig =
        &audio_tx_buf_config[channel];

    return (uint8_t *)&(pTxBufConfig->rfcomm_tx_buf_ptr[chunk * pTxBufConfig->rfcomm_tx_buf_chunk_size]);
}

static int32_t gsound_rfcomm_alloc_tx_chunk(GSoundChannelType channel)
{
    GSOUND_RFCOMM_TX_BUF_CONFIG_T *pTxBufConfig =
        &audio_tx_buf_config[channel];

    uint32_t lock = int_lock_global();

    if (pTxBufConfig->rfcomm_tx_buf_allocated_status_mask ==
        pTxBufConfig->rfcomm_tx_buf_allocated_status)
    {
        int_unlock_global(lock);
        return -1;
    }

    for (int32_t index = 0; index < pTxBufConfig->rfcomm_tx_buf_chunk_cnt; index++)
    {
        if (!(pTxBufConfig->rfcomm_tx_buf_allocated_status & (1 << index)))
        {
            pTxBufConfig->rfcomm_tx_buf_allocated_status |= (1 << index);
            int_unlock_global(lock);
            return index;
        }
    }

    int_unlock_global(lock);
    return -1;
}

bool gsound_rfcomm_free_tx_chunk(GSoundChannelType channel, uint8_t *ptr)
{
    GSOUND_RFCOMM_TX_BUF_CONFIG_T *pTxBufConfig =
        &audio_tx_buf_config[channel];

    uint32_t lock = int_lock_global();

    if (0 == pTxBufConfig->rfcomm_tx_buf_allocated_status)
    {
        int_unlock_global(lock);
        return false;
    }

    for (uint32_t index = 0; index < pTxBufConfig->rfcomm_tx_buf_chunk_cnt; index++)
    {
        if (gsound_rfcomm_tx_buf_addr(channel, index) == ptr)
        {
            pTxBufConfig->rfcomm_tx_buf_allocated_status &= (~(1 << index));
            int_unlock_global(lock);
            return true;
        }
    }

    int_unlock_global(lock);
    return false;
}

void gsound_rfcomm_reset_tx_buf(GSoundChannelType channel)
{
    GSOUND_RFCOMM_TX_BUF_CONFIG_T *pTxBufConfig = &audio_tx_buf_config[channel];

    uint32_t lock = int_lock_global();

    pTxBufConfig->rfcomm_tx_buf_allocated_status = 0;
    int_unlock_global(lock);
}

extern "C" uint32_t gsound_rfcomm_get_available_tx_packet_cnt(void)
{
    uint32_t freeTxPacketsCnt = 0;

    GSOUND_RFCOMM_TX_BUF_CONFIG_T *pTxBufConfig =
        &audio_tx_buf_config[GSOUND_CHANNEL_AUDIO];

    for (int32_t index = 0; index < pTxBufConfig->rfcomm_tx_buf_chunk_cnt; index++)
    {
        if (!(pTxBufConfig->rfcomm_tx_buf_allocated_status & (1 << index)))
        {
            freeTxPacketsCnt++;
        }
    }

    return freeTxPacketsCnt;
}

// static uint32_t gsound_rfcomm_control_get_available_tx_packet_cnt(void)
// {
//     uint32_t freeTxPacketsCnt = 0;

//     GSOUND_RFCOMM_TX_BUF_CONFIG_T *pTxBufConfig =
//         &audio_tx_buf_config[GSOUND_CHANNEL_CONTROL];

//     for (int32_t index = 0; index < pTxBufConfig->rfcomm_tx_buf_chunk_cnt; index++)
//     {
//         if (!(pTxBufConfig->rfcomm_tx_buf_allocated_status & (1 << index)))
//         {
//             freeTxPacketsCnt++;
//         }
//     }

//     return freeTxPacketsCnt;
// }

#endif


GSoundStatus GSoundTargetBtTransmit(GSoundChannelType channel,
                                    const uint8_t *ptrData,
                                    uint32_t length,
                                    uint32_t *bytes_consumed)
{
    GLOG_I("GSound BT Transmit Callback: channel=%s, len=%d",
           gsound_chnl2str(channel),
           length);
    bt_status_t status = BT_STS_FAILED;
    *bytes_consumed = 0;

#ifdef IBRT
    // slave is not allowed to send data out
    if (IBRT_SLAVE == app_ibrt_if_get_ui_role())
    {
        GLOG_I("slave is not allowed to send data.");
        *bytes_consumed = length;
        return GSOUND_STATUS_OK;
    }
#endif

    if (!gsound_bt_is_channel_connected(channel))
    {
        GLOG_I("%s not connected", gsound_chnl2str(channel));
        return GSOUND_STATUS_ERROR;
    }

    ASSERT(GSoundTargetOsMqActive(), "mq not active");

    status = gsound_bt_transmit(channel, ptrData, length);

    if (status == BT_STS_SUCCESS)
    {
        *bytes_consumed = length;
        return GSOUND_STATUS_OK;
    }
    else
    {
        return GSOUND_STATUS_OUT_OF_MEMORY;
    }
}

GSoundStatus GSoundTargetBtInit(const GSoundBtInterface *handler)
{
    gsound_bt_register_target_handle(handler);

    return GSOUND_STATUS_OK;
}

uint16_t GSoundTargetBtGetMtu(void)
{
    // TODO(jkessinger): Should this default to the real minimum value until we have
    // negotiated MTU, or does the RFCOMM guarantee negotiation???
    // Subtract the RFCOMM and L2CAP headers bytes.
    return gsound_bt_get_mtu();
}

GSoundStatus GSoundTargetBtHfpDial(const GSoundTargetBtHfpNumber *msg,
                                   const GSoundBTAddr *gsound_addr)
{
    btif_hf_channel_t *hf_chan = app_bt_get_device(BT_DEVICE_ID_1)->hf_channel;
    static uint8_t number[GSOUND_TARGET_BT_HFP_NUMBER_MAX];

    memcpy(number, msg->number, msg->size > sizeof(number) ? sizeof(number) : msg->size);
    btif_hf_dial_number(hf_chan, number, msg->size);

    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetBtMapSendSms(const GSoundTargetBtMapSmsNumber *number,
                                      const GSoundTargetBtMapSmsMessage *message,
                                      const GSoundBTAddr *gsound_addr)
{
#ifdef BT_MAP_SUPPORT
    bt_bdaddr_t remote;
    gsound_convert_bdaddr_to_plateform(gsound_addr, remote.address);
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(&remote);
    struct bt_map_sms_t sms;
    if (!curr_device->acl_is_connected)
    {
        GLOG_I("gsound target map client send sms: acl not connected %02x:%02x:...:%02x",
            remote.address[0], remote.address[1], remote.address[5]);
        return GSOUND_STATUS_ERROR;
    }
    sms.tel = (char *)number->number;
    sms.tel_len = (uint16_t)number->size;
    sms.msg = (char *)message->message;
    sms.msg_len = (uint16_t)message->size;
    bes_bt_map_send_sms(&remote, &sms);
#endif
    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetBtPauseSecondary(const GSoundBTAddr *gsound_addr)
{
    /*
   * If more than one device connected and
   * if we are streaming on secondary device (i.e. multipoint use-case)
   * We need to pause A2DP on that device and flag it to resume
   * when query completes.
   */
    if (GSoundTargetBtNumDevicesConnected() > 1)
    {
        uint8_t bisto_addr[BTIF_BD_ADDR_SIZE];

        gsound_convert_bdaddr_to_plateform(gsound_addr, bisto_addr);
        GLOG_I("GSound: Multi-point detected: %d", GSoundTargetBtNumDevicesConnected());

        // This call will pause inactive device if streaming
        app_gsound_a2dp_streaming_handler_pre_voice_query(bisto_addr);
    }

    return GSOUND_STATUS_OK;
}

GSoundStatus GSoundTargetBtResumeSecondary(const GSoundBTAddr *gsound_addr)
{
    app_gsound_a2dp_streaming_handler_post_voice_query();
    return GSOUND_STATUS_OK;
}

void GSoundTargetBtA2dpPause(void)
{
}

void GSoundTargetBtA2dpPlay(GSoundBTAddr *addr)
{
}

bool GSoundTargetBtA2dpIsStreaming(GSoundBTAddr *gsound_addr)
{
    return false;
}

bool GSoundTargetBtHfpIsActive(void)
{
    bool ret = gsound_custom_bt_is_hfp_active();
    return ret;
}

bool GSoundTargetBtIsConnected(GSoundBTAddr const *gsound_addr)
{
    uint8_t connectedBtAddr[BTIF_BD_ADDR_SIZE];
    uint8_t convertedBtAddr[BTIF_BD_ADDR_SIZE];

    gsound_convert_bdaddr_to_plateform(gsound_addr, convertedBtAddr);

    GLOG_I("converted add:");
    DUMP8("%02x ", convertedBtAddr, BT_ADDR_OUTPUT_PRINT_NUM);

    for (uint8_t devIndex = 0; devIndex < BT_DEVICE_NUM; devIndex++)
    {
        if (app_bt_get_device_bdaddr(devIndex, connectedBtAddr))
        {
            DUMP8("%02x ", connectedBtAddr, BT_ADDR_OUTPUT_PRINT_NUM);
            if (!memcmp(connectedBtAddr, convertedBtAddr, BTIF_BD_ADDR_SIZE))
            {
                return true;
            }
        }
    }

    return false;
}

void GSoundTargetBtRxComplete(GSoundChannelType type,
                              const uint8_t *buffer,
                              uint32_t len)
{
    gsound_bt_rx_complete_handler(type, buffer, len);
}

GSoundStatus GSoundTargetBtGetActiveAudioSource(
                        GSoundActiveAudioSource *audio_source_out)
{
    *audio_source_out = GSOUND_ACTIVE_AUDIO_SOURCE_UNSUPPORTED;
    return GSOUND_STATUS_OK;
}