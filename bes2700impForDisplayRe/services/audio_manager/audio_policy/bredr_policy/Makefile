
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)/original_v2/*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)/original_v2/*.c))

obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)/original_v2/*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(BT_SOURCE),1)
SOURCE_INCLUDES =  \
	-Iservices/app_bt_source/inc
else
SOURCE_INCLUDES =
endif

ccflags-y += -DBLUETOOTH_BT_IMPL

ccflags-y += \
	$(AAC_INCLUDES) \
	-Iinclude/cmsis_dsp \
	-Iservices/voice_dev \
	-Iservices/audio_bt \
	-Iservices/audio_process \
	-Iservices/audio_dump/include \
	-Iservices/hw_dsp/inc \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	$(BES_BT_IMPL_INCLUDES) \
	$(BLE_APP_INCLUDES) \
	$(BLE_STACK_INCLUDES) \
	$(BT_SERVICE_UX_INCLUDES) \
	-Iservices/multimedia/speech/inc \
	-Iservices/bone_sensor \
	-Iservices/overlay \
	-Ithirdparty/tile/tile_common/tile_storage \
	-Iservices/resources \
	-Iservices/multimedia/rbcodec \
	-Iservices/multimedia/audio/process/resample/include \
	-Iservices/multimedia/audio/process/filters/include \
	-Iservices/multimedia/audio/process/drc/include \
	-Iservices/multimedia/audio/process/anc/include \
	-Iservices/multimedia/audio/process/sidetone/include \
	-Iservices/multimedia/audio/process/adj_mc/inc \
	-Iservices/nv_section/aud_section \
	-Iservices/nv_section/userdata_section \
	-Iservices/nv_section/include \
	-Iservices/voicepath/$(VOICE_DATAPATH_TYPE) \
	-Iservices/voicepath/gsound/gsound_target \
	-Iservices/voicepath/gsound/gsound_custom/inc \
	-Iservices/voicepath/gsound/gsound_target_api_read_only \
	-Iplatform/drivers/uarthci \
	-Iplatform/drivers/ana \
	-Iplatform/cmsis \
	-Iplatform/drivers/bt \
	-Iutils/cqueue \
	-Iutils/heap \
	-Iutils/crc16 \
	-Iutils/hsm \
	-Iservices/audioflinger \
	-Iutils/lockcqueue \
	-Iutils/intersyshci \
	-Iapps/anc/inc \
	-Iapps/key \
	-Iapps/main \
	-Iapps/common \
	-Iservices/audio_manager/audio_policy/bredr_policy/inc \
	-Iapps/audioplayers \
	-Iapps/audioplayers/a2dp_decoder \
	-Iapps/battery \
	-Iapps/common \
	-Iapps/factory \
	-Iutils/hwtimer_list \
	-Iservices/voicepath \
	-Ithirdparty/userapi \
	-Ithirdparty/audio_codec_lib/liblhdc-dec/inc \
	-Iservices/ai_voice/manager \
	-Iservices/ai_voice/audio \
	-Iservices/ai_voice/transport \
	-Iservices/app_ai/inc \
	-Iservices/interconnection/red \
	-Iservices/interconnection/green \
	-Iservices/interconnection/umm_malloc \
	-Iservices/bt_app \
	-Iservices/bt_app/a2dp_codecs/include \
	$(SOURCE_INCLUDES) \
	-Ithirdparty/tile/tile_target \
	-Iservices/osif \
    -Iutils/list \
	-Iservices/audio_manager

ifeq ($(AUDIO_PROMPT_USE_DAC2_ENABLED),1)
CFLAGS_audio_policy.o += -DAUDIO_PROMPT_USE_DAC2_ENABLED
CFLAGS_audio_policy_red.o += -DAUDIO_PROMPT_USE_DAC2_ENABLED
endif

ifeq ($(BT_USB_AUDIO_DUAL_MODE),1)
ccflags-y += \
    -Iapps/btusbaudio
endif

ifeq ($(GFPS_ENABLE),1)
ccflags-y += \
    -Iservices/gfps/inc
endif