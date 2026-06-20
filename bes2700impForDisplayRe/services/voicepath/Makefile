cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

subdir-ccflags-y += \
    -Iplatform/drivers/ana \
    -Iplatform/hal \
    -Iservices/bluedroid/adapter/src \
    -Iapps/key \
    -Iapps/voice_assist/inc \
    -Iservices/audio_bt \
    -Iservices/audio_process \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BT_SERVICE_UX_INCLUDES) \
    -Iservices/overlay \
    -Iservices/resources \
    -Iplatform/drivers/uarthci \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/bt \
    -Iutils/cqueue \
    -Iutils/retention_ram \
    -Iservices/audioflinger \
    -Iutils/lockcqueue \
    -Iutils/intersyshci \
    -Iapps/main \
    -Iapps/common \
    -Iapps/audioplayers \
    -Iapps/factory \
    -Iservices/voice_dev \
    -Iservices/voicepath/gsound \
    -Iservices/voicepath/gsound/gsound_target/ \
    -Iservices/voicepath/gsound/gsound_custom/inc \
    -Iservices/voicepath/gsound/gsound_target_api_read_only \
    -Iservices/bridge/ \
    -Iservices/multimedia/speech/application/Pcm8kCvsd/include \
    -Iservices/multimedia/speech/codec/codecs/adpcm \
    -Iutils/crc32 \
    $(BES_MULTIMEDIA_INCLUDES) \
    -Iservices/multimedia/speech/process/lc_mmse_ns/include \
    -Iservices/multimedia/speech/process/ns_and_ec/include \
    -Iservices/multimedia/speech/process/speech_2mic_ns2_denoise/include \
    -Iservices/multimedia/speech/process/dual_mic_denoise/include \
    -Iservices/multimedia/speech/process/far_field_speech_enhancement/include \
    -Iservices/multimedia/speech/process/plc/include \
    -Iservices/multimedia/speech/process/common/include \
    -Ithirdparty/userapi \
    -Iutils/heap \
    -Iservices/voicepath \
    -Iservices/ota \
    -Iservices/ai_voice/manager \
    -Iservices/app_ai/inc \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iapps/voice_detector

ifeq ($(GSOUND_HOTWORD_EXTERNAL),1)
subdir-ccflags-y += -Iplatform/drivers/cp_accel
endif
