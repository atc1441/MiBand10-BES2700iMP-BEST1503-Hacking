
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

mmlib_obj :=

mmlib_obj += algorithm/fft/src/

mmlib_obj += audio/process/adp/src/
mmlib_obj += audio/process/anc/src/
mmlib_obj += audio/process/psap/src/
mmlib_obj += audio/process/fir2iir/src/
mmlib_obj += audio/process/common/src/
mmlib_obj += audio/process/drc/src/
mmlib_obj += audio/process/filters/src/
mmlib_obj += audio/process/limiter/src/
mmlib_obj += audio/process/resample/src/
mmlib_obj += audio/process/integer_resampling/src/
mmlib_obj += audio/process/bes_fir_filter/src/
mmlib_obj += audio/process/floatlimiter/src/
mmlib_obj += audio/process/adj_mc/src/
mmlib_obj += audio/process/reverb/src/
mmlib_obj += audio/process/sidetone/src/
mmlib_obj += audio/process/sbcplc/src/
mmlib_obj += audio/process/dynamic_boost/src/
mmlib_obj += audio/process/dynamic_eq/src/
mmlib_obj += audio/process/bass_enhancer/src/
mmlib_obj += audio/process/virtual_surround_process/src/
mmlib_obj += audio/process/musicplc/

ifeq ($(FM_ENABLE),1)
mmlib_obj += fm/
endif

ifneq ($(NO_MULTIMEDIA_CODEC),1)
mmlib_obj += ../../multimedia/libs/
endif

#mmlib_obj += rbcodec/src/

ifeq ($(SPEECH_LIB),1)
ifneq ($(SPEECH_ARMCC_LIB),1)
ifneq ($(wildcard $(cur_dir)/speech/src/Makefile),)
mmlib_obj += speech/src/
endif
endif
endif

MULTIMEDIA_LIB_NAME := $(CHIP)_libmultimedia

ifeq ($(ROM_UTILS_ON),1)
MULTIMEDIA_LIB_NAME := $(MULTIMEDIA_LIB_NAME)_romaac
endif

ifeq ($(AAC_REDUCE_SIZE),1)
MULTIMEDIA_LIB_NAME := $(MULTIMEDIA_LIB_NAME)_rom_reduce_aac
endif

ifeq ($(A2DP_CP_ACCEL),1)
MULTIMEDIA_LIB_NAME := $(MULTIMEDIA_LIB_NAME)_cp
endif

ifeq ($(ANC_APP),1)
MULTIMEDIA_LIB_NAME := $(MULTIMEDIA_LIB_NAME)_anc
endif

ifeq ($(USE_TRACE_ID), 1)
MULTIMEDIA_LIB_NAME := $(MULTIMEDIA_LIB_NAME)_log_compressed
endif

ifeq ($(LC3PLUS_ENABLE),1)
MULTIMEDIA_LIB_NAME := $(MULTIMEDIA_LIB_NAME)_lc3plus
endif

ifeq ($(AUDIO_ANC_FIR_HW),1)
MULTIMEDIA_LIB_NAME := $(MULTIMEDIA_LIB_NAME)_fir_anc
endif

ifeq ($(AUDIO_SECTION_ENABLE), 1)
MULTIMEDIA_LIB_NAME := $(MULTIMEDIA_LIB_NAME)_eq
endif

ifeq ($(FREEMAN_ENABLED_STERO), 1)
MULTIMEDIA_LIB_NAME := $(MULTIMEDIA_LIB_NAME)_stero
endif

ifeq ($(CHIP_SUBSYS),sensor_hub)
MULTIMEDIA_LIB_NAME := $(MULTIMEDIA_LIB_NAME)_$(CHIP_SUBSYS)
endif

$(MULTIMEDIA_LIB_NAME)-y := $(mmlib_obj)

rel_src_obj :=
rel_src_obj += audio/process/anc/cfg/
rel_src_obj += audio/process/filters/cfg/
rel_src_obj += audio/process/resample/coef/
rel_src_obj += speech/armcc/

obj-y := $(MULTIMEDIA_LIB_NAME).a $(rel_src_obj)

ifeq ($(SPEECH_ARMCC_LIB),1)
obj-y += lib_speech_${CPU}.a
endif

subdir-ccflags-y += \
	-Iutils/heap \
	-Iinclude/cmsis_dsp \
	-Iinclude/cmsis_nn \
	-Iservices/multimedia/audio/process/filters/include \
	-I$(APPS_DIR)/common \

