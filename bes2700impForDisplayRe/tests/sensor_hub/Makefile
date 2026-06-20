obj-y += core/ test/

ifneq ($(BT_APP),1)
COMPILE_AUD_SRCS ?= 1
COMPILE_APP_SRCS ?= 1
ifeq ($(NOAPP),1)
COMPILE_APP_SRCS := 0
COMPILE_AUD_SRCS := 0
endif
ifeq ($(VOICE_DETECTOR_EN),1)
COMPILE_AUD_SRCS := 1
COMPILE_APP_SRCS := 1
endif
ifeq ($(VAD_CODEC_TEST),1)
COMPILE_AUD_SRCS := 1
endif

ifneq ($(NOSTD),1)
obj-y += ../../utils/heap/ \
	../../utils/cqueue/ \

endif

ifeq ($(COMPILE_AUD_SRCS),1)
obj-y += ../../platform/drivers/codec/ \
	../../platform/drivers/ana/ \
	../../services/audioflinger/ \
	../../services/audio_dump/ \

endif

ifeq ($(COMPILE_APP_SRCS),1)
obj-y += \
	../../apps/common/ \
	../../apps/sensorhub/ \

endif
endif

ifeq ($(SENSOR_HUB_MINIMA),1)
obj-y += ../../platform/drivers/minima/ minima/
endif

ifeq ($(VAD_CODEC_TEST),1)
export AF_DEVICE_INT_CODEC ?= 1
export AF_DEVICE_I2S ?= 1
endif

ifeq ($(VOICE_DETECTOR_EN),1)
export AF_DEVICE_INT_CODEC ?= 1
export AF_DEVICE_I2S ?= 1

obj-y += vad/ \
	../../apps/voice_detector/ \

endif # VOICE_DETECTOR_EN

ifeq ($(AI_VOICE),1)
obj-y += sensor_hub_ai/
endif

ifeq ($(MINIMA_TEST),1)
obj-y += ../../tests/minima_test/
endif

ifeq ($(SENSOR_TEST),1)
obj-y += ../../tests/sensor_test/
endif

