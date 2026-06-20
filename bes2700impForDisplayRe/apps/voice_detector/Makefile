cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

subdir-ccflags-y += \
	-Iservices/bt_app \
	-Iplatform/drivers/ana \
	-Iapps/voice_detector \
	-Iservices/audio_dump/include \
	-Ithirdparty/cyberon_lib/include \
	-Ithirdparty/cyberon_lib/src \
	-Iutils/hwtimer_list \
	-Iservices/multimedia/speech/inc

ifeq ($(VD_TEST),1)
#obj-y := voice_detector.o
ccflags-y += -Iapps/common -Iservices/audioflinger -DVD_TEST
ifneq ($(VD_TEST_TYPE),)
ccflags-y += -DVD_TEST_TYPE=$(VD_TEST_TYPE)
endif
ifneq ($(VD_TEST_TYPE),)
ccflags-y += -DVD_TEST_TYPE=$(VD_TEST_TYPE)
endif
ifneq ($(VD_TEST_DUMP_DATA),)
ccflags-y += -DVD_TEST_DUMP_DATA
endif
endif

ifeq ($(I2C_VAD),1)
ccflags-y += -DI2C_VAD
endif

