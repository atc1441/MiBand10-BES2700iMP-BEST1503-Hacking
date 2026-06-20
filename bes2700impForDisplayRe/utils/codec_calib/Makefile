cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj-y := $(obj-y:.c=.o)

ccflags-y := -Iplatform/hal -Iplatform/cmsis -Iservices/audioflinger \
	-Iplatform/drivers/ana \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	-Iutils/hwtimer_list \
	-Iutils/heap \
	-Iservices/audio_bt \
	-Iapps/common \
	-Iservices/nv_section/userdata_section

AUDIO_OUTPUT_ERROR_LARGE_DC ?= 1

ifeq ($(AUDIO_OUTPUT_DC_PROD_TEST),1)
CODEC_DAC_DC_DYN_BUF ?= 1
ccflags-y += -DAUDIO_OUTPUT_DC_PROD_TEST
endif
ifeq ($(CODEC_DAC_DC_DYN_BUF),1)
ccflags-y += -DCODEC_DAC_DC_DYN_BUF
endif
ifeq ($(CODEC_ADC_DC_DYN_BUF),1)
ccflags-y += -DCODEC_ADC_DC_DYN_BUF
endif
ifeq ($(AUDIO_OUTPUT_SET_LARGE_ANA_DC),1)
ccflags-y += -DAUDIO_OUTPUT_SET_LARGE_ANA_DC
endif
ifeq ($(AUDIO_OUTPUT_ERROR_LARGE_DC),1)
ccflags-y += -DAUDIO_OUTPUT_ERROR_LARGE_DC
endif
ifeq ($(AUDIO_OUTPUT_DC_CALIB_DUAL_CHAN),1)
ccflags-y += -DAUDIO_OUTPUT_DC_CALIB_DUAL_CHAN
endif
ifeq ($(CODEC_DAC_DC_MUTE_TEST),1)
ccflags-y += -DCODEC_DAC_DC_MUTE_TEST
endif
ifeq ($(CODEC_DAC_ON_OFF_TEST),1)
ccflags-y += -DCODEC_DAC_ON_OFF_TEST
endif
ifeq ($(CODEC_DAC_DC_CHECK),1)
ccflags-y += -DCODEC_DAC_DC_CHECK
endif
ifeq ($(ANC_PROD_TEST),1)
ifeq ($(USB_AUDIO_APP),1)
ccflags-y += -DUSB_AUDIO_APP
endif
ccflags-y += -DANC_PROD_TEST
endif
