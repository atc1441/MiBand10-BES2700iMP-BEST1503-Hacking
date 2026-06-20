#ifdef GFPS_ENABLED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "bluetooth.h"
#include "bt_if.h"
#include "../utils/encrypt/aes.h"
#include "bes_gap_api.h"
#include "bes_gatt_api.h"
#ifdef IBRT
#include "app_ibrt_conn_evt.h"
#include "earbud_ux_api.h"
#endif
#ifdef BT_SERVICE_ENABLE
#include "app_ui_devices_mgr.h"
#else
#include "app_ui_api.h"
#endif
#include "nvrecord_fp_account_key.h"
#include "apps.h"
#include "app_bt.h"
#include "ble_gfps.h"
#include "bes_gap_api.h"
#include "gfps_ble.h"
#include "gfps_crypto.h"
#include "gfps.h"
#include "gfps_sass.h"
#ifdef SPOT_ENABLED
#include "bt_drv_interface.h"
#include "stdlib.h"
#include "uECC_vli.h"
#include "hwtimer_list.h"
#include "hal_timer.h"
#endif
#include "app_ble.h"
#include "bes_gfps_api.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_tws_ctrl_thread.h"

/************************private macro defination***************************/
#define USE_BLE_ADDR_AS_SALT      (0)
#define USE_RANDOM_NUM_AS_SALT    (1)
#define GFPS_ACCOUNTKEY_SALT_TYPE (USE_BLE_ADDR_AS_SALT)
#define GFPS_INITIAL_ADV_RAND_SALT          0xFF
#define GFPS_ACCOUNT_KEY_IN_ADV_MAX         (4)

/************************extern function declearation***********************/
extern int rand(void);

/**********************private function declearation************************/
#ifdef SPOT_ENABLED
static uint8_t matched_account_key[FP_ACCOUNT_KEY_SIZE];
static uint8_t spot_ring_nums=0;
static uint8_t hashed_ring_key[GFPS_RING_KEY_SIZE+8];
osTimerId spot_ring_timeout_timer_id = NULL;
static void gfps_ble_spot_find_devices_ring_timeout_handler(void const *param);
osTimerDef (SPOT_FIND_DEVICES_RING_TIMEOUT, gfps_ble_spot_find_devices_ring_timeout_handler);
static HWTIMER_ID spot_EID_adv_timer_id = NULL;
static void gfps_ble_spot_EID_adv_time_handler(const void *param);
static HWTIMER_ID spot_EID_timeout_hard_timer_id = NULL;
static void gfps_ble_spot_EID_hard_timer_rotation_handler(void const *param);
#endif

/************************private variable defination************************/
struct gfps_ble_env_tag gfps_ble_env;
uint8_t salt_accountkey_data[16];
#ifdef SPOT_ENABLED
static struct ble_dult_cb_t gfps_ble_dult_callback = {
    .get_spot_get_mode = gfps_ble_get_spot_get_mode,
    .start_find_ringtone = gfps_start_find_ringtone,
    .stop_find_ringtone = gfps_stop_find_ringtone,
    .get_eid = gfps_ble_get_EID,
    .sha256_hash = gfps_SHA256_hash,
    .beacon_encrpt_data = gfps_beacon_encrpt_data,
};
#endif

void big_little_switch(const uint8_t *in, uint8_t *out, uint8_t len)
{
    if (len < 1)
        return;
    for (int i = 0; i < len; i++)
    {
        out[i] = in[len - i - 1];
    }
    return;
}

void gfps_ble_set_tx_power_in_adv(char rssi)
{
    gfps_ble_env.txPower[APP_GFPS_ADV_POWER_UUID_LEN-1] = rssi;
}

uint8_t *gfps_ble_get_tx_power_in_adv(void)
{
    return gfps_ble_env.txPower;
}

void gfps_ble_update_random_salt(void)
{
    gfps_ble_env.advRandSalt = (uint8_t)rand();
}

uint8_t gfps_ble_get_addr_type(void)
{
    uint8_t type;
    if (gfps_ble_env.bondMode == GFPS_BOND_OVER_BT)
    {
        type = GAPM_GEN_RSLV_ADDR;
    }
    else
    {
        type = GAPM_STATIC_ADDR;
    }
    return type;
}

void gfps_ble_get_addr(uint8_t *addr)
{
    if (!addr)
    {
        TRACE(0, "%s invalid param", __func__);
        return;
    }

    uint8_t *rpa = (uint8_t *)bes_ble_gap_get_local_rpa_by_adv_hdl(BLE_GFPS_ADV_HANDLE);
    uint8_t emptyAddr[6] = {0};
    if (rpa && memcmp(emptyAddr, rpa, 6))
    {
        memcpy(addr, rpa, 6);
    }
    else
    {
        ble_bdaddr_t bleAddr = bes_ble_gap_get_local_identity_addr(0xFF);
        memcpy(addr, bleAddr.addr, 6);
    }
}

uint8_t gfps_ble_get_connect_dev(void)
{
    return gfps_ble_env.connectionIndex;
}

uint8_t gfps_ble_generate_accountkey_data(uint8_t *outputData)
{
    uint8_t keyCount = nv_record_fp_account_key_count();
    uint8_t dataLen = 0, useKey = 0;
    uint8_t salt[APP_GFPS_ADV_LEN_SALT+1] = {0};
    uint8_t index;

    uint8_t batteryFollowingData[1 + GFPS_BATTERY_VALUE_MAX_COUNT];
    uint8_t batteryLevelCount = 0;

    uint8_t accountKeyData[32];
    uint8_t accountKeyDataLen = 2;
    uint8_t hash256Result[32];

    uint8_t sizeOfFilter;
    uint8_t FArray[2 * FP_ACCOUNT_KEY_RECORD_NUM + 3];

    uint8_t currentBleAddr[6] = {0};
#ifdef SASS_ENABLED
    uint8_t inUseKey[16] = {0};
    uint8_t VArray[FP_ACCOUNT_KEY_SIZE + APP_GFPS_ADV_LEN_SALT + GFPS_BATTERY_VALUE_MAX_COUNT+1 + SASS_ADV_LEN_MAX+1] = {0};
#ifdef SASS_SECURE_ENHACEMENT
    uint8_t sassAdv[SASS_ADV_LEN_MAX + 1] = {0};
    uint8_t sassAdvLen = SASS_ADV_LEN_MAX + 1;
#endif
#else
    uint8_t VArray[FP_ACCOUNT_KEY_SIZE + APP_GFPS_ADV_LEN_SALT + GFPS_BATTERY_VALUE_MAX_COUNT+1] = {0};
#endif
    bool findInUseKey = false;

    if (0 == keyCount)
    {
        outputData[0] = 0;
        outputData[1] = 0;
        return 2;
    }
    useKey = (keyCount >= GFPS_ACCOUNT_KEY_IN_ADV_MAX) ? GFPS_ACCOUNT_KEY_IN_ADV_MAX : keyCount;

    if (gfps_is_battery_enabled())
    {
        gfps_get_battery_levels(&batteryLevelCount, batteryFollowingData + 1);
        //include charging case
        if(batteryLevelCount >= GFPS_BATTERY_VALUE_MAX_COUNT)
        {
            batteryLevelCount = GFPS_BATTERY_VALUE_MAX_COUNT;
        }
        batteryFollowingData[0] = gfps_get_battery_datatype() | (batteryLevelCount << 4);
    }

    accountKeyData[0] = (APP_GFPS_ADV_VERSION << 4) | APP_GFPS_ADV_FLAG;
    accountKeyDataLen = 2;

    sizeOfFilter = (((uint8_t)(( float )1.2 * useKey)) + 3);
    memset(FArray, 0, sizeof(FArray));

    gfps_ble_get_addr(currentBleAddr);
    salt[0] = (APP_GFPS_ADV_LEN_SALT << 4) | APP_GFPS_ADV_TYPE_SALT;

    for(index = 0; index < APP_GFPS_ADV_LEN_SALT; index++ )
    {
#if GFPS_ACCOUNTKEY_SALT_TYPE == USE_RANDOM_NUM_AS_SALT
        if (GFPS_INITIAL_ADV_RAND_SALT != gfps_ble_env.advRandSalt)
        {
            salt[index + 1] = gfps_ble_env.advRandSalt;
        }
        else
        {
            salt[index + 1] = (uint8_t)rand();
        }
#else
        salt[index + 1] = currentBleAddr[index%6];
#endif
    }

#ifdef SASS_ENABLED
    gfps_sass_get_inuse_acckey(inUseKey);
#ifdef SASS_SECURE_ENHACEMENT
    uint8_t iv[16] = {0};
    memcpy(iv, salt+1, APP_GFPS_ADV_LEN_SALT);
    gfps_sass_encrypt_connection_state(iv, inUseKey, sassAdv, &sassAdvLen, true, true, NULL);
#endif
#endif
    TRACE(1, "the number of the account key is %d %d", useKey, keyCount);

    for (uint8_t keyIndex = 0; keyIndex < keyCount; keyIndex++)
    {
        uint8_t offsetOfVArray = 0;
        nv_record_fp_account_key_get_by_index(keyCount - 1 - keyIndex, VArray);

#ifdef SASS_ENABLED
        bool isInuseKey = false;
        if (!memcmp(inUseKey, VArray, FP_ACCOUNT_KEY_SIZE))
        {
            isInuseKey = true;
            findInUseKey = true;
        }
        else
        {
            if (!findInUseKey && (keyCount > useKey))
            {
                if ((keyIndex >= useKey - 1) && (keyIndex != keyCount - 1))
                {
                    continue;
                }
                else if (keyIndex == keyCount - 1)
                {
                    nv_record_fp_account_key_get_by_index(keyCount - useKey, VArray);
                }
                else
                {
                }
            }
        }

        if(isInuseKey)
        {
            if (gfps_sass_is_adv_in_use_key())
            {
                VArray[0] = IN_USE_ACCOUNT_KEY_HEADER;
                TRACE(0, "is inuse account key!!");
            }
            else
            {
                VArray[0] = MOST_RECENT_USED_ACCOUNT_KEY_HEADER;
                TRACE(0, "is most recently account key!!");
            }
        }
#endif
        offsetOfVArray += FP_ACCOUNT_KEY_SIZE;
        memcpy(VArray + offsetOfVArray, salt+1, APP_GFPS_ADV_LEN_SALT);
        offsetOfVArray += APP_GFPS_ADV_LEN_SALT;

        if (gfps_is_battery_enabled())
        {
            memcpy(VArray+offsetOfVArray, batteryFollowingData, batteryLevelCount + 1);
            offsetOfVArray += (batteryLevelCount + 1);
        }

#ifdef SASS_ENABLED
#ifdef SASS_SECURE_ENHACEMENT
        memcpy(VArray+offsetOfVArray, sassAdv, sassAdvLen);
        offsetOfVArray += sassAdvLen;
#endif
#endif

        gfps_SHA256_hash(VArray, offsetOfVArray, hash256Result);

        // K = Xi % (s * 8)
        // F[K/8] = F[K/8] | (1 << (K % 8))
        uint32_t pX[8];
        for (index = 0; index < 8; index++)
        {
            pX[index] = (((uint32_t)(hash256Result[index * 4])) << 24) |
                        (((uint32_t)(hash256Result[index * 4 + 1])) << 16) |
                        (((uint32_t)(hash256Result[index * 4 + 2])) << 8) |
                        (((uint32_t)(hash256Result[index * 4 + 3])) << 0);
        }

        for (index = 0; index < 8; index++)
        {
            uint32_t K    = pX[index] % (sizeOfFilter * 8);
            FArray[K / 8] = FArray[K / 8] | (1 << (K % 8));
        }

        if (findInUseKey && (keyIndex >= useKey - 1))
        {
            break;
        }
    }

    memcpy(&accountKeyData[2], FArray, sizeOfFilter);

    accountKeyDataLen += sizeOfFilter;

    accountKeyData[1] = (sizeOfFilter<<4) | APP_GFPS_ADV_TYPE_ACCKEY_FILETER_SHOW_UI;

    accountKeyData[2+sizeOfFilter] = salt[0];
    for(index = 0; index < APP_GFPS_ADV_LEN_SALT; index++ )
    {
        accountKeyData[2 + sizeOfFilter + index + 1] = salt[index + 1];
    }
    accountKeyDataLen += (APP_GFPS_ADV_LEN_SALT + 1);

    memcpy(outputData, accountKeyData, accountKeyDataLen);
    dataLen += accountKeyDataLen;
    
    if (gfps_is_battery_enabled())
    {
        memcpy(outputData + dataLen, batteryFollowingData, batteryLevelCount + 1);
        dataLen += (batteryLevelCount + 1);
    }

#ifdef SASS_ENABLED
#ifdef SASS_SECURE_ENHACEMENT
    memcpy(outputData + dataLen, sassAdv, sassAdvLen);
    dataLen += sassAdvLen;
#else
	gfps_sass_encrypt_adv_data(FArray, sizeOfFilter, inUseKey, outputData, &dataLen);
#endif
#endif

    TRACE(1,"Generated accountkey data len:%d", dataLen);
    DUMP8("%02x ", outputData, dataLen);

    return dataLen;
}

