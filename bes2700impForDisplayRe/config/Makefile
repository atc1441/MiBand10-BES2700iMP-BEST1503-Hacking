cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

#obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))

FILE_LIST := tgt_hardware.c

ifneq ($(SLAVE_BIN_FLASH_OFFSET),)
FILE_LIST += slave_code.S
$(obj)/$(DEFAULT_CFG_SRC)/slave_code.o : out/$(SLAVE_BIN_TARGET_NAME)/$(SLAVE_BIN_TARGET_NAME).bin
AFLAGS_slave_code.o += -DSLAVE_BIN_NAME=$(SLAVE_BIN_TARGET_NAME).bin -Wa,-Iout/$(SLAVE_BIN_TARGET_NAME)
endif

obj-y := $(patsubst $(cur_dir)%,%,$(foreach f,$(FILE_LIST), \
    $(if $(wildcard $(cur_dir)$(T)/$(f)),$(cur_dir)$(T)/$(f),$(cur_dir)$(DEFAULT_CFG_SRC)/$(f))))

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y += \
	-Iplatform/hal \
	-Iplatform/drivers/ana \
	-Iplatform/drivers/usb/usb_dev/inc \
	-Iservices/nv_section/aud_section \
	-Iservices/nv_section/include \
	-Iservices/multimedia/speech/inc \
	-Iservices/multimedia/audio/process/filters/include \
	-Iservices/multimedia/audio/process/drc/include \
	-Iservices/multimedia/audio/process/limiter/include \
	-Iservices/multimedia/audio/process/reverb/include \
	-Iutils/heap \
	-Iservices/multimedia/audio/process/bass_enhancer/include \
	-Iservices/multimedia/audio/process/dynamic_boost/include \
	-Iservices/multimedia/audio/process/dynamic_eq/include \
	-Iservices/multimedia/audio/process/virtual_surround_process/include

ifeq ($(BT_DEV_NAME),)
BT_DEV_NAME := $(T).$(BUILD_HOSTNAME).$(BUILD_USERNAME)
endif

ccflags-y += -DBT_DEV_NAME=$(BT_DEV_NAME) $(CUST_TGT_FLAGS)

ifneq ($(VMIC_MAP_LIST),)
ifneq ($(filter 0 null NULL, $(VMIC_MAP_LIST)),)
VMIC_MAP_CFG := 0
else
vmic_list := $(addprefix AUD_VMIC_MAP_VMIC,$(subst $(comma),$(space),$(VMIC_MAP_LIST)))
VMIC_MAP_CFG := $(subst $(space),\|,$(vmic_list))
endif
endif
ifneq ($(VMIC_MAP_CFG),)
CFLAGS_tgt_hardware.o += -DVMIC_MAP_CFG=$(VMIC_MAP_CFG)
endif

ifneq ($(MAINMIC_MAP_LIST),)
tmp_list := $(subst $(comma),$(space),$(MAINMIC_MAP_LIST))
mic_list := $(addprefix AUD_CHANNEL_MAP_CH,$(filter-out d% e%,$(tmp_list)))
digmic_list := $(addprefix AUD_CHANNEL_MAP_DIGMIC_CH,$(subst d,,$(filter d%,$(tmp_list))))
ecmic_list := $(addprefix AUD_CHANNEL_MAP_ECMIC_CH,$(subst e,,$(filter e%,$(tmp_list))))
MAINMIC_MAP_CFG := $(subst $(space),\|,$(strip $(mic_list) $(digmic_list) $(ecmic_list)))
endif
ifneq ($(MAINMIC_MAP_CFG),)
CFLAGS_tgt_hardware.o += -DMAINMIC_MAP_CFG=$(MAINMIC_MAP_CFG)
endif

ifneq ($(LINEIN_MAP_LIST),)
linein_list := $(addprefix AUD_CHANNEL_MAP_CH,$(subst $(comma),$(space),$(LINEIN_MAP_LIST)))
LINEIN_MAP_CFG := $(subst $(space),\|,$(linein_list))
endif
ifneq ($(LINEIN_MAP_CFG),)
CFLAGS_tgt_hardware.o += -DLINEIN_MAP_CFG=$(LINEIN_MAP_CFG)
endif

ifneq ($(VADMIC_MAP_LIST),)
tmp_list := $(subst $(comma),$(space),$(VADMIC_MAP_LIST))
mic_list := $(addprefix AUD_CHANNEL_MAP_CH,$(filter-out d%,$(tmp_list)))
digmic_list := $(addprefix AUD_CHANNEL_MAP_DIGMIC_CH,$(subst d,,$(filter d%,$(tmp_list))))
VADMIC_MAP_CFG := $(subst $(space),\|,$(strip $(mic_list) $(digmic_list)))
endif
ifneq ($(VADMIC_MAP_CFG),)
CFLAGS_tgt_hardware.o += -DVADMIC_MAP_CFG=$(VADMIC_MAP_CFG)
endif

ifeq ($(USB_AUDIO_SPEECH),1)
CFLAGS_tgt_hardware.o += -DUSB_AUDIO_SPEECH
endif

ifneq ($(BT_DEV_ADDR),)
ccflags-y += -DBT_DEV_ADDR=$(BT_DEV_ADDR)
endif

ifneq ($(BLE_DEV_ADDR),)
ccflags-y += -DBT_DEV_ADDR=$(BLE_DEV_ADDR)
endif

ifneq ($(USB_AUDIO_SEND_CHAN),)
ccflags-y += -DUSB_AUDIO_SEND_CHAN=$(USB_AUDIO_SEND_CHAN)
endif

ifeq ($(SPEECH_TX_AEC_CODEC_REF),1)
ccflags-y += -DSPEECH_TX_AEC_CODEC_REF
endif

ifeq ($(ANC_NOISE_TRACKER),1)
ccflags-y += -DANC_NOISE_TRACKER
endif

ifeq ($(DONGLE_SUPPORT), 1)
ccflags-y += -DDONGLE_SUPPORT
endif

ifeq ($(MIC_KEY), 1)
ccflags-y += -DCFG_MIC_KEY
endif

ifeq ($(CHARGER_1802),1)
CFLAGS_tgt_hardware.o  += -DCHARGER_1802
endif
