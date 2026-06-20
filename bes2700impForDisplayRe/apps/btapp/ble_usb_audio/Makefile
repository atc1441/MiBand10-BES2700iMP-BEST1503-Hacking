cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

ifeq ($(AOB_LOW_LATENCY_MODE),1)
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)test/*.cpp))
endif

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

subdir-ccflags-y += \
    -Iservices/overlay \
    -Iservices/resources \
    -Iservices/audio_bt \
    -Iservices/audioflinger \
    -Iservices/audio_process \
    -Iapps/audioplayers \
    -Iapps/audioplayers/a2dp_decoder \
    -Iservices/multimedia/audio/process/resample/include \
    -Iservices/multimedia/audio/process/filters/include \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BT_SERVICE_UX_INCLUDES) \
    -I$(BLE_GAF_CORE_DIR_PATH)/inc \
    -I$(BLE_AOB_APP_DIR_PATH)/inc \
    -Iplatform/drivers/bt \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iutils/lockcqueue \
    -Iutils/intersyshci \
    -Iapps/main \
    -Iapps/key \
    -Iapps/common \
    -Iapps/battery \
    -Iservices/app_bt_source/inc \
    -Iservices/app_bt_source/src/a2dp_encoder_cp \
    -Iservices/bt_app/ \
    -Iservices/osif/ \
    -Iservices/tota/ \
    -Ithirdparty/audio_codec_lib/liblhdc-enc/inc \
    $(AAC_INCLUDES) \
    -Iplatform/drivers/cp_accel \
    -Iservices/norflash_api \
    -Itests/anc_usb \
    $(BLE_APP_INCLUDES) \
    $(BLE_STACK_INCLUDES) \
    -Iutils/list \
    -Iutils/hsm \
    $(BES_MULTIMEDIA_INCLUDES) \
    -Iapps/bt_sync \
    -Iapps/main
