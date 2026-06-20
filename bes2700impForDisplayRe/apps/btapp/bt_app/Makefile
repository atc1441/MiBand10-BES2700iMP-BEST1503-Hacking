cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

subdir-ccflags-y += \
	-Iapps/btapp/bis_selfscan \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
	$(BT_SERVICE_UX_INCLUDES) \
	-Iservices/lea_player/inc \
	-Iservices/audio_bt/ \
	-Iservices/audio_manager/ \
	-Iservices/audio_manager/audio_policy/bredr_policy/inc \
	-Ibthost/service/bt_app/inc \
	-Iapps/bt_sync \
	-Iapps/key \
	-Iutils/cqueue \
	-Iplatform/drivers/bt