static bool gfps_ble_adv_activity_prepare(ble_adv_activity_t *adv)
{
    gap_adv_param_t *adv_param = &adv->adv_param;
    char gfps_tx_power;
    uint8_t *txPwr;
    bool isUseRPA = true;

    if (!ble_adv_is_allowed())
    {
        return false;
    }

#if defined(IBRT)
    if (!app_ble_check_ibrt_allow_adv(USER_GFPS))
    {
        return false;
    }
#endif

    adv->user = USER_GFPS;
    adv_param->connectable = true;
    adv_param->scannable = true;
    adv_param->use_legacy_pdu = true;
    adv_param->include_tx_power_data = true;
    adv_param->fast_advertising = false; // BLE_FASTPAIR_NORMAL_ADVERTISING_INTERVAL

    if (gfps_is_in_fastpairing_mode())
    {
        adv_param->fast_advertising = true; // BLE_FASTPAIR_FAST_ADVERTISING_INTERVAL
    }

#if BLE_AUDIO_ENABLED
    uint8_t cas_uuid[2];
    cas_uuid[0] = (APP_CAS_SERVICE_UUID >> 0) & 0xFF;
    cas_uuid[1] = (APP_CAS_SERVICE_UUID >> 8) & 0xFF;
#endif

    app_ble_set_adv_tx_power_level(adv, BLE_ADV_TX_POWER_LEVEL_1);

    app_ble_dt_set_flags(adv_param, true);

    txPwr = gfps_ble_get_tx_power_in_adv();
    gfps_tx_power = txPwr[APP_GFPS_ADV_POWER_UUID_LEN-1];
    if (gfps_is_in_fastpairing_mode())
    {
        uint32_t modelId = 0;
        uint8_t model_id_data[3];
        TRACE(0, "fast pair mode");

#ifdef BLE_AUDIO_ENABLED
        if(gfps_ble_env.bondMode == GFPS_BOND_OVER_BLE)
        {
            isUseRPA = false;
        }
#endif
        modelId = gfps_get_model_id();
        model_id_data[0] = (modelId >> 16) & 0xFF;
        model_id_data[1] = (modelId >> 8) & 0xFF;
        model_id_data[2] = (modelId >> 0) & 0xFF;

        gap_dt_add_service_data(&adv_param->adv_data, APP_GFPS_SERVICE_UUID, model_id_data, sizeof(model_id_data));

        gap_dt_add_tx_power(&adv_param->adv_data, gfps_tx_power);

#if BLE_AUDIO_ENABLED
        gap_dt_add_data_type(&adv_param->adv_data, GAP_DT_SERVICE_DATA_16BIT_UUID, cas_uuid, sizeof(cas_uuid));
#endif
    }
    else
    {
#if BLE_APP_GFPS_VER == FAST_PAIR_REV_2_0
        uint8_t service_data[32];
        uint16_t data_len = 0;

        data_len = gfps_ble_generate_accountkey_data(service_data);

        gap_dt_add_service_data(&adv_param->adv_data, APP_GFPS_SERVICE_UUID, service_data, data_len);

        gap_dt_add_tx_power(&adv_param->adv_data, gfps_tx_power);

#if BLE_AUDIO_ENABLED
        gap_dt_add_data_type(&adv_param->scan_rsp_data, GAP_DT_SERVICE_DATA_16BIT_UUID, cas_uuid, sizeof(cas_uuid));
#endif
#endif
    }

    if (isUseRPA)
    {
        adv_param->own_addr_use_rpa = true;
        adv_param->use_custom_local_addr = false;
        adv_param->use_fake_btc_rpa_when_no_irk_exist = true;
    }
    else
    {
        adv_param->own_addr_use_rpa = false;
    }

    app_ble_dt_set_local_name(adv_param, NULL);

    return true;
}

#ifdef SPOT_ENABLED
static bool gfps_ble_spot_adv_activity_prepare(ble_adv_activity_t *adv)
{
    gap_adv_param_t *adv_param = &adv->adv_param;
    bool spot_adv_enable = false;

    if (!ble_adv_is_allowed())
    {
        return false;
    }

#if defined(IBRT)
    if (!app_ble_check_ibrt_allow_adv(USER_SPOT))
    {
        return false;
    }
#endif

    spot_adv_enable = nv_record_fp_get_spot_adv_enable_value();
    if (!spot_adv_enable)
    {
        return false;
    }

    adv->user = USER_SPOT;
    adv_param->connectable = true;
    adv_param->scannable = true;
    adv_param->use_legacy_pdu = true;
    adv_param->include_tx_power_data = true;
    adv_param->own_addr_use_rpa = true;
    adv_param->fast_advertising = false;
    adv_param->use_custom_local_addr = false;
    adv_param->use_fake_btc_rpa_when_no_irk_exist = true;

    adv->adv_param.has_custom_adv_timing = true;
    adv->adv_param.adv_timing.min_adv_slow_interval_ms = 800;
    adv->adv_param.adv_timing.max_adv_slow_interval_ms = BLE_FASTPAIR_SPOT_ADVERTISING_INTERVAL;

    app_ble_set_adv_tx_power_level(adv, BLE_ADV_TX_POWER_LEVEL_1);

    app_ble_dt_set_flags(adv_param, true);

    if (spot_adv_enable)
    {
        uint8_t service_data[24];
        uint8_t *data_ptr = service_data;

        *data_ptr = gfps_ble_env.enable_unwanted_tracking_mode ? FP_EID_FRAME_TYPE_WHEN_ENABLE_UTP : FP_EID_FRAME_TYPE_WHEN_DISABLE_UTP;
        data_ptr += 1;

        memcpy(data_ptr, nv_record_fp_get_spot_adv_data(), 20);
        data_ptr += 20;

        if (gfps_ble_env.orignal_flag != GFPS_BEACON_NOT_SUPPORT_BATTERY)
        {
            *data_ptr =  gfps_ble_env.orignal_flag ^ nv_record_fp_get_hash_value();
            data_ptr += 1;
        }

        gap_dt_add_service_data(&adv_param->adv_data, APP_SPOT_SERVICE_UUID, service_data, data_ptr-service_data);

        app_ble_dt_set_local_name(adv_param, NULL);
    }

    return true;
}

static osTimerId spot_auto_reset_timer_id = NULL;
#define SPOT_AUTO_RESET_TIME_VALUE      300000
static void app_gfps_spot_auto_reset_handler(void const *param);
osTimerDef (SPOT_AUTO_RESET_TIME, app_gfps_spot_auto_reset_handler);

void app_gfps_spot_auto_reset_timer_init(void)
{
    TRACE(1,"%s",__func__);
    if (spot_auto_reset_timer_id == NULL)
    {
        spot_auto_reset_timer_id = osTimerCreate(osTimer(SPOT_AUTO_RESET_TIME), osTimerOnce, NULL);
    }
    osTimerStart(spot_auto_reset_timer_id, SPOT_AUTO_RESET_TIME_VALUE);
}
void app_gfps_spot_auto_reset_timer_stop(void)
{
    TRACE(1,"%s",__func__);
    if(spot_auto_reset_timer_id)
        osTimerStop(spot_auto_reset_timer_id);
}
static void app_gfps_spot_auto_reset_handler(void const *param)
{
    //customer should add led and ring here to tell user
     nv_record_fp_account_key_delete();
}

void gfps_ble_spot_generate_nonce(void)
{
    //generate 8bytes random number for nonce;
    rand_generator(gfps_ble_env.nonce, GFPS_NONCE_SIZE);
}

uint8_t* gfps_ble_spot_get_nonce(void)
{
    return gfps_ble_env.nonce;
}

uint8_t gfps_ble_get_spot_version(void)
{
    return gfps_ble_env.protocol_version;
}

void gfps_ble_set_protocol_version(void)
{
    gfps_ble_env.protocol_version = GFPS_BEACON_PROTOCOL_VERSION;
}

void gfps_ble_spot_send_beacon_provisioning_state(uint8_t *ptrData, uint32_t length)
{
    bes_ble_gfps_send_beacon_data(gfps_ble_env.connectionIndex, ptrData, length);
}

void gfps_ble_spot_ring_timer_set()
{
    TRACE(1,"%s,", __func__);
    if (spot_ring_timeout_timer_id == NULL)
    {
        spot_ring_timeout_timer_id = osTimerCreate(osTimer(SPOT_FIND_DEVICES_RING_TIMEOUT), osTimerPeriodic, NULL);
    }
    osTimerStart(spot_ring_timeout_timer_id, 2*1000);
}

static void gfps_ble_spot_find_devices_ring_timeout_handler(void const *n)
{
    spot_ring_nums+=1;
    gfps_start_find_ringtone();

    if(spot_ring_nums*2 >= gfps_ble_env.remaining_ring_time/10)
    {
        TRACE(0,"ring time out");
        spot_ring_nums=0;
        osTimerStop(spot_ring_timeout_timer_id);
        gfps_stop_find_ringtone();
        gfps_ble_env.ring_state = false;

        gfps_ble_spot_beacon_ring_resp beacon_rsp;
        beacon_rsp.data[0] = GFPS_BEACON_RINGING_STATE_STOPPED_TIMEOUT;
        beacon_rsp.data[1] = GFPS_BEACON_RINGING_NONE;
        memset(&beacon_rsp.data[2], 0x00, 2);

        uint8_t auth_segment[16];
        auth_segment[0] = gfps_ble_get_spot_version();
        memcpy(&auth_segment[1], gfps_ble_spot_get_nonce(), 8);
        auth_segment[9] = GFPS_BEACON_RING;
        auth_segment[10] = 0x0c;
        memcpy(&auth_segment[11], beacon_rsp.data, 4);
        auth_segment[15] = 0x01;
        gfps_beacon_encrpt_data(hashed_ring_key, auth_segment, 16, beacon_rsp.auth_data);
        
        beacon_rsp.data_id = GFPS_BEACON_RING;
        beacon_rsp.data_length = 0x0c;

       gfps_ble_spot_send_beacon_provisioning_state((uint8_t *)&beacon_rsp, sizeof(beacon_rsp));
    }
}

static void gfps_ble_spot_get_current_beacon_timer(void)
{
    uint32_t current_time;
    current_time = nv_record_fp_get_current_beacon_time() + GET_CURRENT_MS()/1000;
    uint8_t time[4];
    memcpy(time, &current_time, sizeof(uint32_t));
    TRACE(1,"gfps_ble_spot_get_current_beacon_timer, time is %d", current_time);
    
    gfps_ble_env.beacon_time[0] = time[0]&0x00;
    gfps_ble_env.beacon_time[1]=  time[1]&0xFC;
    gfps_ble_env.beacon_time[2] = time[2]&0xFF;
    gfps_ble_env.beacon_time[3] = time[3]&0xFF;
}

void gfps_ble_spot_generate_EID_handler(uint8_t* orig_eph_identity_key)
{
    uint8_t Ephemeral_ID_seed[32];
    uint8_t random[32];
    gfps_ble_spot_get_current_beacon_timer();
    
    memset(Ephemeral_ID_seed, 0xFF, 11);        //padding 0xff
    Ephemeral_ID_seed[11] = 0x0a;
    big_little_switch(gfps_ble_env.beacon_time, &Ephemeral_ID_seed[12], 4);

    memset(&Ephemeral_ID_seed[16],0x00, 11);
    
    Ephemeral_ID_seed[27] = 0x0a;
    big_little_switch(gfps_ble_env.beacon_time, &Ephemeral_ID_seed[28], 4);
    TRACE(0,"Ephemeral_ID_seed is");
    DUMP8("0x%x ",  &Ephemeral_ID_seed[12],4);

    gfps_crypto_encrypt(Ephemeral_ID_seed, 32, orig_eph_identity_key, random);
    gfps_crypto_encrypt(&Ephemeral_ID_seed[16], 32, orig_eph_identity_key, &random[16]);

    TRACE(0,"random is:");
    DUMP8("0X%x ", random, 32);
    //ECC to get ephemeral identifier
    uint8_t Ecc_private_key[20];
    uint8_t Ecc_public_key[20];

    TRACE(0,"gfps_beacon_ecc_mod");
    uint8_t random_output[48];
    big_little_switch(random, random_output, 32);
    memset(&random_output[32],0,16);

    uEcc_160r1_mod(Ecc_private_key, random_output);
    memcpy(gfps_ble_env.Ecc_private_key, Ecc_private_key, 20);
    DUMP8("0X%x ",Ecc_private_key, 20);
    TRACE(0,"gfps_beacon_ecc_mult");
    uEcc_160r1_mult(Ecc_public_key, Ecc_private_key);
    DUMP8("0X%x ",Ecc_public_key, 20);

   big_little_switch(Ecc_public_key, gfps_ble_env.adv_identifer, 20);

}

void gfps_ble_spot_generate_hashed_value()
{
    uint8_t hashed_orignal_value[20] = {0};
    uint8_t hashed_output_value[32];
    big_little_switch(gfps_ble_env.Ecc_private_key, hashed_orignal_value, 20);
    gfps_SHA256_hash(hashed_orignal_value, 20, hashed_output_value);
    gfps_ble_env.hashed_flag = hashed_output_value[31];
}

void gfps_ble_spot_EID_adv_timer_set(void)
{
    TRACE(1,"%s,", __func__);
    if (spot_EID_adv_timer_id == NULL)
    {
        spot_EID_adv_timer_id = hwtimer_alloc((HWTIMER_CALLBACK_T)gfps_ble_spot_EID_adv_time_handler, NULL);
    }
    uint32_t rand_timer_value;
    rand_generator((uint8_t *)&rand_timer_value, 4);
    uint16_t timer_value = rand_timer_value%512;

    TRACE(1,"gfps_ble_spot_EID_adv_timer_set %d", timer_value);
    hwtimer_start(spot_EID_adv_timer_id, MS_TO_TICKS(timer_value*1000)); 
}

static void gfps_ble_spot_EID_adv_time_handler(const void * param)
{
    TRACE(0,"gfps_ble_spot_EID_adv_time_handler");
    nv_record_fp_update_spot_adv_data(gfps_ble_env.adv_identifer);
    nv_record_fp_update_spot_hash_value(gfps_ble_env.hashed_flag);
    if(gfps_ble_env.enable_unwanted_tracking_mode == false)
    {
        app_ble_add_devices_info_to_resolving();
    }
    bes_ble_gap_start_connectable_adv(BLE_FASTPAIR_SPOT_ADVERTISING_INTERVAL);
    hwtimer_stop(spot_EID_adv_timer_id);
}

