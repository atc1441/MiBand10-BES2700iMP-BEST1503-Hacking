cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

subdir-ccflags-y += \
    -Iplatform/hal \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/norflash \
    -Iservices/audio_bt \
    -Iservices/norflash_api \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BT_SERVICE_UX_INCLUDES) \
    -Iservices/ota \
    -Iapps/ota \
    -Iutils/crc32 \
    -Iutils/cqueue \
    -Iapps/common \
    -Iapps/key \
    -Iapps/main \
    -Iservices/audio_manager

