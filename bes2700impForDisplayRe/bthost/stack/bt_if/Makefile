cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

src_obj := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)a2dp/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)a2dp/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)avrcp/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)avrcp/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)conmgr/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)conmgr/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)hci/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)hci/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)l2cap/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)l2cap/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)me/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)me/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)sdp/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)sdp/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)spp/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)spp/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)hfp/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)hfp/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)hid/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)hid/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)os/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)os/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)rfcomm/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)rfcomm/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)sync/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)sync/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)besaud/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)besaud/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)dip/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)dip/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)pbap/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)pbap/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)map/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)map/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)pan/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)pan/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)opp/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)opp/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)lhdc_license/*.c))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)lhdc_license/*.cpp))
src_obj += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)sco/*.cpp))

obj_s := $(filter %.S, $(src_obj))
obj_c := $(filter %.c, $(src_obj))
obj_cpp := $(filter %.cpp, $(src_obj))

ifeq ($(IBRT), 1)
BTAPI_LIB_NAME := ibrt_libbt_api
else
BTAPI_LIB_NAME := $(CHIP)_libbt_api
endif

ifeq ($(AI_VOICE),1)
BTAPI_LIB_NAME := $(BTAPI_LIB_NAME)_ai
endif

ifeq ($(HFP_1_9_ENABLE), 1)
BTAPI_LIB_NAME := $(BTAPI_LIB_NAME)_sbc_enc
endif

ifeq ($(SBC_FUNC_IN_ROM), 1)
BTAPI_LIB_NAME := $(BTAPI_LIB_NAME)_sbc_in_rom
endif

ifeq ($(BT_RF_PREFER), 2M)
BTAPI_LIB_NAME := $(BTAPI_LIB_NAME)_2m
endif

ifeq ($(BT_RF_PREFER), 3M)
BTAPI_LIB_NAME := $(BTAPI_LIB_NAME)_3m
endif

ifeq ($(BT_ONE_BRING_TWO), 1)
BTAPI_LIB_NAME := $(BTAPI_LIB_NAME)_1b2
endif

ifeq ($(SUPPORT_REMOTE_COD), 1)
BTAPI_LIB_NAME := $(BTAPI_LIB_NAME)_cod
endif

ifeq ($(BLE), 1)
BTAPI_LIB_NAME := $(BTAPI_LIB_NAME)_ble
endif

ifeq ($(BT_HFP_SUPPORT), 1)
BTAPI_LIB_NAME := $(BTAPI_LIB_NAME)_hfp
endif

ifeq ($(USE_TRACE_ID), 1)
BTAPI_LIB_NAME := $(BTAPI_LIB_NAME)_log_compressed
endif

ifeq ($(BT_SOURCE), 1)
BTAPI_LIB_NAME := $(BTAPI_LIB_NAME)_bt_src
endif

ifeq ($(BLE_AUDIO_ENABLED), 1)
BTAPI_LIB_NAME := $(BTAPI_LIB_NAME)_aob
endif

ifeq ($(GATT_OVER_BR_EDR), 1)
BTAPI_LIB_NAME := $(BTAPI_LIB_NAME)_gatt
endif

ifeq ($(mHDT_LE_SUPPORT),1)
BTAPI_LIB_NAME := $(BTAPI_LIB_NAME)_mhdt
endif

ifeq ($(FREEMAN_ENABLED_STERO), 1)
BTAPI_LIB_NAME := $(BTAPI_LIB_NAME)_stero
endif

ifeq ($(BT_DIP_SUPPORT), 1)
BTAPI_LIB_NAME := $(BTAPI_LIB_NAME)_dip
endif

BTAPI_LIB_NAME := $(BTAPI_LIB_NAME)_ble_new_stack

ifeq ($(ARM_CMNS), 1)
BTAPI_LIB_NAME := $(BTAPI_LIB_NAME)_ARM_CMNS
endif

$(BTAPI_LIB_NAME)-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

obj-y += $(BTAPI_LIB_NAME).a

ccflags-y += -DBLUETOOTH_BT_IMPL

ccflags-y += \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BES_BT_IMPL_INCLUDES) \
    $(BLE_APP_INCLUDES) \
    $(BLE_STACK_INC_INTERNAL) \
    $(BLE_STACK_INCLUDES) \
    $(BT_SERVICE_UX_INCLUDES) \
    -Iservices/app_ai/inc \
    -Iservices/audio_process \
    -Iservices/overlay \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iservices/audioflinger \
    -Iutils/lockcqueue \
    -Iservices/bt_app \
    -Iservices/resources/ \
    -Iservices/osif \
    -Iservices/auto_test/ \
    -Iservices/audio_bt \
    -Iapps/audioplayers \
    -Iapps/main/ \
    -Iapps/key/ \
    -Iplatform/drivers/ana/ \
    -Iplatform/drivers/bt/ \
    -Iutils/intersyshci \
    -Iservices/app_bt_source/inc \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iservices/audio_manager \
    -Iservices/ai_voice/protocol/gva/gsound_target_api_read_only \
    -Iservices/ai_voice/protocol/gva/gsound_custom/inc \
    -Iservices/voicepath/gsound/gsound_target_api_read_only \
    -Iservices/voicepath/gsound/gsound_custom/inc \
    -Iservices/ble_audio_core/inc \
    -Iservices/nv_section/userdata_section \
    -Ibthost/porting \
    -Ibthost/stack/common/patch

CFLAGS_bt_if.o += -DBESLIB_INFO=$(BESLIB_INFO)

ifeq ($(A2DP_LDAC_ON),1)
ccflags-y += -Ithirdparty/audio_codec_lib/ldac/inc
endif

ifeq ($(GATT_OVER_BR_EDR),1)
ccflags-y += -I$(BLE_STACK_DIR_PATH)/ip/ble/hl/src/inc
CCflags-y += -I$(BLE_STACK_DIR_PATH)/ip/ble/hl/api
endif

ifeq ($(ANC_APP),1)
CFLAGS_besaud_api.o += -DANC_APP
endif

ifeq ($(BLE_AUDIO_ENABLED),1)
subdir-ccflags-y += \
    -Iservices/ble_audio_core/inc
endif
