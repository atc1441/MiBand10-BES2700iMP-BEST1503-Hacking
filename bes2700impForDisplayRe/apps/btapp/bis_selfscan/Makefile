cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(BT_SERVICE_ENABLE), 0)
$(error Need BT_SERVICE_ENABLE=1)
endif

subdir-ccflags-y += \
	-Iapps/btapp/bis_selfscan \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
	$(BT_SERVICE_UX_INCLUDES) \
	-Iservices/ux/common/ibrt/ibrt_core/inc \
	-Iservices/lea_player/inc \
	-Iapps/bt_sync \
	-Iutils/cqueue \
	-Iplatform/drivers/bt

