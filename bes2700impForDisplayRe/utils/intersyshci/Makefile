
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)intersyshci.cpp))

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

INTERSYSHCI_LIB_NAME := libintersyshci

ifeq ($(INTERSYS_NO_THREAD),1)
INTERSYSHCI_LIB_NAME := $(INTERSYSHCI_LIB_NAME)_nothread
endif

ifeq ($(BYPASS_STACK_AUDIO_STREAM),1)
INTERSYSHCI_LIB_NAME := $(INTERSYSHCI_LIB_NAME)_bypasssas
endif

INTERSYSHCI_LIB_NAME := $(INTERSYSHCI_LIB_NAME)_enhanced_stack

ifeq ($(USE_TRACE_ID), 1)
INTERSYSHCI_LIB_NAME := $(INTERSYSHCI_LIB_NAME)_log_compressed
endif

ifeq ($(BLE_AUDIO_ENABLED), 1)
INTERSYSHCI_LIB_NAME := $(INTERSYSHCI_LIB_NAME)_aob
endif

CFLAGS_intersyshci.o += -DBESLIB_INFO=$(BESLIB_INFO)

$(INTERSYSHCI_LIB_NAME)-y := $(src_obj)
obj-y += $(INTERSYSHCI_LIB_NAME).a
ccflags-y := \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    -Iapps/common/ \
    -Iutils/cqueue \
    -Iplatform/drivers/bt \
    -Itests/ota_boot/inc

ifeq ($(INTERSYS_NO_THREAD),1)
CFLAGS_intersyshci.o += -D_INTERSYS_NO_THREAD_
endif

ifdef DISPLAY_PREFIX_HCI_CMD_EVT_
ifneq ($(DISPLAY_PREFIX_HCI_CMD_EVT_),0)
subdir-ccflags-y += -DDISPLAY_PREFIX_HCI_CMD_EVT
endif
endif

