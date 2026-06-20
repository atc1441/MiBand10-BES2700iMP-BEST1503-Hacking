cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ifneq ($(LTO_CFLAGS),)
KBUILD_CFLAGS := $(filter-out $(LTO_CFLAGS),$(KBUILD_CFLAGS))
KBUILD_CPPFLAGS := $(filter-out $(LTO_CPPFLAGS),$(KBUILD_CPPFLAGS))
endif

ccflags-y += \
    -Iplatform/hal \
    -Iplatform/drivers/ana \
    -Iutils/heap \
    -Iinclude/watchdog

ifeq ($(FLASH_SUSPEND),1)
CFLAGS_norflash_api.o += -DFLASH_SUSPEND
endif

ifeq ($(FLASH_REMAP),1)
CFLAGS_norflash_api.o += -DFLASH_REMAP
endif

ifeq ($(FLASH_UNIQUE_ID),1)
ccflags-y += -DFLASH_UNIQUE_ID
endif

ifeq ($(FLASH_PROTECTION),1)
CFLAGS_norflash_api.o += -DFLASH_PROTECTION
endif

ifneq ($(OTA_CODE_OFFSET),)
CFLAGS_norflash_api.o += -DOTA_CODE_OFFSET=$(OTA_CODE_OFFSET)
endif

ifneq ($(OTA_REMAP_OFFSET),)
CFLAGS_norflash_api.o += -DOTA_REMAP_OFFSET=$(OTA_REMAP_OFFSET)
endif

ifeq ($(FLASH_PROTECTION_BOOT_SECTION_FIRST),1)
CFLAGS_norflash_api.o += -DFLASH_PROTECTION_BOOT_SECTION_FIRST
endif

ifeq ($(PUYA_FLASH_ERASE_PAGE_ENABLE),1)
CFLAGS_norflash_api.o += -DPUYA_FLASH_ERASE_PAGE_ENABLE
endif

ifeq ($(USB_AUDIO_APP),1)
ifneq (,$(filter $(BTUSB_AUDIO_MODE) $(BT_USB_AUDIO_DUAL_MODE), 1))
export FLASH_API_SIMPLE ?= 1
endif
endif

ifeq ($(FLASH_API_SIMPLE),1)
CFLAGS_norflash_api.o += -DFLASH_API_SIMPLE
else
ifeq ($(FLASH_API_HIGHPERFORMANCE),1)
CFLAGS_norflash_api.o += -DFLASH_API_HIGHPERFORMANCE
else
CFLAGS_norflash_api.o += -DFLASH_API_NORMAL
endif
endif

ifeq ($(FLASH_SECURITY_REGISTER),1)
CFLAGS_norflash_api.o += -DFLASH_SECURITY_REGISTER
endif

ifeq ($(FLASH_GD256M_SR_WHIT_CMP),1)
CFLAGS_norflash_api.o += -DFLASH_GD256M_SR_WHIT_CMP
endif

ifeq ($(NORFLASH_API_FLUSH_IN_SLEEP_HOOK),1)
CFLAGS_norflash_api.o += -DNORFLASH_API_FLUSH_IN_SLEEP_HOOK
endif

ifneq ($(FLASH0_MAX_SIZE_MB),)
CFLAGS_norflash_api.o += -DFLASH0_MAX_SIZE_MB=$(FLASH0_MAX_SIZE_MB)
endif


ifeq ($(ARM_CMNS),1)
ccflags-y += -Iplatform/main/cmse
endif
