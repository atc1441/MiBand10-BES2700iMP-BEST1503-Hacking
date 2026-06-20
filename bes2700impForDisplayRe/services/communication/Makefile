cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ifeq ($(USE_KNOWLES),1)
obj-y += comminication_knowles/
endif

ccflags-y += \
	-Iapps/common \
	-Iplatform/drivers/usb/usb_dev/inc \
	-Iplatform/drivers/ana \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	-Iservices/communication/comminication_knowles \
	-Ithirdparty/knowles_uart_lib \
	-Iutils/crc32 \
	-Iservices/app_ai/inc

ifeq ($(SINGLE_UART_IO_INSTEAD_ACON_INTR),1)
CFLAGS_communication_svr.o += -DSINGLE_UART_IO_INSTEAD_ACON_INTR
endif

