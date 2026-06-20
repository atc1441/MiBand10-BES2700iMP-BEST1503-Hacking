
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := \
	-Iinclude/cmsis_dsp \
        -Iservices/audio_bt \
	-Iservices/audioflinger \
	-Iservices/audio_dump/include \
	-Iservices/multimedia/speech/inc

# FIXME: Because of app_audio.h
ccflags-y += \
	-Iapps \
	-Iapps/common \
	-Iapps/audioplayers \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	-Iutils/heap \
	-Iutils/cqueue

ccflags-y += \
	-Iservices/norflash_api \
	-Iplatform/drivers/cp_accel
