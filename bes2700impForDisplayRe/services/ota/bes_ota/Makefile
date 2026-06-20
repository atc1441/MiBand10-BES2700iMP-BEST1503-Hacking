cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.cpp))

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

OTA_LIB_NAME := lib_ota

ifeq ($(IBRT), 1)
OTA_LIB_NAME := $(OTA_LIB_NAME)_ibrt
else
OTA_LIB_NAME := $(OTA_LIB_NAME)_stereo
endif

ifeq ($(BLE), 1)
OTA_LIB_NAME := $(OTA_LIB_NAME)_ble
endif

$(OTA_LIB_NAME)-y := $(src_obj)

obj-y += $(OTA_LIB_NAME).a

export WIN_PLAT := n
ifeq ($(OS),Windows_NT)
# Detect Cygwin
ifneq ($(findstring /,$(PWD)),/)
# also $(findstring /,$(HOME)) ?
WIN_PLAT := y
endif
endif

# Git revision
ifeq ($(WIN_PLAT),y)
GIT_REVISION := $(shell (where git >nul 2>&1) && (git rev-parse --short HEAD 2>nul))
else
GIT_REVISION := $(shell (which git >/dev/null 2>&1) && (git rev-parse --short HEAD 2>/dev/null))
endif

ifneq ($(GIT_REVISION),)
ifeq ($(WIN_PLAT),y)
GIT_REVISION := $(GIT_REVISION)$(shell (git diff --quiet && git diff --cached --quiet) >nul 2>&1 || echo -$(VERSION_INFO))
else
GIT_REVISION := $(GIT_REVISION)$(shell (git diff --quiet && git diff --cached --quiet) >/dev/null 2>&1 || echo -$(VERSION_INFO))
endif
endif

CFLAGS_ota_control.o += -DGIT_VERSION=$(GIT_REVISION)

CFLAGS_ota_config.o += -DBESLIB_INFO=$(BESLIB_INFO)

subdir-ccflags-y += \
    -Iplatform/hal \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/bt \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BT_SERVICE_UX_INCLUDES) \
    -Iservices/nv_section/factory_section \
    -Iservices/nv_section/userdata_section \
    -Iservices/norflash_api \
    -Iservices/ota/bes_ota/inc \
    -Iservices/audio_bt \
    -Iutils/boot_struct \
    -Iutils/crc32 \
    -Iutils/hwtimer_list \
    -Iutils/heap \
    -Iutils/cqueue \
    -Iapps/key \
    -Iapps/main \
    -Iapps/common \
    -Iapps/factory \
    -Iapps/audioplayers \
    -Iapps/ota \
    -Iservices/ota \
    -Iinclude/watchdog \
    -Iservices/audio_manager \
    -Iapps/earbuds/conn \
	-Ibthost/service/ble_app_new/inc 

ifeq ($(TEST_OVER_THE_AIR_v2),1)
ccflags-y += -Iservices/tota_v2
endif

ifeq ($(TEST_OVER_THE_AIR),1)
ccflags-y += -Iservices/tota
endif

#ifeq ($(AI_OTA),1)
subdir-ccflags-y += -Iservices/ota/ai_ota
#endif