void gfps_ble_spot_rotate_hard_timer_start()
{
    TRACE(0,"gfps_ble_spot_rotate_hard_timer_start");
    if (spot_EID_timeout_hard_timer_id == NULL) {
        spot_EID_timeout_hard_timer_id = hwtimer_alloc((HWTIMER_CALLBACK_T)gfps_ble_spot_EID_hard_timer_rotation_handler, NULL);
    }

    hwtimer_start(spot_EID_timeout_hard_timer_id, MS_TO_TICKS(1024*1000));
}

static void gfps_ble_spot_EID_hard_timer_rotation_handler(void const *param)
{
    TRACE(0,"gfps_ble_spot_EID_hard_timer_rotation_handler");
    gfps_ble_spot_generate_EID_handler(nv_record_fp_get_eph_identity_key());
    gfps_ble_spot_generate_hashed_value();
    gfps_ble_spot_rotate_hard_timer_start();
    gfps_ble_spot_EID_adv_timer_set();
}

static uint8_t gfps_ble_write_spot_beacon_actions_hander(uint8_t condix, uint8_t *param, uint16_t len)
{
    TRACE(0,"%s, data_id is %d", __func__, param[0]);
    uint8_t status = GFPS_SUCCESS;
    if (!param)
    {
        return GFPS_ERROR_INVALID_VALUE;
    }

    uint8_t data_id = param[0];
    uint8_t data_length = param[1];

    if(data_id == GFPS_BEACON_READ_BEACON_PARAM)                //reading the beacon's state;
    {
        uint8_t keyCount = nv_record_fp_account_key_count();
        uint8_t protocol_version = gfps_ble_env.protocol_version;
        uint8_t nonce[8];
        memcpy(nonce, gfps_ble_spot_get_nonce(),8);

        if(keyCount == 0)
        {
            status = GFPS_ERROR_UNAUTHENTICATED;
        }
        for (uint8_t keyIndex = 0; keyIndex < keyCount; keyIndex++)
        {
            uint8_t VArray[FP_ACCOUNT_KEY_SIZE];
            uint8_t calculatedHmacFirst8Bytes[8];
            uint8_t input_encrpyt_data[11];
            input_encrpyt_data[0] = protocol_version;
            memcpy(&input_encrpyt_data[1], nonce, 8);
            input_encrpyt_data[9] = data_id;
            input_encrpyt_data[10] = data_length;

            nv_record_fp_account_key_get_by_index(keyIndex, VArray);
            gfps_beacon_encrpt_data(VArray, input_encrpyt_data, 11, calculatedHmacFirst8Bytes);

            if(!memcmp(calculatedHmacFirst8Bytes, &param[2], 8))
            {
               TRACE(0,"matched");
               status = GFPS_SUCCESS;
               memcpy(matched_account_key, VArray, FP_ACCOUNT_KEY_SIZE);
               break;
            }
            else
            {
                TRACE(0,"not matched");
                status = GFPS_ERROR_UNAUTHENTICATED;
            }
        }

        if(status == GFPS_SUCCESS)
        {
            gfps_ble_spot_read_beacon_additional_data additional_data = {0};
            gfps_ble_spot_read_beacon_state_resp beacon_rsp;
            uint8_t encrypted_data[16];
            uint32_t current_time;
            uint8_t auth_segment[28];
            ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx();
            current_time = nv_record_fp_get_current_beacon_time() + GET_CURRENT_MS()/1000;
            uint8_t time[4];
            memcpy(time, &current_time, sizeof(uint32_t));

            beacon_rsp.data_id = GFPS_BEACON_READ_BEACON_PARAM;
            beacon_rsp.data_length = 0x18;

            additional_data.SECP_method = GFPS_BEACON_SECP_160R1_METHOD;
            additional_data.numbesr_of_ringing = GFPS_BEACON_TWO_CAPABLE_OF_RING;
            additional_data.ringing_capability = GFPS_BEACON_RINGING_VOLUME_AVAILABLE;
            int8_t tx_power_id = 0;
            app_bt_get_tx_power_idx(p_ibrt_ctrl->tws_conhandle, &tx_power_id);
            additional_data.power_value = btdrv_reg_op_txpwr_idx_to_rssidbm(tx_power_id);

            TRACE(1,"tx power is %x", additional_data.power_value);
            big_little_switch(time, additional_data.clock_value, sizeof(uint32_t));
            //memcpy(beacon_rsp.clock_value, &time, 4);
            memset(additional_data.padgding, 0x00, 8);
            gfps_crypto_encrypt((uint8_t *)&additional_data,16, matched_account_key, encrypted_data);
            memcpy(beacon_rsp.additional_data, encrypted_data, 16);

            auth_segment[0] = protocol_version;
            memcpy(&auth_segment[1], nonce, 8);
            auth_segment[9] = data_id;
            auth_segment[10] = 0x18;
            memcpy(&auth_segment[11], encrypted_data, 16);
            auth_segment[27] = 0x01;

            gfps_beacon_encrpt_data(matched_account_key, auth_segment, 28, beacon_rsp.auth_data);
            memcpy(beacon_rsp.additional_data, encrypted_data, 16);

           gfps_ble_env.beacon_time[0] = time[0]&0x00;
           gfps_ble_env.beacon_time[1]=  time[1]&0xFC;
           gfps_ble_env.beacon_time[2] = time[2]&0xFF;
           gfps_ble_env.beacon_time[3] = time[3]&0xFF;
           TRACE(1,"time is %d, ", current_time);
           DUMP8("0x%x ", additional_data.clock_value,4);
           DUMP8("0x%x ",gfps_ble_env.beacon_time,4);

           gfps_ble_spot_send_beacon_provisioning_state((uint8_t *)&beacon_rsp, sizeof(beacon_rsp));
         }
    }
    else if(data_id == GFPS_BEACON_READ_PROVISION_STATE)             //0x01, reading the beacon's provisioning state
    {

        uint8_t keyCount = nv_record_fp_account_key_count();
        uint8_t protocol_version = gfps_ble_env.protocol_version;
        uint8_t nonce[8];
        memcpy(nonce, gfps_ble_spot_get_nonce(),8);
        uint8_t accountkey_Index =0;

        if (keyCount == 0)
        {
            status = GFPS_ERROR_UNAUTHENTICATED;
        }
        for (uint8_t keyIndex = 0; keyIndex < keyCount; keyIndex++)
        {
            uint8_t VArray[FP_ACCOUNT_KEY_SIZE];
            uint8_t calculatedHmacFirst8Bytes[8];

            uint8_t input_encrpyt_data[11];
            input_encrpyt_data[0] = protocol_version;
            memcpy(&input_encrpyt_data[1], nonce, 8);
            input_encrpyt_data[9] = data_id;
            input_encrpyt_data[10] = data_length;

            nv_record_fp_account_key_get_by_index(keyIndex, VArray);
            gfps_beacon_encrpt_data(VArray, input_encrpyt_data, 11, calculatedHmacFirst8Bytes);

            if (!memcmp(calculatedHmacFirst8Bytes, &param[2], 8))
            {
               TRACE(0,"matched");
               status = GFPS_SUCCESS;
               memcpy(matched_account_key, VArray, FP_ACCOUNT_KEY_SIZE);
               accountkey_Index = keyIndex;
               if(keyIndex != 0x00)
               {
                  gfps_ble_env.not_allowed_retry_flag = false;
               }
               break;
            }
            else
            {
                TRACE(0,"not matched");
                gfps_ble_env.not_allowed_retry_flag = true;
                status = GFPS_ERROR_UNAUTHENTICATED;
            }
        }

        if(gfps_ble_env.not_allowed_retry_flag)
        {
            status = GFPS_ERROR_UNAUTHENTICATED;
        }

        if (status == GFPS_SUCCESS)
        {
            if(nv_record_fp_get_spot_adv_enable_value() == false)
            {
                TRACE(0,"spot adv is not enable");
                gfps_ble_spot_read_beacon_provision_resp beacon_rsp;
                uint8_t auth_segment[13];

                auth_segment[0] = protocol_version;
                memcpy(&auth_segment[1], nonce, 8);
                auth_segment[9] = data_id;
                auth_segment[10] = 0x09;
                if (accountkey_Index == 0x00)
                    auth_segment[11] = 0x02;
                else
                    auth_segment[11] = 0x00;
                auth_segment[12] = 0x01;

                gfps_beacon_encrpt_data(matched_account_key, auth_segment, 13, beacon_rsp.auth_data);

                beacon_rsp.data_id = GFPS_BEACON_READ_PROVISION_STATE;
                beacon_rsp.data_length = 0x09;
                if (accountkey_Index == 0x00)
                    beacon_rsp.data = 0x02;
                else
                    beacon_rsp.data = 0x00;

                gfps_ble_spot_send_beacon_provisioning_state((uint8_t *)&beacon_rsp, sizeof(beacon_rsp));
            }
            else
            {
                TRACE(0,"spot adv has been enable yet");
                gfps_ble_spot_read_EIK_beacon_provision_resp beacon_rsp;
                uint8_t auth_segment[33];

                auth_segment[0] = protocol_version;
                memcpy(&auth_segment[1], nonce, 8);
                auth_segment[9] = data_id;
                auth_segment[10] = 0x1d;
                if (accountkey_Index == 0x00)
                    auth_segment[11] = 0x03;
                else
                    auth_segment[11] = 0x01;
                memcpy(&auth_segment[12], gfps_ble_env.adv_identifer, 20);
                auth_segment[32] = 0x01;
                DUMP8("%02x ", auth_segment, 33);

                gfps_beacon_encrpt_data(matched_account_key, auth_segment, 33, beacon_rsp.auth_data);

                beacon_rsp.data_id = GFPS_BEACON_READ_PROVISION_STATE;
                beacon_rsp.data_length = 0x1d;
                if (accountkey_Index == 0x00)
                    beacon_rsp.data = 0x03;
                else
                    beacon_rsp.data = 0x01;
                memcpy(beacon_rsp.EIK, gfps_ble_env.adv_identifer, 20);

                gfps_ble_spot_send_beacon_provisioning_state((uint8_t *)&beacon_rsp, sizeof(beacon_rsp));
            }
        }
    }
    else if (data_id == GFPS_BEACON_SET_EPHEMERAL_IDENTITY_KEY)                //0x02, setting an ephemeral identity key, do not need nofity;
    {
         bes_ble_gap_set_rpa_timeout(0xA1B8);
         gfps_ble_spot_rotate_hard_timer_start();
         uint8_t owner_accountkey[16];
         nv_record_fp_account_key_get_by_index(0, owner_accountkey);
         uint8_t keyCount = nv_record_fp_account_key_count();
         uint8_t protocol_version = gfps_ble_env.protocol_version;
         uint8_t nonce[8];
         memcpy(nonce, gfps_ble_spot_get_nonce(),8);
         uint8_t orig_eph_identity_key[32];

         if(keyCount == 0)
         {
             status = GFPS_ERROR_UNAUTHENTICATED;
         }

         if(nv_record_fp_get_spot_adv_enable_value() == false)    //need to set EIK
         {
             uint8_t VArray[FP_ACCOUNT_KEY_SIZE];
             uint8_t calculatedHmacFirst8Bytes[8];
             uint8_t input_encrpyt_data[43];
             input_encrpyt_data[0] = protocol_version;
             memcpy(&input_encrpyt_data[1], nonce, 8);
             input_encrpyt_data[9] = data_id;
             input_encrpyt_data[10] = data_length;
             memcpy(&input_encrpyt_data[11], &param[10], 32);
             for (uint8_t keyIndex = 0; keyIndex < keyCount; keyIndex++)
             {
                 nv_record_fp_account_key_get_by_index(keyIndex, VArray);
                 gfps_beacon_encrpt_data(VArray, input_encrpyt_data, 43, calculatedHmacFirst8Bytes);

                 if (!memcmp(calculatedHmacFirst8Bytes, &param[2], 8))
                 {
                    TRACE(0,"matched");
                    status = GFPS_SUCCESS;
                    memcpy(matched_account_key, VArray, FP_ACCOUNT_KEY_SIZE);
                    break;
                 }
                 else
                 {
                     TRACE(0,"not matched");
                     status = GFPS_ERROR_UNAUTHENTICATED;
                 }
             }

             if (memcmp(owner_accountkey, matched_account_key, 16))
             {
                 TRACE(0,"it is non-owner accout key");
                 status = GFPS_ERROR_UNAUTHENTICATED;
             }
             else
             {
                 TRACE(0,"it is owner account key");
             }

             if (status == GFPS_SUCCESS)
            {
                TRACE(0,"set key");
                gfps_crypto_decrypt(&param[10], 16, matched_account_key, orig_eph_identity_key);
                gfps_crypto_decrypt(&param[26], 16, matched_account_key, &orig_eph_identity_key[16]);

                TRACE(0,"orig_eph_identity_key is:");
                DUMP8("%02x", orig_eph_identity_key, 32);
                nv_record_fp_update_eph_identity_key(orig_eph_identity_key);
               
                gfps_ble_spot_generate_EID_handler(orig_eph_identity_key);

                //write to flash
                nv_record_fp_update_spot_adv_data(gfps_ble_env.adv_identifer);
                gfps_ble_spot_generate_hashed_value();
                nv_record_fp_update_spot_hash_value(gfps_ble_env.hashed_flag);
                //start adv
                if ( nv_record_fp_get_spot_adv_enable_value() == false)
                {
                   
                    nv_record_fp_update_spot_adv_eanbled(true);
                    bes_ble_gap_start_connectable_adv(BLE_FASTPAIR_SPOT_ADVERTISING_INTERVAL);
                }

                //reply;
                uint8_t auth_segment[12];
                gfps_ble_spot_read_set_beacon_provision_resp beacon_rsp;
                auth_segment[0] = protocol_version;
                memcpy(&auth_segment[1], nonce, 8);
                auth_segment[9] = data_id;
                auth_segment[10] = 0x08;
                auth_segment[11] = 0x01;
                gfps_beacon_encrpt_data(matched_account_key, auth_segment, 12, beacon_rsp.auth_data);

                beacon_rsp.data_id = data_id;
                beacon_rsp.data_length = 0x08;
                gfps_ble_spot_send_beacon_provisioning_state((uint8_t *)&beacon_rsp, sizeof(beacon_rsp));
            }
         }
         else //has been set EIK
         {
             TRACE(0,"has been set the EIK");
             uint8_t VArray[FP_ACCOUNT_KEY_SIZE];
             uint8_t calculatedHmacFirst8Bytes[8];
             uint8_t input_encrpyt_data[51];
             input_encrpyt_data[0] = protocol_version;
             memcpy(&input_encrpyt_data[1], nonce, 8);
             input_encrpyt_data[9] = data_id;
             input_encrpyt_data[10] = data_length;
             memcpy(&input_encrpyt_data[11], &param[10], 40);
            
             for (uint8_t keyIndex = 0; keyIndex < keyCount; keyIndex++)
             {
                 nv_record_fp_account_key_get_by_index(keyIndex, VArray);
                 gfps_beacon_encrpt_data(VArray, input_encrpyt_data, 51, calculatedHmacFirst8Bytes);

                 if (!memcmp(calculatedHmacFirst8Bytes, &param[2], 8))
                 {
                    TRACE(0,"matched");
                    status = GFPS_SUCCESS;
                    memcpy(matched_account_key, VArray, FP_ACCOUNT_KEY_SIZE);
                    break;
                 }
                 else
                 {
                     TRACE(0,"not matched");
                     status = GFPS_ERROR_UNAUTHENTICATED;
                 }
             }

             if (memcmp(owner_accountkey, matched_account_key, 16))
             {
                 TRACE(0,"it is non-owner accout key");
                 status = GFPS_ERROR_UNAUTHENTICATED;
             }
             else
             {
                 TRACE(0,"it is owner account key");
             }

             if (status == GFPS_SUCCESS)
             {
                 uint8_t Eph_identity_key[FP_EPH_IDENTITY_KEY_LEN+8];
                 uint8_t hash_eph_result[32];

                 memcpy(Eph_identity_key,nv_record_fp_get_eph_identity_key(),FP_EPH_IDENTITY_KEY_LEN);
                 memcpy(&Eph_identity_key[32], gfps_ble_spot_get_nonce(), 8);
                 gfps_SHA256_hash(Eph_identity_key, FP_EPH_IDENTITY_KEY_LEN+8, hash_eph_result);
                 DUMP8("%02x ", hash_eph_result, 8);
                 if (!memcmp(hash_eph_result, &param[42], 8))
                 {
                     TRACE(0,"eph key mathed");
                     status = BES_GATT_NO_ERR;
                 }
                 else
                 {
                     TRACE(0,"eph key not matched");
                     status = GFPS_ERROR_UNAUTHENTICATED;
                 }

                 if (status == BES_GATT_NO_ERR)
                 {
                     gfps_crypto_decrypt(&param[10], 16, matched_account_key, orig_eph_identity_key);
                     gfps_crypto_decrypt(&param[26], 16, matched_account_key, &orig_eph_identity_key[16]);
                     nv_record_fp_update_eph_identity_key(orig_eph_identity_key); //write the new eik to the flash;

                     //refresh adv
                     gfps_ble_spot_generate_EID_handler(orig_eph_identity_key);
                     //write to flash
                     nv_record_fp_update_spot_adv_data(gfps_ble_env.adv_identifer);
                     gfps_ble_spot_generate_hashed_value();
                     nv_record_fp_update_spot_hash_value(gfps_ble_env.hashed_flag);
                     app_ble_refresh_adv_state_generic();
                      
                     uint8_t auth_segment[12];
                     gfps_ble_spot_read_set_beacon_provision_resp beacon_rsp;
         
                     auth_segment[0] = protocol_version;
                     memcpy(&auth_segment[1], nonce, 8);
                     auth_segment[9] = data_id;
                     auth_segment[10] = 0x08;
                     auth_segment[11] = 0x01;
 
                     gfps_beacon_encrpt_data(matched_account_key, auth_segment, 12, beacon_rsp.auth_data);
 
                     beacon_rsp.data_id = data_id;
                     beacon_rsp.data_length = 0x08;
                     gfps_ble_spot_send_beacon_provisioning_state((uint8_t *)&beacon_rsp, sizeof(beacon_rsp));
                 }
             }
         }
    }
    else if (data_id == GFPS_BEACON_CLEAR_EPHEMERAL_IDENTITY_KEY)        //clearing the ephemeral identity key
    {
        uint8_t keyCount = nv_record_fp_account_key_count();
        uint8_t protocol_version = gfps_ble_env.protocol_version;
        uint8_t nonce[8];
        memcpy(nonce, gfps_ble_spot_get_nonce(),8);

        if(keyCount == 0)
        {
            status = GFPS_ERROR_UNAUTHENTICATED;
        }
        for (uint8_t keyIndex = 0; keyIndex < keyCount; keyIndex++)
        {
            uint8_t VArray[FP_ACCOUNT_KEY_SIZE];
            uint8_t calculatedHmacFirst8Bytes[8];

            uint8_t input_encrpyt_data[19];
            input_encrpyt_data[0] = protocol_version;
            memcpy(&input_encrpyt_data[1], nonce, 8);
            input_encrpyt_data[9] = data_id;
            input_encrpyt_data[10] = data_length;
            memcpy(&input_encrpyt_data[11], &param[10], 8);

            nv_record_fp_account_key_get_by_index(keyIndex, VArray);
            gfps_beacon_encrpt_data(VArray, input_encrpyt_data, 19, calculatedHmacFirst8Bytes);
            DUMP8("0x%02x ", calculatedHmacFirst8Bytes, 8);

            if(!memcmp(calculatedHmacFirst8Bytes, &param[2], 8))
            {
               TRACE(0,"matched");
               status = GFPS_SUCCESS;
               memcpy(matched_account_key, VArray, FP_ACCOUNT_KEY_SIZE);
               break;
            }
            else
            {
                TRACE(0,"not matched");
                status = GFPS_ERROR_UNAUTHENTICATED;
            }
        }

        if (status == GFPS_SUCCESS)
        {

            uint8_t current_eph_identity_key[FP_EPH_IDENTITY_KEY_LEN+8];
            uint8_t hash256ResultOfEphKey[32];
            memcpy(current_eph_identity_key, nv_record_fp_get_eph_identity_key(), FP_EPH_IDENTITY_KEY_LEN);
            memcpy(&current_eph_identity_key[FP_EPH_IDENTITY_KEY_LEN], gfps_ble_spot_get_nonce(), 8);

            gfps_SHA256_hash(current_eph_identity_key, (FP_EPH_IDENTITY_KEY_LEN+8), hash256ResultOfEphKey);
            if(!memcmp(&param[10], hash256ResultOfEphKey, 8))
            {
                TRACE(0,"eph key matched");
                status = BES_GATT_NO_ERR;
            }
            else
            {
                TRACE(0,"eph key not matched");
                status = GFPS_ERROR_UNAUTHENTICATED;
            }
        }

        if (status == GFPS_SUCCESS)
        {
            nv_record_fp_reset_eph_identity_key();
            
            nv_record_fp_reset_spot_adv_enable_value();
            nv_record_fp_reset_spot_adv_data();
            bes_ble_gap_start_connectable_adv(BLE_FASTPAIR_SPOT_ADVERTISING_INTERVAL);

            uint8_t auth_segment[12];
            gfps_ble_spot_clear_EIK_beacon_provision_resp beacon_rsp;

            auth_segment[0] = protocol_version;
            memcpy(&auth_segment[1], nonce, 8);
            auth_segment[9] = data_id;
            auth_segment[10] = 0x08;
            auth_segment[11] = 0x01;
            gfps_beacon_encrpt_data(matched_account_key, auth_segment, 12, beacon_rsp.auth_data);

            beacon_rsp.data_id= data_id;
            beacon_rsp.data_length = 0x08;
            gfps_ble_spot_send_beacon_provisioning_state((uint8_t *)&beacon_rsp, sizeof(beacon_rsp));
            
        }
    }
    else if (data_id == GFPS_BEACON_READ_EPHEMERAL_IDENTITY_KEY)            //reading the ephemeral identity key
    {
        if (1)
        {
            uint8_t hashed_recover_key[GFPS_RECOVERY_KEY_SIZE+8];
            memset(hashed_recover_key,0, 16);

            uint8_t protocol_version = gfps_ble_env.protocol_version;
            uint8_t hash256Result[32];
            uint8_t eph_identity_key[FP_EPH_IDENTITY_KEY_LEN+1];
            //uint8_t hashResultOfrecoveryKey[32];
            uint8_t calculatedHmacFirst8Bytes[8];
           

            memcpy(eph_identity_key, nv_record_fp_get_eph_identity_key(), FP_EPH_IDENTITY_KEY_LEN);
            eph_identity_key[FP_EPH_IDENTITY_KEY_LEN] = 0x01;
            gfps_SHA256_hash(eph_identity_key, 33, hash256Result);
            memcpy(hashed_recover_key, hash256Result, 8);                    //recovery key;

            uint8_t input_encrpyt_data[11];
            input_encrpyt_data[0] = protocol_version;
            memcpy(&input_encrpyt_data[1], gfps_ble_spot_get_nonce(), 8);
            input_encrpyt_data[9] = data_id;
            input_encrpyt_data[10] = data_length;

            gfps_beacon_encrpt_data(hashed_recover_key, input_encrpyt_data, 11, calculatedHmacFirst8Bytes);
            if(!memcmp(calculatedHmacFirst8Bytes, &param[2], 8))
            {
                TRACE(0,"matched");
                status = GFPS_SUCCESS;
            }
            else
            {
                TRACE(0,"not matched");
                status = GFPS_ERROR_UNAUTHENTICATED;
            }

            if(status == GFPS_SUCCESS)
            {
                gfps_ble_spot_read_beacon_identity_key_resp beacon_rsp;
                uint8_t encrpted_ep_key[32];
                //uint8_t matched_account_key[16];
                //nv_record_fp_account_key_get_by_index(0, matched_account_key);
                DUMP8("%02x ", matched_account_key, 16);
                gfps_crypto_encrypt(eph_identity_key, 16, matched_account_key, encrpted_ep_key);
                gfps_crypto_encrypt(&eph_identity_key[16], 16, matched_account_key, &encrpted_ep_key[16]);

                beacon_rsp.data_id = data_id;
                beacon_rsp.data_length = 0x28;

                uint8_t auth_segment[44];
                auth_segment[0] = protocol_version;
                memcpy(&auth_segment[1], gfps_ble_spot_get_nonce(), 8);
                auth_segment[9] = data_id;
                auth_segment[10] = 0x28;
                memcpy(&auth_segment[11], encrpted_ep_key, 32);    //if eph_identity_key need account key to encrypted
                auth_segment[43] = 0x01;
                gfps_beacon_encrpt_data(hashed_recover_key, auth_segment, 44, beacon_rsp.auth_data);

                memcpy(beacon_rsp.data, encrpted_ep_key, 32);
                gfps_ble_spot_send_beacon_provisioning_state((uint8_t *)&beacon_rsp, sizeof(beacon_rsp));
            }

        }
        else
        {
            TRACE(0,"not in fast pair mode");
            status = GFPS_ERROR_NO_USER_CONSENT;
        }
    }
    else if (data_id == GFPS_BEACON_RING)                    //ringing, data id 0x05
    {
        //uint8_t hashed_ring_key[GFPS_RING_KEY_SIZE+8];
        memset(hashed_ring_key, 0, 16);

        uint8_t protocol_version = gfps_ble_env.protocol_version;
        uint8_t hash256Result[32];
        uint8_t eph_identity_key[FP_EPH_IDENTITY_KEY_LEN+1];
        uint8_t calculatedHmacFirst8Bytes[8];
           
        memcpy(eph_identity_key, nv_record_fp_get_eph_identity_key(), FP_EPH_IDENTITY_KEY_LEN);
        eph_identity_key[FP_EPH_IDENTITY_KEY_LEN] = 0x02;
        gfps_SHA256_hash(eph_identity_key, 33, hash256Result);
        memcpy(hashed_ring_key, hash256Result, 8);                    //ring key;

        uint8_t input_encrpyt_data[15];
        input_encrpyt_data[0] = protocol_version;
        memcpy(&input_encrpyt_data[1], gfps_ble_spot_get_nonce(), 8);
        input_encrpyt_data[9] = data_id;
        input_encrpyt_data[10] = data_length;
        memcpy(&input_encrpyt_data[11], &param[10], 4);
        gfps_beacon_encrpt_data(hashed_ring_key, input_encrpyt_data, 15, calculatedHmacFirst8Bytes);

        if (gfps_ble_env.enable_unwanted_tracking_mode && (gfps_ble_env.control_flag==GFPS_BEACON_CONTROL_FLAG_SKIP_RING_AUT))
        {
            //do not need auth;
            status = GFPS_SUCCESS;
        }
        else
        {
            if (!memcmp(calculatedHmacFirst8Bytes, &param[2], 8))
            {
                TRACE(0,"ring, key matched");
                status = GFPS_SUCCESS;
            }
            else
            {
                TRACE(0,"ring ,key not matched");
                status = GFPS_ERROR_UNAUTHENTICATED;
            }
        }

        if (status == GFPS_SUCCESS)
        {
            uint8_t ring_state;
            memcpy(&ring_state,&param[10],1);
            uint16_t ring_time;
            uint8_t big_ring_time[2];
            uint8_t ring_rsp_time[2];
            uint16_t remaining_ring_time = 0x00;
            //uint8_t ring_volume;

            gfps_ble_spot_beacon_ring_resp beacon_rsp;

            big_little_switch(&param[11], big_ring_time, 2);
            memcpy(&ring_time, big_ring_time, 2);
            ring_time = ring_time/10;
            TRACE(2,"ring_state is %x, remaining_ring_time is %d",ring_state, ring_time);

            if (ring_state != 0x00)
            {
                gfps_ble_spot_ring_timer_set();
            }

            if (ring_state == GFPS_BEACON_RINGING_ALL)
            {
                gfps_start_find_ringtone();
                //ring left and right;
                beacon_rsp.data[0] = GFPS_BEACON_RINGING_STATE_STATED;
                beacon_rsp.data[1] = GFPS_BEACON_RINGING_RIGHT_AND_LEFT;
                remaining_ring_time = (ring_time-spot_ring_nums*2)*10;

                memcpy(ring_rsp_time, &remaining_ring_time, 0x02);
                //memcpy(&beacon_rsp.data[2], ring_rsp_time, 0x02);
                big_little_switch(ring_rsp_time, &beacon_rsp.data[2],2);
                gfps_ble_env.ring_state = true;
            }
            else if (ring_state == GFPS_BEACON_RINGING_RIGHT)
            {
                 //ring right
                gfps_start_find_ringtone();

                beacon_rsp.data[0] = GFPS_BEACON_RINGING_STATE_STATED;
                beacon_rsp.data[1] = GFPS_BEACON_RINGING_RIGHT;
                remaining_ring_time = (ring_time-spot_ring_nums*2)*10;
                
                memcpy(ring_rsp_time, &remaining_ring_time, 0x02);
                big_little_switch(ring_rsp_time, &beacon_rsp.data[2],2);
                gfps_ble_env.ring_state = true;

            }
             else if (ring_state == GFPS_BEACON_RINGING_LEFT)
            {
                //ring left
                gfps_start_find_ringtone();

                beacon_rsp.data[0] = GFPS_BEACON_RINGING_STATE_STATED;
                beacon_rsp.data[1] = GFPS_BEACON_RINGING_LEFT;
                remaining_ring_time = (ring_time-spot_ring_nums*2)*10;
                
                memcpy(ring_rsp_time, &remaining_ring_time, 0x02);
                big_little_switch(ring_rsp_time, &beacon_rsp.data[2],2);
                gfps_ble_env.ring_state = true;
            }
            else if (ring_state == GFPS_BEACON_RINGING_BOX)
            {
                //ring box, box cant't ring in now case, and tell it to the phone;
                //if box can ring, customer can change the reply 
                beacon_rsp.data[0] = GFPS_BEACON_RINGING_STATE_FAILED;
                beacon_rsp.data[1] = GFPS_BEACON_RINGING_BOX;
                remaining_ring_time = 0x00;
                memcpy(&beacon_rsp.data[2],&remaining_ring_time,2);
            }
            else
            {
                //ring none
                gfps_stop_find_ringtone();
                beacon_rsp.data[0] = GFPS_BEACON_RINGING_STATE_STOPPED_REQUEST;
                beacon_rsp.data[1] = GFPS_BEACON_RINGING_NONE;
                remaining_ring_time = 0x00;
                memcpy(&beacon_rsp.data[2],&remaining_ring_time,2);

                spot_ring_nums =0;
                osTimerStop(spot_ring_timeout_timer_id);
            }
            gfps_ble_env.remaining_ring_time = remaining_ring_time;

            uint8_t auth_segment[16];
            auth_segment[0] = protocol_version;
            memcpy(&auth_segment[1], gfps_ble_spot_get_nonce(), 8);
            auth_segment[9] = data_id;
            auth_segment[10] = 0x0c;
            memcpy(&auth_segment[11], beacon_rsp.data, 4);
            auth_segment[15] = 0x01;
            gfps_beacon_encrpt_data(hashed_ring_key, auth_segment, 16, beacon_rsp.auth_data);

            beacon_rsp.data_id = GFPS_BEACON_RING;
            beacon_rsp.data_length = 0x0c;
            gfps_ble_spot_send_beacon_provisioning_state((uint8_t *)&beacon_rsp, sizeof(beacon_rsp));
        }
    }
    else if (data_id == GFPS_BEACON_READ_RING_STATE)     //read ring state,0x06
    {
        //uint8_t hashed_ring_key[GFPS_RING_KEY_SIZE+8];
        memset(hashed_ring_key, 0, 16);

        uint8_t protocol_version = gfps_ble_env.protocol_version;
        uint8_t hash256Result[32];
        uint8_t eph_identity_key[FP_EPH_IDENTITY_KEY_LEN+1];
        uint8_t calculatedHmacFirst8Bytes[8];
        uint8_t ring_rsp_time[2];

        memcpy(eph_identity_key, nv_record_fp_get_eph_identity_key(), FP_EPH_IDENTITY_KEY_LEN);
        eph_identity_key[FP_EPH_IDENTITY_KEY_LEN] = 0x02;
        gfps_SHA256_hash(eph_identity_key, 33, hash256Result);
        memcpy(hashed_ring_key, hash256Result, 8);                    //ring key;

        uint8_t input_encrpyt_data[11];
        input_encrpyt_data[0] = protocol_version;
        memcpy(&input_encrpyt_data[1], gfps_ble_spot_get_nonce(), 8);
        input_encrpyt_data[9] = data_id;
        input_encrpyt_data[10] = data_length;
        gfps_beacon_encrpt_data(hashed_ring_key, input_encrpyt_data, 11, calculatedHmacFirst8Bytes);

        if (!memcmp(calculatedHmacFirst8Bytes, &param[2], 8))
        {
            TRACE(0,"matched");
            status = GFPS_SUCCESS;
            uint16_t remaining_time;
            gfps_ble_spot_read_beacon_ring_state_resp beacon_rsp;

            beacon_rsp.data_id = GFPS_BEACON_READ_RING_STATE;
            beacon_rsp.data_length = 0x0b;
            if(!gfps_ble_env.ring_state)
            {
                beacon_rsp.data[0] = GFPS_BEACON_INCAPABLE_OF_RING;
                remaining_time = 0x00;
            }
            else
            {
                beacon_rsp.data[0] = GFPS_BEACON_TWO_CAPABLE_OF_RING;
                remaining_time = gfps_ble_env.remaining_ring_time;
            }
            memcpy(ring_rsp_time, &remaining_time,  sizeof(uint16_t));
            big_little_switch(ring_rsp_time, &beacon_rsp.data[1], sizeof(uint16_t));

            uint8_t auth_segment[15];
            auth_segment[0] = protocol_version;
            memcpy(&auth_segment[1], gfps_ble_spot_get_nonce(), 8);
            auth_segment[9] = data_id;
            auth_segment[10] = 0x0b;
            memcpy(&auth_segment[11], beacon_rsp.data, 3);
            auth_segment[14] = 0x01;
            gfps_beacon_encrpt_data(hashed_ring_key, auth_segment, 15, beacon_rsp.auth_data);

            gfps_ble_spot_send_beacon_provisioning_state((uint8_t *)&beacon_rsp, sizeof(beacon_rsp));
        }
        else
        {
            TRACE(0,"not matched");
            status = GFPS_ERROR_UNAUTHENTICATED;
        }
    }
    else if (data_id == GFPS_BEACON_ACTIVATE_UNWANTED_TRACK_MODE)      //0x07 
    {
        bes_ble_gap_set_rpa_timeout(0xA1B8);
        gfps_ble_env.enable_unwanted_tracking_mode = true;  //frame type is 0x41
        gfps_ble_env.orignal_flag= GFPS_BEACON_NORMAL_BATTERY | GFPS_BEACON_IN_TRACKING_MODE;   //0x80 don't support battery level indication;
        uint8_t hashed_orignal_value[20] = {0};
        uint8_t hashed_output_value[32];
        big_little_switch(gfps_ble_env.Ecc_private_key, hashed_orignal_value, 20);
        gfps_SHA256_hash(hashed_orignal_value, 20, hashed_output_value);
        gfps_ble_env.hashed_flag = hashed_output_value[31];
        nv_record_fp_update_spot_hash_value(gfps_ble_env.hashed_flag);
        //refresh adv
        if ( nv_record_fp_get_spot_adv_enable_value() == true)
        {
             app_ble_refresh_adv_state_generic();
        }

        uint8_t unwanted_track_key[GFPS_RING_KEY_SIZE+8];
        memset(unwanted_track_key, 0, 16);

        uint8_t protocol_version = gfps_ble_env.protocol_version;
        uint8_t hash256Result[32];
        uint8_t eph_identity_key[FP_EPH_IDENTITY_KEY_LEN+1];
        uint8_t calculatedHmacFirst8Bytes[8];
           
        memcpy(eph_identity_key, nv_record_fp_get_eph_identity_key(), FP_EPH_IDENTITY_KEY_LEN);
        eph_identity_key[FP_EPH_IDENTITY_KEY_LEN] = 0x03;
        gfps_SHA256_hash(eph_identity_key, 33, hash256Result);
        memcpy(unwanted_track_key, hash256Result, 8);                    //unwanted_track_key key;

        if (param[1] == 0x09)
        {
            gfps_ble_env.control_flag = param[10];  //if param is 0x01,just ring not need auth,
            uint8_t input_encrpyt_data[12];
            input_encrpyt_data[0] = protocol_version;
            memcpy(&input_encrpyt_data[1], gfps_ble_spot_get_nonce(), 8);
            input_encrpyt_data[9] = data_id;
            input_encrpyt_data[10] = data_length;
            input_encrpyt_data[11] = param[10];
            gfps_beacon_encrpt_data(unwanted_track_key, input_encrpyt_data, 12, calculatedHmacFirst8Bytes);
        }
        else
        {
            uint8_t input_encrpyt_data[11];
            input_encrpyt_data[0] = protocol_version;
            memcpy(&input_encrpyt_data[1], gfps_ble_spot_get_nonce(), 8);
            input_encrpyt_data[9] = data_id;
            input_encrpyt_data[10] = data_length;
            gfps_beacon_encrpt_data(unwanted_track_key, input_encrpyt_data, 11, calculatedHmacFirst8Bytes);
        }

        if (!memcmp(calculatedHmacFirst8Bytes, &param[2], 8))
        {
            TRACE(0,"acitve unwanted, key matched");
            status = GFPS_SUCCESS;
        }
        else
        {
            TRACE(0,"acitve unwanted, key not matched");
            status = GFPS_ERROR_UNAUTHENTICATED;
        }

        if (status == GFPS_SUCCESS)
        {
            gfps_ble_spot_activate_unwanted_tracking_mode_resp beacon_rsp;
            beacon_rsp.data_id = data_id;
            beacon_rsp.data_length = 0x08;

            uint8_t auth_segment[12];
            auth_segment[0] = protocol_version;
            memcpy(&auth_segment[1], gfps_ble_spot_get_nonce(), 8);
            auth_segment[9] = data_id;
            auth_segment[10]= 0x08;
            auth_segment[11] = 0x01;

            gfps_beacon_encrpt_data(unwanted_track_key, auth_segment, 12, beacon_rsp.auth_data);
            gfps_ble_spot_send_beacon_provisioning_state((uint8_t *)&beacon_rsp, sizeof(beacon_rsp));
        }

    }
    else if (data_id == GFPS_BEACON_DEACTIVATE_UNWANTED_TRACK_MODE)
    {
        hwtimer_stop(spot_EID_timeout_hard_timer_id);
        gfps_ble_spot_rotate_hard_timer_start();
        gfps_ble_env.enable_unwanted_tracking_mode = false;
        gfps_ble_env.orignal_flag= GFPS_BEACON_NORMAL_BATTERY | GFPS_BEACON_IN_UNTRACKING_MODE;
        gfps_ble_spot_generate_hashed_value();
        nv_record_fp_update_spot_hash_value(gfps_ble_env.hashed_flag);
        //refresh adv
        if( nv_record_fp_get_spot_adv_enable_value() == true)
        {
             app_ble_refresh_adv_state_generic();
        }

        uint8_t unwanted_track_key[GFPS_RING_KEY_SIZE+8];
        memset(unwanted_track_key, 0, 16);
        uint8_t protocol_version = gfps_ble_env.protocol_version;
        uint8_t hash256Result[32];
        uint8_t eph_identity_key[FP_EPH_IDENTITY_KEY_LEN+1];
        uint8_t calculatedHmacFirst8Bytes[8];
        uint8_t eph_identity_key_nonce[FP_EPH_IDENTITY_KEY_LEN+8];
        uint8_t hashed_of_eik_and_nonce[32];
           
        memcpy(eph_identity_key, nv_record_fp_get_eph_identity_key(), FP_EPH_IDENTITY_KEY_LEN);
        eph_identity_key[FP_EPH_IDENTITY_KEY_LEN] = 0x03;
        gfps_SHA256_hash(eph_identity_key, 33, hash256Result);
        memcpy(unwanted_track_key, hash256Result, 8);                    //unwanted_track_key key;

        memcpy(eph_identity_key_nonce, nv_record_fp_get_eph_identity_key(), FP_EPH_IDENTITY_KEY_LEN);
        memcpy(&eph_identity_key_nonce[32], gfps_ble_spot_get_nonce(), 8);
        gfps_SHA256_hash(eph_identity_key_nonce, 40, hashed_of_eik_and_nonce);

        uint8_t input_encrpyt_data[19];
        input_encrpyt_data[0] = protocol_version;
        memcpy(&input_encrpyt_data[1], gfps_ble_spot_get_nonce(), 8);
        input_encrpyt_data[9] = data_id;
        input_encrpyt_data[10] = data_length;
        memcpy(&input_encrpyt_data[11], &param[10], 8);
        gfps_beacon_encrpt_data(unwanted_track_key, input_encrpyt_data, 19, calculatedHmacFirst8Bytes);

        if (!memcmp(calculatedHmacFirst8Bytes, &param[2], 8))
        {
            TRACE(0,"matched");

            if(!memcmp(hashed_of_eik_and_nonce, &param[10], 8))
            {
                TRACE(0,"track key matched");
                status = GFPS_SUCCESS;
            }
            else
            {
                TRACE(0, "track key not matched");
                status = GFPS_ERROR_UNAUTHENTICATED;
            }

        }
        else
        {
            TRACE(0,"not matched");
            status = GFPS_ERROR_UNAUTHENTICATED;
        }

        if(status == GFPS_SUCCESS)
        {
            gfps_ble_spot_deactivate_unwanted_tracking_mode_resp beacon_rsp;
            beacon_rsp.data_id = data_id;
            beacon_rsp.data_length = 0x08;

            uint8_t auth_segment[12];
            auth_segment[0] = protocol_version;
            memcpy(&auth_segment[1], gfps_ble_spot_get_nonce(), 8);
            auth_segment[9] = data_id;
            auth_segment[10]= 0x08;
            auth_segment[11] = 0x01;
            gfps_beacon_encrpt_data(unwanted_track_key, auth_segment, 12, beacon_rsp.auth_data);
            
            gfps_ble_spot_send_beacon_provisioning_state((uint8_t *)&beacon_rsp, sizeof(beacon_rsp));
        }
    }
    else if(data_id == GFPS_BEACON_MULITI_OPERATION)
    {
        uint8_t keyCount = nv_record_fp_account_key_count();
        uint8_t protocol_version = gfps_ble_env.protocol_version;
        uint8_t nonce[8];
        memcpy(nonce, gfps_ble_spot_get_nonce(),8);

        if(keyCount == 0)
        {
            status = GFPS_ERROR_UNAUTHENTICATED;
        }
        for (uint8_t keyIndex = 0; keyIndex < keyCount; keyIndex++)
        {
            uint8_t VArray[FP_ACCOUNT_KEY_SIZE];
            uint8_t calculatedHmacFirst8Bytes[8];
            if(param[10] == READ_COMPONENTS_STATE)
            {
                uint8_t input_encrpyt_data[12];
                input_encrpyt_data[0] = protocol_version;
                memcpy(&input_encrpyt_data[1], nonce, 8);
                input_encrpyt_data[9] = data_id;
                input_encrpyt_data[10] = data_length;
                input_encrpyt_data[11] = 0x01;
    
                nv_record_fp_account_key_get_by_index(keyIndex, VArray);
                gfps_beacon_encrpt_data(VArray, input_encrpyt_data, 12, calculatedHmacFirst8Bytes);
            }
            if(param[10] == ACTIVATE_MULTI_TRACKING_MODE)
            {
                uint8_t input_encrpyt_data[14];
                input_encrpyt_data[0] = protocol_version;
                memcpy(&input_encrpyt_data[1], nonce, 8);
                input_encrpyt_data[9] = data_id;
                input_encrpyt_data[10] = data_length;
                memcpy(&input_encrpyt_data[11], &param[10], 3);
    
                nv_record_fp_account_key_get_by_index(keyIndex, VArray);
                gfps_beacon_encrpt_data(VArray, input_encrpyt_data, 14, calculatedHmacFirst8Bytes);
            }

            if(!memcmp(calculatedHmacFirst8Bytes, &param[2], 8))
            {
               TRACE(0,"matched");
               status = GFPS_SUCCESS;
               memcpy(matched_account_key, VArray, FP_ACCOUNT_KEY_SIZE);
               break;
            }
            else
            {
                TRACE(0,"not matched");
                status = GFPS_ERROR_UNAUTHENTICATED;
            }
        }

        if(status == GFPS_SUCCESS)
        {
            if(param[10] == READ_COMPONENTS_STATE)
            {
                uint8_t batteryFollowingData[GFPS_BATTERY_VALUE_MAX_COUNT];
                uint8_t batteryLevelCount = 0;

                gfps_ble_spot_read_components_state_resp beacon_rsp;
                beacon_rsp.data_id = GFPS_BEACON_MULITI_OPERATION;
                beacon_rsp.data_length = 0x0d;
                beacon_rsp.data[0] = READ_COMPONENTS_STATE;
                beacon_rsp.data[1] = GFPS_BEACON_RIGHT_BUD_OUT_OF_CASE | GFPS_BEACON_LEFT_BUD_OUT_OF_CASE;

                gfps_get_battery_levels(&batteryLevelCount, batteryFollowingData);
                beacon_rsp.data[2] = batteryFollowingData[0];
                beacon_rsp.data[3] = batteryFollowingData[1];
                beacon_rsp.data[4] = batteryFollowingData[2];

                uint8_t auth_segment[17];
                auth_segment[0] = protocol_version;
                memcpy(&auth_segment[1], gfps_ble_spot_get_nonce(), 8);
                auth_segment[9] = data_id;
                auth_segment[10] = 0x0d;
                memcpy(&auth_segment[11], beacon_rsp.data, 5);
                auth_segment[16] = 0x01;
                gfps_beacon_encrpt_data(matched_account_key, auth_segment, 17, beacon_rsp.auth_data);

                gfps_ble_spot_send_beacon_provisioning_state((uint8_t *)&beacon_rsp, sizeof(beacon_rsp));
                
            }
            else if(param[10] == ACTIVATE_MULTI_TRACKING_MODE)
            {
                gfps_ble_spot_activate_tracking_mode_resp beacon_rsp;
                beacon_rsp.data_id = GFPS_BEACON_MULITI_OPERATION;
                beacon_rsp.data_length = 0x09;

                uint8_t auth_segment[13];
                auth_segment[0] = protocol_version;
                memcpy(&auth_segment[1], gfps_ble_spot_get_nonce(), 8);
                auth_segment[9] = data_id;
                auth_segment[10] = 0x09;
                auth_segment[11] = 0x02;
                auth_segment[12] = 0x01; 
                gfps_beacon_encrpt_data(matched_account_key, auth_segment, 13, beacon_rsp.auth_data);
                beacon_rsp.data = ACTIVATE_MULTI_TRACKING_MODE;

                gfps_ble_spot_send_beacon_provisioning_state((uint8_t *)&beacon_rsp, sizeof(beacon_rsp));
            }
            else
            {
                TRACE(0,"no this components");
            }
        }
    }
    else
    {
        TRACE(0,"error, no this data id");
    }

     return status;
}

