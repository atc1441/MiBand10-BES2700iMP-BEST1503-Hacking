cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y += \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	$(BT_SERVICE_UX_INCLUDES) \
	-Iapps/audioplayers/a2dp_decoder \
	-Iservices/bt_app \
	-Iservices/audio_bt \
	-Iservices/resources \
	-Iplatform/drivers/bt \
	-Iutils/cqueue \
	-Iutils/list \
	-Iservices/audio_manager/audio_policy/bredr_policy/inc \
	-Iservices/audio_manager

ifeq ($(A2DP_LDAC_ON),1)
ifeq ($(A2DP_LDAC_BCO),1)
ccflags-y += -DA2DP_LDAC_BCO
endif
endif