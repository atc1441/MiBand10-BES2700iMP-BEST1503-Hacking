cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)custom_api/*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)custom_api/*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)custom_api/*.cpp))

obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_ibrt/src/*.s))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_ibrt/src/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_ibrt/src/*.cpp))

obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ibrt_middleware/src/*.s))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ibrt_middleware/src/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ibrt_middleware/src/*.cpp))

ifeq ($(IBRT_UI),1)
obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ibrt_core/src/*.s))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ibrt_core/src/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ibrt_core/src/*.cpp))

obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ibrt_ui/src/*.s))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ibrt_ui/src/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ibrt_ui/src/*.cpp))
endif

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

obj-y := $(src_obj)

subdir-ccflags-y += \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
	$(BT_SERVICE_UX_INCLUDES) \
    -I$(BT_ROM_PORTING_PATH) \
    -Ithirdparty/userapi \
    -Ithirdparty/audio_codec_lib/liblhdc-dec/inc \
    -Imultimedia/inc/audio/process/resample/include \
    -Iservices/resources \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/uarthci \
    -Iplatform/drivers/usb/usb_dev/inc \
    -Iplatform/drivers/sensor_hub \
    -Iplatform/hal \
    -Iutils/cqueue \
    -Iutils/list \
    -Iutils/heap \
    -Iutils/crc16 \
    -Iutils/crc32 \
    -Iutils/cppobject \
    -Iutils/eshell \
    -Iutils/rom_utils \
    -Iutils/hsm \
    -Iutils/kfifo \
    -Iutils/cfifo \
    -Iservices/audioflinger \
    -Iutils/lockcqueue \
    -Iutils/intersyshci \
    -Iutils/hwtimer_list \
    -Iapps \
    -Iapps/key \
    -Iapps/main \
    -Iapps/battery \
    -Iapps/bt_sync \
    -Iapps/common \
    -Iapps/factory \
    -Iapps/anc/inc	\
    -Iapps/btspeaker \
    -Iapps/audioplayers \
    -Iapps/audioplayers/a2dp_decoder \
    -Iapps/audioplayers/audio_trigger/version1 \
    -Iapps/audioplayers/audio_trigger/version1 \
    -Iapps/sensorhub \
    -Iapps/app_test/ble_audio_test \
    -Iapps/earbuds/conn \
    -Iapps/anc/inc \
    -Iapps/common/uart_dma \
    -Iapps/common/chip_bridge \
    -Iapps/earbuds/test \
    -Iapps/psap/inc \
    -Iapps/ble_audio_central_app/ble_audio_usb \
    -Iservices/fs/fat \
    -Iservices/fs/sd \
    -Iservices/fs/fat/ChaN \
    -Iservices/audio_bt \
    -Iservices/lea_player/inc \
    -Iservices/ai_voice/manager \
    -Iservices/ai_voice/transport \
    -Iservices/nv_section/factory_section \
    -Iservices/nv_section/log_section \
    -Iservices/norflash_api \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iservices/multimedia/audio/process/resample/include \
    -Iservices/interconnection/green \
    -Iservices/voicepath	\
    -Iservices/app_ai/inc \
    -Iservices/audio_manager \
    -Iservices/audio_manager/test \
    -Iservices/bis_transport/inc \
    -Iservices/overlay \
    -Iservices/ota \
    -Iservices/tota_v2 \
	-Iservices/ota/ai_ota \
	-Iservices/ota/bes_ota/inc \
    -Iservices/voicepath/gsound/gsound_target_api_read_only \
    -Iservices/voicepath/gsound/gsound_custom/inc \
    -Iservices/voicepath/gsound/gsound_target \
    -Iservices/ai_voice/protocol/bixbyvoice \
    -Iservices/ai_voice/protocol/bixbyvoice/bixbyvoice_manager \
    -Iservices/ai_voice/protocol/gma/gma_manager \
    -Iservices/ai_voice/protocol/gma/gma_crypto \
    -Iservices/ai_voice/protocol/dual_mic_recording/voice_manager \
    -Iservices/ai_voice/codec/compression \
    -Iservices/ai_voice/codec/opus121/include \
    -Iservices/app_debug_info_system/inc \
    -Iservices/btservice/common/config \
    -Iservices/bt_app \
    -Iservices/osif \
    -Iservices/bt_sync \
    -Iservices/osif/

subdir-ccflags-y += -Wno-date-time

CFLAGS_app_ui_mgr.o += -DBESLIB_INFO=$(BESLIB_INFO)
CFLAGS_app_custom_api.o += -DBESLIB_INFO=$(BESLIB_INFO)
CFLAGS_app_tws_ibrt.o += -DBESLIB_INFO=$(BESLIB_INFO)

ifeq ($(BLE_AUDIO_ENABLED),1)
subdir-ccflags-y += \
    $(BLE_STACK_INCLUDES) \
    -Iservices/log_manager \
    -Iservices/audio_manager \
    -Iservices/ux/common/ble_audio_core/inc
endif

ifeq ($(GFPS_ENABLE),1)
ccflags-y += \
   -Iservices/ble_app/app_gfps \
   -Iservices/gfps/inc
endif

export APP_UI_MONITOR ?= 0
ifeq ($(APP_UI_MONITOR),1)
KBUILD_CPPFLAGS += -DAPP_UI_MONITOR

export APP_UI_MONITOR_GUARD_TIMEOUT ?= 45000
KBUILD_CPPFLAGS += -DAPP_UI_MONITOR_GUARD_TIMEOUT=$(APP_UI_MONITOR_GUARD_TIMEOUT)
# /* record buff size(power of 2!) */
export APP_UI_MONITOR_RECORD_BUF_SIZE ?= 1024
KBUILD_CPPFLAGS += -DAPP_UI_MONITOR_RECORD_BUF_SIZE=$(APP_UI_MONITOR_RECORD_BUF_SIZE)
endif

export APP_UI_TEST_MODE_ENABLE ?= 0
ifeq ($(APP_UI_TEST_MODE_ENABLE),1)
KBUILD_CPPFLAGS += -DAPP_UI_TEST_MODE_ENABLE
endif

ifeq ($(AUDIO_PROMPT_USE_DAC2ORDAC3_ENABLED),1)
CFLAGS_app_ibrt_voice_report.o += -DAUDIO_PROMPT_USE_DAC2ORDAC3_ENABLED
endif

ifeq ($(ANC_APP),1)
CFLAGS_app_ibrt_ui_test.o += -DANC_APP
CFLAGS_app_ibrt_customif_cmd.o += -DANC_APP
endif

ifeq ($(A2DP_SBC_PLC_ENABLED),1)
CFLAGS_app_tws_ibrt_ui_test.o += -DA2DP_SBC_PLC_ENABLED
endif

ifeq ($(A2DP_AUDIO_STEREO_MIX_CTRL),1)
CFLAGS_app_ibrt_customif_ui.o += -DA2DP_AUDIO_STEREO_MIX_CTRL
endif

ifeq ($(HEARING_AID_ENABLED),1)
CFLAGS_app_tws_ibrt_ui_test.o += -DHEARING_AID_ENABLED
endif

ifneq ($(APP_UI_THREAD_SIZE),)
KBUILD_CPPFLAGS += -DAPP_UI_THREAD_SIZE=$(APP_UI_THREAD_SIZE)
endif