void gfps_ble_spot_press_stop_ring_handle(APP_KEY_STATUS *status, void *param)
{
    TRACE(0,"gfps_ble_spot_press_stop_ring_handle");
    gfps_ble_spot_beacon_ring_resp beacon_rsp;

    uint16_t remaining_ring_time;
    uint8_t rsp_time[2];

    remaining_ring_time = (gfps_ble_env.remaining_ring_time/10-spot_ring_nums*2)*10;
    gfps_stop_find_ringtone();
    beacon_rsp.data[0] = GFPS_BEACON_RINGING_STATE_STOPPED_PRESS;
    beacon_rsp.data[1] = GFPS_BEACON_INCAPABLE_OF_RING;


    memcpy(rsp_time, &remaining_ring_time ,2);
     
    big_little_switch(rsp_time, &beacon_rsp.data[2],2);
    spot_ring_nums=0;
    osTimerStop(spot_ring_timeout_timer_id);

    uint8_t auth_segment[16];
    auth_segment[0] = gfps_ble_get_spot_version();
    memcpy(&auth_segment[1], gfps_ble_spot_get_nonce(), 8);
    auth_segment[9] = GFPS_BEACON_RING;
    auth_segment[10] = 0x0c;
    memcpy(&auth_segment[11], beacon_rsp.data, 4);
    auth_segment[15] = 0x01;
    gfps_beacon_encrpt_data(hashed_ring_key, auth_segment, 16, beacon_rsp.auth_data);
    
    beacon_rsp.data_id = GFPS_BEACON_RING;
    beacon_rsp.data_length = 0x0c;

    gfps_ble_spot_send_beacon_provisioning_state((uint8_t *)&beacon_rsp, sizeof(beacon_rsp));
}

