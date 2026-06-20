cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

include $(cur_dir)/config.mk

export BLE_IF_DIR_PATH := bthost/stack/ble_if

export BLE_APP_DIR_PATH ?= bthost/service/ble_app_new
export BLE_GAF_CORE_DIR_PATH ?= bthost/service/ble_audio/blueelf_adapter
export BLE_STACK_DIR_PATH ?= bthost/stack/ble_stack_new

export BLE_STACK_INC_INTERNAL := \
    -I$(BLE_STACK_DIR_PATH)/inc/hci \
    -I$(BLE_STACK_DIR_PATH)/inc/l2cap \
    -I$(BLE_STACK_DIR_PATH)/inc/gap \
    -I$(BLE_STACK_DIR_PATH)/inc/gatt \
    -I$(BLE_STACK_DIR_PATH)/inc/gaf \
    -I$(BLE_STACK_DIR_PATH)/inc/iso \
    -I$(BLE_STACK_DIR_PATH)/inc/profiles

export BES_STACK_ADAPTER_INCLUDES := \
	-Ibthost/adapter/inc/adapter_service \
	-Ibthost/adapter/inc/bt \
	-Ibthost/adapter/inc/ble \
	-Ibthost/adapter/inc/bt/source \
	-Ibthost/adapter/inc/bt/me \
	-Ibthost/adapter/inc/bt/l2cap \
	-Ibthost/adapter/inc/bt/a2dp \
	-Ibthost/adapter/inc/bt/avrcp \
	-Ibthost/adapter/inc/bt/hfp \
	-Ibthost/adapter/inc/bt/ibrt \
	-Ibthost/adapter/inc/bt/dip \
	-Ibthost/adapter/inc/bt/common \
	-Ibthost/adapter/inc/ble/common

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj-y := $(obj_c:.c=.o)

ifeq ($(BT_BUILD_WITH_CUSTOMER_HOST),1)
obj-y += \
    adapter/
else
obj-y += \
    adapter/ \
    service/ \
    porting/

ifeq ($(BTH_IN_ROM),1)
obj-y += rom/
else
obj-y += stack/
endif
endif

ifeq ($(BTH_SUBSYS_ONLY),1)
KBUILD_CPPFLAGS += -DBTH_SUBSYS_ONLY
endif

ccflags-y += -DBLUETOOTH_BT_IMPL
ccflags-y += -DBLUETOOTH_BLE_IMPL

ccflags-y += \
	$(AAC_INCLUDES) \
	-Iinclude/cmsis_dsp \
	-Iservices/voice_dev \
	-Iservices/audio_process \
	-Iservices/audio_dump/include \
	-Iservices/hw_dsp/inc \
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
	$(SOURCE_INCLUDES) \
	-Ithirdparty/tile/tile_target \
	-Iservices/osif \
	-Iutils/list \
	-Iutils/string \
	-Iservices/ai_voice/protocol/bixbyvoice \
	-Iservices/ai_voice/protocol/bixbyvoice/bixbyvoice_manager \
	-Iservices/bt_watch_service/inc
