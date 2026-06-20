
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

ifeq ($(A2DP_DECODER_VER),2)
obj_cpp := $(filter-out a2dpplay.cpp,$(obj_cpp))
obj_cpp += a2dp_decoder/
endif

ifeq ($(AUDIO_TRIGGER_VER),1)
obj_cpp += audio_trigger/version1/
else ifeq ($(AUDIO_TRIGGER_VER),2)
obj_cpp += audio_trigger/version2/
endif

bt_sco_chain_obj = bt_sco_chain.c bt_sco_chain_cp.c bt_sco_chain_cp_fifo.c bt_sco_chain_thirdparty.c bt_sco_chain_cp_thirdparty.c bt_sco_chain_reframe.c
bt_sco_chain_obj += bt_sco_chain_thirdparty_alango.c
bt_sco_chain_obj += bt_sco_chain_thirdparty_xiaomi.c bt_sco_chain_cp_xiaomi.c
bt_sco_chain_obj += bt_sco_chain_thirdparty_soundplus.c bt_sco_chain_cp_soundplus.c
obj_c := $(filter-out $(bt_sco_chain_obj),$(obj_c))

ifeq ($(SPEECH_TX_THIRDPARTY_ALANGO),1)
obj_c += bt_sco_chain_thirdparty_alango.c
else ifeq ($(SPEECH_TX_THIRDPARTY_XIAOMI),1)
obj_c += bt_sco_chain_thirdparty_xiaomi.c
obj_c += bt_sco_chain_cp_xiaomi.c
else ifeq ($(SPEECH_TX_THIRDPARTY_SNDP),1)
obj_c += bt_sco_chain_thirdparty_soundplus.c
obj_c += bt_sco_chain_cp_soundplus.c
else ifeq ($(SPEECH_TX_THIRDPARTY),1)
obj_c += bt_sco_chain_thirdparty.c
obj_c += bt_sco_chain_cp.c
# obj_c += bt_sco_chain_cp_thirdparty.c
# obj_c += bt_sco_chain_reframe.c
else
obj_c += bt_sco_chain.c
obj_c += bt_sco_chain_cp.c
endif

ifeq ($(SCO_CP_ACCEL_FIFO),1)
ifeq ($(findstring bt_sco_chain_cp.c,$(obj_c)),bt_sco_chain_cp.c)
obj_c := $(filter-out bt_sco_chain_cp.c,$(obj_c))
obj_c += bt_sco_chain_cp_fifo.c
CFLAGS_voicebtpcmplay_sco_dma_snapshot.o += -DSCO_CP_ACCEL_FIFO
CFLAGS_bt_sco_chain.o += -DSCO_CP_ACCEL_FIFO
endif
endif

ifeq ($(SCO_ADD_CUSTOMER_CODEC),0)
obj_c := $(filter-out bt_sco_codec_xxxx.c,$(obj_c))
endif

ifeq ($(SCO_DMA_SNAPSHOT),1)
obj_cpp := $(filter-out voicebtpcmplay.cpp,$(obj_cpp))
else
obj_cpp := $(filter-out voicebtpcmplay_sco_dma_snapshot.cpp,$(obj_cpp))
endif

