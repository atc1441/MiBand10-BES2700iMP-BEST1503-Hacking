
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := \
	-Iutils/uarthci \
	-Iutils/cqueue \
	-Iplatform/drivers/codec  \
	-Iservices/multimedia/audio/process/floatlimiter/include \
	-Iplatform/drivers/ana \
	-Iservices/multimedia/audio/process/filters/cfg \
	-Iservices/multimedia/audio/process/filters/include

ifeq ($(DMA_RPC_CLI),1)
ccflags-y += -Iplatform/drivers/stream_dma_rpc
endif

ifneq ($(AF_STACK_SIZE),)
CFLAGS_audioflinger.o += -DAF_STACK_SIZE=$(AF_STACK_SIZE)
endif

ifeq ($(AUDIO_CODEC_ASYNC_CLOSE),1)
CFLAGS_audioflinger.o += -D__CODEC_ASYNC_CLOSE__
endif

ifeq ($(CODEC_PLAY_BEFORE_CAPTURE),1)
CFLAGS_audioflinger.o += -DCODEC_PLAY_BEFORE_CAPTURE
endif

ifeq ($(AUDIO_OUTPUT_SMALL_GAIN_ATTN),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_SMALL_GAIN_ATTN
endif

ifeq ($(AUDIO_OUTPUT_I2S_SW_GAIN),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_I2S_SW_GAIN
endif

ifeq ($(AUDIO_OUTPUT_SW_GAIN),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_SW_GAIN
endif

ifeq ($(AUDIO_OUTPUT_SW_GAIN_BEFORE_DRC),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_SW_GAIN_BEFORE_DRC
endif

ifeq ($(AUDIO_OUTPUT_DAC2_SW_GAIN),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_DAC2_SW_GAIN
endif

ifeq ($(AUDIO_OUTPUT_DAC3_SW_GAIN),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_DAC3_SW_GAIN
endif

ifeq ($(AUDIO_OUTPUT_DAC2),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_DAC2
endif

ifeq ($(AUDIO_OUTPUT_DAC3),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_DAC3
endif

ifeq ($(AUDIO_OUTPUT_SW_LIMITER),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_SW_LIMITER
endif

ifeq ($(AUDIO_OUTPUT_DAC2_SW_LIMITER),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_DAC2_SW_LIMITER
endif

ifeq ($(AUDIO_OUTPUT_DAC3_SW_LIMITER),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_DAC3_SW_LIMITER
endif

ifeq ($(ANC_APP),1)
CFLAGS_audioflinger.o += -DANC_APP
endif

ifeq ($(AUDIO_RESAMPLE),1)
CFLAGS_audioflinger.o += -D__AUDIO_RESAMPLE__
endif

ifeq ($(AUDIO_OUTPUT_DC_CALIB),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_DC_CALIB
endif
ifeq ($(AUDIO_OUTPUT_DC_CALIB_ANA),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_DC_CALIB_ANA
endif

ifeq ($(AUDIO_OUTPUT_SET_LARGE_ANA_DC),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_SET_LARGE_ANA_DC
endif

ifeq ($(AUDIO_OUTPUT_ASSERT_LARGE_DC),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_ASSERT_LARGE_DC
endif

ifeq ($(AUDIO_OUTPUT_USE_FIXED_DC),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_USE_FIXED_DC
endif

AUDIO_OUTPUT_ERROR_LARGE_DC ?= 1
ifeq ($(AUDIO_OUTPUT_ERROR_LARGE_DC),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_ERROR_LARGE_DC
endif

ifeq ($(AUDIO_OUTPUT_DIG_DC_DEEP_CALIB),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_DIG_DC_DEEP_CALIB
endif

ifeq ($(AUDIO_OUTPUT_SHORT_CALIB_TIME),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_SHORT_CALIB_TIME
endif

ifeq ($(AUDIO_OUTPUT_GAIN_M60DB_CHECK),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_GAIN_M60DB_CHECK
endif

ifeq ($(CODEC_DSD),1)
CFLAGS_audioflinger.o += -DCODEC_DSD
endif

ifeq ($(AUDIO_OUTPUT_INVERT_RIGHT_CHANNEL),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_INVERT_RIGHT_CHANNEL
endif

ifeq ($(AUDIO_OUTPUT_CALIB_GAIN_MISSMATCH),1)
CFLAGS_audioflinger.o += -DAUDIO_OUTPUT_CALIB_GAIN_MISSMATCH
endif

ifneq ($(CHIP_HAS_I2S),)
CFLAGS_audioflinger.o += -DCHIP_HAS_I2S=$(CHIP_HAS_I2S)
endif

ifneq ($(I2S_RESAMPLE),)
CFLAGS_audioflinger.o += -DI2S_RESAMPLE
endif

ifeq ($(DMA_RPC_CLI),1)
CFLAGS_audioflinger.o += -DDMA_RPC_CLI
endif

#-------------------------------
# Audioflinger device selection
#-------------------------------
# Internal codec default on
AF_DEVICE_INT_CODEC ?= 1

# BT PCM default on for BT applications
ifeq ($(RTOS),1)
AF_DEVICE_BT_PCM ?= 1
endif

# Individual device macro
ifeq ($(AF_DEVICE_EXT_CODEC),1)
CFLAGS_audioflinger.o += -DAF_DEVICE_EXT_CODEC
endif
ifeq ($(AF_DEVICE_I2S),1)
CFLAGS_audioflinger.o += -DAF_DEVICE_I2S
endif
ifeq ($(AF_DEVICE_TDM),1)
CFLAGS_audioflinger.o += -DAF_DEVICE_TDM
endif
ifeq ($(AF_DEVICE_INT_CODEC),1)
CFLAGS_audioflinger.o += -DAF_DEVICE_INT_CODEC
endif
ifeq ($(AF_DEVICE_SPDIF),1)
CFLAGS_audioflinger.o += -DAF_DEVICE_SPDIF
endif
ifeq ($(AF_DEVICE_BT_PCM),1)
CFLAGS_audioflinger.o += -DAF_DEVICE_BT_PCM
endif
ifeq ($(AF_DEVICE_DPD_RX),1)
CFLAGS_audioflinger.o += -DAF_DEVICE_DPD_RX
endif
ifeq ($(AUDIO_ANC_FB_MC),1)
CFLAGS_audioflinger.o += -DAF_DEVICE_MC
endif

#-------------------------------
# End
#-------------------------------

