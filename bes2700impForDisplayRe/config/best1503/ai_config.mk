#### alexa ai voice ####
AMA_VOICE ?= 1

#### xiaodu ai voice ####
DMA_VOICE := 0

#### ali ai voice ####
GMA_VOICE := 0

#### BES ai voice ####
SMART_VOICE := 0

#### samsung ai voice ####
BIXBY_WWE := 0

#### xiaowei ai voice ####
TENCENT_VOICE := 0
CUSTOMIZE_VOICE := 0

#### xiaomi ai voice ####
MMA_VOICE := 0

#### Google related feature ####
# the overall google service switch
# currently, google service includes BISTO and GFPS
export GOOGLE_SERVICE_ENABLE := 1

# BISTO is a GVA service on Bluetooth audio device
# BISTO is an isolated service relative to GFPS
export BISTO_ENABLE ?= 1
ifeq ($(BISTO_ENABLE),1)
BES_OTA := 1
endif

# macro switch for reduced_guesture
export REDUCED_GUESTURE_ENABLE ?= 0

# GSOUND_HOTWORD is a hotword library running on Bluetooth audio device
# GSOUND_HOTWORD is a subset of BISTO
export GSOUND_HOTWORD_ENABLE ?= 1

# this is a subset choice for gsound hotword
export GSOUND_HOTWORD_EXTERNAL ?= 1
ifeq ($(GSOUND_HOTWORD_EXTERNAL),1)
export GSOUND_HOTWORD_EXTERNAL_M33 := 1
ifeq ($(GSOUND_HOTWORD_EXTERNAL_M33),1)
KBUILD_CFLAGS += -DGSOUND_HOTWORD_EXTERNAL_M33
endif
export SHARE_MEM_DATA_SIZE = 1
KBUILD_CFLAGS += -DSHARE_MEM_DATA_SIZE=$(SHARE_MEM_DATA_SIZE)
endif

# GFPS is google fastpair service
# GFPS is an isolated service relative to BISTO
export AMA_ENCODE_USE_SBC ?= 1
#### Google related feature ####

## Microsoft swift pair
export SWIFT_ENABLE ?= 1

SLAVE_ADV_BLE_ENABLED := 0

export CTKD_ENABLE ?= 1

export GATT_OVER_BR_EDR ?= 0
ifeq ($(GATT_OVER_BR_EDR),1)
KBUILD_CPPFLAGS += -D__GATT_OVER_BR_EDR__
endif

export MUTLI_POINT_AI ?= 1
ifeq ($(MUTLI_POINT_AI),1)
KBUILD_CPPFLAGS += -D__MUTLI_POINT_AI__
endif

BLE := 1
export FAST_XRAM_SECTION_SIZE ?= 0x1100

#### VAD config are both for bisto and alexa ####
# 1 to enable the VAD feature, 0 to disable the VAD feature
export VOICE_DETECTOR_SENS_EN ?= 0

#### use the hot word lib of amazon ####
ifeq ($(AMA_VOICE),1)
export AMA_ENCODE_USE_SBC ?= 1
export ALEXA_WWE := 1
#### a subset choice for the hot word lib of amazon -- lite mode####
export ALEXA_WWE_LITE := 1
ifeq ($(ALEXA_WWE),1)
KBUILD_CPPFLAGS += -D__ALEXA_WWE
export USE_THIRDPARTY := 1
#TRACE_BUF_SIZE := 8*1024
FAST_XRAM_SECTION_SIZE := 0x14000

ifeq ($(ALEXA_WWE_LITE),1)
KBUILD_CPPFLAGS += -D__ALEXA_WWE_LITE
export THIRDPARTY_LIB := kws/alexa_lite
else
export THIRDPARTY_LIB := kws/alexa
export MCU_HIGH_PERFORMANCE_MODE := 1
endif
endif
endif

#### use the hot word lib of BES ####
export KWS_ALEXA := 0
ifeq ($(KWS_ALEXA),1)
export MCU_HIGH_PERFORMANCE_MODE :=1
export USE_THIRDPARTY := 1
export THIRDPARTY_LIB := kws/bes

FAST_XRAM_SECTION_SIZE := 0x14000
AQE_KWS_NAME := ALEXA
endif

#### use the hot word lib of bixby ####
ifeq ($(BIXBY_WWE),1)
KBUILD_CPPFLAGS += -D__BIXBY
export USE_THIRDPARTY := 1
FAST_XRAM_SECTION_SIZE ?= 0xF100
export THIRDPARTY_LIB := bixby
# Integer multiple times of encoding chunk size 456 bytes: 57*8 frames
KBUILD_CPPFLAGS += -DVOB_ENCODED_DATA_STORAGE_BUF_SIZE=18240
export BIXBY_VOICE := 1
SV_ENCODE_USE_SBC := 1

KBUILD_CPPFLAGS += -DBIXBY_USE_BESTOOL
export SS_BIXBY_INTEGRATED ?= 1
ifeq ($(SS_BIXBY_INTEGRATED),1)
KBUILD_CPPFLAGS += -DSS_BIXBY_INTEGRATED
endif
endif

ifeq ($(ALEXA_WWE)_$(KWS_ALEXA),0_0)
#KBUILD_CPPFLAGS += -DRTOS_IN_RAM
endif

## ai parts ram optimization ##
## audio configure ##
KBUILD_CPPFLAGS += -DVOB_ENCODED_DATA_STORAGE_BUF_SIZE=1500
KBUILD_CPPFLAGS += -DENCODE_BUF_SIZE=2*1024
KBUILD_CPPFLAGS += -DPCM_DATA_CACHE_BUF_SIZE=4*1024
## end of audio configure ##

## thread configure ##
KBUILD_CPPFLAGS += -DGSOUND_AUDIO_THREAD_SIZE=1024
KBUILD_CPPFLAGS += -DGSOUND_MQ_THREAD_SIZE=1024*2
KBUILD_CPPFLAGS += -DAI_VOICE_THREAD_SIZE=1024*3
KBUILD_CPPFLAGS += -DOTA_BLE_RX_HANDLER_THREAD_SIZE=3*512
KBUILD_CPPFLAGS += -DAI_THREAD_SIZE=1024*3
## end of thread configure ##

## variable configure ##
KBUILD_CPPFLAGS += -DAI_REV_BUF_SIZE=1024*2
## end of variable configure ##

#if anc assist enabled, DPCM_AUDIO_FLINGER_BUF_SIZE=0
#KBUILD_CPPFLGAS += -DPCM_AUDIO_FLINGER_BUF_SIZE=4*1024
ifeq ($(ANC_ASSIST_ENABLED),1)
KBUILD_CPPFLGAS += -DPCM_AUDIO_FLINGER_BUF_SIZE=0
else
KBUILD_CPPFLGAS += -DPCM_AUDIO_FLINGER_BUF_SIZE=4*1024
endif
## end of ai parts ram optimization##