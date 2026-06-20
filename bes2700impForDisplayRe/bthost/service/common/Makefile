
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(AUDIO_TRIGGER_VER),1)
ccflags-y +=   \
		-Iapps/audioplayers/audio_trigger/version1
endif

ccflags-y += -DBLUETOOTH_BT_IMPL

ccflags-y += \
	-I$(BT_ROM_PORTING_PATH) \
	$(NEAR_STACK_INCLUDES) \
	$(BES_MULTIMEDIA_INCLUDES) \
	-Iinclude/cmsis_dsp \
	-Iservices/voice_dev \
	-Iservices/audio_bt \
	-Iservices/audio_process \
	-Iservices/audio_dump/include \
	-Iservices/hw_dsp/inc \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	$(BES_BT_IMPL_INCLUDES) \
	$(BT_SERVICE_UX_INCLUDES) \
	-I$(BLE_APP_DIR_PATH)/inc \
	-Ibthost/stack/common \
	-Imultimedia/inc/speech/inc \
	-Iservices/multimedia/speech/inc \
	-Iservices/bone_sensor \
	-Iservices/overlay \
	-Iservices/gfps/inc \
	-Ithirdparty/tile/tile_common/tile_storage \
	-Iservices/resources \
	-Iservices/bes_bth/bt/bt_if/inc \
	-Imultimedia/inc/rbcodec \
	-Imultimedia/inc/audio/codec/sbc/inc \
	-Imultimedia/inc/audio/process/resample/include \
	-Imultimedia/inc/audio/process/filters/include \
	-Imultimedia/inc/audio/process/drc/include \
	-Imultimedia/inc/audio/process/anc/include\
	-Imultimedia/inc/audio/process/sidetone/include\
	-Imultimedia/inc/audio/process/adj_mc/inc\
	-Iservices/multimedia/rbcodec \
	-Iservices/multimedia/audio/process/resample/include \
	-Iservices/multimedia/audio/process/filters/include \
	-Iservices/multimedia/audio/process/drc/include \
	-Iservices/multimedia/audio/process/anc/include\
	-Iservices/multimedia/audio/process/sidetone/include\
	-Iservices/multimedia/audio/process/adj_mc/inc\
	-Iservices/nv_section/aud_section \
	-Iservices/app_debug_info_system/inc \
	-Iservices/lea_player/inc \
	-Iservices/nv_section/userdata_section \
	-Iservices/nv_section/include \
	-Iservices/voicepath/$(VOICE_DATAPATH_TYPE) \
	-Iservices/voicepath/gsound/gsound_target \
	-Iservices/voicepath/gsound/gsound_custom/inc \
	-Iservices/voicepath/gsound/gsound_target_api_read_only \
	-Iservices/ai_voice/protocol/gva/gsound_target \
	-Iservices/ai_voice/protocol/gva/gsound_custom/inc \
	-Iservices/ai_voice/protocol/gva/gsound_target_api_read_only \
	-Iplatform/drivers/uarthci \
	-Iplatform/drivers/ana \
	-Iplatform/cmsis \
	-Iplatform/drivers/bt \
	-Iutils \
	-Iutils/cqueue \
	-Iutils/heap \
	-Iutils/crc16 \
	-Iservices/audioflinger \
	-Iutils/lockcqueue \
	-Iutils/intersyshci \
	-Iapps/anc/inc \
	-Iapps/key \
	-Iapps/main \
	-Iapps/common/hci_bridge \
	-Iapps/common \
	-Iapps/audioplayers \
	-Iapps/audioplayers/a2dp_decoder \
	-Iapps/battery \
	-Iapps/btapp/bt_app/ \
	-Iapps/factory \
	-Iapps/voice_assist/inc \
	-Iutils/hwtimer_list \
	-Iservices/voicepath \
	-Ithirdparty/userapi \
	-Ithirdparty/audio_codec_lib/liblhdc-dec/inc \
	-Iservices/ai_voice/manager \
	-Iservices/ai_voice/audio \
	-Iservices/ai_voice/transport \
	-Iservices/aiot/coex \
	-Iservices/app_ai/inc \
	-Iservices/interconnection/red \
	-Iservices/interconnection/green \
	-Iservices/interconnection/umm_malloc \
	-Iservices/custom_api \
	-Ithirdparty/tile/tile_target \
	-Iservices/osif \
	-Iutils/list \
	-Iutils/string \
	-Iservices/ai_voice/protocol/bixbyvoice \
	-Iservices/ai_voice/protocol/bixbyvoice/bixbyvoice_manager \
	-Iservices/audio_manager \
	-Iservices/audio_manager/audio_policy/local_audio \
	-Iservices/bt_watch_service/inc \
	-Iapps/earbuds/conn \
	-Iservices/ble_stack/profiles/gfps/api

