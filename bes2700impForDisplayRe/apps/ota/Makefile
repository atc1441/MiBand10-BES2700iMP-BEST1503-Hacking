cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

subdir-ccflags-y += \
    -Iservices/overlay \
    -Iservices/resources \
    -Iservices/audio_bt \
    -Iservices/audioflinger \
    -Iservices/audio_process \
    -Iservices/audio_manager \
    -Iapps/audioplayers \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    -Iplatform/drivers/bt \
    -Iutils/cqueue \
    -Iutils/intersyshci \
    -Iapps/main \
    -Iapps/key \
    -Iapps/common \
    -Iapps/earbuds/conn \
    -Iservices/bt_app/ \
    -Iservices/osif/ \
    -Iservices/ota/ \
    -Iservices/norflash_api \
    -Iplatform/drivers/cp_accel \
    $(BLE_APP_INCLUDES) \
    $(BLE_STACK_INCLUDES) \
    $(BT_SERVICE_UX_INCLUDES) \
    $(BES_MULTIMEDIA_INCLUDES) \
    -Iutils/list \
    -Iservices/ble_audio/ble_audio_core/inc

ifeq ($(BES_OTA),1)
subdir-ccflags-y += \
    -Iservices/ota/bes_ota/inc
endif

