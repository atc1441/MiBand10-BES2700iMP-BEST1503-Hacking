cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)algorithm/src/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)audio/src/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)audio/src/*.cpp))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)manager/src/*.cpp))

exclude_transport_c :=
exclude_transport_cpp :=
transport_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)transport/src/*.c))
transport_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)transport/src/*.cpp))

ifneq ($(BLE),1)
exclude_transport_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)transport/src/*ble.c))
exclude_transport_cpp +=  $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)transport/src/*ble.cpp))
endif

obj_c += $(filter-out $(exclude_transport_c), $(transport_c))
obj_cpp += $(filter-out $(exclude_transport_cpp), $(transport_cpp))

framework_obj :=

ifeq ($(VOC_ENCODE_ENABLE),1)
framework_obj += codec/
endif

ifeq ($(IOS_MFI),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)iAP2/src/*.c))
endif

AI_FRAMEWORK_LIB_NAME := libAI_api

ifeq ($(AMA_VOICE),1)
framework_obj += protocol/ama/
endif

ifeq ($(DMA_VOICE),1)
framework_obj += protocol/dma/
endif

ifeq ($(SMART_VOICE),1)
framework_obj += protocol/smartvoice/
endif

ifeq ($(TENCENT_VOICE),1)
framework_obj += protocol/tencent/
endif

ifeq ($(GMA_VOICE),1)
framework_obj += protocol/gma/
endif

ifeq ($(BIXBY_VOICE),1)
framework_obj += protocol/bixbyvoice/
endif

ifeq ($(CUSTOMIZE_VOICE),1)
framework_obj += protocol/common_api/
AI_FRAMEWORK_LIB_NAME := $(AI_FRAMEWORK_LIB_NAME)_COMMON
endif

ifeq ($(USE_TRACE_ID), 1)
AI_FRAMEWORK_LIB_NAME := $(AI_FRAMEWORK_LIB_NAME)_log_compressed
endif

ifeq ($(GATT_OVER_BR_EDR), 1)
AI_FRAMEWORK_LIB_NAME := $(AI_FRAMEWORK_LIB_NAME)_gatt
endif

AI_FRAMEWORK_LIB_NAME := $(AI_FRAMEWORK_LIB_NAME)_ble_new_stack

ifeq ($(DUAL_MIC_RECORDING),1)
framework_obj += protocol/dual_mic_recording/
endif

ifeq ($(MMA_VOICE),1)
framework_obj += protocol/mma/
endif

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

$(AI_FRAMEWORK_LIB_NAME)-y := $(src_obj)

obj-y := $(AI_FRAMEWORK_LIB_NAME).a
obj-y += $(framework_obj)

subdir-ccflags-y += \
    -Iapps/common \
    -Iapps/key \
    -Iapps/main \
    -Iapps/voice_assist/inc \
    -Iapps/audioplayers \
    -Iapps/factory \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/bt \
    -Iplatform/hal \
    -Irtos\rtx/TARGET_CORTEX_M \
    -Iservices/ai_voice/algorithm \
    -Iservices/ai_voice/codec/compression \
    -Iservices/ai_voice/codec/opus121/include \
    -Iservices/ai_voice/manager \
    -Iservices/ai_voice/protocol/dual_mic_recording/voice_manager \
    -Iservices/ai_voice/protocol/bixbyvoice/ \
    -Iservices/ai_voice/protocol/bixbyvoice/bixbyvoice_manager \
    -Iservices/ai_voice/transport \
    -Iservices/ai_voice/audio \
    -Iservices/ai_voice/iAP2 \
    -Iservices/app_ai/voice_sbc \
    -Iservices/app_ai/inc \
    -Iservices/audio_bt \
    -Iservices/audio_eq \
    -Iservices/audio_dump/include \
    -Iservices/audioflinger \
    -Iservices/bt_app \
    -Iservices/bt_app/a2dp_codecs/include \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BLE_APP_INCLUDES) \
    $(BLE_STACK_INCLUDES) \
    $(BT_SERVICE_UX_INCLUDES) \
    -Iservices/communication \
    -Iservices/communication/comminication_knowles \
    -Iservices/interconnection/umm_malloc \
    -Iservices/multimedia/audio_dump/include \
    -Iservices/multimedia/audio/process/resample/include \
    -Iservices/multimedia/speech/inc \
    -Iinclude/cmsis_dsp \
    -Iservices/overlay \
    -Iservices/resources \
    -Iservices/ble_app/app_main \
    -Iservices/voicepath/gsound/gsound_custom/inc \
	-Iservices/voicepath/gsound \
    -Iservices/voicepath/gsound/gsound_target/ \
    -Iservices/voicepath/gsound/gsound_target_api_read_only/ \
    -Iservices/ai_voice/protocol/ama/ama_stream \
	-Iservices/bt_profiles_enhanced/inc \
    -Ithirdparty/userapi \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iutils/kfifo \
    -Iutils/lockcqueue \
    -Iutils/retention_ram \
    -Iservices/audio_manager

ifeq ($(AUDIO_RESAMPLE),1)
ANC_USB_CFG_FLAGS += -D__AUDIO_RESAMPLE__
endif
ifeq ($(SW_CAPTURE_RESAMPLE),1)
ANC_USB_CFG_FLAGS += -DSW_CAPTURE_RESAMPLE
endif

ifeq ($(VOICE_DETECTOR_SENS_EN),1)
ccflags-y +=    -Itests/sensor_hub/inc \
                -Itests/sensor_hub/sensor_hub_ai \
                -Iapps/sensorhub
endif

ifeq ($(STEREO_RECORD_PROCESS),1)
CFLAGS_app_ai_voice.o += -DSTEREO_RECORD_PROCESS
endif
