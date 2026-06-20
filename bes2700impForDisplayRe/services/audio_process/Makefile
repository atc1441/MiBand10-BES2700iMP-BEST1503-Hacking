
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)
ifeq ($(DIRAC_AUDIO_ENABLE),1)
obj-y += libdirac/
endif
ccflags-y := \
	-Iservices/multimedia/audio/process/filters/include \
	-Iservices/multimedia/audio/process/common/include \
	-Iservices/multimedia/audio/process/drc/include \
	-Iservices/multimedia/audio/process/limiter/include \
	-Iservices/multimedia/audio/process/reverb/include \
	-Iservices/multimedia/audio/process/dynamic_eq/include \
	-Iservices/multimedia/audio/process/bass_enhancer/include \
	-Iservices/multimedia/audio/process/virtual_surround_process/include \
	-Iservices/multimedia/speech/inc \
	-Iplatform/drivers/bt \
	-Iservices/bt_app/a2dp_codecs/include \
	-Iutils/cqueue \
	-Iutils/crc32 \
	-Iutils/heap \
	-Iapps/anc/inc \
	-Iapps/key \
	-Iapps/earbuds/conn \
	-Iapps/audioplayers \
	-Iservices/bt_app \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	$(BT_SERVICE_UX_INCLUDES) \
	-Iservices/nv_section/aud_section \
	-Iservices/nv_section/include \
	-Iservices/hw_dsp/inc \
	-Iservices/audio_dump/include \
	-Iservices/audioflinger \
	-Iservices/audio_manager \
	-Iinclude/cmsis_dsp \
	-Iservices/norflash_api \
	-Iutils/heap \
	-Iapps/common/ \
	-Iapps/audioplayers/a2dp_decoder/ \
	-Iservices/audio_bt/ \
	-Ibthost/service/ble_app_new/inc \
	-Imultimedia/inc/audio/process/psap/include \
	-Iservices/multimedia/audio/process/dynamic_boost/include
	
ifeq ($(DIRAC_AUDIO_ENABLE),1)
ccflags-y += \
	-Iservices/ibrt_core/inc \
	-Iservices/bt_if_enhanced/inc \
	$(BES_MULTIMEDIA_INCLUDES) \
	-Iutils/cqueue \
	-Iplatform/drivers/bt \
	-Iservices/audio_process/libdirac/inc
endif

ifneq ($(AUDIO_DSP_ACCEL),)
subdir-ccflags-y += \
	-Iservices/mcpp \
	-Iutils/kfifo 
endif

ifeq ($(HW_FIR_EQ_PROCESS),1)
ccflags-y += -D__HW_FIR_EQ_PROCESS__
endif

ifeq ($(HW_IIR_EQ_PROCESS),1)
ccflags-y += -D__HW_IIR_EQ_PROCESS__
endif

ifeq ($(SW_IIR_EQ_PROCESS),1)
ccflags-y += -D__SW_IIR_EQ_PROCESS__
endif

ifeq ($(AUDIO_DRC),1)
ccflags-y += -D__AUDIO_DRC__
endif

ifeq ($(AUDIO_LIMITER),1)
ccflags-y += -D__AUDIO_LIMITER__
endif

ifeq ($(AUDIO_REVERB),1)
ccflags-y += -D__AUDIO_REVERB__
endif

ifeq ($(AUDIO_BASS_ENHANCER),1)
ccflags-y += -D__AUDIO_BASS_ENHANCER__
endif

ifeq ($(AUDIO_EQ_DYNAMICS_MEM),1)
ccflags-y += -D__AUDIO_EQ_DYNAMICS_MEM__
endif

ifeq ($(AUDIO_RESAMPLE),1)
ccflags-y += -D__AUDIO_RESAMPLE__
endif

ifeq ($(AUDIO_HW_LIMITER),1)
ccflags-y += -D__AUDIO_HW_LIMITER__
endif

ifeq ($(AUDIO_ADJ_EQ),1)
ccflags-y += -D__AUDIO_ADJ_EQ__
endif

ifeq ($(HW_DAC_IIR_EQ_PROCESS),1)
ccflags-y += -D__HW_DAC_IIR_EQ_PROCESS__
ifneq ($(filter best2300p best1400 best3001 best2300 best2000 best1000,$(CHIP)),)
ifneq ($(CHIP_SUBTYPE),best1402)
ifeq ($(ANC_APP),1)
$(error "error: ANC and Hardware codec iir can't be used in the same")
endif
endif
endif
endif

ifeq ($(FIR_HIGHSPEED),1)
ccflags-y += -DFIR_HIGHSPEED
endif

ifeq ($(PC_CMD_UART),1)
CFLAGS_audio_process.o += -D__PC_CMD_UART__
CFLAGS_audio_process.o += -DAUDIO_EQ_TUNING
endif

ifeq ($(TOTA_EQ_TUNING), 1)
ccflags-y += -DTOTA_EQ_TUNING
CFLAGS_audio_process.o += -DAUDIO_EQ_TUNING
endif

ifeq ($(USB_EQ_TUNING), 1)
ccflags-y += -DUSB_EQ_TUNING
CFLAGS_audio_process.o += -DAUDIO_EQ_TUNING
endif

ifeq ($(USB_AUDIO_APP), 1)
CFLAGS_audio_process.o += -DUSB_AUDIO_APP
endif

ifeq ($(AUDIO_ADAPTIVE_IIR_EQ),1)
CFLAGS_audio_process.o += -DAUDIO_ADAPTIVE_IIR_EQ
CFLAGS_audio_process.o += -DAUDIO_ADAPTIVE_EQ
endif

ifeq ($(AUDIO_ADAPTIVE_FIR_EQ),1)
CFLAGS_audio_process.o += -DAUDIO_ADAPTIVE_FIR_EQ
CFLAGS_audio_process.o += -DAUDIO_ADAPTIVE_EQ
endif

ifeq ($(SPEECH_LIB),1)
CFLAGS_audio_spectrum.o += -DSPEECH_LIB
endif

CFLAGS_audio_spectrum.o += -DUSE_MED_MALLOC

ifeq ($(AUDIO_OUTPUT_SW_GAIN),1)
CFLAGS_audio_process.o += -DAUDIO_OUTPUT_SW_GAIN
endif

ifeq ($(AUDIO_OUTPUT_SW_GAIN_BEFORE_DRC),1)
CFLAGS_audio_process.o += -DAUDIO_OUTPUT_SW_GAIN_BEFORE_DRC
endif

ifeq ($(AUDIO_CUSTOM_EQ),1)
CFLAGS_audio_process.o += -DAUDIO_CUSTOM_EQ
endif

ifeq ($(AUDIO_DYNAMIC_BOOST),1)
CFLAGS_audio_process.o += -D__AUDIO_DYNAMIC_BOOST__
endif

ifeq ($(SPA_AUDIO_ENABLE),1)
ccflags-y += -Ithirdparty/userapi/spa_dummy_app/inc
ifeq ($(SPA_AUDIO_SEC),1)
ccflags-y += -Ithirdparty/userapi/spa_dummy_app/sec
endif
endif

ifeq ($(AUDIO_DYNAMIC_EQ),1)
CFLAGS_audio_process.o += -D__AUDIO_DYNAMIC_EQ__
endif

