cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gsound_target/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gsound_custom/src/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)protobuf/*.c))

all_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gsound_target/*.cpp))
all_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gsound_custom/src/*.cpp))
all_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)protobuf/*.cpp))

filtout_cpp :=
filtout_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gsound_target/*_test.cpp))
filtout_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gsound_custom/src/*_test.cpp))
ifneq ($(TWS_SYSTEM_ENABLED), 1)
filtout_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gsound_target/*_tws.cpp))
filtout_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gsound_custom/src/*_tws.cpp))
endif

ifneq ($(GSOUND_HOTWORD_ENABLE), 1)
filtout_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gsound_target/*hotword*.cpp))
filtout_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gsound_custom/src/*hotword*.cpp))
else
ifeq ($(GSOUND_HOTWORD_EXTERNAL), 1)
filtout_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gsound_target/*hotword_internal.cpp))
filtout_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gsound_custom/src/*hotword_internal.cpp))
else
filtout_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gsound_target/*hotword_external.cpp))
filtout_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gsound_custom/src/*hotword_external.cpp))
endif
endif

obj_cpp := $(filter-out $(filtout_cpp),$(all_cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

LIB_BIN_DIR := lib/thirdparty/gsound
# include the libgsound
obj-y += libgsound.a

# include the dsp library if hotword enabled
ifeq ($(GSOUND_HOTWORD_ENABLE), 1)
#ifneq ($(VOICE_DETECTOR_SENS_EN),1)
obj-y += libcortexm4_google_hotword_dsp_multi_bank_api.a
#endif
endif

subdir-ccflags-y += \
    $(BT_SERVICE_UX_INCLUDES) \
    -Iapps/audioplayers/ \
    -Iapps/common/ \
    -Iapps/battery/ \
    -Iapps/key/ \
    -Iapps/main/ \
    -Iapps/factory \
    -Iapps/voice_detector \
    -Iplatform/hal \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/norflash \
    -Iplatform/drivers/uarthci \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/cp_accel \
    -Iservices/ai_voice/manager \
    -Iservices/app_ai/inc \
    -Iservices/audio_bt \
    -Iservices/audio_dump/include \
    -Iservices/nv_section/customparam_section/ \
    -Iservices/nv_section/log_section/ \
    -Iservices/voicepath \
    -Iservices/voicepath/gsound \
    -Iservices/voicepath/gsound/gsound_target/ \
    -Iservices/voicepath/gsound/gsound_target_api_read_only/ \
    -Iservices/voicepath/gsound/gsound_custom/inc \
    -Iservices/voicepath/gsound/protobuf \
    -Iservices/cp_accel \
    -Iservices/resources \
    -Iservices/bluedroid/adapter/src \
    -Iservices/audio_process \
    -Iservices/overlay \
    -Iservices/audioflinger \
    -Iservices/bridge/ \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BES_MULTIMEDIA_INCLUDES) \
    -Iservices/multimedia/speech/application/Pcm8kCvsd/include \
    -Iservices/multimedia/speech/codec/codecs/adpcm \
    -Iservices/multimedia/speech/process/lc_mmse_ns/include \
    -Iservices/multimedia/speech/process/ns_and_ec/include \
    -Iservices/multimedia/speech/process/speech_2mic_ns2_denoise/include \
    -Iservices/multimedia/speech/process/dual_mic_denoise/include \
    -Iservices/multimedia/speech/process/far_field_speech_enhancement/include \
    -Iservices/multimedia/speech/process/plc/include \
    -Iservices/multimedia/speech/process/common/include \
    -Iservices/multimedia/speech/application/Pcm8kCvsd/include \
    -Iservices/norflash_api \
    -Iservices/ota/ai_ota \
    -Iapps/ota \
    -Iservices/ai_voice/audio \
    -Ithirdparty/userapi \
    -Iutils/cqueue \
    -Iutils/retention_ram \
    -Iutils/crc32 \
    -Iutils/lockcqueue \
    -Iutils/intersyshci \
    -Iutils/heap \
    -Iservices/audio_manager

ifeq ($(FLASH_SUSPEND),1)
CFLAGS_gsound_custom_ota.o += -DFLASH_SUSPEND
endif

ifeq ($(OTA_ENABLE),1)
ccflags-y += -Iservices/ota
endif

ifeq ($(VOICE_DETECTOR_SENS_EN),1)
ccflags-y +=     -Itests/sensor_hub/inc \
                 -Iapps/sensorhub
endif

ifeq ($(GSOUND_M55_ENABLE),1)
ccflags-y += -Iapps/dsp_m55
endif