cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj_tmp_gaf := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gaf_audio/*.cpp))

ifeq ($(GAF_CODEC_CROSS_CORE),1)
obj_tmp_gaf += gaf_codec_off_bth/gaf_codec_cc_common.o
endif

#aob bis
CFG_BAP_BC := 1
ifeq ($(CFG_BAP_BC), 0)
obj_cpp += $(filter-out $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gaf_audio/gaf_bis_*.*)), $(obj_tmp_gaf))
else
obj_cpp := $(obj_tmp_aob)
obj_cpp += $(obj_tmp_gaf)
endif

ifeq ($(BLE_USB_AUDIO_SUPPORT),1)
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)usb_audio/*.cpp))
endif

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

obj-y := $(src_obj)

ccflags-y += -DBLUETOOTH_BLE_IMPL

subdir-ccflags-y += \
    -I$(BT_ROM_PORTING_PATH) \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BLE_APP_INCLUDES) \
    $(BLE_STACK_INCLUDES) \
    $(BT_SERVICE_UX_INCLUDES) \
    -I$(BLE_AOB_APP_DIR_PATH)/gaf_audio \
    $(BES_MULTIMEDIA_INCLUDES) \
    -Iinclude/cmsis_dsp \
    -Iservices/bt_app \
    -Iservices/fs/fat \
    -Iservices/fs/sd \
    -Iservices/fs/fat/ChaN \
    -Iservices/overlay \
    -Iservices/resources \
    -Iservices/audio_bt \
    -Iservices/lea_player/inc \
    -Iservices/multimedia/audio/process/resample/include \
    -Iservices/multimedia/audio/process/filters/include \
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
    -Iapps/key \
    -Iapps/main \
    -Iapps/common \
    -Iapps/audioplayers \
    -Iapps/audioplayers/a2dp_decoder \
    -Iapps/anc/inc \
    -Iapps/anc_assist/inc \
    -Iapps/anc_assist/src/assist \
    -Iapps/factory \
    -Iapps/btapp/ble_usb_audio \
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
    -Iservices/multimedia/speech/inc \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iservices/voicepath/gsound/gsound_target \
    -Iapps/earbuds/conn/ \
    -Iservices/ota \
    -Iservices/audioflinger \
    -Iservices/norflash_api \
    -Iutils/list \
    -Iapps/bt_sync \
    -Iservices/bt_sync \
    -Iservices/audio_manager \
    -Iapps/dsp_m55 \
    -Ibthost/service/ble_audio/aob_app/gaf_audio \
    -Iplatform/drivers/hifi4 \
    -Iservices/multimedia/audio/process/sbcplc/include \
    -Iservices/multimedia/audio/process/musicplc

ifeq ($(BLE_USB_AUDIO_SUPPORT),1)
subdir-ccflags-y += \
    -Iservices/lea_player/usb_audio
endif

ifeq ($(IBRT),1)
ifeq ($(IBRT_UI),1)
subdir-ccflags-y += \
    -Iservices/ibrt/app_ibrt/inc \
    -Iservices/ibrt/ibrt_ui/inc \
    -Iservices/ibrt/ibrt_core/inc
endif
endif

BLE_SMF ?= 0
ifeq ($(BLE_SMF),1)
subdir-ccflags-y += -DBLE_SMF
endif

BLE_LC3 ?= 0
ifeq ($(BLE_LC3),1)
subdir-ccflags-y += -DBLE_LC3
endif

ifeq ($(GAF_LC3_BES_PLC_ON),1)
KBUILD_CPPFLAGS += -DGAF_LC3_BES_PLC_ON
else ifeq ($(GAF_LC3_MUSIC_PLC_ON),1)
KBUILD_CPPFLAGS += -DGAF_LC3_MUSIC_PLC_ON
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