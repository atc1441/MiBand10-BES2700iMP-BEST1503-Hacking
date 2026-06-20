cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)


subdir-ccflags-y +=  \
	-Ithirdparty/kws/alexa_lib/inc \
	-Iapps/audioplayers \
	-Iapps/common \
	-Iapps/key \
	-Iutils/heap \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	-Iservices/resources \
	-Iservices/audio_dump/include \
	-Iservices/multimedia/speech/inc \
	-Iservices/audioflinger \
	-Iservices/ai_voice/manager \
	-Iutils/cqueue \
	-Ithirdparty/userapi
