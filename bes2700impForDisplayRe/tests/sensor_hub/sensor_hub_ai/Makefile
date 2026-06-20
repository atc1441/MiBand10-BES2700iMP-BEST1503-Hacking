cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj-y := $(obj-y:.c=.o)

ccflags-y := -Iplatform/hal \
	-Iplatform/cmsis \
	-Iplatform/drivers/ana \
	-Iutils/hwtimer_list \
	-Iutils/cqueue \
	-Iservices/audioflinger \
	-Iapps/sensorhub \
	-Iapps/voice_detector \
	-Iapps/common \
	-Itests/sensor_hub/inc \
	-Itests/sensor_hub/core \
	-Itests/sensor_hub/vad \
	-Itests/sensor_hub/sensor_hub_ai \
	-Itests/sensor_hub/test \

ifeq ($(AI_VOICE),1)
ccflags-y += -DAI_VOICE
endif

ifeq ($(SENSOR_HUB_AI_AUDIO_DUMP_EN),1)
ccflags-y += -DSENSOR_HUB_AI_AUDIO_DUMP_EN
endif

ifneq ($(THIRDPARTY_LIB),)
ifeq ($(THIRDPARTY_BIXBY),1)
ccflags-y += -DTHIRDPARTY_LIB_SS -Ithirdparty/senshub_lib/bixby/inc
endif
ifeq ($(THIRDPARTY_GSOUND),1)
ccflags-y += -DTHIRDPARTY_LIB_GG -Ithirdparty/senshub_lib/gsound/inc
endif
ifeq ($(THIRDPARTY_ALEXA),1)
ccflags-y += -DTHIRDPARTY_LIB_ALEXA -Ithirdparty/senshub_lib/alexa/inc
endif
endif # THIRDPARTY_LIB

