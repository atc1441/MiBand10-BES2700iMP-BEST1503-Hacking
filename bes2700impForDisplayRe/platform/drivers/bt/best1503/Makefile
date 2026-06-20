cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

CFLAGS_bt_drv_uart_bridge_intsys.o += $(BT_DRV_UART_BRIDGE_FLAGS)

CFLAGS_bt_drv.o += $(BT_DRV_FLAGS)

CFLAGS_bt_drv_patch.o += $(BT_DRV_PATCH_FLAGS)

CFLAGS_bt_drv_config.o += $(BT_DRV_CONFIG_FLAGS)

RX_IQ_CAL ?= 0
ifeq ($(RX_IQ_CAL),1)
subdir-ccflags-y += -DRX_IQ_CAL
endif

subdir-ccflags-y += -Iutils/crash_catcher/include
subdir-ccflags-y += -Iservices/nv_section/userdata_section
subdir-ccflags-y += -Iutils/hwtimer_list
ccflags-y += $(BLUETOOTH_ADAPTER_INCLUDES)


BLE_EXT_ADV_TX_PWR_INDEPEND ?= 0
ifeq ($(BLE_EXT_ADV_TX_PWR_INDEPEND),1)
ccflags-y += -DBLE_EXT_ADV_TX_PWR_INDEPEND
endif


BT_WATCH_PATCH ?= 0
ifeq ($(BT_WATCH_PATCH),1)
ccflags-y += -DBT_WATCH_PATCH
endif

BT_STEREO_PATCH ?= 0
ifeq ($(BT_STEREO_PATCH),1)
ccflags-y += -DBT_STEREO_PATCH
endif

PMU_HIGH_VPA ?= 0
ifeq ($(PMU_HIGH_VPA),1)
ccflags-y += -DPMU_HIGH_VPA
endif

RF_TX_LOW_POWER_MODE ?= 0
ifeq ($(RF_TX_LOW_POWER_MODE),1)
ccflags-y += -DRF_TX_LOW_POWER_MODE
endif

RF_RX_LOW_POWER_MODE ?= 0
ifeq ($(RF_RX_LOW_POWER_MODE),1)
ccflags-y += -DRF_RX_LOW_POWER_MODE
endif

REDUCE_EDGE_CHL_TXPWR ?= 0
ifeq ($(REDUCE_EDGE_CHL_TXPWR),1)
ccflags-y += -DREDUCE_EDGE_CHL_TXPWR
endif

ifeq ($(BT_RF_PREFER), 3M)
ifeq ($(NEW_SWAGC_MODE),1)
ifeq ($(BT_SYSTEM_52M),0)
$(error BT SYSTEM 24M do not support $(BT_RF_PREFER))
endif
endif
endif