ifeq ($(GATT_OVER_BR_EDR),1)
ccflags-y += -Iservices/ble_stack/hl/src/l2c/l2cm
endif

ifeq ($(ANC_NOISE_TRACKER),1)
ccflags-y += \
	-Ithirdparty/noise_tracker_lib \
	-Ithirdparty/userapi/noise_tracker_app
endif

ifeq ($(BTUSB_AUDIO_MODE),1)
ccflags-y += -Iapps/btusbaudio \
	-Iapps/usbaudio/bt_usb_audio
endif

ifeq ($(BT_USB_AUDIO_DUAL_MODE),1)
ccflags-y += -Iapps/btusbaudio \
	-Iapps/usbaudio/bt_usb_audio
ccflags-y += $(BLUETOOTH_ADAPTER_INCLUDES)
endif

ifeq ($(INTERCONNECTION),1)
ccflags-y += $(BLUETOOTH_ADAPTER_INCLUDES)
endif

ifeq ($(INTERACTION),1)
ccflags-y += $(BLUETOOTH_ADAPTER_INCLUDES)
endif

ifeq ($(A2DP_LDAC_ON),1)
ccflags-y += -Ithirdparty/audio_codec_lib/ldac/inc
endif

ifeq ($(A2DP_LHDCV5_ON),1)
ccflags-y += -Ithirdparty/audio_codec_lib/liblhdcv5-dec/inc
endif
ifeq ($(RB_CODEC),1)
ccflags-y += -DRB_CODEC
endif

ifeq ($(TEST_OVER_THE_AIR),1)
ccflags-y += -Iservices/tota
endif

ifeq ($(TEST_OVER_THE_AIR_v2),1)
ccflags-y += -Iservices/tota_v2
endif

ifeq ($(ANC_ASSIST_ENABLED),1)
subdir-ccflags-y += \
	-Iapps/anc_assist/inc
endif

ifeq ($(SW_IIR_PROMPT_EQ_PROCESS),1)
CFLAGS_app_media_player.o += -D__SW_IIR_PROMPT_EQ_PROCESS__
endif

ifeq ($(AUDIO_QUEUE_SUPPORT),1)
CFLAGS_app_bt_media_manager.o += -D__AUDIO_QUEUE_SUPPORT__
endif

CFLAGS_besmain.o += -DBUILD_HOSTNAME=$(BUILD_HOSTNAME) -DBUILD_USERNAME=$(BUILD_USERNAME)

ifeq ($(SPEECH_TX_AEC),1)
CFLAGS_app_bt_stream.o += -DSPEECH_TX_AEC
CFLAGS_app_hfp.o += -DSPEECH_TX_AEC
endif

ifeq ($(SPEECH_TX_NS),1)
CFLAGS_app_bt_stream.o += -DSPEECH_TX_NS
CFLAGS_app_hfp.o += -DSPEECH_TX_NS
endif

ifeq ($(AUDIO_INPUT_MONO),1)
CFLAGS_app_bt_stream.o += -D__AUDIO_INPUT_MONO_MODE__
endif

ifeq ($(AUDIO_OUTPUT_MONO),1)
CFLAGS_app_bt_stream.o += -D__AUDIO_OUTPUT_MONO_MODE__
endif

ifeq ($(CODEC_DAC_MULTI_VOLUME_TABLE),1)
CFLAGS_app_bt_stream.o += -DCODEC_DAC_MULTI_VOLUME_TABLE
endif

ifeq ($(AUDIO_ADAPTIVE_IIR_EQ),1)
CFLAGS_app_bt_stream.o += -DAUDIO_ADAPTIVE_IIR_EQ
CFLAGS_app_bt_stream.o += -DAUDIO_ADAPTIVE_EQ
endif

ifeq ($(AUDIO_ADAPTIVE_FIR_EQ),1)
CFLAGS_app_bt_stream.o += -DAUDIO_ADAPTIVE_FIR_EQ
CFLAGS_app_bt_stream.o += -DAUDIO_ADAPTIVE_EQ
endif

