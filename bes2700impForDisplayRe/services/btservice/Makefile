cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

#——————————————————————————————————————————————
# CONFIG
#——————————————————————————————————————————————
export BT_SERVICE_HEAP_SIZE ?= 1*1024
KBUILD_CPPFLAGS += -DBT_SERVICE_HEAP_SIZE=$(BT_SERVICE_HEAP_SIZE)

#——————————————————————————————————————————————
# build source code
#——————————————————————————————————————————————
BT_SERVICE_NAME := btservice

ifeq ($(IBRT_UI),1)
BT_SERVICE_NAME := $(BT_SERVICE_NAME)_multipoint
endif

ifeq ($(AI_VOICE), 1)
BT_SERVICE_NAME := $(BT_SERVICE_NAME)_ai
endif

ifeq ($(BLE), 1)
BT_SERVICE_NAME := $(BT_SERVICE_NAME)_ble
endif

ifeq ($(USE_TRACE_ID), 1)
BT_SERVICE_NAME := $(BT_SERVICE_NAME)_log_compressed
endif

ifeq ($(SASS_ENABLE), 1)
BT_SERVICE_NAME := $(BT_SERVICE_NAME)_sass
endif

ifeq ($(GFPS_ENABLE), 1)
BT_SERVICE_NAME := $(BT_SERVICE_NAME)_gfps
endif

ifeq ($(BLE_AUDIO_ENABLED), 1)
BT_SERVICE_NAME := $(BT_SERVICE_NAME)_aob
endif

ifeq ($(SUPPORT_REMOTE_COD), 1)
BT_SERVICE_NAME := $(BT_SERVICE_NAME)_cod
endif

ifeq ($(GATT_OVER_BR_EDR), 1)
BT_SERVICE_NAME := $(BT_SERVICE_NAME)_gatt
endif

ifeq ($(FREEMAN_ENABLED_STERO), 1)
BT_SERVICE_NAME := $(BT_SERVICE_NAME)_stero
endif

BT_SERVICE_NAME := $(BT_SERVICE_NAME)_ble_new_stack

ifeq ($(ARM_CMNS), 1)
BT_SERVICE_NAME := $(BT_SERVICE_NAME)_ARM_CMNS
endif

ifeq ($(AOB_MOBILE_ENABLED), 1)
ifeq ($(BLE_USB_AUDIO_SUPPORT), 1)
BT_SERVICE_NAME := $(BT_SERVICE_NAME)_usb_dongle
endif
endif

ifeq ($(ANC_APP), 1)
BT_SERVICE_NAME := $(BT_SERVICE_NAME)_anc
endif

ifeq ($(AOB_MOBILE_ENABLED), 1)
ifeq ($(BLE_USB_AUDIO_SUPPORT), 1)
BT_SERVICE_NAME := $(BT_SERVICE_NAME)_usb_dongle
endif
endif

ifeq ($(BT_SERVICE_TEST_ENABLE),1)
KBUILD_CPPFLAGS += -DBT_SERVICE_TEST_ENABLE
endif

ifneq ($(wildcard $(cur_dir)/base/Makefile),)
src_obj += base/
endif

ifneq ($(wildcard $(cur_dir)/common/Makefile),)
src_obj += common/
endif

ifneq ($(wildcard $(cur_dir)/product/Makefile),)
src_obj += product/
endif

ifeq ($(BT_SVC_MODULE_ENABLED),1)
ifneq ($(wildcard $(cur_dir)/module/Makefile),)
src_obj += module/
endif
endif

ifeq ($(BT_SVC_FW_ENABLED),1)
ifneq ($(wildcard $(cur_dir)/framework/Makefile),)
src_obj += framework/
endif
endif

$(BT_SERVICE_NAME)-y := $(src_obj)
obj-y := $(BT_SERVICE_NAME).a
