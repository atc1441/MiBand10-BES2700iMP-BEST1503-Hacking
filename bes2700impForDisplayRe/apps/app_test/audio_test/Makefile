
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

subdir-ccflags-y += \
    -Iservices/audio_bt \
    -Iservices/bone_sensor \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    -Iservices/resources \
    -Iutils/cqueue \
    -Iservices/mcpp \
    -Iservices/multimedia/speech/inc \
    -Iservices/multimedia/audio/process/anc/include\
    -Iutils/kfifo \
    -Iapps/anc/inc \
    -Iservices/multimedia/speech/inc \
    -Iservices/nv_section/aud_section \
    -Iservices/nv_section/include


ifeq ($(AUDIO_DYNAMIC_BOOST),1)
CFLAGS_audio_test_cmd.o += -D__AUDIO_DYNAMIC_BOOST__
endif

ifeq ($(A2DP_KARAOKE),1)
CFLAGS_audio_test_cmd.o += -DA2DP_KARAOKE
endif