cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/adapter_service.c))

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/bluetooth_nv_mgr.c))

ifneq ($(BLE_ONLY_ENABLED),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/bt_api_export.c))
endif

ifeq ($(BLE),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/ble_api_export.c))
endif

obj-y := $(obj_c:.c=.o)

obj-y += test/

ccflags-y += \
    -Iservices/audio_bt \
	$(AAC_INCLUDES) \
	-Iinclude/cmsis_dsp \
	-Iservices/voice_dev \
	-Iservices/audio_process \
	-Iservices/audio_dump/include \
	-Iservices/hw_dsp/inc \
	-Ibthost/rom/ \
	-Ibthost/stack/ble_if/inc \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	$(BES_BT_IMPL_INCLUDES) \
	$(BLE_APP_INCLUDES) \
	$(BLE_STACK_INCLUDES) \
	$(BLE_STACK_INC_INTERNAL) \
	$(BT_SERVICE_UX_INCLUDES) \
	-Imultimedia/inc/speech/inc \
	-Iservices/bone_sensor \
	-Iservices/overlay \
	-Iservices/audio_manager \
	-Ithirdparty/tile/tile_common/tile_storage \
	-Ithirdparty/tile/tile_common/tile_features \
	-Iservices/resources \
	-Imultimedia/inc/rbcodec \
	-Imultimedia/inc/audio/process/resample/include \
	-Imultimedia/inc/audio/process/filters/include \
	-Imultimedia/inc/audio/process/drc/include \
	-Imultimedia/inc/audio/process/anc/include\
	-Imultimedia/inc/audio/process/sidetone/include\
	-Imultimedia/inc/audio/process/adj_mc/inc\
	-Iservices/nv_section/aud_section \
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
	-Iutils/cqueue \
	-Iutils/heap \
	-Iutils/crc16 \
	-Iutils/rom_utils \
	-Iutils/cfifo \
	-Iservices/audioflinger \
	-Iutils/lockcqueue \
	-Iutils/intersyshci \
	-Iapps/anc/inc \
	-Iapps/key \
	-Iapps/main \
	-Iapps/common \
	-Iapps/audioplayers \
	-Iapps/audioplayers/a2dp_decoder \
	-Iapps/battery \
	-Iapps/common \
	-Iapps/factory \
	-Iapps/voice_assist/inc \
	-Iapps/app_test/ble_audio_test \
	-Iapps/ble_audio_central_app/ble_audio_usb \
	-Iutils/hwtimer_list \
	-Iservices/voicepath \
	-Ithirdparty/userapi \
	-Ithirdparty/audio_codec_lib/liblhdc-dec/inc \
	-Iservices/ai_voice/manager \
	-Iservices/ai_voice/audio \
	-Iservices/ai_voice/transport \
	-Iservices/app_ai/inc \
	-Iservices/interconnection/red \
	-Iservices/interconnection/green \
	-Iservices/interconnection/umm_malloc \
	-Iservices/lea_player/inc \
	$(SOURCE_INCLUDES) \
	-Ithirdparty/tile/tile_target \
	-Iservices/osif \
	-Iutils/list \
	-Iutils/string \
	-Iservices/ai_voice/protocol/bixbyvoice \
	-Iservices/ai_voice/protocol/bixbyvoice/bixbyvoice_manager \
	-Iservices/bt_watch_service/inc \
	-Iservices/gfps/inc \
	-Iservices/bt_watch_service/inc \
	-Iapps/earbuds/conn \
	-Iservices/bt_sync \
	-Iapps/bt_sync/ \
	-Iservices/audio_manager/audio_policy/bredr_policy/inc \
	-Ibthost/service/bt_app/inc \
	-Iservices/lea_player/inc \
	-Iutils/rom_utils/$(CHIP) \
	-I$(BT_ROM_PATCH_PATH) \
	-Ibthost/porting \
	-Ibthost/service/ble_app_new/inc  

ifeq ($(GFPS_ENABLE),1)
ccflags-y += \
	-Iservices/gfps/inc
endif