ifeq ($(AUDIO_OUTPUT_SW_GAIN),1)
CFLAGS_app_bt_stream.o += -DAUDIO_OUTPUT_SW_GAIN
endif

ifeq ($(AUDIO_RESAMPLE),1)
CFLAGS_app_bt_stream.o += -D__AUDIO_RESAMPLE__
endif
ifeq ($(SW_PLAYBACK_RESAMPLE),1)
CFLAGS_app_bt_stream.o += -DSW_PLAYBACK_RESAMPLE
endif
ifeq ($(SW_CAPTURE_RESAMPLE),1)
CFLAGS_app_bt_stream.o += -DSW_CAPTURE_RESAMPLE
endif
ifeq ($(SW_SCO_RESAMPLE),1)
CFLAGS_app_bt_stream.o += -DSW_SCO_RESAMPLE
endif
ifeq ($(NO_SCO_RESAMPLE),1)
CFLAGS_app_bt_stream.o += -DNO_SCO_RESAMPLE
endif
ifeq ($(RESAMPLE_ANY_SAMPLE_RATE),1)
CFLAGS_app_bt_stream.o += -DRESAMPLE_ANY_SAMPLE_RATE
endif

ifeq ($(BT_XTAL_SYNC),1)
CFLAGS_app_bt_stream.o += -DBT_XTAL_SYNC
endif

ifeq ($(AUDIO_SPECTRUM),1)
CFLAGS_app_bt_stream.o += -D__AUDIO_SPECTRUM__
endif

ifeq ($(HW_FIR_EQ_PROCESS),1)
CFLAGS_app_bt_stream.o += -D__HW_FIR_EQ_PROCESS__
endif

ifeq ($(HW_IIR_EQ_PROCESS),1)
CFLAGS_app_bt_stream.o += -D__HW_IIR_EQ_PROCESS__
endif

ifeq ($(SW_IIR_EQ_PROCESS),1)
CFLAGS_app_bt_stream.o += -D__SW_IIR_EQ_PROCESS__
endif

ifeq ($(HW_DAC_IIR_EQ_PROCESS),1)
CFLAGS_app_bt_stream.o += -D__HW_DAC_IIR_EQ_PROCESS__
endif

ifeq ($(TOTA_EQ_TUNING), 1)
CFLAGS_app_bt_stream.o += -DAUDIO_EQ_TUNING
endif

ifeq ($(ANC_APP),1)
CFLAGS_app_bt_stream.o += -DANC_APP
CFLAGS_app_media_player.o += -DANC_APP
endif

ifeq ($(ANC_ADPT_ENABLED),1)
CFLAGS_app_bt_stream.o += -DANC_ADPT_ENABLED
endif

MUSIC_DELAY_CONTROL ?= 1

ifeq ($(MUSIC_DELAY_CONTROL),1)
CFLAGS_app_bt_stream.o += -DMUSIC_DELAY_CONTROL
endif

ifeq ($(A2DP_EQ_24BIT),1)
CFLAGS_app_bt_stream.o += -DA2DP_EQ_24BIT
CFLAGS_app_ring_merge.o += -DA2DP_EQ_24BIT
endif

ifeq ($(APP_MUSIC_26M),1)
CFLAGS_app_bt_stream.o += -DAPP_MUSIC_26M
endif

ifeq ($(AUDIO_DRC),1)
ccflags-y += -D__AUDIO_DRC__
endif

ifeq ($(AUDIO_LIMITER),1)
ccflags-y += -D__AUDIO_LIMITER__
endif

ifeq ($(SPEECH_TX_AEC_CODEC_REF),1)
CFLAGS_app_bt_stream.o += -DSPEECH_TX_AEC_CODEC_REF
endif

ifeq ($(SPEECH_RX_24BIT),1)
CFLAGS_app_bt_stream.o += -DSPEECH_RX_24BIT
endif

ifeq ($(ANC_NOISE_TRACKER),1)
CFLAGS_app_bt_stream.o += -DANC_NOISE_TRACKER
endif

ifeq ($(HFP_DISABLE_NREC),1)
CFLAGS_app_hfp.o += -DHFP_DISABLE_NREC
endif

ifeq ($(MEDIA_PLAY_24BIT),1)
CFLAGS_app_media_player.o += -DMEDIA_PLAY_24BIT
endif

ifeq ($(BT_TEST_CURRENT_KEY),1)
CFLAGS_app_keyhandle.o += -DBT_TEST_CURRENT_KEY
endif

