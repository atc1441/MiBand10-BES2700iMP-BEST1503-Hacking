cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*/*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*/*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*/*.cpp))

A2DPCODECS_LIB_NAME := lib_a2dpcodecs

ifeq ($(A2DP_AAC_ON),1)
A2DPCODECS_LIB_NAME := $(A2DPCODECS_LIB_NAME)_aac
endif

ifeq ($(A2DP_LHDC_ON),1)
A2DPCODECS_LIB_NAME := $(A2DPCODECS_LIB_NAME)_lhdc
endif

ifeq ($(A2DP_LDAC_ON),1)
A2DPCODECS_LIB_NAME := $(A2DPCODECS_LIB_NAME)_ldac
endif

obj-y += $(A2DPCODECS_LIB_NAME).a

$(A2DPCODECS_LIB_NAME)-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

CFLAGS_a2dp_codec_ldac.o += -Ithirdparty/audio_codec_lib/ldac/inc

ccflags-y += -DBLUETOOTH_BT_IMPL

subdir-ccflags-y += \
        -Iservices/audio_bt \
	-Iservices/audio_process \
	-Iservices/hw_dsp/inc \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	$(BES_BT_IMPL_INCLUDES) \
	$(BLE_APP_INCLUDES) \
	$(BLE_STACK_INCLUDES) \
	$(BT_SERVICE_UX_INCLUDES) \
	-Iservices/multimedia/speech/inc \
	-Imultimedia/inc/speech/inc \
	-Iservices/bone_sensor/lis25ba \
	-Iservices/overlay \
	-Iservices/resources \
	$(BES_MULTIMEDIA_INCLUDES) \
	-Imultimedia/inc/rbcodec \
	-Imultimedia/inc/audio/process/resample/include \
	-Imultimedia/inc/audio/process/filters/include \
	-Imultimedia/inc/audio/process/drc/include \
	-Imultimedia/inc/audio/process/anc/include\
	-Iservices/multimedia/rbcodec \
	-Iservices/multimedia/audio/process/resample/include \
	-Iservices/multimedia/audio/process/filters/include \
	-Iservices/multimedia/audio/process/drc/include \
	-Iservices/multimedia/audio/process/anc/include \
	-Iservices/nv_section/aud_section \
	-Iservices/nv_section/userdata_section \
  	-Iservices/nv_section/include   \
	-Iservices/voicepath/$(VOICE_DATAPATH_TYPE) \
	-Iservices/voicepath/gsound \
	-Iservices/ai_voice/protocol/gva \
	-Iplatform/drivers/uarthci \
	-Iplatform/drivers/ana \
	-Iplatform/cmsis \
	-Iplatform/drivers/bt \
	-Iutils/cqueue \
	-Iutils/heap \
	-Iservices/audioflinger \
	-Iutils/lockcqueue \
	-Iutils/intersyshci \
	-Iapps/anc/inc \
	-Iapps/key \
	-Iapps/main \
	-Iapps/common \
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
	-Iservices/audio_manager
