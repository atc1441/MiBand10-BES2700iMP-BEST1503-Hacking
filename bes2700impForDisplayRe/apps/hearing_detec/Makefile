
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)
obj-y += hear_process/
ccflags-y := \
	-Iservices/multimedia/speech/inc \
	-Iapps/hearing_detec/hear_process

ifeq ($(SPEECH_TX_AEC_CODEC_REF),1)
ccflags-y += -DSPEECH_TX_AEC_CODEC_REF
endif