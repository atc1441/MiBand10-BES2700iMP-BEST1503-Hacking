cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
#app_bap
obj_bap_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_bap/*.c))

ifeq ($(CFG_BAP_BC), 0)
obj_c += $(filter-out $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_bap/app_bap_bc*.c)), $(obj_bap_c))
else
obj_c += $(obj_bap_c)
endif

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_arc/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_acc/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_cap/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_hap/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_atc/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_tmap/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_gmap/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_tm/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)media_mgr/*.c))


BLE_AUDIO_LIB_NAME := libble_audio_new

ifeq ($(LC3PLUS_SUPPORT),1)
BLE_AUDIO_LIB_NAME := $(BLE_AUDIO_LIB_NAME)_lc3plus
endif

ifeq ($(AOB_MOBILE_ENABLED), 1)
ifeq ($(BLE_USB_AUDIO_SUPPORT), 1)
BLE_AUDIO_LIB_NAME := $(BLE_AUDIO_LIB_NAME)_usb_dongle
endif
endif

$(BLE_AUDIO_LIB_NAME)-y := $(obj_c:.c=.o)

obj-y :=  $(BLE_AUDIO_LIB_NAME).a

CFLAGS_app_gaf.o += -DBESLIB_INFO=$(BESLIB_INFO)

ccflags-y += -DBLUETOOTH_BLE_IMPL

subdir-ccflags-y += \
    -I$(BT_ROM_PORTING_PATH) \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BLE_APP_INCLUDES) \
    $(BLE_STACK_INCLUDES) \
    $(BT_SERVICE_UX_INCLUDES) \
    -Iservices/bt_app \
    -Iutils/cqueue \
    -Iutils/list \
    -Iapps/bt_sync \
    -Iapps/common \
    -Iservices/audio_dump/include \
    $(BES_MULTIMEDIA_INCLUDES) \
    -Iutils/hwtimer_list \
    -Iutils/lockcqueue \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/bt \
    -Iutils/heap \
    -Iapps/key \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iapps/main \
    -Iservices/resources \
    -Iapps/audioplayers \
    -Iapps/audioplayers/a2dp_decoder \
    -Iservices/bridge

ifeq ($(BLE_AUDIO_ENABLED),1)
subdir-ccflags-y += \
    -Iservices/audio_manager
endif
