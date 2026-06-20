
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := \
    -Iservices/resources \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BT_SERVICE_UX_INCLUDES) \
    -Iplatform/drivers/bt \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iutils/eshell \
    -Iservices/audioflinger \
    -Iutils/lockcqueue \
    -Iutils/intersyshci \
    -Iapps \
    -Iapps/key \
    -Iapps/main \
    -Iapps/battery \
    -Iapps/common \
    -Iapps/anc/inc	\
    -Iapps/audioplayers/a2dp_decoder \
    -Iservices/ai_voice/manager \
    -Iservices/nv_section/factory_section \
    -Iservices/nv_section/log_section \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iservices/voicepath	\
    -Iservices/audio_manager \
    -Iservices/audio_bt \
    -Iservices/bis_transport/inc \
    -Iservices/osif \
    -Iutils/cppobject

# ifeq ($(BLE_AUDIO_ENABLED),1)
# ccflags-y += \
#     -I$(BLE_AUDIO_CORE_DIR_PATH)/inc \
#     -I$(BLE_AOB_APP_DIR_PATH)/gaf_app
# endif

ifeq ($(WIFI_SOUND_UI_ENABLE),1)
ccflags-y += \
    -Inet/net_music/airplay/airplay \
    -Inet/net_music/airplay/sdk \
    -Inet/net_music/airplay/wac/Platform
endif
