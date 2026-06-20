cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj_tmp_aob := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.cpp))


#aob bis
ifeq ($(CFG_BAP_BC), 0)
obj_cpp := $(filter-out $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/aob_bis_*.*)), $(obj_tmp_aob))
else
obj_cpp := $(obj_tmp_aob)
obj_cpp += $(obj_tmp_gaf)
endif

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

obj-y := $(src_obj)

ccflags-y += -DBLUETOOTH_BLE_IMPL

subdir-ccflags-y += \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BLE_APP_INCLUDES) \
    $(BLE_STACK_INCLUDES) \
    $(BLE_STACK_INC_INTERNAL) \
    -Iinclude/cmsis_dsp \
    -Iservices/bt_app \
    -Iservices/fs/fat \
    -Iservices/fs/sd \
    -Iservices/fs/fat/ChaN \
    -Iservices/overlay \
    -Iservices/resources \
    -Iservices/audio_bt \
    -Iservices/lea_player/inc \
    $(BT_SERVICE_UX_INCLUDES) \
    -Imultimedia/inc/audio/process/resample/include \
    -Imultimedia/inc/audio/process/filters/include \
    -Iservices/nv_section/factory_section \
    -Iservices/nv_section/fpga_section \
    -Iplatform/drivers/uarthci \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/cp_accel \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iservices/bone_sensor \
    -Iservices/audioflinger \
    -Iservices/audio_process \
    -Iservices/audio_dump/include \
    -Iutils/lockcqueue \
    -Iutils/intersyshci \
    -Iutils/cfifo \
    -Iapps/key \
    -Iapps/main \
    -Iapps/common \
    -Iapps/audioplayers \
    -Iapps/audioplayers/a2dp_decoder \
    -Iapps/app_test/ble_audio_test \
    -Iapps/anc/inc \
    -Iapps/anc_assist/inc \
    -Iapps/anc_assist/src/assist \
    -Iapps/factory \
    -Iservices/interconnection/green \
    -Iutils/hwtimer_list \
    -Ithirdparty/audio_codec_lib/liblhdc-dec/inc \
    -Iservices/nv_section/log_section \
    -Iservices/ai_voice/manager \
    -Iservices/app_ai/inc \
    -Iapps/anc/src/assist \
    -Iapps/anc_assist/inc \
    -Iapps/anc_assist/src/assist/ \
    -Iapps/battery/ \
    -Iutils/crc16 \
    -Iutils/crc32 \
    -Iutils/signal_generator \
    -Iutils/rom_utils \
    $(BES_MULTIMEDIA_INCLUDES) \
    -Imultimedia/inc/speech/inc \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iservices/voicepath/gsound/gsound_target \
    -Iservices/ota \
    -Iservices/audioflinger \
    -Iservices/norflash_api \
    -Iutils/list \
    -Iapps/bt_sync \
    -Iservices/bt_sync \
    -Iservices/audio_manager \
    -Iservices/ble_audio/ble_audio_core/inc \
    -Iservices/ble_audio/ble_audio_test \
    -Iapps/dsp_m55

BLE_SMF ?= 0
ifeq ($(BLE_SMF),1)
subdir-ccflags-y += -DBLE_SMF
endif

BLE_LC3 ?= 0
ifeq ($(BLE_LC3),1)
subdir-ccflags-y += -DBLE_LC3
endif

ifeq ($(BINAURAL_RECORD_PROCESS),1)
subdir-ccflags-y += -Iservices/binaural_record_process
CFLAGS_gaf_media_stream.o += -DBINAURAL_RECORD_PROCESS
CFLAGS_gaf_stream_process.o += -DBINAURAL_RECORD_PROCESS
endif

ifeq ($(CODEC_DAC_MULTI_VOLUME_TABLE),1)
CFLAGS_gaf_media_stream.o += -DCODEC_DAC_MULTI_VOLUME_TABLE
CFLAGS_aob_volume_api.o += -DCODEC_DAC_MULTI_VOLUME_TABLE
endif
