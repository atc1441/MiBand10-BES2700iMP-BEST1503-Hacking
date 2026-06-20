cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

subdir-ccflags-y += \
    -Iapps/karaoke \
    -Iinclude/cmsis_dsp \
    -Iservices/audio_bt \
    -Iservices/audio_process \
    -Iservices/overlay \
    -Iservices/multimedia/speech/inc

ifeq ($(KARAOKE_ALGO_NS),1)
CFLAGS_app_karaoke.o += -DKARAOKE_ALGO_NS
endif

ifeq ($(KARAOKE_ALGO_EQ),1)
CFLAGS_app_karaoke.o += -DKARAOKE_ALGO_EQ
endif