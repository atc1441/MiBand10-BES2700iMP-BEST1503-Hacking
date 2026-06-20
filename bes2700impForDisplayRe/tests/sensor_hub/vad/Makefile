cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj-y := $(obj-y:.c=.o)

ccflags-y := -Iplatform/hal \
	-Iplatform/cmsis \
	-Iplatform/drivers/ana \
	-Iplatform/drivers/sensor_hub \
	-Iutils/hwtimer_list \
	-Iservices/audioflinger \
	-Iservices/multimedia/speech/inc \
	-Iapps/sensorhub \
	-Iapps/voice_detector \
	-Iapps/common \
	-Itests/sensor_hub/inc \
	-Itests/sensor_hub/core \
	-Itests/sensor_hub/vad \
	-Itests/sensor_hub/minima \
	-Itests/sensor_hub/sensor_hub_ai \
	-Itests/sensor_hub/test \
	-Itests/sensor_hub/capsensor \

ifeq ($(AI_VOICE),1)
ccflags-y += -DAI_VOICE
endif

ifeq ($(VD_TEST),1)
ccflags-y += -DVD_TEST
endif

ifeq ($(VOICE_DETECTOR_EN),1)
ccflags-y += -DVOICE_DETECTOR_EN
endif

ifneq ($(VAD_ADPT_CLOCK_FREQ),)
ccflags-y +=-DVAD_ADPT_CLOCK_FREQ=$(VAD_ADPT_CLOCK_FREQ)
endif

ifeq ($(VAD_ADPT_TEST_ENABLE),1)
ccflags-y += -DVAD_ADPT_TEST_ENABLE
endif

ifeq ($(FULL_WORKLOAD_MODE_ENABLED),1)
ccflags-y += -DFULL_WORKLOAD_MODE_ENABLED
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