ifeq ($(RB_CODEC),1)
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)rbplay/*.cpp))
endif

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(OPT_LEVEL),)
CFLAGS_a2dpplay.o += -O3
endif

ccflags-y := \
	-Iservices/audio_bt \
	-Iservices/audio_process \
	-Iapps/dsp_m55 \
	-Iutils/kfifo \
	-Iservices/bone_sensor \
	-Iservices/resources \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	$(BT_SERVICE_UX_INCLUDES) \
	-Iplatform/drivers/uarthci \
	-Iutils/cqueue \
	-Iservices/mcpp \
	-Iservices/audio_dump/include \
	-Iservices/multimedia/speech/inc \
	-Iservices/multimedia/rbcodec/inc \
	-Iservices/multimedia/audio/process/eq/include \
	-Iservices/multimedia/audio/process/resample/include \
	-Iservices/multimedia/audio/process/filters/include \
	-Iservices/multimedia/fm/inc \
	-Iservices/nv_section/aud_section \
	-Iservices/nv_section/include \
	-Iservices/overlay \
	-Iservices/norflash_api \
	-Iservices/nv_section/log_section \
	-Iapps/main \
	-Iapps/audioplayers/rbplay/ \
	-Iapps/audioplayers/a2dp_decoder \
	-Iutils/list \
	-Iutils/heap \
	-Iplatform/drivers/ana \
	-Ithirdparty/audio_codec_lib/scalable/ \
	-Ithirdparty/audio_codec_lib/liblhdc-dec/inc \
	-Iapps/app_test/apptester \
	-Iapps/key  \
	-Iplatform/drivers/bt \
	-Iapps/anc/inc \
	-Iservices/bt_app/a2dp_codecs/include \
	-Iservices/app_ai/inc\
	-Iinclude/cmsis_dsp/ \
	-Iapps/app_rpc/ \
	-Iapps/app_rpc/rpc_speech_dsp/ \
	-Ithirdparty/tflite \
	-Ithirdparty/tflite/tensorflow/lite/micro/src/dtln_denoise/ \
	-Iservices/audio_manager \
	-Iservices/audio_manager/audio_policy/bredr_policy/inc \
	-Iapps/sensorhub

ccflags-y += $(BES_MULTIMEDIA_INCLUDES)

ifeq ($(A2DP_LDAC_ON),1)
ccflags-y += -Ithirdparty/audio_codec_lib/ldac/inc
endif

ifeq ($(A2DP_LHDCV5_ON),1)
ccflags-y += -Ithirdparty/audio_codec_lib/liblhdcv5-dec/inc
endif

ifeq ($(A2DP_CP_ACCEL),1)
ccflags-y += -Iplatform/drivers/cp_accel
else ifeq ($(SCO_CP_ACCEL),1)
ccflags-y += -Iplatform/drivers/cp_accel
endif

ifeq ($(APP_TEST_AUDIO),1)
CFLAGS_app_audio.o += -DAPP_TEST_AUDIO
endif

ifeq ($(AUDIO_RESAMPLE),1)
CFLAGS_a2dpplay.o += -D__AUDIO_RESAMPLE__
CFLAGS_voicebtpcmplay.o += -D__AUDIO_RESAMPLE__
CFLAGS_voicebtpcmplay_sco_dma_snapshot.o += -D__AUDIO_RESAMPLE__
CFLAGS_bt_sco_chain.o += -D__AUDIO_RESAMPLE__
endif

ifeq ($(SW_PLAYBACK_RESAMPLE),1)
CFLAGS_a2dpplay.o += -DSW_PLAYBACK_RESAMPLE
endif

ifeq ($(RESAMPLE_ANY_SAMPLE_RATE),1)
CFLAGS_a2dpplay.o += -DRESAMPLE_ANY_SAMPLE_RATE
endif

ifeq ($(SW_SCO_RESAMPLE),1)
CFLAGS_voicebtpcmplay.o += -DSW_SCO_RESAMPLE
CFLAGS_voicebtpcmplay_sco_dma_snapshot.o += -DSW_SCO_RESAMPLE
endif

ifeq ($(SPEECH_TX_THIRDPARTY_ALANGO),1)
CFLAGS_bt_sco_chain_thirdparty_alango.o += -Ithirdparty/alango_lib/include
endif

ifeq ($(SPEECH_TX_THIRDPARTY_XIAOMI),1)
CFLAGS_bt_sco_chain_thirdparty_xiaomi.o += -Ithirdparty/xiaomi_enc
endif

ifeq ($(SPEECH_TX_THIRDPARTY_SNDP),1)
CFLAGS_bt_sco_chain_thirdparty_soundplus.o += -Ithirdparty/$(THIRDPARTY_LIB)_lib/sp_ec_mv03
CFLAGS_bt_sco_chain_cp_soundplus.o += -Ithirdparty/$(THIRDPARTY_LIB)_lib/sp_ec_mv03
endif

ifeq ($(VOICE_PROMPT),1)
CFLAGS_app_audio.o += -DMEDIA_PLAYER_SUPPORT
endif

ifeq ($(AUDIO_QUEUE_SUPPORT),1)
CFLAGS_app_audio.o += -D__AUDIO_QUEUE_SUPPORT__
endif

ifeq ($(ANC_APP),1)
CFLAGS_app_audio.o += -DANC_APP
endif

ifeq ($(A2DP_EQ_24BIT),1)
CFLAGS_app_audio.o += -DA2DP_EQ_24BIT
CFLAGS_a2dpplay.o += -DA2DP_EQ_24BIT
endif

ifeq ($(A2DP_TRACE_CP_ACCEL),1)
CFLAGS_a2dpplay.o += -DA2DP_TRACE_CP_ACCEL
endif

ifeq ($(A2DP_TRACE_DEC_TIME),1)
CFLAGS_a2dpplay.o += -DA2DP_TRACE_DEC_TIME
endif

ifeq ($(A2DP_TRACE_CP_DEC_TIME),1)
CFLAGS_a2dpplay.o += -DA2DP_TRACE_CP_DEC_TIME
endif

ifeq ($(SPEECH_TX_AEC_CODEC_REF),1)
CFLAGS_voicebtpcmplay.o += -DSPEECH_TX_AEC_CODEC_REF
CFLAGS_voicebtpcmplay_sco_dma_snapshot.o += -DSPEECH_TX_AEC_CODEC_REF
CFLAGS_bt_sco_chain_cfg_default.o += -DSPEECH_TX_AEC_CODEC_REF
endif

ifeq ($(SPEECH_RX_24BIT),1)
CFLAGS_bt_sco_chain.o += -DSPEECH_RX_24BIT
CFLAGS_bt_sco_chain_thirdparty.o += -DSPEECH_RX_24BIT
CFLAGS_voicebtpcmplay.o += -DSPEECH_RX_24BIT
CFLAGS_voicebtpcmplay_sco_dma_snapshot.o += -DSPEECH_RX_24BIT
endif

ifeq ($(SPEECH_RX_COMPEXP),1)
CFLAGS_bt_sco_chain.o += -DSPEECH_RX_COMPEXP
CFLAGS_bt_sco_chain_thirdparty.o += -DSPEECH_RX_COMPEXP
CFLAGS_bt_sco_chain_cfg_default.o += -DSPEECH_RX_COMPEXP
CFLAGS_bt_sco_chain_tuning.o += -DSPEECH_RX_COMPEXP
endif

ifeq ($(SPEECH_RX_HW_EQ),1)
CFLAGS_bt_sco_chain.o += -DSPEECH_RX_HW_EQ
CFLAGS_bt_sco_chain_thirdparty.o += -DSPEECH_RX_HW_EQ
CFLAGS_bt_sco_chain_cfg_default.o += -DSPEECH_RX_HW_EQ
CFLAGS_bt_sco_chain_tuning.o += -DSPEECH_RX_HW_EQ
endif

ifeq ($(SPEECH_RX_DAC_EQ),1)
CFLAGS_bt_sco_chain.o += -DSPEECH_RX_DAC_EQ
CFLAGS_bt_sco_chain_thirdparty.o += -DSPEECH_RX_DAC_EQ
CFLAGS_bt_sco_chain_cfg_default.o += -DSPEECH_RX_DAC_EQ
CFLAGS_bt_sco_chain_tuning.o += -DSPEECH_RX_DAC_EQ
endif

ifeq ($(SPEECH_TX_2MIC_SWAP_CHANNELS),1)
CFLAGS_bt_sco_chain.o += -DSPEECH_TX_2MIC_SWAP_CHANNELS
endif

ifeq ($(AI_VOICE),1)
ccflags-y += -Iservices/ai_voice/audio
endif

ifeq ($(CALL_BYPASS_SLAVE_TX_PROCESS),1)
CFLAGS_voicebtpcmplay_sco_dma_snapshot.o += -DCALL_BYPASS_SLAVE_TX_PROCESS
endif

ifeq ($(SCO_CP_ACCEL_RPC),1)
subdir-ccflags-y += \
    -Iapps/audioplayers/speech_sco_chain_3rd
obj-y += speech_sco_chain_3rd/
endif

ifeq ($(DSP_HIFI4_TRC_TO_MCU),1)
TRC_FLAGS := -DDSP_HIFI4_TRC_TO_MCU
ifeq ($(RMT_TRC_IN_MSG_CHAN),1)
TRC_FLAGS += -DRMT_TRC_IN_MSG_CHAN
endif
endif

ifeq ($(BT_SCO_CHAIN_AUDIO_DUMP),1)
CFLAGS_bt_sco_chain.o += -DBT_SCO_CHAIN_AUDIO_DUMP
endif

ifeq ($(A2DP_DECODER_CROSS_CORE_USE_M55),1)
ccflags-y += \
    -Iapps/dsp_m55
endif