void gfps_ble_spot_init(void)
{
    gfps_ble_set_protocol_version();
    gfps_ble_env.orignal_flag = GFPS_BEACON_NORMAL_BATTERY;
    gfps_ble_env.hashed_flag = 0x00;
}

void app_gfps_spot_press_to_factory_handle(APP_KEY_STATUS *status, void *param)
{
    TRACE(1,"%s", __func__);
    nv_record_fp_account_key_delete();
}

bool gfps_ble_get_spot_get_mode(void)
{
    return gfps_ble_env.enable_unwanted_tracking_mode;
}

uint8_t* gfps_ble_get_EID(void)
{
    return gfps_ble_env.adv_identifer;
}

uint8_t gfps_ble_get_hashed_value(void)
{
    return gfps_ble_env.hashed_flag;
}

#endif

void gfps_process_bt_user_confirmation(struct bdaddr_t *bdaddr, uint32 numeric_value)
{
    uint8_t passkey[GFPS_PASSKEY_LEN];
    memcpy(passkey, &numeric_value, GFPS_PASSKEY_LEN);
    big_little_switch(passkey, gfps_ble_env.passkey, GFPS_PASSKEY_LEN);

    bes_bt_me_confirmation_resp(bdaddr, true);
}
static uint8_t gfps_ble_handle_decrypted_keybase_pairing_request(gfps_ble_req_resp *raw_req, uint8_t *out_key)
{
    uint8_t rawData[KEY_BASE_RSP_LEN] = {0};
    uint8_t rawDataLen = 0, offsetOfSalt = 0, saltLen = 0;
    gfps_ble_encrypted_resp en_rsp;

    memcpy(gfps_ble_env.keybase_pair_key, out_key, 16);
    memcpy(&gfps_ble_env.seeker_bt_addr.address[0] ,&raw_req->rx_tx.key_based_pairing_req.seeker_addr[0],6);
    if(raw_req->rx_tx.key_based_pairing_req.flags_discoverability ==RAW_REQ_FLAGS_DISCOVERABILITY_BIT0_EN)
    {
        TRACE(0,"TODO discoverable 10S");
        //TODO start a timer keep discoverable 10S...
        //TODO make sure there is no ble ADV with the MODEL ID data
    }

    if(!raw_req->rx_tx.key_based_pairing_req.flags_support_le_audio && !raw_req->rx_tx.key_based_pairing_req.flags_retroactively_write_account_key)
    {
        bes_bt_me_confirmation_register_callback(gfps_process_bt_user_confirmation);
    }

#if BLE_AUDIO_ENABLED
    if (raw_req->rx_tx.key_based_pairing_req.flags_support_le)
    {
#ifdef BT_SERVICE_ENABLE
        app_dev_mgr_user_trigger_paring_adv(true);
#endif

        gfps_ble_raw_ext_resp *ext_rsp = (gfps_ble_raw_ext_resp *)rawData;
        saltLen = KEY_BASE_EXT_RSP_SALT_LEN;

        ext_rsp->message_type = KEY_BASED_PAIRING_EXT_RSP;// Key-based Pairing Response
        ext_rsp->flags_is_le_only = 0;
        ext_rsp->addr_type = 0;
        ext_rsp->addrNum = 1;

        if (!raw_req->rx_tx.key_based_pairing_req.flags_support_le_audio)
        {
            ext_rsp->flags_ble_bonding = 0;
        }
        else if (gfps_ble_env.bondMode == GFPS_BOND_OVER_BT)
        {
            ext_rsp->flags_ble_bonding = 0;
        }
        else
        {
            ext_rsp->flags_ble_bonding = 1;
        }

#if BLE_AUDIO_ENABLED
        if(raw_req->rx_tx.key_based_pairing_req.flags_support_le_audio)
        {
            ext_rsp->addrNum = bes_ble_aob_csip_if_get_device_numbers();
        }
        else
        {
            ext_rsp->addrNum = 1;
        }
#endif

        if (ext_rsp->addrNum == 2)
        {
            big_little_switch(nv_record_get_ibrt_peer_addr(), ext_rsp->peerAddrandSalt, sizeof(bt_bdaddr_t));
            saltLen -= sizeof(bt_bdaddr_t);
            if(!raw_req->rx_tx.key_based_pairing_req.flags_retroactively_write_account_key)
            {
                gfps_set_flag(true);
                gfps_send_flag_to_slave(gfps_get_flag());
            }
        }


        memcpy(ext_rsp->local_addr, gfps_ble_env.local_bt_addr.address, sizeof(bt_bdaddr_t));
        rawDataLen = sizeof(gfps_ble_raw_ext_resp);
    }
    else
#endif
    {
        gfps_ble_raw_resp *raw_rsp = (gfps_ble_raw_resp *)rawData;
        saltLen = KEY_BASE_RSP_SALT_LEN;
        raw_rsp->message_type = KEY_BASED_PAIRING_RSP;// Key-based Pairing Response
        memcpy(raw_rsp->provider_addr, gfps_ble_env.local_bt_addr.address, sizeof(bt_bdaddr_t));

        rawDataLen = sizeof(gfps_ble_raw_resp);
    }

    offsetOfSalt = rawDataLen - saltLen;
    for (uint8_t index = 0; index < saltLen; index++)
    {
        rawData[offsetOfSalt + index] = ( uint8_t )rand();
    }

    TRACE(2,"%d %d raw_rsp:", offsetOfSalt, rawDataLen);
    DUMP8("%02x ", rawData, 16);

    gfps_crypto_encrypt(rawData, rawDataLen, gfps_ble_env.keybase_pair_key, en_rsp.uint128_array);

    TRACE(1,"message type is 0x%x", raw_req->rx_tx.raw_req.message_type);
    DUMP8("%2x ", &(raw_req->rx_tx.key_based_pairing_req), 16);
    TRACE(6,"req bit 0: %d, bit 1: %d, bit 2: %d, bit 3: %d, bit 4:%d, bit 5:%d",
        raw_req->rx_tx.key_based_pairing_req.flags_discoverability,
        raw_req->rx_tx.key_based_pairing_req.flags_bonding_addr,
        raw_req->rx_tx.key_based_pairing_req.flags_get_existing_name,
        raw_req->rx_tx.key_based_pairing_req.flags_retroactively_write_account_key,
        raw_req->rx_tx.key_based_pairing_req.flags_support_le,
        raw_req->rx_tx.key_based_pairing_req.flags_support_le_audio);

    bool isReturnName = raw_req->rx_tx.key_based_pairing_req.flags_get_existing_name;

    if(raw_req->rx_tx.key_based_pairing_req.flags_bonding_addr == RAW_REQ_FLAGS_INTBONDING_SEEKERADDR_BIT1_EN)
    {
        TRACE(0,"try connect to remote BR/EDR addr");
        // TODO:
        bes_ble_gfps_send_keybase_pairing(gfps_ble_env.connectionIndex, ( uint8_t * )en_rsp.uint128_array, sizeof(en_rsp));
    }
    else if (raw_req->rx_tx.key_based_pairing_req.flags_retroactively_write_account_key)
    {
        // check whether the seeker's bd address is the same as already connected mobile
        uint8_t swapedBtAddr[6];
        big_little_switch(gfps_ble_env.seeker_bt_addr.address, swapedBtAddr, sizeof(swapedBtAddr));

        /*uint8_t isMatchMobileAddr = false;
        for (uint32_t devId = 0; devId < app_bt_count_connected_device(); devId++)
        {
            uint8_t connectedAddr[6];
            app_get_connected_device_bdaddr(devId, connectedAddr);
            if (!memcmp(connectedAddr, swapedBtAddr, 6))
            {
                isMatchMobileAddr = true;
                break;
            }
        }*/

        //if (isMatchMobileAddr)
        {
            bes_ble_gfps_send_keybase_pairing(gfps_ble_env.connectionIndex, ( uint8_t * )en_rsp.uint128_array, sizeof(en_rsp));
        }
        //else
        //{
            // reject the write request
            //return BT_STS_INVALID_VALUE;
        //}
    }
    else if (raw_req->rx_tx.key_based_pairing_req.flags_bonding_addr == RAW_REQ_FLAGS_INTBONDING_SEEKERADDR_BIT1_DIS)
    {
        gfps_enter_connectable_mode_req_handler((uint8_t *)en_rsp.uint128_array);
    }
    else
    {
        bes_ble_gfps_send_keybase_pairing(gfps_ble_env.connectionIndex, ( uint8_t * )en_rsp.uint128_array, sizeof(en_rsp));
    }

    if (isReturnName)
    {
        gfps_ble_env.isPendingForWritingNameReq = true;
        TRACE(0,"get existing name.");
        uint8_t response[16+FP_MAX_NAME_LEN];
        uint8_t* ptrRawName;
        uint32_t rawNameLen;
        ptrRawName = nv_record_fp_get_name_ptr(&rawNameLen);

        gfps_encrypt_name(gfps_ble_env.keybase_pair_key,
                          ptrRawName,
                          rawNameLen,
                          &response[16],
                          response,
                          &response[8]);

        ble_app_gfps_send_naming_packet(gfps_ble_env.connectionIndex, response, 16 + rawNameLen);
    }
    else
    {
        TRACE(0,"Unusable bit.");
    }

    return BT_STS_SUCCESS;
}

