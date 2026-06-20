cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(APP_MCPP_CLI),SENS)
obj-y += ../../apps/anc/src/app_anc_table.o
endif

subdir-ccflags-y += \
	-Iservices/multimedia/speech/inc \
	-Iservices/multimedia/speech/src/process/anc_assist/ \
	-Iservices/multimedia/audio/process/resample/include \
	-Iservices/audio_dump/include \
	-Iinclude/cmsis_dsp \
	-Iservices/multimedia/audio/process/anc/include \
	-Iservices/nv_section/aud_section \
	-Iservices/nv_section/include \
	-Iservices/app_ai/inc\
	-Imultimedia/inc/speech/inc\
	-Iservices/multimedia/audio/process/filters/include \
	-Iapps/anc/src/assist\
	-Iapps/anc/src\
	-Iapps/anc/inc\
	-Iapps/common \
	-Iapps/voice_assist/inc \
	-Iservices/audio_bt \
	-Iservices/mcpp \
	-Iservices/audioflinger \
	-Iutils/kfifo \
	-Iutils/heap \
	-Iutils/anc_fir_coeff_config \
	-Itests/sensor_hub/core/
