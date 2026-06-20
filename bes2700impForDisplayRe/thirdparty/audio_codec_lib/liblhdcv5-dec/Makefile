cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))
# -DLHDC_SE_VERSION
# -DHAVE_TEST_KEY
CORP_NAME	   := SAVI
LOG_BUF_SIZE   := 512
SV_LIB_NAME    := libLHDC_V5

VERSION_PREFIX		   := BES-LHDC-
NORMAL_VERSION_NUM	   := 5.0.9

auth_type	   :=
demo_flg	   ?= -DSAVI_DEMO_MODE
codec_type	   := -DLOG_BUF_LEN=$(LOG_BUF_SIZE) -D__LHDC_DEBUG__
VERSION_NUM    := $(NORMAL_VERSION_NUM)
build_time     := $(shell date +%y%m%d_%H%M%S)

ifneq ($(GEN_LIB), 1)
auth_type += -DHAVE_TEST_KEY
endif

ifneq ($(DEMO_MODE), 1)
demo_flg	    := 
endif

auth_type += -DLICE_KEY_TYPE_KEYPRO -DLHDC_VERIFY_UUID -DVERSION=\"$(VERSION_PREFIX)$(VERSION_NUM)_$(build_time)\"

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)dec_src/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)dec_src/svlice_v5/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)dec_src/lhdc_v5_dec/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))


ifneq (,$(findstring best2200_,$(T)))
PLATFORM_NAME := BEST2200
DEF := -DPLAT_BES  -DFLASH_SIZE=$(FLASH_SIZE) $(auth_type) $(demo_flg)
else ifneq (,$(findstring best2300p_,$(T)))
PLATFORM_NAME := BEST2300P
DEF := -DPLAT_BES  -march="armv7e-m" -DFLASH_SIZE=$(FLASH_SIZE) $(auth_type) $(demo_flg) ${codec_type}
else ifneq (,$(findstring best2500p_,$(T)))
PLATFORM_NAME := BEST2500P
DEF := -Wno-implicit-fallthrough -Wno-unused-function -Wno-unused-variable -Wno-maybe-uninitialized -DPLAT_BES -DFLASH_SIZE=$(FLASH_SIZE) $(auth_type) $(demo_flg) ${codec_type}
else ifneq (,$(findstring best2500i_,$(T)))
PLATFORM_NAME := BEST2500I
DEF := -Wno-implicit-fallthrough -Wno-unused-function -Wno-unused-variable -DPLAT_BES -DFLASH_SIZE=$(FLASH_SIZE) $(auth_type) $(demo_flg) ${codec_type}
else ifneq (,$(findstring best2300_,$(T)))
PLATFORM_NAME := BEST2300
DEF := -DPLAT_BES -DFLASH_SIZE=$(FLASH_SIZE) $(auth_type) $(demo_flg)
else ifneq (,$(findstring best2300a_,$(T)))
PLATFORM_NAME := BEST2300A
DEF := -DPLAT_BES -DFLASH_SIZE=$(FLASH_SIZE) $(auth_type) $(demo_flg)
endif

## Add Platform flag
DEF += -D$(PLATFORM_NAME)
## Modify Version number to a string for library name use.
VERSION_STRING := $(subst .,_,$(VERSION_NUM))

ifneq ($(PLATFORM_NAME),)
LHDC_LIB_NAME := $(PLATFORM_NAME)_$(SV_LIB_NAME)_$(VERSION_STRING)_$(CORP_NAME)_KEYPRO_UUID
else
LHDC_LIB_NAME := BEST2500P_libLHDC_V5_5_2_0_SAVI_KEYPRO_UUID
endif

$(LHDC_LIB_NAME)-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

LIB_BIN_DIR := lib/thirdparty/audio_codec/liblhdcv5-dec
obj-y += $(LHDC_LIB_NAME).a

ccflags-y += -lc -O3 -Wchar-subscripts $(DEF)

ccflags-y +=  -DLHDC_V5_DEC_MODEL

subdir-ccflags-y += \
	-Iservices/tws/inc \
	-Iutils/heap \
	-Iutils/cqueue \
	-Iapps/audioplayers \
	-Iservices/bt_app \
	-Iplatform \
	-Iplatform/hal \
    -Ithirdparty/audio_codec_lib/liblhdcv5-dec/dec_src \
	-Ithirdparty/audio_codec_lib/liblhdcv5-dec \
	-Iservices/ble_stack/ke/api \
	-Iservices/ble_stack/ble_ip \
	$(BT_PROFILES_INCLUDES) \
	-Iservices/nvrecord \
	-Iservices/bt_if/inc \
	-Iservices/bt_if_enhanced/inc \
	-Iconfig \
	-Iinclude/cmsis_dsp \
	-Iservices/ble_stack/hl/api

subdir-ccflags-y += -Ithirdparty/audio_codec_lib/liblhdcv5-dec/dec_src/lhdc_v5_dec
subdir-ccflags-y += -Ithirdparty/audio_codec_lib/liblhdcv5-dec/inc

