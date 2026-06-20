cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))


obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	$(BT_SERVICE_UX_INCLUDES) \
	-Iplatform/drivers/bt  \
	-Iplatform/hal  \
	-Iservices/bt_app/a2dp_codecs/include 	\
	-Iutils/cqueue \
	-Iservices/audio_manager \
	-Iapps/audioplayers/a2dp_decoder \
    -Ibthost/service/ble_app_new/inc