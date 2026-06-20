cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y += -Iutils/cqueue -Iutils/heap -Iplatform/hal \
	-Iplatform/drivers/ana \
	-Iapps/common \
	-Iservices/audioflinger \
	-Iservices/bt_app \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	-Iutils/crc32 \
	-Iplatform/drivers/dsp_m55