static bool gfps_ble_decrypt_keybase_pairing_request(uint8_t *pairing_req, uint8_t *output)
{
    uint8_t keyCount = nv_record_fp_account_key_count();
    if (0 == keyCount)
    {
        return false;
    }

    gfps_ble_req_resp raw_req;
    uint8_t accountKey[FP_ACCOUNT_KEY_SIZE];
    for (uint8_t keyIndex = 0; keyIndex < keyCount; keyIndex++)
    {
        nv_record_fp_account_key_get_by_index(keyIndex, accountKey);

        AES128_ECB_decrypt(pairing_req, ( const uint8_t * )accountKey, ( uint8_t * )&raw_req);
        TRACE(0,"Decrypted keybase pairing req result:");
        DUMP8("0x%02x ", ( uint8_t * )&raw_req, 16);

        if ((memcmp(raw_req.rx_tx.key_based_pairing_req.provider_addr,
                gfps_ble_env.local_bt_addr.address , 6)==0) ||
             (memcmp(raw_req.rx_tx.key_based_pairing_req.provider_addr,
                 gfps_ble_env.local_le_addr.address , 6)==0))
        {
            memcpy(output, accountKey, FP_ACCOUNT_KEY_SIZE);
            TRACE(1,"fp message type 0x%02x.", raw_req.rx_tx.raw_req.message_type);
            if (KEY_BASED_PAIRING_REQ == raw_req.rx_tx.raw_req.message_type)
            {
                gfps_ble_handle_decrypted_keybase_pairing_request(&raw_req, accountKey);
                return true;
            }
            else if (ACTION_REQUEST == raw_req.rx_tx.raw_req.message_type)
            {
                memcpy(gfps_ble_env.keybase_pair_key, accountKey, 16);
                memcpy(&gfps_ble_env.seeker_bt_addr.address[0],
                    &(raw_req.rx_tx.key_based_pairing_req.seeker_addr[0]), 6);
                gfps_ble_encrypted_resp  en_rsp;
                gfps_ble_raw_resp  raw_rsp;

                raw_rsp.message_type = KEY_BASED_PAIRING_RSP;// Key-based Pairing Response
                memcpy(raw_rsp.provider_addr, gfps_ble_env.local_bt_addr.address, 6);

                TRACE(0,"raw_rsp.provider_addr:");
                DUMP8("%02x ",raw_rsp.provider_addr, BT_ADDR_OUTPUT_PRINT_NUM);

                for (uint8_t index = 0;index < 9;index++)
                {
                    raw_rsp.salt[index] = (uint8_t)rand();
                }

                gfps_crypto_encrypt((const uint8_t *)(&raw_rsp.message_type), sizeof(raw_rsp),
                    gfps_ble_env.keybase_pair_key,en_rsp.uint128_array);

                bes_ble_gfps_send_keybase_pairing(gfps_ble_env.connectionIndex, (uint8_t *)en_rsp.uint128_array, sizeof(en_rsp));

                if (raw_req.rx_tx.action_req.isDeviceAction)
                {
                    // TODO: device action via BLE
                }
                else if (raw_req.rx_tx.action_req.isFollowedByAdditionalDataCh)
                {
                    // write name request will be received
                    TRACE(0,"FP write name request will be received.");
                    gfps_ble_env.isPendingForWritingNameReq = true;
                }
                return true;
            }
        }


    }

    return false;
}

