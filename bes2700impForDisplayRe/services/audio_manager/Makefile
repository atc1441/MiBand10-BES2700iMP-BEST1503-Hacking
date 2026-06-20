cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)test/*.cpp))

obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)audio_policy/bredr_policy/original_v2/*.cpp))

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

AM_LIB_NAME := libaudio_manager

ifeq ($(BLE_AUDIO_ENABLED), 1)
AM_LIB_NAME := $(AM_LIB_NAME)_aob
endif

ifeq ($(USE_TRACE_ID), 1)
AM_LIB_NAME := $(AM_LIB_NAME)_log_compressed
endif

ifeq ($(USE_TRACE_ID), 1)
AM_LIB_NAME := $(AM_LIB_NAME)_log_compressed
endif

$(AM_LIB_NAME)-y := $(src_obj)

obj-y :=  $(AM_LIB_NAME).a

CFLAGS_app_audio_active_device_manager.o += -DBESLIB_INFO=$(BESLIB_INFO)

subdir-ccflags-y += \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BT_SERVICE_UX_INCLUDES) \
    -I$(BLE_GAF_CORE_DIR_PATH)/inc \
    -I$(BLE_AOB_APP_DIR_PATH)/inc \
    -Iservices/bt_app \
    -Iservices/fs/fat \
    -Iservices/fs/sd \
    -Iservices/fs/fat/ChaN \
    -Iservices/overlay \
    -Iservices/resources \
    -Imultimedia/inc/audio/process/resample/include \
    -Iservices/multimedia/audio/process/resample/include \
    -Iservices/nv_section/factory_section \
    -Iservices/nv_section/fpga_section \
    -Iservices/lea_player/inc \
    -Iplatform/drivers/uarthci \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/bt \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iservices/audioflinger \
    -Iservices/audio_dump/include \
    -Iutils/lockcqueue \
    -Iutils/intersyshci \
    -Iutils/hsm \
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
    -Imultimedia/inc/audio/codec/lc3/inc \
    -Iservices/multimedia/audio/codec/lc3/inc \
    -Iservices/multimedia/audio/codec/sbc/ \
    -Iservices/multimedia/audio/codec/sbc/inc \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iservices/voicepath/gsound/gsound_target \
    -Iservices/ai_voice/protocol/gva/gsound_target \
    $(BT_SERVICE_UX_INCLUDES) \
    -Iservices/ota \
    -Iservices/audioflinger \
    -Iutils/list \
    -Iapps/audio_manager \
    -Iservices/audio_manager/audio_policy/bredr_policy/inc \
    -Iservices/audio_manager/test\
	-Ibthost/service/ble_app_new/inc\
    -Iservices/audio_bt/
ifeq ($(GFPS_ENABLE),1)
ccflags-y += \
    -Iservices/gfps/inc
endif

ifeq ($(BT_USB_AUDIO_DUAL_MODE),1)
ccflags-y += \
    -Iapps/btusbaudio
endif

ccflags-y += -Imultimedia/inc

ifeq ($(AI_VOICE),1)
ccflags-y += \
    -Iservices/ai_voice/audio
endif
