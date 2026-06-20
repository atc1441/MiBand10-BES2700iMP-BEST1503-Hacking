cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y += gaf_codec_cc_common.o
ifeq ($(GAF_ENCODER_CROSS_CORE_USE_M55),1)
obj-y += gaf_encoder_cc_m55_off_mcu.o
obj-y += gaf_encoder_cc_m55_lc3.o
endif

ifeq ($(GAF_DECODER_CROSS_CORE_USE_M55),1)
obj-y += gaf_decoder_cc_m55_off_mcu.o
obj-y += gaf_decoder_cc_m55_lc3.o
endif

ccflags-y += -DBLUETOOTH_BLE_IMPL

subdir-ccflags-y += \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BLE_APP_INCLUDES) \
    $(BLE_STACK_INCLUDES) \
    -I$(BLE_GAF_CORE_DIR_PATH) \
    $(BT_SERVICE_UX_INCLUDES) \
    -Iservices/bt_app \
    -Iservices/fs/fat \
    -Iservices/fs/sd \
    -Iservices/fs/fat/ChaN \
    -Iservices/overlay \
    -Iservices/resources \
    -Iservices/speech_algo_dsp \
    -Iservices/multimedia/speech/inc \
    -Iservices/multimedia/audio/process/resample/include \
    -Iservices/nv_section/factory_section \
    -Iservices/nv_section/fpga_section \
    -Iplatform/drivers/uarthci \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/cp_accel \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iservices/audioflinger \
    -Iservices/audio_dump/include \
    -Iutils/lockcqueue \
    -Iutils/kfifo \
    -Iutils/intersyshci \
    -Iapps/key \
    -Iapps/main \
    -Iapps/common \
    -Iapps/audioplayers \
    -Iapps/audioplayers/a2dp_decoder \
    -Iapps/anc/inc \
    -Iapps/factory \
    -Iapps/btapp/ble_usb_audio \
    -Iservices/interconnection/green \
    -Iutils/hwtimer_list \
    -Ithirdparty/audio_codec_lib/liblhdc-dec/inc \
    -Iservices/nv_section/log_section \
    -Iservices/ai_voice/manager \
    -Iservices/app_ai/inc \
    -Iapps/battery/ \
    -Iutils/crc16 \
    -Iutils/crc32 \
    -Iutils/rom_utils \
    $(BES_MULTIMEDIA_INCLUDES) \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iservices/voicepath/gsound/gsound_target \
    -Iservices/ota \
    -Iservices/audioflinger \
    -Iservices/norflash_api \
    -Iutils/list \
    -Iapps/bt_sync \
    -Iservices/audio_manager \
    -Iservices/lea_player/inc \
    -Iservices/lea_player/gaf_codec_off_bth \
    -Iservices/multimedia/audio/process/sbcplc/include

ccflags-y += -Iapps/dsp_m55