static uint8_t gfps_ble_write_key_based_pairing_handler(uint8_t condix, uint8_t *param, uint16_t len)
{
    uint8_t status = BES_GATT_NO_ERR;
    if (!param)
    {
        return BES_GATT_ERR_INVALID_PERM;
    }

    gfps_ble_key_based_pairing_req en_req;
    gfps_ble_req_resp *ptr_raw_req;

    en_req.en_req            = ( gfps_ble_encrypted_req_uint128 * )&(param[0]);
    en_req.pub_key           = ( gfps_ble_64B_public_key * )&(param[16]);
    uint8_t out_key[16]      = {0};
    uint8_t decryptdata[16]  = {0};

    TRACE(3,"length = %d value = 0x%x  0x%x", len, param[0], param[1]);
    DUMP8("%02x ", param, 80);

    if (len == GFPSP_KEY_BASED_PAIRING_REQ_LEN_WITH_PUBLIC_KEY)
    {
        memset(gfps_ble_env.keybase_pair_key, 0, 16);
        uint32_t gfps_state = gfps_crypto_get_secret_decrypt(( const uint8_t * )en_req.en_req,
                                                             ( const uint8_t * )en_req.pub_key,
                                                             out_key,
                                                             decryptdata);
        if (gfps_state == BES_GATT_NO_ERR)
        {
            memcpy(gfps_ble_env.aesKeyFromECDH, out_key, 16);
            gfps_ble_env.isInitialPairing = true;
            ptr_raw_req = (gfps_ble_req_resp *)decryptdata;
            TRACE(0,"raw req provider's addr:");
            DUMP8("%02x ", ptr_raw_req->rx_tx.key_based_pairing_req.provider_addr, BT_ADDR_OUTPUT_PRINT_NUM);
            TRACE(0,"raw req seeker's addr:");
            DUMP8("%02x ", ptr_raw_req->rx_tx.key_based_pairing_req.seeker_addr, BT_ADDR_OUTPUT_PRINT_NUM);
            TRACE(1,"fp message type 0x%02x.", ptr_raw_req->rx_tx.raw_req.message_type);
            if ((KEY_BASED_PAIRING_REQ == ptr_raw_req->rx_tx.raw_req.message_type) &&
            ((memcmp(ptr_raw_req->rx_tx.key_based_pairing_req.provider_addr,
                gfps_ble_env.local_bt_addr.address , 6)==0) ||
             (memcmp(ptr_raw_req->rx_tx.key_based_pairing_req.provider_addr,
                 gfps_ble_env.local_le_addr.address , 6)==0)))
            {
                status = gfps_ble_handle_decrypted_keybase_pairing_request(ptr_raw_req, out_key);
            }
            else
            {
                TRACE(0,"decrypt false..ingore");
            }

        }else{
            TRACE(1,"error = %x",gfps_state);
        }
    }
    else if (len == GFPSP_KEY_BASED_PAIRING_REQ_LEN_WITHOUT_PUBLIC_KEY)
    {
        gfps_ble_env.isInitialPairing = false;
        bool isDecryptedSuccessful =
            gfps_ble_decrypt_keybase_pairing_request(( uint8_t * )en_req.en_req, out_key);
        TRACE(1,"Decrypt keybase pairing req without public key result: %d", isDecryptedSuccessful);
    }
    else
    {
        TRACE(0,"who you are??");
    }

    return status;
}

static uint8_t gfps_ble_write_passkey_ind_handler(uint8_t conidx, uint8_t *param, uint16_t len)
{
    gfps_ble_raw_pass_key_resp raw_rsp;
    gfps_ble_encrypted_resp en_rsp;
    uint8_t decryptdata[16] = {0};
    TRACE(1,"length = %d value = 0x", len);
    DUMP8("%02X, ", param, 16);
    gfps_crypto_decrypt(param, 16, gfps_ble_env.keybase_pair_key, decryptdata);
    TRACE(0,"decrypt data =0x");
    TRACE(0,"===============================");
    DUMP8("%02X", decryptdata, 16);
    TRACE(0,"===============================");

    TRACE(0,"pass key = 1-3 bytes");
    if(!memcmp(&decryptdata[1], gfps_ble_env.passkey, 3))
    {
        raw_rsp.message_type = 0x03;  //Provider's passkey
        raw_rsp.passkey[0]   = decryptdata[1];
        raw_rsp.passkey[1]   = decryptdata[2];
        raw_rsp.passkey[2]   = decryptdata[3];
        raw_rsp.reserved[0]  = 0x38;  //my magic num  temp test
        raw_rsp.reserved[1]  = 0x30;
        raw_rsp.reserved[2]  = 0x23;
        raw_rsp.reserved[3]  = 0x30;
        raw_rsp.reserved[4]  = 0x06;
        raw_rsp.reserved[5]  = 0x10;
        raw_rsp.reserved[6]  = 0x05;
        raw_rsp.reserved[7]  = 0x13;
        raw_rsp.reserved[8]  = 0x06;
        raw_rsp.reserved[9]  = 0x12;
        raw_rsp.reserved[10] = 0x12;
        raw_rsp.reserved[11] = 0x01;
        gfps_crypto_encrypt(( const uint8_t * )(&raw_rsp.message_type), sizeof(raw_rsp), gfps_ble_env.keybase_pair_key, en_rsp.uint128_array);
        ble_app_gfps_send_passkey(conidx, ( uint8_t * )en_rsp.uint128_array, sizeof(en_rsp));
    }
    else
    {
        TRACE(0,"PIN error");
        return BES_GATT_ERR_INVALID;
    }

    return BES_GATT_NO_ERR;
}

static bool gfps_if_send_passkey = false;
void gfps_set_if_send_passkey(bool flag)
{
    gfps_if_send_passkey = flag;
}
bool gfps_get_if_send_passkey()
{
    return gfps_if_send_passkey;
}
bool gfps_get_flag()
{
    return gfps_ble_env.gfps_flag;
}
void gfps_set_flag(bool flag)
{
    gfps_ble_env.gfps_flag = flag;
}
void gfps_send_flag_to_slave(bool flag)
{
    tws_ctrl_send_cmd(APP_TWS_CMD_SNED_GFPS_FLAG_INFO, (uint8_t *)&flag, 1);
}

void gfps_send_additional_passkey_to_slave(uint8_t *p_buff, uint16_t length)
{
     tws_ctrl_send_cmd(APP_TWS_CMD_SNED_GFPS_PASSKEY, p_buff, length);
}
uint8_t* gfps_get_keybase_pair_key()
{
    return gfps_ble_env.keybase_pair_key;
}
uint8_t* gfps_get_passkey()
{
    return gfps_ble_env.passkey;
}
void gfps_set_pass_key(uint8_t *p_buff, uint16_t length)
{
    memcpy(gfps_ble_env.passkey, p_buff, length);
}

uint8_t* gfps_get_additional_passkey()
{
    return gfps_ble_env.addition_passkey;
}
void gfps_set_additional_pass_key(uint8_t *p_buff, uint16_t length)
{
    memcpy(gfps_ble_env.addition_passkey, p_buff, length);
}

static uint8_t  gfps_ble_write_additional_passkey_ind_handler(uint8_t conidx, uint8_t *param, uint16_t len)
{
    POSSIBLY_UNUSED gfpsp_ble_additional_pass_key_resp raw_rsp;
    uint8_t decryptdata[16] = {0};
    TRACE(2,"%s, length = %d", __func__, len);
    DUMP8("%02X, ", param, 16);
    gfps_crypto_decrypt(param, 16, gfps_ble_env.keybase_pair_key, decryptdata);
    TRACE(0,"decrypt data =0x");
    DUMP8("%02X", decryptdata, 16);
    raw_rsp.message_type = GFPS_PROVIDER_PASSKEY_KEY;
    raw_rsp.passkey[0]   = decryptdata[1];
    raw_rsp.passkey[1]   = decryptdata[2];
    raw_rsp.passkey[2]   = decryptdata[3];

#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
    gfps_send_additional_passkey_to_slave(raw_rsp.passkey, sizeof(raw_rsp.passkey));
#endif
    return BES_GATT_NO_ERR;
}


