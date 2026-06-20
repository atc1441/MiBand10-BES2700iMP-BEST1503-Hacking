
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
    -Iapps/app_test/apptester \
    -Iapps/factory \
    -Iutils/crc32 \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/ana \
    -Iapps/audioplayers/rbplay \
    -Iservices/communication \
    -Iutils/cqueue \
    -Iutils/hsm \
    -Iservices/ai_voice/ama/ama_manager \
    -Iservices/ai_voice/manager \
    -Iservices/interconnection \
    -Iservices/walkie_talkie \
    -Iservices/walkie_talkie/wt_test \
    -Iservices/audio_bt \
    -Iservices/nv_section/factory_section \
    $(EPLAYER_INCLUDES) \
    $(ECOMM_INCLUDES) \
    $(EAUDIO_INCLUDES) \
    $(BT_SERVICE_INCLUDES)

