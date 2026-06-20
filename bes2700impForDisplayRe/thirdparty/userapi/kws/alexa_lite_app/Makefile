cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)


subdir-ccflags-y +=  \
	-Ithirdparty/kws/alexa_lite_lib/inc \
	-Iapps/audioplayers \
	-Iapps/common \
	-Iapps/voice_detector \
	-Iapps/key \
	-Iutils/heap \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	-Iservices/resources \
	-Iservices/audio_dump/include \
	-Iservices/multimedia/speech/inc \
	-Iservices/audioflinger \
	-Iservices/app_ai/inc \
	-Iutils/cqueue \
	-Ithirdparty/userapi \
	-Iservices/ai_voice/manager \
	-Iservices/ai_voice/audio \
	-Iservices/ai_voice/protocol/ama/ama_manager

ifeq ($(VOICE_DETECTOR_SENS_EN),1)
ccflags-y += 	-Itests/sensor_hub/inc \
				-Itests/sensor_hub/sensor_hub_ai \
				-Iapps/sensorhub
endif