static uint8_t gfps_ble_write_name_hander(uint8_t conidx, uint8_t *param, uint16_t len)
{
    bool isSuccessful = false;
    uint8_t status = BES_GATT_NO_ERR;
    if (!gfps_ble_env.isPendingForWritingNameReq)
    {
        TRACE(0,"Pre fp write name request is not received.");
    }
    else
    {
        uint8_t rawName[FP_MAX_NAME_LEN];
        uint8_t *key = NULL;
        if (gfps_ble_env.isInitialPairing)
        {
            key = gfps_ble_env.aesKeyFromECDH;
        }
        else
        {
            key = gfps_ble_env.keybase_pair_key;
        }
        isSuccessful = gfps_decrypt_name(key, param, &(param[8]), &(param[16]), rawName, len-16);

        TRACE(2,"write name successful flag %d %d", isSuccessful, conidx);

        if (isSuccessful)
        {
            nv_record_fp_update_name(rawName, len-16);
            TRACE(1,"Rename BT name: [%s]", rawName);
        #if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
            app_tws_send_fastpair_info_to_slave();
        #endif
        }

        gfps_ble_env.isPendingForWritingNameReq = false;
    }

    if (isSuccessful)
    {
        status = BES_GATT_NO_ERR;
    }
    else
    {
        status = BES_GATT_ERR_INVALID_PERM;
    }

    return status;
}

static uint8_t gfps_ble_write_accountkey_hander(uint8_t conidx, uint8_t *param, uint16_t len)
{
    NV_FP_ACCOUNT_KEY_ENTRY_T accountkey;

    TRACE(1,"length = %d value = 0x", len);
    DUMP8("%02X, ", param, FP_ACCOUNT_KEY_SIZE);
    gfps_crypto_decrypt(param, FP_ACCOUNT_KEY_SIZE, gfps_ble_env.keybase_pair_key, accountkey.key);
    TRACE(0,"decrypt account key:");
    //TRACE(0,"===============================");
    DUMP8("%02X", accountkey.key, FP_ACCOUNT_KEY_SIZE);
    //TRACE(0,"===============================");

    nv_record_fp_account_key_add(&accountkey);
    app_exit_fastpairing_mode();
#ifdef SASS_ENABLED
    gfps_sass_set_inuse_acckey(accountkey.key, NULL);
#endif

#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
    app_tws_send_fastpair_info_to_slave();
#endif

    // update the BLE ADV as account key has been added
    if (!gfps_is_in_fastpairing_mode())
    {
        // restart the BLE adv if it's retro-active pairing
        app_ble_refresh_adv_state_generic();
    }
    return BES_GATT_NO_ERR;
}

static uint8_t gfps_ble_l2cap_connected_handler(uint8_t conidx, bt_bdaddr_t *addr)
{
    GFPS_SRV_EVENT_L2CAP_PARAM_T evtParam;
    evtParam.event = FP_SRV_EVENT_CONNECTED;
    memcpy(evtParam.p.addr.address, addr, sizeof(bt_bdaddr_t));
    gfps_event_l2cap_callback(conidx, &evtParam);
    return BES_GATT_NO_ERR;
}

static uint8_t gfps_ble_l2cap_disconnected_handler(uint8_t conidx)
{
    GFPS_SRV_EVENT_L2CAP_PARAM_T evtParam;
    evtParam.event = FP_SRV_EVENT_DISCONNECTED;
    gfps_event_l2cap_callback(conidx, &evtParam);
    return BES_GATT_NO_ERR;
}

static uint8_t gfps_ble_l2cap_received_handler(uint8_t conidx, uint8_t *param, uint16_t len)
{
    GFPS_SRV_EVENT_L2CAP_PARAM_T evtParam;
    evtParam.event = FP_SRV_EVNET_L2CAP_DATA_IND;
    memcpy(evtParam.p.data.pBuf, param, len);
    evtParam.p.data.len = len;
    gfps_event_l2cap_callback(conidx, &evtParam);
    return BES_GATT_NO_ERR;
}

void gfps_ble_connected_evt_handler(uint8_t conidx, bt_bdaddr_t *localAddr)
{
    gfps_ble_env.connectionIndex = conidx;
    if (localAddr)
    {
        uint8_t ADDR_ZERO[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        if(!memcmp(localAddr->address, ADDR_ZERO, 6))
        {
            uint8_t addrPtr[6] = {0};
            gfps_ble_get_addr(addrPtr);
            big_little_switch(addrPtr, &gfps_ble_env.local_le_addr.address[0], 6);
        }
        else
        {
            big_little_switch(localAddr->address, &gfps_ble_env.local_le_addr.address[0], 6);
        }
    }

    TRACE(0,"local LE addr: ");
    DUMP8("0x%02x ", gfps_ble_env.local_le_addr.address, 6);

#if !defined(IBRT)
    big_little_switch(bt_get_ble_local_address(), &gfps_ble_env.local_bt_addr.address[0], 6);
#else
    ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx();
    uint8_t *bt_local_addr = p_ibrt_ctrl->local_addr.address;
    big_little_switch(bt_local_addr,
        &gfps_ble_env.local_bt_addr.address[0], 6);
#endif
    TRACE(0,"local bt addr: ");
    DUMP8("0x%02x ", gfps_ble_env.local_bt_addr.address, 6);
}

void gfps_ble_disconnected_evt_handler(uint8_t conidx)
{
    if (conidx == gfps_ble_env.connectionIndex)
    {
        gfps_ble_env.isNotificationEnabled[gap_zero_based_conidx(conidx)] = false;
        gfps_ble_env.isPendingForWritingNameReq = false;
        gfps_ble_env.connectionIndex = BLE_INVALID_CONNECTION_INDEX;
    }
}

int gfps_ble_get_conidx(void)
{
    return gfps_ble_env.connectionIndex;
}

void gfps_ble_tx_ccc_changed(uint8_t conidx, bool notify_enabled)
{
    gfps_ble_env.isNotificationEnabled[conidx] = notify_enabled;
}

uint8_t gfps_ble_is_connected(uint8_t conidx)
{
    return  gfps_ble_env.isNotificationEnabled[conidx];
}
static void gfps_ble_update_connection_state(uint8_t conidx, bool enabled, bt_bdaddr_t *localAddr)
{
    if (BLE_INVALID_CONNECTION_INDEX != conidx)
    {
        if (enabled)
        {
            gfps_ble_connected_evt_handler(conidx, localAddr);
        }
        else
        {
            gfps_ble_disconnected_evt_handler(conidx);
        }
    }
}

uint8_t gfps_ble_gatt_callback(ble_app_gfps_event_param_t *param)
{
    uint8_t status = BES_GATT_NO_ERR;
    if (!param)
    {
        return BES_GATT_ERR_POINTER_NULL;
    }

    uint8_t conidx = param->conidx;
    switch (param->event)
    {
        case GFPS_BLE_EVT_LE_CONNECTED:
            gfps_ble_update_connection_state(conidx, true, &(param->p.conn.localAddr));
            break;

        case GFPS_BLE_EVT_LE_DISCONNECTED:
            gfps_ble_update_connection_state(conidx, false, NULL);
            break;

        case GFPS_BLE_EVT_KEY_BASE_PAIRING_IND:
            status = gfps_ble_write_key_based_pairing_handler(conidx, param->p.packet.pBuf, param->p.packet.len);
            break;

        case GFPS_BLE_EVT_KEY_BASE_PAIRING_CCCD:
            //gfps_ble_env.isKeyBasedPairingNotificationEnabled = param->p.enabled;
            //gfps_ble_update_connection_state(conidx, param->p.enabled);
            break;

        case GFPS_BLE_EVT_PASS_KEY_IND:
            status = gfps_ble_write_passkey_ind_handler(conidx, param->p.packet.pBuf, param->p.packet.len);
            break;

        case GFPS_BLE_EVT_PASS_KEY_CCCD:
            //gfps_ble_env.isPassKeyNotificationEnabled = param->p.enabled;
            //gfps_ble_update_connection_state(conidx, param->p.enabled);
            break;

        case GFPS_BLE_EVT_ACCOUNT_KEY_IND:
            status = gfps_ble_write_accountkey_hander(conidx, param->p.packet.pBuf, param->p.packet.len);
            break;

        case GFPS_BLE_EVT_WRITE_NAME_IND:
            status = gfps_ble_write_name_hander(conidx, param->p.packet.pBuf, param->p.packet.len);
            break;
        case GFPS_BLE_EVT_ADDITIONAL_PASS_KEY_CCCD:
            break;

        case GFPS_BLE_EVT_ADDITIONAL_PASS_KEY_IND:
            status = gfps_ble_write_additional_passkey_ind_handler(conidx, param->p.packet.pBuf, param->p.packet.len);
            break;

#ifdef SPOT_ENABLED
        case GFPS_BLE_EVT_SPOT_WRITE_BEACON_IND:
            status = gfps_ble_write_spot_beacon_actions_hander(conidx, param->p.packet.pBuf, param->p.packet.len);
            break;

        case GFPS_BLE_EVT_SPOT_BEACON_CCCD:
            //gfps_ble_update_connection_state(conidx, param->p.enabled);
            break;
#endif
        case GFPS_BLE_EVT_L2CAP_CONNECTED:
            status = gfps_ble_l2cap_connected_handler(conidx, &(param->p.conn.peerAddr));
            break;

        case GFPS_BLE_EVT_L2CAP_DISCONNECTED:
            status = gfps_ble_l2cap_disconnected_handler(conidx);
            break;

        case GFPS_BLE_EVT_L2CAP_RX_REC:
            status = gfps_ble_l2cap_received_handler(conidx, param->p.packet.pBuf, param->p.packet.len);
            break;
        
#ifdef SPOT_ENABLED
        case GFPS_BLE_EVT_GET_SPOT_VERSION:
            if (param->outData)
            {
                param->outData[0] = gfps_ble_get_spot_version();
                param->outLen = 1;
            }
            break;

        case GFPS_BLE_EVT_GEN_SPOT_NOUNCE:
            gfps_ble_spot_generate_nonce();
            if (param->outData)
            {
                memcpy(param->outData, gfps_ble_spot_get_nonce(), GFPS_NONCE_SIZE);
                param->outLen = GFPS_NONCE_SIZE;
            }
            break;

        case GFPS_BLE_EVT_GET_SPOT_NOUNCE:
            if (param->outData)
            {
                memcpy(param->outData, gfps_ble_spot_get_nonce(), GFPS_NONCE_SIZE);
                param->outLen = GFPS_NONCE_SIZE;
            }
            break;
#endif
        case GFPS_BLE_EVT_GET_MODEL_ID:
            if (param->outData)
            {
                uint32_t modelId = gfps_get_model_id();
                memcpy(param->outData, &modelId, sizeof(uint32_t));
                param->outLen = sizeof(uint32_t);
            }
            break;
        case GFPS_BLE_EVT_GET_ADDITIONAL_PASS_KEY:
            if(param->outData)
            {
                 uint8_t rsp_data[16] = {0};
                 rsp_data[0] = KEY_BASED_PAIRING_RSP;
                 memcpy(&rsp_data[1], gfps_get_additional_passkey(), 3);
                 //copy address
                 if (app_tws_ibrt_tws_link_connected())
                 {
                     big_little_switch(nv_record_get_ibrt_peer_addr(), &rsp_data[4], sizeof(bt_bdaddr_t));
                     if (gfps_get_if_send_passkey())
                     {
                         rsp_data[10] = GFPS_STATUS_SUCCESS;
                         gfps_set_if_send_passkey(false);
                     }
                     else
                     {
                          rsp_data[10] = GFPS_STATUS_PEINDING;
                     }
                 }
                 else
                 {
                     rsp_data[10] = GFPS_STATUS_FAIL;
                 }
                 gfps_crypto_encrypt(( const uint8_t * )rsp_data, sizeof(rsp_data), gfps_get_keybase_pair_key(), param->outData);
                 param->outLen = GFPSP_ENCRYPTED_RSP_LEN;
            }
            break;
        case GFPS_BLE_EVT_GET_EVENT_STREAM:
            if(param->outData)
            {
#if defined(IBRT) && !defined(FREEMAN_ENABLED_STERO)
                ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
                if (TWS_UI_SLAVE != app_ibrt_if_get_ui_role()&& p_ibrt_ctrl->init_done)
            
                {
                    param->outData[0] = STATE_READY_CONNECT;
                }
                else
                {
                     param->outData[0] = STATE_UNAVAILABLE;
                }
 #else
                 param->outData[0] = STATE_READY_CONNECT;
 #endif
                 param->outData[1] = (L2CAP_SPSM_GFPS >> 8);
                 param->outData[2] = (L2CAP_SPSM_GFPS & 0xFF);
                 param->outLen = 3;
            }
            break;
        default:
            break;
    }
    return status;
}

void gfps_ble_init(void)
{
    memset(( uint8_t * )&gfps_ble_env, 0, sizeof(struct gfps_ble_env_tag));
    gfps_ble_env.connectionIndex = BLE_INVALID_CONNECTION_INDEX;
    gfps_ble_env.advRandSalt = GFPS_INITIAL_ADV_RAND_SALT;
    gfps_ble_env.bondMode = GFPS_BOND_OVER_BLE;
    memcpy(gfps_ble_env.txPower, APP_GFPS_ADV_POWER_UUID, APP_GFPS_ADV_POWER_UUID_LEN);

    bes_ble_gfps_init(gfps_ble_adv_activity_prepare);
#ifdef SPOT_ENABLED
    gfps_ble_spot_init();
    bes_ble_spot_init(gfps_ble_spot_adv_activity_prepare);
    bes_ble_dult_init(&gfps_ble_dult_callback);
#endif
    ble_app_gfps_register_event_callback(gfps_ble_gatt_callback);
}
#endif
