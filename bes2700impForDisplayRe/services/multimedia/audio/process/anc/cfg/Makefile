cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y := \
	-Iservices/multimedia/audio/process/anc/include \
	-Iservices/multimedia/audio/process/psap/include \
	-Iservices/multimedia/audio/process/filters/include \
	-Iservices/nv_section/aud_section \
	-Iservices/nv_section/include \
	-Iutils/crc32

ifeq ($(HW_DAC_IIR_EQ_PROCESS),1)
CFLAGS_anc_cfg.o += -D__HW_DAC_IIR_EQ_PROCESS__
endif
ifeq ($(HW_IIR_EQ_PROCESS),1)
CFLAGS_anc_cfg.o += -D__HW_IIR_EQ_PROCESS__
endif
ifeq ($(AUDIO_RESAMPLE),1)
CFLAGS_anc_cfg.o += -D__AUDIO_RESAMPLE__
endif
ifeq ($(PSAP_APP),1)
CFLAGS_anc_cfg.o += -DPSAP_APP
endif
