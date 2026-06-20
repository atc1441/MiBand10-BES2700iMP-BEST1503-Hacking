cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))

ifeq ($(VAD_IF_TEST),1)
ccflags-y += -DVAD_IF_TEST
endif

ifeq ($(CORE_BRIDGE_DEMO_MSG),1)
CFLAGS_mcu_sensor_hub_app.o += -DCORE_BRIDGE_DEMO_MSG
endif

ifneq ($(APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE),)
CFLAGS_app_sensor_hub.o += -DAPP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE=$(APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE)
endif

ifneq ($(APP_CORE_BRIDGE_RX_BUFF_SIZE),)
CFLAGS_app_sensor_hub.o += -DAPP_CORE_BRIDGE_RX_BUFF_SIZE=$(APP_CORE_BRIDGE_RX_BUFF_SIZE)
endif

ifneq ($(APP_CORE_BRIDGE_MAX_XFER_DATA_SIZE),)
CFLAGS_app_sensor_hub.o += -DAPP_CORE_BRIDGE_MAX_XFER_DATA_SIZE=$(APP_CORE_BRIDGE_MAX_XFER_DATA_SIZE)
endif

ifneq ($(APP_CORE_BRIDGE_RX_THREAD_TMP_BUF_SIZE),)
CFLAGS_app_sensor_hub.o += -DAPP_CORE_BRIDGE_RX_THREAD_TMP_BUF_SIZE=$(APP_CORE_BRIDGE_RX_THREAD_TMP_BUF_SIZE)
endif

ifneq ($(APP_CORE_BRIDGE_TX_THREAD_STACK_SIZE),)
CFLAGS_app_sensor_hub.o += -DAPP_CORE_BRIDGE_TX_THREAD_STACK_SIZE=$(APP_CORE_BRIDGE_TX_THREAD_STACK_SIZE)
endif

ifneq ($(APP_CORE_BRIDGE_RX_THREAD_STACK_SIZE),)
CFLAGS_app_sensor_hub.o += -DAPP_CORE_BRIDGE_RX_THREAD_STACK_SIZE=$(APP_CORE_BRIDGE_RX_THREAD_STACK_SIZE)
endif

ifneq ($(SENSOR_HUB_BSP_TEST),)
ccflags-y += -DSENSOR_HUB_BSP_TEST
endif

ifeq ($(VPU_CFG_ON_SENSOR_HUB),1)
CFLAGS_mcu_sensor_hub_app.o += -DVPU_CFG_ON_SENSOR_HUB
endif

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y += \
	-Iplatform/drivers/sensor_hub \
	-Iutils/crc32 \
	-Iutils/cqueue \
	-Iutils/heap \
	-Iplatform/hal \
	-Iplatform/drivers/ana \
	-Itests/sensor_hub/inc \
	-Itests/sensor_hub/sensor_hub_ai \
	-Iapps/common \
	-Iapps/key \
	-Iservices/audioflinger \
	-Iservices/bt_app \
	-Iservices/bt_app/a2dp_codecs/include \
	-Iservices/audio_manager \
	$(BLUETOOTH_ADAPTER_INCLUDES)