ifneq ($(filter 8000 16000,$(SPEECH_VQE_FIXED_SAMPLE_RATE)),)
CFLAGS_app_bt_stream.o += -DSPEECH_VQE_FIXED_SAMPLE_RATE=$(SPEECH_VQE_FIXED_SAMPLE_RATE)
endif

ifeq ($(AUDIO_PROMPT_USE_DAC2ORDAC3_ENABLED),1)
CFLAGS_app_bt_media_manager.o += -DAUDIO_PROMPT_USE_DAC2ORDAC3_ENABLED
CFLAGS_app_bt_stream.o += -DAUDIO_PROMPT_USE_DAC2ORDAC3_ENABLED
CFLAGS_app_media_player.o += -DAUDIO_PROMPT_USE_DAC2ORDAC3_ENABLED
CFLAGS_audio_policy.o += -DAUDIO_PROMPT_USE_DAC2ORDAC3_ENABLED
endif

ifeq ($(ENABLE_CALCU_CPU_FREQ_LOG),1)
CFLAGS_app_bt_stream.o += -DENABLE_CALCU_CPU_FREQ_LOG
CFLAGS_app_media_player.o += -DENABLE_CALCU_CPU_FREQ_LOG
endif

ifeq ($(PLAYBACK_FORCE_48K),1)
CFLAGS_app_bt_stream.o += -DPLAYBACK_FORCE_48K
endif

ifeq ($(ANC_ASSIST_USE_INT_CODEC),1)
CFLAGS_app_bt_stream.o += -DANC_ASSIST_USE_INT_CODEC
CFLAGS_app_media_player.o += -DANC_ASSIST_USE_INT_CODEC
endif

ifeq ($(AUDIO_REVERB),1)
CFLAGS_app_bt_stream.o += -D__AUDIO_REVERB__
endif

ifeq ($(AUDIO_BASS_ENHANCER),1)
CFLAGS_app_bt_stream.o += -D__AUDIO_BASS_ENHANCER__
endif

ifeq ($(BT_NO_SLEEP),1)
ccflags-y += -D__BT_NO_SLEEP__
endif

ifeq ($(APP_DEBUG_TOOL), BT_HFP_AT)
CFLAGS_app_hfp.o += -DAPP_DEBUG_TOOL_BT_HFP_AT
endif

ifeq ($(APP_AUDIO_DELAY_AFTER_OPEN_FOR_TRIGGER),1)
CFLAGS_app_bt_stream.o += -DAPP_AUDIO_DELAY_AFTER_OPEN_FOR_TRIGGER
endif

ifeq ($(CALL_BYPASS_SLAVE_TX_PROCESS),1)
CFLAGS_app_bt_stream.o += -DCALL_BYPASS_SLAVE_TX_PROCESS
endif

ifeq ($(AUDIO_OUTPUT_SW_GAIN_BEFORE_DRC),1)
CFLAGS_app_bt_stream.o += -DAUDIO_OUTPUT_SW_GAIN_BEFORE_DRC
endif

ifeq ($(PROMPT_CTRL_ANC),1)
CFLAGS_app_media_player.o += -DPROMPT_CTRL_ANC
CFLAGS_audio_prompt_sbc.o += -DPROMPT_CTRL_ANC
endif

ifeq ($(ANC_PROMPT_CTRL_MUSIC),1)
CFLAGS_app_media_player.o += -DANC_PROMPT_CTRL_MUSIC
endif

ifeq ($(VPU_CFG_ON_SENSOR_HUB),1)
CFLAGS_app_bt_stream.o += -DVPU_CFG_ON_SENSOR_HUB
endif

ifeq ($(SPEECH_ALGO_DSP),1)
CFLAGS_app_bt_stream.o += -DSPEECH_ALGO_DSP
ccflags-y += -Iapps/speech_algo_dsp
endif

ifeq ($(LOCAL_AUDIO_SUPPORT),1)
ccflags-y += -Iservices/audio_manager
endif

ifeq ($(GFPS_ENABLE),1)
subdir-ccflags-y += \
    -Iservices/gfps/inc
endif

ifeq ($(WATCH_AI_ENABLED), 1)
subdir-ccflags-y += -Iservices/watch_ai/manager
endif

ifeq ($(FINDMY_ENABLE),1)
ccflags-y += -Ithirdparty/ios-find-my/inc \
             -Iutils/kfifo
endif
