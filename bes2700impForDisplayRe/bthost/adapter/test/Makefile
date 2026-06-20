cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_bt_cmd.cpp))

ifeq ($(BLE_HOST_SUPPORT),1)
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_ble_test.cpp))
ifeq ($(BLE_ISO_ENABLED),1)
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_cis_iso_test.cpp))
endif
endif

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

obj-y := $(src_obj)

ccflags-y += -DBLUETOOTH_BLE_IMPL

subdir-ccflags-y += \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BLE_APP_INCLUDES) \
    $(BLE_STACK_INCLUDES) \
    $(BLE_STACK_INC_INTERNAL) \
    $(BT_SERVICE_UX_INCLUDES) \
    -I$(BLE_AOB_APP_DIR_PATH)/inc \
    -I./bthost/stack/ble_stack_new/inc \
    -I./bthost/service/ble_app_new/inc \
    -Iservices/bt_app \
    -Iservices/fs/fat \
    -Iservices/fs/sd \
    -Iservices/fs/fat/ChaN \
    -Iservices/overlay \
    -Iservices/resources \
    -Iservices/bt_watch_service/inc \
    -Iservices/lea_player/inc \
    -Imultimedia/inc/audio/process/resample/include \
    -Iservices/nv_section/factory_section \
    -Iservices/nv_section/fpga_section \
    -Iplatform/drivers/uarthci \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/bt \
    -Iutils/cqueue \
    -Iservices/app_custom \
    -Iservices/app_custom/ble_custom \
    -Iutils/heap \
    -Iservices/audioflinger \
    -Iservices/audio_dump/include \
    -Iutils/lockcqueue \
    -Iutils/intersyshci \
    -Iutils/cfifo \
    -Iapps/key \
    -Iapps/main \
    -Iapps/common \
    -Iapps/audioplayers \
    -Iapps/audioplayers/a2dp_decoder \
    -Iapps/anc/inc \
    -Iapps/factory \
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
    -Iservices/ai_voice/protocol/gva/gsound_target \
    -Iservices/osif \
    $(BT_SERVICE_UX_INCLUDES) \
    -Iservices/audio_manager \
	-Iservices/audio_manager/audio_policy/local_audio \
    -Iservices/ota \
    -Iutils/list \
    -Iservices/aob_app/gaf_audio \
    -Iservices/aob_app/gaf_codec_off_bth \
    -Iservices/audio_bt \
    -Iapps/earbuds/conn
