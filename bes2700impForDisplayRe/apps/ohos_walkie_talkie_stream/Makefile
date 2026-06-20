
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))


obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y +=  \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    -Iservices/audioflinger \
    -Iservices/overlay \
    -Iservices/resources \
    -Iservices/audio_process \
    -Iutils/crc32 \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/ana \
    -Iapps/audioplayers/rbplay \
    -Iservices/communication \
    -Iutils/cqueue \
    -Iutils/kfifo \
    -Iservices/ai_voice/ama/ama_manager \
    -Iservices/ai_voice/manager \
    -Iservices/interconnection \
    -Iservices/walkie_talkie \
    -Iservices/audio_bt \
    -Iservices/nv_section/factory_section \
    -Iapps/bth_if \
    -Iapps/bth_if/bt_if_client \
    $(EPLAYER_INCLUDES) \
    $(ECOMM_INCLUDES) \
    $(EAUDIO_INCLUDES) \
    $(BT_SERVICE_INCLUDES)

