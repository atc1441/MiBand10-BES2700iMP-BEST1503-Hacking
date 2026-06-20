cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj-y := $(obj-y:.c=.o)

ccflags-y := -Iplatform/hal \
	-Iplatform/cmsis \
	-Iplatform/drivers/ana \
	-Iplatform/drivers/sensor_hub \
	-Iservices/audioflinger \
	-Itests/sensor_hub/inc \
	-Itests/sensor_hub/core \
	-Itests/sensor_hub/vad \
	-Itests/sensor_hub/test \


ifeq ($(MCU2SENS_MSG_TEST),1)
ccflags-y += -DMCU2SENS_MSG_TEST
endif

ifeq ($(VD_TEST),1)
ccflags-y += -DVD_TEST
endif

ifeq ($(VAD_CODEC_TEST),1)
ccflags-y += -DVAD_CODEC_TEST
endif

